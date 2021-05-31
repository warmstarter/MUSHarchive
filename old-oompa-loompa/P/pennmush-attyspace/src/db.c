/* db.c */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#include <ctype.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif

#include "conf.h"
#include "intrface.h"
#include "mushdb.h"
#include "attrib.h"
#include "externs.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#ifdef ADD_POWERS
#include "convdb.c"
#endif
#include "mymalloc.h"
#include "confmagic.h"

#ifdef WIN32
void shutdown_checkpoint _((void));
#endif

#define QUOTE_STRINGS

#define MAYBE_GET(f,x) \
	(indb_flags & (x)) ? getref(f) : 0

extern int paranoid_checkpt;	/* from game.c */

static long indb_flags;		/* What flags are in the db we read at startup? */

struct object *db = 0;
dbref db_top = 0;

dbref errobj;

#ifndef DB_INITIAL_SIZE
#define DB_INITIAL_SIZE 5000
#endif				/* DB_INITIAL_SIZE */

dbref db_size = DB_INITIAL_SIZE;

extern char ccom[];

struct boolexp *alloc_bool _((void));
void free_bool _((struct boolexp * b));
const char *set_string _((const char **ptr, const char *newstr));
static void db_grow _((dbref newtop));
dbref new_object _((void));
void putref _((FILE * f, long int ref));
void putstring _((FILE * f, const char *s));
static void putbool_subexp _((FILE * f, struct boolexp * b));
void putboolexp _((FILE * f, struct boolexp * b));
void putlocks _((FILE * f, struct lock_list *l));
void db_write_obj_basic _((FILE * f, dbref i, struct object * o));
int db_write_object _((FILE * f, dbref i));
dbref db_write _((FILE * f));
int db_paranoid_write_object _((FILE * f, dbref i, int flag));
dbref db_paranoid_write _((FILE * f, int flag));
long int getref _((FILE * f));
const char *getstring_noalloc _((FILE * f));
static struct boolexp *getboolexp1 _((FILE * f));
struct boolexp *getboolexp _((FILE * f));
void getlocks _((dbref i, FILE * f));
void get_old_locks _((dbref i, FILE * f));
static void tweak_locks _((void));
void free_boolexp _((struct boolexp * b));
struct boolexp *dup_bool _((struct boolexp * b));
void db_free _((void));
int get_list _((FILE * f, dbref i));
dbref db_read _((FILE * f));


/* manage boolean expression free list */
struct boolexp *
alloc_bool()
{
  struct boolexp *b = (struct boolexp *) mush_malloc(sizeof(struct boolexp), "boolexp");
  b->type = BOOLEXP_NULL;
  b->sub1 = NULL;
  b->sub2 = NULL;
  b->thing = NOTHING;
  b->atr_lock = NULL;
  return b;
}

void
free_bool(b)
    struct boolexp *b;
{
  if (b != TRUE_BOOLEXP && b) {
    mush_free((Malloc_t) b, "boolexp");
  }
}

const char *
set_string(ptr, newstr)
    const char **ptr;
    const char *newstr;
{
  /* if pointer not null unalloc it */
  if (*ptr) {
    mush_free((Malloc_t) * ptr, "object_name");
  }
  if (!newstr || !*newstr)
    return (*ptr = NULL);
  *ptr = strdup(newstr);
#ifdef MEM_CHECK
  add_check("object_name");
#endif
  return (*ptr);
}

int db_init = 0;

static void
db_grow(newtop)
    dbref newtop;
{
  struct object *newdb;
  dbref initialized;
  struct object *o;

  if (newtop > db_top) {
    initialized = db_top;
    db_top = newtop;
    if (!db) {
      /* make the initial one */
      db_size = (db_init) ? db_init : DB_INITIAL_SIZE;
      while (db_top > db_size)
	db_size *= 2;
      if ((db = (struct object *)
	   malloc(db_size * sizeof(struct object))) == NULL) {
	fprintf(stderr, "ERROR: out of memory!\n");
	fflush(stderr);
	abort();
      }
    }
    /* maybe grow it */
    if (db_top > db_size) {
      /* make sure it's big enough */
      while (db_top > db_size)
	db_size *= 2;
      if ((newdb = (struct object *)
	   realloc(db, db_size * sizeof(struct object))) == NULL) {
	fprintf(stderr, "ERROR: out of memory!\n");
	fflush(stderr);
	abort();
      }
      db = newdb;
    }
    while (initialized < db_top) {
      o = db + initialized;
      o->name = 0;
      o->list = 0;
      o->location = NOTHING;
      o->contents = NOTHING;
      o->exits = NOTHING;
      o->next = NOTHING;
      o->parent = NOTHING;
      o->locks = NULL;
      o->owner = GOD;
      o->zone = NOTHING;
      o->penn = 0;
      o->flags = TYPE_GARBAGE;
      o->toggles = 0;
      o->powers = 0;
#ifdef CHAT_SYSTEM
      /* initialize channels here, since it's not going to get done otherwise */
      o->channels = NULL;
#endif
#ifdef USE_WARNINGS
      o->warnings = 0;
#endif
#ifdef CREATION_TIMES
      o->modification_time = o->creation_time = time((time_t *) 0);
#endif
#ifdef LOCAL_DATA
      o->local_data = NULL;
#endif
      initialized++;
    }
  }
}

