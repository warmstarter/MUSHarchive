/* db.c */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <stdlib.h>
#include <setjmp.h>
#include "conf.h"
#include "mushdb.h"
#include "attrib.h"
#include "externs.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "mymalloc.h"
#include "game.h"
#include "flags.h"
#include "lock.h"
#include "dbdefs.h"
#include "log.h"
#include "strtree.h"
#include "parse.h"
#include "htab.h"
#include "confmagic.h"

#ifdef WIN32
void shutdown_checkpoint(void);
#endif

extern jmp_buf db_err;
#define OUTPUT(fun) do { if ((fun) < 0) longjmp(db_err, 1); } while (0)

#define MAYBE_GET(f,x) \
        (indb_flags & (x)) ? getref(f) : 0

extern int paranoid_checkpt;	/* from game.c */

long indb_flags;		/* What flags are in the db we read at startup? */

int loading_db = 0;

struct object *db = 0;
dbref db_top = 0;

dbref errobj;

const char *EOD = "***END OF DUMP***\n";

#ifndef DB_INITIAL_SIZE
#define DB_INITIAL_SIZE 5000
#endif				/* DB_INITIAL_SIZE */

dbref db_size = DB_INITIAL_SIZE;

HASHTAB htab_objdata;
HASHTAB htab_objdata_keys;

extern char ccom[];

static void db_grow(dbref newtop);

#ifdef DUMP_OLD_LOCKS
static void putbool_subexp(FILE * f, struct boolexp *b);
#endif
static void db_write_obj_basic(FILE * f, dbref i, struct object *o);
int db_paranoid_write_object(FILE * f, dbref i, int flag);
static struct boolexp *getboolexp1(FILE * f, const char *type);
void putlocks(FILE * f, lock_list *l);
void getlocks(dbref i, FILE * f);
void get_new_locks(dbref i, FILE * f);
int get_list(FILE * f, dbref i);
void db_free(void);
static void init_objdata_htab(int size);
static void db_write_flags(FILE * f);

StrTree object_names;
extern StrTree atr_names;

void init_names(void);

extern struct db_stat_info current_state;

/** Initialize the name strtree.
 */
void
init_names(void)
{
  st_init(&object_names);
}

/** Set an object's name through the name strtree.
 * We maintain object names in a strtree because many objects have
 * the same name (cardinal exits, weapons and armor, etc.)
 * This function is used to set an object's name; if the name's already
 * in the strtree, we just get a pointer to it, saving memory.
 * (If not, we add it to the strtree and use that pointer).
 * \param obj dbref of object whose name is to be set.
 * \param newname name to set on the object, or NULL to clear the name.
 * \return object's new name, or NULL if none is given.
 */
const char *
set_name(dbref obj, const char *newname)
{
  /* if pointer not null unalloc it */
  if (Name(obj))
    st_delete(Name(obj), &object_names);
  if (!newname || !*newname)
    return NULL;
  Name(obj) = st_insert(newname, &object_names);
  return Name(obj);
}

int db_init = 0;

static void
db_grow(dbref newtop)
{
  struct object *newdb;
  dbref initialized;
  struct object *o;

  if (newtop > db_top) {
    initialized = db_top;
    current_state.total = newtop;
    current_state.garbage += newtop - db_top;
    db_top = newtop;
    if (!db) {
      /* make the initial one */
      db_size = (db_init) ? db_init : DB_INITIAL_SIZE;
      while (db_top > db_size)
	db_size *= 2;
      if ((db = (struct object *)
	   malloc(db_size * sizeof(struct object))) == NULL) {
	do_rawlog(LT_ERR, "ERROR: out of memory while creating database!");
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
	do_rawlog(LT_ERR, "ERROR: out of memory while extending database!");
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
      o->type = TYPE_GARBAGE;
      o->flags = NULL;
      o->powers = 0;
      o->warnings = 0;
      o->modification_time = o->creation_time = mudtime;
      o->attrcount = 0;
      initialized++;
    }
  }
}

/** Allocate a new object structure.
 * This function allocates and returns a new object structure.
 * The caller must see that it gets appropriately typed and otherwise
 * initialized.
 * \return dbref of newly allocated object.
 */
dbref
new_object(void)
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
  o->type = TYPE_GARBAGE;
  o->flags = new_flag_bitmask();
  o->powers = 0;
  o->warnings = 0;
  o->modification_time = o->creation_time = mudtime;
  o->attrcount = 0;
  if (current_state.garbage)
    current_state.garbage--;
  return newobj;
}

/** Output a long int to a file.
 * \param f file pointer to write to.
 * \param ref value to write.
 */
void
putref(FILE * f, long int ref)
{
  OUTPUT(fprintf(f, "%ld\n", ref));
}

/** Output a string to a file.
 * This function writes a string to a file, double-quoted, 
 * appropriately escaping quotes and backslashes (the escape character).
 * \param f file pointer to write to.
 * \param s value to write.
 */
void
putstring(FILE * f, const char *s)
{
  OUTPUT(putc('"', f));
  while (*s) {
    switch (*s) {
    case '\\':
    case '"':
      OUTPUT(putc('\\', f));
      /* FALL THROUGH */
    default:
      OUTPUT(putc(*s, f));
    }
    s++;
  }
  OUTPUT(putc('"', f));
  OUTPUT(putc('\n', f));
}

#ifndef DUMP_OLD_LOCKS

