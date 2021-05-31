/* wiz.c */

#include "copyrite.h"
#include "config.h"

/* Wizard-only commands */

#ifdef I_UNISTD
#include <unistd.h>
#endif
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <math.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#include <ctype.h>
#ifdef I_FCNTL
#include <fcntl.h>
#endif
#ifdef WIN32
#include <windows.h>
#undef OPAQUE
#endif
#include "conf.h"
#include "mushdb.h"
#include "intrface.h"
#include "match.h"
#include "externs.h"
#include "access.h"
#include "parse.h"
#include "mymalloc.h"


#ifdef MEM_CHECK
#include "memcheck.h"
#endif

#include "confmagic.h"

extern dbref find_entrance _((dbref door));
extern void delete_player _((dbref player, char *alias));
extern int convert_flags _((dbref player, char *s, object_flag_type *p_mask, object_flag_type *p_toggle, object_flag_type *p_type));
extern int hidden _((dbref player));
extern struct db_stat_info *get_stats _((dbref player, dbref owner));
extern dbref find_player_by_desc _((int port));
extern void local_shutdown _((void));
extern int paranoid_dump;
extern int paranoid_checkpt;

extern int invok_counter;	/* invocation limit */


#ifndef WIN32
#ifdef I_SYS_FILE
#include <sys/file.h>
#endif
extern int reserved;
#else
void Win32stats(dbref player);
extern void compress_stats(long *entries,
			   long *mem_used,
			   long *total_uncompressed,
			   long *total_compressed);
#endif

void do_pcreate _((dbref creator, const char *player_name, const char *player_password));
#ifdef QUOTA
void do_quota _((dbref player, const char *arg1, const char *arg2, int set_q));
void do_allquota _((dbref player, const char *arg1, int quiet));
#endif
int tport_dest_ok _((dbref player, dbref victim, dbref dest));
int tport_control_ok _((dbref player, dbref victim, dbref loc));
void do_teleport _((dbref player, const char *arg1, const char *arg2));
void do_force _((dbref player, const char *what, char *command));
int force_by_number _((dbref player, char *command));
void do_stats _((dbref player, const char *name));
void do_newpassword _((dbref player, const char *name, const char *password));
void do_boot _((dbref player, const char *name, int flag));
#ifdef CONCENTRATOR
int parse_conc_descriptor _((char *str, int cport, int port));
#endif
void do_chownall _((dbref player, const char *name, const char *target));
void do_chzoneall _((dbref player, const char *name, const char *target));
void do_enable _((dbref player, const char *param, int state));
void do_kick _((dbref player, const char *num));
void do_fixdb _((dbref player, const char *name, const char *val, int sw_opt));
void do_debug_examine _((dbref player, const char *name));
void do_power _((dbref player, const char *name, const char *power));
void do_search _((dbref player, const char *arg1, char **arg3));
void do_sitelock _((dbref player, const char *site, const char *opts, int type));
void do_reboot _((dbref player, int flag));
static int mem_usage _((dbref thing));

extern char confname[BUFFER_LEN];
extern char errlog[BUFFER_LEN];

void
do_pcreate(creator, player_name, player_password)
    dbref creator;
    const char *player_name;
    const char *player_password;
{
  dbref player;

  if (!Create_Player(creator)) {
    notify(creator, "You do not have the power over body and mind!");
    return;
  }
  player = create_player(player_name, player_password, "None", "None");
  if (player == NOTHING) {
    notify(creator, tprintf("failure creating '%s' (bad name)", player_name));
    return;
  }
  if (player == AMBIGUOUS) {
    notify(creator, tprintf("failure creating '%s' (bad password)", player_name));
    return;
  }
  notify(creator, tprintf("New player '%s' (#%d) created with password '%s'",
			  player_name, player, player_password));
  do_log(LT_WIZ, creator, player, "Player creation");
}

#ifdef QUOTA
void
do_quota(player, arg1, arg2, set_q)
    dbref player;
    const char *arg1;
    const char *arg2;
    int set_q;			/* 0 = @quota, 1 = @squota */
{

  dbref who, thing;
  int owned, limit, adjust;

  /* determine the victim */
  if (!arg1 || !*arg1 || !strcmp(arg1, "me"))
    who = player;
  else {
    who = lookup_player(arg1);
    if (who == NOTHING) {
      notify(player, "No such player.");
      return;
    }
  }

  /* check permissions */
  if (!Wizard(player) && set_q) {
    notify(player, "Only wizards may change a quota.");
    return;
  }
  if (!Do_Quotas(player) && !See_All(player) && (player != who)) {
    notify(player, "You can't look at someone else's quota.");
    return;
  }
  /* count up all owned objects */
  owned = -1;			/* a player is never included in his own
				 * quota */
  for (thing = 0; thing < db_top; thing++) {
    if (Owner(thing) == who)
      if (!IsGarbage(thing))
	++owned;
  }

  /* the quotas of priv'ed players are unlimited and cannot be set. */
  if (NoQuota(who)) {
    notify(player, tprintf("Objects: %d   Limit: UNLIMITED", owned));
    return;
  }
  /* if we're not doing a change, determine the mortal's quota limit. 
   * RQUOTA is the objects _left_, not the quota itself.
   */

  if (!set_q) {
    limit = get_current_quota(who);
    notify(player, tprintf("Objects: %d   Limit: %d",
			   owned, owned + limit));
    return;
  }
  /* set a new quota */
  if (!arg2 || !*arg2) {
    limit = get_current_quota(who);
    notify(player, tprintf("Objects: %d   Limit: %d",
			   owned, owned + limit));
    notify(player, "What do you want to set the quota to?");
    return;
  }
  adjust = ((*arg2 == '+') || (*arg2 == '-'));
  if (adjust)
    limit = owned + get_current_quota(who) + atoi(arg2);
  else
    limit = atoi(arg2);
  if (limit < owned)		/* always have enough quota for your objects */
    limit = owned;

  atr_add(Owner(who), "RQUOTA", tprintf("%d", limit - owned), GOD, NOTHING);

  notify(player, tprintf("Objects: %d   Limit: %d", owned, limit));
}


void
do_allquota(player, arg1, quiet)
    dbref player;
    const char *arg1;
    int quiet;
{
  int oldlimit, limit, owned;
  dbref who, thing;

  if (!God(player)) {
    notify(player, "Who do you think you are, GOD?");
    return;
  }
  if (!arg1 || !*arg1) {
    limit = -1;
  } else if (!is_integer(arg1)) {
    notify(player, "You can only set quotas to a number.");
    return;
  } else {
    limit = parse_integer(arg1);
    if (limit < 0) {
      notify(player, "You can only set quotas to a positive number.");
      return;
    }
  }

  for (who = 0; who < db_top; who++) {
    if (!IsPlayer(who))
      continue;

    /* count up all owned objects */
    owned = -1;			/* a player is never included in his own
				 * quota */
    for (thing = 0; thing < db_top; thing++) {
      if (Owner(thing) == who)
	if (!IsGarbage(thing))
	  ++owned;
    }

    if (NoQuota(who)) {
      if (!quiet)
	notify(player, tprintf("%s: Objects: %d   Limit: UNLIMITED",
			       Name(who), owned));
      continue;
    }
    if (!quiet) {
      oldlimit = get_current_quota(who);
      notify(player, tprintf("%s: Objects: %d   Limit: %d",
			     Name(who), owned, oldlimit));
    }
    if (limit != -1) {
      if (limit <= owned)
	atr_add(who, "RQUOTA", "0", GOD, NOTHING);
      else
	atr_add(who, "RQUOTA", tprintf("%d", limit - owned), GOD, NOTHING);
    }
  }
  if (limit == -1)
    notify(player, "Quotas not changed.");
  else
    notify(player, tprintf("All quotas changed to %d.", limit));
}
#endif				/* QUOTA */

int
tport_dest_ok(player, victim, dest)
    dbref player;
    dbref victim;
    dbref dest;
{
  /* can player legitimately send something to dest */

  if (Tel_Anywhere(player))
    return 1;

  if (controls(player, dest))
    return 1;

  /* beyond this point, if you don't control it and it's not a room, no hope */
  if (!IsRoom(dest))
    return 0;

  /* Check for a teleport lock. It fails if the player is not wiz or
   * royalty, and the room is tport-locked against the victim, and the
   * victim does not control the room.
   */
  if (!eval_lock(victim, dest, Tport_Lock))
    return 0;

  if (Toggles(dest) & ROOM_JUMP_OK)
    return 1;

  return 0;
}

