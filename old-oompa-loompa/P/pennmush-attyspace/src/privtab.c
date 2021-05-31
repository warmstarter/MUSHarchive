/* privtab.c */
/* Functions to work with privilege tables */

#include "copyrite.h"
#include "config.h"
#include <ctype.h>
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include "conf.h"
#include "privtab.h"
#include "externs.h"
#include "confmagic.h"


/* Given a privs table, a string, and an original set of privileges,
 * return a modified set of privileges by applying the privs in the
 * string to the original set of privileges
 */
int
string_to_privs(table, str, origprivs)
    PRIV *table;
    const char *str;
    long int origprivs;
{
  PRIV *c;
  long int yes = 0;
  long int no = 0;
  char *p, *r;
  char tbuf1[BUFFER_LEN];
  int not;
  int words = 0;

  if (!str || !*str)
    return origprivs;
  strcpy(tbuf1, str);
  r = trim_space_sep(tbuf1, ' ');
  while ((p = split_token(&r, ' '))) {
    words++;
    not = 0;
    if (*p == '!') {
      not = 1;
      if (!*++p)
	continue;
    }
    for (c = table; c->name; c++) {
      if (string_prefix(c->name, p)) {
	if (not)
	  no |= c->bits_to_set;
	else
	  yes |= c->bits_to_set;
	break;
      }
    }
  }
  /* If we made no changes, and were given one word, 
   * we probably were given letters instead */
  if (!no && !yes && (words == 1))
    return letter_to_privs(table, str, origprivs);
  return ((origprivs | yes) & ~no);
}

/* Given a privs table, a string of letters, and an original set of privileges,
 * return a modified set of privileges by applying the privs in the
 * letters to the original set of privileges
 */
int
letter_to_privs(table, str, origprivs)
    PRIV *table;
    const char *str;
    long int origprivs;
{
  PRIV *c;
  long int yes = 0, no = 0;
  const char *p;
  int not;

  if (!str || !*str)
    return origprivs;
  p = str;
  for (c = table; c->name && *p; c++) {
    not = 0;
    if (*p == '!') {
      not = 1;
      if (!*++p)
	break;
    }
    if (c->letter == *p) {
      if (not)
	no = c->bits_to_set;
      else
	yes = c->bits_to_set;
    }
    p++;
  }
  return ((origprivs | yes) & ~no);
}

/* Given a table and a bitmask, return a privs string (static allocation) */
const char *
privs_to_string(table, privs)
    PRIV *table;
    int privs;
{
  PRIV *c;
  static char buf[BUFFER_LEN];
  char *bp;

  bp = buf;
  for (c = table; c->name; c++) {
    if (privs & c->bits_to_show) {
      if (bp != buf)
	safe_chr(' ', buf, &bp);
      safe_str(c->name, buf, &bp);
      privs &= ~c->bits_to_set;
    }
  }
  *bp = '\0';
  return buf;
}


/* Given a table and a bitmask, return a privs letter string 
 * (static allocation) */
const char *
privs_to_letters(table, privs)
    PRIV *table;
    int privs;
{
  PRIV *c;
  static char buf[BUFFER_LEN];
  char *bp;

  bp = buf;
  for (c = table; c->name; c++) {
    if (privs & c->bits_to_show) {
      safe_chr(c->letter, buf, &bp);
      privs &= ~c->bits_to_set;
    }
  }
  *bp = '\0';
  return buf;
}