static void
get_logical_line(FILE * f, char **label, char **value)
{
  static char lbuf[BUFFER_LEN], vbuf[BUFFER_LEN];
  static int line;
  int c;
  char *p;

  *label = lbuf;
  *value = vbuf;

  /* invariant: we start at the beginning of a line. */

  line++;

  do {
    while (isspace(c = getc(f))) {
      if (c == '\n')
	line++;
    }
    if (c == '#') {
      while ((c = getc(f)) != '\n' && c != EOF) {
	/* nothing */
      }
      if (c == '\n')
	line++;
    }
  } while (isspace(c));

  /* return empty strings on end of file */

  if (c == EOF) {
    *lbuf = '\0';
    *vbuf = '\0';
    return;
  }

  /* invariant: we should have the first character of a label in 'c'. */

  p = lbuf;
  do {
    if (c != '_' && c != '-' && c != '!' && c != '.' && c != '>' && c != '<' &&	/* these really should only be first time */
	!isalnum(c)) {
      do_rawlog(LT_ERR, "DB: Illegal character '%c'(%d) in label, line %d",
		c, c, line);
      do_rawlog(LT_ERR, "Aborting...");
      exit(1);
    }
    safe_chr(c, lbuf, &p);
    c = getc(f);
  } while (c != EOF && !isspace(c));
  *p++ = '\0';
  if (p >= lbuf + BUFFER_LEN)
    do_rawlog(LT_ERR, "DB: warning: very long label, line %d", line);

  /* suck up separating whitespace */
  while (c != '\n' && isspace(c))
    c = getc(f);

  /* check for presence of a value */
  if (c == EOF || c == '\n') {
    *vbuf = '\0';
    return;
  }

  /* invariant: we should have the first character of a value in 'c'. */

  p = vbuf;
  if (c == '"') {
    /* quoted string */
    int sline;
    sline = line;
    for (;;) {
      c = getc(f);
      if (c == '"')
	break;
      if (c == '\\')
	c = getc(f);
      if (c == EOF) {
	do_rawlog(LT_ERR, "DB: Unclosed quoted string starting on line %d",
		  sline);
	do_rawlog(LT_ERR, "Aborting...");
	exit(1);
      }
      if (c == '\0')
	do_rawlog(LT_ERR,
		  "DB: warning: null in quoted string, remainder lost, line %d",
		  line);
      if (c == '\n')
	line++;
      safe_chr(c, vbuf, &p);
    }
    do {
      c = getc(f);
      if (c != EOF && !isspace(c)) {
	do_rawlog(LT_ERR, "DB: Garbage after quoted string, line %d", line);
	do_rawlog(LT_ERR, "Aborting...");
	exit(1);
      }
    } while (c != '\n' && c != EOF);
  } else {
    /* non-quoted value */
    do {
      if (c != '_' && c != '-' && c != '!' && c != '.' &&
	  !isalnum(c) && !isspace(c)) {
	do_rawlog(LT_ERR, "DB: Illegal character '%c'(%d) in value, line %d",
		  c, c, line);
	do_rawlog(LT_ERR, "Aborting...");
	exit(1);
      }
      safe_chr(c, vbuf, &p);
      c = getc(f);
    } while (c != EOF && c != '\n');
  }
  *p++ = '\0';
  if (p >= vbuf + BUFFER_LEN)
    do_rawlog(LT_ERR, "DB: warning: very long value, line %d", line);

  /* note no line increment for final newline because of initial increment */
}

static void
db_write_label(FILE * f, char const *l)
{
  fputs(l, f);
  putc(' ', f);
}

static void
db_write_string(FILE * f, char const *s)
{
  putc('"', f);
  while (*s) {
    if (*s == '"' || *s == '\\')
      putc('\\', f);
    putc(*s, f);
    s++;			/* done separately because putc() is a macro */
  }
  putc('"', f);
}

static void
db_write_labeled_string(FILE * f, char const *label, char const *value)
{
  db_write_label(f, label);
  db_write_string(f, value);
  putc('\n', f);
}

static void
db_write_labeled_number(FILE * f, char const *label, int value)
{
  fprintf(f, "%s %d\n", label, value);
}

/** Write a boolexp to a file in unparsed (text) form.
 * \param f file pointer to write to.
 * \param b pointer to boolexp to write.
 */
void
putboolexp(FILE * f, struct boolexp *b)
{
  db_write_labeled_string(f, "key", unparse_boolexp(GOD, b, UB_DBREF));
}

/** Write a list of locks to a file.
 * \param f file pointer to write to.
 * \param l pointer to lock_list to write.
 */
void
putlocks(FILE * f, lock_list *l)
{
  lock_list *ll;
  int count = 0;
  for (ll = l; ll; ll = ll->next)
    count++;
  db_write_labeled_number(f, "lockcount", count);
  for (ll = l; ll; ll = ll->next) {
    db_write_labeled_string(f, "type", ll->type);
    db_write_labeled_number(f, "creator", L_CREATOR(ll));
    db_write_labeled_number(f, "flags", L_FLAGS(ll));
    putboolexp(f, ll->key);
    /* putboolexp adds a '\n', so we won't. */
  }
}


#else				/* DUMP_OLD_LOCKS */

