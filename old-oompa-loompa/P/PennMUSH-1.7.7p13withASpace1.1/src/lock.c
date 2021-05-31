/**
 * \file lock.c
 *
 * \brief Locks for PennMUSH.
 *
 * \verbatim
 *
 * This is the core of Ralph Melton's rewrite of the @lock system.
 * These are some of the underlying assumptions:
 *
 * 1) Locks are checked many more times than they are set, so it is
 * quite worthwhile to spend time when setting locks if it expedites
 * checking locks later.
 *
 * 2) Most possible locks are never used. For example, in the days
 * when there were only basic locks, use locks, and enter locks, in
 * one database of 15000 objects, there were only about 3500 basic
 * locks, 400 enter locks, and 400 use locks.
 * Therefore, it is important to make the case where no lock is present
 * efficient both in time and in memory.
 *
 * 3) It is far more common to have the server itself check for locks
 * than for people to check for locks in MUSHcode. Therefore, it is
 * reasonable to incur a minor slowdown for checking locks in MUSHcode
 * in order to speed up the server's checking.
 *
 * \endverbatim
 */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conf.h"
#include "boolexp.h"
#include "mushdb.h"
#include "attrib.h"
#include "externs.h"
#include "dbdefs.h"
#include "lock.h"
#include "match.h"
#include "log.h"
#include "flags.h"
#include "dbdefs.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "mymalloc.h"
#include "strtree.h"
#include "privtab.h"
#include "parse.h"
#include "confmagic.h"


/* If any lock_type ever contains the character '|', reading in locks
 * from the db will break.
 */
const lock_type Basic_Lock = "Basic";
const lock_type Enter_Lock = "Enter";
const lock_type Use_Lock = "Use";
const lock_type Zone_Lock = "Zone";
const lock_type Page_Lock = "Page";
const lock_type Tport_Lock = "Teleport";
const lock_type Speech_Lock = "Speech";
const lock_type Listen_Lock = "Listen";
const lock_type Command_Lock = "Command";
const lock_type Parent_Lock = "Parent";
const lock_type Link_Lock = "Link";
const lock_type Leave_Lock = "Leave";
const lock_type Drop_Lock = "Drop";
const lock_type Give_Lock = "Give";
const lock_type Mail_Lock = "Mail";
const lock_type Follow_Lock = "Follow";
const lock_type Examine_Lock = "Examine";
const lock_type Chzone_Lock = "Chzone";
const lock_type Forward_Lock = "Forward";
const lock_type Control_Lock = "Control";
const lock_type Dropto_Lock = "Dropto";
const lock_type Destroy_Lock = "Destroy";
/* Define new lock types here. */

const lock_list lock_types[] = {
  {"Basic", NULL, GOD, LF_PRIVATE, NULL},
  {"Enter", NULL, GOD, LF_PRIVATE, NULL},
  {"Use", NULL, GOD, LF_PRIVATE, NULL},
  {"Zone", NULL, GOD, LF_PRIVATE, NULL},
  {"Page", NULL, GOD, LF_PRIVATE, NULL},
  {"Teleport", NULL, GOD, LF_PRIVATE, NULL},
  {"Speech", NULL, GOD, LF_PRIVATE | LF_WIZARD, NULL},
  {"Listen", NULL, GOD, LF_PRIVATE, NULL},
  {"Command", NULL, GOD, LF_PRIVATE, NULL},
  {"Parent", NULL, GOD, LF_PRIVATE, NULL},
  {"Link", NULL, GOD, LF_PRIVATE, NULL},
  {"Leave", NULL, GOD, LF_PRIVATE, NULL},
  {"Drop", NULL, GOD, LF_PRIVATE, NULL},
  {"Give", NULL, GOD, LF_PRIVATE, NULL},
  {"Mail", NULL, GOD, LF_PRIVATE, NULL},
  {"Follow", NULL, GOD, LF_PRIVATE, NULL},
  {"Examine", NULL, GOD, LF_PRIVATE | LF_OWNER, NULL},
  {"Chzone", NULL, GOD, LF_PRIVATE, NULL},
  {"Forward", NULL, GOD, LF_PRIVATE | LF_OWNER, NULL},
  {"Control", NULL, GOD, LF_PRIVATE | LF_OWNER, NULL},
  {"Dropto", NULL, GOD, LF_PRIVATE, NULL},
  {"Destroy", NULL, GOD, LF_PRIVATE | LF_OWNER, NULL},
  /* Add new lock types just before this line. */
  {NULL, NULL, GOD, 0, NULL}
};

