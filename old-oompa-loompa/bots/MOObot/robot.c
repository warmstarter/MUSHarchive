/****************************************************************
 * robot.c: Common robot code for TinyMUD automata
 *
 * HISTORY
 * 22-Dec-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Mods for TinyMUCK, Brigadoon.
 *
 * 29-Nov-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Eleventh remedial release.
 *	Mods for DragonMud, especially visible exits.
 *
 * 19-Sep-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Ninth official release.
 *	Mods for TinyHELL, especially paged messages.
 *
 * 05-May-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Seventh sequential release.
 *	Mods for TinyHELL, especially paged messages.
 *
 * 27-Mar-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Handle new WHO format.
 *
 * 26-Feb-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Sixth experimental release. Changed file formats, added lots
 *	of info to room memory.  Found a memory allocation bug.
 *
 * 10-Feb-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Fifth special release (deal with WHO suffix/prefix mismatch
 *	between TinyMUD and TinyHELL)
 *
 * 09-Jan-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Fourth general release (save room contents, trap ignore)
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

# include <stdio.h>
# include <ctype.h>
# include <sys/types.h>
# include <sys/time.h>
# include <sys/ioctl.h>
# include <sys/socket.h>
# include <setjmp.h>
# include <netinet/in.h>
# include <netdb.h>
# include <ctype.h>
# include <varargs.h>

# include "robot.h"

# define  HUH1 "That is not a valid command."
# define  HUH2 "You're NOT _IN_ a PROGRAM!!!"

# define OUTPUTPREFIX	"<<norm"
# define OUTPUTSUFFIX	">>norm"
# define MOVEPREFIX	"<<move"
# define MOVESUFFIX	">>move"
# define LOOKPREFIX	"<<look"
# define LOOKSUFFIX	">>look"
# define SCRPREFIX	"<<score"
# define SCRSUFFIX	">>score"
# define PAGEPREFIX	"<<page"
# define PAGESUFFIX	">>page"
# define WHOPREFIX	"<<who"
# define WHOSUFFIX	">>who"
# define NUMBPREFIX	"<<numb"
# define NUMBSUFFIX	">>numb"
# define PLYPREFIX	"<<player"
# define PLYSUFFIX	">>player"

# define MINPEN 100
# define MAXPEN 300

# define READ		0
# define WRITE		1

# define abs(A)		((A)>0?(A):-(A))
# define max(A,B)	((A)>(B)?(A):(B))
# define sgn(A)		((A)==0?0:((A)>0?1:-1))


int	fmud, tmud;
int	flisp, tlisp;
long	debug=0, testing=0, terse=0, usedesc=0, quiet=1, debug_conv=0;
long	playing = 1, dead = 0, hanging = 0, pgoal = 0;
long	speed = 5, generous = 0, exploring = 0, vindictive = 0;
long	realmsg = 1, visitold = 1;


/*---- Variables for recording memory usage ----*/

long string_sp = 0;
long string_ct = 0;
long exit_sp = 0;
long exit_ct = 0;
long player_sp = 0;
long room_sp = 0;
long path_sp = 0;
long dialog_sp = 0;
long dialog_ct = 0;
long msgs_sp = 0;
long freed = 0;

/* Prefixes and suffixes */
char *opre = "PREFIX";
char *osuf = "SUFFIX";
char outpre[SMABUF];
char outsuf[SMABUF];
char movpre[SMABUF];
char movsuf[SMABUF];
char loopre[SMABUF];
char loosuf[SMABUF];
char whopre[SMABUF];
char whosuf[SMABUF];
char scrpre[SMABUF];
char scrsuf[SMABUF];
char pagpre[SMABUF];
char pagsuf[SMABUF];
char numpre[SMABUF];
char numsuf[SMABUF];
char plypre[SMABUF];
char plysuf[SMABUF];
char code[64] = "";
char *codeword(), *add_name();
long codeset = 0;

char plname[MSGSIZ] = "Judge";

/* Results from star matcher */
extern char res1[], res2[], res3[], res4[];
extern char res5[], res6[], res7[], res8[];
extern char *result[];

extern char room1[], room2[], room3[], room4[];
extern char *roomstr[];

extern char tmp1[], tmp2[], tmp3[], tmp4[];
extern char tmp5[], tmp6[], tmp7[], tmp8[];
extern char *tmpstr[];

O_ROOM *room = NULL;
PATH *path = NULL;
long rooms = 0, maxrooms = 0;
long lastrm = 0, exits = 0;

char *stexits[] = {
	"north", "south", "east", "west", "n", "s", "e", "w",
	"ne", "nw", "se", "sw", "nne", "nnw", "ene", "wnw",
	"sse", "ssw", "ese", "wsw",
	"nn", "ss", "ee", "ww", "nnn", "www", "eee", "sss",
	"in", "out", "leave", "back", "up", "down", "left", "right",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12",
	"open", "close", "rec", "cw", "ccw", "wait",
	"move around", "look around", "door", "arch",
	"port", "starboard", "fore", "aft", "sideways",
	NULL };

long numexits = (sizeof (stexits) / sizeof (*stexits)) - 1;

char *mname[] = {
  "January",
  "February",
  "March",
  "April",
  "May",
  "June",
  "July",
  "August",
  "September",
  "October",
  "November",
  "December" };


char typecmd[TOKSIZ];

PLAYER *player;
long players = 0, maxplayer = 0;

char glasthere[TOKSIZ] = "", glastdir[TOKSIZ] = "";
long glastrm = -1;

char *modestr[] =
{ "NORMAL", "MOVE", "WHO", "PAGE", "LOOK",
  "SCORE", "NUMBER", "COMMAND", "PLAYER" };

/*-------- Current statistics --------*/
char world[MSGSIZ] = "Druid";
long msgtype = M_ACTION;
long pagedmsgs = 0;
char mydesc[MSGSIZ];
long mlnum = -1;
long pennies = 0;
long objs = 0;
long gave = -1;
long msgstat = 0;		/* 0=no msg, 1=success, -1=failed */
long needpsynch = 0;
char killer[MSGSIZ] = "";
char speaker[MSGSIZ] = "";
char postcmd[BUFSIZ] = "";
char here[TOKSIZ] = "";
char desc[TOKSIZ] = "";
char contents[BIGBUF] = "";
char exlist[MSGSIZ] = "";
char herald[MSGSIZ] = "";
long heraldtime = 0;
long heraldpriority = 0;
long hererm = -1;
long hereid = -1;
long hereispl = 0;
long recrm = -1;
char home[TOKSIZ] = "";
char homedesc[TOKSIZ] = "";
long homerm = -1;
char move1[TOKSIZ] = "";
char move2[TOKSIZ] = "";
char move3[TOKSIZ] = "";
char move4[TOKSIZ] = "";
char pathto[TOKSIZ] = "";
long pagedfrom = -1;
long pagedto = -1;
char pagedby[MSGSIZ] = "";
long pagedat = 0;
long now = 0;
long mode = NORMAL;
long doing_works = 0;
long speaktime = 0;
long me = -1;
long alone = 0;
long awake = 0;
long atdesk = 0;
long inmove = 0;
long confused = 0;
long inwsynch = 0;
long lastwsynch = 0;
long naked = 0;
long incontents = 0;
long checkfreq = 60 * MINUTES;
long lastcheck = 0;
long newrooms = 0;
long newexits = 0;
long printedloc = 0;
long paging = 0;
long nextwait = 120;
long takingnotes = 0;
long playinghearts = 0;
long lastheartsplay = 0;
long first_turn = 1;
long meetingroom = -1;
long termwarn = 0;
long termtarg = -1;
long termat = 0;
char termloc[MSGSIZ] = "";
char thief[MSGSIZ] = "";
long lastlock = 0;
char lastobj[MSGSIZ] = "";

extern long contest_mode;
extern long tty_interface;


/*---- Variables for maintaining RM_REACH flags ----*/
long reach_added = 0;
long reach_changed = 0;

/*-------- Configuration Variables --------*/
long   male = 0;
char *myname = NULL;
long creation = 0;
char *owner = NULL;
char *whoami = NULL;
char *mudhost = NULL;
char *lisphost = NULL;
long mudport = 0;
char *mapfile = NULL;
char *plyfile = NULL;
long statuscmd = 0;
long pagecmd = 0;
long posecmd = 0;
long scorecmd = 0;
long ismuck = 0;
long trusting = 0;
long doecho = 1;
extern long fasttype;
jmp_buf start_state;


/****************************************************************
 * Main routine
 ****************************************************************/

# define SKIPARG		while (*++(*argv)); --(*argv)

main (argc, argv)
int   argc;
char *argv[];
{ char *pname = argv[0];
  char logname[256];
  struct tm *t;
  char *program = argv[0];

  if (streq (argv[0], "-contest"))
  { doecho = 0; }
  
  /* Get the options from the command line */
  while (--argc > 0 && (*++argv)[0] == '-')
  { while (*++(*argv))
    { switch (**argv)
      { 
	case 'H': mudhost = *argv+1; SKIPARG; break;
	case 'L': lisphost = *argv+1; SKIPARG; break;
	case 'P': mudport = atol (*argv+1); SKIPARG; break;
	case 'M': mapfile = *argv+1; SKIPARG; break;
	case 'F': plyfile = *argv+1; SKIPARG; break;
	case 'E': doecho = 0; break;
	case 'S': speed = atol (*argv+1); SKIPARG; break;
	case 'f': fasttype++; break;
	case 'c': contest_mode = 0; break;
	case 'C': tty_interface = 0; break;
	case 'T': testing++; break;
	case 'd': debug++; break;
	case 'n': quiet=0; break;
	case 'D': usedesc++; break;
	case 'e': exploring++; break;
	case 'g': generous++; break;
	case 'p': paging++; break;
	case 't': terse++; break;
	case 'v': vindictive++; break;
	case 'V': visitold = 0; break;

        default:  fprintf (stderr, "Usage: %s [ options ]\n\n", pname);
		  fprintf (stderr, "	-d		debug\n");
		  fprintf (stderr, "	-e		exploring\n");
		  fprintf (stderr, "	-g		generous\n");
		  fprintf (stderr, "	-p		paging\n");
		  fprintf (stderr, "	-t		terse\n");
		  fprintf (stderr, "	-v		vindictive\n");
		  fprintf (stderr, "	-V		no visit old\n");
		  fprintf (stderr, "	-D		use descriptions for exits\n");
		  fprintf (stderr, "	-C		contest mode on\n");
		  fprintf (stderr, "	-T		testing (connect to stdin/stdout)\n\n");

		  fprintf (stderr, "	-H'host'	Host to connect to game\n");
		  fprintf (stderr, "	-P<port>	Port number\n");
		  fprintf (stderr, "	-M'mapfile'	Map file name\n");
		  fprintf (stderr, "	-F'playfile'	File for player information\n");


		  exit (1);
      }
    }
  }

  /* If running directly on terminal, put stderr elsewhere */
  if (tty_interface)
  {
    /* Pick unique log file name */
    now = time (0);
    t = localtime (&now);
  
    sprintf (logname, "log.%02d%02d.%02d%02d",
	     t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min);
    if (access (logname, 0) == 0)
    { int ver = 'a';
      do
      { sprintf (logname, "log.%02d%02d.%02d%02d%c",
		 t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, ver++);
      } while (access (logname, 0) == 0);
    }
	  
    umask (0);
    freopen (logname, "a", stderr);
  }
  
  fprintf (stderr, "Running program %s\n", program);
  
  setconfig ();
  
  if (testing || tty_interface)
  { tmud = 1; fmud = 0; }
  else
  { tmud = fmud = connectmud (); }

  if (tmud < 0)
  { now = time (0);
    fprintf (stderr,"Connect failed at %15.15s\n", ctime (&now)+4);
    exit (-1);
  }

  /* For Loebner contest, open connection to Lisp process */
# ifdef DO_LISP
  if (contest_mode)
  { tlisp = flisp = connectlisp ();
    if (tty_interface)
    { printf ("[ %s ]\r\n",
	      (tlisp >= 0) ? "Lisp based talk program" :
			     "C based talk program...");
    }
  }
  else
# endif
  { tlisp = flisp = -1; }

  /* Start random generator */
  srand (time (0) + 40169 * getpid () + 58741 * getuid ());
  
  init_prefix ();
  lastcheck = now = time (0);

  robot ();
  now = time (0);
  fprintf (stderr, "\nReturned from Robot main program at %15.15s.\n",
	   ctime (&now) + 4);
  quit_robot ();
}


/****************************************************************
 * lsynch: Guarantee room direction by paging
 ****************************************************************/