static void
putbool_subexp(FILE * f, struct boolexp *b)
{
  switch (b->type) {
  case BOOLEXP_IS:
    OUTPUT(putc('(', f));
    OUTPUT(putc(IS_TOKEN, f));
    putbool_subexp(f, b->data.sub.a);
    OUTPUT(putc(')', f));
    break;
  case BOOLEXP_CARRY:
    OUTPUT(putc('(', f));
    OUTPUT(putc(IN_TOKEN, f));
    putbool_subexp(f, b->data.sub.b);
    OUTPUT(putc(')', f));
    break;
  case BOOLEXP_OWNER:
    OUTPUT(putc('(', f));
    OUTPUT(putc(OWNER_TOKEN, f));
    putbool_subexp(f, b->sub1);
    OUTPUT(putc(')', f));
    break;
  case BOOLEXP_IND:
    OUTPUT(putc('(', f));
    OUTPUT(putc(AT_TOKEN, f));
    putbool_subexp(f, b->sub1);
    OUTPUT(putc(')', f));
    break;
  case BOOLEXP_AND:
    OUTPUT(putc('(', f));
    putbool_subexp(f, b->sub1);
    OUTPUT(putc(AND_TOKEN, f));
    putbool_subexp(f, b->sub2);
    OUTPUT(putc(')', f));
    break;
  case BOOLEXP_OR:
    OUTPUT(putc('(', f));
    putbool_subexp(f, b->sub1);
    OUTPUT(putc(OR_TOKEN, f));
    putbool_subexp(f, b->sub2);
    OUTPUT(putc(')', f));
    break;
  case BOOLEXP_NOT:
    OUTPUT(putc('(', f));
    OUTPUT(putc(NOT_TOKEN, f));
    putbool_subexp(f, b->sub1);
    OUTPUT(putc(')', f));
    break;
  case BOOLEXP_CONST:
    OUTPUT(fprintf(f, "%d", b->thing));
    break;
  case BOOLEXP_ATR:
    putstring(f, b->data.atr_lock->name);
    OUTPUT(putc(':', f));
    putstring(f, uncompress(b->data.atr_lock->text));
    break;
  case BOOLEXP_EVAL:
    putstring(f, b->data.atr_lock->name);
    OUTPUT(putc('/', f));
    putstring(f, uncompress(b->data.atr_lock->text));
    break;
  default:
    break;
  }
}

void
putboolexp(FILE * f, struct boolexp *b)
{
  if (b != TRUE_BOOLEXP) {
    putbool_subexp(f, b);
  }
  OUTPUT(putc('\n', f));
}

void
putlocks(FILE * f, struct lock_list *l)
{
  struct lock_list *ll;
  for (ll = l; ll; ll = ll->next) {
    OUTPUT(putc('_', f));
    OUTPUT(fputs(ll->type, f));
    OUTPUT(putc('|', f));
    putboolexp(f, ll->key);
    /* putboolexp adds a '\n', so we won't. */
  }
}

#endif

/** Write out the basics of an object.
 * This function writes out the basic information associated with an
 * object - just about everything but the attributes.
 * \param f file pointer to write to.
 * \param i dbref of object to write.
 * \param o pointer to object to write.
 */
static void
db_write_obj_basic(FILE * f, dbref i, struct object *o)
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
  putref(f, Typeof(i));
  putstring(f, bits_to_string(o->flags, GOD, NOTHING));
  putref(f, o->powers);
  /* Original chat system wrote chat channels to the db here. 
   * That's no longer done, they're written to chatdb 
   */
  putref(f, o->warnings);
  putref(f, o->creation_time);
  putref(f, o->modification_time);
}

/** Write out an object.
 * This function writes a single object out to a file.
 * \param f file pointer to write to.
 * \param i dbref of object to write.
 */
int
db_write_object(FILE * f, dbref i)
{
  struct object *o;
  ALIST *list;
  o = db + i;
  db_write_obj_basic(f, i, o);

  /* write the attribute list */
  for (list = o->list; list; list = AL_NEXT(list)) {
    if (AL_FLAGS(list) & AF_NODUMP)
      continue;
    OUTPUT(fprintf(f, "]%s^%d^%d\n", AL_NAME(list), Owner(AL_CREATOR(list)),
		   AL_FLAGS(list)));
    putstring(f, uncompress(AL_STR(list)));
  }
  OUTPUT(fprintf(f, "<\n"));
  return 0;
}

/** Write out the object database to disk.
 * \verbatim
 * This function writes the databsae out to disk. The database
 * structure currently looks something like this:
 * +V<header line>
 * +FLAGS LIST
 * <flag data>
 * ~<number of objects>
 * <object data>
 * \endverbatim
 * \param f file pointer to write to.
 * \param flag 0 for normal dump, DBF_PANIC for panic dumps.
 * \return the number of objects in the database (db_top)
 */
