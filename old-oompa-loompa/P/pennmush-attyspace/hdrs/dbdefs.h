/* dbdefs.h */

#ifndef __DBDEFS_H
#define __DBDEFS_H

#include <stdio.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#endif
#ifdef I_TIME
#include <time.h>
#endif
#include "attrib.h"
#include "options.h"

extern int depth;

typedef int object_flag_type;

#ifdef CHAT_SYSTEM
/* Must come after dbref is defined */
#include "extchat.h"
#endif

extern dbref first_free;	/* pointer to free list */

/* macro to make set string easier to use */
#define SET(a,b) set_string(&(a),b)

/* set macro for compressed strings */

#ifdef COMPRESS
#define SETC(a,b) SET(a,compress(b))
#else
#define SETC(a,b) SET(a,b)
#endif				/* COMPRESS */

/* Updates an objects age */
#define Access(x) (Flags(x) |= ACCESSED)

/*-------------------------------------------------------------------------
 * Database access macros
 */

/* References an whole object */
#define REFDB(x)	&db[x]

#define Name(x)		(db[(x)].name)
#define Flags(x)  	(db[(x)].flags)
#define Toggles(x) 	(db[(x)].toggles)
#define Owner(x)  	(db[(x)].owner)

#define Location(x) 	(db[(x)].location)
#define Zone(x)     	(db[(x)].zone)

#define Contents(x) (db[(x)].contents)
#define Next(x)     (db[(x)].next)
#define Home(x)     (db[(x)].exits)
#define Exits(x)    (db[(x)].exits)
#define List(x)     (db[(x)].list)

/* These are only for exits */
#define Source(x)   (db[(x)].exits)
#define Destination(x) (db[(x)].location)

#define Locks(x)	(db[(x)].locks)

#ifdef CREATION_TIMES
#define CreTime(x)	(db[(x)].creation_time)
#define ModTime(x)	(db[(x)].modification_time)
#endif

#ifdef LOCAL_DATA
#define LocData(x)	(db[(x)].local_data)
#endif

/* Moved from warnings.c because create.c needs it. */
#ifdef USE_WARNINGS
#define Warnings(x)      (db[(x)].warnings)
#endif				/* USE_WARNINGS */

#define Pennies(thing) (db[thing].penn)
#define s_Pennies(thing,p) (db[thing].penn=(p))

#define Parent(x)  (db[(x)].parent)
#define Powers(x)  (db[(x)].powers)

/* Generic type check */
#define Typeof(x) (Flags(x) & TYPE_MASK)

/* Check for a specific one */
#define IsPlayer(x)	(Typeof(x) == TYPE_PLAYER)
#define IsRoom(x)	(Typeof(x) == TYPE_ROOM)
#define IsThing(x)	(Typeof(x) == TYPE_THING)
#define IsExit(x)	(Typeof(x) == TYPE_EXIT)
/* Was Destroyed() */
#define IsGarbage(x)	(Typeof(x) == TYPE_GARBAGE)

#define IS(thing,type,flag) \
                     ((Typeof(thing) == type) && (Toggles(thing) & flag))

#define Hasflag(thing,type,flag) \
                     ((Typeof(thing) == type) && (Flags(thing) & flag))

#define GoodObject(x) ((x >= 0) && (x < db_top))



/******* Player toggles */
#define Suspect(x)  	(IS(x, TYPE_PLAYER, PLAYER_SUSPECT))	/* 0x40 */
#define Connected(x)	(IS(x, TYPE_PLAYER, PLAYER_CONNECT))	/* 0x200 */
#define ZMaster(x)      (IS(x, TYPE_PLAYER, PLAYER_ZONE))	/* 0x800 */
#define Unregistered(x) (IS(x, TYPE_PLAYER, PLAYER_UNREG))

/* If the FIXED flag isn't compiled in, have the check always fail */
#ifdef FIXED_FLAG
#define Fixed(x)	(Toggles(Owner(x)) & PLAYER_FIXED)	/* 0x4000 */
#else
#define Fixed(x)  0
#endif

