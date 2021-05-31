/* set.c */

#include "copyrite.h"
#include "config.h"

/* commands which set parameters */
#include <stdio.h>
#include <ctype.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif

#include "conf.h"
#ifdef WIN32
#include "shs.h"
#endif
#include "mushdb.h"
#include "match.h"
#include "intrface.h"
#include "externs.h"
#include "ansi.h"
#include "command.h"
#include "mymalloc.h"
#include "confmagic.h"

extern void set_flag _((dbref player, dbref thing, char *flag, int negate, int hear, int listener));	/* from flags.c */

extern void delete_player _((dbref player, char *alias));
extern void add_player _((dbref player, char *alias));
extern void charge_action _((dbref player, dbref thing, const char *awhat));
void do_name _((dbref player, const char *name, char *newname));
void do_chown _((dbref player, const char *name, const char *newobj));
static int chown_ok _((dbref player, dbref thing, dbref newowner));
void chown_object _((dbref player, dbref thing, dbref newowner));
void do_chzone _((dbref player, const char *name, const char *newobj));
void do_attrib_flags _((dbref player, const char *obj, const char *atrname, const char *flag));
int do_set _((dbref player, const char *name, char *flag));
void do_cpattr _((dbref player, char *oldpair, char **newpair, int move));
void do_gedit _((dbref player, char *it, char **argv));
void do_edit _((dbref player, dbref thing, char *q, char **argv));
void do_trigger _((dbref player, char *object, char **argv));
void do_use _((dbref player, const char *what));
void do_parent _((dbref player, char *name, char *parent_name));
void do_wipe _((dbref player, char *name));
static int af_helper _((dbref player, dbref thing, char const *pattern,
			ATTR *atr, void *args));
static int gedit_helper _((dbref player, dbref thing, char const *pattern,
			   ATTR *atr, void *args));
static int wipe_helper _((dbref player, dbref thing, char const *pattern,
			  ATTR *atr, void *args));
static void copy_attrib_flags _((dbref player, dbref target, ATTR *atr, int flags));

void
do_name(player, name, newname)
    dbref player;
    const char *name;
    char *newname;
{
  dbref thing;
  char *password;
  char tbuf1[BUFFER_LEN];

  if ((thing = match_controlled(player, name)) != NOTHING) {
    /* check for bad name */
    if ((*newname == '\0') || index(newname, '[')) {
      notify(player, "Give it what new name?");
      return;
    }
    /* check for renaming a player */
    if (IsPlayer(thing)) {
      if (Guest(player)) {
	notify(player, "Guests may not rename themselves.");
	return;
      }
      if (PLAYER_NAME_SPACES) {
	if (*newname == '\"') {
	  for (; *newname && ((*newname == '\"') || isspace(*newname)); newname++) ;
	  password = newname;
	  while (*password && (*password != '\"')) {
	    while (*password && (*password != '\"'))
	      password++;
	    if (*password == '\"') {
	      *password++ = '\0';
	      while (*password && isspace(*password))
		password++;
	      break;
	    }
	  }
	} else {
	  password = newname;
	  while (*password && !isspace(*password))
	    password++;
	  *password++ = '\0';
	  while (*password && isspace(*password))
	    password++;
	}
      } else {

	/* split off password */
	for (password = newname + strlen(newname) - 1;
	     *password && !isspace(*password);
	     password--) ;
	for (; *password && isspace(*password); password--) ;
	/* eat whitespace */
	if (*password) {
	  *++password = '\0';	/* terminate name */
	  password++;
	  while (*password && isspace(*password))
	    password++;
	}
      }
      /* check for null password */
      if (!God(player) && !*password) {
	notify(player,
	       "You must specify a password to change a player name.");
	notify(player, "E.g.: name player = newname password");
	return;
      } else if (!God(player) && !password_check(thing, password)) {
	notify(player, "Incorrect password.");
	return;
      } else if (strcasecmp(newname, Name(thing))
		 && !ok_player_name(newname)) {
	/* strcasecmp allows changing foo to Foo, etc. */
	notify(player, "You can't give a player that name.");
	return;
      }
      /* everything ok, notify */
      do_log(LT_CONN, 0, 0, "Name change by %s(#%d) to %s",
	     Name(thing), thing, newname);
      fflush(connlog_fp);
      if (Suspect(thing))
	flag_broadcast(WIZARD, 0, "Broadcast: Suspect %s changed name to %s.",
		       Name(thing), newname);
      if (USE_RWHO) {
	sprintf(tbuf1, "%d@%s", thing, MUDNAME);
	rwhocli_userlogout(tbuf1);
      }
      delete_player(thing, NULL);
      SET(Name(thing), newname);
      add_player(thing, NULL);
      if (USE_RWHO)
	rwhocli_userlogin(tbuf1, newname, time((time_t *) 0));
      notify(player, "Name set.");
      return;
    } else {
      if (!ok_name(newname)) {
	notify(player, "That is not a reasonable name.");
	return;
      }
    }

    /* everything ok, change the name */
    SET(Name(thing), newname);
    notify(player, "Name set.");
  }
}

