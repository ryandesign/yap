/*************************************************************************
 *									 *
 *	 YAP Prolog 							 *
 *									 *
 *	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
 *									 *
 * Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
 *									 *
 **************************************************************************
 *									 *
 * File:		utilpreds.c * Last rev:	4/03/88
 *
 * mods:
 * comments:	new utility predicates for YAP *
 *									 *
 *************************************************************************/

#ifndef TERMS_C
#define TERMS_C 1

//#define DEB_D0(S) printf("%s %s: d0=%lx ptd0=%p *ptd0=%lx\n",S,d0, ptd0,
/**
 * @file C/terms.c
 *
 * @brief applications of the tree walker pattern.
 *
 * @addtogroup Terms
 *
 * @{
 *
 */

#include "absmi.h"

#include "YapHeap.h"

#include "heapgc.h"

#include "attvar.h"
#include "clause.h"
#include "yapio.h"
#ifdef HAVE_STRING_H
#include "string.h"
#endif

#define DEB_DOOBOUT(d0)                                                    \
  // (fprintf(stderr, ">>>>>>> %s ", __FUNCTION__), Yap_DebugPlWriteln(d0))
#define DEB_DOOBIN(d0)  //(fprintf(stderr,"<<<<<<<<<<<<<<<<<<<<<<< %s:%d\n", __FUNCTION__, __LINE__)/*, Yap_DebugPlWriteln(d0))*/)
#define DEB_DOOB(S,sp)  //  fprintf(stderr, "%s %s:%d %ld \n ", S,__FUNCTION__, __LINE__, sp->pt- sp->pt0)

/*#define err, "%s %ld %p->%p=%lx ", s, st->pt - st->pt0, pt0, ptd0, d0), */
/*   Yap_DebugPlWriteln(d0)) */

#define push_sub_term(A,B,C,D,E) ( DEB_DOOB("+",A)    push_sub_term__(A,B,C,D,E))
#define pop_sub_term(A,B,C) ( DEB_DOOB("-",A)   pop_sub_term__(A,B,C))
static inline bool push_sub_term__(Ystack_t *sp, CELL d0, CELL *pt0, CELL *b,
                                 CELL *e) {
  copy_frame *pt = sp->pt;
  if (sp->max == pt)
    return false;
  pt->pt0 = b;
  pt->pt0_end = e;
  (pt)->oldv = d0;
  (pt)->oldp = pt0;
  sp->pt++;
  return true;
}

static inline bool pop_sub_term__(Ystack_t *sp, CELL **b, CELL **e) {
  copy_frame *pt = --(sp->pt);
  if (pt < sp->pt0)
    return false;
  if (pt->oldp != NULL)
    pt->oldp[0] = pt->oldv;
  if (b)
    *b = pt->pt0;
  if (e)
    *e = pt->pt0_end;
  return true;
}

#define END_WALK()                                                             \
  }                                                                            \
  }                                                                            \
  /***** start of bottom-macro ************/                                   \
  nomore:

#define RESET_TERM_VISITOR()                                                                  \
  if (LOCAL_Error_TYPE){   Term* pt0,*pt0_end;                                                         stt->arenap= NULL; \
      stt->t =  t;                                                             \
      stt->bindp = NULL;                                                       \
      while (pop_sub_term(stt, &pt0, &pt0_end));\
      reset_trail(stt->tr0 PASS_REGS);\
term_error_handler(stt);	      \
              close_stack(stt);\
                pop_text_stack(lvl);\
LOCAL_Error_TYPE = 0;                                                    \
   lvl =  push_text_stack();\
   t = stt->t;\
   sz+=4096;		\
}  else {\
                      close_stack(stt);\
		     break;			\
   }


static bool term_error_handler(Ystack_t *stt) {
  yhandle_t ctx;
  Term *bindp;
  bindp = stt->bindp;
  ctx = Yap_InitHandle(stt->t);
  if (bindp) {
    Yap_InitHandle(*bindp);
  }
  if (stt->err == RESOURCE_ERROR_AUXILIARY_STACK) {
     // *pdepth += 4096;
  } else if (stt->err == RESOURCE_ERROR_TRAIL) {

    if (!Yap_growtrail(0, false)) {
      Yap_ThrowError(RESOURCE_ERROR_TRAIL, TermNil, "while visiting terms");
    }
  } else if (stt->err == RESOURCE_ERROR_STACK) {
    //    printf("In H0=%p Hb=%ld H=%ld G0=%ld GF=%ld ASP=%ld\n",H0, cs->oHB-H0,
    //     cs->oH-H0, ArenaPt(*arenap)-H0,ArenaLimit(*arenap)-H0,(LCL0-cs->oASP)-H0)  ;
      if (!Yap_dogcl(0)) {
       Yap_ThrowError(RESOURCE_ERROR_STACK, TermNil, LOCAL_ErrorMessage);       
      }
    //     printf("In H0=%p Hb=%ld H=%ld G0=%ld GF=%ld ASP=%ld\n",H0, cs->oHB-H0,
     //      cs->oH-H0, ArenaPt(*arenap)-H0,ArenaLimit(*arenap)-H0,LCL0-cs->oASP-H0)  ;
  }
  LOCAL_Error_TYPE = 0;
  if (bindp) {
    *bindp = Yap_GetFromHandle(ctx + 1);
  }
  stt->t = Yap_PopHandle(ctx);


  return true;
}

