/**
 * \file set.c
 *
 * \brief PennMUSH commands that set parameters.
 *
 *
 */

#include "copyrite.h"
#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#include <stdlib.h>

#include "conf.h"
#include "mushdb.h"
#include "match.h"
#include "attrib.h"
#include "externs.h"
#include "ansi.h"
#include "command.h"
#include "mymalloc.h"
#include "flags.h"
#include "dbdefs.h"
#include "lock.h"
#include "log.h"
#include "confmagic.h"

void do_name(dbref player, const char *name, char *newname);
void do_chown(dbref player, const char *name, const char *newobj, int preserve);
static int chown_ok(dbref player, dbref thing, dbref newowner);
int do_chzone(dbref player, const char *name, const char *newobj, int noisy);
void do_attrib_flags
  (dbref player, const char *obj, const char *atrname, const char *flag);
int do_set(dbref player, const char *name, char *flag);
void do_cpattr
  (dbref player, char *oldpair, char **newpair, int move, int noflagcopy);
void do_gedit(dbref player, char *it, char **argv);
void do_trigger(dbref player, char *object, char **argv);
void do_use(dbref player, const char *what);
void do_parent(dbref player, char *name, char *parent_name);
void do_wipe(dbref player, char *name);
static int af_helper(dbref player, dbref thing, char const *pattern,
		     ATTR *atr, void *args);
static int gedit_helper(dbref player, dbref thing, char const *pattern,
			ATTR *atr, void *args);
static int wipe_helper(dbref player, dbref thing, char const *pattern,
		       ATTR *atr, void *args);
static void copy_attrib_flags(dbref player, dbref target, ATTR *atr, int flags);


/** Rename something.
 * \verbatim
 * This implements @name.
 * \endverbatim
 * \param player the enactor.
 * \param name current name of object to rename.
 * \param newname new name for object.
 */
void
do_name(dbref player, const char *name, char *newname)
{
  dbref thing;
  char *password;

  if ((thing = match_controlled(player, name)) != NOTHING) {
    /* check for bad name */
    if ((*newname == '\0') || strchr(newname, '[')) {
      notify(player, T("Give it what new name?"));
      return;
    }
    /* check for renaming a player */
    if (IsPlayer(thing)) {
      if (PLAYER_NAME_SPACES) {
	if (*newname == '\"') {
	  for (; *newname && ((*newname == '\"') || isspace(*newname));
	       newname++) ;
	  password = newname;
	  while (*password && (*password != '\"')) {
	    while (*password && (*password != '\"'))
	      password++;
	    if (*password == '\"') {
	      *password++ = '\0';
	      while (*password && isspace((unsigned char) *password))
		password++;
	      break;
	    }
	  }
	} else {
	  password = newname;
	  while (*password && !isspace((unsigned char) *password))
	    password++;
	  if (*password) {
	    *password++ = '\0';
	    while (*password && isspace((unsigned char) *password))
	      password++;
	  }
	}
      } else {

	/* split off password */
	for (password = newname + strlen(newname) - 1;
	     *password && !isspace((unsigned char) *password); password--) ;
	for (; *password && isspace((unsigned char) *password); password--) ;
	/* eat whitespace */
	if (*password) {
	  *++password = '\0';	/* terminate name */
	  password++;
	  while (*password && isspace((unsigned char) *password))
	    password++;
	}
      }
      if (strcasecmp(newname, Name(thing))
	  && !ok_player_name(newname)) {
	/* strcasecmp allows changing foo to Foo, etc. */
	notify(player, T("You can't give a player that name."));
	return;
      }
      /* everything ok, notify */
      do_log(LT_CONN, 0, 0, T("Name change by %s(#%d) to %s"),
	     Name(thing), thing, newname);
      if (Suspect(thing))
	flag_broadcast("WIZARD", 0,
		       T("Broadcast: Suspect %s changed name to %s."),
		       Name(thing), newname);
      delete_player(thing, NULL);
      set_name(thing, newname);
      add_player(thing, NULL);
      notify(player, T("Name set."));
      return;
    } else {
      if (!ok_name(newname)) {
	notify(player, T("That is not a reasonable name."));
	return;
      }
    }

    /* everything ok, change the name */
    set_name(thing, newname);
    notify(player, T("Name set."));
  }
}

