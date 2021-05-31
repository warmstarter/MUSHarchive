/* look.c */

#include "config.h"
#include "copyrite.h"

/* commands which look at things */
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
#include "ansi.h"
#include "pueblo.h"
#ifdef CHAT_SYSTEM
#include "extchat.h"
#endif
#include "parse.h"
#include "privtab.h"
#include "confmagic.h"

extern void decompile_flags _((dbref player, dbref thing, const char *name));	/* from flags.c */
extern void decompile_powers _((dbref player, dbref thing, const char *name));	/* from flags.c */
extern PRIV attr_privs[];

static void look_exits _((dbref player, dbref loc, const char *exit_name));
static void look_contents _((dbref player, dbref loc, const char *contents_name));
static void look_atrs _((dbref player, dbref thing, const char *mstr));
static void look_simple _((dbref player, dbref thing));
static int decompile_helper _((dbref player, dbref thing, char const *pattern, ATTR *atr, void *args));
static int look_helper _((dbref player, dbref thing, char const *pattern, ATTR *atr, void *args));
void look_room _((dbref player, dbref loc, int flag));
void do_look_around _((dbref player));
void do_look_at _((dbref player, const char *name, int key));
#ifdef CHAT_SYSTEM
static const char *channel_description _((dbref player));
#endif
void do_examine _((dbref player, const char *name, int brief));
void do_score _((dbref player));
void do_inventory _((dbref player));
void do_find _((dbref player, const char *name, char **argv));
void do_sweep _((dbref player, const char *arg1));
void do_whereis _((dbref player, const char *name));
void do_entrances _((dbref player, const char *where, char **argv, int val));
void decompile_atrs _((dbref player, dbref thing, const char *name, const char *pattern, const char *prefix, int skipdef));
void decompile_locks _((dbref player, dbref thing, const char *name));
void do_decompile _((dbref player, const char *name, int dbflag, int skipdef));

static void
look_exits(player, loc, exit_name)
    dbref player;
    dbref loc;
    const char *exit_name;
{
  dbref thing;
  char *tbuf1, *tbuf2, *nbuf;
  char *s1, *s2;
  char *p;
  int exit_count, this_exit, total_count;
  ATTR *a;
  int texits;
  PUEBLOBUFF;

  /* make sure location is a room */
  if (!IsRoom(loc))
    return;

  tbuf1 = (char *) mush_malloc(BUFFER_LEN, "string");
  tbuf2 = (char *) mush_malloc(BUFFER_LEN, "string");
  nbuf = (char *) mush_malloc(BUFFER_LEN, "string");
  if (!tbuf1 || !tbuf2 || !nbuf)
    panic("Unable to allocate memory in look_exits");
  s1 = tbuf1;
  s2 = tbuf2;
  texits = exit_count = total_count = 0;
  this_exit = 1;

  a = atr_get(loc, "EXITFORMAT");
  if (a) {
    char *wsave[10], *rsave[10];
    char *arg, *buff, *bp;
    char const *sp, *save;
    int j;

    arg = (char *) mush_malloc(BUFFER_LEN, "string");
    buff = (char *) mush_malloc(BUFFER_LEN, "string");
    if (!arg || !buff)
      panic("Unable to allocate memory in look_exits");
    save_global_regs("look_exits", rsave);
    for (j = 0; j < 10; j++) {
      wsave[j] = wenv[j];
      wenv[j] = NULL;
      renv[j][0] = '\0';
    }
    bp = arg;
    DOLIST(thing, Exits(loc)) {
      if (!(DarkLegal(thing) || (Dark(loc) && !Light(thing)))) {
	if (bp != arg)
	  safe_chr(' ', arg, &bp);
	safe_str(unparse_dbref(thing), arg, &bp);
      }
    }
    *bp = '\0';
    wenv[0] = arg;
    sp = save = safe_uncompress(a->value);
    bp = buff;
    process_expression(buff, &bp, &sp, loc, player, player,
		       PE_DEFAULT, PT_DEFAULT, NULL);
    *bp = '\0';
    free((Malloc_t) save);
    notify_by(loc, player, buff);
    for (j = 0; j < 10; j++) {
      wenv[j] = wsave[j];
    }
    restore_global_regs("look_exits", rsave);
    mush_free((Malloc_t) tbuf1, "string");
    mush_free((Malloc_t) tbuf2, "string");
    mush_free((Malloc_t) nbuf, "string");
    mush_free((Malloc_t) arg, "string");
    mush_free((Malloc_t) buff, "string");
    return;
  }
  if (COMMA_EXIT_LIST) {
    if (Dark(loc)) {
      for (thing = Exits(loc); thing != NOTHING; thing = Next(thing)) {
	if (!(DarkLegal(thing)) && Light(thing)) {
	  total_count++;
	  if (!Transparented(loc) || Opaque(thing))
	    exit_count++;
	}
      }
    } else {
      for (thing = Exits(loc); thing != NOTHING; thing = Next(thing)) {
	if (!(DarkLegal(thing))) {
	  total_count++;
	  if (!Transparented(loc) || Opaque(thing))
	    exit_count++;
	}
      }
    }
    if (total_count == 0) {
      mush_free((Malloc_t) tbuf1, "string");
      mush_free((Malloc_t) tbuf2, "string");
      mush_free((Malloc_t) nbuf, "string");
      return;
    }
    thing = Exits(loc);

  } else {

    /* Scan the room and see if there are any visible exits */
    if (Dark(loc))
      for (thing = Exits(loc);
	   (thing != NOTHING) && (DarkLegal(thing) || !Light(thing));
	   thing = Next(thing)) ;
    else
      for (thing = Exits(loc);
	   (thing != NOTHING) && DarkLegal(thing);
	   thing = Next(thing)) ;
    if (thing == NOTHING) {
      mush_free((Malloc_t) tbuf1, "string");
      mush_free((Malloc_t) tbuf2, "string");
      mush_free((Malloc_t) nbuf, "string");
      return;
    }
  }

  PUSE;
  tag_wrap("FONT", "SIZE=+1", exit_name);
  PEND;
  notify_by(loc, player, pbuff);

  for (; thing != NOTHING; thing = Next(thing)) {
    if (Name(thing) && !DarkLegal(thing) && (!Dark(loc) ||
					     Light(thing))) {
      strcpy(pbuff, Name(thing));
      if ((p = strchr(pbuff, ';')))
	*p = '\0';
      p = nbuf;
      safe_tag_wrap("A", tprintf("XCH_CMD=\"go #%d\"", thing), pbuff, nbuf, &p);
      *p = '\0';
      if (Transparented(loc) && !(Opaque(thing))) {
	if (SUPPORT_PUEBLO && !texits) {
	  texits = 1;
	  notify_noenter_by(loc, player, tprintf("%cUL%c", TAG_START, TAG_END));
	}
	s1 = tbuf1;
	safe_tag("LI", tbuf1, &s1);
	safe_chr(' ', tbuf1, &s1);
	safe_str(nbuf, tbuf1, &s1);
	if (Location(thing) == NOTHING)
	  safe_str(" leads nowhere.", tbuf1, &s1);
	else if (Location(thing) == HOME)
	  safe_str(" leads home.", tbuf1, &s1);
	else if (Location(thing) == AMBIGUOUS)
	  safe_str(" leads to a variable location.", tbuf1, &s1);
	else if (!GoodObject(thing))
	  safe_str(" is corrupt!", tbuf1, &s1);
	else {
	  safe_str(" leads to ", tbuf1, &s1);
	  if (Name(Location(thing)))
	    safe_str(Name(Location(thing)), tbuf1, &s1);
	  safe_chr('.', tbuf1, &s1);
	}
	*s1 = '\0';
	notify_nopenter_by(loc, player, tbuf1);
      } else {
	safe_str(nbuf, tbuf2, &s2);
	if (COMMA_EXIT_LIST) {
	  if (exit_count > 1) {
	    this_exit++;
	    if (this_exit < exit_count)
	      safe_str(", ", tbuf2, &s2);
	    else if (this_exit == exit_count) {
	      if (exit_count > 2)
		safe_str(", and ", tbuf2, &s2);
	      else
		safe_str(" and ", tbuf2, &s2);
	    }
	  }
	} else {
	  safe_str("  ", tbuf2, &s2);
	}
      }
    }
  }
  if (SUPPORT_PUEBLO && texits) {
    PUSE;
    tag_cancel("UL");
    PEND;
    notify_noenter_by(loc, player, pbuff);
  }
  *s2 = '\0';
  notify_by(loc, player, tbuf2);
  mush_free((Malloc_t) tbuf1, "string");
  mush_free((Malloc_t) tbuf2, "string");
  mush_free((Malloc_t) nbuf, "string");
}


