/* lock.c */

/*
 * This is the core of Ralph Melton's rewrite of the @lock system.
 * These are some of my underlying assumptions:
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
 */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif

#include "conf.h"
#include "mushdb.h"
#include "attrib.h"
#include "externs.h"
#include "lock.h"
#include "match.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "mymalloc.h"
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
const lock_type Parent_Lock = "Parent";
const lock_type Link_Lock = "Link";
const lock_type Leave_Lock = "Leave";
const lock_type Drop_Lock = "Drop";
const lock_type Give_Lock = "Give";
const lock_type Mail_Lock = "Mail";

/* Define new lock types here. */
/* OFFLINE #ifdef TREKMUSH
const lock_type DBRead_Lock= "DBRead";
const lock_type DBWrite_Lock = "DBWrite";
#endif */

const char *lock_types[] =
{
  "Basic",
  "Enter",
  "Use",
  "Zone",
  "Page",
  "Teleport",
#ifdef SPEECH_LOCK
  "Speech",
#endif
#ifdef LISTEN_LOCK
  "Listen",
#endif
  "Parent",
  "Link",
#ifdef LEAVE_LOCK
  "Leave",
#endif
#ifdef DROP_LOCK
  "Drop",
#endif
#ifdef GIVE_LOCK
  "Give",
#endif
  "Mail",
/* OFFLINE #ifdef TREKMUSH
  "DBRead",
  "DBWrite",
#endif */
  /* Add new lock types just before this line. */
  NULL
};


struct boolexp *getlock _((dbref thing, lock_type type));
lock_type match_lock _((lock_type type));
void add_lock _((dbref thing, lock_type type, struct boolexp * key));
static void free_one_lock_list _((lock_list *ll));
void delete_lock _((dbref thing, lock_type type));
void free_locks _((lock_list *ll));
static lock_type check_lock_type _((dbref player, dbref thing, char *name));
void do_unlock _((dbref player, const char *name, lock_type type));
void do_lock _((dbref player, const char *name, const char *keyname, lock_type type));
int eval_lock _((dbref player, dbref thing, lock_type ltype));

/* Given a lock type, find a lock.
 * This depends on the implementation of lock_type as a string.
 */
struct boolexp *
getlock(thing, type)
    dbref thing;
    lock_type type;
{
  struct lock_list *ll;
  if (!GoodObject(thing))
    return TRUE_BOOLEXP;
  ll = Locks(thing);
  while (ll && ll->type) {
    if (strcasecmp(ll->type, type) == 0) {
      return (ll->key);
    }
    ll = ll->next;
  }
  return TRUE_BOOLEXP;
}


/* Is this lock in our set of canonical locks? */
lock_type
match_lock(type)
    lock_type type;
{
  int i;
  for (i = 0; lock_types[i] != NULL; i++) {
    if (strcasecmp(lock_types[i], type) == 0) {
      return lock_types[i];
    }
  }
  return NULL;
}

/* Set the lock type on thing to boolexp.
 * This is a primitive routine, to be called by other routines.
 * It will go somewhat wonky if given a NULL boolexp.
 * It will allocate memory if called with a string that is not already
 * in the lock table.
 */
void
add_lock(thing, type, key)
    dbref thing;
    lock_type type;
    struct boolexp *key;
{
  struct lock_list *ll;
  lock_type real_type = type;

  if (!GoodObject(thing)) {
    return;
  }
  ll = Locks(thing);
  while (ll && (strcasecmp(ll->type, type) != 0)) {
    ll = ll->next;
  }
  if (ll) {
    /* We're replacing an existing lock. */
    free_boolexp(ll->key);
    ll->key = key;
  } else {
    ll = (lock_list *) mush_malloc(sizeof(lock_list), "lock_list");
    if (!ll) {
      /* Oh, this sucks */
      do_log(LT_ERR, 0, 0, "Unable to malloc memory for lock_list!");
    } else {
      real_type = match_lock(type);
      if (real_type == NULL) {
	real_type = strdup(type);
#ifdef MEM_CHECK
	add_check("lock_type");
#endif
      }
      ll->type = real_type;
      ll->key = key;
      ll->next = Locks(thing);
      Locks(thing) = ll;
    }
  }
}