/** Change an object's owner.
 * \verbatim
 * This implements @chown.
 * \endverbatim
 * \param player the enactor.
 * \param name name of object to change owner of.
 * \param newobj name of new owner for object.
 * \param preserve if 1, preserve privileges and don't halt the object.
 */
void
do_chown(dbref player, const char *name, const char *newobj, int preserve)
{
  dbref thing;
  dbref newowner = NOTHING;
  char *sp;
  long match_flags = MAT_POSSESSION | MAT_HERE | MAT_EXIT | MAT_ABSOLUTE;


  /* check for '@chown <object>/<atr>=<player>'  */
  sp = strchr(name, '/');
  if (sp) {
    do_atrchown(player, name, newobj);
    return;
  }
  if (Wizard(player))
    match_flags |= MAT_PLAYER;

  if ((thing = noisy_match_result(player, name, TYPE_THING, match_flags))
      == NOTHING)
    return;

  if (!*newobj || !strcasecmp(newobj, "me")) {
    newowner = player;
  } else {
    if ((newowner = lookup_player(newobj)) == NOTHING) {
      notify(player, T("I couldn't find that player."));
      return;
    }
  }

  if (IsPlayer(thing) && !God(player)) {
    notify(player, T("Players always own themselves."));
    return;
  }
  /* Permissions checking */
  if (!chown_ok(player, thing, newowner)) {
    notify(player, T("Permission denied."));
    return;
  }
  if (IsThing(thing) && !Hasprivs(player) &&
      !(GoodObject(Location(thing)) && (Location(thing) == player))) {
    notify(player, T("You must carry the object to @chown it."));
    return;
  }
  if (preserve && !Wizard(player)) {
    notify(player, T("You cannot @CHOWN/PRESERVE. Use normal @CHOWN."));
    return;
  }
  /* chowns to the zone master don't count towards fees */
  if (!ZMaster(newowner)) {
    /* Debit the owner-to-be */
    if (!can_pay_fees(newowner, Pennies(thing))) {
      /* not enough money or quota */
      if (newowner != player)
	notify(player,
	       T
	       ("That player doesn't have enough money or quota to receive that object."));
      return;
    }
    /* Credit the current owner */
    giveto(Owner(thing), Pennies(thing));
#ifdef QUOTA
    change_quota(Owner(thing), QUOTA_COST);
#endif
  }
  chown_object(player, thing, newowner, preserve);
  notify(player, T("Owner changed."));
}

static int
chown_ok(dbref player, dbref thing, dbref newowner)
{
  /* Cant' touch garbage */
  if (IsGarbage(thing))
    return 0;

  /* Wizards can do it all */
  if (Wizard(player))
    return 1;

  /* In order for non-wiz player to @chown thing to newowner,
   * player must control newowner or newowner must be a Zone Master
   * and player must pass its zone lock.
   *
   * In addition, one of the following must apply:
   *   1.  player owns thing, or
   *   2.  player controls Owner(thing), newowner is a zone master,
   *       and Owner(thing) passes newowner's zone-lock, or
   *   3.  thing is CHOWN_OK, and player holds thing if it's an object.
   *
   * The third condition is syntactic sugar to handle the situation
   * where Joe owns Box, an ordinary object, and Tool, an inherit object, 
   * and ZMP, a Zone Master Player, is zone-locked to =tool.
   * In this case, if Joe doesn't pass ZMP's lock, we don't want
   *   Joe to be able to @fo Tool=@chown Box=ZMP
   */

  /* Does player control newowner, or is newowner a Zone Master and player
   * passes the lock?
   */
  if (!(controls(player, newowner) ||
	(ZMaster(newowner) && eval_lock(player, newowner, Zone_Lock))))
    return 0;

  /* Target player is legitimate. Does player control the object? */
  if (Owns(player, thing))
    return 1;

  if (controls(player, Owner(thing)) &&
      ZMaster(newowner) && eval_lock(Owner(thing), newowner, Zone_Lock))
    return 1;

  if (ChownOk(thing) && (!IsThing(thing) || (Location(thing) == player)))
    return 1;

  return 0;
}


