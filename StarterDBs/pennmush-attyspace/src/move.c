
/* move.c */

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
#include "intrface.h"
#include "match.h"
#include "externs.h"
#include "globals.h"
#include "parse.h"
#include "command.h"
#include "confmagic.h"

extern void do_destroy _((dbref player, char *name, int confirm));

void moveto _((dbref what, dbref where));
void moveit _((dbref what, dbref where));
static void send_contents _((dbref loc, dbref dest));
static void maybe_dropto _((dbref loc, dbref dropto));
void enter_room _((dbref player, dbref loc));
void safe_tel _((dbref player, dbref dest));
int can_move _((dbref player, const char *direction));
static dbref find_var_dest _((dbref player, dbref exit_obj));
void do_move _((dbref player, const char *direction, int type));
void do_firstexit _((dbref player, const char *what));
void do_get _((dbref player, const char *what));
void do_drop _((dbref player, const char *name));
void do_enter _((dbref player, const char *what, int is_alias));
void do_leave _((dbref player));
dbref global_exit _((dbref player, const char *direction));
dbref remote_exit _((dbref player, const char *direction));
void move_wrapper _((dbref player, const char *command));


void
moveto(what, where)
    dbref what;
    dbref where;
{
  enter_room(what, where);
}

void
moveit(what, where)
    dbref what;
    dbref where;
{
  dbref loc, old;

  /* remove what from old loc */
  if ((loc = old = Location(what)) != NOTHING) {
    Contents(loc) = remove_first(Contents(loc), what);
  }
  /* test for special cases */
  switch (where) {
  case NOTHING:
    Location(what) = NOTHING;
    return;			/* NOTHING doesn't have contents */
  case HOME:
    where = Home(what);	/* home */
    safe_tel(what, where);
    return;
    /*NOTREACHED */
    break;
  }

  /* now put what in where */
  PUSH(what, Contents(where));

  Location(what) = where;
  if (!WIZ_NOAENTER || !(Wizard(what) && DarkLegal(what)))
    if ((where != NOTHING) && (old != where)) {
      did_it(what, what, NULL, NULL, "OXMOVE", NULL, NULL, old);
      if (Hearer(what)) {
	did_it(what, old, "LEAVE", NULL, "OLEAVE", "has left.",
	       "ALEAVE", old);
	if (!IsRoom(where))
	  did_it(what, where, NULL, NULL, "OXENTER", NULL, NULL, old);
	if (!IsRoom(old))
	  did_it(what, old, NULL, NULL, "OXLEAVE", NULL, NULL, where);
	did_it(what, where, "ENTER", NULL, "OENTER", "has arrived.",
	       "AENTER", where);
      } else {
	/* non-listeners only trigger the actions not the messages */
	did_it(what, old, NULL, NULL, NULL, NULL, "ALEAVE", old);
	did_it(what, where, NULL, NULL, NULL, NULL, "AENTER", where);
      }
    }
  did_it(what, what, "MOVE", NULL, "OMOVE", NULL, "AMOVE", where);
}

#define Dropper(thing) (Hearer(thing) && Connected(Owner(thing)))

static void
send_contents(loc, dest)
    dbref loc;
    dbref dest;
{
  dbref first;
  dbref rest;
  first = Contents(loc);
  Contents(loc) = NOTHING;

  /* blast locations of everything in list */
  DOLIST(rest, first) {
    Location(rest) = NOTHING;
  }

  while (first != NOTHING) {
    rest = Next(first);
    if (Dropper(first)) {
      Location(first) = loc;
      PUSH(first, Contents(loc));
    } else
      enter_room(first, Sticky(first) ? HOME : dest);
    first = rest;
  }

  Contents(loc) = reverse(Contents(loc));
}

static void
maybe_dropto(loc, dropto)
    dbref loc;
    dbref dropto;
{
  dbref thing;
  if (loc == dropto)
    return;			/* bizarre special case */
  if (!IsRoom(loc))
    return;
  /* check for players */
  DOLIST(thing, Contents(loc)) {
    if (Dropper(thing))
      return;
  }

  /* no players, send everything to the dropto */
  send_contents(loc, dropto);
}

