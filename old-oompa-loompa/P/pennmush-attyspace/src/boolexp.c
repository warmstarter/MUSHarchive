/* boolexp.c */

#include "copyrite.h"
#include "config.h"

#include <ctype.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif

#include "conf.h"
#include "mushdb.h"
#include "match.h"
#include "externs.h"
#include "lock.h"
#include "intrface.h"
#include "parse.h"
#include "confmagic.h"

int sizeof_boolexp _((struct boolexp * b));
int eval_boolexp _((dbref player, struct boolexp * b, dbref target, int nrecurs, lock_type ltype));
static void skip_whitespace _((void));
static struct boolexp *test_atr _((char *s, char c));
static struct boolexp *parse_boolexp_L _((void));
static struct boolexp *parse_boolexp_O _((void));
static struct boolexp *parse_boolexp_C _((void));
static struct boolexp *parse_boolexp_I _((void));
static struct boolexp *parse_boolexp_A _((void));
static struct boolexp *parse_boolexp_F _((void));
static struct boolexp *parse_boolexp_T _((void));
static struct boolexp *parse_boolexp_E _((void));
struct boolexp *parse_boolexp _((dbref player, const char *buf));
static int check_attrib_lock _((dbref player, dbref target, const char *atrname, const char *str));

int
sizeof_boolexp(b)
    struct boolexp *b;
{
  if (!b || b == TRUE_BOOLEXP)
    return 0;
  switch (b->type) {
  case BOOLEXP_CONST:
    return sizeof(struct boolexp);
  case BOOLEXP_IND:
  case BOOLEXP_NOT:
  case BOOLEXP_IS:
  case BOOLEXP_CARRY:
  case BOOLEXP_OWNER:
    return sizeof(struct boolexp) + sizeof_boolexp(b->sub1);
  case BOOLEXP_AND:
  case BOOLEXP_OR:
    return sizeof(struct boolexp) +
     sizeof_boolexp(b->sub1) + sizeof_boolexp(b->sub2);
  case BOOLEXP_ATR:
  case BOOLEXP_EVAL:
    return sizeof(struct boolexp) + sizeof(struct boolatr) +
     strlen(b->atr_lock->name) + 1 +
#ifdef macintosh
     strlen((const char *)(*(b->atr_lock->text))) + 1;
#else
     strlen(b->atr_lock->text) + 1;
#endif
  }
  /* Broken lock */
  return sizeof(struct boolexp);
}

int
eval_boolexp(player, b, target, nrecurs, ltype)
    dbref player;		/* The player trying to pass */
    struct boolexp *b;		/* The boolexp */
    dbref target;		/* The object with the lock */
    int nrecurs;		/* Recursion depth */
    lock_type ltype;
{
  ATTR *a;
  char tbuf1[BUFFER_LEN], tbuf2[BUFFER_LEN];

  if (!GoodObject(player))
    return 0;

  if (nrecurs > MAX_DEPTH) {
    notify(player, "Sorry, broken lock!");
    return 0;
  }
  if (!b)
    return 1;
  if (b == TRUE_BOOLEXP) {
    return 1;
  } else {
    switch (b->type) {
    case BOOLEXP_IND:
      if (b->sub1->type != BOOLEXP_CONST)
	return 0;
      /* We only allow evaluation of indirect locks if target can run
       * the lock on the referenced object.
       */
      if (!Can_Read_Lock(target, b->sub1->thing, ltype))
	return 0;
      nrecurs++;
      return (eval_boolexp(player, getlock(b->sub1->thing, ltype),
			   b->sub1->thing, nrecurs, ltype));
    case BOOLEXP_AND:
      return (eval_boolexp(player, b->sub1, target, nrecurs, ltype)
	      && eval_boolexp(player, b->sub2, target, nrecurs, ltype));
    case BOOLEXP_OR:
      return (eval_boolexp(player, b->sub1, target, nrecurs, ltype)
	      || eval_boolexp(player, b->sub2, target, nrecurs, ltype));
    case BOOLEXP_NOT:
      return !eval_boolexp(player, b->sub1, target, nrecurs, ltype);
    case BOOLEXP_CONST:
      return (b->thing == player
	      || member(b->thing, Contents(player)));
    case BOOLEXP_IS:
      return (b->sub1->thing == player);
    case BOOLEXP_CARRY:
      return (member(b->sub1->thing, Contents(player)));
    case BOOLEXP_OWNER:
      return (Owner(b->sub1->thing) == Owner(player));
    case BOOLEXP_ATR:
      a = atr_complete_match(player, b->atr_lock->name, target);
      if (!a)
	return 0;
      strcpy(tbuf1, uncompress(b->atr_lock->text));
      strcpy(tbuf2, uncompress(a->value));
      return local_wild_match(tbuf1, tbuf2);
    case BOOLEXP_EVAL:
      strcpy(tbuf1, uncompress(b->atr_lock->text));
      return check_attrib_lock(player, target, b->atr_lock->name, tbuf1);
    default:
      do_log(LT_ERR, 0, 0, "Bad boolexp type %d in object #%d",
	     b->type, b->thing);
      report();
      return 0;
    }				/* switch */
    /* should never be reached */
    do_log(LT_ERR, 0, 0, "Broken lock type %s in object called by #%d",
	   ltype, player);
    return 0;
  }				/* else */
}