/* Actually change the ownership of something, and fix bits.
 * \param player the enactor.
 * \param thing object to change ownership of.
 * \param newowner new owner for thing.
 * \param preserve if 1, preserve privileges and don't halt.
 */
void
chown_object(dbref player, dbref thing, dbref newowner, int preserve)
{
  undestroy(player, thing);
  if (God(player)) {
    Owner(thing) = newowner;
  } else {
    Owner(thing) = Owner(newowner);
  }
  Zone(thing) = Zone(newowner);
  clear_flag_internal(thing, "CHOWN_OK");
  if (!preserve || !Wizard(player)) {
    clear_flag_internal(thing, "WIZARD");
    clear_flag_internal(thing, "ROYALTY");
    clear_flag_internal(thing, "TRUST");
    set_flag_internal(thing, "HALT");
    Powers(thing) = 0;		/* wipe out all powers */
    do_halt(thing, "", thing);
  } else {
    if ((newowner != player) && Wizard(thing) && !God(player)) {
      notify(player,
	     T
	     ("Warning: WIZ flag reset because @CHOWN/PRESERVE is to a third party."));
      clear_flag_internal(thing, "WIZARD");
    }
    if (Powers(thing) || Wizard(thing) || Royalty(thing) || Inherit(thing))
      notify(player,
	     T
	     ("Warning: @CHOWN/PRESERVE on a target with WIZ, ROY, INHERIT, or @power privileges."));
  }
}


/** Change an object's zone.
 * \verbatim
 * This implements @chzone.
 * \endverbatim
 * \param player the enactor.
 * \param name name of the object to change zone of.
 * \param newobj name of new ZMO.
 * \param noisy if 1, notify player about success and failure.
 * \retval 0 failed to change zone.
 * \retval 1 successfully changed zone.
 */