PRIV lock_privs[] = {
  {"visual", 'v', LF_VISUAL, LF_VISUAL},
  {"no_inherit", 'i', LF_PRIVATE, LF_PRIVATE},
  {"no_clone", 'c', LF_NOCLONE, LF_NOCLONE},
  {"wizard", 'w', LF_WIZARD, LF_WIZARD},
  /*  {"owner", 'o', LF_OWNER, LF_OWNER}, */
  {"locked", '+', LF_LOCKED, LF_LOCKED},
  {NULL, '\0', 0, 0}
};

StrTree lock_names;

static void free_one_lock_list(lock_list *ll);
static lock_type check_lock_type(dbref player, dbref thing, lock_type name);
static int delete_lock(dbref player, dbref thing, lock_type type);
static int can_write_lock(dbref player, dbref thing, lock_list *lock);
static lock_list *getlockstruct(dbref thing, lock_type type);
static lock_list *getlockstruct_noparent(dbref thing, lock_type type);

/* Assuming 4096 byte pages */
#define LOCKS_PER_PAGE 200

static lock_list *free_list = NULL;

static lock_list *next_free_lock(void);
static void free_lock(lock_list *ll);

/** Return a list of lock flag characters.
 * \param ll pointer to a lock.
 * \return string of lock flag characters.
 */
const char *
lock_flags(lock_list *ll)
{
  return privs_to_letters(lock_privs, L_FLAGS(ll));
}

/** Return a list of lock flag names.
 * \param ll pointer to a lock.
 * \return string of lock flag names, space-separated.
 */
const char *
lock_flags_long(lock_list *ll)
{
  return privs_to_string(lock_privs, L_FLAGS(ll));
}


static int
string_to_lockflag(dbref player, char const *p)
{
  int f;
  f = string_to_privs(lock_privs, p, 0);
  if (!f)
    return -1;
  if (!See_All(player) && (f & LF_WIZARD))
    return -1;
  return f;
}

/** Initialize the lock strtree. */
void
init_locks(void)
{
  st_init(&lock_names);
}

static int
can_write_lock(dbref player, dbref thing, lock_list *lock)
{
  if (God(player))
    return 1;
  if (God(thing))
    return 0;
  if (Wizard(player))
    return 1;
  if (L_FLAGS(lock) & LF_WIZARD)
    return 0;
  if (L_FLAGS(lock) & LF_OWNER)
    return player == Owner(thing);
  if (L_FLAGS(lock) & LF_LOCKED)
    return Owner(player) == lock->creator;
  return 1;
}


static lock_list *
next_free_lock(void)
{
  lock_list *ll;

  if (!free_list) {
    size_t n;

    ll = mush_malloc(sizeof(lock_list) * LOCKS_PER_PAGE, "lock_page");

    if (!ll)
      panic("Unable to allocate memory for locks!");

    for (n = 0; n < LOCKS_PER_PAGE - 1; n++) {
      ll[n].type = NULL;
      ll[n].key = NULL;
      ll[n].creator = NOTHING;
      ll[n].flags = 0;
      ll[n].next = &ll[n + 1];
    }

    ll[n].next = NULL;
    ll[n].type = NULL;
    ll[n].key = NULL;
    ll[n].creator = NOTHING;
    ll[n].flags = 0;

    free_list = &ll[0];
  }

  ll = free_list;
  free_list = ll->next;

  ll->type = NULL;
  ll->key = NULL;

  return ll;
}

static void
free_lock(lock_list *ll)
{
  ll->type = NULL;
  ll->key = NULL;
  ll->creator = NOTHING;
  ll->flags = 0;
  ll->next = free_list;
  free_list = ll;
}

/** Given a lock type, find a lock, possibly checking parents.
 * \param thing object on which lock is to be found.
 * \param type type of lock to find.
 * \return pointer to boolexp of lock.
 */
struct boolexp *
getlock(dbref thing, lock_type type)
{
  struct lock_list *ll = getlockstruct(thing, type);
  if (!ll)
    return TRUE_BOOLEXP;
  else
    return L_KEY(ll);
}

/** Given a lock type, find a lock without checking parents. 
 * \param thing object on which lock is to be found.
 * \param type type of lock to find.
 * \return pointer to boolexp of lock.
 */
struct boolexp *
getlock_noparent(dbref thing, lock_type type)
{
  struct lock_list *ll = getlockstruct_noparent(thing, type);
  if (!ll)
    return TRUE_BOOLEXP;
  else
    return L_KEY(ll);
}

