/* predicat.c */

#include "copyrite.h"
#include "config.h"

/* Predicates for testing various conditions */

#include <stdio.h>
#ifdef I_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <ctype.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif

#include "conf.h"
#include "externs.h"
#include "mushdb.h"
#include "intrface.h"
#include "globals.h"
#include "match.h"
#include "ansi.h"
#include "parse.h"
#include "dbdefs.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "mymalloc.h"
#include "confmagic.h"

extern int first_free;		/* free object list, from destroy.c */

int could_doit _((dbref player, dbref thing));
void charge_action _((dbref player, dbref thing, const char *awhat));
void did_it _((dbref player, dbref thing, const char *what, const char *def, const char *owhat, const char *odef, const char *awhat, dbref loc));
int can_see _((dbref player, dbref thing, int can_see_loc));
int controls _((dbref who, dbref what));
int can_pay_fees _((dbref who, int pennies));
void giveto _((dbref who, dbref pennies));
int payfor _((dbref who, int cost));
int forbidden_name _((const char *name));
int ok_name _((const char *name));
int ok_player_name _((const char *name));
int ok_password _((const char *password));
void sstrcat _((char *string, char *app));
void do_switch _((dbref player, char *expression, char **argv, dbref cause, int first));
dbref parse_match_possessive _((dbref player, const char *str));
void page_return _((dbref player, dbref target, const char *type, const char *message, const char *def));
dbref where_is _((dbref thing));
int nearby _((dbref obj1, dbref obj2));
void do_verb _((dbref player, dbref cause, char *arg1, char **argv));
char *grep_util _((dbref player, dbref thing, char *pattern, char *lookfor, int len, int insensitive));
static int grep_util_helper _((dbref player, dbref thing, char const *pattern,
			       ATTR *atr, void *args));
static int grep_helper _((dbref player, dbref thing, char const *pattern,
			  ATTR *atr, void *args));
void do_grep _((dbref player, char *obj, char *lookfor, int flag, int insensitive));
#ifdef __GNUC__
char *tprintf (const char *fmt,...) 
     __attribute__ ((__format__ (__printf__, 1, 2)));
#else
char *tprintf _((const char *fmt,...));
#endif

#ifdef I_STDARG
char *
tprintf(const char *fmt,...)
#else
char *
tprintf(va_alist)
    va_dcl
#endif
{
  /* this is a generic function used to generate a format string */

  static char buff[BUFFER_LEN * 3];	/* safety margin */
  va_list args;
#ifndef I_STDARG
  char *fmt;

  va_start(args);
  fmt = va_arg(args, char *);
#else
  va_start(args, fmt);
#endif

  (void) vsprintf(buff, fmt, args);
  buff[BUFFER_LEN - 1] = '\0';
  va_end(args);
  return (buff);
}

int
could_doit(player, thing)
    dbref player;
    dbref thing;
{
  /* lock evaluation -- determines if player passes lock on thing, for
   * the purposes of picking up an object or moving through an exit
   */

  if (!IsRoom(thing) && Location(thing) == NOTHING)
    return 0;
  return (eval_lock(player, thing, Basic_Lock));
}

void
charge_action(player, thing, awhat)
    dbref player;
    dbref thing;
    const char *awhat;
{
  ATTR *d;
  ATTR *b;
  char tbuf[BUFFER_LEN];
  char tbuf2[BUFFER_LEN];
  int num;

  if (awhat && *awhat && (d = atr_get(thing, awhat))) {
    strcpy(tbuf, uncompress(d->value));

    /* check if object has # of charges */
    b = atr_get_noparent(thing, "CHARGES");

    if (!b) {
      /* no charges set, just execute the action */
      parse_que(thing, tbuf, player);
      return;
    } else {
      strcpy(tbuf2, uncompress(b->value));
      num = atoi(tbuf2);
      if (num) {
	/* charges left, decrement and execute */
	(void) atr_add(thing, "CHARGES", tprintf("%d", num - 1),
		       Owner(b->creator), NOTHING);
	parse_que(thing, tbuf, player);
	return;
      } else if (!(d = atr_get(thing, "RUNOUT")))
	/* no charges left and no runout; do nothing */
	return;
      /* no charges left, execute runout */
      strcpy(tbuf, uncompress(d->value));
      parse_que(thing, tbuf, player);
    }
  }
}

