/****************************************************************
 * vars.h: Data structures for TinyMUD Robots
 *
 * HISTORY
 * 05-Jan-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Second General Release.
 *
 * 31-Dec-89  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created.
 ****************************************************************/

extern	int   fmud, tmud;
extern	long   debug, testing, terse, usedesc, quiet;
extern	long   playing, dead, hanging, pgoal;
extern	long   speed, generous, exploring;
extern	long   realmsg;

/*---- Variables for recording memory usage ----*/

extern	long string_sp;
extern	long string_ct;
extern	long exit_sp;
extern	long exit_ct;
extern	long player_sp;
extern	long room_sp;
extern	long path_sp;
extern	long dialog_sp;
extern	long dialog_ct;

/* Prefixes and suffixes */
extern	char *opre;
extern	char *osuf;
extern	char outpre[];
extern	char outsuf[];
extern	char movpre[];
extern	char movsuf[];
extern	char loopre[];
extern	char loosuf[];
extern	char whopre[];
extern	char whosuf[];
extern	char scrpre[];
extern	char scrsuf[];
extern	char pagpre[];
extern	char pagsuf[];
extern	char numpre[];
extern	char numsuf[];
extern	char plypre[];
extern	char plysuf[];

/* Results from star matcher */
extern	char res1[], res2[], res3[], res4[];
extern	char res5[], res6[], res7[], res8[];
extern	char *result[];

extern	char room1[], room2[], room3[], room4[];
extern	char *roomstr[];

extern	char tmp1[], tmp2[], tmp3[], tmp4[];
extern	char *tmpstr[];

extern	O_ROOM *room;
extern	PATH   *path;
  
extern	long rooms, maxrooms, lastrm, exits;

extern	char *stexits[];

extern	long numexits;

extern	char typecmd[];

extern	PLAYER *player;
extern	long players, maxplayer;

extern	char *modestr[];

/*-------- Current statistics --------*/
extern  char mydesc[];
extern	long pennies;
extern  long gave;
extern	char killer[];
extern	char speaker[];
extern	char here[];
extern	char desc[];
extern	char contents[];
extern	long hererm;
extern	char home[];
extern	char homedesc[];
extern	long homerm;
extern	char move1[];
extern	char move2[];
extern	char pathto[];
extern	long pagedfrom;
extern	long pagedto;
extern	char pagedby[];
extern	long pagedat;
extern	long now;
extern	long mode;
extern	long speaktime;
extern	long me;
extern	long alone;
extern	long awake;
extern	long atdesk;
extern	long inmove;
extern	long confused;
extern	long inwsynch;
extern	long lastwsynch;
extern	long naked;
extern	long incontents;
extern	long checkfreq;
extern	long lastcheck;
extern	long newrooms;
extern	long newexits;
extern	long printedloc;
extern	long paging;
extern	long nextwait;
extern	long takingnotes;
extern	long meetingroom;
extern	long termwarn;
extern	long termtarg;
extern	long termat;
extern	char termloc[];
extern	char thief[];
extern	lastlock;
extern	char lastobj[];

extern	long reach_added;
extern	long reach_changed;

extern	long   male;
extern	char *myname;
extern	char *owner;
extern	char *whoami;
extern	char *mudhost;
extern	long mudport;
extern	char *mapfile;
extern	char *plyfile;
extern	jmp_buf start_state;
