/* destroy.c */
#include "config.h"

#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif

#include "copyrite.h"
#include "conf.h"
#include "mushdb.h"
#include "match.h"
#include "externs.h"
#include "globals.h"
#include "confmagic.h"

/* OFFLINE #ifdef TREKMUSH
#include "offline.h"
#endif */

/* These lengthy comments are by Ralph Melton, December 1995. */
/*
 * First, a discourse on the theory of how we handle destruction.
 *
 * We want to maintain the following invariants:
 * 1. All destroyed objects are on the free list. (linked through the next
 *    fields.)
 * 2. All objects on the free list are destroyed objects.
 * 3. No undestroyed object has its next, contents, location, or home
 *    field pointing to a destroyed object.
 * 4. No object's zone or parent is a destroyed object.
 * 5. No object's owner is a destroyed object.
 * 
 * For the sake of efficiency, we allow indirect locks and other locks to
 * refer to destroyed objects; boolexp.c had better be able to cope with
 * these.
 *
 *
 * There are three logically distinct parts to destroying an object:
 *
 * Part 1: we do all the permissions checks, check for the SAFE flag
 * and the override switch, and decide that yes, we are going to destroy
 * this object.
 *
 * Part 2: we eliminate all the links from other objects in the database
 * to this object. This processing may depend on the object's type.
 *
 * Part 3: (logically concurrent with part 2, and must happen together
 * with part 2) Remove any commands the object may have in the queue,
 * free all the storage associated with the object, set the name to
 * 'Garbage', and set this object to be a destroyed object, and put it
 * on the free list. This process is independent of object type.
 *
 * Note that phases 2 and 3 do not have to happen immediately after Phase 1.
 * To allow some delay, we set the object GOING, and then process it on
 * the check that happens every ten minutes. 
 */

/*
 * This file has two main parts. One part is the functions for destroying
 * objects and getting objects off of the free list. The major public
 * functions here are do_destroy(), free_get(), and purge().
 *
 * The other part is functions for checking the consistency of the
 * database, and repairing any inconsistencies that are found. The
 * major function in this group is dbck().
 */

dbref first_free = NOTHING;

void do_destroy _((dbref player, char *name, int confirm));
void do_undestroy _((dbref player, char *name));
dbref free_get _((void));
void fix_free_list _((void));
void purge _((void));
void do_purge _((dbref player));

void dbck _((void));
void do_dbck _((dbref player));

static dbref what_to_destroy _((dbref player, char *name, int confirm));

static void pre_destroy _((dbref player, dbref thing));
void undestroy _((dbref player, dbref thing));
static void free_object _((dbref thing));
static void empty_contents _((dbref thing));
static void clear_thing _((dbref thing));
static void clear_player _((dbref thing));
static void clear_room _((dbref thing));
static void clear_exit _((dbref thing));

static void check_fields _((void));
static void check_connected_rooms _((void));
static void mark_connected _((dbref loc));
static void check_connected_marks _((void));
static void mark_contents _((dbref loc));
static void check_contents _((void));
static void check_locations _((void));
static int attribute_owner_helper _((dbref player, dbref thing, char const *pattern, ATTR *atr, void *args));

extern void remove_all_obj_chan _((dbref thing));
extern void chan_chownall _((dbref old, dbref new));

/* Section I: do_destroy() and related functions. This section is where
 * the human interface of do_destroy() should largely be determined.
 */

/* Methinks it's time to consider human interfaces criteria for destruction
 * as well as to consider the invariants that need to be maintained.
 *
 * My major criteria are these (with no implied ranking, since I haven't
 * decided how they balance out):
 * 
 * 1) It's easy to destroy things you intend to destroy.
 * 
 * 2) It's easy to correct from destroying things that you don't intend
 * to destroy. This includes both typos and realizing that you didn't mean
 * to destroy that. This principle requires two sub-principles:
 *      a) The player gets notified when something 'important' gets
 *         marked to be destroyed--and gets told .what. is marked.
 *      b) The actual destruction of the important thing is delayed
 *         long enough that you can recover from it.
 *
 * 3) You can't destroy something you don't have the proper privileges to
 * destroy. (Obvious, but still worth writing down.)
 *
 * To try to achieve a reasonable balance between items 1) and 2), we
 * have the following design:
 * Everything is finally destroyed on the second purge after the @destroy
 * command is done, unless it is set !GOING in the meantime.
 * @destroying an object while it is set GOING destroys it immediately.
 *
 * Let me introduce a little jargon for this discussion:
 *      pre-destroying an object == setting it GOING, running the @adestroy.
 *              (Pre-destroying corresponds to phase 1 above.)
 *      purging an object == actually irrevocably making it vanish.
 *              (This corresponds to phases 2 and 3 above.)
 *      undestroying an object == setting it !GOING, etc.
 *
 * We would also like to have an @adestroy attribute that contains
 * code to be executed when the object is destroyed. This is
 * complicated by the fact that the object is going to be
 * destroyed. To work around this, we run the @adestroy when the
 * object is pre-destroyed, not when it's actually purged. This
 * introduces the possibility that the adestroy may be invoked for
 * something that is then undestroyed. To compensate for that, we run
 * the @startup attribute when something is undestroyed.
 *
 * Another issue is how to run the @adestroy for objects that are
 * destroyed as a consequence of other objects being destroyed. For
 * example, when rooms are destroyed, any exits leading from those
 * rooms are also destroyed, and when a player is destroyed, !SAFE
 * objects they own may also be destroyed.
 * 
 * To handle this, we do the following:
 * pre-destroying a room pre-destroys all its exits.
 * pre-destroying a player pre-destroys all the objects that will be purged
 * when that player is purged.
 *
 * This requires the following about undestroys:
 * undestroying an exit undestroys its source room.
 * undestroying any object requires undestroying its owner.
 *
 * But it also seems to require the following in order to make '@destroy
 * foo; @undestroy foo' a no-op for all foo:
 * undestroying a room undestroys all its exits.
 * undestroying a player undestroys all its GOING things.
 * 
 * Now, consider this scenario:
 * Player A owns room #1. Player B owns exit #2, whose source is room #1.
 * Player B owns thing #3. Player A and player B are both pre-destroyed;
 * none of the objects are set SAFE. Thing #3 is then undestroyed.
 * 
 * If you trace through the dependencies, you find that this involves
 * undestroying all the objects, including both players! Is that what
 * we want? It seems to me that it would be very surprising in practice.
 * 
 * To reconcile this, we introduce the following compromise.
 * undestroying a room undestroys all exits in the room that are not owned
 *      by a GOING player or set SAFE..
 * undestroying a player undestroys all objects he owns that are not exits
 *      in a GOING room that he does not own.
 * 
 * In this way, the propagation of previous scenario would die out at exit
 * #2, which would stay GOING. Metaphorically, there are two 'votes' for
 * its destruction: the destruction of room #1, and the destruction of
 * player B. Undestroying player B by undestroying thing #3 removes one
 * of the 'votes' for exit #2's destruction, but there would still be
 * the vote from room #1.  
 */