void
did_it(player, thing, what, def, owhat, odef, awhat, loc)
    dbref player;
    dbref thing;
    const char *what;
    const char *def;
    const char *owhat;
    const char *odef;
    const char *awhat;
    dbref loc;
{
  /* executes the @attr, @oattr, @aattr for a command - gives a message
   * to the enactor and others in the room with the enactor, and executes
   * an action.
   */

  ATTR *d;
  char buff[BUFFER_LEN], *bp, *sp;
  char const *asave, *ap;
  int j;
  char *preserve[10];
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
	  save_global_regs("did_it_save", preserve);
	}
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
	    save_global_regs("did_it_save", preserve);
	  }
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
  if (need_pres)
    restore_global_regs("did_it_save", preserve);
  for (j = 0; j < 10; j++) {
    wnxt[j] = NULL;
    rnxt[j] = NULL;
  }
  charge_action(player, thing, awhat);
}

dbref
first_visible(player, thing)
    dbref player;
    dbref thing;
{
  int lck = 0;
  int ldark;
  dbref loc;

  if (!(GoodObject(thing)))
    return NOTHING;
  loc = IsExit(thing) ? Source(thing) : Location(thing);
  ldark = IsPlayer(loc) ? Opaque(loc) : Dark(loc);

  while (GoodObject(thing)) {
    if (DarkLegal(thing) || (ldark && !Light(thing))) {
      if (!lck) {
	if (See_All(player) || (loc == player) || controls(player, loc))
	  return thing;
	lck = 1;
      }
      if (controls(player, thing))
	return thing;
    } else {
      return thing;
    }
    thing = Next(thing);
  }
  return thing;
}




