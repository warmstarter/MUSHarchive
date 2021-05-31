
/* warnings.c - check to make sure rooms and exits are the way they
 * should be. */

#include "config.h"

#include <stdio.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif

#include "copyrite.h"
#include "conf.h"
#include "mushdb.h"
#include "intrface.h"
#include "externs.h"
#include "match.h"
#include "warnings.h"
#include "confmagic.h"

#ifdef USE_WARNINGS

#define W_UNLOCKED	0x1
#define W_LOCKED	0x2

#define W_EXIT_ONEWAY	0x1
#define W_EXIT_MULTIPLE	0x2
#define W_EXIT_MSGS	0x4
#define W_EXIT_DESC	0x8
#define W_EXIT_UNLINKED	0x10
#ifdef TREKMUSH
#define W_EXIT_ZONING	0x20
#define W_EXIT_BUILDER	0x40
#define W_EXIT_BADLINK	0x80
#endif /* TREKMUSH */
/* Space for more exit stuff */
#define W_THING_MSGS	0x100
#define W_THING_DESC	0x200
/* Space for more thing stuff */
#define W_ROOM_DESC	0x1000
#ifdef TREKMUSH
#define W_ROOM_ROOMTYPE	0x2000
#define W_ROOM_ZONING	0x4000
#define W_ROOM_BUILDER	0x8000
#endif /* TREKMUSH */
/* Space for more room stuff */
#define W_PLAYER_DESC	0x10000

/* Groups of warnings */
#define W_NONE		0
#ifdef TREKMUSH
#define W_SERIOUS	(W_EXIT_UNLINKED|W_THING_DESC|W_ROOM_DESC|W_PLAYER_DESC|W_ROOM_ZONING|W_ROOM_ROOMTYPE|W_ROOM_BUILDER|W_EXIT_ZONING|W_EXIT_BUILDER|W_EXIT_BADLINK)
#else /* TREKMUSH */
#define W_SERIOUS	(W_EXIT_UNLINKED|W_THING_DESC|W_ROOM_DESC|W_PLAYER_DESC)
#endif /* TREKMUSH */
#define W_NORMAL	(W_SERIOUS|W_EXIT_ONEWAY|W_EXIT_MULTIPLE|W_EXIT_MSGS)
#define W_EXTRA		(W_NORMAL|W_THING_MSGS)
#define W_ALL		(W_EXTRA|W_EXIT_DESC)

typedef struct a_tcheck tcheck;
struct a_tcheck {
  const char *name;
  warn_type flag;
};


static int warning_lock_type _((dbref i, const struct boolexp * l));
static void complain _((dbref player, dbref i, const char *name, const char *desc));
static void ct_room _((dbref player, dbref i, warn_type flags));
static void ct_exit _((dbref player, dbref i, warn_type flags));
static void ct_player _((dbref player, dbref i, warn_type flags));
static void ct_thing _((dbref player, dbref i, warn_type flags));
void set_initial_warnings _((dbref player));
void do_warnings _((dbref player, char *name, char *warns));
const char *unparse_warnings _((dbref thing));
static void check_topology_on _((dbref player, dbref i));
void run_topology _((void));
void do_wcheck_all _((dbref player));
void do_wcheck _((dbref player, char *name));


static tcheck checklist[] =
{
  {"none", W_NONE},		/* MUST BE FIRST! */
  {"exit-unlinked", W_EXIT_UNLINKED},
  {"thing-desc", W_THING_DESC},
  {"room-desc", W_ROOM_DESC},
  {"my-desc", W_PLAYER_DESC},
  {"exit-oneway", W_EXIT_ONEWAY},
  {"exit-multiple", W_EXIT_MULTIPLE},
  {"exit-msgs", W_EXIT_MSGS},
  {"thing-msgs", W_THING_MSGS},
  {"exit-desc", W_EXIT_DESC},
#ifdef TREKMUSH
  {"room-roomtype", W_ROOM_ROOMTYPE},
  {"room-spaceobject", W_ROOM_ZONING},
  {"room-ownership", W_ROOM_BUILDER},
  {"exit-spaceobject", W_EXIT_ZONING},
  {"exit-ownership", W_EXIT_BUILDER},
  {"exit-badlink", W_EXIT_BADLINK},
#endif /* TREKMUSH */

  /* These should stay at the end */
  {"serious", W_SERIOUS},
  {"normal", W_NORMAL},
  {"extra", W_EXTRA},
  {"all", W_ALL},
  {NULL, 0}
};

