
#include "copyrite.h"

#include "config.h"
#include <math.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#include "conf.h"
#include "externs.h"
#include "intrface.h"
#include "parse.h"
#include "confmagic.h"

#ifdef WIN32
#pragma warning( disable : 4761)	/* NJG: disable warning re conversion */
#endif


/* ARGSUSED */
FUNCTION(fun_add)
{
  int j;
  NVAL sum;

  if (!is_number(args[0])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  sum = parse_number(args[0]);
  for (j = 1; j < nargs; j++) {
    if (!is_number(args[j])) {
      safe_str(e_nums, buff, bp);
      return;
    }
    sum += parse_number(args[j]);
  }
  safe_str(unparse_number(sum), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_sub)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  safe_str(unparse_number(parse_number(args[0]) - parse_number(args[1])),
	   buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_mul)
{
  int j;
  NVAL prod;

  if (!is_number(args[0])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  prod = parse_number(args[0]);
  for (j = 1; j < nargs; j++) {
    if (!is_number(args[j])) {
      safe_str(e_nums, buff, bp);
      return;
    }
    prod *= parse_number(args[j]);
  }
  safe_str(unparse_number(prod), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_gt)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  safe_chr((parse_number(args[0]) > parse_number(args[1])) ? '1' : '0',
	   buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_gte)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  safe_chr((parse_number(args[0]) >= parse_number(args[1])) ? '1' : '0',
	   buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_lt)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  safe_chr((parse_number(args[0]) < parse_number(args[1])) ? '1' : '0',
	   buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_lte)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  safe_chr((parse_number(args[0]) <= parse_number(args[1])) ? '1' : '0',
	   buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_eq)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  safe_chr((parse_number(args[0]) == parse_number(args[1])) ? '1' : '0',
	   buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_neq)
{
  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  safe_chr((parse_number(args[0]) != parse_number(args[1])) ? '1' : '0',
	   buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_max)
{
  int j;
  NVAL max, num;

  if (!is_number(args[0])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  max = parse_number(args[0]);
  for (j = 1; j < nargs; j++) {
    if (!is_number(args[j])) {
      safe_str(e_nums, buff, bp);
      return;
    }
    num = parse_number(args[j]);
    if (num > max)
      max = num;
  }
  safe_str(unparse_number(max), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_min)
{
  int j;
  NVAL min, num;

  if (!is_number(args[0])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  min = parse_number(args[0]);
  for (j = 1; j < nargs; j++) {
    if (!is_number(args[j])) {
      safe_str(e_nums, buff, bp);
      return;
    }
    num = parse_number(args[j]);
    if (num < min)
      min = num;
  }
  safe_str(unparse_number(min), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_sign)
{
  NVAL x;

  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
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
  if (!is_integer(args[0]) || !is_integer(args[1])) {
    safe_str(e_ints, buff, bp);
    return;
  }
  safe_str(unparse_integer(parse_integer(args[0]) << parse_integer(args[1])),
	   buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_shr)
{
  if (!is_integer(args[0]) || !is_integer(args[1])) {
    safe_str(e_ints, buff, bp);
    return;
  }
  safe_str(unparse_integer(parse_integer(args[0]) >> parse_integer(args[1])),
	   buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_inc)
{
  int num;
  char *p;
  /* Handle the case of a pure number */
  if (is_integer(args[0])) {
    safe_str(unparse_integer(parse_integer(args[0]) + 1), buff, bp);
    return;
  }
  p = args[0] + strlen(args[0]) - 1;
  if (!isdigit(*p)) {
    safe_str("#-1 ARGUMENT MUST END IN AN INTEGER", buff, bp);
    return;
  }
  while ((isdigit(*p) || (*p == '-')) && p != args[0]) {
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
  safe_str(unparse_integer(num), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_dec)
{
  int num;
  char *p;
  /* Handle the case of a pure number */
  if (is_integer(args[0])) {
    safe_str(unparse_integer(parse_integer(args[0]) - 1), buff, bp);
    return;
  }
  p = args[0] + strlen(args[0]) - 1;
  if (!isdigit(*p)) {
    safe_str("#-1 ARGUMENT MUST END IN AN INTEGER", buff, bp);
    return;
  }
  while ((isdigit(*p) || (*p == '-')) && p != args[0]) {
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
  safe_str(unparse_integer(num), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_trunc)
{
  /* This function does not have the non-number check because
   * the help file explicitly states that this function can
   * be used to turn "101dalmations" into "101".
   */
  safe_str(unparse_integer(parse_integer(args[0])), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_div)
{
  int bot;

  if (!is_integer(args[0]) || !is_integer(args[1])) {
    safe_str(e_ints, buff, bp);
    return;
  }
  bot = parse_integer(args[1]);
  if (bot == 0) {
    safe_str("#-1 DIVISION BY ZERO", buff, bp);
    return;
  }
  safe_str(unparse_integer(parse_integer(args[0]) / bot), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_mod)
{
  int bot;

  if (!is_integer(args[0]) || !is_integer(args[1])) {
    safe_str(e_ints, buff, bp);
    return;
  }
  bot = parse_integer(args[1]);
  if (bot == 0) {
    safe_str("#-1 DIVISION BY ZERO", buff, bp);
    return;
  }
  safe_str(unparse_integer(parse_integer(args[0]) % bot), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_abs)
{
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
#ifdef FLOATING_POINTS
  safe_str(unparse_number(fabs(parse_number(args[0]))), buff, bp);
#else
  safe_str(unparse_number(abs(parse_number(args[0]))), buff, bp);
#endif
}

/* ARGSUSED */
FUNCTION(fun_dist2d)
{
  int d1, d2;
  double r;

  if (!is_integer(args[0]) || !is_integer(args[1]) ||
      !is_integer(args[2]) || !is_integer(args[3])) {
    safe_str(e_ints, buff, bp);
    return;
  }
  d1 = parse_integer(args[0]) - parse_integer(args[2]);
  d2 = parse_integer(args[1]) - parse_integer(args[3]);
  r = (double) d1 *d1 + (double) d2 *d2;
#ifndef HAS_IEEE_MATH
  /* You can overflow, which is bad. */
  if (r < 0) {
    safe_str("#-1 OVERFLOW ERROR", buff, bp);
    return;
  }
#endif
#ifdef FLOATING_POINTS
  safe_str(unparse_number(sqrt(r)), buff, bp);
#else
  safe_str(unparse_integer((int) sqrt(r)), buff, bp);
#endif
}

/* ARGSUSED */
FUNCTION(fun_dist3d)
{
  int d1, d2, d3;
  double r;

  if (!is_integer(args[0]) || !is_integer(args[1]) ||
      !is_integer(args[2]) || !is_integer(args[3]) ||
      !is_integer(args[4]) || !is_integer(args[5])) {
    safe_str(e_ints, buff, bp);
    return;
  }
  d1 = parse_integer(args[0]) - parse_integer(args[3]);
  d2 = parse_integer(args[1]) - parse_integer(args[4]);
  d3 = parse_integer(args[2]) - parse_integer(args[5]);
  r = (double) d1 *d1 + (double) d2 *d2 + (double) d3 *d3;
#ifndef HAS_IEEE_MATH
  /* You can overflow, which is bad. */
  if (r < 0) {
    safe_str("#-1 OVERFLOW ERROR", buff, bp);
    return;
  }
#endif
#ifdef FLOATING_POINTS
  safe_str(unparse_number(sqrt(r)), buff, bp);
#else
  safe_str(unparse_integer((int) sqrt(r)), buff, bp);
#endif
}

/* ------------------------------------------------------------------------
 * Dune's vector functions: VADD, VSUB, VMUL, VCROSS, VMAG, VUNIT, VDIM
 *  VCRAMER?
 * Vectors are space-separated numbers.
 */

#define MAXDIM	20

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
  safe_str(unparse_number(parse_number(split_token(&p1, sep)) +
			  parse_number(split_token(&p2, sep))), buff, bp);
  while (p1 && p2) {
    safe_chr(sep, buff, bp);
    safe_str(unparse_number(parse_number(split_token(&p1, sep)) +
			 parse_number(split_token(&p2, sep))), buff, bp);
  }

  /* make sure vectors were the same length */
  if (p1 || p2) {
    *bp = start;
    safe_str("#-1 VECTORS MUST BE SAME DIMENSIONS", buff, bp);
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
  safe_str(unparse_number(parse_number(split_token(&p1, sep)) -
			  parse_number(split_token(&p2, sep))), buff, bp);
  while (p1 && p2) {
    safe_chr(sep, buff, bp);
    safe_str(unparse_number(parse_number(split_token(&p1, sep)) -
			 parse_number(split_token(&p2, sep))), buff, bp);
  }

  /* make sure vectors were the same length */
  if (p1 || p2) {
    *bp = start;
    safe_str("#-1 VECTORS MUST BE SAME DIMENSIONS", buff, bp);
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
    safe_str(unparse_number(e1 * e2), buff, bp);
    while (p2) {
      safe_chr(sep, buff, bp);
      safe_str(unparse_number(e1 * parse_number(split_token(&p2, sep))),
	       buff, bp);
    }
  } else if (!p2) {
    /* vector * scalar */
    safe_str(unparse_number(e1 * e2), buff, bp);
    while (p1) {
      safe_chr(sep, buff, bp);
      safe_str(unparse_number(parse_number(split_token(&p1, sep)) * e2),
	       buff, bp);
    }
  } else {
    /* vector * vector elementwise product */
    safe_str(unparse_number(e1 * e2), buff, bp);
    while (p1 && p2) {
      safe_chr(sep, buff, bp);
      safe_str(unparse_number(parse_number(split_token(&p1, sep)) * parse_number(split_token(&p2, sep))), buff, bp);
    }
    /* make sure vectors were the same length */
    if (p1 || p2) {
      *bp = start;
      safe_str("#-1 VECTORS MUST BE SAME DIMENSIONS", buff, bp);
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
    safe_str("#-1 VECTORS MUST BE SAME DIMENSIONS", buff, bp);
    return;
  }
  safe_str(unparse_number(product), buff, bp);
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

#ifdef FLOATING_POINTS
  safe_str(unparse_number(sqrt(sum)), buff, bp);
#else
  safe_str(unparse_number((int) (sqrt(sum) + 0.5)), buff, bp);
#endif				/* FLOATING_POINTS */
}

/* ARGSUSED */
FUNCTION(fun_vunit)
{
  /* This function is pretty useless without FLOATING_POINTS,
   * but I see no reason to disable it when FLOATING_POINTS
   * isn't defined...
   */
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
#ifdef FLOATING_POINTS
  sum = sqrt(sum);
#else
  sum = (NVAL) (sqrt(sum) + 0.5);
#endif				/* FLOATING_POINTS */

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
  safe_str(unparse_number(parse_number(split_token(&p1, sep)) / sum),
	   buff, bp);
  while (p1) {
    safe_chr(sep, buff, bp);
    safe_str(unparse_number(parse_number(split_token(&p1, sep)) / sum),
	     buff, bp);
  }
}


#ifdef FLOATING_POINTS

/* ARGSUSED */
FUNCTION(fun_fdiv)
{
  NVAL bot;

  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  bot = parse_number(args[1]);
  if (bot == 0) {
    safe_str("#-1 DIVISION BY ZERO", buff, bp);
    return;
  }
  safe_str(unparse_number(parse_number(args[0]) / bot), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_floor)
{
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  safe_str(unparse_number(floor(parse_number(args[0]))), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_ceil)
{
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  safe_str(unparse_number(ceil(parse_number(args[0]))), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_round)
{
  char temp[BUFFER_LEN];
  const char *fstr;
  int places;

  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  if (nargs == 2) {
    if (!is_integer(args[1])) {
      safe_str(e_int, buff, bp);
      return;
    }
    places = parse_integer(args[1]);
  } else
    places = 0;

  switch (places) {
  case 1:
    fstr = "%.1f";
    break;
  case 2:
    fstr = "%.2f";
    break;
  case 3:
    fstr = "%.3f";
    break;
  case 4:
    fstr = "%.4f";
    break;
  case 5:
    fstr = "%.5f";
    break;
  case 6:
    fstr = "%.6f";
    break;
  default:
    fstr = "%.0f";
    break;
  }
  /* The 0.0000001 is a kludge because .15 gets represented as .149999...
   * on many systems, and rounds down to .1. Lame. */
  sprintf(temp, fstr, parse_number(args[0]) + 0.0000001);

  /* Handle the bizarre "-0" sprintf problem. */
  if (!strcmp(temp, "-0"))
    safe_chr('0', buff, bp);
  else
    safe_str(temp, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_pi)
{
  safe_str("3.141593", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_e)
{
  safe_str("2.718282", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_sin)
{
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  safe_str(unparse_number(sin(parse_number(args[0]))), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_asin)
{
  NVAL num;
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  num = parse_number(args[0]);
  if ((num < -1) || (num > 1)) {
    safe_str("#-1 OUT OF RANGE", buff, bp);
    return;
  }
  safe_str(unparse_number(asin(num)), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_cos)
{
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  safe_str(unparse_number(cos(parse_number(args[0]))), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_acos)
{
  NVAL num;
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  num = parse_number(args[0]);
  if ((num < -1) || (num > 1)) {
    safe_str("#-1 OUT OF RANGE", buff, bp);
    return;
  }
  safe_str(unparse_number(acos(num)), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_tan)
{
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  safe_str(unparse_number(tan(parse_number(args[0]))), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_atan)
{
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  safe_str(unparse_number(atan(parse_number(args[0]))), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_exp)
{
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  safe_str(unparse_number(exp(parse_number(args[0]))), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_power)
{
  NVAL num;
  NVAL m;

  if (!is_number(args[0]) || !is_number(args[1])) {
    safe_str(e_nums, buff, bp);
    return;
  }
  num = parse_number(args[0]);
  m = parse_number(args[1]);
  if (num < 0 && (m != (int) m)) {
    safe_str("#-1 FRACTIONAL POWER OF NEGATIVE", buff, bp);
    return;
  }
#ifndef HAS_IEEE_MATH
  if ((num > 100) || (m > 100)) {
    safe_str("#-1 OUT OF RANGE", buff, bp);
    return;
  }
#endif
  safe_str(unparse_number(pow(num, m)), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_ln)
{
  NVAL num;
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  num = parse_number(args[0]);
#ifndef HAS_IEEE_MATH
  /* log(0) is bad for you */
  if (num == 0) {
    safe_str("#-1 INFINITY", buff, bp);
    return;
  }
#endif
  if (num < 0) {
    safe_str("#-1 OUT OF RANGE", buff, bp);
    return;
  }
  safe_str(unparse_number(log(num)), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_log)
{
  NVAL num;
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  num = parse_number(args[0]);
#ifndef HAS_IEEE_MATH
  /* log(0) is bad for you */
  if (num == 0) {
    safe_str("#-1 INFINITY", buff, bp);
    return;
  }
#endif
  if (num < 0) {
    safe_str("#-1 OUT OF RANGE", buff, bp);
    return;
  }
  safe_str(unparse_number(log10(num)), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_sqrt)
{
  NVAL num;
  if (!is_number(args[0])) {
    safe_str(e_num, buff, bp);
    return;
  }
  num = parse_number(args[0]);
  if (num < 0) {
    safe_str("#-1 SQUARE ROOT OF NEGATIVE", buff, bp);
    return;
  }
  safe_str(unparse_number(sqrt(num)), buff, bp);
}

#endif				/* FLOATING_POINTS */

/* ARGSUSED */
FUNCTION(fun_isnum)
{
  safe_chr(is_strict_number(args[0]) ? '1' : '0', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_and)
{
  int j;

  for (j = 0; j < nargs; j++)
    if (!parse_boolean(args[j])) {
      safe_chr('0', buff, bp);
      return;
    }
  safe_chr('1', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_or)
{
  int j;

  for (j = 0; j < nargs; j++)
    if (parse_boolean(args[j])) {
      safe_chr('1', buff, bp);
      return;
    }
  safe_chr('0', buff, bp);
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
  safe_chr(parse_boolean(args[0]) ? '0' : '1', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_t)
{
  safe_chr(parse_boolean(args[0]) ? '1' : '0', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_xor)
{
  int j, x;

  x = parse_boolean(args[0]);
  for (j = 1; j < nargs; j++)
    x ^= parse_boolean(args[j]);
  safe_chr(x ? '1' : '0', buff, bp);
}