static dbref
what_to_destroy(player, name, confirm)
    dbref player;
    char *name;
    int confirm;

/* Do all matching and permissions checking. Returns the object to be
 * destroyed if all the permissions checks are successful, otherwise
 * return NOTHING.
 */
{
  dbref thing;

  thing = noisy_match_result(player, name, NOTYPE, MAT_EVERYTHING);
  if (thing == NOTHING)
    return NOTHING;

  if (IsGarbage(thing)) {
    notify(player, "Destroying that again is hardly necessary.");
    return NOTHING;
  }
  if (God(thing)) {
    notify(player, "Destroying God would be blasphemous.");
    return NOTHING;
  }
  /* To destroy, you must either:
   * 1. Own it (and control it)
   * 2. Be at least a Wizard
   * 3. Control its source or destination if it's an exit
   * 4. Be dealing with a dest-ok thing
   */
  if (!(Owns(player, thing) && controls(player, thing)) &&
      !Wizard(player) &&
      !(IsExit(thing) &&
	(controls(player, Destination(thing)) ||
	 controls(player, Source(thing)))) &&
      !DestOk(thing)) {
    notify(player, "Permission denied.");
    return NOTHING;
  }
  if (thing == PLAYER_START || thing == MASTER_ROOM || God(thing)) {
    notify(player, "That is too special to be destroyed.");
    return NOTHING;
  }
  if (REALLY_SAFE) {
    if (Safe(thing) && !DestOk(thing)) {
      notify(player, "That object is set SAFE. You must set it !SAFE before destroying it.");
      return NOTHING;
    }
  } else {			/* REALLY_SAFE */
    if (Safe(thing) && !DestOk(thing) && !confirm) {
      notify(player, "That object is marked SAFE. Use @nuke to destroy it.");
      return NOTHING;
    }
  }
  /* check to make sure there's no accidental destruction */
  if (!confirm && !Owns(player, thing) && !DestOk(thing)) {
    notify(player,
	 "That object does not belong to you. Use @nuke to destroy it.");
    return NOTHING;
  }
  /* what kind of thing we are destroying? */
  switch (Typeof(thing)) {
  case TYPE_PLAYER:
    if (!IsPlayer(player)) {
      notify(player, "Programs don't kill people; people kill people!");
      return NOTHING;
    }
    /* The only player a player can own() is themselves...
     * If they somehow manage to own() another player, they can't
     * nuke that one either...which seems like a good plan, although
     * the error message is a bit confusing. -DTC
     */
    if (!Wizard(player)) {
      notify(player, "Sorry, no suicide allowed.");
      return NOTHING;
    }
    /* Already checked for God(thing), so use Wizard() */
    if (Wizard(thing) && !God(player)) {
      notify(player, "Even you can't do that!");
      return NOTHING;
    }
    if (Connected(thing)) {
      notify(player, "How gruesome. You may not destroy players who are connected.");
      return NOTHING;
    }
    if (!confirm) {
      notify(player, "You must use @nuke to destroy a player.");
      return NOTHING;
    }
    break;
  case TYPE_THING:
    if (!confirm && Wizard(thing)) {
      notify(player,
	 "That object is set WIZARD. You must use @nuke to destroy it.");
      return NOTHING;
    }
    if (thing == Home(player) && !IsExit(player)) {
      notify(player, "No home-wrecking allowed! Relink first.");
      return NOTHING;
    }
    break;
  case TYPE_ROOM:
    if (thing == Home(player) && !IsExit(player)) {
      notify(player, "No home-wrecking allowed! Relink first.");
      return NOTHING;
    }
    break;
  case TYPE_EXIT:
    break;
  }
  return thing;

}