dbref
new_object()
{
  dbref newobj;
  struct object *o;
  /* if stuff in free list use it */
  if ((newobj = free_get()) == NOTHING) {
    /* allocate more space */
    newobj = db_top;
    db_grow(db_top + 1);
  }
  /* clear it out */
  o = db + newobj;
  o->name = 0;
  o->list = 0;
  o->location = NOTHING;
  o->contents = NOTHING;
  o->exits = NOTHING;
  o->next = NOTHING;
  o->parent = NOTHING;
  o->locks = NULL;
  o->owner = GOD;
  o->zone = NOTHING;
  o->penn = 0;
  o->flags = TYPE_GARBAGE;
  o->toggles = 0;
  o->powers = 0;
#ifdef CHAT_SYSTEM
  /* initialize channels here, since it's not going to get done otherwise */
  o->channels = NULL;
#endif
#ifdef USE_WARNINGS
  o->warnings = 0;
#endif
#ifdef CREATION_TIMES
  o->modification_time = o->creation_time = time((time_t *) 0);
#endif
#ifdef LOCAL_DATA
  o->local_data = NULL;
#endif
  /* flags you must initialize yourself */
  return newobj;
}

void
putref(f, ref)
    FILE *f;
    long int ref;
{
  fprintf(f, "%ld\n", ref);
}

#ifdef QUOTE_STRINGS
void
putstring(f, s)
    FILE *f;
    const char *s;
{
  putc('"', f);
  while (*s) {
    switch (*s) {
    case '\\':
    case '"':
      putc('\\', f);
      /* FALL THROUGH */
    default:
      putc(*s, f);
    }
    s++;
  }
  putc('"', f);
  putc('\n', f);
}
#else				/* !QUOTE_STRINGS */
void
putstring(f, s)
    FILE *f;
    const char *s;
{
  if (s) {
    fputs(s, f);
  }
  putc('\n', f);
}
#endif				/* !QUOTE_STRINGS */

static void
putbool_subexp(f, b)
    FILE *f;
    struct boolexp *b;
{
  switch (b->type) {
  case BOOLEXP_IS:
    putc('(', f);
    putc(IS_TOKEN, f);
    putbool_subexp(f, b->sub1);
    putc(')', f);
    break;
  case BOOLEXP_CARRY:
    putc('(', f);
    putc(IN_TOKEN, f);
    putbool_subexp(f, b->sub1);
    putc(')', f);
    break;
  case BOOLEXP_OWNER:
    putc('(', f);
    putc(OWNER_TOKEN, f);
    putbool_subexp(f, b->sub1);
    putc(')', f);
    break;
  case BOOLEXP_IND:
    putc('(', f);
    putc(AT_TOKEN, f);
    putbool_subexp(f, b->sub1);
    putc(')', f);
    break;
  case BOOLEXP_AND:
    putc('(', f);
    putbool_subexp(f, b->sub1);
    putc(AND_TOKEN, f);
    putbool_subexp(f, b->sub2);
    putc(')', f);
    break;
  case BOOLEXP_OR:
    putc('(', f);
    putbool_subexp(f, b->sub1);
    putc(OR_TOKEN, f);
    putbool_subexp(f, b->sub2);
    putc(')', f);
    break;
  case BOOLEXP_NOT:
    putc('(', f);
    putc(NOT_TOKEN, f);
    putbool_subexp(f, b->sub1);
    putc(')', f);
    break;
  case BOOLEXP_CONST:
    fprintf(f, "%d", b->thing);
    break;
#ifdef QUOTE_STRINGS
  case BOOLEXP_ATR:
    putstring(f, b->atr_lock->name);
    putc(':', f);
    putstring(f, uncompress(b->atr_lock->text));
    break;
  case BOOLEXP_EVAL:
    putstring(f, b->atr_lock->name);
    putc('/', f);
    putstring(f, uncompress(b->atr_lock->text));
    break;
#else				/* !QUOTE_STRINGS */
  case BOOLEXP_ATR:
    fprintf(f, "%s:%s", b->atr_lock->name, uncompress(b->atr_lock->text));
    break;
  case BOOLEXP_EVAL:
    fprintf(f, "%s/%s", b->atr_lock->name, uncompress(b->atr_lock->text));
    break;
#endif				/* !QUOTE_STRINGS */
  default:
    break;
  }
}

void
putboolexp(f, b)
    FILE *f;
    struct boolexp *b;
{
  if (b != TRUE_BOOLEXP) {
    putbool_subexp(f, b);
  }
  putc('\n', f);
}

void
putlocks(f, l)
    FILE *f;
    struct lock_list *l;
{
  struct lock_list *ll;
  for (ll = l; ll; ll = ll->next) {
    putc('_', f);
    fputs(ll->type, f);
    putc('|', f);
    putboolexp(f, ll->key);
    /* putboolexp adds a '\n', so we won't. */
  }
}

void
db_write_obj_basic(f, i, o)
    FILE *f;
    dbref i;
    struct object *o;
{
  putstring(f, o->name);
  putref(f, o->location);
  putref(f, o->contents);
  putref(f, o->exits);
  putref(f, o->next);
  putref(f, o->parent);
  putlocks(f, Locks(i));
  putref(f, o->owner);
  putref(f, o->zone);
  putref(f, Pennies(i));
  putref(f, o->flags);
  putref(f, o->toggles);
#ifndef DELETE_POWERS
  putref(f, o->powers);
#endif
  /* Original chat system wrote chat channels to the db here. 
   * That's no longer done, they're written to chatdb 
   */
#ifdef USE_WARNINGS
  putref(f, o->warnings);
#endif
#ifdef CREATION_TIMES
  putref(f, o->creation_time);
  putref(f, o->modification_time);
#endif
}