static lock_list *
getlockstruct(dbref thing, lock_type type)
{
  lock_list *ll;
  dbref p;
  int cmp, count;

  count = 0;
  for (p = thing; GoodObject(p); p = Parent(p)) {
    if (count++ > 100)
      return NULL;
    ll = Locks(p);
    while (ll && L_TYPE(ll)) {
      cmp = strcasecmp(L_TYPE(ll), type);
      if (cmp == 0)
	return (p != thing && (ll->flags & LF_PRIVATE)) ? NULL : ll;
      else if (cmp > 0)
	break;
      ll = ll->next;
    }
  }
  return NULL;
}

static lock_list *
getlockstruct_noparent(dbref thing, lock_type type)
{
  lock_list *ll = Locks(thing);
  int cmp;

  while (ll && L_TYPE(ll)) {
    cmp = strcasecmp(L_TYPE(ll), type);
    if (cmp == 0)
      return ll;
    else if (cmp > 0)
      break;
    ll = ll->next;
  }
  return NULL;
}


/** Determine if a lock type is one of the standard types or not.
 * \param type type of lock to check.
 * \return canonical lock type or NULL
 */
lock_type
match_lock(lock_type type)
{
  int i;
  for (i = 0; lock_types[i].type != NULL; i++) {
    if (strcasecmp(lock_types[i].type, type) == 0) {
      return lock_types[i].type;
    }
  }
  return NULL;
}

/** Return the proper entry from lock_types, or NULL.
 * \param type of lock to look up.
 * \return lock_types entry for lock.
 */
const lock_list *
get_lockproto(lock_type type)
{
  const lock_list *ll;

  for (ll = lock_types; ll->type; ll++)
    if (strcmp(type, ll->type) == 0)
      return ll;

  return NULL;

}

/** Add a lock to an object (primitive).
 * Set the lock type on thing to boolexp.
 * This is a primitive routine, to be called by other routines.
 * It will go somewhat wonky if given a NULL boolexp.
 * It will allocate memory if called with a string that is not already
 * in the lock table.
 * \param player the enactor, for permission checking.
 * \param thing object on which to set the lock.
 * \param type type of lock to set.
 * \param key lock boolexp pointer (should not be NULL!).
 * \param flags lock flags.
 * \retval 0 failure.
 * \retval 1 success.
 */
int
add_lock(dbref player, dbref thing, lock_type type, struct boolexp *key,
	 int flags)
{
  lock_list *ll, **t;
  lock_type real_type = type;

  if (!GoodObject(thing)) {
    return 0;
  }

  ll = getlockstruct_noparent(thing, type);

  if (ll) {
    if (!can_write_lock(player, thing, ll)) {
      free_boolexp(key);
      return 0;
    }
    /* We're replacing an existing lock. */
    free_boolexp(ll->key);
    ll->key = key;
    ll->creator = player;
    if (flags != -1)
      ll->flags = flags;
  } else {
    ll = next_free_lock();
    if (!ll) {
      /* Oh, this sucks */
      do_log(LT_ERR, 0, 0, "Unable to malloc memory for lock_list!");
    } else {
      real_type = st_insert(type, &lock_names);
      ll->type = real_type;
      ll->key = key;
      ll->creator = player;
      if (flags == -1) {
	const lock_list *l2 = get_lockproto(real_type);
	if (l2)
	  ll->flags = l2->flags;
	else
	  ll->flags = 0;
      } else {
	ll->flags = flags;
      }
      if (!can_write_lock(player, thing, ll)) {
	st_delete(real_type, &lock_names);
	free_boolexp(key);
	return 0;
      }
      t = &Locks(thing);
      while (*t && strcmp(L_TYPE(*t), L_TYPE(ll)) < 0)
	t = &L_NEXT(*t);
      L_NEXT(ll) = *t;
      *t = ll;
    }
  }
  return 1;
}

/** Add a lock to an object on db load.
 * Set the lock type on thing to boolexp.
 * Used only on db load, when we can't safely test the player's
 * permissions because they're not loaded yet.
 * This is a primitive routine, to be called by other routines.
 * It will go somewhat wonky if given a NULL boolexp.
 * It will allocate memory if called with a string that is not already
 * in the lock table.
 * \param player lock creator.
 * \param thing object on which to set the lock.
 * \param type type of lock to set.
 * \param key lock boolexp pointer (should not be NULL!).
 * \param flags lock flags.
 * \retval 0 failure.
 */
