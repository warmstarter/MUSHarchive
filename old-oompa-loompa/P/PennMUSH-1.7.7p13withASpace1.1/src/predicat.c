/**
 * \file predicat.c
 *
 * \brief Predicates for testing various conditions in PennMUSH.
 *
 *
 */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <stdlib.h>

#include "conf.h"
#include "externs.h"
#include "mushdb.h"
#include "attrib.h"
#include "lock.h"
#include "flags.h"
#include "match.h"
#include "ansi.h"
#include "parse.h"
#include "dbdefs.h"
#include "privtab.h"
#include "mymalloc.h"
#include "confmagic.h"

int forbidden_name(const char *name);
void do_switch(dbref player, char *expression, char **argv,
	       dbref cause, int first, int notifyme);
void do_verb(dbref player, dbref cause, char *arg1, char **argv);
static int grep_util_helper(dbref player, dbref thing,
			    char const *pattern, ATTR *atr, void *args);
static int grep_helper(dbref player, dbref thing,
		       char const *pattern, ATTR *atr, void *args);
void do_grep(dbref player, char *obj, char *lookfor, int flag, int insensitive);
#ifdef QUOTA
static int pay_quota(dbref, int);
#endif				/* QUOTA */
extern PRIV attr_privs[];

/** A generic function to generate a formatted string. The
 * return value is a statically allocated buffer.
 *
 * \param fmt format string.
 * \return formatted string.
 */
char *WIN32_CDECL
tprintf(const char *fmt, ...)
{
#ifdef HAS_VSNPRINTF
  static char buff[BUFFER_LEN];
#else
  static char buff[BUFFER_LEN * 3];	/* safety margin */
#endif
  va_list args;

  va_start(args, fmt);

#ifdef HAS_VSNPRINTF
  vsnprintf(buff, sizeof buff, fmt, args);
#else
  vsprintf(buff, fmt, args);
#endif

  buff[BUFFER_LEN - 1] = '\0';
  va_end(args);
  return (buff);
}

/** Notify a player with a formatted string.
 * This is a safer replacement for notify(player, tprintf(fmt, ...))
 * \param player player to notify.
 * \param fmt format string.
 */
void WIN32_CDECL
notify_format(dbref player, const char *fmt, ...)
{
#ifdef HAS_VSNPRINTF
  char buff[BUFFER_LEN];
#else
  char buff[BUFFER_LEN * 3];
#endif
  va_list args;

  va_start(args, fmt);

#ifdef HAS_VSNPRINTF
  vsnprintf(buff, sizeof buff, fmt, args);
#else
  vsprintf(buff, fmt, args);
#endif

  buff[BUFFER_LEN - 1] = '\0';
  va_end(args);
  notify(player, buff);
}


/** lock evaluation -- determines if player passes lock on thing, for
 * the purposes of picking up an object or moving through an exit.
 * \param player to check against lock.
 * \param thing thing to check the basic lock on.
 * \retval 1 player passes lock.
 * \retval 0 player fails lock.
 */
int
could_doit(dbref player, dbref thing)
{
  if (!IsRoom(thing) && Location(thing) == NOTHING)
    return 0;
  return (eval_lock(player, thing, Basic_Lock));
}

/** Execute an action on an object, and handle objects with limited charges.
 * \param player the enactor.
 * \param thing object being used.
 * \param awhat action attribute on object to be triggered.
 * \retval 0 no action taken.
 * \retval 1 action taken.
 */
int
charge_action(dbref player, dbref thing, const char *awhat)
{
  ATTR *b;
  char tbuf2[BUFFER_LEN];
  int num;

  if (!awhat || !*awhat)
    return 0;

  /* check if object has # of charges */
  b = atr_get_noparent(thing, "CHARGES");

  if (!b) {
    /* no charges set, just execute the action */
    return queue_attribute(thing, awhat, player);
  } else {
    strcpy(tbuf2, uncompress(AL_STR(b)));
    num = atoi(tbuf2);
    if (num) {
      /* charges left, decrement and execute */
      int res;
      if ((res = queue_attribute(thing, awhat, player)))
	(void) atr_add(thing, "CHARGES", tprintf("%d", num - 1),
		       Owner(b->creator), NOTHING);
      return res;
    } else {
      /* no charges left, try to execute runout */
      return queue_attribute(thing, "RUNOUT", player);
    }
  }
}