int
db_write_object(f, i)
    FILE *f;
    dbref i;
{
  struct object *o;
  ALIST *list;
  o = db + i;
  db_write_obj_basic(f, i, o);

  /* write the attribute list */
  for (list = o->list; list; list = AL_NEXT(list)) {
#ifndef DUMP_EMPTY_ATTRS
    if (!*AL_STR(list))
      continue;
#endif
    fprintf(f, "]%s^%d^%d\n", AL_NAME(list), Owner(AL_CREATOR(list)),
	    AL_FLAGS(list));
    putstring(f, uncompress(AL_STR(list)));
  }
  fprintf(f, "<\n");
  return 0;
}

dbref
db_write(f)
    FILE *f;
{
  dbref i;
  int dbflag;

/* print a header line to make a later conversion to 2.0 easier to do.
 * the odd choice of numbers is based on 256*x + 2 offset
 * The original PennMUSH had x=5 (chat) or x=6 (nochat), and Tiny expects
 * to deal with that. We need to use some extra flags as well, so
 * we may be adding to 5/6 as needed, using successive binary numbers.
 */
  dbflag = 5;
  dbflag += DBF_NO_CHAT_SYSTEM;
#ifdef USE_WARNINGS
  dbflag += DBF_WARNINGS;
#endif
#ifdef CREATION_TIMES
  dbflag += DBF_CREATION_TIMES;
#endif
#ifndef ADD_POWERS
#ifdef DELETE_POWERS
  dbflag += DBF_NO_POWERS;
#endif
#endif
  dbflag += DBF_NEW_LOCKS;
#ifdef QUOTE_STRINGS
  dbflag += DBF_NEW_STRINGS;
#endif
  dbflag += DBF_TYPE_GARBAGE;
  dbflag += DBF_SPLIT_IMMORTAL;
  dbflag += DBF_NO_TEMPLE;
#ifdef DUMP_LESS_GARBAGE
  dbflag += DBF_LESS_GARBAGE;
#endif
  dbflag += DBF_AF_VISUAL;
  dbflag += DBF_VALUE_IS_COST;

  fprintf(f, "+V%d\n", dbflag * 256 + 2);

  fprintf(f, "~%d\n", db_top);
  for (i = 0; i < db_top; i++) {
#ifdef WIN32
    /* Keep the service manager happy */
    if (shutdown_flag && (i & 0xFF) == 0)
      shutdown_checkpoint();
#endif
#ifdef DUMP_LESS_GARBAGE
    if (IsGarbage(i))
      continue;
#endif
    fprintf(f, "!%d\n", i);
    db_write_object(f, i);
  }
  fputs("***END OF DUMP***\n", f);
/*  fflush(f); */
  return (db_top);
}

int
db_paranoid_write_object(f, i, flag)
    FILE *f;
    dbref i;
    int flag;			/* 1 = debug, 0 = normal */
{
  struct object *o;
  ALIST *list, *next;
  char name[BUFFER_LEN];
  char tbuf1[BUFFER_LEN];
  int err = 0;
  char *p;
  char lastp;
  dbref owner;
  int flags;
  int fixmemdb = 0;
  int count;

  o = db + i;
  db_write_obj_basic(f, i, o);
/*  fflush(f); */

  /* write the attribute list, scanning */
  for (list = o->list; list; list = next) {
    next = AL_NEXT(list);
#ifndef DUMP_EMPTY_ATTRS
    if (!*AL_STR(list))
      continue;
#endif
    fixmemdb = err = 0;
    /* smash unprintable characters in the name, replace with ! */
    strcpy(name, AL_NAME(list));
    for (p = name; *p; p++) {
      if (!isprint(*p) || isspace(*p)) {
	*p = '!';
	fixmemdb = err = 1;
      }
    }
    if (err) {
      /* If name already exists on this object, try adding a
       * number to the end. Give up if we can't find one < 10000
       */
      if (atr_get_noparent(i, name)) {
	count = 0;
	do {
	  name[BUFFER_LEN - 6] = '\0';
	  sprintf(tbuf1, "%s%d", name, count);
	  count++;
	} while (count < 10000 && atr_get_noparent(i, tbuf1));
	strcpy(name, tbuf1);
      }
      fprintf(checklog_fp,
	      " * Bad attribute name on #%d. Changing name to %s.\n",
	      i, name);
      err = 0;
    }
    /* check the owner */
    owner = AL_CREATOR(list);
    if (!GoodObject(owner)) {
      fprintf(checklog_fp,
	      " * Bad owner on attribute %s on #%d.\n", name, i);
      owner = GOD;
      fixmemdb = 1;
    } else {
      owner = Owner(owner);
    }

    /* write that info out */
    fprintf(f, "]%s^%d^%d\n", name, (int) owner, AL_FLAGS(list));

    /* now check the attribute */
    strcpy(tbuf1, uncompress(AL_STR(list)));
    /* get rid of unprintables and hard newlines */
    lastp = '\0';
    for (p = tbuf1; *p; p++) {
      if (!isascii(*p)) {
	*p = '!';
	err = 1;
      } else if (!isprint(*p)) {
#ifdef QUOTE_STRINGS
	if (!isspace(*p)) {
	  *p = '!';
	  err = 1;
	}
#else				/* !QUOTE_STRINGS */
	/* If we've got a \n that's not preceded by a \r, stomp it. */
	if (!isspace(*p) || ((*p == '\n') && (lastp != '\r'))) {
	  *p = '!';
	  err = 1;
	}
#endif				/* !QUOTE_STRINGS */
      }
      lastp = *p;
    }
    if (err) {
      fixmemdb = 1;
      fprintf(checklog_fp,
	   " * Bad text in attribute %s on #%d. Changed to:\n", name, i);
      fprintf(checklog_fp, "%s\n", tbuf1);
    }
    putstring(f, tbuf1);
    if (flag && fixmemdb) {
      /* Fix the db in memory */
      flags = AL_FLAGS(list);
      atr_clr(i, AL_NAME(list), owner);
      atr_add(i, name, tbuf1, owner, flags);
      list = atr_get_noparent(i, name);
      AL_FLAGS(list) = flags;
    }
  }
  fprintf(f, "<\n");
  return 0;
}

