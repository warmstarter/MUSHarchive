#include "copyrite.h"

#include "config.h"
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#include "conf.h"
#include "attrib.h"
#include "dbdefs.h"
#include "externs.h"
#include "match.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "oldattrb.h"
#include "parse.h"
#include "htab.h"
#include "privtab.h"
#include "mymalloc.h"
#include "confmagic.h"


struct boolatr *alloc_atr _((const char *name, char *s));

ATTR *atr_match _((char const *string));
int string_to_atrflag _((dbref player, char const *p));
void atr_new_add _((dbref thing, char const *atr, char const *s,
		    dbref player, int flags));
int atr_add _((dbref thing, char const *atr, char const *s,
	       dbref player, int flags));
int atr_clr _((dbref thing, char const *atr, dbref player));
ATTR *atr_get _((dbref thing, char const *atr));
ATTR *atr_get_noparent _((dbref thing, char const *atr));
ATTR *atr_complete_match _((dbref player, char const *atr, dbref privs));
void atr_free _((dbref thing));
void atr_cpy _((dbref dest, dbref source));

char const *const convert_atr _((int oldatr));

int atr_comm_match _((dbref thing, dbref player, int type, int end, char const *str, int just_match));

int do_set_atr _((dbref thing, char const *atr, char const *s, dbref player,
		  int flags));
void do_atrlock _((dbref player, char const *arg1, char const *arg2));
void do_atrchown _((dbref player, char const *arg1, char const *arg2));

#ifdef TREKMUSH
int good_atr_name _((char const *s));
#else /* TREKMUSH */
static int good_atr_name _((char const *s));
#endif /* TREKMUSH */
static char const *standard_atr_name _((char const *atr));

static char *atr_hash_lookup _((char const *s));
static void atr_hash_insert _((char const *name));
static void atr_hash_init _((void));
static void atr_hash_free _((void));
static int atr_hash_initialized = 0;

/* A hash table that temporarily stores matching attributes from current
 * and parent objects when we're searching for commands on attributes
 */
HASHTAB htab_temp_attrib;
extern PRIV attr_privs[];

/*======================================================================*/

/* This should be with the lock stuff, not the attribute stuff.
 * Who was the idiot who put it here? */

struct boolatr *
alloc_atr(name, s)
    const char *name;
    char *s;
{
  struct boolatr *a;
  const unsigned char *p;

  a = (struct boolatr *) mush_malloc(sizeof(struct boolatr), "boolatr");
  a->name = (char *) mush_malloc(strlen(name) + 1, "boolatr_name");
  strcpy(a->name, name);
  p = compress(s);
  a->text = (unsigned char *) mush_malloc(u_strlen(p) + 1, "boolatr_text");
  u_strcpy(a->text, p);
  return a;
}

/*======================================================================*/

#define ATTRS_PER_PAGE (200)

typedef struct atrpage {
  ATTR atrs[ATTRS_PER_PAGE];
} ATTRPAGE;

static ATTR *atr_free_list;

/* for attrib hash stuff, in atr_tab.c */
extern ATTR *aname_hash_lookup _((const char *name));

/*======================================================================*/

#ifdef TREKMUSH
int
#else /* TREKMUSH */
static int
#endif /* TREKMUSH */
good_atr_name(s)
    char const *s;
{
  const char *a;
  if (!s || !*s)
    return 0;
  for (a = s; *a; a++)
    if (!(isascii(*a) && isprint(*a) && !isspace(*a) && (*a != '^'))) {
      return 0;
    }
  return 1;
}

static char const *
standard_atr_name(atr)
    char const *atr;
{
  return NULL;
}

ATTR *
atr_match(string)
    char const *string;
{
  return aname_hash_lookup(string);
}

int
string_to_atrflag(player, p)
    dbref player;
    char const *p;
{
  int f;
  f = string_to_privs(attr_privs, p, 0);
  if (!f)
    return -1;
  if (!Hasprivs(player) && (f & (AF_MDARK | AF_WIZARD)))
    return -1;
  return f;
}

