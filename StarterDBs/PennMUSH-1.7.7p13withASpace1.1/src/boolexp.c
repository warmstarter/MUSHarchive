/**
 * \file boolexp.c
 *
 * \brief Boolean expression parser.
 *
 * This code implements a parser for boolean expressions of the form
 * used in locks. Summary of parsing rules, lowest to highest precedence:
 * \verbatim
 * E -> T; E -> T | E                   (or)
 * T -> F; T -> F & T                   (and)
 * F -> !F;F -> A                       (not)
 * A -> @L; A -> I                      (indirect)
 * I -> =Identifier ; I -> C            (equality)
 * C -> +Identifier ; C -> O            (carry)
 * O -> $Identifier ; O -> L            (owner)
 * L -> (E); L -> eval/attr/flag lock   (parens, special atoms)
 * L -> E, L is an object name or dbref or #t* or #f*   (simple atoms)
 * \endverbatim
 *
 */

#include "copyrite.h"
#include "config.h"

#include <ctype.h>
#include <string.h>

#include "conf.h"
#include "mushdb.h"
#include "match.h"
#include "externs.h"
#include "lock.h"
#include "parse.h"
#include "attrib.h"
#include "flags.h"
#include "dbdefs.h"
#include "log.h"
#include "extchat.h"
#include "strtree.h"
#include "memcheck.h"
#include "confmagic.h"

static void unparse_boolexp1
  (dbref player, struct boolexp *b, boolexp_type outer_type, enum u_b_f flag);
static void skip_whitespace(void);
static void free_bool(struct boolexp *b);
static struct boolexp *test_atr(char *s, char c);
static struct boolexp *parse_boolexp_R(void);
static struct boolexp *parse_boolexp_L(void);
static struct boolexp *parse_boolexp_O(void);
static struct boolexp *parse_boolexp_C(void);
static struct boolexp *parse_boolexp_I(void);
static struct boolexp *parse_boolexp_A(void);
static struct boolexp *parse_boolexp_F(void);
static struct boolexp *parse_boolexp_T(void);
static struct boolexp *parse_boolexp_E(void);
static int check_attrib_lock(dbref player, dbref target,
			     const char *atrname, const char *str);

/* Assuming 4096 byte pages and 16 byte boolexps */
#define BOOLEXPS_PER_PAGE 250

static struct boolexp *free_list = NULL;

extern StrTree atr_names;
extern StrTree lock_names;
extern int loading_db;

static int boolexp_recursion = 0;

struct boolatr *
alloc_atr(const char *name, const char *s)
{
  struct boolatr *a;
  size_t len;

  if (s)
    len = strlen(s) + 1;
  else
    len = 1;

  a = (struct boolatr *)
    mush_malloc(sizeof(struct boolatr) - BUFFER_LEN + len, "boolatr");
  if (!a)
    return NULL;
  a->name = st_insert(strupper(name), &atr_names);
  if (!a->name) {
    mush_free(a, "boolatr");
    return NULL;
  }
  if (s)
    memcpy(a->text, s, len);
  else
    a->text[0] = '\0';
  return a;
}

/** Allocate memory for a boolexp.
 * This function returns a new boolexp. If there are none available on
 * the current free list, we allocate a new page of boolexps and
 * construct a new free list.
 */
struct boolexp *
alloc_bool(void)
{
  struct boolexp *b;

  if (!free_list) {
    size_t n;

    b = (struct boolexp *) mush_malloc
      (sizeof(struct boolexp) * BOOLEXPS_PER_PAGE, "bool_page");

    if (!b)
      panic("Unable to allocate memeory for boolexps!");

    for (n = 0; n < BOOLEXPS_PER_PAGE - 1; n++)
      b[n].data.n = &b[n + 1];

    b[n].data.n = NULL;

    free_list = &b[0];
  }

  b = free_list;
  free_list = b->data.n;

  b->data.sub.a = NULL;
  b->data.sub.b = NULL;
  b->thing = NOTHING;

  return b;
}