int
add_lock_raw(dbref player, dbref thing, lock_type type, struct boolexp *key,
	     int flags)
{
  lock_list *ll, **t;
  lock_type real_type = type;

  if (!GoodObject(thing)) {
    return 0;
  }

  ll = next_free_lock();
  if (!ll) {
    /* Oh, this sucks */
    do_log(LT_ERR, 0, 0, "Unable to malloc memory for lock_list!");
  } else {
    real_type = st_insert(type, &lock_names);
    ll->type = real_type;
    ll->key = key;
    ll->creator = player;
    if (flags == -1) {
      const lock_list *l2 = get_lockproto(real_type);
      if (l2)
	ll->flags = l2->flags;
      else
	ll->flags = 0;
    } else {
      ll->flags = flags;
    }
    t = &Locks(thing);
    while (*t && strcmp(L_TYPE(*t), L_TYPE(ll)) < 0)
      t = &L_NEXT(*t);
    L_NEXT(ll) = *t;
    *t = ll;
  }
  return 1;
}

/* Very primitive. */
static void
free_one_lock_list(lock_list *ll)
{
  if (ll == NULL)
    return;
  free_boolexp(ll->key);
  st_delete(ll->type, &lock_names);
  free_lock(ll);
}

/** Delete a lock from an object (primitive).
 * Another primitive routine.
 * \param player the enactor, for permission checking.
 * \param thing object on which to remove the lock.
 * \param type type of lock to remove.
 */
int
delete_lock(dbref player, dbref thing, lock_type type)
{
  lock_list *ll, **llp;
  if (!GoodObject(thing)) {
    return 0;
  }
  llp = &(Locks(thing));
  while (*llp && strcasecmp((*llp)->type, type) != 0) {
    llp = &((*llp)->next);
  }
  if (*llp != NULL) {
    if (can_write_lock(player, thing, *llp)) {
      ll = *llp;
      *llp = ll->next;
      free_one_lock_list(ll);
      return 1;
    } else
      return 0;
  } else
    return 1;
}

/** Free all locks in a list.
 * Used by the object destruction routines.
 * \param ll pointer to list of locks.
 */
void
free_locks(lock_list *ll)
{
  lock_list *ll2;
  while (ll) {
    ll2 = ll->next;
    free_one_lock_list(ll);
    ll = ll2;
  }
}


/** Check to see that the lock type is a valid type.
 * If it's not in our lock table, it's not valid,
 * unless it begins with 'user:' or an abbreviation thereof,
 * in which case the lock type is the part after the :.
 * As an extra check, we don't allow '|' in lock names because it
 * will confuse our db-reading routines.
 *
 * Might destructively modify name.
 *
 * \param player the enactor, for notification.
 * \param thing object on which to check the lock.
 * \param name name of lock type.
 * \return lock type, or NULL.
 */
static lock_type
check_lock_type(dbref player, dbref thing, lock_type name)
{
  lock_type ll;
  char *sp;
  /* Special-case for basic locks. */
  if (!name || !*name)
    return Basic_Lock;

  /* Normal locks. */
  ll = match_lock(name);
  if (ll != NULL)
    return ll;

  /* If the lock is set, it's allowed, whether it exists normally or not. */
  if (getlock(thing, name) != TRUE_BOOLEXP)
    return name;
  /* Check to see if it's a well-formed user-defined lock. */
  sp = strchr(name, ':');
  if (!sp) {
    notify(player, T("Unknown lock type."));
    return NULL;
  }
  *sp++ = '\0';

  if (!string_prefix("User", name)) {
    notify(player, T("Unknown lock type."));
    return NULL;
  }
  if (strchr(sp, '|')) {
    notify(player, T("The character \'|\' may not be used in lock names."));
    return NULL;
  }
  if (!good_atr_name(sp)) {
    notify(player, T("That is not a valid lock name."));
    return NULL;
  }

  return sp;
}

/** Unlock a lock (user interface).
 * \verbatim
 * This implements @unlock.
 * \endverbatim
 * \param player the enactor.
 * \param name name of object to unlock.
 * \param type type of lock to unlock.
 */
void
do_unlock(dbref player, const char *name, lock_type type)
{
  dbref thing;
  char *sp;
  lock_type real_type;

  /* check for '@unlock <object>/<atr>'  */
  sp = strchr(name, '/');
  if (sp) {
    do_atrlock(player, name, "off");
    return;
  }
  if ((thing = match_controlled(player, name)) != NOTHING) {
    if ((real_type = check_lock_type(player, thing, type)) != NULL) {
      if (getlock(thing, real_type) == TRUE_BOOLEXP)
	notify_format(player, T("%s(%s) - %s (already) unlocked."), Name(thing),
		      unparse_dbref(thing), real_type);
      else if (delete_lock(player, thing, real_type)) {
	notify_format(player, T("%s(%s) - %s unlocked."), Name(thing),
		      unparse_dbref(thing), real_type);
	ModTime(thing) = mudtime;
      } else
	notify(player, T("Permission denied."));
    }
  }
}

