/*************************************************************************
*									 *
*	 YAP Prolog 	@(#)YapEval.h	1.2
*									 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		YapEval.h							 *
* Last rev:								 *
* mods:									 *
* comments:	arithmetical functions info				 *
*									 *
*************************************************************************/

/**

@file YapEval.h
*/

/**

      @defgroup arithmetic Arithmetic in YAP

    @ingroup builtins

    @{
+ See @ref arithmetic_predicates for the predicates that implement arithmetic.

+ See @ref arithmetic_cmps for the arithmetic comparisons supported in YAP

+ See @ref arithmetic_operators for what arithmetic operations are supported
in YAP

    YAP supports several  numeric types:

1. *Tagged integers*

 YAP supports integers of up to almost the CPU word size: 28 bits on 32-bit
 machines, and 60-bits on 64-bit machines.The engine tags
 smaller integers are tagged so that they fit in a single word. These
 are the so called <em>tagged integers</em>.

2. *Large integers*

 Integers that cannot fit a tag, but who can fit a word are represented
 by adding a special functor, and a special end cell.  The difference
 between these integers and tagged integers should be transparent to
 the programmer.

3. *Multiple Precision Integers*

When YAP is built using the GNU multiple precision arithmetic library
(GMP), integer arithmetic is unbounded, which means that the size of
integers is only limited by available memory. The type of integer
support can be detected using the Prolog flags bounded, min_integer
and max_integer. As the use of GMP is default, most of the following
descriptions assume unbounded integer arithmetic.

4. *Rational numbers (Q)*

Rational numbers are quotients of two integers. Rational arithmetic is
provided if GMP is used. Rational numbers that are returned from is/2
are canonical, which means the denominator _M_ is positive and that
the numerator _N_ and _M_ have no common divisors. Rational numbers
are introduced in the computation using the rational/1, rationalize/1
or the rdiv/2 (rational division) function.

5. *Floating point numbers*

  Floating point numbers are represented using the C-type double. On
most today platforms these are 64-bit IEEE-754 floating point
numbers. YAP now includes the built-in predicates isinf/1 and to
isnan/1 tests.

Arithmetic functions that require integer arguments accept, in addition
to integers, rational numbers with denominator `1' and floating point
numbers that can be accurately converted to integers. If the required
argument is a float the argument is converted to float. Note that
conversion of integers to floating point numbers may raise an overflow
exception. In all other cases, arguments are converted to the same type
using the order integer to rational number to floating point number.

Evaluation generates the following _Call_
exceptions:

   @exception "error(instantiation_error, Call )" if not ground

   @exception "type_error(evaluable( V ), Call)" if not evaluable term
   @exception "type_error(integer( V ), Call)" if must be integer
   @exception "type_error(float( V ), Call)" if must be float

   @exception "domain_error(out_of_range( V ), Call)" if argument invalid
   @exception "domain_error(not_less_than_zero( V ), Call)" if argument must be
positive or zero

   @exception "evaluation_error(undefined( V ), Call)" result is not defined
(nan)
   @exception "evaluation_error(overflow( V ), Call)" result is arithmetic
overflow

@}

 **/

#ifndef EVAL_H
#define EVAL_H 1

#include <stdlib.h>

/* C library used to implement floating point functions */
#if HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_FENV_H
#include <fenv.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef LONG_MAX
#define Int_MAX LONG_MAX
#else
#define Int_MAX ((Int)((~((CELL)0)) >> 1))
#endif
#ifdef LONG_MIN
#define Int_MIN LONG_MIN
#else
#define Int_MIN (-Int_MAX - (CELL)1)
#endif

#define PLMAXTAGGEDINT (MAX_ABS_INT - ((CELL)1))
#define PLMINTAGGEDINT (-MAX_ABS_INT)

#define PLMAXINT Int_MAX
#define PLMININT Int_MIN

#ifndef INFINITY
#define INFINITY (1.0 / 0.0)
#endif

#ifndef NAN
#define NAN (0.0 / 0.0)
#endif

/**
 * @addtogroup arithmetic_operators
 * @enum arith0_op constant operators
 * @brief specifies the available arithmetic "constants".
*/
typedef enum {
  /** pi [ISO]

     An approximation to the value of <em>pi</em>, that is, the ratio of a
   circle's circumference to its diameter.
   *
   */
  op_pi,
  /** e

     Euler's number, the base of the natural logarithms (approximately
   2.718281828).
   *
   */
  op_e,
  /** epsilon

     The difference between the float `1.0` and the next largest floating point
   number.
   *
   */
  op_epsilon,
  /** inf

     Infinity according to the IEEE Floating-Point standard. Note that
   evaluating this term will generate a domain error in the `iso` language mode.
   Also note that
   *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.prolog}
   *  ?- +inf =:= -inf.
   * false.
   *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   */
  op_inf,
  op_nan,
  op_random,
  op_cputime,
  op_heapused,
  op_localsp,
  op_globalsp,
  op_b,
  op_env,
  op_tr,
  op_stackfree
} arith0_op;

