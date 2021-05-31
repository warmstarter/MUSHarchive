/**
 * \file attrib.c
 *
 * \brief Manipulate attributes on objects.
 *
 *
 */
#include "copyrite.h"

#include "config.h"
#include <string.h>
#include <ctype.h>
#include "conf.h"
#include "attrib.h"
#include "dbdefs.h"
#include "externs.h"
#include "match.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "parse.h"
#include "htab.h"
#include "privtab.h"
#include "mymalloc.h"
#include "strtree.h"
#include "flags.h"
#include "mushdb.h"
#include "lock.h"
#include "log.h"
#include "confmagic.h"

/* A hash table that temporarily stores matching attributes from current
 * and parent objects when we're searching for commands on attributes
 */
StrTree temp_attrib;
StrTree atr_names;
extern PRIV attr_privs[];

/*======================================================================*/

#define ATTRS_PER_PAGE (200)

/** A page of memory for attributes.
 * This structure is a collection of attribute memory. Rather than
 * allocate new attributes one at a time, we allocate them in pages,
 * and build a linked free list from the allocated page.
 */
typedef struct atrpage {
  ATTR atrs[ATTRS_PER_PAGE];	/**< Array of attribute structures */
} ATTRPAGE;

static ATTR *atr_free_list;
static ATTR *get_atr_free_list(void);

/*======================================================================*/

/** Initialize the attribute string tree.
 */
void
init_atr_name_tree(void)
{
  st_init(&atr_names);
}

/* Lookup table for good_atr_name */
extern char atr_name_table[UCHAR_MAX + 1];

/** Decide if a name is valid for an attribute.
 * A good attribute name is at least one character long, no more than
 * ATTRIBUTE_NAME_LIMIT characters long, and every character is a 
 * valid character.
 * \param s a string to test for validity as an attribute name.
 */
int
good_atr_name(char const *s)
{
  const unsigned char *a;
  int len = 0;
  if (!s || !*s)
    return 0;
  for (a = (const unsigned char *) s; *a; a++, len++)
    if (!atr_name_table[*a])
      return 0;
  return len <= ATTRIBUTE_NAME_LIMIT;
}

/** Find an attribute table entry, given a name.
 * A trivial wrapper around aname_hash_lookup.
 * \param string an attribute name.
 */
ATTR *
atr_match(const char *string)
{
  return aname_hash_lookup(string);
}

/** Convert a string of attribute flags to a bitmask.
 * Given a space-separated string of attribute flags, look them up
 * and return a bitmask of them if player is permitted to use
 * all of those flags.
 * \param player the dbref to use for privilege checks.
 * \param p a space-separated string of attribute flags.
 * \return an attribute flag bitmask.
 */
int
string_to_atrflag(dbref player, char const *p)
{
  int f;
  f = string_to_privs(attr_privs, p, 0);
  if (!f)
    return -1;
  if (!Hasprivs(player) && (f & AF_MDARK))
    return -1;
  if (!See_All(player) && (f & AF_WIZARD))
    return -1;
  return f;
}

/** Add an attribute to an object, dangerously.
 * This is a stripped down version of atr_add, without duplicate checking,
 * permissions checking, attribute count checking, or auto-ODARKing.  
 * If anyone uses this outside of database load or atr_cpy (below), 
 * I will personally string them up  by their toes.  - Alex 
 * \param thing object to set the attribute on.
 * \param atr name of the attribute to set.
 * \param s value of the attribute to set.
 * \param player the attribute creator.
 * \param flags bitmask of attribute flags for this attribute.
 */