static void
free_bool(struct boolexp *b)
{
  if (b && b != TRUE_BOOLEXP) {
    b->data.n = free_list;
    free_list = b;
  }
}

/** Free a boolexp.
 * This function frees a boolexp, including all subexpressions,
 * recursively.
 * \param b boolexp to free.
 */
void
free_boolexp(struct boolexp *b)
{
  if (b && b != TRUE_BOOLEXP) {
    switch (b->type) {
    case BOOLEXP_AND:
    case BOOLEXP_OR:
      free_boolexp(b->data.sub.a);
      free_boolexp(b->data.sub.b);
      free_bool(b);
      break;
    case BOOLEXP_NOT:
      free_boolexp(b->data.n);
      free_bool(b);
      break;
    case BOOLEXP_CONST:
    case BOOLEXP_CARRY:
    case BOOLEXP_IS:
    case BOOLEXP_OWNER:
    case BOOLEXP_BOOL:
      free_bool(b);
      break;
    case BOOLEXP_IND:
      if (b->data.ind_lock)
	st_delete(b->data.ind_lock, &lock_names);
      free_bool(b);
      break;
    case BOOLEXP_ATR:
    case BOOLEXP_EVAL:
    case BOOLEXP_FLAG:
      if (b->data.atr_lock) {
	if (b->data.atr_lock->name)
	  st_delete(b->data.atr_lock->name, &atr_names);
	mush_free((Malloc_t) b->data.atr_lock, "boolatr");
      }
      free_bool(b);
      break;
    }
  }
}

/** Copy a boolexp.
 * This function makes a copy of a boolexp, allocating new memory for
 * the copy. This is a deep copy; it recursively copies all subexpressions
 * necessary.
 * \param b a boolexp to copy.
 * \return an allocated copy of the boolexp.
 */
struct boolexp *
dup_bool(struct boolexp *b)
{
  struct boolexp *r;
  if (b == TRUE_BOOLEXP)
    return (TRUE_BOOLEXP);
  r = alloc_bool();
  switch (r->type = b->type) {
  case BOOLEXP_AND:
  case BOOLEXP_OR:
    r->data.sub.a = dup_bool(b->data.sub.a);
    r->data.sub.b = dup_bool(b->data.sub.b);
    break;
  case BOOLEXP_NOT:
    r->data.n = dup_bool(b->data.n);
    break;
  case BOOLEXP_CONST:
  case BOOLEXP_IS:
  case BOOLEXP_CARRY:
  case BOOLEXP_OWNER:
  case BOOLEXP_BOOL:
    r->thing = b->thing;
    break;
  case BOOLEXP_IND:
    r->thing = b->thing;
    r->data.ind_lock = st_insert(b->data.ind_lock, &atr_names);
    break;
  case BOOLEXP_ATR:
  case BOOLEXP_EVAL:
  case BOOLEXP_FLAG:
    r->data.atr_lock = alloc_atr(b->data.atr_lock->name,
				 b->data.atr_lock->text);
    break;
  default:
    do_rawlog(LT_ERR, T("ERROR: bad bool type in dup_bool!\n"));
    return (TRUE_BOOLEXP);
  }
  return (r);
}

/** Determine the memory usage of a boolexp.
 * This function computes the total memory usage of a boolexp, including
 * subexpressions.
 * \param b boolexp to analyze.
 * \return size of boolexp in bytes.
 */
int
sizeof_boolexp(struct boolexp *b)
{
  if (!b || b == TRUE_BOOLEXP)
    return 0;
  switch (b->type) {
  case BOOLEXP_CONST:
  case BOOLEXP_IS:
  case BOOLEXP_CARRY:
  case BOOLEXP_OWNER:
  case BOOLEXP_IND:
  case BOOLEXP_BOOL:
    return sizeof(struct boolexp);
  case BOOLEXP_NOT:
    return sizeof(struct boolexp) + sizeof_boolexp(b->data.n);
  case BOOLEXP_AND:
  case BOOLEXP_OR:
    return sizeof(struct boolexp) +
      sizeof_boolexp(b->data.sub.a) + sizeof_boolexp(b->data.sub.b);
  case BOOLEXP_ATR:
  case BOOLEXP_EVAL:
  case BOOLEXP_FLAG:
    return sizeof(struct boolexp) + sizeof(struct boolatr) - BUFFER_LEN +
      strlen(b->data.atr_lock->text) + 1;
  }
  /* Broken lock */
  return sizeof(struct boolexp);
}

