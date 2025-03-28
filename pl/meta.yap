/**

  @file meta.yap

  @defgroup YAPMetaPredicates Using Meta-Calls with Modules
 @ingroup YAPModules
 @{

  @pred meta_predicate(G1 , Gj , Gn) is directive

Declares that this predicate manipulates references to predicates.
Each _Gi_ is a mode specification.

If the argument is `:`, it does not refer directly to a predicate
but must be module expanded. If the argument is an integer, the argument
is a goal or a closure and must be expanded. Otherwise, the argument is
not expanded. Note that the system already includes declarations for all
built-ins.

For example, the declaration for call/1 and setof/3 are:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
:- meta_predicate call(0), setof(?,0,?).
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

meta_predicate declaration
 implemented by asserting 

meta_predicate(SourceModule,Declaration)

*/

% directive now meta_predicate Ps :- $meta_predicate(Ps).

:- use_system_module( '$_arith', ['$c_built_in'/4]).

%% handle module transparent predicates by defining a
%% new context module.
'$is_mt'(H, B, HM, _SM, M, (context_module(CM),B), CM) :-
    '$yap_strip_module'(HM:H, M, NH),
    '$module_transparent'(_, M, _, NH).


% I assume the clause has been processed, so the
% var case is long gone! Yes :)
'$clean_cuts'(G,('$current_choice_point'(DCP),NG)) :-
	'$conj_has_cuts'(G,DCP,NG,OK), OK == ok, !.
'$clean_cuts'(G,G).		       

'$clean_cuts'(G,DCP,NG) :-
	'$conj_has_cuts'(G,DCP,NG,OK), OK == ok, !.
'$clean_cuts'(G,_,G).

'$conj_has_cuts'(V,_,V, _) :- var(V), !.
'$conj_has_cuts'(!,DCP,'$$cut_by'(DCP), ok) :- !.
'$conj_has_cuts'((G1,G2),DCP,(NG1,NG2), OK) :- !,
	'$conj_has_cuts'(G1, DCP, NG1, OK),
	'$conj_has_cuts'(G2, DCP, NG2, OK).
'$conj_has_cuts'((G1;G2),DCP,(NG1;NG2), OK) :- !,
	'$conj_has_cuts'(G1, DCP, NG1, OK),
	'$conj_has_cuts'(G2, DCP, NG2, OK).
'$conj_has_cuts'((G1->G2),DCP,(G1;NG2), OK) :- !,
	% G1: the system must have done it already
	'$conj_has_cuts'(G2, DCP, NG2, OK).
'$conj_has_cuts'((G1*->G2),DCP,(G1;NG2), OK) :- !,
	% G1: the system must have done it already
	'$conj_has_cuts'(G2, DCP, NG2, OK).
'$conj_has_cuts'(if(G1,G2,G3),DCP,if(G1,NG2,NG3), OK) :- !,
	% G1: the system must have done it already
	'$conj_has_cuts'(G2, DCP, NG2, OK),
	'$conj_has_cuts'(G3, DCP, NG3, OK).
'$conj_has_cuts'(G,_,G, _).

% return list of vars in expanded positions on the head of a clause.
%
% these variables should not be expanded by meta-calls in the body of the goal.
%
%  should be defined before caller.
%
'$module_u_vars'(M, H, UVars) :-
    '$do_module_u_vars'(M:H,UVars).

'$do_module_u_vars'(M:H,UVars) :-
	functor(H,F,N),
	functor(D,F,N),
	(recorded('$m',meta_predicate(M,D),_)->true; recorded('$m',meta_predicate(prolog,D),_)), !,
	'$do_module_u_vars'(N,D,H,UVars).
'$do_module_u_vars'(_,[]).

'$do_module_u_vars'(0,_,_,[]) :- !.
'$do_module_u_vars'(I,D,H,LF) :-
	arg(I,D,X), ( X=':' -> true ; integer(X)),
	arg(I,H,A), '$uvar'(A, LF, L), !,
	I1 is I-1,
	'$do_module_u_vars'(I1,D,H,L).
'$do_module_u_vars'(I,D,H,L) :-
	I1 is I-1, 
	'$do_module_u_vars'(I1,D,H,L).