int
do_chzone(dbref player, char const *name, char const *newobj, int noisy)
{
  dbref thing;
  dbref zone;

  if ((thing = noisy_match_result(player, name, NOTYPE, MAT_NEARBY)) == NOTHING)
    return 0;

  if (!newobj || !*newobj || !strcasecmp(newobj, "none"))
    zone = NOTHING;
  else {
    if ((zone = noisy_match_result(player, newobj, NOTYPE, MAT_EVERYTHING))
	== NOTHING)
      return 0;
  }

  if (Zone(thing) == zone) {
    if (noisy)
      notify(player, T("That object is already in that zone."));
    return 0;
  }

  /* we do use ownership instead of control as a criterion because
   * we only want the owner to be able to rezone the object. Also,
   * this allows players to @chzone themselves to an object they own.
   */
  if (!Wizard(player) && !Owns(player, thing)) {
    if (noisy)
      notify(player, T("You don't have the power to shift reality."));
    return 0;
  }
  /* a player may change an object's zone to:
   * 1.  NOTHING 
   * 2.  an object he owns
   * 3.  an object with a chzone-lock that the player passes.
   * Note that an object with no chzone-lock isn't valid
   */
  if (!(Wizard(player) || (zone == NOTHING) || Owns(player, zone) ||
	((getlock(zone, Chzone_Lock) != TRUE_BOOLEXP) &&
	 eval_lock(player, zone, Chzone_Lock)))) {
    if (noisy)
      notify(player, T("You cannot move that object to that zone."));
    return 0;
  }
  /* Don't chzone object to itself for mortals! */
  if ((zone == thing) && !Hasprivs(player)) {
    if (noisy)
      notify(player, T("You shouldn't zone objects to themselves!"));
    return 0;
  }
  /* Don't allow circular zones */
  if (GoodObject(zone)) {
    dbref tmp;
    for (tmp = Zone(zone); GoodObject(tmp); tmp = Zone(tmp)) {
      if (tmp == thing) {
	notify(player, T("You can't make circular zones!"));
	return 0;
      }
      if (tmp == Zone(tmp))	/* Ran into an object zoned to itself */
	break;
    }
  }

  /* Don't allow chzone to objects without elocks! 
   * If no lock is set, set a default lock (warn if zmo are used for control)
   * This checks for many trivial elocks (canuse/1, where &canuse=1)
   */
  if (zone != NOTHING) {
    struct boolexp *key = getlock(zone, Zone_Lock);
    if (key == TRUE_BOOLEXP) {
      add_lock(GOD, zone, Zone_Lock, parse_boolexp(zone, "$me", Zone_Lock), -1);
      if (noisy && !ZONE_CONTROL_ZMP)
	notify(player,
	       T("Unlocked ZMO - automatically zone-locking ZMO to its owner"));
    } else if (eval_lock(Location(player), zone, Zone_Lock)) {
      /* Does #0 and #2 pass it? If so, probably trivial elock */
      if (eval_lock(PLAYER_START, zone, Zone_Lock) &&
	  eval_lock(MASTER_ROOM, zone, Zone_Lock)) {
	if (noisy)
	  notify(player,
		 T("ZMO requires a more secure zone-lock before you @chzone!"));
	return 0;
      }
      /* Probably inexact zone lock */
      notify(player,
	     T
	     ("Warning: ZMO may have loose zone lock. Lock ZMOs to =player, not player"));
    }
  }
  /* Warn Wiz/Royals when they zone their stuff */
  if ((zone != NOTHING) && Hasprivs(Owner(thing))) {
    if (noisy)
      notify(player, T("Warning: @chzoning admin-owned object!"));
  }
  /* everything is okay, do the change */
  Zone(thing) = zone;
  /* If we're not unzoning, and we're working with a non-player object,
   * we'll remove wizard, royalty, inherit, and powers, for security.
   */
  if ((zone != NOTHING) && !IsPlayer(thing)) {
    /* if the object is a player, resetting these flags is rather
     * inconvenient -- although this may pose a bit of a security
     * risk. Be careful when @chzone'ing wizard or royal players.
     */
    clear_flag_internal(thing, "WIZARD");
    clear_flag_internal(thing, "ROYALTY");
    clear_flag_internal(thing, "TRUST");
    Powers(thing) = 0;		/* wipe out all powers */
  } else {
    if (noisy && (zone != NOTHING)) {
      if (Hasprivs(thing))
	notify(player, T("Warning: @chzoning a privileged player."));
      if (Inherit(thing))
	notify(player, T("Warning: @chzoning an TRUST player."));
    }
  }
  if (noisy)
    notify(player, T("Zone changed."));
  return 1;
}

/** Structure for af_helper() data. */
struct af_args {
  int f;		/**< flag bits */
  char const *flag;	/**< flag name */
};

static int
af_helper(dbref player, dbref thing,
	  char const *pattern
	  __attribute__ ((__unused__)), ATTR *atr, void *args)
{
  struct af_args *af = args;

  /* We must be able to write to that attribute normally,
   * to prevent players from doing funky things to, say, LAST.
   * There is one special case - the resetting of the SAFE flag.
   */
  if (!(Can_Write_Attr(player, thing, AL_ATTR(atr)) ||
	((*af->flag == NOT_TOKEN) && (af->f == AF_SAFE) &&
	 Can_Write_Attr_Ignore_Safe(player, thing, AL_ATTR(atr))))) {
    notify_format(player, T("You cannot set %s/%s"), Name(thing), AL_NAME(atr));
    return 0;
  }
  if (*af->flag == NOT_TOKEN)
    AL_FLAGS(atr) &= ~af->f;
  else
    AL_FLAGS(atr) |= af->f;

  if (!Quiet(player) && !(Quiet(thing) && (Owner(thing) == player)))
    notify_format(player, "%s/%s - Set.", Name(thing), AL_NAME(atr));

  return 1;
}

static void
copy_attrib_flags(dbref player, dbref target, ATTR *atr, int flags)
{
  if (!atr)
    return;
  if (!Can_Write_Attr(player, target, AL_ATTR(atr))) {
    notify_format(player,
		  T("You cannot set attrib flags on %s/%s"), Name(target),
		  AL_NAME(atr));
    return;
  }
  AL_FLAGS(atr) = flags;
}

/** Set a flag on an attribute.
 * \param player the enactor.
 * \param obj the name of the object with the attribute.
 * \param atrname the name of the attribute.
 * \param flag the name of the flag to set or clear.
 */
