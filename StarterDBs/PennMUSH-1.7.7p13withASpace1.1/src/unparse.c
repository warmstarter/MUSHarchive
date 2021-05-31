/* unparse.c */
#include "copyrite.h"
#include "config.h"

#include <string.h>
#include "conf.h"
#include "mushdb.h"
#include "dbdefs.h"
#include "flags.h"
#include "lock.h"
#include "externs.h"
#include "attrib.h"
#include "ansi.h"
#include "pueblo.h"
#include "parse.h"
#include "confmagic.h"

/* Hack added by Thorvald for object_header Pueblo */
static int couldunparse;

const char *
unparse_object(player, loc)
    dbref player;
    dbref loc;
{
  static PUEBLOBUFF;
  const char *result;
  result = real_unparse(player, loc, 0, 0, 0);
  if (couldunparse) {
    PUSE;
    tag_wrap("A", tprintf("XCH_CMD=\"examine #%d\"", loc), result);
    PEND;
    return pbuff;
  } else {
    return result;
  }
}

const char *
unparse_object_myopic(player, loc)
    dbref player;
    dbref loc;
{
  static PUEBLOBUFF;
  const char *result;
  result = real_unparse(player, loc, 1, 0, 1);
  if (couldunparse) {
    PUSE;
    tag_wrap("A", tprintf("XCH_CMD=\"examine #%d\"", loc), result);
    PEND;
    return pbuff;
  } else {
    return result;
  }
}

/* Like unparse_object, but tell real_unparse to use @NAMEFORMAT if present
 * This should only be used if we're looking at our container, to prevent
 * confusion in matching, since you can't match containers with their
 * names anyway, but only with 'here'.
 */
const char *
unparse_room(player, loc)
    dbref player;
    dbref loc;
{
  static PUEBLOBUFF;
  const char *result;
  result = real_unparse(player, loc, 1, 1, 1);
  if (couldunparse) {
    PUSE;
    tag_wrap("A", tprintf("XCH_CMD=\"examine #%d\"", loc), result);
    PEND;
    return pbuff;
  } else {
    return result;
  }
}

const char *
real_unparse(dbref player, dbref loc, int obey_myopic, int use_nameformat,
	     int use_nameaccent)
{
  static char buf[BUFFER_LEN], *bp;
  static char tbuf1[BUFFER_LEN];
  char *p;
  int got_nameformat = 0;

  couldunparse = 0;
  if (!(GoodObject(loc) || (loc == NOTHING) || (loc == AMBIGUOUS) ||
	(loc == HOME)))
    return T("*NOTHING*");
  switch (loc) {
  case NOTHING:
    return T("*NOTHING*");
  case AMBIGUOUS:
    return T("*VARIABLE*");
  case HOME:
    return T("*HOME*");
  default:
    if (use_nameformat && nameformat(player, loc, buf)) {
      strcpy(tbuf1, buf);
      got_nameformat = 1;
    } else {
      /* Not using @nameformat or couldn't get one */
      if (use_nameaccent)
	strcpy(tbuf1, accented_name(loc));
      else
	strcpy(tbuf1, Name(loc));
    }
    if (IsExit(loc) && obey_myopic) {
      if ((p = strchr(tbuf1, ';')))
	*p = '\0';
    }
    if ((Can_Examine(player, loc) || can_link_to(player, loc) ||
	 JumpOk(loc) || ChownOk(loc) || DestOk(loc)) &&
	(!Myopic(player) || !obey_myopic) &&
	!(use_nameformat && got_nameformat)) {
      /* show everything */
      if (SUPPORT_PUEBLO)
	couldunparse = 1;
      bp = buf;
      if (ANSI_NAMES && ShowAnsi(player) && !got_nameformat)
	safe_format(buf, &bp, "%s%s%s(#%d%s)", ANSI_HILITE, tbuf1,
		    ANSI_NORMAL, loc, unparse_flags(loc, player));
      else
	safe_format(buf, &bp, "%s(#%d%s)", tbuf1, loc,
		    unparse_flags(loc, player));
      *bp = '\0';
      return buf;
    } else {
      /* show only the name */
      if (ANSI_NAMES && ShowAnsi(player) && !got_nameformat) {
	bp = buf;
	safe_format(buf, &bp, "%s%s%s", ANSI_HILITE, tbuf1, ANSI_NORMAL);
	*bp = '\0';
	return buf;
      } else
	return tbuf1;
    }
  }
}