/** Evaluate a boolexp.
 * This is the main function to be called by other hardcode. It
 * determines whether a player can pass a boolexp lock on a given
 * object.
 * \param player the player trying to pass the lock.
 * \param b the boolexp to evaluate.
 * \param target the object with the lock.
 * \retval 0 player fails to pass lock.
 * \retval 1 player successfully passes lock.
 */
int
eval_boolexp(dbref player /* The player trying to pass */ ,
	     struct boolexp *b /* The boolexp */ ,
	     dbref target /* The object with the lock */ )
{
  ATTR *a;
  char tbuf1[BUFFER_LEN], tbuf2[BUFFER_LEN];
  int pwr;
#ifdef CHAT_SYSTEM
  CHAN *chan;
#endif
  int retval;

  if (!GoodObject(player))
    return 0;

  if (boolexp_recursion > MAX_DEPTH) {
    notify(player, T("Too much recursion in lock!"));
    return 0;
  }
  if (!b)
    return 1;
  if (b == TRUE_BOOLEXP) {
    return 1;
  } else {
    switch (b->type) {
    case BOOLEXP_IND:
      /* We only allow evaluation of indirect locks if target can run
       * the lock on the referenced object.
       */
      if (b->thing == TRUE_ATOM)
	return 1;
      if (b->thing == FALSE_ATOM || !GoodObject(b->thing)
	  || IsGarbage(b->thing))
	return 0;
      if (!Can_Read_Lock(target, b->thing, b->data.ind_lock))
	return 0;
      boolexp_recursion++;
      retval = eval_boolexp(player, getlock(b->thing, b->data.ind_lock),
			    b->thing);
      boolexp_recursion--;
      return retval;
    case BOOLEXP_AND:
      retval = eval_boolexp(player, b->data.sub.a, target)
	&& eval_boolexp(player, b->data.sub.b, target);
      return retval;
    case BOOLEXP_OR:
      retval = eval_boolexp(player, b->data.sub.a, target)
	|| eval_boolexp(player, b->data.sub.b, target);
      return retval;
    case BOOLEXP_NOT:
      retval = !eval_boolexp(player, b->data.n, target);
      return retval;
    case BOOLEXP_CONST:
      return (GoodObject(b->thing)
	      && !IsGarbage(b->thing)
	      && (b->thing == player || member(b->thing, Contents(player))));
    case BOOLEXP_BOOL:
      return b->thing == TRUE_ATOM;
    case BOOLEXP_IS:
      return (GoodObject(b->thing)
	      && !IsGarbage(b->thing)
	      && b->thing == player);
    case BOOLEXP_CARRY:
      return (GoodObject(b->thing)
	      && !IsGarbage(b->thing)
	      && member(b->thing, Contents(player)));
    case BOOLEXP_OWNER:
      return (GoodObject(b->thing)
	      && !IsGarbage(b->thing)
	      && Owner(b->thing) == Owner(player));
    case BOOLEXP_ATR:
      boolexp_recursion++;
      a = atr_get(player, b->data.atr_lock->name);
      if (!a || !Can_Read_Attr(target, player, a)) {
	boolexp_recursion--;
	return 0;
      }
      boolexp_recursion--;
      strcpy(tbuf1, b->data.atr_lock->text);
      strcpy(tbuf2, uncompress(a->value));
      return local_wild_match(tbuf1, tbuf2);
    case BOOLEXP_EVAL:
      boolexp_recursion++;
      strcpy(tbuf1, b->data.atr_lock->text);
      retval = check_attrib_lock(player, target, b->data.atr_lock->name, tbuf1);
      boolexp_recursion--;
      return retval;
    case BOOLEXP_FLAG:
      /* Note that both fields of a boolattr struct are upper-cased */
      if (strcmp(b->data.atr_lock->name, "FLAG") == 0) {
	if (sees_flag(target, player, b->data.atr_lock->text))
	  return 1;
	else
	  return 0;
      } else if (strcmp(b->data.atr_lock->name, "POWER") == 0) {
	if ((pwr = find_power(b->data.atr_lock->text)) == -1)
	  return 0;
	if (HASPOWER_RESTRICTED && !Can_Examine(target, player)) {
	  return 0;
	}
	return Powers(player) & pwr;
#ifdef CHAT_SYSTEM
      } else if (strcmp(b->data.atr_lock->name, "CHANNEL") == 0) {
	boolexp_recursion++;
	find_channel(b->data.atr_lock->text, &chan, target);
	boolexp_recursion--;
	return chan && onchannel(player, chan);
#endif
      } else if (strcmp(b->data.atr_lock->name, "TYPE") == 0) {
	switch (*b->data.atr_lock->text) {
	case 'R':
	  return Typeof(player) == TYPE_ROOM;
	  break;
	case 'E':
	  return Typeof(player) == TYPE_EXIT;
	  break;
	case 'T':
	  return Typeof(player) == TYPE_THING;
	  break;
	case 'P':
	  return Typeof(player) == TYPE_PLAYER;
	  break;
	}
      } else {
	return 0;
      }
    default:
      do_log(LT_ERR, 0, 0, "Bad boolexp type %d in object #%d",
	     b->type, b->thing);
      report();
      return 0;
    }				/* switch */
    /* should never be reached */
    do_log(LT_ERR, 0, 0, T("Broken lock on #%d in object called by #%d"),
	   target, player);
    return 0;
  }				/* else */
}