int
tport_control_ok(player, victim, loc)
    dbref player;
    dbref victim;
    dbref loc;
{
  /* can player legitimately move victim from loc */

  if (God(victim) && !God(player))
    return 0;

  if (Tel_Anything(player))
    return 1;

  if (controls(player, victim))
    return 1;

  /* mortals can't @tel priv'ed players just on basis of location ownership */

  if (controls(player, loc) && (!Hasprivs(victim) || Owns(player, victim)))
    return 1;

  return 0;
}

void
do_teleport(player, arg1, arg2)
    dbref player;
    const char *arg1;
    const char *arg2;
{
  dbref victim;
  dbref destination;
  dbref loc;
  const char *to;
  dbref absroom;		/* "absolute room", for NO_TEL check */
  int rec = 0;			/* recursion counter */

  /* get victim, destination */
  if (*arg2 == '\0') {
    victim = player;
    to = arg1;
  } else {
    if ((victim = noisy_match_result(player, arg1, NOTYPE, MAT_OBJECTS)) == NOTHING) {
      return;
    }
    to = arg2;
  }

  if (IsRoom(victim)) {
    notify(player, "You can't teleport rooms.");
    return;
  }
  if (IsGarbage(victim)) {
    notify(player, "Garbage belongs in the garbage dump.");
    return;
  }
  /* get destination */

  if (!strcasecmp(to, "home")) {
    /* If the object is @tel'ing itself home, treat it the way we'd  
     * treat a 'home' command 
     */
    if (player == victim) {
      if (Fixed(victim))
	notify(player, "You can't do that IC!");
      else
	safe_tel(victim, HOME);
      return;
    } else
      destination = Home(victim);
  } else {
    destination = match_result(player, to, TYPE_PLAYER, MAT_EVERYTHING);
  }

  switch (destination) {
  case NOTHING:
    notify(player, "No match.");
    break;
  case AMBIGUOUS:
    notify(player, "I don't know which destination you mean!");
    break;
  case HOME:
    destination = Home(victim);
    /* FALL THROUGH */
  default:
    /* check victim, destination types, teleport if ok */
    if (!GoodObject(destination)) {
      notify(player, "Bad destination.");
      return;
    }
    if (recursive_member(destination, victim, 0) || (victim == destination)) {
      notify(player, "Bad destination.");
      return;
    }
    if (!Tel_Anywhere(player) && IsPlayer(victim) &&
	IsPlayer(destination)) {
      notify(player, "Bad destination.");
      return;
    }
    if (IsExit(victim)) {
      /* Teleporting an exit means moving its source */
      if (!IsRoom(destination)) {
	notify(player, "Exits can only be teleported to other rooms.");
	return;
      }
      if (Going(destination)) {
	notify(player, "You can't move an exit to someplace that's crumbling.");
	return;
      }
      if (!GoodObject(Home(victim)))
	loc = find_entrance(victim);
      else
	loc = Home(victim);
      /* Unlike normal teleport, you must control the destination 
       * or have the open_anywhere power
       */
      if (!(tport_control_ok(player, victim, loc) &&
	    (controls(player,destination) || Open_Anywhere(player)))) {
	notify(player, "Permission denied.");
	return;
      }
      /* Remove it from its old room */
      Exits(loc) = remove_first(Exits(loc), victim);
      /* Put it into its new room */
      Source(victim) = destination;
      PUSH(victim, Exits(destination));
      notify(player, "Teleported.");
      return;
    }
    loc = Location(victim);

    /* if royal or wiz and destination is player, tel to location */
    if (IsPlayer(destination) &&
	Tel_Anywhere(player) && IsPlayer(victim)) {
      if (loc != Location(destination))
	did_it(victim, victim, NULL, NULL, "OXTPORT", NULL, NULL, loc);
      safe_tel(victim, Location(destination));
      if (loc != Location(destination))
	did_it(victim, victim, "TPORT", NULL, "OTPORT", NULL, "ATPORT",
	       Location(destination));
      return;
    }
    /* check needed for NOTHING. Especially important for unlinked exits */
    if ((absroom = Location(victim)) == NOTHING) {
      notify(victim, "You're in the Void. This is not a good thing.");
      if (Home(victim) == absroom)
	Home(victim) = PLAYER_START;
      do_move(victim, "home", 0);
      return;
    } else {
      /* valid location, perform other checks */

      /* if player is inside himself, send him home */
      if (absroom == victim) {
	notify(player, "What are you doing inside of yourself?");
	if (Home(victim) == absroom)
	  Home(victim) = PLAYER_START;
	do_move(victim, "home", 0);
	return;
      }
      /* find the "absolute" room */
      while (GoodObject(absroom) && !IsRoom(absroom)
	     && (rec <= 15)) {
	absroom = Location(absroom);
	rec++;
      }

      if (!GoodObject(absroom)) {
	notify(victim, "You're in the void - sending you home.");
	if (Home(victim) == Location(victim))
	  Home(victim) = PLAYER_START;
	do_move(victim, "home", 0);
	return;
      }
      /* if there are a lot of containers, send him home */
      if (rec > 15) {
	notify(victim, "You're in too many containers.");
	if (Home(victim) == Location(victim))
	  Home(victim) = PLAYER_START;
	do_move(victim, "home", 0);
	return;
      }
      /* note that we check the NO_TEL status of the victim rather
       * than the player that issued the command. This prevents someone
       * in a NO_TEL room from having one of his objects @tel him out.
       * The control check, however, is detemined by command-giving
       * player. */

      /* now check to see if the absolute room is set NO_TEL */
      if (NoTel(absroom)
	  && !controls(player, absroom)
	  && !Tel_Anywhere(player)) {
	notify(player, "Teleports are not allowed in this room.");
	return;
      }
      /* Now check the Z_TEL status of the victim's room.
       * Just like NO_TEL above, except that if the room (or its
       * Zone Master Room, if any, is Z_TEL,
       * the destination must also be a room in the same zone
       */
      if (GoodObject(Zone(absroom)) &&
	  (ZTel(absroom) || ZTel(Zone(absroom)))
	  && !controls(player, absroom)
	  && !Tel_Anywhere(player)
	  && (Zone(absroom) != Zone(destination))) {
	notify(player, "You may not teleport out of the zone from this room.");
	return;
      }
    }

    if (!IsExit(destination)) {
      if (tport_control_ok(player, victim, Location(victim)) &&
	  tport_dest_ok(player, victim, destination)
	  && (Tel_Anything(player) ||
	      (Tel_Anywhere(player) && (player == victim)) ||
	      (destination == Owner(victim)) ||
	      (!Fixed(Owner(victim)) && !Fixed(player)))) {
	if (loc != destination)
	  did_it(victim, victim, NULL, NULL, "OXTPORT", NULL, NULL, loc);
	safe_tel(victim, destination);
	if (loc != destination)
	  did_it(victim, victim, "TPORT", NULL, "OTPORT", NULL, "ATPORT",
		 destination);
	if ((victim != player) && !(Puppet(victim) &&
				    (Owner(victim) == Owner(player))))
	  notify(player, "Teleported.");
	return;
      }
      /* we can't do it */
      did_it(player, destination, "EFAIL", "Permission denied.",
	     "OEFAIL", NULL, "AEFAIL", Location(player));
      return;
    } else {
      /* attempted teleport to an exit */
      if (controls(player, victim) ||
	  controls(player, Location(victim)))
	do_move(victim, to, 0);
      else
	notify(victim,
	       tprintf("%s tries to impose his will on you and fails.",
		       Name(player)));
    }
  }
}

void
do_force(player, what, command)
    dbref player;
    const char *what;
    char *command;
{
  dbref victim;
  int j;

  if ((victim = match_controlled(player, what)) == NOTHING) {
    notify(player, "Sorry.");
    return;
  }
  if (!Mobile(victim)) {
    notify(player, "You can only force players and things.");
    return;
  }
  if (options.log_forces) {
    if (Wizard(player)) {
      /* Log forces by wizards */
      if (Owner(victim) != Owner(player))
	do_log(LT_WIZ, player, victim, "** FORCE: %s", command);
      else
	do_log(LT_WIZ, player, victim, "FORCE: %s", command);
    } else if (Wizard(Owner(victim))) {
      /* Log forces of wizards */
      do_log(LT_WIZ, player, victim, "** FORCE WIZ-OWNED: %s", command);
    }
  }
  if (God(victim) && !God(player)) {
    notify(player, "You can't force God!");
    return;
  }
  /* force victim to do command */
  for (j = 0; j < 10; j++) {
    wnxt[j] = wenv[j];
    rnxt[j] = renv[j];
  }
  parse_que(victim, command, player);
}

