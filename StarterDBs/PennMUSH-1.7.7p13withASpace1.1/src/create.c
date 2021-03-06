/**
 * \file create.c
 *
 * \brief Functions for creating objects of all types.
 *
 *
 */

#include "copyrite.h"
#include "config.h"
#include <string.h>
#include "conf.h"
#include "mushdb.h"
#include "attrib.h"
#include "externs.h"
#include "match.h"
#include "extchat.h"
#include "log.h"
#include "flags.h"
#include "dbdefs.h"
#include "lock.h"
#include "parse.h"
#include "game.h"
#include "command.h"
#include "confmagic.h"

static dbref parse_linkable_room(dbref player, const char *room_name);
static dbref check_var_link(const char *dest_name);
static dbref clone_object(dbref player, dbref thing, const char *newname,
			  int preserve);

struct db_stat_info current_state;

/* utility for open and link */
static dbref
parse_linkable_room(dbref player, const char *room_name)
{
  dbref room;

  /* parse room */
  if (!strcasecmp(room_name, "here")) {
    room = IsExit(player) ? Source(player) : Location(player);
  } else if (!strcasecmp(room_name, "home")) {
    return HOME;		/* HOME is always linkable */
  } else {
    room = parse_dbref(room_name);
  }

  /* check room */
  if (!GoodObject(room)) {
    notify(player, T("That is not a valid object."));
    return NOTHING;
  } else if (Going(room)) {
    notify(player, T("That room is being destroyed. Sorry."));
    return NOTHING;
  } else if (!can_link_to(player, room)) {
    notify(player, T("You can't link to that."));
    return NOTHING;
  } else {
    return room;
  }
}

static dbref
check_var_link(const char *dest_name)
{
  /* This allows an exit to be linked to a 'variable' destination.
   * Such exits can be linked by anyone but the owner's ability
   * to link to the destination is checked when it's computed.
   */

  if (!strcasecmp("VARIABLE", dest_name))
    return AMBIGUOUS;
  else
    return NOTHING;
}

/** Create an exit.
 * This function opens an exit and optionally links it.
 * \param player the enactor.
 * \param direction the name of the exit.
 * \param linkto the room to link to, as a string.
 * \param pseudo a phony location for player if a back exit is needed. This is bpass by do_open() as the source room of the back exit.
 * \return dbref of the new exit, or NOTHING.
 */
dbref
do_real_open(dbref player, const char *direction, const char *linkto,
	     dbref pseudo)
{
  dbref loc =
    (pseudo !=
     NOTHING) ? pseudo : (IsExit(player) ? Source(player) : Location(player));
  dbref new_exit;
  if (!command_check_byname(player, "@dig")) {
    notify(player, "Permission denied.");
    return NOTHING;
  }
  if ((loc == NOTHING) || (!IsRoom(loc))) {
    notify(player, T("Sorry you can only make exits out of rooms."));
    return NOTHING;
  }
  if (Going(loc)) {
    notify(player, T("You can't make an exit in a place that's crumbling."));
    return NOTHING;
  }
  if (!*direction) {
    notify(player, T("Open where?"));
    return NOTHING;
  } else if (!ok_name(direction)) {
    notify(player, T("That's a strange name for an exit!"));
    return NOTHING;
  }
  if (!Open_Anywhere(player) && !controls(player, loc)) {
    notify(player, T("Permission denied."));
  } else if (can_pay_fees(player, EXIT_COST)) {
    /* create the exit */
    new_exit = new_object();

    /* initialize everything */
    set_name(new_exit, direction);
    Owner(new_exit) = Owner(player);
    Zone(new_exit) = Zone(player);
    Source(new_exit) = loc;
    Type(new_exit) = TYPE_EXIT;
    Flags(new_exit) = string_to_bits(options.exit_flags);

    /* link it in */
    PUSH(new_exit, Exits(loc));

    /* and we're done */
    notify_format(player, T("Opened exit %s"), unparse_dbref(new_exit));

    /* check second arg to see if we should do a link */
    if (linkto && *linkto != '\0') {
      notify(player, T("Trying to link..."));
      if ((loc = check_var_link(linkto)) == NOTHING)
	loc = parse_linkable_room(player, linkto);
      if (loc != NOTHING) {
	if (!payfor(player, LINK_COST)) {
	  notify_format(player, T("You don't have enough %s to link."), MONIES);
	} else {
	  /* it's ok, link it */
	  Location(new_exit) = loc;
	  notify_format(player, T("Linked exit #%d to #%d"), new_exit, loc);
	}
      }
    }
    current_state.exits++;
    local_data_create(new_exit);
    return new_exit;
  }
  return NOTHING;
}

