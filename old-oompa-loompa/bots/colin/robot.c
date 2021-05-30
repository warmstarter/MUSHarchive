/****************************************************************
 * robot.c: Common robot code for TinyMUD automata
 *
 * HISTORY
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
# include <sys/ioctl.h>
# include <sys/socket.h>
# include <setjmp.h>
# include <netinet/in.h>
# include <netdb.h>
# include <ctype.h>
# include <varargs.h>

# include "robot.h"

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
long	debug=0, testing=0, terse=0, usedesc=0, quiet=1;
long	playing = 1, dead = 0, hanging = 0, pgoal = 0;
long	speed = 5, generous = 0, exploring = 0;
long	realmsg = 1;

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
long freed = 0;

/* Prefixes and suffixes */
char *opre = "OUTPUTPREFIX";
char *osuf = "OUTPUTSUFFIX";
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

/* Results from star matcher */
char res1[BUFSIZ], res2[BUFSIZ], res3[BUFSIZ], res4[BUFSIZ];
char res5[BUFSIZ], res6[BUFSIZ], res7[BUFSIZ], res8[BUFSIZ];
char *result[] = { res1, res2, res3, res4, res5, res6, res7, res8 };

char room1[BUFSIZ], room2[BUFSIZ], room3[BUFSIZ], room4[BUFSIZ];
char *roomstr[] = { room1, room2, room3, room4 };

char tmp1[BUFSIZ], tmp2[BUFSIZ], tmp3[BUFSIZ], tmp4[BUFSIZ];
char tmp5[BUFSIZ], tmp6[BUFSIZ], tmp7[BUFSIZ], tmp8[BUFSIZ];
char *tmpstr[] = { tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8 };

O_ROOM *room = NULL;
PATH *path = NULL;
long rooms = 0, maxrooms = 0;
long lastrm = 0, exits = 0;

char *stexits[] = {
	"north", "south", "east", "west", "n", "s", "e", "w",
	"ne", "nw", "se", "sw", "nne", "nnw", "ene", "wnw",
	"sse", "ssw", "ese", "wsw", "in", "out", "leave", "back",
	"up", "down", "left", "right", "0", "1", "2", "3", "4",
	"5", "6", "7", "8", "9", "10", "11", "12", "open", "close",
	"rec", "cw", "ccw", "wait", "move around", "look around",
	"door", "port", "starboard", "fore", "aft", "sideways",
	NULL };

long numexits = (sizeof (stexits) / sizeof (*stexits)) - 1;

char typecmd[TOKSIZ];

PLAYER *player;
long players = 0, maxplayer = 0;

char *modestr[] =
{ "NORMAL", "MOVE", "WHO", "PAGE", "LOOK",
  "SCORE", "NUMBER", "COMMAND", "PLAYER" };

/*-------- Current statistics --------*/
char mydesc[MSGSIZ];
long mlnum = -1;
long pennies = 0;
long gave = -1;
long needpsynch = 0;
char killer[MSGSIZ] = "";
char speaker[MSGSIZ] = "";
char here[TOKSIZ] = "";
char desc[TOKSIZ] = "";
char contents[BIGBUF] = "";
long hererm = -1;
char home[TOKSIZ] = "";
char homedesc[TOKSIZ] = "";
long homerm = -1;
char move1[TOKSIZ] = "";
char move2[TOKSIZ] = "";
char pathto[TOKSIZ] = "";
long pagedfrom = -1;
long pagedto = -1;
char pagedby[MSGSIZ] = "";
long pagedat = 0;
long now = 0;
long mode = NORMAL;
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
long meetingroom = -1;
long termwarn = 0;
long termtarg = -1;
long termat = 0;
char termloc[MSGSIZ] = "";
char thief[MSGSIZ] = "";
long lastlock = 0;
char lastobj[MSGSIZ] = "";

/*---- Variables for maintaining RM_REACH flags ----*/
long reach_added = 0;
long reach_changed = 0;

/*-------- Configuration Variables --------*/
long   male = 0;
char *myname = NULL;
char *owner = NULL;
char *whoami = NULL;
char *mudhost = NULL;
long mudport = 0;
char *mapfile = NULL;
char *plyfile = NULL;
jmp_buf start_state;