/* This is a stripped down version of atr_add, without duplicate checking,
 * permissions checking, or auto-ODARKing.  If anyone uses this outside
 * of database load or atr_cpy (below), I will personally string them up
 * by their toes.  - Alex */
void
atr_new_add(thing, atr, s, player, flags)
    dbref thing;
    char const *atr;
    char const *s;
    dbref player;
    int flags;
{
  ATTR *ptr, *std;
  char *name;

#ifndef EMPTY_ATTRS
  if (!*s)
    return;
#endif

  /* allocate a new page, if needed */
  if (!atr_free_list) {
    ATTRPAGE *page;
    int j;
    page = (ATTRPAGE *) mush_malloc(sizeof(ATTRPAGE), "ATTRPAGE");
    if (!page)
      return;
    for (j = 0; j < ATTRS_PER_PAGE - 1; j++)
      AL_NEXT(page->atrs + j) = page->atrs + j + 1;
    AL_NEXT(page->atrs + ATTRS_PER_PAGE - 1) = NULL;
    atr_free_list = page->atrs;
  }
  ptr = atr_free_list;
  atr_free_list = AL_NEXT(ptr);

  /* initialize atr */
  std = atr_match(atr);
  if (std && !strcmp(AL_NAME(std), atr)) {
    AL_NAME(ptr) = AL_NAME(std);
    AL_FLAGS(ptr) = flags | AF_STATIC;
  } else {
    name = (char *) mush_malloc(strlen(atr) + 1, "ATTR name");
    if (!name) {
      AL_NEXT(ptr) = atr_free_list;
      atr_free_list = ptr;
      return;
    }
    strcpy(name, atr);
    AL_NAME(ptr) = name;
    AL_FLAGS(ptr) = 0;
    if (flags != NOTHING)
      AL_FLAGS(ptr) |= flags;
  }
  AL_CREATOR(ptr) = player;

  /* link it in */
  AL_NEXT(ptr) = List(thing);
  List(thing) = ptr;

  AL_FLAGS(ptr) &= ~AF_COMMAND & ~AF_LISTEN;

  /* replace string with new string */
  if (!s || !*s) {
    AL_STR(ptr) = (unsigned char *) "";
  } else {
    AL_STR(ptr) = (unsigned char *) u_strdup(compress((char *) s));
    if (!AL_STR(ptr)) {
      AL_STR(ptr) = (unsigned char *) "";
      return;
    }
#ifdef MEM_CHECK
    add_check("ATTR value");
#endif
  }
  if (*s == '$')
    AL_FLAGS(ptr) |= AF_COMMAND;
  if (*s == '^')
    AL_FLAGS(ptr) |= AF_LISTEN;
}