/* If the parser returns TRUE_BOOLEXP, you lose */
/* TRUE_BOOLEXP cannot be typed in by the user; use @unlock instead */
static const char *parsebuf;
static dbref parse_player;
static lock_type parse_ltype;

static char boolexp_buf[BUFFER_LEN];
static char *buftop;

static void
unparse_boolexp1(dbref player, struct boolexp *b, boolexp_type outer_type,
		 enum u_b_f flag)
{
  if (b == TRUE_BOOLEXP) {
    safe_str("*UNLOCKED*", boolexp_buf, &buftop);
    return;
  } else {
    switch (b->type) {
    case BOOLEXP_AND:
      if (outer_type == BOOLEXP_NOT) {
	safe_chr('(', boolexp_buf, &buftop);
      }
      unparse_boolexp1(player, b->data.sub.a, b->type, flag);
      safe_chr(AND_TOKEN, boolexp_buf, &buftop);
      unparse_boolexp1(player, b->data.sub.b, b->type, flag);
      if (outer_type == BOOLEXP_NOT) {
	safe_chr(')', boolexp_buf, &buftop);
      }
      break;
    case BOOLEXP_OR:
      if (outer_type == BOOLEXP_NOT || outer_type == BOOLEXP_AND) {
	safe_chr('(', boolexp_buf, &buftop);
      }
      unparse_boolexp1(player, b->data.sub.a, b->type, flag);
      safe_chr(OR_TOKEN, boolexp_buf, &buftop);
      unparse_boolexp1(player, b->data.sub.b, b->type, flag);
      if (outer_type == BOOLEXP_NOT || outer_type == BOOLEXP_AND) {
	safe_chr(')', boolexp_buf, &buftop);
      }
      break;
    case BOOLEXP_IND:
      safe_chr(AT_TOKEN, boolexp_buf, &buftop);
      if (flag == UB_DBREF)
	safe_format(boolexp_buf, &buftop, "#%d", b->thing);
      else
	safe_str(unparse_object(player, b->thing), boolexp_buf, &buftop);
      safe_format(boolexp_buf, &buftop, "/%s", b->data.ind_lock);
      break;
    case BOOLEXP_IS:
      safe_chr(IS_TOKEN, boolexp_buf, &buftop);
      if (flag == UB_DBREF)
	safe_format(boolexp_buf, &buftop, "#%d", b->thing);
      else
	safe_str(unparse_object(player, b->thing), boolexp_buf, &buftop);
      break;
    case BOOLEXP_CARRY:
      safe_chr(IN_TOKEN, boolexp_buf, &buftop);
      if (flag == UB_DBREF)
	safe_format(boolexp_buf, &buftop, "#%d", b->thing);
      else
	safe_str(unparse_object(player, b->thing), boolexp_buf, &buftop);
      break;
    case BOOLEXP_OWNER:
      safe_chr(OWNER_TOKEN, boolexp_buf, &buftop);
      if (flag == UB_DBREF)
	safe_format(boolexp_buf, &buftop, "#%d", b->thing);
      else
	safe_str(unparse_object(player, b->thing), boolexp_buf, &buftop);
      break;
    case BOOLEXP_NOT:
      safe_chr(NOT_TOKEN, boolexp_buf, &buftop);
      unparse_boolexp1(player, b->data.n, b->type, flag);
      break;
    case BOOLEXP_CONST:
      if (flag == UB_DBREF)
	safe_format(boolexp_buf, &buftop, "#%d", b->thing);
      else
	safe_str(unparse_object(player, b->thing), boolexp_buf, &buftop);
      break;
    case BOOLEXP_BOOL:
      if (b->thing == TRUE_ATOM)
	safe_str("#TRUE", boolexp_buf, &buftop);
      else
	safe_str("#FALSE", boolexp_buf, &buftop);
      break;
    case BOOLEXP_ATR:
      safe_format(boolexp_buf, &buftop, "%s:%s",
		  b->data.atr_lock->name, b->data.atr_lock->text);
      break;
    case BOOLEXP_EVAL:
      safe_format(boolexp_buf, &buftop, "%s/%s",
		  b->data.atr_lock->name, b->data.atr_lock->text);
      break;
    case BOOLEXP_FLAG:
      safe_format(boolexp_buf, &buftop, "%s^%s", b->data.atr_lock->name,
		  b->data.atr_lock->text);
      break;
    default:
      safe_str("Bad boolexp type!", boolexp_buf, &buftop);
      do_rawlog(LT_ERR,
		"ERROR: unparse_boolexp1 bad boolexp type on #%d\n", player);
      break;
    }
  }
}