int
force_by_number(player, command)
    dbref player;
    char *command;
{
  /* the command handler has given us something of the format
   * <#dbref> <command>. Split it up and pass it to @force handler.
   * We can hack it up in-place, since we won't need it the input again.
   */

  char *s;

  for (s = command; *s && !isspace(*s); s++) ;
  if (!*s)
    return (0);
  *s++ = '\0';

  do_force(player, command, s);
  return (1);
}

extern struct db_stat_info *
get_stats(player, owner)
    dbref player;
    dbref owner;
{
  dbref i;
  static struct db_stat_info si;
  si.total = si.rooms = si.exits = si.things = si.players = si.garbage = 0;
  for (i = 0; i < db_top; i++) {
    if (owner == ANY_OWNER || owner == Owner(i)) {
      si.total++;
      if (IsGarbage(i)) {
	si.garbage++;
      } else {
	switch (Typeof(i)) {
	case TYPE_ROOM:
	  si.rooms++;
	  break;
	case TYPE_EXIT:
	  si.exits++;
	  break;
	case TYPE_THING:
	  si.things++;
	  break;
	case TYPE_PLAYER:
	  si.players++;
	  break;
	default:
	  break;
	}
      }
    }
  }
  return &si;
}

void
do_stats(player, name)
    dbref player;
    const char *name;
{
  struct db_stat_info *si;
  dbref owner;

  if (*name == '\0')
    owner = ANY_OWNER;
  else if (*name == '#') {
    owner = atoi(&name[1]);
    if (!GoodObject(owner))
      owner = NOTHING;
    else if (!IsPlayer(owner))
      owner = NOTHING;
  } else if (strcasecmp(name, "me") == 0)
    owner = player;
  else
    owner = lookup_player(name);
  if (owner == NOTHING) {
    notify(player, tprintf("%s: No such player.", name));
    return;
  }
  if (!Search_All(player)) {
    if (owner != ANY_OWNER && owner != player) {
      notify(player, "You need a search warrant to do that!");
      return;
    }
  }
  si = get_stats(player, owner);
  if (owner == ANY_OWNER) {
    notify(player, tprintf("%d objects = %d rooms, %d exits, %d things, %d players, %d garbage.",
			   si->total, si->rooms, si->exits, si->things, si->players, si->garbage));
  } else {
    notify(player, tprintf("%d objects = %d rooms, %d exits, %d things, %d players.",
			   si->total - si->garbage, si->rooms, si->exits, si->things, si->players));
  }
#ifdef MEM_CHECK
  if (God(player))
    list_mem_check(player);
#endif
#ifdef USE_SMALLOC
#ifdef SLOW_STATISTICS
  if (God(player))
    dump_malloc_data(player);
#endif
#endif
#ifdef WIN32
  if (Wizard(player))
    Win32stats(player);
#endif
#if defined(DEBUG) && defined(USE_SMALLOC)
  if (Hasprivs(player))
    notify(player, tprintf("%d bytes memory used.", memused()));
  if (God(player))
    verify_sfltable(player);
#endif
}

void
do_newpassword(player, name, password)
    dbref player;
    const char *name;
    const char *password;
{
  dbref victim;
  if (!Wizard(player)) {
    notify(player, "Your delusions of grandeur have been duly noted.");
    return;
  } else if ((victim = lookup_player(name)) == NOTHING) {
    notify(player, "No such player.");
  } else if (*password != '\0' && !ok_password(password)) {
    /* Wiz can set null passwords, but not bad passwords */
    notify(player, "Bad password.");
  } else if (God(victim) && !God(player)) {
    notify(player, "You cannot change that player's password.");
  } else {
    /* it's ok, do it */
    atr_add(victim, "XYXXY", mush_crypt(password), GOD, NOTHING);
    notify(player, "Password changed.");
    notify(victim, tprintf("Your password has been changed by %s.",
			   Name(player)));
    do_log(LT_WIZ, player, victim, "*** NEWPASSWORD ***");
  }
}

void
do_boot(player, name, flag)
    dbref player;
    const char *name;
    int flag;


				/* 0, normal boot. 1, descriptor boot */
				/* 2, self boot */
{
  dbref victim;
#ifdef CONCENTRATOR
  int cport, port;
#endif
  DESC *d = NULL;

  victim = NOTHING;
  switch (flag) {
  case 2:
    /* self boot */
    victim = player;
    break;
  case 1:
    /* boot by descriptor */
#ifdef CONCENTRATOR
    if (!parse_conc_descriptor(name, &cport, &port)) {
      notify(player, "Invalid descriptor specification (use d,d).");
      return;
    }
    victim = find_player_by_desc(cport, port);
#else
    victim = find_player_by_desc(atoi(name));
#endif
    if (victim == NOTHING) {
      notify(player, "There is no one connected on that descriptor.");
      return;
    }
    break;
  case 0:
    /* boot by name */
    if ((victim = noisy_match_result(player, name, TYPE_PLAYER, MAT_LIMITED | MAT_ME)) == NOTHING) {
      notify(player, "No such connected player.");
      return;
    }
    if (victim == player)
      flag = 2;
    break;
  }

  if ((victim != player) && !Can_Boot(player)) {
    notify(player, "You can't boot other people!");
    return;
  }
  if (God(victim) && !God(player)) {
    notify(player, "You're good.  That's spelled with two 'o's.");
    return;
  }
  /* look up descriptor */
  switch (flag) {
  case 0:
    d = player_desc(victim);
    break;
  case 1:
#ifdef CONCENTRATOR
    d = port_desc(cport, port);
#else
    d = port_desc(atoi(name));
#endif
    break;
  case 2:
    d = inactive_desc(victim);
    break;
  }

  /* check for more errors */
  if (!d) {
    if (flag == 2)
      notify(player, "None of your connections has been idle.");
    else
      notify(player, "That player isn't connected!");
  } else if (d == cdesc) {
    notify(player, tprintf("Try %s instead.", QUIT_COMMAND));
  } else {
    do_log(LT_WIZ, player, victim, "*** BOOT ***");
    if (flag == 2)
      notify(player, "You boot an idle self.");
    else {
      notify(player, tprintf("You booted %s off!", Name(victim)));
      notify(victim, "You are politely shown to the door.");
    }
    boot_desc(d);
  }
}

#ifdef CONCENTRATOR
int
parse_conc_descriptor(str, cport, port)
    char *str;
    int *cport;
    int *port;
{
  char *p, *q;

  p = str;
  q = str;
  while (p && *p && (*p != ','))
    p++;
  if (!p || !*p)
    return 0;
  *p = 0;
  if (!(p + 1) || !*(p + 1))
    return 0;
  *cport = atoi(q);
  *port = atoi(p + 1);
  return 1;
}
#endif				/* CONCENTRATOR */

void
do_chownall(player, name, target)
    dbref player;
    const char *name;
    const char *target;
{
  int i;
  dbref victim;
  dbref n_target;
  int count = 0;

  if (!Wizard(player)) {
    notify(player, "Try asking them first!");
    return;
  }
  if ((victim = noisy_match_result(player, name, TYPE_PLAYER, MAT_LIMITED)) == NOTHING)
    return;

  if (!target || !*target) {
    n_target = player;
  } else {
    if ((n_target = noisy_match_result(player, target, TYPE_PLAYER, MAT_LIMITED)) == NOTHING)
      return;
  }

  for (i = 0; i < db_top; i++) {
    if ((Owner(i) == victim) && (!IsPlayer(i))) {
      Owner(i) = n_target;
      count++;
    }
  }

#ifdef QUOTA
  /* change quota (this command is wiz only and we can assume that
   * we intend for the recipient to get all the objects, so we
   * don't do a quota check earlier.
   */
  change_quota(victim, count);
  change_quota(n_target, -count);
#endif

  notify(player, tprintf("Ownership changed for %d objects.", count));
}

void
do_chzoneall(player, name, target)
    dbref player;
    const char *name;
    const char *target;
{
  int i;
  dbref victim;
  dbref zone;
  int count = 0;

  if (!Wizard(player)) {
    notify(player, "You do not have the power to change reality.");
    return;
  }
  if ((victim = noisy_match_result(player, name, TYPE_PLAYER, MAT_LIMITED)) == NOTHING)
    return;

  if (!target || !*target) {
    notify(player, "No zone specified.");
    return;
  }
  if (!strcasecmp(target, "none"))
    zone = NOTHING;
  else {
    switch (zone = match_result(player, target, TYPE_THING, MAT_LIMITED)) {
    case NOTHING:
      notify(player, "I can't seem to find that.");
      return;
    case AMBIGUOUS:
      notify(player, "I don't know which one you mean!");
      return;
    }
    if (!IsThing(zone)) {
      notify(player, "Invalid zone object type.");
      return;
    }
  }

  for (i = 0; i < db_top; i++) {
    if (Owner(i) == victim) {
      Zone(i) = zone;
      count++;
    }
  }
  notify(player, tprintf("Zone changed for %d objects.", count));
}