void
do_chown(player, name, newobj)
    dbref player;
    const char *name;
    const char *newobj;
{
  dbref thing;
  dbref newowner = NOTHING;
  char *sp;
  long match_flags = MAT_POSSESSION | MAT_HERE | MAT_EXIT;


  /* check for '@chown <object>/<atr>=<player>'  */
  sp = (char *) index(name, '/');
  if (sp) {
    do_atrchown(player, name, newobj);
    return;
  }
  if (Wizard(player)) {
    match_flags |= MAT_PLAYER | MAT_ABSOLUTE;
  } else if (Hasprivs(player)) {
    match_flags |= MAT_ABSOLUTE;
  }
  if ((thing = noisy_match_result(player, name, TYPE_THING, match_flags)) == NOTHING)
    return;

  if (!*newobj || !strcasecmp(newobj, "me")) {
    newowner = player;
  } else {
    if ((newowner = lookup_player(newobj)) == NOTHING) {
      notify(player, "I couldn't find that player.");
      return;
    }
  }

  if (IsPlayer(thing) && !God(player)) {
    notify(player, "Players always own themselves.");
    return;
  }
  /* Permissions checking */
  if (!chown_ok(player, thing, newowner)) {
    notify(player, "Permission denied.");
    return;
  }
  /* chowns to the zone master don't count towards fees */
  if (!ZMaster(newowner)) {
    if (!can_pay_fees(player, OBJECT_DEPOSIT(Pennies(thing))))	/* not enough money or quota */
      return;
    giveto(Owner(thing), OBJECT_DEPOSIT(Pennies(thing)));
#ifdef QUOTA
    change_quota(Owner(thing), QUOTA_COST);
#endif
  }
  chown_object(player, thing, newowner);
  notify(player, "Owner changed.");
}

static int
chown_ok(player, thing, newowner)
    dbref player;
    dbref thing;
    dbref newowner;
{
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

  if (ChownOk(thing) &&
      (!IsThing(thing) || (Location(thing) == player)))
    return 1;

  return 0;
}


/* Actually change the ownership of something, and fix bits */
void
chown_object(player, thing, newowner)
    dbref player;
    dbref thing;
    dbref newowner;
{
  undestroy(player, thing);
  if (God(player)) {
    Owner(thing) = newowner;
  } else {
    Owner(thing) = Owner(newowner);
  }
  Zone(thing) = Zone(newowner);
  Flags(thing) &= ~CHOWN_OK;
  Flags(thing) &= ~WIZARD;
#ifdef ROYALTY_FLAG
  Flags(thing) &= ~ROYALTY;
#endif
  Flags(thing) &= ~INHERIT;
  Flags(thing) |= HALT;
  Powers(thing) = 0;		/* wipe out all powers */
  do_halt(thing, "", thing);
}