dbref
db_write(FILE * f, int flag)
{
  dbref i;
  int dbflag;

  /* print a header line to make a later conversion to 2.0 easier to do.
   * the odd choice of numbers is based on 256*x + 2 offset
   * The original PennMUSH had x=5 (chat) or x=6 (nochat), and Tiny expects
   * to deal with that. We need to use some extra flags as well, so
   * we may be adding to 5/6 as needed, using successive binary numbers.
   */
  dbflag = 5 + flag;
  dbflag += DBF_NO_CHAT_SYSTEM;
  dbflag += DBF_WARNINGS;
  dbflag += DBF_CREATION_TIMES;
  dbflag += DBF_SPIFFY_LOCKS;
  dbflag += DBF_NEW_STRINGS;
  dbflag += DBF_TYPE_GARBAGE;
  dbflag += DBF_SPLIT_IMMORTAL;
  dbflag += DBF_NO_TEMPLE;
  dbflag += DBF_LESS_GARBAGE;
  dbflag += DBF_AF_VISUAL;
  dbflag += DBF_VALUE_IS_COST;
  dbflag += DBF_LINK_ANYWHERE;
  dbflag += DBF_NO_STARTUP_FLAG;
  dbflag += DBF_AF_NODUMP;
  dbflag += DBF_NEW_FLAGS;

  OUTPUT(fprintf(f, "+V%d\n", dbflag * 256 + 2));
  db_write_flags(f);

  OUTPUT(fprintf(f, "~%d\n", db_top));
  for (i = 0; i < db_top; i++) {
#ifdef WIN32
    /* Keep the service manager happy */
    if (shutdown_flag && (i & 0xFF) == 0)
      shutdown_checkpoint();
#endif
    if (IsGarbage(i))
      continue;
    OUTPUT(fprintf(f, "!%d\n", i));
    db_write_object(f, i);
  }
  OUTPUT(fputs(EOD, f));
  return (db_top);
}

static void
db_write_flags(FILE * f)
{
  OUTPUT(fprintf(f, "+FLAGS LIST\n"));
  flag_write_all(f);
}


/** Write out an object, in paranoid fashion.
 * This function writes a single object out to a file in paranoid
 * mode, which warns about several potential types of corruption,
 * and can fix some of them.
 * \param f file pointer to write to.
 * \param i dbref of object to write.
 * \param flag 1 = debug, 0 = normal
 */
int
db_paranoid_write_object(FILE * f, dbref i, int flag)
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
  /* fflush(f); */

  /* write the attribute list, scanning */
  for (list = o->list; list; list = next) {
    next = AL_NEXT(list);
    if (AL_FLAGS(list) & AF_NODUMP)
      continue;
    fixmemdb = err = 0;
    /* smash unprintable characters in the name, replace with ! */
    strcpy(name, AL_NAME(list));
    for (p = name; *p; p++) {
      if (!isprint((unsigned char) *p) || isspace((unsigned char) *p)) {
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
      do_rawlog(LT_CHECK,
		T(" * Bad attribute name on #%d. Changing name to %s.\n"),
		i, name);
      err = 0;
    }
    /* check the owner */
    owner = AL_CREATOR(list);
    if (!GoodObject(owner)) {
      do_rawlog(LT_CHECK, T(" * Bad owner on attribute %s on #%d.\n"), name, i);
      owner = GOD;
      fixmemdb = 1;
    } else {
      owner = Owner(owner);
    }

    /* write that info out */
    OUTPUT(fprintf(f, "]%s^%d^%d\n", name, (int) owner, AL_FLAGS(list)));

    /* now check the attribute */
    strcpy(tbuf1, uncompress(AL_STR(list)));
    /* get rid of unprintables and hard newlines */
    lastp = '\0';
    for (p = tbuf1; *p; p++) {
      if (!isprint((unsigned char) *p)) {
	if (!isspace((unsigned char) *p)) {
	  *p = '!';
	  err = 1;
	}
      }
      lastp = *p;
    }
    if (err) {
      fixmemdb = 1;
      do_rawlog(LT_CHECK,
		T(" * Bad text in attribute %s on #%d. Changed to:\n"), name,
		i);
      do_rawlog(LT_CHECK, "%s\n", tbuf1);
    }
    putstring(f, tbuf1);
    if (flag && fixmemdb) {
      /* Fix the db in memory */
      flags = AL_FLAGS(list);
      atr_clr(i, AL_NAME(list), owner);
      (void) atr_add(i, name, tbuf1, owner, flags);
      list = atr_get_noparent(i, name);
      AL_FLAGS(list) = flags;
    }
  }
  OUTPUT(fprintf(f, "<\n"));
  return 0;
}



/** Write out the object database to disk, in paranoid mode.
 * \verbatim
 * This function writes the databsae out to disk, in paranoid mode. 
 * The database structure currently looks something like this:
 * +V<header line>
 * +FLAGS LIST
 * <flag data>
 * ~<number of objects>
 * <object data>
 * \endverbatim
 * \param f file pointer to write to.
 * \param flag 0 for normal paranoid dump, 1 for debug paranoid dump.
 * \return the number of objects in the database (db_top)
 */