void
do_attrib_flags(dbref player, const char *obj, const char *atrname,
		const char *flag)
{
  struct af_args af;
  dbref thing;
  const char *p;

  if ((thing = match_controlled(player, obj)) == NOTHING)
    return;

  if (!flag || !*flag) {
    notify(player, T("What flag do you want to set?"));
    return;
  }
  /* move past NOT token if there is one */
  for (p = flag; *p && ((*p == NOT_TOKEN) || isspace((unsigned char) *p));
       p++) ;

  if ((af.f = string_to_atrflag(player, p)) < 0) {
    notify(player, T("Unrecognized attribute flag."));
    return;
  }
  af.flag = flag;
  if (!atr_iter_get(player, thing, atrname, af_helper, &af))
    notify(player, T("No attribute found to change."));
}


/** Set a flag, attribute flag, or attribute.
 * \verbatim
 * This implements @set.
 * \endverbatim
 * \param player the enactor.
 * \param name the first (left) argument to the command.
 * \param flag the second (right) argument to the command.
 * \retval 1 successful set.
 * \retval 0 failure to set.
 */
int
do_set(dbref player, const char *name, char *flag)
{
  dbref thing;
  int her, listener;
  char *p;

  /* check for attribute flag set first */
  if ((p = strchr(name, '/')) != NULL) {
    *p++ = '\0';
    do_attrib_flags(player, name, p, flag);
    return 1;
  }
  /* find thing */
  if ((thing = match_controlled(player, name)) == NOTHING)
    return 0;
  if (God(thing) && !God(player)) {
    notify(player, T("Only God can set himself!"));
    return 0;
  }
  her = Hearer(thing);
  listener = Listener(thing);
  /* check for attribute set first */
  if ((p = strchr(flag, ':')) != NULL) {
    *p++ = '\0';
    if (!command_check_byname(player, "ATTRIB_SET")) {
      notify(player, T("You may not set attributes."));
      return 0;
    }
    return do_set_atr(thing, flag, p, player, 1);
  }
  /* we haven't set an attribute, so we must be setting a flag */

  /* move p past NOT_TOKEN if present */
  for (p = (char *) flag;
       *p && (*p == NOT_TOKEN || isspace((unsigned char) *p)); p++) ;

  /* identify flag */
  if (*p == '\0') {
    notify(player, T("You must specify a flag to set."));
    return 0;
  }
  set_flag(player, thing, p, (*flag == NOT_TOKEN) ? 1 : 0, her, listener);
  return 1;
}

/** Copy or move an attribute.
 * \verbatim
 * This implements @cpattr and @mvattr.
 * the command is of the format:
 * @cpattr oldobj/oldattr = newobj1/newattr1, newobj2/newattr2, etc.
 * \endverbatim
 * \param player the enactor.
 * \param oldpair the obj/attribute pair to copy from.
 * \param newpair array of obj/attribute pairs to copy to.
 * \param move if 1, move rather than copy.
 * \param noflagcopy if 1, don't copy associated flags.
 */
