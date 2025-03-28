s/*************************************************************************
*									 *
*	 YAP Prolog 							 *
*									 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		control.yap						 *
* Last rev:     20/08/09						 *
* mods:									 *
* comments:	control predicates available in yap			 *
*									 *
*************************************************************************/

/**
 * @file   control.yap
 * @author VITOR SANTOS COSTA <vsc@VITORs-MBP.lan>
 * @date   Thu Nov 19 10:26:35 2015
 *
 * @brief  Control Predicates
 *
 *
*/

:- system_module( '$_control', [at_halt/1,
        b_getval/2,
        break/0,
        call/2,
        call/3,
        call/4,
        call/5,
        call/6,
        call/7,
        call/8,
        call/9,
        call/10,
        call/11,
        call/12,
        call_cleanup/2,
        call_cleanup/3,
        forall/2,
        garbage_collect/0,
        garbage_collect_atoms/0,
        gc/0,
        grow_heap/1,
        grow_stack/1,
        halt/0,
        halt/1,
        if/3,
        ignore/1,
        nb_getval/2,
        nogc/0,
        notrace/1,
        once/1,
        prolog_current_frame/1,
        prolog_initialization/1,
        setup_call_catcher_cleanup/4,
        setup_call_cleanup/3,
        version/0,
        version/1], ['$run_atom_goal'/1,
        '$set_toplevel_hook'/1]).

:- use_system_module( '$_boot', ['$call'/4,
        '$disable_debugging'/0,
        '$do_live'/0,
        '$enable_debugging'/0,
        '$system_catch'/4,
        '$version'/0]).

:- use_system_module( '$_debug', ['$init_debugger'/0]).

:- use_system_module( '$_errors', ['$do_error'/2]).

:- use_system_module( '$_utils', ['$getval_exception'/3]).

:- use_system_module( '$coroutining', [freeze_goal/2]).

/**


@addtogroup YAPControl

%% @{

*/

'$comma'(PA,A,PB,B) :-
   '$exec'(PA,A),
   '$exec'(PB,B).

/** @pred  forall(: _Cond_,: _Action_)


For all alternative bindings of  _Cond_  _Action_ can be
proven. The example verifies that all arithmetic statements in the list
 _L_ are correct. It does not say which is wrong if one proves wrong.

~~~~~{.prolog}
?- forall(member(Result = Formula, [2 = 1 + 1, 4 = 2 * 2]),
                 Result =:= Formula).
~~~~~


*/
/** @pred forall(+ _Cond_,+ _Action_)




For all alternative bindings of  _Cond_  _Action_ can be proven.
The next example verifies that all arithmetic statements in the list
 _L_ are correct. It does not say which is wrong if one proves wrong.

~~~~~
?- forall(member(Result = Formula, [2 = 1 + 1, 4 = 2 * 2]),
                 Result =:= Formula).
~~~~~



*/
forall(Cond, Action) :- \+((Cond, \+(Action))).

/** @pred  ignore(: _Goal_)


Calls  _Goal_ as once/1, but succeeds, regardless of whether
`Goal` succeeded or not. Defined as:

~~~~~{.prolog}
ignore(Goal) :-
        Goal, !.
ignore(_).
~~~~~


*/
ignore(Goal) :- (Goal->true;true).

notrace(G) :-
	 strip_module(G, M, G1),
	 ( '$$save_by'(CP),
	   '$debug_stop'( State ),
	   '$call'(G1, CP, G, M),
	   '$$save_by'(CP2),
	   (CP == CP2 -> ! ; '$debug_state'( NState ), ( true ; '$debug_restart'(NState), fail ) ),
	   '$debug_restart'( State )
     ;
	'$debug_restart'( State ),
	fail
    ).

/** @pred  if(? _G_,? _H_,? _I_)

Call goal  _H_ once per each solution of goal  _H_. If goal
 _H_ has no solutions, call goal  _I_.

The built-in `if/3` is similar to `->/3`, with the difference
that it will backtrack over the test goal. Consider the following
small data-base:

~~~~~{.prolog}
a(1).        b(a).          c(x).
a(2).        b(b).          c(y).
~~~~~

Execution of an `if/3` query will proceed as follows:

~~~~~{.prolog}
   ?- if(a(X),b(Y),c(Z)).

X = 1,
Y = a ? ;

X = 1,
Y = b ? ;

X = 2,
Y = a ? ;

X = 2,
Y = b ? ;

no
~~~~~

The system will backtrack over the two solutions for `a/1` and the
two solutions for `b/1`, generating four solutions.

Cuts are allowed inside the first goal  _G_, but they will only prune
over  _G_.

If you want  _G_ to be deterministic you should use if-then-else, as
it is both more efficient and more portable.

*/
if(X,Y,Z) :-
    	 CP0 is '$last_choice_pt',
	(
	 CP1 is '$last_choice_pt',
	 '$call'(X,CP1,if(X,Y,Z),M),
	 '$execute'(X),
	 '$clean_ifcp'(CP0,CP1),
	 '$call'(Y,CP0,if(X,Y,Z),M)
	;
	 '$call'(Z,CP0,if(X,Y,Z),M)
	).