/** Set/lock a lock (user interface).
 * \verbatim
 * This implements @lock.
 * \endverbatim
 * \param player the enactor.
 * \param name name of object to lock.
 * \param keyname key to lock the lock to, as a string.
 * \param type type of lock to lock.
 */
void
do_lock(dbref player, const char *name, const char *keyname, lock_type type)
{
  lock_type real_type;
  dbref thing;
  struct boolexp *key;
  char *sp;

  /* check for '@lock <object>/<atr>'  */
  sp = strchr(name, '/');
  if (sp) {
    do_atrlock(player, name, "on");
    return;
  }
  if (!keyname || !*keyname) {
    do_unlock(player, name, type);
    return;
  }
  switch (thing = match_result(player, name, NOTYPE, MAT_EVERYTHING)) {
  case NOTHING:
    notify(player, T("I don't see what you want to lock!"));
    return;
  case AMBIGUOUS:
    notify(player, T("I don't know which one you want to lock!"));
    return;
  default:
    if (!controls(player, thing)) {
      notify(player, T("You can't lock that!"));
      return;
    }
    if (IsGarbage(thing)) {
      notify(player, T("Why would you want to lock garbage?"));
      return;
    }
    break;
  }

  key = parse_boolexp(player, keyname, type);

  /* do the lock */
  if (key == TRUE_BOOLEXP) {
    notify(player, T("I don't understand that key."));
  } else {
    if ((real_type = check_lock_type(player, thing, type)) != NULL) {
      /* everything ok, do it */
      if (add_lock(player, thing, real_type, key, -1)) {
	notify_format(player, T("%s(%s) - %s locked."), Name(thing),
		      unparse_dbref(thing), real_type);
	ModTime(thing) = mudtime;
      } else
	notify(player, T("Permission denied."));
    }
  }
}

/** Copy the locks from one object to another.
 * \param player the enactor.
 * \param orig the source object.
 * \param clone the destination object.
 */
void
clone_locks(dbref player, dbref orig, dbref clone)
{
  lock_list *ll;
  for (ll = Locks(orig); ll; ll = ll->next) {
    if (!(L_FLAGS(ll) & LF_NOCLONE))
      add_lock(player, clone, L_TYPE(ll), dup_bool(L_KEY(ll)), L_FLAGS(ll));
  }
}


/** Evaluate a lock.
 * Evaluate lock ltype on thing for player.
 * \param player dbref attempting to pass the lock.
 * \param thing object containing the lock.
 * \param ltype type of lock to check.
 * \retval 1 player passes the lock.
 * \retval 0 player does not pass the lock.
 */
int
eval_lock(dbref player, dbref thing, lock_type ltype)
{
  return eval_boolexp(player, getlock(thing, ltype), thing);
}

/** Determine if a lock is visual.
 * \param thing object containing the lock.
 * \param ltype type of lock to check.
 * \retval (non-zero) lock is visual.
 * \retval 0 lock is not visual.
 */
int
lock_visual(dbref thing, lock_type ltype)
{
  lock_list *l = getlockstruct(thing, ltype);
  if (l)
    return l->flags & LF_VISUAL;
  else
    return 0;
}

/** Set flags on a lock (user interface).
 * \verbatim
 * This implements @lset.
 * \endverbatim
 * \param player the enactor.
 * \param what string in the form obj/lock.
 * \param flags list of flags to set.
 */
void
do_lset(dbref player, char *what, char *flags)
{
  dbref thing;
  lock_list *l;
  char *lname;
  int flag;
  int unset = 0;

  if ((lname = strchr(what, '/')) == NULL) {
    notify(player, T("No lock name given."));
    return;
  }
  *lname++ = '\0';

  if ((thing = match_controlled(player, what)) == NOTHING)
    return;

  if (*flags == '!') {
    unset = 1;
    flags++;
  }

  if ((flag = string_to_lockflag(player, flags)) < 0) {
    notify(player, T("Unrecognized lock flag."));
    return;
  }

  l = getlockstruct_noparent(thing, lname);
  if (!l || !Can_Read_Lock(player, thing, L_TYPE(l))) {
    notify(player, T("No such lock."));
    return;
  }

  if (!can_write_lock(player, thing, l)) {
    notify(player, T("Permission denied."));
    return;
  }

  if (unset)
    L_FLAGS(l) &= ~flag;
  else
    L_FLAGS(l) |= flag;

  notify_format(player, "%s/%s - %s.", Name(thing), L_TYPE(l),
		unset ? T("lock flags unset") : T("lock flags set"));
  ModTime(thing) = mudtime;
}