static void
look_contents(player, loc, contents_name)
    dbref player;
    dbref loc;
    const char *contents_name;
{
  dbref thing;
  dbref can_see_loc;
  ATTR *a;
  PUEBLOBUFF;
  /* check to see if he can see the location */
  /*
   * patched so that player can't see in dark rooms even if owned by that
   * player.  (he must use examine command)
   */
  can_see_loc = !Dark(loc);

  a = atr_get(loc, "CONFORMAT");
  if (a) {
    char *wsave[10], *rsave[10];
    char *arg, *buff, *bp;
    char const *sp, *save;
    int j;

    arg = (char *) mush_malloc(BUFFER_LEN, "string");
    buff = (char *) mush_malloc(BUFFER_LEN, "string");
    if (!arg || !buff)
      panic("Unable to allocate memory in look_contents");
    save_global_regs("look_contents", rsave);
    for (j = 0; j < 10; j++) {
      wsave[j] = wenv[j];
      wenv[j] = NULL;
      renv[j][0] = '\0';
    }
    bp = arg;
    DOLIST(thing, Contents(loc)) {
      if (can_see(player, thing, can_see_loc)) {
	if (bp != arg)
	  safe_chr(' ', arg, &bp);
	safe_str(unparse_dbref(thing), arg, &bp);
      }
    }
    *bp = '\0';
    wenv[0] = arg;
    sp = save = safe_uncompress(a->value);
    bp = buff;
    process_expression(buff, &bp, &sp, loc, player, player,
		       PE_DEFAULT, PT_DEFAULT, NULL);
    *bp = '\0';
    free((Malloc_t) save);
    notify_by(loc, player, buff);
    for (j = 0; j < 10; j++) {
      wenv[j] = wsave[j];
    }
    restore_global_regs("look_contents", rsave);
    mush_free((Malloc_t) arg, "string");
    mush_free((Malloc_t) buff, "string");
    return;
  }
  /* check to see if there is anything there */
  DOLIST(thing, Contents(loc)) {
    if (can_see(player, thing, can_see_loc)) {
      /* something exists!  show him everything */
      PUSE;
      tag_wrap("FONT", "SIZE=+1", contents_name);
      tag("UL");
      PEND;
      notify_nopenter_by(loc, player, pbuff);
      DOLIST(thing, Contents(loc)) {
	if (can_see(player, thing, can_see_loc)) {
	  PUSE;
	  tag("LI");
	  tag_wrap("A", tprintf("XCH_CMD=\"look #%d\"", thing), real_unparse(player, thing, 1));
	  PEND;
	  notify_by(loc, player, pbuff);
	}
      }
      PUSE;
      tag_cancel("UL");
      PEND;
      notify_noenter_by(loc, player, pbuff);
      break;			/* we're done */
    }
  }
}

