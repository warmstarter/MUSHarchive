/* create.c */

/* Commands that create new objects */
#include "copyrite.h"
#include "config.h"

#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif

#ifdef I_MEMORY
#include <memory.h>
#endif
#include "conf.h"
#include "mushdb.h"
#include "attrib.h"
#include "intrface.h"
#include "externs.h"
#include "match.h"
#include "confmagic.h"

static dbref parse_linkable_room _((dbref player, const char *room_name));
static dbref check_var_link _((dbref player, const char *dest_name));
dbref do_real_open _((dbref player, const char *direction, const char *linkto, dbref pseudo));
void do_open _((dbref player, const char *direction, char **links));
void do_unlink _((dbref player, const char *name));
void do_link _((dbref player, const char *name, const char *room_name));
dbref do_dig _((dbref player, const char *name, char **argv, int tport));
dbref do_create _((dbref player, char *name, int cost));
dbref do_clone _((dbref player, char *name));

/* utility for open and link */
static dbref
parse_linkable_room(player, room_name)
    dbref player;
    const char *room_name;
{
  dbref room;

  /* parse room */
  if (!strcasecmp(room_name, "here")) {
    room = Location(player);
  } else if (!strcasecmp(room_name, "home")) {
    return HOME;		/* HOME is always linkable */
  } else {
    room = parse_dbref(room_name);
  }

  /* check room */
  if (!GoodObject(room)) {
    notify(player, "That is not a valid object.");
    return NOTHING;
  } else if (Going(room)) {
    notify(player, "That room is being destroyed. Sorry.");
    return NOTHING;
  } else if (!can_link_to(player, room)) {
    notify(player, "You can't link to that.");
    return NOTHING;
  } else {
    return room;
  }
}

static dbref
check_var_link(player, dest_name)
    dbref player;
    const char *dest_name;
{
  /* This allows an exit to be linked to a 'variable' destination.
   * Such exits can only be linked by a wizard, since they have the
   * potential to lead anywhere.
   */

  if (Wizard(player) && !strcasecmp("VARIABLE", dest_name))
    return AMBIGUOUS;
  else
    return NOTHING;
}

dbref
do_real_open(player, direction, linkto, pseudo)
    dbref player;
    const char *direction;
    const char *linkto;
    dbref pseudo;



				/* a phony location for a player if a back
				 * exit is needed */
{
  /* function creates an exit. Note that it returns 0 and NOT -1
   * if it fails.
   */

  dbref loc = (pseudo != NOTHING) ? pseudo : Location(player);
  dbref new_exit;
  if ((loc == NOTHING) || (!IsRoom(loc))) {
    notify(player, "Sorry you can only make exits out of rooms.");
    return NOTHING;
  }
  if (RESTRICTED_BUILDING && !Builder(player)) {
    notify(player, "That command is restricted to authorized builders.");
    return NOTHING;
  }
  if (Guest(player)) {
    notify(player, "Guests are not allowed to build.");
    return NOTHING;
  }
  if (Going(loc)) {
    notify(player, "You can't make an exit in a place that's crumbling.");
    return NOTHING;
  }
  if (!*direction) {
    notify(player, "Open where?");
    return NOTHING;
  } else if (!ok_name(direction)) {
    notify(player, "That's a strange name for an exit!");
    return NOTHING;
  }
  if (!Open_Anywhere(player) && !controls(player, loc)) {
    notify(player, "Permission denied.");
  } else if (can_pay_fees(player, EXIT_COST)) {
    /* create the exit */
    new_exit = new_object();

    /* initialize everything */
    SET(Name(new_exit), direction);
    Owner(new_exit) = Owner(player);
    Zone(new_exit) = Zone(player);
    Source(new_exit) = loc;
    Flags(new_exit) = TYPE_EXIT;
    Flags(new_exit) |= options.exit_flags;
    Toggles(new_exit) |= options.exit_toggles;

    /* link it in */
    PUSH(new_exit, Exits(loc));

    /* and we're done */
    notify(player, "Opened.");

    /* check second arg to see if we should do a link */
    if (linkto && *linkto != '\0') {
      notify(player, "Trying to link...");
      if ((loc = check_var_link(player, linkto)) == NOTHING)
	loc = parse_linkable_room(player, linkto);
      if (loc != NOTHING) {
	if (!payfor(player, LINK_COST)) {
	  notify(player, tprintf("You don't have enough %s to link.", MONIES));
	} else {
	  /* it's ok, link it */
	  Location(new_exit) = loc;
	  notify(player, "Linked.");
	}
      }
    }
#ifdef LOCAL_DATA
    local_data_create(new_exit);
#endif
    return new_exit;
  }
  return NOTHING;
}