dbref
db_paranoid_write(f, flag)
    FILE *f;
    int flag;			/* 1 = debug, 0 = normal */
{
  dbref i;
  int dbflag;

/* print a header line to make a later conversion to 2.0 easier to do.
 * the odd choice of numbers is based on 256*x + 2 offset
 */
  dbflag = 5;
  dbflag += DBF_NO_CHAT_SYSTEM;
#ifdef USE_WARNINGS
  dbflag += DBF_WARNINGS;
#endif
#ifdef CREATION_TIMES
  dbflag += DBF_CREATION_TIMES;
#endif
#ifndef ADD_POWERS
#ifdef DELETE_POWERS
  dbflag += DBF_NO_POWERS;
#endif
#endif
  dbflag += DBF_NEW_LOCKS;
#ifdef QUOTE_STRINGS
  dbflag += DBF_NEW_STRINGS;
#endif
  dbflag += DBF_TYPE_GARBAGE;
  dbflag += DBF_SPLIT_IMMORTAL;
  dbflag += DBF_NO_TEMPLE;
#ifdef DUMP_LESS_GARBAGE
  dbflag += DBF_LESS_GARBAGE;
#endif
  dbflag += DBF_AF_VISUAL;
  dbflag += DBF_VALUE_IS_COST;

  fprintf(f, "+V%d\n", dbflag * 256 + 2);

  fprintf(checklog_fp, "PARANOID WRITE BEGINNING...\n");
  fflush(checklog_fp);

  /* print total number of objects at top of file */
  fprintf(f, "~%d\n", db_top);

  /* write out each object */
  for (i = 0; i < db_top; i++) {
#ifdef WIN32
    /* Keep the service manager happy */
    if (shutdown_flag && (i & 0xFF) == 0)
      shutdown_checkpoint();
#endif
#ifdef DUMP_LESS_GARBAGE
    if (IsGarbage(i))
      continue;
#endif
    fprintf(f, "!%d\n", i);
    db_paranoid_write_object(f, i, flag);
    /* print out a message every so many objects */
    if (i % paranoid_checkpt == 0) {
      fprintf(checklog_fp, "\t...wrote up to object #%d\n", i);
      fflush(checklog_fp);
    }
  }
  fputs("***END OF DUMP***\n", f);
  fprintf(checklog_fp, "\t...finished at object #%d\n", i - 1);
  fprintf(checklog_fp, "END OF PARANOID WRITE.\n");
  fflush(checklog_fp);
  return (db_top);
}


long
getref(f)
    FILE *f;
{
  static char buf[BUFFER_LEN];
  fgets(buf, sizeof(buf), f);
  return (atol(buf));
}

#ifdef QUOTE_STRINGS

const char *
getstring_noalloc(f)
    FILE *f;
{
  static char buf[BUFFER_LEN];
  char *p;
  char c;

  p = buf;
  c = fgetc(f);
  if (!(indb_flags & DBF_NEW_STRINGS) || (c != '"')) {
    for (;;) {
      if ((c == '\0') || (c == EOF) ||
	  ((c == '\n') && ((p == buf) || (p[-1] != '\r')))) {
	*p = '\0';
	return buf;
      }
      safe_chr(c, buf, &p);
      c = fgetc(f);
    }
  } else {
    for (;;) {
      c = fgetc(f);
      if (c == '"') {
	if ((c = fgetc(f)) != '\n')
	  ungetc(c, f);
	*p = '\0';
	return buf;
      } else if (c == '\\') {
	c = fgetc(f);
      }
      if ((c == '\0') || (c == EOF)) {
	*p = '\0';
	return buf;
      }
      safe_chr(c, buf, &p);
    }
  }
}
#else				/* !QUOTE_STRINGS */
#ifndef OLD_NEWLINES
const char *
getstring_noalloc(f)
    FILE *f;
{
  static char buf[BUFFER_LEN];
  char *p;
  char c, lastc;
  int starting_quote = 0;

  p = buf;
  c = '\0';

  if (indb_flags & DBF_NEW_STRINGS) {
    c = fgetc(f);		/* Get the initial quote */
    if (c == '"')
      starting_quote = 1;
    else
      ungetc(c, f);
  }
  for (;;) {

    lastc = c;
    c = fgetc(f);

    /* if EOF or null, return */
    if ((c == '\0') || (c == EOF)) {
      *p = '\0';
      return buf;
    }
    /* if it's a newline, return if prior char is not a carriage return.
     * "\r\n" sequences are created by using the %r substitution, and
     * thus may occur in the middle of a buffer.
     */
    if (starting_quote) {
      if (c == '"') {
	fgetc(f);		/* Get the newline */
	*p = '\0';
	return buf;
      } else if (c == '\\') {
	c = fgetc(f);
      }
    } else {
      if ((c == '\n') && (lastc != '\r')) {
	*p = '\0';
	return buf;
      }
    }
    /* copy it in */
    safe_chr(c, buf, &p);
  }
}

