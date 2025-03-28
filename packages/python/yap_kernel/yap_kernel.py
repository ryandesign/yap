from __future__ import print_function

import signal
import yap4py.yapi
import io
import getpass
import sys
import traceback

from yap_ipython.core import release
from ipython_genutils.py3compat import builtin_mod, PY3, unicode_type, safe_unicode
from IPython.utils.tokenutil import token_at_cursor, line_at_cursor
from traitlets import Instance, Type, Any, List

from ipykernel.comm import CommManager
from ipykernel.kernelbase import Kernel as KernelBase
from ipykernel.zmqshell import ZMQInteractiveShell
from .interactiveshell import YAPInteractiveShell
from IPython.core.interactiveshell import InteractiveShellABC, InteractiveShell
from contextlib import redirect_stdout

major=release._version_major
minor=release._version_minor

kernel_json = {
    "argv":  [sys.executable,
             "-m", "yap_kernel",
             "-f", "{connection_file}"],
    "display_name": " YAP -"+major+"."+minor,
    "language": "prolog",
    "name": "yap_kernel",
}


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


class YAPKernel(KernelBase):
    shell = Instance('IPython.core.interactiveshell.InteractiveShellABC',
                     allow_none=True)
    shell_class = Type(ZMQInteractiveShell )
    user_ns = Instance(dict, args=None, allow_none=True)
    def _user_ns_changed(self, name, old, new):
        if self.shell is not None:
            self.shell.user_ns = new
            self.shell.init_user_ns()

    # A reference to the Python builtin 'raw_input' function.
    # (i.e., __builtin__.raw_input for Python 2.7, builtins.input for Python 3)
    _sys_raw_input = Any()
    _sys_eval_input = Any()


    implementation = 'YAP Kernel'
    implementation_version = '1.0'
    language = 'text'
    language_version = major+'.'+minor
    banner = "YAP-"+major+"."+minor
    language_info = {
        'mimetype': 'text/prolog',
        'name': 'text',
        # ------ If different from 'language':
        'codemirror_mode': {
            "version": 2,
            "name": "prolog"
        },
        'pygments_lexer': 'prolog',
        'version': "0.0.1",
        'file_extension': '.yap',
    }


#-------------------------------------------------------------------------
    # Things related to history management
    #-------------------------------------------------------------------------


    def __init__(self, **kwargs):
        # sp = super(YAPKernel, self)
        super(YAPKernel, self).__init__(**kwargs)
        # Initialize the InteractiveShell subclass
        self.shell = self.shell_class.instance(parent=self,
            profile_dir = self.profile_dir,
            user_ns     = self.user_ns,
            kernel      = self,
        )
        self.shell.displayhook.session = self.session
        self.shell.displayhook.pub_socket = self.iopub_socket
        self.shell.displayhook.topic = self._topic('execute_result')
        self.shell.display_pub.session = self.session
        self.shell.display_pub.pub_socket = self.iopub_socket

        self.comm_manager = CommManager(parent=self, kernel=self)