/****************************************************************
 * Main routine
 ****************************************************************/

# define SKIPARG		while (*++(*argv)); --(*argv)

main (argc, argv)
int   argc;
char *argv[];
{ char *pname = argv[0];

  /* Get the options from the command line */
  while (--argc > 0 && (*++argv)[0] == '-')
  { while (*++(*argv))
    { switch (**argv)
      { 
	case 'H': mudhost = *argv+1; SKIPARG; break;
	case 'P': mudport = atoi (*argv+1); SKIPARG; break;
	case 'M': mapfile = *argv+1; SKIPARG; break;
	case 'F': plyfile = *argv+1; SKIPARG; break;
	case 'S': speed = atoi (*argv+1); SKIPARG; break;
	case 'T': testing++; break;
	case 'd': debug++; break;
	case 'n': quiet=0; break;
	case 'D': usedesc++; break;
	case 'e': exploring++; break;
	case 'g': generous++; break;
	case 'p': paging++; break;
	case 't': terse++; break;

        default:  fprintf (stderr, "Usage: %s [ options ]\n\n", pname);
		  fprintf (stderr, "	-d		debug\n");
		  fprintf (stderr, "	-e		exploring\n");
		  fprintf (stderr, "	-g		generous\n");
		  fprintf (stderr, "	-p		paging\n");
		  fprintf (stderr, "	-t		terse\n");
		  fprintf (stderr, "	-D		use descriptions for exits\n");
		  fprintf (stderr, "	-T		testing (connect to stdin/stdout)\n\n");

		  fprintf (stderr, "	-H'host'	Host to connect to game\n");
		  fprintf (stderr, "	-P<port>	Port number\n");
		  fprintf (stderr, "	-M'mapfile'	Map file name\n");
		  fprintf (stderr, "	-F'playfile'	File for player information\n");


		  exit (1);
      }
    }
  }

  setconfig ();

  if (testing)
  { tmud = 1; fmud = 0; }
  else
  { tmud = fmud = connectmud (); }

  if (tmud < 0)
  { now = time (0);
    fprintf (stderr,"Connect failed at %15.15s\n", ctime (&now)+4);
    exit (-1);
  }

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
  sendmud ("%s %s\npage %s = 0\n%s %s",
	   opre, pagpre, myname, opre, outpre);
  waitfor (outsuf);
}

/****************************************************************
 * psynch: Simple sync for stationary commands
 ****************************************************************/

psynch ()
{
  if (debug) fprintf (stderr, "In psynch()...\n");
  sendmud ("%s %s\nscore = 0\n%s %s",
	   opre, scrpre, opre, outpre);
  waitfor (outsuf);

  needpsynch = 0;
}

/****************************************************************
 * msynch: Complex sync for moving commands commands
 ****************************************************************/

msynch ()
{
  if (debug) fprintf (stderr, "In msynch()...\n");
  sendmud ("%s %s\nlook = 0\n%s %s",
	   opre, loopre, opre, outpre);
  waitfor (outsuf);
}

/****************************************************************
 * wsynch: Complex sync for moving commands commands
 ****************************************************************/

wsynch ()
{
  if (debug) fprintf (stderr, "In wsynch()...\n");
  inwsynch++;
# ifndef NEWMUD
  sendmud ("%s %s\n%s\nscore\n%s\nWHO\n%s %s\nscore\n%s %s",
           opre, whopre, osuf, opre, osuf, outsuf, opre, outpre);
# else
  sendmud ("%s %s\nWHO\n%s %s",
	   opre, whopre, opre, outpre);
# endif
  waitfor (outsuf);
  inwsynch--;
  lastwsynch = now;
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

      /* Reset dead man timer */
      start = now = time (0);
    }
    else
    { 
      /* Check timer, if 10 minutes since last IO, timeout */
      if ((now = time (0)) > start + 10 * MINUTES)
      { fprintf (stderr, "Quit: timed out in waitfor after %s\n",
		 time_dur (now-start));
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
  }

  
  return (1);
}


/*****************************************************************
 * smatch: Given a data string and a pattern containing one or
 * more embedded stars (*) (which match any number of characters)
 * return true if the match succeeds, and set res[i] to the
 * characters matched by the 'i'th *.
 *****************************************************************/