#undef LIST_HOOK_CODE
#undef COMPOUND_HOOK_CODE
#undef ATOMIC_HOOK_CODE
#define LIST_HOOK_CODE {if (IS_VISIT_MARKER(ptd1[0])) goto found;}
#define COMPOUND_HOOK_CODE {if (IS_VISIT_MARKER(ptd1[0])) goto found;}
#define ATOMIC_HOOK_CODE                                                       \
  {}

/**
   @brief routine to locate all variables in a term, and its applications
   */

static Term cyclic_complex_term(CELL *pt0_, CELL *pt0_end_,
                                Ystack_t *stt USES_REGS) {
    COPY(pt0_[1]);
#include "term_visit.h"
            /* leave an empty slot to fill in later */
    END_WALK();
    // no cycles found
    return false;
 found:
  while (pop_sub_term(stt, NULL, NULL)) {
    };
    return true;
  }

bool Yap_IsCyclicTerm(Term t USES_REGS) {
    if (IsVarTerm(t)) {
        return false;
    } else if (IsPrimitiveTerm(t)) {
        return false;
    } else {
      bool rc;
        Ystack_t stt_, *stt = &stt_;
 int lvl = push_text_stack();
 size_t sz = 1024;
  do {
        init_stack(stt, sz);
        rc = cyclic_complex_term(&(t)-1, &(t), stt PASS_REGS);
        RESET_TERM_VISITOR();
  } while (true);
  pop_text_stack(lvl);
   return rc;
    }
}

/** @pred  cyclic_term( + _T_ )


    Succeeds if the graph representation of the term has loops. Say,
    The representation of a term `X` that obeys the equation `X=[X]`
    term has a loop from the list to its head.


*/
static Int cyclic_term(USES_REGS1) /* cyclic_term(+T)		 */
{

    Term t=Deref(ARG1);
    Ystack_t stt_, *stt = &stt_;
    Int rc;
  int lvl = push_text_stack();
   size_t sz = 1024;
  do {

    init_stack(stt, sz);
    rc = cyclic_complex_term(&(t)-1, &(t), stt PASS_REGS);
    RESET_TERM_VISITOR();
  } while (true);
  pop_text_stack(lvl);
   return rc;
}


/**
   @brief routine to locate all variables in a term, and its application
   s */

#undef LIST_HOOK_CODE
#undef COMPOUND_HOOK_CODE
#undef ATOMIC_HOOK_CODE
#define LIST_HOOK_CODE                                                         \
  {}
#define COMPOUND_HOOK_CODE                                                     \
  {}
#define ATOMIC_HOOK_CODE                                                       \
  {}

static bool ground_complex_term(CELL *pt0_, CELL *pt0_end_,
                                Ystack_t *stt USES_REGS) {
  //  pp(pt0_     [1], 0);
  COPY(pt0_[1]);
#include "term_visit.h"
  while (pop_sub_term(stt, NULL, NULL)) {
  };
  //  pp(pt0_[1], 0);
  return false;

  END_WALK();
  return true;
  /* should have a return before this point */
  /* close main processing loop */
}

bool Yap_IsGroundTerm(Term t) {
  CACHE_REGS
  if (IsVarTerm(t)) {
    return false;
  } else if (IsPrimitiveTerm(t)) {
    return true;
  } else {
    bool rc;
    Ystack_t stt_, *stt = &stt_;
 int lvl = push_text_stack();
 size_t sz = 1024;
  do {

 init_stack(stt, sz);
    rc = ground_complex_term(&(t)-1, &(t), stt PASS_REGS);
      RESET_TERM_VISITOR();
  } while (true);
    HB = B->cp_h;
  pop_text_stack(lvl);

    reset_trail(stt->tr0 PASS_REGS);

    return rc;
  }
}

/** @pred  ground( _T_) is iso

    Succeeds if there are no free variables in the term  _T_.



*/
static Int ground(USES_REGS1) /* ground(+T)		 */
{
  Ystack_t stt_, *stt = &stt_;
  Term t;
  int rc;
   size_t sz = 1024;
  t = Deref(ARG1);
  if (IsVarTerm(t)) {
    return false;
  } else if (IsPrimitiveTerm(t)) {
    return true;
  } 
 int lvl = push_text_stack();
  do {
  init_stack(stt, sz);

    rc = ground_complex_term(&(t)-1, &(t), stt PASS_REGS);
      RESET_TERM_VISITOR();
  } while (true);
  pop_text_stack(lvl);
    HB = B->cp_h;
    reset_trail(stt->tr0);

    return rc;

}