void
enter_room(player, loc)
    dbref player;
    dbref loc;
{
  dbref old;
  dbref dropto;
  static int deep = 0;
  if (deep++ > 15) {
    deep--;
    return;
  }
  if (!GoodObject(player)) {
    deep--;
    return;
  }
  /* check for room == HOME */
  if (loc == HOME)
    loc = Home(player);

  if (!Mobile(player)) {
    fprintf(stderr, "ERROR: Non object moved!! %d\n", player);
    fflush(stderr);
    deep--;
    return;
  }
  if (IsExit(loc)) {
    fprintf(stderr, "ERROR: Attempt to move %d to exit %d\n", player, loc);
    deep--;
    return;
  }
  if (loc == player) {
    fprintf(stderr, "ERROR: Attempt to move player %d into itself\n", player);
    deep--;
    return;
  }
  /* get old location */
  old = Location(player);

  /* go there */
  moveit(player, loc);

  /* if old location has STICKY dropto, send stuff through it */

  if ((loc != old) && Dropper(player) &&
      (old != NOTHING) && (IsRoom(old)) &&
      ((dropto = Location(old)) != NOTHING) && Sticky(old))
    maybe_dropto(old, dropto);


  /* autolook */
  look_room(player, loc, 2);
  deep--;
}


/* teleports player to location while removing items they shouldnt take */
void
safe_tel(player, dest)
    dbref player;
    dbref dest;
{
  dbref first;
  dbref rest;
  if (dest == HOME)
    dest = Home(player);
  if (Owner(Location(player)) == Owner(dest)) {
    enter_room(player, dest);
    return;
  }
  first = Contents(player);
  Contents(player) = NOTHING;

  /* blast locations of everything in list */
  DOLIST(rest, first) {
    Location(rest) = NOTHING;
  }

  while (first != NOTHING) {
    rest = Next(first);
    /* if thing is ok to take then move to player else send home.
     * thing is not okay to move if it's STICKY and its home is not
     * the player.
     */
    if (!controls(player, first) && (Sticky(first) && (Home(first) != player)))
      enter_room(first, HOME);
    else {
      PUSH(first, Contents(player));
      Location(first) = player;
    }
    first = rest;
  }
  Contents(player) = reverse(Contents(player));
  enter_room(player, dest);
}

int
can_move(player, direction)
    dbref player;
    const char *direction;
{
  if (!strcasecmp(direction, "home") && !Fixed(player))
    return 1;

  if (!command_check_byname(player, "GOTO"))
    return 0;

  /* otherwise match on exits */
  return (last_match_result(player, direction, TYPE_EXIT, MAT_EXIT | MAT_ABSOLUTE) != NOTHING);
}

static dbref
find_var_dest(player, exit_obj)
    dbref player;
    dbref exit_obj;
{
  /* This is used to evaluate the u-function DESTINATION on an exit with
   * a VARIABLE (ambiguous) link.
   */

  char const *abuf, *ap;
  char buff[BUFFER_LEN], *bp;
  ATTR *a;
  dbref dest_room;

  a = atr_get(exit_obj, "DESTINATION");
  if (!a)
    return NOTHING;

  abuf = safe_uncompress(a->value);
  if (!abuf)
    return NOTHING;
  if (!*abuf) {
    free((Malloc_t) abuf);
    return NOTHING;
  }
  ap = abuf;
  bp = buff;
  process_expression(buff, &bp, &ap, exit_obj, player, player,
		     PE_DEFAULT, PT_DEFAULT, NULL);
  *bp = '\0';
  dest_room = parse_dbref(buff);

  free((Malloc_t) abuf);
  return (dest_room);
}