/** Open a new exit.
 * \verbatim
 * This is the top-level function for @open. It calls do_real_open()
 * to do the real work of opening both the exit forward and the exit back.
 * \endverbatim
 * \param player the enactor.
 * \param direction name of the exit forward.
 * \param links 1-based array containing name of destination and optionally name of exit back.
 */
void
do_open(dbref player, const char *direction, char **links)
{
  dbref forward;
  forward = do_real_open(player, direction, links[1], NOTHING);
  if (links[2] && GoodObject(forward) && GoodObject(Location(forward))) {
    do_real_open(player, links[2], "here", Location(forward));
  }
}

/** Unlink an exit or room.
 * \verbatim
 * This is the top-level function for @unlink, which can unlink an exit
 * or remove a drop-to from a room.
 * \endverbatim
 * \param player the enactor.
 * \param name name of the object to unlink.
 */
void
do_unlink(dbref player, const char *name)
{
  dbref exit_l;
  long match_flags = MAT_EXIT | MAT_HERE | MAT_ABSOLUTE;

  if (!Wizard(player)) {
    match_flags |= MAT_CONTROL;
  }
  switch (exit_l = match_result(player, name, TYPE_EXIT, match_flags)) {
  case NOTHING:
    notify(player, T("Unlink what?"));
    break;
  case AMBIGUOUS:
    notify(player, T("I don't know which one you mean!"));
    break;
  default:
    if (!controls(player, exit_l)) {
      notify(player, T("Permission denied."));
    } else {
      switch (Typeof(exit_l)) {
      case TYPE_EXIT:
	Location(exit_l) = NOTHING;
	notify_format(player, T("Unlinked exit #%d."), exit_l);
	break;
      case TYPE_ROOM:
	Location(exit_l) = NOTHING;
	notify(player, T("Dropto removed."));
	break;
      default:
	notify(player, T("You can't unlink that!"));
	break;
      }
    }
  }
}

/** Link an exit, room, player, or thing.
 * \verbatim
 * This is the top-level function for @link, which is used to link an
 * exit to a destination, set a player or thing's home, or set a 
 * drop-to on a room.
 *
 * Linking an exit usually seizes ownership of the exit and costs 1 penny.
 * 1 penny is also transferred to the former owner.
 * \endverbatim
 * \param player the enactor.
 * \param name the name of the object to link.
 * \param room_name the name of the link destination.
 * \param preserve if 1, preserve ownership and zone data on exit relink.
 */