#undef LIST_HOOK_CODE
#undef COMPOUND_HOOK_CODE
#undef ATOMIC_HOOK_CODE
#define LIST_HOOK_CODE                                                         \
  {}
#define COMPOUND_HOOK_CODE                                                     \
  {}
#define ATOMIC_HOOK_CODE                                                       \
  {}

static Int var_in_complex_term(CELL *pt0_, CELL *pt0_end_, Term v,
                               Ystack_t *stt USES_REGS) {
  COPY(pt0_[1]);

#include "term_visit.h"

  if ((CELL)ptd0 == v) { /* we found it */
    /* Do we still have compound terms to visit */
    while (pop_sub_term(stt, NULL, NULL)) {
    };
    return true;
  }
  END_WALK();
  return false;
}

/** @pred variable_in_term(? _Term_,? _Var_)

    True if the second argument  _Var_ is a variable that occurs in
    term  _Term_.


*/
static Int variable_in_term(USES_REGS1) {
  Ystack_t stt_, *stt = &stt_;
  Term t, v;
 size_t sz = 1024;
   Int rc;
  t = Deref(ARG1), v = Deref(ARG2);
  must_be_variable(v);
  v = MkGlobal(v);
  if (IsVarTerm(t)) {
    return (v == t);
  } else if (IsPrimitiveTerm(t)) {
    return (false);
  }
   int lvl = push_text_stack();
  do {
  init_stack(stt, sz);
  rc = var_in_complex_term(&(t)-1, &(t), v, stt PASS_REGS);
      RESET_TERM_VISITOR();
  } while (true);
    HB = B->cp_h;
  pop_text_stack(lvl);

  reset_trail(stt->tr0 PASS_REGS);
  return rc;
}

#undef LIST_HOOK_CODE
#undef COMPOUND_HOOK_CODE
#undef ATOMIC_HOOK_CODE
#define LIST_HOOK_CODE                                                         \
  {}
#define COMPOUND_HOOK_CODE                                                     \
  {}
#define ATOMIC_HOOK_CODE                                                       \
  {}

/**
 *  @brief routine to locate all variables in a term, and its applications.
 */
static Term vars_in_complex_term(CELL *pt0_, CELL *pt0_end_, tr_fr_ptr TR0,
                                 Term tail, Ystack_t *stt USES_REGS) {

  Term *end = NULL, first = tail;
  COPY(pt0_[1]);

#include "term_visit.h"

  if (HR + 1024 > ASP) {
    LOCAL_Error_TYPE = RESOURCE_ERROR_STACK;
    return 0;
  }
  if (end == NULL) {
    first = AbsPair(HR);
  } else {
    end[0] = AbsPair(HR);
  }
  HR[0] = (CELL)ptd0;
  HR[1] = tail;
  end = HR + 1;
  HR += 2;

  /* next make sure noone will see this, *start = HRs as a variable again */
  if (TR > (tr_fr_ptr)LOCAL_TrailTop - 256) {
    /* Trail overflow */
    LOCAL_Error_TYPE = RESOURCE_ERROR_TRAIL;
    return 0;
  }
  mBind_And_Trail(ptd0, TermFoundVar);

  END_WALK();

  return first;
}

/**
 * @pred variables_in_term( +_T_, +_SetOfVariables_, +_ExtendedSetOfVariables_
 * )
 *
 * _SetOfVariables_ must be a list of unbound variables. If so,
 * _ExtendedSetOfVariables_ will include all te variables in the union
 * of `vars(_T_)` and _SetOfVariables_.
 */
static Int variables_in_term(USES_REGS1) /* variables in term t		 */
{
  Ystack_t stt_, *stt = &stt_;

  Term out, t;
  t = Deref(ARG1);
  if (IsVarTerm(t)) {
    t = MkGlobal(t);
  }

    int lvl = push_text_stack();
 size_t sz = 1024;
 restart:
  do {
  if (!init_stack(stt, sz)) {
    LOCAL_Error_TYPE = RESOURCE_ERROR_AUXILIARY_STACK;
    return 0;
  }

  out = vars_in_complex_term(&(t)-1, &(t), TR, TermNil, stt PASS_REGS);
     RESET_TERM_VISITOR();
     sz += sz;
  } while(true);
  reset_trail(stt->tr0 PASS_REGS);
  if (out) {
    pop_text_stack(lvl);
      return Yap_unify(ARG3, out);
  } else {
    sz += sz;
      goto restart;
  }
}