void
do_move(player, direction, type)
    dbref player;
    const char *direction;
    int type;


				/* type 0 is normal, type 1 is global */
{
  dbref exit_m, loc, var_dest;

#ifdef TREKMUSH
  if (IS(player, TYPE_PLAYER, PLAYER_GAGGED)) {
    notify(player, "You are not conscious!");
    return;
  }
#endif /* TREKMUSH */

  if (!strcasecmp(direction, "home")) {
    /* send him home */
    /* but steal all his possessions */
    if (!Mobile(player) || !GoodObject(Home(player)) ||
	recursive_member(Home(player), player, 0) || (player == Home(player))) {
      notify(player, "Bad destination.");
      return;
    }
    if ((loc = Location(player)) != NOTHING && !Dark(player) && !Dark(loc)) {
      /* tell everybody else */
      notify_except(Contents(loc), player,
		    tprintf("%s goes home.", Name(player)));
    }
    /* give the player the messages */
    notify(player, "There's no place like home...");
    notify(player, "There's no place like home...");
    notify(player, "There's no place like home...");
    safe_tel(player, HOME);
  } else {
    /* find the exit */
    if (DO_GLOBALS && (type == 1))
      exit_m = match_result(player, direction, TYPE_EXIT, MAT_EXIT | MAT_GLOBAL | MAT_CHECK_KEYS | MAT_ABSOLUTE);
    else if (DO_GLOBALS && (type == 2))
      exit_m = match_result(player, direction, TYPE_EXIT, MAT_EXIT | MAT_REMOTES | MAT_CHECK_KEYS | MAT_ABSOLUTE);
    else
      exit_m = match_result(player, direction, TYPE_EXIT, MAT_EXIT | MAT_CHECK_KEYS | MAT_ABSOLUTE);
    if (!Hasprivs(player) && GoodObject(exit_m) && is_dbref(direction) && (Source(exit_m) != Location(player)))
      exit_m = NOTHING;
    switch (exit_m) {
    case NOTHING:
      /* try to force the object */
      notify(player, "You can't go that way.");
      break;
    case AMBIGUOUS:
      notify(player, "I don't know which way you mean!");
      break;
    default:
      /* we got one */
      /* check to see if we got through */
      if (could_doit(player, exit_m)) {

	switch (Location(exit_m)) {
	case HOME:
	  var_dest = Home(player);
	  break;
	case AMBIGUOUS:
	  var_dest = find_var_dest(player, exit_m);
	  break;
	default:
	  var_dest = Location(exit_m);
	}

	did_it(player, exit_m, "SUCCESS", NULL, "OSUCCESS", NULL,
	       "ASUCCESS", NOTHING);
	did_it(player, exit_m, "DROP", NULL, "ODROP", NULL, "ADROP",
	       var_dest);

/* This comes from Talek's patch. Looks to me like it interferes with
 * normal operation of unlinked exits, though?   -- Amby.
 */
/* It doesn't, because unlinked exits were caught in could_doit(),
 * 25 lines back.  Reinstating to prevent core dumps.  - Talek
 */

	if (!GoodObject(var_dest)) {
	  fprintf(stderr,
		  "Exit #%d destination became %d during move.\n",
		  exit_m, var_dest);
	  notify(player, "Exit destination is invalid.");
	  return;
	}
	switch (Typeof(var_dest)) {

	case TYPE_ROOM:
	  enter_room(player, var_dest);
	  break;

	case TYPE_PLAYER:
	case TYPE_THING:
	  if (IsGarbage(var_dest)) {
	    notify(player, "You can't go that way.");
	    return;
	  }
	  if (Location(var_dest) == NOTHING)
	    return;
	  safe_tel(player, var_dest);
	  break;
	case TYPE_EXIT:
	  notify(player, "This feature coming soon.");
	  break;
	}
      } else
	did_it(player, exit_m, "FAILURE", "You can't go that way.",
	       "OFAILURE", NULL, "AFAILURE", NOTHING);
      break;
    }
  }
}

void
do_firstexit(player, what)
    dbref player;
    const char *what;
{
  dbref thing;
  dbref loc;

  if ((thing = noisy_match_result(player, what, TYPE_EXIT, MAT_EXIT)) == NOTHING)
    return;

  loc = Home(thing);
  if (!controls(player, loc)) {
    notify(player, "You cannot modify exits in that room.");
    return;
  }
  Exits(loc) = remove_first(Exits(loc), thing);
  Source(thing) = loc;
  PUSH(thing, Exits(loc));
  notify(player, tprintf("%s is now the first exit.", Name(thing)));
}


