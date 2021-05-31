/* unparse.c */
#include "copyrite.h"
#include "config.h"

#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include "conf.h"
#include "mushdb.h"
#include "externs.h"
#include "intrface.h"
#ifdef EXTENDED_ANSI
#include "ansi.h"
#endif
#include "pueblo.h"
#include "parse.h"
#include "confmagic.h"


const char *real_unparse _((dbref player, dbref loc, int obey_myopic));
static void unparse_boolexp1 _((dbref player, struct boolexp * b, boolexp_type outer_type, int flag));
char *unparse_boolexp _((dbref player, struct boolexp * b, int flag));

/* Hack added by Thorvald for object_header Pueblo */
static int couldunparse;

const char *
object_header(player, loc)
    dbref player;
    dbref loc;
{
  static PUEBLOBUFF;
  const char *result;
  result = real_unparse(player, loc, 0);
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
unparse_object(player, loc)
    dbref player;
    dbref loc;
{
  static PUEBLOBUFF;
  const char *result;
  result = real_unparse(player, loc, 1);
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
real_unparse(player, loc, obey_myopic)
    dbref player;
    dbref loc;
    int obey_myopic;
{
  static char buf[BUFFER_LEN];
  static char tbuf1[BUFFER_LEN];
  char *p;

  couldunparse = 0;
  if (!(GoodObject(loc) || (loc == NOTHING) || (loc == AMBIGUOUS) ||
	(loc == HOME)))
    return "*NOTHING*";
  switch (loc) {
  case NOTHING:
    return "*NOTHING*";
  case AMBIGUOUS:
    return "*VARIABLE*";
  case HOME:
    return "*HOME*";
  default:
    Access(loc);
    strcpy(tbuf1, Name(loc));
    if (IsExit(loc) && obey_myopic) {
      if ((p = strchr(tbuf1, ';')))
	*p = '\0';
    }
    if ((Can_Examine(player, loc) || can_link_to(player, loc) ||
	 JumpOk(loc) || ChownOk(loc) || DestOk(loc)) &&
	(!Myopic(player) || !obey_myopic)) {
      /* show everything */
      if (SUPPORT_PUEBLO)
	couldunparse = 1;
#ifdef EXTENDED_ANSI
      if (ANSI_NAMES && ShowAnsi(player))
#ifdef TREKMUSH
	if (!Ic(loc) && Typeof(loc) == TYPE_PLAYER) {
	  sprintf(buf, "%s%s%s(#%d%s) <OOC>", ANSI_HILITE, tbuf1,
	    ANSI_NORMAL, loc, unparse_flags(loc, player));
	} else
#endif /* TREKMUSH */
	sprintf(buf, "%s%s%s(#%d%s)", ANSI_HILITE, tbuf1,
		ANSI_NORMAL, loc, unparse_flags(loc, player));
      else
#endif
#ifdef TREKMUSH
	if (!Ic(loc) && Typeof(loc) == TYPE_PLAYER) {
	  sprintf(buf, "%s(#%d%s) <OOC>", tbuf1, loc,
	    unparse_flags(loc, player));
	} else
#endif /* TREKMUSH */
	sprintf(buf, "%s(#%d%s)", tbuf1, loc,
		unparse_flags(loc, player));
      return buf;
    } else {
      /* show only the name */
#ifdef EXTENDED_ANSI
      if (ANSI_NAMES && ShowAnsi(player)) {
#ifdef TREKMUSH
	if (!Ic(loc) && Typeof(loc) == TYPE_PLAYER) {
	  sprintf(buf, "%s%s%s <OOC>", ANSI_HILITE, tbuf1, ANSI_NORMAL);
	} else
#endif /* TREKMUSH */
	sprintf(buf, "%s%s%s", ANSI_HILITE, tbuf1, ANSI_NORMAL);
	return buf;
      } else
#endif
	return tbuf1;
    }
  }
}

static char boolexp_buf[BUFFER_LEN];
static char *buftop;

#ifdef CAN_NEWSTYLE
static void
unparse_boolexp1(dbref player, struct boolexp *b, boolexp_type outer_type,
		 int flag)
#else
static void
unparse_boolexp1(player, b, outer_type, flag)
    dbref player;
    struct boolexp *b;
    boolexp_type outer_type;
    int flag;
#endif
/*  0 is full unparse, 1 is numbers-only */
{
  char tbuf1[BUFFER_LEN];

  if (b == TRUE_BOOLEXP) {
    safe_str((char *) "*UNLOCKED*", boolexp_buf, &buftop);
    return;
  } else {
    switch (b->type) {
    case BOOLEXP_AND:
      if (outer_type == BOOLEXP_NOT) {
	safe_chr('(', boolexp_buf, &buftop);
      }
      unparse_boolexp1(player, b->sub1, b->type, flag);
      safe_chr(AND_TOKEN, boolexp_buf, &buftop);
      unparse_boolexp1(player, b->sub2, b->type, flag);
      if (outer_type == BOOLEXP_NOT) {
	safe_chr(')', boolexp_buf, &buftop);
      }
      break;
    case BOOLEXP_OR:
      if (outer_type == BOOLEXP_NOT || outer_type == BOOLEXP_AND) {
	safe_chr('(', boolexp_buf, &buftop);
      }
      unparse_boolexp1(player, b->sub1, b->type, flag);
      safe_chr(OR_TOKEN, boolexp_buf, &buftop);
      unparse_boolexp1(player, b->sub2, b->type, flag);
      if (outer_type == BOOLEXP_NOT || outer_type == BOOLEXP_AND) {
	safe_chr(')', boolexp_buf, &buftop);
      }
      break;
    case BOOLEXP_IND:
      safe_chr(AT_TOKEN, boolexp_buf, &buftop);
      unparse_boolexp1(player, b->sub1, b->type, flag);
      break;
    case BOOLEXP_IS:
      safe_chr(IS_TOKEN, boolexp_buf, &buftop);
      unparse_boolexp1(player, b->sub1, b->type, flag);
      break;
    case BOOLEXP_CARRY:
      safe_chr(IN_TOKEN, boolexp_buf, &buftop);
      unparse_boolexp1(player, b->sub1, b->type, flag);
      break;
    case BOOLEXP_OWNER:
      safe_chr(OWNER_TOKEN, boolexp_buf, &buftop);
      unparse_boolexp1(player, b->sub1, b->type, flag);
      break;
    case BOOLEXP_NOT:
      safe_chr(NOT_TOKEN, boolexp_buf, &buftop);
      unparse_boolexp1(player, b->sub1, b->type, flag);
      break;
    case BOOLEXP_CONST:
      if (flag) {
	sprintf(tbuf1, "#%d", b->thing);
	safe_str(tbuf1, boolexp_buf, &buftop);
      } else
	safe_str(unparse_object(player, b->thing), boolexp_buf, &buftop);
      break;
    case BOOLEXP_ATR:
      sprintf(tbuf1, "%s:%s", b->atr_lock->name,
	      uncompress(b->atr_lock->text));
      safe_str(tbuf1, boolexp_buf, &buftop);
      break;
    case BOOLEXP_EVAL:
      sprintf(tbuf1, "%s/%s", b->atr_lock->name,
	      uncompress(b->atr_lock->text));
      safe_str(tbuf1, boolexp_buf, &buftop);
      break;
    default:
      safe_str((char *) "Bad boolexp type!", boolexp_buf, &buftop);
      fprintf(stderr, "ERROR: unparse_boolexp1 bad boolexp type on #%d\n",
	      player);
      break;
    }
  }
}

char *
unparse_boolexp(player, b, flag)
    dbref player;
    struct boolexp *b;
    int flag;			/*  0 is full unparse, 1 is numbers-only */
{
  buftop = boolexp_buf;
  unparse_boolexp1(player, b, BOOLEXP_CONST, flag);	/* no outer type */
  *buftop++ = '\0';
  return boolexp_buf;
}

/* Give a string representation of a dbref */
char *
unparse_dbref(num)
    dbref num;
{
  static char str[16];

  sprintf(str, "#%d", num);
  return str;
}

/* Give a string representation of an integer */
char *
unparse_integer(num)
    int num;
{
  static char str[16];

  sprintf(str, "%d", num);
  return str;
}

/* Give a string representation of a number */
char *
unparse_number(num)
    NVAL num;
{
  static char str[100];		/* Should be large enough for even the HUGE floats */

#ifdef FLOATING_POINTS
  char *p;
  sprintf(str, "%.6f", num);
  if ((p = (char *) index(str, '.'))) {
    p += strlen(p);
    while (p[-1] == '0')
      p--;
    if (p[-1] == '.')
      p--;
    *p = '\0';
  }
#else
  sprintf(str, "%d", num);
#endif				/* FLOATING_POINTS */
  return str;
}