/* The name of loc as seen by a player inside it if nameformat is set */
/* This function needs to avoid using a static buffer, so pass in 
 * a pointer to an allocated BUFFER_LEN array
 */
int
nameformat(player, loc, tbuf1)
    dbref player;
    dbref loc;
    char *tbuf1;
{
  ATTR *a;
  char *wsave[10], *rsave[NUMQ];
  char *arg, *bp;
  char const *sp, *save;

  int j;
  a = atr_get(loc, "NAMEFORMAT");
  if (a) {
    arg = (char *) mush_malloc(BUFFER_LEN, "string");
    if (!arg)
      panic("Unable to allocate memory in nameformat");
    save_global_regs("nameformat", rsave);
    for (j = 0; j < 10; j++) {
      wsave[j] = wenv[j];
      wenv[j] = NULL;
    }
    for (j = 0; j < NUMQ; j++)
      renv[j][0] = '\0';
    strcpy(arg, unparse_dbref(loc));
    wenv[0] = arg;
    sp = save = safe_uncompress(a->value);
    bp = tbuf1;
    process_expression(tbuf1, &bp, &sp, loc, player, player,
		       PE_DEFAULT, PT_DEFAULT, NULL);
    *bp = '\0';
    free((Malloc_t) save);
    for (j = 0; j < 10; j++) {
      wenv[j] = wsave[j];
    }
    restore_global_regs("nameformat", rsave);
    mush_free((Malloc_t) arg, "string");
    return 1;
  } else {
    /* No @nameformat attribute */
    return 0;
  }
}

/* Give a string representation of a dbref */
char *
unparse_dbref(num)
    dbref num;
{
  /* Not BUFFER_LEN, but no dbref will come near this long */
  static char str[SBUF_LEN];
  char *strp;

  strp = str;
  safe_dbref(num, str, &strp);
  *strp = '\0';
  return str;
}

/* Give a string representation of an integer */
char *
unparse_integer(num)
    int num;
{
  static char str[SBUF_LEN];
  char *strp;

  strp = str;
  safe_integer_sbuf(num, str, &strp);
  *strp = '\0';
  return str;
}

/* Give a string representation of an unsigned integer */
char *
unparse_uinteger(num)
    unsigned int num;
{
  static char str[16];

  sprintf(str, "%u", num);
  return str;
}


/* Give a string representation of a number */
char *
unparse_number(num)
    NVAL num;
{
  static char str[100];		/* Should be large enough for even the HUGE floats */
  char *p;
  sprintf(str, "%.*f", FLOAT_PRECISION, num);

  if ((p = strchr(str, '.'))) {
    p += strlen(p);
    while (p[-1] == '0')
      p--;
    if (p[-1] == '.')
      p--;
    *p = '\0';
  }
  return str;
}

const char *
accented_name(dbref thing)
{
  ATTR *na;
  static char fbuf[BUFFER_LEN];

  na = atr_get(thing, "NAMEACCENT");
  if (!na)
    return Name(thing);
  else {
    char tbuf[BUFFER_LEN];
    char *bp = fbuf;
    size_t len;

    strcpy(tbuf, uncompress(AL_STR(na)));

    len = strlen(Name(thing));

    if (len != strlen(tbuf))
      return Name(thing);

    safe_accent(Name(thing), tbuf, len, fbuf, &bp);
    *bp = '\0';

    return fbuf;
  }
}