dbref
db_paranoid_write(FILE * f, int flag)
{
  dbref i;
  int dbflag;

/* print a header line to make a later conversion to 2.0 easier to do.
 * the odd choice of numbers is based on 256*x + 2 offset
 */
  dbflag = 5;
  dbflag += DBF_NO_CHAT_SYSTEM;
  dbflag += DBF_WARNINGS;
  dbflag += DBF_CREATION_TIMES;
  dbflag += DBF_SPIFFY_LOCKS;
  dbflag += DBF_NEW_STRINGS;
  dbflag += DBF_TYPE_GARBAGE;
  dbflag += DBF_SPLIT_IMMORTAL;
  dbflag += DBF_NO_TEMPLE;
  dbflag += DBF_LESS_GARBAGE;
  dbflag += DBF_AF_VISUAL;
  dbflag += DBF_VALUE_IS_COST;
  dbflag += DBF_LINK_ANYWHERE;
  dbflag += DBF_NO_STARTUP_FLAG;
  dbflag += DBF_AF_NODUMP;

  OUTPUT(fprintf(f, "+V%d\n", dbflag * 256 + 2));

  do_rawlog(LT_CHECK, "PARANOID WRITE BEGINNING...\n");

  /* print total number of objects at top of file */
  OUTPUT(fprintf(f, "~%d\n", db_top));

  /* write out each object */
  for (i = 0; i < db_top; i++) {
#ifdef WIN32
    /* Keep the service manager happy */
    if (shutdown_flag && (i & 0xFF) == 0)
      shutdown_checkpoint();
#endif
    if (IsGarbage(i))
      continue;
    OUTPUT(fprintf(f, "!%d\n", i));
    db_paranoid_write_object(f, i, flag);
    /* print out a message every so many objects */
    if (i % paranoid_checkpt == 0)
      do_rawlog(LT_CHECK, T("\t...wrote up to object #%d\n"), i);
  }
  OUTPUT(fputs(EOD, f));
  do_rawlog(LT_CHECK, T("\t...finished at object #%d\n"), i - 1);
  do_rawlog(LT_CHECK, "END OF PARANOID WRITE.\n");
  return (db_top);
}


/** Read in a long int.
 * \param file file pointer to read from.
 * \return long int read.
 */
long int
getref(FILE * f)
{
  static char buf[BUFFER_LEN];
  fgets(buf, sizeof(buf), f);
  return (atol(buf));
}


/** Read in a string, into a static buffer.
 * This function reads a double-quoted escaped string of the form
 * written by putstring. The string is read into a static buffer
 * that is not allocated, so the return value must usually be copied
 * elsewhere.
 * \param file file pointer to read from.
 * \return pointer to static buffer containing string read.
 */