'$uvar'(Y, [Y|L], L)  :- var(Y), !.
% support all/3
'$uvar'(same( G, _), LF, L)  :-
    '$uvar'(G, LF, L).
'$uvar'('^'( _, G), LF, L)  :-
    '$uvar'(G, LF, L).

'$expand_args'([],  _, [], _, []).
'$expand_args'([A|GArgs], SM,   [M|GDefs], HVars, [NA|NGArgs]) :-
    ( M == ':' -> true ; number(M) ),
    !,
    '$expand_arg'(A, M, SM, HVars, NA),
    '$expand_args'(GArgs, SM, GDefs, HVars, NGArgs).
'$expand_args'([A|GArgs],  SM, [_|GDefs], HVars, [A|NGArgs]) :-
	'$expand_args'(GArgs, SM, GDefs, HVars, NGArgs).


% check if an argument should be expanded
'$expand_arg'(G, _, SM, HVars, OG) :-
    var(G),
    !,
    ( lists:identical_member(G, HVars) -> OG = G; OG = SM:G).
'$expand_arg'(M:G, _, _SM, _HVars, M:G) :-
    !.
'$expand_arg'((G1,G2), MA, SM, _HVars, (NG1,NG2)) :-
    number(MA),
    !,
    '$expand_arg'(G1, MA, SM, _HVars, NG1),
    '$expand_arg'(G2, MA, SM, _HVars, NG2).
'$expand_arg'((G1;G2), _HVars, (NG1;NG2)) :-
    number(MA),
    !,
    '$expand_arg'(G1, MA, SM, _HVars, NG1),
    '$expand_arg'(G2, MA, SM, _HVars, NG2).
'$expand_arg'((G1->G2), MA, SM, _HVars, (NG1->NG2)) :-
    number(MA),
    !,
    '$expand_arg'(G1, MA, SM, _HVars, NG1),
    '$expand_arg'(G2, MA, SM, _HVars, NG2).

'$expand_arg'(G, _, SM, _HVars, SM:G).


