/**
 * \file wild.c
 *
 * \brief Wildcard matching routings for PennMUSH
 *
 * Written by T. Alexander Popiel, 24 June 1993
 * Last modified by Javelin, 2002-2003
 *
 * Thanks go to Andrew Molitor for debugging
 * Thanks also go to Rich $alz for code to benchmark against
 *
 * Copyright (c) 1993,2000 by T. Alexander Popiel
 * This code is available under the terms of the GPL,
 * see http://www.gnu.org/copyleft/gpl.html for details.
 *
 * This code is included in PennMUSH under the PennMUSH
 * license by special dispensation from the author,
 * T. Alexander Popiel.  If you wish to include this
 * code in other packages, but find the GPL too onerous,
 * then please feel free to contact the author at
 * popiel@wolfskeep.com to work out agreeable terms.
 */

#include "config.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "copyrite.h"
#include "conf.h"
#include "case.h"
#include "externs.h"
#include "memcheck.h"
#include "mymalloc.h"
#include "parse.h"
#include "pcre.h"
#include "confmagic.h"

#define FIXCASE(a) (DOWNCASE(a))
#define EQUAL(cs,a,b) ((cs) ? (a == b) : (FIXCASE(a) == FIXCASE(b)))
#define NOTEQUAL(cs,a,b) ((cs) ? (a != b) : (FIXCASE(a) != FIXCASE(b)))
#define NUMARGS (10)

const unsigned char *tables = NULL;

static char wspace[3 * BUFFER_LEN + NUMARGS];	/* argument return buffer */
						/* big to match tprintf */

static int wild1
  (const char *RESTRICT tstr, const char *RESTRICT dstr, int arg,
   char *RESTRICT wbuf, int cs);
static int wild(const char *RESTRICT s, const char *RESTRICT d, int p, int cs);
static int check_literals(const char *RESTRICT tstr, const char *RESTRICT dstr,
			  int cs);
static char *strip_backslashes(const char *str);

/** Do a wildcard match, without remembering the wild data.
 *
 * This routine will cause crashes if fed NULLs instead of strings.
 *
 * \param tstr pattern to match against.
 * \param dstr string to check.
 * \retval 1 dstr matches the tstr pattern.
 * \retval 0 dstr does not match the tstr pattern.
 */
int
quick_wild(const char *RESTRICT tstr, const char *RESTRICT dstr)
{
  /* quick_wild_new does the real work, but before we call it, 
   * we do some sanity checking. 
   */
  if (!check_literals(tstr, dstr, 0))
    return 0;
  return quick_wild_new(tstr, dstr, 0);
}

/** Do a wildcard match, possibly case-sensitive, without memory.
 *
 * This probably crashes if fed NULLs instead of strings, too.
 *
 * \param tstr pattern to match against.
 * \param dstr string to check.
 * \param cs if 1, case-sensitive; if 0, case-insensitive.
 * \retval 1 dstr matches the tstr pattern.
 * \retval 0 dstr does not match the tstr pattern.
 */
int
quick_wild_new(const char *RESTRICT tstr, const char *RESTRICT dstr, int cs)
{
  while (*tstr != '*') {
    switch (*tstr) {
    case '?':
      /* Single character match.  Return false if at
       * end of data.
       */
      if (!*dstr)
	return 0;
      break;
    case '\\':
      /* Escape character.  Move up, and force literal
       * match of next character.
       */
      tstr++;
      /* FALL THROUGH */
    default:
      /* Literal character.  Check for a match.
       * If matching end of data, return true.
       */
      if (NOTEQUAL(cs, *dstr, *tstr))
	return 0;
      if (!*dstr)
	return 1;
    }
    tstr++;
    dstr++;
  }

  /* Skip over '*'. */
  tstr++;

  /* Return true on trailing '*'. */
  if (!*tstr)
    return 1;

  /* Skip over wildcards. */
  while ((*tstr == '?') || (*tstr == '*')) {
    if (*tstr == '?') {
      if (!*dstr)
	return 0;
      dstr++;
    }
    tstr++;
  }

  /* Skip over a backslash in the pattern string if it is there. */
  if (*tstr == '\\')
    tstr++;

  /* Return true on trailing '*'. */
  if (!*tstr)
    return 1;

  /* Scan for possible matches. */
  while (*dstr) {
    if (EQUAL(cs, *dstr, *tstr) && quick_wild_new(tstr + 1, dstr + 1, cs))
      return 1;
    dstr++;
  }
  return 0;
}