const char *
getstring_noalloc(FILE * f)
{
  static char buf[BUFFER_LEN];
  char *p;
  int c;

  p = buf;
  c = fgetc(f);
  if (c != '"') {
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

static struct boolexp *
getboolexp1(FILE * f, const char *type)
{
  struct boolexp *b, *t;
  int c;
  char tbuf1[BUFFER_LEN], tbuf2[BUFFER_LEN];

  c = getc(f);
  switch (c) {
  case '\n':
    ungetc(c, f);
    return TRUE_BOOLEXP;
    /* break; */
  case EOF:
    do_rawlog(LT_ERR, "ERROR: Unexpected EOF in boolexp.  Object #%d", errobj);
    return TRUE_BOOLEXP;
    /*NOTREACHED */
    break;
  case '(':
    b = alloc_bool();
    if ((c = getc(f)) == NOT_TOKEN) {
      b->type = BOOLEXP_NOT;
      b->data.n = getboolexp1(f, type);
      if (getc(f) != ')')
	goto error;
      return b;
    } else if (c == IS_TOKEN) {
      b->type = BOOLEXP_IS;
      t = getboolexp1(f, type);
      b->thing = t->thing;
      free_boolexp(t);
      if (getc(f) != ')')
	goto error;
      return b;
    } else if (c == IN_TOKEN) {
      b->type = BOOLEXP_CARRY;
      t = getboolexp1(f, type);
      b->thing = t->thing;
      free_boolexp(t);
      if (getc(f) != ')')
	goto error;
      return b;
    } else if (c == OWNER_TOKEN) {
      b->type = BOOLEXP_OWNER;
      t = getboolexp1(f, type);
      b->thing = t->thing;
      free_boolexp(t);
      if (getc(f) != ')')
	goto error;
      return b;
    } else if (c == AT_TOKEN) {
      b->type = BOOLEXP_IND;
      t = getboolexp1(f, type);
      b->thing = t->thing;
      free_boolexp(t);
      b->data.ind_lock = st_insert(type, &atr_names);
      if (getc(f) != ')')
	goto error;
      return b;
    } else {
      ungetc(c, f);
      b->data.sub.a = getboolexp1(f, type);
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
      b->data.sub.b = getboolexp1(f, type);
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
	do_rawlog(LT_ERR, "ERROR: Unexpected EOF in boolexp. Object #%d",
		  errobj);
	return TRUE_BOOLEXP;
      }
    ungetc(c, f);
    return TRUE_BOOLEXP;
    /* break */
  case '"':
    /* Either a BOOLEXP_ATR or a BOOLEXP_EVAL */
    ungetc(c, f);
    strcpy(tbuf1, getstring_noalloc(f));
    c = fgetc(f);
    if (c == EOF) {
      do_rawlog(LT_ERR, "ERROR: Unexpected EOF in boolexp. Object #%d", errobj);
      return TRUE_BOOLEXP;
    }
    b = alloc_bool();
    b->type = (c == ':') ? BOOLEXP_ATR : BOOLEXP_EVAL;
    b->data.atr_lock = alloc_atr(tbuf1, getstring_noalloc(f));
    return b;
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

      if (c == ':') {		/* old style boolexp lock */
	do_rawlog(LT_ERR, T("ERROR: Old-style @lock on object #%d\n"), errobj);
	return TRUE_BOOLEXP;
      }
      ungetc(c, f);
      return (b);
    } else {
      /* we indulge in a bit of a kludge. We either have a colon
       * separated string (attribute lock) or a slash-separate string
       * (eval lock).
       */
      char *p = tbuf1, *s;
      char savec;

      *p++ = c;
      for (; ((c = getc(f)) != EOF) && (c != '\n') && (c != ':')
	   && (c != '/'); *p++ = c) ;
      *p = '\0';
      if ((c == EOF) || (c == '\n'))
	goto error;
      if ((c != ':') && (c != '/'))
	goto error;
      savec = c;
      for (s = tbuf2;
	   ((c = getc(f)) != EOF) && (c != '\n') && (c != ')' && (c != OR_TOKEN)
						     && (c != AND_TOKEN));
	   *s++ = c) ;
      if (c == EOF)
	goto error;
      *s++ = 0;
      ungetc(c, f);
      b->data.atr_lock = alloc_atr(tbuf1, tbuf2);
      if (savec == ':')
	b->type = BOOLEXP_ATR;
      else
	b->type = BOOLEXP_EVAL;
      return (b);
    }
  }
error:
  do_rawlog(LT_ERR, "ERROR: Unknown error in boolexp. Object #%d", errobj);
  return TRUE_BOOLEXP;
}

/** Read a boolexp from a file.
 * This function reads a boolexp from a file. It expects the format that
 * put_boolexp writes out.
 * \param f file pointer to read from.
 * \param type pointer to lock type being read.
 * \return pointer to boolexp read.
 */
struct boolexp *
getboolexp(FILE * f, const char *type)
{
  struct boolexp *b;
  if (indb_flags & DBF_SPIFFY_LOCKS) {
    char *label, *val;
    get_logical_line(f, &label, &val);
    b = parse_boolexp(GOD, val, type);
  } else {
    b = getboolexp1(f, type);
    if (getc(f) != '\n') {
      do_rawlog(LT_ERR, "ERROR: Invalid boolexp format on object #%d", errobj);
      return TRUE_BOOLEXP;
    }
  }
  return b;
}

/** Read locks for an object.
 * This function is used for DBF_SPIFFY_LOCKS to read a whole list
 * of locks from an object and set them.
 * \param i dbref of the object.
 * \param f file pointer to read from.
 */
void
get_new_locks(dbref i, FILE * f)
{
  char *label, *val;
  dbref creator;
  int flags;
  char type[BUFFER_LEN];
  struct boolexp *b;
  int count = 0, n;

  get_logical_line(f, &label, &val);
  count = parse_integer(val);

  for (n = 0; n < count; n++) {
    /* Name of the lock */
    get_logical_line(f, &label, &val);
    strcpy(type, val);
    /* Creator */
    get_logical_line(f, &label, &val);
    creator = parse_integer(val);
    /* Flags */
    get_logical_line(f, &label, &val);
    flags = parse_integer(val);
    /* boolexp */
    b = getboolexp(f, type);
    add_lock_raw(creator, i, type, b, flags);
  }
}


/** Read locks for an object.
 * This function is used for DBF_NEW_LOCKS to read a whole list
 * of locks from an object and set them. DBF_NEW_LOCKS aren't really
 * new any more, and get_new_locks() is probably being used instead of
 * this function.
 * \param i dbref of the object.
 * \param f file pointer to read from.
 */
void
getlocks(dbref i, FILE * f)
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
      do_rawlog(LT_ERR, T("ERROR: Invalid lock format on object #%d"), i);
      return;
    }
    b = getboolexp(f, buf);	/* Which will clobber a '\n' */
    if (b == TRUE_BOOLEXP) {
      /* getboolexp() would already have complained. */
      return;
    } else {
      add_lock_raw(Owner(i), i, buf, b, -1);
    }
  }
  ungetc(c, f);
  return;
}

/** Free the entire database.
 * This function frees the name, attributes, and locks on every object
 * in the database, and then free the entire database structure and
 * resets db_top.
 */
void
db_free(void)
{
  dbref i;

  if (db) {

    for (i = 0; i < db_top; i++) {
      set_name(i, NULL);
      atr_free(i);
      free_locks(Locks(i));
    }

    free((char *) db);
    db = NULL;
    db_init = db_top = 0;
  }
}

/** Read an attribute list for an object from a file
 * \param f file pointer to read from.
 * \param i dbref for the attribute list.
 */
