/* wiz.c */

#include "copyrite.h"
#include "config.h"

/* Wizard-only commands */

#ifdef I_UNISTD
#include <unistd.h>
#endif
#include <string.h>
#include <math.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#ifdef I_FCNTL
#include <fcntl.h>
#endif
#ifdef WIN32
#include <windows.h>
#include "process.h"
#endif
#include "conf.h"
#include "mushdb.h"
#include "attrib.h"
#include "match.h"
#include "externs.h"
#include "access.h"
#include "parse.h"
#include "mymalloc.h"
#include "flags.h"
#include "lock.h"
#include "log.h"
#include "game.h"
#include "command.h"
#include "dbdefs.h"
#ifdef USE_MAILER
#include "extmail.h"
#endif

#ifdef MEM_CHECK
#include "memcheck.h"
#endif

#include "confmagic.h"

extern dbref find_entrance(dbref door);
struct db_stat_info *get_stats(dbref owner);
extern dbref find_player_by_desc(int port);
extern int paranoid_dump;
extern int paranoid_checkpt;

extern int invok_counter;	/* invocation limit */


#ifndef WIN32
#ifdef I_SYS_FILE
#include <sys/file.h>
#endif
extern int reserved;
#endif

int tport_dest_ok(dbref player, dbref victim, dbref dest);
int tport_control_ok(dbref player, dbref victim, dbref loc);
static int mem_usage(dbref thing);

#ifdef INFO_SLAVE
void kill_info_slave(void);
#endif

extern char confname[BUFFER_LEN];
extern char errlog[BUFFER_LEN];

