/* dbdefs.h */

#ifndef __DBDEFS_H
#define __DBDEFS_H

#include <stdio.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#endif
#include <time.h>
#include "htab.h"

extern int depth;

extern dbref first_free;	/* pointer to free list */

/* Some stuff we moved from option.h, of interest to people trying
 * to use really old databases or code.
 */

/* With this option, '&<attr> <obj>;' will delete an attribute,
 * otherwise it will set the attribute empty.  Recommended, unless
 * emulating pre-1.6.9 behavior.
 */
#define DELETE_ATTRS		/* */

/* With this option, empty attributes will be visible with hasattr(),
 * lattr(), grep(), etc.  Also controls whether empty attrubutes stop
 * searches through parent chains.  Recommended, unless emulating
 * pre-1.6.9 behavior.
 */
#define VISIBLE_EMPTY_ATTRS	/* */

/*-------------------------------------------------------------------------
 * Database access macros
 */

/* References an whole object */
#define REFDB(x)        &db[x]

#define Name(x)         (db[(x)].name)
#define Flags(x)        (db[(x)].flags)
#define Owner(x)        (db[(x)].owner)

#define Location(x)     (db[(x)].location)
#define Zone(x)         (db[(x)].zone)

#define Contents(x) (db[(x)].contents)
#define Next(x)     (db[(x)].next)
#define Home(x)     (db[(x)].exits)
#define Exits(x)    (db[(x)].exits)
#define List(x)     (db[(x)].list)

/* These are only for exits */
#define Source(x)   (db[(x)].exits)
#define Destination(x) (db[(x)].location)

#define Locks(x)        (db[(x)].locks)

#define CreTime(x)      (db[(x)].creation_time)
#define ModTime(x)      (db[(x)].modification_time)

#define AttrCount(x)    (db[(x)].attrcount)

/* Moved from warnings.c because create.c needs it. */
#define Warnings(x)      (db[(x)].warnings)

#define Pennies(thing) (db[thing].penn)
#define s_Pennies(thing,p) (db[thing].penn=(p))

#define Parent(x)  (db[(x)].parent)
#define Powers(x)  (db[(x)].powers)

/* Generic type check */
#define Type(x)   (db[(x)].type)
#define Typeof(x) (Type(x) & ~TYPE_MARKED)

/* Check for a specific one */
#define IsPlayer(x)     (Typeof(x) & TYPE_PLAYER)
#define IsRoom(x)       (Typeof(x) & TYPE_ROOM)
#define IsThing(x)      (Typeof(x) & TYPE_THING)
#define IsExit(x)       (Typeof(x) & TYPE_EXIT)
/* Was Destroyed() */
#define IsGarbage(x)    (Typeof(x) & TYPE_GARBAGE)
#define Marked(x)       (db[(x)].type & TYPE_MARKED)

#define IS(thing,type,flag) \
                     ((Typeof(thing) == type) && has_flag_by_name(thing,flag,type))

#define GoodObject(x) ((x >= 0) && (x < db_top))



/******* Player toggles */
#define Suspect(x)      (IS(x, TYPE_PLAYER, "SUSPECT"))	/* 0x40 */
#define Connected(x)    (IS(x, TYPE_PLAYER, "CONNECTED"))	/* 0x200 */
#define ZMaster(x)      (IS(x, TYPE_PLAYER, "ZONE"))	/* 0x800 */
#define Unregistered(x) (IS(x, TYPE_PLAYER, "UNREGISTERD"))
#define Fixed(x)        (IS(Owner(x), TYPE_PLAYER, "FIXED"))

#ifdef VACATION_FLAG
#define Vacation(x)     (IS(x, TYPE_PLAYER, "ON-VACATION"))	/* 0x10000 */
#else
#define Vacation(x)     0
#endif

/* Flags that apply to players, and all their stuff,
 * so check the Owner() of the object.
 */

#define Terse(x)        (IS(Owner(x), TYPE_PLAYER, "TERSE"))
#define Myopic(x)       (IS(Owner(x), TYPE_PLAYER, "MYOPIC"))
#define Nospoof(x)      (IS(Owner(x), TYPE_PLAYER, "NOSPOOF"))
#define Paranoid(x)     (IS(Owner(x), TYPE_PLAYER, "PARANOID"))
#define Gagged(x)       (IS(Owner(x), TYPE_PLAYER, "GAGGED"))
#define ShowAnsi(x)     (IS(Owner(x), TYPE_PLAYER, "ANSI"))
#define ShowAnsiColor(x) (IS(Owner(x), TYPE_PLAYER, "COLOR"))

