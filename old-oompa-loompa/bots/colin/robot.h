/****************************************************************
 * robot.h: Data structures for TinyMUD Robots
 *
 * HISTORY
 * 1-Mar-90  Michael Mauldin (mlm) at Carnegie-Mellon University
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

# define VERSION "sixth experimental release of 6-Mar-90"

/* Macros */
# define streq(A,B) (!strcmp ((A), (B)))
# define MATCH(D,P)	smatch ((D),(P),result)

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
	long flags;		/* Future expansion */
	long lastlook;		/* Time last looked at player */
	long user1;		/* User defined variable */
	long user2;		/* User defined variable */
	char *desc;		/* Players last description */
	char *carry;		/* Players last contents */
	char *dialog;		/* Last 512 characters of speech */
	char *email;		/* Email description from the phone booth */
} PLAYER;

# define PL_ASLEEP	0x1	/* True if player was asleep at lastsaw */
# define PL_JERK	0x2	/* True if we ignore players commands */
# define PL_TRUST	0x4	/* True if we trust this player especially */

# define PLAYER_SET(N,F)	(player[N].flags |= (F))
# define PLAYER_CLR(N,F)	(player[N].flags &= ~(F))
# define PLAYER_GET(N,F)	(player[N].flags & (F))


# define DIALOGSIZE 512
# define SMABUF 64
# define MSGSIZ 512
# define TOKSIZ 1024
# define BIGBUF 4096

/* Exit data structure, exits are stored in a linked list */
typedef struct estruct {
	char	*dir;
	long	room;
	struct estruct *next;
} O_EXIT;

/* Rooms, room ID (if known), the name and description, and list of exits */
typedef struct mstruct {
	long	number, firstin, lastin, cntin, totalin;
	long	awakesum, sleepsum, msgsum;
	char	*name, *desc, *contents;
	int	flags;
	O_EXIT	*exits;
} O_ROOM;

# define RM_REACH	0x1	/* True if room is reachable from home */
# define RM_SEARCHED	0x2	/* Temp flags for graph searching */

# define ROOM_SET(N,F)	(room[N].flags |= (F))
# define ROOM_CLR(N,F)	(room[N].flags &= ~(F))
# define ROOM_GET(N,F)	(room[N].flags & (F))

typedef struct pathstruct {
  char *dir;
  int depth;
  long loc;
} PATH;

char *makestring (), *exitstring (), *find_path (), *time_dur ();
char *getmud (), *explore_exit (), *hashf (), *lcstr (), *malloc ();
char *exit_to (), *car(), *cdr(), *last(), *room_name(), *unexp_exit();
char *contentsstring(), *ralloc(), *makefixstring();
int   onintr ();
long  atol(), filelength(), ffilelength();

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