void
do_destroy(player, name, confirm)
    dbref player;
    char *name;
    int confirm;
{
  dbref thing;
  thing = what_to_destroy(player, name, confirm);
  if (!GoodObject(thing))
    return;

  /* If thing has already been marked for destruction, go ahead and
   * destroy immediately.
   */
  if (Going(thing)) {
    free_object(thing);
    notify(player, "Destroyed.");
    return;
  }
  /* Present informative messages. */
  switch (Typeof(thing)) {
  case TYPE_ROOM:
    /* wait until dbck */
    notify_except(Contents(thing), NOTHING,
		  "The room shakes and begins to crumble.");
    if (Owns(player, thing))
      notify(player,
	     tprintf("You will be rewarded shortly for %s.",
		     object_header(player, thing)));
    else {
      notify(player, tprintf("The wrecking ball is on its way for %s's %s and its exits.",
		      Name(Owner(thing)), object_header(player, thing)));
      notify(Owner(thing),
	     tprintf("%s has scheduled your room %s to be destroyed.",
		     Name(player), object_header(Owner(thing), thing)));
    }
    break;
  case TYPE_PLAYER:
    /* wait until dbck */
    notify(player, tprintf("%s and all their objects are scheduled to be destroyed.",
			   object_header(player, thing)));
    break;
  case TYPE_THING:
    if (!Owns(player, thing)) {
      notify(player, tprintf("%s's %s is scheduled to be destroyed.",
			     Name(Owner(thing)),
			     object_header(player, thing)));
      if (!DestOk(thing))
	notify(Owner(thing), tprintf("%s has scheduled your %s for destruction.",
				     Name(player),
				     object_header(Owner(thing), thing)));
    } else {
      notify(player, tprintf("%s is scheduled to be destroyed.",
			     object_header(player, thing)));
    }
    break;
  case TYPE_EXIT:
    if (!Owns(player, thing)) {
      notify(Owner(thing), tprintf("%s has scheduled your %s for destruction.",
				   Name(player),
				   object_header(Owner(thing), thing)));
      notify(player, tprintf("%s's %s is scheduled to be destroyed.",
			     Name(Owner(thing)),
			     object_header(player, thing)));
    } else
      notify(player, tprintf("%s is scheduled to be destroyed.",
			     object_header(player, thing)));
    break;
  default:
    do_log(LT_ERR, NOTHING, NOTHING, "Surprising type in do_destroy.");
    return;
  }

  pre_destroy(player, thing);
  return;
}


/* Not undestroy, quite--it's actually 'remove it from its status as about
 * to be destroyed.'
 */
void
do_undestroy(player, name)
    dbref player;
    char *name;
{
  dbref thing;
  thing = noisy_match_result(player, name, NOTYPE, MAT_EVERYTHING);
  if (!GoodObject(thing)) {
    return;
  }
  if (!controls(player, thing)) {
    notify(player, "Alas, your efforts of mercy are in vain.");
    return;
  }
  undestroy(player, thing);
  notify(Owner(thing),
	 tprintf("Your %s has been spared from destruction.",
		 object_header(Owner(thing), thing)));
  if (player != Owner(thing)) {
    notify(player,
	   tprintf("%s has been spared from destruction.",
		   object_header(player, thing)));
  }
}



/* Section II: Functions that manage the actual work of destroying
 * Objects.
 */

/* Schedule something to be destroyed, run @adestroy, etc. */
static void
pre_destroy(player, thing)
    dbref player;
    dbref thing;
{
  dbref tmp;
  if (Going(thing) || IsGarbage(thing)) {
    /* we've already covered this thing. No need to do so again. */
    return;
  }
  Flags(thing) |= GOING;
  Flags(thing) &= ~GOING_TWICE;

  /* Present informative messages, and do recursive destruction. */
  switch (Typeof(thing)) {
  case TYPE_ROOM:
    DOLIST(tmp, Exits(thing)) {
      pre_destroy(player, tmp);
    }
    break;
  case TYPE_PLAYER:
    if (DESTROY_POSSESSIONS) {
      for (tmp = 0; tmp < db_top; tmp++) {
	if (Owner(tmp) == thing &&
	    (tmp != thing) && (!REALLY_SAFE || !Safe(thing))
	  ) {
	  pre_destroy(player, tmp);
	}
      }
    }
    break;
  case TYPE_THING:
    break;
  case TYPE_EXIT:
    /* This is the only case in which we might end up destroying something
     * whose owner hasn't already been notified. */
    if ((Owner(thing) != Owner(Source(thing))) &&
	Going(Source(thing))) {
      if (!Owns(player, thing)) {
	notify(Owner(thing), tprintf("%s has scheduled your %s for destruction.",
				     Name(player),
				     object_header(Owner(thing), thing)));
      }
    }
    break;
  default:
    do_log(LT_ERR, NOTHING, NOTHING, "Surprising type in pre_destroy.");
    return;
  }

  if (ADESTROY_ATTR)
    did_it(player, thing, NULL, NULL, NULL, NULL, "ADESTROY", NOTHING);

  return;

}