/* Very primitive. */
static void
free_one_lock_list(ll)
    lock_list *ll;
{
  int i;
  if (ll == NULL)
    return;
  free_boolexp(ll->key);
  /* A quandary: How do we tell whether to free the type string?
   * If it's in our string table, we want not to free it, but otherwise,
   * we do.
   * Answer: since this is called infrequently, we can just check again
   * to see whether the string is in the table of locks.
   */
  for (i = 0; lock_types[i] != NULL; i++) {
    /* n.b. We use ==, not strcasecmp. If it's a duplicate, we want
     * to free it.
     */
    if (ll->type == lock_types[i]) {
      break;
    }
  }
  if (lock_types[i] == NULL) {	/* We didn't find it in the table. */
    mush_free((Malloc_t) ll->type, "lock_type");
  }
  mush_free((Malloc_t) ll, "lock_list");
}

/* Another primitive routine. */
void
delete_lock(thing, type)
    dbref thing;
    lock_type type;
{
  struct lock_list *ll, **llp;
  if (!GoodObject(thing)) {
    return;
  }
  llp = &(Locks(thing));
  while (*llp && strcasecmp((*llp)->type, type) != 0) {
    llp = &((*llp)->next);
  }
  if (*llp != NULL) {
    ll = *llp;
    *llp = ll->next;
    free_one_lock_list(ll);
  }
}


void
free_locks(ll)
    lock_list *ll;
{
  lock_list *ll2;
  while (ll) {
    ll2 = ll->next;
    free_one_lock_list(ll);
    ll = ll2;
  }
}


/* Check to see that the lock type is a valid type.
 * If it's not in our lock table, it's not valid,
 * unless it begins with 'user:' or an abbreviation thereof,
 * in which case the lock type is the part after the :.
 * As an extra check, we don't allow '|' in lock names because it
 * will confuse our db-reading routines.
 *
 * Might destructively modify name.
 */
static lock_type
check_lock_type(player, thing, name)
    dbref player;
    dbref thing;
    char *name;
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
    notify(player, "Unknown lock type.");
    return NULL;
  }
  *sp++ = '\0';

  if (!string_prefix("User", name)) {
    notify(player, "Unknown lock type.");
    return NULL;
  }
  if (index(sp, '|')) {
    notify(player, "The character \'|\' may not be used in lock names.");
    return NULL;
  }
  return sp;

}

void
do_unlock(player, name, type)
    dbref player;
    const char *name;
    lock_type type;
{
  dbref thing;
  char *sp;
  lock_type real_type;

  /* check for '@unlock <object>/<atr>'  */
  sp = (char *) index(name, '/');
  if (sp) {
    do_atrlock(player, name, "off");
    return;
  }
  if ((thing = match_controlled(player, name)) != NOTHING) {
    if ((real_type = check_lock_type(player, thing, (char *) type)) != NULL) {
      delete_lock(thing, real_type);
      notify(player, "Unlocked.");
    }
  }
}

void
do_lock(player, name, keyname, type)
    dbref player;
    const char *name;
    const char *keyname;
    lock_type type;
{
  lock_type real_type;
  dbref thing;
  struct boolexp *key;
  char *sp;

  /* check for '@lock <object>/<atr>'  */
  sp = (char *) index(name, '/');
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
    notify(player, "I don't see what you want to lock!");
    return;
  case AMBIGUOUS:
    notify(player, "I don't know which one you want to lock!");
    return;
  default:
    if (!controls(player, thing)) {
      notify(player, "You can't lock that!");
      return;
    }
    if (IsGarbage(thing)) {
      notify(player, "Why would you want to lock garbage?");
      return;
    }
    break;
  }

  key = parse_boolexp(player, keyname);

  /* do the lock */
  if (key == TRUE_BOOLEXP) {
    notify(player, "I don't understand that key.");
  } else {
    if ((real_type = check_lock_type(player, thing, (char *) type)) != NULL) {
      /* everything ok, do it */
      add_lock(thing, real_type, key);
      notify(player, "Locked.");
    }
  }
}

/* Evaluate lock ltype on thing for player */
int
eval_lock(player, thing, ltype)
    dbref player;
    dbref thing;
    lock_type ltype;
{
  return eval_boolexp(player, getlock(thing, ltype), thing, 0, ltype);
}