void
atr_new_add(dbref thing, const char *RESTRICT atr, const char *RESTRICT s,
	    dbref player, int flags)
{
  ATTR *ptr;
  char const *name;
  ATTR **ins;

#ifndef EMPTY_ATTRS
  if (!*s)
    return;
#endif

  ptr = get_atr_free_list();
  atr_free_list = AL_NEXT(ptr);

  /* Don't fail on a bad name, but do log it */
  if (!good_atr_name(atr))
    do_rawlog(LT_ERR, T("Bad attribute name %s on object %s"), atr,
	      unparse_dbref(thing));

  /* put the name in the string table */
  name = st_insert(atr, &atr_names);
  if (!name) {
    AL_NEXT(ptr) = atr_free_list;
    atr_free_list = ptr;
    return;
  }
  /* initialize atr */
  AL_NAME(ptr) = name;
  AL_FLAGS(ptr) = (flags != NOTHING) ? flags : 0;
  AL_FLAGS(ptr) &= ~AF_COMMAND & ~AF_LISTEN;
  AL_CREATOR(ptr) = player;
  AL_STR(ptr) = NULL;

  /* link it in */
  ins = &List(thing);
  while (*ins && strcoll(AL_NAME(*ins), AL_NAME(ptr)) < 0)
    ins = &AL_NEXT(*ins);
  AL_NEXT(ptr) = *ins;
  *ins = ptr;

  /* replace string with new string */
  if (!s || !*s) {
    AL_STR(ptr) = (unsigned char *) "";
  } else {
    AL_STR(ptr) = compress(s);
    if (!AL_STR(ptr)) {
      AL_STR(ptr) = (unsigned char *) "";
      return;
    }
#ifdef MEM_CHECK
    add_check(T("ATTR value"));
#endif
  }
  if (*s == '$')
    AL_FLAGS(ptr) |= AF_COMMAND;
  if (*s == '^')
    AL_FLAGS(ptr) |= AF_LISTEN;
}

#define AE_BADNAME -3
#define AE_SAFE -2
#define AE_ERROR -1

/** Add an attribute to an object, safely.
 * \verbatim
 * This is the function that should be called in hardcode to add
 * an attribute to an object (but not to process things like @set that
 * may add or clear an attribute - see do_set_atr() for that).
 * \endverbatim
 * \param thing object to set the attribute on.
 * \param atr name of the attribute to set.
 * \param s value of the attribute to set.
 * \param player the attribute creator.
 * \param flags bitmask of attribute flags for this attribute.
 * \retval AE_BADNAME invalid attribute name.
 * \retval AE_SAFE attempt to overwrite a SAFE attribute.
 * \retval AE_ERROR general failure.
 * \retval 1 success.
 */
