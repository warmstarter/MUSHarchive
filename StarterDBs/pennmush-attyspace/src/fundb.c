
#include "copyrite.h"

#include "config.h"
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include "conf.h"
#include "dbdefs.h"
#include "externs.h"
#include "flags.h"
#include "intrface.h"
#include "match.h"
#include "parse.h"
#include "command.h"
#include "mushdb.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "confmagic.h"

#ifdef WIN32
#pragma warning( disable : 4761)	/* NJG: disable warning re conversion */
#endif

#ifdef CHAT_SYSTEM
#include "extchat.h"
extern CHAN *channels;
#endif

extern char ccom[BUFFER_LEN];	/* bsd.c */
char *do_get_attrib _((dbref executor, dbref thing, char *attrib));
dbref next_con _((dbref player, dbref this));
dbref next_exit _((dbref player, dbref this));
static lock_type get_locktype _((dbref obj, const char *str));
extern struct db_stat_info *get_stats _((dbref player, dbref owner));
extern ATTR *aname_hash_lookup _((const char *name));
static int lattr_helper _((dbref player, dbref thing, char const *pattern,
			   ATTR *atr, void *args));


char *
do_get_attrib(executor, thing, attrib)
    dbref executor;
    dbref thing;
    char *attrib;
{
  ATTR *a;
  char const *value;

  a = atr_get(thing, upcasestr(attrib));
  if (a) {
    if (Can_Read_Attr(executor, thing, a)) {
      if (strlen(value = uncompress(a->value)) < BUFFER_LEN)
	return (char *) value;
      else
	return (char *) "#-1 ATTRIBUTE LENGTH TOO LONG";
    }
    return (char *) "#-1 NO PERMISSION TO GET ATTRIBUTE";
  }
  a = atr_match(upcasestr(attrib));
  if (a) {
    if (Can_Read_Attr(executor, thing, a))
      return (char *) "";
    return (char *) "#-1 NO PERMISSION TO GET ATTRIBUTE";
  }
  if (!Can_Examine(executor, thing))
    return (char *) "#-1 NO PERMISSION TO GET ATTRIBUTE";
  return (char *) "";
}

/* this function produces a space-separated list of attributes that are
 * on an object.
 */
struct lh_args {
  int first;
  char *buff;
  char **bp;
};

/* ARGSUSED */
static int
lattr_helper(player, thing, pattern, atr, args)
    dbref player;
    dbref thing;
    char const *pattern;
    ATTR *atr;
    void *args;
{
  struct lh_args *lh = args;
  if (lh->first)
    lh->first = 0;
  else
    safe_chr(' ', lh->buff, lh->bp);
  safe_str(AL_NAME(atr), lh->buff, lh->bp);
  return 1;
}

/* ARGSUSED */
FUNCTION(fun_lattr)
{
  dbref thing;
  char *pattern;
  struct lh_args lh;

  pattern = (char *) index(args[0], '/');
  if (pattern)
    *pattern++ = '\0';
  else
    pattern = (char *) "*";	/* match anything */

  thing = match_thing(executor, args[0]);
  if (thing == NOTHING) {
    safe_str("#-1 NO MATCH", buff, bp);
    return;
  }
  if (!Can_Examine(executor, thing)) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  lh.first = 1;
  lh.buff = buff;
  lh.bp = bp;
  (void) atr_iter_get(executor, thing, pattern, lattr_helper, &lh);
}