void
do_cpattr(dbref player, char *oldpair, char **newpair, int move, int noflagcopy)
{
  dbref oldobj, newobj;
  char tbuf1[BUFFER_LEN], tbuf2[BUFFER_LEN];
  int i;
  char *p, *q;
  ATTR *a;
  char *text;
  int copies = 0;

  /* must copy from something */
  if (!oldpair || !*oldpair) {
    notify(player, T("What do you want to copy from?"));
    return;
  }
  /* find the old object */
  strcpy(tbuf1, oldpair);
  p = strchr(tbuf1, '/');
  if (!p || !*p) {
    notify(player, T("What object do you want to copy the attribute from?"));
    return;
  }
  *p++ = '\0';
  oldobj = noisy_match_result(player, tbuf1, NOTYPE, MAT_EVERYTHING);
  if (!GoodObject(oldobj))
    return;

  strcpy(tbuf2, p);
  p = tbuf2;
  /* find the old attribute */
  a = atr_get_noparent(oldobj, strupper(p));
  if (!a) {
    notify(player, T("No such attribute to copy from."));
    return;
  }
  /* check permissions to get it */
  if (!Can_Read_Attr(player, oldobj, a)) {
    notify(player, T("Permission to read attribute denied."));
    return;
  }
  /* we can read it. Copy the value. */
  text = safe_uncompress(a->value);

  /* now we loop through our new object pairs and copy, calling @set. */
  for (i = 1; i < MAX_ARG && (newpair[i] != NULL); i++) {
    if (!*newpair[i]) {
      notify(player, T("What do you want to copy to?"));
    } else {
      strcpy(tbuf1, newpair[i]);
      q = strchr(tbuf1, '/');
      if (!q || !*q) {
	q = (char *) AL_NAME(a);
      } else {
	*q++ = '\0';
      }
      newobj = noisy_match_result(player, tbuf1, NOTYPE, MAT_EVERYTHING);
      if (GoodObject(newobj) &&
	  ((newobj != oldobj) || strcasecmp(AL_NAME(a), q)) &&
	  do_set_atr(newobj, q, text, player, 1))
	copies++;
      /* copy the attribute flags too */
      if (!noflagcopy)
	copy_attrib_flags(player, newobj,
			  atr_get_noparent(newobj, strupper(q)), a->flags);

    }
  }

  free((Malloc_t) text);	/* safe_uncompress malloc()s memory */
  if (copies) {
    notify_format(player, T("Attribute %s (%d copies)"),
		  (move ? "moved" : "copied"), copies);
    if (move)
      do_set_atr(oldobj, AL_NAME(a), NULL, player, 1);
  } else {
    notify_format(player, T("Unable to %s attribute."),
		  (move ? "move" : "copy"));
  }
  return;
}

static int
gedit_helper(dbref player, dbref thing,
	     char const *pattern
	     __attribute__ ((__unused__)), ATTR *a, void *args)
{
  int ansi_long_flag = 0;
  const char *r;
  char *s, *val;
  char tbuf1[BUFFER_LEN], tbuf_ansi[BUFFER_LEN];
  char *tbufp, *tbufap;
  char **argv = (char **) args;

  val = argv[1];
  r = (argv[2]) ? argv[2] : "";

  tbufp = tbuf1;
  tbufap = tbuf_ansi;

  if (!a) {			/* Shouldn't ever happen, but better safe than sorry */
    notify(player, T("No such attribute, try set instead."));
    return 0;
  }
  if (!Can_Write_Attr(player, thing, a)) {
    notify(player, T("You need to control an attribute to edit it."));
    return 0;
  }
  s = (char *) uncompress(a->value);	/* warning: pointer to static buffer */

  if (!strcmp(val, "$")) {
    /* append */
    safe_str(s, tbuf1, &tbufp);
    safe_str(r, tbuf1, &tbufp);

    if (safe_format(tbuf_ansi, &tbufap, "%s%s%s%s", s, ANSI_HILITE, r,
		    ANSI_NORMAL))
      ansi_long_flag = 1;
  } else if (!strcmp(val, "^")) {
    /* prepend */
    safe_str(r, tbuf1, &tbufp);
    safe_str(s, tbuf1, &tbufp);

    if (safe_format(tbuf_ansi, &tbufap, "%s%s%s%s", ANSI_HILITE, r, ANSI_NORMAL,
		    s))
      ansi_long_flag = 1;
  } else {
    /* find and replace */
    char *p, *start = s;
    size_t vlen;
    int too_long = 0;

    /* This will have problems if val is ever "", but do_gedit makes sure
     *  it isn't.
     */

    vlen = strlen(val);

    while (*s && (p = strstr(s, val)) != NULL) {
      /* Copy from start to p */
      *p = '\0';
      if (safe_str(start, tbuf1, &tbufp)) {
	too_long = 1;
	break;
      }
      if (!ansi_long_flag) {
	if (safe_str(start, tbuf_ansi, &tbufap))
	  ansi_long_flag = 1;
      }
      s = p + vlen;
      start = s;
      /* Copy in r */
      if (safe_str(r, tbuf1, &tbufp)) {
	too_long = 1;
	break;
      }
      if (!ansi_long_flag) {
	if (safe_format(tbuf_ansi, &tbufap, "%s%s%s", ANSI_HILITE, r,
			ANSI_NORMAL))
	  ansi_long_flag = 1;
      }
    }
    /* No more val's in the string */
    if (*s && !too_long) {
      safe_str(s, tbuf1, &tbufp);
      if (!ansi_long_flag) {
	if (safe_str(s, tbuf_ansi, &tbufap))
	  ansi_long_flag = 1;
      }
    }
  }

  *tbufp = '\0';
  *tbufap = '\0';

  if (do_set_atr(thing, AL_NAME(a), tbuf1, player, 0) &&
      !Quiet(player) && !Quiet(thing)) {
    if (!ansi_long_flag && ShowAnsi(player))
      notify_format(player, "%s - Set: %s", AL_NAME(a), tbuf_ansi);
    else
      notify_format(player, "%s - Set: %s", AL_NAME(a), tbuf1);
  }

  return 1;
}