int
can_see(player, thing, can_see_loc)
    dbref player;
    dbref thing;
    int can_see_loc;
{
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

int
controls(who, what)
    dbref who;
    dbref what;
{
  /* Wizard controls everything 
   * owners control their stuff
   * something which is in the enterlock of a ZMO controls non-INHERIT
   * and non-player objects.
   * INHERIT checks between two objects are checked in the code for the
   * specific function in question (do_trigger, do_set, etc.)
   * Those who pass the enterlock of a ZoneMaster control his objects,
   * but not the ZoneMaster himself.
   */

  if (!GoodObject(what))
    return 0;

  if (God(what) && !God(who))
    return 0;

  if (Wizard(who))
    return 1;

  if (Wizard(what) || (Hasprivs(what) && !Hasprivs(who)))
    return 0;

  if (Owns(who, what) && (!Inheritable(what) || Inheritable(who)))
    return 1;

  if ((Zone(what) != NOTHING) &&
      (!IsPlayer(what)) &&
      !Inheritable(what) &&
      (eval_lock(who, Zone(what), Zone_Lock)))
    return 1;

  if (ZMaster(Owner(what)) && !IsPlayer(what) &&
      (eval_lock(who, Owner(what), Zone_Lock)))
    return 1;

  return 0;
}

int
can_pay_fees(who, pennies)
    dbref who;
    int pennies;
{
  /* does who have enough pennies to pay for something, and if something
   * is being built, does who have enough quota? Wizards, roys
   * aren't subject to either.
   */

#ifdef QUOTA
  int pay_quota();
#endif				/* QUOTA */

  /* check database size -- EVERYONE is subject to this! */
  if (DBTOP_MAX && (db_top >= DBTOP_MAX + 1) && (first_free == NOTHING)) {
    notify(who, "Sorry, there is no more room in the database.");
    return 0;
  }
  /* Can they afford it? */
  if (!NoPay(who) && (Pennies(Owner(who)) < pennies)) {
    notify(who, tprintf("Sorry, you don't have enough %s.", MONIES));
    return 0;
  }
  /* check building quota */
#ifdef QUOTA
  if (!NoQuota(who) && !pay_quota(who, QUOTA_COST)) {
    notify(who, "Sorry, your building quota has run out.");
    return 0;
  }
#endif				/* QUOTA */

  /* charge */
  payfor(who, pennies);

  return 1;
}

void
giveto(who, pennies)
    dbref who;
    dbref pennies;
{
  /* give who pennies */

  /* wizards and royalty don't need pennies */
  if (NoPay(who))
    return;

  who = Owner(who);
  if ((Pennies(who) + pennies) > MAX_PENNIES)
    s_Pennies(who, MAX_PENNIES);
  else
    s_Pennies(who, Pennies(who) + pennies);
}

int
payfor(who, cost)
    dbref who;
    int cost;
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

int
get_current_quota(who)
    dbref who;
{
  /* figure out a player's quota. Add the RQUOTA attribute if he doesn't
   * have one already. This function returns the REMAINING quota, not
   * the TOTAL limit.
   */

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

  atr_add(Owner(who), "RQUOTA", tprintf("%d", limit), GOD, NOTHING);

  return (limit);
}


void
change_quota(who, payment)
    dbref who;
    int payment;
{
  /* add or subtract from quota */

  /* wizards and royalty don't need a quota */
  if (NoQuota(Owner(who)))
    return;

  atr_add(Owner(who), "RQUOTA",
	  tprintf("%d", get_current_quota(who) + payment),
	  GOD, NOTHING);
}

int
pay_quota(who, cost)
    dbref who;
    int cost;
{
  /* determine if we've got enough quota to pay for an object,
   * and, if so, return true, and subtract from the quota.
   */

  int curr;

  /* wizards and royalty don't need a quota */
  if (NoQuota(Owner(who)))
    return 1;

  /* figure out how much we have, and if it's big enough */
  curr = get_current_quota(who);

  if (curr - cost < 0)		/* not enough */
    return 0;

  change_quota(who, -cost);

  return 1;
}
#endif				/* QUOTA */

int
forbidden_name(name)
    const char *name;
{
  /* checks to see if name is in the forbidden names file */

  char buf[BUFFER_LEN], *newlin, *ptr;
  FILE *fp;

  fp = fopen(NAMES_FILE, "r");
  if (!fp)
    return 0;
  while (!feof(fp)) {
    fgets(buf, BUFFER_LEN, fp);
    /* step on the newline */
    if ((newlin = (char *) index(buf, '\n')) != NULL)
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

int
ok_name(name)
    const char *name;
{
  /* is name valid for an object? */

  const char *p;

  /* No leading spaces */
  if (isspace(*name))
    return 0;

  /* only printable characters */
  for (p = name; p && *p; p++) {
    if (!isprint(*p))
      return 0;
    if ((*p == '[') || (*p == ']') ||
	(*p == '%') || (*p == '\\'))
      return 0;
  }

  /* No trailing spaces */
  p--;
  if (isspace(*p))
    return 0;

  /* Not too long */
  if (strlen(name) >= OBJECT_NAME_LIMIT)
    return 0;

  /* No magic cookies */
  return (name
	  && *name
	  && *name != LOOKUP_TOKEN
	  && *name != NUMBER_TOKEN
	  && *name != NOT_TOKEN
	  && !index(name, ARG_DELIMITER)
	  && !index(name, AND_TOKEN)
	  && !index(name, OR_TOKEN)
	  && strcasecmp(name, "me")
	  && strcasecmp(name, "home")
	  && strcasecmp(name, "here"));
}

int
ok_player_name(name)
    const char *name;
{
  /* is name okay for a player? */

  const char *scan, *good;

  if (!ok_name(name) || forbidden_name(name) ||
      strlen(name) >= PLAYER_NAME_LIMIT)
    return 0;

  good = PLAYER_NAME_SPACES ? " `$_-.,'" : "`$_-.,'";

  /* Make sure that the name contains legal characters only */
  for (scan = name; scan && *scan; scan++) {
    if (isalpha(*scan) || isdigit(*scan))
      continue;
    if (!index(good, *scan))
      return 0;
  }

  return (lookup_player(name) == NOTHING);
}

int
ok_password(password)
    const char *password;
{
  /* is password an acceptable password? */

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

void
sstrcat(string, app)
    char *string;
    char *app;
{
  char *s;
  char tbuf1[BUFFER_LEN];

  if ((strlen(app) + strlen(string)) >= BUFFER_LEN)
    return;
  sprintf(tbuf1, "%s", app);
  for (s = tbuf1; *s; s++)
    if ((*s == ',') || (*s == ';'))
      *s = ' ';
  strcat(string, tbuf1);
}

/* for lack of better place the @switch code is here */
void
do_switch(player, expression, argv, cause, first)
    dbref player;
    char *expression;
    char **argv;
    dbref cause;
    int first;




				/* 0, match all, 1, match first */
{
  int any = 0, a;
  char buff[BUFFER_LEN], *bp;
  char const *ap;
  char *tbuf1;

  if (!argv[1])
    return;

  /* set up environment for any spawned commands */
  for (a = 0; a < 10; a++) {
    wnxt[a] = wenv[a];
    rnxt[a] = renv[a];
  }

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
      free(tbuf1);
#ifdef MEM_CHECK
      del_check("replace_string.buff");
#endif
    }
  }

  /* do default if nothing has been matched */
  if ((a < MAX_ARG) && !any && argv[a]) {
    tbuf1 = replace_string("#$", expression, argv[a]);
    parse_que(player, tbuf1, cause);
    free(tbuf1);
#ifdef MEM_CHECK
    del_check("replace_string.buff");
#endif
  }
}

dbref
parse_match_possessive(player, str)
    dbref player;
    const char *str;
{
  char *box;			/* name of container */
  char *obj;			/* name of object */
  dbref loc;			/* dbref of container */
  char name[BUFFER_LEN];	/* str would be destructively modified */

  strcpy(name, str);
  box = name;

  /* check to see if we have an 's sequence */
  if ((obj = (char *) index(name, '\'')) == NULL)
    return NOTHING;
  *obj++ = '\0';		/* terminate */
  if ((*obj == '\0') || ((*obj != 's') && (*obj != 'S')))
    return NOTHING;

  /* skip over the 's' and whitespace */
  do {
    obj++;
  } while (isspace(*obj));

  /* we already have a terminating null, so we're okay to just do matches */
  loc = match_result(player, box, NOTYPE, MAT_NEIGHBOR | MAT_POSSESSION);
  if (!GoodObject(loc))
    return NOTHING;

  /* now we match on the contents */
  return (match_result(loc, obj, NOTYPE, MAT_POSSESSION));
}


void
page_return(player, target, type, message, def)
    dbref player;
    dbref target;
    const char *type;
    const char *message;
    const char *def;
{
  /* code for auto-return page - HAVEN, IDLE, and AWAY messages */

  ATTR *d;
  char buff[BUFFER_LEN], *bp;
  char const *asave, *ap;
  struct tm *ptr;
  time_t t;

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
	t = time(NULL);
	ptr = (struct tm *) localtime(&t);
	notify(player, tprintf("%s message from %s: %s", type,
			       Name(target), buff));
	if (!Haven(target))
	  notify(target,
		 tprintf("[%d:%02d] %s message sent to %s.",
			 MILITARY_TIME ? ptr->tm_hour :
			 (ptr->tm_hour ? (ptr->tm_hour > 12 ? (ptr->tm_hour - 12) : ptr->tm_hour) : 12),
			 ptr->tm_min, type, Name(player)));
      }
    } else if (def && *def)
      notify(player, def);
  }
}