dbref
do_pcreate(creator, player_name, player_password)
    dbref creator;
    const char *player_name;
    const char *player_password;
{
  dbref player;

  if (!Create_Player(creator)) {
    notify(creator, T("You do not have the power over body and mind!"));
    return NOTHING;
  }
  if (!can_pay_fees(creator, 0))
    return NOTHING;
  player = create_player(player_name, player_password, "None", "None");
  if (player == NOTHING) {
    notify_format(creator, T("Failure creating '%s' (bad name)"), player_name);
    return NOTHING;
  }
  if (player == AMBIGUOUS) {
    notify_format(creator, T("Failure creating '%s' (bad password)"),
		  player_name);
    return NOTHING;
  }
  notify_format(creator, T("New player '%s' (#%d) created with password '%s'"),
		player_name, player, player_password);
  do_log(LT_WIZ, creator, player, T("Player creation"));
  return player;
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
      notify(player, T("No such player."));
      return;
    }
  }

  /* check permissions */
  if (!Wizard(player) && set_q) {
    notify(player, T("Only wizards may change a quota."));
    return;
  }
  if (!Do_Quotas(player) && !See_All(player) && (player != who)) {
    notify(player, T("You can't look at someone else's quota."));
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
    notify_format(player, T("Objects: %d   Limit: UNLIMITED"), owned);
    return;
  }
  /* if we're not doing a change, determine the mortal's quota limit. 
   * RQUOTA is the objects _left_, not the quota itself.
   */

  if (!set_q) {
    limit = get_current_quota(who);
    notify_format(player, T("Objects: %d   Limit: %d"), owned, owned + limit);
    return;
  }
  /* set a new quota */
  if (!arg2 || !*arg2) {
    limit = get_current_quota(who);
    notify_format(player, T("Objects: %d   Limit: %d"), owned, owned + limit);
    notify(player, T("What do you want to set the quota to?"));
    return;
  }
  adjust = ((*arg2 == '+') || (*arg2 == '-'));
  if (adjust)
    limit = owned + get_current_quota(who) + atoi(arg2);
  else
    limit = atoi(arg2);
  if (limit < owned)		/* always have enough quota for your objects */
    limit = owned;

  (void) atr_add(Owner(who), "RQUOTA", tprintf("%d", limit - owned), GOD,
		 NOTHING);

  notify_format(player, T("Objects: %d   Limit: %d"), owned, limit);
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
    notify(player, T("Who do you think you are, GOD?"));
    return;
  }
  if (!arg1 || !*arg1) {
    limit = -1;
  } else if (!is_integer(arg1)) {
    notify(player, T("You can only set quotas to a number."));
    return;
  } else {
    limit = parse_integer(arg1);
    if (limit < 0) {
      notify(player, T("You can only set quotas to a positive number."));
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
	notify_format(player, "%s: Objects: %d   Limit: UNLIMITED",
		      Name(who), owned);
      continue;
    }
    if (!quiet) {
      oldlimit = get_current_quota(who);
      notify_format(player, "%s: Objects: %d   Limit: %d",
		    Name(who), owned, oldlimit);
    }
    if (limit != -1) {
      if (limit <= owned)
	(void) atr_add(who, "RQUOTA", "0", GOD, NOTHING);
      else
	(void) atr_add(who, "RQUOTA", tprintf("%d", limit - owned), GOD,
		       NOTHING);
    }
  }
  if (limit == -1)
    notify(player, T("Quotas not changed."));
  else
    notify_format(player, T("All quotas changed to %d."), limit);
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

  if (JumpOk(dest))
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
do_teleport(player, arg1, arg2, silent)
    dbref player;
    const char *arg1;
    const char *arg2;
    int silent;
{
  dbref victim;
  dbref destination;
  dbref loc;
  const char *to;
  dbref absroom;		/* "absolute room", for NO_TEL check */

  /* get victim, destination */
  if (*arg2 == '\0') {
    victim = player;
    to = arg1;
  } else {
    if ((victim =
	 noisy_match_result(player, arg1, NOTYPE,
			    MAT_OBJECTS | MAT_ENGLISH)) == NOTHING) {
      return;
    }
    to = arg2;
  }

  if (IsRoom(victim)) {
    notify(player, T("You can't teleport rooms."));
    return;
  }
  if (IsGarbage(victim)) {
    notify(player, T("Garbage belongs in the garbage dump."));
    return;
  }
  /* get destination */

  if (!strcasecmp(to, "home")) {
    /* If the object is @tel'ing itself home, treat it the way we'd  
     * treat a 'home' command 
     */
    if (player == victim) {
      if (Fixed(victim))
	notify(player, T("You can't do that IC!"));
      else
	safe_tel(victim, HOME, silent);
      return;
    } else
      destination = Home(victim);
  } else {
    destination = match_result(player, to, TYPE_PLAYER, MAT_EVERYTHING);
  }

  switch (destination) {
  case NOTHING:
    notify(player, T("No match."));
    break;
  case AMBIGUOUS:
    notify(player, T("I don't know which destination you mean!"));
    break;
  case HOME:
    destination = Home(victim);
    /* FALL THROUGH */
  default:
    /* check victim, destination types, teleport if ok */
    if (!GoodObject(destination) || IsGarbage(destination)) {
      notify(player, T("Bad destination."));
      return;
    }
    if (recursive_member(destination, victim, 0)
	|| (victim == destination)) {
      notify(player, T("Bad destination."));
      return;
    }
    if (!Tel_Anywhere(player) && IsPlayer(victim) && IsPlayer(destination)) {
      notify(player, T("Bad destination."));
      return;
    }
    if (IsExit(victim)) {
      /* Teleporting an exit means moving its source */
      if (!IsRoom(destination)) {
	notify(player, T("Exits can only be teleported to other rooms."));
	return;
      }
      if (Going(destination)) {
	notify(player,
	       T("You can't move an exit to someplace that's crumbling."));
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
	    (controls(player, destination) || Open_Anywhere(player)))) {
	notify(player, T("Permission denied."));
	return;
      }
      /* Remove it from its old room */
      Exits(loc) = remove_first(Exits(loc), victim);
      /* Put it into its new room */
      Source(victim) = destination;
      PUSH(victim, Exits(destination));
      if (!Quiet(player) && !(Quiet(victim) && (Owner(victim) == player)))
	notify(player, T("Teleported."));
      return;
    }
    loc = Location(victim);

    /* if royal or wiz and destination is player, tel to location */
    if (IsPlayer(destination) && Tel_Anywhere(player) && IsPlayer(victim)) {
      if (!silent && loc != Location(destination))
	did_it(victim, victim, NULL, NULL, "OXTPORT", NULL, NULL, loc);
      safe_tel(victim, Location(destination), silent);
      if (!silent && loc != Location(destination))
	did_it(victim, victim, "TPORT", NULL, "OTPORT", NULL, "ATPORT",
	       Location(destination));
      return;
    }
    /* check needed for NOTHING. Especially important for unlinked exits */
    if ((absroom = Location(victim)) == NOTHING) {
      notify(victim, T("You're in the Void. This is not a good thing."));
      /* At this point, they're in a bad location, so let's check
       * if home is valid before sending them there. */
      if (!GoodObject(Home(victim)))
	Home(victim) = PLAYER_START;
      do_move(victim, "home", 0);
      return;
    } else {
      /* valid location, perform other checks */

      /* if player is inside himself, send him home */
      if (absroom == victim) {
	notify(player, T("What are you doing inside of yourself?"));
	if (Home(victim) == absroom)
	  Home(victim) = PLAYER_START;
	do_move(victim, "home", 0);
	return;
      }
      /* find the "absolute" room */
      absroom = absolute_room(victim);

      if (absroom == NOTHING) {
	notify(victim, T("You're in the void - sending you home."));
	if (Home(victim) == Location(victim))
	  Home(victim) = PLAYER_START;
	do_move(victim, "home", 0);
	return;
      }
      /* if there are a lot of containers, send him home */
      if (absroom == AMBIGUOUS) {
	notify(victim, T("You're in too many containers."));
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
      if ((NoTel(absroom) || !eval_lock(player, absroom, Leave_Lock))
	  && !controls(player, absroom)
	  && !Tel_Anywhere(player)) {
	notify(player, T("Teleports are not allowed in this room."));
	return;
      }
      /* Now check the Z_TEL status of the victim's room.
       * Just like NO_TEL above, except that if the room (or its
       * Zone Master Room, if any, is Z_TEL,
       * the destination must also be a room in the same zone
       */
      if (GoodObject(Zone(absroom)) && (ZTel(absroom) || ZTel(Zone(absroom)))
	  && !controls(player, absroom) && !Tel_Anywhere(player)
	  && (Zone(absroom) != Zone(destination))) {
	notify(player,
	       T("You may not teleport out of the zone from this room."));
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
	if (!silent && loc != destination)
	  did_it(victim, victim, NULL, NULL, "OXTPORT", NULL, NULL, loc);
	safe_tel(victim, destination, silent);
	if (!silent && loc != destination)
	  did_it(victim, victim, "TPORT", NULL, "OTPORT", NULL, "ATPORT",
		 destination);
	if ((victim != player) && !(Puppet(victim) &&
				    (Owner(victim) == Owner(player)))) {
	  if (!Quiet(player) && !(Quiet(victim) && (Owner(victim) == player)))
	    notify(player, T("Teleported."));
	}
	return;
      }
      /* we can't do it */
      did_it(player, destination, "EFAIL", T("Permission denied."),
	     "OEFAIL", NULL, "AEFAIL", Location(player));
      return;
    } else {
      /* attempted teleport to an exit */
      if (Tel_Anything(player) || controls(player, victim)
	  || controls(player, Location(victim)))
	do_move(victim, to, 0);
      else
	notify_format(victim,
		      T("%s tries to impose his will on you and fails."),
		      Name(player));
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
    notify(player, T("You can't force God!"));
    return;
  }
  /* force victim to do command */
  for (j = 0; j < 10; j++)
    wnxt[j] = wenv[j];
  for (j = 0; j < NUMQ; j++)
    rnxt[j] = renv[j];
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

  for (s = command; *s && !isspace((unsigned char) *s); s++) ;
  if (!*s)
    return (0);
  *s++ = '\0';

  do_force(player, command, s);
  return (1);
}

extern struct db_stat_info current_state;

struct db_stat_info *
get_stats(owner)
    dbref owner;
{
  dbref i;
  static struct db_stat_info si;

  if (owner == ANY_OWNER)
    return &current_state;

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
    notify_format(player, T("%s: No such player."), name);
    return;
  }
  if (!Search_All(player)) {
    if (owner != ANY_OWNER && owner != player) {
      notify(player, T("You need a search warrant to do that!"));
      return;
    }
  }
  si = get_stats(owner);
  if (owner == ANY_OWNER) {
    notify_format(player,
		  T
		  ("%d objects = %d rooms, %d exits, %d things, %d players, %d garbage."),
		  si->total, si->rooms, si->exits, si->things, si->players,
		  si->garbage);
    if (first_free != NOTHING)
      notify_format(player, T("The next object to be created will be #%d."),
		    first_free);
  } else {
    notify_format(player,
		  T("%d objects = %d rooms, %d exits, %d things, %d players."),
		  si->total - si->garbage, si->rooms, si->exits, si->things,
		  si->players);
  }
}

