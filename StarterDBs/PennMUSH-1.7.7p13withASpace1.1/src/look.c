/**
 * \file look.c
 *
 * \brief Commands that look at things.
 *
 *
 */

#include "config.h"
#include "copyrite.h"

#include <string.h>

#include "conf.h"
#include "mushdb.h"
#include "dbdefs.h"
#include "flags.h"
#include "lock.h"
#include "attrib.h"
#include "match.h"
#include "externs.h"
#include "ansi.h"
#include "pueblo.h"
#ifdef CHAT_SYSTEM
#include "extchat.h"
#endif
#include "game.h"
#include "command.h"
#include "parse.h"
#include "privtab.h"
#include "confmagic.h"
#include "log.h"

extern void decompile_flags(dbref player, dbref thing, const char *name);	/* from flags.c */
extern void decompile_powers(dbref player, dbref thing, const char *name);	/* from flags.c */
extern const char *channel_description(dbref player);
extern PRIV attr_privs[];

static void look_exits(dbref player, dbref loc, const char *exit_name);
static void look_contents(dbref player, dbref loc, const char *contents_name);
static void look_atrs(dbref player, dbref thing, const char *mstr, int all);
static void look_simple(dbref player, dbref thing);
static void look_description(dbref player, dbref thing, char *def,
			     const char *descname, const char *descformatname);
static int decompile_helper
  (dbref player, dbref thing, char const *pattern, ATTR *atr, void *args);
static int look_helper
  (dbref player, dbref thing, char const *pattern, ATTR *atr, void *args);
static int look_helper_veiled
  (dbref player, dbref thing, char const *pattern, ATTR *atr, void *args);
void decompile_atrs(dbref player, dbref thing, const char *name,
		    const char *pattern, const char *prefix, int skipdef);
void decompile_locks(dbref player, dbref thing, const char *name, int skipdef);