#else
const char *
getstring_noalloc(f)
    FILE *f;
{
  static char buf[2 * BUFFER_LEN];
  char *p;
  fgets(buf, sizeof(buf), f);
  for (p = buf; *p; p++) {
    if (*p == '\n') {
      *p = '\0';
      break;
    }
  }
  return buf;
}
#endif				/* OLD_NEWLINES */
#endif				/* !QUOTE_STRINGS */

#define getstring(x,p) {p=NULL; SET(p,getstring_noalloc(x));}
#define getstring_compress(x,p)  {p=NULL; SETC(p,getstring_noalloc(x));}

static struct boolexp *
getboolexp1(f)
    FILE *f;
{
  struct boolexp *b;
  int c;
  char tbuf1[BUFFER_LEN], tbuf2[BUFFER_LEN];

  c = getc(f);
  switch (c) {
  case '\n':
    ungetc(c, f);
    return TRUE_BOOLEXP;
    /* break; */
  case EOF:
    fprintf(stderr, "ERROR: Unexpected EOF in boolexp.  Object #%d\n",
	    errobj);
    return TRUE_BOOLEXP;
    /*NOTREACHED */
    break;
  case '(':
    b = alloc_bool();
    if ((c = getc(f)) == NOT_TOKEN) {
      b->type = BOOLEXP_NOT;
      b->sub1 = getboolexp1(f);
      if (getc(f) != ')')
	goto error;
      return b;
    } else if (c == IS_TOKEN) {
      b->type = BOOLEXP_IS;
      b->sub1 = getboolexp1(f);
      if (getc(f) != ')')
	goto error;
      return b;
    } else if (c == IN_TOKEN) {
      b->type = BOOLEXP_CARRY;
      b->sub1 = getboolexp1(f);
      if (getc(f) != ')')
	goto error;
      return b;
    } else if (c == OWNER_TOKEN) {
      b->type = BOOLEXP_OWNER;
      b->sub1 = getboolexp1(f);
      if (getc(f) != ')')
	goto error;
      return b;
    } else if (c == AT_TOKEN) {
      b->type = BOOLEXP_IND;
      b->sub1 = getboolexp1(f);
      if (getc(f) != ')')
	goto error;
      return b;
    } else {
      ungetc(c, f);
      b->sub1 = getboolexp1(f);
      switch (c = getc(f)) {
      case AND_TOKEN:
	b->type = BOOLEXP_AND;
	break;
      case OR_TOKEN:
	b->type = BOOLEXP_OR;
	break;
      default:
	goto error;
	/* break */
      }
      b->sub2 = getboolexp1(f);
      if (getc(f) != ')')
	goto error;
      return b;
    }
    /* break; */
  case '-':
    /* obsolete NOTHING key */
    /* eat it */
    while ((c = getc(f)) != '\n')
      if (c == EOF) {
	fprintf(stderr, "ERROR: Unexpected EOF in boolexp. Object #%d\n",
		errobj);
	return TRUE_BOOLEXP;
      }
    ungetc(c, f);
    return TRUE_BOOLEXP;
    /* break */
#ifdef QUOTE_STRINGS
  case '"':
    /* Either a BOOLEXP_ATR or a BOOLEXP_EVAL */
    ungetc(c, f);
    strcpy(tbuf1, getstring_noalloc(f));
    c = fgetc(f);
    if (c == EOF) {
      fprintf(stderr, "ERROR: Unexpected EOF in boolexp. Object #%d\n",
	      errobj);
      return TRUE_BOOLEXP;
    }
    b = alloc_bool();
    b->type = (c == ':') ? BOOLEXP_ATR : BOOLEXP_EVAL;
    b->atr_lock = alloc_atr(tbuf1, (char *) getstring_noalloc(f));
    return b;
#endif
  default:
    /* can be either a dbref or : seperated string */
    ungetc(c, f);
    b = alloc_bool();
    b->type = BOOLEXP_CONST;
    b->thing = 0;

    /* constant dbref */
    if (isdigit(c = getc(f))) {
      while (isdigit(c)) {
	b->thing = b->thing * 10 + c - '0';
	c = getc(f);
      }
      switch (c) {
      case ':':		/* old style boolexp lock */
	{
	  char *p;

	  for (p = tbuf1;
	       ((c = getc(f)) != EOF) && (c != '\n') && (c != ')');
	       *p++ = c) ;
	  *p = '\0';
	  if (c == EOF)
	    goto error;
	  b->atr_lock = alloc_atr(convert_atr(b->thing), tbuf1);
	  b->thing = 0;
	  b->type = BOOLEXP_ATR;
	  /* this is only needed because of the braindeath of the previous
	   * version of atrlocks.. bleah.
	   */
	  if (c == '\n')
	    return (b);
	}
      default:
	ungetc(c, f);
	break;
      }
      return (b);
    } else {
      /* we indulge in a bit of a kludge. We either have a colon
       * separated string (attribute lock) or a slash-separate string
       * (eval lock).
       */
      char *p = tbuf1, *s;
      char savec;

      *p++ = c;
      for (;
       ((c = getc(f)) != EOF) && (c != '\n') && (c != ':') && (c != '/');
	   *p++ = c) ;
      *p = '\0';
      if ((c == EOF) || (c == '\n'))
	goto error;
      if ((c != ':') && (c != '/'))
	goto error;
      savec = c;
      for (s = tbuf2;
	   ((c = getc(f)) != EOF) && (c != '\n') && (c != ')' &&
				    (c != OR_TOKEN) && (c != AND_TOKEN));
	   *s++ = c) ;
      if (c == EOF)
	goto error;
      *s++ = 0;
      ungetc(c, f);
      b->atr_lock = alloc_atr(tbuf1, tbuf2);
      if (savec == ':')
	b->type = BOOLEXP_ATR;
      else
	b->type = BOOLEXP_EVAL;
      return (b);
    }
  }
error:
  fprintf(stderr, "ERROR: Unknown error in boolexp. Object #%d\n", errobj);
  return TRUE_BOOLEXP;
}