int
atr_add(dbref thing, const char *RESTRICT atr, const char *RESTRICT s,
	dbref player, int flags)
{
  ATTR *ptr, *std;
  char const *name;
  ATTR **ins;
  int comp = 0;

  if (!s)
    return atr_clr(thing, atr, player);

#ifndef EMPTY_ATTRS
  if (!*s)
    return atr_clr(thing, atr, player);
#endif

  if (!good_atr_name(atr))
    return AE_BADNAME;

  /* walk the list, looking for a preexisting value */
  ptr = List(thing);
  while (ptr && (comp = strcoll(atr, AL_NAME(ptr))) > 0)
    ptr = AL_NEXT(ptr);
  if (comp)
    ptr = NULL;

  /* check for permission to modify existing atr */
  if (ptr && (AL_FLAGS(ptr) & AF_SAFE))
    return AE_SAFE;
  if (ptr && !Can_Write_Attr(player, thing, ptr))
    return AE_ERROR;

  /* make a new atr, if needed */
  if (!ptr) {
    /* allocate a new page, if needed */
    ptr = get_atr_free_list();
    atr_free_list = AL_NEXT(ptr);

    /* put the name in the string table */
    name = st_insert(atr, &atr_names);
    if (!name) {
      AL_NEXT(ptr) = atr_free_list;
      atr_free_list = ptr;
      return AE_ERROR;
    }
    /* initialize atr */
    AL_NAME(ptr) = name;
    std = atr_match(atr);
    if (std && !strcmp(AL_NAME(std), atr)) {
      AL_FLAGS(ptr) = AL_FLAGS(std);
    } else {
      AL_FLAGS(ptr) = 0;
      if (flags != NOTHING)
	AL_FLAGS(ptr) |= flags;
    }
    AL_CREATOR(ptr) = player;
    AL_STR(ptr) = NULL;

    /* check for permission to create atr */
    if (!Can_Write_Attr(player, thing, ptr) ||
	(AttrCount(thing) >= MAX_ATTRCOUNT)) {
      st_delete(AL_NAME(ptr), &atr_names);
      AL_NEXT(ptr) = atr_free_list;
      atr_free_list = ptr;
      if (AttrCount(thing) >= MAX_ATTRCOUNT)
	do_log(LT_ERR, player, thing,
	       T("Attempt by %s(%d) to create too many attributes on %s(%d)"),
	       Name(player), player, Name(thing), thing);
      return AE_ERROR;
    }
    /* link it in */
    ins = &List(thing);
    while (*ins && strcoll(AL_NAME(*ins), AL_NAME(ptr)) < 0)
      ins = &AL_NEXT(*ins);
    AL_NEXT(ptr) = *ins;
    *ins = ptr;
    AttrCount(thing)++;
  }
  /* update modification time here, because from now on,
   * we modify even if we fail */
  if (!IsPlayer(thing) && !(AL_FLAGS(ptr) & AF_NODUMP))
    ModTime(thing) = mudtime;

  /* change owner */
  AL_CREATOR(ptr) = Owner(player);

  AL_FLAGS(ptr) &= ~AF_COMMAND & ~AF_LISTEN;

  /* replace string with new string */
  if (AL_STR(ptr) && *AL_STR(ptr))
    mush_free((Malloc_t) AL_STR(ptr), T("ATTR value"));
  if (!s || !*s) {
    AL_STR(ptr) = (unsigned char *) "";
  } else {
    AL_STR(ptr) = compress(s);
    if (!AL_STR(ptr)) {
      AL_STR(ptr) = (unsigned char *) "";
      return AE_ERROR;
    }
#ifdef MEM_CHECK
    add_check(T("ATTR value"));
#endif
  }
  if (*s == '$')
    AL_FLAGS(ptr) |= AF_COMMAND;
  if (*s == '^')
    AL_FLAGS(ptr) |= AF_LISTEN;

  return 1;
}

/** Remove an attribute from an object.
 * This function clears an attribute from an object. 
 * \param thing object to clear attribute from.
 * \param atr name of attribute to remove.
 * \param player enactor attempting to remove attribute.
 */
int
atr_clr(dbref thing, char const *atr, dbref player)
{
  ATTR *ptr, **prev;
  int comp = 0;

#ifdef EMPTY_ATTRS
#ifndef DELETE_ATTRS
  return atr_add(thing, atr, "", player, NOTHING);
#endif
#endif

  prev = &List(thing);
  ptr = *prev;
  while (ptr && (comp = strcoll(atr, AL_NAME(ptr))) > 0) {
    prev = &AL_NEXT(ptr);
    ptr = *prev;
  }
  if (comp)
    ptr = NULL;

  if (!ptr)
    return 0;

  if (!Can_Write_Attr(player, thing, ptr))
    return -1;

  if (!IsPlayer(thing) && !(AL_FLAGS(ptr) & AF_NODUMP))
    ModTime(thing) = mudtime;

  *prev = AL_NEXT(ptr);

  if (AL_STR(ptr) && *AL_STR(ptr))
    mush_free((Malloc_t) AL_STR(ptr), T("ATTR value"));
  st_delete(AL_NAME(ptr), &atr_names);

  AL_NEXT(ptr) = atr_free_list;
  AL_FLAGS(ptr) = 0;
  atr_free_list = ptr;
  AttrCount(thing)--;
  return 1;
}

/** Retrieve an attribute from an object or its ancestors.
 * This function retrieves an attribute from an object, or from its
 * parent chain, returning a pointer to the first attribute that
 * matches or NULL. This is a pointer to an attribute structure, not
 * to the value of the attribute, so the (compressed) value is usually 
 * accessed through AL_STR() (and then uncompressed).
 * \param thing the object containing the attribute.
 * \param atr the name of the attribute.
 * \return pointer to the attribute structure retrieved, or NULL.
 */
