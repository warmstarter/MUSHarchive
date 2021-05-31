/* match.c */
/* Routines for parsing arguments */

/***********************************************************************
 * These are the PennMUSH name-matching routines, rewritten by
 * Javelin to be fully re-entrant instead of using global variables,
 * which fixes some bugs. [pl12]
 *
 * Calling these routines no longer requires using init_match,
 * a bunch of match_<foo> calls, and then getting match_result().
 * Instead, call one of these functions:
 *  match_result(who,name,type,flags) - return match, AMBIGUOUS, or NOTHING
 *  noisy_match_result(who,name,type,flags) - return match or NOTHING,
 *	and notify player on failures
 *  last_match_result(who,name,type,flags) - return match or NOTHING,
 *	and return the last match found in ambiguous situations
 *
 * who = dbref of player to match for
 * name = string to match on
 * type = preferred type of match (TYPE_THING, etc.) or NOTYPE
 * flags = a set of bits indicating what kind of matching to do
 *
 * flags are defined in match.h, but here they are for reference:
 * MAT_CHECK_KEYS	- check locks when matching
 * MAT_GLOBAL		- match in master room
 * MAT_REMOTES		- match things not nearby
 * MAT_NEAR		- match things nearby
 * MAT_CONTROL		- do a control check after matching
 * MAT_ME		- match "me"
 * MAT_HERE		- match "here"
 * MAT_ABSOLUTE		- match "#<dbref>"
 * MAT_PLAYER		- match a player's name
 * MAT_NEIGHBOR		- match something in the same room
 * MAT_POSSESSION	- match something I'm carrying
 * MAT_EXIT		- match an exit
 * MAT_CARRIED_EXIT	- match a carried exit (rare)
 * MAT_CONTAINER	- match a container I'm in
 * MAT_REMOTE_CONTENTS	- match the contents of a remote location
 * MAT_EVERYTHING	- me,here,absolute,player,neighbor,possession,exit
 * MAT_NEARBY		- everything near
 * MAT_OBJECTS		- me,absolute,player,neigbor,possession
 * MAT_NEAR_THINGS	- objects near
 * MAT_REMOTE		- absolute,player,remote_contents,exit,remotes
 * MAT_LIMITED		- absolute,player,neighbor
 */

#include "copyrite.h"
#include "config.h"
#include <ctype.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#include "conf.h"
#include "mushdb.h"
#include "externs.h"
#include "globals.h"
#include "match.h"
#include "confmagic.h"

extern dbref short_page _((const char *match));		/* in bsd.c */


dbref match_result _((const dbref who, const char *name, const int type, const long int flags));
dbref noisy_match_result _((const dbref who, const char *name, const int type, const long int flags));
dbref last_match_result _((const dbref who, const char *name, const int type, const long int flags));
dbref match_controlled _((dbref player, const char *name));
static dbref match_result_internal _((const dbref who, const char *name, const int type, const long int flags));
static dbref match_me _((const dbref who, const char *name));
static dbref match_here _((const dbref who, const char *name));
static dbref match_absolute _((const char *name));
static dbref match_player _((const char *match_name));
static dbref match_list _((const dbref match_who, const char *match_name, const int type, const long int flags, dbref first, dbref *last_match, int *match_count));
static dbref match_neighbor _((const dbref who, const char *name, const int type, const long int flags, dbref *last_match, int *match_count));
static dbref match_possession _((const dbref who, const char *name, const int type, const long int flags, dbref *last_match, int *match_count));
static dbref match_exit _((const dbref who, const char *name, const int type, const long int flags));
static dbref match_exit_internal _((const dbref match_who, const char *match_name, const int type, const long int flags, const dbref loc));
static dbref match_remote_contents _((const dbref who, const char *name, const int type, const long int flags, dbref *last_match, int *match_count));
static dbref match_container _((const dbref who, const char *name, const int type, const long int flags, dbref *last_match, int *match_count));
static dbref choose_thing _((const dbref match_who, const int preferred_type, long int flags, dbref thing1, dbref thing2));


char const *nomatch_message = "I can't see that here.";
char const *ambiguous_message = "I don't know which one you mean!";

/* A wrapper for returning a match, AMBIGUOUS, or NOTHING
 */
dbref
match_result(who, name, type, flags)
    const dbref who;
    const char *name;
    const int type;
    const long int flags;
{
  return match_result_internal(who, name, type, flags);
}

/* A wrapper for returning a match or NOTHING
 * Ambiguous matches return NOTHING
 * It will also notify the player of non-matches or ambiguous matches
 */
dbref
noisy_match_result(who, name, type, flags)
    const dbref who;
    const char *name;
    const int type;
    const long int flags;
{
  return match_result_internal(who, name, type, flags | MAT_NOISY);
}

/* A wrapper for returning a match or NOTHING
 * Ambiguous matches return the last matched thing
 */
dbref
last_match_result(who, name, type, flags)
    const dbref who;
    const char *name;
    const int type;
    const long int flags;
{
  return match_result_internal(who, name, type, flags | MAT_LAST);
}