/** @pred  call(+ _Closure_,...,? _Ai_,...) is iso


Meta-call where  _Closure_ is a closure that is converted into a goal by
appending the  _Ai_ additional arguments. The number of arguments varies
between 0 and 10.


*/

/** @pred call_cleanup(: _Goal_, : _CleanUpGoal_)

This is similar to call_cleanup/1 but with an additional
 _CleanUpGoal_ which gets called after  _Goal_ is finished.

*/
call_cleanup(Goal, Cleanup) :-
'$gated_call'( false , Goal,_Catcher, Cleanup)  .

call_cleanup(Goal, Catcher, Cleanup) :-
'$gated_call'( false , Goal, Catcher, Cleanup)  .

/** @pred setup_call_cleanup(: _Setup_,: _Goal_, : _CleanUpGoal_)


Calls `(Setup, Goal)`. For each sucessful execution of _Setup_,
calling _Goal_, the cleanup handler _Cleanup_ is guaranteed to be
called exactly once.  This will happen after _Goal_ completes, either
through failure, deterministic success, commit, or an exception.
_Setup_ will contain the goals that need to be protected from
asynchronous interrupts such as the ones received from
`call_with_time_limit/2` or thread_signal/2.  In most uses, _Setup_
will perform temporary side-effects required by _Goal_ that are
finally undone by _Cleanup_.

*/

setup_call_cleanup(Setup,Goal, Cleanup) :-
	setup_call_catcher_cleanup(Setup, Goal, _Catcher, Cleanup).

setup_call_catcher_cleanup(Setup, Goal, Catcher, Cleanup) :-
    '$setup_call_catcher_cleanup'(Setup),
	call_cleanup(Goal, Catcher, Cleanup).


/** @pred  call_with_args(+ _Name_,...,? _Ai_,...)


Meta-call where  _Name_ is the name of the procedure to be called and
the  _Ai_ are the arguments. The number of arguments varies between 0
and 10. New code should use `call/N` for better portability.

If  _Name_ is a complex term, then call_with_args/n behaves as
call/n:

~~~~~{.prolog}
call(p(X1,...,Xm), Y1,...,Yn) :- p(X1,...,Xm,Y1,...,Yn).
~~~~~


*/

%%% Some "dirty" predicates