/* ---------------------------------------------------------------------------
 * wild1: INTERNAL: do a wildcard match, remembering the wild data.
 *
 * DO NOT CALL THIS FUNCTION DIRECTLY - DOING SO MAY RESULT IN
 * SERVER CRASHES AND IMPROPER ARGUMENT RETURN.
 *
 * Side Effect: this routine modifies the 'wnxt' global variable,
 * and what it points to.
 */
static int
wild1(const char *RESTRICT tstr, const char *RESTRICT dstr, int arg,
      char *RESTRICT wbuf, int cs)
{
  const char *datapos;
  char *wnext;
  int argpos, numextra;

  while (*tstr != '*') {
    switch (*tstr) {
    case '?':
      /* Single character match.  Return false if at
       * end of data.
       */
      if (!*dstr)
	return 0;

      wnxt[arg++] = wbuf;
      *wbuf++ = *dstr;
      *wbuf++ = '\0';

      /* Jump to the fast routine if we can. */

      if (arg >= NUMARGS)
	return quick_wild_new(tstr + 1, dstr + 1, cs);
      break;
    case '\\':
      /* Escape character.  Move up, and force literal
       * match of next character.
       */
      tstr++;
      /* FALL THROUGH */
    default:
      /* Literal character.  Check for a match.
       * If matching end of data, return true.
       */
      if (NOTEQUAL(cs, *dstr, *tstr))
	return 0;
      if (!*dstr)
	return 1;
    }
    tstr++;
    dstr++;
  }

  /* If at end of pattern, slurp the rest, and leave. */
  if (!tstr[1]) {
    wnxt[arg] = wbuf;
    strcpy(wbuf, dstr);
    return 1;
  }
  /* Remember current position for filling in the '*' return. */
  datapos = dstr;
  argpos = arg;

  /* Scan forward until we find a non-wildcard. */
  do {
    if (argpos < arg) {
      /* Fill in arguments if someone put another '*'
       * before a fixed string.
       */
      wnxt[argpos++] = wbuf;
      *wbuf++ = '\0';

      /* Jump to the fast routine if we can. */
      if (argpos >= NUMARGS)
	return quick_wild_new(tstr, dstr, cs);

      /* Fill in any intervening '?'s */
      while (argpos < arg) {
	wnxt[argpos++] = wbuf;
	*wbuf++ = *datapos++;
	*wbuf++ = '\0';

	/* Jump to the fast routine if we can. */
	if (argpos >= NUMARGS)
	  return quick_wild_new(tstr, dstr, cs);
      }
    }
    /* Skip over the '*' for now... */
    tstr++;
    arg++;

    /* Skip over '?'s for now... */
    numextra = 0;
    while (*tstr == '?') {
      if (!*dstr)
	return 0;
      tstr++;
      dstr++;
      arg++;
      numextra++;
    }
  } while (*tstr == '*');

  /* Skip over a backslash in the pattern string if it is there. */
  if (*tstr == '\\')
    tstr++;

  /* Check for possible matches.  This loop terminates either at
   * end of data (resulting in failure), or at a successful match.
   */
  if (!*tstr)
    while (*dstr)
      dstr++;
  else {
    wnext = wbuf;
    wnext++;
    while (1) {
      if (EQUAL(cs, *dstr, *tstr) &&
	  ((arg < NUMARGS) ? wild1(tstr, dstr, arg, wnext, cs)
	   : quick_wild_new(tstr, dstr, cs)))
	break;
      if (!*dstr)
	return 0;
      dstr++;
      wnext++;
    }
  }

  /* Found a match!  Fill in all remaining arguments.
   * First do the '*'...
   */
  wnxt[argpos++] = wbuf;
  strncpy(wbuf, datapos, (dstr - datapos) - numextra);
  wbuf += (dstr - datapos) - numextra;
  *wbuf++ = '\0';
  datapos = dstr - numextra;

  /* Fill in any trailing '?'s that are left. */
  while (numextra) {
    if (argpos >= NUMARGS)
      return 1;
    wnxt[argpos++] = wbuf;
    *wbuf++ = *datapos++;
    *wbuf++ = '\0';
    numextra--;
  }

  /* It's done! */
  return 1;
}