void
do_link(dbref player, const char *name, const char *room_name, int preserve)
{
  /* Use this to link to a room that you own. 
   * It usually seizes ownership of the exit and costs 1 penny,
   * plus a penny transferred to the exit owner if they aren't you.
   * You must own the linked-to room AND specify it by room number.
   */

  dbref thing;
  dbref room;

  if (!room_name || !*room_name) {
    do_unlink(player, name);
    return;
  }
  if (!IsRoom(player) && GoodObject(Location(player)) &&
      IsExit(Location(player))) {
    notify(player, T("You somehow wound up in a exit. No biscuit."));
    return;
  }
  if ((thing = noisy_match_result(player, name, TYPE_EXIT, MAT_EVERYTHING))
      != NOTHING) {
    switch (Typeof(thing)) {
    case TYPE_EXIT:
      if ((room = check_var_link(room_name)) == NOTHING)
	room = parse_linkable_room(player, room_name);
      if (room == NOTHING)
	return;
      if (GoodObject(room) && !can_link_to(player, room)) {
	notify(player, T("Permission denied."));
	break;
      }
      /* We may link an exit if it's unlinked and we pass the link-lock
       * or if we control it.
       */
      if (!(controls(player, thing)
	    || ((Location(thing) == NOTHING)
		&& eval_lock(player, thing, Link_Lock)))) {
	notify(player, T("Permission denied."));
	return;
      }
      if (preserve && !Wizard(player)) {
	notify(player, T("Permission denied."));
	return;
      }
      /* handle costs */
      if (Owner(thing) == Owner(player)) {
	if (!payfor(player, LINK_COST)) {
	  notify_format(player, T("It costs %d %s to link this exit."),
			LINK_COST, ((LINK_COST == 1) ? MONEY : MONIES));
	  return;
	}
      } else {
	if (!payfor(player, LINK_COST + EXIT_COST)) {
	  int a = LINK_COST + EXIT_COST;
	  notify_format(player, T("It costs %d %s to link this exit."), a,
			((a == 1) ? MONEY : MONIES));
	  return;
	} else if (!preserve) {
	  /* pay the owner for his loss */
	  giveto(Owner(thing), EXIT_COST);
	  chown_object(player, thing, player, 0);
	}
      }

      /* link has been validated and paid for; do it */
      if (!preserve) {
	Owner(thing) = Owner(player);
	Zone(thing) = Zone(player);
      }
      Location(thing) = room;

      /* notify the player */
      notify_format(player, T("Linked exit #%d to #%d"), thing, room);
      break;
    case TYPE_PLAYER:
    case TYPE_THING:
      if ((room =
	   noisy_match_result(player, room_name, NOTYPE, MAT_EVERYTHING)) < 0) {
	notify(player, T("No match."));
	return;
      }
      if (IsExit(room)) {
	notify(player, T("That is an exit."));
	return;
      }
      if (thing == room) {
	notify(player, T("You may not link something to itself."));
	return;
      }
      /* abode */
      if (!controls(player, room) && !Abode(room)) {
	notify(player, T("Permission denied."));
	break;
      }
      if (!controls(player, thing)) {
	notify(player, T("Permission denied."));
      } else if (room == HOME) {
	notify(player, T("Can't set home to home."));
      } else {
	/* do the link */
	Home(thing) = room;	/* home */
	notify(player, T("Home set."));
      }
      break;
    case TYPE_ROOM:
      if ((room = parse_linkable_room(player, room_name)) == NOTHING)
	return;
      if ((room != HOME) && (!IsRoom(room))) {
	notify(player, T("That is not a room!"));
	return;
      }
      if (!controls(player, thing)) {
	notify(player, T("Permission denied."));
      } else {
	/* do the link, in location */
	Location(thing) = room;	/* dropto */
	notify(player, T("Dropto set."));
      }
      break;
    default:
      notify(player, "Internal error: weird object type.");
      do_log(LT_ERR, NOTHING, NOTHING,
	     T("Weird object! Type of #%d is %d"), thing, Typeof(thing));
      break;
    }
  }
}

/** Create a room.
 * \verbatim
 * This is the top-level interface for @dig.
 * \endverbatim
 * \param player the enactor.
 * \param name the name of the room to create.
 * \param argv array of additional arguments to command (exit forward,exit back)
 * \param tport if 1, teleport the player to the new room.
 * \return dbref of new room, or NOTHING.
 */
dbref
do_dig(dbref player, const char *name, char **argv, int tport)
{
  dbref room;

  /* we don't need to know player's location!  hooray! */
  if (*name == '\0') {
    notify(player, T("Dig what?"));
  } else if (!ok_name(name)) {
    notify(player, T("That's a silly name for a room!"));
  } else if (can_pay_fees(player, ROOM_COST)) {
    room = new_object();

    /* Initialize everything */
    set_name(room, name);
    Owner(room) = Owner(player);
    Zone(room) = Zone(player);
    Type(room) = TYPE_ROOM;
    Flags(room) = string_to_bits(options.room_flags);

    notify_format(player, T("%s created with room number %d."), name, room);
    if (argv[1] && *argv[1]) {
      char nbuff[MAX_COMMAND_LEN];
      sprintf(nbuff, "#%d", room);
      do_real_open(player, argv[1], nbuff, NOTHING);
    }
    if (argv[2] && *argv[2]) {
      do_real_open(player, argv[2], "here", room);
    }
    current_state.rooms++;
    local_data_create(room);
    if (tport) {
      /* We need to use the full command, because we need NO_TEL
       * and Z_TEL checking */
      char roomstr[MAX_COMMAND_LEN];
      sprintf(roomstr, "#%d", room);
      do_teleport(player, "me", roomstr, 0);	/* if flag, move the player */
    }
    return room;
  }
  return NOTHING;
}