/**
 * @addtogroup arithmetic_operators
 * @enum arith1_op unary operators
 * @brief specifies the available unary arithmetic operators
 */
typedef enum {
  /** \+ _X_: the value of _X_ .
   *
   *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.prolog}
   *  X =:= +X.
   *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  op_uplus,
  /**  \- _X_: the complement of _X_ .
   *
   *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.prolog}
   *  0-X =:= -X.
   *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   */
  op_uminus,
  /** \\ _X_, The bitwise negation of _X_ .
   *
   *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.prolog}
   *  \X /\ X =:= 0.
   *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   * Note that the number of bits of an integer is at least the size in bits of
   * a Prolog term cell.
   */
  op_unot,
  /** exp( _X_ ), natural exponentiation of _X_ .
   *
   *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.prolog}
   *  X = 0.0, abs(1.0 - exp( _X_ )) < 0.0001
   *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   */
  op_exp,
  /** log( _X_ ), natural logarithm of _X_ .
   *
   *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.prolog}
   *  X = 1.0, abs( log( exp( _X_ )) -1.0) < 0.0001
   *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *
   */
  op_log,
  /** log10( _X_ ) [ISO]
        *
    * Decimal logarithm.
    *
    *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.prolog}
    *   ?- between(1, 10, I), Delta is log10(I*10) + log10(1/(I*10)), format('0
   * == ~3g~n',[Delta]), fail.
    *   0 == 0
    *   0 == 0
    *   0 == 0
    *   0 == 0
    *   0 == 0
    *   0 == 0
    *   0 == 0
    *   0 == 0
    *   0 == 2.22e-16
    *   0 == 0
    *   false.
    *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    */
  op_log10,
  op_sqrt,
  op_sin,
  op_cos,
  op_tan,
  op_sinh,
  op_cosh,
  op_tanh,
  op_asin,
  op_acos,
  op_atan,
  op_asinh,
  op_acosh,
  op_atanh,
  op_floor,
  op_ceiling,
  op_round,
  op_truncate,
  op_integer,
  op_float,
  op_abs,
  op_lsb,
  op_msb,
  op_popcount,
  op_ffracp,
  op_fintp,
  op_sign,
  op_lgamma,
  op_erf,
  op_erfc,
  op_rational,
  op_rationalize,
  op_random1
} arith1_op;

/**
 * @addtogroup arithmetic_operators
 * @{
 * @enum arith2_op binary operators
 * @brief specifies the available binary arithmetic operators
 */
typedef enum {
	      op_plus, //>  *_X_+ _Y_ [ISO]*,
	      //> Addition, implemented between any two    types of numbers
	      op_minus, //>  *_X_- _Y_ [ISO]*,
	      //> Subtraction, implemented between any two    types of numbers
	      op_times, //> *_X_\* _Y_ [ISO]*,
	      //> Product.
	      op_fdiv, //> *_X_/ _Y_ [ISO]*,
	      //> Floating Point Division.
  op_mod, //> *_X_ mod _Y_ [ISO]* @anchor mod_2,
//> Integer Modulus, always positive
  op_rem, //> *_X_ rem _Y_ [ISO]* @anchor rem_2,
//> Integer Remainder, always with the same size as the first argument, _X_.
  op_div, //>*_X_// _Y_ [ISO]*,
//>   Integer division.
  op_idiv, //* _X_ div  _Y_ [ISO]* @anchor div_2,
	      //>   Integer division, as if defined by `( _X_ -  _X_ mod  _Y_)//  _Y_`.

  op_sll, //>
  op_slr, //>
  op_and, //>
  op_or, //>
  op_xor, //>
  op_atan2, //>
  /* C-Prolog exponentiation */
  op_power, //>
  /* ISO-Prolog exponentiation */
  /*  op_power, //> */
  op_power2, //>
  /* Quintus exponentiation */
  /* op_power, //> */
  op_gcd, //>
  op_min, //>
  op_max, //>
	      op_rdiv //>
} arith2_op;

///@}

