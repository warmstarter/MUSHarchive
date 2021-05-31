/**
 * \file funmath.c
 *
 * \brief Mathematical functions for mushcode.
 * 
 *
 */

#include "copyrite.h"

#include "config.h"
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "conf.h"
#include "externs.h"

#include "parse.h"
#include "htab.h"
#include "confmagic.h"

#ifdef WIN32
#pragma warning( disable : 4761)	/* NJG: disable warning re conversion */
#endif

#ifndef M_PI
/** The ratio of the circumference of a circle to its denominator. */
#define M_PI 3.14159265358979323846264338327
#endif

static void do_spellnum(char *num, unsigned int len, char **buff, char ***bp);
static int nval_sort(const void *, const void *);
static NVAL find_median(NVAL *, int);

#define MATH_FUNC(func) static void func(char **ptr, int nptr, char *buff, char **bp)

#define MATH_PROTO(func) static void func (char **ptr, int nptr, char *buff, char **bp)

HASHTAB htab_math;

/** A math function. */
typedef struct {
  const char *name;	/**< Name of the function. */
  void (*func) (char **, int, char *, char **);	/**< Pointer to function code. */
} MATH;

static void math_hash_insert(const char *, MATH *);
static MATH *math_hash_lookup(char *);
static NVAL angle_to_rad(NVAL angle, const char *from);
static NVAL rad_to_angle(NVAL angle, const char *to);

MATH_PROTO(math_add);
MATH_PROTO(math_sub);
MATH_PROTO(math_mul);
MATH_PROTO(math_div);
MATH_PROTO(math_floordiv);
MATH_PROTO(math_remainder);
MATH_PROTO(math_modulo);
MATH_PROTO(math_min);
MATH_PROTO(math_max);
MATH_PROTO(math_and);
MATH_PROTO(math_nand);
MATH_PROTO(math_or);
MATH_PROTO(math_nor);
MATH_PROTO(math_xor);
MATH_PROTO(math_band);
MATH_PROTO(math_bor);
MATH_PROTO(math_bxor);
MATH_PROTO(math_fdiv);
MATH_PROTO(math_mean);
MATH_PROTO(math_median);
MATH_PROTO(math_stddev);