int
atr_add(thing, atr, s, player, flags)
    dbref thing;
    char const *atr;
    char const *s;
    dbref player;
    int flags;
{
  ATTR *ptr, *std;
  char *name;

  if (!s)
    return atr_clr(thing, atr, player);

#ifndef EMPTY_ATTRS
  if (!*s)
    return atr_clr(thing, atr, player);
#endif

  if (!good_atr_name(atr))
    return -1;

  /* walk the list, looking for a preexisting value */
  ptr = List(thing);
  while (ptr && strcmp(atr, AL_NAME(ptr)))
    ptr = AL_NEXT(ptr);

  /* check for permission to modify existing atr */
  if (ptr && !Can_Write_Attr(Owner(player), thing, ptr))
    return -1;

  /* make a new atr, if needed */
  if (!ptr) {
    /* allocate a new page, if needed */
    if (!atr_free_list) {
      ATTRPAGE *page;
      int j;
      page = (ATTRPAGE *) mush_malloc(sizeof(ATTRPAGE), "ATTRPAGE");
      if (!page)
	return -1;
      for (j = 0; j < ATTRS_PER_PAGE - 1; j++)
	AL_NEXT(page->atrs + j) = page->atrs + j + 1;
      AL_NEXT(page->atrs + ATTRS_PER_PAGE - 1) = NULL;
      atr_free_list = page->atrs;
    }
    ptr = atr_free_list;
    atr_free_list = AL_NEXT(ptr);

    /* initialize atr */
    std = atr_match(atr);
    if (std && !strcmp(AL_NAME(std), atr)) {
      AL_NAME(ptr) = AL_NAME(std);
      AL_FLAGS(ptr) = AL_FLAGS(std) | AF_STATIC;
    } else {
      name = (char *) mush_malloc(strlen(atr) + 1, "ATTR name");
      if (!name) {
	AL_NEXT(ptr) = atr_free_list;
	atr_free_list = ptr;
	return -1;
      }
      strcpy(name, atr);
      AL_NAME(ptr) = name;
      AL_FLAGS(ptr) = 0;
      if (flags != NOTHING)
	AL_FLAGS(ptr) |= flags;
    }
    AL_CREATOR(ptr) = player;
    AL_STR(ptr) = NULL;

    /* check for permission to create atr */
    if (!Can_Write_Attr(Owner(player), thing, ptr)) {
      AL_NEXT(ptr) = atr_free_list;
      atr_free_list = ptr;
      return -1;
    }
    /* link it in */
    AL_NEXT(ptr) = List(thing);
    List(thing) = ptr;
  }
  /* update modification time here, because from now on,
   * we modify even if we fail */
#ifdef CREATION_TIMES
  if (!IsPlayer(thing))
    ModTime(thing) = time((time_t *) 0);
#endif

  /* change owner */
  AL_CREATOR(ptr) = Owner(player);

  AL_FLAGS(ptr) &= ~AF_COMMAND & ~AF_LISTEN;

  /* replace string with new string */
  if (AL_STR(ptr) && *AL_STR(ptr))
    mush_free((Malloc_t) AL_STR(ptr), "ATTR value");
  if (!s || !*s) {
    AL_STR(ptr) = (unsigned char *) "";
  } else {
    AL_STR(ptr) = (unsigned char *) u_strdup(compress((char *) s));
    if (!AL_STR(ptr)) {
      AL_STR(ptr) = (unsigned char *) "";
      return -1;
    }
#ifdef MEM_CHECK
    add_check("ATTR value");
#endif
  }
  if (*s == '$')
    AL_FLAGS(ptr) |= AF_COMMAND;
  if (*s == '^')
    AL_FLAGS(ptr) |= AF_LISTEN;

  return 1;
}

int
atr_clr(thing, atr, player)
    dbref thing;
    char const *atr;
    dbref player;
{
  ATTR *ptr, **prev;

#ifdef EMPTY_ATTRS
#ifndef DELETE_ATTRS
  return atr_add(thing, atr, "", player, NOTHING);
#endif
#endif

  if (!good_atr_name(atr))
    return -1;

  prev = &List(thing);
  ptr = *prev;
  while (ptr && strcmp(atr, AL_NAME(ptr))) {
    prev = &AL_NEXT(ptr);
    ptr = *prev;
  }

  if (!ptr)
    return 0;

  if (!Can_Write_Attr(Owner(player), thing, ptr))
    return -1;

#ifdef CREATION_TIMES
  if (!IsPlayer(thing))
    ModTime(thing) = time((time_t *) 0);
#endif

  *prev = AL_NEXT(ptr);

  if (AL_STR(ptr) && *AL_STR(ptr))
    mush_free((Malloc_t) AL_STR(ptr), "ATTR value");
  if (!(AL_FLAGS(ptr) & AF_STATIC))
    mush_free((Malloc_t) AL_NAME(ptr), "ATTR name");

  AL_NEXT(ptr) = atr_free_list;
  AL_FLAGS(ptr) = 0;
  atr_free_list = ptr;
  return 1;
}