/** Edit an attribute.
 * \verbatim
 * This implements @edit obj/attribute = {search}, {replace}
 * \endverbatim
 * \param player the enactor.
 * \param it the object/attribute pair.
 * \param argv array containing the search and replace strings.
 */
void
do_gedit(dbref player, char *it, char **argv)
{
  dbref thing;
  char tbuf1[BUFFER_LEN];
  char *q;

  if (!(it && *it)) {
    notify(player, T("I need to know what you want to edit."));
    return;
  }
  strcpy(tbuf1, it);
  q = strchr(tbuf1, '/');
  if (!(q && *q)) {
    notify(player, T("I need to know what you want to edit."));
    return;
  }
  *q++ = '\0';
  thing = noisy_match_result(player, tbuf1, NOTYPE, MAT_EVERYTHING);

  if ((thing == NOTHING) || !controls(player, thing)) {
    notify(player, T("Permission denied."));
    return;
  }

  if (!argv[1] || !*argv[1]) {
    notify(player, T("Nothing to do."));
    return;
  }
  if (!atr_iter_get(player, thing, q, gedit_helper, argv))
    notify(player, T("No matching attributes."));
}

/** Trigger an attribute.
 * \verbatim
 * This implements @trigger obj/attribute = list-of-arguments.
 * \endverbatim
 * \param player the enactor.
 * \param object the object/attribute pair.
 * \param argv array of arguments.
 */
void
do_trigger(dbref player, char *object, char **argv)
{
  dbref thing;
  int a;
  char *s;
  char tbuf1[BUFFER_LEN];

  strcpy(tbuf1, object);
  for (s = tbuf1; *s && (*s != '/'); s++) ;
  if (!*s) {
    notify(player, T("I need to know what attribute to trigger."));
    return;
  }
  *s++ = '\0';

  thing = noisy_match_result(player, tbuf1, NOTYPE, MAT_EVERYTHING);

  if (thing == NOTHING)
    return;

  if (!controls(player, thing) && !(Owns(player, thing) && LinkOk(thing))) {
    notify(player, T("Permission denied."));
    return;
  }
  if (God(thing) && !God(player)) {
    notify(player, T("You can't trigger God!"));
    return;
  }
  /* trigger modifies the stack */
  for (a = 0; a < 10; a++)
    wnxt[a] = argv[a + 1];

  if (charge_action(player, thing, upcasestr(s))) {
    if (!Quiet(player) && !Quiet(thing))
      notify_format(player, "%s - Triggered.", Name(thing));
  } else {
    notify(player, T("No such attribute."));
  }
}


/** The use command.
 * It's here for lack of a better place.
 * \param player the enactor.
 * \param what name of the object to use.
 */
void
do_use(dbref player, const char *what)
{
  dbref thing;

  /* if we pass the use key, do it */

  if ((thing =
       noisy_match_result(player, what, TYPE_THING,
			  MAT_NEAR_THINGS)) != NOTHING) {
    if (!eval_lock(player, thing, Use_Lock)) {
      did_it(player, thing, "UFAIL", T("Pemission denied."), "OUFAIL", NULL,
	     "AUFAIL", NOTHING);
      return;
    } else
      did_it(player, thing, "USE", "Used.", "OUSE", NULL, "AUSE", NOTHING);
  }
}