/** @pred  term_variables(? _Term_, - _Variables_, +_ExternalVars_) is iso

    Unify the difference list between _Variables_ and _ExternaVars_
    with the list of all variables of term _Term_.  The variables
    occur in the order of their first appearance when traversing the
    term depth-first, left-to-right.
*/
static Int term_variables3(USES_REGS1) /* variables in term t		 */
{
  Ystack_t stt_, *stt = &stt_;
  Term t, in, out;



  t = Deref(ARG1);

  in = Deref(ARG3);

  if (!IsVarTerm(t) && IsPrimitiveTerm(t)) {
    return in;
  } 
  int lvl = push_text_stack();
 size_t sz = 1024;
  do {
 init_stack(stt, sz);
    vars_in_complex_term(&(in)-1, &(in), stt->tr0, TermNil,
                             stt PASS_REGS);
    out =
            vars_in_complex_term(&(in)-1, &(in), stt->tr0, in, stt PASS_REGS);
  
      RESET_TERM_VISITOR();
  } while (true);
    HB = B->cp_h;
  pop_text_stack(lvl);

  reset_trail(stt->tr0 PASS_REGS);

  return Yap_unify(ARG2, out);
}

/**
 * Exports a nil-terminated list with all the variables in a term.
 * @param[t] the term
 * @param[arity] the arity of the calling predicate (required for exact
 * garbage collection).
 * @param[USES_REGS] threading
 */
Term Yap_TermVariables(Term t, Term t0 USES_REGS) /* variables in term t  */
{
  Term out;
  Ystack_t stt_, *stt = &stt_;
  t = Deref(t);
  if (!IsVarTerm(t) && IsPrimitiveTerm(t)) {
    return TermNil;
  } 
  int lvl = push_text_stack();
 size_t sz = 1024;
  do {
 if (!init_stack(stt, sz)) {
    LOCAL_Error_TYPE = RESOURCE_ERROR_AUXILIARY_STACK;
    return 0;
  }

    out = vars_in_complex_term(&(t)-1, &(t), NULL, t0, stt PASS_REGS);
    out = vars_in_complex_term(&(t)-1, &(t), TR, out, stt PASS_REGS);
    reset_trail(stt->tr0 PASS_REGS);
  } while(true);
    pop_text_stack(lvl);
  return out;
}

/** @pred  term_variables(? _Term_, - _Variables_) is iso

    Unify  _Variables_ with the list of all variables of term
    _Term_.  The variables occur in the order of their first
    appearance when traversing the term depth-first, left-to-right.

*/
static Int term_variables(USES_REGS1) /* variables in term t		 */
{
  Ystack_t stt_, *stt = &stt_;
  if (!Yap_IsListOrPartialListTerm(Deref(ARG2))) {
    Yap_ThrowError(TYPE_ERROR_LIST, ARG2, "term_variables/2");
    return false;
  }
  Term out;
  int lvl = push_text_stack();
 size_t sz = 1024;
 do {


    Term t;
    t = Deref(ARG1);
    init_stack(stt, sz);
    out = vars_in_complex_term(&(t)-1, &(t), TR, TermNil, stt PASS_REGS);
    reset_trail(stt->tr0 PASS_REGS);
      RESET_TERM_VISITOR();
  } while (true);
    HB = B->cp_h;
  pop_text_stack(lvl);

    
  return Yap_unify(ARG2, out);
}

/** routine to locate attributed variables */

typedef struct att_rec {
  CELL *beg, *end, d0;
} att_rec_t;

#undef LIST_HOOK_CODE
#undef COMPOUND_HOOK_CODE
#undef ATOMIC_HOOK_CODE
#define LIST_HOOK_CODE                                                         \
  {}
#define COMPOUND_HOOK_CODE                                                     \
  {}
#define ATOMIC_HOOK_CODE                                                       \
  {}

static Term attvars_in_complex_term(CELL *pt0_, CELL *pt0_end_, Term inp,
                                    Ystack_t *stt USES_REGS) {

  CELL output = inp;
  COPY(pt0_[1]);

#include "term_visit.h"
  if (!IS_VISIT_MARKER(*ptd0) && GlobalIsAttVar(ptd0)) {
    if (HR + 1024 > ASP) {
      LOCAL_Error_TYPE = RESOURCE_ERROR_STACK;
      return 0;
    }
    output = MkPairTerm((CELL)ptd0, output);
    /* store the terms to visit */
    if (TR > (tr_fr_ptr)LOCAL_TrailTop - 256) {
      LOCAL_Error_TYPE = RESOURCE_ERROR_TRAIL;
      return 0;
    }
    Bind_and_Trail(ptd0, TermFoundVar);
    ptd0 += 2;
    dd0 = *ptd0;
  mderef_head(d0, dd0, var_in_term_unk); /*DEB_DOOB();*/
goto  var_in_term_nvar;
  }
  END_WALK();
  // no more variables to be found
  /*fprintf(stderr,"<%ld at %s\n", d0, __FUNCTION__)*/;
  return output;
}