lsynch ()
{
  if (debug) fprintf (stderr, "In lsynch()...\n");
  pennies--;
  sendmud ("%s %s\npage %s\n%s %s",
	   opre, pagpre, myname, opre, outpre);
  waitfor (outsuf);
}

/****************************************************************
 * psynch: Simple sync for stationary commands
 ****************************************************************/

psynch ()
{
  if (debug) fprintf (stderr, "In psynch()...\n");
  if (! ispueblo)
    { if (ismuck && scorecmd)
	{ sendmud ("%s %s\n#%d\n%s %s",
		   opre, scrpre, scorecmd, opre, outpre);
	}
    else
      { sendmud ("%s %s\nscore\n%s %s",
		 opre, scrpre, opre, outpre);
      }
      waitfor (outsuf);
      
      needpsynch = 0;
    }
}
/****************************************************************
 * usynch: Simple sync for number of objects
 ****************************************************************/

usynch ()
{
  if (debug) fprintf (stderr, "In usynch()...\n");
  sendmud ("%s %s\n@stats = 0\n%s %s",
	   opre, scrpre, opre, outpre);
  waitfor (outsuf);
}

/****************************************************************
 * msynch: Complex sync for moving commands commands
 ****************************************************************/

msynch ()
{
  if (debug) fprintf (stderr, "In msynch()...\n");
  if (ismuck && statuscmd)
  { sendmud ("%s %s\n#%d\n%s %s",
	     opre, loopre, statuscmd, opre, outpre);
  }
  else
  { sendmud ("%s %s\nlook\n%s %s",
	     opre, loopre, opre, outpre);
  }
  waitfor (outsuf);
}

/****************************************************************
 * wsynch: Complex sync for moving commands commands
 ****************************************************************/

wsynch ()
{
  if (debug) fprintf (stderr, "In wsynch()...\n");
  inwsynch++;
  sendmud ("%s %s\n%sWHO\n%s %s",
	   opre, whopre, streq (world, "HoloMuck") ? "!" : "", opre, outpre);
  waitfor (outsuf);
  inwsynch--;
  lastwsynch = now;
  if (me >= 0) player[me].active = player[me].present = 0;
}

/****************************************************************
 * waitfor:
 ****************************************************************/

waitfor (pat)
char *pat;
{ char *msg;
  long start = time (0);

  while (1)
  { if (msg = getmud ())
    { procmsg (msg);
      if (stlmatch (msg, pat)) return (1);
      if (streq (pat, "Welcome to ") && sindex (msg, "Mail sjade")) return(1);
      if (streq (pat, "Welcome to ") && sindex (msg, "> Welcome to")) return(1);

      /* Reset dead man timer */
      start = now = time (0);
    }
    else
    { 
      /* Check timer, if 10 minutes since last IO, timeout */
      if ((now = time (0)) > start + 10 * MINUTES)
      { fprintf (stderr,
		 "%s(%s) after %s, here '%s', %s '%s'(%d), lastdir '%s'\n",
		 "Quit: timed out in waitfor", pat, exact_dur (now-start),
		 here, "lasthere", glasthere, glastrm, glastdir);
	lost_connect (pat, now-start);
        quit_robot ();
      }

      sleep (1);
    }
  }
}

/****************************************************************
 * hangaround: Wait here a specified number of seconds, but
 * keep processing incoming messages;
 ****************************************************************/

hangaround (sec)
long sec;
{ char *msg;
  long alarm;
  long tick=0;

  if (*typecmd) sec = 1;

  alarm = time (0) + sec;
  hanging = 1;

  if (!terse && sec > speed)
  { fprintf (stderr, "Hang: %ld seconds\n", sec); }

  while (1)
  { while (msg = getmud ())
    { procmsg (msg); }

    if ((now = time (0)) >= alarm  || dead || !hanging) break;

    sleep (1);
    
    if (tick++ > 10) { do_msgs(); tick = 0; }
  }

  
  return (1);
}

/****************************************************************
 * quit_robot: We are exiting.  Write out any long term memory first.
 ****************************************************************/

quit_robot ()
{
  close (tmud);
  checkpoint ();
  now = time (0);
  fprintf (stderr, "Quit: %s quitting at %15.15s\n", myname, ctime (&now)+4);
  exit (0);
}

/****************************************************************
 * connectmud: Open the MUD socket
 ****************************************************************/

connectmud()
{
  struct sockaddr_in sin;
  struct hostent *hp;
  int     fd;

  if (debug)
  { fprintf (stderr, "Connecting to %s, port %ld...\n", mudhost, mudport); }

  bzero((char *) &sin, sizeof(sin));

  sin.sin_port = htons(mudport);

  /* Handle numeric or host name addresses */
  if (isdigit (*mudhost))
  { sin.sin_addr.s_addr = inet_addr (mudhost);
    sin.sin_family = AF_INET;
  }
  else
  { if ((hp = gethostbyname(mudhost)) == 0) return (-1);

    bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length);
    sin.sin_family = hp->h_addrtype;
  }

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) return -1;

  if (connect(fd,(struct sockaddr *) &sin, sizeof(sin)) < 0) return -1;

  return fd;
}

/****************************************************************
 * sendmud: Send a command to the TinyMUD process
 *
 * WARNING: possible 16/32 bit int problems here - fuzzy
 ****************************************************************/

