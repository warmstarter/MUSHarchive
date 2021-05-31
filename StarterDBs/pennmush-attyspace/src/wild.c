/* wild.c - wildcard routines

 * * Written by T. Alexander Popiel, 24 June 1993
 * * Last modified by T. Alexander Popiel, 15 October 1993
 * *
 * * Thanks go to Andrew Molitor for debugging
 * * Thanks also go to Rich $alz for code to benchmark against
 * *
 * * Copyright (c) 1993 by T. Alexander Popiel
 * * This code is hereby placed under GNU copyleft,
 * * see copyrite.h for details.
 */
#include "config.h"
#include <ctype.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif

#include "copyrite.h"
#include "conf.h"
#include "intrface.h"
#include "globals.h"
#include "externs.h"
#include "mymalloc.h"
#include "myregexp.h"
#include "confmagic.h"

#define FIXCASE(a) (DOWNCASE(a))
#define EQUAL(cs,a,b) ((cs) ? (a == b) : (FIXCASE(a) == FIXCASE(b)))
#define NOTEQUAL(cs,a,b) ((cs) ? (a != b) : (FIXCASE(a) != FIXCASE(b)))
#define NUMARGS	(10)

static char wspace[3 * BUFFER_LEN + NUMARGS];	/* argument return buffer */
						/* big to match tprintf */

int wild_match_case _((const char *s, const char *d, int cs));
int regexp_match_case _((const char *s, const char *d, int cs));
int local_wild_match _((const char *s, const char *d));
int wildcard _((const char *));
int quick_wild_new _((const char *tstr, const char *dstr, int cs));
static int wild1 _((const char *tstr, const char *dstr, int arg, char *wbuf, int cs));
static int wild _((const char *s, const char *d, int p, int cs));

/* ---------------------------------------------------------------------------
 * quick_wild: do a wildcard match, without remembering the wild data.
 *
 * This routine will cause crashes if fed NULLs instead of strings.
 */
int
quick_wild_new(tstr, dstr, cs)
    const char *tstr;
    const char *dstr;
    int cs;			/* Case sensitive? */
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
    if (EQUAL(cs, *dstr, *tstr) &&
	quick_wild_new(tstr + 1, dstr + 1, cs))
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
wild1(tstr, dstr, arg, wbuf, cs)
    const char *tstr;
    const char *dstr;
    int arg;
    char *wbuf;
    int cs;			/* Case sensitive? */
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
wild(s, d, p, cs)
    const char *s;
    const char *d;
    int p;
    int cs;			/* Case sensitive? */
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

  /* Do the match. */
  return wild1(s, d, p, wspace, cs);
}

/* ---------------------------------------------------------------------------
 * wild_match: do a wildcard
 * match, remembering the wild data, if wildcard match is done.
 *
 * This routine will cause crashes if fed NULLs instead of strings.
 */
int
wild_match_case(s, d, cs)
    const char *s;
    const char *d;
    int cs;			/* Case sensitive match? */
{
  int j;
  /* Clear %0-%9 and r(0) - r(9) */
  for (j = 0; j < NUMARGS; j++)
    wnxt[j] = (char *) NULL;
  for (j = 0; j < 10; j++)
    rnxt[j] = (char *) NULL;
  return wild(s, d, 0, cs);
}

/* ---------------------------------------------------------------------------
 * regexp_match: do a regexp match, remembering the matched subexpressions
 *
 * Based on TinyMUSH 2.2.4
 */
int
regexp_match_case(s, d, cs)
    const char *s;
    const char *d;
    int cs;			/* Case sensitive? */
{
  int j;
  regexp *re;
  int i, len;
  static char wtmp[NUMARGS][BUFFER_LEN];
  char *news, *newd;

  if (cs) {
    news = (char *) s;
    newd = (char *) d;
  } else {
    news = strdup(strupper(s));
    newd = strdup(strupper(d));
#ifdef MEM_CHECK
    add_check("regexp_upcase");
    add_check("regexp_upcase");
#endif
  }


  if ((re = regcomp((char *) news)) == NULL) {
    /*
     * This is a matching error. We have an error message in
     * regexp_errbuf that we can ignore, since we're doing
     * command-matching.
     */
    if (!cs) {
      mush_free(news, "regexp_upcase");
      mush_free(newd, "regexp_upcase");
    }
    return 0;
  }
  /* 
   * Now we try to match the pattern. The relevant fields will
   * automatically be filled in by this.
   */
  if (!regexec(re, (char *) newd)) {
    mush_free(re, "regexp");
    if (!cs) {
      mush_free(news, "regexp_upcase");
      mush_free(newd, "regexp_upcase");
    }
    return 0;
  }
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
    rnxt[j] = (char *) NULL;
  }

  for (i = 0;
       (i < NSUBEXP) && (i < NUMARGS);
       i++) {
    if (re->startp[i] && re->endp[i]) {
      len = re->endp[i] - re->startp[i];
      if (len > BUFFER_LEN - 1)
	len = BUFFER_LEN - 1;
      if (len < 0)
	len = 0;
      strncpy(wtmp[i], re->startp[i], len);
      wtmp[i][len] = '\0';	/* strncpy() does not null-terminate */
      wnxt[i] = wtmp[i];
    }
  }

  mush_free(re, "regexp");
  if (!cs) {
    mush_free(news, "regexp_upcase");
    mush_free(newd, "regexp_upcase");
  }
  return 1;
}


/* ---------------------------------------------------------------------------
 * local_wild_match: do either an order comparison or a wildcard match.
 * Ignore match correspondences.
 *
 * This routine will cause crashes if fed NULLs instead of strings.
 */
int
local_wild_match(s, d)
    const char *s;
    const char *d;
{
  switch (*s) {
  case '>':
    s++;
    if ((isascii(*s) && isdigit(*s)) || (*s == '-'))
      return (atoi(s) < atoi(d));
    else
      return (strcmp(s, d) < 0);
  case '<':
    s++;
    if ((isascii(*s) && isdigit(*s)) || (*s == '-'))
      return (atoi(s) > atoi(d));
    else
      return (strcmp(s, d) > 0);
  }

  return quick_wild_new(s, d, 0);
}

/* ---------------------------------------------------------------------------
 * wildcard - return 1 if the string has a wildcard character (* or ?)
 * in it, and 0 otherwise. Not used by the wild matching routines, but
 * suitable for outside use.
 */
int
wildcard(s)
    const char *s;
{
  if (strchr(s, '*') || strchr(s, '?'))
    return 1;
  return 0;
}