/** Display a boolexp.
 * This function returns the textual representation of the boolexp.
 */
char *
unparse_boolexp(dbref player, struct boolexp *b, enum u_b_f flag)
{
  buftop = boolexp_buf;
  unparse_boolexp1(player, b, BOOLEXP_CONST, flag);	/* no outer type */
  *buftop++ = '\0';
  return boolexp_buf;
}


static void
skip_whitespace()
{
  while (*parsebuf && isspace((unsigned char) *parsebuf))
    parsebuf++;
}


static struct boolexp *
test_atr(char *s, char c)
{
  struct boolexp *b;
  char tbuf1[BUFFER_LEN];
  strcpy(tbuf1, strupper(s));
  for (s = tbuf1; *s && (*s != c); s++) ;
  if (!*s)
    return 0;
  *s++ = 0;
  if (strlen(tbuf1) == 0 || !good_atr_name(tbuf1))
    return 0;
  b = alloc_bool();
  if (c == ':')
    b->type = BOOLEXP_ATR;
  else if (c == '/')
    b->type = BOOLEXP_EVAL;
  else if (c == '^')
    b->type = BOOLEXP_FLAG;
  b->data.atr_lock = alloc_atr(tbuf1, s);
  return b;
}

/* L -> E, L is an object name or dbref or #t* or #f* */
static struct boolexp *
parse_boolexp_R()
{
  struct boolexp *b;
  char tbuf1[BUFFER_LEN];
  char *p;
  b = alloc_bool();
  b->type = BOOLEXP_CONST;
  p = tbuf1;
  while (*parsebuf
	 && *parsebuf != AND_TOKEN && *parsebuf != '/'
	 && *parsebuf != OR_TOKEN && *parsebuf != ')') {
    *p++ = *parsebuf++;
  }
  /* strip trailing whitespace */
  *p-- = '\0';
  while (isspace((unsigned char) *p))
    *p-- = '\0';
  /* do the match */
  if (loading_db) {
    if (*tbuf1 == '#' && *(tbuf1 + 1)) {
      if (*(tbuf1 + 1) == 't' || *(tbuf1 + 1) == 'T') {
	b->type = BOOLEXP_BOOL;
	b->thing = TRUE_ATOM;
      } else if (*(tbuf1 + 1) == 'f' || *(tbuf1 + 1) == 'F') {
	b->type = BOOLEXP_BOOL;
	b->thing = FALSE_ATOM;
      } else {
	b->thing = parse_integer(tbuf1 + 1);
      }
    } else {
      /* Ooog. Dealing with a malformed lock in the database. */
      free_bool(b);
      return TRUE_BOOLEXP;
    }
    return b;
  } else {
    /* Are these special atoms? */
    if (*tbuf1 && *tbuf1 == '#' && *(tbuf1 + 1)) {
      if (*(tbuf1 + 1) == 't' || *(tbuf1 + 1) == 'T') {
	b->type = BOOLEXP_BOOL;
	b->thing = TRUE_ATOM;
	return b;
      } else if (*(tbuf1 + 1) == 'f' || *(tbuf1 + 1) == 'F') {
	b->type = BOOLEXP_BOOL;
	b->thing = FALSE_ATOM;
	return b;
      }
    }
    b->thing = match_result(parse_player, tbuf1, TYPE_THING, MAT_EVERYTHING);
    if (b->thing == NOTHING) {
      notify_format(parse_player, T("I don't see %s here."), tbuf1);
      free_bool(b);
      return TRUE_BOOLEXP;
    } else if (b->thing == AMBIGUOUS) {
      notify_format(parse_player, T("I don't know which %s you mean!"), tbuf1);
      free_bool(b);
      return TRUE_BOOLEXP;
    } else {
      return b;
    }
  }
}