/* This is really simple-minded for efficiency. Basically, if it's
 * unlocked, it's unlocked. If it's locked to something starting with
 * a specific db#, it's locked. Anything else, and we don't know.
 */
static int
warning_lock_type(i, l)
    dbref i;
    const struct boolexp *l;	/* 0== unlocked. 1== locked, 2== sometimes */
{
  if (l == TRUE_BOOLEXP)
    return W_UNLOCKED;
  if (l->type == BOOLEXP_CONST ||
      l->type == BOOLEXP_CARRY ||
      l->type == BOOLEXP_IS ||
      l->type == BOOLEXP_OWNER)
    return W_LOCKED;
  return (W_LOCKED | W_UNLOCKED);
}

static void
complain(player, i, name, desc)
    dbref player;
    dbref i;
    const char *name;
    const char *desc;
{
  notify(player, tprintf("Warning '%s' for %s:\r\n     %s",
			 name, real_unparse(player, i, 0), desc));
}

static void
ct_room(player, i, flags)
    dbref player;
    dbref i;
    warn_type flags;
{
  if ((flags & W_ROOM_DESC) && !atr_get(i, "DESCRIBE"))
    complain(player, i, "room-desc", "room has no description");
#ifdef TREKMUSH
  if ((flags & W_ROOM_ROOMTYPE) && !atr_get(i, "ROOMTYPE"))
    complain(player, i, "room-roomtype", "room has no roomtype (see 'help @setroomtype')");
  if (flags & W_ROOM_ZONING && !SpaceObj(Zone(i)))
    complain(player, i, "room-zoning", "room is not zoned to a space object");
  if (flags & W_ROOM_BUILDER && (!Tel_Anything(Owner(i)) || !Search_All(Owner(i)) || !Builder(Owner(i)) || !Long_Fingers(Owner(i))))
    complain(player, i, "room-ownership", "room is not owned by a builder character");
#endif /* TREKMUSH */
}

static void
ct_exit(player, i, flags)
    dbref player;
    dbref i;
    warn_type flags;
{
  dbref j, src, dst;
  int count = 0;
  int lt;

  /* i must be an exit, must be in a valid room, and must lead to a
   * different room
   * Remember, for exit i, Exits(i) = source room
   * and Location(i) = destination room
   */

  dst = Destination(i);
  if ((flags & W_EXIT_UNLINKED) && (dst == NOTHING))
    complain(player, i, "exit-unlinked", "exit is unlinked; anyone can steal it");

  if (!Dark(i)) {
    if (flags & W_EXIT_MSGS) {
      lt = warning_lock_type(i, Key(i));
      if ((lt & W_UNLOCKED) &&
	  (!atr_get(i, "OSUCCESS") || !atr_get(i, "ODROP") ||
	   !atr_get(i, "SUCCESS")))
	complain(player, i, "exit-msgs", "possibly unlocked exit missing succ/osucc/odrop");
      if ((lt & W_LOCKED) && !atr_get(i, "FAIL"))
	complain(player, i, "exit-msgs", "possibly locked exit missing fail");
    }
    if (flags & W_EXIT_DESC) {
      if (!atr_get(i, "DESCRIBE"))
	complain(player, i, "exit-desc", "exit is missing description");
    }
  }
  src = Source(i);
  if (!GoodObject(src) || !IsRoom(src))
    return;
  if (src == dst)
    return;
  /* Don't complain about exits linked to HOME or variable exits. */
  if (!GoodObject(dst))
    return;

  for (j = Exits(dst); GoodObject(j); j = Next(j))
    if (Location(j) == src) {
      if (!(flags & W_EXIT_MULTIPLE))
	return;
      else
	count++;
    }
  if ((count == 0) && (flags & W_EXIT_ONEWAY))
    complain(player, i, "exit-oneway", "exit has no return exit");
  else if ((count > 1) && (flags & W_EXIT_MULTIPLE)) {
    char buff[BUFFER_LEN];
    strncpy(buff, tprintf("exit has multiple (%d) return exits", count), BUFFER_LEN);
    buff[BUFFER_LEN - 1] = '\0';
    complain(player, i, "exit-multiple", buff);
  }
#ifdef TREKMUSH
  if (flags & W_EXIT_ZONING && !SpaceObj(Zone(i)))
    complain(player, i, "exit-zoning", "room is not zoned to a space object");
  if (flags & W_EXIT_BUILDER && (!Tel_Anything(Owner(i)) || !Search_All(Owner(i)) || !Builder(Owner(i)) || !Long_Fingers(Owner(i))))
    complain(player, i, "exit-ownership", "room is not owned by a builder character");
#endif /* TREKMUSH */
}



