/**
 * \file funstr.c
 *
 * \brief String functions for mushcode.
 *
 *
 */
#include "copyrite.h"

#include "config.h"
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include "conf.h"
#include "ansi.h"
#include "externs.h"
#include "case.h"
#include "match.h"
#include "parse.h"
#include "pueblo.h"
#include "attrib.h"
#include "flags.h"
#include "dbdefs.h"
#include "mushdb.h"
#include "htab.h"
#include "lock.h"
#include "confmagic.h"


#ifdef WIN32
#define LC_MESSAGES 6
#pragma warning( disable : 4761)	/* NJG: disable warning re conversion */
#endif

#ifdef __APPLE__
#define LC_MESSAGES 6
#endif

HASHTAB htab_tag;

static int wraplen(char *str, int maxlen);
void init_tag_hashtab(void);
void init_pronouns(void);

/** Return an indicator of a player's gender.
 * \param player player whose gender is to be checked.
 * \retval 0 neuter.
 * \retval 1 female.
 * \retval 2 male.
 * \retval 3 plural.
 */
int
get_gender(dbref player)
{
  ATTR *a;

  a = atr_get(player, "SEX");

  if (!a)
    return 0;

  switch (*uncompress(a->value)) {
  case 'T':
  case 't':
  case 'P':
  case 'p':
    return 3;
  case 'M':
  case 'm':
    return 2;
  case 'F':
  case 'f':
  case 'W':
  case 'w':
    return 1;
  default:
    return 0;
  }
}

char *subj[4];
char *poss[4];
char *obj[4];
char *absp[4];

/** Initialize the pronoun translation strings.
 * This function sets up the values of the arrays of subjective,
 * possessive, objective, and absolute possessive pronouns with
 * locale-appropriate values.
 */
void
init_pronouns(void)
{
  int translate = 0;
#ifdef HAS_SETLOCALE
  char *loc;
  if ((loc = setlocale(LC_MESSAGES, NULL))) {
    if (strcmp(loc, "C") && strncmp(loc, "en", 2))
      translate = 1;
  }
#endif
#define SET_PRONOUN(p,v,u)  p = strdup((translate) ? (v) : (u))
  SET_PRONOUN(subj[0], T("pronoun:neuter,subjective"), "it");
  SET_PRONOUN(subj[1], T("pronoun:feminine,subjective"), "she");
  SET_PRONOUN(subj[2], T("pronoun:masculine,subjective"), "he");
  SET_PRONOUN(subj[3], T("pronoun:plural,subjective"), "they");
  SET_PRONOUN(poss[0], T("pronoun:neuter,possessive"), "its");
  SET_PRONOUN(poss[1], T("pronoun:feminine,possessive"), "her");
  SET_PRONOUN(poss[2], T("pronoun:masculine,possessive"), "his");
  SET_PRONOUN(poss[3], T("pronoun:plural,possessive"), "their");
  SET_PRONOUN(obj[0], T("pronoun:neuter,objective"), "it");
  SET_PRONOUN(obj[1], T("pronoun:feminine,objective"), "her");
  SET_PRONOUN(obj[2], T("pronoun:masculine,objective"), "him");
  SET_PRONOUN(obj[3], T("pronoun:plural,objective"), "them");
  SET_PRONOUN(absp[0], T("pronoun:neuter,absolute possessive"), "its");
  SET_PRONOUN(absp[1], T("pronoun:feminine,absolute possessive"), "hers");
  SET_PRONOUN(absp[2], T("pronoun:masculine,absolute possessive"), "his");
  SET_PRONOUN(absp[3], T("pronoun:plural,absolute possessive "), "theirs");
#undef SET_PRONOUN
}