/* L -> (E); L -> eval/attr/flag lock, (lock) */
static struct boolexp *
parse_boolexp_L()
{
  struct boolexp *b;
  char *p;
  const char *savebuf;
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
    savebuf = parsebuf;
    while (*parsebuf
	   && *parsebuf != AND_TOKEN
	   && *parsebuf != OR_TOKEN && *parsebuf != ')') {
      *p++ = *parsebuf++;
    }
    /* strip trailing whitespace */
    *p-- = '\0';
    while (isspace((unsigned char) *p))
      *p-- = '\0';
    /* check for an attribute */
    b = test_atr(tbuf1, ':');
    if (b)
      return b;
    /* check for an eval */
    b = test_atr(tbuf1, '/');
    if (b)
      return b;
    /* Check for a flag */
    b = test_atr(tbuf1, '^');
    if (b)
      return b;
    /* Nope. Check for an object reference */
    parsebuf = savebuf;
    return parse_boolexp_R();
  }
}

/* O -> $Identifier ; O -> L */
static struct boolexp *
parse_boolexp_O()
{
  struct boolexp *b2, *t;
  skip_whitespace();
  if (*parsebuf == OWNER_TOKEN) {
    parsebuf++;
    b2 = alloc_bool();
    b2->type = BOOLEXP_OWNER;
    t = parse_boolexp_R();
    if (t == TRUE_BOOLEXP) {
      free_boolexp(b2);
      return TRUE_BOOLEXP;
    } else {
      b2->thing = t->thing;
      free_boolexp(t);
      return b2;
    }
  }
  return parse_boolexp_L();
}

/* C -> +Identifier ; C -> O */
static struct boolexp *
parse_boolexp_C()
{
  struct boolexp *b2, *t;
  skip_whitespace();
  if (*parsebuf == IN_TOKEN) {
    parsebuf++;
    b2 = alloc_bool();
    b2->type = BOOLEXP_CARRY;
    t = parse_boolexp_R();
    if (t == TRUE_BOOLEXP) {
      free_boolexp(b2);
      return TRUE_BOOLEXP;
    } else {
      b2->thing = t->thing;
      free_boolexp(t);
      return b2;
    }
  }
  return parse_boolexp_O();
}