/* ---------------------------------------------------------------------------
 * wild: do a wildcard match, remembering the wild data.
 *
 * This routine will cause crashes if fed NULLs instead of strings.
 *
 * This function may crash if malloc() fails.
 *
 * Side Effect: this routine modifies the 'wnxt' global variable.
 */
static int
wild(const char *RESTRICT s, const char *RESTRICT d, int p, int cs)
{
  /* Do fast match. */
  while ((*s != '*') && (*s != '?')) {
    if (*s == '\\')
      s++;
    if (NOTEQUAL(cs, *d, *s))
      return 0;
    if (!*d)
      return 1;
    s++;
    d++;
  }

  /* Do sanity check */
  if (!check_literals(s, d, cs))
    return 0;

  /* Do the match. */
  return wild1(s, d, p, wspace, cs);
}

/** Wildcard match, possibly case-sensitive, and remember the wild data.
 *
 * This routine will cause crashes if fed NULLs instead of strings.
 *
 * \param s pattern to match against.
 * \param d string to check.
 * \param cs if 1, case-sensitive; if 0, case-insensitive.
 * \retval 1 d matches s.
 * \retval 0 d doesn't match s.
 */
int
wild_match_case(const char *RESTRICT s, const char *RESTRICT d, int cs)
{
  int j;
  /* Clear %0-%9 and r(0) - r(9) */
  for (j = 0; j < NUMARGS; j++)
    wnxt[j] = (char *) NULL;
  for (j = 0; j < NUMQ; j++)
    rnxt[j] = (char *) NULL;
  return wild(s, d, 0, cs);
}

/** Regexp match, possibly case-sensitive, and remember matched subexpressions.
 *
 * This routine will cause crashes if fed NULLs instead of strings.
 *
 * \param s regexp to match against.
 * \param d string to check.
 * \param cs if 1, case-sensitive; if 0, case-insensitive.
 * \retval 1 d matches s.
 * \retval 0 d doesn't match s.
 */
int
regexp_match_case(const char *RESTRICT s, const char *RESTRICT d, int cs)
{
  int j;
  pcre *re;
  int i;
  static char wtmp[NUMARGS][BUFFER_LEN];
  const char *errptr;
  int erroffset;
  int offsets[99];
  int subpatterns;

  if (!tables) {
    /* Initialize char tables so they match current locale. */
    tables = pcre_maketables();
  }

  if ((re = pcre_compile(s, (cs ? 0 : PCRE_CASELESS), &errptr, &erroffset,
			 tables)) == NULL) {
    /*
     * This is a matching error. We have an error message in
     * errptr that we can ignore, since we're doing
     * command-matching.
     */
    return 0;
  }
#ifdef MEM_CHECK
  add_check("pcre");
#endif
  /* 
   * Now we try to match the pattern. The relevant fields will
   * automatically be filled in by this.
   */
  if ((subpatterns = pcre_exec(re, NULL, d, strlen(d), 0, 0, offsets, 99))
      < 0) {
    mush_free(re, "pcre");
    return 0;
  }
  /* If we had too many subpatterns for the offsets vector, set the number
   * to 1/3 of the size of the offsets vector
   */
  if (subpatterns == 0)
    subpatterns = 33;

  /*
   * Now we fill in our args vector. Note that in regexp matching,
   * 0 is the entire string matched, and the parenthesized strings
   * go from 1 to 9. We DO PRESERVE THIS PARADIGM, for consistency
   * with other languages.
   */

  /* Clear %0-%9 and r(0) - r(9) */
  for (j = 0; j < NUMARGS; j++) {
    wtmp[j][0] = '\0';
    wnxt[j] = (char *) NULL;
  }
  for (j = 0; j < NUMQ; j++)
    rnxt[j] = (char *) NULL;

  for (i = 0; (i < 10) && (i < NUMARGS); i++) {
    pcre_copy_substring(d, offsets, subpatterns, i, wtmp[i], BUFFER_LEN);
    wnxt[i] = wtmp[i];
  }

  mush_free(re, "pcre");
  return 1;
}