dbref
where_is(thing)
    dbref thing;
{
  /* returns "real" location of object. This is the location for players
   * and things, source for exits, and NOTHING for rooms.
   */

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

int
nearby(obj1, obj2)
    dbref obj1;
    dbref obj2;
{
  /* returns 1 if obj1 is "nearby" object2. "Nearby" is defined as:  
   *   obj1 is in the same room as obj2, obj1 is being carried by   
   *   obj2, obj1 is carrying obj2. Returns 0 if object isn't nearby 
   *   or the input is invalid.
   */
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

void
do_verb(player, cause, arg1, argv)
    dbref player;
    dbref cause;
    char *arg1;
    char **argv;
{
  /* user-defined verbs */

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
    notify(player, "What was the victim of the verb?");
    return;
  }
  /* find the object that executes the action */

  if (!argv || !argv[1] || !*argv[1]) {
    notify(player, "What do you want to do with the verb?");
    return;
  }
  actor = match_result(player, argv[1], NOTYPE, MAT_NEAR_THINGS);

  if (actor == NOTHING) {
    notify(player, "What do you want to do the verb?");
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
    notify(player, "Permission denied.");
    return;
  }
  /* We're okay.  Send out messages. */

  for (i = 0; i < 10; i++) {
    wsave[i] = wenv[i];
    wenv[i] = argv[i + 7];
  }

  did_it(actor, victim,
	 upcasestr(argv[2]), argv[3], upcasestr(argv[4]), argv[5],
	 NULL, Location(actor));

  for (i = 0; i < 10; i++)
    wenv[i] = wsave[i];

  /* Now we copy our args into the stack, and do the command. */

  for (i = 0; i < 10; i++)
    wnxt[i] = argv[i + 7];

  charge_action(actor, victim, upcasestr(argv[6]));
}

struct guh_args {
  char *buff;
  char *bp;
  char *lookfor;
  int len;
  int insensitive;
};

static int
grep_util_helper(player, thing, pattern, atr, args)
    dbref player;
    dbref thing;
    char const *pattern;
    ATTR *atr;
    void *args;
{
  struct guh_args *guh = args;
  int found;
  char *s;

  s = (char *) uncompress(AL_STR(atr));		/* warning: static */
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

char *
grep_util(player, thing, pattern, lookfor, len, insensitive)
    dbref player;
    dbref thing;
    char *pattern;
    char *lookfor;
    int len;
    int insensitive;
				/* strlen(lookfor) */
				/* 1 if case-insensitive grep, 0 if not */
{
  /* returns a list of attributes which match <pattern> on <thing>
   * whose contents have <lookfor>
   */
  struct guh_args guh;

  guh.buff = (char *) malloc(BUFFER_LEN + 1);
  guh.bp = guh.buff;
  guh.lookfor = lookfor;
  guh.len = len;
  guh.insensitive = insensitive;
  (void) atr_iter_get(player, thing, pattern, grep_util_helper, &guh);
  *guh.bp = '\0';
  return guh.buff;
}

struct gh_args {
  char *lookfor;
  int len;
  int insensitive;
  char repl[BUFFER_LEN];
  int rlen;
};

static int
grep_helper(player, thing, pattern, atr, args)
    dbref player;
    dbref thing;
    char const *pattern;
    ATTR *atr;
    void *args;
{
  struct gh_args *gh = args;
  int found;
  int d;
  char *s;
  char tbuf1[BUFFER_LEN];

  found = 0;
  s = (char *) uncompress(AL_STR(atr));		/* warning: static */
  for (d = 0; (d < BUFFER_LEN) && *s;) {
    if ((gh->insensitive && !strncasecmp(gh->lookfor, s, gh->len)) ||
	(!gh->insensitive && !strncmp(gh->lookfor, s, gh->len))) {
      found = 1;
      if ((d + gh->rlen) < BUFFER_LEN) {
	strcpy(tbuf1 + d, ANSI_HILITE);
	strncpy(tbuf1 + d + strlen(ANSI_HILITE), s, gh->len);
	strcpy(tbuf1 + d + strlen(ANSI_HILITE) + gh->len, ANSI_NORMAL);
	d += gh->rlen;
	s += gh->len;
      } else
	tbuf1[d++] = *s++;
    } else
      tbuf1[d++] = *s++;
  }
  tbuf1[d++] = 0;
  /* if we got it, display it */
  if (found)
    notify(player, tprintf("%s%s [#%d%s%s %s",
		       ANSI_HILITE, AL_NAME(atr), Owner(AL_CREATOR(atr)),
			   (AL_FLAGS(atr) & AF_LOCKED) ? "+]:" : "]:",
			   ANSI_NORMAL, tbuf1));
  return found;
}

void
do_grep(player, obj, lookfor, flag, insensitive)
    dbref player;
    char *obj;
    char *lookfor;
    int flag;
    int insensitive;


				/* this is an UNPARSED arg */
				/* 0 for just list, 1 for hilites */
				/* 0 for case-sensitive, 1 if not */
{
  struct gh_args gh;
  dbref thing;
  char *pattern;
  int len;
  char *tp;

  if ((len = strlen(lookfor)) < 1) {
    notify(player, "What pattern do you want to grep for?");
    return;
  }
  /* find the attribute pattern */
  pattern = (char *) index(obj, '/');
  if (!pattern)
    pattern = (char *) "*";	/* set it to global match */
  else
    *pattern++ = '\0';

  /* now we've got the object. match for it. */
  if ((thing = noisy_match_result(player, obj, NOTYPE, MAT_EVERYTHING)) == NOTHING)
    return;
  if (!Can_Examine(player, thing)) {
    notify(player, "Permission denied.");
    return;
  }
  /* we can think of adding in the hilites like doing a find and replace */
  sprintf(gh.repl, "%s%s%s", ANSI_HILITE, lookfor, ANSI_NORMAL);
  gh.rlen = strlen(gh.repl);

  if (flag) {
    gh.lookfor = lookfor;
    gh.len = len;
    gh.insensitive = insensitive;
    (void) atr_iter_get(player, thing, pattern, grep_helper, &gh);
  } else {
    tp = grep_util(player, thing, pattern, lookfor, len, insensitive);
    notify(player, tprintf("Matches of '%s' on %s(#%d): %s", lookfor,
			   Name(thing), thing, tp));
    free(tp);
  }
}