void
do_chzone(player, name, newobj)
    dbref player;
    const char *name;
    const char *newobj;
{
  dbref thing;
  dbref zone;

  if ((thing = noisy_match_result(player, name, NOTYPE, MAT_NEARBY)) == NOTHING)
    return;

  if (!strcasecmp(newobj, "none"))
    zone = NOTHING;
  else {
    if ((zone = noisy_match_result(player, newobj, NOTYPE, MAT_EVERYTHING)) == NOTHING)
      return;

    if (!IsThing(zone)
	&& (!DO_GLOBALS || !IsRoom(zone))) {
      notify(player, "Invalid zone object type.");
      return;
    }
  }

  /* we do use ownership instead of control as a criterion because
   * we only want the owner to be able to rezone the object. Also,
   * this allows players to @chzone themselves to an object they own.
   */
  if (!Wizard(player) && !Owns(player, thing)) {
    notify(player, "You don't have the power to shift reality.");
    return;
  }
  /* a player may change an object's zone to NOTHING or to an object he owns */
  if ((zone != NOTHING) && !Wizard(player) &&
      !Owns(player, zone)) {
    notify(player, "You cannot move that object to that zone.");
    return;
  }
  /* only rooms may be zoned to other rooms */
  if ((zone != NOTHING) &&
      IsRoom(zone) && !IsRoom(thing)) {
    notify(player, "Only rooms may have zone master rooms.");
    return;
  }
  /* Don't chzone object to itself for mortals! */
  if ((zone == thing) && !Hasprivs(player)) {
    notify(player, "You shouldn't zone objects to themselves!");
    return;
  }
  /* Don't allow chzone to objects without elocks! 
   * This checks for many trivial elocks (canuse/1, where &canuse=1)
   */
  if (zone != NOTHING) {
    struct boolexp *key = getlock(zone, Zone_Lock);
    if (key == TRUE_BOOLEXP) {
      notify(player, "ZMOs must be zone-locked before you @chzone!");
      return;
    }
    /* Does the player's location pass it? If so, we have either
     * an inexact or trivial elock */
    if (eval_lock(Location(player), zone, Zone_Lock)) {
      /* Does #0 and #2 pass it? If so, probably trivial elock */
      if (eval_lock(PLAYER_START, zone, Zone_Lock) &&
	  eval_lock(MASTER_ROOM, zone, Zone_Lock)) {
	notify(player, "ZMO requires a more secure zone-lock before you @chzone!");
	return;
      }
      /* Probably inexact zone lock */
      notify(player, "Warning: ZMO may have loose zone lock. Lock ZMOs to =player, not player");
    }
  }
  /* Warn Wiz/Royals when they zone their stuff */
  if ((zone != NOTHING) && Hasprivs(Owner(thing))) {
    notify(player, "Warning: @chzoning admin-owned object!");
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
    Flags(thing) &= ~WIZARD;
#ifdef ROYALTY_FLAG
    Flags(thing) &= ~ROYALTY;
#endif
    Flags(thing) &= ~INHERIT;
    Powers(thing) = 0;		/* wipe out all powers */
  } else {
    if (zone != NOTHING) {
      if Hasprivs
	(thing)
	  notify(player, "Warning: @chzoning a privileged player.");
      if (Flags(thing) & INHERIT)
	notify(player, "Warning: @chzoning an INHERIT player.");
    }
  }
  notify(player, "Zone changed.");
}

struct af_args {
  int f;
  char const *flag;
};

static int
af_helper(player, thing, pattern, atr, args)
    dbref player;
    dbref thing;
    char const *pattern;
    ATTR *atr;
    void *args;
{
  struct af_args *af = args;

  /* We must be able to write to that attribute normally,
   * to prevent players from doing funky things to, say, LAST.
   */
  if (!Can_Write_Attr(player, thing, AL_ATTR(atr))) {
    notify(player, tprintf("You cannot set %s/%s", Name(thing), AL_NAME(atr)));
    return 0;
  }
  if (*af->flag == NOT_TOKEN)
    AL_FLAGS(atr) &= ~af->f;
  else
    AL_FLAGS(atr) |= af->f;

  if (!Quiet(player) && !Quiet(thing))
    notify(player, tprintf("%s/%s - Set.", Name(thing), AL_NAME(atr)));

  return 1;
}