struct boolexp *
getboolexp(f)
    FILE *f;
{
  struct boolexp *b;
  b = getboolexp1(f);
  if (getc(f) != '\n') {
    fprintf(stderr, "ERROR: Invalid boolexp format on object #%d\n", errobj);
    return TRUE_BOOLEXP;
  }
  return b;
}

void
getlocks(i, f)
    dbref i;
    FILE *f;
{
  /* Assumes it begins at the beginning of a line. */
  int c;
  struct boolexp *b;
  char buf[BUFFER_LEN], *p;
  while ((c = getc(f)), c != EOF && c == '_') {
    p = buf;
    while ((c = getc(f)), c != EOF && c != '|') {
      *p++ = c;
    }
    *p = '\0';
    if (c == EOF || (p - buf == 0)) {
      fprintf(stderr, "ERROR: Invalid lock format on object #%d\n", i);
      return;
    }
    b = getboolexp(f);		/* Which will clobber a '\n' */
    if (b == TRUE_BOOLEXP) {
      /* getboolexp() would already have complained. */
      return;
    } else {
      add_lock(i, buf, b);
    }
  }
  ungetc(c, f);
  return;
}

void
get_old_locks(i, f)
    dbref i;
    FILE *f;
{
  /* To review the format: There are three boolexps in order, with no
   * identifiers. They are these:
   * Basic Lock
   * Enter/Tport/Zone Lock
   * Use/Page Lock.
   * Most of the subtlety of this routine comes from deciding what locks
   * to set when locks have been overloaded.
   */
  /* Oops. When we read the locks in, we do not know what the flags of
   * the object are, so we have no grounds for subtle deduction about
   * what locks are appropriate. Therefore, all the subtlety is relegated
   * to the tweak_locks() routine.
   */
  struct boolexp *basic, *enter, *use;
  basic = getboolexp(f);
  if (basic != TRUE_BOOLEXP) {
    add_lock(i, Basic_Lock, basic);
  }
  use = getboolexp(f);
  if (use != TRUE_BOOLEXP) {
    add_lock(i, Use_Lock, use);
  }
  enter = getboolexp(f);
  if (enter != TRUE_BOOLEXP) {
    add_lock(i, Enter_Lock, enter);
  }
}

  /* This should be called after a whole database with DBF_NEW_LOCKS not
   * set is read in, to do some post-processing on the locks.
   */
static void
tweak_locks()
{
  int i;
  struct boolexp *b1, *b2;
  for (i = 0; i < db_top; i++) {
    switch (Typeof(i)) {
    case TYPE_ROOM:
      /* Turn elocks into tport locks on rooms. */
      b1 = getlock(i, Enter_Lock);
      if (b1 != TRUE_BOOLEXP) {
	b2 = dup_bool(b1);
	delete_lock(i, Enter_Lock);
	add_lock(i, Tport_Lock, b2);
      }
      break;
    case TYPE_PLAYER:
      /* Clone the uselock to get a pagelock for players. */
      b1 = getlock(i, Use_Lock);
      if (b1 != TRUE_BOOLEXP) {
	b2 = dup_bool(b1);
	/* Do not delete the uselock. */
	add_lock(i, Page_Lock, b2);
      }
      if (ZMaster(i)) {
	/* Copy the elock to get a Zone Lock. */
	b1 = getlock(i, Enter_Lock);
	if (b1 != TRUE_BOOLEXP) {
	  b2 = dup_bool(b1);
	  add_lock(i, Zone_Lock, b2);
	}
      }
      break;
    default:
      break;
    }
    if (Zone(i) != NOTHING) {
      /* If Zone(i) has an enter lock but does not have a zone lock,
       * clone the enter lock to get a zone lock.
       */
      if (getlock(Zone(i), Enter_Lock) != TRUE_BOOLEXP &&
	  getlock(Zone(i), Zone_Lock) == TRUE_BOOLEXP) {
	b1 = getlock(Zone(i), Enter_Lock);
	b2 = dup_bool(b1);
	add_lock(Zone(i), Zone_Lock, b2);
      }
    }
  }
}

void
free_boolexp(b)
    struct boolexp *b;
{
  if (b != TRUE_BOOLEXP && b) {
    switch (b->type) {
    case BOOLEXP_AND:
    case BOOLEXP_OR:
      free_boolexp(b->sub1);
      free_boolexp(b->sub2);
      free_bool(b);
      break;
    case BOOLEXP_NOT:
    case BOOLEXP_CARRY:
    case BOOLEXP_IND:
    case BOOLEXP_IS:
      free_boolexp(b->sub1);
      free_bool(b);
      break;
    case BOOLEXP_CONST:
      free_bool(b);
      break;
    case BOOLEXP_ATR:
    case BOOLEXP_EVAL:
      if (b->atr_lock->name)
	mush_free((Malloc_t) b->atr_lock->name, "boolatr_name");
      if (b->atr_lock->text)
	mush_free((Malloc_t) b->atr_lock->text, "boolatr_text");
      if (b->atr_lock)
	mush_free((Malloc_t) b->atr_lock, "boolatr");
      free_bool(b);
      break;
    }
  }
}