ATTR *
atr_get(thing, atr)
    dbref thing;
    char const *atr;
{
  ATTR *ptr;
  int parent_depth;
  dbref temp;

  if (thing == NOTHING || !good_atr_name(atr))
    return NULL;

  /* try real name */
  for (parent_depth = 0, temp = thing;
       parent_depth < MAX_PARENTS && temp != NOTHING;
       parent_depth++, temp = Parent(temp))
    for (ptr = List(temp); ptr; ptr = AL_NEXT(ptr))
      if (!strcmp(AL_NAME(ptr), atr)
#ifndef VISIBLE_EMPTY_ATTRS
	  && *AL_STR(ptr)
#endif
	) {
	if (temp == thing || !(AL_FLAGS(ptr) & AF_PRIVATE))
	  return ptr;
	else
	  break;
      }
  ptr = atr_match(atr);
  if (!ptr || !strcmp(atr, AL_NAME(ptr)))
    return NULL;
  atr = AL_NAME(ptr);

  /* try alias */
  for (parent_depth = 0, temp = thing;
       parent_depth < MAX_PARENTS && temp != NOTHING;
       parent_depth++, temp = Parent(temp))
    for (ptr = List(temp); ptr; ptr = AL_NEXT(ptr))
      if (!strcmp(AL_NAME(ptr), atr)
#ifndef VISIBLE_EMPTY_ATTRS
	  && *AL_STR(ptr)
#endif
	) {
	if (temp == thing || !(AL_FLAGS(ptr) & AF_PRIVATE))
	  return ptr;
	else
	  break;
      }
  return NULL;
}

ATTR *
atr_get_noparent(thing, atr)
    dbref thing;
    char const *atr;
{
  ATTR *ptr;

  if (thing == NOTHING || !good_atr_name(atr))
    return NULL;

  /* try real name */
  for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr))
    if (!strcmp(AL_NAME(ptr), atr)
#ifndef VISIBLE_EMPTY_ATTRS
	&& *AL_STR(ptr)
#endif
      )
      return ptr;

  ptr = atr_match(atr);
  if (!ptr || !strcmp(atr, AL_NAME(ptr)))
    return NULL;
  atr = AL_NAME(ptr);

  /* try alias */
  for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr))
    if (!strcmp(AL_NAME(ptr), atr)
#ifndef VISIBLE_EMPTY_ATTRS
	&& *AL_STR(ptr)
#endif
      )
      return ptr;

  return NULL;
}

int
atr_iter_get(player, thing, name, func, args)
    dbref player;
    dbref thing;
    char const *name;
    int (*func) ();
    void *args;
{
  ATTR *ptr, *next;
  int result;

  result = 0;
  if (!name || !*name)
    name = "*";

  if (!wildcard(name)) {
    ptr = atr_get_noparent(thing, strupper(name));
    if (ptr &&
#ifndef VISIBLE_EMPTY_ATTRS
	*AL_STR(ptr) &&
#endif
	Can_Read_Attr(player, thing, ptr))
      result = func(player, thing, name, ptr, args);
  } else {
    for (ptr = List(thing); ptr; ptr = next) {
      next = AL_NEXT(ptr);
      if (Can_Read_Attr(player, thing, ptr) &&
#ifndef VISIBLE_EMPTY_ATTRS
	  *AL_STR(ptr) &&
#endif
	  local_wild_match(name, AL_NAME(ptr)))
	result += func(player, thing, name, ptr, args);
    }
  }

  return result;
}

ATTR *
atr_complete_match(player, atr, privs)
    dbref player;
    char const *atr;
    dbref privs;
{
  ATTR *ptr;
  char *name;

  name = strupper(atr);

  if ((ptr = atr_get(player, name)) && Can_Read_Attr(privs, player, ptr))
    return ptr;

  for (player = Contents(player); player != NOTHING; player = Next(player))
    if ((ptr = atr_get(player, name)) && Can_Read_Attr(privs, player, ptr))
      return ptr;

  return NULL;
}