static void
look_exits(dbref player, dbref loc, const char *exit_name)
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
    char *wsave[10], *rsave[NUMQ];
    char *arg, *buff, *bp, *save;
    char const *sp;
    int j;

    arg = (char *) mush_malloc(BUFFER_LEN, "string");
    buff = (char *) mush_malloc(BUFFER_LEN, "string");
    if (!arg || !buff)
      panic("Unable to allocate memory in look_exits");
    save_global_regs("look_exits", rsave);
    for (j = 0; j < 10; j++) {
      wsave[j] = wenv[j];
      wenv[j] = NULL;
    }
    for (j = 0; j < NUMQ; j++)
      renv[j][0] = '\0';
    bp = arg;
    DOLIST(thing, Exits(loc)) {
      if ((!(DarkLegal(thing) || (Dark(loc) && !Light(thing))))
	  && can_interact(thing, player, INTERACT_SEE)) {
	if (bp != arg)
	  safe_chr(' ', arg, &bp);
	safe_dbref(thing, arg, &bp);
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
  /* Scan the room and see if there are any visible exits */
  if (Dark(loc)) {
    for (thing = Exits(loc); thing != NOTHING; thing = Next(thing)) {
      if (!DarkLegal(thing) && Light(thing) &&
	  can_interact(thing, player, INTERACT_SEE)) {
	total_count++;
	if (!Transparented(loc) || Opaque(thing))
	  exit_count++;
      }
    }
  } else {
    for (thing = Exits(loc); thing != NOTHING; thing = Next(thing)) {
      if (!DarkLegal(thing) && can_interact(thing, player, INTERACT_SEE)) {
	total_count++;
	if (!Transparented(loc) || Opaque(thing))
	  exit_count++;
      }
    }
  }
  if (total_count == 0) {
    /* No visible exits. We are outta here */
    mush_free((Malloc_t) tbuf1, "string");
    mush_free((Malloc_t) tbuf2, "string");
    mush_free((Malloc_t) nbuf, "string");
    return;
  }

  PUSE;
  tag_wrap("FONT", "SIZE=+1", exit_name);
  PEND;
  notify_by(loc, player, pbuff);

  for (thing = Exits(loc); thing != NOTHING; thing = Next(thing)) {
    if (!DarkLegal(thing) && (!Dark(loc) || Light(thing))
	&& can_interact(thing, player, INTERACT_SEE)) {
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
	if (Location(thing) == NOTHING)
	  safe_format(tbuf1, &s1, T("%s leads nowhere."), nbuf);
	else if (Location(thing) == HOME)
	  safe_format(tbuf1, &s1, T("%s leads home."), nbuf);
	else if (Location(thing) == AMBIGUOUS)
	  safe_format(tbuf1, &s1, T("%s leads to a variable location."), nbuf);
	else if (!GoodObject(thing))
	  safe_format(tbuf1, &s1, T("%s is corrupt!"), nbuf);
	else {
	  safe_format(tbuf1, &s1, T("%s leads to %s."), nbuf,
		      Name(Location(thing)));
	}
	*s1 = '\0';
	notify_nopenter_by(loc, player, tbuf1);
      } else {
	if (COMMA_EXIT_LIST) {
	  safe_itemizer(this_exit, (this_exit == exit_count),
			",", T("and"), " ", tbuf2, &s2);
	  safe_str(nbuf, tbuf2, &s2);
	  this_exit++;
	} else {
	  safe_str(nbuf, tbuf2, &s2);
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
look_contents(dbref player, dbref loc, const char *contents_name)
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
    char *wsave[10], *rsave[NUMQ];
    char *arg, *buff, *bp, *save;
    char const *sp;
    int j;

    arg = (char *) mush_malloc(BUFFER_LEN, "string");
    buff = (char *) mush_malloc(BUFFER_LEN, "string");
    if (!arg || !buff)
      panic("Unable to allocate memory in look_contents");
    save_global_regs("look_contents", rsave);
    for (j = 0; j < 10; j++) {
      wsave[j] = wenv[j];
      wenv[j] = NULL;
    }
    for (j = 0; j < NUMQ; j++)
      renv[j][0] = '\0';
    bp = arg;
    DOLIST(thing, Contents(loc)) {
      if (can_see(player, thing, can_see_loc)) {
	if (bp != arg)
	  safe_chr(' ', arg, &bp);
	safe_dbref(thing, arg, &bp);
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
	  tag_wrap("A", tprintf("XCH_CMD=\"look #%d\"", thing),
		   unparse_object_myopic(player, thing));
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
look_helper_veiled(dbref player, dbref thing __attribute__ ((__unused__)),
		   char const *pattern, ATTR *atr, void *args
		   __attribute__ ((__unused__)))
{
  char fbuf[BUFFER_LEN];
  char *r;

  if (EX_PUBLIC_ATTRIBS &&
      !strcmp(AL_NAME(atr), "DESCRIBE") && !strcmp(pattern, "*"))
    return 0;
  strcpy(fbuf, privs_to_letters(attr_privs, AL_FLAGS(atr)));
  if (AL_FLAGS(atr) & AF_VEILED) {
    if (ShowAnsi(player))
      notify_format(player,
		    "%s%s [#%d%s]%s is veiled", ANSI_HILITE, AL_NAME(atr),
		    Owner(AL_CREATOR(atr)), fbuf, ANSI_NORMAL);
    else
      notify_format(player,
		    "%s [#%d%s] is veiled", AL_NAME(atr),
		    Owner(AL_CREATOR(atr)), fbuf);
  } else {
    r = safe_uncompress(AL_STR(atr));
    if (ShowAnsi(player))
      notify_format(player,
		    "%s%s [#%d%s]:%s %s", ANSI_HILITE, AL_NAME(atr),
		    Owner(AL_CREATOR(atr)), fbuf, ANSI_NORMAL, r);
    else
      notify_format(player, "%s [#%d%s]: %s", AL_NAME(atr),
		    Owner(AL_CREATOR(atr)), fbuf, r);
    free((Malloc_t) r);
  }
  return 1;
}

static int
look_helper(dbref player, dbref thing __attribute__ ((__unused__)),
	    char const *pattern, ATTR *atr, void *args
	    __attribute__ ((__unused__)))
{
  char fbuf[BUFFER_LEN];
  char *r;

  if (EX_PUBLIC_ATTRIBS &&
      !strcmp(AL_NAME(atr), "DESCRIBE") && !strcmp(pattern, "*"))
    return 0;
  strcpy(fbuf, privs_to_letters(attr_privs, AL_FLAGS(atr)));
  r = safe_uncompress(AL_STR(atr));
  if (ShowAnsi(player))
    notify_format(player,
		  "%s%s [#%d%s]:%s %s", ANSI_HILITE, AL_NAME(atr),
		  Owner(AL_CREATOR(atr)), fbuf, ANSI_NORMAL, r);
  else
    notify_format(player, "%s [#%d%s]: %s", AL_NAME(atr),
		  Owner(AL_CREATOR(atr)), fbuf, r);
  free((Malloc_t) r);
  return 1;
}

static void
look_atrs(dbref player, dbref thing, const char *mstr, int all)
{
  if (all || (mstr && *mstr && !wildcard(mstr))) {
    if (!atr_iter_get(player, thing, mstr, look_helper, NULL) && mstr)
      notify(player, T("No matching attributes."));
  } else {
    if (!atr_iter_get(player, thing, mstr, look_helper_veiled, NULL) && mstr)
      notify(player, T("No matching attributes."));
  }
}

static void
look_simple(dbref player, dbref thing)
{
  int flag = 0;
  PUEBLOBUFF;

  PUSE;
  tag_wrap("FONT", "SIZE=+2", unparse_object_myopic(player, thing));
  PEND;
  notify(player, pbuff);
  look_description(player, thing, T("You see nothing special."), "DESCRIBE",
		   "DESCFORMAT");
  did_it(player, thing, NULL, NULL, "ODESCRIBE", NULL, "ADESCRIBE", NOTHING);
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

/** Look at a room.
 * The style parameter tells you what kind of look it is:
 * LOOK_NORMAL (caused by "look"), LOOK_TRANS (look through a transparent
 * exit), LOOK_AUTO (automatic look, by moving),
 * LOOK_CLOUDY (look through a cloudy exit - contents only), LOOK_CLOUDYTRANS
 * (look through a cloudy transparent exit - desc only).
 * \param player the looker.
 * \param loc room being looked at.
 * \param style how the room is being looked at.
 */
void
look_room(dbref player, dbref loc, enum look_type style)
{

  PUEBLOBUFF;
  ATTR *a;

  if (loc == NOTHING)
    return;

  /* don't give the unparse if looking through Transparent exit */
  if (style == LOOK_NORMAL || style == LOOK_AUTO) {
    PUSE;
    tag("XCH_PAGE CLEAR=\"LINKS IMAGES PLUGINS\"");
    if (SUPPORT_PUEBLO && style == LOOK_AUTO) {
      a = atr_get(loc, "VRML_URL");
      if (a) {
	tag(tprintf("IMG XCH_GRAPH=LOAD HREF=\"%s\"", uncompress(a->value)));
      } else {
	tag("IMG XCH_GRAPH=HIDE");
      }
    }
    tag("HR");
    tag_wrap("FONT", "SIZE=+3", unparse_room(player, loc));
    PEND;
    notify_by(loc, player, pbuff);
  }
  if (!IsRoom(loc)) {
    if (style != LOOK_AUTO || !Terse(player)) {
      if (atr_get(loc, "IDESCRIBE")) {
	look_description(player, loc, NULL, "IDESCRIBE", "IDESCFORMAT");
	did_it(player, loc, NULL, NULL, "OIDESCRIBE", NULL,
	       "AIDESCRIBE", NOTHING);
      } else
	look_description(player, loc, NULL, "DESCRIBE", "DESCFORMAT");
    }
  }
  /* tell him the description */
  else {
    if (style == LOOK_NORMAL || style == LOOK_AUTO) {
      if (style == LOOK_NORMAL || !Terse(player)) {
	look_description(player, loc, NULL, "DESCRIBE", "DESCFORMAT");
	did_it(player, loc, NULL, NULL, "ODESCRIBE", NULL,
	       "ADESCRIBE", NOTHING);
      } else
	did_it(player, loc, NULL, NULL, "ODESCRIBE", NULL, "ADESCRIBE",
	       NOTHING);
    } else if (style != LOOK_CLOUDY)
      look_description(player, loc, NULL, "DESCRIBE", "DESCFORMAT");
  }
  /* tell him the appropriate messages if he has the key */
  if (IsRoom(loc) && (style == LOOK_NORMAL || style == LOOK_AUTO)) {
    if (style == LOOK_AUTO && Terse(player)) {
      if (could_doit(player, loc))
	did_it(player, loc, NULL, NULL, "OSUCCESS", NULL, "ASUCCESS", NOTHING);
      else
	did_it(player, loc, NULL, NULL, "OFAILURE", NULL, "AFAILURE", NOTHING);
    } else if (could_doit(player, loc))
      did_it(player, loc, "SUCCESS", NULL, "OSUCCESS", NULL, "ASUCCESS",
	     NOTHING);
    else
      did_it(player, loc, "FAILURE", NULL, "OFAILURE", NULL, "AFAILURE",
	     NOTHING);
  }
  /* tell him the contents */
  if (style != LOOK_CLOUDYTRANS)
    look_contents(player, loc, T("Contents:"));
  if (style == LOOK_NORMAL || style == LOOK_AUTO) {
    look_exits(player, loc, T("Obvious exits:"));
  }
}

static void
look_description(dbref player, dbref thing, char *def, const char *descname,
		 const char *descformatname)
{
  /* Show thing's description to player, obeying DESCFORMAT if set */
  ATTR *a, *f;
  char *preserveq[NUMQ];
  char *preserves[10];
  char buff[BUFFER_LEN], fbuff[BUFFER_LEN];
  char *bp, *fbp, *asave;
  char const *ap;

  if (!GoodObject(player) || !GoodObject(thing))
    return;
  save_global_regs("look_desc_save", preserveq);
  save_global_env("look_desc_save", preserves);
  a = atr_get(thing, descname);
  if (a) {
    /* We have a DESCRIBE, evaluate it into buff */
    asave = safe_uncompress(a->value);
    ap = asave;
    bp = buff;
    process_expression(buff, &bp, &ap, thing, player, player,
		       PE_DEFAULT, PT_DEFAULT, NULL);
    *bp = '\0';
    free((Malloc_t) asave);
  }
  f = atr_get(thing, descformatname);
  if (f) {
    /* We have a DESCFORMAT, evaluate it into fbuff and use it */
    /* If we have a DESCRIBE, pass the evaluated version as %0 */
    wenv[0] = a ? buff : NULL;
    asave = safe_uncompress(f->value);
    ap = asave;
    fbp = fbuff;
    process_expression(fbuff, &fbp, &ap, thing, player, player,
		       PE_DEFAULT, PT_DEFAULT, NULL);
    *fbp = '\0';
    free((Malloc_t) asave);
    notify_by(thing, player, fbuff);
  } else if (a) {
    /* DESCRIBE only */
    notify_by(thing, player, buff);
  } else if (def) {
    /* Nothing, go with the default message */
    notify_by(thing, player, def);
  }
  restore_global_regs("look_desc_save", preserveq);
  restore_global_env("look_desc_save", preserves);
}

/** An automatic look (due to motion).
 * \param player the looker.
 */
void
do_look_around(dbref player)
{
  dbref loc;
  if ((loc = Location(player)) == NOTHING)
    return;
  look_room(player, loc, LOOK_AUTO);	/* auto-look. Obey TERSE. */
}

/** Look at something.
 * \param player the looker.
 * \param name name of object to look at.
 * \param key 0 for normal look, 1 for look/outside.
 */
void
do_look_at(dbref player, const char *name, int key)
{
  dbref thing;
  dbref loc;

  if (!GoodObject(Location(player)))
    return;

  if (key) {			/* look outside */
    /* can't see through opaque objects */
    if (IsRoom(Location(player)) || Opaque(Location(player))) {
      notify(player, T("You can't see through that."));
      return;
    }
    loc = Location(Location(player));

    if (!GoodObject(loc))
      return;

    /* look at location of location */
    if (*name == '\0') {
      look_room(player, loc, LOOK_NORMAL);
      return;
    }
    thing =
      match_result(loc, name, NOTYPE,
		   MAT_PLAYER | MAT_REMOTE_CONTENTS | MAT_EXIT | MAT_REMOTES);
    if (thing == NOTHING) {
      notify(player, T("I don't see that here."));
      return;
    } else if (thing == AMBIGUOUS) {
      notify(player, T("I don't know which one you mean."));
      return;
    }
  } else {			/* regular look */
    if (*name == '\0') {
      look_room(player, Location(player), LOOK_NORMAL);
      return;
    }
    /* look at a thing in location */
    if ((thing = match_result(player, name, NOTYPE, MAT_NEARBY)) == NOTHING) {
      thing = parse_match_possessive(player, name);
      if (!GoodObject(thing)) {
	notify(player, T("I don't see that here."));
	return;
      }
      if (Opaque(Location(thing)) &&
	  (!See_All(player) &&
	   !controls(player, thing) && !controls(player, Location(thing)))) {
	notify(player, T("You can't look at that from here."));
	return;
      }
    } else if (thing == AMBIGUOUS) {
      notify(player, T("I can't tell which one you mean."));
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
    look_room(player, thing, LOOK_NORMAL);
    return;
  }
  switch (Typeof(thing)) {
  case TYPE_ROOM:
    look_room(player, thing, LOOK_NORMAL);
    break;
  case TYPE_THING:
  case TYPE_PLAYER:
    look_simple(player, thing);
    if (!(Opaque(thing)))
      look_contents(player, thing, "Carrying:");
    break;
  default:
    look_simple(player, thing);
    break;
  }
}


/** Examine an object.
 * \param player the enactor doing the examining.
 * \param name name of object to examine.
 * \param brief if 1, a brief examination.
 * \param all if 1, include veiled attributes.
 */
void
do_examine(dbref player, const char *name, int brief, int all)
{
  dbref thing;
  ATTR *a;
  char *r;
  dbref content;
  dbref exit_dbref;
  const char *real_name = NULL;
  char *attrib_name = NULL;
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

    if ((attrib_name = strchr(name, '/')) != NULL) {
      *attrib_name = '\0';
      attrib_name++;
    }
    real_name = name;
    /* look it up */
    if ((thing =
	 noisy_match_result(player, real_name, NOTYPE,
			    MAT_EVERYTHING)) == NOTHING)
      return;
  }
  /*  can't examine destructed objects  */
  if (IsGarbage(thing)) {
    notify(player, T("Garbage is garbage."));
    return;
  }
  /*  only look at some of the attributes */
  if (attrib_name && *attrib_name) {
    look_atrs(player, thing, attrib_name, all);
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
    safe_str(T(" is owned by "), tbuf, &tp);
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
  if (EX_PUBLIC_ATTRIBS && (brief != 1)) {
    a = atr_get_noparent(thing, "DESCRIBE");
    if (a) {
      r = safe_uncompress(a->value);
      notify(player, r);
      free((Malloc_t) r);
    }
  }
  if (ok) {
    notify_format(player,
		  T("Owner: %s  Zone: %s  %s: %d"),
		  Name(Owner(thing)),
		  object_header(player, Zone(thing)), MONIES, Pennies(thing));
    notify_format(player,
		  T("Parent: %s"), object_header(player, Parent(thing)));
    {
      struct lock_list *ll;
      for (ll = Locks(thing); ll; ll = ll->next) {
	notify_format(player, T("%s Lock [#%d%s]: %s"),
		      L_TYPE(ll), L_CREATOR(ll), lock_flags(ll),
		      unparse_boolexp(player, L_KEY(ll), UB_ALL));
      }
    }
    notify_format(player, T("Powers: %s"), power_description(thing));

#ifdef CHAT_SYSTEM
    notify(player, channel_description(thing));
#endif				/* CHAT_SYSTEM */

    notify_format(player, T("Warnings checked: %s"), unparse_warnings(thing));

    tp = (char *) ctime(&CreTime(thing));
    tp[strlen(tp) - 1] = '\0';
    notify_format(player, T("Created: %s"), tp);
    if (!IsPlayer(thing)) {
      tp = (char *) ctime(&ModTime(thing));
      tp[strlen(tp) - 1] = '\0';
      notify_format(player, T("Last Modification: %s"), tp);
    }
  }
  if ((brief != 1) && (EX_PUBLIC_ATTRIBS || ok)) {
    look_atrs(player, thing, NULL, all);
  }
  /* show contents */
  if ((Contents(thing) != NOTHING) &&
      (ok || (!IsRoom(thing) && !Opaque(thing)))) {
    DOLIST_VISIBLE(content, Contents(thing), (ok) ? GOD : player) {
      if (!listed) {
	listed = 1;
	if (IsPlayer(thing))
	  notify(player, T("Carrying:"));
	else
	  notify(player, T("Contents:"));
      }
      notify(player, object_header(player, content));
    }
  }
  if (!ok) {
    /* if not examinable, just show obvious exits and name and owner */
    if (IsRoom(thing))
      look_exits(player, thing, T("Obvious exits:"));
    tp = tbuf;
    safe_str(object_header(player, thing), tbuf, &tp);
    safe_str(T(" is owned by "), tbuf, &tp);
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
      notify(player, T("Exits:"));
      DOLIST(exit_dbref, Exits(thing))
	notify(player, object_header(player, exit_dbref));
    } else
      notify(player, T("No exits."));
    /* print dropto if present */
    if (Location(thing) != NOTHING) {
      notify_format(player,
		    T("Dropped objects go to: %s"),
		    object_header(player, Location(thing)));
    }
    break;
  case TYPE_THING:
  case TYPE_PLAYER:
    /* print home */
    notify_format(player, T("Home: %s"), object_header(player, Home(thing)));	/* home */
    /* print location if player can link to it */
    if (Location(thing) != NOTHING)
      notify_format(player,
		    T("Location: %s"), object_header(player, Location(thing)));
    break;
  case TYPE_EXIT:
    /* print source */
    switch (Source(thing)) {
    case NOTHING:
      do_rawlog(LT_ERR,
		T
		("*** BLEAH *** Weird exit %s(#%d) in #%d with source NOTHING."),
		Name(thing), thing, Destination(thing));
      break;
    case AMBIGUOUS:
      do_rawlog(LT_ERR,
		T("*** BLEAH *** Weird exit %s(#%d) in #%d with source AMBIG."),
		Name(thing), thing, Destination(thing));
      break;
    case HOME:
      do_rawlog(LT_ERR,
		T("*** BLEAH *** Weird exit %s(#%d) in #%d with source HOME."),
		Name(thing), thing, Destination(thing));
      break;
    default:
      notify_format(player,
		    T("Source: %s"), object_header(player, Source(thing)));
      break;
    }
    /* print destination */
    switch (Destination(thing)) {
    case NOTHING:
      notify(player, T("Destination: *UNLINKED*"));
      break;
    case HOME:
      notify(player, T("Destination: *HOME*"));
      break;
    default:
      notify_format(player,
		    T("Destination: %s"),
		    object_header(player, Destination(thing)));
      break;
    }
    break;
  default:
    /* do nothing */
    break;
  }
  mush_free((Malloc_t) tbuf, "string");
}

/** The score command: check a player's money.
 * \param player the enactor.
 */
void
do_score(dbref player)
{

  notify_format(player,
		T("You have %d %s."),
		Pennies(player), Pennies(player) == 1 ? MONEY : MONIES);
}

/** The inventory command.
 * \param player the enactor.
 */
void
do_inventory(dbref player)
{
  dbref thing;
  if ((thing = Contents(player)) == NOTHING) {
    notify(player, T("You aren't carrying anything."));
  } else {
    notify(player, T("You are carrying:"));
    DOLIST(thing, thing) {
      notify(player, unparse_object_myopic(player, thing));
    }
  }

  do_score(player);
}

/** The find command.
 * \param player the enactor.
 * \param name name pattern to search for.
 * \param argv array of additional arguments (for dbref ranges)
 */
void
do_find(dbref player, const char *name, char *argv[])
{
  dbref i;
  int count = 0;
  int bot = 0;
  int top = db_top;

  if (!payfor(player, FIND_COST)) {
    notify_format(player, T("Finds cost %d %s."), FIND_COST,
		  ((FIND_COST == 1) ? MONEY : MONIES));
    return;
  }
  /* determinte range */
  if (argv[1] && *argv[1]) {
    size_t offset = 0;
    if (argv[1][0] == '#')
      offset = 1;
    bot = parse_integer(argv[1] + offset);
    if (!GoodObject(bot)) {
      notify(player, T("Invalid range argument"));
      return;
    }
  }
  if (argv[2] && *argv[2]) {
    size_t offset = 0;
    if (argv[2][0] == '#')
      offset = 1;
    top = parse_integer(argv[2] + offset);
    if (!GoodObject(top)) {
      notify(player, T("Invalid range argument"));
      return;
    }
  }

  for (i = bot; i < top; i++) {
    if (!IsGarbage(i) && !IsExit(i) && controls(player, i) &&
	(!*name || string_match(Name(i), name))) {
      notify(player, object_header(player, i));
      count++;
    }
  }
  notify_format(player, T("*** %d objects found ***"), count);
}

/** Sweep the current location for bugs.
 * \verbatim
 * This implements @sweep.
 * \endverbatim
 * \param player the enactor.
 * \param arg1 optional area to sweep.
 */
void
do_sweep(dbref player, const char *arg1)
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
      notify(player, T("Invalid parameter."));
      return;
    }
  }
  if (!inven_flag && !exit_flag) {
    notify(player, T("Listening in ROOM:"));

    if (connect_flag) {
      /* only worry about puppet and players who's owner's are connected */
      if (Connected(here) || (Puppet(here) && Connected(Owner(here)))) {
	if (IsPlayer(here)) {
	  notify_format(player, T("%s is listening."), Name(here));
	} else {
	  notify_format(player, T("%s [owner: %s] is listening."),
			Name(here), Name(Owner(here)));
	}
      }
    } else {
      if (Hearer(here) || Listener(here)) {
	if (Connected(here))
	  notify_format(player, T("%s (this room) [speech]. (connected)"),
			Name(here));
	else
	  notify_format(player, T("%s (this room) [speech]."), Name(here));
      }
      if (Commer(here))
	notify_format(player, T("%s (this room) [commands]."), Name(here));
      if (Audible(here))
	notify_format(player, T("%s (this room) [broadcasting]."), Name(here));
    }

    for (here = Contents(here); here != NOTHING; here = Next(here)) {
      if (connect_flag) {
	/* only worry about puppet and players who's owner's are connected */
	if (Connected(here) || (Puppet(here) && Connected(Owner(here)))) {
	  if (IsPlayer(here)) {
	    notify_format(player, T("%s is listening."), Name(here));
	  } else {
	    notify_format(player, T("%s [owner: %s] is listening."),
			  Name(here), Name(Owner(here)));
	  }
	}
      } else {
	if (Hearer(here) || Listener(here)) {
	  if (Connected(here))
	    notify_format(player, "%s [speech]. (connected)", Name(here));
	  else
	    notify_format(player, "%s [speech].", Name(here));
	}
	if (Commer(here))
	  notify_format(player, "%s [commands].", Name(here));
      }
    }
  }
  if (!connect_flag && !inven_flag && IsRoom(Location(player))) {
    notify(player, T("Listening EXITS:"));
    if (Audible(Location(player))) {
      /* listening exits only work if the room is AUDIBLE */
      for (here = Exits(Location(player)); here != NOTHING; here = Next(here)) {
	if (Audible(here)) {
	  strcpy(tbuf1, Name(here));
	  for (p = tbuf1; *p && (*p != ';'); p++) ;
	  *p = '\0';
	  notify_format(player, "%s [broadcasting].", tbuf1);
	}
      }
    }
  }
  if (!here_flag && !exit_flag) {
    notify(player, T("Listening in your INVENTORY:"));

    for (here = Contents(player); here != NOTHING; here = Next(here)) {
      if (connect_flag) {
	/* only worry about puppet and players who's owner's are connected */
	if (Connected(here) || (Puppet(here) && Connected(Owner(here)))) {
	  if (IsPlayer(here)) {
	    notify_format(player, T("%s is listening."), Name(here));
	  } else {
	    notify_format(player, T("%s [owner: %s] is listening."),
			  Name(here), Name(Owner(here)));
	  }
	}
      } else {
	if (Hearer(here) || Listener(here)) {
	  if (Connected(here))
	    notify_format(player, "%s [speech]. (connected)", Name(here));
	  else
	    notify_format(player, "%s [speech].", Name(here));
	}
	if (Commer(here))
	  notify_format(player, "%s [commands].", Name(here));
      }
    }
  }
}

/** Locate a player.
 * \verbatim
 * This implements @whereis.
 * \endverbatim
 * \param player the enactor.
 * \param name name of player to locate.
 */
void
do_whereis(dbref player, const char *name)
{
  dbref thing;
  if (*name == '\0') {
    notify(player, T("You must specify a valid player name."));
    return;
  }
  if ((thing = lookup_player(name)) == NOTHING) {
    notify(player, T("That player does not seem to exist."));
    return;
  }
  if (!Can_Locate(player, thing)) {
    notify(player, T("That player wishes to have some privacy."));
    notify_format(thing, T("%s tried to locate you and failed."), Name(player));
    return;
  }
  notify_format(player,
		T("%s is at: %s."), Name(thing),
		unparse_object(player, Location(thing)));
  if (!See_All(player))
    notify_format(thing, T("%s has just located your position."), Name(player));
  return;

}

/** Find the entrances to a room.
 * \verbatim
 * This implements @entrances, which finds things linked to an object
 * (typically exits, but can be any type).
 * \endverbatim
 * \param player the enactor.
 * \param where name of object to find entrances on.
 * \param argv array of arguments for dbref range limitation.
 * \param val what type of 'entrances' to find.
 */
void
do_entrances(dbref player, const char *where, char *argv[], enum ent_type val)
{
  dbref place;
  dbref counter;
  int exc, tc, pc, rc;		/* how many we've found */
  int exd, td, pd, rd;		/* what we're looking for */
  int bot = 0;
  int top = db_top;

  exc = tc = pc = rc = exd = td = pd = rd = 0;

  if (!where || !*where) {
    if ((place = Location(player)) == NOTHING)
      return;
  } else {
    if ((place = noisy_match_result(player, where, NOTYPE, MAT_EVERYTHING))
	== NOTHING)
      return;
  }

  if (!controls(player, place) && !Search_All(player)) {
    notify(player, T("Permission denied."));
    return;
  }
  if (!payfor(player, FIND_COST)) {
    notify_format(player, T("You don't have enough %d %s to do that."),
		  FIND_COST, ((FIND_COST == 1) ? MONEY : MONIES));
    return;
  }
  /* figure out what we're looking for */
  switch (val) {
  case ENT_EXITS:
    exd = 1;
    td = pd = rd = 0;
    break;
  case ENT_THINGS:
    td = 1;
    exd = pd = rd = 0;
    break;
  case ENT_PLAYERS:
    pd = 1;
    exd = td = rd = 0;
    break;
  case ENT_ROOMS:
    rd = 1;
    exd = td = pd = 0;
    break;
  case ENT_ALL:
    exd = td = pd = rd = 1;
  }

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
	    notify_format(player,
			  "%s(#%d) [from: %s(#%d)]", Name(counter),
			  counter, Name(Source(counter)), Source(counter));
	    exc++;
	  }
	}
	break;
      case TYPE_ROOM:
	if (rd) {
	  if (Location(counter) == place) {
	    notify_format(player, "%s(#%d) [dropto]", Name(counter), counter);
	    rc++;
	  }
	}
	break;
      case TYPE_THING:
	if (td) {
	  if (Home(counter) == place) {
	    notify_format(player, "%s(#%d) [home]", Name(counter), counter);
	    tc++;
	  }
	}
	break;
      case TYPE_PLAYER:
	if (pd) {
	  if (Home(counter) == place) {
	    notify_format(player, "%s(#%d) [home]", Name(counter), counter);
	    pc++;
	  }
	}
	break;
      }
    }
  }

  if (!exc && !tc && !pc && !rc) {
    notify(player, T("Nothing found."));
    return;
  } else {
    notify(player, T("----------  Entrances Done  ----------"));
    notify_format(player,
		  "Totals: Rooms...%d  Exits...%d  Objects...%d  Players...%d",
		  rc, exc, tc, pc);
    return;
  }
}