int
get_list(FILE * f, dbref i)
{
  int c;
  char *p, *q;
  char tbuf1[BUFFER_LEN + 150];
  int flags;
  int count = 0;

  List(i) = NULL;
  tbuf1[0] = '\0';
  while (1)
    switch (c = getc(f)) {
    case ']':			/* new style attribs, read name then value */
      /* Using getstring_noalloc here will cause problems with attribute
         names starting with ". This is probably a better fix than just
         disallowing " in attribute names. */
      fgets(tbuf1, BUFFER_LEN + 150, f);
      if (!(p = strchr(tbuf1, '^'))) {
	do_rawlog(LT_ERR, T("ERROR: Bad format on new attributes. object #%d"),
		  i);
	return -1;
      }
      *p++ = '\0';
      if (!(q = strchr(p, '^'))) {
	do_rawlog(LT_ERR,
		  T("ERROR: Bad format on new attribute %s. object #%d"),
		  tbuf1, i);
	return -1;
      }
      *q++ = '\0';
      flags = atoi(q);
      /* Remove obsolete AF_NUKED flag and AF_STATIC, just in case */
      flags &= ~AF_NUKED;
      flags &= ~AF_STATIC;
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
      atr_new_add(i, tbuf1, getstring_noalloc(f), atoi(p), flags);
      count++;
      /* Check removed for atoi(q) == 0  (which results in NOTHING for that
       * parameter, and thus no flags), since this eliminates 'visual'
       * attributes (which, if not built-in attrs, have a flag val of 0.)
       */
      break;
    case '>':			/* old style attribs, die noisily */
      do_rawlog(LT_ERR, T("ERROR: old-style attribute format in object %d"), i);
      return -1;
      break;
    case '<':			/* end of list */
      if ('\n' != getc(f)) {
	do_rawlog(LT_ERR, T("ERROR: no line feed after < on object %d"), i);
	return -1;
      }
      return count;
    default:
      if (c == EOF) {
	do_rawlog(LT_ERR, T("ERROR: Unexpected EOF on file."));
	return -1;
      }
      do_rawlog(LT_ERR,
		T
		("ERROR: Bad character %c (%d) in attribute list on object %d"),
		c, c, i);
      do_rawlog(LT_ERR,
		T("  (expecting ], >, or < as first character of the line.)"));
      if (*tbuf1)
	do_rawlog(LT_ERR, T("  Last attribute read was: %s"), tbuf1);
      else
	do_rawlog(LT_ERR, T("  No attributes had been read yet."));
      return -1;
    }
}



/** Read the object database from a file.
 * This function reads the entire database from a file. See db_write()
 * for some notes about the expected format.
 * \param f file pointer to read from.
 * \return number of objects in the database.
 */
dbref
db_read(FILE * f)
{
  int c;
  dbref i;
  struct object *o;
  int temp = 0;
  time_t temp_time = 0;
  int minimum_flags =
    DBF_NEW_STRINGS | DBF_TYPE_GARBAGE | DBF_SPLIT_IMMORTAL | DBF_NO_TEMPLE;

  loading_db = 1;

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
      init_objdata_htab(db_init);
      break;
      /* Use the MUSH 2.0 header stuff to see what's in this db */
    case '+':
      c = getc(f);		/* Skip the V */
      if (c == 'V') {
	indb_flags = ((getref(f) - 2) / 256) - 5;
	/* if you want to read in an old-style database, use an earlier
	 * patchlevel to upgrade.
	 */
	if (((indb_flags & minimum_flags) != minimum_flags) ||
	    (indb_flags & DBF_NO_POWERS)) {
	  do_rawlog(LT_ERR, T("ERROR: Old database without required dbflags."));
	  return -1;
	}
	if (!(indb_flags & (DBF_NEW_LOCKS | DBF_SPIFFY_LOCKS))) {
	  do_rawlog(LT_ERR, T("Error: Unsupported @lock format in database."));
	  return -1;
	}
      } else if (c == 'F') {
	(void) getstring_noalloc(f);
	flag_read_all(f);
      } else {
	do_rawlog(LT_ERR, T("Unrecognized database format!"));
	return -1;
      }
      break;
      /* old fashioned database */
    case '#':
    case '&':			/* zone oriented database */
      do_rawlog(LT_ERR, T("ERROR: old style database."));
      return -1;
      break;
      /* new database */
    case '!':			/* non-zone oriented database */
      /* make space */
      i = getref(f);
      db_grow(i + 1);
      /* read it in */
      o = db + i;
      set_name(i, getstring_noalloc(f));
      o->location = getref(f);
      o->contents = getref(f);
      o->exits = getref(f);
      o->next = getref(f);
      o->parent = getref(f);
      o->locks = NULL;
      if (indb_flags & DBF_SPIFFY_LOCKS)
	get_new_locks(i, f);
      else
	getlocks(i, f);
      o->owner = getref(f);
      o->zone = getref(f);
      s_Pennies(i, getref(f));
      if (indb_flags & DBF_NEW_FLAGS) {
	o->type = getref(f);
	o->flags = string_to_bits(getstring_noalloc(f));
      } else {
	int old_flags, old_toggles;
	old_flags = getref(f);
	old_toggles = getref(f);
	if ((o->type = type_from_old_flags(old_flags)) < 0) {
	  do_rawlog(LT_ERR, T("Unable to determine type of #%d\n"), i);
	  return -1;
	}
	o->flags = flags_from_old_flags(old_flags, old_toggles, o->type);
      }

      /* We need to have flags in order to do this right, which is why
       * we waited until now
       */
      switch (Typeof(i)) {
      case TYPE_PLAYER:
	current_state.players++;
	current_state.garbage--;
	break;
      case TYPE_THING:
	current_state.things++;
	current_state.garbage--;
	break;
      case TYPE_EXIT:
	current_state.exits++;
	current_state.garbage--;
	break;
      case TYPE_ROOM:
	current_state.rooms++;
	current_state.garbage--;
	break;
      }

      if (IsPlayer(i) && (strlen(o->name) > (Size_t) PLAYER_NAME_LIMIT)) {
	char buff[BUFFER_LEN + 1];	/* The name plus a NUL */
	strncpy(buff, o->name, PLAYER_NAME_LIMIT);
	buff[PLAYER_NAME_LIMIT] = '\0';
	set_name(i, buff);
	do_rawlog(LT_CHECK,
		  T(" * Name of #%d is longer than the maximum, truncating.\n"),
		  i);
      } else if (!IsPlayer(i) && (strlen(o->name) > OBJECT_NAME_LIMIT)) {
	char buff[OBJECT_NAME_LIMIT + 1];	/* The name plus a NUL */
	strncpy(buff, o->name, OBJECT_NAME_LIMIT);
	buff[OBJECT_NAME_LIMIT] = '\0';
	set_name(i, buff);
	do_rawlog(LT_CHECK,
		  T(" * Name of #%d is longer than the maximum, truncating.\n"),
		  i);
      }

      if (!(indb_flags & DBF_VALUE_IS_COST) && IsThing(i))
	s_Pennies(i, (Pennies(i) + 1) * 5);
      o->powers = getref(f);
      /* If we've got a variable exit predating the link_anywhere power,
       * give it the link_anywhere power now.
       */
      if (!(indb_flags & DBF_LINK_ANYWHERE)) {
	if (IsExit(i) && (Destination(i) == AMBIGUOUS))
	  o->powers |= LINK_ANYWHERE;
      }

      /* Remove the STARTUP and ACCESSED flags */
      if (!(indb_flags & DBF_NO_STARTUP_FLAG)) {
	clear_flag_internal(i, "STARTUP");
	clear_flag_internal(i, "ACCESSED");
      }

      /* Clear the GOING flags. If it was scheduled for destruction
       * when the db was saved, it gets a reprieve.
       */
      clear_flag_internal(i, "GOING");
      clear_flag_internal(i, "GOING_TWICE");

      /* If there are channels in the db, read 'em in */
      /* We don't support this anymore, so we just discard them */
      if (!(indb_flags & DBF_NO_CHAT_SYSTEM))
	temp = getref(f);
      else
	temp = 0;

      /* If there are warnings in the db, read 'em in */
      temp = MAYBE_GET(f, DBF_WARNINGS);
      o->warnings = temp;
      /* If there are creation times in the db, read 'em in */
      temp_time = MAYBE_GET(f, DBF_CREATION_TIMES);
      if (temp_time)
	o->creation_time = (time_t) temp_time;
      else
	o->creation_time = mudtime;
      temp_time = MAYBE_GET(f, DBF_CREATION_TIMES);
      if (temp_time || IsPlayer(i))
	o->modification_time = (time_t) temp_time;
      else
	o->modification_time = o->creation_time;

      /* read attribute list for item */
      if ((o->attrcount = get_list(f, i)) < 0) {
	do_rawlog(LT_ERR, T("ERROR: bad attribute list object %d"), i);
	return -1;
      }
      if (!(indb_flags & DBF_AF_NODUMP)) {
	/* Clear QUEUE and SEMAPHORE attributes */
	atr_clr(i, "QUEUE", GOD);
	atr_clr(i, "SEMAPHORE", GOD);
      }
      /* check to see if it's a player */
      if (IsPlayer(i)) {
	add_player(i, NULL);
	clear_flag_internal(i, "CONNECTED");
      }
      break;

    case '*':
      {
	char buff[80];
	ungetc('*', f);
	fgets(buff, sizeof buff, f);
	if (strcmp(buff, EOD) != 0) {
	  do_rawlog(LT_ERR, T("ERROR: No end of dump after object #%d"), i - 1);
	  return -1;
	} else {
	  do_rawlog(LT_ERR, "READING: done");
	  loading_db = 0;
	  fix_free_list();
	  dbck();
	  return db_top;
	}
      }
    default:
      do_rawlog(LT_ERR, T("ERROR: failed object %d"), i);
      return -1;
    }
  }
}