ATTR *
atr_get(dbref thing, char const *atr)
{
  ATTR *ptr;
  int parent_depth;
  dbref temp;
  int comp;

  if (thing == NOTHING || !good_atr_name(atr))
    return NULL;

  /* try real name */
  for (parent_depth = 0, temp = thing;
       parent_depth < MAX_PARENTS && temp != NOTHING;
       parent_depth++, temp = Parent(temp))
    for (ptr = List(temp); ptr; ptr = AL_NEXT(ptr)) {
      comp = strcoll(atr, AL_NAME(ptr));
      if (comp < 0)
	break;
      if (comp == 0
#ifndef VISIBLE_EMPTY_ATTRS
	  && *AL_STR(ptr)
#endif
	) {
	if (temp == thing || !(AL_FLAGS(ptr) & AF_PRIVATE))
	  return ptr;
	else
	  break;
      }
    }
  ptr = atr_match(atr);
  if (!ptr || !strcmp(atr, AL_NAME(ptr)))
    return NULL;
  atr = AL_NAME(ptr);

  /* try alias */
  for (parent_depth = 0, temp = thing;
       parent_depth < MAX_PARENTS && temp != NOTHING;
       parent_depth++, temp = Parent(temp))
    for (ptr = List(temp); ptr; ptr = AL_NEXT(ptr)) {
      comp = strcoll(atr, AL_NAME(ptr));
      if (comp < 0)
	break;
      if (comp == 0
#ifndef VISIBLE_EMPTY_ATTRS
	  && *AL_STR(ptr)
#endif
	) {
	if (temp == thing || !(AL_FLAGS(ptr) & AF_PRIVATE))
	  return ptr;
	else
	  break;
      }
    }
  return NULL;
}

/** Retrieve an attribute from an object.
 * This function retrieves an attribute from an object, and does not
 * check the parent chain. It returns a pointer to the attribute
 * or NULL.  This is a pointer to an attribute structure, not
 * to the value of the attribute, so the (compressed) value is usually 
 * accessed through AL_STR() (and then uncompressed).
 * \param thing the object containing the attribute.
 * \param atr the name of the attribute.
 * \return pointer to the attribute structure retrieved, or NULL.
 */
ATTR *
atr_get_noparent(dbref thing, char const *atr)
{
  ATTR *ptr;
  int comp;

  if (thing == NOTHING || !good_atr_name(atr))
    return NULL;

  /* try real name */
  for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr)) {
    comp = strcoll(atr, AL_NAME(ptr));
    if (comp < 0)
      break;
    if (comp == 0
#ifndef VISIBLE_EMPTY_ATTRS
	&& *AL_STR(ptr)
#endif
      )
      return ptr;
  }

  ptr = atr_match(atr);
  if (!ptr || !strcmp(atr, AL_NAME(ptr)))
    return NULL;
  atr = AL_NAME(ptr);

  /* try alias */
  for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr)) {
    comp = strcoll(atr, AL_NAME(ptr));
    if (comp < 0)
      break;
    if (comp == 0
#ifndef VISIBLE_EMPTY_ATTRS
	&& *AL_STR(ptr)
#endif
      )
      return ptr;
  }

  return NULL;
}


/** Apply a function to a set of attributes.
 * This function applies another function to a set of attributes on an
 * object specified by a (wildcarded) pattern to match against the
 * attribute name.
 * \param player the enactor.
 * \param thing the object containing the attribute.
 * \param name the pattern to match against the attribute name.
 * \param func the function to call for each matching attribute.
 * \param args additional arguments to pass to the function.
 * \return the sum of the return values of the functions called.
 */
int
atr_iter_get(dbref player, dbref thing, const char *name, aig_func func,
	     void *args)
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

/** Free the memory associated with all attributes of an object.
 * This function frees all of an object's attribute memory.
 * This includes the memory allocated to hold the attribute's value,
 * and the attribute's entry in the object's string tree.
 * Freed attribute structures are added to the free list.
 * \param thing dbref of object
 */