% Only efective if yap compiled with -DDEBUG
% this predicate shows the code produced by the compiler
'$show_code' :- '$debug'(0'f). %' just make emacs happy

/** @pred  grow_heap(+ _Size_)
Increase heap size  _Size_ kilobytes.


*/
grow_heap(X) :- '$grow_heap'(X).
/** @pred  grow_stack(+ _Size_)


Increase stack size  _Size_ kilobytes


 */
grow_stack(X) :- '$grow_stack'(X).

/** @pred  gc


The goal `gc` enables garbage collection. The same as
`yap_flag(gc,on)`.


*/
gc :-
	yap_flag(gc,on).
/** @pred  nogc


The goal `nogc` disables garbage collection. The same as
`yap_flag(gc,off)`.


*/
nogc :-
	yap_flag(gc,off).


/** @pred  garbage_collect_atoms


The goal `garbage_collect` forces a garbage collection of the atoms
in the data-base. Currently, only atoms are recovered.


*/
garbage_collect_atoms :-
	'$atom_gc'.

'$force_environment_for_gc'.

'$good_list_of_character_codes'(V) :- var(V), !.
'$good_list_of_character_codes'([]).
'$good_list_of_character_codes'([X|L]) :-
	'$good_character_code'(X),
	'$good_list_of_character_codes'(L).

'$good_character_code'(X) :- var(X), !.
'$good_character_code'(X) :- integer(X), X > -2, X < 256.

/** @pred prolog_initialization( _G_)


Add a goal to be executed on system initialization. This is compatible
with SICStus Prolog's initialization/1.


*/
prolog_initialization(G) :- var(G), !,
	'$do_error'(instantiation_error,initialization(G)).
prolog_initialization(T) :- must_be_callable(T), !,
	'$assert_init'(T).
prolog_initialization(T) :-
	'$do_error'(type_error(callable,T),initialization(T)).

'$assert_init'(T) :- recordz('$startup_goal',T,_), fail.
'$assert_init'(_).

/** @pred version

Write YAP's boot message.


*/
version :- 
	'$version_specs'(Specs),
	print_message(informational, version(Specs)).
		
	

/** @pred version(- _Message_)

Add a message to be written when yap boots or after aborting. It is not
possible to remove messages.


*/
version(V) :- var(V),  !,
	'$do_error'(instantiation_error,version(V)).
version(T) :- atom(T), !, '$assert_version'(T).
version(T) :-
	'$do_error'(type_error(atom,T),version(T)).

'$assert_version'(T) :- recordz('$version',T,_), fail.
'$assert_version'(_).

'$set_toplevel_hook'(_) :-
	recorded('$toplevel_hooks',_,R),
	erase(R),
	fail.
'$set_toplevel_hook'(H) :-
	recorda('$toplevel_hooks',H,_),
	fail.
'$set_toplevel_hook'(_).

query_to_answer(G, V, Status, Bindings) :-
	gated_call( true, (G,'$delayed_goals'(G, V, Vs, LGs, _DCP)), Status, '$answer'( Status, LGs, Vs, Bindings) ).

  '$answer'( exit, LGs, Vs, Bindings) :-
      !,
      '$process_answer'(Vs, LGs, Bindings).
      '$answer'( answer, LGs, Vs, Bindings) :-
          !,
          '$process_answer'(Vs, LGs, Bindings).
'$answer'(!, _, _, _).
'$answer'(fail,_,_,_).
'$answer'(exception(E),_,_,_) :-
        '$LoopError'(E,error).
'$answer'(external_exception(_),_,_,_).


%% @}

%% @{

%% @addtogroup Global_Variables

/** @pred  nb_getval(+ _Name_, - _Value_)


The nb_getval/2 predicate is a synonym for b_getval/2,
introduced for compatibility and symmetry. As most scenarios will use
a particular global variable either using non-backtrackable or
backtrackable assignment, using nb_getval/2 can be used to
document that the variable is used non-backtrackable.


*/
/** @pred nb_getval(+ _Name_,- _Value_)


The nb_getval/2 predicate is a synonym for b_getval/2, introduced for
compatibility and symmetry.  As most scenarios will use a particular
global variable either using non-backtrackable or backtrackable
assignment, using nb_getval/2 can be used to document that the
variable is used non-backtrackable.


*/
nb_getval(GlobalVariable, Val) :-
	'__NB_getval__'(GlobalVariable, Val, Error),
	(var(Error)
	->
	 true
	;
	 '$getval_exception'(GlobalVariable, Val, nb_getval(GlobalVariable, Val)) ->
	 nb_getval(GlobalVariable, Val)
	;
	 '$do_error'(existence_error(variable, GlobalVariable),nb_getval(GlobalVariable, Val))
	).


/** @pred  b_getval(+ _Name_, - _Value_)


Get the value associated with the global variable  _Name_ and unify
it with  _Value_. Note that this unification may further
instantiate the value of the global variable. If this is undesirable
the normal precautions (double negation or copy_term/2) must be
taken. The b_getval/2 predicate generates errors if  _Name_ is not
an atom or the requested variable does not exist.

Notice that for compatibility with other systems  _Name_ <em>must</em> be already associated with a term: otherwise the system will generate an error.


*/
/** @pred b_getval(+ _Name_,- _Value_)


Get the value associated with the global variable  _Name_ and unify
it with  _Value_. Note that this unification may further instantiate
the value of the global variable. If this is undesirable the normal
precautions (double negation or copy_term/2) must be taken. The
b_getval/2 predicate generates errors if  _Name_ is not an atom or
the requested variable does not exist.


*/
b_getval(GlobalVariable, Val) :-
	'__NB_getval__'(GlobalVariable, Val, Error),
	(var(Error)
	->
	 true
	;
	 '$getval_exception'(GlobalVariable, Val, b_getval(GlobalVariable, Val)) ->
	 true
	;
	 '$do_error'(existence_error(variable, GlobalVariable),b_getval(GlobalVariable, Val))
	).


%% @}