static void
copy_attrib_flags(player, target, atr, flags)
    dbref player;
    dbref target;
    ATTR *atr;
    int flags;
{
  if (!atr)
    return;
  if (!Can_Write_Attr(player, target, AL_ATTR(atr))) {
    notify(player, tprintf("You cannot set attrib flags on %s/%s", Name(target), AL_NAME(atr)));
    return;
  }
  /* Must preserve AF_STATIC */
  if (AL_FLAGS(atr) & AF_STATIC)
    AL_FLAGS(atr) = flags | AF_STATIC;
  else
    AL_FLAGS(atr) = flags;
}

void
do_attrib_flags(player, obj, atrname, flag)
    dbref player;
    const char *obj;
    const char *atrname;
    const char *flag;
{
  struct af_args af;
  dbref thing;
  const char *p;

  if ((thing = match_controlled(player, obj)) == NOTHING)
    return;

  if (!flag || !*flag) {
    notify(player, "What flag do you want to set?");
    return;
  }
  /* move past NOT token if there is one */
  for (p = flag; *p && ((*p == NOT_TOKEN) || isspace(*p)); p++) ;

  if ((af.f = string_to_atrflag(player, p)) < 0) {
    notify(player, "Unrecognized attribute flag.");
    return;
  }
  af.flag = flag;
  (void) atr_iter_get(player, thing, atrname, af_helper, &af);
}


int
do_set(player, name, flag)
    dbref player;
    const char *name;
    char *flag;
{
  dbref thing;
  int her, listener;
  char *p;

  /* check for attribute flag set first */
  if ((p = (char *) index(name, '/')) != NULL) {
    *p++ = '\0';
    do_attrib_flags(player, name, p, flag);
    return 1;
  }
  /* find thing */
  if ((thing = match_controlled(player, name)) == NOTHING)
    return 0;
  if (God(thing) && !God(player)) {
    notify(player, "Only God can set himself!");
    return 0;
  }
  her = Hearer(thing);
  listener = Listener(thing);
  /* check for attribute set first */
  if ((p = (char *) index(flag, ':')) != NULL) {
    *p++ = '\0';
    if (!command_check_byname(player, "ATTRIB_SET")) {
      notify(player, "You may not set attributes.");
      return 0;
    }
    return do_set_atr(thing, flag, p, player, 1);
  }
  /* we haven't set an attribute, so we must be setting a flag */

  /* move p past NOT_TOKEN if present */
  for (p = (char *) flag; *p && (*p == NOT_TOKEN || isspace(*p)); p++) ;

  /* identify flag */
  if (*p == '\0') {
    notify(player, "You must specify a flag to set.");
    return 0;
  }
  set_flag(player, thing, p, (*flag == NOT_TOKEN) ? 1 : 0, her, listener);
  return 1;
}