struct boolexp *
dup_bool(b)
    struct boolexp *b;
{
  struct boolexp *r;
  if (b == TRUE_BOOLEXP)
    return (TRUE_BOOLEXP);
  r = alloc_bool();
  switch (r->type = b->type) {
  case BOOLEXP_AND:
  case BOOLEXP_OR:
    r->sub2 = dup_bool(b->sub2);
  case BOOLEXP_NOT:
  case BOOLEXP_IND:
  case BOOLEXP_IS:
  case BOOLEXP_CARRY:
  case BOOLEXP_OWNER:
    r->sub1 = dup_bool(b->sub1);
  case BOOLEXP_CONST:
    r->thing = b->thing;
    break;
  case BOOLEXP_ATR:
  case BOOLEXP_EVAL:
    r->atr_lock = alloc_atr(b->atr_lock->name, uncompress(b->atr_lock->text));
    break;
  default:
    fprintf(stderr, "ERROR: bad bool type in dup_bool!\n");
    return (TRUE_BOOLEXP);
  }
  return (r);
}

void
db_free()
{
  dbref i;
  struct object *o;

  if (db) {

    for (i = 0; i < db_top; i++) {
      o = REFDB(i);
      SET(o->name, NULL);
      atr_free(i);
      free_locks(Locks(i));
    }

    free((char *) db);
    db = NULL;
    db_init = db_top = '\0';
  }
}

/* read attribute list */
int
get_list(f, i)
    FILE *f;
    dbref i;
{
  int c;
  char *p, *q;
  char tbuf1[BUFFER_LEN];
  int flags;

#ifdef OLD_NEWLINES
  char nextc;
#endif				/* OLD_NEWLINES */

  List(i) = NULL;
  tbuf1[0] = '\0';
  while (1)
    switch (c = getc(f)) {
    case ']':			/* new style attribs, read name then value */
      strcpy(tbuf1, getstring_noalloc(f));
      if (!(p = (char *) index(tbuf1, '^'))) {
	fprintf(stderr, "ERROR: Bad format on new attributes. object #%d\n",
		i);
	return 0;
      }
      *p++ = '\0';
      if (!(q = (char *) index(p, '^'))) {
	fprintf(stderr, "ERROR: Bad format on new attribute %s. object #%d\n",
		tbuf1, i);
	return 0;
      }
      *q++ = '\0';
      flags = atoi(q);
      /* Remove obsolete AF_NUKED flag, just in case */
      flags &= ~AF_NUKED;
      if (!(indb_flags & DBF_AF_VISUAL)) {
	/* Remove AF_ODARK flag. If it wasn't there, set AF_VISUAL */
	if (!(flags & AF_ODARK))
	  flags |= AF_VISUAL;
	flags &= ~AF_ODARK;
      }
      /* We add the attribute assuming that atoi(p) is an ok dbref
       * since we haven't loaded the whole db and can't really tell
       * if it is or not. We'll fix this up at the end of the load 
       */
      atr_new_add(i, tbuf1, (char *) getstring_noalloc(f), atoi(p), flags);
      /* Check removed for atoi(q) == 0  (which results in NOTHING for that
       * parameter, and thus no flags), since this eliminates 'visual'
       * attributes (which, if not built-in attrs, have a flag val of 0.)
       */
#ifdef OLD_NEWLINES
      /* hack to check if there's an unexpected '\n' that got in */
      nextc = getc(f);
      if ((nextc != ']') && (nextc != '>') && (nextc != '<') &&
	  (nextc != '!')) {
	/* we have a problem. print error */
	fprintf(stderr,
	   "** WARNING **  Hard newline in attribute %s on object #%d\n",
		tbuf1, i);
	/* throw out everything until next good attrib or object.
	 * we can tell a good attrib or object by looking for a \n
	 * followed by a ], >, <, or !. Otherwise, we can be "spoofed"
	 */
	do {
	  while ((nextc = getc(f)) != '\n') ;
	  nextc = getc(f);	/* go past EOL */
	} while ((nextc != ']') && (nextc != '>') && (nextc != '<') &&
		 (nextc != '!'));
      }
      /* we've eaten a character, so go back. */
      ungetc(nextc, f);
#endif				/* OLD_NEWLINES */
      break;
    case '>':			/* old style attribs, read # then value */
      strcpy(tbuf1, convert_atr(getref(f)));
      atr_new_add(i, tbuf1, (char *) getstring_noalloc(f), Owner(i), NOTHING);
      break;
    case '<':			/* end of list */
      if ('\n' != getc(f)) {
	fprintf(stderr, "ERROR: no line feed after < on object %d\n", i);
	return (0);
      }
      return (1);
    default:
      if (c == EOF) {
	fprintf(stderr, "ERROR: Unexpected EOF on file.\n");
	return (0);
      }
      fprintf(stderr, "ERROR: Bad character %c (%d) in attribute list on object %d\n", c, c, i);
      fprintf(stderr, "  (expecting ], >, or < as first character of the line.)\n");
      if (*tbuf1)
	fprintf(stderr, "  Last attribute read was: %s\n", tbuf1);
      else
	fprintf(stderr, "  No attributes had been read yet.\n");
      return (0);
    }
}