yap_error_number Yap_MathException__(USES_REGS1);
Functor EvalArg(Term);

/* Needed to handle numbers:
        these two macros are fundamental in the integer/float conversions */

#ifdef C_PROLOG
#define FlIsInt(X) ((X) == (Int)(X) && IntInBnd((X)))
#else
#define FlIsInt(X) (FALSE)
#endif

#ifdef M_WILLIAMS
#define MkEvalFl(X) MkFloatTerm(X)
#else
#define MkEvalFl(X) (FlIsInt(X) ? MkIntTerm((Int)(X)) : MkFloatTerm(X))
#endif

/* Macros used by some of the eval functions */
#define REvalInt(I)                                                            \
  {                                                                            \
    eval_int = (I);                                                            \
    return (FInt);                                                             \
  }
#define REvalFl(F)                                                             \
  {                                                                            \
    eval_flt = (F);                                                            \
    return (FFloat);                                                           \
  }

#define REvalError()                                                           \
  { return (FError); }

/* this macro, dependent on the particular implementation
        is used to interface the arguments into the C libraries */
#ifdef MPW
#define FL(X) ((extended)(X))
#else
#define FL(X) ((double)(X))
#endif

void Yap_InitConstExps(void);
void Yap_InitUnaryExps(void);
void Yap_InitBinaryExps(void);

int Yap_ReInitConstExps(void);
int Yap_ReInitUnaryExps(void);
int Yap_ReInitBinaryExps(void);

Term Yap_eval_atom(Int);
Term Yap_eval_unary(Int, Term);
Term Yap_eval_binary(Int, Term, Term);

Term Yap_InnerEval__(Term USES_REGS);

#define Yap_EvalError(id, t, ...)                                              \
  Yap_EvalError__(__FILE__, __FUNCTION__, __LINE__, id, t, __VA_ARGS__)
void Yap_EvalError__(const char *, const char *, int, yap_error_number, Term,
                       ...);

#define Yap_ArithError(id, t, ...)                                             \
  Yap_ThrowError__(__FILE__, __FUNCTION__, __LINE__, id, t, __VA_ARGS__)
#define Yap_BinError(id)                                                       \
  Yap_Error__(true,__FILE__, __FUNCTION__, __LINE__, id, 0L, "")
#define Yap_AbsmiError(id)                                                     \
  Yap_Error__(true,__FILE__, __FUNCTION__, __LINE__, id, 0L, "")


#include "inline-only.h"

#define Yap_MathException() Yap_MathException__(PASS_REGS1)

#define Yap_InnerEval(x) Yap_InnerEval__(x PASS_REGS)
#define Yap_Eval(x) Yap_Eval__(x PASS_REGS)
#define Yap_FoundArithError() Yap_FoundArithError__(PASS_REGS1)

INLINE_ONLY Term Yap_Eval__(Term t USES_REGS);

INLINE_ONLY Term Yap_Eval__(Term t USES_REGS) {
  if (t == 0L || (!IsVarTerm(t) && IsNumTerm(t)))
    return t;
  return Yap_InnerEval(t);
}

#if HAVE_FECLEAREXCEPT
inline static void Yap_ClearExs(void) { feclearexcept(FE_ALL_EXCEPT); }
#else
inline static void Yap_ClearExs(void) {}
#endif

inline static yap_error_number Yap_FoundArithError__(USES_REGS1) {
  if (LOCAL_Error_TYPE != YAP_NO_ERROR)
    return LOCAL_Error_TYPE;
  if (trueGlobalPrologFlag(
          ARITHMETIC_EXCEPTIONS_FLAG)) // test support for exception
    return Yap_MathException();
  return YAP_NO_ERROR;
}

static inline Term takeIndicator(Term t) {
  Term ts[2];
  if (IsAtomTerm(t)) {
    ts[0] = t;
    ts[1] = MkIntTerm(0);
  } else if (IsPairTerm(t)) {
    ts[0] = TermNil;
    ts[1] = MkIntTerm(2);
  } else {
    CACHE_REGS
    ts[0] = MkAtomTerm(NameOfFunctor(FunctorOfTerm(t)));
    ts[1] = MkIntegerTerm(ArityOfFunctor(FunctorOfTerm(t)));
  }
  return Yap_MkApplTerm(FunctorSlash, 2, ts);
}

Atom Yap_NameOfUnaryOp(int i);
Atom Yap_NameOfBinaryOp(int i);