static int
look_helper(player, thing, pattern, atr, args)
    dbref player;
    dbref thing;
    char const *pattern;
    ATTR *atr;
    void *args;
{
  char fbuf[BUFFER_LEN];
  char *r;

  if (EX_PUBLIC_ATTRIBS &&
      !strcmp(AL_NAME(atr), "DESCRIBE") && !strcmp(pattern, "*"))
    return 0;

  r = safe_uncompress(AL_STR(atr));
  strcpy(fbuf, privs_to_letters(attr_privs, AL_FLAGS(atr)));
  if (TINY_ATTRS) {
    if (ShowAnsi(player)) {
      if (Owner(AL_CREATOR(atr)) != Owner(thing))
	notify(player,
	       tprintf("%s%s(#%d%s):%s%s", ANSI_HILITE, AL_NAME(atr),
		       Owner(AL_CREATOR(atr)), fbuf, ANSI_NORMAL, r));
      else if (*fbuf != '\0')
	notify(player, tprintf("%s%s(%s):%s%s", ANSI_HILITE, AL_NAME(atr),
			       fbuf, ANSI_NORMAL, r));
      else
	notify(player, tprintf("%s%s:%s%s", ANSI_HILITE, AL_NAME(atr),
			       ANSI_NORMAL, r));
    } else {
      if (Owner(AL_CREATOR(atr)) != Owner(thing))
	notify(player, tprintf("%s(#%d%s):%s", AL_NAME(atr),
			       Owner(AL_CREATOR(atr)), fbuf, r));
      else if (*fbuf != '\0')
	notify(player, tprintf("%s(%s):%s", AL_NAME(atr), fbuf, r));
      else
	notify(player, tprintf("%s:%s", AL_NAME(atr), r));
    }
  } else {
    if (ShowAnsi(player)) {
      notify(player,
	     tprintf("%s%s [#%d%s]:%s %s", ANSI_HILITE, AL_NAME(atr),
		     Owner(AL_CREATOR(atr)), fbuf, ANSI_NORMAL, r));
    } else {
      notify(player, tprintf("%s [#%d%s]: %s", AL_NAME(atr),
			     Owner(AL_CREATOR(atr)), fbuf, r));
    }
  }
  free((Malloc_t) r);

  return 1;
}

static void
look_atrs(player, thing, mstr)
    dbref player;
    dbref thing;
    const char *mstr;
{
  if (!atr_iter_get(player, thing, mstr, look_helper, NULL) && mstr)
    notify(player, "No matching attributes.");
}

static void
look_simple(player, thing)
    dbref player;
    dbref thing;
{
  int flag = 0;
  PUEBLOBUFF;
  Access(thing);
  if (controls(player, thing) || Visual(thing)) {
    PUSE;
    tag_wrap("FONT", "SIZE=+2", unparse_object(player, thing));
    PEND;
    notify(player, pbuff);
  }
  did_it(player, thing, "DESCRIBE", "You see nothing special.", "ODESCRIBE",
	 NULL, "ADESCRIBE", NOTHING);
  if (IsExit(thing) && Transparented(thing)) {
    if (Cloudy(thing))
      flag = 3;
    else
      flag = 1;
  } else if (Cloudy(thing))
    flag = 4;
  if (flag) {
    if (Location(thing) == HOME)
      look_room(player, Home(player), flag);
    else if (GoodObject(thing) && GoodObject(Destination(thing)))
      look_room(player, Destination(thing), flag);
  }
}

void
look_room(player, loc, flag)
    dbref player;
    dbref loc;
    int flag;
{
  /* look at a room. Flag value of this function:
   *   0  --  normal look, caused by "look" command.
   *   1  --  remote look, done through a TRANSPARENTED exit.
   *   2  --  auto-look, caused by moving. Obey TERSE.
   *   3  --  remote look, through a CLOUDY TRANSPARENTED exit: desc only
   *   4  --  remote look, though a CLOUDY (!TRANS) exit: contents only
   */

  PUEBLOBUFF;
  ATTR *a;

  if (loc == NOTHING)
    return;

  /* don't give the unparse if looking through Transparent exit */
  if ((flag == 0) || (flag == 2)) {
    PUSE;
    tag("XCH_PAGE CLEAR=\"LINKS IMAGES PLUGINS\"");
    if (SUPPORT_PUEBLO && flag == 2) {
      a = atr_get(loc, "VRML_URL");
      if (a) {
	tag(tprintf("IMG XCH_GRAPH=LOAD HREF=\"%s\"", uncompress(a->value)));
      } else {
	tag("IMG XCH_GRAPH=HIDE");
      }
    }
    tag("HR");
#ifdef TREKMUSH
    if (Typeof(loc) != TYPE_ROOM) {
      tag_wrap("FONT", "SIZE=+3", unparse_object(player, loc));
    } else
      if (!SpaceObj(Zone(loc)) || !GoodObject(Zone(loc))) {
        tag_wrap("FONT", "SIZE=+3", tprintf("%s %s%s[%s%sUNZONED ROOM%s%s%s]%s%s",
         unparse_object(player, loc), ANSI_BLUE, ANSI_HILITE,
         ANSI_RED, ANSI_BLINK, ANSI_NORMAL, ANSI_BLUE, ANSI_HILITE,
         ANSI_WHITE, ANSI_NORMAL));
      } else
        tag_wrap("FONT", "SIZE=+3", tprintf("%s %s%s[%s%s%s]%s%s",
         unparse_object(player, loc), ANSI_BLUE, ANSI_HILITE,
         ANSI_CYAN, Name(Zone(loc)), ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL));
#else /* TREKMUSH */
    tag_wrap("FONT", "SIZE=+3", unparse_object(player, loc));
#endif /* TREKMUSH */
    PEND;
    notify_by(loc, player, pbuff);
  }
  if (!IsRoom(loc)) {
    if ((flag != 2) || !Terse(player)) {
      if (atr_get(loc, "IDESCRIBE"))
	did_it(player, loc, "IDESCRIBE", NULL, "OIDESCRIBE", NULL, "AIDESCRIBE", NOTHING);
      else
	did_it(player, loc, "DESCRIBE", NULL, NULL, NULL, NULL, NOTHING);
    }
  }
  /* tell him the description */
  else {
    if ((flag == 0) || (flag == 2)) {
      if ((flag == 0) || !Terse(player))
	did_it(player, loc, "DESCRIBE", NULL, "ODESCRIBE", NULL, "ADESCRIBE",
	       NOTHING);
      else
	did_it(player, loc, NULL, NULL, "ODESCRIBE", NULL, "ADESCRIBE",
	       NOTHING);
    } else if (flag != 4)
      did_it(player, loc, "DESCRIBE", NULL, NULL, NULL, NULL, NOTHING);
  }
  /* tell him the appropriate messages if he has the key */
  if (IsRoom(loc) && ((flag == 0) || (flag == 2))) {
    if ((flag == 2) && Terse(player)) {
      if (could_doit(player, loc))
	did_it(player, loc, NULL, NULL, "OSUCCESS", NULL, "ASUCCESS",
	       NOTHING);
      else
	did_it(player, loc, NULL, NULL, "OFAILURE", NULL, "AFAILURE",
	       NOTHING);
    } else if ((flag != 4) && could_doit(player, loc))
      did_it(player, loc, "SUCCESS", NULL, "OSUCCESS", NULL, "ASUCCESS",
	     NOTHING);
    else
      did_it(player, loc, "FAILURE", NULL, "OFAILURE", NULL, "AFAILURE",
	     NOTHING);
  }
  /* tell him the contents */
  if (flag != 3)
    look_contents(player, loc, "Contents:");
  if ((flag == 0) || (flag == 2)) {
    look_exits(player, loc, "Obvious exits:");
  }
}