void
atr_free(thing)
    dbref thing;
{
  ATTR *ptr;

  if (!List(thing))
    return;

#ifdef CREATION_TIMES
  if (!IsPlayer(thing))
    ModTime(thing) = time((time_t *) 0);
#endif

  while ((ptr = List(thing))) {
    List(thing) = AL_NEXT(ptr);

    if (AL_STR(ptr) && *AL_STR(ptr))
      mush_free((Malloc_t) AL_STR(ptr), "ATTR value");
    if (!(AL_FLAGS(ptr) & AF_STATIC))
      mush_free((Malloc_t) AL_NAME(ptr), "ATTR name");

    AL_NEXT(ptr) = atr_free_list;
    atr_free_list = ptr;
  }
}

void
atr_cpy(dest, source)
    dbref dest;
    dbref source;
{
  ATTR *ptr;

  List(dest) = NULL;
  for (ptr = List(source); ptr; ptr = AL_NEXT(ptr))
    if (!(AL_FLAGS(ptr) & AF_NOCOPY)
#ifndef VISIBLE_EMPTY_ATTRS
	&& *AL_STR(ptr)
#endif
      )
      atr_new_add(dest, AL_NAME(ptr), uncompress(AL_STR(ptr)),
		  AL_CREATOR(ptr), AL_FLAGS(ptr));
}

/*======================================================================*/

char const *const
convert_atr(oldatr)
    int oldatr;
{
  static char result[3];

  switch (oldatr) {
  case A_OSUCC:
    return "OSUCCESS";
  case A_OFAIL:
    return "OFAILURE";
  case A_FAIL:
    return "FAILURE";
  case A_SUCC:
    return "SUCCESS";
  case A_PASS:
    return "XYXXY";
  case A_DESC:
    return "DESCRIBE";
  case A_SEX:
    return "SEX";
  case A_ODROP:
    return "ODROP";
  case A_DROP:
    return "DROP";
  case A_OKILL:
    return "OKILL";
  case A_KILL:
    return "KILL";
  case A_ASUCC:
    return "ASUCCESS";
  case A_AFAIL:
    return "AFAILURE";
  case A_ADROP:
    return "ADROP";
  case A_AKILL:
    return "AKILL";
  case A_USE:
    return "DOES";
  case A_CHARGES:
    return "CHARGES";
  case A_RUNOUT:
    return "RUNOUT";
  case A_STARTUP:
    return "STARTUP";
  case A_ACLONE:
    return "ACLONE";
  case A_APAY:
    return "APAYMENT";
  case A_OPAY:
    return "OPAYMENT";
  case A_PAY:
    return "PAYMENT";
  case A_COST:
    return "COST";
  case A_RAND:
    return "RAND";
  case A_LISTEN:
    return "LISTEN";
  case A_AAHEAR:
    return "AAHEAR";
  case A_AMHEAR:
    return "AMHEAR";
  case A_AHEAR:
    return "AHEAR";
  case A_LAST:
    return "LAST";
  case A_QUEUE:
    return "QUEUE";
  case A_IDESC:
    return "IDESCRIBE";
  case A_ENTER:
    return "ENTER";
  case A_OXENTER:
    return "OXENTER";
  case A_AENTER:
    return "AENTER";
  case A_ADESC:
    return "ADESCRIBE";
  case A_ODESC:
    return "ODESCRIBE";
  case A_RQUOTA:
    return "RQUOTA";
  case A_ACONNECT:
    return "ACONNECT";
  case A_ADISCONNECT:
    return "ADISCONNECT";
  case A_LEAVE:
    return "LEAVE";
  case A_ALEAVE:
    return "ALEAVE";
  case A_OLEAVE:
    return "OLEAVE";
  case A_OENTER:
    return "OENTER";
  case A_OXLEAVE:
    return "OXLEAVE";
  default:
    if (oldatr >= 100 && oldatr < 178) {
      result[0] = 'V' + (oldatr - 100) / 26;
      result[1] = 'A' + (oldatr - 100) % 26;
      return result;
    } else {
      fprintf(stderr,
	  "ERROR: Invalid attribute number in convert_atr. aborting.\n");
      fflush(stderr);
      abort();
    }
  }
  /*NOTREACHED */
  return "";
}