/** Create a thing.
 * \verbatim
 * This is the top-level function for @create.
 * \endverbatim
 * \param player the enactor.
 * \param name name of thing to create.
 * \param cost pennies spent in creation.
 * \return dbref of new thing, or NOTHING.
 */
dbref
do_create(dbref player, char *name, int cost)
{
  dbref loc;
  dbref thing;

  if (*name == '\0') {
    notify(player, T("Create what?"));
    return NOTHING;
  } else if (!ok_name(name)) {
    notify(player, T("That's a silly name for a thing!"));
    return NOTHING;
  } else if (cost < OBJECT_COST) {
    cost = OBJECT_COST;
  }
  if (can_pay_fees(player, cost)) {
    /* create the object */
    thing = new_object();

    /* initialize everything */
    set_name(thing, name);
    if (!IsExit(player))	/* Exits shouldn't have contents! */
      Location(thing) = player;
    else
      Location(thing) = Source(player);
    Owner(thing) = Owner(player);
    Zone(thing) = Zone(player);
    s_Pennies(thing, cost);
    Type(thing) = TYPE_THING;
    Flags(thing) = string_to_bits(options.thing_flags);

    /* home is here (if we can link to it) or player's home */
    if ((loc = Location(player)) != NOTHING &&
	(controls(player, loc) || Abode(loc))) {
      Home(thing) = loc;	/* home */
    } else {
      Home(thing) = Home(player);	/* home */
    }

    /* link it in */
    if (!IsExit(player))
      PUSH(thing, Contents(player));
    else
      PUSH(thing, Contents(Source(player)));

    /* and we're done */
    notify_format(player, "Created: Object %s.", unparse_dbref(thing));
    current_state.things++;
    local_data_create(thing);
    return thing;
  }
  return NOTHING;
}

/* Clone an object. The new object is owned by the cloning player */
static dbref
clone_object(dbref player, dbref thing, const char *newname, int preserve)
{
  dbref clone;

  clone = new_object();

  memcpy(REFDB(clone), REFDB(thing), sizeof(struct object));
  Owner(clone) = Owner(player);
  Name(clone) = NULL;
  if (newname && *newname)
    set_name(clone, newname);
  else
    set_name(clone, Name(thing));
  s_Pennies(clone, Pennies(thing));
  atr_cpy(clone, thing);
  Locks(clone) = NULL;
  clone_locks(player, thing, clone);
  Zone(clone) = Zone(thing);
  Parent(clone) = Parent(thing);
  Flags(clone) = clone_flag_bitmask(Flags(thing));
  if (!preserve) {
    clear_flag_internal(clone, "WIZARD");
    clear_flag_internal(clone, "ROYALTY");
    Warnings(clone) = 0;	/* zap warnings */
    Powers(clone) = 0;		/* zap powers */
  } else if (Wizard(clone) ||
	     Royalty(clone) || Warnings(clone) || Powers(clone)) {
    notify(player,
	   T
	   ("Warning: @CLONE/PRESERVE on an object with WIZ, ROY, @powers, or @warnings."));
  }
  /* We give the clone the same modification time that its
   * other clone has, but update the creation time */
  CreTime(clone) = mudtime;

  Contents(clone) = Location(clone) = Next(clone) = NOTHING;

  return clone;

}

/** Clone an object.
 * \verbatim
 * This is the top-level function for @clone, which creates a duplicate
 * of a (non-player) object.
 * \endverbatim
 * \param player the enactor.
 * \param name the name of the object to clone.
 * \param newname the name to give the duplicate.
 * \param preserve if 1, preserve ownership and privileges on duplicate.
 * \return dbref of the duplicate, or NOTHING.
 */
