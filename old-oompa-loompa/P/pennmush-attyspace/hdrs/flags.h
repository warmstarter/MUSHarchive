/* flags.h */

/* flag and powers stuff */

#ifndef __FLAGS_H
#define __FLAGS_H

#include "conf.h"

/*--------------------------------------------------------------------------
 * Generic flags
 */

#define TYPE_ROOM 	0x0
#define TYPE_THING 	0x1
#define TYPE_EXIT 	0x2
#define TYPE_PLAYER 	0x3
#define TYPE_GARBAGE    0x6
#define NOTYPE		0x7	/* no particular type */
#define TYPE_MASK 	0x7	/* room for expansion */

/* -- empty slot 0x8 -- */
#define WIZARD		0x10	/* gets automatic control */
#define LINK_OK		0x20	/* anybody can link to this room */
#define DARK		0x40	/* contents of room are not printed */
				/* exit doesn't appear as 'obvious' */
#define VERBOSE		0x80	/* print out command before executing it */
#define STICKY		0x100	/* goes home when dropped */
#define TRANSPARENTED     0x200	/* can look through exit to see next room,
				   * or room "long exit display.
				   * We don't call it TRANSPARENT because
				   * that's a Solaris macro
				 */
#define	HAVEN		0x400	/* this room disallows kills in it */
				/* on a player, disallow paging, */
#define QUIET		0x800	/* On an object, will not emit 'set'
				   * messages.. on a player.. will not see ANY
				   * set messages
				 */
#define HALT		0x1000	/* object cannot perform actions */
#define UNFIND          0x2000	/* object cannot be found (or found in */
#define GOING 		0x4000	/* object is available for recycling */

#define ACCESSED 	0x8000	/* object has been accessed recently */
#define MARKED   	0x10000	/* flag used to trace db checking of room
				   * linkages. */
/* We don't #ifdef USE_WARNINGS because this slot is now filled,
 * regardless 
 */