/* ARGSUSED */
FUNCTION(fun_ctu)
{
  NVAL angle;
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }

  if (!args[1] || !args[2]) {
    safe_str(T("#-1 INVALID ANGLE TYPE"), buff, bp);
    return;
  }
  angle = angle_to_rad(parse_number(args[0]), args[1]);
  safe_number(rad_to_angle(angle, args[2]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_add)
{
  math_add(args, nargs, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_sub)
{
  math_sub(args, nargs, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_mul)
{
  math_mul(args, nargs, buff, bp);
}

/* TO-DO: I have better code for comparing floating-point numbers
   lying around somewhere. The idea is that numbers that are very
   close can be 'close enough' to be equal or whatever, without having
   to be exactly the same. */

/* ARGSUSED */
FUNCTION(fun_gt)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  safe_boolean(parse_number(args[0]) > parse_number(args[1]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_gte)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  safe_boolean(parse_number(args[0]) >= parse_number(args[1]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_lt)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  safe_boolean(parse_number(args[0]) < parse_number(args[1]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_lte)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  safe_boolean(parse_number(args[0]) <= parse_number(args[1]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_eq)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  safe_boolean(parse_number(args[0]) == parse_number(args[1]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_neq)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  safe_boolean(parse_number(args[0]) != parse_number(args[1]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_max)
{
  math_max(args, nargs, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_min)
{
  math_min(args, nargs, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_sign)
{
  NVAL x;

  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  x = parse_number(args[0]);
  if (x == 0)
    safe_chr('0', buff, bp);
  else if (x > 0)
    safe_chr('1', buff, bp);
  else
    safe_str("-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_shl)
{
  if (!is_uinteger(args[0]) || !is_uinteger(args[1])) {
    safe_str(T(e_uints), buff, bp);
    return;
  }
  safe_uinteger(parse_uinteger(args[0]) << parse_uinteger(args[1]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_shr)
{
  if (!is_uinteger(args[0]) || !is_uinteger(args[1])) {
    safe_str(T(e_uints), buff, bp);
    return;
  }
  safe_uinteger(parse_uinteger(args[0]) >> parse_uinteger(args[1]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_inc)
{
  int num;
  char *p;
  /* Handle the case of a pure number */
  if (is_strict_integer(args[0])) {
    safe_integer(parse_integer(args[0]) + 1, buff, bp);
    return;
  }
  /* Handle a null string */
  if (!*args[0]) {
    safe_str(NULL_EQ_ZERO ? "1" : T("#-1 ARGUMENT MUST END IN AN INTEGER"),
	     buff, bp);
    return;
  }
  p = args[0] + arglens[0] - 1;
  if (!isdigit((unsigned char) *p)) {
    if (NULL_EQ_ZERO) {
      safe_str(args[0], buff, bp);
      safe_str("1", buff, bp);
    } else
      safe_str(T("#-1 ARGUMENT MUST END IN AN INTEGER"), buff, bp);
    return;
  }
  while ((isdigit((unsigned char) *p) || (*p == '-')) && p != args[0]) {
    if (*p == '-') {
      p--;
      break;
    }
    p--;
  }
  /* p now points to the last non-numeric character in the string
   * Move it to the first numeric character
   */
  p++;
  num = parse_integer(p) + 1;
  *p = '\0';
  safe_str(args[0], buff, bp);
  safe_integer(num, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_dec)
{
  int num;
  char *p;
  /* Handle the case of a pure number */
  if (is_strict_integer(args[0])) {
    safe_integer(parse_integer(args[0]) - 1, buff, bp);
    return;
  }
  /* Handle a null string */
  if (!*args[0]) {
    safe_str(NULL_EQ_ZERO ? "-1" : T("#-1 ARGUMENT MUST END IN AN INTEGER"),
	     buff, bp);
    return;
  }
  p = args[0] + arglens[0] - 1;
  if (!isdigit((unsigned char) *p)) {
    if (NULL_EQ_ZERO) {
      safe_str(args[0], buff, bp);
      safe_str("-1", buff, bp);
    } else
      safe_str(T("#-1 ARGUMENT MUST END IN AN INTEGER"), buff, bp);
    return;
  }
  while ((isdigit((unsigned char) *p) || (*p == '-')) && p != args[0]) {
    if (*p == '-') {
      p--;
      break;
    }
    p--;
  }
  /* p now points to the last non-numeric character in the string
   * Move it to the first numeric character
   */
  p++;
  num = parse_integer(p) - 1;
  *p = '\0';
  safe_str(args[0], buff, bp);
  safe_integer(num, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_trunc)
{
  /* This function does not have the non-number check because
   * the help file explicitly states that this function can
   * be used to turn "101dalmations" into "101".
   */
  safe_integer(parse_integer(args[0]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_div)
{
  math_div(args, nargs, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_floordiv)
{
  math_floordiv(args, nargs, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_modulo)
{
  math_modulo(args, nargs, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_remainder)
{
  math_remainder(args, nargs, buff, bp);
}


/* ARGSUSED */
FUNCTION(fun_abs)
{
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  safe_number(fabs(parse_number(args[0])), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_dist2d)
{
  NVAL d1, d2, r;

  if (!is_number(args[0]) || !is_number(args[1]) ||
      !is_number(args[2]) || !is_number(args[3])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  d1 = parse_number(args[0]) - parse_number(args[2]);
  d2 = parse_number(args[1]) - parse_number(args[3]);
  r = d1 * d1 + d2 * d2;
#ifndef HAS_IEEE_MATH
  /* You can overflow, which is bad. */
  if (r < 0) {
    safe_str(T("#-1 OVERFLOW ERROR"), buff, bp);
    return;
  }
#endif
  safe_number(sqrt(r), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_dist3d)
{
  NVAL d1, d2, d3, r;

  if (!is_number(args[0]) || !is_number(args[1]) ||
      !is_number(args[2]) || !is_number(args[3]) ||
      !is_number(args[4]) || !is_number(args[5])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  d1 = parse_number(args[0]) - parse_number(args[3]);
  d2 = parse_number(args[1]) - parse_number(args[4]);
  d3 = parse_number(args[2]) - parse_number(args[5]);
  r = d1 * d1 + d2 * d2 + d3 * d3;
#ifndef HAS_IEEE_MATH
  /* You can overflow, which is bad. */
  if (r < 0) {
    safe_str(T("#-1 OVERFLOW ERROR"), buff, bp);
    return;
  }
#endif
  safe_number(sqrt(r), buff, bp);
}

/* ------------------------------------------------------------------------
 * Dune's vector functions: VADD, VSUB, VMUL, VCROSS, VMAG, VUNIT, VDIM
 *  VCRAMER?
 * Vectors are space-separated numbers.
 */

/* ARGSUSED */
FUNCTION(fun_vmax)
{
  char *p1, *p2;
  char *start;
  char sep;
  NVAL a, b;

  /* return if a list is empty */
  if (!args[0] || !args[1])
    return;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;
  p1 = trim_space_sep(args[0], sep);
  p2 = trim_space_sep(args[1], sep);

  /* return if a list is empty */
  if (!*p1 || !*p2)
    return;

  /* max the vectors */
  start = *bp;
  a = parse_number(split_token(&p1, sep));
  b = parse_number(split_token(&p2, sep));
  safe_number((a > b) ? a : b, buff, bp);

  while (p1 && p2) {
    safe_chr(sep, buff, bp);
    a = parse_number(split_token(&p1, sep));
    b = parse_number(split_token(&p2, sep));
    safe_number((a > b) ? a : b, buff, bp);
  }

  /* make sure vectors were the same length */
  if (p1 || p2) {
    *bp = start;
    safe_str(T("#-1 VECTORS MUST BE SAME DIMENSIONS"), buff, bp);
    return;
  }
}

/* ARGSUSED */
FUNCTION(fun_vmin)
{
  char *p1, *p2;
  char *start;
  char sep;
  NVAL a, b;

  /* return if a list is empty */
  if (!args[0] || !args[1])
    return;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;
  p1 = trim_space_sep(args[0], sep);
  p2 = trim_space_sep(args[1], sep);

  /* return if a list is empty */
  if (!*p1 || !*p2)
    return;

  /* max the vectors */
  start = *bp;
  a = parse_number(split_token(&p1, sep));
  b = parse_number(split_token(&p2, sep));
  safe_number((a < b) ? a : b, buff, bp);

  while (p1 && p2) {
    safe_chr(sep, buff, bp);
    a = parse_number(split_token(&p1, sep));
    b = parse_number(split_token(&p2, sep));
    safe_number((a < b) ? a : b, buff, bp);
  }

  /* make sure vectors were the same length */
  if (p1 || p2) {
    *bp = start;
    safe_str(T("#-1 VECTORS MUST BE SAME DIMENSIONS"), buff, bp);
    return;
  }
}


/* ARGSUSED */
FUNCTION(fun_vadd)
{
  char *p1, *p2;
  char *start;
  char sep;

  /* return if a list is empty */
  if (!args[0] || !args[1])
    return;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;
  p1 = trim_space_sep(args[0], sep);
  p2 = trim_space_sep(args[1], sep);

  /* return if a list is empty */
  if (!*p1 || !*p2)
    return;

  /* add the vectors */
  start = *bp;
  safe_number(parse_number(split_token(&p1, sep)) +
	      parse_number(split_token(&p2, sep)), buff, bp);
  while (p1 && p2) {
    safe_chr(sep, buff, bp);
    safe_number(parse_number(split_token(&p1, sep)) +
		parse_number(split_token(&p2, sep)), buff, bp);
  }

  /* make sure vectors were the same length */
  if (p1 || p2) {
    *bp = start;
    safe_str(T("#-1 VECTORS MUST BE SAME DIMENSIONS"), buff, bp);
    return;
  }
}


/* ARGSUSED */
FUNCTION(fun_vsub)
{
  char *p1, *p2;
  char *start;
  char sep;

  /* return if a list is empty */
  if (!args[0] || !args[1])
    return;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;
  p1 = trim_space_sep(args[0], sep);
  p2 = trim_space_sep(args[1], sep);

  /* return if a list is empty */
  if (!*p1 || !*p2)
    return;

  /* subtract the vectors */
  start = *bp;
  safe_number(parse_number(split_token(&p1, sep)) -
	      parse_number(split_token(&p2, sep)), buff, bp);
  while (p1 && p2) {
    safe_chr(sep, buff, bp);
    safe_number(parse_number(split_token(&p1, sep)) -
		parse_number(split_token(&p2, sep)), buff, bp);
  }

  /* make sure vectors were the same length */
  if (p1 || p2) {
    *bp = start;
    safe_str(T("#-1 VECTORS MUST BE SAME DIMENSIONS"), buff, bp);
    return;
  }
}

/* ARGSUSED */
FUNCTION(fun_vmul)
{
  NVAL e1, e2;
  char *p1, *p2;
  char *start;
  char sep;

  /* return if a list is empty */
  if (!args[0] || !args[1])
    return;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;
  p1 = trim_space_sep(args[0], sep);
  p2 = trim_space_sep(args[1], sep);

  /* return if a list is empty */
  if (!*p1 || !*p2)
    return;

  /* multiply the vectors */
  start = *bp;
  e1 = parse_number(split_token(&p1, sep));
  e2 = parse_number(split_token(&p2, sep));
  if (!p1) {
    /* scalar * vector */
    safe_number(e1 * e2, buff, bp);
    while (p2) {
      safe_chr(sep, buff, bp);
      safe_number(e1 * parse_number(split_token(&p2, sep)), buff, bp);
    }
  } else if (!p2) {
    /* vector * scalar */
    safe_number(e1 * e2, buff, bp);
    while (p1) {
      safe_chr(sep, buff, bp);
      safe_number(parse_number(split_token(&p1, sep)) * e2, buff, bp);
    }
  } else {
    /* vector * vector elementwise product */
    safe_number(e1 * e2, buff, bp);
    while (p1 && p2) {
      safe_chr(sep, buff, bp);
      safe_number
	(parse_number(split_token(&p1, sep)) *
	 parse_number(split_token(&p2, sep)), buff, bp);
    }
    /* make sure vectors were the same length */
    if (p1 || p2) {
      *bp = start;
      safe_str(T("#-1 VECTORS MUST BE SAME DIMENSIONS"), buff, bp);
      return;
    }
  }
}


/* ARGSUSED */
FUNCTION(fun_vdot)
{
  NVAL product;
  char *p1, *p2;
  char sep;

  /* return if a list is empty */
  if (!args[0] || !args[1])
    return;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;
  p1 = trim_space_sep(args[0], sep);
  p2 = trim_space_sep(args[1], sep);

  /* return if a list is empty */
  if (!*p1 || !*p2)
    return;

  /* multiply the vectors */
  product = 0;
  while (p1 && p2) {
    product += parse_number(split_token(&p1, sep)) *
      parse_number(split_token(&p2, sep));
  }
  if (p1 || p2) {
    safe_str(T("#-1 VECTORS MUST BE SAME DIMENSIONS"), buff, bp);
    return;
  }
  safe_number(product, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_vmag)
{
  NVAL num, sum;
  char *p1;
  char sep;

  /* return if a list is empty */
  if (!args[0])
    return;

  if (!delim_check(buff, bp, nargs, args, 2, &sep))
    return;
  p1 = trim_space_sep(args[0], sep);

  /* return if a list is empty */
  if (!*p1)
    return;

  /* sum the squares */
  num = parse_number(split_token(&p1, sep));
  sum = num * num;
  while (p1) {
    num = parse_number(split_token(&p1, sep));
    sum += num * num;
  }

  safe_number(sqrt(sum), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_vunit)
{
  NVAL num, sum;
  char tbuf[BUFFER_LEN];
  char *p1;
  char sep;

  /* return if a list is empty */
  if (!args[0])
    return;

  if (!delim_check(buff, bp, nargs, args, 2, &sep))
    return;
  p1 = trim_space_sep(args[0], sep);

  /* return if a list is empty */
  if (!*p1)
    return;

  /* copy the vector, since we have to walk it twice... */
  strcpy(tbuf, p1);

  /* find the magnitude */
  num = parse_number(split_token(&p1, sep));
  sum = num * num;
  while (p1) {
    num = parse_number(split_token(&p1, sep));
    sum += num * num;
  }
  sum = sqrt(sum);

  if (!sum) {
    /* zero vector */
    p1 = tbuf;
    safe_chr('0', buff, bp);
    while (split_token(&p1, sep), p1) {
      safe_chr(sep, buff, bp);
      safe_chr('0', buff, bp);
    }
    return;
  }
  /* now make the unit vector */
  p1 = tbuf;
  safe_number(parse_number(split_token(&p1, sep)) / sum, buff, bp);
  while (p1) {
    safe_chr(sep, buff, bp);
    safe_number(parse_number(split_token(&p1, sep)) / sum, buff, bp);
  }
}


/* ARGSUSED */
FUNCTION(fun_fdiv)
{
  math_fdiv(args, nargs, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_fmod)
{
  NVAL x, y;
  if (!is_strict_number(args[0]) || !is_strict_number(args[1])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  y = parse_number(args[1]);
  if (y == 0) {
    safe_str(T("#-1 DIVISION BY ZERO"), buff, bp);
    return;
  }
  x = parse_number(args[0]);
  safe_number(fmod(x, y), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_floor)
{
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  safe_number(floor(parse_number(args[0])), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_ceil)
{
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  safe_number(ceil(parse_number(args[0])), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_round)
{
  char temp[BUFFER_LEN];
  int places;

  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  if (nargs == 2) {
    if (!is_integer(args[1])) {
      safe_str(T(e_int), buff, bp);
      return;
    }
    places = parse_integer(args[1]);
  } else
    places = 0;

  if (places < 0)
    places = 0;
  else if (places > FLOAT_PRECISION)
    places = FLOAT_PRECISION;

  /* The 0.0000001 is a kludge because .15 gets represented as .149999...
   * on many systems, and rounds down to .1. Lame. */
  sprintf(temp, "%.*f", places, parse_number(args[0]) + 0.0000001);

  /* Handle the bizarre "-0" sprintf problem. */
  if (!strcmp(temp, "-0"))
    safe_chr('0', buff, bp);
  else
    safe_str(temp, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_pi)
{
  safe_number(M_PI, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_e)
{
  safe_number(2.71828182845904523536, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_sin)
{
  NVAL angle;
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  angle = angle_to_rad(parse_number(args[0]), args[1]);
  safe_number(sin(angle), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_asin)
{
  NVAL num;
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  num = parse_number(args[0]);
  if ((num < -1) || (num > 1)) {
    safe_str(T(e_range), buff, bp);
    return;
  }
  safe_number(rad_to_angle(asin(num), args[1]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_cos)
{
  NVAL angle;
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  angle = angle_to_rad(parse_number(args[0]), args[1]);
  safe_number(cos(angle), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_acos)
{
  NVAL num;
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  num = parse_number(args[0]);
  if ((num < -1) || (num > 1)) {
    safe_str(T(e_range), buff, bp);
    return;
  }
  safe_number(rad_to_angle(acos(num), args[1]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_tan)
{
  NVAL angle;
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  angle = angle_to_rad(parse_number(args[0]), args[1]);
  safe_number(tan(angle), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_atan)
{
  NVAL angle;
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  angle = parse_number(args[0]);
  safe_number(rad_to_angle(atan(angle), args[1]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_atan2)
{
  NVAL x, y;
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  x = parse_number(args[0]);
  y = parse_number(args[1]);
  safe_number(rad_to_angle(atan2(x, y), args[2]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_exp)
{
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  safe_number(exp(parse_number(args[0])), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_power)
{
  NVAL num;
  NVAL m;

  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  num = parse_number(args[0]);
  m = parse_number(args[1]);
  if (num < 0 && (m != (int) m)) {
    safe_str(T("#-1 FRACTIONAL POWER OF NEGATIVE"), buff, bp);
    return;
  }
#ifndef HAS_IEEE_MATH
  if ((num > 100) || (m > 100)) {
    safe_str(T(e_range), buff, bp);
    return;
  }
#endif
  safe_number(pow(num, m), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_ln)
{
  NVAL num;
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  num = parse_number(args[0]);
#ifndef HAS_IEEE_MATH
  /* log(0) is bad for you */
  if (num == 0) {
    safe_str(T("#-1 INFINITY"), buff, bp);
    return;
  }
#endif
  if (num < 0) {
    safe_str(T(e_range), buff, bp);
    return;
  }
  safe_number(log(num), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_log)
{
  NVAL num, base;
  if (!is_number(args[0])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  num = parse_number(args[0]);
#ifndef HAS_IEEE_MATH
  /* log(0) is bad for you */
  if (num == 0) {
    safe_str(T("#-1 INFINITY"), buff, bp);
    return;
  }
#endif
  if (num < 0) {
    safe_str(T(e_range), buff, bp);
    return;
  }
  if (nargs == 2) {
    if (!is_number(args[1])) {
      safe_str(T(e_nums), buff, bp);
      return;
    }
    base = parse_number(args[1]);

    if (base <= 1) {
      safe_str(T("#-1 BASE OUT OF RANGE"), buff, bp);
      return;
    }
    safe_number(log(num) / log(base), buff, bp);
  } else
    safe_number(log10(num), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_sqrt)
{
  NVAL num;
  if (!is_number(args[0])) {
    safe_str(T(e_num), buff, bp);
    return;
  }
  num = parse_number(args[0]);
  if (num < 0) {
    safe_str(T("#-1 SQUARE ROOT OF NEGATIVE"), buff, bp);
    return;
  }
  safe_number(sqrt(num), buff, bp);
}

FUNCTION(fun_isint)
{
  safe_boolean(is_strict_integer(args[0]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_isnum)
{
  safe_boolean(is_strict_number(args[0]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_and)
{
  math_and(args, nargs, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_or)
{
  math_or(args, nargs, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_cand)
{
  int j;
  char tbuf[BUFFER_LEN], *tp;
  char const *sp;

  for (j = 0; j < nargs; j++) {
    tp = tbuf;
    sp = args[j];
    process_expression(tbuf, &tp, &sp, executor, caller, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
    *tp = '\0';
    if (!parse_boolean(tbuf)) {
      safe_chr('0', buff, bp);
      return;
    }
  }
  safe_chr('1', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_cor)
{
  int j;
  char tbuf[BUFFER_LEN], *tp;
  char const *sp;

  for (j = 0; j < nargs; j++) {
    tp = tbuf;
    sp = args[j];
    process_expression(tbuf, &tp, &sp, executor, caller, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
    *tp = '\0';
    if (parse_boolean(tbuf)) {
      safe_chr('1', buff, bp);
      return;
    }
  }
  safe_chr('0', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_not)
{
  safe_boolean(!parse_boolean(args[0]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_t)
{
  safe_boolean(parse_boolean(args[0]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_xor)
{
  math_xor(args, nargs, buff, bp);
}

static void
do_spellnum(num, len, buff, bp)
    char *num;			/* string containing an integer. */
    unsigned int len;		/* The string length must be a multiple of 3, 
				   with a maximum of 15. */
    char **buff;		/* output is the integer spelled out */
    char ***bp;
{
  static const char *bigones[] =
    { "", "thousand", "million", "billion", "trillion" };
  static const char *singles[] = { "", "one", "two", "three", "four",
    "five", "six", "seven", "eight", "nine"
  };
  static const char *special[] =
    { "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen",
    "sixteen",
    "seventeen", "eighteen", "nineteen"
  };
  static const char *tens[] = { "", " ", "twenty", "thirty", "forty",
    "fifty", "sixty", "seventy", "eighty", "ninety"
  };
  unsigned int x0, x1, x2;
  int pos = len / 3;
  int started = 0;

  while (pos > 0) {

    pos--;

    x0 = num[0] - '0';
    x1 = num[1] - '0';
    x2 = num[2] - '0';

    if (x0) {
      if (started)
	safe_chr(' ', *buff, *bp);
      safe_format(*buff, *bp, "%s %s", singles[x0], "hundred");
      started = 1;
    }

    if (x1 == 1) {
      if (started)
	safe_chr(' ', *buff, *bp);
      safe_str(special[x2], *buff, *bp);
      started = 1;
    } else if (x1 || x2) {
      if (started)
	safe_chr(' ', *buff, *bp);
      if (x1) {
	safe_str(tens[x1], *buff, *bp);
	if (x2)
	  safe_chr('-', *buff, *bp);
      }
      safe_str(singles[x2], *buff, *bp);
      started = 1;
    }

    if (pos && (x0 || x1 || x2)) {
      if (started)
	safe_chr(' ', *buff, *bp);
      safe_str(bigones[pos], *buff, *bp);
      started = 1;
    }

    num += 3;
  }
}				/* do_spellnum */

/** adds zeros to the beginning of the string, untill its length is 
 * a multiple of 3.
 */
#define add_zeros(p) \
   m = strlen(p) % 3; \
   switch (m) { \
    case 0: strcpy(num, p); break; \
    case 1: num[0] = '0'; num[1] = '0'; strcpy(num + 2, p); break; \
    case 2: num[0] = '0'; strcpy(num + 1, p); break; \
   } \
   p = num;

/* ARGSUSED */
FUNCTION(fun_spellnum)
{
  static const char *tail[] = { "", "tenth", "hundredth", "thousandth",
    "ten-thousandth", "hundred-thousandth", "millionth",
    "ten-millionth", "hundred-millionth", "billionth",
    "ten-billionth", "hundred-billionth", "trillionth",
    "ten-trillionth", "hundred-trillionth"
  };

  char num[BUFFER_LEN];
  char *number,			/* the whole number (without -/+ sign and leading zeros) */
  *pnumber = args[0], *pnum1, *pnum2 = NULL;	/* part 1 and 2 of the number respectively */
  unsigned int len, m, minus = 0,	/* is the number negative? */
   dot = 0, len1, len2;		/* length of part 1 and 2 respectively */

  pnumber = trim_space_sep(args[0], ' ');

  /* Is the number negative? */
  if (pnumber[0] == '-') {
    minus = 1;
    pnumber++;
  } else if (pnumber[0] == '+') {
    pnumber++;
  }

  /* remove leading zeros */
  while (*pnumber == '0')
    pnumber++;

  pnum1 = number = pnumber;

  /* Is it a number? 
   * If so, devide the number in two parts: pnum1.pnum2 
   */
  len = strlen(number);
  for (m = 0; m < len; m++) {

    if (*pnumber == '.') {
      if (dot) {
	safe_str(T(e_num), buff, bp);
	return;
      }
      dot = 1;			/* allow only 1 dot in a number */
      *pnumber = '\0';		/* devide the string */
      pnum2 = pnumber + 1;
    } else if (!isdigit((unsigned char) *pnumber)) {
      safe_str(T(e_num), buff, bp);
      return;
    }
    pnumber++;
  }

  add_zeros(pnum1);

  len1 = strlen(pnum1);
  len2 = (pnum2 == NULL ? 0 : strlen(pnum2));

  /* Max number is 999,999,999,999,999.999,999,999,999 */
  if (len1 > 15 || len2 > 14) {
    safe_str(T(e_range), buff, bp);
    return;
  }

  /* before the . */

  /* zero is special */
  if (*pnum1 == '\0') {
    if (len2 == 0)
      safe_str("zero", buff, bp);
  } else {
    if (minus)
      safe_str("negative ", buff, bp);
    do_spellnum(pnum1, len1, &buff, &bp);
  }

  if (len2 > 0) {
    /* after the . */

    /* remove leading zeros */
    while (*pnum2 == '0')
      pnum2++;

    add_zeros(pnum2);
    pnumber = num;

    len = strlen(pnumber);

    if (len1 > 0)
      safe_str(" and ", buff, bp);
    else if (minus && len)
      safe_str("negative ", buff, bp);

    /* zero is special */
    if (!len) {
      safe_str("zero ", buff, bp);
      safe_format(buff, bp, "%ss", tail[len2]);
    } else
      /* one is special too */
    if (len == 3 && parse_integer(pnumber) == 1) {
      safe_format(buff, bp, "one %s", tail[len2]);
    } else {
      /* rest is normal */
      do_spellnum(pnum1, len, &buff, &bp);
      safe_format(buff, bp, " %ss", tail[len2]);
    }

  }
}

#undef add_zeros


FUNCTION(fun_bound)
{
  if (!is_number(args[0]) ||
      !is_number(args[1]) || (nargs == 3 && !is_number(args[2]))) {
    safe_str(T(e_nums), buff, bp);
    return;
  }

  if (parse_number(args[0]) < parse_number(args[1]))
    safe_strl(args[1], arglens[1], buff, bp);
  else if (nargs == 3 && parse_number(args[0]) > parse_number(args[2]))
    safe_strl(args[2], arglens[2], buff, bp);
  else
    safe_strl(args[0], arglens[0], buff, bp);
}

FUNCTION(fun_band)
{
  math_band(args, nargs, buff, bp);
}

FUNCTION(fun_bnand)
{
  unsigned int retval;
  if (!is_uinteger(args[0]) || !is_uinteger(args[1])) {
    safe_str(T(e_uints), buff, bp);
    return;
  }
  retval = parse_uinteger(args[0]) & (~parse_uinteger(args[1]));
  safe_uinteger(retval, buff, bp);
}

FUNCTION(fun_bor)
{
  math_bor(args, nargs, buff, bp);
}

FUNCTION(fun_bxor)
{
  math_bxor(args, nargs, buff, bp);
}

FUNCTION(fun_bnot)
{
  if (!is_uinteger(args[0])) {
    safe_str(T(e_uint), buff, bp);
    return;
  }
  safe_uinteger(~parse_uinteger(args[0]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_nand)
{
  math_nand(args, nargs, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_nor)
{
  math_nor(args, nargs, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_lmath)
{
/* lmath(<op>, <list>[, <sep>])
* is equivalant to
*
* &op me=<op>(%0, %1)
* fold(me/op, <list>, <sep>)
* 
* but a lot more efficient. The Tiny l-OP functions
* can be simulated with @function if needed.
*/

  int nptr;
  char sep;
  char **ptr;
  MATH *op;

  /* Allocate memory */
  ptr = (char **) mush_malloc(BUFFER_LEN, "string");

  if (!delim_check(buff, bp, nargs, args, 3, &sep)) {
    mush_free((Malloc_t) ptr, "string");
    return;
  }

  nptr = list2arr(ptr, BUFFER_LEN, args[1], sep);

  op = math_hash_lookup(args[0]);

  if (!op) {
    safe_str(T("#-1 UNKNOWN OPERATION"), buff, bp);
    mush_free((Malloc_t) ptr, "string");
    return;
  }
  op->func(ptr, nptr, buff, bp);

  mush_free((Malloc_t) ptr, "string");
}

MATH_FUNC(math_add)
{
  NVAL result = 0;
  int n;

  for (n = 0; n < nptr; n++) {
    if (!is_number(ptr[n])) {
      safe_str(T(e_nums), buff, bp);
      return;
    }
    result += parse_number(ptr[n]);
  }

  safe_number(result, buff, bp);
}

MATH_FUNC(math_and)
{
  int n;

  for (n = 0; n < nptr; n++) {
    if (!parse_boolean(ptr[n])) {
      safe_chr('0', buff, bp);
      return;
    }
  }

  safe_chr('1', buff, bp);
  return;
}

MATH_FUNC(math_sub)
{
/* Subtraction */
  NVAL result;
  int n;

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  if (!is_number(ptr[0])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }

  result = parse_number(ptr[0]);

  for (n = 1; n < nptr; n++) {
    if (!is_number(ptr[n])) {
      safe_str(T(e_nums), buff, bp);
      return;
    }
    result -= parse_number(ptr[n]);
  }
  safe_number(result, buff, bp);
}

MATH_FUNC(math_mul)
{
  NVAL result;
  int n;
/* Multiplication */

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  if (!is_number(ptr[0])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  result = parse_number(ptr[0]);

  for (n = 1; n < nptr; n++) {
    if (!is_number(ptr[n])) {
      safe_str(T(e_nums), buff, bp);
      return;
    }
    result *= parse_number(ptr[n]);
  }
  safe_number(result, buff, bp);
}


MATH_FUNC(math_min)
{
  NVAL result;
  int n;

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  if (!is_number(ptr[0])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  result = parse_number(ptr[0]);

  for (n = 1; n < nptr; n++) {
    NVAL test;
    if (!is_number(ptr[n])) {
      safe_str(T(e_nums), buff, bp);
      return;
    }
    test = parse_number(ptr[n]);
    result = (result > test) ? test : result;
  }
  safe_number(result, buff, bp);
}

MATH_FUNC(math_max)
{
  NVAL result;
  int n;

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  if (!is_number(ptr[0])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  result = parse_number(ptr[0]);

  for (n = 1; n < nptr; n++) {
    NVAL test;
    if (!is_number(ptr[n])) {
      safe_str(T(e_nums), buff, bp);
      return;
    }
    test = parse_number(ptr[n]);
    result = (result > test) ? result : test;
  }
  safe_number(result, buff, bp);
}

MATH_FUNC(math_mean)
{
  NVAL result = 0, count = 0;
  int n;

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  for (n = 0; n < nptr; n++) {
    if (!is_number(ptr[n])) {
      safe_str(T(e_nums), buff, bp);
      return;
    }
    result += parse_number(ptr[n]);
    count++;
  }

  safe_number(result / count, buff, bp);
}

MATH_FUNC(math_div)
{
/* Division, truncating to match remainder */
  int divresult, n;

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  if (!is_integer(ptr[0])) {
    safe_str(T(e_ints), buff, bp);
    return;
  }
  divresult = parse_integer(ptr[0]);

  for (n = 1; n < nptr; n++) {
    int temp;
    if (!is_integer(ptr[n])) {
      safe_str(T(e_ints), buff, bp);
      return;
    }
    temp = parse_integer(ptr[n]);

    if (temp == 0) {
      safe_str(T("#-1 DIVISION BY ZERO"), buff, bp);
      return;
    }

    if (divresult < 0) {
      if (temp < 0)
	divresult = -divresult / -temp;
      else
	divresult = -(-divresult / temp);
    } else {
      if (temp < 0)
	divresult = -(divresult / -temp);
      else
	divresult = divresult / temp;
    }
  }
  safe_integer(divresult, buff, bp);
}

MATH_FUNC(math_floordiv)
{
/* Division taking the floor, to match modulo */
  int divresult, n;

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  if (!is_integer(ptr[0])) {
    safe_str(T(e_ints), buff, bp);
    return;
  }
  divresult = parse_integer(ptr[0]);

  for (n = 1; n < nptr; n++) {
    int temp;
    if (!is_integer(ptr[n])) {
      safe_str(T(e_ints), buff, bp);
      return;
    }
    temp = parse_integer(ptr[n]);

    if (temp == 0) {
      safe_str(T("#-1 DIVISION BY ZERO"), buff, bp);
      return;
    }

    if (divresult < 0) {
      if (temp < 0)
	divresult = -divresult / -temp;
      else
	divresult = -((-divresult + temp - 1) / temp);
    } else {
      if (temp < 0)
	divresult = -((divresult - temp - 1) / -temp);
      else
	divresult = divresult / temp;
    }
  }
  safe_integer(divresult, buff, bp);
}

MATH_FUNC(math_fdiv)
{
  NVAL result;
  int n;
/* Floating-point division */

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  if (!is_number(ptr[0])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  result = parse_number(ptr[0]);

  for (n = 1; n < nptr; n++) {
    NVAL temp;
    if (!is_number(ptr[n])) {
      safe_str(T(e_nums), buff, bp);
      return;
    }
    temp = parse_number(ptr[n]);

    if (temp == 0) {
      safe_str(T("#-1 DIVISION BY ZERO"), buff, bp);
      return;
    }

    result /= temp;
  }
  safe_number(result, buff, bp);
}

MATH_FUNC(math_modulo)
{
/* Modulo */
  int divresult, n;

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  if (!is_integer(ptr[0])) {
    safe_str(T(e_ints), buff, bp);
    return;
  }
  divresult = parse_integer(ptr[0]);

  for (n = 1; n < nptr; n++) {
    int temp;
    if (!is_integer(ptr[n])) {
      safe_str(T(e_ints), buff, bp);
      return;
    }
    temp = parse_integer(ptr[n]);

    if (temp == 0) {
      safe_str(T("#-1 DIVISION BY ZERO"), buff, bp);
      return;
    }

    if (divresult < 0) {
      if (temp < 0)
	divresult = -(-divresult % -temp);
      else
	divresult = (temp - (-divresult % temp)) % temp;
    } else {
      if (temp < 0)
	divresult = -((-temp - (divresult % -temp)) % -temp);
      else
	divresult = divresult % temp;
    }
  }
  safe_integer(divresult, buff, bp);
}

MATH_FUNC(math_remainder)
{
/* Remainder */
  int divresult, n;

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  if (!is_integer(ptr[0])) {
    safe_str(T(e_ints), buff, bp);
    return;
  }
  divresult = parse_integer(ptr[0]);

  for (n = 1; n < nptr; n++) {
    int temp;
    if (!is_integer(ptr[n])) {
      safe_str(T(e_ints), buff, bp);
      return;
    }
    temp = parse_integer(ptr[n]);

    if (temp == 0) {
      safe_str(T("#-1 DIVISION BY ZERO"), buff, bp);
      return;
    }

    if (divresult < 0) {
      if (temp < 0)
	divresult = -(-divresult % -temp);
      else
	divresult = -(-divresult % temp);
    } else {
      if (temp < 0)
	divresult = divresult % -temp;
      else
	divresult = divresult % temp;
    }
  }
  safe_integer(divresult, buff, bp);
}

MATH_FUNC(math_band)
{
  unsigned int bretval;
  int n;

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  if (!is_uinteger(ptr[0])) {
    safe_str(T(e_uints), buff, bp);
    return;
  }

  bretval = parse_uinteger(ptr[0]);

  for (n = 1; n < nptr; n++) {
    if (!is_uinteger(ptr[n])) {
      safe_str(T(e_uints), buff, bp);
      return;
    }
    bretval &= parse_uinteger(ptr[n]);
  }
  safe_uinteger(bretval, buff, bp);
}

MATH_FUNC(math_bor)
{
  unsigned int bretval;
  int n;

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  if (!is_uinteger(ptr[0])) {
    safe_str(T(e_uints), buff, bp);
    return;
  }

  bretval = parse_uinteger(ptr[0]);

  for (n = 1; n < nptr; n++) {
    if (!is_uinteger(ptr[n])) {
      safe_str(T(e_uints), buff, bp);
      return;
    }
    bretval |= parse_uinteger(ptr[n]);
  }
  safe_uinteger(bretval, buff, bp);
}

MATH_FUNC(math_bxor)
{
  unsigned int bretval;
  int n;

  if (nptr < 1) {
    safe_chr('0', buff, bp);
    return;
  }

  if (!is_uinteger(ptr[0])) {
    safe_str(T(e_uints), buff, bp);
    return;
  }

  bretval = parse_uinteger(ptr[0]);

  for (n = 1; n < nptr; n++) {
    if (!is_uinteger(ptr[n])) {
      safe_str(T(e_uints), buff, bp);
      return;
    }
    bretval ^= parse_uinteger(ptr[n]);
  }
  safe_uinteger(bretval, buff, bp);
}

MATH_FUNC(math_or)
{
  int n;
/* Or */
  for (n = 0; n < nptr; n++) {
    if (parse_boolean(ptr[n])) {
      safe_chr('1', buff, bp);
      return;
    }
  }
  safe_chr('0', buff, bp);
}

MATH_FUNC(math_nor)
{
  int n;
/* nor */

  for (n = 0; n < nptr; n++) {
    if (parse_boolean(ptr[n])) {
      safe_chr('0', buff, bp);
      return;
    }
  }
  safe_chr('1', buff, bp);
}

MATH_FUNC(math_nand)
{
  int n;

  for (n = 0; n < nptr; n++) {
    if (!parse_boolean(ptr[n])) {
      safe_chr('1', buff, bp);
      return;
    }
  }
  safe_chr('0', buff, bp);
}

MATH_FUNC(math_xor)
{
  int found = 0, n;

  for (n = 0; n < nptr; n++) {
    if (parse_boolean(ptr[n])) {
      if (found == 0) {
	found = 1;
      } else {
	safe_chr('0', buff, bp);
	return;
      }
    }
  }
  if (found)
    safe_chr('1', buff, bp);
  else
    safe_chr('0', buff, bp);
}


static int
nval_sort(const void *a, const void *b)
{
  const NVAL *x = a, *y = b;
  const double epsilon = pow(10, -FLOAT_PRECISION);
  int eq = (fabs(*x - *y) <= (epsilon * fabs(*x)));
  return eq ? 0 : (*x > *y ? 1 : -1);
}


static NVAL
find_median(NVAL *numbers, int nargs)
{
  if (nargs == 0)
    return 0;
  if (nargs == 1)
    return numbers[0];

  qsort(numbers, nargs, sizeof(NVAL), nval_sort);

  if ((nargs % 2) == 1)		/* Odd # of items */
    return numbers[nargs / 2];
  else
    return (numbers[(nargs / 2) - 1] + numbers[nargs / 2]) / (NVAL) 2;
}

FUNCTION(fun_median)
{
  math_median(args, nargs, buff, bp);
}

FUNCTION(fun_mean)
{
  math_mean(args, nargs, buff, bp);
}

FUNCTION(fun_stddev)
{
  math_stddev(args, nargs, buff, bp);
}

/* ARGSUSED */
MATH_FUNC(math_median)
{
  NVAL median;
  NVAL *numbers;
  int n;

  numbers = mush_malloc(nptr * sizeof(NVAL), "number_array");

  for (n = 0; n < nptr; n++) {
    if (!is_number(ptr[n])) {
      safe_str(T(e_nums), buff, bp);
      mush_free(numbers, "number_array");
      return;
    }
    numbers[n] = parse_number(ptr[n]);
  }

  median = find_median(numbers, nptr);
  mush_free(numbers, "number_array");
  safe_number(median, buff, bp);
}

MATH_FUNC(math_stddev)
{
  NVAL m, om, s, os, v;
  int n;

  if (nptr < 2) {
    safe_number(0, buff, bp);
    return;
  }
  if (!is_number(ptr[0])) {
    safe_str(T(e_nums), buff, bp);
    return;
  }
  m = parse_number(ptr[0]);
  s = 0;
  for (n = 1; n < nptr; n++) {
    om = m;
    os = s;
    if (!is_number(ptr[n])) {
      safe_str(T(e_nums), buff, bp);
      return;
    }
    v = parse_number(ptr[n]);
    m = om + (v - om) / (n + 1);
    s = os + (v - om) * (v - m);
  }

  safe_number(sqrt(s / (nptr - 1)), buff, bp);
}

MATH mlist[] = {
  {"ADD", math_add}
  ,
  {"SUB", math_sub}
  ,
  {"MUL", math_mul}
  ,
  {"DIV", math_div}
  ,
  {"FLOORDIV", math_floordiv}
  ,
  {"MOD", math_modulo}
  ,
  {"MODULO", math_modulo}
  ,
  {"MODULUS", math_modulo}
  ,
  {"REMAINDER", math_remainder}
  ,
  {"MIN", math_min}
  ,
  {"MAX", math_max}
  ,
  {"AND", math_and}
  ,
  {"NAND", math_nand}
  ,
  {"OR", math_or}
  ,
  {"NOR", math_nor}
  ,
  {"XOR", math_xor}
  ,
  {"BAND", math_band}
  ,
  {"BOR", math_bor}
  ,
  {"BXOR", math_bxor}
  ,
  {"FDIV", math_fdiv}
  ,
  {"MEAN", math_mean}
  ,
  {"MEDIAN", math_median}
  ,
  {"STDDEV", math_stddev}
  ,
  {NULL, NULL}
};

static MATH *
math_hash_lookup(char *name)
{
  return (MATH *) hashfind(strupper(name), &htab_math);
}

void init_math_hashtab(void);

static void
math_hash_insert(const char *name, MATH *func)
{
  hashadd(name, (void *) func, &htab_math);
}

void
init_math_hashtab(void)
{
  MATH *fp;

  hashinit(&htab_math, 32, sizeof(MATH));
  for (fp = mlist; fp->name; fp++)
    math_hash_insert(fp->name, (MATH *) fp);
}

static NVAL
angle_to_rad(NVAL angle, const char *from)
{
  if (!from)
    return angle;
  switch (*from) {
  case 'r':
  case 'R':
    return angle;
  case 'd':
  case 'D':
    return angle * (M_PI / 180.0);
  case 'g':
  case 'G':
    return angle * (M_PI / 200.0);
  default:
    return angle;
  }
}

static NVAL
rad_to_angle(NVAL angle, const char *to)
{
  if (!to)
    return angle;
  switch (*to) {
  case 'r':
  case 'R':
    return angle;
  case 'd':
  case 'D':
    return angle * (180.0 / M_PI);
  case 'g':
  case 'G':
    return angle * (200.0 / M_PI);
  default:
    return angle;
  }
}