void
do_get(player, what)
    dbref player;
    const char *what;
{
  dbref loc = Location(player);
  dbref thing;
  char tbuf1[BUFFER_LEN];
  long match_flags = MAT_NEIGHBOR | MAT_ABSOLUTE | MAT_CHECK_KEYS;

  if (!IsRoom(loc) && !EnterOk(loc) &&
      !controls(player, loc)) {
    notify(player, "Permission denied.");
    return;
  }
  if (!Long_Fingers(player))
    match_flags |= MAT_CONTROL;

  if (match_result(player, what, TYPE_THING, match_flags) == NOTHING) {
    if (POSSESSIVE_GET) {
      /* take care of possessive get (stealing) */
      thing = parse_match_possessive(player, what);
      if (!GoodObject(thing)) {
	notify(player, "I don't see that here.");
	return;
      }
      /* to steal something, you have to be able to get it, and the
       * object must be ENTER_OK and not locked against you.
       */
      if (could_doit(player, thing) &&
	  (POSSGET_ON_DISCONNECTED ||
	   (!IsPlayer(Location(thing)) ||
	    Connected(Location(thing)))) &&
	  (controls(player, thing) ||
	   (EnterOk(Location(thing)) &&
	    eval_lock(player, Location(thing), Enter_Lock)
	   ))) {
	notify(Location(thing), tprintf("%s was taken from you.", Name(thing)));
	moveto(thing, player);
	notify(thing, "Taken.");
	sprintf(tbuf1, "takes %s.", Name(thing));
	did_it(player, thing, "SUCCESS", "Taken.", "OSUCCESS", tbuf1, "ASUCCESS",
	       NOTHING);
      } else
	did_it(player, thing, "FAILURE", "You can't take that from there.",
	       "OFAILURE", NULL, "AFAILURE", NOTHING);
    } else {
      notify(player, "I don't see that here.");
    }
    return;
  } else {
    if ((thing = noisy_match_result(player, what, TYPE_THING, match_flags)) != NOTHING) {
      if (Location(thing) == player) {
	notify(player, "You already have that!");
	return;
      }
      switch (Typeof(thing)) {
      case TYPE_PLAYER:
      case TYPE_THING:
	if (thing == player) {
	  notify(player, "You cannot get yourself!");
	  return;
	}
	if (could_doit(player, thing)) {
	  moveto(thing, player);
	  notify(thing, "Taken.");
	  sprintf(tbuf1, "takes %s.", Name(thing));
	  did_it(player, thing, "SUCCESS", "Taken.", "OSUCCESS", tbuf1,
		 "ASUCCESS", NOTHING);
	} else
	  did_it(player, thing, "FAILURE", "You can't pick that up.",
		 "OFAILURE", NULL, "AFAILURE", NOTHING);
	break;
      case TYPE_EXIT:
	notify(player, "You can't pick up exits.");
	return;
      default:
	notify(player, "You can't take that!");
	break;
      }
    }
  }
}