smatch (dat, pat, res)
register char *dat, *pat, **res;
{ register char *star = 0, *starend, *resp;
  int nres = 0;

  while (1)
  { if (*pat == '*')
    { star = ++pat; 			     /* Pattern after * */
      starend = dat; 			     /* Data after * match */
      resp = res[nres++]; 		     /* Result string */
      *resp = '\0'; 			     /* Initially null */
    }
    else if (*dat == *pat) 		     /* Characters match */
    { if (*pat == '\0') 		     /* Pattern matches */
	return (1);
      pat++; 				     /* Try next position */
      dat++;
    }
    else
    { if (*dat == '\0') 		     /* Pattern fails - no more */
	return (0); 			     /* data */
      if (star == 0) 			     /* Pattern fails - no * to */
	return (0); 			     /* adjust */
      pat = star; 			     /* Restart pattern after * */
      *resp++ = *starend; 		     /* Copy character to result */
      *resp = '\0'; 			     /* null terminate */
      dat = ++starend; 			     /* Rescan after copied char */
    }
  }
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
  
  if (write (tmud, buf, len) != len)
  { fprintf (stderr, "Write failed: %s", buf);
    quit_robot ();
  }
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

  sendmud (buf);
  waitfor (outsuf);

  if (debug || cmdlevel > 1)
  { fprintf (stderr, "Ackn: level %ld\n", cmdlevel--); }
}

/****************************************************************
 * movemud: Send a movement command to the TinyMUD process
 ****************************************************************/