void
do_look_around(player)
    dbref player;
{
  dbref loc;
  if ((loc = Location(player)) == NOTHING)
    return;
  look_room(player, loc, 2);	/* auto-look. Obey TERSE. */
}

void
do_look_at(player, name, key)
    dbref player;
    const char *name;
    int key;			/* 0 is normal, 1 is "outside" */
{
  dbref thing;
  dbref loc;

  if (!GoodObject(Location(player)))
    return;

  if (key) {			/* look outside */
    /* can't see through opaque objects */
    if (IsRoom(Location(player)) ||
	Opaque(Location(player))) {
      notify(player, "You can't see through that.");
      return;
    }
    loc = Location(Location(player));

    if (!GoodObject(loc))
      return;

    /* look at location of location */
    if (*name == '\0') {
      look_room(player, loc, 0);
      return;
    }
    thing = match_result(loc, name, NOTYPE, MAT_PLAYER | MAT_REMOTE_CONTENTS | MAT_EXIT | MAT_REMOTES);
    if (thing == NOTHING) {
      notify(player, "I don't see that here.");
      return;
    } else if (thing == AMBIGUOUS) {
      notify(player, "I don't know which one you mean.");
      return;
    }
  } else {			/* regular look */
    if (*name == '\0') {
      look_room(player, Location(player), 0);
      return;
    }
    /* look at a thing in location */
    if ((thing = match_result(player, name, NOTYPE, MAT_NEARBY)) == NOTHING) {
      thing = parse_match_possessive(player, name);
      if (!GoodObject(thing)) {
	notify(player, "I don't see that here.");
	return;
      }
      if (Opaque(Location(thing)) &&
	  (!See_All(player) &&
       !controls(player, thing) && !controls(player, Location(thing)))) {
	notify(player, "You can't look at that from here.");
	return;
      }
    } else if (thing == AMBIGUOUS) {
      notify(player, "I can't tell which one you mean.");
      return;
    }
  }

  /* once we've determined the object to look at, it doesn't matter whether
   * this is look or look/outside.
   */

  /* we need to check for the special case of a player doing 'look here'
   * while inside an object.
   */
  if (Location(player) == thing) {
    look_room(player, thing, 0);
    return;
  }
  switch (Typeof(thing)) {
  case TYPE_ROOM:
    look_room(player, thing, 0);
    /* look_atrs(player, thing); */
    break;
  case TYPE_THING:
  case TYPE_PLAYER:
    look_simple(player, thing);
    /* look_atrs(player,thing); */
    if (!(Opaque(thing)))
      look_contents(player, thing, "Carrying:");
    break;
  default:
    look_simple(player, thing);
    /* look_atrs(player,thing); */
    break;
  }
}


#ifdef CHAT_SYSTEM
static const char *
channel_description(player)
    dbref player;
{
  static char buf[BUFFER_LEN];
  CHANLIST *c;

  if (Chanlist(player)) {
    strcpy(buf, "Channels:");
    for (c = Chanlist(player); c; c = c->next)
      sprintf(buf, "%s %s", buf, ChanName(c->chan));
  } else
    strcpy(buf, "Channels: *NONE*");
  return buf;
}
#endif