/** @pred term_attvars(+ _Term_,- _AttVars_)

    _AttVars_ is a list of all attributed variables in  _Term_ and
    its attributes. I.e., term_attvars/2 works recursively through
    attributes.  This predicate is Cycle-safe.
*/
static Int term_attvars(USES_REGS1) /* variables in term t		 */
{
  Ystack_t stt_, *stt = &stt_;

  Term out=0, t = Deref(ARG1);
    if (IsPrimitiveTerm(t)) {
      return Yap_unify(TermNil, ARG2);
    } 
    int lvl = push_text_stack();
 size_t sz = 1024;
 restart:
 while (true) {
    init_stack(stt, sz);
    out = attvars_in_complex_term(&(t)-1, &(t), TermNil, stt PASS_REGS);
    RESET_TERM_VISITOR();
 }
  reset_trail(stt->tr0 PASS_REGS);
  pop_text_stack(lvl);
  if (out == 0) {
    sz += sz;
    goto restart;
  } else {
    return Yap_unify(ARG2, out);
  }
}

/** @brief output the difference between variables in _T_ and variables in
 * some list.
 */
#undef LIST_HOOK_CODE
#undef COMPOUND_HOOK_CODE
#undef ATOMIC_HOOK_CODE
#define LIST_HOOK_CODE                                                         \
  {}
#define COMPOUND_HOOK_CODE                                                     \
  {}
#define ATOMIC_HOOK_CODE                                                       \
  {}

static int bind_vars_in_complex_term(CELL *pt0_, CELL *pt0_end_,
                                     Ystack_t *stt USES_REGS) {

  COPY(pt0_[1]);
#include "term_visit.h"
  mBind_And_Trail(ptd0, TermFoundVar);
  if ((tr_fr_ptr)LOCAL_TrailTop - TR < 1024) {
    LOCAL_Error_TYPE = RESOURCE_ERROR_TRAIL;
    return 0;
  }
  continue;
  END_WALK();

  return 0;
}

/** @brief output the difference between variables in _T_ and variables in
 * some list.
 */

/** @pred  new_variables_in_term(+_CurrentVariables_, ? _Term_, -_Variables_)

    Unify  _Variables_ with the list of all variables of term
    _Term_ that do not occur in _CurrentVariables_. That is:

    `Variables = vars(Term) - CurrentVariables`

    The variables occur in
the order of their first appearance when traversing the term depth-first,
left-to-right.


*/
static Int
p_new_variables_in_term(USES_REGS1) /* variables within term t		 */
{
  Term out;
  Ystack_t stt_, *stt = &stt_;
  Term t, vs0;
 size_t sz = 1024;
  t = Deref(ARG2);
  vs0 = Deref(ARG1);
  if (IsVarTerm(t)) {
    t = MkGlobal(t);
  }
  if (IsPrimitiveTerm(t)) {
    out = TermNil;
  } 
 int lvl = push_text_stack();

  do {

    init_stack(stt,sz);
    bind_vars_in_complex_term(&(vs0)-1, &(vs0), stt PASS_REGS);
    reset_stack_but_not_trail(stt);
    out = vars_in_complex_term(&(t)-1, &(t), stt->tr0, TermNil,
			       stt PASS_REGS);
    
      RESET_TERM_VISITOR();
  } while (true);

    HB = B->cp_h;
  pop_text_stack(lvl);
  reset_trail(stt->tr0 PASS_REGS);


  return Yap_unify(ARG3, out);
}

#define FOUND_VAR                                                              \
  if (d0 == TermFoundVar) {                                                    \
    /* leave an empty slot to fill in later */                                 \
    if (HR + 1024 > ASP) {                                                     \
      LOCAL_Error_TYPE = RESOURCE_ERROR_TRAIL;                                 \
      return 0;                                                                \
    }                                                                          \
    HR[1] = AbsPair(HR + 2);                                                   \
    HR += 2;                                                                   \
    HR[-2] = (CELL)ptd0;                                                       \
    mTrailedMaBind(ptd0, TermNil);                                             \
  }

#undef LIST_HOOK_CODE
#undef COMPOUND_HOOK_CODE
#undef ATOMIC_HOOK_CODE
#define LIST_HOOK_CODE                                                         \
  {}
#define COMPOUND_HOOK_CODE                                                     \
  {}
#define ATOMIC_HOOK_CODE FOUND_VAR