void
atr_free(dbref thing)
{
  ATTR *ptr;

  if (!List(thing))
    return;

  if (!IsPlayer(thing))
    ModTime(thing) = mudtime;

  while ((ptr = List(thing))) {
    List(thing) = AL_NEXT(ptr);

    if (AL_STR(ptr) && *AL_STR(ptr))
      mush_free((Malloc_t) AL_STR(ptr), T("ATTR value"));
    st_delete(AL_NAME(ptr), &atr_names);

    AL_NEXT(ptr) = atr_free_list;
    atr_free_list = ptr;
  }
}

/** Copy all of the attributes from one object to another.
 * \verbatim
 * This function is used by @clone to copy all of the attributes
 * from one object to another.
 * \endverbatim
 * \param dest destination object to receive attributes.
 * \param source source object containing attributes.
 */
void
atr_cpy(dbref dest, dbref source)
{
  ATTR *ptr;

  List(dest) = NULL;
  for (ptr = List(source); ptr; ptr = AL_NEXT(ptr))
    if (!(AL_FLAGS(ptr) & AF_NOCOPY)
#ifndef VISIBLE_EMPTY_ATTRS
	&& *AL_STR(ptr)
#endif
	&& (AttrCount(dest) < MAX_ATTRCOUNT)) {
      atr_new_add(dest, AL_NAME(ptr), uncompress(AL_STR(ptr)),
		  AL_CREATOR(ptr), AL_FLAGS(ptr));
      AttrCount(dest)++;
    }
}

/** Match input against a $command or ^listen attribute.
 * This function attempts to match a string against either the $commands
 * or ^listens on an object. Matches may be glob or regex matches, 
 * depending on the attribute's flags.
 * \param thing object containing attributes to check.
 * \param player the enactor, for privilege checks.
 * \param type either '$' or '^', indicating the type of attribute to check.
 * \param end character that denotes the end of a command (usually ':').
 * \param str string to match against attributes.
 * \param just_match if true, return match without executing code.
 * \param atrname used to return the list of matching object/attributes.
 * \param abp pointer to end of atrname.
 * \return number of attributes that matched, or 0
 */