void
do_open(player, direction, links)
    dbref player;
    const char *direction;
    char **links;
{
  do_real_open(player, direction, links[1], NOTHING);
  if (links[2]) {
    do_real_open(player, links[2], "here",
		 parse_linkable_room(player, links[1]));
  }
}

void
do_unlink(player, name)
    dbref player;
    const char *name;
{
  dbref exit_l;
  long match_flags = MAT_EXIT | MAT_HERE | MAT_ABSOLUTE;

  if (!Wizard(player)) {
    match_flags |= MAT_CONTROL;
  }
  switch (exit_l = match_result(player, name, TYPE_EXIT, match_flags)) {
  case NOTHING:
    notify(player, "Unlink what?");
    break;
  case AMBIGUOUS:
    notify(player, "I don't know which one you mean!");
    break;
  default:
    if (!controls(player, exit_l)) {
      notify(player, "Permission denied.");
    } else {
      switch (Typeof(exit_l)) {
      case TYPE_EXIT:
	Location(exit_l) = NOTHING;
	notify(player, "Unlinked.");
	break;
      case TYPE_ROOM:
	Location(exit_l) = NOTHING;
	notify(player, "Dropto removed.");
	break;
      default:
	notify(player, "You can't unlink that!");
	break;
      }
    }
  }
}

void
do_link(player, name, room_name)
    dbref player;
    const char *name;
    const char *room_name;
{
  /* Use this to link to a room that you own. 
   * It seizes ownership of the exit and costs 1 penny,
   * plus a penny transferred to the exit owner if they aren't you.
   * You must own the linked-to room AND specify it by room number.
   */

  dbref thing;
  dbref room;

  if (!room_name || !*room_name) {
    do_unlink(player, name);
    return;
  }
  if (!IsRoom(player) && IsExit(Location(player))) {
    notify(player, "You somehow wound up in a exit. No biscuit.");
    return;
  }
  if ((thing = noisy_match_result(player, name, TYPE_EXIT, MAT_EVERYTHING)) != NOTHING) {
    switch (Typeof(thing)) {
    case TYPE_EXIT:
      if ((room = check_var_link(player, room_name)) == NOTHING)
	room = parse_linkable_room(player, room_name);
      if (room == NOTHING)
	return;
      if (GoodObject(room) && !controls(player, room) && !LinkOk(room)) {
	notify(player, "Permission denied.");
	break;
      }
      /* If the exit is already linked, we can only link it if we
       * control it.
       */
      if ((Location(thing) != NOTHING) && !controls(player, thing)) {
	notify(player, "Permission denied.");
	return;
      }
      /* handle costs */
      if (Owner(thing) == Owner(player)) {
	if (!payfor(player, LINK_COST)) {
	  notify(player, tprintf("It costs %d %s to link this exit.",
				 LINK_COST,
				 ((LINK_COST == 1) ? MONEY : MONIES)));
	  return;
	}
      } else {
	if (RESTRICTED_BUILDING && !Builder(player)) {
	  notify(player,
		 "Only authorized builders may seize exits.");
	  return;
	} else if (Guest(player)) {
	  notify(player, "Guests are not allowed to build.");
	  return;
	} else if (!payfor(player, LINK_COST + EXIT_COST)) {
	  int a = 0;
	  notify(player, tprintf("It costs %d %s to link this exit.",
				 (a = LINK_COST + EXIT_COST),
				 ((a == 1) ? MONEY : MONIES)));
	  return;
	} else {
	  /* pay the owner for his loss */
	  giveto(Owner(thing), EXIT_COST);
	  chown_object(player, thing, player);
	}
      }

      /* link has been validated and paid for; do it */
      Owner(thing) = Owner(player);
      Zone(thing) = Zone(player);
      Location(thing) = room;

      /* notify the player */
      notify(player, "Linked.");
      break;
    case TYPE_PLAYER:
    case TYPE_THING:
      if ((room = noisy_match_result(player, room_name, NOTYPE, MAT_EVERYTHING)) < 0) {
	notify(player, "No match.");
	return;
      }
      if (IsExit(room)) {
	notify(player, "That is an exit.");
	return;
      }
      if (thing == room) {
	notify(player, "You may not link something to itself.");
	return;
      }
      /* abode */
      if (!controls(player, room) && !Abode(room)) {
	notify(player, "Permission denied.");
	break;
      }
      if (!controls(player, thing)) {
	notify(player, "Permission denied.");
      } else if (room == HOME) {
	notify(player, "Can't set home to home.");
      } else {
	/* do the link */
	Home(thing) = room;	/* home */
	notify(player, "Home set.");
      }
      break;
    case TYPE_ROOM:
      if ((room = parse_linkable_room(player, room_name)) == NOTHING)
	return;
      if ((room != HOME) && (!IsRoom(room))) {
	notify(player, "That is not a room!");
	return;
      }
      if (!controls(player, thing)) {
	notify(player, "Permission denied.");
      } else {
	/* do the link, in location */
	Location(thing) = room;	/* dropto */
	notify(player, "Dropto set.");
      }
      break;
    default:
      notify(player, "Internal error: weird object type.");
      do_log(LT_ERR, NOTHING, NOTHING,
	     "Weird object! Type of #%d is %d", thing, Typeof(thing));
      break;
    }
  }
}

