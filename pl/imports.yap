/*************************************************************************
*									 *
*	 YAP Prolog 							 *
*									 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
****************              
*									 *
*************************************************************************/

/**
 *
 * @ file imports.yap
 * 
 * this file implements search for available predicates for the current module. Search can be called
 * at compile-time or at run-time.
 *
 * @defgroup PImport Predicate Import Mechanism
 * 
 * @{
 * 
 * The import mechanism is as follows:
 *   - built-ina (module prolog)
 *   - explicit imports (import table).
 *   - parent module mechanism.
 *   - SWI auto-loader.
 */

:- '$mk_dynamic'( prolog:'$parent_module'(_,_)).
% You can have a default parent (user)
% '$across_modules'(_:G, Visited, ExportingMod:G)  :-
%     current_prolog_flag(default_parent_module, ExportingModuleI),
%     recorded('$module','$module'( _, ExportingModuleI, _, _, Exports), _),
%     lists:member(G, Exports),
%     \+ lists:member(ExportingModuleI:G, Visited),
%     '$check_definition'(ExportingModuleI:G, [ExportingModuleI:G|Visited], ExportingMod:G).  
%  % parent module mechanism
% '$across_modules'(ImportingMod:G, Visited, ExportingMod:G ) :-  
%     '$parent_module'(ImportingMod,ExportingModI),
%     recorded('$module','$module'( _, ExportingModI, _, _, Exports), _),	lists:member(G, Exports),
% 	\+ lists:member(ExportingModI:G, Visited),
% 	'$check_definition'(ExportingModI , [ExportingModI:G|Visited], ExportingMod:G).  
%   % autoload
% '$across_modules'(_ImportingMod:G, Visited, ExportingMod:G ) :-  
%     recorded('$dialect',swi,_),
%     fail,
%     prolog_flag(autoload, true),
%     prolog_flag(unknown, _OldUnk, fail),
%     (
% 	recorded('$module','$module'( _, autoloader, _, _, _Exports), _)
%     ->
%     true
%     ;
%     use_module(library(autoloader))
%     ;
%     true
%     ),
%     autoload(G, _File, ExportingModI), 
%     \+ lists:member(ExportingModI:G, Visited),
%     '$check_definition'(ExportingModI:G , [ExportingModI:G|Visited], ExportingMod:G).

% be careful here not to generate an undefined exception.
'$imported_predicate'(G, _ImportingMod, G, prolog) :-
	nonvar(G), '$is_system_predicate'(G, prolog), !.
'$imported_predicate'(G, ImportingMod, G0, ExportingMod) :-
	nonvar(G),
	'$follow_import_chain'(ImportingMod,G,ExportingMod,G0).