void
do_examine(player, name, brief)
    dbref player;
    const char *name;
    int brief;
{
  dbref thing;
  ATTR *a;
  char *r;
  dbref content;
  dbref exit_dbref;
  char *real_name = NULL, *attrib_name = NULL;
  char *tp;
  char *tbuf;
  int ok = 0;
  int listed = 0;
  PUEBLOBUFF;

  if (*name == '\0') {
    if ((thing = Location(player)) == NOTHING)
      return;
    attrib_name = NULL;
  } else {

    if ((attrib_name = (char *) index(name, '/')) != NULL) {
      *attrib_name = '\0';
      attrib_name++;
    }
    real_name = (char *) name;
    /* look it up */
    if ((thing = noisy_match_result(player, real_name, NOTYPE, MAT_EVERYTHING)) == NOTHING)
      return;
  }
  /*  can't examine destructed objects  */
  if (IsGarbage(thing)) {
    notify(player, "Garbage is garbage.");
    return;
  }
  /*  only look at some of the attributes */
  if (attrib_name && *attrib_name) {
    look_atrs(player, thing, attrib_name);
    return;
  }
  if (brief == 2) {
    ok = 0;
  } else {
    ok = Can_Examine(player, thing);
  }

  tbuf = (char *) mush_malloc(BUFFER_LEN, "string");
  if (!ok && (!EX_PUBLIC_ATTRIBS || !nearby(player, thing))) {
    /* if it's not examinable and we're not near it, we can only get the
     * name and the owner.
     */
    tp = tbuf;
    safe_str(object_header(player, thing), tbuf, &tp);
    safe_str((char *) " is owned by ", tbuf, &tp);
    safe_str(object_header(player, Owner(thing)), tbuf, &tp);
    *tp = '\0';
    notify(player, tbuf);
    mush_free((Malloc_t) tbuf, "string");
    return;
  }
  if (ok) {
    PUSE;
    tag_wrap("FONT", "SIZE=+3", object_header(player, thing));
    PEND;
    notify(player, pbuff);
    if (FLAGS_ON_EXAMINE)
      notify(player, flag_description(player, thing));
  }
  if (EX_PUBLIC_ATTRIBS && !brief) {
    a = atr_get_noparent(thing, "DESCRIBE");
    if (a) {
      r = safe_uncompress(a->value);
      notify(player, r);
      free((Malloc_t) r);
    }
  }
  if (ok) {
    notify(player,
	   tprintf("Owner: %s  Zone: %s  %s: %d",
		   Name(Owner(thing)),
		   object_header(player, Zone(thing)),
		   MONIES, Pennies(thing)));
    notify(player, tprintf("Parent: %s",
			   object_header(player, Parent(thing))));
    {
      struct lock_list *ll;
      for (ll = Locks(thing); ll; ll = ll->next) {
	notify(player,
	       tprintf("%s Lock: %s",
		       ll->type,
		       unparse_boolexp(player, ll->key, 0)));
      }
    }
    notify(player, tprintf("Powers: %s", power_description(thing)));

#ifdef CHAT_SYSTEM
    if (IsPlayer(thing))
      notify(player, channel_description(thing));
#endif				/* CHAT_SYSTEM */

#ifdef USE_WARNINGS
    notify(player, tprintf("Warnings checked: %s", unparse_warnings(thing)));
#endif

#ifdef CREATION_TIMES
    tp = (char *) ctime(&CreTime(thing));
    tp[strlen(tp) - 1] = '\0';
    notify(player, tprintf("Created: %s", tp));
    if (!IsPlayer(thing)) {
      tp = (char *) ctime(&ModTime(thing));
      tp[strlen(tp) - 1] = '\0';
      notify(player, tprintf("Last Modification: %s", tp));
    }
#endif
  }
  if (!brief && (EX_PUBLIC_ATTRIBS || ok)) {
    look_atrs(player, thing, NULL);
  }
  /* show contents */
  if ((Contents(thing) != NOTHING) &&
      (ok || (!IsRoom(thing) && !Opaque(thing)))) {
    DOLIST_VISIBLE(content, Contents(thing), (ok) ? GOD : player) {
      if (!listed) {
	listed = 1;
	if (IsPlayer(thing))
	  notify(player, "Carrying:");
	else
	  notify(player, "Contents:");
      }
      notify(player, object_header(player, content));
    }
  }
  if (!ok) {
    /* if not examinable, just show obvious exits and name and owner */
    if (IsRoom(thing))
      look_exits(player, thing, "Obvious exits:");
    tp = tbuf;
    safe_str(object_header(player, thing), tbuf, &tp);
    safe_str((char *) " is owned by ", tbuf, &tp);
    safe_str(object_header(player, Owner(thing)), tbuf, &tp);
    *tp = '\0';
    notify(player, tbuf);
    mush_free((Malloc_t) tbuf, "string");
    return;
  }
  switch (Typeof(thing)) {
  case TYPE_ROOM:
    /* tell him about exits */
    if (Exits(thing) != NOTHING) {
      notify(player, "Exits:");
      DOLIST(exit_dbref, Exits(thing))
	notify(player, object_header(player, exit_dbref));
    } else
      notify(player, "No exits.");
    /* print dropto if present */
    if (Location(thing) != NOTHING) {
      notify(player,
	     tprintf("Dropped objects go to: %s",
		     object_header(player, Location(thing))));
    }
    break;
  case TYPE_THING:
  case TYPE_PLAYER:
    /* print home */
    notify(player,
	   tprintf("Home: %s",
		   object_header(player, Home(thing))));	/* home */
    /* print location if player can link to it */
    if (Location(thing) != NOTHING)
      notify(player,
	     tprintf("Location: %s",
		     object_header(player, Location(thing))));
    break;
  case TYPE_EXIT:
    /* print source */
    switch (Source(thing)) {
    case NOTHING:
      fprintf(stderr,
	"*** BLEAH *** Weird exit %s(#%d) in #%d with source NOTHING.\n",
	      Name(thing), thing, Destination(thing));
      break;
    case AMBIGUOUS:
      fprintf(stderr,
	  "*** BLEAH *** Weird exit %s(#%d) in #%d with source AMBIG.\n",
	      Name(thing), thing, Destination(thing));
      break;
    case HOME:
      fprintf(stderr,
	   "*** BLEAH *** Weird exit %s(#%d) in #%d with source HOME.\n",
	      Name(thing), thing, Destination(thing));
      break;
    default:
      notify(player,
	     tprintf("Source: %s",
		     object_header(player, Source(thing))));
      break;
    }
    /* print destination */
    switch (Destination(thing)) {
    case NOTHING:
      notify(player, "Destination: *UNLINKED*");
      break;
    case HOME:
      notify(player, "Destination: *HOME*");
      break;
    default:
      notify(player,
	     tprintf("Destination: %s",
		     object_header(player, Destination(thing))));
      break;
    }
    break;
  default:
    /* do nothing */
    break;
  }
  mush_free((Malloc_t) tbuf, "string");
}