void
do_newpassword(player, name, password)
    dbref player;
    const char *name;
    const char *password;
{
  dbref victim;
  if ((victim = lookup_player(name)) == NOTHING) {
    notify(player, T("No such player."));
  } else if (*password != '\0' && !ok_password(password)) {
    /* Wiz can set null passwords, but not bad passwords */
    notify(player, T("Bad password."));
  } else if (God(victim) && !God(player)) {
    notify(player, T("You cannot change that player's password."));
  } else {
    /* it's ok, do it */
    (void) atr_add(victim, "XYXXY", mush_crypt(password), GOD, NOTHING);
    notify_format(player, T("Password for %s changed."), Name(victim));
    notify_format(victim, T("Your password has been changed by %s."),
		  Name(player));
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
  DESC *d = NULL;

  victim = NOTHING;
  switch (flag) {
  case 2:
    /* self boot */
    victim = player;
    break;
  case 1:
    /* boot by descriptor */
    victim = find_player_by_desc(atoi(name));
    if (victim == player)
      flag = 2;
    else if (victim == NOTHING) {
      d = port_desc(atoi(name));
      if (!d) {
	notify(player, "There is no one connected on that descriptor.");
	return;
      } else
	victim = AMBIGUOUS;
    }
    break;
  case 0:
    /* boot by name */
    if ((victim =
	 noisy_match_result(player, name, TYPE_PLAYER,
			    MAT_LIMITED | MAT_ME)) == NOTHING) {
      notify(player, T("No such connected player."));
      return;
    }
    if (victim == player)
      flag = 2;
    break;
  }

  if ((victim != player) && !Can_Boot(player)) {
    notify(player, T("You can't boot other people!"));
    return;
  }
  if (God(victim) && !God(player)) {
    notify(player, T("You're good.  That's spelled with two 'o's."));
    return;
  }
  /* look up descriptor */
  switch (flag) {
  case 0:
    d = player_desc(victim);
    break;
  case 1:
    d = port_desc(atoi(name));
    break;
  case 2:
    d = inactive_desc(victim);
    break;
  }

  /* check for more errors */
  if (!d) {
    if (flag == 2)
      notify(player, T("None of your connections are idle."));
    else
      notify(player, T("That player isn't connected!"));
  } else {
    do_log(LT_WIZ, player, victim, "*** BOOT ***");
    if (flag == 2)
      notify(player, T("You boot an idle self."));
    else if (victim == AMBIGUOUS)
      notify_format(player, T("You booted unconnected port %s!"), name);
    else {
      notify_format(player, T("You booted %s off!"), Name(victim));
      notify(victim, T("You are politely shown to the door."));
    }
    boot_desc(d);
  }
}

void
do_chownall(player, name, target, preserve)
    dbref player;
    const char *name;
    const char *target;
    int preserve;
{
  int i;
  dbref victim;
  dbref n_target;
  int count = 0;

  if (!Wizard(player)) {
    notify(player, T("Try asking them first!"));
    return;
  }
  if ((victim = noisy_match_result(player, name, TYPE_PLAYER, MAT_LIMITED))
      == NOTHING)
    return;

  if (!target || !*target) {
    n_target = player;
  } else {
    if ((n_target =
	 noisy_match_result(player, target, TYPE_PLAYER,
			    MAT_LIMITED)) == NOTHING)
      return;
  }

  for (i = 0; i < db_top; i++) {
    if ((Owner(i) == victim) && (!IsPlayer(i))) {
      chown_object(player, i, n_target, preserve);
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

  notify_format(player, T("Ownership changed for %d objects."), count);
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
    notify(player, T("You do not have the power to change reality."));
    return;
  }
  if ((victim = noisy_match_result(player, name, TYPE_PLAYER, MAT_LIMITED))
      == NOTHING)
    return;

  if (!target || !*target) {
    notify(player, T("No zone specified."));
    return;
  }
  if (!strcasecmp(target, "none"))
    zone = NOTHING;
  else {
    switch (zone = match_result(player, target, NOTYPE, MAT_EVERYTHING)) {
    case NOTHING:
      notify(player, T("I can't seem to find that."));
      return;
    case AMBIGUOUS:
      notify(player, T("I don't know which one you mean!"));
      return;
    }
  }

  /* Okay, now that we know we're not going to spew all sorts of errors,
   * call the normal do_chzone for all the relevant objects.  This keeps
   * consistency on things like flag resetting, etc... */
  for (i = 0; i < db_top; i++) {
    if (Owner(i) == victim && Zone(i) != zone) {
      count += do_chzone(player, unparse_dbref(i), target, 0);
    }
  }
  notify_format(player, T("Zone changed for %d objects."), count);
}

/*-----------------------------------------------------------------------
 * Nasty management: @kick, examine/debug
 */

void
do_kick(player, num)
    dbref player;
    const char *num;
{
  /* executes <num> commands off the queue immediately */

  int n;

  if (!Wizard(player)) {
    notify(player, T("Permission denied."));
    return;
  }
  if (!num || !*num) {
    notify(player, T("How many commands do you want to execute?"));
    return;
  }
  n = atoi(num);

  if (n <= 0) {
    notify(player, T("Number out of range."));
    return;
  }
  n = do_top(n);

  notify_format(player, T("%d commands executed."), n);

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
    notify(player, T("Permission denied."));
    return;
  }
  /* find it */
  thing = noisy_match_result(player, name, NOTYPE, MAT_EVERYTHING);
  if (!GoodObject(thing))
    return;

  notify(player, object_header(player, thing));
  notify_format(player, T("Flags value: %s"),
		bits_to_string(Flags(thing), GOD, NOTHING));
  notify_format(player, T("Powers value: 0x%08x"), Powers(thing));

  notify_format(player, "Next: %d", Next(thing));
  notify_format(player, "Contents: %d", Contents(thing));

  switch (Typeof(thing)) {
  case TYPE_PLAYER:
#ifdef USE_MAILER
    mp = desc_mail(thing);
    notify_format(player, T("First mail sender: %d"), mp ? mp->from : NOTHING);
#endif
  case TYPE_THING:
    notify_format(player, "Location: %d", Location(thing));
    notify_format(player, "Home: %d", Home(thing));
    break;
  case TYPE_EXIT:
    notify_format(player, "Destination: %d", Location(thing));
    notify_format(player, "Source: %d", Source(thing));
    break;
  case TYPE_ROOM:
    notify_format(player, "Drop-to: %d", Location(thing));
    notify_format(player, "Exits: %d", Exits(thing));
    break;
  case TYPE_GARBAGE:
    break;
  default:
    notify(player, T("Bad object type."));
  }
}

/*-------------------------------------------------------------------------
 * Powers stuff
 */

void
do_power(dbref player, const char *name, const char *power)
{
  int pwr;
  const char *s;
  dbref thing;

  if (!Wizard(player)) {
    notify(player, T("Only wizards may grant powers."));
    return;
  }
  if ((thing = noisy_match_result(player, name, NOTYPE, MAT_EVERYTHING)) ==
      NOTHING)
    return;
#ifdef ONLINE_REG
  if (Unregistered(thing)) {
    notify(player, T("You can't grant powers to unregistered players."));
    return;
  }
#endif
  if (God(thing) && !God(player)) {
    notify(player, T("God is already all-powerful."));
    return;
  }
  /* move past the not token if there is one */
  for (s = power; *s && ((*s == NOT_TOKEN) || isspace((unsigned char) *s));
       s++) ;

  if (*s == '\0') {
    notify(player, T("You must specify a power."));
    return;
  }
  pwr = find_power(s);
  if (pwr == -1) {
    notify(player, T("That is not a power."));
    return;
  }
  if (*power == NOT_TOKEN) {
    Powers(thing) &= ~pwr;
    if (!AreQuiet(player, thing))
      notify_format(player, T("%s - power removed."), Name(thing));
    do_log(LT_WIZ, player, thing, T("Power Removed: %s"), power);
  } else {
    if (Hasprivs(thing) && (pwr == IS_GUEST)) {
      notify(player, T("You can't make admin into guests."));
      return;
    }
    Powers(thing) |= pwr;
    if (!AreQuiet(player, thing))
      notify_format(player, T("%s - power granted."), Name(thing));
    do_log(LT_WIZ, player, thing, T("Power Granted: %s"), power);
  }
}

/*----------------------------------------------------------------------------
 * Search functions
 */


int raw_search(dbref player, const char *owner, const char *class,
	       const char *restriction, const char *start, const char *stop,
	       dbref **result, PE_Info * pe_info);

void
do_search(dbref player, const char *arg1, char **arg3)
{
  char tbuf[BUFFER_LEN], *arg2 = tbuf, *tbp;
  dbref *results = NULL;
  int nresults;

  /* parse first argument into two */
  if (!arg1 || *arg1 == '\0')
    arg1 = "me";

  /* First argument is a player, so we could have a quoted name */
  if (PLAYER_NAME_SPACES && *arg1 == '\"') {
    for (; *arg1 && ((*arg1 == '\"') || isspace((unsigned char) *arg1));
	 arg1++) ;
    strcpy(tbuf, arg1);
    while (*arg2 && (*arg2 != '\"')) {
      while (*arg2 && (*arg2 != '\"'))
	arg2++;
      if (*arg2 == '\"') {
	*arg2++ = '\0';
	while (*arg2 && isspace((unsigned char) *arg2))
	  arg2++;
	break;
      }
    }
  } else {
    strcpy(tbuf, arg1);
    while (*arg2 && !isspace((unsigned char) *arg2))
      arg2++;
    if (*arg2)
      *arg2++ = '\0';
    while (*arg2 && isspace((unsigned char) *arg2))
      arg2++;
  }

  if (!*arg2) {
    if (!arg3[1] || !*arg3[1])
      arg2 = (char *) "";	/* arg1 */
    else {
      arg2 = (char *) arg1;	/* arg2=arg3 */
      tbuf[0] = '\0';
    }
  }

  nresults = raw_search(player, tbuf, arg2, arg3[1], arg3[2], arg3[3],
			&results, NULL);

  if (nresults == 0) {
    notify(player, T("Nothing found."));
  } else if (nresults > 0) {
    /* Split the results up by type and report. */
    int n;
    int nthings = 0, nexits = 0, nrooms = 0, nplayers = 0;
    dbref *things, *exits, *rooms, *players;

    things = (dbref *) mush_malloc(sizeof(dbref) * nresults, "dbref_list");
    exits = (dbref *) mush_malloc(sizeof(dbref) * nresults, "dbref_list");
    rooms = (dbref *) mush_malloc(sizeof(dbref) * nresults, "dbref_list");
    players = (dbref *) mush_malloc(sizeof(dbref) * nresults, "dbref_list");

    for (n = 0; n < nresults; n++) {
      switch (Typeof(results[n])) {
      case TYPE_THING:
	things[nthings++] = results[n];
	break;
      case TYPE_EXIT:
	exits[nexits++] = results[n];
	break;
      case TYPE_ROOM:
	rooms[nrooms++] = results[n];
	break;
      case TYPE_PLAYER:
	players[nplayers++] = results[n];
	break;
      default:
	/* Unknown type. Ignore. */
	do_rawlog(LT_ERR, T("Weird type for dbref #%d"), results[n]);
      }
    }

    if (nrooms) {
      notify(player, "\nROOMS:");
      for (n = 0; n < nrooms; n++) {
	tbp = tbuf;
	safe_format(tbuf, &tbp, "%s [owner: ", object_header(player, rooms[n]));
	safe_str(object_header(player, Owner(rooms[n])), tbuf, &tbp);
	safe_chr(']', tbuf, &tbp);
	*tbp = '\0';
	notify(player, tbuf);
      }
    }

    if (nexits) {
      dbref from, to;

      notify(player, "\nEXITS:");
      for (n = 0; n < nexits; n++) {
	tbp = tbuf;
	if (Source(exits[n]) == NOTHING)
	  from = NOTHING;
	else
	  from = Source(exits[n]);
	to = Destination(exits[n]);
	safe_format(tbuf, &tbp, "%s [from ", object_header(player, exits[n]));
	safe_str((from == NOTHING) ? "NOWHERE" : object_header(player, from),
		 tbuf, &tbp);
	safe_str(" to ", tbuf, &tbp);
	safe_str((to == NOTHING) ? "NOWHERE" : object_header(player, to),
		 tbuf, &tbp);
	safe_chr(']', tbuf, &tbp);
	*tbp = '\0';
	notify(player, tbuf);
      }
    }

    if (nthings) {
      notify(player, "\nOBJECTS:");
      for (n = 0; n < nthings; n++) {
	tbp = tbuf;
	safe_format(tbuf, &tbp, "%s [owner: ",
		    object_header(player, things[n]));
	safe_str(object_header(player, Owner(things[n])), tbuf, &tbp);
	safe_chr(']', tbuf, &tbp);
	*tbp = '\0';
	notify(player, tbuf);
      }
    }

    if (nplayers) {
      int is_wizard = Search_All(player) || See_All(player);
      notify(player, "\nPLAYERS:");
      for (n = 0; n < nplayers; n++) {
	tbp = tbuf;
	safe_str(object_header(player, players[n]), tbuf, &tbp);
	if (is_wizard)
	  safe_format(tbuf, &tbp,
		      T(" [location: %s]"),
		      object_header(player, Location(players[n])));
	*tbp = '\0';
	notify(player, tbuf);
      }
    }

    notify(player, T("----------  Search Done  ----------"));
    notify_format(player,
		  T
		  ("Totals: Rooms...%d  Exits...%d  Objects...%d  Players...%d"),
		  nrooms, nexits, nthings, nplayers);
    mush_free((Malloc_t) rooms, "dbref_list");
    mush_free((Malloc_t) exits, "dbref_list");
    mush_free((Malloc_t) things, "dbref_list");
    mush_free((Malloc_t) players, "dbref_list");
  }
  if (results)
    mush_free(results, "search_results");
}

FUNCTION(fun_lsearch)
{
  int nresults;
  dbref *results = NULL;
  int rev = !strcmp(called_as, "LSEARCHR");

  if (!command_check_byname(executor, "@search")) {
    safe_str(T(e_perm), buff, bp);
    return;
  }

  if (!strcmp(called_as, "CHILDREN"))
    nresults = raw_search(executor, NULL, "PARENT", args[0], NULL,
			  NULL, &results, pe_info);
  else
    nresults = raw_search(executor, args[0], args[1], args[2], args[3], args[4],
			  &results, pe_info);

  if (nresults < 0) {
    safe_str("#-1", buff, bp);
  } else if (nresults == 0) {
    notify(executor, T("Nothing found."));
    safe_str("#-1", buff, bp);
  } else {
    int first = 1, n;
    if (!rev) {
      for (n = 0; n < nresults; n++) {
	if (first)
	  first = 0;
	else if (safe_chr(' ', buff, bp))
	  break;
	if (safe_dbref(results[n], buff, bp))
	  break;
      }
    } else {
      for (n = nresults - 1; n >= 0; n--) {
	if (first)
	  first = 0;
	else if (safe_chr(' ', buff, bp))
	  break;
	if (safe_dbref(results[n], buff, bp))
	  break;
      }
    }
  }
  if (results)
    mush_free(results, "search_results");
}


enum search_class { S_OWNER, S_TYPE, S_PARENT, S_ZONE, S_FLAG,
  S_POWER, S_EVAL, S_NAME, S_LFLAG
};

/* Does the actual searching */
int
raw_search(dbref player, const char *owner, const char *class,
	   const char *restriction, const char *start, const char *stop,
	   dbref **result, PE_Info * pe_info)
{
  size_t result_size;
  size_t nresults = 0;
  enum search_class sclass = S_OWNER;
  int n;
  int restrict_type = NOTYPE;
  int restrict_powers = 0;
  dbref restrict_obj = NOTHING, restrict_owner = ANY_OWNER;
  int is_wiz;
  dbref low = 0, high = db_top - 1;

  is_wiz = Search_All(player) || See_All(player);

  /* Range limits */
  if (start && *start) {
    size_t offset = 0;
    if (start[0] == '#')
      offset = 1;
    low = parse_integer(start + offset);
    if (!GoodObject(low))
      low = 0;
  }
  if (stop && *stop) {
    size_t offset = 0;
    if (stop[0] == '#')
      offset = 1;
    high = parse_integer(stop + offset);
    if (!GoodObject(high))
      high = db_top - 1;
  }

  /* set limits on who we search */
  if (!owner || !*owner || strcasecmp(owner, "all") == 0)
    restrict_owner = is_wiz ? ANY_OWNER : player;
  else if (strcasecmp(owner, "me") == 0)
    restrict_owner = player;
  else
    restrict_owner = lookup_player(owner);
  if (restrict_owner == NOTHING) {
    notify(player, T("Unknown owner."));
    return -1;
  }

  /* Figure out the class */
  if (!class || !*class || strcasecmp(class, "none") == 0) {
    sclass = S_OWNER;
  } else if (string_prefix("type", class)) {
    sclass = S_TYPE;
    if (string_prefix("things", restriction)
	|| string_prefix("objects", restriction)) {
      restrict_type = TYPE_THING;
    } else if (string_prefix("rooms", restriction)) {
      restrict_type = TYPE_ROOM;
    } else if (string_prefix("exits", restriction)) {
      restrict_type = TYPE_EXIT;
    } else if (string_prefix("rooms", restriction)) {
      restrict_type = TYPE_ROOM;
    } else if (string_prefix("players", restriction)) {
      restrict_type = TYPE_PLAYER;
    } else {
      notify(player, T("Unknown type."));
      return -1;
    }
  } else if (string_prefix("things", class) || string_prefix("objects", class)) {
    sclass = S_NAME;
    restrict_type = TYPE_THING;
  } else if (string_prefix("exits", class)) {
    sclass = S_NAME;
    restrict_type = TYPE_EXIT;
  } else if (string_prefix("rooms", class)) {
    sclass = S_NAME;
    restrict_type = TYPE_ROOM;
  } else if (string_prefix("players", class)) {
    sclass = S_NAME;
    restrict_type = TYPE_PLAYER;
  } else if (string_prefix("name", class)) {
    sclass = S_NAME;
  } else if (string_prefix("parent", class)) {
    sclass = S_PARENT;
    if (!is_dbref(restriction)) {
      notify(player, T("Unknown parent."));
      return -1;
    }
    restrict_obj = parse_dbref(restriction);
    if (!GoodObject(restrict_obj)) {
      notify(player, T("Unknown parent."));
      return -1;
    }
  } else if (string_prefix("zone", class)) {
    sclass = S_ZONE;
    if (!is_dbref(restriction)) {
      notify(player, T("Unknown zone."));
      return -1;
    }
    restrict_obj = parse_dbref(restriction);
    if (!GoodObject(restrict_obj)) {
      notify(player, T("Unknown zone."));
      return -1;
    }
  } else if (string_prefix("eval", class)) {
    sclass = S_EVAL;
  } else if (string_prefix("ethings", class) ||
	     string_prefix("eobjects", class)) {
    sclass = S_EVAL;
    restrict_type = TYPE_THING;
  } else if (string_prefix("eexits", class)) {
    sclass = S_EVAL;
    restrict_type = TYPE_EXIT;
  } else if (string_prefix("erooms", class)) {
    sclass = S_EVAL;
    restrict_type = TYPE_ROOM;
  } else if (string_prefix("eplayers", class)) {
    sclass = S_EVAL;
    restrict_type = TYPE_PLAYER;
  } else if (string_prefix("powers", class)) {
    sclass = S_POWER;
    restrict_powers = find_power(restriction);
    if (restrict_powers == -1) {
      notify(player, T("No such power to search for."));
      return -1;
    }
  } else if (string_prefix("flags", class)) {
    /* Handle the checking later.  */
    sclass = S_FLAG;
    if (!restriction || !*restriction) {
      notify(player, T("You must give a string of flag characters."));
      return -1;
    }
  } else if (string_prefix("lflags", class)) {
    /* Handle the checking later.  */
    sclass = S_LFLAG;
    if (!restriction || !*restriction) {
      notify(player, T("You must give a list of flag names."));
      return -1;
    }
  } else {
    notify(player, T("Unknown search class."));
    return -1;
  }

  if ((restrict_owner != ANY_OWNER && restrict_owner != player) &&
      !(is_wiz || (sclass == S_TYPE && restrict_type == TYPE_PLAYER))) {
    notify(player, T("You need a search warrant to do that."));
    return -1;
  }

  /* make sure player has money to do the search */
  if (!payfor(player, FIND_COST)) {
    notify_format(player, T("Searches cost %d %s."), FIND_COST,
		  ((FIND_COST == 1) ? MONEY : MONIES));
    return -1;
  }

  result_size = (db_top / 4) + 1;
  *result =
    (dbref *) mush_malloc(sizeof(dbref) * result_size, "search_results");
  if (!*result)
    panic(T("Couldn't allocate memory in search!"));

  switch (sclass) {
  case S_OWNER:		/* @search someone */
  case S_TYPE:			/* @search type=whatever */
    for (n = low; n <= high; n++) {
      if ((restrict_owner == ANY_OWNER || Owner(n) == restrict_owner)
	  && (restrict_type == NOTYPE || Typeof(n) == restrict_type)) {
	if (nresults >= result_size) {
	  dbref *newresults;
	  result_size *= 2;
	  newresults = (dbref *) realloc((Malloc_t) *result,
					 sizeof(dbref) * result_size);
	  if (!newresults)
	    panic(T("Couldn't reallocate memory in search!"));
	  *result = newresults;
	}
	(*result)[nresults++] = (dbref) n;
      }
    }
    break;
  case S_ZONE:			/* @search ZONE=#1234 */
    for (n = low; n <= high; n++) {
      if ((restrict_owner == ANY_OWNER || Owner(n) == restrict_owner)
	  && Zone(n) == restrict_obj) {
	if (nresults >= result_size) {
	  dbref *newresults;
	  result_size *= 2;
	  newresults =
	    (dbref *) realloc((Malloc_t) *result, sizeof(dbref) * result_size);
	  if (!newresults)
	    panic(T("Couldn't reallocate memory in search!"));
	  *result = newresults;
	}

	(*result)[nresults++] = (dbref) n;
      }
    }
    break;
  case S_PARENT:		/* @search parent=#1234 */
    for (n = low; n <= high; n++) {
      if ((restrict_owner == ANY_OWNER || Owner(n) == restrict_owner)
	  && Parent(n) == restrict_obj) {
	if (nresults >= result_size) {
	  dbref *newresults;
	  result_size *= 2;
	  newresults =
	    (dbref *) realloc((Malloc_t) *result, sizeof(dbref) * result_size);
	  if (!newresults)
	    panic(T("Couldn't reallocate memory in search!"));
	  *result = newresults;
	}

	(*result)[nresults++] = (dbref) n;
      }
    }
    break;
  case S_NAME:			/* @search (?:name|exits|objects|rooms|players|things)=name */
    for (n = low; n <= high; n++) {
      if ((restrict_owner == ANY_OWNER || Owner(n) == restrict_owner)
	  && (restrict_type == NOTYPE || Typeof(n) == restrict_type)
	  && string_match(Name(n), restriction)) {
	if (nresults >= result_size) {
	  dbref *newresults;
	  result_size *= 2;
	  newresults =
	    (dbref *) realloc((Malloc_t) *result, sizeof(dbref) * result_size);
	  if (!newresults)
	    panic(T("Couldn't reallocate memory in search!"));
	  *result = newresults;
	}

	(*result)[nresults++] = (dbref) n;
      }
    }
    break;
  case S_EVAL:			/* @search (?:eval|ething|eroom|eplayer|eexit)=code */
    {
      char *ebuf1;
      const char *ebuf2;
      char tbuf1[BUFFER_LEN];
      char *bp;

      if (!restriction || !*restriction)
	break;

      for (n = low; n <= high; n++) {
	if (!((restrict_owner == ANY_OWNER || Owner(n) == restrict_owner)
	      && (restrict_type == NOTYPE || Typeof(n) == restrict_type)))
	  continue;

	ebuf1 = replace_string("##", unparse_dbref(n), restriction);
	ebuf2 = ebuf1;
	bp = tbuf1;
	process_expression(tbuf1, &bp, &ebuf2, player, player, player,
			   PE_DEFAULT, PT_DEFAULT, pe_info);
	mush_free((Malloc_t) ebuf1, "replace_string.buff");
	*bp = '\0';
	if (!parse_boolean(tbuf1))
	  continue;

	if (nresults >= result_size) {
	  dbref *newresults;
	  result_size *= 2;
	  newresults =
	    (dbref *) realloc((Malloc_t) *result, sizeof(dbref) * result_size);
	  if (!newresults)
	    panic(T("Couldn't reallocate memory in search!"));
	  *result = newresults;
	}
	(*result)[nresults++] = (dbref) n;
	if (pe_info && pe_info->fun_invocations >= FUNCTION_LIMIT)
	  break;
      }
    }
    break;
  case S_POWER:		/* @search power=see_all */
    for (n = low; n <= high; n++) {
      if ((restrict_owner == ANY_OWNER || Owner(n) == restrict_owner)
	  && (Powers(n) & restrict_powers) == restrict_powers) {
	if (nresults >= result_size) {
	  dbref *newresults;
	  result_size *= 2;
	  newresults =
	    (dbref *) realloc((Malloc_t) *result, sizeof(dbref) * result_size);
	  if (!newresults)
	    panic(T("Couldn't reallocate memory in search!"));
	  *result = newresults;
	}

	(*result)[nresults++] = (dbref) n;
      }
    }
    break;
  case S_FLAG:
  case S_LFLAG:
    for (n = low; n <= high; n++) {
      if ((restrict_owner == ANY_OWNER || Owner(n) == restrict_owner)
	  && (restrict_type == NOTYPE || Typeof(n) == restrict_type)
	  && ((sclass == S_FLAG) ? flaglist_check(player, n, restriction, 1)
	      : flaglist_check_long(player, n, restriction, 1))) {
	if (nresults >= result_size) {
	  dbref *newresults;
	  result_size *= 2;
	  newresults =
	    (dbref *) realloc((Malloc_t) *result, sizeof(dbref) * result_size);
	  if (!newresults)
	    panic(T("Couldn't reallocate memory in search!"));
	  *result = newresults;
	}

	(*result)[nresults++] = (dbref) n;
      }
    }
    break;
  }

  return (int) nresults;
}

#ifdef WIN32
#pragma warning( disable : 4761)	/* Disable bogus conversion warning */
#endif
/* ARGSUSED */
FUNCTION(fun_hidden)
{
  dbref it = match_thing(executor, args[0]);
  if (!See_All(executor)) {
    notify(executor, T("Permission denied."));
    safe_str("#-1", buff, bp);
    return;
  }
  if ((it == NOTHING) || (!IsPlayer(it))) {
    notify(executor, T("Couldn't find that player."));
    safe_str("#-1", buff, bp);
    return;
  }
  safe_boolean(hidden(it), buff, bp);
  return;
}

#ifdef WIN32
#pragma warning( default : 4761)	/* Re-enable conversion warning */
#endif

#ifdef QUOTA
/* ARGSUSED */
FUNCTION(fun_quota)
{
  int owned;
  /* Tell us player's quota */
  dbref thing;
  dbref who = lookup_player(args[0]);
  if ((who == NOTHING) || !IsPlayer(who)) {
    notify(executor, T("Couldn't find that player."));
    safe_str("#-1", buff, bp);
    return;
  }
  if (!(Do_Quotas(executor) || See_All(executor)
	|| (who == executor))) {
    notify(executor, T("You can't see someone else's quota!"));
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

  safe_integer(owned + get_current_quota(who), buff, bp);
  return;
}
#endif

void
do_sitelock(player, site, opts, who, type)
    dbref player;
    const char *site;
    const char *opts;
    const char *who;
    int type;
  /* 0 = registration, 1 = siteban, 2 = names, 3 = remove, 4 = check */
{
  FILE *fp, *fptmp;
  char buffer[80];
  char *p;
  if (!Wizard(player)) {
    notify(player, T("Your delusions of grandeur have been noted."));
    return;
  }
  if (opts && *opts) {
    int can, cant;
    dbref whod = AMBIGUOUS;
    /* Options form of the command. */
    if (!site || !*site) {
      notify(player, T("What site did you want to lock?"));
      return;
    }
    can = cant = 0;
    if (!parse_access_options(opts, NULL, &can, &cant, player)) {
      notify(player, T("No valid options found."));
      return;
    }
    if (who && *who) {		/* Specify a character */
      whod = lookup_player(who);
      if (!GoodObject(whod)) {
	notify(player, T("Who do you want to lock?"));
	return;
      }
    }

    add_access_sitelock(player, site, whod, can, cant);
    write_access_file();
    if (whod != AMBIGUOUS) {
      notify_format(player,
		    T("Site %s access options for %s(%s) set to %s"),
		    site, Name(whod), unparse_dbref(whod), opts);
      do_log(LT_WIZ, player, NOTHING,
	     T("*** SITELOCK *** %s for %s(%s) --> %s"), site,
	     Name(whod), unparse_dbref(whod), opts);
    } else {
      notify_format(player, T("Site %s access options set to %s"), site, opts);
      do_log(LT_WIZ, player, NOTHING, "*** SITELOCK *** %s --> %s", site, opts);
    }
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
	add_access_sitelock(player, site, AMBIGUOUS, 0, ACS_CREATE);
      else
	add_access_sitelock(player, site, AMBIGUOUS, 0, ACS_DEFAULT);
      write_access_file();
      notify_format(player, T("Site %s locked"), site);
      do_log(LT_WIZ, player, NOTHING, "*** SITELOCK *** %s", site);
      break;
    case 4:{
	struct access *ap;
	char tbuf[BUFFER_LEN], *bp;
	int rulenum;
	if (!site || !*site) {
	  do_list_access(player);
	  return;
	}
	ap = site_check_access(site, AMBIGUOUS, &rulenum);
	bp = tbuf;
	format_access(ap, rulenum, AMBIGUOUS, tbuf, &bp);
	*bp = '\0';
	notify(player, tbuf);
	break;
      }
    case 3:{
	int n;
	n = remove_access_sitelock(site);
	write_access_file();
	notify_format(player, T("%d sitelocks removed."), n);
	break;
      }
    case 2:
      if (!site || !*site) {
	/* List bad names */
#ifndef WIN32
	close(reserved);
#endif
#ifdef macintosh
	if ((fp = fopen(NAMES_FILE, "rb")) == NULL) {
#else
	if ((fp = fopen(NAMES_FILE, "r")) == NULL) {
#endif
	  notify(player, T("Unable to open names file."));
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
#ifdef macintosh
	if ((fp = fopen(NAMES_FILE, "rb")) != NULL) {
	  if ((fptmp = fopen("tmp.tmp", "wb")) == NULL) {
#else
	if ((fp = fopen(NAMES_FILE, "r")) != NULL) {
	  if ((fptmp = fopen("tmp.tmp", "w")) == NULL) {
#endif
	    notify(player, T("Unable to open names file."));
	    fclose(fp);
	  } else {
	    while (fgets(buffer, 79, fp)) {
	      if ((p = strrchr(buffer, '\n')))
		*p = '\0';
	      if (strcasecmp(buffer, site + 1) != 0)
		fprintf(fptmp, "%s\n", buffer);
	    }
	    fclose(fp);
	    fclose(fptmp);
	    if (unlink(NAMES_FILE) == 0) {
	      if (rename("tmp.tmp", NAMES_FILE) == 0) {
		notify(player, T("Name removed."));
		do_log(LT_WIZ, player, NOTHING, "*** UNLOCKED NAME *** %s",
		       site + 1);
	      } else {
		notify(player, T("Unable to delete name."));
	      }
	    } else {
	      notify(player, T("Unable to delete name."));
	    }
	  }
	} else
	  notify(player, T("Unable to delete name."));
#ifndef WIN32
	reserved = open("/dev/null", O_RDWR);
#endif
	return;
      }
      /* Add a name */
#ifndef WIN32
      close(reserved);
#endif
#ifdef macintosh
      if ((fp = fopen(NAMES_FILE, "ab")) != NULL) {
#else
      if ((fp = fopen(NAMES_FILE, "a")) != NULL) {
#endif
	/* Put a newline at the end of the site */
	fprintf(fp, "%s\n", site);
	fclose(fp);
	notify_format(player, T("Name %s locked"), site);
	do_log(LT_WIZ, player, NOTHING, "*** NAMELOCK *** %s", site);
      } else {
	notify(player, T("Error writing to file."));
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
    if (AL_STR(m) && *AL_STR(m))
      k += u_strlen(AL_STR(m)) + 1;
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
    k += sizeof_boolexp(l->key);
  }
  return k;
}

/* ARGSUSED */
FUNCTION(fun_objmem)
{
  dbref thing;
  if (!Search_All(executor)) {
    safe_str(T(e_perm), buff, bp);
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
    safe_str(T(e_perm), buff, bp);
    return;
  }
  safe_integer(mem_usage(thing), buff, bp);
}



/* ARGSUSED */
FUNCTION(fun_playermem)
{
  int tot = 0;
  dbref thing;
  dbref j;

  if (!Search_All(executor)) {
    safe_str(T(e_perm), buff, bp);
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
    safe_str(T(e_perm), buff, bp);
    return;
  }
  for (j = 0; j < db_top; j++)
    if (Owner(j) == thing)
      tot += mem_usage(j);
  safe_integer(tot, buff, bp);
}


/*
 * ---------------------------------------------------------------------------
 * do_reboot: Reboots the game w/o disconnecting players.
 */ void
do_reboot(player, flag)
    dbref player;
    int flag;			/* 0 = normal, 1 = paranoid */
{
#ifdef macintosh
  notify(player, T("Alas, I can't reboot under MacOS.  Sorry."));
#else
  if (player == NOTHING) {
    flag_broadcast(0, 0,
		   T
		   ("GAME: Reboot w/o disconnect from game account, please wait."));
  } else {
    flag_broadcast(0, 0,
		   T
		   ("GAME: Reboot w/o disconnect by %s, please wait."),
		   Name(Owner(player)));
  }
  if (flag) {
    paranoid_dump = 1;
    paranoid_checkpt = db_top / 5;
    if (paranoid_checkpt < 1)
      paranoid_checkpt = 1;
  }
  fork_and_dump(0);
#ifndef WIN32
  /* Some broken libcs appear to retain the itimer across exec!
   * So we make sure that if we get a SIGPROF in our next incarnation,
   * we ignore it until our proper handler is set up.
   */
  ignore_signal(SIGPROF);
#endif
  dump_reboot_db();
#ifdef INFO_SLAVE
  kill_info_slave();
#endif
  local_shutdown();
  end_all_logs();
#ifndef WIN32
  execl("netmush", "netmush", confname, errlog, NULL);
#else
  execl("pennmush.exe", "pennmush.exe", "/run", NULL);
#endif				/* WIN32 */
#endif				/* macintosh */
  exit(1);			/* Shouldn't ever get here, but just in case... */
}
