"""An in-process kernel"""

# Copyright (c) yap_ipython Development Team.
# Distributed under the terms of the Modified BSD License.

from contextlib import contextmanager
import logging
import sys

from yap_ipython.core.interactiveshell import InteractiveShellABC
from yap_kernel.jsonutil import json_clean
from traitlets import Any, Enum, Instance, List, Type, default
from yap_kernel.ipkernel import YAPKernel
from yap_kernel.zmqshell import ZMQInteractiveShell

from .constants import INPROCESS_KEY
from .socket import DummySocket
from ..iostream import OutStream, BackgroundSocket, IOPubThread

#-----------------------------------------------------------------------------
# Main kernel class
#-----------------------------------------------------------------------------

class InProcessKernel(YAPKernel):

    #-------------------------------------------------------------------------
    # InProcessKernel interface
    #-------------------------------------------------------------------------

    # The frontends connected to this kernel.
    frontends = List(
        Instance('yap_kernel.inprocess.client.InProcessKernelClient',
                 allow_none=True)
    )

    # The GUI environment that the kernel is running under. This need not be
    # specified for the normal operation for the kernel, but is required for
    # yap_ipython's GUI support (including pylab). The default is 'inline' because
    # it is safe under all GUI toolkits.
    gui = Enum(('tk', 'gtk', 'wx', 'qt', 'qt4', 'inline'),
               default_value='inline')

    raw_input_str = Any()
    stdout = Any()
    stderr = Any()

    #-------------------------------------------------------------------------
    # Kernel interface
    #-------------------------------------------------------------------------

    shell_class = Type(allow_none=True)
    shell_streams = List()
    control_stream = Any()
    _underlying_iopub_socket = Instance(DummySocket, ())
    iopub_thread = Instance(IOPubThread)

    @default('iopub_thread')
    def _default_iopub_thread(self):
        thread = IOPubThread(self._underlying_iopub_socket)
        thread.start()
        return thread

    iopub_socket = Instance(BackgroundSocket)

    @default('iopub_socket')
    def _default_iopub_socket(self):
        return self.iopub_thread.background_socket

    stdin_socket = Instance(DummySocket, ())

    def __init__(self, **traits):
        super(InProcessKernel, self).__init__(**traits)

        self._underlying_iopub_socket.observe(self._io_dispatch, names=['message_sent'])
        self.shell.kernel = self

    def execute_request(self, stream, ident, parent):
        """ Override for temporary IO redirection. """
        with self._redirected_io():
            super(InProcessKernel, self).execute_request(stream, ident, parent)

    def start(self):
        """ Override registration of dispatchers for streams. """
        self.shell.exit_now = False

    def _abort_queue(self, stream):
        """ The in-process kernel doesn't abort requests. """
        pass

    def _input_request(self, prompt, ident, parent, password=False):
        # Flush output before making the request.
        self.raw_input_str = None
        sys.stderr.flush()
        sys.stdout.flush()

        # Send the input request.
        content = json_clean(dict(prompt=prompt, password=password))
        msg = self.session.msg(u'input_request', content, parent)
        for frontend in self.frontends:
            if frontend.session.session == parent['header']['session']:
                frontend.stdin_channel.call_handlers(msg)
                break
        else:
            logging.error('No frontend found for raw_input request')
            return str()

        # Await a response.
        while self.raw_input_str is None:
            frontend.stdin_channel.process_events()
        return self.raw_input_str

    #-------------------------------------------------------------------------
    # Protected interface
    #-------------------------------------------------------------------------

    @contextmanager
    def _redirected_io(self):
        """ Temporarily redirect IO to the kernel.
        """
        sys_stdout, sys_stderr = sys.stdout, sys.stderr
        sys.stdout, sys.stderr = self.stdout, self.stderr
        yield
        sys.stdout, sys.stderr = sys_stdout, sys_stderr

    #------ Trait change handlers --------------------------------------------

    def _io_dispatch(self, change):
        """ Called when a message is sent to the IO socket.
        """
        ident, msg = self.session.recv(self.iopub_socket, copy=False)
        for frontend in self.frontends:
            frontend.iopub_channel.call_handlers(msg)

    #------ Trait initializers -----------------------------------------------

    @default('log')
    def _default_log(self):
        return logging.getLogger(__name__)

    @default('session')
    def _default_session(self):
        from jupyter_client.session import Session
        return Session(parent=self, key=INPROCESS_KEY)

    @default('shell_class')
    def _default_shell_class(self):
        return InProcessInteractiveShell

    @default('stdout')
    def _default_stdout(self):
        return OutStream(self.session, self.iopub_thread, u'stdout')

    @default('stderr')
    def _default_stderr(self):
        return OutStream(self.session, self.iopub_thread, u'stderr')

#-----------------------------------------------------------------------------
# Interactive shell subclass
#-----------------------------------------------------------------------------

class InProcessInteractiveShell(ZMQInteractiveShell):

    kernel = Instance('yap_kernel.inprocess.ipkernel.InProcessKernel',
                      allow_none=True)

    #-------------------------------------------------------------------------
    # InteractiveShell interface
    #-------------------------------------------------------------------------

    def enable_gui(self, gui=None):
        """Enable GUI integration for the kernel."""
        from yap_kernel.eventloops import enable_gui
        if not gui:
            gui = self.kernel.gui
        enable_gui(gui, kernel=self.kernel)
        self.active_eventloop = gui


    def enable_matplotlib(self, gui=None):
        """Enable matplotlib integration for the kernel."""
        if not gui:
            gui = self.kernel.gui
        return super(InProcessInteractiveShell, self).enable_matplotlib(gui)

    def enable_pylab(self, gui=None, import_all=True, welcome_message=False):
        """Activate pylab support at runtime."""
        if not gui:
            gui = self.kernel.gui
        return super(InProcessInteractiveShell, self).enable_pylab(gui, import_all,
                                                            welcome_message)

InteractiveShellABC.register(InProcessInteractiveShell)