/** A wrapper for real_did_it that clears the environment first.
 * \param player the enactor.
 * \param thing object being triggered.
 * \param what message attribute for enactor.
 * \param def default message to enactor.
 * \param owhat message attribute for others.
 * \param odef default message to others.
 * \param awhat action attribute to trigger.
 * \param loc location in which action is taking place.
 */
void
did_it(dbref player, dbref thing, const char *what, const char *def,
       const char *owhat, const char *odef, const char *awhat, dbref loc)
{
  /* Bunch o' nulls */
  static char *myenv[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  real_did_it(player, thing, what, def, owhat, odef, awhat, loc, myenv);
}


/** Take an action on an object and trigger attributes.
 * \verbatim
 * executes the @attr, @oattr, @aattr for a command - gives a message
 * to the enactor and others in the room with the enactor, and executes
 * an action. We load wenv with the values in myenv.
 * \endverbatim
 *
 * \param player the enactor.
 * \param thing object being triggered.
 * \param what message attribute for enactor.
 * \param def default message to enactor.
 * \param owhat message attribute for others.
 * \param odef default message to others.
 * \param awhat action attribute to trigger.
 * \param loc location in which action is taking place.
 * \param myenv copy of the environment.
 */
void
real_did_it(dbref player, dbref thing, const char *what, const char *def,
	    const char *owhat, const char *odef, const char *awhat, dbref loc,
	    char *myenv[10])
{

  ATTR *d;
  char buff[BUFFER_LEN], *bp, *sp, *asave;
  char const *ap;
  int j;
  char *preserves[10];
  char *preserveq[NUMQ];
  int need_pres = 0;

  loc = (loc == NOTHING) ? Location(player) : loc;

  /* only give messages if the location is good */
  if (GoodObject(loc)) {

    /* message to player */
    if (what && *what) {
      d = atr_get(thing, what);
      if (d) {
	if (!need_pres) {
	  need_pres = 1;
	  save_global_regs("did_it_save", preserveq);
	  save_global_env("did_it_save", preserves);
	}
	restore_global_env("did_it", myenv);
	asave = safe_uncompress(d->value);
	ap = asave;
	bp = buff;
	process_expression(buff, &bp, &ap, thing, player, player,
			   PE_DEFAULT, PT_DEFAULT, NULL);
	*bp = '\0';
	notify_by(thing, player, buff);
	free((Malloc_t) asave);
      } else if (def && *def)
	notify_by(thing, player, def);
    }
    /* message to neighbors */
    if (!DarkLegal(player)) {
      if (owhat && *owhat) {
	d = atr_get(thing, owhat);
	if (d) {
	  if (!need_pres) {
	    need_pres = 1;
	    save_global_regs("did_it_save", preserveq);
	    save_global_env("did_it_save", preserves);
	  }
	  restore_global_env("did_it", myenv);
	  asave = safe_uncompress(d->value);
	  ap = asave;
	  bp = buff;
	  safe_str(Name(player), buff, &bp);
	  safe_chr(' ', buff, &bp);
	  sp = bp;
	  process_expression(buff, &bp, &ap, thing, player, player,
			     PE_DEFAULT, PT_DEFAULT, NULL);
	  *bp = '\0';
	  if (bp != sp)
	    notify_except2(Contents(loc), player, thing, buff);
	  free((Malloc_t) asave);
	} else {
	  if (odef && *odef) {
	    notify_except2(Contents(loc), player, thing,
			   tprintf("%s %s", Name(player), odef));
	  }
	}
      }
    }
  }
  if (need_pres) {
    restore_global_regs("did_it_save", preserveq);
    restore_global_env("did_it_save", preserves);
  }
  for (j = 0; j < 10; j++)
    wnxt[j] = myenv[j];
  for (j = 0; j < NUMQ; j++)
    rnxt[j] = NULL;
  charge_action(player, thing, awhat);
}

/** Return the first object near another object that is visible to a player.
 *
 * BEWARE:
 *
 * first_visible() does not behave as intended. It _should_ return the first
 * object in `thing' that is !DARK. However, because of the controls() check
 * the function will return a DARK object if the player owns it.
 *
 * The behavior is left as is because so many functions in fundb.c rely on
 * the incorrect behavior to return expected values. The lv*() functions
 * also make rewriting this fairly pointless.
 *
 * \param player the looker.
 * \param thing an object in the location to be inspected.
 * \return dbref of first visible object or NOTHING.
 */
dbref
first_visible(dbref player, dbref thing)
{
  int lck = 0;
  int ldark;
  dbref loc;

  if (!GoodObject(thing) || IsRoom(thing))
    return NOTHING;
  loc = IsExit(thing) ? Source(thing) : Location(thing);
  if (!GoodObject(loc))
    return NOTHING;
  ldark = IsPlayer(loc) ? Opaque(loc) : Dark(loc);

  while (GoodObject(thing)) {
    if (DarkLegal(thing) || (ldark && !Light(thing))) {
      if (!lck) {
	if (See_All(player) || (loc == player) || controls(player, loc))
	  return thing;
	lck = 1;
      }
      if (controls(player, thing))	/* this is what causes DARK objects to show */
	return thing;
    } else {
      return thing;
    }
    thing = Next(thing);
  }
  return thing;
}



/** Can a player see something?
 * \param player the looker.
 * \param thing object to be seen.
 * \param can_see_loc 1 if player can see the location, 0 if location is dark.
 * \retval 1 player can see thing.
 * \retval 0 player can not see thing.
 */
int
can_see(dbref player, dbref thing, int can_see_loc)
{
  if (!can_interact(thing, player, INTERACT_SEE))
    return 0;

  /*
   * 1) your own body isn't listed in a 'look' 2) exits aren't listed in a
   * 'look' 3) unconnected (sleeping) players aren't listed in a 'look'
   */
  if (player == thing || IsExit(thing) ||
      (IsPlayer(thing) && !Connected(thing)))
    return 0;

  /* if the room is lit, you can see any non-dark objects */
  else if (can_see_loc)
    return (!DarkLegal(thing));

  /* otherwise room is dark and you can only see lit things */
  else
    return (Light(thing) && !DarkLegal(thing));
}

/** Can a player control a thing?
 * The control rules are, in order:
 *   Only God controls God.
 *   Wizards control everything else.
 *   Nothing else controls a wizard, and only royalty control royalty.
 *   Mistrusted objects control only themselves.
 *   Objects with the same owner control each other, unless the 
 *     target object is TRUST and the would-be controller isn't.
 *   If ZMOs allow control, and you pass the ZMO, you control.
 *   If the owner is a Zone Master, and you pass the ZM, you control.
 *   If you pass the control lock, you control.
 *   Otherwise, no dice.
 * \param who object attempting to control.
 * \param what object to be controlled.
 * \retval 1 who controls what.
 * \retval 0 who doesn't control what.
 */
int
controls(dbref who, dbref what)
{
  struct boolexp *c;

  if (!GoodObject(what))
    return 0;

  if (God(what) && !God(who))
    return 0;

  if (Wizard(who))
    return 1;

  if (Wizard(what) || (Hasprivs(what) && !Hasprivs(who)))
    return 0;

  if (Mistrust(who) && (who != what))
    return 0;

  if (Owns(who, what) && (!Inheritable(what) || Inheritable(who)))
    return 1;

  if (Inheritable(what) || IsPlayer(what))
    return 0;

  if (!ZONE_CONTROL_ZMP && (Zone(what) != NOTHING) &&
      eval_lock(who, Zone(what), Zone_Lock))
    return 1;

  if (ZMaster(Owner(what)) && !IsPlayer(what) &&
      eval_lock(who, Owner(what), Zone_Lock))
    return 1;

  c = getlock_noparent(what, Control_Lock);
  if (c != TRUE_BOOLEXP) {
    if (eval_boolexp(who, c, what))
      return 1;
  }
  return 0;
}

/** Can someone pay for something (in cash and quota)?
 * Does who have enough pennies to pay for something, and if something
 * is being built, does who have enough quota? Wizards, roys
 * aren't subject to either. This function not only checks that they
 * can afford it, but actually charges them.
 * \param who player attempting to pay.
 * \param pennies cost in pennies.
 * \retval 1 who can pay.
 * \retval 0 who can't pay.
 */
int
can_pay_fees(dbref who, int pennies)
{
  /* check database size -- EVERYONE is subject to this! */
  if (DBTOP_MAX && (db_top >= DBTOP_MAX + 1) && (first_free == NOTHING)) {
    notify(who, T("Sorry, there is no more room in the database."));
    return 0;
  }
  /* Can they afford it? */
  if (!NoPay(who) && (Pennies(Owner(who)) < pennies)) {
    notify_format(who, T("Sorry, you don't have enough %s."), MONIES);
    return 0;
  }
  /* check building quota */
#ifdef QUOTA
  if (!NoQuota(who) && !pay_quota(who, QUOTA_COST)) {
    notify(who, T("Sorry, your building quota has run out."));
    return 0;
  }
#endif				/* QUOTA */

  /* charge */
  payfor(who, pennies);

  return 1;
}

/** Transfer pennies to a player.
 * \param who recipient.
 * \param pennies amount of pennies to give.
 */
void
giveto(dbref who, int pennies)
{
  /* wizards and royalty don't need pennies */
  if (NoPay(who))
    return;

  who = Owner(who);
  if ((Pennies(who) + pennies) > Max_Pennies(who))
    s_Pennies(who, Max_Pennies(who));
  else
    s_Pennies(who, Pennies(who) + pennies);
}

/** Debit a player's pennies, if they can afford it.
 * \param who player to debit.
 * \param cost number of pennies to debit.
 * \retval 1 player successfully debited.
 * \retval 0 player can't afford the cost.
 */
int
payfor(dbref who, int cost)
{
  /* subtract cost from who's pennies */

  dbref tmp;
  if (NoPay(who))
    return 1;
  else if ((tmp = Pennies(Owner(who))) >= cost) {
    s_Pennies(Owner(who), tmp - cost);
    return 1;
  } else
    return 0;
}

#ifdef QUOTA

/** Retrieve the amount of quote remaining to a player.
 * Figure out a player's quota. Add the RQUOTA attribute if he doesn't
 * have one already. This function returns the REMAINING quota, not
 * the TOTAL limit.
 * \param who player to check.
 * \return player's remaining quota.
 */
int
get_current_quota(dbref who)
{
  ATTR *a;
  int i;
  int limit;
  int owned = 0;

  /* if he's got an RQUOTA attribute, his remaining quota is that */
  a = atr_get_noparent(Owner(who), "RQUOTA");
  if (a)
    return (atoi(uncompress(a->value)));

  /* else, count up his objects. If he has less than the START_QUOTA,
   * then his remaining quota is that minus his number of current objects.
   * Otherwise, it's his current number of objects. Add the attribute
   * if he doesn't have it.
   */

  for (i = 0; i < db_top; i++)
    if (Owner(i) == Owner(who))
      owned++;
  owned--;			/* don't count the player himself */

  if (owned <= START_QUOTA)
    limit = START_QUOTA - owned;
  else
    limit = owned;

  (void) atr_add(Owner(who), "RQUOTA", tprintf("%d", limit), GOD, NOTHING);

  return (limit);
}


/** Add or subtract from a player's quota.
 * \param who object whose owner has the quota changed.
 * \param payment amount to add to quota (may be negative).
 */
void
change_quota(dbref who, int payment)
{
  /* wizards and royalty don't need a quota */
  if (NoQuota(who))
    return;

  (void) atr_add(Owner(who), "RQUOTA",
		 tprintf("%d", get_current_quota(who) + payment), GOD, NOTHING);
}

/** Debit a player's quota, if they can afford it.
 * \param who player whose quota is to be debitted.
 * \param cost amount of quota to be charged.
 * \retval 1 quota successfully debitted.
 * \retval 0 not enough quota to debit.
 */
static int
pay_quota(dbref who, int cost)
{
  int curr;

  /* wizards and royalty don't need a quota */
  if (NoQuota(who))
    return 1;

  /* figure out how much we have, and if it's big enough */
  curr = get_current_quota(who);

  if (curr - cost < 0)		/* not enough */
    return 0;

  change_quota(who, -cost);

  return 1;
}
#endif				/* QUOTA */

/** Is a name in the forbidden names file?
 * \param name name to check.
 * \retval 1 name is forbidden.
 * \retval 0 name is not forbidden.
 */
int
forbidden_name(const char *name)
{
  char buf[BUFFER_LEN], *newlin, *ptr;
  FILE *fp;
  char *upname;

  upname = strupper(name);
#ifdef macintosh
  fp = fopen(NAMES_FILE, "rb");
#else
  fp = fopen(NAMES_FILE, "r");
#endif
  if (!fp)
    return 0;
  while (fgets(buf, BUFFER_LEN - 1, fp)) {
    upcasestr(buf);
    /* step on the newline */
    if ((newlin = strchr(buf, '\n')) != NULL)
      *newlin = '\0';
    ptr = buf;
    if (name && ptr && quick_wild(ptr, name)) {
      fclose(fp);
      return 1;
    }
  }
  fclose(fp);
  return 0;
}

/** Is a name valid for an object?
 * This involves several checks.
 *   Names may not have leading or trailing spaces.
 *   Names must be only printable characters.
 *   Names may not exceed the length limit.
 *   Names may not start with certain tokens, or be "me", "home", "here"
 * \param n name to check.
 * \retval 1 name is valid.
 * \retval 0 name is not valid.
 */
int
ok_name(const char *n)
{
  const unsigned char *p, *name = (const unsigned char *) n;

  if (!name || !*name)
    return 0;

  /* No leading spaces */
  if (isspace((unsigned char) *name))
    return 0;

  /* only printable characters */
  for (p = name; p && *p; p++) {
    if (!isprint((unsigned char) *p))
      return 0;
    if (ONLY_ASCII_NAMES && *p > 127)
      return 0;
    if (strchr("[]%\\=&|", *p))
      return 0;
  }

  /* No trailing spaces */
  p--;
  if (isspace((unsigned char) *p))
    return 0;

  /* Not too long */
  if (u_strlen(name) >= OBJECT_NAME_LIMIT)
    return 0;

  /* No magic cookies */
  return (name
	  && *name
	  && *name != LOOKUP_TOKEN
	  && *name != NUMBER_TOKEN
	  && *name != NOT_TOKEN && strcasecmp((char *) name, "me")
	  && strcasecmp((char *) name, "home")
	  && strcasecmp((char *) name, "here"));
}

/** Is a name a valid player name?
 * Player names must be valid object names, but also not forbidden,
 * subject to a different length limit, and subject to more stringent
 * restrictions on valid characters. Finally, it can't be the same as
 * an existing player name or alias.
 * \param name name to check.
 * \retval 1 name is valid for players.
 * \retval 0 name is not valid for players.
 */
int
ok_player_name(const char *name)
{
  const unsigned char *scan, *good;

  if (!ok_name(name) || forbidden_name(name) ||
      strlen(name) >= (size_t) PLAYER_NAME_LIMIT)
    return 0;

  good = (unsigned char *) (PLAYER_NAME_SPACES ? " `$_-.,'" : "`$_-.,'");

  /* Make sure that the name contains legal characters only */
  for (scan = (unsigned char *) name; scan && *scan; scan++) {
    if (isalnum((unsigned char) *scan))
      continue;
    if (!strchr((char *) good, *scan))
      return 0;
  }

  return (lookup_player(name) == NOTHING);
}

/** Is a password acceptable?
 * Acceptable passwords must be non-null and must contain only
 * printable characters and no whitespace.
 * \param password password to check.
 * \retval 1 password is acceptable.
 * \retval 0 password is not acceptable.
 */
int
ok_password(const char *password)
{
  const char *scan;
  if (*password == '\0')
    return 0;

  for (scan = password; *scan; scan++) {
    if (!(isprint(*scan) && !isspace(*scan))) {
      return 0;
    }
  }

  return 1;
}

/** Is a name ok for a command or function?
 * It must begin with an uppercase alpha, and contain only
 * uppercase alpha, numbers, or underscore thereafter.
 * \param name name to check.
 * \retval 1 name is acceptable.
 * \retval 0 name is not acceptable.
 */
int
ok_command_name(const char *name)
{
  const unsigned char *p;
  if (!isupper((unsigned char) *name))
    return 0;
  for (p = (unsigned char *) name; p && *p; p++) {
    if (!(isupper(*p) || isdigit(*p) || (*p == '_')))
      return 0;
  }

  /* Not too long */
  if (strlen(name) >= COMMAND_NAME_LIMIT)
    return 0;

  return 1;
}

/* The switch command.
 * \verbatim
 * For lack of better place the @switch code is here.
 * @switch expression=args
 * \endverbatim
 * \param player the enactor.
 * \param expression the expression to test against cases.
 * \param argv array of cases and actions.
 * \param cause the object that caused this code to run.
 * \param first if 1, run only first matching case; if 0, run all matching cases.
 * \param notifyme if 1, perform a notify after executing matched cases.
 */
void
do_switch(dbref player, char *expression, char **argv, dbref cause,
	  int first, int notifyme)
{
  int any = 0, a;
  char buff[BUFFER_LEN], *bp;
  char const *ap;
  char *tbuf1;

  if (!argv[1])
    return;

  /* set up environment for any spawned commands */
  for (a = 0; a < 10; a++)
    wnxt[a] = wenv[a];
  for (a = 0; a < NUMQ; a++)
    rnxt[a] = renv[a];

  /* now try a wild card match of buff with stuff in coms */
  for (a = 1;
       !(first && any) && (a < (MAX_ARG - 1)) && argv[a] && argv[a + 1];
       a += 2) {
    /* eval expression */
    ap = argv[a];
    bp = buff;
    process_expression(buff, &bp, &ap, player, cause, cause,
		       PE_DEFAULT, PT_DEFAULT, NULL);
    *bp = '\0';

    /* check for a match */
    if (local_wild_match(buff, expression)) {
      any = 1;
      tbuf1 = replace_string("#$", expression, argv[a + 1]);
      parse_que(player, tbuf1, cause);
      mush_free(tbuf1, "replace_string.buff");
    }
  }

  /* do default if nothing has been matched */
  if ((a < MAX_ARG) && !any && argv[a]) {
    tbuf1 = replace_string("#$", expression, argv[a]);
    parse_que(player, tbuf1, cause);
    mush_free(tbuf1, "replace_string.buff");
  }

  /* Pop on @notify me, if requested */
  if (notifyme)
    parse_que(player, "@notify me", cause);
}

/** Parse possessive matches.
 * This function parses strings of the form "Sam's bag" and attempts
 * to match "bag" in the contents of "Sam". It returns NOTHING if
 * there's no possessive 's in the string.
 * \param player the enactor/looker.
 * \param str a string to check for possessive matches.
 * \return matching dbref or NOTHING.
 */
dbref
parse_match_possessive(dbref player, const char *str)
{
  char *box;			/* name of container */
  char *obj;			/* name of object */
  dbref loc;			/* dbref of container */
  char name[BUFFER_LEN];	/* str would be destructively modified */

  strcpy(name, str);
  box = name;

  /* check to see if we have an 's sequence */
  if ((obj = strchr(name, '\'')) == NULL)
    return NOTHING;
  *obj++ = '\0';		/* terminate */
  if ((*obj == '\0') || ((*obj != 's') && (*obj != 'S')))
    return NOTHING;

  /* skip over the 's' and whitespace */
  do {
    obj++;
  } while (isspace((unsigned char) *obj));

  /* we already have a terminating null, so we're okay to just do matches */
  loc = match_result(player, box, NOTYPE, MAT_NEIGHBOR | MAT_POSSESSION);
  if (!GoodObject(loc))
    return NOTHING;

  /* now we match on the contents */
  return (match_result(loc, obj, NOTYPE, MAT_POSSESSION));
}


/** Autoreply messages for pages (HAVEN, IDLE, AWAY).
 * \param player the paging player.
 * \param target the paged player.
 * \param type type of message to return.
 * \param message name of attribute containing the message.
 * \param def default message to return.
 */
void
page_return(dbref player, dbref target, const char *type,
	    const char *message, const char *def)
{
  ATTR *d;
  char buff[BUFFER_LEN], *bp, *asave;
  char const *ap;
  struct tm *ptr;

  if (message && *message) {
    d = atr_get(target, message);
    if (d) {
      asave = safe_uncompress(d->value);
      ap = asave;
      bp = buff;
      process_expression(buff, &bp, &ap, target, player, player,
			 PE_DEFAULT, PT_DEFAULT, NULL);
      *bp = '\0';
      free((Malloc_t) asave);
      if (*buff) {
	ptr = (struct tm *) localtime(&mudtime);
	notify_format(player, T("%s message from %s: %s"), type,
		      Name(target), buff);
	if (!Haven(target))
	  notify_format(target,
			T("[%d:%02d] %s message sent to %s."), ptr->tm_hour,
			ptr->tm_min, type, Name(player));
      }
    } else if (def && *def)
      notify(player, def);
  }
}

/* Returns the apparent location of object. 
 * This is the location for players and things, source for exits, and 
 * NOTHING for rooms.
 * \param thing object to get location of.
 * \return apparent location of object (NOTHING for rooms).
 */
dbref
where_is(dbref thing)
{
  if (!GoodObject(thing))
    return NOTHING;
  switch (Typeof(thing)) {
  case TYPE_ROOM:
    return NOTHING;
  case TYPE_EXIT:
    return Home(thing);
  default:
    return Location(thing);
  }
}

/* Are two objects near each other?
 * Returns 1 if obj1 is "nearby" object2. "Nearby" is a commutative
 * relation defined as:  
 *   obj1 is in the same room as obj2, obj1 is being carried by   
 *   obj2, or obj1 is carrying obj2. 
 * Returns 0 if object isn't nearby or the input is invalid.
 * \param obj1 first object.
 * \param obj2 second object.
 * \retval 1 the objects are near each other.
 * \retval 0 the objects are not near each other.
 */
int
nearby(dbref obj1, dbref obj2)
{
  dbref loc1, loc2;

  if (!GoodObject(obj1) || !GoodObject(obj2))
    return 0;
  loc1 = where_is(obj1);
  if (loc1 == obj2)
    return 1;
  loc2 = where_is(obj2);
  if ((loc2 == obj1) || (loc2 == loc1))
    return 1;
  return 0;
}

/* User-defined verbs.
 * \verbatim
 * This implements the @verb command.
 * \endverbatim
 * \param player the enactor.
 * \param cause the object causing this command to run.
 * \param arg1 the object to read verb attributes from.
 * \param argv the array of remaining arguments to the verb command.
 */
void
do_verb(dbref player, dbref cause, char *arg1, char **argv)
{
  dbref victim;
  dbref actor;
  int i;
  char *wsave[10];

  /* find the object that we want to read the attributes off
   * (the object that was the victim of the command)
   */

  /* our victim object can be anything */
  victim = match_result(player, arg1, NOTYPE, MAT_EVERYTHING);

  if (victim == NOTHING) {
    notify(player, T("What was the victim of the verb?"));
    return;
  }
  /* find the object that executes the action */

  if (!argv || !argv[1] || !*argv[1]) {
    notify(player, T("What do you want to do with the verb?"));
    return;
  }
  actor = match_result(player, argv[1], NOTYPE, MAT_EVERYTHING);

  if (actor == NOTHING) {
    notify(player, T("What do you want to do the verb?"));
    return;
  }
  /* Control check is fascist. 
   * First check: we don't want <actor> to do something involuntarily.
   *   Both victim and actor have to be controlled by the thing which did 
   *   the @verb (for speed we do a WIZARD check first), or: cause controls
   *   actor plus the second check is passed.
   * Second check: we need read access to the attributes.
   *   Either the player controls victim or the player
   *   must be priviledged, or the victim has to be VISUAL.
   */

  if (!(Wizard(player) ||
	(controls(player, victim) && controls(player, actor)) ||
	((controls(cause, actor) && Can_Examine(player, victim))))) {
    notify(player, T("Permission denied."));
    return;
  }
  /* We're okay.  Send out messages. */

  for (i = 0; i < 10; i++) {
    wsave[i] = wenv[i];
    wenv[i] = argv[i + 7];
  }

  real_did_it(actor, victim,
	      upcasestr(argv[2]), argv[3], upcasestr(argv[4]), argv[5],
	      NULL, Location(actor), wenv);

  for (i = 0; i < 10; i++)
    wenv[i] = wsave[i];

  /* Now we copy our args into the stack, and do the command. */

  for (i = 0; i < 10; i++)
    wnxt[i] = argv[i + 7];

  charge_action(actor, victim, upcasestr(argv[6]));
}

/** Structure for passing arguments to grep_util_helper().
 */
struct guh_args {
  char *buff;		/**< Buffer for output */
  char *bp;		/**< Pointer to buff's current position */
  char *lookfor;	/**< String to grep for */
  int len;		/**< Length of lookfor */
  int insensitive;	/**< If 1, case-insensitive match; if 0, sensitive */
};

static int
grep_util_helper(dbref player __attribute__ ((__unused__)),
		 dbref thing __attribute__ ((__unused__)),
		 char const *pattern
		 __attribute__ ((__unused__)), ATTR *atr, void *args)
{
  struct guh_args *guh = args;
  int found;
  char *s;

  s = (char *) uncompress(AL_STR(atr));	/* warning: static */
  found = 0;
  while (*s && !found) {
    if ((!guh->insensitive && !strncmp(guh->lookfor, s, guh->len)) ||
	(guh->insensitive && !strncasecmp(guh->lookfor, s, guh->len)))
      found = 1;
    else
      s++;
  }
  if (found) {
    if (guh->bp != guh->buff)
      safe_chr(' ', guh->buff, &guh->bp);
    safe_str(AL_NAME(atr), guh->buff, &guh->bp);
  }
  return found;
}

/** Utility function for grep funtions/commands.
 * This function returns a list of attributes on an object that
 * match a name pattern and contain another string.
 * \param player the enactor.
 * \param thing object to check attributes on.
 * \param pattern wildcard pattern for attributes to check.
 * \param lookfor string to find within each attribute.
 * \param len length of lookfor.
 * \param insensitive if 1, case-insensitive matching; if 0, case-sensitive.
 * \return string containing list of attribute names with matching data.
 */
char *
grep_util(dbref player, dbref thing, char *pattern, char *lookfor,
	  int len, int insensitive)
{
  struct guh_args guh;

  guh.buff = (char *) mush_malloc(BUFFER_LEN + 1, "grep_util.buff");
  guh.bp = guh.buff;
  guh.lookfor = lookfor;
  guh.len = len;
  guh.insensitive = insensitive;
  (void) atr_iter_get(player, thing, pattern, grep_util_helper, &guh);
  *guh.bp = '\0';
  return guh.buff;
}

/** Structure for grep_helper() arguments. */
struct gh_args {
  char *lookfor;	/**< Pattern to look for. */
  int len;		/**< Length of lookfor. */
  int insensitive;	/**< if 1, case-insensitive matching; if 0, sensitive */
};

static int
grep_helper(dbref player, dbref thing __attribute__ ((__unused__)),
	    char const *pattern
	    __attribute__ ((__unused__)), ATTR *atr, void *args)
{
  struct gh_args *gh = args;
  int found;
  char *s;
  char tbuf1[BUFFER_LEN];
  char buf[BUFFER_LEN];
  char *tbp;

  found = 0;
  tbp = tbuf1;

  s = (char *) uncompress(AL_STR(atr));	/* warning: static */
  while (s && *s) {
    if ((gh->insensitive && !strncasecmp(gh->lookfor, s, gh->len)) ||
	(!gh->insensitive && !strncmp(gh->lookfor, s, gh->len))) {
      found = 1;
      strncpy(buf, s, gh->len);
      buf[gh->len] = '\0';
      s += gh->len;
      safe_format(tbuf1, &tbp, "%s%s%s", ANSI_HILITE, buf, ANSI_NORMAL);
    } else {
      safe_chr(*s, tbuf1, &tbp);
      s++;
    }
  }
  *tbp = '\0';

  /* if we got it, display it */
  if (found)
    notify_format(player, "%s%s [#%d%s]%s %s",
		  ANSI_HILITE, AL_NAME(atr),
		  Owner(AL_CREATOR(atr)),
		  privs_to_letters(attr_privs, AL_FLAGS(atr)),
		  ANSI_NORMAL, tbuf1);
  return found;
}

/** The grep command
 * \verbatim
 * This implements @grep.
 * \endverbatim
 * \param player the enactor.
 * \param obj string containing obj/attr pattern to grep through.
 * \param lookfor unparsed string to search for.
 * \param flag if 0, return attribute names; if 1, return attrib text.
 * \param insensitive if 1, case-insensitive match; if 0, sensitive.
 */
void
do_grep(dbref player, char *obj, char *lookfor, int flag, int insensitive)
{
  struct gh_args gh;
  dbref thing;
  char *pattern;
  int len;
  char *tp;

  if ((len = strlen(lookfor)) < 1) {
    notify(player, T("What pattern do you want to grep for?"));
    return;
  }
  /* find the attribute pattern */
  pattern = strchr(obj, '/');
  if (!pattern)
    pattern = (char *) "*";	/* set it to global match */
  else
    *pattern++ = '\0';

  /* now we've got the object. match for it. */
  if ((thing = noisy_match_result(player, obj, NOTYPE, MAT_EVERYTHING)) ==
      NOTHING)
    return;
  if (!Can_Examine(player, thing)) {
    notify(player, T("Permission denied."));
    return;
  }

  if (flag) {
    gh.lookfor = lookfor;
    gh.len = len;
    gh.insensitive = insensitive;
    if (!atr_iter_get(player, thing, pattern, grep_helper, &gh))
      notify(player, T("No matching attributes."));
  } else {
    tp = grep_util(player, thing, pattern, lookfor, len, insensitive);
    notify_format(player, T("Matches of '%s' on %s(#%d): %s"), lookfor,
		  Name(thing), thing, tp);
    mush_free((Malloc_t) tp, "grep_util.buff");
  }
}