int
atr_comm_match(dbref thing, dbref player, int type, int end, char const *str,
	       int just_match, char *atrname, char **abp)
{
  int flag_mask;
  ATTR *ptr;
  int parent_depth;
  char tbuf1[BUFFER_LEN];
  char tbuf2[BUFFER_LEN];
  char *s;
  int match;
  dbref parent;

  /* check for lots of easy ways out */
  if ((type != '$' && type != '^') || !GoodObject(thing) || Halted(thing)
      || (type == '$'
	  && (NoCommand(thing) || !eval_lock(player, thing, Command_Lock)))
      || (type == '^' && !eval_lock(player, thing, Listen_Lock))
      || !eval_lock(player, thing, Use_Lock))
    return 0;

  if (type == '$') {
    flag_mask = AF_COMMAND;
    parent_depth = GoodObject(Parent(thing));
  } else {
    flag_mask = AF_LISTEN;
    if (ThingInhearit(thing) || RoomInhearit(thing)) {
      parent_depth = GoodObject(Parent(thing));
    } else {
      parent_depth = 0;
    }
  }

  match = 0;
  st_init(&temp_attrib);

  for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr)) {
#ifndef VISIBLE_EMPTY_ATTRS
    if (!*AL_STR(ptr))
      continue;
#endif
    if (parent_depth)
      st_insert(AL_NAME(ptr), &temp_attrib);
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

    if (AL_FLAGS(ptr) & AF_REGEXP) {
      /* Turn \: into : */
      char *from, *to;
      for (from = tbuf1, to = tbuf2; *from; from++, to++) {
	if (*from == '\\' && *(from + 1) == ':')
	  from++;
	*to = *from;
      }
      *to = '\0';
    } else
      strcpy(tbuf2, tbuf1);

    if ((AL_FLAGS(ptr) & AF_REGEXP) ?
	regexp_match_case(tbuf2 + 1, str, (AL_FLAGS(ptr) & AF_CASE)) :
	wild_match_case(tbuf2 + 1, str, (AL_FLAGS(ptr) & AF_CASE))) {
      match++;
      if (atrname && abp) {
	safe_chr(' ', atrname, abp);
	safe_dbref(thing, atrname, abp);
	safe_chr('/', atrname, abp);
	safe_str(AL_NAME(ptr), atrname, abp);
      }
      if (!just_match)
	parse_que(thing, s, player);
    }
  }

  if (!parent_depth)
    return match;

  for (parent_depth = MAX_PARENTS, parent = Parent(thing);
       parent_depth-- && parent != NOTHING; parent = Parent(parent)) {
    for (ptr = List(parent); ptr; ptr = AL_NEXT(ptr)) {
#ifndef VISIBLE_EMPTY_ATTRS
      if (!*AL_STR(ptr))
	continue;
#endif
      if (st_find(AL_NAME(ptr), &temp_attrib))
	continue;
      if (Parent(parent) != NOTHING)
	st_insert(AL_NAME(ptr), &temp_attrib);
      if (AL_FLAGS(ptr) & (AF_NOPROG | AF_PRIVATE)
	  || !(AL_FLAGS(ptr) & flag_mask))
	continue;
      strcpy(tbuf1, uncompress(AL_STR(ptr)));
      s = tbuf1;
      do {
	s = strchr(s + 1, end);
      } while (s && s[-1] == '\\');
      if (!s)
	continue;
      *s++ = '\0';

      if (AL_FLAGS(ptr) & AF_REGEXP) {
	/* Turn \: into : */
	char *from, *to;
	for (from = tbuf1, to = tbuf2; *from; from++, to++) {
	  if (*from == '\\' && *(from + 1) == ':')
	    from++;
	  *to = *from;
	}
	*to = '\0';
      } else
	strcpy(tbuf2, tbuf1);

      if ((AL_FLAGS(ptr) & AF_REGEXP) ?
	  regexp_match_case(tbuf2 + 1, str, (AL_FLAGS(ptr) & AF_CASE)) :
	  wild_match_case(tbuf2 + 1, str, (AL_FLAGS(ptr) & AF_CASE))) {
	match++;
	if (atrname && abp) {
	  safe_chr(' ', atrname, abp);
	  if (Can_Examine(player, parent))
	    safe_dbref(parent, atrname, abp);
	  else
	    safe_dbref(thing, atrname, abp);
	  safe_chr('/', atrname, abp);
	  safe_str(AL_NAME(ptr), atrname, abp);
	}
	if (!just_match)
	  parse_que(thing, s, player);
      }
    }
  }
  st_flush(&temp_attrib);
  return match;
}

/*======================================================================*/

/** Set or clear an attribute on an object.
 * \verbatim
 * This is the primary function for implementing @set.
 * A new interface (as of 1.6.9p0) for setting attributes,
 * which takes care of case-fixing, object-level flag munging,
 * alias recognition, add/clr distinction, etc.  Enjoy.
 * \endverbatim
 * \param thing object to set the attribute on or remove it from.
 * \param atr name of the attribute to set or clear.
 * \param s value to set the attribute to (or NULL to clear).
 * \param player enactor, for permission checks.
 * \param flags attribute flags.
 * \retval -1 failure of one sort.
 * \retval 0 failure of another sort.
 * \retval 1 success.
 */
