#include "copyrite.h"

#include "config.h"
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#include "conf.h"
#include "ansi.h"
#include "externs.h"
#include "globals.h"
#include "intrface.h"
#include "match.h"
#include "parse.h"
#include "pueblo.h"
#include "myregexp.h"
#include "confmagic.h"

#ifdef WIN32
#pragma warning( disable : 4761)	/* NJG: disable warning re conversion */
#endif

int get_gender _((dbref player));

int
get_gender(player)
    dbref player;
{
  /* 0 for error, 1 for neuter, 2 for female, 3 for male, 4 for plural */

  ATTR *a;

  a = atr_get(player, "SEX");

  if (!a)
    return 1;

  switch (*uncompress(a->value)) {
  case 'T':
  case 't':
  case 'P':
  case 'p':
    return 4;
  case 'M':
  case 'm':
    return 3;
  case 'F':
  case 'f':
  case 'W':
  case 'w':
    return 2;
  default:
    return 1;
  }
}

char const *subj[5] =
{"", "it", "she", "he", "they"};
char const *poss[5] =
{"", "its", "her", "his", "their"};
char const *obj[5] =
{"", "it", "her", "him", "them"};
char const *absp[5] =
{"", "its", "hers", "his", "theirs"};

/* ARGSUSED */
FUNCTION(fun_isword)
{
  /* is every character a letter? */

  char *p;
  for (p = args[0]; *p; p++) {
    if (!isalpha(*p)) {
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
  p = args[0];
  /* Skip any starting ANSI */
  while (*p == ESC_CHAR) {
    while (*p && *p != 'm')
      safe_chr(*p++, buff, bp);
    if (*p)
      safe_chr(*p++, buff, bp);	/* Tack on 'm' */
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

  char c = tolower(*args[0]);
  if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
    safe_str("an", buff, bp);
  else
    safe_str("a", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_subj)
{
  dbref thing;

  thing = match_thing(executor, args[0]);
  if (thing == NOTHING) {
    safe_str("#-1 NO MATCH", buff, bp);
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
    safe_str("#-1 NO MATCH", buff, bp);
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
    safe_str("#-1 NO MATCH", buff, bp);
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
    safe_str("#-1 NO MATCH", buff, bp);
    return;
  }
  safe_str(absp[get_gender(thing)], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_alphamax)
{
  char *amax;
  int j;

  amax = args[0];
  for (j = 1; j < nargs; j++)
    if (strcmp(amax, args[j]) < 0)
      amax = args[j];
  safe_str(amax, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_alphamin)
{
  char *amin;
  int j;

  amin = args[0];
  for (j = 1; j < nargs; j++)
    if (strcmp(amin, args[j]) > 0)
      amin = args[j];
  safe_str(amin, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_strlen)
{
  safe_str(unparse_integer(strlen(args[0])), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_mid)
{
  int pos, len;

  if (!is_integer(args[1]) || !is_integer(args[2])) {
    safe_str(e_ints, buff, bp);
    return;
  }
  pos = parse_integer(args[1]);
  len = parse_integer(args[2]);

  /* There was an error for pos + len > BUFFER_LEN, but I removed it.
   * After all, why should the behavior for past-end-of-string
   * change suddenly at BUFFER_LEN?
   */
  if ((pos < 0) || (len < 0)) {
    safe_str("#-1 OUT OF RANGE", buff, bp);
    return;
  }
  if (((pos + len) < BUFFER_LEN) && ((pos + len) >= 0))
    args[0][pos + len] = '\0';
  if ((Size_t) pos < strlen(args[0]))
    safe_str(args[0] + pos, buff, bp);
}


/* ARGSUSED */
FUNCTION(fun_left)
{
  int len;

  if (!is_integer(args[1])) {
    safe_str(e_int, buff, bp);
    return;
  }
  len = parse_integer(args[1]);

  if (len < 0) {
    safe_str("#-1 OUT OF RANGE", buff, bp);
    return;
  }
  if (len < BUFFER_LEN)
    args[0][len] = '\0';
  safe_str(args[0], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_right)
{
  int len;

  if (!is_integer(args[1])) {
    safe_str(e_int, buff, bp);
    return;
  }
  len = parse_integer(args[1]);

  if (len < 0) {
    safe_str("#-1 OUT OF RANGE", buff, bp);
    return;
  }
  len = strlen(args[0]) - len;
  if (len <= 0) {
    safe_str(args[0], buff, bp);
    return;
  }
  safe_str(args[0] + len, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_delete)
{
  /* delete a range of characters */

  int pos, num, len;

  if (!is_integer(args[1]) || !is_integer(args[2])) {
    safe_str(e_ints, buff, bp);
    return;
  }
  pos = parse_integer(args[1]);
  num = parse_integer(args[2]);
  len = strlen(args[0]);

  if (pos < 0) {
    safe_str("#-1 OUT OF RANGE", buff, bp);
    return;
  }
  if ((num > 0) && (pos < BUFFER_LEN))
    args[0][pos] = '\0';
  safe_str(args[0], buff, bp);
  if ((num > 0) && ((pos + num) < len))
    safe_str(args[0] + pos + num, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_comp)
{
  int x;
  x = strcmp(args[0], args[1]);
  if (x > 0)
    safe_chr('1', buff, bp);
  else if (x < 0)
    safe_str("-1", buff, bp);
  else
    safe_chr('0', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_pos)
{
  char *pos;
  pos = strstr(args[1], args[0]);
  if (pos)
    safe_str(unparse_integer(pos - args[1] + 1), buff, bp);
  else
    safe_str("#-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_strmatch)
{
  /* matches a wildcard pattern for an _entire_ string */
  safe_chr(quick_wild(args[1], args[0]) ? '1' : '0', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_strcat)
{
  int j;

  for (j = 0; j < nargs; j++)
    safe_str(args[j], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_flip)
{
  char *p;
  p = args[0] + strlen(args[0]);
  while (p > args[0])
    safe_chr(*--p, buff, bp);
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
  if (strlen(args[0]) != strlen(args[1])) {
    safe_str("#-1 STRING LENGTHS MUST BE EQUAL", buff, bp);
    return;
  }
  if (strlen(args[2]) > 1) {
    safe_str("#-1 TOO MANY CHARACTERS", buff, bp);
    return;
  }
  /* find the character to look for */
  if (!*args[2])
    c = ' ';
  else
    c = *args[2];

  /* walk strings, copy from the appropriate string */
  for (str = args[0], rep = args[1];
       *str && *rep;
       str++, rep++) {
    safe_chr((*str == c) ? *rep : *str, buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_lcstr)
{
  char *p;
  p = args[0];
  while (*p) {
    if (*p == ESC_CHAR) {
      /* Don't uppercase ANSI codes! */
      while (*p && *p != 'm')
	safe_chr(*p++, buff, bp);
      if (*p)
	safe_chr(*p++, buff, bp);	/* Tack on 'm' */
    } else {
      safe_chr(DOWNCASE(*p), buff, bp);
      p++;
    }
  }
}

/* ARGSUSED */
FUNCTION(fun_ucstr)
{
  char *p;
  p = args[0];
  while (*p) {
    if (*p == ESC_CHAR) {
      /* Don't uppercase ANSI codes! */
      while (*p && *p != 'm')
	safe_chr(*p++, buff, bp);
      if (*p)
	safe_chr(*p++, buff, bp);	/* Tack on 'm' */
    } else {
      safe_chr(UPCASE(*p), buff, bp);
      p++;
    }
  }
}

/* ARGSUSED */
FUNCTION(fun_repeat)
{
  int times;

  if (!is_integer(args[1])) {
    safe_str(e_int, buff, bp);
    return;
  }
  times = parse_integer(args[1]);
  if (times < 0) {
    safe_str("#-1 ARGUMENT MUST BE NON-NEGATIVE INTEGER", buff, bp);
    return;
  }
  if (!*args[0])
    return;
  while (times--)
    if (safe_str(args[0], buff, bp) != 0)
      break;
}

/* ARGSUSED */
FUNCTION(fun_scramble)
{
  int n, i, j;

  if (!*args[0])
    return;

  n = strlen(args[0]);
  for (i = 0; i < n; i++) {
    j = getrandom(n - i) + i;
    safe_chr(args[0][j], buff, bp);
    args[0][j] = args[0][i];
  }
}

/* ARGSUSED */
FUNCTION(fun_ljust)
{
  /* pads a string with trailing blanks (or other fill character) */

  int spaces;
  char sep;

  if (!is_integer(args[1])) {
    safe_str(e_int, buff, bp);
    return;
  }
  spaces = parse_integer(args[1]) - ansi_strlen(args[0]);
  if (spaces >= BUFFER_LEN)
    spaces = BUFFER_LEN - 1;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  safe_str(args[0], buff, bp);
  for (; spaces > 0; spaces--)
    safe_chr(sep, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_rjust)
{
  /* pads a string with leading blanks (or other fill character) */

  int spaces;
  char sep;

  if (!is_integer(args[1])) {
    safe_str(e_int, buff, bp);
    return;
  }
  spaces = parse_integer(args[1]) - ansi_strlen(args[0]);
  if (spaces >= BUFFER_LEN)
    spaces = BUFFER_LEN - 1;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  for (; spaces > 0; spaces--)
    safe_chr(sep, buff, bp);
  safe_str(args[0], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_center)
{
  /* pads a string with leading blanks (or other fill character) */

  int lsp, rsp;
  char sep;

  if (!is_integer(args[1])) {
    safe_str(e_int, buff, bp);
    return;
  }
  rsp = parse_integer(args[1]) - ansi_strlen(args[0]);
  lsp = rsp / 2;
  rsp -= lsp;
  if (lsp >= BUFFER_LEN)
    lsp = BUFFER_LEN - 1;
  if (rsp >= BUFFER_LEN)
    rsp = BUFFER_LEN - 1;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  for (; lsp > 0; lsp--)
    safe_chr(sep, buff, bp);
  safe_str(args[0], buff, bp);
  for (; rsp > 0; rsp--)
    safe_chr(sep, buff, bp);
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
  char const *asave, *ap, *lp;
  char cbuf[2];
  char *tptr;
  char sep;

  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;

  /* find our object and attribute */
  parse_attrib(executor, args[0], &thing, &attrib);
  if (!GoodObject(thing) || !attrib || !Can_Read_Attr(executor, thing, attrib))
    return;

  asave = safe_uncompress(attrib->value);

  /* save our stack */
  tptr = wenv[0];

  lp = trim_space_sep(args[1], ' ');
  cbuf[1] = '\0';
  wenv[0] = cbuf;
  while (*lp) {
    *cbuf = *lp++;
    ap = asave;
    process_expression(buff, bp, &ap, executor, caller, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
  }

  free((Malloc_t) asave);
  wenv[0] = tptr;
}

/* ARGSUSED */
FUNCTION(fun_secure)
{
  /* this function smashes all occurences of "unsafe" characters in a string.
   * "unsafe" characters are ( ) [ ] { } $ % , ; \
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
    case '%':
    case ',':
    case ';':
    case '\\':
      *p = ' ';
      break;
    }
    p++;
  }
  safe_str(args[0], buff, bp);
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
    q = args[0] + strlen(args[0]) - 1;
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
  safe_str(args[0], buff, bp);
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

  /* get rid of trailing spaces first, so we don't have to worry about
   * them later.
   */
  tp = args[0] + strlen(args[0]) - 1;
  while ((tp > args[0]) && (*tp == ' '))
    tp--;
  tp[1] = '\0';

  for (tp = args[0]; *tp == ' '; tp++)	/* skip leading spaces */
    ;

  while (*tp) {
    safe_chr(*tp, buff, bp);
    if (*tp == ' ')
      while (*tp == ' ')
	tp++;
    else
      tp++;
  }
}

/* ARGSUSED */
FUNCTION(fun_space)
{
  int s;

  if (!is_integer(args[0])) {
    safe_str(e_int, buff, bp);
    return;
  }
  s = parse_integer(args[0]);

  if (s >= BUFFER_LEN)
    s = BUFFER_LEN - 1;

  for (; s > 0; s--)
    safe_chr(' ', buff, bp);
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
      safe_str(e_int, buff, bp);
      return;
    }
    k = parse_integer(args[0]);
  } else
    k = 1;

  if (!Hasprivs(executor) || (k <= 0) || (k > 5)) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  for (; k; k--)
    safe_chr(BEEP_CHAR, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_html)
{
  safe_tag(args[0], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_tag)
{
  int i;
  safe_chr(TAG_START, buff, bp);
  safe_str(args[0], buff, bp);
  for (i = 1; i < nargs; i++) {
    safe_chr(' ', buff, bp);
    safe_str(args[i], buff, bp);
  }
  safe_chr(TAG_END, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_endtag)
{
  safe_tag_cancel(args[0], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_tagwrap)
{
  if (nargs == 2)
    safe_tag_wrap(args[0], NULL, args[1], buff, bp);
  else
    safe_tag_wrap(args[0], args[1], args[2], buff, bp);
}

#ifdef EXTENDED_ANSI
/* ARGSUSED */
FUNCTION(fun_ansi)
{
  char *s = args[0];

  while (*s) {
    switch (*s) {
    case 'h':			/* hilite */
      safe_str(ANSI_HILITE, buff, bp);
      break;
    case 'i':			/* inverse */
      safe_str(ANSI_INVERSE, buff, bp);
      break;
    case 'f':			/* flash */
      safe_str(ANSI_BLINK, buff, bp);
      break;
    case 'u':			/* underscore */
      safe_str(ANSI_UNDERSCORE, buff, bp);
      break;
    case 'n':			/* normal */
      safe_str(ANSI_NORMAL, buff, bp);
      break;
    case 'x':			/* black fg */
      safe_str(ANSI_BLACK, buff, bp);
      break;
    case 'r':			/* red fg */
      safe_str(ANSI_RED, buff, bp);
      break;
    case 'g':			/* green fg */
      safe_str(ANSI_GREEN, buff, bp);
      break;
    case 'y':			/* yellow fg */
      safe_str(ANSI_YELLOW, buff, bp);
      break;
    case 'b':			/* blue fg */
      safe_str(ANSI_BLUE, buff, bp);
      break;
    case 'm':			/* magenta fg */
      safe_str(ANSI_MAGENTA, buff, bp);
      break;
    case 'c':			/* cyan fg */
      safe_str(ANSI_CYAN, buff, bp);
      break;
    case 'w':			/* white fg */
      safe_str(ANSI_WHITE, buff, bp);
      break;
    case 'X':			/* black bg */
      safe_str(ANSI_BBLACK, buff, bp);
      break;
    case 'R':			/* red bg */
      safe_str(ANSI_BRED, buff, bp);
      break;
    case 'G':			/* green bg */
      safe_str(ANSI_BGREEN, buff, bp);
      break;
    case 'Y':			/* yellow bg */
      safe_str(ANSI_BYELLOW, buff, bp);
      break;
    case 'B':			/* blue bg */
      safe_str(ANSI_BBLUE, buff, bp);
      break;
    case 'M':			/* magenta bg */
      safe_str(ANSI_BMAGENTA, buff, bp);
      break;
    case 'C':			/* cyan bg */
      safe_str(ANSI_BCYAN, buff, bp);
      break;
    case 'W':			/* white bg */
      safe_str(ANSI_BWHITE, buff, bp);
      break;
/* #ifdef TREKMUSH */
    case 'z':           /* screen clear */
      safe_str(ANSI_SCREEN_CLEAR, buff, bp);
      break;
    case 'l':           /* lolite */
      safe_str(ANSI_LOLITE, buff, bp);
      break;
    case 't':           /* italic */
      safe_str(ANSI_ITALIC, buff, bp);
      break;
/* #endif TREKMUSH */
    }
    s++;
  }

  safe_str(args[1], buff, bp);
  safe_str(ANSI_NORMAL, buff, bp);
}
#else
/* ARGSUSED */
FUNCTION(fun_ansi)
{
  safe_str("#-1 FUNCTION DISABLED", buff, bp);
  return;
}
#endif				/* EXTENDED_ANSI */

/* ARGSUSED */
FUNCTION(fun_stripansi)
{
  /* Strips ANSI codes away from a given string of text. Starts by
   * finding the '\x' character and stripping until it hits an 'm'.
   */

  char *cp = args[0];

  while (*cp) {
    if (*cp == ESC_CHAR)
      while (*cp && *cp++ != 'm') ;
    else
      safe_chr(*cp++, buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_edit)
{
  int len;
  char *str, *f, *r;

  str = args[0];		/* complete string */
  f = args[1];			/* find this */
  r = args[2];			/* replace it with this */

  if (!*f && !*r) {		/* check for nothing, or we'll infinite loop */
    safe_str(str, buff, bp);
    return;
  }
  if (!strcmp(f, "$")) {
    /* append */
    safe_str(str, buff, bp);
    safe_str(r, buff, bp);
  } else if (!strcmp(f, "^")) {
    /* prepend */
    safe_str(r, buff, bp);
    safe_str(str, buff, bp);
  } else {
    len = strlen(f);
    while (*str) {
      if (!strncmp(str, f, len)) {
	safe_str(r, buff, bp);
	if (len)
	  str += len;
	else
	  safe_chr(*str++, buff, bp);
      } else
	safe_chr(*str++, buff, bp);
    }
    if (!*f)
      safe_str(r, buff, bp);
  }
}