movemud (dir)
char *dir;
{ long len;
  char buf[MSGSIZ], lasthere[MSGSIZ], lastdesc[TOKSIZ];
  long oldrm = -1;
  long wenthome = 0;
  char *lcmd = "look = 0";
  static long entered = 0;
  register long pl;

  if (debug)
  { fprintf (stderr, "In movemud(%s), here is '%s'\n", dir, here); }
  
  /* Build an atomic prefix/suffix/move command */
  if (streq (dir, "home"))
  { sprintf (buf, "%s %s\nhome\n%s %s\n%s %s\n%s\n%s %s\n%s %s",
	     opre, movpre, osuf, movsuf, opre, loopre,
	     lcmd,
	     opre, outpre, osuf, outsuf);
    wenthome++;
  }
  else
  { sprintf (buf, "%s %s\ngo %s\n%s %s\n%s %s\n%s\n%s %s\n%s %s",
	     opre, movpre, dir, osuf, movsuf, opre, loopre,
	     lcmd,
	     opre, outpre, osuf, outsuf);
  }

  /* Actions upon leaving a room */
  before_move_hook ();

  /* Actually send the MOVE command */
  if (debug) fprintf (stderr, "Move: %s\n", dir);

  strcpy (lasthere, here);
  strcpy (lastdesc, desc);
  oldrm = hererm;

  len = strlen (buf);

  strcpy (here, "");
  strcpy (desc, "");

  inmove++;

  sendmud (buf);  
  waitfor (movsuf);

  inmove--;
  
  if (debug)
  { fprintf (stderr, "Here: After look, here '%s', desc '%s'\n",
	     here, desc);
  }

  confused = 0;

  /*---- Determine where we think we are, match MOVE and LOOK output ----*/

  /* If LOOK matches first line, everything is cool */
  if (streq (move1, here))
  { if (debug) fprintf (stderr, "Okay: move matches look '%s'\n", move1); }

  /* If LOOK matches second line, everything is still cool */
  else if (streq (move2, here))
  { if (debug) fprintf (stderr, "Okay: had @su message '%s'\n", move1); }

  /* If MOVE1 is fail message, then we are probably in the same place */
  else if (streq (move1, "You can't go that way."))
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
    else
    { 
      /* Must mark exit as used, to avoid infinite loops */
      add_exit (oldrm, dir, oldrm);
    }

    *lasthere = '\0'; hererm = -1; *here = '\0';
    fprintf (stderr, "Conf: going hom from '%s'(%ld)\n", here, hererm);
    movemud ("home");
    return;
  }

  /* Now set current room */
  hererm = add_room (here, desc);
  room[hererm].lastin = now = time (0);

  /* If we think we found a penny, must do a psynch */
  if (needpsynch) psynch ();

  /* No matter what we did, add the exit to our map */
  if (!confused && *here && *lasthere && !wenthome)
  { add_exit (oldrm, dir, hererm); }

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

  /* If we went home, call home_hook */
  if (wenthome) home_hook ();

  /* If we queued any messages during the move, process them */
  procmsg (NULL);

  /* Actions after sending a move command (may still be in old room) */
  after_move_hook ();

  /* If we ARE in a new room, perform new room actions */
  if (hererm != oldrm)
  { printedloc = 0;

    /* Track total visits to this room, total time in old room */
    room[hererm].cntin++;
    if (entered)
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
  char buf[BIGBUF];
  register char *s;
  
  /* Log our reply */
  if (!printedloc) print_locn ();

  sprintf (buf, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  command (buf);

  if (terse)
  { s = (*buf == '"') ? buf+1 : buf;
    fprintf (stderr, "Rply: %s\n", s);
  }
}

/****************************************************************
 * getmud: Read one line from TinyMUD
 ****************************************************************/

char *getmud ()
{ long len, result = 0;
  static long havecnt = 0;
  static char buf[BUFSIZ], rbuf[4], plybuf[MSGSIZ];
  char junk[MSGSIZ];
  register char *s=buf;
  static long pl;

  /* No input waiting */
  if (!charsavail (fmud)) return (NULL);

  /* Read one line, save printing chars only */
  while ((len = read (fmud, rbuf, 1)) > 0)
  { if (*rbuf == '\n')		break;
    if (isprint (*rbuf))	*s++ = *rbuf;
  }
  *s = '\0';

  /* Check for error */  
  if (len < 0)
  { fprintf (stderr, "Error %ld reading from mud\n", len);
    quit_robot ();
  }

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
    { strcpy (move1, buf); strcpy (move2, ""); }
    else if (mlnum == 2)
    { strcpy (move2, buf); }

    if (mlnum > 1 && streq (buf, "You found a penny!"))
    { needpsynch++; }
  }
  
  /*---- Handle output from looking at a room (using "look = 0") ----*/
  else if (mode == LOOK)
  { if (!havecnt)
    { switch (mlnum)
      { case 1:	strcpy (here, buf);
		strcpy (desc, "");
		if (debug)
		{ fprintf (stderr, "Here: setting here to line1 '%s'\n", buf);}
		*contents = '\0';
		clear_present ();
		break;

        case 2:	if (streq (buf, "Contents:"))
		{ havecnt++; }
		else
		{ strcpy (desc, buf);
		  if (debug)
		  { fprintf (stderr,
			    "Here: setting desc to line2 '%s'\n", buf);
		  }
		}
		break;
		
        default: if (streq (buf, "Contents:"))
		{ havecnt++; }
		break;
      }
    }
    
    else
    { if (find_player (buf) >= 0)
      { saw_player (buf, here, desc); }
      else
      { if (debug) fprintf (stderr, "Obj:  %s\n", buf);
	if ((strlen (contents) + strlen (buf) + 2) < BIGBUF)
	{ if (*contents) strcat (contents, ", ");
	  strcat (contents, buf);
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
    { lastlock = atoi (tmp5);
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
	  
	  fprintf (stderr, "Look: player '%s'(%ld), desc: %-32.32s\n",
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

# define MAXQ 32

procmsg (msg)
char *msg;
{ static long msgrm = -1, dropped = 0, inproc = 0;
  static msgqueue[MAXQ][MSGSIZ];
  register long i;  

  /*---- Queue messages that happen "between rooms" ----*/

  if (inmove && msg)
  { 
    /* Special case for WHO mode output */
    if (MATCH (msg, "* idle * seconds"))
    { if (debug) fprintf (stderr, "Who:  [%s] %s seconds\n", res1, res2);
      idle_player (res1, atoi (res2));
      return;
    }

    /* Only queue NORMAL mode messages */
    if (mode == NORMAL)
    { if (stlmatch (msg, ">>")) return;
    
      if (inproc)
      { fprintf (stderr,
		 "Ignr: recursive proc, dropping '%s' from rm %s(%ld)\n",
		 msg, room[msgrm].name, msgrm);
      }
      
      else
      { if (msgrm != hererm) { dropped = 0; msgrm = hererm; }

	/* If too many messages to queue, at least send to log */
	if (dropped >= MAXQ)
	{ fprintf (stderr, "Ignr: dropping message '%s' from %s(%ld)\n",
		   msg, room[msgrm].name, msgrm);

	}
	
	/* Add the message to the queue, only log if not terse */
	else
        { strncpy (msgqueue[dropped++], msg, MSGSIZ);
	  if (!terse)
	  { fprintf (stderr, "Ignr: queuing message '%s' from %s(%ld)\n",
		     msg, room[msgrm].name, msgrm);
	  }
	}
      }
    }

    return;
  }

  if (dropped > 0)
  { inproc++;

    if (hererm == msgrm)
    { for (i=0; i<dropped; i++) readmsg (msgqueue[i]); }
    else
    { for (i=0; i<dropped; i++)
      { if (stlmatch (msgqueue[i], "You sense") ||
	    ematch (msgqueue[i], "killed you!") ||
	    stlmatch (msgqueue[i], "Your insurance policy pays"))
        { readmsg (msgqueue[i]); }
	else
	{ fprintf (stderr, "Ignr: dropping msg '%s' from %s(%ld)\n",
		   msgqueue[i], room[msgrm].name, msgrm);
	}
      }
    }
    
    inproc--;
    dropped = 0, msgrm = -1;
  }

  if (msg) readmsg (msg);
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
  { crash_robot ("malloc returns NULL in makestring"); }
}

/****************************************************************
 * ralloc: Malloc replacement for debugging
 ****************************************************************/

char *ralloc (len)
long len;
{ char *str;

  str = (char *) malloc (len);

  if (debug)
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
  char msg[TOKSIZ];

  sprintf (msg, fmt, a1, a2, a3, a4);

  sendmud ("@desc me = %s has crashed.", myname);
  sendmud ("QUIT");
  checkpoint ();
  now = time (0);
  fprintf (stderr, "\n\nCrash at %15.15s: %s\n", ctime (&now)+4, msg);
  abort ();
}

/****************************************************************
 * time_dur: 
 ****************************************************************/

char *time_dur (dur)
long dur;
{ long cnt = dur;
  char *units = "seconds", *s;
  static char buf[SMABUF];
    
  if (dur > (365 * DAYS))	return ("a long time");

  if (dur < 101)		{ cnt = dur; units = "second"; }
  else if (dur < 6001)		{ cnt = dur/MINUTES; units = "minute"; }
  else if (dur < 120000)	{ cnt = dur/HOURS; units = "hour"; }
  else if (dur < 1200000)	{ cnt = dur/DAYS; units = "day"; }
  else				{ cnt = dur/(7*DAYS); units = "week"; }

  sprintf (buf, "%ld %s%s", cnt, units, (cnt == 1) ? "" : "s");

  return (s = buf);
}

/****************************************************************
 * lcstr: lower case a string
 ****************************************************************/

char *lcstr (str)
char *str;
{ register char *s, *t;
  static char buf[TOKSIZ];

  for (s=str, t=buf; *s; )
  { *t++ = isupper (*s) ? tolower (*s++) : *s++; }
  
  *t = '\0';

  return (t = buf);  
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

    if (stlmatch (buf, "home"))
    { fprintf (stderr, "Warn: typing '%s'\n", buf);
      movemud (buf);
    }

    else if (stlmatch (buf, "go "))
    { movemud (buf + 3); }

    else if (stlmatch (buf, "give ") ||
	     stlmatch (buf, "page ") ||
	     stlmatch (buf, "kill ") ||
	     stlmatch (buf, "say ") ||
	     stlmatch (buf, "QUIT") ||
	     stlmatch (buf, "WHO") ||
	     stlmatch (buf, "@") ||
	     *buf == ':' ||
	     *buf == '"')
    { reply (":decides not to type <%s>.", buf); }

    else
    { 
      command (":types <%s>", buf);
      reply (buf);
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
      if (streq (pagedby, "<EXPLORING>"))
      { /* Dont say anything */ }
      else if (streq (pagedby, "<WHIM>") ||
	       streq (pagedby, "<HOME>") ||
	       streq (pagedby, "<COMMAND>"))
      { if (takingnotes && !quiet)
	{ command (":scribbles on %s pad.", male ? "his" : "her"); }
      }
      else if (pl >= 0 && player[pl].present)
      { reply ("\"Here I am, %s.", pagedby); 
	strcpy (speaker, pagedby);
	speaktime = now;
	spoke_player (speaker);
	if (!nextwait) nextwait = 180;
      }

      clear_page ();
      return (1);
    }
    else
    { dir = NULL;

      if (now > pagedat + 10 * MINUTES)
      { fprintf (stderr, "%s: timing out search for %s in %s after %s.\n",
		 *pagedby == '<' ? "Path" : "Page", pagedby,
		 room[pagedto].name, time_dur (now - pagedat));
	clear_page (); return (1);
      }

      else if ((from = hererm) < 0 ||			/* find here */
	       (to = pagedto) < 0 ||			/* find there*/
	  (dir = find_path (from, to, NEXTMOVE)) == NULL) /* find path */
      { 
	fprintf (stderr, "Path: couldn't find path (%s)%ld -> (%s)%ld\n",
		   here, from, room[pagedto].name, pagedto);

	if (from == to)
	{ crash_robot ("In paging, from == to == %ld, room_match failed",
			from);
	}

	if (to >= 0 &&
	    (homerm == to) || (dir = find_path (homerm, to, NEXTMOVE)))
	{ if (homerm != to)
	  { fprintf (stderr,
		     "Path: could find path %ld -> %ld (%s), going 'home'\n",
		     homerm, to, dir);
	  }
	  else
	  { fprintf (stderr, "Warn: going home from %ld to %ld\n", hererm, to); }

	  movemud ("home");
	  strcpy (here, home);
	  return (1);
	}

	if (to != from)
	{ fprintf (stderr,
		   "%s: %s for %s in %s(%ld) (from %ld, to %ld, dir %s).\n",
		   *pagedby == '<' ? "Path" : "Page",
		   "giving up search", pagedby, room[pagedto].name,
		   pagedto, from, to, dir ? dir : "(nil)");
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
  { fprintf (stderr, "Page: redundant page by %s to %s from %s(%ld)\n",
	     caller, loc, room_name (hererm), hererm);
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
  { if ((pl = find_player (caller)) && player[pl].present)
    { reply ("\"I'm right here, %s.", caller); }
    else if (pennies > MINPEN)
    { pennies--; reply ("page %s = 0", caller); psynch (); }
    nextwait = 180;
    return (1);
  }

  if (page_okay (caller, loc) == 0)
  { return (1); }

  /* If we are busy taking notes, say so */
  if (!isowner (caller) && takingnotes)
  { reply ("\"Oh dear, %s is paging me from %s, but %s.",
	   caller, loc, "I'm busy taking notes");

    if (pennies > MINPEN)
    { pennies -= 2;
      reply ("page %s = 0", caller); reply ("page %s = 0", caller);
      psynch ();
    }
    
    return (1);
  }

  /* If we are not taking calls, say so */
  if (!isowner (caller) && !paging)
  { reply ("\"Oh dear, %s is paging me from %s, but %s.",
	   caller, loc, "I'm not taking calls right now");

    if (pennies > MINPEN)
    { pennies -= 2;
      reply ("page %s = 0", caller); reply ("page %s = 0", caller);
      psynch ();
    }
    
    return (1);
  }

  /* If we are ignoring player, say so */
  if (!isowner (caller) && is_jerk (caller))
  { reply ("\"Oh dear, %s is paging me from %s, but %s.",
	   caller, loc, "I'm ignoring jerks");
    
    return (1);
  }

  /* If we are already on a call, say so */
  if (!isowner (caller) && *pagedby && *pagedby != '<' &&
      !streq (caller, pagedby))
  { reply ("\"Oh dear, %s is paging me from %s, but %s to %s for %s.",
	   caller, loc, "I'm on my way", room[pagedto].name, pagedby);

    if (pennies > MINPEN)
    { pennies -= 2;
      reply ("page %s = 0", caller); reply ("page %s = 0", caller);
      psynch ();
    }
    
    return (1);
  }

  /* If paged to home, go there directly */
  if (rm == homerm)
  { if (*speaker && speaktime + 60 < now || !quiet)
    { reply ("\"Excuse me, %s is paging me from %s.",
	     caller, room_name (rm));
    }
    pennies--;
    reply ("page %s = 0", caller);
    hanging = 0; pgoal = 0;
    fprintf (stderr, "Warn: paged to home by %s, going 'home'\n", caller);
    strcpy (typecmd, "go home");
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
    { pennies -= 2;
      reply ("page %s = 0", caller); reply ("page %s = 0", caller);
      psynch ();
    }
    
    return (1);
  }

  /* If we do not have a path, say so */
  if (find_path (hererm, rm, NEXTMOVE) == NULL &&
      find_path (homerm, rm, NEXTMOVE) == NULL)
  { if (!quiet)
    { reply ("\"Oh dear, %s is paging me from %s, but %s.",
	     caller, room_name (rm), "I don't know how to get there");
    }

    fprintf (stderr,
	     "Page: can't find path from %s(%ld) or %s(%ld) to %s(%ld)\n",
	     here, hererm, home, homerm, room[rm].name, rm);

    if (pennies > MINPEN)
    { pennies -= 2;
      reply ("page %s = 0", caller); reply ("page %s = 0", caller);
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
	   pagedby, room[pagedto].name, pagedto,
	   room[pagedfrom].name, pagedfrom);

  if (*speaker && speaktime + 240 > now)
  { reply ("\"Excuse me, %s is paging me from %s.",
	   pagedby, room_name (pagedto));
  }
  hanging = 0; pgoal = 0;

  /* Page back a single time to signal caller */
  if (pennies > MINPEN || isowner (caller))
  { pennies--;
    reply ("page %s = 0", caller);
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
	       room[meetingroom].name, meetingroom);
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
    { hangaround (300);
      command (":scribbles on %s pad.", male ? "his" : "her");
      return (1);
    }
  }

  return (0);
}

/****************************************************************
 * reset_dead: We died, reset
 ****************************************************************/

reset_dead ()
{
  if (dead)
  { fprintf (stderr, "Dead: killed by %s, restarting\n", killer);

    dead = 0;
    nextwait = 0;

# ifdef NOLONGJMP
    hererm = homerm;
    strcpy (here, home);
    strcpy (desc, homedesc);
    movemud ("home");
# else
    reset_robot ("Killed, restarting");
# endif

    return (1);
  }

  return (0);
}

/****************************************************************
 * special_mode: Handle special TinyMUD output (LOOK, SCORE, WHO)
 ****************************************************************/

special_mode (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{
  /* Ignore message that might be spoofing, or are junk */
  if (msg == NULL || *msg == '\0')
  { ignore_msg (msg); return (1); }
  
  if (stlmatch (msg, "<<") || stlmatch (msg, ">>"))
  { ignore_msg (msg); return (1); }

  if (stlmatch (msg, "You say \"") ||
      stlmatch (msg, "You whisper \"") ||
      stlmatch (msg, myname))
  { return (1); }

  /*---------------- SCORE mode output ----------------*/
  if (mode == SCORE)
  { if (MATCH (msg, "You have * pennies.") ||
        MATCH (msg, "You have * penny."))
    { if (isdigit (res1[0]))
      { pennies = atoi (res1); }
      else
      { fprintf (stderr, "Bgus: '%s', non integer pennies.\n", msg); }
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
  { if (stlmatch (msg, "Current Players:"))
    { clear_active (); }
  
    else if (MATCH (msg, "* idle * seconds"))
    { if (debug) fprintf (stderr, "Who:  [%s] %s seconds\n", res1, res2);
      idle_player (res1, atoi (res2));
    }
    
    return (1);
  }


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

  /* Check the WHO list once a minute */
  if ((now = time (0)) - lastwsynch > 240) wsynch ();

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
    /* Look at everyone (sleepers twice a day, others every 10 minutes) */
    for (pl = 0; pl<players; pl++)
    { if (player[pl].present &&
	  (now - player[pl].lastlook) >
	   (player[pl].active ? (10 * MINUTES) : (12 * HOURS)))
      { look_at_thing (player[pl].name); }
    }

    /* Do we need to checkpoint our map and players files? */
    if ((now > (lastcheck + checkfreq)) || (newrooms > 10) || (newexits > 500))
    { if (*speaker && speaktime + 120 > now)
      { command ("\"Excuse me a minute, I have to save my map, %s.",
		 speaker);
      }
      else
      { command ("\"Excuse me a minute, I have to save my map."); }
      command ("@describe me = %s is busy saving %s map.",
	       myname, male ? "his" : "her");
      checkpoint ();
      command ("@describe me = %s.", mydesc);
      command ("\"Okay, I saved my map.");
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
  
  if (*desc)
  { fprintf (stderr, "Desc: %-72.72s\n", desc); }

  if (*contents)
  { fprintf (stderr, "Cont: %-72.72s\n", contents); }

  player[me].active = player[me].present = 0;
  for (pl = 0, printed=0; pl<players; pl++)
  { if (player[pl].active && player[pl].present)
    { if (!printed++) fprintf (stderr, "Pres: (%ld) ", alone);
      fprintf (stderr, " %s", player[pl].name);
    }
  }

  if (printed) fprintf (stderr, "\n");

  for (pl = 0, printed=0; pl<players; pl++)
  { if (!player[pl].active && player[pl].present)
    { if (!printed++) fprintf (stderr, "Inac:");
      fprintf (stderr, " %s", player[pl].name);
    }
  }

  if (printed) fprintf (stderr, "\n");

  fprintf (stderr, "\n");

  printedloc++;
}

/****************************************************************
 * ematch: Match end of string
 ****************************************************************/

ematch (big, small)
register char *big, *small;
{ register long blen = strlen (big), slen = strlen (small);

  return (streq (big+blen-slen, small));
}

/****************************************************************
 * car: Returns a pointer to a static containing the first
 * word of a string.
 ****************************************************************/

# define isword(C) (isalpha (C) || index ("'-", (C)))

char *car (str)
register char *str;
{ static char buf[MSGSIZ];
  register char *s = buf;

  if (!str) return (NULL);

  while (*str && !isword (*str)) str++;
  while (*str && isword (*str)) *s++ = *str++;
  *s = '\0';

# ifndef CRAZY
  if (stlmatch (buf, "OUTPUT")) crash_robot ("bad car\n");
# endif
  
  return (s = buf);
}

/****************************************************************
 * cdr: all but the first word
 ****************************************************************/

char *cdr (str)
register char *str;
{ register char *p = str;

  if (!str) return (NULL);

  /* Skip initial white space */
  while (*str && !isword (*str))  str++;

  if (!*str) return (NULL);

  /* Skip over one word */  
  while (*str && isword (*str)) str++;

  if (!*str) return (NULL);

  /* return pointer to second word */  
  return (str);
}

/****************************************************************
 * last: Returns a pointer to the last word of a string
 ****************************************************************/

char *last (str)
register char *str;
{ static char buf[MSGSIZ];
  register char *s, *t = buf;

  if (!str) return (NULL);

  for (s=str; *s; s++) ;
  while (s>str && !isword (s[-1])) s--;
  while (s>str &&  isword (s[-1])) s--;
  while (s>str &&  isword (*s))    *t++ = *s++;
  *t = '\0';

  return (s = buf);
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

  /* Jump to the top of the main loop */
  longjmp (start_state, 0);
}

/****************************************************************
 * fatal: Fatal error message
 ****************************************************************/

fatal (fmt, a1, a2, a3, a4)
char *fmt, *a1, *a2, *a3, *a4;
{ 
  close (tmud);
  fprintf (stderr, fmt, a1, a2, a3, a4);  
  exit (1);
}

/****************************************************************
 * is_me: True if string matches my name
 ****************************************************************/

is_me (name)
char *name;
{ char buf1[MSGSIZ], buf2[MSGSIZ];

  strcpy (buf1, lcstr (name));
  strcpy (buf2, lcstr (myname));
  return (streq (buf1, buf2));
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