/*======================================================================*/

static char *
atr_hash_lookup(s)
    char const *s;
{
  return (char *) hashfind(s, &htab_temp_attrib);
}

static void
atr_hash_insert(name)
    char const *name;
{
  hashadd(name, (void *) name, &htab_temp_attrib);
}

static void
atr_hash_free()
{
  hashflush(&htab_temp_attrib, 32);
}

static void
atr_hash_init()
{
  hashinit(&htab_temp_attrib, 32);
  atr_hash_initialized = 1;
}

int
atr_comm_match(thing, player, type, end, str, just_match)
    dbref thing;
    dbref player;
    int type;
    int end;
    char const *str;
    int just_match;
{
  int flag_mask;
  ATTR *ptr;
  int parent_depth;
  char tbuf1[BUFFER_LEN];
  char *s;
  int match;
  dbref parent;

  /* check for lots of easy ways out */
  if ((type != '$' && type != '^') || !GoodObject(thing) || Halted(thing) ||
      (NoCommand(thing) && type == '$') || !eval_lock(player, thing, Use_Lock))
    return 0;

  if (type == '$') {
    flag_mask = AF_COMMAND;
    parent_depth = GoodObject(Parent(thing));
  } else {
    flag_mask = AF_LISTEN;
    parent_depth = 0;
  }

  match = 0;
  if (!atr_hash_initialized)
    atr_hash_init();

  for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr)) {
#ifndef VISIBLE_EMPTY_ATTRS
    if (!*AL_STR(ptr))
      continue;
#endif
    if (parent_depth)
      atr_hash_insert(AL_NAME(ptr));
    if (AL_FLAGS(ptr) & AF_NOPROG || !(AL_FLAGS(ptr) & flag_mask))
      continue;
    strcpy(tbuf1, uncompress(AL_STR(ptr)));
    s = tbuf1;
    do {
      s = strchr(s + 1, end);
    } while (s && s[-1] == '\\');
    if (!s)
      continue;
    *s++ = '\0';
    if ((AL_FLAGS(ptr) & AF_REGEXP) ?
	regexp_match_case(tbuf1 + 1, str, (AL_FLAGS(ptr) & AF_CASE)) :
	wild_match_case(tbuf1 + 1, str, (AL_FLAGS(ptr) & AF_CASE))) {
      match++;
      if (!just_match)
	parse_que(thing, s, player);
    }
  }

  if (!parent_depth)
    return match;

  for (parent_depth = MAX_PARENTS, parent = Parent(thing);
       parent_depth-- && parent != NOTHING;
       parent = Parent(parent)) {
    if (!eval_lock(player, parent, Use_Lock))
      break;
    for (ptr = List(parent); ptr; ptr = AL_NEXT(ptr)) {
#ifndef VISIBLE_EMPTY_ATTRS
      if (!*AL_STR(ptr))
	continue;
#endif
      if (atr_hash_lookup(AL_NAME(ptr)))
	continue;
      if (Parent(parent) != NOTHING)
	atr_hash_insert(AL_NAME(ptr));
      if (AL_FLAGS(ptr) & AF_NOPROG || !(AL_FLAGS(ptr) & flag_mask))
	continue;
      strcpy(tbuf1, uncompress(AL_STR(ptr)));
      s = tbuf1;
      do {
	s = strchr(s + 1, end);
      } while (s && s[-1] == '\\');
      if (!s)
	continue;
      *s++ = '\0';
      if ((AL_FLAGS(ptr) & AF_REGEXP) ?
	  regexp_match_case(tbuf1 + 1, str, (AL_FLAGS(ptr) & AF_CASE)) :
	  wild_match_case(tbuf1 + 1, str, (AL_FLAGS(ptr) & AF_CASE))) {
	match++;
	if (!just_match)
	  parse_que(thing, s, player);
      }
    }
  }

  atr_hash_free();
  return match;
}