/* Wrapper for a noisy match with control checks */
dbref
match_controlled(player, name)
    dbref player;
    const char *name;
{
  dbref match;
  match = noisy_match_result(player, name, NOTYPE, MAT_EVERYTHING);
  if (GoodObject(match) && !controls(player, match)) {
    notify(player, "Permission denied.");
    return NOTHING;
  } else {
    return match;
  }
}

/* The real work. */
static dbref
match_result_internal(who, name, type, flags)
    const dbref who;
    const char *name;
    const int type;
    const long int flags;
{
  dbref match = NOTHING, last_match = NOTHING;
  int match_count = 0;
  if (flags & MAT_ME) {
    match = match_me(who, name);
    if (GoodObject(match))
      return match;
  }
  if (flags & MAT_HERE) {
    match = match_here(who, name);
    if (GoodObject(match))
      return match;
  }
  if (!(flags & MAT_NEAR) || Long_Fingers(who)) {
    if (flags & MAT_ABSOLUTE) {
      match = match_absolute(name);
      if (GoodObject(match)) {
	if (flags & MAT_CONTROL) {
	  /* Check for control */
	  if (controls(who, match) || nearby(who, match))
	    return match;
	} else {
	  return match;
	}
      }
    }
    if (flags & MAT_PLAYER) {
      match = match_player(name);
      if (GoodObject(match))
	return match;
    }
  } else {
    /* We're doing a nearby match and the player doesn't have
     * long_fingers, so it's a controlled absolute
     */
    match = match_absolute(name);
    if (GoodObject(match) &&
	(controls(who, match) || nearby(who, match)))
      return match;
  }

  /* These return a match if the match is exact, and otherwise
   * store last thing matched in last_match and the number of matches
   * in match_count. Remote_contents and Neighbor are exclusive.
   */
  if (DO_GLOBALS && (flags & MAT_REMOTE_CONTENTS)) {
    match = match_remote_contents(who, name, type, flags, &last_match, &match_count);
  }
  if (flags & MAT_NEIGHBOR) {
    match = match_neighbor(who, name, type, flags, &last_match, &match_count);
  }
  if (flags & MAT_POSSESSION) {
    match = choose_thing(who, type, flags, match,
    match_possession(who, name, type, flags, &last_match, &match_count));
  }
  if (GoodObject(match))
    return match;
  if (flags & MAT_EXIT) {
    match = match_exit(who, name, type, flags);
    if (GoodObject(match))
      return match;
  }
  /* Miscellaneous ones */
  if (flags & MAT_CONTAINER) {
    match = match_container(who, name, type, flags, &last_match, &match_count);
  }
  if (flags & MAT_CARRIED_EXIT) {
    match = match_exit_internal(who, name, type, flags, who);
  }
  /* If we've got an exact match, we'll return it */
  if (GoodObject(match))
    return match;

  /* If it's a last_match_result, just return last_match */
  if (flags & MAT_LAST)
    return last_match;

  /* Set up the default match_result behavior */
  switch (match_count) {
  case 0:
    match = NOTHING;
    break;
  case 1:
    match = last_match;
    break;
  default:
    match = AMBIGUOUS;
    break;
  }

  /* Handle noisy_match_result */
  if (flags & MAT_NOISY) {
    switch (match) {
    case NOTHING:
      notify(who, nomatch_message);
      return NOTHING;
    case AMBIGUOUS:
      notify(who, ambiguous_message);
      return NOTHING;
    default:
      return match;
    }
  }
  return match;
}

static dbref
match_me(who, name)
    const dbref who;
    const char *name;
{
  if (!strcasecmp(name, "me"))
    return who;
  return NOTHING;
}

static dbref
match_here(who, name)
    const dbref who;
    const char *name;
{
  if (!strcasecmp(name, "here") && GoodObject(Location(who)))
    return Location(who);
  return NOTHING;
}

static dbref
match_absolute(name)
    const char *name;
{
  return parse_dbref(name);
}

static dbref
match_player(match_name)
    const char *match_name;
{
  dbref match;
  const char *p;


  if (*match_name == LOOKUP_TOKEN) {
    for (p = match_name + 1; isspace(*p); p++) ;
    if ((match = lookup_player(p)) != NOTHING) {
      return match;
    } else {
      /* try a partial match on a connected player, 2.0 style */
      /* Can return a match, NOTHING, or AMBIGUOUS */
      return short_page(p);
    }
  }
  return NOTHING;
}

static dbref
match_list(match_who, match_name, type, flags, first, last_match, match_count)
    const dbref match_who;
    const char *match_name;
    const int type;
    const long int flags;
    dbref first;
    dbref *last_match;
    int *match_count;
{
  dbref absolute;
  dbref the_match = NOTHING;
  absolute = match_absolute(match_name);
  if (!controls(match_who, absolute))
    absolute = NOTHING;

  DOLIST(first, first) {
    Access(first);
    if (first == absolute) {
      return first;
    } else if (!strcasecmp(Name(first), match_name)) {
      /* if there are multiple exact matches, randomly choose one */
      the_match = choose_thing(match_who, type, flags, the_match, first);
    } else if (string_match(Name(first), match_name)) {
      *last_match = first;
      *match_count += 1;
    }
  }
  return the_match;
}

