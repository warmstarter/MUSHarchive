
/* strutil.c */

/* String utilities */
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
#ifdef I_MEMORY
#include <memory.h>
#endif
#include "copyrite.h"
#include "conf.h"
#include "intrface.h"
#include "globals.h"
#include "ansi.h"
#include "pueblo.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "mymalloc.h"
#include "confmagic.h"


char *chopstr _((const char *str, int lim));
int string_prefix _((const char *string, const char *prefix));
const char *string_match _((const char *src, const char *sub));
char *strupper _((const char *s));
char *upcasestr _((char *s));
int safe_chr _((char c, char *buf, char **bufp));
int safe_copy_str _((char *c, char *buff, char **bp, int maxlen));
char *skip_space _((const char *s));
char *seek_char _((const char *s, char c));
unsigned char *u_strdup _((const unsigned char *s));
int u_strlen _((const unsigned char *s));
unsigned char *u_strcpy _((unsigned char *target, const unsigned char *source));
char *replace_string _((const char *old, const char *newbit, const char *string));
char *trim_space_sep _((char *str, char sep));
char *next_token _((char *str, char sep));
char *split_token _((char **sp, char sep));
int do_wordcount _((char *str, char sep));
int minmatch _((const char *str, const char *target, int min));
int ansi_strlen _((char *string));

/* Return the string chopped at lim characters */
char *
chopstr(str, lim)
    const char *str;
    int lim;
{
  static char tbuf1[BUFFER_LEN];
  if (strlen(str) <= (Size_t) lim)
    return (char *) str;
  strncpy(tbuf1, str, lim);
  tbuf1[lim] = '\0';
  return tbuf1;
}


#ifndef HAS_STRCASECMP
int strcasecmp _((const char *s1, const char *s2));
int strncasecmp _((const char *s1, const char *s2, size_t n));

int
strcasecmp(s1, s2)
    const char *s1;
    const char *s2;
{
  while (*s1 && *s2 && DOWNCASE(*s1) == DOWNCASE(*s2))
    s1++, s2++;

  return (DOWNCASE(*s1) - DOWNCASE(*s2));
}

int
strncasecmp(s1, s2, n)
    const char *s1;
    const char *s2;
    size_t n;
{
  for (; 0 < n; ++s1, ++s2, --n)
    if (DOWNCASE(*s1) != DOWNCASE(*s2))
      return DOWNCASE(*s1) - DOWNCASE(*s2);
    else if (*s1 == 0)
      return 0;
  return 0;

}
#endif

int
string_prefix(string, prefix)
    const char *string;
    const char *prefix;
{
  if (!string || !prefix)
    return 0;
  while (*string && *prefix && DOWNCASE(*string) == DOWNCASE(*prefix))
    string++, prefix++;
  return *prefix == '\0';
}

/* accepts only nonempty matches starting at the beginning of a word */
const char *
string_match(src, sub)
    const char *src;
    const char *sub;
{
  if (!src || !sub)
    return 0;

  if (*sub != '\0') {
    while (*src) {
      if (string_prefix(src, sub))
	return src;
      /* else scan to beginning of next word */
      while (*src && (isalpha(*src) || isdigit(*src)))
	src++;
      while (*src && !isalpha(*src) && !isdigit(*src))
	src++;
    }
  }
  return 0;
}

char *
strupper(s)
    const char *s;
{
  static char buf1[BUFFER_LEN];
  char *p;

  if (!s || !*s) {
    strcpy(buf1, "");
    return buf1;
  }
  strcpy(buf1, s);
  for (p = buf1; *p; p++)
    *p = UPCASE(*p);
  return buf1;
}

char *
upcasestr(s)
    char *s;
{
  /* modifies a string in-place to be upper-case */

  char *p;
  for (p = s; p && *p; p++)
    *p = UPCASE(*p);
  return s;
}

/* safe_chr and safe_str are essentially straight out of the 2.0 code,
 * but safe_chr now returns 1 on FAILURE, not success, to match safe_copy_str
 */
#ifdef CAN_NEWSTYLE
int
safe_chr(char c, char *buf, char **bufp)
#else
int
safe_chr(c, buf, bufp)
    char c;
    char *buf;
    char **bufp;
#endif
{
  /* adds a character to a string, being careful not to overflow buffer */

  if ((*bufp - buf >= BUFFER_LEN - 1))
    return 1;

  *(*bufp)++ = c;
  return 0;
}

int
safe_copy_str(c, buff, bp, maxlen)
    char *c;
    char *buff;
    char **bp;
    int maxlen;
{
  /* copies a string into a buffer, making sure there's no overflow. */
  int len, blen, clen;

  if (!c)
    return 0;

  len = strlen(c);
  blen = *bp - buff;

  if (blen > maxlen)
    return len;

  if ((len + blen) <= maxlen)
    clen = len;
  else
    clen = maxlen - blen;

  memcpy(*bp, c, clen);
  *bp += clen;

  return len - clen;
}