/* Not undestroy, quite--it's actually 'remove it from its status as about
 * to be destroyed.'
 */
void
undestroy(player, thing)
    dbref player;
    dbref thing;
{
  dbref tmp;
  if (!Going(thing)) {
    return;
  }
  Flags(thing) &= ~GOING;
  Flags(thing) &= ~GOING_TWICE;
  if (Startup(thing) && !Halted(thing)) {
    ATTR *s;
    char *r;
    s = atr_get_noparent(thing, "STARTUP");
    if (s) {
      r = safe_uncompress(s->value);
      parse_que(thing, r, thing);
      free((Malloc_t) r);
    }
  }
  /* undestroy owner, if need be. */
  if (Going(Owner(thing))) {
    if (Owner(thing) != player) {
      notify(player,
	     tprintf("%s has been spared from destruction.",
		     object_header(player, Owner(thing)))
	);
      notify(Owner(thing),
	     tprintf("You have been spared from destruction by %s.",
		     Name(player))
	);
    } else {
      notify(player, "You have been spared from destruction.");
    }
    undestroy(player, Owner(thing));
  }
  switch (Typeof(thing)) {
  case TYPE_PLAYER:
    if (DESTROY_POSSESSIONS)
      /* Undestroy all objects owned by players, except exits that are in
       * rooms owned by other players that are set GOING, since those will
       * be purged when the room is purged.
       */
      for (tmp = 0; tmp < db_top; tmp++) {
	if (Owns(thing, tmp) &&
	    (tmp != thing) &&
	    !(IsExit(tmp) &&
	      !Owns(thing, Source(tmp)) &&
	      Going(Source(tmp)))) {
	  undestroy(player, tmp);
	}
      }
    break;
  case TYPE_THING:
    break;
  case TYPE_EXIT:
    /* undestroy containing room. */
    if (Going(Source(thing))) {
      undestroy(player, Source(thing));
      notify(player,
	     tprintf("The room %s has been spared from destruction.",
		     object_header(player, Source(thing))));
      if (Owner(Source(thing)) != player) {
	notify(Owner(Source(thing)),
	   tprintf("The room %s has been spared from destruction by %s.",
		   object_header(Owner(Source(thing)), Source(thing)),
		   Name(player))
	  );
      }
    }
    break;
  case TYPE_ROOM:
    /* undestroy exits in this room, except exits that are going to be
       * destroyed anyway due to a GOING player.
     */
    DOLIST(tmp, Exits(thing)) {
      if (DESTROY_POSSESSIONS ? (!Going(Owner(tmp)) || Safe(tmp)) : 1) {
	undestroy(player, tmp);
      }
    }
    break;
  default:
    do_log(LT_ERR, NOTHING, NOTHING, "Surprising type in un_destroy.");
    return;
  }
}


static void
free_object(thing)
    dbref thing;
/* Does the real work of freeing all the memory and unlinking an object. */
/* This is going to have to be very tightly coupled with the implementation;
 * if the database format changes, this will likely have to change too.
 */
{
  dbref i;
  if (!GoodObject(thing))
    return;
#ifdef LOCAL_DATA
  local_data_free(thing);
#endif
  switch (Typeof(thing)) {
  case TYPE_THING:
    clear_thing(thing);
    break;
  case TYPE_PLAYER:
    clear_player(thing);
    break;
  case TYPE_EXIT:
    clear_exit(thing);
    break;
  case TYPE_ROOM:
    clear_room(thing);
    break;
  default:
    do_log(LT_ERR, NOTHING, NOTHING, "Unknown type on #%d in free_object.", thing);
    return;
  }
#ifdef QUOTA
  change_quota(Owner(thing), QUOTA_COST);
#endif				/* QUOTA */
  do_halt(thing, "", thing);
  nfy_que(thing, 2, 0);		/* The equivalent of an @drain. */

  /* if something is zoned or parented or linked or chained or located
   * to/in destroyed object, undo */
  for (i = 0; i < db_top; i++) {
    if (Zone(i) == thing) {
      Zone(i) = NOTHING;
    }
    if (Parent(i) == thing) {
      Parent(i) = NOTHING;
    }
    if (Home(i) == thing) {
      switch (Typeof(i)) {
      case TYPE_PLAYER:
      case TYPE_THING:
	Home(i) = PLAYER_START;
	break;
      case TYPE_EXIT:
	/* Huh.  An exit that claims to be from here, but wasn't linked
	 * in properly. */
	do_rawlog(LT_ERR, "ERROR: Exit %s leading from invalid room #%d destroyed.",
		  real_unparse(GOD, i, 0), thing);
	free_object(i);
	break;
      case TYPE_ROOM:
	/* Hrm.  It claims we're an exit from it, but we didn't agree.
	 * Clean it up anyway. */
	do_log(LT_ERR, NOTHING, NOTHING,
	       "Found a destroyed exit #%d in room #%d", thing, i);
	break;
      }
    }
    /* The location check MUST be done AFTER the home check. */
    if (Location(i) == thing) {
      switch (Typeof(i)) {
      case TYPE_PLAYER:
      case TYPE_THING:
	/* Huh.  It thought it was here, but we didn't agree. */
	enter_room(i, Home(i));
	break;
      case TYPE_EXIT:
	/* If our destination is destroyed, then we relink to the
	 * source room (so that the exit can't be stolen). Yes, it's
	 * inconsistent with the treatment of exits leading from
	 * destroyed rooms, but it's a lot better than turning exits
	 * into nasty limbo exits.
	 */
	Destination(i) = Source(i);
	break;
      case TYPE_ROOM:
	/* Just remove a dropto. */
	Location(i) = NOTHING;
	break;
      }
    }
    if (Next(i) == thing) {
      Next(i) = NOTHING;
    }
  }

  /* chomp chomp */
  atr_free(thing);
  List(thing) = NULL;
  /* don't eat name otherwise examine will crash */

  free_locks(Locks(thing));
  Locks(thing) = NULL;

/* OFFLINE #ifdef TREKMUSH
  offline_object_wipe(thing);
  GDBMQuota(thing)=0;
  GDBMUsage(thing)=0;
#endif */

  s_Pennies(thing, 0);
  Owner(thing) = GOD;
  Parent(thing) = NOTHING;
  Zone(thing) = NOTHING;
  Flags(thing) = TYPE_GARBAGE;
#ifdef CHAT_SYSTEM
  remove_all_obj_chan(thing);
#endif
  Location(thing) = NOTHING;
  SET(Name(thing), "Garbage");
  Exits(thing) = NOTHING;
  Home(thing) = NOTHING;

  Next(thing) = first_free;
  first_free = thing;

#ifdef HAS_ASSERT
  assert(IsGarbage(thing));
#endif
}