void
do_score(player)
    dbref player;
{

  notify(player,
	 tprintf("You have %d %s.",
		 Pennies(player),
		 Pennies(player) == 1 ? MONEY : MONIES));
}

void
do_inventory(player)
    dbref player;
{
  dbref thing;
  if ((thing = Contents(player)) == NOTHING) {
    notify(player, "You aren't carrying anything.");
  } else {
    notify(player, "You are carrying:");
    DOLIST(thing, thing) {
      notify(player, unparse_object(player, thing));
    }
  }

  do_score(player);
}

void
do_find(player, name, argv)
    dbref player;
    const char *name;
    char *argv[];
{
  dbref i;
  int count = 0;
  int bot = 0;
  int top = db_top;

  if (options.daytime) {
    notify(player, "Sorry, that command has been temporarily disabled.");
    return;
  }
  if (!payfor(player, FIND_COST)) {
    notify(player, tprintf("Finds cost %d %s.", FIND_COST,
			   ((FIND_COST == 1) ? MONEY : MONIES)));
    return;
  }
  /* determinte range */
  if (argv[1] && *argv[1])
    bot = atoi(argv[1]);
  if (bot < 0)
    bot = 0;
  if (argv[2] && *argv[2])
    top = atoi(argv[2]) + 1;
  if (top > db_top)
    top = db_top;

  for (i = bot; i < top; i++) {
    if (!IsGarbage(i) && !IsExit(i) && controls(player, i) &&
	(!*name || string_match(Name(i), name))) {
      notify(player, object_header(player, i));
      count++;
    }
  }
  notify(player, tprintf("*** %d objects found ***", count));
}

/* check the current location for bugs */
void
do_sweep(player, arg1)
    dbref player;
    const char *arg1;
{
  char tbuf1[BUFFER_LEN];
  char *p;
  dbref here = Location(player);
  int connect_flag = 0;
  int here_flag = 0;
  int inven_flag = 0;
  int exit_flag = 0;

  if (here == NOTHING)
    return;

  if (arg1 && *arg1) {
    if (string_prefix(arg1, "connected"))
      connect_flag = 1;
    else if (string_prefix(arg1, "here"))
      here_flag = 1;
    else if (string_prefix(arg1, "inventory"))
      inven_flag = 1;
    else if (string_prefix(arg1, "exits"))
      exit_flag = 1;
    else {
      notify(player, "Invalid parameter.");
      return;
    }
  }
  if (!inven_flag && !exit_flag) {
    notify(player, "Listening in ROOM:");

    if (connect_flag) {
      /* only worry about puppet and players who's owner's are connected */
      if (Connected(here) || (Puppet(here) && Connected(Owner(here)))) {
	if (IsPlayer(here)) {
	  notify(player, tprintf("%s is listening", Name(here)));
	} else {
	  notify(player, tprintf("%s [owner: %s] is listening.",
				 Name(here), Name(Owner(here))));
	}
      }
    } else {
      if (Hearer(here) || Listener(here)) {
	if (Connected(here))
	  notify(player, tprintf("%s (this room) [speech]. (connected)",
				 Name(here)));
	else
	  notify(player, tprintf("%s (this room) [speech].", Name(here)));
      }
      if (Commer(here))
	notify(player, tprintf("%s (this room) [commands].", Name(here)));
      if (Audible(here))
	notify(player, tprintf("%s (this room) [broadcasting].",
			       Name(here)));
    }

    for (here = Contents(here); here != NOTHING; here = Next(here)) {
      if (connect_flag) {
	/* only worry about puppet and players who's owner's are connected */
	if (Connected(here) || (Puppet(here) && Connected(Owner(here)))) {
	  if (IsPlayer(here)) {
	    notify(player, tprintf("%s is listening", Name(here)));
	  } else {
	    notify(player, tprintf("%s [owner: %s] is listening.",
				   Name(here), Name(Owner(here))));
	  }
	}
      } else {
	if (Hearer(here) || Listener(here)) {
	  if (Connected(here))
	    notify(player, tprintf("%s [speech]. (connected)", Name(here)));
	  else
	    notify(player, tprintf("%s [speech].", Name(here)));
	}
	if (Commer(here))
	  notify(player, tprintf("%s [commands].", Name(here)));
      }
    }
  }
  if (!connect_flag && !inven_flag &&
      IsRoom(Location(player))) {
    notify(player, "Listening EXITS:");
    if (Audible(Location(player))) {
      /* listening exits only work if the room is AUDIBLE */
      for (here = Exits(Location(player)); here != NOTHING;
	   here = Next(here)) {
	if (Audible(here)) {
	  strcpy(tbuf1, Name(here));
	  for (p = tbuf1; *p && (*p != ';'); p++) ;
	  *p = '\0';
	  notify(player, tprintf("%s [broadcasting].", tbuf1));
	}
      }
    }
  }
  if (!here_flag && !exit_flag) {
    notify(player, "Listening in your INVENTORY:");

    for (here = Contents(player); here != NOTHING; here = Next(here)) {
      if (connect_flag) {
	/* only worry about puppet and players who's owner's are connected */
	if (Connected(here) || (Puppet(here) && Connected(Owner(here)))) {
	  if (IsPlayer(here)) {
	    notify(player, tprintf("%s is listening", Name(here)));
	  } else {
	    notify(player, tprintf("%s [owner: %s] is listening.",
				   Name(here), Name(Owner(here))));
	  }
	}
      } else {
	if (Hearer(here) || Listener(here)) {
	  if (Connected(here))
	    notify(player, tprintf("%s [speech]. (connected)", Name(here)));
	  else
	    notify(player, tprintf("%s [speech].", Name(here)));
	}
	if (Commer(here))
	  notify(player, tprintf("%s [commands].", Name(here)));
      }
    }
  }
}