/*======================================================================*/

/* This is a new interface (as of 1.6.9p0) for setting attributes,
 * which takes care of case-fixing, object-level flag munging,
 * alias recognition, add/clr distinction, etc.  Enjoy. */
int
do_set_atr(thing, atr, s, player, flags)
    dbref thing;
    char const *atr;
    char const *s;
    dbref player;
    int flags;
{
  ATTR *old;
  char name[BUFFER_LEN];
  char tbuf1[BUFFER_LEN];
  int res;
  int was_hearer;
  int was_listener;
  dbref contents;


#ifndef DELETE_ATTRS
  if (!s)
    s = "";
#endif
#ifndef EMPTY_ATTRS
  if (s && !*s)
    s = NULL;
#endif

  if (!controls(player, thing))
    return 0;

  strcpy(name, atr);
  upcasestr(name);
  if (!strcmp(name, "ALIAS") && IsPlayer(thing)) {
    old = atr_get_noparent(thing, "ALIAS");
    if (old) {
      /* Old alias - we're allowed to change to a different case */
      strcpy(tbuf1, uncompress(AL_STR(old)));
      if (s && *s && strcasecmp(s, tbuf1) && !ok_player_name(s)) {
	notify(player, "That is not a valid alias.");
	return -1;
      }
      if (Can_Write_Attr(player, thing, old))
	delete_player(thing, tbuf1);
    } else {
      /* No old alias */
      if (s && *s && !ok_player_name(s)) {
	notify(player, "That is not a valid alias.");
	return -1;
      }
    }
  }
  was_hearer = Hearer(thing);
  was_listener = Listener(thing);

  res = s ? atr_add(thing, name, s, player, (flags & 0x02) ? AF_NOPROG : NOTHING)
    : atr_clr(thing, name, player);
  if (res == -1) {
    notify(player, "That attribute cannot be changed by you.");
    return 0;
  } else if (!res) {
    notify(player, "No such attribute to reset.");
    return 0;
  }
  if (!strcmp(name, "ALIAS") && IsPlayer(thing)) {
    if (s && *s) {
      add_player(thing, (char *) s);
      notify(player, "Alias set.");
    } else {
      notify(player, "Alias removed.");
    }
    return 1;
  } else if (!strcmp(name, "LISTEN")) {
    if (IsRoom(thing))
      contents = Contents(thing);
    else
      contents = Contents(Location(thing));
    if (!s && !was_listener && !Hearer(thing)) {
      notify_except(contents, thing,
		    tprintf("%s loses its ears and becomes deaf.",
			    Name(thing)));
    } else if (s && !was_hearer && !was_listener) {
      notify_except(contents, thing,
		    tprintf("%s grows ears and can now hear.",
			    Name(thing)));
    }
  } else if (!strcmp(name, "STARTUP")) {
    if (s && *s)
      Flags(thing) |= STARTUP;
    else
      Flags(thing) &= ~STARTUP;
  }
  if ((flags & 0x01) && !Quiet(player) && !Quiet(thing))
    notify(player, tprintf("%s/%s - %s.", Name(thing), name, s ? "Set" : "Cleared"));
  return 1;
}