#define RINT(v) return (MkIntegerTerm(v))
#define RFLOAT(v) return (MkFloatTerm(v))
#define RBIG(v) return (Yap_MkBigIntTerm(v))
#define RERROR()                                                               \
  {                                                                            \
    return (0L);                                                               \
  }

static inline blob_type ETypeOfTerm(Term t) {
  if (IsIntTerm(t))
    return long_int_e;
  if (IsApplTerm(t)) {
    Functor f = FunctorOfTerm(t);
    if (f == FunctorDouble)
      return double_e;
    if (f == FunctorLongInt)
      return long_int_e;
    if (f == FunctorBigInt) {
      return big_int_e;
    }
  }
  return db_ref_e;
}

char *Yap_mpz_to_string(MP_INT *b, char *s, size_t sz, int base);

Term Yap_gmq_rdiv_int_int(Int, Int);
Term Yap_gmq_rdiv_int_big(Int, Term);
Term Yap_gmq_rdiv_big_int(Term, Int);
Term Yap_gmq_rdiv_big_big(Term, Term);

Term Yap_gmp_add_ints(Int, Int);
Term Yap_gmp_sub_ints(Int, Int);
Term Yap_gmp_mul_ints(Int, Int);
Term Yap_gmp_sll_ints(Int, Int);
Term Yap_gmp_add_int_big(Int, Term);
Term Yap_gmp_sub_int_big(Int, Term);
Term Yap_gmp_sub_big_int(Term, Int);
Term Yap_gmp_mul_int_big(Int, Term);
Term Yap_gmp_div_int_big(Int, Term);
Term Yap_gmp_div_big_int(Term, Int);
Term Yap_gmp_div2_big_int(Term, Int);
Term Yap_gmp_fdiv_int_big(Int, Term);
Term Yap_gmp_fdiv_big_int(Term, Int);
Term Yap_gmp_and_int_big(Int, Term);
Term Yap_gmp_ior_int_big(Int, Term);
Term Yap_gmp_xor_int_big(Int, Term);
Term Yap_gmp_sll_big_int(Term, Int);
Term Yap_gmp_add_big_big(Term, Term);
Term Yap_gmp_sub_big_big(Term, Term);
Term Yap_gmp_mul_big_big(Term, Term);
Term Yap_gmp_div_big_big(Term, Term);
Term Yap_gmp_div2_big_big(Term, Term);
Term Yap_gmp_fdiv_big_big(Term, Term);
Term Yap_gmp_and_big_big(Term, Term);
Term Yap_gmp_ior_big_big(Term, Term);
Term Yap_gmp_xor_big_big(Term, Term);
Term Yap_gmp_mod_big_big(Term, Term);
Term Yap_gmp_mod_big_int(Term, Int);
Term Yap_gmp_mod_int_big(Int, Term);
Term Yap_gmp_rem_big_big(Term, Term);
Term Yap_gmp_rem_big_int(Term, Int);
Term Yap_gmp_rem_int_big(Int, Term);
Term Yap_gmp_exp_int_int(Int, Int);
Term Yap_gmp_exp_int_big(Int, Term);
Term Yap_gmp_exp_big_int(Term, Int);
Term Yap_gmp_exp_big_big(Term, Term);
Term Yap_gmp_gcd_int_big(Int, Term);
Term Yap_gmp_gcd_big_big(Term, Term);

Term Yap_gmp_big_from_64bits(YAP_LONG_LONG);

Term Yap_gmp_float_to_big(Float);
Term Yap_gmp_float_to_rational(Float);
Term Yap_gmp_float_rationalize(Float);
Float Yap_gmp_to_float(Term);
Term Yap_gmp_add_float_big(Float, Term);
Term Yap_gmp_sub_float_big(Float, Term);
Term Yap_gmp_sub_big_float(Term, Float);
Term Yap_gmp_mul_float_big(Float, Term);
Term Yap_gmp_fdiv_float_big(Float, Term);
Term Yap_gmp_fdiv_big_float(Term, Float);

int Yap_gmp_cmp_big_int(Term, Int);
int Yap_gmp_cmp_int_big(Int, Term);
int Yap_gmp_cmp_big_float(Term, Float);
#define Yap_gmp_cmp_float_big(D, T) (-Yap_gmp_cmp_big_float(T, D))
int Yap_gmp_cmp_big_big(Term, Term);

int Yap_gmp_tcmp_big_int(Term, Int);
int Yap_gmp_tcmp_int_big(Int, Term);
int Yap_gmp_tcmp_big_float(Term, Float);
#define Yap_gmp_tcmp_float_big(D, T) (-Yap_gmp_tcmp_big_float(T, D))
int Yap_gmp_tcmp_big_big(Term, Term);