void
do_drop(player, name)
    dbref player;
    const char *name;
{
  dbref loc;
  dbref thing;
  char tbuf1[BUFFER_LEN];

  if ((loc = Location(player)) == NOTHING)
    return;

  switch (thing = match_result(player, name, TYPE_THING, MAT_POSSESSION | MAT_ABSOLUTE | MAT_CONTROL)) {
  case NOTHING:
    notify(player, "You don't have that!");
    return;
  case AMBIGUOUS:
    notify(player, "I don't know which you mean!");
    return;
  default:
    if (Location(thing) != player) {
      /* Shouldn't ever happen. */
      notify(player, "You can't drop that.");
      return;
    } else if (IsExit(thing)) {
      notify(player, "Sorry you can't drop exits.");
      return;
#ifdef DROP_LOCK
    } else if (!eval_lock(player, thing, Drop_Lock)) {
      notify(player, "You can't seem to get rid of that.");
      return;
    } else if (IsRoom(loc) &&
	       !eval_lock(player, loc, Drop_Lock)) {
      notify(player, "You can't seem to drop things here.");
      return;
#endif
    } else if (Sticky(thing) && !Fixed(thing)) {
      notify(thing, "Dropped.");
      safe_tel(thing, HOME);
    } else if ((Location(loc) != NOTHING) && IsRoom(loc) &&
	       !Sticky(loc)) {
      /* location has immediate dropto */
      notify(thing, "Dropped.");
      moveto(thing, Location(loc));
    } else {
      notify(thing, "Dropped.");
      moveto(thing, loc);
    }
    break;
  }
  sprintf(tbuf1, "drops %s.", Name(thing));
  did_it(player, thing, "DROP", "Dropped.", "ODROP", tbuf1, "ADROP", NOTHING);
}


void
do_enter(player, what, is_alias)
    dbref player;
    const char *what;
    int is_alias;


				/* 1 if we got here via enter alias */
{
  dbref thing;
  long match_flags = MAT_CHECK_KEYS | MAT_NEIGHBOR | MAT_EXIT;
  if (is_alias || Hasprivs(player))
    match_flags |= MAT_ABSOLUTE;	/* necessary for enter aliases to work */

  if ((thing = noisy_match_result(player, what, TYPE_THING, match_flags)) == NOTHING) {
    /* notify(player,"I don't see that here.");   */
    return;
  }
  switch (Typeof(thing)) {
  case TYPE_ROOM:
  case TYPE_EXIT:
    notify(player, "Permission denied.");
    break;
  default:
    /* the object must pass the lock. Also, the thing being entered */
    /* has to be controlled, or must be enter_ok */
    if (!(((Flags(thing) & ENTER_OK) || controls(player, thing)) &&
	  (eval_lock(player, thing, Enter_Lock))
	)) {
      did_it(player, thing, "EFAIL", "Permission denied.", "OEFAIL",
	     NULL, "AEFAIL", NOTHING);
      return;
    }
    if (thing == player) {
      notify(player, "Sorry, you must remain beside yourself!");
      return;
    }
    safe_tel(player, thing);
    break;
  }
}

void
do_leave(player)
    dbref player;
{
  if (IsRoom(Location(player)) || NoLeave(Location(player))
#ifdef LEAVE_LOCK
      || !eval_lock(player, Location(player), Leave_Lock)
#endif
    ) {
    did_it(player, Location(player), "LFAIL", "You can't leave.", "OLFAIL",
	   NULL, "ALFAIL", NOTHING);
    return;
  }
  enter_room(player, Location(Location(player)));
}

dbref
global_exit(player, direction)
    dbref player;
    const char *direction;
{
  return (last_match_result(player, direction, TYPE_EXIT, MAT_GLOBAL | MAT_EXIT) != NOTHING);
}

dbref
remote_exit(player, direction)
    dbref player;
    const char *direction;
{
  return (last_match_result(player, direction, TYPE_EXIT, MAT_REMOTES | MAT_EXIT) != NOTHING);
}

void
move_wrapper(player, command)
    dbref player;
    const char *command;
{
  /* check local exit, then zone exit, then global. If nothing is
   * matched, treat it as local so player will get an error message.
   */

  if (!Mobile(player))
    return;

  if (!strcasecmp(command, "home") && Fixed(player)) {
    notify(player, "You can't do that IC!");
    return;
  }
  if (DO_GLOBALS) {
    if (can_move(player, command))
      do_move(player, command, 0);
    else if ((Zone(Location(player)) != NOTHING) &&
	     remote_exit(player, command))
      do_move(player, command, 2);
    else if ((Location(player) != MASTER_ROOM) &&
	     global_exit(player, command))
      do_move(player, command, 1);
    else
      do_move(player, command, 0);
  } else {
    do_move(player, command, 0);
  }
}