void
do_atrlock(player, arg1, arg2)
    dbref player;
    const char *arg1;
    const char *arg2;
{
  dbref thing;
  char *p;
  ALIST *ptr;
  int status;

  if (!arg2 || !*arg2)
    status = 0;
  else {
    if (!strcasecmp(arg2, "on")) {
      status = 1;
    } else if (!strcasecmp(arg2, "off")) {
      status = 2;
    } else
      status = 0;
  }

  if (!arg1 || !*arg1) {
    notify(player, "You need to give an object/attribute pair.");
    return;
  }
  if (!(p = (char *) index(arg1, '/')) || !(*(p + 1))) {
    notify(player, "You need to give an object/attribute pair.");
    return;
  }
  *p++ = '\0';

  if ((thing = noisy_match_result(player, arg1, NOTYPE, MAT_EVERYTHING)) == NOTHING)
    return;

  for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr))
    if (!strcasecmp(AL_NAME(ptr), p)
#ifndef VISIBLE_EMPTY_ATTRS
	&& *AL_STR(ptr)
#endif
      )
      break;

  if (!ptr) {
    ptr = atr_match(strupper(p));
    if (!ptr || !strcmp(p, AL_NAME(ptr)))
      ptr = NULL;
    else {
      p = (char *) AL_NAME(ptr);
      for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr))
	if (!strcmp(AL_NAME(ptr), p)
#ifndef VISIBLE_EMPTY_ATTRS
	    && *AL_STR(ptr)
#endif
	  )
	  break;
    }
  }
  if (ptr) {
    if (!status) {
      notify(player, tprintf("That attribute is %slocked.",
			     (AL_FLAGS(ptr) & AF_LOCKED) ? "" : "un"));
      return;
    } else if (!Wizard(player) &&
	       (Owner(AL_CREATOR(ptr)) != Owner(player))) {
      notify(player, "You need to own the attribute to change its lock.");
      return;
    } else {
      if (status == 1) {
	AL_FLAGS(ptr) |= AF_LOCKED;
	notify(player, "Attribute locked.");
	return;
      } else if (status == 2) {
	AL_FLAGS(ptr) &= ~AF_LOCKED;
	notify(player, "Attribute unlocked.");
	return;
      } else {
	notify(player, "Invalid status on atrlock.. Notify god.");
	return;
      }
    }
  } else
    notify(player, "No such attribute.");
  return;
}

void
do_atrchown(player, arg1, arg2)
    dbref player;
    const char *arg1;
    const char *arg2;
{
  dbref thing, new_owner;
  char *p;
  ALIST *ptr;

  if (!arg1 || !*arg1) {
    notify(player, "You need to give an object/attribute pair.");
    return;
  }
  if (!(p = (char *) index(arg1, '/')) || !(*(p + 1))) {
    notify(player, "You need to give an object/attribute pair.");
    return;
  }
  *p++ = '\0';

  if ((thing = noisy_match_result(player, arg1, NOTYPE, MAT_EVERYTHING)) == NOTHING)
    return;

  if ((!arg2 && !*arg2) || !strcasecmp(arg2, "me"))
    new_owner = player;
  else
    new_owner = lookup_player(arg2);

  if (new_owner == NOTHING) {
    notify(player, "I can't find that player");
    return;
  }
  for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr))
    if (!strcasecmp(AL_NAME(ptr), p)
#ifndef VISIBLE_EMPTY_ATTRS
	&& *AL_STR(ptr)
#endif
      )
      break;

  if (!ptr) {
    ptr = atr_match(strupper(p));
    if (!ptr || !strcmp(p, AL_NAME(ptr)))
      ptr = NULL;
    else {
      p = (char *) AL_NAME(ptr);
      for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr))
	if (!strcmp(AL_NAME(ptr), p)
#ifndef VISIBLE_EMPTY_ATTRS
	    && *AL_STR(ptr)
#endif
	  )
	  break;
    }
  }
  if (ptr) {
    if ((controls(player, thing) && !(AL_FLAGS(ptr) & AF_LOCKED)) ||
	(Owner(player) == Owner(AL_CREATOR(ptr)))) {
      if (new_owner != Owner(thing) && !Wizard(player)) {
	notify(player,
	       "You can only chown an attribute to the current owner of the object.");
	return;
      }
      AL_CREATOR(ptr) = Owner(new_owner);
      notify(player, "Attribute owner changed.");
      return;
    } else {
      notify(player, "You don't have the permission to chown that.");
      return;
    }
  } else
    notify(player, "No such attribute.");
}