% expand module names in a body
% args are:
%       goals to expand
%       code to pass to listing
%       code to pass to compiler
%       head module   HM
%       source module  SM
%       current module for looking up preds  M
%
% to understand the differences, you can consider:
%
%  a:(d:b(X)) :- g:c(X), d(X), user:hello(X)).
%
% when we process meta-predicate c, HM=d, DM=a, BM=a, M=g and we should get:
%
%  d:b(X) :- g:c(g:X), a:d(X), user:hello(X).
%
% on the other hand,
%
%  a:(d:b(X) :- c(X), d(X), d:e(X)).
%
% will give
%
%  d:b(X) :- a:c(a:X), a:d(X), e(X).
%
%
%       head variab'$expand_goals'(M:G,G1,GO,HM,SM,,_M,HVars)les.
%       goals or arguments/sub-arguments?
% I cannot use call here because of format/3
% modules:
% A4: module for body of clause (this is the one used in looking up predicates)
% A5: context module (this is the current context
				% A6: head module (this is the one used in compiling and accessing).
%
'$expand_goals'(V,NG,NGO,HM,SM,BM,HVarsH) :-
    var(V),!,
    '$expand_goals'(call(V),NG,NGO,HM,SM,BM,HVarsH).
'$expand_goals'(BM:G,NG,NGO,HM,SM,BM,HVarsH) :-
	    '$yap_strip_module'( BM:G, CM, G1),
	     !,
	     (var(CM) ->
	     '$expand_goals'(call(BM:G),NG,NGO,HM,SM,BM,HVarsH)
	     ;
	     '$expand_goals'(G1,NG,NGO,HM,CM,CM,HVarsH)
	     ).
'$expand_goals'(call(BMG),NG,NGO,HM,SM,BM,HVarsH) :-
	    nonvar(BMG),
	    '$yap_strip_module'( BMG, CM, G1),
	    nonvar(G1),
	     !,
	     (var(CM) ->
	     '$expand_goal'(call(CM:G1),NG,NGO,HM,SM,BM,HVarsH)
	     ;
	     '$expand_goals'(G1,NG,NGO,HM,CM,CM,HVarsH)
	     ).
'$expand_goals'((A*->B;C),(A1*->B1;C1),
	(	yap_hacks:current_choicepoint(CP0),
        (
          yap_hacks:current_choicepoint(CP),
          AO,
          yap_hacks:cut_at(CP0,CP),
	  BO
          ;
          CO
        )),
        HM,SM,BM,HVars) :- !,
	'$expand_goals'(A,A1,AOO,HM,SM,BM,HVars),
	'$clean_cuts'(AOO, AO),
	'$expand_goals'(B,B1,BO,HM,SM,BM,HVars),
	'$expand_goals'(C,C1,CO,HM,SM,BM,HVars).
'$expand_goals'((A->B;C),(A1->B1;C1),
        (AO->BO;CO),
        HM,SM,BM,HVars) :- !,
	'$expand_goals'(A,A1,AOO,HM,SM,BM,HVars),
	'$clean_cuts'(AOO, AO),
	'$expand_goals'(B,B1,BO,HM,SM,BM,HVars),
	'$expand_goals'(C,C1,CO,HM,SM,BM,HVars).
'$expand_goals'(if(A,B,C),if(A1,B1,C1),
		('$current_choice_point'(CP0),
		('$current_choice_point'(CP),AO,yap_hacks:cut_at(CP0,CP),BO; CO)),HM,SM,BM,HVars) :- !,
	'$expand_goals'(A,A1,AO0,HM,SM,BM,HVars),
	'$expand_goals'(B,B1,BO,HM,SM,BM,HVars),
	'$expand_goals'(C,C1,CO,HM,SM,BM,HVars),
        '$clean_cuts'(AO0, CP, AO).
'$expand_goals'((A,B),(A1,B1),(AO,BO),HM,SM,BM,HVars) :- !,
	'$expand_goals'(A,A1,AO,HM,SM,BM,HVars),
	'$expand_goals'(B,B1,BO,HM,SM,BM,HVars).
'$expand_goals'((A;B),(A1;B1),(AO;BO),HM,SM,BM,HVars) :- var(A), !,
	'$expand_goals'(A,A1,AO,HM,SM,BM,HVars),
	'$expand_goals'(B,B1,BO,HM,SM,BM,HVars).
'$expand_goals'((A|B),(A1|B1),(AO|BO),HM,SM,BM,HVars) :- !,
	'$expand_goals'(A,A1,AO,HM,SM,BM,HVars),
	'$expand_goals'(B,B1,BO,HM,SM,BM,HVars).
'$expand_goals'((A->B),(A1->B1),
		(
		    '$$save_by'(DCP),
		    AO,
		    '$$cut_by'(DCP),
		    BO
		),
          HM,SM,BM,HVars) :- !,
	'$expand_goals'(A,A1,AOO,HM,SM,BM,HVars),
	'$clean_cuts'(AOO, AO),
	'$expand_goals'(B,B1,BO,HM,SM,BM,HVars).
'$expand_goals'(\+G,\+G,A\=B,_HM,_BM,_SM,_HVars) :-
    nonvar(G),
    G = (A = B),
    !.
'$expand_goals'(\+A,\+A1,(AO-> fail;true),HM,SM,BM,HVars) :- !,
	'$expand_goals'(A,A1,AOO,HM,SM,BM,HVars),
	'$clean_cuts'(AOO, AO).
'$expand_goals'(once(A),once(A1),
	('$current_choice_point'(CP),AO,'$$cut_by'(CP)),HM,SM,BM,HVars) :- !,
	'$expand_goals'(A,A1,AO0,HM,SM,BM,HVars),
        '$clean_cuts'(AO0, CP, AO).
'$expand_goals'((:-A),(:-A1),
	(:-AO),HM,SM,BM,HVars) :- !,
	'$expand_goals'(A,A1,AO,HM,SM,BM,HVars).
'$expand_goals'(ignore(A),ignore(A1),
	('$current_choice_point'(CP),AO,'$$cut_by'(CP)-> true ; true),HM,SM,BM,HVars) :- !,
	'$expand_goals'(A,A1,AO0,HM,SM,BM,HVars),
    '$clean_cuts'(AO0, AO).
'$expand_goals'(forall(A,B),forall(A1,B1),
		\+( (AO,\+ ( BO ) )),
		HM,SM,BM,HVars) :- !,
    '$expand_goals'(A,A1,AO0,HM,SM,BM,HVars),
	'$expand_goals'(B,B1,BO0,HM,SM,BM,HVars),
        '$clean_cuts'(AO0, AO),
        '$clean_cuts'(BO0, BO).
'$expand_goals'(not(A),not(A1),('$current_choice_point'(CP),AO,'$$cut_by'(CP) -> fail; true),HM,SM,BM,HVars) :- !,
	'$expand_goals'(A,A1,AO,HM,SM,BM,HVars).
'$expand_goals'((A*->B),(A1*->B1),
	('$current_choice_point'(DCP),AO,BO),HM,SM,BM,HVars) :- !,
	'$expand_goals'(A,A1,AO0,HM,SM,BM,HVars),
	'$expand_goals'(B,B1,BO,HM,SM,BM,HVars),
    '$clean_cuts'(AO0, DCP, AO).
'$expand_goals'(true,true,true,_,_,_,_) :- !.
'$expand_goals'(fail,fail,fail,_,_,_,_) :- !.
'$expand_goals'(false,false,false,_,_,_,_) :- !.
'$expand_goals'(G, G1, GO, HM, SM, BM, HVars) :-
    '$yap_strip_module'(BM:G,  NBM, GM),
    '$expand_goal'(GM, G1, GO, HM, SM, NBM, HVars).


'$import_expansion'(M:G, M1:G1) :-
    '$imported_predicate'(G, M, G1, M1),
     !.
'$import_expansion'(MG, MG).

'$meta_expansion'(G, GM, SM, HVars, OG) :-
	 functor(G, F, Arity ),
	 functor(PredDef, F, Arity ),
	 (recorded('$m',meta_predicate(GM,PredDef),_)->true;recorded('$m',meta_predicate(prolog,PredDef),_)),
	 !,
	 G =.. [F|LArgs],
	 PredDef =.. [F|LMs],
	 '$expand_args'(LArgs, SM, LMs, HVars, OArgs),
	 OG =.. [F|OArgs].


'$meta_expansion'(G, _GM, _SM, _HVars, G).

 /**
 * @brief Perform meta-variable and user expansion on a goal _G_
 *
 * given the example
~~~~~
:- module(m, []).

o:p(B) :- n:g, X is 2+3, call(B).
~~~~~
 *
 * @param G input goal, without module quantification.
 * @param G1F output, non-optimised for debugging
 * @param GOF output, optimised, ie, `n:g`, `prolog:(X is 2+3)`, `call(m:B)`, where `prolog` does not  need to be explicit
 * @param GOF output, optimised, `n:g`, `prolog:(X=5)`,  `call(m:B)`
 * @param HM head module, input, o
 * @param HM source module, input, m
 * @param M current module, input, `n`, `m`, `m`
 * @param HVars-H, list of meta-variables and initial head, `[]` and `p(B)`
 *
 *
 */
 '$expand_goal'(G0, G1F, GOF, HM, SM0, BM0, HVars-H) :-
       % we have a context
      '$yap_strip_module'( BM0:G0, M0N, G), % MON is both the source and goal module
      (G == G0 % use the environments SM and HM
      ->
      BM0 = BM, SM0 = SM
      ;
      % use the one(s) given by the user
      M0N = BM, M0N= SM),
      % we still may be using an imported predicate:

     '$user_expansion'(BM:G, M1:G1),
     '$import_expansion'(M1:G1, M2:G2),
     '$meta_expansion'(G2, M2, SM,	HVars, G3),
    '$match_mod'(G3, HM, SM, M2, G1F),
    '$c_built_in'(G1F, M2, H, GOF).

'$user_expansion'(M0N:G0N, M1:G1) :-
    '_user_expand_goal'(M0N:G0N, M:G),
    !,
    ( M:G == M0N:G0N
    ->
      M1:G1 = M:G
    ;
      '$user_expansion'(M:G, M1:G1)
    ).
'$user_expansion'(MG, MG).

'$match_mod'(G, HMod, SMod, M, O) :-
    (
	'$is_metapredicate'(G,M)
    ->
    O = M:G
    ;
      '$is_system_predicate'(G,prolog)
     ->
      O = G
    ;
      user == HMod, user == SMod, user == M
    ->
     O = G
    ;
     O = M:G
    ).

'$build_up'(HM, NH, SM, true, NH, true, NH) :- HM == SM, !.
'$build_up'(HM, NH, _SM, true, HM:NH, true, HM:NH) :- !.
'$build_up'(HM, NH, SM, B1, (NH :- B1), BO, ( NH :- BO)) :- HM == SM, !.
'$build_up'(HM, NH, _SM, B1, (NH :- B1), BO, ( HM:NH :- BO)) :- !.

'$expand_goals'(BM:G,H,HM,_SM,_BM,B1,BO) :-
	    '$yap_strip_module'( BM:G, CM, G1),
	     !,
	     (var(CM) ->
	     '$expand_goals'(call(BM:G),H,HM,_SM,_BM,B1,BO)
	     ;
	     '$expand_goals'(G1,H,HM,CM,CM,B1,BO)
	     ).

'$expand_clause_body'(V, _NH1, _HM1, _SM, M, call(M:V), call(M:V) ) :-
    var(V), !.
'$expand_clause_body'(true, _NH1, _HM1, _SM, _M, true, true ) :- !.
'$expand_clause_body'(B, H, HM, SM, M, B1, BO ) :-
	'$module_u_vars'(HM , H, UVars),
				% collect head variables in
                                % expanded positions
                                % support for SWI's meta primitive.
	(
	  '$is_mt'(H, B, HM, SM, M, IB, BM)
	->
	  IB = B1, IB = BO0
	;
	  M = BM, '$expand_goals'(B, B1, BO0, HM, SM, BM, UVars-H)
	),
	(
	  '$full_clause_optimisation'(H, BM, BO0, BO)
	->
	  true
	;
	  BO = BO0
	).

%
% check if current module redefines an imported predicate.
% and remove import.
%
'$not_imported'(H, Mod) :-
	recorded('$import','$import'(NM,Mod,NH,H,_,_),R),
	NM \= Mod,
	functor(NH,N,Ar),
	print_message(warning,redefine_imported(Mod,NM,N/Ar)),
	erase(R),
	fail.
'$not_imported'(_, _).


'$verify_import'(M:G, NM:NG) :-
	'$follow_import_chain'(M,G,NM,NG).


'$expand_meta_call'(M0:G, HVars, M:GF ) :-
    !,
    '$yap_strip_module'(M0:G, M, IG),
    '$expand_goals'(IG, GF, _GF0, M, M, M, HVars-IG).
'$expand_meta_call'(G, HVars, M:GF ) :-
    source_module(SM0),
    '$yap_strip_module'(SM0:G, M, IG),
    '$expand_goals'(IG, GF, _GF0, SM, SM, M, HVars-IG).


'$expand_a_clause'(MHB, SM0, Cl1, ClO) :- % MHB is the original clause, SM0 the current source, Cl1 and ClO output clauses
     '$yap_strip_module'(SM0:MHB, SM, HB),  % remove layers of modules over the clause. SM is the source module.
    '$head_and_body'(HB, H, B),           % HB is H :- B.
    '$yap_strip_module'(SM:H, HM, NH), % further module expansion
    '$not_imported'(NH, HM),
    '$yap_strip_module'(SM:B, BM, B0), % further module expansion
    '$expand_clause_body'(B0, NH, HM, SM0, BM, B1, BO),
    !,
    '$build_up'(HM, NH, SM0, B1, Cl1, BO, ClO).
'$expand_a_clause'(Cl, _SM, Cl, Cl).




% expand arguments of a meta-predicate
% $meta_expansion(ModuleWhereDefined,CurrentModule,Goal,ExpandedGoal,MetaVariables)


% expand module names in a clause (interface predicate).
% A1: Input Clause
% A2: Output Class to Compiler (lives in module HM)
% A3: Output Class to clause/2 and listing (lives in module HM)
%
% modules:
% A6: head module (this is the one used in compiling and accessing).
% A5: context module (this is the current context
% A4: module for body of clause (this is the one used in looking up predicates)
%
                             % has to be last!!!
expand_goal(Input, Output) :-
    '$expand_meta_call'(Input, [], Output ).