#ifdef VACATION_FLAG
#define Vacation(x)	(IS(x, TYPE_PLAYER, PLAYER_VACATION))	/* 0x10000 */
#else
#define Vacation(x)	0
#endif

/* Flags that apply to players, and all their stuff,
 * so check the Owner() of the object.
 */

#define Terse(x)	(Toggles(Owner(x)) & PLAYER_TERSE)	/* 0x8 */
#define Myopic(x)	(Toggles(Owner(x)) & PLAYER_MYOPIC)	/* 0x10 */
#define Nospoof(x)	(Toggles(Owner(x)) & PLAYER_NOSPOOF)	/* 0x20 */
#define Gagged(x)	(Toggles(Owner(x)) & PLAYER_GAGGED)	/* 0x80 */

#define ShowAnsi(x)	(Toggles(Owner(x)) & PLAYER_ANSI)	/* 0x400 */

#ifdef EXTENDED_ANSI
#define ShowAnsiColor(x) (Toggles(Owner(x)) & PLAYER_COLOR)
#else
#define ShowAnsiColor(x) 0
#endif

/******* Thing toggles */
#define DestOk(x)	(IS(x, TYPE_THING, THING_DEST_OK))
#define Puppet(x)	(IS(x, TYPE_THING, THING_PUPPET))
#define NoLeave(x)	(IS(x, TYPE_THING, THING_NOLEAVE))
#define ThingListen(x)	(IS(x, TYPE_THING, THING_LISTEN))

/******* Room toggles */
#define Floating(x)	(IS(x, TYPE_ROOM, ROOM_FLOATING))	/* 0x8 */
#define Abode(x)	(IS(x, TYPE_ROOM, ROOM_ABODE))	/* 0x10 */
#define JumpOk(x)	(IS(x, TYPE_ROOM, ROOM_JUMP_OK))	/* 0x20 */
#define NoTel(x)	(IS(x, TYPE_ROOM, ROOM_NO_TEL))		/* 0x40 */
#define RoomListen(x)	(IS(x, TYPE_ROOM, ROOM_LISTEN))		/* 0x100 */
#define ZTel(x)		(IS(x, TYPE_ROOM, ROOM_Z_TEL))	/* 0x200 */

#ifdef UNINSPECTED_FLAG
#define Uninspected(x)	(IS(x, TYPE_ROOM, ROOM_UNINSPECT))	/* 0x1000 */
#else
#define Uninspected(x)  0
#endif

/* Included for completeness. */
#define Temple(x)	(IS(x, TYPE_ROOM, ROOM_LISTEN))		/* 0x80 */

/******* Exit toggles */
#define Cloudy(x)	(IS(x, TYPE_EXIT, EXIT_CLOUDY))		/* 0x8 */

/* Flags anything can have */

#define Audible(x)	(Flags(x) & AUDIBLE)
#define ChownOk(x)	(Flags(x) & CHOWN_OK)
#define Dark(x)		(Flags(x) & DARK)
#define Debug(x)	(Flags(x) & DEBUGGING)
#define EnterOk(x)	(Flags(x) & ENTER_OK)
#define Going(x)	(Flags(x) & GOING)
#define Going_Twice(x)	(Flags(x) & GOING_TWICE)
#define Halted(x)	(Flags(x) & HALT)
#define Haven(x)	(Flags(x) & HAVEN)
#define Inherit(x)	(Flags(x) & INHERIT)
#define Light(x)	(Flags(x) & LIGHT)
#define LinkOk(x)	(Flags(x) & LINK_OK)
#define Marked(x)	(Flags(x) & MARKED)
#define NoCommand(x)	(Flags(x) & NO_COMMAND)
#define NoWarn(x)	(Flags(x) & NOWARN)
#define Opaque(x)	(Flags(x) & OPAQUE)
#define Quiet(x)	(Flags(x) & QUIET)
#define Safe(x)		(Flags(x) & SAFE)
#define Startup(x)	(Flags(x) & STARTUP)
#define Sticky(x)	(Flags(x) & STICKY)
#define Transparented(x)       (Flags(x) & TRANSPARENTED)
#define Unfind(x)	(Flags(x) & UNFIND)
#define Verbose(x)	(Flags(x) & VERBOSE)
#define Visual(x)	(Flags(x) & VISUAL)