/** Regexp match, possibly case-sensitive, and with no memory.
 *
 * This routine will cause crashes if fed NULLs instead of strings.
 *
 * \param s regexp to match against.
 * \param d string to check.
 * \param cs if 1, case-sensitive; if 0, case-insensitive.
 * \retval 1 d matches s.
 * \retval 0 d doesn't match s.
 */
int
quick_regexp_match(const char *RESTRICT s, const char *RESTRICT d, int cs)
{
  pcre *re;
  const char *errptr;
  int erroffset;
  int offsets[99];
  int r;

  if (!tables) {
    /* Initialize char tables so they match current locale. */
    tables = pcre_maketables();
  }

  if ((re = pcre_compile(s, (cs ? 0 : PCRE_CASELESS), &errptr, &erroffset,
			 tables)) == NULL) {
    /*
     * This is a matching error. We have an error message in
     * errptr that we can ignore, since we're doing
     * command-matching.
     */
    return 0;
  }
#ifdef MEM_CHECK
  add_check("pcre");
#endif
  /* 
   * Now we try to match the pattern. The relevant fields will
   * automatically be filled in by this.
   */
  r = pcre_exec(re, NULL, d, strlen(d), 0, 0, offsets, 99);

  mush_free(re, "pcre");

  return r >= 0;
}


/** Either an order comparison or a wildcard match with no memory.
 *
 * This routine will cause crashes if fed NULLs instead of strings.
 *
 * \param s pattern to match against.
 * \param d string to check.
 * \param cs if 1, case-sensitive; if 0, case-insensitive.
 * \retval 1 d matches s.
 * \retval 0 d doesn't match s.
 */
int
local_wild_match_case(const char *RESTRICT s, const char *RESTRICT d, int cs)
{
  switch (*s) {
  case '>':
    s++;
    if (is_number(s) && is_number(d))
      return (parse_number(s) < parse_number(d));
    else
      return (strcoll(s, d) < 0);
  case '<':
    s++;
    if (is_number(s) && is_number(d))
      return (parse_number(s) > parse_number(d));
    else
      return (strcoll(s, d) > 0);
  }

  return quick_wild_new(s, d, cs);
}

/** Does a string contain a wildcard character (* or ?)?
 * Not used by the wild matching routines, but suitable for outside use.
 * \param s string to check.
 * \retval 1 s contains a * or ?
 * \retval 0 s does not contain a * or ?
 */
int
wildcard(const char *s)
{
  if (strchr(s, '*') || strchr(s, '?'))
    return 1;
  return 0;
}

static int
check_literals(const char *RESTRICT tstr, const char *RESTRICT dstr, int cs)
{
  /* Every literal string in tstr must appear, in order, in dstr,
   * or no match can happen. That is, tstr is the pattern and dstr
   * is the string-to-match
   */
  char tbuf1[BUFFER_LEN];
  char dbuf1[BUFFER_LEN];
  const char delims[] = "?*";
  char *sp, *dp;
  strncpy(dbuf1, dstr, BUFFER_LEN - 1);
  dbuf1[BUFFER_LEN - 1] = '\0';
  strcpy(tbuf1, strip_backslashes(tstr));
  if (!cs) {
    upcasestr(tbuf1);
    upcasestr(dbuf1);
  }
  dp = dbuf1;
  sp = strtok(tbuf1, delims);
  while (sp) {
    if (!dp)
      return 0;
    if (!(dp = strstr(dp, sp)))
      return 0;
    dp += strlen(sp);
    sp = strtok(NULL, delims);
  }
  return 1;
}


static char *
strip_backslashes(const char *str)
{
  /* Remove backslashes from a string, and return it in a static buffer */
  static char buf[BUFFER_LEN];
  int i = 0;

  while (*str && (i < BUFFER_LEN - 1)) {
    if (*str == '\\' && *(str + 1))
      str++;
    buf[i++] = *str++;
  }
  buf[i] = '\0';
  return buf;
}