dbref
db_read(f)
    FILE *f;
{
  int c;
  dbref i;
  struct object *o;
  const char *end;
#ifdef ADD_POWERS
  int flags, toggles, powers;
#endif
  int temp = 0;
  time_t temp_time = 0;

  clear_players();
  db_free();
  indb_flags = 1;
  for (i = 0;; i++) {
    /* Loop invariant: we always begin at the beginning of a line. */
    errobj = i;
    c = getc(f);
    switch (c) {
      /* make sure database is at least this big *1.5 */
    case '~':
      db_init = (getref(f) * 3) / 2;
      break;
      /* Use the MUSH 2.0 header stuff to see what's in this db */
    case '+':
      c = getc(f);		/* Skip the V */
      indb_flags = ((getref(f) - 2) / 256) - 5;
      break;
      /* old fashioned database */
    case '#':
      fprintf(stderr, "ERROR: old style database.\n");
      /* if you want to read in an old-style database, use an earlier
       * patchlevel to upgrade.
       */
      return -1;
      break;
      /* new database */
    case '!':			/* non-zone oriented database */
    case '&':			/* zone oriented database */
      /* make space */
      i = getref(f);
      db_grow(i + 1);
      /* read it in */
      o = db + i;
      getstring(f, o->name);
      o->location = getref(f);
      /* --- get zone number --- */
      (c == '&') ? (int) getref(f) : 0;
      /* ----------------------- */
      o->contents = getref(f);
      o->exits = getref(f);
      o->next = getref(f);
      o->parent = getref(f);
      if (indb_flags & DBF_NEW_LOCKS) {
	o->locks = NULL;
	getlocks(i, f);
      } else {
	o->locks = NULL;
	get_old_locks(i, f);
      }
      o->owner = getref(f);
      o->zone = getref(f);
      s_Pennies(i, getref(f));
      o->flags = getref(f);
      /* We need to have flags in order to do this right, which is why
       * we waited until now
       */
      if (!(indb_flags & DBF_VALUE_IS_COST) && IsThing(i))
	s_Pennies(i, (Pennies(i) + 1) * 5);
      o->toggles = getref(f);
      if (!(indb_flags & DBF_NO_POWERS))
	o->powers = getref(f);
      else
	o->powers = 0;
      /* Split the immortal power into NO_PAY, NO_QUOTA, and UNKILLABLE
       * Remove the CAN_DEBUG power, too.
       */
      if (!(indb_flags & DBF_SPLIT_IMMORTAL)) {
	o->powers &= ~CAN_DEBUG;
	if (o->powers & IMMORTAL) {
	  o->powers &= ~IMMORTAL;
	  o->powers |= NO_PAY | NO_QUOTA | UNKILLABLE;
	}
      }
      /* Munge the representation of Garbage objects. */
      if (!(indb_flags & DBF_TYPE_GARBAGE)) {
	if ((o->flags & ~ACCESSED) == (TYPE_THING | GOING)) {
	  /* Old-style garbage object. Convert to new-style. */
	  o->flags = TYPE_GARBAGE;
	}
      }
      /* Remove the TEMPLE flag */
      if (!(indb_flags & DBF_NO_TEMPLE)) {
	if ((o->flags & TYPE_MASK) == TYPE_ROOM)
	  o->toggles &= ~ROOM_TEMPLE;
      }
      /* Clear the GOING flags. If it was scheduled for destruction
       * when the db was saved, it gets a reprieve.
       */
      o->flags &= ~GOING;
      o->flags &= ~GOING_TWICE;

      /* If there are channels in the db, read 'em in */
      /* We don't support this anymore, so we just discard them */
      if (!(indb_flags & DBF_NO_CHAT_SYSTEM))
	temp = getref(f);
      else
	temp = 0;
#ifdef CHAT_SYSTEM
      o->channels = NULL;
#endif				/* CHAT_SYSTEM */

      /* If there are warnings in the db, read 'em in */
      temp = MAYBE_GET(f, DBF_WARNINGS);
#ifdef USE_WARNINGS
      o->warnings = temp;
#endif
      /* If there are creation times in the db, read 'em in */
      temp_time = MAYBE_GET(f, DBF_CREATION_TIMES);
#ifdef CREATION_TIMES
      if (temp_time)
	o->creation_time = (time_t) temp_time;
      else
	o->creation_time = time((time_t *) 0);
#endif
      temp_time = MAYBE_GET(f, DBF_CREATION_TIMES);
#ifdef CREATION_TIMES
      if (temp_time || IsPlayer(i))
	o->modification_time = (time_t) temp_time;
      else
	o->modification_time = o->creation_time;
#endif

      /* read attribute list for item */
      if (!get_list(f, i)) {
	fprintf(stderr, "ERROR: bad attribute list object %d\n", i);
	return -1;
      }
      /* check to see if it's a player */
      if (IsPlayer(i)) {
	add_player(i, NULL);
	Toggles(i) &= ~PLAYER_CONNECT;
#ifdef ADD_POWERS
	Flags(i) &= ~0x200;	/* zap old connect flag, too */
#endif
      }
#ifdef LOCAL_DATA
      o->local_data = NULL;
#endif
      break;

    case '*':
      end = getstring_noalloc(f);
      if (!strcmp(end, "***END OF DUMP***")) {
	fprintf(stderr, "ERROR: No end of dump %d\n", i);
	return -1;
      } else {
	{
	  fprintf(stderr, "READING: done\n");
	  fix_free_list();
	  if (!(indb_flags & DBF_NEW_LOCKS))
	    tweak_locks();
	  dbck();
	  return db_top;
	}
      }
    default:
      fprintf(stderr, "ERROR: failed object %d\n", i);
      return -1;
    }
  }
}