static void
empty_contents(thing)
    dbref thing;
{
  /* Destroy any exits they may be carrying, send everything else home. */
  dbref first;
  dbref rest;
  notify_except(Contents(thing), NOTHING,
		"The floor disappears under your feet, you fall through NOTHINGness and then:");
  first = Contents(thing);
  Contents(thing) = NOTHING;
  /* send all objects to nowhere */
  DOLIST(rest, first) {
    Location(rest) = NOTHING;
  }
  /* now send them home */
  while (first != NOTHING) {
    rest = Next(first);
    /* if home is in thing set it to limbo */
    switch (Typeof(first)) {
    case TYPE_EXIT:		/* if holding exits, destroy it */
      free_object(first);
      break;
    case TYPE_THING:		/* move to home */
    case TYPE_PLAYER:
      if (Home(first) == thing || IsExit(Home(first)))
	Home(first) = PLAYER_START;
      if (Home(first) != NOTHING) {
	PUSH(first, Contents(Home(first)));
	Location(first) = Home(first);
	/* notify players they have been moved */
	if (Hearer(first)) {
	  enter_room(first, HOME);
	}
      }
      break;
    }
    first = rest;
  }
}

static void
clear_thing(thing)
    dbref thing;
{
  dbref loc;
  int a;
  /* Remove object from room's contents */
  loc = Location(thing);
  if (loc != NOTHING) {
    Contents(loc) = remove_first(Contents(loc), thing);
  }
  /* give player money back */
  giveto(Owner(thing), (a = OBJECT_DEPOSIT(Pennies(thing))));
  empty_contents(thing);
  if (Toggles(thing) & THING_PUPPET)
    Toggles(thing) &= ~THING_PUPPET;
  if (!Quiet(thing) && !Quiet(Owner(thing)))
    notify(Owner(thing),
	   tprintf("You get your %d %s deposit back for %s.",
		   a, ((a == 1) ? MONEY : MONIES),
		   object_header(Owner(thing), thing)));
}

static void
clear_player(thing)
    dbref thing;
{
  dbref i;
  ATTR *atemp;
  char alias[PLAYER_NAME_LIMIT];

  /* Clear out mail. */
#ifdef USE_MAILER
  do_mail_clear(thing, NULL);
  do_mail_purge(thing);
#endif

  /* Chown any chat channels they own to God */
#ifdef CHAT_SYSTEM
  chan_chownall(thing, GOD);
#endif

  /* Clear out names from the player list */
  delete_player(thing, NULL);
  if ((atemp = atr_get_noparent(thing, "ALIAS")) != NULL) {
    strcpy(alias, uncompress(atemp->value));
    delete_player(thing, alias);
  }
  /* Do all the thing-esque manipulations. */
  clear_thing(thing);

  /* Deal with objects owned by the player. */
  for (i = 0; i < db_top; i++) {
    if (Owner(i) == thing && i != thing) {
      if (DESTROY_POSSESSIONS ? (REALLY_SAFE ? Safe(i) : 0) : 1) {
	chown_object(GOD, i, GOD);
      } else {
	free_object(i);
      }
    }
  }
}

static void
clear_room(thing)
    dbref thing;
{
  dbref first, rest;
  /* give player money back */
  giveto(Owner(thing), ROOM_COST);
  empty_contents(thing);
  /* Remove exits */
  first = Exits(thing);
  Source(thing) = NOTHING;
  /* set destination of all exits to nothing */
  DOLIST(rest, first) {
    Destination(rest) = NOTHING;
  }
  /* Clear all exits out of exit list */
  while (first != NOTHING) {
    rest = Next(first);
    if (IsExit(first)) {
      free_object(first);
    }
    first = rest;
  }
}