void
do_enable(player, param, state)
    dbref player;
    const char *param;
    int state;			/* enable is 1, disable is 0 */
{
  const char *name;

  if (!Wizard(player)) {
    notify(player, "Unable to authenticate you, sorry.");
    return;
  }
  if (!strcasecmp(param, "pueblo")) {
    name = "PUEBLO";
    options.support_pueblo = state;
  } else if (!strcasecmp(param, "logins")) {
    name = "LOGINS";
    options.login_allow = state;
  } else if (!strcasecmp(param, "daytime")) {
    name = "DAYTIME";
    options.daytime = state;
  } else if (!strcasecmp(param, "command_log")) {
    name = "LOG COMMANDS";
    options.log_commands = state;
  } else if (!strcasecmp(param, "huh_log")) {
    name = "LOG HUHS";
    options.log_huhs = state;
  } else if (!strcasecmp(param, "force_log")) {
    name = "LOG FORCES";
    options.log_forces = state;
  } else if (!strcasecmp(param, "wall_log")) {
    name = "LOG WIZWALLS";
    options.log_walls = state;
  } else if (!strcasecmp(param, "guests")) {
    name = "GUEST LOGINS";
    options.guest_allow = state;
  } else if (!strcasecmp(param, "creation")) {
    name = "PLAYER CREATION";
    options.create_allow = state;
  } else {
    notify(player, "Unknown parameter.");
    return;
  }

  if (state == 0)
    notify(player, "Disabled.");
  else
    notify(player, "Enabled.");

  do_log(LT_WIZ, player, NOTHING, "%s %s",
	 name, (state) ? "ENABLED" : "DISABLED");
}

/*-----------------------------------------------------------------------
 * Nasty management: @kick, examine/debug, and @fixdb
 */

void
do_kick(player, num)
    dbref player;
    const char *num;
{
  /* executes <num> commands off the queue immediately */

  int n;

  if (!Wizard(player)) {
    notify(player, "Permission denied.");
    return;
  }
  if (!num || !*num) {
    notify(player, "How many commands do you want to execute?");
    return;
  }
  n = atoi(num);

  if (n <= 0) {
    notify(player, "Number out of range.");
    return;
  }
  n = do_top(n);

  notify(player, tprintf("%d commands executed.", n));

}

void
do_fixdb(player, name, val, sw_opt)
    dbref player;
    const char *name;
    const char *val;
    int sw_opt;
{
  dbref thing;
  dbref newobj;

  /* do all the checks */
  if (!Wizard(player)) {
    notify(player, "Reality is not yours to warp.");
    return;
  }
  if (!name || !*name) {
    notify(player, "You must specify an object.");
    return;
  }
  if (!val || !*val) {
    notify(player, "You must specify a value.");
    return;
  }
  if (!is_strict_number(val)) {
    notify(player, "The value must be an integer.");
    return;
  }
  /* find the object */
  thing = noisy_match_result(player, name, NOTYPE, MAT_EVERYTHING);
  if (thing == NOTHING)
    return;

  /* change the pointers. Do no error checking. */

  newobj = atoi(val);

  switch (sw_opt) {
  case 0:
    Location(thing) = newobj;
    break;
  case 1:
    Contents(thing) = newobj;
    break;
  case 2:
    if (IsExit(thing)) {
      Source(thing) = newobj;
    }
    else if (IsRoom(thing)) {
      Exits(thing) = newobj;
    }
    else {
      Home(thing) = newobj;
    }
    break;
  case 3:
    Next(thing) = newobj;
    break;
  }

  notify(player, "Fixed.");
}

void
do_debug_examine(player, name)
    dbref player;
    const char *name;
{
#ifdef USE_MAILER
  struct mail *mp;
#endif
  dbref thing;

  if (!Hasprivs(player)) {
    notify(player, "Permission denied.");
    return;
  }
  /* find it */
  thing = noisy_match_result(player, name, NOTYPE, MAT_EVERYTHING);
  if (!GoodObject(thing))
    return;

  notify(player, object_header(player, thing));
  notify(player, tprintf("Flags value: 0x%08x", Flags(thing)));
  notify(player, tprintf("Toggles value: 0x%08x", Toggles(thing)));
  notify(player, tprintf("Powers value: 0x%08x", Powers(thing)));

  notify(player, tprintf("Next: %d", Next(thing)));
  notify(player, tprintf("Contents: %d", Contents(thing)));

  switch (Typeof(thing)) {
  case TYPE_PLAYER:
#ifdef USE_MAILER
    mp = desc_mail(thing);
    notify(player, tprintf("First mail sender: %d",
			   mp ? mp->from : NOTHING));
#endif
  case TYPE_THING:
    notify(player, tprintf("Location: %d", Location(thing)));
    notify(player, tprintf("Home: %d", Home(thing)));
    break;
  case TYPE_EXIT:
    notify(player, tprintf("Destination: %d", Location(thing)));
    notify(player, tprintf("Source: %d", Source(thing)));
    break;
  case TYPE_ROOM:
    notify(player, tprintf("Drop-to: %d", Location(thing)));
    notify(player, tprintf("Exits: %d", Exits(thing)));
    break;
  case TYPE_GARBAGE:
    break;
  default:
    notify(player, "Bad object type.");
  }
}

/*-------------------------------------------------------------------------
 * Powers stuff
 */