int
do_set_atr(dbref thing, const char *RESTRICT atr, const char *RESTRICT s,
	   dbref player, int flags)
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

  if (IsGarbage(thing)) {
    notify(player, T("Garbage is garbage."));
    return 0;
  }
  if (!controls(player, thing))
    return 0;

  strcpy(name, atr);
  upcasestr(name);
  if (!strcmp(name, "ALIAS") && IsPlayer(thing)) {
    old = atr_get_noparent(thing, "ALIAS");
    if (old) {
      /* Old alias - we're allowed to change to a different case */
      strcpy(tbuf1, uncompress(AL_STR(old)));
      if (s && (!*s || (strcasecmp(s, tbuf1) && !ok_player_name(s)))) {
	notify(player, T("That is not a valid alias."));
	return -1;
      }
      if (Can_Write_Attr(player, thing, old))
	delete_player(thing, tbuf1);
    } else {
      /* No old alias */
      if (s && *s && !ok_player_name(s)) {
	notify(player, T("That is not a valid alias."));
	return -1;
      }
    }
  } else if (s && *s && !strcmp(name, "FORWARDLIST")) {
    /* You can only set this to dbrefs of things you're allowed to
     * forward to. If you get one wrong, we puke.
     */
    char *fwdstr, *curr;
    dbref fwd;
    strcpy(tbuf1, s);
    fwdstr = trim_space_sep(tbuf1, ' ');
    while ((curr = split_token(&fwdstr, ' ')) != NULL) {
      if (!is_dbref(curr)) {
	notify(player, T("@forwardlist should contain only dbrefs."));
	return -1;
      }
      fwd = parse_dbref(curr);
      if (!GoodObject(fwd) || IsGarbage(fwd)) {
	notify_format(player, T("Invalid dbref #%d in @forwardlist."), fwd);
	return -1;
      }
      if (!Can_Forward(thing, fwd)) {
	notify_format(player, T("I don't think #%d wants to hear from you."),
		      fwd);
	return -1;
      }
    }
    /* If you made it here, all your dbrefs were ok */
  }

  was_hearer = Hearer(thing);
  was_listener = Listener(thing);

  res =
    s ? atr_add(thing, name, s, player, (flags & 0x02) ? AF_NOPROG : NOTHING)
    : atr_clr(thing, name, player);
  if (res == AE_SAFE) {
    notify(player, T("That attribute is SAFE.  Set it !SAFE to modify it."));
    return 0;
  } else if (res == AE_BADNAME) {
    notify(player, T("That's not a very good name for an attribute."));
    return 0;
  } else if (res == AE_ERROR) {
    notify(player, T("That attribute cannot be changed by you."));
    return 0;
  } else if (!res) {
    notify(player, T("No such attribute to reset."));
    return 0;
  }
  if (!strcmp(name, "ALIAS") && IsPlayer(thing)) {
    if (s && *s) {
      add_player(thing, s);
      notify(player, T("Alias set."));
    } else {
      notify(player, T("Alias removed."));
    }
    return 1;
  } else if (!strcmp(name, "LISTEN")) {
    if (IsRoom(thing))
      contents = Contents(thing);
    else {
      contents = Location(thing);
      if (GoodObject(contents))
	contents = Contents(contents);
    }
    if (GoodObject(contents)) {
      if (!s && !was_listener && !Hearer(thing)) {
	notify_except(contents, thing,
		      tprintf(T("%s loses its ears and becomes deaf."),
			      Name(thing)), NA_INTERACTION);
      } else if (s && !was_hearer && !was_listener) {
	notify_except(contents, thing,
		      tprintf(T("%s grows ears and can now hear."),
			      Name(thing)), NA_INTERACTION);
      }
    }
  }
  if ((flags & 0x01) && !AreQuiet(player, thing))
    notify_format(player,
		  "%s/%s - %s.", Name(thing), name,
		  s ? T("Set") : T("Cleared"));
  return 1;
}

/** Lock or unlock an attribute.
 * Attribute locks are largely obsolete and should be deprecated,
 * but this is the code that does them.
 * \param player the enactor.
 * \param arg1 the object/attribute, as a string.
 * \param arg2 the desired lock status ('on' or 'off').
 */