/* If the parser returns TRUE_BOOLEXP, you lose */
/* TRUE_BOOLEXP cannot be typed in by the user; use @unlock instead */
static const char *parsebuf;
static dbref parse_player;

static void
skip_whitespace()
{
  while (*parsebuf && isspace(*parsebuf))
    parsebuf++;
}


#ifdef CAN_NEWSTYLE
static struct boolexp *
test_atr(char *s, char c)
#else
static struct boolexp *
test_atr(s, c)
    char *s;
    char c;
#endif
{
  struct boolexp *b;
  char tbuf1[BUFFER_LEN];

  strcpy(tbuf1, s);
  for (s = tbuf1; *s && (*s != c); s++) ;
  if (!*s)
    return (0);
  *s++ = 0;
  if (strlen(tbuf1) == 0)
    return (0);
  b = alloc_bool();

  if (c == ':')
    b->type = BOOLEXP_ATR;
  else
    b->type = BOOLEXP_EVAL;

  /* !!! Possible portability problem!!!!! */
  b->atr_lock = alloc_atr(tbuf1, s);
  return (b);
}

/* L -> (E); L -> object identifier */
static struct boolexp *
parse_boolexp_L()
{
  struct boolexp *b;
  char *p;
  char tbuf1[BUFFER_LEN];

  skip_whitespace();
  switch (*parsebuf) {
  case '(':
    parsebuf++;
    b = parse_boolexp_E();
    skip_whitespace();
    if (b == TRUE_BOOLEXP || *parsebuf++ != ')') {
      free_boolexp(b);
      return TRUE_BOOLEXP;
    } else {
      return b;
    }
    /* break; */
  default:
    /* must have hit an object ref */
    /* load the name into our buffer */
    p = tbuf1;
    while (*parsebuf
	   && *parsebuf != AND_TOKEN
	   && *parsebuf != OR_TOKEN
	   && *parsebuf != ')') {
      *p++ = *parsebuf++;
    }
    /* strip trailing whitespace */
    *p-- = '\0';
    while (isspace(*p))
      *p-- = '\0';

    /* check for an attribute */
    b = test_atr(tbuf1, ':');
    if (b)
      return (b);

    /* check for an eval */
    b = test_atr(tbuf1, '/');
    if (b)
      return b;

    b = alloc_bool();
    b->type = BOOLEXP_CONST;

    /* do the match */
    b->thing = match_result(parse_player, tbuf1, TYPE_THING, MAT_EVERYTHING);

    if (b->thing == NOTHING) {
      notify(parse_player,
	     tprintf("I don't see %s here.", tbuf1));
      free_bool(b);
      return TRUE_BOOLEXP;
    } else if (b->thing == AMBIGUOUS) {
      notify(parse_player,
	     tprintf("I don't know which %s you mean!", tbuf1));
      free_bool(b);
      return TRUE_BOOLEXP;
    } else {
      return b;
    }
    /* break */
  }
}

/* O -> $Identifier ; O -> L */
static struct boolexp *
parse_boolexp_O()
{
  struct boolexp *b2;
  skip_whitespace();
  if (*parsebuf == OWNER_TOKEN) {
    parsebuf++;
    b2 = alloc_bool();
    b2->type = BOOLEXP_OWNER;
    if (((b2->sub1 = parse_boolexp_L()) == TRUE_BOOLEXP) ||
	(b2->sub1->type != BOOLEXP_CONST)) {
      free_boolexp(b2);
      return (TRUE_BOOLEXP);
    } else
      return (b2);
  }
  return (parse_boolexp_L());
}

/* C -> +Identifier ; C -> O */
static struct boolexp *
parse_boolexp_C()
{
  struct boolexp *b2;
  skip_whitespace();
  if (*parsebuf == IN_TOKEN) {
    parsebuf++;
    b2 = alloc_bool();
    b2->type = BOOLEXP_CARRY;
    if (((b2->sub1 = parse_boolexp_L()) == TRUE_BOOLEXP) ||
	(b2->sub1->type != BOOLEXP_CONST)) {
      free_boolexp(b2);
      return (TRUE_BOOLEXP);
    } else
      return (b2);
  }
  return (parse_boolexp_O());
}