static dbref
match_neighbor(who, name, type, flags, last_match, match_count)
    const dbref who;
    const char *name;
    const int type;
    const long int flags;
    dbref *last_match;
    int *match_count;
{
  dbref loc;
  loc = Location(who);
  if (GoodObject(loc)) {
    return match_list(who, name, type, flags, Contents(loc), last_match, match_count);
  }
  return NOTHING;
}

static dbref
match_possession(who, name, type, flags, last_match, match_count)
    const dbref who;
    const char *name;
    const int type;
    const long flags;
    dbref *last_match;
    int *match_count;
{
  return match_list(who, name, type, flags, Contents(who), last_match, match_count);
}

static dbref
match_exit(who, name, type, flags)
    const dbref who;
    const char *name;
    const int type;
    const long flags;
{
  dbref loc;
  loc = (IsRoom(who)) ? who : Location(who);
  if (DO_GLOBALS) {
    if (flags & MAT_REMOTES) {
      if (GoodObject(loc))
	return match_exit_internal(who, name, type, flags, Zone(loc));
      else
	return NOTHING;
    } else if (flags & MAT_GLOBAL)
      return match_exit_internal(who, name, type, flags, MASTER_ROOM);
  }
  return match_exit_internal(who, name, type, flags, loc);
}

static dbref
match_exit_internal(match_who, match_name, type, flags, loc)
    const dbref match_who;
    const char *match_name;
    const int type;
    const long int flags;
    const dbref loc;
{
  dbref exit_tmp;
  dbref absolute;
  const char *match;
  const char *p;
  dbref the_match = NOTHING;

  if (!GoodObject(loc) || !match_name || !*match_name)
    return NOTHING;
  if (!IsRoom(loc))
    return NOTHING;
  absolute = match_absolute(match_name);
  if (!controls(match_who, absolute)) {
    absolute = NOTHING;
  }
  DOLIST(exit_tmp, Exits(loc)) {
    Access(exit_tmp);
    if (exit_tmp == absolute) {
      the_match = exit_tmp;
    } else {
      match = Name(exit_tmp);
      while (*match) {
	/* check out this one */
	for (p = match_name;
	     (*p
	      && DOWNCASE(*p) == DOWNCASE(*match)
	      && *match != EXIT_DELIMITER);
	     p++, match++) ;
	/* did we get it? */
	if (*p == '\0') {
	  /* make sure there's nothing afterwards */
	  while (isspace(*match))
	    match++;
	  if (*match == '\0' || *match == EXIT_DELIMITER) {
	    /* we got it */
	    the_match = choose_thing(match_who, type, flags, the_match, exit_tmp);
	    goto next_exit;
	  }
	}
	/* we didn't get it, find next match */
	while (*match && *match++ != EXIT_DELIMITER) ;
	while (isspace(*match))
	  match++;
      }
    }
  next_exit:
    ;
  }
  return the_match;
}

static dbref
match_remote_contents(who, name, type, flags, last_match, match_count)
    const dbref who;
    const char *name;
    const int type;
    const long int flags;
    dbref *last_match;
    int *match_count;
{
  if (GoodObject(who))
    return match_list(who, name, type, flags, Contents(who), last_match, match_count);
  return NOTHING;
}

static dbref
match_container(who, name, type, flags, last_match, match_count)
    const dbref who;
    const char *name;
    const int type;
    const long int flags;
    dbref *last_match;
    int *match_count;
{
  dbref loc;
  loc = Location(who);
  if (GoodObject(loc))
    return match_list(who, name, type, flags, loc, last_match, match_count);
  return NOTHING;
}


static dbref
choose_thing(match_who, preferred_type, flags, thing1, thing2)
    const dbref match_who;
    const int preferred_type;
    long int flags;
    dbref thing1;
    dbref thing2;
{
  int has1;
  int has2;
  if (thing1 == NOTHING) {
    return thing2;
  } else if (thing2 == NOTHING) {
    return thing1;
  }
  if (preferred_type != NOTYPE) {
    if (Typeof(thing1) == preferred_type) {
      if (Typeof(thing2) != preferred_type) {
	return thing1;
      }
    } else if (Typeof(thing2) == preferred_type) {
      return thing2;
    }
  }
  if (flags & MAT_CHECK_KEYS) {
    has1 = could_doit(match_who, thing1);
    has2 = could_doit(match_who, thing2);

    if (has1 && !has2) {
      return thing1;
    } else if (has2 && !has1) {
      return thing2;
    }
    /* else fall through */
  }
  return (getrandom(2) ? thing1 : thing2);
}