static void
init_objdata_htab(int size)
{
  hashinit(&htab_objdata, size, 4);
  hashinit(&htab_objdata_keys, 8, 32);
}

/** Add data to the object data hashtable. 
 * This hash table is typically used to store transient object data
 * that is built at database load and isn't saved to disk, but it
 * can be used for other purposes as well - it's a good general
 * tool for hackers who want to add their own data to objects.
 * This function adds data to the hashtable.
 * \param thing dbref of object to associate the data with.
 * \param keybase base string for type of data.
 * \param data pointer to the data to store.
 * \return data passed in.
 */
void *
set_objdata(dbref thing, const char *keybase, void *data)
{
  hashdelete(tprintf("%s_#%d", keybase, thing), &htab_objdata);
  if (data) {
    if (hashadd(tprintf("%s_#%d", keybase, thing), data, &htab_objdata) < 0)
      return NULL;
    if (hash_find(&htab_objdata_keys, keybase) == NULL) {
      char *newkey = strdup(keybase);
      hashadd(keybase, (void *) &newkey, &htab_objdata_keys);
    }
  }
  return data;
}

/** Retrieve data from the object data hashtable.
 * \param thing dbref of object data is associated with.
 * \param keybase base string for type of data.
 * \return data stored for that object and keybase, or NULL.
 */
void *
get_objdata(dbref thing, const char *keybase)
{
  return hashfind(tprintf("%s_#%d", keybase, thing), &htab_objdata);
}

/** Clear all of an object's data from the object data hashtable.
 * This function clears any data associated with a given object
 * that's in the object data hashtable (under any keybase).
 * It's used before we free the object.
 * \param thing dbref of object data is associated with.
 */
void
clear_objdata(dbref thing)
{
  char *p;
  for (p = (char *) hash_firstentry(&htab_objdata_keys);
       p; p = (char *) hash_nextentry(&htab_objdata_keys)) {
    set_objdata(thing, p, NULL);
  }
}