%% @{

%% @addtogroup YAPControl

/* This is the break predicate,
	it saves the importante data about current streams and
	debugger state */

'$debug_state'(state(Trace, Debug, State, SPY_GN, GList, GDList)) :-
	'$init_debugger',
	nb_getval('$trace',Trace),
	nb_getval('$debug_state',State),
	current_prolog_flag(debug, Debug),
	nb_getval('$spy_gn',SPY_GN),
	b_getval('$spy_glist',GList),
	b_getval('$spy_depth',GDList).


'$debug_stop' :-
	nb_setval('$debug_state', state(creep,0,stop)),
	b_setval('$trace',off),
	set_prolog_flag(debug, false),
	b_setval('$spy_glist',[]),
	b_setval('$spy_gdlist',[]),
	'$disable_debugging'.

'$debug_restart'(state(Trace, Debug, State, SPY_GN, GList, GDList)) :-
	b_setval('$spy_glist',GList),
	b_setval('$spy_gdlist',GDList),
	b_setval('$spy_gn',SPY_GN),
	set_prolog_flag(debug, Debug),
    nb_setval('$debug_state',State),
	b_setval('$trace',Trace),
	'$enable_debugging'.

/** @pred  break


Suspends the execution of the current goal and creates a new execution
level similar to the top level, displaying the following message:

~~~~~{.prolog}
 [ Break (level <number>) ]
~~~~~
telling the depth of the break level just entered. To return to the
previous level just type the end-of-file character or call the
end_of_file predicate.  This predicate is especially useful during
debugging.


*/
break :-
        '$debug_state'(DState),
        '$debug_start',
	'$break'( true ),
	current_output(OutStream), current_input(InpStream),
	current_prolog_flag(break_level, BL ),
        NBL is BL+1,
	set_prolog_flag(break_level, NBL ),
	format(user_error, '% Break (level ~w)~n', [NBL]),
	'$do_live',
	!,
	set_value('$live','$true'),
        '$debug_restore'(DState),
	set_input(InpStream),
	set_output(OutStream),
	set_prolog_flag(break_level, BL ),
	'$break'( false ).


at_halt(G) :-
	recorda('$halt', G, _),
	fail.
at_halt(_).

/** @pred  halt is iso

Halts Prolog, and exits to the calling application. In YAP,
halt/0 returns the exit code `0`.
*/
halt :-
	print_message(informational, halt),
	fail.
halt :-
    halt(0).

/** @pred  halt(+  _I_) is iso

Halts Prolog, and exits to 1the calling application returning the code
given by the integer  _I_.

*/
halt(_) :-
	recorded('$halt', G, _),
	catch(once(G), Error, user:'$Error'(Error)),
	fail.
halt(X) :-
	'$sync_mmapped_arrays',
	set_value('$live','$false'),
	'$halt'(X).

prolog_current_frame(Env) :-
	Env is '$env'.

'$run_atom_goal'(GA) :-
	'$current_module'(Module),
	atom_to_term(GA, G, _),
	catch(once(Module:G), Error,user:'$Error'(Error)).

'$add_dot_to_atom_goal'([],[0'.]) :- !. %'
'$add_dot_to_atom_goal'([0'.],[0'.]) :- !.
'$add_dot_to_atom_goal'([C|Gs0],[C|Gs]) :-
    '$add_dot_to_atom_goal'(Gs0,Gs).

/**

@pred call_in_module( +M:G )

   This predicate ensures that both deterministic and non-deterministic execution of the goal $G$ takes place in the context of goal _G_?
**/

yap_hacks:call_in_module(M:G) :-
    gated_call(
	'$module_boundary'(call, M0, M),
	call(G),
	Event,
	'$module_boundary'(Event,M0,M)
    ).



'$module_boundary'(call, M0, M) :-
    current_source_module(M0, M).
'$module_boundary'(answer, M0, _) :-
    current_source_module(_M, M0).
'$module_boundary'(exit, M0, _) :-
    current_source_module(_M, M0).
'$module_boundary'(redo, M0, _M) :-
    current_source_module(_, M0).
'$module_boundary'(fail, M0, _M) :-
    current_source_module(_, M0).
'$module_boundary'(!, _, _).
'$module_boundary'(external_exception(_), _, _).
'$module_boundary'(exception(_), M0, _M) :-
    current_source_module(_, M0).

/**
@}
*/