/* I -> =Identifier ; I -> C */
static struct boolexp *
parse_boolexp_I()
{
  struct boolexp *b2;
  skip_whitespace();
  if (*parsebuf == IS_TOKEN) {
    parsebuf++;
    b2 = alloc_bool();
    b2->type = BOOLEXP_IS;
    if (((b2->sub1 = parse_boolexp_L()) == TRUE_BOOLEXP) ||
	(b2->sub1->type != BOOLEXP_CONST)) {
      free_boolexp(b2);
      return (TRUE_BOOLEXP);
    } else
      return (b2);
  }
  return (parse_boolexp_C());
}

/* A -> @L; A -> I */
static struct boolexp *
parse_boolexp_A()
{
  struct boolexp *b2;
  skip_whitespace();
  if (*parsebuf == AT_TOKEN) {
    parsebuf++;
    b2 = alloc_bool();
    b2->type = BOOLEXP_IND;
    if (((b2->sub1 = parse_boolexp_L()) == TRUE_BOOLEXP) ||
	(b2->sub1->type != BOOLEXP_CONST)) {
      free_boolexp(b2);
      return (TRUE_BOOLEXP);
    } else
      return (b2);
  }
  return (parse_boolexp_I());
}

/* F -> !F;F -> A */
static struct boolexp *
parse_boolexp_F()
{
  struct boolexp *b2;
  skip_whitespace();
  if (*parsebuf == NOT_TOKEN) {
    parsebuf++;
    b2 = alloc_bool();
    b2->type = BOOLEXP_NOT;
    if ((b2->sub1 = parse_boolexp_F()) == TRUE_BOOLEXP) {
      free_boolexp(b2);
      return (TRUE_BOOLEXP);
    } else
      return (b2);
  }
  return (parse_boolexp_A());
}


/* T -> F; T -> F & T */
static struct boolexp *
parse_boolexp_T()
{
  struct boolexp *b;
  struct boolexp *b2;
  if ((b = parse_boolexp_F()) == TRUE_BOOLEXP) {
    return b;
  } else {
    skip_whitespace();
    if (*parsebuf == AND_TOKEN) {
      parsebuf++;

      b2 = alloc_bool();
      b2->type = BOOLEXP_AND;
      b2->sub1 = b;
      if ((b2->sub2 = parse_boolexp_T()) == TRUE_BOOLEXP) {
	free_boolexp(b2);
	return TRUE_BOOLEXP;
      } else {
	return b2;
      }
    } else {
      return b;
    }
  }
}

/* E -> T; E -> T | E */
static struct boolexp *
parse_boolexp_E()
{
  struct boolexp *b;
  struct boolexp *b2;
  if ((b = parse_boolexp_T()) == TRUE_BOOLEXP) {
    return b;
  } else {
    skip_whitespace();
    if (*parsebuf == OR_TOKEN) {
      parsebuf++;

      b2 = alloc_bool();
      b2->type = BOOLEXP_OR;
      b2->sub1 = b;
      if ((b2->sub2 = parse_boolexp_E()) == TRUE_BOOLEXP) {
	free_boolexp(b2);
	return TRUE_BOOLEXP;
      } else {
	return b2;
      }
    } else {
      return b;
    }
  }
}

struct boolexp *
parse_boolexp(player, buf)
    dbref player;
    const char *buf;
{
  parsebuf = buf;
  parse_player = player;
  return parse_boolexp_E();
}

static int
check_attrib_lock(player, target, atrname, str)
    dbref player;
    dbref target;
    const char *atrname;
    const char *str;
{
  /* player is attempting to pass the lock on target, which has
   * an attribute lock of the form  "atrname/value".
   * This lock is passed if target performing get_eval(target/atrname)
   * would yield value. This does NOT do a wildcard
   * match. It is also case-sensitive for matching.
   */

  ATTR *a;
  char const *asave, *ap;
  char buff[BUFFER_LEN], *bp;
  char *preserve[10];

  if (!atrname || !*atrname || !str || !*str)
    return 0;

  /* fail if there's no matching attribute */
  a = atr_get(target, strupper(atrname));
  if (!a)
    return 0;
  if (!Can_Read_Attr(target, target, a))
    return 0;
  asave = safe_uncompress(a->value);

  /* perform pronoun substitution */
  save_global_regs("check_attrib_lock_save", preserve);
  bp = buff;
  ap = asave;
  process_expression(buff, &bp, &ap, target, player, player,
		     PE_DEFAULT, PT_DEFAULT, NULL);
  *bp = '\0';
  restore_global_regs("check_attrib_lock_save", preserve);
  free((Malloc_t) asave);

  return !strcmp(buff, str);
}