/* use this to create a room */
dbref
do_dig(player, name, argv, tport)
    dbref player;
    const char *name;
    char **argv;
    int tport;
				/* @tel player to new location? */
{
  dbref room;

  if (RESTRICTED_BUILDING && !Builder(player)) {
    notify(player, "That command is restricted to authorized builders.");
    return NOTHING;
  }
  if (Guest(player)) {
    notify(player, "Guests are not allowed to build.");
    return NOTHING;
  }
  /* we don't need to know player's location!  hooray! */
  if (*name == '\0') {
    notify(player, "Dig what?");
  } else if (!ok_name(name)) {
    notify(player, "That's a silly name for a room!");
  } else if (can_pay_fees(player, ROOM_COST)) {
    room = new_object();

    /* Initialize everything */
    SET(Name(room), name);
    Owner(room) = Owner(player);
    Zone(room) = Zone(player);
    Flags(room) = TYPE_ROOM;
    Flags(room) |= options.room_flags;
    Toggles(room) |= options.room_toggles;

    notify(player,
	   tprintf("%s created with room number %d.", name, room));
    if (argv[1] && *argv[1]) {
      char nbuff[MAX_COMMAND_LEN];
      sprintf(nbuff, "#%d", room);
      do_real_open(player, argv[1], nbuff, NOTHING);
    }
    if (argv[2] && *argv[2]) {
      do_real_open(player, argv[2], "here", room);
    }
#ifdef LOCAL_DATA
    local_data_create(room);
#endif
    if (tport) {
      /* We need to use the full command, because we need NO_TEL
       * and Z_TEL checking */
      char roomstr[MAX_COMMAND_LEN];
      sprintf(roomstr, "#%d", room);
      do_teleport(player, "me", roomstr);	/* if flag, move the player */
    }
    return room;
  }
  return NOTHING;
}

/* use this to create an object */
dbref
do_create(player, name, cost)
    dbref player;
    char *name;
    int cost;
{
  dbref loc;
  dbref thing;

  if (RESTRICTED_BUILDING && !FREE_OBJECTS && !Builder(Owner(player))) {
    notify(player, "That command is restricted to authorized builders.");
    return NOTHING;
  }
  if (Guest(player)) {
    notify(player, "Guests are not allowed to build.");
    return NOTHING;
  }
  if (*name == '\0') {
    notify(player, "Create what?");
    return NOTHING;
  } else if (!ok_name(name)) {
    notify(player, "That's a silly name for a thing!");
    return NOTHING;
  } else if (cost < OBJECT_COST) {
    cost = OBJECT_COST;
  }
  if (can_pay_fees(player, cost)) {
    /* create the object */
    thing = new_object();

    /* initialize everything */
    SET(Name(thing), name);
    Location(thing) = player;
    Owner(thing) = Owner(player);
    Zone(thing) = Zone(player);
    s_Pennies(thing, OBJECT_ENDOWMENT(cost));
    Flags(thing) = TYPE_THING;
    Flags(thing) |= options.thing_flags;
    Toggles(thing) |= options.thing_toggles;

    /* home is here (if we can link to it) or player's home */
    if ((loc = Location(player)) != NOTHING &&
	(controls(player, loc) ||
	 IS(loc, TYPE_ROOM, ROOM_ABODE))) {
      Home(thing) = loc;	/* home */
    } else {
      Home(thing) = Home(player);	/* home */
    }

    /* link it in */
    PUSH(thing, Contents(player));

    /* and we're done */
    notify(player, tprintf("Created: Object #%d.", thing));
#ifdef LOCAL_DATA
    local_data_create(thing);
#endif
    return thing;
  }
  return NOTHING;
}