static void
clear_exit(thing)
    dbref thing;
{
  dbref loc;
  loc = Source(thing);
  if (GoodObject(loc)) {
    Exits(loc) = remove_first(Exits(loc), thing);
  };
  giveto(Owner(thing), EXIT_COST);
}


/* return a cleaned up object off the free list or NOTHING */
dbref
free_get()
{
  dbref newobj;
  if (first_free == NOTHING)
    return (NOTHING);
  newobj = first_free;
  first_free = Next(first_free);
  /* Make sure this object really should be in free list */
  if (!IsGarbage(newobj)) {
    static int nrecur = 0;
    dbref temp;
    if (nrecur++ == 20) {
      first_free = NOTHING;
      report();
      do_rawlog(LT_ERR, "ERROR: Removed free list and continued\n");
      return (NOTHING);
    }
    report();
    do_rawlog(LT_TRACE, "ERROR: Object #%d should not be free\n", newobj);
    do_rawlog(LT_TRACE, "ERROR: Corrupt free list, fixing\n");
    fix_free_list();
    temp = free_get();
    nrecur--;
    return (temp);
  }
  /* free object name */
  SET(Name(newobj), NULL);
  return (newobj);
}

/* Build the free list with a sledgehammer. Only do this when it's actually
 * necessary.
 * Since we only do it if things are corrupted, we do not free any memory.
 * Presumably, this will only waste a reasonable amount of memory, since
 * it's only called in exceptional cases.
 */
void
fix_free_list()
{
  dbref thing;
  first_free = NOTHING;
  for (thing = 0; thing < db_top; thing++) {
    if (IsGarbage(thing)) {
      Next(thing) = first_free;
      first_free = thing;
    }
  }
}



/* Destroy all the objects we said we would destroy later. */
void
purge()
{
  dbref thing;
  for (thing = 0; thing < db_top; thing++) {
    if (IsGarbage(thing)) {
      continue;
    } else if (Going(thing)) {
      if (Going_Twice(thing)) {
	free_object(thing);
      } else {
	Flags(thing) |= GOING_TWICE;
      }
    } else {
      continue;
    }
  }
}


/* Destroy objects slated for destruction. */
void
do_purge(player)
    dbref player;
{
  if (Wizard(player)) {
    purge();
    notify(player, "Purge complete.");
  } else
    notify(player, "Sorry, you are a mortal.");
}



/* Section III: dbck() and related functions. */

/* The regular db checkup. */
void
dbck()
{
  check_fields();
  check_contents();
  check_locations();
  check_connected_rooms();
}

/* Do sanity checks on non-destroyed objects. */
static void
check_fields()
{
  dbref thing;
  for (thing = 0; thing < db_top; thing++) {
    if (IsGarbage(thing)) {
      /* The only relevant thing is that the Next field ought to be pointing
       * to a destroyed object.
       */
      dbref next;
      next = Next(thing);
      if ((!GoodObject(next) || !IsGarbage(next)) && (next != NOTHING)) {
	do_rawlog(LT_ERR, "ERROR: Invalid next pointer #%d from object %s",
		  next, real_unparse(GOD, thing, 0));
	Next(thing) = NOTHING;
	fix_free_list();
      }
      continue;
    } else {
      /* Do sanity checks on non-destroyed objects */
      dbref zone, loc, parent, home, owner, next;
      zone = Zone(thing);
      if (GoodObject(zone) && IsGarbage(zone))
	Zone(thing) = NOTHING;
      parent = Parent(thing);
      if (GoodObject(parent) && IsGarbage(parent))
	Parent(thing) = NOTHING;
      owner = Owner(thing);
      if (!GoodObject(owner) || IsGarbage(owner) ||
	  !IsPlayer(owner)) {
	do_rawlog(LT_ERR, "ERROR: Invalid object owner on %s(%d)", Name(thing),
		  thing);
	report();
	Owner(thing) = GOD;
      }
      next = Next(thing);
      if ((!GoodObject(next) || IsGarbage(next)) && (next != NOTHING)) {
	do_rawlog(LT_ERR, "ERROR: Invalid next pointer #%d from object %s",
		  next, real_unparse(GOD, thing, 0));
	Next(thing) = NOTHING;
      }
      /* This next bit has to be type-specific because of different uses
       * of the home and location fields.
       */
      home = Home(thing);
      loc = Location(thing);
      switch (Typeof(thing)) {
      case TYPE_PLAYER:
      case TYPE_THING:
	if (!GoodObject(home) || IsGarbage(home) || IsExit(home))
	  Home(thing) = PLAYER_START;
	if (!GoodObject(loc) || IsGarbage(loc) || IsExit(loc))
	  enter_room(thing, Home(thing));
	break;
      case TYPE_EXIT:
	if (Contents(thing) != NOTHING) {
	  /* Eww.. Exits can't have contents. Bad news */
	  Contents(thing) = NOTHING;
	  do_rawlog(LT_ERR, "ERROR: Exit %s has a contents list. Wiping it out.", real_unparse(GOD, thing, 0));
	}
	if (GoodObject(loc) && IsGarbage(loc)) {
	  /* If our destination is destroyed, then we relink to the
	   * source room (so that the exit can't be stolen). Yes, it's
	   * inconsistent with the treatment of exits leading from
	   * destroyed rooms, but it's a lot better than turning exits
	   * into nasty limbo exits.
	   */
	  Destination(thing) = Source(thing);
	  do_rawlog(LT_ERR, "ERROR: Exit %s leading to invalid room #%d relinked to its source room.",
		    real_unparse(GOD, thing, 0), home);
	}
	/* This must come last */
	if (!GoodObject(home) || !IsRoom(home)) {
	  /* If our source is destroyed, just destroy the exit. */
	  do_rawlog(LT_ERR, "ERROR: Exit %s leading from invalid room #%d destroyed.",
		    real_unparse(GOD, thing, 0), home);
	  free_object(thing);
	}
	break;
      case TYPE_ROOM:
	if (GoodObject(home) && IsGarbage(home)) {
	  /* Eww. Destroyed exit. This isn't supposed to happen. */
	  do_log(LT_ERR, NOTHING, NOTHING,
		 "Found a destroyed exit #%d in room #%d", home, thing);
	}
	if (GoodObject(loc) && (IsGarbage(loc) || IsExit(loc))) {
	  /* Just remove a dropto. */
	  Location(thing) = NOTHING;
	}
	break;
      }
      /* Check attribute ownership. If the attribute is owned by
       * an invalid dbref, change its ownership to God.
       */
      if (!IsGarbage(thing))
	atr_iter_get(GOD, thing, NULL, attribute_owner_helper, NULL);
    }
  }
}