/* Non-mortal checks */
#define God(x)  ((x) == GOD)
#ifdef ROYALTY_FLAG
#define Royalty(x)  (Flags(x) & ROYALTY)
#else				/* ROYALTY_FLAG */
#define Royalty(x)  0
#endif				/* ROYALTY_FLAG */
#define Wizard(x)	((Flags(x) & WIZARD) || God(x))
#define Hasprivs(x)	(Royalty(x) || Wizard(x) || God(x) )

#define IsQuiet(x)	(Quiet(x) || Quiet(Owner(x)))
#define AreQuiet(x,y)	(Quiet(x) || Quiet(y))
#define Mobile(x)	(IsPlayer(x) || IsThing(x))
#define Alive(x) 	(IsPlayer(x) || Puppet(x))
/* Was Dark() */
#define DarkLegal(x)	(Dark(x) && (Wizard(x) || !Alive(x)))



/* This is carefully ordered, from most to least likely. Hopefully. */
#define CanEval(x,y)	(!(SAFER_UFUN) || !Hasprivs(y) || God(x) || \
	((Wizard(x) || (Royalty(x) && !Wizard(y))) && !God(y)))

/* Note that this is a utility to determine the objects which may or may */
/* not be controlled, rather than a strict check for the INHERIT flag */
#define Owns(p,x) (Owner(p) == Owner(x))


/* Was Inherit() */
#define Inheritable(x)	(IsPlayer(x) || Inherit(x) || \
			Inherit(Owner(x)) || Wizard(x))


#ifdef TREKMUSH
#define Ic(x)       (IS(x, TYPE_PLAYER, PLAYER_IN_CHARACTER))
#define SpaceObj(x) (IS(x, TYPE_THING, THING_SPACE_OBJECT))
#define OrgObj(x)   (IS(x, TYPE_THING, THING_ORG_OBJECT))
#endif /* TREKMUSH */

/*--------------------------------------------------------------------------
 * Other db stuff
 */

/* Boolean expressions, for locks */
typedef char boolexp_type;
#define BOOLEXP_AND 0
#define BOOLEXP_OR 1
#define BOOLEXP_NOT 2
#define BOOLEXP_CONST 3
#define BOOLEXP_ATR 4
#define BOOLEXP_IND 5
#define BOOLEXP_CARRY 6
#define BOOLEXP_IS 7
#define BOOLEXP_OWNER 8
#define BOOLEXP_EVAL 9
#define BOOLEXP_NULL 127	/* Mark a lack of a boolexp */

struct boolexp {
  boolexp_type type;		/* and, or, not, indirect, is, carry, etc. */
  struct boolexp *sub1;		/* first part of lock */
  struct boolexp *sub2;		/* second part of lock */
  dbref thing;			/* thing refers to an object */
  struct boolatr *atr_lock;
};

#define TRUE_BOOLEXP ((struct boolexp *) 0)

/* special dbref's */
#define NOTHING (-1)		/* null dbref */
#define AMBIGUOUS (-2)		/* multiple possibilities, for matchers */
#define HOME (-3)		/* virtual room, represents mover's home */
#define ANY_OWNER (-2)		/* For lstats and @stat */