void
do_whereis(player, name)
    dbref player;
    const char *name;
{
  dbref thing;
  if (*name == '\0') {
    notify(player, "You must specify a valid player name.");
    return;
  }
  if ((thing = lookup_player(name)) == NOTHING) {
    notify(player, "That player does not seem to exist.");
    return;
  }
  if (!Can_Locate(player, thing)) {
    notify(player, "That player wishes to have some privacy.");
    notify(thing, tprintf("%s tried to locate you and failed.",
			  Name(player)));
    return;
  }
  notify(player,
	 tprintf("%s is at: %s.", Name(thing),
		 unparse_object(player, Location(thing))));
  if (!See_All(player))
    notify(thing,
	   tprintf("%s has just located your position.",
		   Name(player)));
  return;

}

void
do_entrances(player, where, argv, val)
    dbref player;
    const char *where;
    char *argv[];
    int val;			/* 0 all, 1 exits, 2 things, 3 players, 4 rooms */
{
  dbref place;
  dbref counter;
  int exc, tc, pc, rc;		/* how many we've found */
  int exd, td, pd, rd;		/* what we're looking for */
  int bot = 0;
  int top = db_top;

  if (options.daytime) {
    notify(player, "Sorry, that command has been temporarily disabled.");
    return;
  }
  if (!where || !*where) {
    if ((place = Location(player)) == NOTHING)
      return;
  } else {
    if ((place = noisy_match_result(player, where, NOTYPE, MAT_EVERYTHING)) == NOTHING)
      return;
  }

  if (!controls(player, place) && !Search_All(player)) {
    notify(player, "Permission denied.");
    return;
  }
  if (!payfor(player, FIND_COST)) {
    notify(player, tprintf("You don't have enough %d %s to do that.",
			   FIND_COST,
			   ((FIND_COST == 1) ? MONEY : MONIES)));
    return;
  }
  /* figure out what we're looking for */
  switch (val) {
  case 1:
    exd = 1;
    td = pd = rd = 0;
    break;
  case 2:
    td = 1;
    exd = pd = rd = 0;
    break;
  case 3:
    pd = 1;
    exd = td = rd = 0;
    break;
  case 4:
    rd = 1;
    exd = td = pd = 0;
    break;
  default:
    exd = td = pd = rd = 1;
  }

  exc = tc = pc = rc = 0;

  /* determine range */
  if (argv[1] && *argv[1])
    bot = atoi(argv[1]);
  if (bot < 0)
    bot = 0;
  if (argv[2] && *argv[2])
    top = atoi(argv[2]) + 1;
  if (top > db_top)
    top = db_top;

  for (counter = bot; counter < top; counter++) {
    if (controls(player, place) || controls(player, counter)) {
      switch (Typeof(counter)) {
      case TYPE_EXIT:
	if (exd) {
	  if (Location(counter) == place) {
	    notify(player, tprintf("%s(#%d) [from: %s(#%d)]", Name(counter),
				   counter, Name(Source(counter)),
				   Source(counter)));
	    exc++;
	  }
	}
	break;
      case TYPE_ROOM:
	if (rd) {
	  if (Location(counter) == place) {
	    notify(player, tprintf("%s(#%d) [dropto]",
				   Name(counter), counter));
	    rc++;
	  }
	}
	break;
      case TYPE_THING:
	if (td) {
	  if (Home(counter) == place) {
	    notify(player, tprintf("%s(#%d) [home]",
				   Name(counter), counter));
	    tc++;
	  }
	}
	break;
      case TYPE_PLAYER:
	if (pd) {
	  if (Home(counter) == place) {
	    notify(player, tprintf("%s(#%d) [home]",
				   Name(counter), counter));
	    pc++;
	  }
	}
	break;
      }
    }
  }

  if (!exc && !tc && !pc && !rc) {
    notify(player, "Nothing found.");
    return;
  } else {
    notify(player, "----------  Entrances Done  ----------");
    notify(player,
    tprintf("Totals: Rooms...%d  Exits...%d  Objects...%d  Players...%d",
	    rc, exc, tc, pc));
    return;
  }
}

struct dh_args {
  char const *prefix;
  char const *name;
  int skipdef;
};