/* ARGSUSED */
FUNCTION(fun_isword)
{
  /* is every character a letter? */
  char *p;
  if (!args[0] || !*args[0]) {
    safe_chr('0', buff, bp);
    return;
  }
  for (p = args[0]; *p; p++) {
    if (!isalpha((unsigned char) *p)) {
      safe_chr('0', buff, bp);
      return;
    }
  }
  safe_chr('1', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_capstr)
{
  char *p;
  p = skip_leading_ansi(args[0]);
  if (!p) {
    safe_strl(args[0], arglens[0], buff, bp);
    return;
  } else if (p != args[0]) {
    char x = *p;
    *p = '\0';
    safe_strl(args[0], p - args[0], buff, bp);
    *p = x;
  }
  if (*p) {
    safe_chr(UPCASE(*p), buff, bp);
    p++;
  }
  if (*p)
    safe_str(p, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_art)
{
  /* checks a word and returns the appropriate article, "a" or "an" */
  char c;
  char *p = skip_leading_ansi(args[0]);

  if (!p) {
    safe_chr('a', buff, bp);
    return;
  }
  c = tolower(*p);
  if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
    safe_str("an", buff, bp);
  else
    safe_chr('a', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_subj)
{
  dbref thing;

  thing = match_thing(executor, args[0]);
  if (thing == NOTHING) {
    safe_str(T(e_match), buff, bp);
    return;
  }
  safe_str(subj[get_gender(thing)], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_poss)
{
  dbref thing;

  thing = match_thing(executor, args[0]);
  if (thing == NOTHING) {
    safe_str(T(e_match), buff, bp);
    return;
  }
  safe_str(poss[get_gender(thing)], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_obj)
{
  dbref thing;

  thing = match_thing(executor, args[0]);
  if (thing == NOTHING) {
    safe_str(T(e_match), buff, bp);
    return;
  }
  safe_str(obj[get_gender(thing)], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_aposs)
{
  dbref thing;

  thing = match_thing(executor, args[0]);
  if (thing == NOTHING) {
    safe_str(T(e_match), buff, bp);
    return;
  }
  safe_str(absp[get_gender(thing)], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_alphamax)
{
  char amax[BUFFER_LEN];
  char *c;
  int j, m = 0;
  size_t len;

  c = remove_markup(args[0], &len);
  memcpy(amax, c, len);
  for (j = 1; j < nargs; j++) {
    c = remove_markup(args[j], &len);
    if (strcoll(amax, c) < 0) {
      memcpy(amax, c, len);
      m = j;
    }
  }
  safe_strl(args[m], arglens[m], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_alphamin)
{
  char amin[BUFFER_LEN];
  char *c;
  int j, m = 0;
  size_t len;

  c = remove_markup(args[0], &len);
  memcpy(amin, c, len);
  for (j = 1; j < nargs; j++) {
    c = remove_markup(args[j], &len);
    if (strcoll(amin, c) > 0) {
      memcpy(amin, c, len);
      m = j;
    }
  }
  safe_strl(args[m], arglens[m], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_strlen)
{
  safe_integer(ansi_strlen(args[0]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_mid)
{
  ansi_string *as;
  int pos, len;

  if (!is_integer(args[1]) || !is_integer(args[2])) {
    safe_str(T(e_ints), buff, bp);
    return;
  }

  as = parse_ansi_string(args[0]);
  pos = parse_integer(args[1]);
  len = parse_integer(args[2]);

  if ((pos < 0) || (len < 0)) {
    safe_str(T(e_range), buff, bp);
    free_ansi_string(as);
    return;
  }

  safe_ansi_string(as, pos, len, buff, bp);
  free_ansi_string(as);
}

/* ARGSUSED */
FUNCTION(fun_left)
{
  int len;
  ansi_string *as;

  if (!is_integer(args[1])) {
    safe_str(T(e_int), buff, bp);
    return;
  }
  len = parse_integer(args[1]);

  if (len < 0) {
    safe_str(T(e_range), buff, bp);
    return;
  }

  as = parse_ansi_string(args[0]);
  safe_ansi_string(as, 0, len, buff, bp);
  free_ansi_string(as);
}

/* ARGSUSED */
FUNCTION(fun_right)
{
  int len;
  ansi_string *as;

  if (!is_integer(args[1])) {
    safe_str(T(e_int), buff, bp);
    return;
  }
  len = parse_integer(args[1]);

  if (len < 0) {
    safe_str(T(e_range), buff, bp);
    return;
  }

  as = parse_ansi_string(args[0]);
  if ((size_t) len > as->len)
    safe_strl(args[0], arglens[0], buff, bp);
  else
    safe_ansi_string(as, as->len - len, as->len, buff, bp);
  free_ansi_string(as);
}

/* ARGSUSED */
FUNCTION(fun_strinsert)
{
  /* Insert a string into another */
  ansi_string *as;
  int pos;

  if (!is_integer(args[1])) {
    safe_str(e_int, buff, bp);
    return;
  }

  pos = parse_integer(args[1]);
  if (pos < 0) {
    safe_str(T(e_range), buff, bp);
    return;
  }

  as = parse_ansi_string(args[0]);

  if ((size_t) pos > as->len) {
    /* Fast special case - concatenate args[2] to args[0] */
    safe_strl(args[0], arglens[0], buff, bp);
    safe_strl(args[2], arglens[0], buff, bp);
    free_ansi_string(as);
    return;
  }

  safe_ansi_string(as, 0, pos, buff, bp);
  safe_strl(args[2], arglens[2], buff, bp);
  safe_ansi_string(as, pos, as->len, buff, bp);
  free_ansi_string(as);

}

/* ARGSUSED */
FUNCTION(fun_delete)
{
  ansi_string *as;
  int pos, num;


  if (!is_integer(args[1]) || !is_integer(args[2])) {
    safe_str(T(e_ints), buff, bp);
    return;
  }

  pos = parse_integer(args[1]);
  num = parse_integer(args[2]);

  if (pos < 0) {
    safe_str(T(e_range), buff, bp);
    return;
  }

  as = parse_ansi_string(args[0]);

  if ((size_t) pos > as->len || num <= 0) {
    safe_strl(args[0], arglens[0], buff, bp);
    free_ansi_string(as);
    return;
  }

  safe_ansi_string(as, 0, pos, buff, bp);
  safe_ansi_string(as, pos + num, as->len, buff, bp);
  free_ansi_string(as);
}

/* ARGSUSED */
FUNCTION(fun_comp)
{
  char type = 'A';

  if (nargs == 3 && !(args[2] && *args[2])) {
    safe_str(T("#-1 INVALID THIRD ARGUMENT"), buff, bp);
    return;
  } else if (nargs == 3) {
    type = toupper(*args[2]);
  }

  switch (type) {
  case 'A':			/* Case-sensitive lexicographic */
    {
      char left[BUFFER_LEN], right[BUFFER_LEN], *l, *r;
      size_t llen, rlen;
      l = remove_markup(args[0], &llen);
      memcpy(left, l, llen);
      r = remove_markup(args[1], &rlen);
      memcpy(right, r, rlen);
      safe_integer(gencomp(left, right, ALPHANUM_LIST), buff, bp);
      return;
    }
  case 'I':			/* Case-insensitive lexicographic */
    {
      char left[BUFFER_LEN], right[BUFFER_LEN], *l, *r;
      size_t llen, rlen;
      l = remove_markup(args[0], &llen);
      memcpy(left, l, llen);
      r = remove_markup(args[1], &rlen);
      memcpy(right, r, rlen);
      safe_integer(gencomp(left, right, INSENS_ALPHANUM_LIST), buff, bp);
      return;
    }
  case 'N':			/* Integers */
    if (!is_strict_integer(args[0]) || !is_strict_integer(args[1])) {
      safe_str(T(e_ints), buff, bp);
      return;
    }
    safe_integer(gencomp(args[0], args[1], NUMERIC_LIST), buff, bp);
    return;
  case 'F':
    if (!is_strict_number(args[0]) || !is_strict_number(args[1])) {
      safe_str(T(e_nums), buff, bp);
      return;
    }
    safe_integer(gencomp(args[0], args[1], FLOAT_LIST), buff, bp);
    return;
  case 'D':
    {
      dbref a, b;
      a = parse_dbref(args[0]);
      b = parse_dbref(args[1]);
      if (a == NOTHING || b == NOTHING) {
	safe_str(T("#-1 INVALID DBREF"), buff, bp);
	return;
      }
      safe_integer(gencomp(args[0], args[1], DBREF_LIST), buff, bp);
      return;
    }
  default:
    safe_str(T("#-1 INVALID THIRD ARGUMENT"), buff, bp);
    return;
  }
}

/* ARGSUSED */
FUNCTION(fun_pos)
{
  char tbuf[BUFFER_LEN];
  char *pos;
  size_t len;

  pos = remove_markup(args[1], &len);
  memcpy(tbuf, pos, len);
  pos = strstr(tbuf, remove_markup(args[0], NULL));
  if (pos)
    safe_integer(pos - tbuf + 1, buff, bp);
  else
    safe_str("#-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_lpos)
{
  char *pos;
  char c = ' ';
  size_t n, len;
  int first = 1;

  if (args[1][0])
    c = args[1][0];

  pos = remove_markup(args[0], &len);
  for (n = 0; n < len; n++)
    if (pos[n] == c) {
      if (first)
	first = 0;
      else
	safe_chr(' ', buff, bp);
      safe_integer(n, buff, bp);
    }
}


/* ARGSUSED */
FUNCTION(fun_strmatch)
{
  char tbuf[BUFFER_LEN];
  char *t;
  size_t len;
  /* matches a wildcard pattern for an _entire_ string */

  t = remove_markup(args[0], &len);
  memcpy(tbuf, t, len);
  safe_boolean(quick_wild(remove_markup(args[1], NULL), tbuf), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_strcat)
{
  int j;

  for (j = 0; j < nargs; j++)
    safe_strl(args[j], arglens[j], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_flip)
{
  ansi_string *as;
  int p, n;

  as = parse_ansi_string(args[0]);
  populate_codes(as);

  for (p = 0, n = as->len - 1; p < n; p++, n--) {
    char *tcode;
    char t;

    tcode = as->codes[p];
    t = as->text[p];
    as->codes[p] = as->codes[n];
    as->text[p] = as->text[n];
    as->codes[n] = tcode;
    as->text[n] = t;
  }

  safe_ansi_string(as, 0, as->len, buff, bp);

  free_ansi_string(as);
}

/* ARGSUSED */
FUNCTION(fun_merge)
{
  /* given s1, s2, and a char, for each character in s1, if the char
   * is the same as the given char, replace it with the corresponding
   * char in s2.
   */

  char *str, *rep;
  char c;

  /* do length checks first */
  if (arglens[0] != arglens[1]) {
    safe_str(T("#-1 STRING LENGTHS MUST BE EQUAL"), buff, bp);
    return;
  }
  if (arglens[2] > 1) {
    safe_str(T("#-1 TOO MANY CHARACTERS"), buff, bp);
    return;
  }
  /* find the character to look for */
  if (!*args[2])
    c = ' ';
  else
    c = *args[2];

  /* walk strings, copy from the appropriate string */
  for (str = args[0], rep = args[1]; *str && *rep; str++, rep++) {
    safe_chr((*str == c) ? *rep : *str, buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_lcstr)
{
  char *p, *y;
  p = args[0];
  while (*p) {
    y = skip_leading_ansi(p);
    if (y != p) {
      char t;
      t = *y;
      *y = '\0';
      safe_str(p, buff, bp);
      *y = t;
      p = y;
    }
    if (*p) {
      safe_chr(DOWNCASE(*p), buff, bp);
      p++;
    }
  }
}

/* ARGSUSED */
FUNCTION(fun_ucstr)
{
  char *p, *y;
  p = args[0];
  while (*p) {
    y = skip_leading_ansi(p);
    if (y != p) {
      char t;
      t = *y;
      *y = '\0';
      safe_str(p, buff, bp);
      *y = t;
      p = y;
    }
    if (*p) {
      safe_chr(UPCASE(*p), buff, bp);
      p++;
    }
  }
}

/* ARGSUSED */
FUNCTION(fun_repeat)
{
  int times;
  char *ap;

  if (!is_integer(args[1])) {
    safe_str(T(e_int), buff, bp);
    return;
  }
  times = parse_integer(args[1]);
  if (times < 0) {
    safe_str(T("#-1 ARGUMENT MUST BE NON-NEGATIVE INTEGER"), buff, bp);
    return;
  }
  if (!*args[0])
    return;

  /* Special-case repeating one character */
  if (arglens[0] == 1) {
    safe_fill(args[0][0], times, buff, bp);
    return;
  }

  /* Do the repeat in O(lg n) time. */
  /* This takes advantage of the fact that we're given a BUFFER_LEN
   * buffer for args[0] that we are free to trash.  Huzzah! */
  ap = args[0] + arglens[0];
  while (times) {
    if (times & 1) {
      if (safe_strl(args[0], arglens[0], buff, bp) != 0)
	break;
    }
    safe_str(args[0], args[0], &ap);
    *ap = '\0';
    arglens[0] = strlen(args[0]);
    times = times >> 1;
  }
}

/* ARGSUSED */
FUNCTION(fun_scramble)
{
  int n, i, j;
  ansi_string *as;

  if (!*args[0])
    return;

  as = parse_ansi_string(args[0]);
  populate_codes(as);
  n = as->len;
  for (i = 0; i < n; i++) {
    char t, *tcode;
    j = get_random_long(i, n - 1);
    t = as->text[j];
    as->text[j] = as->text[i];
    as->text[i] = t;
    tcode = as->codes[j];
    as->codes[j] = as->codes[i];
    as->codes[i] = tcode;
  }
  safe_ansi_string(as, 0, as->len, buff, bp);
  free_ansi_string(as);
}

/* ARGSUSED */
FUNCTION(fun_ljust)
{
  /* pads a string with trailing blanks (or other fill character) */

  size_t spaces, len;
  char sep;

  if (!is_uinteger(args[1])) {
    safe_str(T(e_uint), buff, bp);
    return;
  }
  len = ansi_strlen(args[0]);
  spaces = parse_uinteger(args[1]);
  if (spaces >= BUFFER_LEN)
    spaces = BUFFER_LEN - 1;

  if (len >= spaces) {
    safe_strl(args[0], arglens[0], buff, bp);
    return;
  }
  spaces -= len;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  safe_strl(args[0], arglens[0], buff, bp);
  safe_fill(sep, spaces, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_rjust)
{
  /* pads a string with leading blanks (or other fill character) */

  size_t spaces, len;
  char sep;

  if (!is_uinteger(args[1])) {
    safe_str(T(e_uint), buff, bp);
    return;
  }
  len = ansi_strlen(args[0]);
  spaces = parse_uinteger(args[1]);
  if (spaces >= BUFFER_LEN)
    spaces = BUFFER_LEN - 1;

  if (len >= spaces) {
    safe_strl(args[0], arglens[0], buff, bp);
    return;
  }
  spaces -= len;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  safe_fill(sep, spaces, buff, bp);
  safe_strl(args[0], arglens[0], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_center)
{
  /* pads a string with leading blanks (or other fill character) */

  size_t width, len, lsp, rsp;
  char sep;

  if (!is_uinteger(args[1])) {
    safe_str(T(e_uint), buff, bp);
    return;
  }
  width = parse_uinteger(args[1]);
  len = ansi_strlen(args[0]);
  if (len >= width) {
    safe_strl(args[0], arglens[0], buff, bp);
    return;
  }
  rsp = width - len;
  lsp = rsp / 2;
  rsp -= lsp;
  if (lsp >= BUFFER_LEN)
    lsp = BUFFER_LEN - 1;
  if (rsp >= BUFFER_LEN)
    rsp = BUFFER_LEN - 1;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  safe_fill(sep, lsp, buff, bp);
  safe_strl(args[0], arglens[0], buff, bp);
  safe_fill(sep, rsp, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_foreach)
{
  /* Like map(), but it operates on a string, rather than on a list,
   * calling a user-defined function for each character in the string.
   * No delimiter is inserted between the results.
   */

  dbref thing;
  ATTR *attrib;
  char const *ap, *lp;
  char *asave, cbuf[2];
  char *tptr[2];
  char place[SBUF_LEN];
  int placenr = 0;
  int funccount;
  char *oldbp;
  char start, end;
  char letters[BUFFER_LEN];
  size_t len;

  if (nargs >= 3) {
    if (!delim_check(buff, bp, nargs, args, 3, &start))
      return;
  }

  if (nargs == 4) {
    if (!delim_check(buff, bp, nargs, args, 4, &end))
      return;
  } else {
    end = '\0';
  }

  /* find our object and attribute */
  parse_attrib(executor, args[0], &thing, &attrib);
  if (!GoodObject(thing) || !attrib || !Can_Read_Attr(executor, thing, attrib))
    return;

  strcpy(place, "0");
  asave = safe_uncompress(attrib->value);

  /* save our stack */
  tptr[0] = wenv[0];
  tptr[1] = wenv[1];
  wenv[1] = place;

  ap = remove_markup(args[1], &len);
  memcpy(letters, ap, len);

  lp = trim_space_sep(letters, ' ');
  if (nargs >= 3) {
    char *tmp = strchr(lp, start);

    if (!tmp) {
      safe_str(lp, buff, bp);
      free((Malloc_t) asave);
      wenv[1] = tptr[1];
      return;
    }
    oldbp = place;
    placenr = (tmp + 1) - lp;
    safe_integer_sbuf(placenr, place, &oldbp);
    oldbp = *bp;

    *tmp = '\0';
    safe_str(lp, buff, bp);
    lp = tmp + 1;
  }

  cbuf[1] = '\0';
  wenv[0] = cbuf;
  oldbp = *bp;
  funccount = pe_info->fun_invocations;
  while (*lp && *lp != end) {
    *cbuf = *lp++;
    ap = asave;
    if (process_expression(buff, bp, &ap, thing, executor, enactor,
			   PE_DEFAULT, PT_DEFAULT, pe_info))
      break;
    if (*bp == oldbp && pe_info->fun_invocations == funccount)
      break;
    oldbp = place;
    safe_integer_sbuf(++placenr, place, &oldbp);
    *oldbp = '\0';
    oldbp = *bp;
    funccount = pe_info->fun_invocations;
  }
  if (*lp)
    safe_str(lp + 1, buff, bp);
  free((Malloc_t) asave);
  wenv[0] = tptr[0];
  wenv[1] = tptr[1];
}

/* ARGSUSED */
FUNCTION(fun_secure)
{
  /* this function smashes all occurences of "unsafe" characters in a string.
   * "unsafe" characters are ( ) [ ] { } $ ^ % , ; \
   * these characters get replaced by spaces
   */
  char *p;

  p = args[0];
  while (*p) {
    switch (*p) {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '$':
    case '^':
    case '%':
    case ',':
    case ';':
    case '\\':
      *p = ' ';
      break;
    }
    p++;
  }
  safe_strl(args[0], arglens[0], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_escape)
{
  /* another function more or less right out of 2.0 code */

  char *s;

  s = args[0];
  while (*s) {
    switch (*s) {
    default:
      if (s != args[0])
	break;
    case '%':
    case '\\':
    case '[':
    case ']':
    case '{':
    case '}':
    case ';':
      safe_chr('\\', buff, bp);
    }
    safe_chr(*s, buff, bp);
    s++;
  }
}

/* ARGSUSED */
FUNCTION(fun_trim)
{
  /* Similar to squish() but it doesn't trim spaces in the center, and
   * takes a delimiter argument and trim style.
   */

  char *p, *q, sep;
  int trim;
  int trim_style_arg, trim_char_arg;

  /* Alas, PennMUSH and TinyMUSH used different orders for the arguments.
   * We'll give the users an option about it
   */
  if (TINY_TRIM_FUN) {
    trim_style_arg = 2;
    trim_char_arg = 3;
  } else {
    trim_style_arg = 3;
    trim_char_arg = 2;
  }

  if (!delim_check(buff, bp, nargs, args, trim_char_arg, &sep))
    return;

  /* If a trim style is provided, it must be the third argument. */
  if (nargs >= trim_style_arg) {
    switch (DOWNCASE(*args[trim_style_arg - 1])) {
    case 'l':
      trim = 1;
      break;
    case 'r':
      trim = 2;
      break;
    default:
      trim = 3;
      break;
    }
  } else
    trim = 3;

  /* We will never need to check for buffer length overrunning, since
   * we will always get a smaller string. Thus, we can copy at the
   * same time we skip stuff.
   */

  /* If necessary, skip over the leading stuff. */
  p = args[0];
  if (trim != 2) {
    while (*p == sep)
      p++;
  }
  /* Cut off the trailing stuff, if appropriate. */
  if ((trim != 1) && (*p != '\0')) {
    q = args[0] + arglens[0] - 1;
    while ((q > p) && (*q == sep))
      q--;
    q[1] = '\0';
  }
  safe_str(p, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_lit)
{
  /* Just returns the argument, literally */
  safe_strl(args[0], arglens[0], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_squish)
{
  /* zaps leading and trailing spaces, and reduces other spaces to a single
   * space. This only applies to the literal space character, and not to
   * tabs, newlines, etc.
   * We do not need to check for buffer length overflows, since we're
   * never going to end up with a longer string.
   */

  char *tp;
  char sep;

  /* Figure out the character to squish */
  if (!delim_check(buff, bp, nargs, args, 2, &sep))
    return;

  /* get rid of trailing spaces first, so we don't have to worry about
   * them later.
   */
  tp = args[0] + arglens[0] - 1;
  while ((tp > args[0]) && (*tp == sep))
    tp--;
  tp[1] = '\0';

  for (tp = args[0]; *tp == sep; tp++)	/* skip leading spaces */
    ;

  while (*tp) {
    safe_chr(*tp, buff, bp);
    if (*tp == sep)
      while (*tp == sep)
	tp++;
    else
      tp++;
  }
}

/* ARGSUSED */
FUNCTION(fun_space)
{
  int s;

  if (!is_uinteger(args[0])) {
    safe_str(T(e_uint), buff, bp);
    return;
  }
  s = parse_integer(args[0]);
  safe_fill(' ', s, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_beep)
{
  int k;

  /* this function prints 1 to 5 beeps. The alert character '\a' is
   * an ANSI C invention; non-ANSI-compliant implementations may ignore
   * the '\' character and just print an 'a', or do something else nasty,
   * so we define it to be something reasonable in ansi.h.
   */

  if (nargs) {
    if (!is_integer(args[0])) {
      safe_str(T(e_int), buff, bp);
      return;
    }
    k = parse_integer(args[0]);
  } else
    k = 1;

  if (!Hasprivs(executor) || (k <= 0) || (k > 5)) {
    safe_str(T(e_perm), buff, bp);
    return;
  }
  safe_fill(BEEP_CHAR, k, buff, bp);
}

/** Initialize the html tag hash table with all the safe tags from HTML 4.0 */
void
init_tag_hashtab(void)
{
  static char dummy = 1;
  int i = 0;
  static const char *safetags[] = { "A", "B", "I", "U", "STRONG", "EM",
    "ADDRESS", "BLOCKQUOTE", "CENTER", "DEL", "DIV",
    "H1", "H2", "H3", "H4", "H5", "H6", "HR", "INS",
    "P", "PRE", "DIR", "DL", "DT", "DD", "LI", "MENU", "OL", "UL",
    "TABLE", "CAPTION", "COLGROUP", "COL", "THEAD", "TFOOT",
    "TBODY", "TR", "TD", "TH",
    "BR", "FONT", "IMG", "SPAN", "SUB", "SUP",
    "ABBR", "ACRONYM", "CITE", "CODE", "DFN", "KBD", "SAMP", "VAR",
    "BIG", "S", "SMALL", "STRIKE", "TT",
    NULL
  };
  hashinit(&htab_tag, 64, 1);
  while (safetags[i]) {
    hashadd(safetags[i], (void *) &dummy, &htab_tag);
    i++;
  }
}

FUNCTION(fun_ord)
{
  char *m;
  size_t len = 0;
  if (!args[0] || !args[0][0]) {
    safe_str(T("#-1 FUNCTION EXPECTS ONE CHARACTER"), buff, bp);
    return;
  }
  m = remove_markup(args[0], &len);

  if (len != 2)			/* len includes trailing nul */
    safe_str(T("#-1 FUNCTION EXPECTS ONE CHARACTER"), buff, bp);
  else if (isprint((unsigned char) *m))
    safe_integer((unsigned char) *m, buff, bp);
  else
    safe_str(T("#-1 UNPRINTABLE CHARACTER"), buff, bp);
}

FUNCTION(fun_chr)
{
  int c;

  if (!is_integer(args[0])) {
    safe_str(T(e_uint), buff, bp);
    return;
  }
  c = parse_integer(args[0]);
  if (c < 0 || c > UCHAR_MAX)
    safe_str(T("#-1 THIS ISN'T UNICODE"), buff, bp);
  else if (isprint(c))
    safe_chr(c, buff, bp);
  else
    safe_str(T("#-1 UNPRINTABLE CHARACTER"), buff, bp);

}

FUNCTION(fun_accent)
{
  if (arglens[0] != arglens[1]) {
    safe_str(T("#-1 STRING LENGTHS MUST BE EQUAL"), buff, bp);
    return;
  }
  safe_accent(args[0], args[1], arglens[0], buff, bp);
}

FUNCTION(fun_stripaccents)
{
  int n;
  for (n = 0; n < arglens[0]; n++) {
    if (accent_table[(unsigned char) args[0][n]].base)
      safe_str(accent_table[(unsigned char) args[0][n]].base, buff, bp);
    else
      safe_chr(args[0][n], buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_html)
{
  if (!Wizard(executor))
    safe_str(T(e_perm), buff, bp);
  else
    safe_tag(args[0], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_tag)
{
  int i;
  if (!Wizard(executor) && !hash_find(&htab_tag, strupper(args[0])))
    safe_str("#-1", buff, bp);
  else {
    safe_chr(TAG_START, buff, bp);
    safe_strl(args[0], arglens[0], buff, bp);
    for (i = 1; i < nargs; i++) {
      safe_chr(' ', buff, bp);
      safe_strl(args[i], arglens[i], buff, bp);
    }
    safe_chr(TAG_END, buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_endtag)
{
  if (!Wizard(executor) && !hash_find(&htab_tag, strupper(args[0])))
    safe_str("#-1", buff, bp);
  else
    safe_tag_cancel(args[0], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_tagwrap)
{
  if (!Wizard(executor) && !hash_find(&htab_tag, strupper(args[0])))
    safe_str("#-1", buff, bp);
  else {
    if (nargs == 2)
      safe_tag_wrap(args[0], NULL, args[1], buff, bp);
    else
      safe_tag_wrap(args[0], args[1], args[2], buff, bp);
  }
}

#define COL_FLASH       (1)
#define COL_HILITE      (2)
#define COL_INVERT      (4)
#define COL_UNDERSCORE  (8)

#define VAL_FLASH       (5)
#define VAL_HILITE      (1)
#define VAL_INVERT      (7)
#define VAL_UNDERSCORE  (4)

#define COL_BLACK       (30)
#define COL_RED         (31)
#define COL_GREEN       (32)
#define COL_YELLOW      (33)
#define COL_BLUE        (34)
#define COL_MAGENTA     (35)
#define COL_CYAN        (36)
#define COL_WHITE       (37)

typedef struct {
  char flags;
  char fore;
  char back;
} ansi_data;

static void dump_ansi_codes(ansi_data * ad, char *buff, char **bp);

#define EDGE_UP(x,y,z)  (((y) & (z)) && !((x) & (z)))

static void
dump_ansi_codes(ad, buff, bp)
    ansi_data *ad;
    char *buff;
    char **bp;
{
  static ansi_data old_ad = { 0, 0, 0 };
  int f = 0;

  if ((old_ad.fore && !ad->fore)
      || (old_ad.back && !ad->back)
      || ((old_ad.flags & ad->flags) != old_ad.flags)) {
    safe_str(ANSI_NORMAL, buff, bp);
    old_ad.flags = 0;
    old_ad.fore = 0;
    old_ad.back = 0;
  }

  if ((old_ad.fore == ad->fore)
      && (old_ad.back == ad->back)
      && (old_ad.flags == ad->flags))
    /* If nothing has changed, don't bother doing anything.
     * This stops the entirely pointless \e[m being generated. */
    return;

  safe_str(ANSI_BEGIN, buff, bp);

  if (EDGE_UP(old_ad.flags, ad->flags, COL_FLASH)) {
    if (f++)
      safe_chr(';', buff, bp);
    safe_integer(VAL_FLASH, buff, bp);
  }

  if (EDGE_UP(old_ad.flags, ad->flags, COL_HILITE)) {
    if (f++)
      safe_chr(';', buff, bp);
    safe_integer(VAL_HILITE, buff, bp);
  }

  if (EDGE_UP(old_ad.flags, ad->flags, COL_INVERT)) {
    if (f++)
      safe_chr(';', buff, bp);
    safe_integer(VAL_INVERT, buff, bp);
  }

  if (EDGE_UP(old_ad.flags, ad->flags, COL_UNDERSCORE)) {
    if (f++)
      safe_chr(';', buff, bp);
    safe_integer(VAL_UNDERSCORE, buff, bp);
  }

  if (ad->fore != old_ad.fore) {
    if (f++)
      safe_chr(';', buff, bp);
    safe_integer(ad->fore, buff, bp);
  }

  if (ad->back != old_ad.back) {
    if (f++)
      safe_chr(';', buff, bp);
    safe_integer(ad->back + 10, buff, bp);
  }

  safe_str(ANSI_END, buff, bp);

  old_ad = *ad;

}


/* ARGSUSED */
FUNCTION(fun_ansi)
{
  static char tbuff[BUFFER_LEN];
  static ansi_data stack[1024] = { {0, 0, 0} }, *sp = stack;
  char const *arg0, *arg1;
  char *tbp;

  tbp = tbuff;
  arg0 = args[0];
  process_expression(tbuff, &tbp, &arg0, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  *tbp = '\0';

  sp[1] = sp[0];
  sp++;

  for (tbp = tbuff; *tbp; tbp++) {
    switch (*tbp) {
    case 'n':			/* normal */
      sp->flags = 0;
      sp->fore = 0;
      sp->back = 0;
      break;
    case 'f':			/* flash */
      sp->flags |= COL_FLASH;
      break;
    case 'h':			/* hilite */
      sp->flags |= COL_HILITE;
      break;
    case 'i':			/* inverse */
      sp->flags |= COL_INVERT;
      break;
    case 'u':			/* underscore */
      sp->flags |= COL_UNDERSCORE;
      break;
    case 'F':			/* flash */
      sp->flags &= ~COL_FLASH;
      break;
    case 'H':			/* hilite */
      sp->flags &= ~COL_HILITE;
      break;
    case 'I':			/* inverse */
      sp->flags &= ~COL_INVERT;
      break;
    case 'U':			/* underscore */
      sp->flags &= ~COL_UNDERSCORE;
      break;
    case 'b':			/* blue fg */
      sp->fore = COL_BLUE;
      break;
    case 'c':			/* cyan fg */
      sp->fore = COL_CYAN;
      break;
    case 'g':			/* green fg */
      sp->fore = COL_GREEN;
      break;
    case 'm':			/* magenta fg */
      sp->fore = COL_MAGENTA;
      break;
    case 'r':			/* red fg */
      sp->fore = COL_RED;
      break;
    case 'w':			/* white fg */
      sp->fore = COL_WHITE;
      break;
    case 'x':			/* black fg */
      sp->fore = COL_BLACK;
      break;
    case 'y':			/* yellow fg */
      sp->fore = COL_YELLOW;
      break;
    case 'B':			/* blue bg */
      sp->back = COL_BLUE;
      break;
    case 'C':			/* cyan bg */
      sp->back = COL_CYAN;
      break;
    case 'G':			/* green bg */
      sp->back = COL_GREEN;
      break;
    case 'M':			/* magenta bg */
      sp->back = COL_MAGENTA;
      break;
    case 'R':			/* red bg */
      sp->back = COL_RED;
      break;
    case 'W':			/* white bg */
      sp->back = COL_WHITE;
      break;
    case 'X':			/* black bg */
      sp->back = COL_BLACK;
      break;
    case 'Y':			/* yellow bg */
      sp->back = COL_YELLOW;
      break;
    }
  }

  dump_ansi_codes(sp, buff, bp);

  arg1 = args[1];
  process_expression(buff, bp, &arg1, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);

  dump_ansi_codes(--sp, buff, bp);

}

/* ARGSUSED */
FUNCTION(fun_stripansi)
{
  /* Strips ANSI codes away from a given string of text. Starts by
   * finding the '\x' character and stripping until it hits an 'm'.
   */

  char *cp;

  cp = remove_markup(args[0], NULL);
  safe_str(cp, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_edit)
{
  int len;
  char *str, *f, *r;
  int i;
  char prebuf[BUFFER_LEN], *prep;
  char postbuf[BUFFER_LEN], *postp;

  postp = postbuf;
  safe_str(args[0], postbuf, &postp);
  *postp = '\0';
  for (i = 1; i < nargs - 1; i += 2) {
    /* Old postbuf is new prebuf */
    prep = prebuf;
    safe_str(postbuf, prebuf, &prep);
    *prep = '\0';
    str = prebuf;		/* The current string */
    f = args[i];		/* find this */
    r = args[i + 1];		/* replace it with this */
    postp = postbuf;

    /* Check for nothing to avoid infinite loop */
    if (!*f && !*r)
      continue;

    if (!strcmp(f, "$")) {
      /* append */
      safe_str(str, postbuf, &postp);
      safe_str(r, postbuf, &postp);
    } else if (!strcmp(f, "^")) {
      /* prepend */
      safe_str(r, postbuf, &postp);
      safe_str(str, postbuf, &postp);
    } else {
      len = strlen(f);
      while (*str) {
	if (!strncmp(str, f, len)) {
	  safe_str(r, postbuf, &postp);
	  if (len)
	    str += len;
	  else {
	    safe_chr(*str, postbuf, &postp);
	    str++;
	  }
	} else {
	  safe_chr(*str, postbuf, &postp);
	  str++;
	}
      }
      if (!*f)
	safe_str(r, postbuf, &postp);
    }
    *postp = '\0';
  }
  safe_str(postbuf, buff, bp);
}

FUNCTION(fun_brackets)
{
  char *str;
  int rbrack, lbrack, rbrace, lbrace, lcurl, rcurl;

  lcurl = rcurl = rbrack = lbrack = rbrace = lbrace = 0;
  str = args[0];		/* The string to count the brackets in */
  while (*str) {
    switch (*str) {
    case '[':
      lbrack++;
      break;
    case ']':
      rbrack++;
      break;
    case '(':
      lbrace++;
      break;
    case ')':
      rbrace++;
      break;
    case '{':
      lcurl++;
      break;
    case '}':
      rcurl++;
      break;
    default:
      break;
    }
    str++;
  }
  safe_format(buff, bp, "%d %d %d %d %d %d", lbrack, rbrack,
	      lbrace, rbrace, lcurl, rcurl);
}


/* Returns the length of str up to the first return character, 
 * or else the last space, or else 0.
 */
static int
wraplen(char *str, int maxlen)
{
  const int length = strlen(str);
  int i = 0;

  if (length <= maxlen) {
    /* Find the first return char
     * so %r will not mess with any alignment
     * functions.
     */
    while (i < length) {
      if ((str[i] == '\n') || (str[i] == '\r'))
	return i;
      i++;
    }
    return length;
  }

  /* Find the first return char
   * so %r will not mess with any alignment
   * functions.
   */
  while (i <= maxlen + 1) {
    if ((str[i] == '\n') || (str[i] == '\r'))
      return i;
    i++;
  }

  /* No return char was found. Now 
   * find the last space in str.
   */
  while (str[maxlen] != ' ' && maxlen > 0)
    maxlen--;

  return (maxlen ? maxlen : -1);
}

/* The integer in string a will be stored in v, 
 * if a is not an integer then d (efault) is stored in v. 
 */
#define initint(a, v, d) \
  do \
   if (arglens[a] == 0) { \
      v = d; \
   } else { \
     if (!is_integer(args[a])) { \
        safe_str(T(e_int), buff, bp); \
        return; \
     } \
     v = parse_integer(args[a]); \
  } \
 while (0)

FUNCTION(fun_wrap)
{
/*  args[0]  =  text to be wrapped (required)
 *  args[1]  =  line width (width) (required)
 *  args[2]  =  width of first line (width1st)
 *  args[3]  =  output delimiter (btwn lines)
 */

  char *pstr;			/* start of string */
  ansi_string *as;
  const char *pend;		/* end of string */
  int linewidth, width1st, width;
  int linenr = 0;
  const char *linesep;
  int ansiwidth, ansilen;

  if (!args[0] || !*args[0])
    return;

  initint(1, width, 72);
  width1st = width;
  if (nargs > 2)
    initint(2, width1st, width);
  if (nargs > 3)
    linesep = args[3];
  else if (NEWLINE_ONE_CHAR)
    linesep = "\n";
  else
    linesep = "\r\n";

  if (width < 2 || width1st < 2) {
    safe_str(T("#-1 WIDTH TOO SMALL"), buff, bp);
    return;
  }

  as = parse_ansi_string(args[0]);
  pstr = as->text;
  pend = as->text + as->len;

  linewidth = width1st;
  while (pstr < pend) {
    if (linenr++ == 1)
      linewidth = width;
    if ((linenr > 1) && linesep && *linesep)
      safe_str(linesep, buff, bp);

    ansiwidth = ansi_strnlen(pstr, linewidth);
    ansilen = wraplen(pstr, ansiwidth);

    if (ansilen < 0) {
      /* word doesn't fit on one line, so cut it */
      safe_ansi_string(as, pstr - as->text, ansiwidth - 1, buff, bp);
      safe_chr('-', buff, bp);
      pstr += ansiwidth - 1;	/* move to start of next line */
    } else {
      /* normal line */
      safe_ansi_string(as, pstr - as->text, ansilen, buff, bp);
      if (pstr[ansilen] == '\r')
	++ansilen;
      pstr += ansilen + 1;	/* move to start of next line */
    }
  }
  free_ansi_string(as);
}

#undef initint