sendmud (fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
char *fmt;
int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
{ int len;
  char buf[BIGBUF], *sindex();
  register char *s;
  static int hcnt = 0;
  
  if (!fmt) crash_robot ("Null fmt in sendmud");

  sprintf (buf, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  strcat (buf, "\n");
  len = strlen (buf);

  /* Log sends */
  if (debug)
  { fprintf (stderr, "\nSend: ");
    for (s=buf; *s; s++) fputc ((*s == '\n' ? '|' : *s), stderr);
    fprintf (stderr, "\n\n");
  }
  else if (!terse)
  { int comma = 0;

    fprintf (stderr, "Send: ");
    for (s=buf; *s; s++)
    { if (stlmatch (s, "OUTPUT"))	{ while (*s && s[0] != '\n') s++; }
      else if (*s == '\n')		{ comma++; }
      else				{ if (comma) fprintf (stderr, ", ");
					  comma=0;
					  fputc (*s, stderr);
					}
    }
    fprintf (stderr, "\n");
  }
  
  if (tty_interface)
  { fakeprint (buf); }
  else
  { if (write (tmud, buf, len) != len)
    { fprintf (stderr, "Write failed: %s", buf);
      quit_robot ();
    }
  }
  
  fflush (stderr);
}

/****************************************************************
 * command: Send a command to Tinymud and wait for the response
 *
 * WARNING: possible 16/32 bit int problems here - fuzzy
 ****************************************************************/

command (fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
char *fmt;
int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
{ static cmdlevel = 0;
  char buf[BIGBUF];

  sprintf (buf, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);

  if (debug || cmdlevel)
  { fprintf (stderr, "Cmnd: level %ld\n", ++cmdlevel); }
  
  sendmud ("%s", buf);
  if (!tty_interface) waitfor (outsuf);

  if (debug || cmdlevel > 1)
  { fprintf (stderr, "Ackn: level %ld\n", cmdlevel--); }

  fflush (stderr);
}

/****************************************************************
 * movemud: Send a movement command to the TinyMUD process
 ****************************************************************/

movemud (dirstr)
char *dirstr;
{ long len;
  char buf[BIGBUF], insert[BUFSIZ], lasthere[MSGSIZ], lastdesc[TOKSIZ];
  char dir[MSGSIZ];
  static char lbuf[128];
  long oldrm = -1, waspl = 0;
  long wenthome = 0;
  char *lcmd, *wstr = outsuf;
  static long entered = 0;
  register long pl;
  
  if (!isprint (*dirstr))
  { crash_robot ("Bogus direction in movemud, first char was %03o\n",
		 *dirstr);
  }

# ifdef MAP_NEWS  
  if (strfoldeq (world, "Time Traveller") &&
      strfoldeq (dirstr, "news"))
  { dirstr = "read";
    add_exit (hererm, dir, hererm);
  }
# endif

  strcpy (dir, dirstr);

  if (debug)
  { fprintf (stderr, "In movemud(%s), here is '%s'\n", dir, here); }

  /*---- Set post move command ----*/
  if (ismuck && statuscmd && streq (world, "HoloMuck"))
  { lcmd = sprintf (lbuf, "pgm10823"); }
  else if (ismuck && streq (world, "HoloMuck"))
  { lcmd = sprintf (lbuf, "#2735"); }
  else if (ismuck && statuscmd)
  { lcmd = sprintf (lbuf, "#%d", statuscmd); }
  else
  { lcmd = "look"; }
  
  /*---- Build an atomic prefix/suffix/move command ----*/
  
  /* Build a suffix command */
  if (*postcmd)
  { sprintf (insert, "%s\n%s\n%s\n", opre, osuf, postcmd); }
  else
  { strcpy (insert, ""); }
  
  if (ismuck)
  { if (streq (dir, "straighthome"))
    { sprintf (buf,
	      "%s\n%s\n%s\n%s %s\nstraighthome\n%s %s\n%s %s\n%s\n%s%s %s\n%s %s",
	       opre, osuf, randint (100) < 50 ? "@q" : "@q\n@q",
	       opre, movpre, osuf, movsuf, opre, loopre,
	       lcmd, insert,
	       opre, outpre, osuf, outsuf);
      wenthome++;
    }
    else
    { sprintf (buf,
	      "%s %s\ngo %s\n%s\n%s\n%s\n%s %s\n%s %s\n%s\n%s%s %s\n%s %s",
	       opre, movpre, dir,
	       opre, osuf, randint (100) < 50 ? "@q" : "@q\n@q",
	       osuf, movsuf, opre, loopre,
	       lcmd, insert,
	       opre, outpre, osuf, outsuf);
    }
  }
  else
  { if (streq (dir, "straighthome"))
    { sprintf (buf, "%s %s\nstraighthome\n%s %s\n%s %s\n%s\n%s%s %s\n%s %s",
	       opre, movpre, osuf, movsuf, opre, loopre,
	       lcmd, insert,
	       opre, outpre, osuf, outsuf);
      wenthome++;
    }
    else
    { sprintf (buf, "%s %s\ngo %s\n%s %s\n%s %s\n%s\n%s%s %s\n%s %s",
	       opre, movpre, dir, osuf, movsuf, opre, loopre,
	       lcmd, insert,
	       opre, outpre, osuf, outsuf);
    }
  }
  

  /* Actions upon leaving a room */
  before_move_hook ();

  /* Actually send the MOVE command */
  if (debug) fprintf (stderr, "Move: %s\n", dir);

  strcpy (lasthere, here);
  strcpy (glasthere, here);
  strcpy (glastdir, dir);
  strcpy (lastdesc, desc);
  oldrm = hererm;
  glastrm = hererm;
  waspl = hereispl;

  len = strlen (buf);

  strcpy (here, "");
  strcpy (desc, "");
  hereid = -1;

  inmove++;

  sendmud ("%s", buf);  
  waitfor (movsuf);

  while (*here == '\0')
  { int tries = 0;

    debug++;

    fprintf (stderr, "Here: After look %d, here '%s', desc '%s'\n",
	     ++tries, here, desc);

    if (!ismuck || tries >= 10)
    { crash_robot ("Can't find current room after %d tries.", tries); }

    sprintf (buf, "%s\n%s\n%s\n%s %s\n%s %s\n%s\n%s %s\n%s %s",
	     opre, osuf,
	     randint (100) < 50 ? "@q" : "@q\n@q",
	     osuf, movsuf, opre, loopre,
	     lcmd,
	     opre, outpre, osuf, outsuf);

    sendmud ("%s", buf);  
    waitfor (movsuf);
    
    debug--;
  }

  inmove--;
  
  confused = 0;

  /*---- Determine where we think we are, match MOVE and LOOK output ----*/

  if (ismuck)
  { /* Everything is okay */
    if (*here == '\0')
    { crash_robot ("After loop, could not exit MUF program.");
    }
    
  }

  /* If LOOK matches first line, everything is cool */
  else if (streq (move2, here))
  { if (debug) fprintf (stderr, "Okay: move matches look '%s'\n", move1); }

  /* If LOOK matches second line, everything is still cool */
  else if (streq (move3, here))
  { if (debug) fprintf (stderr, "Okay: had @su message '%s'\n", move1); }

  /* If LOOK matches third line, everything is still cool */
  else if (streq (move3, here))
  { if (debug) 
    { fprintf (stderr, "Okay: had @su messages '%s' & '%s'\n", move1, move2); }
  }

  /* If MOVE1 is fail message, then we are probably in the same place */
  else if (streq (move1, "You can't go that way.") || 
           streq (move1, "You're already home!") ||
           streq (move1, HUH1))
  { if (debug)
    { fprintf (stderr, "Okay: didn't move, got error '%s'\n", move1); }
  }

  /* If here is eq to LASTHERE, we are in the same place */
  else if (streq (here, lasthere))
  { if (debug)
    { fprintf (stderr, "Okay: didn't move, got error '%s'\n", move1); }
  }

  /* Trouble: LOOK and MOVE gave different messages */
  else
  { /* Page myself to find out where I really am */
    if (pennies > 10)
    { lsynch ();
      fprintf (stderr, "Conf: Lsynch returns '%s'\n", here);
    }
    
    fprintf (stderr, "Conf: confusing room, don't believe exit\n");
    confused++;
  }
  
  /* Handle confusing rooms by cowardice */
  if (confused)
  { if (wenthome)
    { fprintf (stderr, "Conf: confused after going home, quitting.\n");
      quit_robot ();
    }
    else if (!dead)
    { 
      /* Must mark exit as used, to avoid infinite loops */
      fprintf (stderr, "Conf: erasing exit '%s' from room %s(%d)\n",
	       dir, room_name (oldrm), oldrm);
      add_exit (oldrm, dir, oldrm);
    }
    else
    { 
      /* Must mark exit as used, to avoid infinite loops */
      fprintf (stderr, "Dead: would have erased exit '%s' from room %s(%d)\n",
	       dir, room_name (oldrm), oldrm);
    }

    *lasthere = '\0'; hererm = -1; *here = '\0';

    if (!dead)
    { fprintf (stderr, "Conf: going home from '%s'(%ld)\n", here, hererm);
      movemud ("home");
    }
    return;
  }

  /* Set current location: special for rec room */
  if (streq (world, "TinyMUD Classic") &&
      oldrm == homerm && recrm >= 0 && streq (dir, "rec"))
  { long roomnum;

    hererm = recrm;

    /* Strip off room number from name */
    if (smatch (here, "* (#*)", roomstr) && isdigit (*room2))
    { roomnum = atoi (room2); }
    else if (smatch (here, "*(#*)", roomstr) && isdigit (*room2))
    { roomnum = atoi (room2); }
    else
    { strcpy (room1, here); roomnum = -1; }

    /* Check for name change */
    if (!streq (room[hererm].name, room1))
    { fprintf (stderr, "Warn: Changing room name '%s' to '%s' (%ld)\n",
		room[hererm].name, room1, hererm);
      freestring (room[hererm].name);
      room[hererm].name = makestring (room1);
    }

    /* Check for desc change */
    if (desc && (!room[hererm].desc || !streq (room[hererm].desc, desc)))
    { fprintf (stderr,
	    "Warn: Changing room desc for %s from '%s' to '%s' (%ld)\n",
		room[hererm].name, room[hererm].desc, desc, hererm);
      freestring (room[hererm].desc);
      room[hererm].desc = makestring (desc);
    }

    /* Check for number change */    
    if (room[hererm].number < 0 && roomnum >= 0) room[hererm].number = roomnum;
  }
  else
  { hererm = add_room (here, desc); }
  
  /* Cant add new room? Crash */
  if (hererm < 0)
  { crash_robot ("Bad room returned from add_room(\"%s\", \"%s\") -> %d\n",
		 *here ? here : "(null)", *desc ? desc : "(null)", hererm);
  }

  /* Now set room times */
  room[hererm].lastin = now = time (0);

  /* If we think we found a penny, must do a psynch */
  if (needpsynch) psynch ();

  /* No matter what we did, add the exit to our map */
  if (!confused && *here && *lasthere && !wenthome)
  { if (hereispl)
    { fprintf (stderr,
	       "Warn: ignoring exit '%s' from %s(%d) to player %s(%d)\n",
	       dir, room_name (oldrm), oldrm, room_name (hererm), hererm);
      add_exit (oldrm, dir, oldrm);
    }
    else if (waspl)
    { fprintf (stderr,
	       "Warn: ignoring exit '%s' from player %s(%d) to %s(%d)\n",
	       dir, room_name (oldrm), oldrm, room_name (hererm), hererm);
      add_exit (oldrm, dir, oldrm);
    }
    else
    { add_exit (oldrm, dir, hererm); }
  }

  /* Handle home room changing somehow */
  if (wenthome && hererm != homerm)
  { if (homerm >= 0)
    { fprintf (stderr, "Home: uh oh... home went to '%s'(%ld), not '%s'(%ld)\n",
	       here, hererm, home, homerm);
    }
    strcpy (home, here);
    strcpy (homedesc, desc);
    homerm = hererm;
  }
  
  /* Set room contents */
  if (*contents)
  { if (room[hererm].contents)
    { freestring (room[hererm].contents); }

    room[hererm].contents = makestring (contents);
  }
  else if (room[hererm].contents)
  { freestring (room[hererm].contents);
    room[hererm].contents = NULL;
  }
  
  /* Set room exits */
  if (*exlist)
  { if (room[hererm].exlist)
    { freestring (room[hererm].exlist); }

    room[hererm].exlist = makestring (exlist);
  }
  else if (room[hererm].exlist)
  { freestring (room[hererm].exlist);
    room[hererm].exlist = NULL;
  }
  
  /* If we went home, call home_hook */
  if (wenthome) home_hook ();

  /* If we queued any messages during the move, process them */
  procmsg (NULL);

  /* Actions after sending a move command (may still be in old room) */
  after_move_hook ();

  /* If we ARE in a new room, perform new room actions */
  if (hererm != oldrm)
  { printedloc = 0;

# ifdef VERBOSE_TERSE
    if (terse)
    { fprintf (stderr, "Move: <%s> to %s(%ld)\n",
	       dir, room_name (hererm), hererm);
    }
# endif

    /* Track total visits to this room, total time in old room */
    if (hererm >= 0) room[hererm].cntin++;
    if (entered && oldrm >= 0)
    { room[oldrm].totalin += (now - entered); }
    entered = now;
    
    /* Count players present */
    player[me].active = player[me].present = 0;
    for (pl = 0, alone = 0; pl<players; pl++)
    { if (player[pl].present)
      { if (player[pl].active)
        { alone++;
	  room[hererm].awakesum++;
	}
	else
	{ room[hererm].sleepsum++; }
      }
    }
    
    new_room_hook ();
  }
}

/****************************************************************
 * reply: Send and echo a command
 *
 * WARNING: possible 16/32 bit int problems here - fuzzy
 ****************************************************************/

reply (fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
char *fmt;
int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
{ int len;
  long pl;
  char buf[BIGBUF], cmd[BIGBUF];
  register char *s;
  char *ntype = "Rply";
  
  /* Log our reply */
  if (!printedloc) print_locn ();

  sprintf (buf, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);

  s = (*buf == '"') ? buf+1 : buf;

  if (*speaker && (pl = find_player (speaker)) >= 0 &&
      (now - player[pl].firstsaw) < 3 * DAYS)
  { ntype = "NewR"; }

  if (*speaker && (pl = find_player (speaker)) >= 0 &&
      sindex (buf, "{"))
  { strcpy (buf, add_name (buf)); }

  if (terse && msgtype > M_SPOKEN && *speaker)
  { fprintf (stderr, "%s: [%c to %s] %s\n",
	     ntype, MTYPESTR[msgtype], speaker, s); }
  else if (terse)
  { fprintf (stderr, "%s: %s\n", ntype, s); }

  /* Various commands */
  if (*buf == '"')
  { switch (msgtype)
    { 
      /* Basic case:	say the command using '"' */
      case M_UNKNOWN:
      case M_ACTION:
      case M_SPOKEN:	send_say (buf+1); break;
      
      /* Whisper:	whisper back */
      case M_WHISPER:	if (*speaker) send_whisper (speaker, buf+1);
			break;
      
      /* Page, shout:	page a message back (1.5.4 & later) */
      case M_PAGE:      
      case M_SHOUT:	if (*speaker) send_page (speaker, buf+1); break;
    }
  }

  else if (*buf == ':' || *buf == '|')
  { switch (msgtype)
    { 
      /* Basic case:	perform the action using ':' */
      case M_UNKNOWN:
      case M_ACTION:
      case M_SPOKEN:	send_pose (buf+1);
			break;

      /* Whisper:	whisper back */
      case M_WHISPER:	if (index (buf+1, ':') || !isalpha (buf[1]) ||
			    *buf == '|')
			{ if (*speaker) send_whisper (speaker, buf+1); }
			else
			{ sprintf (cmd, "%s %s", myname, buf+1);
			  if (*speaker) send_whisper (speaker, cmd);
			}
			break;
      
      /* Page, shout:	page a message back (1.5.4 & later) */
      case M_PAGE:      
      case M_SHOUT:	if (index (buf+1, ':') || !isalpha (buf[1]) ||
			    *buf == '|')
			{ if (!buf[1])
			  { if (*speaker) send_page (speaker, "."); }
			  else
			  { if (*speaker) send_page (speaker, buf+1); }
			}
			else
			{ sprintf (cmd, "%s %s", myname, buf+1);
			  if (*speaker) send_page (speaker, cmd);
			}
			break;
    }
  }

  /* Not a say/pose/whisper/page */
  else if (*buf != '"' && *buf != ':')
  { command ("%s", buf); }
}

/****************************************************************
 * unlogged: Like reply, but dont log the command
 *
 * WARNING: possible 16/32 bit int problems here - fuzzy
 ****************************************************************/

unlogged (fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
char *fmt;
int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
{ int len;
  char buf[BIGBUF], cmd[BIGBUF];
  register char *s;
  long pl;
  
  sprintf (buf, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);

  s = (*buf == '"') ? buf+1 : buf;

  /* Various commands */
  if (*buf == '"')
  {
    if (*speaker && (pl = find_player (speaker)) >= 0 &&
        sindex (buf, "{"))
    { strcpy (buf, add_name (buf)); }

    switch (msgtype)
    { 
      /* Basic case:	say the command using '"' */
      case M_UNKNOWN:
      case M_ACTION:
      case M_SPOKEN:	send_say (buf+1); break;
      
      /* Whisper:	whisper back */
      case M_WHISPER:	if (*speaker) send_whisper (speaker, buf+1);
			break;
      
      /* Page, shout:	page a message back (1.5.4 & later) */
      case M_PAGE:      
      case M_SHOUT:	if (*speaker) send_page (speaker, buf+1); break;
    }
  }

  else if (*buf == ':'  || *buf == '|')
  { switch (msgtype)
    { 
      /* Basic case:	perform the action using ':' */
      case M_UNKNOWN:
      case M_ACTION:
      case M_SPOKEN:	send_pose (buf+1); break;

      /* Whisper:	whisper back */
      case M_WHISPER:	if (index (buf+1, ':') || !isalpha (buf[1]))
			{ if (*speaker) send_whisper (speaker, buf+1); }
			else
			{ sprintf (cmd, "%s %s", myname, buf+1);
			  if (*speaker) send_whisper (speaker, cmd);
			}
			break;
      
      /* Page, shout:	page a message back (1.5.4 & later) */
      case M_PAGE:      
      case M_SHOUT:	if (index (buf+1, ':') || !isalpha (buf[1]))
			{ if (!buf[1])
			  { if (*speaker) send_page (speaker, "."); }
			  else
			  { if (*speaker) send_page (speaker, buf+1); }
			}
			else
			{ sprintf (cmd, "%s %s", myname, buf+1);
			  if (*speaker) send_page (speaker, cmd);
			}
			break;
    }
  }

  /* Not a say/pose/whisper/page */
  else if (*buf != '"' && *buf != ':')
  { command ("%s", buf); }
}

/****************************************************************
 * zinger: Same as reply, but different log
 *
 * WARNING: possible 16/32 bit int problems here - fuzzy
 ****************************************************************/

zinger (fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
char *fmt;
int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
{ int len;
  char buf[BIGBUF], cmd[BIGBUF];
  register char *s;
  long pl;
  char *ntype = "Zing";
  
  /* Log our reply */
  if (!printedloc) print_locn ();

  sprintf (buf, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);

  s = (*buf == '"') ? buf+1 : buf;

  if (*speaker && (pl = find_player (speaker)) >= 0 &&
      sindex (buf, "{"))
  { strcpy (buf, add_name (buf)); }

  if (*speaker && (pl = find_player (speaker)) >= 0 &&
      (now - player[pl].firstsaw) < 3 * DAYS)
  { ntype = "NewZ"; }


  if (terse && msgtype > M_SPOKEN && *speaker)
  { fprintf (stderr, "%s: [%c to %s] %s\n",
	     ntype, MTYPESTR[msgtype], speaker, s);
  }
  else if (terse)
  { fprintf (stderr, "%s: %s\n", ntype, s); }

  /* Various commands */
  if (*buf == '"')
  { switch (msgtype)
    { 
      /* Basic case:	say the command using '"' */
      case M_UNKNOWN:
      case M_ACTION:
      case M_SPOKEN:	send_say (buf+1); break;
      
      /* Whisper:	whisper back */
      case M_WHISPER:	if (*speaker) send_whisper (speaker, buf+1);
			break;
      
      /* Page, shout:	page a message back (1.5.4 & later) */
      case M_PAGE:      
      case M_SHOUT:	if (*speaker) send_page (speaker, buf+1); break;
    }
  }
  else if (*buf == ':'  || *buf == '|')
  { switch (msgtype)
    { 
      /* Basic case:	perform the action using ':' */
      case M_UNKNOWN:
      case M_ACTION:
      case M_SPOKEN:	send_pose (buf+1); break;

      /* Whisper:	whisper back */
      case M_WHISPER:	if (index (buf+1, ':') || !isalpha (buf[1]))
			{ if (*speaker) send_whisper (speaker, buf+1); }
			else
			{ sprintf (cmd, "%s %s", myname, buf+1);
			  if (*speaker) send_whisper (speaker, cmd);
			}
			break;
      
      /* Page, shout:	page a message back (1.5.4 & later) */
      case M_PAGE:      
      case M_SHOUT:	if (index (buf+1, ':') || !isalpha (buf[1]))
			{ if (!buf[1])
			  { if (*speaker) send_page (speaker, "."); }
			  else
			  { if (*speaker) send_page (speaker, buf+1); }
			}
			else
			{ sprintf (cmd, "%s %s", myname, buf+1);
			  if (*speaker) send_page (speaker, cmd);
			}
			break;
    }
  }

  /* Not a say/pose/whisper/page */
  else if (*buf != '"' && *buf != ':')
  { command ("%s", buf); }
}

/****************************************************************
 * add_name: find special name insertion commands, replace with
 * speakers name
 ****************************************************************/

char *add_name (str)
char *str;
{ static char buf[BIGBUF];
  register char *s = str, *t = buf;

  while (*s)
  { if (*s == '{')
    {
      /*
       * If not a contest, and speaking out loud, and more than
       * one other player, include recipients name
       */

      if (!contest_mode && msgtype == M_SPOKEN && alone > 1 && *speaker)
      { if (debug)
        { fprintf (stderr,
		   "Name: adding name %s, alone %d\n", speaker, alone);
	}

        for (s++; *s && *s != '}'; s++)
        { switch (*s)
	  { case 'n':	strcpy (t, speaker);
			t += strlen (speaker);

	    case '{': case '}': break;

	    default:	*t++ = *s;
	  }
	}
      }
      else if (debug)
      { fprintf (stderr, "Name: skipping name %s, alone %d\n",
		 speaker, alone);
      }
      
      /* Now skip over command */
      while (*s && *s != '}') s++;
      s++;
    }
    
    else
    { *t++  = *s++; }
  }

  *t = '\0';
  
  return (s = buf);
}



/****************************************************************
 * getmud: Read one line from TinyMUD
 ****************************************************************/

char *getmud ()
{ long len;
  static long havecnt = 0;
  static char buf[BUFSIZ], rbuf[4], plybuf[MSGSIZ];
  char junk[MSGSIZ], pname[MSGSIZ];
  long pnumber = -1;
  register char *s=buf, *tail=buf+MSGSIZ-1;
  static long pl;

  if (tty_interface)
  { crash_robot ("Error, called getmud during tty_interface mode"); }

  /* No input waiting */
  if (!charsavail (fmud)) return (NULL);

  /* Read one line, save printing chars only */
  while ((len = read (fmud, rbuf, 1)) > 0)
  { if (*rbuf == '\n')			break;
    if (isprint (*rbuf) && s < tail)	*s++ = *rbuf;
  }
  *s = '\0';

  /* Check for error */  
  if (len < 0)
  { fprintf (stderr, "Error %ld reading from mud\n", len);
    quit_robot ();
  }

  /* Detect World Name */
  if (!awake &&
      (MATCH (buf, "*Welcome to *,*") ||
       MATCH (buf, "*Welcome to *!") ||
       MATCH (buf, "*Welcome to *.") ||
       MATCH (buf, "*Welcome to *") ||
       MATCH (buf, "DruidMuck:*") && strcpy (res2, "DruidMuck") ||
       MATCH (buf, "*Mail sjade*") && strcpy (res2, "HoloMuck")) &&
      !sindex (lcstr (res2), "port concentrator"))
  { strcpy (world, res2);
    now = time (0);
    fprintf (stderr, "\n-------- %s: %s log starts at %15.15s --------\n\n",
	     world, myname, ctime (&now) + 4);

  }
  else if (!awake && debug)
  { fprintf (stderr, "Smsg: %s\n", buf); }


  /*-------- Number output lines --------*/
  if      (streq (buf, outpre))	{ mlnum = havecnt = 0; mode = COMMAND;}
  else if (streq (buf, outsuf))	{ mlnum = -1; mode = NORMAL; }
  else if (streq (buf, loopre))	{ mlnum = havecnt = 0; mode = LOOK; }
  else if (streq (buf, movpre))	{ mlnum = havecnt = 0; mode = MOVE; }
  else if (streq (buf, numpre))	{ mlnum = havecnt = 0; mode = NUMBER; }
  else if (streq (buf, pagpre))	{ mlnum = havecnt = 0; mode = PAGE; }
  else if (streq (buf, scrpre))	{ mlnum = havecnt = 0; mode = SCORE; }
  else if (streq (buf, whopre))	{ mlnum = havecnt = 0; mode = WHO; }
  else if (stlmatch (buf, plypre)) { mlnum = havecnt = 0; mode = PLYLOOK; }
  else if (streq (buf, loosuf))	{ mlnum = -1; mode = NORMAL; }
  else if (streq (buf, movsuf))	{ mlnum = -1; mode = NORMAL; }
  else if (streq (buf, numsuf))	{ mlnum = -1; mode = NORMAL; }
  else if (streq (buf, pagsuf))	{ mlnum = -1; mode = NORMAL; }
  else if (streq (buf, scrsuf))	{ mlnum = -1; mode = NORMAL; }
  else if (streq (buf, whosuf))	{ mlnum = -1; mode = NORMAL; }
  else if (streq (buf, plysuf))	{ mlnum = -1; mode = NORMAL; }

  else if (mlnum == 0 && mode == MOVE &&
	   (stlmatch (buf, "There's no place like home...")))
  { }
  else if (mlnum >= 0)
  { mlnum++; }

  /*-------- Debugging: log input --------*/
  if (debug)
  { if (mlnum >= 0)
    { fprintf (stderr, "%s%03d: %s\n",
	       ((mode == NORMAL) ? "N" :
		(mode == NUMBER) ? "I" :
		(mode == COMMAND) ? "C" :
		(mode == LOOK) ? "L" :
		(mode == PLYLOOK) ? "V" :
		(mode == SCORE) ? "S" :
		(mode == PAGE) ? "P" :
		(mode == MOVE) ? "M" : "W"),
		mlnum, buf);
    }
    else
    { fprintf (stderr, "Mud:  %s\n", buf); }
  }
  
  /*-------- Certain parts of the input are parsed here --------*/

  /*---- Handle implicit look (and @os msg) from movement ----*/
  if (mode == MOVE)
  { if (mlnum == 1)
    { strcpy (move1, buf); *move2 = *move3 = *move4 = '\0'; }
    else if (mlnum == 2)
    { strcpy (move2, buf); }
    else if (mlnum == 3)
    { strcpy (move3, buf); }
    else if (mlnum == 4)
    { strcpy (move4, buf); }

    if (mlnum > 1 && streq (buf, "You found a penny!"))
    { needpsynch++; }
    else if (mlnum > 1 && streq (buf, "You found a cookie!"))
    { needpsynch++; }
  }
  
  /*---- Handle output from looking at a room (using "look") ----*/
  else if (mode == LOOK)
  { if (ismuck && statuscmd)
    { if (*buf == '=')
      { if (stlmatch (buf, "=numb="))
	{ hereid = atoi (buf+6); }
        else if (stlmatch (buf, "=ispl="))
	{ hereispl = atoi (buf+6); }
        else if (stlmatch (buf, "=name="))
	{ strcpy (here, buf+6); }
        else if (stlmatch (buf, "=penn="))
	{ pennies = atoi (buf+6); }
        else if (stlmatch (buf, "=desc="))
	{ if (buf[6] == '@' && isdigit (buf[7]))
	  { if (streq (here, move1))
	    { strcpy (desc, move2);
	      if (!terse)
	      { fprintf (stderr, "Look: using move2 for desc '%s'\n", move2); }
	    }
	    else if (streq (here, move2))
	    { strcpy (desc, move3);
	      if (!terse)
	      { fprintf (stderr, "Look: using move3 for desc '%s'\n", move3); }
	    }
	    else if (streq (here, move3))
	    { strcpy (desc, move4);
	      if (!terse)
	      { fprintf (stderr, "Look: using move4 for desc '%s'\n", move4); }
	    }
	    else
	    { strcpy (desc, buf+6);
	      if (debug)
	      { fprintf (stderr,
		         "Look: using real desc '%s', can't match MOVE/LOOK\n",
			 desc);
		fprintf (stderr, "Look:\tname: %s\n\tmove1: %s\n\tmove2: %s\n\tmove3: %s\n\tmove4: %s\n",
			 here, move1, move2, move3, move4);
	      }
	    }
	  }
	  else
	  { strcpy (desc, buf+6); }

	  /* Now add the room number */
	  if (hereid >= 0) sprintf (here, "%s(#%d)", here, hereid);
	}

	else if (streq (buf, "=contents="))
	{ *contents = '\0'; havecnt++; }
	else if (streq (buf, "=players="))
	{ clear_present (); }
	else if (sscanf (buf, "=plyr=%d=%[^\n]", &pnumber, pname))
	{ player[add_player (pname)].number = pnumber;
	  saw_player (pname, here, desc);
	  if (debug) fprintf (stderr, "Plyr: %s[%d]\n", pname, pnumber);
	}
	else if (sscanf (buf, "=thng=%[^\n]", pname))
	{ if ((strlen (contents) + strlen (pname) + 2) < BIGBUF)
	  { if (*contents) strcat (contents, ", ");
	    strcat (contents, pname);
          }
	}
	else
	{ fprintf (stderr, "Look: unknown '=' message '%s'\n", buf); }
      }
      else if (*buf != '<' && !streq (buf, HUH1) && !streq (buf, HUH2))
      { fprintf (stderr, "Look: unexpected response '%s', here '%s'\n",
		 buf, *here ? here : "(unknown)");
      }
    }
    else if (!havecnt)
/* VJ: For some obscure reason, Druid and Pueblo print out a blank line first.*/
    { switch (mlnum)
      { case 2:	strcpy (here, buf);
		if (debug)
		{ fprintf (stderr, "Here: setting here to line2 '%s'\n", buf);}
		*desc = '\0';
		*contents = '\0';
		*exlist = '\0';
		clear_present ();
		break;

        case 3:	if (streq (buf, "Contents:"))
		{ havecnt++; }
		else if (stlmatch (buf, "Obvious exits: "))
		{ strcpy (exlist, buf+7);
		  if (debug)
		  { fprintf (stderr,
			    "Here: setting exits to line3 '%s'\n", exlist);
		  }
		}
		else
		{ strcpy (desc, buf);
		  if (debug)
		  { fprintf (stderr,
			    "Here: setting desc to line3 '%s'\n", buf);
		  }
		}
		break;
		
        default: if (streq (buf, "Contents:"))
		{ havecnt++; }
		else if (!havecnt && stlmatch (buf, "Obvious exits:"))
		{ strcpy (exlist, buf);
		  if (debug)
		  { fprintf (stderr,
			    "Here: setting exits to '%s'\n", exlist);
		  }
		}
		break;
      }
    }
    
    else /* Strip object numbers from things */
    { if (!(smatch (buf, "* (#*)", roomstr) && isdigit (*room2) ||
	    smatch (buf, "*(#*)", roomstr) && isdigit (*room2)))
      { strcpy (room1, buf); }

      if (find_player (room1) >= 0)
      { saw_player (room1, here, desc); }
      else
      { if (debug) fprintf (stderr, "Obj:  %s\n", room1);
	if ((strlen (contents) + strlen (room1) + 2) < BIGBUF)
	{ if (*contents) strcat (contents, ", ");
	  strcat (contents, room1);
        }
      }
    }
  }

  /*---- Handle output from 'find player id' ----*/
  else if (mode == NUMBER)
  {
# ifdef OLD_EXAMINE
    if (streq (buf, "I can't find that key!"))
    { lastlock = -1; }
    else if (lastlock == 0 && smatch (buf, "*(#*)*Key:  *(#*)*", tmpstr))
    { lastlock = atol (tmp5);
      strcpy (lastobj, tmp4);
    }
# endif
  }

  /*---- Handle output from look_at_thing ----*/
  else if (mode == PLYLOOK)
  {
    /* Line 0 contains the name of the player being viewed */
    if (mlnum == 0)
    { if (stlmatch (buf, plypre) &&
	  (strlen (buf) > strlen (plypre) + 1) &&
	  strcpy (plybuf, buf + strlen (plypre) + 1) &&
	  (pl = find_player (plybuf)) >= 0)
      { /* have a valid look at player prefix */
	if (player[pl].carry) strcpy (player[pl].carry, "");
      }
      else
      { pl = -1; *plybuf = 0; }

      if (debug) fprintf (stderr, "Look: got output '%s', player '%s'(%ld)\n",
			  buf, plybuf, pl);
    }

    /* Line one is players description */
    else if (mlnum == 1 && pl >= 0)
    { if (*buf &&
	  !streq (buf, "I don't see that here.") &&
	  !streq (buf, "I don't know which one you mean!") &&
	  !streq (buf, "You see nothing special."))
      { if (player[pl].desc == NULL || !streq (player[pl].desc, buf))
        { freestring (player[pl].desc);
	  player[pl].desc = (*buf ? makestring (buf) : NULL);
	  
	  fprintf (stderr, "Look: player '%s'(%ld), desc: %s\n",
		   plybuf, pl, buf);
        }
	else if (!terse)
	{ fprintf (stderr, "Look: player '%s'(%ld), old desc: %-32.32s\n",
		   plybuf, pl, buf);
	}
      }
      else if (!terse)
      { fprintf (stderr, "Ignoring look output for %s: '%s'\n", plybuf, buf);
      }
    }

    /* Lines 3 and following are players inventory */
    else if ((mlnum > 2) && pl >= 0)
    {
      if (player[pl].carry == NULL)
      { player[pl].carry = makefixstring ("", DIALOGSIZE); }

      if (player[pl].carry[0])
      { if ((strlen (buf) + strlen (player[pl].carry) + 2) < DIALOGSIZE)
        { strcat (player[pl].carry, ", ");
	  strcat (player[pl].carry, buf);
	}
      }
      else
      { strncpy (player[pl].carry, buf, DIALOGSIZE);
        player[pl].carry[DIALOGSIZE-1] = '\0';
      }
    }
    
  }

  /*-------- Now return message --------*/
  return (s = buf);
}

/****************************************************************
 * procmsg: send messages to the robots readmsg function
 ****************************************************************/

# define MAXQ 256

int dont_drop = 1;


procmsg (msg)
char *msg;
{ static long msgrm = -1, dropped = 0, inproc = 0;
  static msgqueue[MAXQ][MSGSIZ];
  static int i=0;


  /* debugging */
  if (inproc && !terse)
  { fprintf (stderr, "Proc: entering procmsg %d, msg: %s\n", inproc, msg);
    /* debug++; */
  }

  /*---- Queue messages that happen "between rooms" ----*/

  if (inmove && msg)
  { 
    /* Ignore output fropm @Q command in MUCKs */
    if (streq (msg, HUH1) || streq (msg, HUH2))
    { /* Ignore */
      return;
    }

    /* Special case for WHO mode output */
    /* VJ Sun Oct 15 06:53:30 1995 Set up to match output on Druid and Pueblo.*/
   /* Note that newt does have his who-options set. */ 
    if (MATCH (msg, "* * seconds"))
       { if (debug) fprintf (stderr, "Who:  [%s] %s seconds\n", res1, res2);
         idle_player (res1, atol (res2));
         return;
       }
    else 
      { if (MATCH (msg, "* * minutes"))
         { if (debug) fprintf (stderr, "Who:  [%s] %s minutes\n", res1, res2);
           idle_player (res1, atol (res2) * 60);
           return;
          }
        else 
	  { if (MATCH (msg, "* * hours"))
	      { if (debug) fprintf (stderr, "Who:  [%s] %s hours\n", res1, res2);
		idle_player (res1, atol (res2) * 60 * 60);
		return;
	      }
            else 
	      { if (MATCH (msg, "* * days"))
	      { if (debug) fprintf (stderr, "Who:  [%s] %s days\n", res1, res2);
		    idle_player (res1, atol (res2) * 60 * 60 * 24);
		    return;
		  }
	      }
	  }
      }

    /* Special case for Hearts page */
    if (mode == NORMAL && MATCH (msg, "(> *There is a Hearts game starting*"))
    { hearts_msg (raw_car (msg), msg, lcstr (msg));
      return;
    }

    /* Only queue NORMAL mode messages */
    if (mode == NORMAL)
    { if (stlmatch (msg, ">>")) return;
    
      if (inproc)
      { fprintf (stderr,
		 "Ignr: recursive proc, dropping '%s' from rm %s(%ld)\n",
		 msg, room_name (msgrm), msgrm);
      }
      
      else
      { if (msgrm != hererm) { dropped = 0; msgrm = hererm; }

	/* If too many messages to queue, at least send to log */
	if (dropped >= MAXQ)
	{ if (!stlmatch (msg, "I don't see that"))
	  { fprintf (stderr,
		     "Ignr: dropping message '%s' from %s(%ld), dropped %d\n",
		     msg, room_name (msgrm), msgrm, dropped);
	  }
	}
	
	/* Add the message to the queue, only log if not terse */
	else
        { strncpy (msgqueue[dropped++], msg, MSGSIZ);
	  if (!terse)
	  { fprintf (stderr, "Ignr: queuing message '%s' from %s(%ld)\n",
		     msg, room_name (msgrm), msgrm);
	  }
	}
      }
    }

    return;
  }

  if (dropped > 0)
  { inproc++;

    if (hererm == msgrm)
    { while (i<dropped) readmsg (msgqueue[i++]); }
    else
    { while (i<dropped)
      { if (dont_drop ||
	    (stlmatch (msgqueue[i], "You sense") ||
	     stlmatch (msgqueue[i], "You killed") ||
	     (stlmatch (msgqueue[i], "Tinker") ||
	      stlmatch (msgqueue[i], "Fuzzy")) &&
	     (sindex (msgqueue[i], " off") ||
	      sindex (msgqueue[i], " on")) ||
	     ematch (msgqueue[i], "killed you!") ||
	     sindex (msgqueue[i], " pages, ") || /* Changed from pages: VJ Sun Oct 15 06:58:25 1995*/
	     stlmatch (msgqueue[i], "Your insurance policy pays") ||
	     stlmatch (msgqueue[i], "Your insurance policy has been revoked")))
        { if (dont_drop && debug)
	  { fprintf (stderr, "Proc: processing queued message '%s'\n",
		     msgqueue[i]);
	  }
	  readmsg (msgqueue[i++]);
        }
	else if (!stlmatch (msgqueue[i], "I don't see that player"))
	{ fprintf (stderr, "Ignr: dropping msg '%s' from %s(%ld)\n",
		   msgqueue[i], room_name (msgrm), msgrm);
	  i++;
	}
	else
	{ i++; }
      }
    }
    
    inproc--;
    dropped = 0, msgrm = -1;
  }
  
  i=0;

  if (msg) readmsg (msg);

  /* Turn off debugging */
  if (inproc && !terse)
  { fprintf (stderr, "Proc: exiting procmsg %d\n", inproc); /* debug--; */ }
}

/*****************************************************************
 * charsavail: check for input available from 'fd'
 *****************************************************************/

charsavail (fd)
int fd;
{ long n;
  long retc;
  
  if (retc = ioctl (fd, FIONREAD, &n))
  { fprintf (stderr, "Ioctl returns %ld, n=%ld.\n", retc, n);
    quit_robot ();
  }

  return ((int) n);
}

/****************************************************************
 * swap_files: Do a pointer swap with a temp file and a real file
 ****************************************************************/

swap_files (tmp, real)
char *tmp, *real;
{ char bakfile[MSGSIZ];

  sprintf (bakfile, "%s.BAK", real);

  unlink (bakfile);

  if (link (real, bakfile) < 0)
  { fprintf (stderr, "Warn: cannot link old %s to %s in swap_files\n",
	     real, bakfile);
    fprintf (stderr, "      New file left in %s\n", tmp);
    return (0);
  }
  
  if (unlink (real) < 0)
  { fprintf (stderr, "Warn: cannot unlink old %s in swap_files\n", real);
    fprintf (stderr, "      New file left in %s\n", tmp);
    return (0);
  }
  
  if (link (tmp, real) < 0)
  { fprintf (stderr, "Warn: cannot link new %s to %s in swap_files\n",
	     tmp, real);
    fprintf (stderr, "      New file left in %s\n", tmp);
    return (0);
  }
  
  unlink (tmp);
  
  return (1);
}

/****************************************************************
 * makestring:
 ****************************************************************/

char *makestring (str)
char *str;
{ register char *s;
  register long len = strlen (str) + 1;

  if (s = ralloc (len))
  { string_sp += len; string_ct++;
    strcpy (s, str);
    return (s);
  }
  else
  { crash_robot ("malloc returns NULL in makestring"); }
}

/****************************************************************
 * makefixstring: Make a string that must be a fixed size
 ****************************************************************/

char *makefixstring (str, size)
char *str;
long size;
{ register char *s;
  register long len = strlen (str);

  if (s = ralloc (size))
  { dialog_sp += size; dialog_ct++;

    if (len > size)
    { strncpy (s, str, size);
      s[size-1] = '\0';
      fprintf (stderr, "Warn: truncating string '%32.32s...' to %ld bytes\n",
		str, size);
    }
    else
    { strcpy (s, str); }

    return (s);
  }
  else
  { crash_robot ("malloc returns NULL in makefixstring"); }
}

/****************************************************************
 * ralloc: Malloc replacement for debugging
 ****************************************************************/

char *ralloc (len)
long len;
{ char *str;

  str = (char *) malloc (len);

  if (debug > 1)
  { fprintf (stderr, "Mall: %08x %ld bytes\n", str, len); }
  
  return (str);
}

/****************************************************************
 * freestring:
 ****************************************************************/

freestring (str)
char *str;
{ register char *s;
  register long len;

  if (str == NULL) return;

  len = strlen (str) + 1;

  string_sp -= len; string_ct--;
  freed++;

  if (debug)
  { fprintf (stderr, "Free: %08x, at least %ld bytes.\n", str, len); }

  free (str);
}

/****************************************************************
 * clear_page: Clear the paging variables
 ****************************************************************/

clear_page ()
{
  strcpy (pagedby, "");
  pagedto = -1;
  pagedfrom = -1;
  pagedat = 0;
}

/****************************************************************
 * crash_robot:
 ****************************************************************/

crash_robot (fmt, a1, a2, a3, a4)
char *fmt;
{ long now;
  static long crashing = 0;
  char msg[TOKSIZ];

  /* Handle recursive failures */
  if (crashing++) abort ();

  /* Log error */
  sprintf (msg, fmt, a1, a2, a3, a4);

  /* Get out of Mud somewhat cleanly */
  sendmud ("@desc me as %s has crashed.", myname);
  sendmud ("@quit");
  
  /* Attemp to save state */
  checkpoint ();
  now = time (0);
  fprintf (stderr, "\n\nCrash at %15.15s: %s\n", ctime (&now)+4, msg);

  /* Get a core dump */
  abort ();
}

/****************************************************************
 * init_prefix: Use random numbers to build spoof-proof prefixes
 ****************************************************************/

init_prefix ()
{ long cookie = rand ();

  sprintf (outpre, "%s-%08x", OUTPUTPREFIX, cookie);
  sprintf (outsuf, "%s-%08x", OUTPUTSUFFIX, cookie);
  sprintf (movpre, "%s-%08x", MOVEPREFIX, cookie);
  sprintf (movsuf, "%s-%08x", MOVESUFFIX, cookie);
  sprintf (loopre, "%s-%08x", LOOKPREFIX, cookie);
  sprintf (loosuf, "%s-%08x", LOOKSUFFIX, cookie);
  sprintf (whopre, "%s-%08x", WHOPREFIX, cookie);
  sprintf (whosuf, "%s-%08x", WHOSUFFIX, cookie);
  sprintf (scrpre, "%s-%08x", SCRPREFIX, cookie);
  sprintf (scrsuf, "%s-%08x", SCRSUFFIX, cookie);
  sprintf (pagpre, "%s-%08x", PAGEPREFIX, cookie);
  sprintf (pagsuf, "%s-%08x", PAGESUFFIX, cookie);
  sprintf (numpre, "%s-%08x", NUMBPREFIX, cookie);
  sprintf (numsuf, "%s-%08x", NUMBSUFFIX, cookie);
  sprintf (plypre, "%s-%08x", PLYPREFIX, cookie);
  sprintf (plysuf, "%s-%08x", PLYSUFFIX, cookie);
}

/****************************************************************
 * ignore_msg:
 ****************************************************************/

ignore_msg (msg)
char *msg;
{
  if (debug)
  { fprintf (stderr, "Ignr: %-8s %s\n", modestr[mode], msg); }
}

/****************************************************************
 * do_typecmd: Execute an order
 ****************************************************************/

do_typecmd ()
{ char buf[MSGSIZ];

  /* Follow orders */
  if (*typecmd && !dead)
  { strcpy (buf, typecmd);
    strcpy (typecmd, "");

    if (stlmatch (buf, "straighthome"))
    { fprintf (stderr, "Warn: typing '%s'\n", buf);
      movemud (buf);
    }

    else if (stlmatch (buf, "give ") || stlmatch (buf, "go give ") ||
	     stlmatch (buf, "rob ") || stlmatch (buf, "go rob ") ||
	     stlmatch (buf, "p ") || stlmatch (buf, "go p ") ||
	     stlmatch (buf, "pa ") || stlmatch (buf, "go pa ") ||
	     stlmatch (buf, "pag ") || stlmatch (buf, "go pag ") ||
	     stlmatch (buf, "page ") || stlmatch (buf, "go page ") ||
	     stlmatch (buf, "ki ") || stlmatch (buf, "go ki ") ||
	     stlmatch (buf, "kil ") || stlmatch (buf, "go kil ") ||
	     stlmatch (buf, "kill ") || stlmatch (buf, "go kill ") ||
	     stlmatch (buf, "say ") || stlmatch (buf, "go say") ||
	     stlmatch (buf, "po ") || stlmatch (buf, "go po ") ||
	     stlmatch (buf, "pos ") || stlmatch (buf, "go pos ") ||
	     stlmatch (buf, "pose ") || stlmatch (buf, "go pose") ||
	     stlmatch (buf, "wh ") || stlmatch (buf, "go wh") ||
	     stlmatch (buf, "whi ") || stlmatch (buf, "go whi") ||
	     stlmatch (buf, "whis ") || stlmatch (buf, "go whis") ||
	     stlmatch (buf, "whisp ") || stlmatch (buf, "go whisp") ||
	     stlmatch (buf, "whispe ") || stlmatch (buf, "go whispe") ||
	     stlmatch (buf, "whisper ") || stlmatch (buf, "go whisper") ||
	     stlmatch (buf, "QUIT") || stlmatch (buf, "go QUIT") ||
	     stlmatch (buf, "WHO") || stlmatch (buf, "go WHO") ||
	     stlmatch (buf, "OUTPUT") || stlmatch (buf, "go OUTPUT") ||
	     index (buf, '@') || index (buf, '#') || index (buf, '=') ||
	     *buf == ':' || *buf == '"')
    { msgtype = M_ACTION;
      reply (":decides not to type <%s>.", buf);
    }

    else if (stlmatch (buf, "go "))
    { if (ismuck) fprintf (stderr, "Type: movemud <%s>\n", buf + 3);
      movemud (buf + 3);
    }

    else
    { msgtype = M_UNKNOWN;
      reply (":types <%s>", buf);
      reply ("%s", buf);
      psynch ();
    }
    
    return (1);
  }

  return (0);
}

/****************************************************************
 * do_page: Handle page (also used to implement movement goals)
 ****************************************************************/

do_page ()
{ long pl, from, to;
  char *dir;

  /* answer pages */
  if (*pagedby && !*typecmd && !dead)
  { if (!*here)
    { static long cnt=0;

      if (++cnt > 10)
      { crash_robot ("In main, no location"); }
      else
      { fprintf (stderr, "Waiting for location to appear\n");
	hangaround (60);
	msynch (); return (1);
      }
    }

    pl = (*pagedby == '<') ? -1 : find_player (pagedby);
    
    /* Are we there, yet? Stop if we find our caller */
    if (hererm == pagedto || pl >= 0 && player[pl].present)
    { 
      if (doing_works)
      { command ("@doing me is hanging around in %s", room_name (hererm)); }

      if (streq (pagedby, "<EXPLORING>") ||
	  streq (pagedby, "<AUTORETURN>"))
      { /* Dont say anything */ }

      else if (streq (pagedby, "<HEARTS>"))      
      { strcpy (speaker, ""); msgtype = M_SPOKEN;

	switch (nrrint (188, 4))
        { case 0: reply ("\"I'd like to play"); break;
	  case 1: reply ("\"Please add me to the game"); break;
	  case 2: reply ("\"I'm up for a game of hearts"); break;
	  case 3: reply ("\"Me, me, me!"); break;
        }
      }

      else if (streq (pagedby, "<WHIM>") ||
	       streq (pagedby, "<HOME>") ||
	       streq (pagedby, "<COMMAND>"))
      { if (takingnotes && !quiet)
	{ msgtype = M_UNKNOWN;
	  unlogged (":scribbles on %s pad.", male ? "his" : "her");
	}
      }

      else if (pl >= 0)
      { if (player[pl].present)
        { strcpy (speaker, pagedby);
	  spoke_player (speaker);
	  speaktime = now;
	  
	  reply ("\"Here I am, %s.", pagedby); 
	  
	  if (nextwait < 180) nextwait = 180;
	}
	else
	{ strcpy (speaker, ""); msgtype = M_SPOKEN;
	  reply ("\"I was called here by %s.", player[pl].name);
	  nextwait = 30;
	}
      }

      clear_page ();
      return (1);
    }
    else
    { dir = NULL;

      if (now > pagedat + 20 * MINUTES)
      { fprintf (stderr, "%s: timing out search for %s in %s after %s.\n",
		 *pagedby == '<' ? "Path" : "Page", pagedby,
		 room_name (pagedto), exact_dur (now - pagedat));
	clear_page (); return (1);
      }

      else if ((from = hererm) < 0 ||			/* find here */
	       (to = pagedto) < 0 ||			/* find there*/
	  (dir = find_path (from, to, NEXTMOVEHOME)) == NULL) /* find path */
      { 
	fprintf (stderr, "Path: couldn't find path (%s)%ld -> (%s)%ld\n",
		   here, from, room_name (pagedto), pagedto);

	if (from == to)
	{ crash_robot ("In paging, from == to == %ld, room_match failed",
			from);
	}

	if (to >= 0 &&
	    ((homerm == to) || (dir = find_path (homerm, to, NEXTMOVE))))
	{ if (homerm != to)
	  { fprintf (stderr,
		     "Path: could find path %ld -> %ld (%s), going 'home'\n",
		     homerm, to, dir);
	  }
	  else
	  { fprintf (stderr, "Warn: going home from %ld to %ld\n", hererm, to); }

	  movemud ("straighthome");
	  strcpy (here, home);
	  return (1);
	}

	if (to != from)
	{ fprintf (stderr,
		   "%s: %s for %s in %s(%ld) (from %ld, to %ld, dir %s).\n",
		   *pagedby == '<' ? "Path" : "Page",
		   "giving up search", pagedby, room_name (pagedto),
		   pagedto, from, to, dir ? dir : "(nil)");
	  cant_reach (to);
	}
	
	takingnotes = 0; meetingroom = -1;

	clear_page ();
	return (1);
      }
      else	/* Make the move */
      { 
	movemud (dir); hangaround (1); return (1);
      }
    }
  }

  return (0);
}

/****************************************************************
 * set_page: Set page (if its okay)
 ****************************************************************/

set_page (tmp_caller, tmp_loc)
char *tmp_caller, *tmp_loc;
{ static lastownpage = 0;
  static char lastcaller[MSGSIZ], lastloc[MSGSIZ];
  static long lastat = -1, lastpage = 0;
  char caller[MSGSIZ], loc[MSGSIZ];
  long rm, pl;

  /* Make local copies of variables, just in case */
  strcpy (caller, tmp_caller);
  strcpy (loc, tmp_loc);
  
  /* Avoid generating infinite page loops */
  if (!isowner (caller) &&
      lastat == hererm && lastpage + 60 > now &&
      streq (caller, lastcaller) && streq (loc, lastloc))
  { if (!terse)
    { fprintf (stderr, "Page: redundant page by %s to %s from %s(%ld)\n",
	       caller, loc, room_name (hererm), hererm);
    }
    return (1);
  }
  
  strcpy (lastcaller, caller);
  strcpy (lastloc, loc);
  lastat = hererm;
  lastpage = now;

  /* Now find target room */
  rm = find_room (loc, NULL);

  if (!printedloc) print_locn ();
  fprintf (stderr, "Page: [%s] %s\n", caller, loc);
		  
  active_player (caller, ((rm < 0) ? NULL : loc), NULL);

  /* Check for double pages from owner */
  if (isowner (caller))
  {
    /* Two pages within 15 seconds turns off paging */
    if (lastownpage + 15 > (now = time (0))) { paging = 0; }
    lastownpage = now;
    clear_page (); nextwait = 180;
    takingnotes = 0; meetingroom = -1;
  }
  
  if (rm == hererm)
  { 
    if ((pl = find_player (caller)) >= 0 && player[pl].present)
    { msgtype = M_WHISPER;
      strcpy (speaker, caller);
      reply ("\"I'm right here, %s.", caller);
    }

    else if (pennies > MINPEN && !pagedmsgs)
    { pennies--; send_page (caller, NULL); psynch (); }

    else if (pennies > MINPEN)
    { pennies--;
      msgtype = M_PAGE;
      strcpy (speaker, caller);
      reply ("\"I'm here in %s.", room_name (hererm));
      psynch ();
    }

    nextwait = 180;
    return (1);
  }

  if (page_okay (caller, loc) == 0)
  { return (1); }

  /* If we are busy taking notes, say so */
  if (!isowner (caller) && (takingnotes || playinghearts))
  { if (!quiet)
    { reply ("\"Oh dear, %s is paging me from %s, but I'm busy %s.",
	     caller, loc, takingnotes ? "taking notes" : "playing hearts");
    }

    if (pennies > MINPEN)
    { if (!pagedmsgs)
      { pennies -= 2;
        send_page (caller, NULL); send_page (caller, NULL);
      }
      else
      { pennies --;
        msgtype = M_PAGE;
	strcpy (speaker, caller);
        reply ("\"Sorry, %s, I'm %s in %s right now",
	       caller,
	       takingnotes ? "taking notes" : "playing hearts",
	       room_name (hererm));
      }
      psynch ();
    }
    
    return (1);
  }

  /* If we are not taking calls, say so */
  if (!isowner (caller) && !paging)
  { if (!quiet)
    { reply ("\"Oh dear, %s is paging me from %s, but %s.",
	     caller, loc, "I'm not taking calls right now");
    }

    if (pennies > MINPEN)
    { if (!pagedmsgs)
      { pennies -= 2;
        send_page (caller, NULL); send_page (caller, NULL);
      }
      else
      { pennies --;
        msgtype = M_PAGE;
	strcpy (speaker, caller);
        reply ("\"Sorry, I can't take pages right now, %s", caller);
      }
      psynch ();
    }
    
    return (1);
  }

  /* If we are ignoring player, say so */
  if (!isowner (caller) && is_jerk (caller))
  { if (!quiet)
    { reply ("\"Oh dear, %s is paging me from %s, but %s.",
	     caller, loc, "I'm ignoring jerks");
    }
    
    return (1);
  }

  /* If we are already on a call, say so */
  if (!isowner (caller) && *pagedby && *pagedby != '<' &&
      !streq (caller, pagedby))
  { /* Special case, if the two rooms are the same */
    if (streq (room[pagedto].name, loc))
    { pennies --;
      send_page (caller, NULL);
      psynch ();
      return (1);
    }

    /* Double page to indicate unavailability */
    if (!quiet)
    { reply ("\"Oh dear, %s is paging me from %s, but %s to %s for %s.",
	     caller, loc, "I'm on my way", room_name (pagedto), pagedby);
    }

    if (pennies > MINPEN)
    { if (!pagedmsgs)
      { pennies -= 2;
        send_page (caller, NULL); send_page (caller, NULL);
      }
      else
      { pennies --;
        msgtype = M_PAGE;
	strcpy (speaker, caller);
        reply ("\"Sorry, %s, I'm on my way to %s for %s.",
		caller, room_name (pagedto), pagedby);
      }
      psynch ();
    }
    
    return (1);
  }

  /* If paged to home, go there directly */
  if (rm == homerm)
  { if (*speaker && speaktime + 60 < now)
    { if (quiet) msgtype = M_WHISPER;
      reply ("\"Excuse me, %s is paging me from %s.",
	     caller, room_name (rm));
    }

    send_page (caller, NULL);
    hanging = 0; pgoal = 0;
    fprintf (stderr, "Warn: paged to home by %s, going 'home'\n", caller);
    
    if (isowner (caller) && streq (typecmd, "straighthome"))
    { fprintf (stderr, "Warn: double page to home, resetting...\n");
      reset_robot ("page to home twice");
    }
    
    strcpy (typecmd, "straighthome");
    clear_page ();
    nextwait = 180;
    
    return (1);
  }

  /* We do not know the room, say so */
  if (rm < 0)
  { if (!quiet)
    { reply ("\"Oh dear, %s is paging me from %s, but %s.",
	     caller, loc, "I've never been there");
    }

    fprintf (stderr, "Page: can't find room '%s' for %s\n", loc, caller);

    if (pennies > MINPEN)
    { if (!pagedmsgs)
      { pennies -= 2;
	send_page (caller, NULL); send_page (caller, NULL);
      }
      else
      { pennies--;
        msgtype = M_PAGE;
	strcpy (speaker, caller);
        reply ("\"I've never been to %s, before, %s.", loc, caller);
      }
      psynch ();
    }
    
    return (1);
  }

  /* If we do not have a path, say so */
  if (find_path (hererm, rm, NEXTMOVEHOME) == NULL &&
      find_path (homerm, rm, NEXTMOVE) == NULL)
  { if (!quiet)
    { reply ("\"Oh dear, %s is paging me from %s, but %s.",
	     caller, room_name (rm), "I don't know how to get there");
    }

    fprintf (stderr,
	     "Page: can't find path from %s(%ld) or %s(%ld) to %s(%ld)\n",
	     here, hererm, home, homerm, room_name (rm), rm);

    if (pennies > MINPEN)
    { if (!pagedmsgs)
      { pennies -= 2;
	send_page (caller, NULL); send_page (caller, NULL);
      }
      else
      { pennies--;
        msgtype = M_PAGE;
	strcpy (speaker, caller);
        reply ("\"I don't know how to get to %s, %s", loc, caller);
      }
      psynch ();
    }
    
    return (1);
  }

  /* Okay, everything is okay, set the goal */
  strcpy (pagedby, caller);
  pagedto = rm;
  pagedfrom = hererm;
  pagedat = now = time (0);
  nextwait = 180;

  fprintf (stderr,"Page: by %s, to %s(%ld), from %s(%ld)\n",
	   pagedby, room_name (pagedto), pagedto,
	   room_name (pagedfrom), pagedfrom);

  if (doing_works)
  { command ("@doing me is heading to %s", room_name (pagedto)); }

  if (*speaker && speaktime + 240 > now)
  { if (quiet) msgtype = M_WHISPER;
    reply ("\"Excuse me, %s is paging me from %s.",
	     pagedby, room_name (pagedto));
  }

  hanging = 0; pgoal = 0;

  /* Page back a single time to signal caller */
  if (pennies > MINPEN || isowner (caller))
  { pennies--;

    if (!pagedmsgs)
    { send_page (caller, NULL); }
    else
    { msgtype = M_PAGE;
      strcpy (speaker, caller);
      reply ("\"I'm on my way to you, %s.", caller);
    }

    psynch ();
  }
  
  return (1);
}

/****************************************************************
 * do_notes: Take notes somwhere
 ****************************************************************/

do_notes ()
{
  /* Take notes */
  if (takingnotes && !dead)
  { if (!terse)
    { fprintf (stderr, "Note: taking notes in %s(%ld)\n",
	       room_name (meetingroom), meetingroom);
    }

    if (hererm != meetingroom)
    { pagedto = meetingroom;
      pagedfrom = hererm;
      strcpy (pagedby, "<COMMAND>");
      pagedat = now;

      hangaround (1);
      return (1);
    }
    else
    { if (doing_works)
      { command ("@doing me is taking notes in %s", room_name (hererm)); }

      hangaround (300);
      msgtype = M_UNKNOWN;
      unlogged (":scribbles on %s pad.", male ? "his" : "her");
      return (1);
    }
  }

  return (0);
}

/****************************************************************
 * reset_dead: We died, reset
 ****************************************************************/

reset_dead ()
{ extern long first_turn;

  if (dead)
  { fprintf (stderr, "Dead: killed by %s, restarting\n", killer);

    if (pagedto < 0 && hererm >= 0 && hererm != homerm && randint (100) < 90)
    { pagedto = hererm;
      pagedfrom = homerm;
      pagedat = now;
      strcpy (pagedby, "<AUTORETURN>");
    }

    dead = 0;
    nextwait = 0;

# ifdef NOLONGJMP
    hererm = homerm;
    strcpy (here, home);
    strcpy (desc, homedesc);
    movemud ("straighthome");
# else
    reset_robot ("Killed, restarting");
# endif

    playinghearts = 0;
    first_turn = 1;

    return (1);
  }

  return (0);
}

/****************************************************************
 * reset_teleported: We were teleported, reset
 ****************************************************************/

reset_teleported ()
{
  if (dead) return (0);

  nextwait = 0;

# ifdef NOLONGJMP
  hererm = homerm;
  strcpy (here, home);
  strcpy (desc, homedesc);
  movemud ("straighthome");
# else
  reset_robot ("Teleported, restarting");
# endif

  return (1);
}

/****************************************************************
 * special_mode: Handle special TinyMUD output (LOOK, SCORE, WHO)
 ****************************************************************/

special_mode (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ static long oldobjs = 0;

  /* Ignore message that might be spoofing, or are junk */
  if (msg == NULL || *msg == '\0')
  { ignore_msg (msg); return (1); }
  
  if (stlmatch (msg, "<<") || stlmatch (msg, ">>"))
  { ignore_msg (msg); return (1); }

  if (stlmatch (msg, "You say \"") ||
      stlmatch (msg, "You whisper \"") ||
      (mode != NORMAL && stlmatch (msg, myname)))
  { return (1); }

  /*---------------- SCORE mode output ----------------*/
  if (mode == SCORE)
  { if (MATCH (msg, "You have * pennies.") ||
        MATCH (msg, "You have * penny.") ||
	MATCH (msg, "You have * cookies.") ||
        MATCH (msg, "You have * cookie."))
    { if (isdigit (res1[0]))
      { pennies = atol (res1); }
      else
      { fprintf (stderr, "Bgus: '%s', non integer pennies.\n", msg); }
    }
    else if (MATCH (msg, "The universe contains * objects."))
    { objs = atol (res1);

      if (objs != oldobjs)
      {	if (!terse)
        { fprintf (stderr, "Objs: %8.8s @stat = %ld, %ld new\n",
	         ctime (&now) + 11, objs, objs - oldobjs);
	}
        oldobjs = objs;
        do_number ();
      }
      else if (debug)
      { fprintf (stderr, "Same: still %ld objects\n", objs);
      }
    }
    else
    { ignore_msg (msg); }

    return (1);
  }
  
  /*---------------- COMMAND mode output ----------------*/
  if (mode == COMMAND)
  { /* If we kill someone, they leave during our command listing */
    if (MATCH (msg, "* has left."))
    { fprintf (stderr, "Dprt: [%s]\n", res1);
      leave_player (res1, here, desc);
      alone--;
    }
    
    else if (MATCH (lcmsg, "that player is not accepting pages*"))
    { msgstat = -4; }

    else if (MATCH (lcmsg, "i don't recognize that name*") ||
	     MATCH (lcmsg, "i don't recognize that player*"))
    { msgstat = -3; }

    else if (MATCH (lcmsg, "that person is not connected*") ||
	     (MATCH (lcmsg, "* is not connected*") &&
	      find_player (res1) >= 0))
    { msgstat = -2; }

    else if (MATCH (lcmsg, "whisper to whom?*"))
    { msgstat = -1; }

    else if (MATCH (lcmsg, "you whisper \"*") ||
	     MATCH (lcmsg, "you whisper, \"*") ||
	     MATCH (lcmsg, "your message has been sent*"))
    { msgstat = 1; }

    else
    { ignore_msg (msg); }

    return (1);
  }
  
  /*---------------- PAGE mode output ----------------*/
  if (mode == PAGE)
  { if (MATCH (msg, "You sense that * is looking for you in *."))
    { if (streq (res1, myname))
      { if (!room_match (here, res2))
        { fprintf (stderr, "Bgus: paging myself returns '%s', look '%s'\n",
		   res1, here);
	  strcpy (here, res2);
	}
	else
	{ fprintf (stderr, "Good: paging myself gives '%s'\n", res2); }

	return (1);
      }
    }

    ignore_msg (msg);
    return (1);
  }

  /*---------------- WHO mode output ----------------*/
  if (mode == WHO)
  { if (MATCH (lcmsg, "current players*") ||
        MATCH (lcmsg, "player name*on for*idle*"))
    { clear_active (); }
  
    else if (MATCH (lcmsg, "* players are connected*") ||
    	     MATCH (lcmsg, "1 player is connected*") ||
	     MATCH (lcmsg, "* users are connected*") ||
    	     MATCH (lcmsg, "1 user is connected*"))
    { /* ignore */ }
  
    else if (MATCH (msg, "* idle * seconds"))
    { if (debug) fprintf (stderr, "Who:  [%s] %s seconds\n", res1, res2);
      idle_player (res1, atol (res2));
    }
    
    else if (MATCH (msg, "You have * pennies.") ||
	     MATCH (msg, "You have * penny.") ||
	     MATCH (msg, "You have * cookies.") ||
	     MATCH (msg, "You have * cookie.") )
    { if (isdigit (res1[0]))
      { pennies = atol (res1); }
      else
      { fprintf (stderr, "Bgus: '%s', non integer pennies.\n", msg); }
    }
    
    /* Parse new format of idle, from tinymud 1.5.2 on */
    else if (msg[24] == ':' || msg[27] == ':')
    { char pname[TOKSIZ], idlestr[TOKSIZ];
      long idletim;
      register char *s, *t;

      /* Strip off first word (name) */
      s=msg;

      while (*s && isspace (*s)) s++;
      if (streq (world, "HoloMuck") && *s == '+') s++;

      for (t=pname; *s && !isspace (*s); ) *t++ = *s++;
      *t = '\0';

      /* Find idle time (last token or ends at col 35 in Holo) */
      if (doing_works && strlen (msg) > 35)
      { s = msg+35; }
      else
      { s = msg + strlen (msg) - 1; }

      while (s>msg && !isspace (s[-1])) s--;
      strcpy (idlestr, s);

      idletim = atol (idlestr);
      for (s=idlestr; *s && isdigit (*s); s++);
      switch (*s)
      { case 's':	/* base case */ break;
        case 'm':	idletim *= MINUTES; break;
	case 'h':	idletim *= HOURS; break;
	case 'd':	idletim *= DAYS; break;
	default:	fprintf (stderr,
			         "Can't parse time modifer '%s', msg '%s'\n",
				 idlestr, msg);
			break;
      }

      if (!terse) fprintf (stderr, "Who:  [%s] %ld seconds\n", pname, idletim);
      idle_player (pname, idletim);
    }
    
    return (1);
  }

  /*---------------- Ignore some messages ----------------*/
  if (stlmatch (msg, "I don't see that player here."))
  { ignore_msg (msg); return (1); }
  
  /*---------------- Ignore other modes output ----------------*/
  if (mode != NORMAL)
  { ignore_msg (msg); return (1); }
  

  /* Message not matched by this rule set */
  return (0);
}

/****************************************************************
 * check_time: Check time varying modes
 ****************************************************************/

check_time ()
{ long pl;

  /* Check the WHO list once every two minutes */
  if ((now = time (0)) - lastwsynch > 120)
  { wsynch ();
    /* usynch (); */
    do_msgs ();
  }
  
  /* Clear the code word after 10 minutes */
  if ((now - codeset) > 10 * MINUTES && *code) *code = '\0';

  /* Alone tracks the number of other players in the room */
  alone = 0;
  player[me].active = player[me].present = 0;

  for (pl = 0; pl<players; pl++)
  { if (player[pl].active && player[pl].present) alone++; }

  /* Print out current location */
  if (!terse && !printedloc) print_locn ();

  /* Actions if we are not really, really busy */
  if (!*pagedby || *pagedby == '<')
  {
    /* Look at everyone (sleepers twice a day, others every 45 minutes) */
    for (pl = 0; pl<players; pl++)
    { if (player[pl].present &&
	  (randint (100) > 95 ||
	  (now - player[pl].lastlook) >
	   (player[pl].active ? (45 * MINUTES) : (12 * HOURS))))
      { look_at_thing (player[pl].name); }
    }

    /* Do we need to checkpoint our map and players files? */
    if (alone < 2 && ((now > (lastcheck + checkfreq)) ||
		      (newrooms > 10) || (newexits > 500)) ||
	((now > (lastcheck + checkfreq + 20 * MINUTES)) ||
		      (newrooms > 20) || (newexits > 1000)))
    { if (*speaker && speaktime + 120 > now &&
	  (pl = find_player (speaker)) >= 0 &&
	  player[pl].present)
      { reply ("\"Excuse me a minute, I have to save my map, %s.",
		 speaker);
      }
      else
      { strcpy (speaker, "");
        unlogged ("\"Excuse me a minute, I have to save my map.");
      }
      command ("@describe me as %s is busy saving %s map.",
	       myname, male ? "his" : "her");
      checkpoint ();
      command ("@describe me as %s.", mydesc);
      unlogged ("\"Okay, I saved my map.");
    }
  }

  return (1);
}

/****************************************************************
 * print_locn:
 ****************************************************************/

print_locn ()
{ long printed;
  long pl;

  /* Print location */
  if (*typecmd)
  { fprintf (stderr, "\nLocn: [%8.8s] %s(%ld), typecmd '%s'\n",
	     ctime (&now)+11, *here ? here : "(nowhere)", hererm, typecmd);
  }
  else
  { fprintf (stderr, "\nLocn: %8.8s %s(%ld)\n",
	     ctime (&now)+11, *here ? here : "(nowhere)", hererm);
  }
  
  if (*desc && !terse)
  { fprintf (stderr, "Desc: %-72.72s\n", desc); }

  if (*contents && !terse)
  { fprintf (stderr, "Cont: %-72.72s\n", contents); }

  player[me].active = player[me].present = 0;
  for (pl = 0, printed=0; pl<players; pl++)
  { if (player[pl].active && player[pl].present)
    { if (!printed++) fprintf (stderr, "Pres: (%ld) ", alone);
      fprintf (stderr, " %s", player[pl].name);
    }
  }

  if (printed) fprintf (stderr, "\n");

  if (!terse)
  { for (pl = 0, printed=0; pl<players; pl++)
    { if (!player[pl].active && player[pl].present)
      { if (!printed++) fprintf (stderr, "Inac:");
        fprintf (stderr, " %s", player[pl].name);
      }
    }
  }

  if (printed) fprintf (stderr, "\n");

  fprintf (stderr, "\n");

  printedloc++;
}

/****************************************************************
 * reset_robot: Reset by longjumping to the robot initialize.
 *	leaves the map and player info in the current state.
 ****************************************************************/

reset_robot (msg)
char *msg;
{ char buf[4];
  long len;

  fprintf (stderr, "Rset: resetting at %15.15s, message: %s\n",
	   ctime (&now)+4, msg);

  /* Drain input */
  while (charsavail (fmud))
  { while ((len = read (fmud, buf, 1)) > 0)
    { if (*buf == '\n') break; }
  }

  /* Reset reachable map */
  reach_changed++;
  
  /* Jump to the top of the main loop */
  longjmp (start_state, 0);
}

/****************************************************************
 * fatal: Fatal error message
 ****************************************************************/

fatal (fmt, a1, a2, a3, a4, a5, a6, a7, a8)
char *fmt, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8;
{ 
  close (tmud);
  fprintf (stderr, fmt, a1, a2, a3, a4, a5, a6, a7, a8);
  exit (1);
}

/****************************************************************
 * is_me: True if string matches my name
 ****************************************************************/

is_me (name)
char *name;
{ return (strfoldeq (name, myname)); }

/****************************************************************
 * is_tell: True if msg is a 'JuliaGram' message to be sent to
 *	    another player
 ****************************************************************/

char *is_tell (lcmsg)
char *lcmsg;
{ register char *s;

  if (tty_interface) return (NULL);

  if (( MATCH (lcmsg, "julia tell *") ||
	MATCH (lcmsg, "tell *") ||
	MATCH (lcmsg, "would you tell *") ||
	MATCH (lcmsg, "could you tell *") ||
	MATCH (lcmsg, "would you please tell *") ||
	MATCH (lcmsg, "could you please tell *") ||
	MATCH (lcmsg, "ask *") ||
	MATCH (lcmsg, "julia ask *") ||
	MATCH (lcmsg, "also tell *") ||
	MATCH (lcmsg, "also ask *") ||
	MATCH (lcmsg, "and tell *") ||
	MATCH (lcmsg, "and ask *") ||
	MATCH (lcmsg, "please, tell *") ||
	MATCH (lcmsg, "please, ask *") ||
	MATCH (lcmsg, "please tell *")||
	MATCH (lcmsg, "please ask *")) && (s = car (res1)) ||
       (MATCH (lcmsg, "*: please tell *") ||
	MATCH (lcmsg, "*: please ask *") ||
	MATCH (lcmsg, "*, please tell *") ||
	MATCH (lcmsg, "*, please ask *") ||
	MATCH (lcmsg, "*: please, tell *") ||
	MATCH (lcmsg, "*: please, ask *") ||
	MATCH (lcmsg, "*, please, tell *") ||
	MATCH (lcmsg, "*, please, ask *") ||
	MATCH (lcmsg, "*: tell *") ||
	MATCH (lcmsg, "*: ask *") ||
	MATCH (lcmsg, "*, tell *") ||
	MATCH (lcmsg, "*, also tell *") ||
	MATCH (lcmsg, "*, ask *")) && strfoldeq (res1, myname) &&
          (s = car (res2)) ||
       (MATCH (lcmsg, "*: *, tell *") ||
	MATCH (lcmsg, "*: *, ask *") ||
	MATCH (lcmsg, "*, *, tell *") ||
	MATCH (lcmsg, "*, *, please tell *") ||
	MATCH (lcmsg, "*, *, also tell *") ||
	MATCH (lcmsg, "*, *, ask *")) && strfoldeq (res1, myname) &&
          (s = car (res3)))
  { if (stlmatch (res2, "your owner") ||
	stlmatch (res2, "your programmer") ||
	stlmatch (res2, "your master") ||
	stlmatch (res2, "your amway representative"))
    { return (owner); }
    else if (streq (s, "me") ||
	     streq (s, "all") ||
	     streq (s, "everybody") ||
	     streq (s, "everyone") ||
	     streq (s, "every") ||
	     streq (s, "us") ||
	     streq (s, "your") ||
	     streq (s, "you") ||
	     streq (s, "yourself") ||
	     reserved (s))
    { return (NULL); }
    else
    { return (s); }
  }
  else
  { return (NULL); }
}

/****************************************************************
 * send_page:
 ****************************************************************/

send_page (name, msg)
char *name, *msg;
{ char cmd[BIGBUF];

  if (tty_interface)
  { strcpy (cmd, msg); }
  else if (ismuck && pagecmd)
  { if (msg && *msg)
    { sprintf (cmd, "%s\n%s\n#%d\n%s\n%s %s\n%s %s\npages: \"%s\"",
	       opre, osuf, pagecmd, name, opre, outpre,
	       osuf, outsuf, msg);
    }
    else
    { sprintf (cmd, "%s\n%s\n#%d\n%s\n%s %s\n%s %s\n.",
	       opre, osuf, pagecmd, name, opre, outpre,
	       osuf, outsuf);
    }
  }
  else
  { if (ispueblo)
    { if (msg && *msg)
      { sprintf (cmd, "page %s %s", name, msg); }
      else
      { sprintf (cmd, "page %s", name); }
    }
    else 
     {if (msg && *msg)
        { sprintf (cmd, "page %s = %s", name, msg); }
      else
        { sprintf (cmd, "page %s", name); }
    }
  }
  
  command ("%s", cmd);
}

/****************************************************************
 * send_whisper:
 ****************************************************************/

send_whisper (name, msg)
char *name, *msg;
{ char cmd[BIGBUF];

  if (tty_interface)
  { strcpy (cmd, msg); }
  else if (ismuck && pagecmd)
  { sprintf (cmd,
	     "%s\n%s\n#%d\n%s\n%s %s\n%s %s\nwhispers, \"%s\"",
	     opre, osuf, pagecmd, name, opre, outpre,
	     osuf, outsuf, msg);
  }
  else
   { if (ispueblo)
      { sprintf (cmd, "whisper \"%s\" to %s", msg, name); }
     else 
      { sprintf (cmd, "whisper \"%s\" =  %s", name, msg); }
   }
  command ("%s", cmd);
}

/****************************************************************
 * send_say
 ****************************************************************/

send_say (msg)
char *msg;
{ char cmd[BIGBUF];

  if (tty_interface)
  { strcpy (cmd, msg); }
  else if (ismuck && posecmd)
  { sprintf (cmd, "%s\n%s\n#%d\n%s %s\n%s %s\nsays, \"%s\"",
	     opre, osuf, posecmd, opre, outpre, osuf, outsuf, msg);
  }
  else
  { sprintf (cmd, "\"%s", msg);}
  
  command ("%s", cmd);
}

/****************************************************************
 * send_pose
 ****************************************************************/

send_pose (msg)
char *msg;
{ char cmd[BIGBUF];

  if (tty_interface)
  { sprintf (cmd, "Julia %s", msg); }
  else if (ismuck && posecmd)
  { sprintf (cmd, "%s\n%s\n#%d\n%s %s\n%s %s\n%s",
	     opre, osuf, posecmd, opre, outpre, osuf, outsuf, msg);
  }
  else
  { sprintf (cmd, ":%s", msg);}
  
  command ("%s", cmd);
}

/****************************************************************
 * is_newbie: Return true is player is 'new' (someone we've known
 * less than 3 days).  Also a 30 percent chance of treating an
 * old player like a newbie
 ****************************************************************/

is_newbie (name)
{ long pl;

  if (isowner (name)) return (randint (100) < 50);

  if ((pl = find_player (name)) < 0) return (1);
  
  if ((now - player[pl].firstsaw) < 3 * DAYS) return (1);
  
  if (randint (100) < 30) return (1);
  
  return (0);
}


# ifndef cmu
# define TRUE 1
# define FALSE 0

/****************************************************************
 * Routines from CMU's libcs.a, for compatibility
 ****************************************************************/


/*  stlmatch  --  match leftmost part of string
 *
 *  Usage:  i = stlmatch (big,small)
 *	int i;
 *	char *small, *big;
 *
 *  Returns 1 iff initial characters of big match small exactly;
 *  else 0.
 *
 *  HISTORY
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX from Ken Greer's routine.
 *
 *  Originally from klg (Ken Greer) on IUS/SUS UNIX
 */

int stlmatch (big,small)
char *small, *big;
{
	register char *s, *b;
	s = small;
	b = big;
	do {
		if (*s == '\0')  return (TRUE);
	} 
	while (*s++ == *b++);
	return (FALSE);
}

/*  sindex  --  find index of one string within another
 *
 *  Usage:  p = sindex (big,small)
 *	char *p,*big,*small;
 *
 *  Sindex searches for a substring of big which matches small,
 *  and returns a pointer to this substring.  If no matching
 *  substring is found, 0 is returned.
 *
 * HISTORY
 * 26-Jun-81  David Smith (drs) at Carnegie-Mellon University
 *	Rewritten to avoid call on strlen(), and generally speed up things.
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Adapted for VAX from indexs() on the PDP-11 (thanx to Ralph
 *	Guggenheim).  The name has changed to be more like the index()
 *	and rindex() functions from Bell Labs; the return value (pointer
 *	rather than integer) has changed partly for the same reason,
 *	and partly due to popular usage of this function.
 *
 *  Originally from rjg (Ralph Guggenheim) on IUS/SUS UNIX.
 */


char *sindex (big,small) char *big,*small;
    {
    register char *bp, *bp1, *sp;
    register char c = *small++;

    if (c==0) return(0);
    for (bp=big;  *bp;  bp++)
	if (*bp == c)
	    {
	    for (sp=small,bp1=bp+1;   *sp && *sp == *bp1++;  sp++)
		;
	    if (*sp==0) return(bp);
	    }
    return 0;
    }
# endif