static void
ct_player(player, i, flags)
    dbref player;
    dbref i;
    warn_type flags;
{
  if ((flags & W_PLAYER_DESC) && !atr_get(i, "DESCRIBE"))
    complain(player, i, "my-desc", "player is missing description");
}



static void
ct_thing(player, i, flags)
    dbref player;
    dbref i;
    warn_type flags;
{
  int lt;

  /* Ignore carried objects */
  if (Location(i) == player)
    return;
  if ((flags & W_THING_DESC) && !atr_get(i, "DESCRIBE"))
    complain(player, i, "thing-desc", "thing is missing description");

  if (flags & W_THING_MSGS) {
    lt = warning_lock_type(i, Key(i));
    if ((lt & W_UNLOCKED) &&
	(!atr_get(i, "OSUCCESS") || !atr_get(i, "ODROP") ||
	 !atr_get(i, "SUCCESS") || !atr_get(i, "DROP")))
      complain(player, i, "thing-msgs", "possibly unlocked thing missing succ/osucc/drop/odrop");
    if ((lt & W_LOCKED) && !atr_get(i, "FAIL"))
      complain(player, i, "thing-msgs", "possibly locked thing missing fail");
  }
}

void
set_initial_warnings(player)
    dbref player;
{
  Warnings(player) = W_NORMAL;
  return;
}

void
do_warnings(player, name, warns)
    dbref player;
    char *name;
    char *warns;
{
  dbref thing;
  int found = 0;
  warn_type flags, negate_flags;
  char tbuf1[BUFFER_LEN];
  char *w, *s;
  tcheck *c;

  switch (thing = match_result(player, name, NOTYPE, MAT_EVERYTHING)) {
  case NOTHING:
    notify(player, "I don't see that object.");
    return;
  case AMBIGUOUS:
    notify(player, "I don't know which one you mean.");
    return;
  default:
    if (!controls(player, thing)) {
      notify(player, "Permission denied.");
      return;
    }
    if (IsGarbage(thing)) {
      notify(player, "Why would you want to be warned about garbage?");
      return;
    }
    break;
  }

  flags = W_NONE;
  negate_flags = W_NONE;
  if (warns && *warns) {
    strcpy(tbuf1, warns);
    /* Loop through whatever's listed and add on those warnings */
    s = trim_space_sep(tbuf1, ' ');
    w = split_token(&s, ' ');
    while (w && *w) {
      found = 0;
      if (*w == '!') {
	/* Found a negated warning */
	w++;
	for (c = checklist; c->name; c++) {
	  if (!strcasecmp(w, c->name)) {
	    negate_flags |= c->flag;
	    found++;
	  }
	}
      } else {
	for (c = checklist; c->name; c++) {
	  if (!strcasecmp(w, c->name)) {
	    flags |= c->flag;
	    found++;
	  }
	}
      }
      /* At this point, we haven't matched any warnings. */
      if (!found) {
	notify(player, tprintf("Unknown warning: %s", w));
      }
      w = split_token(&s, ' ');
    }
    if (flags || !negate_flags) {
      Warnings(thing) = (flags & ~negate_flags);
    } else {
      Warnings(thing) &= ~negate_flags;
    }
    notify(player, tprintf("@warnings set to %s", unparse_warnings(thing)));
    return;
  }
}