void
do_atrlock(dbref player, const char *arg1, const char *arg2)
{
  dbref thing;
  char *p;
  ATTR *ptr;
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
    notify(player, T("You need to give an object/attribute pair."));
    return;
  }
  if (!(p = strchr(arg1, '/')) || !(*(p + 1))) {
    notify(player, T("You need to give an object/attribute pair."));
    return;
  }
  *p++ = '\0';

  if ((thing = noisy_match_result(player, arg1, NOTYPE, MAT_EVERYTHING)) ==
      NOTHING)
    return;

  if (!controls(player, thing)) {
    notify(player, T("Permission denied."));
    return;
  }

  ptr = atr_get_noparent(thing, strupper(p));

  if (ptr && Can_Read_Attr(player, thing, ptr)) {
    if (!status) {
      notify_format(player, T("That attribute is %slocked."),
		    (AL_FLAGS(ptr) & AF_LOCKED) ? "" : "un");
      return;
    } else if (!Can_Write_Attr(player, thing, ptr)) {
      notify(player,
	     T("You need to be able to set the attribute to change its lock."));
      return;
    } else {
      if (status == 1) {
	AL_FLAGS(ptr) |= AF_LOCKED;
	AL_CREATOR(ptr) = Owner(player);
	notify(player, T("Attribute locked."));
	return;
      } else if (status == 2) {
	AL_FLAGS(ptr) &= ~AF_LOCKED;
	notify(player, T("Attribute unlocked."));
	return;
      } else {
	notify(player, T("Invalid status on atrlock.. Notify god."));
	return;
      }
    }
  } else
    notify(player, T("No such attribute."));
  return;
}

/** Change ownership of an attribute.
 * \verbatim
 * This function is used to implement @atrchown.
 * \endverbatim
 * \param player the enactor, for permission checking.
 * \param arg1 the object/attribute to change, as a string.
 * \param arg2 the name of the new owner (or "me").
 */
void
do_atrchown(dbref player, const char *arg1, const char *arg2)
{
  dbref thing, new_owner;
  char *p;
  ATTR *ptr;

  if (!arg1 || !*arg1) {
    notify(player, T("You need to give an object/attribute pair."));
    return;
  }
  if (!(p = strchr(arg1, '/')) || !(*(p + 1))) {
    notify(player, T("You need to give an object/attribute pair."));
    return;
  }
  *p++ = '\0';

  if ((thing = noisy_match_result(player, arg1, NOTYPE, MAT_EVERYTHING)) ==
      NOTHING)
    return;

  if (!controls(player, thing)) {
    notify(player, T("Permission denied."));
    return;
  }

  if ((!arg2 && !*arg2) || !strcasecmp(arg2, "me"))
    new_owner = player;
  else
    new_owner = lookup_player(arg2);

  if (new_owner == NOTHING) {
    notify(player, T("I can't find that player"));
    return;
  }

  ptr = atr_get_noparent(thing, strupper(p));

  if (ptr && Can_Read_Attr(player, thing, ptr)) {
    if (Can_Write_Attr(player, thing, ptr)) {
      if (new_owner != Owner(player) && !Wizard(player)) {
	notify(player, T("You can only chown an attribute to yourself."));
	return;
      }
      AL_CREATOR(ptr) = Owner(new_owner);
      notify(player, T("Attribute owner changed."));
      return;
    } else {
      notify(player, T("You don't have the permission to chown that."));
      return;
    }
  } else
    notify(player, T("No such attribute."));
}


/** Return the head of the attribute free list.
 * This function returns the head of the attribute free list. If there
 * are no more ATTR's on the free list, allocate a new page.
 */
static ATTR *
get_atr_free_list(void)
{
  if (!atr_free_list) {
    ATTRPAGE *page;
    int j;
    page = (ATTRPAGE *) mush_malloc(sizeof(ATTRPAGE), "ATTRPAGE");
    if (!page)
      panic("Couldn't allocate memory in get_atr_free_list");
    for (j = 0; j < ATTRS_PER_PAGE - 1; j++)
      AL_NEXT(page->atrs + j) = page->atrs + j + 1;
    AL_NEXT(page->atrs + ATTRS_PER_PAGE - 1) = NULL;
    atr_free_list = page->atrs;
  }
  return atr_free_list;
}