static int
decompile_helper(player, thing, pattern, atr, args)
    dbref player;
    dbref thing;
    char const *pattern;
    ATTR *atr;
    void *args;
{
  struct dh_args *dh = args;
  ATTR *ptr;
  char msg[BUFFER_LEN];
  char *bp;
  ptr = atr_match(AL_NAME(atr));
  bp = msg;
  safe_str(dh->prefix, msg, &bp);
  if (ptr && !strcmp(AL_NAME(atr), AL_NAME(ptr)))
    safe_chr('@', msg, &bp);
  else {
    ptr = NULL; /* To speed later checks */
    safe_chr('&', msg, &bp);
  }
  safe_str(AL_NAME(atr), msg, &bp);
  safe_chr(' ', msg, &bp);
  safe_str(dh->name, msg, &bp);
  safe_chr('=', msg, &bp);
  safe_str(uncompress(AL_STR(atr)), msg, &bp);
  *bp = '\0';
  notify(player, msg);
  /* Now deal with attribute flags, if not FugueEditing */
  if (!*dh->prefix) {
    /* If skipdef is on, only show sets that aren't the defaults */
    char *privs = NULL;
    if (dh->skipdef && ptr) {
      /* Standard attribute. Get the default perms, if any. */
      /* Are we different? If so, do as usual */
      if (AL_FLAGS(atr) != (AL_FLAGS(ptr) & ~AF_STATIC))
        privs = (char *)privs_to_string(attr_privs, AL_FLAGS(atr));
    } else {
      privs = (char *)privs_to_string(attr_privs, AL_FLAGS(atr));
    }
    if (privs && *privs)
      notify(player, tprintf("@set %s/%s=%s", dh->name, AL_NAME(atr), privs));
  }
  return 1;
}

void
decompile_atrs(player, thing, name, pattern, prefix, skipdef)
    dbref player;
    dbref thing;
    const char *name;
    const char *pattern;
    const char *prefix;
    int skipdef;
{
  struct dh_args dh;
  dh.prefix = prefix;
  dh.name = name;
  dh.skipdef = skipdef;
  if (!atr_iter_get(player, thing, pattern, decompile_helper, &dh))
    notify(player, "No attributes found.");
}

void
decompile_locks(player, thing, name)
    dbref player;
    dbref thing;
    const char *name;
{
  struct lock_list *ll;
  for (ll = Locks(thing); ll; ll = ll->next) {
    if (match_lock(ll->type) != NULL) {
      notify(player,
	     tprintf("@lock/%s %s=%s",
		   ll->type, name, unparse_boolexp(player, ll->key, 1)));
    } else {
      notify(player,
	     tprintf("@lock/user:%s %s=%s",
		   ll->type, name, unparse_boolexp(player, ll->key, 1)));
    }
  }
}

void
do_decompile(player, name, dbflag, skipdef)
    dbref player;
    const char *name;
    int dbflag;			/* 0 = normal, 1 = db, 2 = tf, 3=flag,4=attr */
    int skipdef;
{
  dbref thing;
  const char *object = NULL;
  char *attrib;
  ATTR *a;
  char dbnum[40];

  /* @decompile must always have an argument */
  if (!name || !*name) {
    notify(player, "What do you want to @decompile?");
    return;
  }
  attrib = (char *) index(name, '/');
  if (attrib)
    *attrib++ = '\0';

  /* find object */
  if ((thing = noisy_match_result(player, name, NOTYPE, MAT_EVERYTHING)) == NOTHING)
    return;

  if (IsGarbage(thing)) {
    notify(player, "Garbage is garbage.");
    return;
  }
  sprintf(dbnum, "#%d", thing);

  /* if we have an attribute arg specified, wild match on it */
  if (attrib && *attrib) {
    switch (dbflag) {
    case 1:
      decompile_atrs(player, thing, dbnum, attrib, "", skipdef);
      break;
    case 2:
      if (((a = atr_get_noparent(player, "TFPREFIX")) != NULL) &&
	  AL_STR(a) && *AL_STR(a)) {
	decompile_atrs(player, thing, dbnum, attrib, uncompress(AL_STR(a)), skipdef);
      } else
	decompile_atrs(player, thing, dbnum, attrib, "FugueEdit > ", skipdef);
      break;
    default:
      decompile_atrs(player, thing, Name(thing), attrib, "", skipdef);
      break;
    }
    return;
  }
  /* else we have a full decompile */
  if (!Can_Examine(player, thing)) {
    notify(player, "Permission denied.");
    return;
  }
  /* determine creation and what we call the object */
  switch (Typeof(thing)) {
  case TYPE_PLAYER:
    if (!strcasecmp(name, "me"))
      object = "me";
    else if (dbflag == 1)
      object = dbnum;
    else
      object = Name(thing);
    break;
  case TYPE_THING:
    if (dbflag == 1) {
      object = dbnum;
      break;
    } else
      object = Name(thing);
    if (dbflag != 4)
      notify(player, tprintf("@create %s", object));
    break;
  case TYPE_ROOM:
    if (dbflag == 1) {
      object = dbnum;
      break;
    } else
      object = "here";
    if (dbflag != 4)
      notify(player, tprintf("@dig/teleport %s", Name(thing)));
    break;
  case TYPE_EXIT:
    if (dbflag == 1) {
      object = dbnum;
    } else {
      object = shortname(thing);
      if (dbflag != 4)
	notify(player, tprintf("@open %s", Name(thing)));
    }
    break;
  }

  if (dbflag != 4) {
    if (Mobile(thing)) {
      if (GoodObject(Home(thing)))
	notify(player, tprintf("@link %s = #%d", object, Home(thing)));
      else if (Home(thing) == HOME)
	notify(player, tprintf("@link %s = HOME", object));
    } else {
      if (GoodObject(Destination(thing)))
	notify(player, tprintf("@link %s = #%d", object, Destination(thing)));
      else if (Destination(thing) == AMBIGUOUS)
	notify(player, tprintf("@link %s = VARIABLE", object));
      else if (Destination(thing) == HOME)
	notify(player, tprintf("@link %s = HOME", object));
    }

    if (GoodObject(Zone(thing)))
      notify(player, tprintf("@chzone %s = #%d", object, Zone(thing)));
    if (GoodObject(Parent(thing)))
      notify(player, tprintf("@parent %s=#%d", object, Parent(thing)));

    decompile_locks(player, thing, object);
    decompile_flags(player, thing, object);
    decompile_powers(player, thing, object);
  }
  if (dbflag != 3) {
    decompile_atrs(player, thing, object, "*", "", skipdef);
  }
}