/** Store arguments for decompile_helper() */
struct dh_args {
  char const *prefix;	/**< Decompile/tf prefix */
  char const *name;	/**< Decompile object name */
  int skipdef;		/**< Skip default flags on attributes if true */
};

static int
decompile_helper(dbref player, dbref thing
		 __attribute__ ((__unused__)), const char *pattern
		 __attribute__ ((__unused__)), ATTR *atr, void *args)
{
  struct dh_args *dh = args;
  ATTR *ptr;
  char msg[BUFFER_LEN];
  char *bp;

  if (AL_FLAGS(atr) & AF_NODUMP)
    return 0;

  ptr = atr_match(AL_NAME(atr));
  bp = msg;
  safe_str(dh->prefix, msg, &bp);
  if (ptr && !strcmp(AL_NAME(atr), AL_NAME(ptr)))
    safe_chr('@', msg, &bp);
  else {
    ptr = NULL;			/* To speed later checks */
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
    const char *privs = NULL;
    if (dh->skipdef && ptr) {
      /* Standard attribute. Get the default perms, if any. */
      /* Are we different? If so, do as usual */
      int npmflags = AL_FLAGS(ptr) & (~AF_PREFIXMATCH);
      if (AL_FLAGS(atr) != AL_FLAGS(ptr) && AL_FLAGS(atr) != npmflags)
	privs = privs_to_string(attr_privs, AL_FLAGS(atr));
    } else {
      privs = privs_to_string(attr_privs, AL_FLAGS(atr));
    }
    if (privs && *privs)
      notify_format(player, "@set %s/%s=%s", dh->name, AL_NAME(atr), privs);
  }
  return 1;
}