#define FOUND_VAR_AGAIN_AND_ADD                                                \
  if (d0 == TermFoundVar) {                                                    \
    if (HR > ASP - 1024) {                                                     \
      LOCAL_Error_TYPE = RESOURCE_ERROR_STACK;                                 \
      return 0;                                                                \
    }                                                                          \
    if (end == NULL) {                                                         \
      first = AbsPair(HR);                                                     \
    } else {                                                                   \
      end[0] = AbsPair(HR);                                                    \
    }                                                                          \
    HR[0] = (CELL)ptd0;                                                        \
    end = HR + 1;                                                              \
    HR += 2;                                                                   \
    if (TR > (tr_fr_ptr)LOCAL_TrailTop - 256) {                                \
      /* Trail overflow */                                                     \
      LOCAL_Error_TYPE = RESOURCE_ERROR_TRAIL;                                 \
      return 0;                                                                \
    }                                                                          \
    mTrailedMaBind(ptd0, TermRefoundVar);                                      \
    continue;                                                                  \
  }

#undef LIST_HOOK_CODE
#undef COMPOUND_HOOK_CODE
#undef ATOMIC_HOOK_CODE
#define LIST_HOOK_CODE                                                         \
  {}
#define COMPOUND_HOOK_CODE                                                     \
  {}
#define ATOMIC_HOOK_CODE FOUND_VAR_AGAIN_AND_ADD

    static Term intersection_vars_in_complex_term(CELL *pt0_, CELL *pt0_end_,
                                                Ystack_t *stt USES_REGS) {
    CELL first = AbsPair(HR);
    CELL *end;

    COPY(pt0_[1]);
    #include "term_visit.h"
    continue;
    END_WALK();
    if (HR != stt->hlow) {
        HR[-1] = TermNil;
        return first;
    } else {
        return TermNil;
    }
    }

    /** @pred  variables_in_both_terms(+_CurrentVariables_, ? _Term_,
    -_Variables_)

        Unify _Variables_ with the list of all variables of term _Term_
        that *also* occur in _CurrentVariables_.  The variables occur in
        the order of their first appearance when traversing the term
        depth-first, left-to-right.

        This predicate performs the opposite of new_variables_in_term/3.

    */
    static Int p_variables_in_both_terms(USES_REGS1) /* variables within term t */
    {
    Term out, t, inp;
    Ystack_t stt_, *stt = &stt_;
    t = Deref(ARG2);
    inp = Deref(ARG1);
  if (IsVarTerm(t)) {
    t = MkGlobal(t);
  }
  if (IsPrimitiveTerm(t)){
        out = TermNil;
    }
int lvl = push_text_stack();
 size_t sz = 1024;
  do {

    if (!init_stack(stt, sz)) {
        LOCAL_Error_TYPE = RESOURCE_ERROR_AUXILIARY_STACK;
        return 0;
    }

        bind_vars_in_complex_term(&inp - 1, &inp, stt PASS_REGS);
        reset_stack_but_not_trail(stt);
        out = intersection_vars_in_complex_term(&(t)-1, &(t), stt PASS_REGS);
      RESET_TERM_VISITOR();
  } while (true);
    HB = B->cp_h;
  pop_text_stack(lvl);

    reset_trail(stt->tr0);

    return Yap_unify(ARG3, out);
    }

/* variables within term t		 */
static Int free_variables_in_term(USES_REGS1) {
  Term out;
  Term t;
  Term bounds;

  t = Deref(ARG1);
  bounds = TermNil;
  if (IsVarTerm(t)) {
    t = MkGlobal(t);
  }

  while (!IsVarTerm(t) && IsApplTerm(t)) {
    Functor f = FunctorOfTerm(t);
    if (f == FunctorHat) {
      bounds = MkPairTerm(ArgOfTerm(1, t), bounds);
      t = ArgOfTerm(2, t);
    } else if (f == FunctorModule && IsAtomTerm(ArgOfTerm(1, t))) {
      t = ArgOfTerm(2, t);
    } else if (f == FunctorCall) {
      t = ArgOfTerm(1, t);
    } else if (f == FunctorExecuteInMod) {
      t = ArgOfTerm(1, t);
    } else {
      break;
    }
  }

  if (IsPrimitiveTerm(t)) {
    
    return TermNil;
  } 
  Ystack_t stt_, *stt = &stt_;
 size_t sz = 1024;
 int lvl = push_text_stack();
   do {
 init_stack(stt, sz);
  //reset:

    bind_vars_in_complex_term(&bounds - 1, &bounds, stt PASS_REGS);
        reset_stack_but_not_trail(stt);
    tr_fr_ptr tr = TR;
	out = vars_in_complex_term((&t)-1, &t, tr, TermNil, stt PASS_REGS);
      RESET_TERM_VISITOR();
  } while (true);
  

  reset_trail(stt->tr0 PASS_REGS);
    HB = B->cp_h;
  pop_text_stack(lvl);


  return Yap_unify(ARG2, t) && Yap_unify(ARG3, out);
}
    
#define RENUMBER_VAR()                                                         \
  if (f == FunctorDollarVar) {                                                 \
    if (show_singletons && ptd1[1] == MkIntTerm(-1)) {                         \
      Term d0 = MkIntTerm(vno++);                                              \
            if (TR > (tr_fr_ptr)LOCAL_TrailTop - 256) {\
                /* Trail overflow */\
                LOCAL_Error_TYPE = RESOURCE_ERROR_TRAIL;\
                return 0;\
            }\
      MaBind(ptd1 + 1, d0);                                                    \
    }                                                                          \
  }