/** Parent an object to another.
 * \verbatim
 * This implements @parent.
 * \endverbatim
 * \param player the enactor.
 * \param name the name of the child object.
 * \param parent_name the name of the new parent object.
 */
void
do_parent(dbref player, char *name, char *parent_name)
{
  dbref thing;
  dbref parent;
  dbref check;
  int i;

  if ((thing = noisy_match_result(player, name, NOTYPE, MAT_NEARBY)) == NOTHING)
    return;

  if (!parent_name || !*parent_name || !strcasecmp(parent_name, "none"))
    parent = NOTHING;
  else if ((parent = noisy_match_result(player, parent_name, NOTYPE,
					MAT_EVERYTHING)) == NOTHING)
    return;

  /* do control check */
  if (!controls(player, thing) && !(Owns(player, thing) && LinkOk(thing))) {
    notify(player, T("Permission denied."));
    return;
  }
  /* a player may change an object's parent to NOTHING or to an 
   * object he owns, or one that is LINK_OK when the player passes
   * the parent lock
   * mod: also when the player controls the parent, it passes the parent lock
   * [removed owner and wizard check and added
   * control check (wich does those things
   * anyway, right?)]
   */
  if ((parent != NOTHING) && !controls(player, parent) &&
      !(LinkOk(parent) && eval_lock(player, parent, Parent_Lock))) {
    notify(player, T("Permission denied."));
    return;
  }
  /* check to make sure no recursion can happen */
  if (parent == thing) {
    notify(player, T("A thing cannot be its own ancestor!"));
    return;
  }
  if (parent != NOTHING) {
    for (i = 0, check = Parent(parent);
	 (i < MAX_PARENTS) && (check != NOTHING); i++, check = Parent(check)) {
      if (check == thing) {
	notify(player, T("You are not allowed to be your own ancestor!"));
	return;
      }
    }
    if (i >= MAX_PARENTS) {
      notify(player, T("Too many ancestors."));
      return;
    }
  }
  /* everything is okay, do the change */
  Parent(thing) = parent;
  notify(player, T("Parent changed."));
}

static int
wipe_helper(dbref player, dbref thing, char const *pattern,
	    ATTR *atr, void *args __attribute__ ((__unused__)))
{
  /* for added security, only God can modify wiz-only-modifiable
   * attributes using this command and wildcards.  Wiping a specific
   * attr still works, though.
   */
  if (wildcard(pattern) && (AL_FLAGS(atr) & AF_WIZARD) && !God(player))
    return 0;
  return do_set_atr(thing, AL_NAME(atr), NULL, player, 0) == 1;
}

/** Clear an attribute.
 * \verbatim
 * This implements @wipe.
 * \endverbatim
 * \param player the enactor.
 * \param name the object/attribute-pattern to wipe.
 */
void
do_wipe(dbref player, char *name)
{
  dbref thing;
  char *pattern;

  if ((pattern = strchr(name, '/')) != NULL)
    *pattern++ = '\0';

  if ((thing = noisy_match_result(player, name, NOTYPE, MAT_NEARBY)) == NOTHING)
    return;

  /* this is too destructive of a command to be used by someone who
   * doesn't own the object. Thus, the check is on Owns not controls.
   */
  if (!Wizard(player) && !Owns(player, thing)) {
    notify(player, T("Permission denied."));
    return;
  }
  /* protect SAFE objects unless doing a non-wildcard pattern */
  if (Safe(thing) && !(pattern && *pattern && !wildcard(pattern))) {
    notify(player, T("That object is protected."));
    return;
  }
  if (!atr_iter_get(player, thing, pattern, wipe_helper, NULL))
    notify(player, T("No matching attributes."));
  else
    notify(player, T("Attributes wiped."));
}

COMMAND (cmd_with) {
  dbref what;

  what = noisy_match_result(player, arg_left, NOTYPE, MAT_NEARBY);
  if (!GoodObject(what))
    return;

  if (!atr_comm_match(what, player, '$', ':', arg_right, 0, NULL, NULL))
    notify(player, T("No matching command."));
}
