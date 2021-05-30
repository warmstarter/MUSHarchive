/****************************************************************
 * robot.h: Data structures for TinyMUD Robots
 *
 * HISTORY
 * 30-Oct-93  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Fourteenth animal release, third Loebner contest
 *
 * 12-Jun-92  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Thirteenth prodigal release.
 *	Mods for Time Traveller MUCK, Second Loebner contest
 *
 * 01-May-91  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Twelfth surgical release. Added more personality and
 *	descriptions to Julia.  Physical stats, wardrobe, appearance,
 *	Moods, Personal likes and dislikes.  Word association mode.
 *
 * 29-Nov-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Eleventh remedial release.
 *	Mods for DragonMud, especially visible exits.
 *
 * 25-Nov-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Eleventh remedial release.  Bug fixes, a few more responses.
 *
 * 17-Oct-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Tenth ceremonial release.  Mods for planck DB switching.
 *
 * 04-Apr-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Handle new WHO format, added checking to see if old rooms
 *	are still there, fix to can_reach, moved initial call to
 *	add_player(me), NULL deref problem in explore.c.
 *
 * 7-Mar-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Sixth experimental release. Changed file formats, added lots
 *	of info to room memory.  Found a memory allocation bug.
 *
 * 10-Feb-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Fifth special release.
 *
 * 04-Feb-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Fourth general release
 *
 * 25-Jan-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Third interim release (allow numeric IP addresses)
 *
 * 05-Jan-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Second General Release.
 *
 * 31-Dec-89  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created.
 ****************************************************************/

# define VERSION "fourteenth animal release of 12-Dec-93"

# define CONTEST_AGE 32216880
# define CONTEST_MEMTIME 3600 /* 1 hour in seconds */

/* Macros */
# define streq(A,B) (!strcmp ((A), (B)))
# define MATCH(D,P)	smatch ((D),(P),result)

/* Data structure for linked list of mail messages */
typedef struct msgstruct {
	char *text;
	long timestmp;
	struct msgstruct *next;
} MSGS;

/* Player data structure, tracks who we have seen and where */
typedef struct pstruct {
	char *name;		/* Character name */
	long number;		/* TinyMUD ID */
	int present, active;	/* temporary, in this room / logged in */
	long firstsaw;		/* Time first noticed this player */
	long lastsaw;		/* Time last present */
	long lastspoke;		/* Time last spoken to */
	long lastheard;		/* Time last spoke to us */
	long lastplace;		/* Last room number */
	long lastactive;	/* Time last active (from IDLE) */
	long lastgave;		/* Time last gave him pennies */
	long lastdona;		/* Time of his last donation */
	long dontotal;		/* His total donations */
	long lastkill;		/* Time last assaulted us */
	long lastoffend;	/* Time last did or said something offensive */
	long flags;		/* Various player flags */
	long lastheralded;	/* Last time we gave them the herald msg */
	long lastlook;		/* Time last looked at player */
	long user1;		/* User defined variable: friendly */
	long user2;		/* User defined variable */
	char *desc;		/* Players last description */
	char *carry;		/* Players last contents */
	char *dialog;		/* Last 512 characters of speech */
	char *email;		/* Email description from the phone booth */
	MSGS *msgs;		/* Pointer to circularly linked list of msgs */
} PLAYER;

# define PL_ASLEEP	0x1	/* player was asleep at lastsaw */
# define PL_JERK	0x2	/* we ignore players commands */
# define PL_REMEMBER	0x4	/* we trust this player especially */
# define PL_HAVEN	0x8	/* page to player failed */
# define PL_OLD		0x10	/* this player is to be weeded out */
# define PL_HERALDED	0x20	/* player has received current herald */
# define PL_HERMIT	0x40	/* player doesnt like talking to bots */
# define PL_WIZARD	0x80	/* play is a wizard/tinker/god */
# define PL_MALE	0x100	/* player has told us he is male */
# define PL_FEMALE	0x200	/* player has told us he is female */
# define PL_ANDRO	0x400	/* player has told us SH is androgenous */

# define PLAYER_SET(N,F)	(player[N].flags |= (F))
# define PLAYER_CLR(N,F)	(player[N].flags &= ~(F))
# define PLAYER_GET(N,F)	(player[N].flags & (F))


# define DIALOGSIZE 2048
# define SMABUF 64
# define MSGSIZ 512
# define TOKSIZ 1024
# define BIGBUF 4096 /* Must be bigger than DIALOGSIZE */

/* Exit data structure, exits are stored in a linked list */
typedef struct estruct {
	char	*dir;
	long	room;
	struct estruct *next;
} O_EXIT;

/* Rooms, room ID (if known), the name and description, and list of exits */
typedef struct mstruct {
	long	number, firstin, lastin, cntin, totalin;
	long	awakesum, sleepsum, msgsum, friendly;
	char	*name, *desc, *contents, *exlist;
	int	flags;
	O_EXIT	*exits;
} O_ROOM;

# define RM_REACH	0x01	/* True if room is reachable from home */
# define RM_SEARCHED	0x02	/* Temp flags for graph searching */
# define RM_EXPLORED	0x04	/* True if all room exits have been tried */
# define RM_PUBLIC	0x08	/* True if room is considered public */
# define RM_HAVEN	0x10	/* True if room is haven */
# define RM_NOISY	0x20	/* True if room is noisy */

# define ROOM_SET(N,F)	(room[N].flags |= (F))
# define ROOM_CLR(N,F)	(room[N].flags &= ~(F))
# define ROOM_GET(N,F)	(room[N].flags & (F))

# define UNK_EXIT_P(E)	((E)->room < 0)

# define NOISE 1
# define PEOPLE 2
# define SLEEPERS 3
# define FRIENDLY 4

typedef struct pathstruct {
  char *dir;
  int depth;
  long loc;
} PATH;

char *makestring(), *exitstring(), *find_path(), *time_dur(), *exact_dur();
char *getmud(), *explore_exit(), *hashf(), *lcstr(), *malloc();
char *exit_to(), *car(), *cdr(), *last(), *room_name(), *unexp_exit();
char *contentsstring(), *ralloc(), *makefixstring(), *strip_robot_name();
char *sindex(), *codeword(), *timeofday(), *othername(), *doodify();

int   onintr();

long  atol(), filelength(), ffilelength();
long  find_player(), close_player(), add_room(), look_up_player();
long  msg_count(), msg_total();
long  find_room(), close_room(), add_room(), unexplored_room(), leads_to();

# define NORMAL  0
# define MOVE    1
# define WHO     2
# define PAGE	 3
# define LOOK	 4
# define SCORE	 5
# define NUMBER  6
# define COMMAND 7
# define PLYLOOK  8

# define FIRST	0
# define RANDOM	1

# define MAXPEN 300
# define MINPEN 100

# define MINUTES 60
# define HOURS   3600
# define DAYS    86400

# define NEXTMOVE 0
# define LONGPATH 1
# define MEDIUMPATH 2
# define SHORTPATH 3
# define NEXTMOVEHOME 4
# define SHORTPATHHOME 5

# define M_UNKNOWN 0
# define M_ACTION 1
# define M_SPOKEN 2
# define M_WHISPER 3
# define M_PAGE 4
# define M_SHOUT 5

# define MTYPESTR "UASWP!"

# define max(A,B) ((A) > (B) ? (A) : (B))
# define min(A,B) ((A) < (B) ? (A) : (B))
# define abs(A)   ((A) >  0  ? (A) : -(A))
# define sgn(A)   ((A) == 0  ?  0  : ((A) > 0 ? 1 : -1))

extern long ispueblo; 