/** Decompile attributes on an object.
 * \param player the enactor.
 * \param thing object with attributes to decompile.
 * \param name name to refer to object by in decompile.
 * \param pattern pattern to match attributes to decompile.
 * \param prefix prefix to use for decompile/tf.
 * \param skipdef if true, skip showing default attribute flags.
 */
void
decompile_atrs(dbref player, dbref thing, const char *name, const char *pattern,
	       const char *prefix, int skipdef)
{
  struct dh_args dh;
  dh.prefix = prefix;
  dh.name = name;
  dh.skipdef = skipdef;
  /* Comment complaints if none are found */
  if (!atr_iter_get(player, thing, pattern, decompile_helper, &dh))
    notify(player, T("@@ No attributes found. @@"));
}

/** Decompile locks on an object.
 * \param player the enactor.
 * \param thing object with attributes to decompile.
 * \param name name to refer to object by in decompile.
 * \param skipdef if true, skip showing default lock flags.
 */
void
decompile_locks(dbref player, dbref thing, const char *name, int skipdef)
{
  lock_list *ll;
  for (ll = Locks(thing); ll; ll = ll->next) {
    if (match_lock(ll->type) != NULL) {
      notify_format(player, "@lock/%s %s=%s",
		    L_TYPE(ll), name, unparse_boolexp(player, L_KEY(ll),
						      UB_DBREF));
      if (L_FLAGS(ll)) {
	if (skipdef) {
	  const lock_list *p = get_lockproto(L_TYPE(ll));
	  if (p && L_FLAGS(ll) == L_FLAGS(p))
	    continue;
	}
	notify_format(player,
		      "@lset %s/%s=%s", name, L_TYPE(ll), lock_flags_long(ll));
      }
    } else {
      notify_format(player, "@lock/user:%s %s=%s",
		    ll->type, name, unparse_boolexp(player, ll->key, UB_DBREF));
      if (L_FLAGS(ll))
	notify_format(player,
		      "@lset %s/%s=%s", name, L_TYPE(ll), lock_flags_long(ll));

    }
  }
}