dbref
do_clone(dbref player, char *name, char *newname, int preserve)
{
  dbref clone, thing;
  char dbnum[BUFFER_LEN];

  if (newname && *newname && !ok_name(newname)) {
    notify(player, T("That is not a reasonable name."));
    return NOTHING;
  }
  thing = noisy_match_result(player, name, NOTYPE, MAT_EVERYTHING);
  if ((thing == NOTHING))
    return NOTHING;

  if (!controls(player, thing) || IsPlayer(thing) ||
      (IsRoom(thing) && !command_check_byname(player, "@dig")) ||
      (IsExit(thing) && !command_check_byname(player, "@open")) ||
      (IsThing(thing) && !command_check_byname(player, "@create"))) {
    notify(player, T("Permission denied."));
    return NOTHING;
  }
  /* don't allow cloning of destructed things */
  if (IsGarbage(thing)) {
    notify(player, T("There's nothing left of it to clone!"));
    return NOTHING;
  }
  if (preserve && !Wizard(player)) {
    notify(player,
	   T("You cannot @CLONE/PRESERVE.  Use normal @CLONE instead."));
    return NOTHING;
  }
  /* make sure owner can afford it */
  switch (Typeof(thing)) {
  case TYPE_THING:
    if (can_pay_fees(player, Pennies(thing))) {
      clone = clone_object(player, thing, newname, preserve);
      notify_format(player, T("Cloned: Object %s."), unparse_dbref(clone));
      if (IsRoom(player))
	moveto(clone, player);
      else
	moveto(clone, Location(player));
      current_state.things++;
      local_data_clone(clone, thing);
      real_did_it(player, clone, NULL, NULL, NULL, NULL, "ACLONE", NOTHING,
		  wenv);
      return clone;
    }
    return NOTHING;
    break;
  case TYPE_ROOM:
    if (can_pay_fees(player, ROOM_COST)) {
      clone = clone_object(player, thing, newname, preserve);
      Exits(clone) = NOTHING;
      notify_format(player, T("Cloned: Room #%d."), clone);
      current_state.rooms++;
      local_data_clone(clone, thing);
      real_did_it(player, clone, NULL, NULL, NULL, NULL, "ACLONE", NOTHING,
		  wenv);
      return clone;
    }
    return NOTHING;
    break;
  case TYPE_EXIT:
    /* For exits, we don't want people to be able to link it to
       a location they can't with @open. So, all this stuff. 
     */
    switch (Location(thing)) {
    case NOTHING:
      strcpy(dbnum, "#-1");
      break;
    case HOME:
      strcpy(dbnum, "home");
      break;
    case AMBIGUOUS:
      strcpy(dbnum, "variable");
      break;
    default:
      strcpy(dbnum, unparse_dbref(Location(thing)));
    }
    if (newname && *newname)
      clone = do_real_open(player, newname, dbnum, NOTHING);
    else
      clone = do_real_open(player, Name(thing), dbnum, NOTHING);
    if (!GoodObject(clone))
      return NOTHING;
    else {
      atr_cpy(clone, thing);
      clone_locks(player, thing, clone);
      Zone(clone) = Zone(thing);
      Parent(clone) = Parent(thing);
      Flags(clone) = clone_flag_bitmask(Flags(thing));
      if (!preserve) {
	clear_flag_internal(clone, "WIZARD");
	clear_flag_internal(clone, "ROYALTY");
	Warnings(clone) = 0;	/* zap warnings */
	Powers(clone) = 0;	/* zap powers */
      } else {
	Warnings(clone) = Warnings(thing);
	Powers(clone) = Powers(thing);
      }
      if (Wizard(clone) || Royalty(clone) || Warnings(clone) || Powers(clone))
	notify(player,
	       T
	       ("Warning: @CLONE/PRESERVE on an exit with WIZ, ROY, @Powers, or Warnings."));
      notify_format(player, T("Cloned: Exit #%d."), clone);
      local_data_clone(clone, thing);
      return clone;
    }
  }
  return NOTHING;

}