void
do_cpattr(player, oldpair, newpair, move)
    dbref player;
    char *oldpair;
    char **newpair;
    int move;
{
  /* the command is of the format:
   * @cpattr oldobj/oldattr = newobj1/newattr1, newobj2/newattr2, etc.
   * If move = 1, it's @mvattr, erase original attrib.
   */

  dbref oldobj, newobj;
  char tbuf1[BUFFER_LEN], tbuf2[BUFFER_LEN];
  int i;
  char *p, *q;
  ATTR *a;
  char *text;
  int copies = 0;

  /* must copy from something */
  if (!oldpair || !*oldpair) {
    notify(player, "What do you want to copy from?");
    return;
  }
  /* find the old object */
  strcpy(tbuf1, oldpair);
  p = (char *) index(tbuf1, '/');
  if (!p || !*p) {
    notify(player, "What object do you want to copy the attribute from?");
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
    notify(player, "No such attribute to copy from.");
    return;
  }
  /* check permissions to get it */
  if (!Can_Read_Attr(player, oldobj, a)) {
    notify(player, "Permission to read attribute denied.");
    return;
  }
  /* we can read it. Copy the value. */
  text = safe_uncompress(a->value);

  /* now we loop through our new object pairs and copy, calling @set. */
  for (i = 1; i < MAX_ARG && (newpair[i] != NULL); i++) {
    if (!*newpair[i]) {
      notify(player, "What do you want to copy to?");
    } else {
      strcpy(tbuf1, newpair[i]);
      q = (char *) index(tbuf1, '/');
      if (!q || !*q) {
	q = (char *) AL_NAME(a);
      } else {
	*q++ = '\0';
      }
      newobj = noisy_match_result(player, tbuf1, NOTYPE, MAT_EVERYTHING);
      if (GoodObject(newobj) && do_set_atr(newobj, q, text, player, 1)) {
	copies++;
	/* copy the attribute flags too */
	copy_attrib_flags(player, newobj,
			atr_get_noparent(newobj, strupper(q)), a->flags);
      }
    }
  }

  free((Malloc_t) text);	/* safe_uncompress malloc()s memory */
  if (move) {
    if (copies) {
      do_set_atr(oldobj, p, NULL, player, 1);
      notify(player, tprintf("Attribute moved (%d copies)", copies));
    } else {
      notify(player, "Unable to move attribute.");
    }
  } else {
    notify(player, tprintf("Attributes copied (%d copies)", copies));
  }
  return;
}

static int
gedit_helper(player, thing, pattern, atr, args)
    dbref player;
    dbref thing;
    char const *pattern;
    ATTR *atr;
    void *args;
{
  do_edit(player, thing, (char *) AL_NAME(atr), (char **) args);
  return 1;
}

void
do_gedit(player, it, argv)
    dbref player;
    char *it;
    char **argv;
{
  dbref thing;
  char tbuf1[BUFFER_LEN];
  char *q;

  if (!(it && *it)) {
    notify(player, "I need to know what you want to edit.");
    return;
  }
  strcpy(tbuf1, it);
  q = (char *) index(tbuf1, '/');
  if (!(q && *q)) {
    notify(player, "I need to know what you want to edit.");
    return;
  }
  *q++ = '\0';
  thing = noisy_match_result(player, tbuf1, NOTYPE, MAT_EVERYTHING);

  if ((thing == NOTHING) || !controls(player, thing)) {
    notify(player, "Permission denied.");
    return;
  }
  if (!argv[1] || !*argv[1]) {
    notify(player, "Nothing to do.");
    return;
  }
  (void) atr_iter_get(player, thing, q, gedit_helper, argv);
}