/******* Thing toggles */
#define DestOk(x)       (IS(x, TYPE_THING, "DESTROY_OK"))
#define Puppet(x)       (IS(x, TYPE_THING, "PUPPET"))
#define NoLeave(x)      (IS(x, TYPE_THING, "NOLEAVE"))
#define ThingListen(x)  (IS(x, TYPE_THING, "MONITOR"))
#define ThingInhearit(x) \
                        (IS(x, TYPE_THING, "LISTEN_PARENT"))	/* 0x80 */
#define ThingZTel(x)            (IS(x, TYPE_THING, "Z_TEL"))

/******* Room toggles */
#define Floating(x)     (IS(x, TYPE_ROOM, "FLOATING"))	/* 0x8 */
#define Abode(x)        (IS(x, TYPE_ROOM, "ABODE"))	/* 0x10 */
#define JumpOk(x)       (IS(x, TYPE_ROOM, "JUMP_OK"))	/* 0x20 */
#define NoTel(x)        (IS(x, TYPE_ROOM, "NO_TEL"))	/* 0x40 */
#define RoomListen(x)   (IS(x, TYPE_ROOM, "LISTENER"))	/* 0x100 */
#define RoomZTel(x)             (IS(x, TYPE_ROOM, "Z_TEL"))	/* 0x200 */
#define RoomInhearit(x) (IS(x, TYPE_ROOM, "LISTEN_PARENT"))	/* 0x400 */

#ifdef UNINSPECTED_FLAG
#define Uninspected(x)  (IS(x, TYPE_ROOM, "UNINSPECTED"))	/* 0x1000 */
#else
#define Uninspected(x)  0
#endif

#define ZTel(x) (ThingZTel(x) || RoomZTel(x))

/******* Exit toggles */
#define Cloudy(x)       (IS(x, TYPE_EXIT, "CLOUDY"))	/* 0x8 */

/* Flags anything can have */

#define Audible(x)      (has_flag_by_name(x, "AUDIBLE", NOTYPE))
#define ChownOk(x)      (has_flag_by_name(x, "CHOWN_OK", NOTYPE))
#define Dark(x)         (has_flag_by_name(x, "DARK", NOTYPE))
#define Debug(x)        (has_flag_by_name(x, "DEBUG", NOTYPE))
#define EnterOk(x)      (has_flag_by_name(x, "ENTER_OK", NOTYPE))
#define Going(x)        (has_flag_by_name(x, "GOING", NOTYPE))
#define Going_Twice(x)  (has_flag_by_name(x, "GOING_TWICE", NOTYPE))
#define Halted(x)       (has_flag_by_name(x, "HALT", NOTYPE))
#define Haven(x)        (has_flag_by_name(x, "HAVEN", NOTYPE))
#define Inherit(x)      (has_flag_by_name(x, "TRUST", NOTYPE))
#define Light(x)        (has_flag_by_name(x, "LIGHT", NOTYPE))
#define LinkOk(x)       (has_flag_by_name(x, "LINK_OK", NOTYPE))
#define Mistrust(x)     (has_flag_by_name(x, "MISTRUST", TYPE_THING|TYPE_EXIT|TYPE_ROOM))
#define NoCommand(x)    (has_flag_by_name(x, "NO_COMMAND", NOTYPE))
#define NoWarn(x)       (has_flag_by_name(x, "NOWARN", NOTYPE))
#define Opaque(x)       (has_flag_by_name(x, "OPAQUE", NOTYPE))
#define Quiet(x)        (has_flag_by_name(x, "QUIET", NOTYPE))
#define Safe(x)         (has_flag_by_name(x, "SAFE", NOTYPE))
#define Sticky(x)       (has_flag_by_name(x, "STICKY", NOTYPE))
#define Transparented(x)      (has_flag_by_name(x, "TRANSPARENT", NOTYPE))
#define Unfind(x)       (has_flag_by_name(x, "UNFINDABLE", NOTYPE))
#define Verbose(x)      (has_flag_by_name(x, "VERBOSE", NOTYPE))
#define Visual(x)       (has_flag_by_name(x, "VISUAL", NOTYPE))

/* Non-mortal checks */
#define God(x)  ((x) == GOD)
#define Royalty(x)      (has_flag_by_name(x, "ROYALTY", NOTYPE))
#define Wizard(x)       (God(x) || has_flag_by_name(x,"WIZARD", NOTYPE))
#define Hasprivs(x)     (God(x) || Royalty(x) || Wizard(x))