#undef LIST_HOOK_CODE
#undef COMPOUND_HOOK_CODE
#undef ATOMIC_HOOK_CODE

#define LIST_HOOK_CODE                                                         \
  {}
#define COMPOUND_HOOK_CODE RENUMBER_VAR()
#define ATOMIC_HOOK_CODE                                                       \
  {}
static Term numbervars_in_complex_term(CELL *pt0_, CELL *pt0_end_, size_t vno,
                                       bool show_singletons,
                                       Ystack_t *stt USES_REGS) {
    COPY(pt0_[1]);

#include "term_visit.h"

            /* next make sure noone will see this as a variable again */
            if (TR > (tr_fr_ptr)LOCAL_TrailTop - 256) {
                /* Trail overflow */
                LOCAL_Error_TYPE = RESOURCE_ERROR_TRAIL;
                return 0;
            }
            if (HR + 1024 > ASP) {
                LOCAL_Error_TYPE = RESOURCE_ERROR_STACK;
                return 0;
            }
            Term o = AbsAppl(HR);
            HR[0] = (CELL)FunctorDollarVar;
            if (show_singletons)
                HR[1] = MkIntTerm(-1);
            else

	      HR[1] = MkIntTerm(vno++);
            HR += 2;
            YapBind(ptd0, o);

            continue;
    END_WALK();
    return vno;
}

/** numbervariables in term t         */
size_t Yap_NumberVars(Term t, size_t numbv, bool handle_singles USES_REGS) {
    Ystack_t stt_, *stt = &stt_;
    size_t sz = 1024, rc;
  if (IsVarTerm(t)) {
    t = MkGlobal(t);
  }
    if (IsPrimitiveTerm(t)) {
    return numbv;
    }
 int lvl = push_text_stack();
   do {
    init_stack(stt, sz);
    t = Deref(t);
    HB=ASP;
     rc = numbervars_in_complex_term(&t - 1, &t, numbv, handle_singles,
                                           stt PASS_REGS);
      RESET_TERM_VISITOR();
   } while (true);
   HB=B->cp_h;
   pop_text_stack(lvl);
    return rc;
}

/** @pred  numbervars( _T_,+ _N1_,- _Nn_)

    Instantiates each variable in term  _T_ to a term of the form:
    `$VAR( _I_)`, with  _I_ increasing from  _N1_ to  _Nn_.
*/
static Int p_numbervars(USES_REGS1) {
    bool handle_singles = false;

    Term t, numbt;
    size_t out;
    t = Deref(ARG1);
    numbt = Deref(ARG2);
    if (IsVarTerm(t)) {
    t = MkGlobal(t);
  }
    if (IsVarTerm(numbt)) {
        Yap_Error(INSTANTIATION_ERROR, numbt, "numbervars/3");
        return false;
    }
    if (!IsIntegerTerm(numbt)) {
        Yap_Error(TYPE_ERROR_INTEGER, numbt, "numbervars/3");
        return (false);
    }
    Int numbv = IntegerOfTerm(numbt);
    if (IsPrimitiveTerm(t)) {
        return Yap_unify(ARG3, numbt);
    }
    Ystack_t stt_, *stt = &stt_;
   size_t sz = 1024;
     int lvl = push_text_stack();
  do {
    init_stack(stt, sz);
    HB = ASP;
    out = numbervars_in_complex_term(&t - 1, &t, numbv, handle_singles,
                                        stt PASS_REGS);

       RESET_TERM_VISITOR();
  } while (true);
    HB=B->cp_h;
 pop_text_stack(lvl);
 
  return Yap_unify(ARG3, MkIntegerTerm(out));
}

/** @pred  singleton_vs_numbervars( _T_,+ _N1_,- _Nn_)

    Instantiates each variable in term  _T_ to a term of the form:
    `$VAR( _I_)`, with  _I_ increasing from  _N1_ to  _Nn_.
*/