void
do_edit(player, thing, q, argv)
    dbref player;
    dbref thing;
    char *q;
    char **argv;		/* attribute name */
{
  int d, len, ansi_d, ansi_long_flag = 0;
  ATTR *a;
  char *r, *s, *val;
  char tbuf1[BUFFER_LEN], tbuf_ansi[BUFFER_LEN];

  if (God(thing) && !God(player)) {
    notify(player, "Only God can edit himself!");
    return;
  }
  val = argv[1];
  r = (argv[2]) ? argv[2] : (char *) "";

  a = atr_get_noparent(thing, q);
  if (!a) {
    notify(player, "No such attribute, try set instead.");
    return;
  }
  if ((a->flags & AF_LOCKED) && (Owner(player) != Owner(a->creator))) {
    notify(player, "You need to control an attribute to edit it.");
    return;
  }
  s = (char *) uncompress(a->value);	/* warning: pointer to static buffer */

  if (!strcmp(val, "$")) {
    /* append */
    strcpy(tbuf1, s);
    if (strlen(s) + strlen(r) < BUFFER_LEN)
      strcat(tbuf1, r);
    if (strlen(tbuf1) + strlen(ANSI_HILITE) + strlen(ANSI_NORMAL) < BUFFER_LEN) {
      sprintf(tbuf_ansi, "%s%s%s%s", s, ANSI_HILITE, r, ANSI_NORMAL);
    } else {
      strcpy(tbuf_ansi, tbuf1);
    }

  } else if (!strcmp(val, "^")) {
    /* prepend */
    strcpy(tbuf1, r);
    strncat(tbuf1, s, BUFFER_LEN - strlen(r) - 1);
    if (strlen(tbuf1) + strlen(ANSI_HILITE) + strlen(ANSI_NORMAL) <
	BUFFER_LEN) {
      sprintf(tbuf_ansi, "%s%s%s%s", ANSI_HILITE, r, ANSI_NORMAL, s);
    } else {
      strcpy(tbuf_ansi, tbuf1);
    }
  } else {
    /* find and replace */
    len = strlen(val);
    for (d = 0, ansi_d = 0; (d < BUFFER_LEN) && *s;)
      if (strncmp(val, s, len) == 0) {
	if ((d + strlen(r)) < BUFFER_LEN) {
	  strcpy(tbuf1 + d, r);
	  d += strlen(r);
	  if ((ansi_d + strlen(r) + strlen(ANSI_HILITE) + strlen(ANSI_NORMAL)) < BUFFER_LEN) {
	    sprintf(tbuf_ansi + ansi_d, "%s%s%s", ANSI_HILITE, r, ANSI_NORMAL);
	    ansi_d += strlen(r) + strlen(ANSI_HILITE) + strlen(ANSI_NORMAL);
	  } else {
	    /* Hmm. What here? We can't assume that r will still fit. */
	    /* For now, let's just give up. After all, how often is this
	     * going to arise, anyway? */
	    if (!ansi_long_flag) {
	      ansi_long_flag = 1;
	      strcpy(tbuf_ansi, "Sorry, that attribute became too long to display. Examine it to see its new value.");
	      /* Not informative, but at least not a lie. */
	    }
	  }
	  s += len;
	} else {
	  break;
	}
      } else {
	tbuf1[d++] = tbuf_ansi[ansi_d++] = *s++;
      }
    tbuf1[d++] = 0;
    tbuf_ansi[ansi_d++] = '\0';
  }

  if (do_set_atr(thing, a->name, tbuf1, player, 0) &&
      !Quiet(player) && !Quiet(thing)) {
    if (!ansi_long_flag && ShowAnsi(player))
      notify(player, tprintf("%s - Set: %s", a->name, tbuf_ansi));
    else
      notify(player, tprintf("%s - Set: %s", a->name, tbuf1));
  }
}

void
do_trigger(player, object, argv)
    dbref player;
    char *object;
    char **argv;
{
  dbref thing;
  int a;
  char *s;
  char tbuf1[BUFFER_LEN];

  strcpy(tbuf1, object);
  for (s = tbuf1; *s && (*s != '/'); s++) ;
  if (!*s) {
    notify(player, "I need to know what attribute to trigger.");
    return;
  }
  *s++ = '\0';

  thing = noisy_match_result(player, tbuf1, NOTYPE, MAT_EVERYTHING);

  if (thing == NOTHING)
    return;

  if (!controls(player, thing) && !(Owns(player, thing) && LinkOk(thing))) {
    notify(player, "Permission denied.");
    return;
  }
  if (God(thing) && !God(player)) {
    notify(player, "You can't trigger God!");
    return;
  }
  /* trigger modifies the stack */
  for (a = 0; a < 10; a++)
    wnxt[a] = argv[a + 1];

  charge_action(player, thing, upcasestr(s));
  if (!Quiet(player) && !Quiet(thing))
    notify(player, tprintf("%s - Triggered.", Name(thing)));
}


/* for lack of a better place, the use code is here */