#define IsQuiet(x)      (Quiet(x) || Quiet(Owner(x)))
#define AreQuiet(x,y)   (Quiet(x) || (Quiet(y) && (Owner(y) == x)))
#define Mobile(x)       (IsPlayer(x) || IsThing(x))
#define Alive(x)        (IsPlayer(x) || Puppet(x) || \
   (Audible(x) && atr_get_noparent(x,"FORWARDLIST")))
/* Was Dark() */
#define DarkLegal(x)    (Dark(x) && (Wizard(x) || !Alive(x)))



/* This is carefully ordered, from most to least likely. Hopefully. */
#define CanEval(x,y)    (!(SAFER_UFUN) || !Hasprivs(y) || God(x) || \
        ((Wizard(x) || (Royalty(x) && !Wizard(y))) && !God(y)))

/* Note that this is a utility to determine the objects which may or may */
/* not be controlled, rather than a strict check for the INHERIT flag */
#define Owns(p,x) (Owner(p) == Owner(x))


/* Was Inherit() */
#define Inheritable(x)  (IsPlayer(x) || Inherit(x) || \
                        Inherit(Owner(x)) || Wizard(x))


/*--------------------------------------------------------------------------
 * Other db stuff
 */

/** An object in the database.
 *
 */
struct object {
  const char *name;		/**< The name of the object */
  /** An overloaded pointer.
   * For things and players, points to container object.
   * For exits, points to destination.
   * For rooms, points to drop-to.
   */
  dbref location;
  dbref contents;		/**< Pointer to first item */
  /** An overloaded pointer.
   * For things and players, points to home.
   * For rooms, points to first exit.
   * For exits, points to source room.
   */
  dbref exits;
  dbref next;			/**< pointer to next in contents/exits chain */
  dbref parent;			/**< pointer to parent object */
  struct lock_list *locks;	/**< list of locks set on the object */
  dbref owner;			/**< who controls this object */
  dbref zone;			/**< zone master object number */
  int penn;			/**< number of pennies object contains */
  int warnings;			/**< bitflags of warning types */
  time_t creation_time;		/**< Time/date of object creation */
  /** Last modifiction time.
   * For players, the number of failed logins.
   * For other objects, the time/date of last modification to its attributes.
   */
  time_t modification_time;
  int attrcount;		/**< Number of attribs on the object */
  int type;			/**< Object's type */
  object_flag_type flags;	/**< Pointer to flag bit array */
  int powers;			/**< bitflags of powers */
  ALIST *list;			/**< list of attributes on the object */
};

/** A structure to hold database statistics.
 * This structure is used by get_stats() in wiz.c to group
 * counts of various objects in the database.
 */
struct db_stat_info {
  int total;	/**< Total count */
  int players;	/**< Player count */
  int rooms;	/**< Room count */
  int exits;	/**< Exit count */
  int things;	/**< Thing count */
  int garbage;	/**< Garbage count */
};

extern struct object *db;
extern dbref db_top;

extern void *get_objdata(dbref thing, const char *keybase);
extern void *set_objdata(dbref thing, const char *keybase, void *data);
extern void clear_objdata(dbref thing);

#define DOLIST(var, first)\
    for((var) = (first); GoodObject((var)); (var) = Next(var))

#define PUSH(thing, locative) \
    ((Next(thing) = (locative)), (locative) = (thing))

#define DOLIST_VISIBLE(var, first, player)\
    for((var) = first_visible((player), (first)); GoodObject((var)); (var) = first_visible((player), Next(var)))

#ifdef USE_MAILER
/** A mail message.
 * This structure represents a single mail message in the linked list
 * of messages that comprises the mail database. Mail messages are
 * stored in a doubly-linked list sorted by message recipient.
 */
struct mail {
  struct mail *next;		/**< Pointer to next message */
  struct mail *prev;		/**< Pointer to previous message */
  dbref to;			/**< Recipient dbref */
  dbref from;			/**< Sender's dbref */
  unsigned char *message;	/**< Message text, compressed */
  unsigned char *time;		/**< Message date/time, compressed */
  unsigned char *subject;	/**< Message subject, compressed */
  int read;			/**< Bitflags of message status */
};

#endif				/* USE_MAILER */

extern const char *EOD;

#endif				/* __DBDEFS_H */