#define NOWARN		0x20000	/* Object will not cause warnings. 
				   * If set on a player, player will not
				   * get warnings (independent of player's
				   * @warning setting
				 */

#define CHOWN_OK 	0x40000	/* object can be 'stolen' and made yours */
#define ENTER_OK	0x80000	/* object basically acts like a room with
				   * only one exit (leave), on players 
				   * means that items can be given freely, AND
				   * taken from!
				 */
#define VISUAL		0x100000	/* People other than owners can see 
					   * property list of object.
					 */
#define LIGHT		0x200000	/* Visible in DARK rooms */

#ifdef ROYALTY_FLAG
#define ROYALTY		0x400000	/* can ex, and @tel like a wizard */
#endif				/* ROYALTY_FLAG */

#define OPAQUE  	0x800000	/* Objects inside object will not be
					   * seen on a look.
					 */
#define INHERIT 	0x1000000	/* Inherit objects can force their
					   * owners. Inherit players have all
					   * objects inherit.
					 */

#define DEBUGGING	0x2000000	/* returns parser evals */
#define SAFE		0x4000000	/* cannot be destroyed */
#define STARTUP 	0x8000000	/* These objects have a @startup
					   * triggered when the MUSH restarts.
					 */
#define AUDIBLE 	0x10000000	/* rooms are flagged as having emitter 
					   * exits. exits act like emitters, 
					   * sound propagates to the exit dest. 
					 */
#define NO_COMMAND      0x20000000	/* don't check for $commands */

#define GOING_TWICE     0x40000000	/* Marked for destruction, but
					   * spared once. */
/* -- empty slot 0x80000000 -- */

/*--------------------------------------------------------------------------
 * Player flags
 */

#define PLAYER_TERSE    0x8	/* suppress autolook messages */
#define PLAYER_MYOPIC   0x10	/* look at everything as if player
				   * doesn't control it.
				 */
#define PLAYER_NOSPOOF  0x20	/* sees origin of emits */
#define PLAYER_SUSPECT  0x40	/* notifies of a player's name changes,
				   * (dis)connects, and possible logs
				   * logs commands.
				 */
#define PLAYER_GAGGED   0x80	/* can only move */
#define PLAYER_MONITOR  0x100	/* sees (dis)connects broadcasted */
#define PLAYER_CONNECT  0x200	/* connected to game */
#define PLAYER_ANSI     0x400	/* enable sending of ansi control
				   * sequences (for examine).
				 */
#define PLAYER_ZONE     0x800	/* Zone Master (zone control owner) */
#ifdef JURY_OK
#define PLAYER_JURY   0x1000	/* Jury_ok Player */
#define PLAYER_JUDGE  0x2000	/* Judge player */
#endif
#ifdef FIXED_FLAG
#define PLAYER_FIXED  0x4000	/* can't @tel or home */
#endif
#ifdef ONLINE_REG
#define PLAYER_UNREG  0x8000	/* Not yet registered */
#endif
#ifdef VACATION_FLAG
#define PLAYER_VACATION 0x10000	/* On vacation */
#endif
#ifdef EXTENDED_ANSI
#define PLAYER_COLOR	  0x80000	/* ANSI color ok */
#define PLAYER_FORCEWHITE 0x100000	/* Force ansi_white after output */
#endif
#ifdef TREKMUSH
#define PLAYER_IN_CHARACTER	0x200000	/* Player is 'In-Character' */
#endif /* TREKMUSH */

/*--------------------------------------------------------------------------
 * Thing flags
 */

#define THING_DEST_OK	0x8	/* can be destroyed by anyone */
#define THING_PUPPET	0x10	/* echoes to its owner */
#define THING_LISTEN	0x20	/* checks for ^ patterns */
#define THING_NOLEAVE	0x40	/* Can't be left */

#ifdef TREKMUSH
#define THING_SPACE_OBJECT	0x80	/* Object is a Space-Object */
#define THING_ORG_OBJECT	0x100	/* Object is an Org-Object */
#endif /* TREKMUSH */

/*--------------------------------------------------------------------------
 * Room flags
 */

#define ROOM_FLOATING	0x8	/* room is not linked to rest of
				   * MUSH. Don't blather about it.
				 */
#define ROOM_ABODE	0x10	/* players may link themselves here */
#define ROOM_JUMP_OK	0x20	/* anyone may @teleport to here */
#define ROOM_NO_TEL	0x40	/* mortals cannot @tel from here */
#define ROOM_TEMPLE	0x80	/* objects dropped here are sacrificed
				   * (destroyed) and player gets money.
				   * Now used only for conversion.
				 */
#define ROOM_LISTEN    0x100	/* checks for ^ patterns */
#define ROOM_Z_TEL     0x200	/* If set on a room, players may
				   * only @tel to another room in the
				   * same zone 
				 */
#ifdef UNINSPECTED_FLAG
#define ROOM_UNINSPECT 0x1000	/* Not inspected */
#endif


/*--------------------------------------------------------------------------
 * Exit flags
 */

#define EXIT_CLOUDY	0x8	/* Looking through a cloudy transparent
				   * exit shows the room's desc, not contents.
				   * Looking through a cloudy !trans exit,
				   * shows the room's contents, not desc
				 */

/*--------------------------------------------------------------------------
 * Flag permissions
 */

#define F_ANY		0x10	/* can be set by anyone */
#define F_INHERIT	0x20	/* must pass inherit check */
#define F_OWNED		0x40	/* can be set on owned objects */
#define F_ROYAL		0x80	/* can only be set by royalty */
#define F_WIZARD	0x100	/* can only be set by wizards */
#define F_GOD           0x200	/* can only be set by God */
#define F_INTERNAL	0x400	/* only the game can set this */
#define F_DARK  	0x800	/* only God can see this flag */
#define F_MDARK   	0x1000	/* admin/God can see this flag */
#define F_ODARK		0x2000	/* owner/admin/God can see this flag */


/*--------------------------------------------------------------------------
 * Powers table
 */

#define CAN_BUILD	0x10	/* can use builder commands */
#define TEL_ANYWHERE	0x20	/* teleport self anywhere */
#define TEL_OTHER       0x40	/* teleport someone else */
#define SEE_ALL	        0x80	/* can examine all and use priv WHO */
#define NO_PAY		0x100	/* Needs no money */
#define CHAT_PRIVS	0x200	/* can use restricted channels */
#define CAN_HIDE        0x400	/* can go DARK on the WHO list */
#define LOGIN_ANYTIME   0x800	/* not affected by restricted logins */
#define UNLIMITED_IDLE  0x1000	/* no inactivity timeout */
#define LONG_FINGERS    0x2000	/* can grab stuff remotely */
#define CAN_BOOT        0x4000	/* can boot off players */
#define CHANGE_QUOTAS   0x8000	/* can change other players' quotas */
#define SET_POLL        0x10000	/* can change the poll */
#define HUGE_QUEUE	0x20000	/* queue limit of db_top + 1 */
#define PS_ALL          0x40000	/* look at anyone's queue */
#define HALT_ANYTHING   0x80000	/* do @halt on others, and @allhalt */
#define SEARCH_ALL	0x100000	/* @stats, @search, @entrances all */
#define GLOBAL_FUNCS    0x200000	/* add global functions */
#define CREATE_PLAYER   0x400000	/* @pcreate */
#define IS_GUEST	0x800000	/* Guest, restrict access */
#define CAN_WALL	0x1000000	/* @wall */
#define CEMIT		0x2000000	/* Can @cemit */
#define UNKILLABLE	0x4000000	/* Cannot be killed */
#define PEMIT_ALL	0x8000000	/* Can @pemit to HAVEN players */
#define NO_QUOTA	0x10000000	/* Has no quota restrictions */
#define OPEN_ANYWHERE	0x20000000	/* Can @open exits between any rooms */

/* These powers are obsolete, but are kept around to implement
 * DBF_SPLIT_IMMORTAL
 */
#define CAN_DEBUG	0x4000000	/* Can set/unset the debug flag */
#define IMMORTAL	0x100	/* cannot be killed, uses no money */
#endif				/* __FLAGS_H */