static Int singleton_vs_numbervars(USES_REGS1) {
    Ystack_t stt_, *stt = &stt_;

    Term vt, t;
    Int out;
     size_t sz = 1024;
    Functor fdv = FunctorDollarVar;
    FunctorDollarVar = FunctorHiddenVar;
    t = Deref(ARG1);
  if (IsVarTerm(t)) {
    t = MkGlobal(t);
  }
    vt = Deref(ARG2);
    if (IsVarTerm(vt)) {
        FunctorDollarVar = fdv;
        Yap_Error(INSTANTIATION_ERROR, vt, "numbervars/3");
        return false;
    }
    if (!IsIntegerTerm(vt)) {
        FunctorDollarVar = fdv;
        Yap_Error(TYPE_ERROR_INTEGER, vt, "numbervars/3");
        return (false);
    }
    size_t numbv = IntegerOfTerm(vt);
    if (IsPrimitiveTerm(t)) {
        FunctorDollarVar = fdv;
        return Yap_unify(ARG3, MkIntegerTerm(numbv));
    }
     int lvl = push_text_stack();
     do {
    if (!init_stack(stt, sz)) {
        LOCAL_Error_TYPE = RESOURCE_ERROR_AUXILIARY_STACK;
        return 0;
    }
   out = numbervars_in_complex_term(&t - 1, &t, numbv, true,
                                        stt PASS_REGS);
      RESET_TERM_VISITOR();
  } while (true);
    FunctorDollarVar = fdv;
  pop_text_stack(lvl);
  return Yap_unify(ARG3, MkIntegerTerm(out));
}


#define HARD_RENUMBER_VAR()                                                         \
  if (f == FunctorDollarVar) {                                                 \
    if (show_singletons && ptd1[1] == MkIntTerm(-1)) {                         \
      Term d0 = MkIntTerm(vno++);                                              \
      TrailedMaBind(ptd1 + 1, d0);                                                    \
    }                                                                          \
  }

#undef LIST_HOOK_CODE
#undef COMPOUND_HOOK_CODE
#undef ATOMIC_HOOK_CODE

#define LIST_HOOK_CODE                                                         \
  {}
#define COMPOUND_HOOK_CODE HARD_RENUMBER_VAR()
#define ATOMIC_HOOK_CODE                                                       \
  {}


static Term hard_numbervars_in_complex_term(CELL *pt0_, CELL *pt0_end_, size_t vno,
                                            bool show_singletons,
                                            Ystack_t *stt USES_REGS) {
    COPY(pt0_[1]);

#include "term_visit.h"

            /* next make sure noone will see this as a variable again */
            if (TR > (tr_fr_ptr)LOCAL_TrailTop - 256) {
                /* Trail overflow */
                LOCAL_Error_TYPE = RESOURCE_ERROR_TRAIL;
                return 0;
            }
            if (HR + 1024 > ASP) {
                LOCAL_Error_TYPE = RESOURCE_ERROR_STACK;
                return 0;
            }
            Term o = AbsAppl(HR);
            HR[0] = (CELL)FunctorDollarVar;
            if (show_singletons)
                HR[1] = MkIntTerm(-1);
            else
                HR[1] = MkIntTerm(vno++);
            HR += 2;
            mBind_And_Trail(ptd0, o);

            continue;
    END_WALK();
    return vno;
}

/** numbervariables in term t         */
size_t Yap_HardNumberVars(Term t, size_t numbv, bool handle_singles USES_REGS) {
    size_t rc;
    if (IsPrimitiveTerm(t)) {
    return numbv;
    }
    HB = HR;
    Ystack_t stt_, *stt = &stt_;
     size_t sz = 1024;
 int lvl = push_text_stack();
  do {
   t = Deref(t);
  if (IsVarTerm(t)) {
    t = MkGlobal(t);
  }
    init_stack(stt, sz);
    rc = hard_numbervars_in_complex_term(&t - 1, &t, numbv, handle_singles,
                                                stt PASS_REGS);
      RESET_TERM_VISITOR();
  } while (true);
  pop_text_stack(lvl);
  return rc;
}


void Yap_InitTermCPreds(void) {
    Yap_InitCPred("cyclic_term", 1, cyclic_term, SafePredFlag);

    Yap_InitCPred("ground", 1, ground, SafePredFlag);
           Yap_InitCPred("numbervars", 3, p_numbervars, 0);
    Yap_InitCPred("$singleton_vs_numbervars", 3, singleton_vs_numbervars, 0);
    CurrentModule = TERMS_MODULE;
        Yap_InitCPred("variable_in_term", 2, variable_in_term, 0);
        Yap_InitCPred("new_variables_in_term", 3, p_new_variables_in_term, 0);
    Yap_InitCPred("variables_in_both_terms", 3, p_variables_in_both_terms, 0);
    CurrentModule = PROLOG_MODULE;
#if 1
    Yap_InitCPred("term_variables", 2, term_variables, 0);
    Yap_InitCPred("term_variables", 3, term_variables3, 0);
    Yap_InitCPred("variables_in_term", 3, variables_in_term, 0);
    Yap_InitCPred("$variables_in_term", 3, variables_in_term, 0);

    Yap_InitCPred("$free_variables_in_term", 3, free_variables_in_term, 0);
    Yap_InitCPred("free_variables_in_term", 3, free_variables_in_term, 0);

    Yap_InitCPred("term_attvars", 2, term_attvars, 0);

    Yap_InitCPred("variables_in_both_terms", 3, p_variables_in_both_terms, 0);






#endif
  }

#endif

  ///@}

  