/* skip_space and seek_char are essentially right out of the 2.0 code */

char *
skip_space(s)
    const char *s;
{
  /* returns pointer to the next non-space char in s, or NULL if s == NULL
   * or *s == NULL or s has only spaces.
   */

  char *c = (char *) s;
  while (c && *c && isspace(*c))
    c++;
  return c;
}

#ifdef CAN_NEWSTYLE
char *
seek_char(const char *s, char c)
#else
char *
seek_char(s, c)
    const char *s;
    char c;
#endif
{
  /* similar to strchr(). returns a pointer to the next char in s which
   * matches c, or a pointer to the terminating null at the end of s.
   */

  char *p = (char *) s;
  while (p && *p && (*p != c))
    p++;
  return p;
}

int
u_strlen(s)
    const unsigned char *s;
{
  return strlen((char *) s);
}

unsigned char *
u_strcpy(target, source)
    unsigned char *target;
    const unsigned char *source;
{
  return (unsigned char *) strcpy((char *) target, (char *) source);
}

char *
replace_string(old, newbit, string)
    const char *old;
    const char *newbit;
    const char *string;
{
  /* another 2.0 function: replaces string "old" with string "newbit".
   * The result returned by this must be freed.
   */

  char *result, *r, *s;
  int len;

  if (!string)
    return NULL;

  s = (char *) string;
  len = strlen(old);
  r = result = (char *) malloc(BUFFER_LEN + 1);
#ifdef MEM_CHECK
  add_check("replace_string.buff");
#endif

  while (*s) {

    /* copy up to the next occurence of first char of old */
    while (*s && *s != *old) {
      safe_chr(*s, result, &r);
      s++;
    }

    /* if we've really found  old, append newbit to the result and
     * move past the occurence of old. Else, copy the char and
     * continue.
     */
    if (*s) {
      if (!strncmp(old, s, len)) {
	safe_copy_str((char *) newbit, result, &r, BUFFER_LEN);
	s += len;
      } else {
	safe_chr(*s, result, &r);
	s++;
      }
    }
  }

  *r = '\0';
  return result;
}

#ifdef CAN_NEWSTYLE
char *
trim_space_sep(char *str, char sep)
#else
char *
trim_space_sep(str, sep)
    char *str;
    char sep;
#endif
{
  /* Trim leading and trailing spaces if we've got a space separator. */

  char *p;

  if (sep != ' ')
    return str;
  while (*str && (*str == ' '))
    str++;
  for (p = str; *p; p++) ;
  for (p--; (*p == ' ') && (p > str); p--) ;
  p++;
  *p = '\0';
  return str;
}

#ifdef CAN_NEWSTYLE
char *
next_token(char *str, char sep)
#else
char *
next_token(str, sep)
    char *str;
    char sep;
#endif
{
  /* move pointer to start of the next token */

  while (*str && (*str != sep))
    str++;
  if (!*str)
    return NULL;
  str++;
  if (sep == ' ') {
    while (*str == sep)
      str++;
  }
  return str;
}

#ifdef CAN_NEWSTYLE
char *
split_token(char **sp, char sep)
#else
char *
split_token(sp, sep)
    char **sp;
    char sep;
#endif
{
  /* Get next token from string as a null-terminated string, depending
   * on the separator character. This destructively modifies the string.
   * Code from 2.0.
   */

  char *str, *save;

  save = str = *sp;
  if (!str) {
    *sp = NULL;
    return NULL;
  }
  while (*str && (*str != sep))
    str++;
  if (*str) {
    *str++ = '\0';
    if (sep == ' ') {
      while (*str == sep)
	str++;
    }
  } else {
    str = NULL;
  }
  *sp = str;
  return save;
}

#ifdef CAN_NEWSTYLE
int
do_wordcount(char *str, char sep)
#else
int
do_wordcount(str, sep)
    char *str;
    char sep;
#endif
{
  /* count the number of words in a string */
  int n;

  if (!*str)
    return 0;
  for (n = 0; str; str = next_token(str, sep), n++) ;
  return n;
}

/* Pretty much straight from Tiny 2.2, we see if str and target
 * match in the first min chars
 */
int
minmatch(str, target, min)
    const char *str, *target;
    int min;
{
  while (*str && *target && (DOWNCASE(*str) == DOWNCASE(*target))) {
    str++;
    target++;
    min--;
  }
  if (*str)
    return 0;
  if (!*target)
    return 1;
  return ((min <= 0) ? 1 : 0);
}

/* Strlen that ignores ansi sequences */

int
ansi_strlen(string)
    char *string;
{
  int i = 0;
  char *p;
  if (!ANSI_JUSTIFY)
    return strlen(string);
  p = string;
  if (!p)
    return 0;
  while (*p) {
    if (*p == ESC_CHAR) {
      while ((*p) && (*p != 'm'))
	p++;
    } else if (*p == TAG_START) {
      while ((*p) && (*p != TAG_END))
	p++;
    } else {
      i++;
    }
    p++;
  }
  return i;
}