static int
attribute_owner_helper(player, thing, pattern, atr, args)
    dbref player;
    dbref thing;
    char const *pattern;
    ATTR *atr;
    void *args;
{
  if (!GoodObject(AL_CREATOR(atr)))
    AL_CREATOR(atr) = GOD;
  return 0;
}

static void
check_connected_rooms()
{
  mark_connected(BASE_ROOM);
  check_connected_marks();
}

static void
mark_connected(loc)
    dbref loc;
{
  dbref thing;
  if (!GoodObject(loc) || Marked(loc) || !IsRoom(loc))
    return;
  Flags(loc) |= MARKED;
  /* recursively trace */
  for (thing = Exits(loc); thing != NOTHING; thing = Next(thing))
    mark_connected(Destination(thing));
}

static void
check_connected_marks()
{
  dbref loc;
  for (loc = 0; loc < db_top; loc++)
    if (Marked(loc))
      Flags(loc) &= ~MARKED;
    else if (IsRoom(loc)) {
      if (!Floating(loc) &&
	  (!EXITS_CONNECT_ROOMS || (Exits(loc) == NOTHING))) {
	if (!Name(loc)) {
	  do_log(LT_ERR, NOTHING, NOTHING, "ERROR: no name for room #%d.", loc);
	  SET(Name(loc), "XXXX");
	}
	notify(Owner(loc), tprintf("You own a disconnected room, %s",
				   object_header(Owner(loc), loc)));
      }
    }
}


/* An auxiliary function for check_contents. */
static void
mark_contents(thing)
    dbref thing;
{
  /* In this next macro, field must be an lvalue whose evaluation has
   * no side effects.  All hell will break loose if this is not so.
   */
#define CHECK(field)            \
  if ((field) != NOTHING) { \
     if (!GoodObject(field) || IsGarbage(field)) { \
       do_rawlog(LT_ERR, "Bad reference #%d from %s severed.", \
                 (field), real_unparse(GOD, thing, 0)); \
       (field) = NOTHING; \
     } else if (IsRoom(field)) { \
       do_rawlog(LT_ERR, "Reference to room #%d from %s severed.", \
                 (field), real_unparse(GOD, thing, 0)); \
       (field) = NOTHING; \
     } else if (Marked(field)) {  \
       do_rawlog(LT_ERR, "Multiple references to %s. Reference from #%d severed.", \
                 real_unparse(GOD, (field), 0), thing); \
       (field) = NOTHING; \
     } else { \
       Flags(field) |= MARKED; \
       mark_contents(field); \
     } \
  }

  if (!GoodObject(thing))
    return;

  Flags(thing) |= MARKED;
  switch (Typeof(thing)) {
  case TYPE_ROOM:
    CHECK(Exits(thing));
    CHECK(Contents(thing));
    break;
  case TYPE_PLAYER:
  case TYPE_THING:
    CHECK(Contents(thing));
    CHECK(Next(thing));
    break;
  case TYPE_EXIT:
    CHECK(Next(thing));
    break;
  default:
    do_rawlog(LT_ERR, "Bad object type found for %s in mark_contents",
	      real_unparse(GOD, thing, 0));
    break;
  }
#undef CHECK
}

/* Check that for every thing, player, and exit, you can trace exactly one
 * path to that object from a room by following the exits field of rooms,
 * the next field of non-rooms, and the contents field of non-exits.
 */