Term Yap_gmp_neg_int(Int);
Term Yap_gmp_abs_big(Term);
Term Yap_gmp_neg_big(Term);
Term Yap_gmp_unot_big(Term);
Term Yap_gmp_floor(Term);
Term Yap_gmp_ceiling(Term);
Term Yap_gmp_round(Term);
Term Yap_gmp_trunc(Term);
Term Yap_gmp_float_fractional_part(Term);
Term Yap_gmp_float_integer_part(Term);
Term Yap_gmp_sign(Term);
Term Yap_gmp_lsb(Term);
Term Yap_gmp_msb(Term);
Term Yap_gmp_popcount(Term);

char *Yap_gmp_to_string(Term, char *, size_t, int);
size_t Yap_gmp_to_size(Term, int);

int Yap_term_to_existing_big(Term, MP_INT *);
int Yap_term_to_existing_rat(Term, MP_RAT *);

void Yap_gmp_set_bit(Int i, Term t);

#define Yap_Mk64IntegerTerm(i) __Yap_Mk64IntegerTerm((i)PASS_REGS)

INLINE_ONLY Term __Yap_Mk64IntegerTerm(YAP_LONG_LONG USES_REGS);

INLINE_ONLY Term
__Yap_Mk64IntegerTerm(YAP_LONG_LONG i USES_REGS) {
  if (i <= Int_MAX && i >= Int_MIN) {
    return MkIntegerTerm((Int)i);
  } else {
    return Yap_gmp_big_from_64bits(i); 
 }
}

#if __clang__ && FALSE /* not in OSX yet */
#define DO_ADD()                                                               \
  if (__builtin_sadd_overflow(i1, i2, &z)) {                                   \
    goto overflow;                                                             \
  }
#endif

inline static Term add_int(Int i, Int j USES_REGS) {
  UInt w = (UInt)i + (UInt)j;
  if (i > 0) {
    if (j > 0 && (Int)w < 0)
      goto overflow;
  } else {
    if (j < 0 && (Int)w > 0)
      goto overflow;
  }
  RINT((Int)w);
/* Integer overflow, we need to use big integers */
overflow:
  return Yap_gmp_add_ints(i, j);
}

/* calculate the most significant bit for an integer */
Int Yap_msb(Int inp USES_REGS);

static inline Term p_plus(Term t1, Term t2 USES_REGS) {
  switch (ETypeOfTerm(t1)) {
  case long_int_e:
    switch (ETypeOfTerm(t2)) {
    case long_int_e:
      /* two integers */
      return add_int(IntegerOfTerm(t1), IntegerOfTerm(t2) PASS_REGS);
    case double_e: {
      /* integer, double */
      Float fl1 = (Float)IntegerOfTerm(t1);
      Float fl2 = FloatOfTerm(t2);
      RFLOAT(fl1 + fl2);
    }
    case big_int_e:
      return (Yap_gmp_add_int_big(IntegerOfTerm(t1), t2));
    default:
      RERROR();
    }
  case double_e:
    switch (ETypeOfTerm(t2)) {
    case long_int_e:
      /* float * integer */
      RFLOAT(FloatOfTerm(t1) + IntegerOfTerm(t2));
    case double_e:
      RFLOAT(FloatOfTerm(t1) + FloatOfTerm(t2));
    case big_int_e:
      return Yap_gmp_add_float_big(FloatOfTerm(t1), t2);
    default:
      RERROR();
    }
  case big_int_e:
    switch (ETypeOfTerm(t2)) {
    case long_int_e:
      return Yap_gmp_add_int_big(IntegerOfTerm(t2), t1);
    case big_int_e:
      /* two bignums */
      return Yap_gmp_add_big_big(t1, t2);
    case double_e:
      return Yap_gmp_add_float_big(FloatOfTerm(t2), t1);
    default:
      RERROR();
    }
  default:
    RERROR();
  }
  RERROR();
}

#ifndef PI
#ifdef M_PI
#define PI M_PI
#else
#define PI 3.14159265358979323846
#endif
#endif

#ifndef M_E
#define M_E 2.7182818284590452354
#endif

#ifndef INFINITY
#define INFINITY (1.0 / 0.0)
#endif

#ifndef NAN
#define NAN (0.0 / 0.0)
#endif

/* copied from SWI-Prolog */
#ifndef DBL_EPSILON /* normal for IEEE 64-bit double */
#define DBL_EPSILON 0.00000000000000022204
#endif

#endif