#        self.shell._last_traceback  = None
        self.shell.configurables.append(self.comm_manager)
        comm_msg_types = [ 'comm_open', 'comm_msg', 'comm_close' ]
        for msg_type in comm_msg_types:
            self.shell_handlers[msg_type] = getattr(self.comm_manager, msg_type)
        self.yap_shell = YAPInteractiveShell( self )


    def get_usage(self):
        return "This is the YAP kernel."

    help_links = List([
        {
            'text': "Python",
            'url': "http://docs.python.org/%i.%i" % sys.version_info[:2],
        },
        {
            'text': "YAP",
            'url': "http://YAP.org/documentation.html",
        },
        {
            'text': "NumPy",
            'url': "http://docs.scipy.org/doc/numpy/reference/",
        },
        {
            'text': "SciPy",
            'url': "http://docs.scipy.org/doc/scipy/reference/",
        },
        {
            'text': "Matplotlib",
            'url': "http://matplotlib.org/contents.html",
        },
        {
            'text': "SymPy",
            'url': "http://docs.sympy.org/latest/index.html",
        },
        {
            'text': "pandas",
            'url': "http://pandas.pydata.org/pandas-docs/stable/",
        },
    ]).tag(config=True)

    # Kernel info fields
    implementation = 'YAP'
    implementation_version = release.version
    language_info = {
        'name': 'python',
        'version': sys.version.split()[0],
        'mimetype': 'text/x-python',
        'codemirror_mode': {
            'name': 'prolog',
            'version': sys.version_info[0]
        },
        'pygments_lexer': 'prolog',
        'nbconvert_exporter': 'python',
        'file_extension': '.yap'
    }

    @property
    def banner(self):
        return self.shell.banner

    def start(self):
        self.shell.exit_now = False
        super(YAPKernel, self).start()

    def set_parent(self, ident, parent):
        """Overridden from parent to tell the display hook and output streams
        about the parent message.
        """
        super(YAPKernel, self).set_parent(ident, parent)
        self.shell.set_parent(parent)

    def init_metadata(self, parent):
        """Initialize metadata.

        Run at the beginning of each execution request.
        """
        md = super(YAPKernel, self).init_metadata(parent)
        # FIXME: remove deprecated ipyparallel-specific code
        # This is required for ipyparallel < 5.0
        md.update({
            'dependencies_met' : True,
            'engine' : self.ident,
        })
        return md

    def finish_metadata(self, parent, metadata, reply_content):
        """Finish populating metadata.

        Run after completing an execution request.
        """
        # FIXME: remove deprecated ipyparallel-specific code
        # This is required by ipyparallel < 5.0
        metadata['status'] = reply_content['status']
        if reply_content['status'] == 'error' and reply_content['ename'] == 'UnmetDependency':
                metadata['dependencies_met'] = False

        return metadata

    def _forward_input(self, allow_stdin=False):
        """Forward raw_input and getpass to the current frontend.

        via input_request
        """
        self._allow_stdin = allow_stdin

        if PY3:
            self._sys_raw_input = builtin_mod.input
            builtin_mod.input = self.raw_input
        else:
            self._sys_raw_input = builtin_mod.raw_input
            self._sys_eval_input = builtin_mod.input
            builtin_mod.raw_input = self.raw_input
            builtin_mod.input = lambda prompt='': eval(self.raw_input(prompt))
        self._save_getpass = getpass.getpass
        getpass.getpass = self.getpass

    def _restore_input(self):
        """Restore raw_input, getpass"""
        if PY3:
            builtin_mod.input = self._sys_raw_input
        else:
            builtin_mod.raw_input = self._sys_raw_input
            builtin_mod.input = self._sys_eval_input

        getpass.getpass = self._save_getpass

    @property
    def execution_count(self):
        return self.shell.execution_count

    @execution_count.setter
    def execution_count(self, value):
        # Ignore the incrememnting done by KernelBase, in favour of our shell's
        # execution counter.
        pass

    def do_execute(self, code, silent, store_history=True,
                   user_expressions=None, allow_stdin=False):
        shell = self.shell # we'll need this a lot here

        self._forward_input(allow_stdin)

        reply_content = {}
        try:
            res = shell.run_cell(code, store_history=store_history, silent=silent)
        finally:
            self._restore_input()

        if res.error_before_exec is not None:
            err = res.error_before_exec
        else:
            err = res.error_in_exec

        if res.success:
            reply_content[u'status'] = u'ok'
        elif isinstance(err, KeyboardInterrupt):
            reply_content[u'status'] = u'aborted'
        else:
            reply_content[u'status'] = u'error'

            reply_content.update({
                #      u'traceback': shell._last_traceback or [],
                u'ename': unicode_type(type(err).__name__),
                u'evalue': safe_unicode(err),
            })

            # FIXME: deprecate piece for ipyparallel:
            e_info = dict(engine_uuid=self.ident, engine_id=self.int_id,
                          method='execute')
            reply_content['engine_info'] = e_info


        # Return the execution counter so clients can display prompts
        reply_content['execution_count'] = shell.execution_count - 1

        if 'traceback' in reply_content:
            self.log.info("Exception in execute request:\n%s", '\n'.join(reply_content['traceback']))


        # At this point, we can tell whether the main code execution succeeded
        # or not.  If it did, we proceed to evaluate user_expressions
        if reply_content['status'] == 'ok':
            reply_content[u'user_expressions'] = \
                         shell.user_expressions(user_expressions or {})
        else:
            # If there was an error, don't even try to compute expressions
            reply_content[u'user_expressions'] = {}

        # Payloads should be retrieved regardless of outcome, so we can both
        # recover partial output (that could have been generated early in a
        # block, before an error) and always clear the payload system.
        reply_content[u'payload'] = shell.payload_manager.read_payload()
        # Be aggressive about clearing the payload because we don't want
        # it to sit in memory until the next execute_request comes in.
        shell.payload_manager.clear_payload()

        return reply_content

    def do_complete(self, code, cursor_pos):
        # FIXME: YAP completers currently assume single line,
        # but completion messages give multi-line context
        # For now, extract line from cell, based on cursor_pos:
        if cursor_pos is None:
            cursor_pos = len(code)
        line, offset = line_at_cursor(code, cursor_pos)
        line_cursor = cursor_pos - offset

        txt, matches = self.shell.complete('', line, line_cursor)
        return {'matches' : matches,
                'cursor_end' : cursor_pos,
                'cursor_start' : cursor_pos - len(txt),
                'metadata' : {},
                'status' : 'ok'}

    def do_inspect(self, code, cursor_pos, detail_level=0):
        name = token_at_cursor(code, cursor_pos)
        info = self.shell.object_inspect(name)

        reply_content = {'status' : 'ok'}
        reply_content['data'] = data = {}
        reply_content['metadata'] = {}
        reply_content['found'] = info['found']
        if info['found']:
            info_text = self.shell.object_inspect_text(
                name,
                detail_level=detail_level,
            )
            data['text/plain'] = info_text

        return reply_content

    def do_history(self, hist_access_type, output, raw, session=0, start=0,
                   stop=None, n=None, pattern=None, unique=False):
        if hist_access_type == 'tail':
            hist = self.shell.history_manager.get_tail(n, raw=raw, output=output,
            include_latest=True)

        elif hist_access_type == 'range':
            hist = self.shell.history_manager.get_range(session, start, stop,
                                                        raw=raw, output=output)

        elif hist_access_type == 'search':
            hist = self.shell.history_manager.search(
                pattern, raw=raw, output=output, n=n, unique=unique)
        else:
            hist = []

        return {
            'status': 'ok',
            'history' : list(hist),
        }

    def do_shutdown(self, restart):
        self.shell.exit_now = True
        return dict(status='ok', restart=restart)

    def do_is_complete(self, code):
        status, indent_spaces = self.shell.input_transformer_manager.check_complete(code)
        r = {'status': status}
        if status == 'incomplete':
            r['indent'] = ' ' * indent_spaces
        return r

    def do_apply(self, content, bufs, msg_id, reply_metadata):
        from .serialize import serialize_object, unpack_apply_message
        shell = self.shell
        try:
            working = shell.user_ns

            prefix = "_"+str(msg_id).replace("-","")+"_"

            f,args,kwargs = unpack_apply_message(bufs, working, copy=False)

            fname = getattr(f, '__name__', 'f')

            fname = prefix+"f"
            argname = prefix+"args"
            kwargname = prefix+"kwargs"
            resultname = prefix+"result"

            ns = { fname : f, argname : args, kwargname : kwargs , resultname : None }
            # print ns
            working.update(ns)
            code = "%s = %s(*%s,**%s)" % (resultname, fname, argname, kwargname)
            try:
                exec(code, shell.user_global_ns, shell.user_ns)
                result = working.get(resultname)
            finally:
                for key in ns:
                    working.pop(key)

            result_buf = serialize_object(result,
                buffer_threshold=self.session.buffer_threshold,
                item_threshold=self.session.item_threshold,
            )

        except BaseException as e:
            # invoke YAP traceback formatting
            shell.showtraceback()
            reply_content = {
                u'traceback': shell._last_traceback or [],
                u'ename': unicode_type(type(e).__name__),
                u'evalue': safe_unicode(e),
            }
            # FIXME: deprecate piece for ipyparallel:
            e_info = dict(engine_uuid=self.ident, engine_id=self.int_id, method='apply')
            reply_content['engine_info'] = e_info

            self.send_response(self.iopub_socket, u'error', reply_content,
                                ident=self._topic('error'))
            self.log.info("Exception in apply request:\n%s", '\n'.join(reply_content['traceback']))
            result_buf = []
            reply_content['status'] = 'error'
        else:
            reply_content = {'status' : 'ok'}

        return reply_content, result_buf

    def do_clear(self):
        self.shell.reset(False)
        return dict(status='ok')