static void
check_contents()
{
  dbref thing;
  for (thing = 0; thing < db_top; thing++) {
    if (IsRoom(thing)) {
      mark_contents(thing);
    }
  }
  for (thing = 0; thing < db_top; thing++) {
    if (!Marked(thing) && !IsRoom(thing) && !IsGarbage(thing)) {
      do_rawlog(LT_ERR, "Object %s not pointed to by anything.",
		real_unparse(GOD, thing, 0));
      notify(Owner(thing), tprintf("You own an object %s that was \'orphaned\'.",
				   object_header(Owner(thing), thing)));
      /* We try to fix this by trying to send players and things to
       * their current location, to their home, or to PLAYER_START, in
       * that order, and relinking exits to their source.
       */
      Next(thing) = NOTHING;
      switch (Typeof(thing)) {
      case TYPE_PLAYER:
      case TYPE_THING:
	if (GoodObject(Location(thing)) &&
	    !IsGarbage(Location(thing)) &&
	    Marked(Location(thing))) {
	  PUSH(thing, Contents(Location(thing)));
	} else if (GoodObject(Home(thing)) &&
		   !IsGarbage(Home(thing)) &&
		   Marked(Home(thing))) {
	  Contents(Location(thing)) = remove_first(Contents(Location(thing)), thing);
	  PUSH(thing, Contents(Home(thing)));
	  Location(thing) = Home(thing);
	} else {
	  Contents(Location(thing)) = remove_first(Contents(Location(thing)), thing);
	  PUSH(thing, Contents(PLAYER_START));
	  Location(thing) = PLAYER_START;
	}
	enter_room(thing, Location(thing));
	/* If we've managed to reconnect it, then we've reconnected
	 * its contents. */
	mark_contents(Contents(thing));
	notify(Owner(thing), tprintf("It was moved to %s.",
			  object_header(Owner(thing), Location(thing))));
	do_rawlog(LT_ERR, "Moved to %s.", real_unparse(GOD, Location(thing), 0));
	break;
      case TYPE_EXIT:
	if (GoodObject(Source(thing)) &&
	    IsRoom(Source(thing))) {
	  PUSH(thing, Exits(Source(thing)));
	  notify(Owner(thing), tprintf("It was moved to %s.",
			    object_header(Owner(thing), Source(thing))));
	  do_rawlog(LT_ERR, "Moved to %s.", real_unparse(GOD, Source(thing), 0));
	} else {
	  /* Just destroy the exit. */
	  Source(thing) = NOTHING;
	  notify(Owner(thing), "It was destroyed.");
	  do_rawlog(LT_ERR, "Orphaned exit destroyed.");
	  free_object(thing);
	}
	break;
      case TYPE_ROOM:
	/* We should never get here. */
	do_log(LT_ERR, NOTHING, NOTHING, "Disconnected room. So what?");
	break;
      default:
	do_log(LT_ERR, NOTHING, NOTHING,
	       "Surprising type on #%d found in check_cycles.", thing);
	break;
      }
    }
  }
  for (thing = 0; thing < db_top; thing++) {
    Flags(thing) &= ~MARKED;
  }
}


/* Check that every player and thing occurs in the contents list of its
 * location, and that every exit occurs in the exit list of its source.
 */
static void
check_locations()
{
  dbref thing;
  dbref loc;
  for (loc = 0; loc < db_top; loc++) {
    if (!IsExit(loc)) {
      for (thing = Contents(loc); thing != NOTHING; thing = Next(thing)) {
	if (!Mobile(thing)) {
	  do_rawlog(LT_ERR,
	     "ERROR: Contents of object %d corrupt at object %d cleared",
		    loc, thing);
	  /* Remove this from the list and start over. */
	  Contents(loc) = remove_first(Contents(loc), thing);
	  thing = Contents(loc);
	  continue;
	} else if (Location(thing) != loc) {
	  /* Well, it would fit here, and it can't be elsewhere because
	   * we've done a check_contents already, so let's just put it
	   * here.
	   */
	  do_rawlog(LT_ERR, "Incorrect location on object %s. Reset to #%d.",
		    real_unparse(GOD, thing, 0), loc);
	  Location(thing) = loc;
	}
	Flags(thing) |= MARKED;
      }
    }
    if (IsRoom(loc)) {
      for (thing = Exits(loc); thing != NOTHING; thing = Next(thing)) {
	if (!IsExit(thing)) {
	  do_rawlog(LT_ERR,
		  "ERROR: Exits of room %d corrupt at object %d cleared",
		    loc, thing);
	  /* Remove this from the list and start over. */
	  Exits(loc) = remove_first(Exits(loc), thing);
	  thing = Exits(loc);
	  continue;
	} else if (Source(thing) != loc) {
	  do_rawlog(LT_ERR,
		    "Incorrect source on exit %s. Reset to #%d.",
		    real_unparse(GOD, thing, 0), loc);
	}
      }
    }
  }


  for (thing = 0; thing < db_top; thing++)
    if (Marked(thing))
      Flags(thing) &= ~MARKED;
    else if (Mobile(thing)) {
      do_rawlog(LT_ERR, "ERROR DBCK: Moved object %d", thing);
      moveto(thing, PLAYER_START);
    }
}


/* Do database checkup. */
/* N.B. Unlike before, this is only a user command. The real work is
 * farmed out to the dbck() routine. When a dbck is done automatically,
 * call dbck(), not do_dbck()!
 */
void
do_dbck(player)
    dbref player;
{
  if (!Wizard(player)) {
    notify(player, "Silly mortal chicks are for kids!");
    return;
  }
  notify(player, "GAME: Performing database consistency check.");
  do_log(LT_WIZ, player, NOTHING, "DBCK done.");
  dbck();
  notify(player, "GAME: Database consistency check complete.");
}