struct object {
  const char *name;
  dbref location;		/* pointer to container */
  /* for exits, pointer to destination */
  /* for rooms, pointer to drop-to */
  dbref contents;		/* pointer to first item */
  dbref exits;			/* pointer to first exit for rooms */
  /* pointer to home for things and players */
  /* pointer to room it's in for exits. */
  dbref next;			/* pointer to next in contents/exits chain */
  dbref parent;			/* pointer to last person paged for player */
  /* parent object for everything else */

  struct lock_list *locks;

  dbref owner;			/* who controls this object */
  dbref zone;			/* zone master object number */
  int penn;			/* number of pennies object contains */
#ifdef CHAT_SYSTEM
  CHANLIST *channels;
#endif
#ifdef USE_WARNINGS
  int warnings;			/* bitflags of warning types */
#endif
#ifdef CREATION_TIMES
  time_t creation_time;		/* Time/date of object creation */
  time_t modification_time;	/* Time/data of last modification to the
				 * object's attribs; number of failed
				 * logins for players. */
#endif
#ifdef LOCAL_DATA
  void *local_data;
#endif
  object_flag_type flags;
  object_flag_type toggles;
  object_flag_type powers;
  ALIST *list;
/* OFFLINE #ifdef TREKMUSH
  int gdbmquota;   
  int gdbmusage;    
#endif */
};

struct dblist {			/* used by routines in util.c */
  dbref obj;
  struct dblist *next;
};

struct db_stat_info {		/* used by get_stats in wiz.c (@stat, etc) */
  int total;
  int players;
  int rooms;
  int exits;
  int things;
  int garbage;
};

extern struct object *db;
extern dbref db_top;

extern const char *alloc_string _((void));
extern dbref new_object _((void));
extern void putstring _((FILE * f, const char *s));
extern long getref _((FILE * f));
extern void putref _((FILE * f, long int ref));
extern const char *getstring_noalloc _((FILE * f));
extern struct boolexp *getboolexp _((FILE * f));
extern void putboolexp _((FILE * f, struct boolexp * b));
extern int db_write_object _((FILE * f, dbref i));
extern dbref db_write _((FILE * f));
extern dbref db_read _((FILE * f));
/* Warning: destroys existing db contents! */
extern void free_bool _((struct boolexp * b));
extern struct boolexp *dup_bool _((struct boolexp * b));
extern struct boolexp *alloc_bool _((void));
extern void free_boolexp _((struct boolexp * b));
extern void db_free _((void));
extern dbref parse_dbref _((const char *s));
extern void putlocks _((FILE * f, struct lock_list *l));
extern void getlocks _((dbref i, FILE * f));
extern void get_old_locks _((dbref i, FILE * f));

#define DOLIST(var, first)\
    for((var) = (first); GoodObject((var)); (var) = Next(var))

#define PUSH(thing, locative) \
    ((Next(thing) = (locative)), (locative) = (thing))

#define DOLIST_VISIBLE(var, first, player)\
    for((var) = first_visible((player), (first)); GoodObject((var)); (var) = first_visible((player), Next(var)))

#ifdef USE_MAILER
struct mail {
  struct mail *next;
  struct mail *prev;
  dbref to;
  dbref from;
  unsigned char *message;	/* compressed */
  unsigned char *time;		/* compressed */
#ifdef MAIL_SUBJECTS
  unsigned char *subject;	/* compressed */
#endif
  int read;
};

#endif				/* USE_MAILER */

/* log types */
#define LT_ERR    0
#define LT_CMD    1
#define LT_WIZ    2
#define LT_CONN   3
#define LT_TRACE  4
#define LT_RPAGE  5
#define LT_CHECK  6
#define LT_HUH    7

#ifdef TREKMUSH
#define LT_SPACE    8
#endif /* TREKMUSH */

/* tokens for locks */
#define NOT_TOKEN '!'
#define AND_TOKEN '&'
#define OR_TOKEN '|'
#define AT_TOKEN '@'
#define IN_TOKEN '+'
#define IS_TOKEN '='
#define OWNER_TOKEN '$'

#endif				/* __DBDEFS_H */