/** Decompile.
 * \verbatim
 * This implements @decompile.
 * \endverbatim
 * \param player the enactor.
 * \param name name of object to decompile.
 * \param dbflag flag for type of decompile to perform.
 * \param skipdef if true, skip showing default flags on attributes/locks.
 */
void
do_decompile(dbref player, const char *name, enum dec_type dbflag, int skipdef)
{
  dbref thing;
  const char *object = NULL;
  char *attrib;
  ATTR *a;
  char dbnum[40];

  /* @decompile must always have an argument */
  if (!name || !*name) {
    notify(player, T("What do you want to @decompile?"));
    return;
  }
  attrib = strchr(name, '/');
  if (attrib)
    *attrib++ = '\0';

  /* find object */
  if ((thing = noisy_match_result(player, name, NOTYPE, MAT_EVERYTHING)) ==
      NOTHING)
    return;

  if (IsGarbage(thing)) {
    notify(player, T("Garbage is garbage."));
    return;
  }
  sprintf(dbnum, "#%d", thing);

  /* if we have an attribute arg specified, wild match on it */
  if (attrib && *attrib) {
    switch (dbflag) {
    case DEC_DB:
      decompile_atrs(player, thing, dbnum, attrib, "", skipdef);
      break;
    case DEC_TF:
      if (((a = atr_get_noparent(player, "TFPREFIX")) != NULL) &&
	  AL_STR(a) && *AL_STR(a)) {
	decompile_atrs(player, thing, dbnum, attrib, uncompress(AL_STR(a)),
		       skipdef);
      } else
	decompile_atrs(player, thing, dbnum, attrib, "FugueEdit > ", skipdef);
      break;
    default:
      if (IsRoom(thing))
	decompile_atrs(player, thing, "here", attrib, "", skipdef);
      else
	decompile_atrs(player, thing, Name(thing), attrib, "", skipdef);
      break;
    }
    return;
  }
  /* else we have a full decompile */
  if (!Can_Examine(player, thing)) {
    notify(player, T("Permission denied."));
    return;
  }
  /* determine creation and what we call the object */
  switch (Typeof(thing)) {
  case TYPE_PLAYER:
    if (!strcasecmp(name, "me"))
      object = "me";
    else if (dbflag == DEC_DB)
      object = dbnum;
    else
      object = Name(thing);
    break;
  case TYPE_THING:
    if (dbflag == DEC_DB) {
      object = dbnum;
      break;
    } else
      object = Name(thing);
    if (dbflag != DEC_ATTR)
      notify_format(player, "@create %s", object);
    break;
  case TYPE_ROOM:
    if (dbflag == DEC_DB) {
      object = dbnum;
      break;
    } else
      object = "here";
    if (dbflag != DEC_ATTR)
      notify_format(player, "@dig/teleport %s", Name(thing));
    break;
  case TYPE_EXIT:
    if (dbflag == DEC_DB) {
      object = dbnum;
    } else {
      object = shortname(thing);
      if (dbflag != DEC_ATTR)
	notify_format(player, "@open %s", Name(thing));
    }
    break;
  }

  if (dbflag != DEC_ATTR) {
    if (Mobile(thing)) {
      if (GoodObject(Home(thing)))
	notify_format(player, "@link %s = #%d", object, Home(thing));
      else if (Home(thing) == HOME)
	notify_format(player, "@link %s = HOME", object);
    } else {
      if (GoodObject(Destination(thing)))
	notify_format(player, "@link %s = #%d", object, Destination(thing));
      else if (Destination(thing) == AMBIGUOUS)
	notify_format(player, "@link %s = VARIABLE", object);
      else if (Destination(thing) == HOME)
	notify_format(player, "@link %s = HOME", object);
    }

    if (GoodObject(Zone(thing)))
      notify_format(player, "@chzone %s = #%d", object, Zone(thing));
    if (GoodObject(Parent(thing)))
      notify_format(player, "@parent %s=#%d", object, Parent(thing));

    decompile_locks(player, thing, object, skipdef);
    decompile_flags(player, thing, object);
    decompile_powers(player, thing, object);
  }
  if (dbflag != DEC_FLAG) {
    decompile_atrs(player, thing, object, "*", "", skipdef);
  }
}