/* ARGSUSED */
FUNCTION(fun_hasattr)
{
  dbref thing;
  ATTR *a;

  thing = match_thing(executor, args[0]);
  if (thing == NOTHING) {
    safe_str("#-1 NO SUCH OBJECT VISIBLE", buff, bp);
    return;
  }
  if (strchr(called_as, 'P'))
    a = atr_get(thing, upcasestr(args[1]));
  else
    a = atr_get_noparent(thing, upcasestr(args[1]));
  if (a && Can_Read_Attr(executor, thing, a)) {
    if (strchr(called_as, 'V'))
      safe_chr(*AL_STR(a) ? '1' : '0', buff, bp);
    else
      safe_chr('1', buff, bp);
    return;
  } else if (a || !Can_Examine(executor, thing)) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  safe_chr('0', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_get)
{
  dbref thing;
  char *s;

  s = strchr(args[0], '/');
  if (!s) {
    safe_str("#-1 BAD ARGUMENT FORMAT TO GET", buff, bp);
    return;
  }
  *s++ = '\0';
  thing = match_thing(executor, args[0]);
  if (thing == NOTHING) {
    safe_str("#-1 NO SUCH OBJECT VISIBLE", buff, bp);
    return;
  }
  safe_str(do_get_attrib(executor, thing, s), buff, bp);
}

/* Functions like get, but uses the standard way of passing arguments */
/* to a function, and thus doesn't choke on nested functions within.  */

/* ARGSUSED */
FUNCTION(fun_xget)
{
  dbref thing;

  thing = match_thing(executor, args[0]);
  if (thing == NOTHING) {
    safe_str("#-1 NO SUCH OBJECT VISIBLE", buff, bp);
    return;
  }
  safe_str(do_get_attrib(executor, thing, args[1]), buff, bp);
}

/* Functions like get, but includes a default response if the
 * attribute isn't present or is null
 */
/* ARGSUSED */
FUNCTION(fun_default)
{
  dbref thing;
  ATTR *attrib;
  char *dp;
  char const *sp;
  char mstr[BUFFER_LEN];
  /* find our object and attribute */
  dp = mstr;
  sp = args[0];
  process_expression(mstr, &dp, &sp, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  *dp = '\0';
  parse_attrib(executor, mstr, &thing, &attrib);
  if (GoodObject(thing) && attrib &&
      Can_Read_Attr(executor, thing, attrib)) {
    /* Ok, we've got it */
    sp = safe_uncompress(attrib->value);
    safe_str(sp, buff, bp);
    free((Malloc_t) sp);
    return;
  }
  /* We couldn't get it. Evaluate args[1] and return it */
  sp = args[1];
  process_expression(buff, bp, &sp, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  return;
}

/* ARGSUSED */
FUNCTION(fun_eval)
{
  /* like xget, except pronoun substitution is done */

  dbref thing;
  char *tbuf;
  char const *tp;
  ATTR *a;

  thing = match_thing(executor, args[0]);
  if (thing == NOTHING) {
    safe_str("#-1 NO SUCH OBJECT VISIBLE", buff, bp);
    return;
  }
  a = atr_get(thing, upcasestr(args[1]));
  if (a && Can_Read_Attr(executor, thing, a)) {
    if (!CanEval(executor, thing)) {
      safe_str("#-1 PERMISSION DENIED", buff, bp);
      return;
    }
    tp = tbuf = safe_uncompress(a->value);
#ifdef MEM_CHECK
    add_check("fun_eval.attr_value");
#endif
    process_expression(buff, bp, &tp, thing, executor, executor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
    mush_free((Malloc_t) tbuf, "fun_eval.attr_value");
    return;
  } else if (a || !Can_Examine(executor, thing)) {
    safe_str("#-1 NO PERMISSION TO GET ATTRIBUTE", buff, bp);
    return;
  }
  return;
}

/* ARGSUSED */
FUNCTION(fun_get_eval)
{
  /* like eval, except uses obj/attr syntax. 2.x compatibility */

  dbref thing;
  char *tbuf, *s;
  char const *tp;
  ATTR *a;

  s = strchr(args[0], '/');
  if (!s) {
    safe_str("#-1 BAD ARGUMENT FORMAT TO GET_EVAL", buff, bp);
    return;
  }
  *s++ = '\0';
  thing = match_thing(executor, args[0]);
  if (thing == NOTHING) {
    safe_str("#-1 NO SUCH OBJECT VISIBLE", buff, bp);
    return;
  }
  a = atr_get(thing, upcasestr(s));
  if (a && Can_Read_Attr(executor, thing, a)) {
    if (!CanEval(executor, thing)) {
      safe_str("#-1 PERMISSION DENIED", buff, bp);
      return;
    }
    tp = tbuf =
      safe_uncompress(a->value);
#ifdef MEM_CHECK
    add_check("fun_eval.attr_value");
#endif
    process_expression(buff, bp, &tp, thing, executor, executor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
    mush_free((Malloc_t) tbuf, "fun_eval.attr_value");
    return;
  } else if (a || !Can_Examine(executor, thing)) {
    safe_str("#-1 NO PERMISSION TO GET ATTRIBUTE", buff, bp);
    return;
  }
  return;
}

/* Functions like eval, but includes a default response if the
 * attribute isn't present or is null
 */
/* ARGSUSED */
FUNCTION(fun_edefault)
{
  dbref thing;
  ATTR *attrib;
  char *dp, *sbuf;
  char const *sp;
  char mstr[BUFFER_LEN];
  /* find our object and attribute */
  dp = mstr;
  sp = args[0];
  process_expression(mstr, &dp, &sp, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  *dp = '\0';
  parse_attrib(executor, mstr, &thing, &attrib);
  if (GoodObject(thing) && attrib &&
      Can_Read_Attr(executor, thing, attrib)) {
    if (!CanEval(executor, thing)) {
      safe_str("#-1 PERMISSION DENIED", buff, bp);
      return;
    }
    /* Ok, we've got it */
    sp = sbuf = safe_uncompress(attrib->value);
#ifdef MEM_CHECK
    add_check("fun_edefault.attr_value");
#endif
    process_expression(buff, bp, &sp, thing, executor, executor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
    mush_free((Malloc_t) sbuf, "fun_edefault.attr_value");
    return;
  }
  /* We couldn't get it. Evaluate args[1] and return it */
  sp = args[1];
  process_expression(buff, bp, &sp, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  return;
}

/* ARGSUSED */
FUNCTION(fun_v)
{
  /* handle 0-9, va-vz, n, l, # */

  int c;

  if (args[0][0] && !args[0][1]) {
    switch (c = args[0][0]) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if (wenv[c - '0'])
	safe_str(wenv[c - '0'], buff, bp);
      return;
    case '#':
      /* enactor dbref */
      safe_str(unparse_dbref(enactor), buff, bp);
      return;
    case '@':
      /* caller dbref */
      safe_str(unparse_dbref(caller), buff, bp);
      return;
    case '!':
      /* executor dbref */
      safe_str(unparse_dbref(executor), buff, bp);
      return;
    case 'n':
    case 'N':
      /* enactor name */
      safe_str(Name(enactor), buff, bp);
      return;
    case 'l':
    case 'L':
      /* giving the location does not violate security,
       * since the object is the enactor.  */
      safe_str(unparse_dbref(Location(enactor)), buff, bp);
      return;
    case 'c':
    case 'C':
      safe_str(ccom, buff, bp);
      return;
    }
  }
  safe_str(do_get_attrib(executor, executor, args[0]), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_flags)
{
  dbref thing;
  thing = match_thing(executor, args[0]);

  if (thing == NOTHING)
    safe_str("#-1", buff, bp);
  else
    safe_str(unparse_flags(thing, executor), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_num)
{
  safe_str(unparse_dbref(match_thing(executor, args[0])), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_rnum)
{
  dbref place = match_thing(executor, args[0]);
  char *name = args[1];
  dbref thing;
  if ((place != NOTHING) &&
      (Can_Examine(executor, place) || (Location(executor) == place) ||
       (enactor == place))) {
    switch (thing = match_result(place, name, NOTYPE, MAT_REMOTE)) {
    case NOTHING:
      safe_str("#-1 NO MATCH", buff, bp);
      break;
    case AMBIGUOUS:
      safe_str("#-1 AMBIGUOUS MATCH", buff, bp);
      break;
    default:
      safe_str(unparse_dbref(thing), buff, bp);
    }
  } else
    safe_str("#-1", buff, bp);
}

/*
 * fun_lcon, fun_lexits, fun_con, fun_exit, fun_next, and next_exit were all
 * re-coded by d'mike, 7/12/91.  next_con was added at this time as well.
 *
 * The function behavior was changed by Amberyl, to remove what she saw
 * as a security problem, since mortals could get the contents of rooms
 * they didn't control, thus (if they were willing to go through the trouble)
 * they could build a scanner to locate anything they wanted.
 *
 * You can get the contents of any room you control, regardless of whether
 * or not the object is dark. You can get the contents of your current
 * location, _except_ for dark objects (and DARK/OPAQUE rooms). You CANNOT 
 * get the contents of anything else, regardless of whether or not you have 
 * objects in it. This latter behavior is exhibited by 2.0.
 *
 * The same behavior is true for exits, except OPAQUE doesn't apply.
 */

/* ARGSUSED */
FUNCTION(fun_lcon)
{
  dbref thing;
  int first;
  dbref it = match_thing(executor, args[0]);

  if ((it != NOTHING) &&
      (Can_Examine(executor, it) || (Location(executor) == it) ||
       (enactor == it))) {
    first = 1;
    DOLIST_VISIBLE(thing, Contents(it), executor) {
      if (first)
	first = 0;
      else
	safe_chr(' ', buff, bp);
      safe_str(unparse_dbref(thing), buff, bp);
    }
  } else
    safe_str("#-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_con)
{
  dbref it = match_thing(executor, args[0]);

  if ((it != NOTHING) &&
      (Can_Examine(executor, it) || (where_is(executor) == it) ||
       (enactor == it))) {
    it = first_visible(executor, it);
    if (it == NOTHING)
      safe_str("#-1", buff, bp);
    else
      safe_str(unparse_dbref(Contents(it)), buff, bp);
  } else {
    safe_str("#-1", buff, bp);
  }
}

/* return next contents that is ok to see */
dbref
next_con(player, this)
    dbref player;
    dbref this;
{
  dbref loc;
  int sees_loc;

  if ((this == NOTHING) || ((loc = Location(this)) == NOTHING))
    return NOTHING;
  sees_loc = Can_Examine(player, loc);

  if (sees_loc || ((Location(player) == loc) && !Dark(loc) && !Opaque(loc))) {
    this = first_visible(player, this);
  }
  return (this);
}

/* return next exit that is ok to see */
dbref
next_exit(player, this)
    dbref player;
    dbref this;
{
  dbref loc;
  int sees_loc;

  if ((this == NOTHING) || ((loc = Home(this)) == NOTHING))
    return NOTHING;
  sees_loc = Can_Examine(player, loc);

  if (sees_loc || ((Location(player) == loc) && !Dark(loc))) {
    this = first_visible(player, this);
  }
  return (this);
}

/* ARGSUSED */
FUNCTION(fun_lexits)
{
  dbref it = match_thing(executor, args[0]);
  dbref thing;
  int first;
  int sees_loc;

  sees_loc = Can_Examine(executor, it);

  first = 1;
  if (GoodObject(it) && IsRoom(it)) {
    if (sees_loc || (Location(executor) == it)) {
      DOLIST_VISIBLE(thing, Exits(it), executor) {
	if (first)
	  first = 0;
	else
	  safe_chr(' ', buff, bp);
	safe_str(unparse_dbref(thing), buff, bp);
      }
    }
  } else
    safe_str("#-1", buff, bp);
}

/* fun_exit is really just a wrapper for next_exit now... */
/* ARGSUSED */
FUNCTION(fun_exit)
{
  dbref it = match_thing(executor, args[0]);
  if (GoodObject(it) && IsRoom(it))
    safe_str(unparse_dbref(next_exit(executor, Exits(it))), buff, bp);
  else
    safe_str("#-1", buff, bp);
  return;
}

/* ARGSUSED */
FUNCTION(fun_next)
{
  dbref it = match_thing(executor, args[0]);

  if (GoodObject(it)) {
    switch (Typeof(it)) {
    case TYPE_EXIT:
      safe_str(unparse_dbref(next_exit(executor, Next(it))), buff, bp);
      break;
    case TYPE_THING:
    case TYPE_PLAYER:
      safe_str(unparse_dbref(next_con(executor, Next(it))), buff, bp);
      break;
    default:
      safe_str("#-1", buff, bp);
      break;
    }
  } else
    safe_str("#-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_entrances)
{
  /* All args are optional.
   * The first argument is the dbref to check (default = this room)
   * The second argument to this function is a set of characters:
   * (a)ll (default), (e)xits, (t)hings, (p)layers, (r)ooms
   * The third and fourth args limit the range of dbrefs (default=0,db_top)
   */
  dbref where = Location(executor);
  dbref low = 0;
  dbref high = db_top - 1;
  dbref counter;
  dbref entrance;
  int found;
  int exd, td, pd, rd;		/* what we're looking for */
  char *p;

  if (options.daytime) {
    notify(executor, "Function disabled.");
    safe_str("#-1", buff, bp);
    return;
  }
  if (nargs > 0)
    where = match_result(executor, args[0], NOTYPE, MAT_EVERYTHING);
  if (!GoodObject(where)) {
    safe_str("#-1 INVALID LOCATION", buff, bp);
    return;
  }
  exd = td = pd = rd = 0;
  if (nargs > 1) {
    if (!args[1] || !*args[1]) {
      safe_str("#-1 INVALID SECOND ARGUMENT", buff, bp);
      return;
    }
    p = args[1];
    while (*p) {
      switch (*p) {
      case 'a':
      case 'A':
	exd = td = pd = rd = 1;
	break;
      case 'e':
      case 'E':
	exd = 1;
	break;
      case 't':
      case 'T':
	td = 1;
	break;
      case 'p':
      case 'P':
	pd = 1;
	break;
      case 'r':
      case 'R':
	rd = 1;
	break;
      default:
	safe_str("#-1 INVALID SECOND ARGUMENT", buff, bp);
	return;
      }
      p++;
    }
  }
  if (!exd && !td && !pd && !rd) {
    exd = td = pd = rd = 1;
  }
  if (nargs > 2) {
    if (is_integer(args[2])) {
      low = parse_integer(args[2]);
    } else if (is_dbref(args[2])) {
      low = parse_dbref(args[2]);
    } else {
      safe_str(e_ints, buff, bp);
      return;
    }
  }
  if (nargs > 3) {
    if (is_integer(args[3])) {
      high = parse_integer(args[3]);
    } else if (is_dbref(args[3])) {
      high = parse_dbref(args[3]);
    } else {
      safe_str(e_ints, buff, bp);
      return;
    }
  }
  if (!GoodObject(low))
    low = 0;
  if (!GoodObject(high))
    high = db_top - 1;

  if (!controls(executor, where) && !Search_All(executor)) {
    safe_str("#-1 PERMISSION DENIED.", buff, bp);
    return;
  }
  if (!payfor(executor, FIND_COST)) {
    notify(executor, tprintf("You don't have %d %s to do that.",
			     FIND_COST,
			     ((FIND_COST == 1) ? MONEY : MONIES)));
    safe_str("#-1", buff, bp);
    return;
  }
  /* Ok, do the real work */
  found = 0;
  for (counter = low; counter <= high; counter++) {
    if (controls(executor, where) || controls(executor, counter)) {
      if ((exd && IsExit(counter)) ||
	  (td && IsThing(counter)) ||
	  (pd && IsPlayer(counter)) ||
	  (rd && IsRoom(counter))) {
	if (Mobile(counter))
	  entrance = Home(counter);
	else
	  entrance = Location(counter);
	if (entrance == where) {
	  if (!found)
	    found = 1;
	  else
	    safe_chr(' ', buff, bp);
	  safe_str(unparse_dbref(counter), buff, bp);
	}
      }
    }
  }
  return;
}


/* ARGSUSED */
FUNCTION(fun_nearby)
{
  dbref obj1 = match_thing(executor, args[0]);
  dbref obj2 = match_thing(executor, args[1]);

  if (!controls(executor, obj1) && !controls(executor, obj2)
      && !nearby(executor, obj1) && !nearby(executor, obj2)) {
    safe_str("#-1 NO OBJECTS CONTROLLED", buff, bp);
    return;
  }
  if (!GoodObject(obj1) || !GoodObject(obj2)) {
    safe_str("#-1", buff, bp);
    return;
  }
  safe_chr(nearby(obj1, obj2) ? '1' : '0', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_controls)
{
  dbref it = match_thing(executor, args[0]);
  dbref thing = match_thing(executor, args[1]);

  if (it == NOTHING)
    safe_str("#-1 ARG1 NOT FOUND", buff, bp);
  else if (thing == NOTHING)
    safe_str("#-1 ARG2 NOT FOUND", buff, bp);
  else if (!(controls(executor, it) || See_All(executor)))
    safe_str("#-1 PERMISSION DENIED", buff, bp);
  else
    safe_chr(controls(it, thing) ? '1' : '0', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_visible)
{
  /* check to see if we have an object-attribute pair. If we don't,
   * then we want to know about the whole object; otherwise, we're
   * just interested in a single attribute.
   * If we encounter an error, we return 0 rather than an error
   * code, since if it doesn't exist, it obviously can't see 
   * anything or be seen.
   */

  dbref it, thing;
  char *name;
  ATTR *a;

  if ((it = match_thing(executor, args[0])) == NOTHING) {
    safe_chr('0', buff, bp);
    return;
  }
  if ((name = strchr(args[1], '/')))
    *name++ = '\0';
  if ((thing = match_thing(executor, args[1])) == NOTHING) {
    safe_chr('0', buff, bp);
    return;
  }
  if (name) {
    a = atr_get(thing, upcasestr(name));
    safe_chr((a && Can_Read_Attr(it, thing, a)) ? '1' : '0', buff, bp);
  } else {
    safe_chr(Can_Examine(it, thing) ? '1' : '0', buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_type)
{
  dbref it = match_thing(executor, args[0]);
  if (it == NOTHING) {
    safe_str("#-1", buff, bp);
    return;
  }
  switch (Typeof(it)) {
  case TYPE_PLAYER:
    safe_str("PLAYER", buff, bp);
    break;
  case TYPE_THING:
    safe_str("THING", buff, bp);
    break;
  case TYPE_EXIT:
    safe_str("EXIT", buff, bp);
    break;
  case TYPE_ROOM:
    safe_str("ROOM", buff, bp);
    break;
  default:
    safe_str("WEIRD OBJECT", buff, bp);
    do_rawlog(LT_ERR, "WARNING: Weird object #%d (type %d)\n", it, Typeof(it));
  }
}

/* ARGSUSED */
FUNCTION(fun_hasflag)
{
  dbref thing;
  ATTR *attrib;
  int f;

  if (strchr(args[0], '/')) {
    parse_attrib(executor, args[0], &thing, &attrib);
    if (!attrib)
      safe_str("#-1", buff, bp);
    else if ((f = string_to_atrflag(executor, args[1])) < 0)
      safe_str("#-1", buff, bp);
    else
      safe_chr((attrib->flags & f) ? '1' : '0', buff, bp);
  } else {
    thing = match_thing(executor, args[0]);
    if (thing == NOTHING)
      safe_str("#-1", buff, bp);
    else
      safe_chr(sees_flag(executor, thing, args[1]) ? '1' : '0', buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_hastype)
{
  dbref it = match_thing(executor, args[0]);
  if (it == NOTHING) {
    safe_str("#-1", buff, bp);
    return;
  }
  switch (*args[1]) {
  case 'r':
  case 'R':
    safe_chr(IsRoom(it) ? '1' : '0', buff, bp);
    break;
  case 'e':
  case 'E':
    safe_chr(IsExit(it) ? '1' : '0', buff, bp);
    break;
  case 'p':
  case 'P':
    safe_chr(IsPlayer(it) ? '1' : '0', buff, bp);
    break;
  case 't':
  case 'T':
    safe_chr(IsThing(it) ? '1' : '0', buff, bp);
    break;
  default:
    safe_str("#-1 NO SUCH TYPE", buff, bp);
    break;
  };
}

/* ARGSUSED */
FUNCTION(fun_orflags)
{
  safe_chr(handle_flaglists(executor, args[0], args[1], 0) ? '1' : '0',
	   buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_andflags)
{
  safe_chr(handle_flaglists(executor, args[0], args[1], 1) ? '1' : '0',
	   buff, bp);
}

static lock_type
get_locktype(obj, str)
    dbref obj;
    char const *str;
{
  /* figure out a lock type */

  if (!str || !*str)
    return Basic_Lock;
  return str;
}

/* ARGSUSED */
FUNCTION(fun_lock)
{
  dbref it;
  char *p;
  lock_type ltype;

  if ((p = strchr(args[0], '/')))
    *p++ = '\0';

  it = match_thing(executor, args[0]);
  ltype = get_locktype(it, p);

  if (nargs == 2) {
    if (!command_check_byname(executor, "@lock")) {
      safe_str("#-1 PERMISSION DENIED", buff, bp);
      return;
    }
#ifdef FUNCTION_SIDE_EFFECTS
    do_lock(executor, args[0], args[1], ltype);
#else
    safe_str("#-1 FUNCTION DISABLED", buff, bp);
#endif
    return;
  }
  if ((it != NOTHING) && (ltype != NULL) && Can_Read_Lock(executor, it, ltype)) {
    safe_str(unparse_boolexp(executor, getlock(it, ltype), 1), buff, bp);
    return;
  }
  safe_str("#-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_elock)
{
  char *p;
  lock_type ltype;
  dbref it;
  dbref victim = match_thing(executor, args[1]);

  p = (char *) strchr(args[0], '/');
  if (p)
    *p++ = '\0';

  it = match_thing(executor, args[0]);
  ltype = get_locktype(it, p);

  if ((it == NOTHING) ||
      (ltype == NULL) ||
      !Can_Read_Lock(executor, it, ltype)) {
    safe_str("#-1", buff, bp);
    return;
  }
  safe_chr(eval_lock(victim, it, ltype) ? '1' : '0', buff, bp);
  return;
}

/* ARGSUSED */
FUNCTION(fun_findable)
{
  dbref obj = match_thing(executor, args[0]);
  dbref victim = match_thing(executor, args[1]);

  if (obj == NOTHING)
    safe_str("#-1 ARG1 NOT FOUND", buff, bp);
  else if (victim == NOTHING)
    safe_str("#-1 ARG2 NOT FOUND", buff, bp);
  else
    safe_chr(Can_Locate(obj, victim) ? '1' : '0', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_loc)
{
  dbref it = match_thing(executor, args[0]);
  if ((it != NOTHING) && Can_Locate(executor, it))
    safe_str(unparse_dbref(Location(it)), buff, bp);
  else
    safe_str("#-1", buff, bp);
}

#ifdef CREATION_TIMES
/* ARGSUSED */
FUNCTION(fun_ctime)
{
  dbref it = match_thing(executor, args[0]);
  if ((it != NOTHING) && Can_Examine(executor, it)) {
    safe_str(ctime(&CreTime(it)), buff, bp);
    (*bp)--;
    return;
  }
  safe_str("#-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_mtime)
{
  dbref it = match_thing(executor, args[0]);
  if ((it != NOTHING) && Can_Examine(executor, it) &&
      !IsPlayer(it)) {
    safe_str(ctime(&ModTime(it)), buff, bp);
    (*bp)--;
    return;
  }
  safe_str("#-1", buff, bp);
}
#endif

/* ARGSUSED */
FUNCTION(fun_where)
{
  /* finds the "real" location of an object */

  dbref it = match_thing(executor, args[0]);
  if ((it != NOTHING) && Can_Locate(executor, it))
    safe_str(unparse_dbref(where_is(it)), buff, bp);
  else
    safe_str("#-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_room)
{
  dbref room;
  int rec = 0;
  dbref it = match_thing(executor, args[0]);

  if ((it != NOTHING) && Can_Locate(executor, it)) {
    room = Location(it);
    if (!GoodObject(room)) {
      safe_str("#-1", buff, bp);
      return;
    }
    while (!IsRoom(room)) {
      room = Location(room);
      rec++;
      if (rec > 20) {
	notify(executor, "Too many containers.");
	safe_str("#-1", buff, bp);
	return;
      }
    }
    safe_str(unparse_dbref(room), buff, bp);
  } else {
    notify(executor, "Permission denied.");
    safe_str("#-1", buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_rloc)
{
  int i;
  int deep = parse_integer(args[1]);
  dbref it = match_thing(executor, args[0]);

  if (deep > 20)
    deep = 20;

  if ((it != NOTHING) && Can_Locate(executor, it)) {
    for (i = 0; i < deep; i++) {
      if (!GoodObject(it) || IsRoom(it))
	break;
      it = Location(it);
    }
    safe_str(unparse_dbref(it), buff, bp);
    return;
  }
  safe_str("#-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_zone)
{
  dbref it;

  if (nargs == 2) {
    if (!command_check_byname(executor, "@chzone")) {
      safe_str("#-1 PERMISSION DENIED", buff, bp);
      return;
    }
#ifdef FUNCTION_SIDE_EFFECTS
    do_chzone(executor, args[0], args[1]);
#else
    safe_str("#-1 FUNCTION DISABLED", buff, bp);
    return;
#endif
  }
  it = match_thing(executor, args[0]);
  if (it == NOTHING || !Can_Examine(executor, it)) {
    safe_str("#-1", buff, bp);
    return;
  }
  safe_str(unparse_dbref(Zone(it)), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_parent)
{
  dbref it;

  if (nargs == 2) {
    if (!command_check_byname(executor, "@parent")) {
      safe_str("#-1 PERMISSION DENIED", buff, bp);
      return;
    }
#ifdef FUNCTION_SIDE_EFFECTS
    do_parent(executor, args[0], args[1]);
#else
    safe_str("#-1 FUNCTION DISABLED", buff, bp);
    return;
#endif
  }
  it = match_thing(executor, args[0]);
  if ((it == NOTHING) || !Can_Examine(executor, it)) {
    safe_str("#-1", buff, bp);
    return;
  }
  safe_str(unparse_dbref(Parent(it)), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_lparent)
{
  dbref it;
  dbref par;

  it = match_thing(executor, args[0]);
  if (!GoodObject(it)) {
    safe_str("#-1", buff, bp);
    return;
  }
  safe_str(unparse_dbref(it), buff, bp);
  par = Parent(it);
  while (GoodObject(par) && Can_Examine(executor, it)) {
    safe_chr(' ', buff, bp);
    safe_str(unparse_dbref(par), buff, bp);
    it = par;
    par = Parent(par);
  }
}

/* ARGSUSED */
FUNCTION(fun_home)
{
  dbref it = match_thing(executor, args[0]);
  if ((it == NOTHING) || !Can_Examine(executor, it) ||
      IsRoom(it)) {
    safe_str("#-1", buff, bp);
    return;
  }
  if (IsExit(it)) {
    safe_str(unparse_dbref(Source(it)),buff, bp);
  }
  else {
    safe_str(unparse_dbref(Home(it)),buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_money)
{
  dbref it = match_thing(executor, args[0]);
  if (it == NOTHING) {
    safe_str("#-1", buff, bp);
    return;
  }
  safe_str(unparse_integer(Pennies(it)), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_owner)
{
  dbref thing;
  ATTR *attrib;

  if (strchr(args[0], '/')) {
    parse_attrib(executor, args[0], &thing, &attrib);
    if (!attrib)
      safe_str("#-1", buff, bp);
    else
      safe_str(unparse_dbref(attrib->creator), buff, bp);
  } else {
    thing = match_thing(executor, args[0]);
    if (thing == NOTHING)
      safe_str("#-1", buff, bp);
    else
      safe_str(unparse_dbref(Owner(thing)), buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_name)
{
  dbref it;

  if (nargs == 2) {
    if (!command_check_byname(executor, "@name")) {
      safe_str("#-1 PERMISSION DENIED", buff, bp);
      return;
    }
#ifdef FUNCTION_SIDE_EFFECTS
    do_name(executor, args[0], args[1]);
#else
    safe_str("#-1 FUNCTION DISABLED", buff, bp);
    return;
#endif
  }
  if ((it = match_thing(executor, args[0])) != NOTHING)
    safe_str(shortname(it), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_fullname)
{
  dbref it;

  if ((it = match_thing(executor, args[0])) != NOTHING)
    safe_str(Name(it), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_pmatch)
{
  dbref target;

#ifdef TREKMUSH
  if (!strcasecmp(args[0], "me"))
    if (Typeof(enactor) == TYPE_PLAYER) {
      safe_str(unparse_dbref(enactor), buff, bp);
      return;
    }
#endif /* TREKMUSH */

  if (*args[0] == '*')
    target = lookup_player(args[0] + 1);
  else if (*args[0] == NUMBER_TOKEN) {
    target = parse_dbref(args[0]);
    if (!(GoodObject(target) && IsPlayer(target))) {
      notify(executor, "No match.");
      safe_str("#-1", buff, bp);
      return;
    } else {
      safe_str(unparse_dbref(target), buff, bp);
      return;
    }
  } else
    target = lookup_player(args[0]);
  if (target == NOTHING) {
    if (*args[0] == '*')
      target = short_page(args[0] + 1);
    else
      target = short_page(args[0]);
    if (hidden(target) && !Priv_Who(executor))
      target = NOTHING;
  }
  switch (target) {
  case NOTHING:
    notify(executor, "No match.");
    safe_str("#-1", buff, bp);
    break;
  case AMBIGUOUS:
    notify(executor, "I'm not sure who you mean.");
    safe_str("#-2", buff, bp);
    break;
  default:
    safe_str(unparse_dbref(target), buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_locate)
{
  dbref looker;
  object_flag_type pref_type;
  dbref item;
  char *p;
  int keys = 0;
  int done = 0;
  int ambig_ok = 0;
  long match_flags = 0;

  /* find out what we're matching in relation to */
  looker = match_thing(executor, args[0]);
  if (looker == NOTHING) {
    notify(executor, "I don't see that here.");
    safe_str("#-1", buff, bp);
    return;
  }
  if (!See_All(executor) && !controls(executor, looker)) {
    notify(executor, "Permission denied.");
    safe_str("#-1", buff, bp);
    return;
  }
  /* find out our preferred match type */
  pref_type = NOTYPE;
  for (p = args[2]; *p; p++) {
    switch (*p) {
    case 'N':
      pref_type = NOTYPE;
      break;
    case 'E':
      pref_type = TYPE_EXIT;
      break;
    case 'P':
      pref_type = TYPE_PLAYER;
      break;
    case 'R':
      pref_type = TYPE_ROOM;
      break;
    case 'T':
      pref_type = TYPE_THING;
      break;
    case 'L':
      keys = 1;
      break;
    }
  }

  if (keys)
    match_flags = MAT_CHECK_KEYS;

  /* now figure out where what kinds of matches we want done */
  for (p = args[2]; *p && !done; p++) {
    switch (*p) {
    case '*':
      match_flags |= MAT_EVERYTHING;
      done = 1;
      break;
    case 'a':
      match_flags |= MAT_ABSOLUTE;
      break;
    case 'c':
      match_flags |= MAT_CARRIED_EXIT;
      break;
    case 'e':
      match_flags |= MAT_EXIT;
      break;
    case 'h':
      match_flags |= MAT_HERE;
      break;
    case 'i':
      match_flags |= MAT_POSSESSION;
      break;
    case 'l':
      match_flags |= MAT_CONTAINER;
      break;
    case 'm':
      match_flags |= MAT_ME;
      break;
    case 'n':
      match_flags |= MAT_NEIGHBOR;
      break;
    case 'p':
      match_flags |= MAT_PLAYER;
      break;
    case 'N':
    case 'E':
    case 'P':
    case 'R':
    case 'T':
    case 'L':
      /* these are from previous type switch check. ignore. */
      break;
    case 'X':
      ambig_ok = 1;		/* okay to pick last match */
    default:
      notify(executor, tprintf("I don't understand switch '%c'.", *p));
      break;
    }
  }

  /* report the results */
  if (!ambig_ok)
    item = match_result(looker, args[1], pref_type, match_flags);
  else
    item = last_match_result(looker, args[1], pref_type, match_flags);

  if (item == NOTHING)
    notify(executor, "Nothing found.");
  else if (item == AMBIGUOUS)
    notify(executor, "More than one match found.");
  safe_str(tprintf("#%d", item), buff, bp);
}

#ifdef FUNCTION_SIDE_EFFECTS

/* --------------------------------------------------------------------------
 * Creation functions: CREATE, OPEN, DIG
 */

/* ARGSUSED */
FUNCTION(fun_create)
{
  int cost;

  if (!command_check_byname(executor, "@create")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  if (nargs == 2)
    cost = parse_integer(args[1]);
  else
    cost = 10;
  safe_str(unparse_dbref(do_create(executor, args[0], cost)), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_open)
{
  if (!command_check_byname(executor, "@open")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  safe_str(unparse_dbref(do_real_open(executor, args[0], args[1], NOTHING)),
	   buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_dig)
{
  if (!command_check_byname(executor, "@dig")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  safe_str(unparse_dbref(do_dig(executor, args[0], args, 0)), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_clone)
{
  if (!command_check_byname(executor, "@clone")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  safe_str(unparse_dbref(do_clone(executor, args[0])), buff, bp);
}


/* --------------------------------------------------------------------------
 * Attribute functions: LINK, SET
 */

/* ARGSUSED */
FUNCTION(fun_link)
{
  if (!command_check_byname(executor, "@link")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  do_link(executor, args[0], args[1]);
}

/* ARGSUSED */
FUNCTION(fun_set)
{
  if (!command_check_byname(executor, "@set")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  do_set(executor, args[0], args[1]);
}

/* ARGSUSED */
FUNCTION(fun_wipe)
{
  if (!command_check_byname(executor, "@wipe")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  do_wipe(executor, args[0]);
}


/* --------------------------------------------------------------------------
 * Misc functions: TEL
 */

/* ARGSUSED */
FUNCTION(fun_tel)
{
  if (!command_check_byname(executor, "@tel")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  do_teleport(executor, args[0], args[1]);
}


#else

/* ARGSUSED */
FUNCTION(fun_create)
{
  safe_str("#-1 FUNCTION DISABLED", buff, bp);
}
/* ARGSUSED */
FUNCTION(fun_open)
{
  safe_str("#-1 FUNCTION DISABLED", buff, bp);
}
/* ARGSUSED */
FUNCTION(fun_dig)
{
  safe_str("#-1 FUNCTION DISABLED", buff, bp);
}
/* ARGSUSED */
FUNCTION(fun_clone)
{
  safe_str("#-1 FUNCTION DISABLED", buff, bp);
}
/* ARGSUSED */
FUNCTION(fun_link)
{
  safe_str("#-1 FUNCTION DISABLED", buff, bp);
}
/* ARGSUSED */
FUNCTION(fun_set)
{
  safe_str("#-1 FUNCTION DISABLED", buff, bp);
}
/* ARGSUSED */
FUNCTION(fun_wipe)
{
  safe_str("#-1 FUNCTION DISABLED", buff, bp);
}
/* ARGSUSED */
FUNCTION(fun_tel)
{
  safe_str("#-1 FUNCTION DISABLED", buff, bp);
}

#endif				/* FUNCTION_SIDE_EFFECTS */

/* ARGSUSED */
FUNCTION(fun_isdbref)
{
  safe_chr((parse_dbref(args[0]) != NOTHING) ? '1' : '0', buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_grep)
{
  char *tp;

  dbref it = match_thing(executor, args[0]);
  if (it == NOTHING) {
    safe_str("#-1 NO SUCH OBJECT VISIBLE", buff, bp);
    return;
  }
  /* make sure there's an attribute and a pattern */
  if (!*args[1]) {
    safe_str("#-1 NO SUCH ATTRIBUTE", buff, bp);
    return;
  }
  if (!*args[2]) {
    safe_str("#-1 INVALID GREP PATTERN", buff, bp);
    return;
  }
  tp = grep_util(executor, it, args[1], args[2], strlen(args[2]),
		 strcmp(called_as, "GREP"));
#ifdef MEM_CHECK
  add_check("fun_grep.attr_list");
#endif
  safe_str(tp, buff, bp);
  mush_free((Malloc_t) tp, "fun_grep.attr_list");
}

/* Get database size statistics */
/* ARGSUSED */
FUNCTION(fun_lstats)
{
  dbref who;
  char tbuf1[BUFFER_LEN];
  struct db_stat_info *si;

  if ((!args[0]) || !*args[0] || !strcasecmp(args[0], "all")) {
    who = ANY_OWNER;
  } else if (!strcasecmp(args[0], "me")) {
    who = executor;
  } else {
    who = lookup_player(args[0]);
    if (who == NOTHING) {
      safe_str("#-1 NOT FOUND", buff, bp);
      return;
    }
  }
  if (!Search_All(executor)) {
    if (who != ANY_OWNER && who != executor) {
      safe_str("#-1 PERMISSION DENIED", buff, bp);
      return;
    }
  }
  si = get_stats(executor, who);
  if (who != ANY_OWNER) {
    sprintf(tbuf1, "%d %d %d %d %d", si->total - si->garbage, si->rooms, si->exits, si->things, si->players);
  } else {
    sprintf(tbuf1, "%d %d %d %d %d %d", si->total, si->rooms, si->exits, si->things,
	    si->players, si->garbage);
  }
  safe_str(tbuf1, buff, bp);
}


/* ARGSUSED */
FUNCTION(fun_atrlock)
{
  dbref thing;
  char *p;
  ALIST *ptr;
  int status;

#ifdef FUNCTION_SIDE_EFFECTS
  if (nargs == 1)
    status = 0;
  else {
    if (!strcasecmp(args[1], "on")) {
      status = 1;
    } else if (!strcasecmp(args[1], "off")) {
      status = 2;
    } else
      status = 0;
  }
#else
  status = 0;
#endif

  if (!args[0] || !*args[0]) {
    safe_str("#-1 ARGUMENT MUST BE OBJ/ATTR", buff, bp);
    return;
  }
  if (!(p = (char *) index(args[0], '/')) || !(*(p + 1))) {
    safe_str("#-1 ARGUMENT MUST BE OBJ/ATTR", buff, bp);
    return;
  }
  *p++ = '\0';

  if ((thing = noisy_match_result(executor, args[0], NOTYPE, MAT_EVERYTHING)) == NOTHING) {
    safe_str("#-1", buff, bp);
    return;
  }
  for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr))
    if (!strcasecmp(AL_NAME(ptr), p)
#ifndef VISIBLE_EMPTY_ATTRS
	&& *AL_STR(ptr)
#endif
      )
      break;

  if (!ptr) {
    ptr = aname_hash_lookup(strupper(p));
    if (!ptr || !strcmp(p, AL_NAME(ptr)))
      ptr = NULL;
    else {
      p = (char *) AL_NAME(ptr);
      for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr))
	if (!strcmp(AL_NAME(ptr), p)
#ifndef VISIBLE_EMPTY_ATTRS
	    && *AL_STR(ptr)
#endif
	  )
	  break;
    }
  }
  if (ptr) {
    switch (status) {
    case 0:
      safe_str(unparse_integer(!!(AL_FLAGS(ptr) & AF_LOCKED)), buff, bp);
      return;
    case 1:
      if (!command_check_byname(executor, "@atrlock")) {
	safe_str("#-1 PERMISSION DENIED", buff, bp);
	return;
      }
      if (!Wizard(executor) &&
	  (Owner(AL_CREATOR(ptr)) != Owner(executor))) {
	safe_str("#-1 PERMISSION DENIED", buff, bp);
	return;
      }
      AL_FLAGS(ptr) |= AF_LOCKED;
      return;
    case 2:
      if (!command_check_byname(executor, "@atrlock")) {
	safe_str("#-1 PERMISSION DENIED", buff, bp);
	return;
      }
      if (!Wizard(executor) &&
	  (Owner(AL_CREATOR(ptr)) != Owner(executor))) {
	safe_str("#-1 PERMISSION DENIED", buff, bp);
	return;
      }
      AL_FLAGS(ptr) &= ~AF_LOCKED;
      return;
    default:
      safe_str("#-1", buff, bp);
      return;
    }
  } else
    safe_str("#-1 NO SUCH ATTRIBUTE", buff, bp);
  return;
}

#ifdef CHAT_SYSTEM
FUNCTION(fun_channels)
{
  dbref it;

  if (nargs == 1) {
    CHANLIST *c;
    /* Given an argument, return list of channels it's on */
    it = match_thing(executor, args[0]);
    if ((it == NOTHING) || !Can_Examine(executor, it)) {
      safe_str("#-1", buff, bp);
      return;
    }
    for (c = Chanlist(it); c; c = c->next) {
      if (c != Chanlist(it))
	safe_chr(' ', buff, bp);
      safe_str(ChanName(c->chan), buff, bp);
    }
  } else {
    CHAN *c;
    /* No arguments - return list of all channels */
    for (c = channels; c; c = c->next) {
      if (Chan_Can_See(c, executor)) {
	if (c != channels)
	  safe_chr(' ', buff, bp);
	safe_str(ChanName(c), buff, bp);
      }
    }
  }

  return;
}
#endif