#ifdef WIN32
#pragma warning( disable : 4761)	/* Disable bogus conversion warning */
#endif
/* ARGSUSED */
FUNCTION(fun_haspower)
{
  dbref it;
  object_flag_type pwr;

  it = match_thing(executor, args[0]);
  if (it == NOTHING) {
    safe_str("#-1", buff, bp);
    return;
  }
  if (HASPOWER_RESTRICTED)
    if (!Can_Examine(executor, it)) {
      notify(executor,
	   "We could let you see that, but then we'd have to kill you.");
      safe_str("#-1", buff, bp);
      return;
    }
  pwr = find_power(args[1]);
  if (pwr == -1)
    safe_str("#-1 NO SUCH POWER", buff, bp);
  else
    safe_chr((Powers(it) & pwr) ? '1' : '0', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_powers)
{
  dbref it;

  it = match_thing(executor, args[0]);
  if (it == NOTHING) {
    safe_str("#-1", buff, bp);
    return;
  }
  if (HASPOWER_RESTRICTED)
    if (!Can_Examine(executor, it)) {
      notify(executor,
	   "We could let you see that, but then we'd have to kill you.");
      safe_str("#-1", buff, bp);
      return;
    }
  safe_str(power_description(it), buff, bp);
}

#ifdef WIN32
#pragma warning( default : 4761)	/* Re-enable conversion warning */
#endif

void
do_power(player, name, power)
    dbref player;
    const char *name;
    const char *power;
{
  object_flag_type pwr;
  char *s;
  dbref thing;

  if (!Wizard(player)) {
    notify(player, "Only wizards may grant powers.");
    return;
  }
  if ((thing = noisy_match_result(player, name, NOTYPE, MAT_EVERYTHING)) == NOTHING)
    return;
#ifdef ONLINE_REG
  if (Unregistered(thing)) {
    notify(player, "You can't grant powers to unregistered players.");
    return;
  }
#endif
  if (God(thing) && !God(player)) {
    notify(player, "God is already all-powerful.");
    return;
  }
  /* move past the not token if there is one */
  for (s = (char *) power; *s && ((*s == NOT_TOKEN) || isspace(*s)); s++) ;

  if (*s == '\0') {
    notify(player, "You must specify a power.");
    return;
  }
  pwr = find_power(s);
  if (pwr == -1) {
    notify(player, "That is not a power.");
    return;
  }
  if (*power == NOT_TOKEN) {
    Powers(thing) &= ~pwr;
    if (!Quiet(player) && !Quiet(thing))
      notify(player, tprintf("%s - power removed.", Name(thing)));
    do_log(LT_WIZ, player, thing, "Power Removed: %s", power);
  } else {
    if (Hasprivs(thing) && (pwr == IS_GUEST)) {
      notify(player, "You can't make admin into guests.");
      return;
    }
    Powers(thing) |= pwr;
    if (!Quiet(player) && !Quiet(thing))
      notify(player, tprintf("%s - power granted.", Name(thing)));
    do_log(LT_WIZ, player, thing, "Power Granted: %s", power);
  }
}

/*----------------------------------------------------------------------------
 * Search functions
 */


void
do_search(player, arg1, arg3)
    dbref player;
    const char *arg1;
    char **arg3;
{
  int flag, is_wizard;
  char *arg2, *restrict_name, *ebuf1, *bp;
  char const *ebuf2;
  dbref thing, from, to;
  int destitute = 1;
  int eval_search = 0;
  dbref restrict_owner = NOTHING;
  dbref restrict_zone = NOTHING;
  dbref restrict_parent = NOTHING;
  object_flag_type flag_mask, toggle_mask, restrict_type, power_mask;
  char tbuf1[BUFFER_LEN];
  int rcount, ecount, pcount, ocount;
  struct dblist *found = NULL, *ptr = NULL, *rlist = NULL, *elist = NULL,
  *olist = NULL, *plist = NULL;
  int bot = 0;
  int top = db_top - 1;

  if (options.daytime) {
    notify(player, "Sorry, that command has been temporarily disabled.");
    return;
  }
  /* parse first argument into two */
  if (!arg1 || *arg1 == '\0')
    arg1 = "me";
  arg2 = (char *) index(arg1, ' ');
  if (arg2 != NULL)
    *arg2++ = '\0';		/* arg1, arg2, arg3 */
  else {
    if (!arg3[1] || !*arg3[1])
      arg2 = (char *) "";	/* arg1 */
    else {
      arg2 = (char *) arg1;	/* arg2, arg3 */
      arg1 = (char *) "";
    }
  }

  is_wizard = Search_All(player) || See_All(player);

  /* set limits on who we search */
  if (*arg1 == '\0') {
    restrict_owner = is_wizard ? ANY_OWNER : player;
  } else if (arg1[0] == '#') {
    restrict_owner = atoi(&arg1[1]);
    if (!GoodObject(restrict_owner))
      restrict_owner = NOTHING;
    else if (!IsPlayer(restrict_owner))
      restrict_owner = NOTHING;
  } else if (strcmp(arg1, "me") == 0) {
    restrict_owner = player;
  } else {
    restrict_owner = lookup_player(arg1);
  }

  if (restrict_owner == NOTHING) {
    notify(player, tprintf("%s: No such player.", arg1));
    return;
  }
  /* set limits on what we search for */
  flag = flag_mask = power_mask = toggle_mask = 0;
  restrict_name = NULL;
  restrict_type = NOTYPE;
  switch (arg2[0]) {
  case '\0':
    /* the no class requested class  :)  */
    break;
  case 'e':
  case 'E':
    if (string_prefix("exits", arg2)) {
      restrict_name = (char *) arg3[1];
      restrict_type = TYPE_EXIT;
    } else if (string_prefix("eval", arg2)) {
      eval_search = 1;
    } else if (string_prefix("eplayer", arg2)) {
      eval_search = 1;
      restrict_type = TYPE_PLAYER;
    } else if (string_prefix("eroom", arg2)) {
      eval_search = 1;
      restrict_type = TYPE_ROOM;
    } else if (string_prefix("eexit", arg2)) {
      eval_search = 1;
      restrict_type = TYPE_EXIT;
    } else if (string_prefix("eobject", arg2)) {
      eval_search = 1;
      restrict_type = TYPE_THING;
    } else {
      flag = 1;
    }
    break;
  case 'f':
  case 'F':
    if (string_prefix("flags", arg2)) {
      /*
       * convert_flags ignores previous values of flag_mask and
       * restrict_type while setting them
       */
      if (arg3[1] && *arg3[1] &&
	  !convert_flags(player, arg3[1], &flag_mask, &toggle_mask,
			 &restrict_type))
	return;
    } else
      flag = 1;
    break;
  case 'n':
  case 'N':
    if (string_prefix("name", arg2))
      restrict_name = (char *) arg3[1];
    else
      flag = 1;
    break;
  case 'o':
  case 'O':
    if (string_prefix("objects", arg2)) {
      restrict_name = (char *) arg3[1];
      restrict_type = TYPE_THING;
    } else
      flag = 1;
    break;
  case 'p':
  case 'P':
    switch (arg2[1]) {
    case 'a':
    case 'A':
      if (string_prefix("parent", arg2)) {
	if (arg3[1] && *arg3[1]) {
	  restrict_parent = match_result(player, arg3[1], NOTYPE, MAT_ABSOLUTE);
	}
	if (restrict_parent == NOTHING) {
	  notify(player, "Unknown parent.");
	  return;
	}
      } else
	flag = 1;
      break;
    case 'l':
    case 'L':
      if (string_prefix("players", arg2)) {
	restrict_name = (char *) arg3[1];
	if (!arg1 || !*arg1)
	  restrict_owner = ANY_OWNER;
	restrict_type = TYPE_PLAYER;
      } else
	flag = 1;
      break;
    case 'o':
    case 'O':
      if (string_prefix("powers", arg2)) {
	power_mask = find_power(arg3[1]);
	if (power_mask == -1) {
	  notify(player, "No such power to search for.");
	  return;
	}
      } else
	flag = 1;
      break;
    default:
      flag = 1;
    }
    break;
  case 'r':
  case 'R':
    if (string_prefix("rooms", arg2)) {
      restrict_name = (char *) arg3[1];
      restrict_type = TYPE_ROOM;
    } else
      flag = 1;
    break;
  case 't':
  case 'T':
    switch (arg2[1]) {
    case 'h':
    case 'H':
      if (string_prefix("things", arg2)) {
	restrict_name = (char *) arg3[1];
	restrict_type = TYPE_THING;
      } else
	flag = 1;
      break;
    case 'y':
    case 'Y':
      if (string_prefix("type", arg2)) {
	if (!arg3[1] || !*arg3[1])
	  break;
	if (string_prefix("rooms", arg3[1]))
	  restrict_type = TYPE_ROOM;
	else if (string_prefix("exits", arg3[1]))
	  restrict_type = TYPE_EXIT;
	else if (string_prefix("objects", arg3[1]))
	  restrict_type = TYPE_THING;
	else if (string_prefix("things", arg3[1]))
	  restrict_type = TYPE_THING;
	else if (string_prefix("players", arg3[1])) {
	  if (!arg1 || !*arg1)
	    restrict_owner = ANY_OWNER;
	  restrict_type = TYPE_PLAYER;
	} else {
	  notify(player, tprintf("%s: unknown type.", arg3[1]));
	  return;
	}
      } else
	flag = 1;
      break;
    default:
      flag = 1;
      break;
    }
    break;
  case 'z':
  case 'Z':
    if (string_prefix("zone", arg2)) {
      if (arg3[1] && *arg3[1]) {
	restrict_zone = match_result(player, arg3[1], TYPE_THING, MAT_ABSOLUTE);
      }
      if (restrict_zone == NOTHING) {
	notify(player, "Unknown zone.");
	return;
      }
    } else
      flag = 1;
    break;
  default:
    flag = 1;
  }
  if (flag) {
    notify(player, tprintf("%s: unknown class.", arg2));
    return;
  }
  /* make sure player is authorized to do requested search */
  if (!is_wizard && restrict_type != TYPE_PLAYER &&
      (restrict_owner == ANY_OWNER || restrict_owner != player)) {
    notify(player, "You need a search warrant to do that!");
    return;
  }
  /* find range */
  if (arg3[2] && *arg3[2]) {
    bot = parse_integer(arg3[2]);
    if (!GoodObject(bot)) {
      notify(player, "Invalid range argument");
      return;
    }
  }
  if (arg3[3] && *arg3[3]) {
    top = parse_integer(arg3[3]);
    if (!GoodObject(top)) {
      notify(player, "Invalid range argument");
      return;
    }
  }
  /* make sure player has money to do the search */
  if (!payfor(player, FIND_COST)) {
    notify(player, tprintf("Searches cost %d %s.", FIND_COST,
			   ((FIND_COST == 1) ? MONEY : MONIES)));
    return;
  }
  /* the search loop here */
  flag = 1;
  for (thing = bot; thing <= top; thing++) {
    if (IsGarbage(thing))
      continue;
    if ((restrict_owner != ANY_OWNER) && (restrict_owner != Owner(thing)))
      continue;
    if (!eval_search) {
      if (restrict_type != NOTYPE && restrict_type != Typeof(thing))
	continue;
      if ((Flags(thing) & flag_mask) != flag_mask)
	continue;
      if ((Toggles(thing) & toggle_mask) != toggle_mask)
	continue;
      if ((Powers(thing) & power_mask) != power_mask)
	continue;
      if ((restrict_name != NULL) &&
	  !string_match(Name(thing), restrict_name))
	continue;
      if ((restrict_parent != NOTHING) &&
	  (restrict_parent != Parent(thing)))
	continue;
      if ((restrict_zone != NOTHING) &&
	  (restrict_zone != Zone(thing)))
	continue;
    } else {
      if ((restrict_type != NOTYPE) && (restrict_type != Typeof(thing)))
        continue;

      ebuf1 = replace_string("##", tprintf("#%d", thing), arg3[1]);
      ebuf2 = ebuf1;
      bp = tbuf1;
      process_expression(tbuf1, &bp, &ebuf2, player, player, player,
			 PE_DEFAULT, PT_DEFAULT, NULL);
      mush_free((Malloc_t) ebuf1, "replace_string.buff");
      *bp = '\0';
      if (*tbuf1 != '1' || tbuf1[1])
	continue;
    }
    switch (Typeof(thing)) {
    case TYPE_PLAYER:
      found = plist;
      break;
    case TYPE_EXIT:
      found = elist;
      break;
    case TYPE_THING:
      found = olist;
      break;
    case TYPE_ROOM:
      found = rlist;
      break;
    default:
      continue;
    }
    if (!found) {
      flag = 0;
      destitute = 0;
      found = listcreate(thing);
    } else
      listadd(found, thing);

    switch (Typeof(thing)) {
    case TYPE_PLAYER:
      plist = found;
      break;
    case TYPE_EXIT:
      elist = found;
      break;
    case TYPE_THING:
      olist = found;
      break;
    case TYPE_ROOM:
      rlist = found;
      break;
    default:
      break;
    }
  }
  /* if nothing found matching search criteria */
  if (destitute) {
    notify(player, "Nothing found.");
    return;
  }
  /* Walk down the list and print */
  rcount = ecount = pcount = ocount = 0;
  if (restrict_type == TYPE_ROOM || restrict_type == NOTYPE) {
    notify(player, "\nROOMS:");
    for (ptr = rlist; ptr; ptr = ptr->next) {
      sprintf(tbuf1, "%s [owner: ", object_header(player, ptr->obj));
      strcat(tbuf1, tprintf("%s]",
			    object_header(player, Owner(ptr->obj))));
      notify(player, tbuf1);
      rcount++;
    }
  }
  if (restrict_type == TYPE_EXIT || restrict_type == NOTYPE) {
    notify(player, "\nEXITS:");
    for (ptr = elist; ptr; ptr = ptr->next) {
      if (Source(ptr->obj) == NOTHING)
	from = find_entrance(thing);
      else
	from = Source(ptr->obj);
      to = Destination(ptr->obj);
      sprintf(tbuf1, "%s [from ", object_header(player, ptr->obj));
      strcat(tbuf1, tprintf("%s to ", (from == NOTHING) ? "NOWHERE" :
			    object_header(player, from)));
      strcat(tbuf1, tprintf("%s]", (to == NOTHING) ? "NOWHERE" :
			    object_header(player, to)));
      notify(player, tbuf1);
      ecount++;
    }
  }
  if (restrict_type == TYPE_THING || restrict_type == NOTYPE) {
    notify(player, "\nOBJECTS:");
    for (ptr = olist; ptr; ptr = ptr->next) {
      sprintf(tbuf1, "%s [owner: ", object_header(player, ptr->obj));
      strcat(tbuf1, tprintf("%s]",
			    object_header(player, Owner(ptr->obj))));
      notify(player, tbuf1);
      ocount++;
    }
  }
  if ((restrict_type == TYPE_PLAYER) || (restrict_type == NOTYPE)) {
    notify(player, "\nPLAYERS:");
    for (ptr = plist; ptr; ptr = ptr->next) {
      strcpy(tbuf1, object_header(player, ptr->obj));
      if (is_wizard) {
	strcat(tbuf1, " [location: ");
	strcat(tbuf1, object_header(player, Location(ptr->obj)));
	strcat(tbuf1, "]");
      }
      notify(player, tbuf1);
      pcount++;
    }
  }
  notify(player, "----------  Search Done  ----------");
  notify(player,
    tprintf("Totals: Rooms...%d  Exits...%d  Objects...%d  Players...%d",
	    rcount, ecount, ocount, pcount));
  if (plist)
    listfree(plist);
  if (rlist)
    listfree(rlist);
  if (olist)
    listfree(olist);
  if (elist)
    listfree(elist);
}

#ifdef WIN32
#pragma warning( disable : 4761)	/* Disable bogus conversion warning */
#endif
/* ARGSUSED */
FUNCTION(fun_hidden)
{
  dbref it = match_thing(executor, args[0]);

  if (!See_All(executor)) {
    notify(executor, "Permission denied.");
    safe_str("#-1", buff, bp);
    return;
  }
  if ((it == NOTHING) || (!IsPlayer(it))) {
    notify(executor, "Couldn't find that player.");
    safe_str("#-1", buff, bp);
    return;
  }
  safe_chr(hidden(it) ? '1' : '0', buff, bp);
  return;
}
#ifdef WIN32
#pragma warning( default : 4761)	/* Re-enable conversion warning */
#endif


/* ARGSUSED */
FUNCTION(fun_lsearch)
{
  /* takes arguments in the form of: player, class, restriction */
  int flag, is_wiz;
  char *who, *class, *res, *restrict_name, *ebuf1, *tp;
  char const *ebuf2;
  char tbuf1[BUFFER_LEN];
  object_flag_type flag_mask, toggle_mask, restrict_type, power_mask;
  dbref restrict_owner = NOTHING;
  dbref restrict_zone = NOTHING;
  dbref restrict_parent = NOTHING;
  dbref thing;
  int destitute = 1;
  int eval_search = 0;
  int bot = 0;
  int top = db_top - 1;
  int rev = !strcmp(called_as, "LSEARCHR");

  if (options.daytime) {
    notify(executor, "Function disabled.");
    strcpy(buff, "#-1");
    return;
  }
  is_wiz = Search_All(executor) || See_All(executor);

  who = args[0];
  class = args[1];
  res = args[2];

  if (nargs > 3) {
    bot = parse_integer(args[3]);
    if (!GoodObject(bot)) {
      notify(executor, "Invalid range argument");
      safe_str("#-1", buff, bp);
      return;
    }
  }
  if (nargs > 4) {
    top = parse_integer(args[4]);
    if (!GoodObject(top)) {
      notify(executor, "Invalid range argument");
      safe_str("#-1", buff, bp);
      return;
    }
  }
  /* set limits on who we search */
  if (!strcasecmp(who, "all"))
    restrict_owner = is_wiz ? ANY_OWNER : executor;
  else if (!strcasecmp(who, "me"))
    restrict_owner = executor;
  else
    restrict_owner = lookup_player(who);

  if (restrict_owner == NOTHING) {
    notify(executor, "No such player.");
    safe_str("#-1", buff, bp);
    return;
  }
  /* set limits on what we search for */
  flag = 0;
  flag_mask = 0;
  toggle_mask = 0;
  power_mask = 0;
  restrict_name = NULL;
  restrict_type = NOTYPE;
  switch (class[0]) {
  case 'e':
  case 'E':
    if (string_prefix("exits", class)) {
      restrict_name = (char *) res;
      restrict_type = TYPE_EXIT;
    } else if (string_prefix("eval", class)) {
      eval_search = 1;
    } else if (string_prefix("eplayer", class)) {
      eval_search = 1;
      restrict_type = TYPE_PLAYER;
    } else if (string_prefix("eroom", class)) {
      eval_search = 1;
      restrict_type = TYPE_ROOM;
    } else if (string_prefix("eexit", class)) {
      eval_search = 1;
      restrict_type = TYPE_EXIT;
    } else if (string_prefix("eobject", class)) {
      eval_search = 1;
      restrict_type = TYPE_THING;
    } else
      flag = 1;
    break;
  case 'f':
  case 'F':
    if (string_prefix("flags", class)) {
      if (!convert_flags(executor, res, &flag_mask, &toggle_mask,
			 &restrict_type)) {
	notify(executor, "Unknown flag.");
	safe_str("#-1", buff, bp);
	return;
      }
    } else
      flag = 1;
    break;
  case 'n':
  case 'N':
    if (string_prefix("name", class))
      restrict_name = (char *) res;
    else if (!string_prefix("none", class))
      flag = 1;
    break;
  case 'o':
  case 'O':
    if (string_prefix("objects", class)) {
      restrict_name = (char *) res;
      restrict_type = TYPE_THING;
    } else
      flag = 1;
    break;
  case 'p':
  case 'P':
    switch (class[1]) {
    case 'a':
    case 'A':
      if (string_prefix("parent", class)) {
	restrict_parent = match_result(executor, res, NOTYPE, MAT_ABSOLUTE);
	if (restrict_parent == NOTHING) {
	  notify(executor, "Unknown parent.");
	  safe_str("#-1", buff, bp);
	  return;
	}
      } else
	flag = 1;
      break;
    case 'l':
    case 'L':
      if (string_prefix("players", class)) {
	restrict_name = (char *) res;
	restrict_type = TYPE_PLAYER;
      } else
	flag = 1;
      break;
    case 'o':
    case 'O':
      if (string_prefix("powers", class)) {
	power_mask = find_power(res);
	if (power_mask == -1) {
	  notify(executor, "Unknown power.");
	  safe_str("#-1", buff, bp);
	  return;
	}
      } else
	flag = 1;
      break;
    default:
      flag = 1;
    }
    break;
  case 'r':
  case 'R':
    if (string_prefix("rooms", class)) {
      restrict_name = (char *) res;
      restrict_type = TYPE_ROOM;
    } else
      flag = 1;
    break;
  case 't':
  case 'T':
    if (string_prefix("type", class)) {
      if (!strcasecmp(res, "none"))
	break;
      if (string_prefix("rooms", res))
	restrict_type = TYPE_ROOM;
      else if (string_prefix("exits", res))
	restrict_type = TYPE_EXIT;
      else if (string_prefix("objects", res))
	restrict_type = TYPE_THING;
      else if (string_prefix("players", res))
	restrict_type = TYPE_PLAYER;
      else {
	notify(executor, "Unknown type.");
	strcpy(buff, "#-1");
	return;
      }
    } else
      flag = 1;
    break;
  case 'z':
  case 'Z':
    if (string_prefix("zone", class)) {
      restrict_zone = match_result(executor, res, TYPE_THING, MAT_ABSOLUTE);
      if (restrict_zone == NOTHING) {
	notify(executor, "Unknown zone.");
	strcpy(buff, "#-1");
	return;
      }
    } else
      flag = 1;
    break;
  default:
    flag = 1;
  }
  if (flag) {
    notify(executor, "Unknown type.");
    safe_str("#-1", buff, bp);
    return;
  }
  /* check privs */
  if (!is_wiz && (restrict_type != TYPE_PLAYER) &&
      ((restrict_owner == ANY_OWNER) || (restrict_owner != executor))) {
    notify(executor, "Permission denied.");
    safe_str("#-1", buff, bp);
    return;
  }
  /* check cash */
  if (!payfor(executor, FIND_COST)) {
    notify(executor, "Not enough money to do search.");
    strcpy(buff, "#-1");
    return;
  }
  /* search loop */
  flag = 1;
  for (rev ? (thing = top) : (thing = bot);
       rev ? (thing >= bot) : (thing <= top);
       rev ? (thing--) : (thing++)) {
    if (IsGarbage(thing))
      continue;
    if ((restrict_owner != ANY_OWNER) && (restrict_owner != Owner(thing)))
      continue;
    if (!eval_search) {
      if ((restrict_type != NOTYPE) && (restrict_type != Typeof(thing)))
	continue;
      if ((Flags(thing) & flag_mask) != flag_mask)
	continue;
      if ((Toggles(thing) & toggle_mask) != toggle_mask)
	continue;
      if ((Powers(thing) & power_mask) != power_mask)
	continue;
      if ((restrict_name != NULL) &&
	  (!string_match(Name(thing), restrict_name)))
	continue;
      if ((restrict_parent != NOTHING) &&
	  (restrict_parent != Parent(thing)))
	continue;
      if ((restrict_zone != NOTHING) &&
	  (restrict_zone != Zone(thing)))
	continue;
    } else {
      if ((restrict_type != NOTYPE) && (restrict_type != Typeof(thing)))
        continue;

      ebuf1 = replace_string("##", tprintf("#%d", thing), res);
      ebuf2 = ebuf1;
      tp = tbuf1;
      process_expression(tbuf1, &tp, &ebuf2, executor, caller, enactor,
			 PE_DEFAULT, PT_DEFAULT, pe_info);
      mush_free((Malloc_t) ebuf1, "replace_string.buff");
      *tp = '\0';
      if (*tbuf1 != '1' || tbuf1[1])
	continue;
    }
    if (destitute) {
      safe_str(unparse_dbref(thing), buff, bp);
      destitute = 0;
    } else {
      safe_chr(' ', buff, bp);
      safe_str(unparse_dbref(thing), buff, bp);
    }
  }

  /* nothing found matching search criteria */
  if (destitute) {
    notify(executor, "Nothing found.");
    safe_str("#-1", buff, bp);
    return;
  }
}

#ifdef QUOTA
/* ARGSUSED */
FUNCTION(fun_quota)
{
  int owned;
  /* Tell us player's quota */
  dbref thing;
  dbref who = lookup_player(args[0]);

  if ((who == NOTHING) || !IsPlayer(who)) {
    notify(executor, "Couldn't find that player.");
    safe_str("#-1", buff, bp);
    return;
  }
  if (!(Do_Quotas(executor) || See_All(executor) || (who == executor))) {
    notify(executor, "You can't see someone else's quota!");
    safe_str("#-1", buff, bp);
    return;
  }
  if (NoQuota(who)) {
    /* Unlimited, but return a big number to be sensible */
    safe_str("99999", buff, bp);
    return;
  }
  /* count up all owned objects */
  owned = -1;			/* a player is never included in his own
				 * quota */
  for (thing = 0; thing < db_top; thing++) {
    if (Owner(thing) == who)
      if (!IsGarbage(thing))
	++owned;
  }

  safe_str(unparse_integer(owned + get_current_quota(who)), buff, bp);
  return;
}
#endif

void
do_sitelock(player, site, opts, type)
    dbref player;
    const char *site;
    const char *opts;
    int type;
    /* 0 = registration, 1 = siteban, 2 = names */
{
  FILE *fp, *fptmp;
  char buffer[80];
  char *p;

  if (!Wizard(player)) {
    notify(player, "Your delusions of grandeur have been noted.");
    return;
  }
  if (opts && *opts) {
    int can, cant;
    /* Options form of the command. */
    if (!site || !*site) {
      notify(player, "What site did you want to lock?");
      return;
    }
    can = cant = 0;
    if (!parse_access_options(opts, &can, &cant, player)) {
      notify(player, "No valid options found.");
      return;
    }
    add_access_sitelock(player, site, can, cant);	/* HERE */
    write_access_file();
    notify(player, tprintf("Site %s access options set to %s", site, opts));
    do_log(LT_WIZ, player, NOTHING, "*** SITELOCK *** %s --> %s", site, opts);
    return;
  } else {
    /* Backward-compatible non-options form of the command,
     * or @sitelock/name
     */
    switch (type) {
    case 0:
    case 1:
      if (!site || !*site) {
	/* List bad sites */
	do_list_access(player);
	return;
      }
      /* Add a site */
      if (type == 0)
	add_access_sitelock(player, site, 0, ACS_CREATE);
      else
	add_access_sitelock(player, site, 0, ACS_DEFAULT);
      write_access_file();
      notify(player, tprintf("Site %s locked", site));
      do_log(LT_WIZ, player, NOTHING, "*** SITELOCK *** %s", site);
      break;
    case 2:
      if (!site || !*site) {
	/* List bad names */
#ifndef WIN32
	close(reserved);
#endif
	if ((fp = fopen(NAMES_FILE, "r")) == NULL) {
	  notify(player, "Unable to open names file.");
	} else {
	  while (fgets(buffer, 79, fp)) {
	    if ((p = strrchr(buffer, '\n')))	/* extra parens for gcc */
	      *p = '\0';
	    notify(player, buffer);
	  }
	  fclose(fp);
	}
#ifndef WIN32
	reserved = open("/dev/null", O_RDWR);
#endif
	return;
      }
      if (site[0] == '!') {	/* Delete a name */
#ifndef WIN32
	close(reserved);
#endif
	if ((fp = fopen(NAMES_FILE, "r")) != NULL) {
	  if ((fptmp = fopen("tmp.tmp", "w")) == NULL) {
	    notify(player, "Unable to open names file.");
	    fclose(fp);
	  } else {
	    while (fgets(buffer, 79, fp)) {
	      if ((p = strrchr(buffer, '\n')))
		*p = '\0';
	      if (strcmp(buffer, site + 1) != 0)
		fprintf(fptmp, "%s\n", buffer);
	    }
	    fclose(fp);
	    fclose(fptmp);
	    if (unlink(NAMES_FILE) == 0) {
	      if (rename("tmp.tmp", NAMES_FILE) == 0) {
		notify(player, "Name removed.");
		do_log(LT_WIZ, player, NOTHING, "*** UNLOCKED NAME *** %s",
		       site + 1);
	      } else {
		notify(player, "Unable to delete name.");
	      }
	    } else {
	      notify(player, "Unable to delete name.");
	    }
	  }
	} else
	  notify(player, "Unable to delete name.");
#ifndef WIN32
	reserved = open("/dev/null", O_RDWR);
#endif
	return;
      }
      /* Add a name */
#ifndef WIN32
      close(reserved);
#endif
      if ((fp = fopen(NAMES_FILE, "a")) != NULL) {
	/* Put a newline at the end of the site */
	fprintf(fp, "%s\n", site);
	fclose(fp);
	notify(player, tprintf("Name %s locked", site));
	do_log(LT_WIZ, player, NOTHING, "*** NAMELOCK *** %s", site);
      } else {
	notify(player, "Error writing to file.");
      }
#ifndef WIN32
      reserved = open("/dev/null", O_RDWR);
#endif
      break;
    }
  }
}



/*-----------------------------------------------------------------
 * Functions which give memory information on objects or players.
 * Source code originally by Kalkin, modified by Javelin
 */

static int
mem_usage(thing)
    dbref thing;
{
  int k;
  ATTR *m;
  lock_list *l;

  k = sizeof(struct object);	/* overhead */
  k += strlen(Name(thing)) + 1;	/* The name */
  for (m = List(thing); m; m = AL_NEXT(m)) {
    k += sizeof(ATTR);
    if (!(AL_FLAGS(m) & AF_STATIC))
      k += strlen(AL_NAME(m)) + 1;
    if (*AL_STR(m))
#ifdef macintosh
      k += strlen((const char *)(*(AL_STR(m)))) + 1;
#else
      k += strlen(AL_STR(m)) + 1;
#endif
    /* NOTE! In the above, we're getting the size of the
     * compressed attrib, not the uncompressed one (as Kalkin did)
     * because (1) it's more efficient, (2) it's more accurate
     * since that's how the object is in memory. This relies on
     * compressed attribs being terminated with \0's, which they
     * are in compress.c. If that changes, this breaks.
     */
  }
  for (l = Locks(thing); l; l = l->next) {
    k += sizeof(lock_list);
    if (!match_lock(l->type))
      k += strlen(l->type) + 1;
    k += sizeof_boolexp(l->key);
  }
  return k;
}

/* ARGSUSED */
FUNCTION(fun_objmem)
{
  dbref thing;

  if (!Search_All(executor)) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  if (!strcasecmp(args[0], "me"))
    thing = executor;
  else if (!strcasecmp(args[0], "here"))
    thing = Location(executor);
  else {
    thing = noisy_match_result(executor, args[0], NOTYPE, MAT_OBJECTS);
  }

  if (!GoodObject(thing) || !Can_Examine(executor, thing)) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  safe_str(unparse_integer(mem_usage(thing)), buff, bp);
}



/* ARGSUSED */
FUNCTION(fun_playermem)
{
  int tot = 0;
  dbref thing;
  dbref j;


  if (options.daytime) {
    notify(executor, "Function disabled.");
    safe_str("#-1", buff, bp);
    return;
  }
  if (!Search_All(executor)) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  if (!strcasecmp(args[0], "me") && IsPlayer(executor))
    thing = executor;
  else if (*args[0] && *args[0] == '*')
    thing = lookup_player(args[0] + 1);
  else if (*args[0] && *args[0] == '#')
    thing = atoi(args[0] + 1);
  else
    thing = lookup_player(args[0]);

  if (!GoodObject(thing) || !Can_Examine(executor, thing)) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  for (j = 0; j < db_top; j++)
    if (Owner(j) == thing)
      tot += mem_usage(j);
  safe_str(unparse_integer(tot), buff, bp);
}


#ifdef WIN32
void
Win32stats(dbref player)
{				/* written by NJG */
  MEMORYSTATUS memstat;
  double mem;
#if (COMPRESSION_TYPE == 3)
  long items, used, total_comp, total_uncomp;
  double percent;
#endif

  memstat.dwLength = sizeof(memstat);
  GlobalMemoryStatus(&memstat);

  notify(player, "---------- Windows memory usage ------------");
  notify(player, tprintf("%10ld %% memory in use",
			 memstat.dwMemoryLoad));

  mem = memstat.dwAvailPhys / 1024.0 / 1024.0;
  notify(player, tprintf("%10.3f Mb free physical memory", mem));
  mem = memstat.dwTotalPhys / 1024.0 / 1024.0;
  notify(player, tprintf("%10.3f Mb total physical memory", mem));
  mem = memstat.dwAvailPageFile / 1024.0 / 1024.0;
  notify(player, tprintf("%10.3f Mb available in the paging file ", mem));
  mem = memstat.dwTotalPageFile / 1024.0 / 1024.0;
  notify(player, tprintf("%10.3f Mb total paging file size", mem));

#if (COMPRESSION_TYPE == 3)

  compress_stats(&items, &used, &total_uncomp, &total_comp);
  notify(player, "---------- Internal attribute compression  ----------");
  notify(player, tprintf("%10ld compression table items used, "
			 "taking %ld bytes.", items, used));
  notify(player, tprintf("%10ld bytes in text before compression. ",
			 total_uncomp));
  notify(player, tprintf("%10ld bytes in text AFTER  compression. ",
			 total_comp));
  percent = ((float) (total_comp)) / ((float) total_uncomp) * 100.0;
  notify(player, tprintf("%10.0f %% text    compression ratio (lower is better). ",
			 percent));
  percent = ((float) (total_comp + used + (32768L * sizeof(char *))))
  / ((float) total_uncomp) * 100.0;
  notify(player, tprintf("%10.0f %% OVERALL compression ratio (lower is better). ",
			 percent));
  notify(player, tprintf("          (Includes table items, and table of words pointers of %ld bytes)",
			 32768L * sizeof(char *)));
  if (percent >= 100.0)
    notify(player, "          "
	   "(Compression ratio improves with larger database)");

#endif

}				/* end of calc_memory_used */
#endif


/*
 * ---------------------------------------------------------------------------
 * do_reboot: Reboots the game w/o disconnecting players.
 */

void
do_reboot(player, flag)
    dbref player;
    int flag;			/* 0 = normal, 1 = paranoid */
{
#ifdef macintosh
  notify(player, "Alas, I can't reboot under MacOS.  Sorry.");
#else
#ifdef WIN32
  notify(player, "Alas, I can't reboot under Windows.  Sorry.");
#else
  if (player == NOTHING) {
    flag_broadcast(0, 0, "GAME: Reboot w/o disconnect from game account, please wait.");
  } else {
    if (!Wizard(player)) {
      notify(player, "You don't have the authority to do that!");
      return;
    }
    flag_broadcast(0, 0, "GAME: Reboot w/o disconnect by %s, please wait.",
		   Name(Owner(player)));
  }
  if (flag) {
    paranoid_dump = 1;
    paranoid_checkpt = db_top / 5;
    if (paranoid_checkpt < 1)
      paranoid_checkpt = 1;
  }
  fork_and_dump(0);
  alarm(0);
  dump_reboot_db();

#ifdef INFO_SLAVE
  kill_info_slave();
#endif

  local_shutdown();

/* OFFLINE #ifdef TREKMUSH
  offline_shutdown();
#endif */

#ifndef SINGLE_LOGFILE
  /* close up the log files */
  end_log(connlog_fp);
  end_log(checklog_fp);
  end_log(wizlog_fp);
  end_log(tracelog_fp);
  end_log(cmdlog_fp);
#ifdef TREKMUSH
  end_log(spacelog_fp);
#endif /* TREKMUSH */
#endif				/* SINGLE_LOGFILE */

  execl("netmush", "netmush", confname, errlog, NULL);
#endif				/* WIN32 */
#endif              /* macintosh */
}