dbref
do_clone(player, name)
    dbref player;
    char *name;
{
  dbref clone, thing;

  if (Guest(player)) {
    notify(player, "Guests are not allowed to build.");
    return NOTHING;
  }
  thing = noisy_match_result(player, name, NOTYPE, MAT_EVERYTHING);
  if ((thing == NOTHING))
    return NOTHING;

  if (!controls(player, thing)) {
    notify(player, "Permission denied.");
    return NOTHING;
  }
  /* don't allow cloning of destructed things */
  if (IsGarbage(thing)) {
    notify(player, "There's nothing left of it to clone!");
    return NOTHING;
  }
  /* make sure owner can afford it */
  switch (Typeof(thing)) {
  case TYPE_THING:
    if (RESTRICTED_BUILDING && !FREE_OBJECTS && !Builder(player)) {
      notify(player, "Command is for builders only.");
      return NOTHING;
    }
    if (can_pay_fees(player, OBJECT_DEPOSIT(Pennies(thing)))) {
      clone = new_object();
      memcpy(REFDB(clone), REFDB(thing), sizeof(struct object));
#ifdef CHAT_SYSTEM
      Chanlist(clone) = NULL;
#endif
      Name(clone) = NULL;
      SET(Name(clone), Name(thing));
      s_Pennies(clone, Pennies(thing));
      atr_cpy(clone, thing);
      {
	struct lock_list *ll;
	Locks(clone) = NULL;
	for (ll = Locks(thing); ll; ll = ll->next) {
	  add_lock(clone, ll->type, dup_bool(ll->key));
	}
      }
      Zone(clone) = Zone(thing);
      Parent(clone) = Parent(thing);
      Flags(clone) &= ~WIZARD;
#ifdef ROYALTY_FLAG
      Flags(clone) &= ~ROYALTY;
#endif
#ifdef USE_WARNINGS
      Warnings(clone) = 0;	/* zap warnings */
#endif
#ifdef CREATION_TIMES
      /* We give the clone the same modification time that its
       * other clone has, but update the creation time */
      CreTime(clone) = time((time_t *) 0);
#endif
      Powers(clone) = 0;	/* zap powers */

      Contents(clone) = Location(clone) = Next(clone) = NOTHING;
      notify(player, tprintf("Cloned: Object #%d.", clone));
      if (IsRoom(player))
	moveto(clone, player);
      else
	moveto(clone, Location(player));
#ifdef LOCAL_DATA
      LocData(clone) = NULL;
      local_data_clone(clone, thing);
#endif
      did_it(player, clone, NULL, NULL, NULL, NULL, "ACLONE", NOTHING);
      return clone;
    }
    return NOTHING;
    break;
  case TYPE_EXIT:
    if (RESTRICTED_BUILDING && !Builder(player)) {
      notify(player, "Command is for builders only.");
      return NOTHING;
    }
    clone = do_real_open(player, Name(thing),
			 tprintf("#%d", Location(thing)), NOTHING);
    if (!GoodObject(clone))
      return NOTHING;
    else {
      atr_cpy(clone, thing);
      {
	struct lock_list *ll;
	for (ll = Locks(thing); ll; ll = ll->next) {
	  add_lock(clone, ll->type, dup_bool(ll->key));
	}
      }
      Zone(clone) = Zone(thing);
      Parent(clone) = Parent(thing);
      Flags(clone) = Flags(thing);
      Flags(clone) &= ~WIZARD;
#ifdef ROYALTY_FLAG
      Flags(clone) &= ~ROYALTY;
#endif
#ifdef USE_WARNINGS
      Warnings(clone) = 0;	/* zap warnings */
#endif
      Powers(clone) = 0;	/* zap powers */
      notify(player, "Exit cloned.");
#ifdef LOCAL_DATA
      LocData(clone) = NULL;
      local_data_clone(clone, thing);
#endif
      return clone;
    }
    break;
  default:
    notify(player, "You can only clone things and exits.");
    return NOTHING;
  }

}