/* Given an object, return a string of warnings on it */
const char *
unparse_warnings(thing)
    dbref thing;
{
  static char tbuf1[BUFFER_LEN];
  int warns, listsize, indexx;
  warn_type the_flag;

  warns = Warnings(thing);

  if (!warns)
    return "none";

  strcpy(tbuf1, "");

  /* Get the # of elements in checklist */
  listsize = sizeof(checklist) / sizeof(tcheck);

  /* Point c at last non-null in checklist, and go backwards */
  for (indexx = listsize - 2; warns && (indexx >= 0); indexx--) {
    the_flag = checklist[indexx].flag;
    if (!(the_flag & ~warns)) {
      /* Which is to say:
       * if the bits set on the_flag is a subset of the bits set on warns
       */
      strcat(tbuf1, checklist[indexx].name);
      strcat(tbuf1, " ");
      /* If we've got a flag which subsumes smaller ones, don't
       * list the smaller ones
       */
      warns &= ~the_flag;
    }
  }
  return tbuf1;
}

static void
check_topology_on(player, i)
    dbref player;
    dbref i;
{
  warn_type flags;

  /* Skip it if it's NOWARN or the player checking is the owner and
   * is NOWARN 
   */
  if (Flags(i) & NOWARN)
    return;

  /* If the owner is checking, use the flags on the object, and fall back
   * on the owner's flags as default. If it's not the owner checking
   * (therefore, an admin), ignore the object flags, use the admin's flags
   */
  if (Owner(player) == Owner(i)) {
    if (!(flags = Warnings(i)))
      flags = Warnings(player);
  } else
    flags = Warnings(player);

  switch (Typeof(i)) {
  case TYPE_ROOM:
    ct_room(player, i, flags);
    break;
  case TYPE_THING:
    ct_thing(player, i, flags);
    break;
  case TYPE_EXIT:
    ct_exit(player, i, flags);
    break;
  case TYPE_PLAYER:
    ct_player(player, i, flags);
    break;
  }
  return;
}


/* Loop through all objects and check their topology */
void
run_topology()
{
  int ndone;
  for (ndone = 0; ndone < db_top; ndone++) {
    if (!(IsGarbage(ndone)) && Connected(Owner(ndone)) &&
	!(Flags(Owner(ndone)) & NOWARN)) {
      check_topology_on(Owner(ndone), ndone);
    }
  }
}

void
do_wcheck_all(player)
    dbref player;
{
  if (!Wizard(player)) {
    notify(player, "You'd better check your wizbit first.");
    return;
  }
  notify(player, "Running database topology warning checks");
  run_topology();
  notify(player, "Warning checks complete.");
}

/* Called when a player wants to do a check on something. We check for
 * ownership or hasprivs before allowing it
 */
void
do_wcheck(player, name)
    dbref player;
    char *name;
{
  dbref thing;

  switch (thing = match_result(player, name, NOTYPE, MAT_EVERYTHING)) {
  case NOTHING:
    notify(player, "I don't see that object.");
    return;
  case AMBIGUOUS:
    notify(player, "I don't know which one you mean.");
    return;
  default:
    if (!(See_All(player) || (Owner(player) == Owner(thing)))) {
      notify(player, "Permission denied.");
      return;
    }
    if (IsGarbage(thing)) {
      notify(player, "Why would you want to be warned about garbage?");
      return;
    }
    break;
  }

  check_topology_on(player, thing);
  notify(player, "@wcheck complete.");
  return;
}


#endif				/* USE_WARNINGS */