void
do_use(player, what)
    dbref player;
    const char *what;
{
  dbref thing;

  /* if we pass the use key, do it */

  if ((thing = noisy_match_result(player, what, TYPE_THING, MAT_NEAR_THINGS)) != NOTHING) {
    if (!eval_lock(player, thing, Use_Lock)) {
      did_it(player, thing, "UFAIL", "Pemission denied.", "OUFAIL", NULL, "AUFAIL", NOTHING);
      return;
    } else
      did_it(player, thing, "USE", "Used.", "OUSE", NULL, "AUSE", NOTHING);
  }
}

void
do_parent(player, name, parent_name)
    dbref player;
    char *name;
    char *parent_name;
{
  dbref thing;
  dbref parent;
  dbref check;
  int i;

  if ((thing = noisy_match_result(player, name, NOTYPE, MAT_NEARBY)) == NOTHING)
    return;

#ifdef NEVER
  /* in the old days, players could not be parented */
  if (IsRoom(thing)) {
    notify(player, "Don't you like your biological parents?");
    return;
  }
#endif
#ifdef TREKMUSH
  if (Typeof(thing) == TYPE_PLAYER && !Wizard(player)) {
    notify(player, "Don't you like your species?");
    return;
  }
#endif /* TREKMUSH */
  if (!parent_name || !*parent_name || !strcasecmp(parent_name, "none"))
    parent = NOTHING;
  else {
    if ((parent = noisy_match_result(player, parent_name, NOTYPE, MAT_EVERYTHING)) == NOTHING)
      return;
  }

  /* do control check */
  if (!controls(player, thing) && !(Owns(player, thing) && LinkOk(thing))) {
    notify(player, "Permission denied.");
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
  if ((parent != NOTHING) &&
      !controls(player, parent) &&
      !(LinkOk(parent)
	&& eval_lock(player, parent, Parent_Lock)
      )) {
    notify(player, "Permission denied.");
    return;
  }
  /* check to make sure no recursion can happen */
  if (parent == thing) {
    notify(player, "A thing cannot be its own ancestor!");
    return;
  }
  if (parent != NOTHING) {
    for (i = 0, check = Parent(parent);
	 (i < MAX_PARENTS) && (check != NOTHING);
	 i++, check = Parent(check)) {
      if (check == thing) {
	notify(player, "You are not allowed to be your own ancestor!");
	return;
      }
    }
    if (i >= MAX_PARENTS) {
      notify(player, "Too many ancestors.");
      return;
    }
  }
  /* everything is okay, do the change */
  Parent(thing) = parent;
  notify(player, "Parent changed.");
}

static int
wipe_helper(player, thing, pattern, atr, args)
    dbref player;
    dbref thing;
    char const *pattern;
    ATTR *atr;
    void *args;
{
  /* for added security, only God can modify wiz-only-modifiable
   * attributes using this command and wildcards.  Wiping a specific
   * attr still works, though.
   */
  if (wildcard(pattern) && (AL_FLAGS(atr) & AF_WIZARD) && !God(player))
    return 0;
  return do_set_atr(thing, AL_NAME(atr), NULL, player, 0) == 1;
}

void
do_wipe(player, name)
    dbref player;
    char *name;
{
  /* obliterate attributes from an object */

  dbref thing;
  char *pattern;

  if ((pattern = (char *) index(name, '/')) != NULL)
    *pattern++ = '\0';

  if ((thing = noisy_match_result(player, name, NOTYPE, MAT_NEARBY)) == NOTHING)
    return;

  /* this is too destructive of a command to be used by someone who
   * doesn't own the object. Thus, the check is on Owns not controls.
   */
  if (!Wizard(player) && !Owns(player, thing)) {
    notify(player, "Permission denied.");
    return;
  }
  /* protect SAFE objects unless doing a non-wildcard pattern */
  if (Safe(thing) && !(pattern && *pattern && !wildcard(pattern))) {
    notify(player, "That object is protected.");
    return;
  }
  (void) atr_iter_get(player, thing, pattern, wipe_helper, NULL);

  notify(player, "Attributes wiped.");
}