/* I -> =Identifier ; I -> C */
static struct boolexp *
parse_boolexp_I()
{
  struct boolexp *b2, *t;
  skip_whitespace();
  if (*parsebuf == IS_TOKEN) {
    parsebuf++;
    b2 = alloc_bool();
    b2->type = BOOLEXP_IS;
    t = parse_boolexp_R();
    if (t == TRUE_BOOLEXP) {
      free_boolexp(b2);
      return TRUE_BOOLEXP;
    } else {
      b2->thing = t->thing;
      free_boolexp(t);
      return b2;
    }
  }
  return parse_boolexp_C();
}

/* A -> @L; A -> I */
static struct boolexp *
parse_boolexp_A()
{
  struct boolexp *b2, *t;
  skip_whitespace();
  if (*parsebuf == AT_TOKEN) {
    parsebuf++;
    b2 = alloc_bool();
    b2->type = BOOLEXP_IND;
    t = parse_boolexp_R();
    if (t == TRUE_BOOLEXP) {
      free_boolexp(b2);
      return TRUE_BOOLEXP;
    }
    b2->thing = t->thing;
    free_boolexp(t);
    if (*parsebuf == '/') {
      char tbuf1[BUFFER_LEN], *p;
      const char *m;
      parsebuf++;
      p = tbuf1;
      while (*parsebuf
	     && *parsebuf != AND_TOKEN
	     && *parsebuf != OR_TOKEN && *parsebuf != ')') {
	*p++ = *parsebuf++;
      }
      /* strip trailing whitespace */
      *p-- = '\0';
      while (isspace((unsigned char) *p))
	*p-- = '\0';
      upcasestr(tbuf1);
      if (!good_atr_name(tbuf1)) {
	free_boolexp(b2);
	return TRUE_BOOLEXP;
      }
      m = match_lock(tbuf1);
      b2->data.ind_lock = st_insert(m ? m : tbuf1, &lock_names);
    } else {
      b2->data.ind_lock = st_insert(parse_ltype, &atr_names);
    }
    return b2;
  }
  return parse_boolexp_I();
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
    if ((b2->data.n = parse_boolexp_F()) == TRUE_BOOLEXP) {
      free_boolexp(b2);
      return TRUE_BOOLEXP;
    } else
      return b2;
  }
  return parse_boolexp_A();
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
      b2->data.sub.a = b;
      if ((b2->data.sub.b = parse_boolexp_T()) == TRUE_BOOLEXP) {
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
      b2->data.sub.a = b;
      if ((b2->data.sub.b = parse_boolexp_E()) == TRUE_BOOLEXP) {
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

/** Parse a string into a boolexp.
 * Given a textual representation of a boolexp in a string, parse it
 * and return a pointer to a boolexp structure.
 * \param player the enactor.
 * \param buf string representation of a boolexp.
 * \param ltype the type of lock for which the boolexp is being parsed.
 * \return pointer to a newly allocated boolexp.
 */
struct boolexp *
parse_boolexp(dbref player, const char *buf, lock_type ltype)
{
  parsebuf = buf;
  parse_player = player;
  parse_ltype = ltype;
  return parse_boolexp_E();
}

static int
check_attrib_lock(dbref player, dbref target,
		  const char *atrname, const char *str)
{
  /* player is attempting to pass the lock on target, which has
   * an attribute lock of the form  "atrname/value".
   * This lock is passed if target performing get_eval(target/atrname)
   * would yield value. This does NOT do a wildcard
   * match. It is also case-sensitive for matching.
   */

  ATTR *a;
  char *asave;
  const char *ap;
  char buff[BUFFER_LEN], *bp;
  char *preserve[NUMQ];
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
  process_expression(buff, &bp, &ap, target, player,
		     player, PE_DEFAULT, PT_DEFAULT, NULL);
  *bp = '\0';
  restore_global_regs("check_attrib_lock_save", preserve);
  free(asave);

  return !strcasecmp(buff, str);
}
