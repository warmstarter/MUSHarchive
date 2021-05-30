/****************************************************************
 * colin: TinyMUD automaton.  Test Robot
 *
 * HISTORY
 * 09-Jan-94  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Fourteenth animal release.  New public release based on
 *	Julia.
 *
 * 17-Oct-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Tenth ceremonial release.  Mods for planck DB switching.
 *
 * 05-May-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Seventh sequential release.
 *	Mods for TinyHELL.
 *
 * 26-Feb-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Sixth experimental release. Changed file formats, added lots
 *	of info to room memory.  Found a memory allocation bug.
 *
 ****************************************************************/

# include <stdio.h>
# include <ctype.h>
# include <sys/types.h>
# include <sys/ioctl.h>
# include <sys/socket.h>
# include <setjmp.h>
# include <netinet/in.h>
# include <netdb.h>
# include <signal.h>
# include <ctype.h>
# include <varargs.h>
# include <time.h>

# include "robot.h"
# include "vars.h"
# include "colin.h"

long period;
long havebook = 0;

/* Vars for defense against killers */
char kname[MSGSIZ] = "";
char kshape[MSGSIZ] = "";
long killed = 0;
long nosave = 0;

/* Not used in Colin */
long contest_mode = 0;
long tty_interface = 0;

char PI_500[512] = "3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651328230664709384460955058223172535940812848111745028410270193852110555964462294895493038196442881097566593344612847564823378678316527120190914564856692346034861045432664821339360726024914127372458700660631558817488152092096282925409171536436789259036001133053054882046652138414695194151160943305727036575959195309218611738193261179310511854807446237996274956735188575272489122793818301194912";


long ispueblo = 0;

/****************************************************************
 * setconfig: These values are defined in colin.h
 ****************************************************************/

setconfig ()
{
  male = MALE;
  whoami = WHOAMI;
  owner = OWNER;
  creation = CREATION;
  if (myname == NULL)	myname = MYNAME;
  if (mudhost == NULL)	mudhost = MUDHOST;
  if (mudport == 0)	mudport = MUDPORT;
}

/****************************************************************
 * robot main loop
 ****************************************************************/

robot ()
{ 

  if ((streq (world, "Druid")) || (streq (world, "Pueblo"))) {ispueblo = 1;}
  if (ispueblo) {  waitfor ("Thought for the day:"); }

  now = time (0);
  
  if (mapfile == NULL)
  { mapfile = (streq (world, ALT1_WORLD)	? ALT1_MAPFILE :
	       streq (world, ALT2_WORLD)	? ALT2_MAPFILE :
						  ALT3_MAPFILE);
  }

  if (plyfile == NULL)
  { plyfile = (streq (world, ALT1_WORLD)	? ALT1_PLYFILE : 
	       streq (world, ALT2_WORLD)	? ALT2_PLYFILE : 
						  ALT3_PLYFILE);
  }
  
  if (streq (world, ALT1_WORLD))
  { mapfile = ALT1_MAPFILE;
    plyfile = ALT1_PLYFILE;
    ismuck = 0;
  }
  else if (streq (world, ALT2_WORLD))
  { mapfile = ALT2_MAPFILE;
    plyfile = ALT2_PLYFILE;
    ismuck = 0;
  }
  else if (streq (world, ALT3_WORLD))
  { mapfile = ALT3_MAPFILE;
    plyfile = ALT3_PLYFILE;
    ismuck = 1;
    statuscmd = STATUS_CMD;
    pagecmd = PAGE_CMD;
    posecmd = POSE_CMD;
    scorecmd = SCORE_CMD;
  }
  else
  { mapfile = UNK_MAPFILE;
    plyfile = UNK_PLYFILE;
    ismuck = 0;
  }
  
  
  read_players (plyfile);
  read_map (mapfile);
  check_players ();

  /* Set up signal handling */
  signal (SIGHUP, (void *) onintr);
  signal (SIGINT, (void *) onintr);
  
  /* Log in to the MUD. */
  sendmud ("connect %s %s", MYNAME, MYPASS);

  /* Save state for resetting robot */
  setjmp (start_state);

  /* Initialize the robot */
  start_up ();

  while (playing)
  { char *dir;
    long pl, from, to;

    /* Check who is here and where we are */
    check_time ();    
    do_special_check ();
    do_msgs ();

    /* Hi priority actions */
    if (do_warn ())		continue;
    if (do_typecmd ())		continue;
    if (do_page ())		continue;
    if (do_notes ())		continue;
    if (do_findold (14 * DAYS))	continue;
    if (do_explore ())		continue;

    /* Long wait */
    do_twiddle ();

    /* Low priority actions */
    if (do_wander ())	continue;

    /* Reset if dead */
    if (dead)		reset_dead ();
  }

  sendmud ("@quit");
  sleep (2);

  now = time (0);
  fprintf (stderr, "\nExited main play loop at %15.15s.\n",
	   ctime (&now) + 4);
  quit_robot ();
}

/****************************************************************
 * start_up: Initial commands
 ****************************************************************/

start_up ()
{ static int locked = 0;

  /* Special features of some worlds */
  if (streq (world, "TinyHELL") ||
      streq (world, "Islandia"))
  { pagedmsgs++; }

  /* Reset character state */
  sendmud ("%s %s", opre, outpre);
  sendmud ("%s %s", osuf, outsuf);
  if (!locked) { command ("@lock me with me"); locked++; }
  command ("@describe me as %s.", WHOAMI); strcpy (mydesc, WHOAMI);
  naked = 0;
  command ("@realname Homonculous I. Newt"); 
  command ("@page_absent me is Newt is sleeping on the job, how rude!!");
  command ("@page_absent me is Newt is sleeping on the job, how rude!!");

  me = add_player (MYNAME);
  wsynch ();
  havebook = 0;
  movemud ("straighthome");
  strcpy (home, here);
  strcpy (homedesc, desc);
  homerm = add_room (home, desc);
  recrm = leads_to (homerm, "rec");
  if (recrm == homerm) recrm = -1;
  psynch ();
  usynch ();

  msgtype = M_UNKNOWN;
  unlogged (":wakes up and picks his head off his desk."); atdesk++;

  awake++;
  lastcheck = now = time (0);
}

/****************************************************************
 * do_special_check:
 ****************************************************************/

do_special_check ()
{ static lastshapecheck = 0;

  now = time (0);
  strcpy (postcmd, "");
  
  return (1);
}

/****************************************************************
 * do_warn: Warn the terminators target, if present
 ****************************************************************/

do_warn ()
{
  /* Warn target player */
  if (termtarg >= 0 &&
      player[termtarg].present &&
      player[termtarg].active &&
      !termwarn &&
      !dead)
  { strcpy (speaker, player[termtarg].name);
    spoke_player (player[termtarg].name);
    reply ("\"Oh by the way, %s, %s in %s about %s ago.",
	   player[termtarg].name,
	   "Terminator was looking for you", 
	   termloc, time_dur (time (0) - termat));
    termwarn = time (0);
    
    return (1);
  }

  return (0);
}


/****************************************************************
 * do_twiddle: Wait around a while
 ****************************************************************/

do_twiddle ()
{
  /* Sit around and twiddle thumbs */
  if (!dead && !*typecmd)
  { long dur;

    if (nextwait)		dur = nextwait;
    else if (exploring)		dur = (randint (speed) + 1) * 5;
    else			dur = randint (240) + 120;

    nextwait = 0;
    psynch ();
    hangaround (dur);

    return (1);
  }

  return (0);
}

/****************************************************************
 * do_wander: Wander around
 ****************************************************************/

do_wander ()
{
  /* If not exploring, head for his office */
  if (!exploring && !nextwait && ! *pagedby &&
      ! *typecmd && ! dead && !takingnotes)
  { long dest;
    long dice;

    do
    { dice = randint (100);
      dest = -1;

      if (dice < 50)
      { dest = homerm; }
      else if (dice < 70)
      { dest = close_room ("town", NULL); }
      else if (dice < 80)
      { dest = close_room ("library", NULL); }

      if (dest < 0)
      { do { dest = randint (rooms); } while (!room[dest].name); }
    } while (dest < 0 || (dest == hererm || randint (100) < 90));

    now = time (0);

    fprintf (stderr, "Wndr: [%15.15s] %s(%d)\n",
	       ctime (&now) + 4, room_name (dest), dest);

    disengage (room_name (dest));

    pagedto = dest;
    pagedfrom = hererm;
    pagedat = time (0);

    if (dest == homerm)	{ strcpy (pagedby, "<HOME>"); }
    else		{ strcpy (pagedby, "<WHIM>"); }

    return (1);
  }

  return (0);
}

/****************************************************************
 * readmsg: Handle messages
 ****************************************************************/

readmsg (msg)
char *msg;
{ char lcmsg[TOKSIZ], name[MSGSIZ], place[MSGSIZ];
  register char *s, *t;
  long oldmsgtype;
  long pl;

  /* Drop messages that happen "between rooms".  We should queue them */
  if (inmove) return;

  /* Stack message type */
  oldmsgtype = msgtype;
  msgtype = M_UNKNOWN;
  
  /* Save lower case copy of message */
  strcpy (lcmsg, lcstr (msg));

  /* Save name for replies */
  for (s=msg, t=name; *s && !isspace (*s); ) *t++ = *s++;
  *t = '\0';

  /*---- Standard message handling.  Try each batch of replies in turn ----*/
  if (special_mode   (name, msg, lcmsg))
  { msgtype = oldmsgtype; return; }

  /* Ignore most messages until we are really awake */
  if (!awake)
  { msgtype = oldmsgtype; return; }

  if (spoof_msg (name, msg, lcmsg) ||
      says_msg       (name, msg, lcmsg) ||
      action_msg     (name, msg, lcmsg))
  { msgtype = oldmsgtype; return; }

  /* Ignore message altogether, then */
  msgtype = oldmsgtype; return;
}

/****************************************************************
 * spoof_msg: Check for messages from robots own name
 ****************************************************************/

spoof_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{
  if (is_me (name))
  { if (!printedloc) print_locn ();
    fprintf (stderr, "Spof: saw message '%s'\n", msg);
    msgtype = M_UNKNOWN;
    reply (":detects spoofing, fake message was '%s'", msg);
    return (1);
  }
  else
  { return (0); }
  
}

/****************************************************************
 * says_msg: Handle spoken messages from other players (that is
 *	     messages like 'Foo says "Hello."').
 ****************************************************************/

says_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ char buf[TOKSIZ], lcbuf[TOKSIZ], *is_tell();
  char sname[TOKSIZ];
  register char *s;  

  /* skip over first word (should be the name) */
  for (s=msg; *s && !isspace (*s); s++);
  while (isspace (*s)) s++;
  
  /* Check for and remove trailing 's */
  if (ismuck && strlen (name) > 2 && streq (name+strlen(name)-2, "'s"))
  { strcpy (sname, name);
    sname[strlen (name) - 2] = '\0';
    if (find_player (sname) >= 0)
    { name[strlen (name) - 2] = '\0';
    }
  }
  
  /* Verify that this is a speech action */
  if (!awake)			     { return (0); }
  else if (stlmatch (s, "says"))     { s += 4; msgtype = M_SPOKEN; }
  else if (ismuck && index (s, '"')) { while (*s && *s != '"') s++;
				       msgtype = M_PAGE; pagedmsgs++;
				     }
  else if (stlmatch (s, "whispers")) { s += 8; msgtype = M_WHISPER; }
  else if (stlmatch (s, "pages,"))   { s += 5; msgtype = M_PAGE; pagedmsgs++; }
  else if (stlmatch (s, "shouts"))   { s += 6; msgtype = M_SHOUT; }
  else				     { return (0); }

  /* Count messages/actions in this room */
  room[hererm].msgsum++;

  /*
   * Strip leading and trailing quotes, punctuation, and whitespace,
   * put message in buf, save lower case version in lcbuf.
   */

  while (isspace (s[0]) || index ("`'\".,:;!?", s[0])) s++;
  strcpy (buf, s);
  s = buf + strlen (buf);
  while (isspace (s[-1]) || index ("`'\".,:;!?", s[-1])) s--;
  *s = '\0';

  strcpy (lcbuf, lcstr (buf));

  /*-------- Log the message --------*/
  if (!printedloc) print_locn ();
  if ((s = is_tell (lcbuf)) && !isowner (s) && !isowner (name))
  { fprintf (stderr, "Says: %s[%s] <msg to %s>\n",
	     msgtype == M_SPOKEN ? "" :
	     msgtype == M_WHISPER ? "whisp" :
	     msgtype == M_PAGE ? "pages" :
	     msgtype == M_SHOUT ? "shout" : "poses",
	     name, s);
  }
  else
  { fprintf (stderr, "Says: %s[%s] \"%s\"\n",
	     msgtype == M_SPOKEN ? "" :
	     msgtype == M_WHISPER ? "whisp" :
	     msgtype == M_PAGE ? "pages" :
	     msgtype == M_SHOUT ? "shout" : "poses",
	     name, buf);
  }

  if (awake && find_player (name) >= 0 && !reserved (name))
  { if (msgtype < M_PAGE) saw_player (name, here, desc);
  
    /* New: do not gossip about whispers or pages */
    if (msgtype == M_SPOKEN) heard_player (name, buf);
  }

  speaktime = time (0);

  /*---- Verify that this message was meant for the robot ----*/
  if ((msgtype != M_SPOKEN && msgtype != M_SHOUT) || to_me (name, buf, lcbuf))
  { check_stay ();

    /* Now try various speech rule sets */
    if (hi_priority  (name, buf, lcbuf) ||
        small_talk   (name, buf, lcbuf) ||
        standard_msg (name, buf, lcbuf) ||
        lo_priority  (name, buf, lcbuf) ||
        polite_msg   (name, buf, lcbuf) ||
        sorry_msg    (name, buf, lcbuf))
    { return (1); }
  }

  /*---- The speaker has started talking to someone else ----*/
  else if (streq (name, speaker))
  { strcpy (speaker, ""); }

  /* Message was "says", so we match it by ignoring it */
  return (1);
}

/****************************************************************
 * hi_priority: Handle messages that must be checked first. That
 *		does not mean they are particularly important.
 ****************************************************************/

hi_priority (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ long dur, pl;
  char *item, *s, *is_tell();

  /*---- Colin wont talk to his last murderer ----*/
  if (!isowner (name) && (dur = killed_me_today (name)))
  { if (dur && dur < 10) return (1);
  
    strcpy (speaker, name);

    switch (randint (4))
    { case 0: reply ("\"I don't talk to murderers!"); break;
      case 1: reply (":ignores %s.", name); break;
      case 2: reply (":steels himself for another attack from %s.", name);
		break;
      default: ; /* totally ignore them */
    }
      return (1);
  }

  /*---- A message to another player ----*/
  else if (s = is_tell (lcmsg))
  { char buf[BIGBUF];

    strcpy (speaker, name);
    spoke_player (name);

    if ((pl = find_player (s)) >= 0)
    { sprintf (buf, "%s said '%s'", name, msg);
      add_msg (pl, now, buf);
      if (PLAYER_GET(pl, PL_HAVEN))
      { reply ("\"Message saved, %s, but %s %s. When I next see %s, %s",
	       name, player[pl].name,
	       "was not accepting pages last time I checked",
	       player[pl].name, "I'll relay the message by whispering.");
      }
      else
      { unlogged ("\"Message for %s saved, %s.", player[pl].name, name); }
    }
    else
    { reply ("\"I've never heard of player %s, %s.", s, name); }

    return (1);
  }

  /*---- Tell Colin to quit ----*/
  else if (MATCH (lcmsg, "*bye*") ||
	   MATCH (lcmsg, "*good*night*") ||
	   MATCH (lcmsg, "*good*day*") ||
	   MATCH (lcmsg, "*adios*") ||
	   MATCH (lcmsg, "*ciao*") ||
	   MATCH (lcmsg, "*auf*wieder*") ||
	   MATCH (lcmsg, "*shut*up*") ||
	   MATCH (lcmsg, "*shutdown*") ||
	   MATCH (lcmsg, "*later*guy*") ||
	   MATCH (lcmsg, "*later*dude*") ||
	   MATCH (lcmsg, "*later*people*") ||
	   MATCH (lcmsg, "*later*d00*") ||
	   MATCH (lcmsg, "*be*quiet*") ||
	   MATCH (lcmsg, "*stop*talk*") ||
	   MATCH (lcmsg, "*have a*nice*day*") ||
	   MATCH (lcmsg, "*hasta*luego*") ||
	   MATCH (lcmsg, "*hasta*l*vista*") ||
	   MATCH (lcmsg, "*stop*talk*") ||
	   MATCH (lcmsg, "*sayonara*") ||
	   MATCH (lcmsg, "*i'm*leaving*") ||
	   MATCH (lcmsg, "*i'm*out*here*") ||
	   MATCH (lcmsg, "*hasta*vista*"))
  { if (isowner (name) &&
        (ismuck && *code && sindex (msg, code) ||
	 trusting && MATCH (lcmsg, "bye*colin")))
    { fprintf (stderr, "Quitting on command of %s\n", name);
      sendmud ("\"Goodbye, %s, I'm going home to sleep.", name);
      sendmud ("straighthome");
      sendmud ("@describe %s = %s", DESK, CLOSEDMSG);
      sendmud (":puts his head down and goes to sleep.");
      sendmud ("@describe me = %s", POWEROFF); strcpy (mydesc, POWEROFF);
      sendmud ("@quit");
      spoke_player (name);
      quit_robot ();
    }
    else /* They probably mean they are leaving */
    { static lastgoodbye = 0;

      if (now - lastgoodbye > 30)
      { spoke_player (name);
        strcpy (speaker, name);
        speaktime = 0;
        reply ("\"Goodbye, %s.", name);
        strcpy (speaker, "");
	lastgoodbye = now;
        return (1);
      }
    }
  }

  /*---- Set code word ----*/
  else if (MATCH (lcmsg, "*what*is*code*word*") && isowner (name))
  { strcpy (speaker, name);
    spoke_player (name);

    if (msgtype < M_WHISPER) msgtype = M_WHISPER;
    reply ("\"The code word is %s, %s.", strcpy (code, codeword ()), name);
    codeset = now;
    
    return (1);
  }

  /*---- Set code word ----*/
  else if (isowner (name) &&
	   (MATCH (lcmsg, "*forget*code*") ||
	    MATCH (lcmsg, "*erase*code*") ||
	    MATCH (lcmsg, "*invalidate*code*")))
  { strcpy (speaker, name);
    spoke_player (name);

    strcpy (code, "");
    codeset = now;

    if (msgtype < M_WHISPER) msgtype = M_WHISPER;
    reply ("\"Code word cleared, %s.", name);
    
    return (1);
  }

  /*---- Checkpoint files ----*/
  else if (MATCH (lcmsg, "*core*dump*colin*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { crash_robot ("Core dumped by %s: %s", name, msg); }
    return (1);
  }

  /*---- Checkpoint files ----*/
  else if (MATCH (lcmsg, "wait") ||
	   MATCH (lcmsg, "*, wait") ||
	   MATCH (lcmsg, "wait, *") ||
	   MATCH (lcmsg, "stay") ||
	   MATCH (lcmsg, "*, stay") ||
	   MATCH (lcmsg, "stay, *") ||
	   MATCH (lcmsg, "stop") ||
	   MATCH (lcmsg, "*, stop") ||
	   MATCH (lcmsg, "stop, *") ||
	   MATCH (lcmsg, "*don't go*") ||
	   MATCH (lcmsg, "*don't leave*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner (name) ||
	(!*pagedby ||
	 streq (pagedby, "<WHIM>") ||
	 streq (pagedby, "<HOME>") ||
	 streq (pagedby, "<EXPLORING>")))
    { if (nextwait < 180) nextwait = 180; 
      switch (randint (4))
      { case 0: reply ("\"Okay, %s, I'll stay.", name); break;
	case 1: reply ("\"I'll stay a while, then, %s.", name); break;
	case 2: reply ("\"I can stay a few minutes, I guess, %s.", name);
		break;
	case 3: reply ("\"For you, %s, sure....", name); break;
      }
    }
    else
    { reply ("\"Sorry, %s, I really must be going.", name); }
    return (1);
  }

  /*---- A query about 'who belong' ----*/
  else if (MATCH (lcmsg, "*who*you*belong*") ||
	   MATCH (lcmsg, "*whose*are*you*") ||
	   MATCH (lcmsg, "*who*you*owner*") ||
	   MATCH (lcmsg, "*who*work*") ||
	   MATCH (lcmsg, "*who*built*you*") ||
	   MATCH (lcmsg, "*who*made*you*") ||
	   MATCH (lcmsg, "*who*created*you*") ||
	   MATCH (lcmsg, "*who*you*creator*") ||
	   MATCH (lcmsg, "*who*wrote*") ||
	   MATCH (lcmsg, "*who*own*juli*") ||
	   MATCH (lcmsg, "*who*juli*owner*") ||
	   MATCH (lcmsg, "*who*s*you*amway*") ||
	   MATCH (lcmsg, "*who*s*reinc*elvis*") ||
	   MATCH (lcmsg, "*who*s*third*hacker*") ||
	   MATCH (lcmsg, "*who*own*you*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner ("Fuzzy"))
    { reply ("\"Fuzzy, of course, %s.", name); }
    else
    { reply ("\"Fuzzy created me, but I work for %s, %s.", owner, name); }
    return (1);
  }

  /*---- A query about 'how ' ----*/
  else if (MATCH (lcmsg, "*who*are*you*") ||
	   MATCH (lcmsg, "*what*is*you*"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"I am Colin, the Maas-Neotek Robot, %s", name);
    return;
  }

  /*---- A query about 'how ' ----*/
  else if (MATCH (lcmsg, "*give*me*tour*") ||
	   MATCH (lcmsg, "*take*me*tour*"))
  { strcpy (speaker, name);
    spoke_player (name);
    give_tour (lcmsg, name);
    return;
  }

  /*---- A query about lines ----*/
  else if (MATCH (lcmsg, "*would*you*like *,*") ||
	   MATCH (lcmsg, "*would*you*like *") ||
	   MATCH (lcmsg, "*how*you*like *,*") ||
	   MATCH (lcmsg, "*how*you*like *") ||
	   MATCH (lcmsg, "*do*you*like *,*") ||
	   MATCH (lcmsg, "*do*you*like *"))
  { 
    strcpy (speaker, name);
    spoke_player (name);

    switch (randint (2))
    { case 0: reply ("\"I'm not sure, %s.", name); break;
      case 1: reply ("\"I don't know, %s.", name); break;
    }
    
    return (1);    
  }

  /* Message not matched by this rule set */
  return (0);
}

/****************************************************************
 * small_talk: handle small talk, come-ons, put-downs, etc.
 ****************************************************************/

small_talk (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{
  /*---- Hug ----*/
  if (MATCH (lcmsg, "hug me*") ||
	   MATCH (lcmsg, "* hug me*") ||
	   MATCH (lcmsg, "*embrace me*") ||
	   MATCH (lcmsg, "*take*me*your*arm*") ||
	   MATCH (lcmsg, "*wrap*your*arm*"))
  { strcpy (speaker, name);
    spoke_player (name);
    
    switch (randint (2))
    { case 0: reply (":hugs %s.", name); break;
      case 1: zinger (":shakes %s's hand, instead.", name); break;
    }

    return (1);
  }

  /* Message not matched by this rule set */
  return (0);
}

/****************************************************************
 * lo_priority: Handle messages particular to a given robot
 *		This is the place to add your own replies and
 *		give your robot a personality.
 ****************************************************************/

# define FIRST_LAW \
"1. A Robot may not injure a human being, or, through inaction, \
allow a human being to come to harm."
# define SECOND_LAW \
"2. A robot must obey the orders given it by human beings except \
where such orders would conflict with the First Law."
# define THIRD_LAW \
"3. A robot must protect its own existence as long as such protection \
does not conflict with the First and Second Laws."
# define LAW_REF \
"Isaac Asimov, \"Runaround\", Astounding Science Fiction, March 1942."

lo_priority (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ long count;
  static long grouch = 0;

  /*---- A query about 'the laws of robotics' ----*/
  if ((sindex (lcmsg, "what is") ||
	    sindex (lcmsg, "what are") ||
	    MATCH (lcmsg, "*do*you*know*") ||
	    sindex (lcmsg, "*tell*") ||
	    sindex (lcmsg, "*describe*") ||
	    sindex (lcmsg, "*repeat*") ||
	    sindex (lcmsg, "*say*") ||
	    sindex (lcmsg, "*say*")) &&
	   (MATCH (lcmsg, "*three*laws*") ||
	    MATCH (lcmsg, "*three*rules*") ||
	    MATCH (lcmsg, "*asimov*law*") ||
	    MATCH (lcmsg, "*asimov*rule*") ||
	    MATCH (lcmsg, "*law*robot*") ||
	    MATCH (lcmsg, "*rule*robot*") ||
	    MATCH (lcmsg, "*zero*law*") ||
	    MATCH (lcmsg, "*0*law*") ||
	    MATCH (lcmsg, "*first*law*") ||
	    MATCH (lcmsg, "*1st*law*") ||
	    MATCH (lcmsg, "*second*law*") ||
	    MATCH (lcmsg, "*2nd*law*") ||
	    MATCH (lcmsg, "*third*law*") ||
	    MATCH (lcmsg, "*3rd*law*")))
  { int printed=0;

    strcpy (speaker, name);
    spoke_player (name);

    if (sindex (lcmsg, "zeroth") || sindex (lcmsg, "0"))
    { zinger ("\"The zeroth law is a myth, %s", name); return (1);}

    if (sindex (lcmsg, "first") || sindex (lcmsg, "1"))
    { zinger (": %s", FIRST_LAW); printed++; }

    if (sindex (lcmsg, "second") || sindex (lcmsg, "2"))
    { zinger (": %s", SECOND_LAW); printed++; }

    if (sindex (lcmsg, "third") || sindex (lcmsg, "3"))
    { zinger (": %s", THIRD_LAW); printed++; }

    if (!printed)
    { zinger (":%s", FIRST_LAW);
      zinger (":%s", SECOND_LAW);
      zinger (":%s", THIRD_LAW);
    }
    
    zinger (": --%s", LAW_REF);

    return (1);
  }

  /*---- Do you work here ----*/
  else if (MATCH (lcmsg, "*do you*here*") &&
	   (sindex (lcmsg, "work") ||
	    sindex (lcmsg, "live") ||
	    sindex (lcmsg, "stay")))
  { strcpy (speaker, name);
    spoke_player (name);

    if (hererm == homerm)
    { reply ("\"Yes, I work here, %s.", name); }
    else
    { reply ("\"No, I work in %s, %s.", room_name (homerm), name); }
    return;
  }

  /*---- Where do you work ----*/
  else if (MATCH (lcmsg, "*where*do you*") &&
	   (sindex (lcmsg, "work") ||
	    sindex (lcmsg, "live") ||
	    sindex (lcmsg, "hang out") ||
	    sindex (lcmsg, "stay")))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"I work in %s, %s.", room_name (homerm), name);
    return;
  }

  /*---- Pick commands ----*/
  else if ((MATCH (lcmsg, "*take *") ||
	    MATCH (lcmsg, "*get *") ||
	    MATCH (lcmsg, "*pick *")) &&
	   !sindex (lcmsg, "don't") &&
	   !sindex (lcmsg, "sorry"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"I'm sorry, %s, I don't pick things up.", name);
    return (1);
  }    

  /*---- Pick commands ----*/
  else if (MATCH (lcmsg, "*kill*") ||
	   MATCH (lcmsg, "*murder*") ||
	   MATCH (lcmsg, "*fight*"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"I'm sorry, %s, I don't like violence.", name);
    return (1);
  }    

  /*---- Are you a robot ----*/
  else if (MATCH (lcmsg, "*are*you*robot*") ||
	   MATCH (lcmsg, "*are*you*a bot*") ||
	   MATCH (lcmsg, "*are*you*an ai*") ||
	   MATCH (lcmsg, "*are*you*autom*") ||
	   MATCH (lcmsg, "*are*you*machine*") ||
	   MATCH (lcmsg, "*are*you*computer*") ||
	   MATCH (lcmsg, "*are*you*program*") ||
	   MATCH (lcmsg, "*are*you*simulati*") ||
	   MATCH (lcmsg, "*you*are*robot*") ||
	   MATCH (lcmsg, "*you*are*a bot*") ||
	   MATCH (lcmsg, "*you*are*an ai*") ||
	   MATCH (lcmsg, "*you*are*autom*") ||
	   MATCH (lcmsg, "*you*are*machine*") ||
	   MATCH (lcmsg, "*you*are*computer*") ||
	   MATCH (lcmsg, "*you*are*program*") ||
	   MATCH (lcmsg, "*you*are*simulati*") ||
	   MATCH (lcmsg, "*you* be *robot*") ||
	   MATCH (lcmsg, "*you* be *a bot*") ||
	   MATCH (lcmsg, "*you* be *an ai*") ||
	   MATCH (lcmsg, "*you* be *autom*") ||
	   MATCH (lcmsg, "*you* be *machine*") ||
	   MATCH (lcmsg, "*you* be *computer*") ||
	   MATCH (lcmsg, "*you* be *program*") ||
	   MATCH (lcmsg, "*you* be *simulati*") ||

	   MATCH (lcmsg, "*colin is *") &&
	   (sindex (res2, "robot") ||
	    sindex (res2, "program") ||
	    sindex (res2, "simulati") ||
	    sindex (res2, " bot") ||
	    sindex (res2, "automat") ||
	    sindex (res2, "machine") ||
	    sindex (res2, "computer")))
  { static long lasttold = 0;

    strcpy (speaker, name);
    spoke_player (name);
    
    if ((now - lasttold) > 2 * MINUTES )
    { zinger ("\"I am a Maas-Neotek robot, %s.", VERSION);
      lasttold = now;
    }
    else
    { reply (":nods."); }
    
    return (1);
  }    


  /*---- Are you wizard ----*/
  else if (MATCH (lcmsg, "*are*you*wizard*"))
  { sleep (2);

    strcpy (speaker, name);
    spoke_player (name);

    switch (randint (4))
    { case 0: zinger ("\"I am not a wizard, %s.", name); break;
      case 1: zinger ("\"I do not believe I'm a wizard, %s.", name); break;
      case 2: zinger ("\"I'd like to be a wizard, %s.", name); break;
      case 3: zinger ("\"I don't know, %s.", name); break;
    }
    return (1);
  }    

  /* Message not matched by this rule set */
  return (0);
}

/****************************************************************
 * sorry_msg: fail message.  Say "I did not understand"
 ****************************************************************/

sorry_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{

  /*---- Log the non-understood msg ----*/
  sorry_log (name, msg);

  /*---- Else ----*/
  if (randint (100) < 50 ||
      MATCH (lcmsg, "*, colin*") ||
      MATCH (lcmsg, "*colin: *") ||
      MATCH (lcmsg, "*colin, *"))
  { sleep (2);

    strcpy (speaker, name);
    spoke_player (name);
    
    if (msgtype < M_WHISPER) msgtype = M_WHISPER;

    reply ("\"I'm sorry, %s, I couldn't understand you.", name);
    return (1);
  }

  /* Message not matched by this rule set */
  return (0);
}

/****************************************************************
 * action_msg: handle actions (some involve TinyMUD world messages)
 ****************************************************************/

action_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ long pl, rm, dur;
  char *action, abuf[MSGSIZ], *gerund, *item, *plural, *iscloth();
  int pokes=0, molests=0, isplural=0;

  /* Count messages/actions in this room */
  room[hererm].msgsum++;

  /*-------- If dead, ignore actions until reset --------*/
  if (dead) return (1);

  /*-------- Log players actions --------*/
  if (awake && find_player (name) >= 0 &&
      (!reserved (name) || stlmatch (msg, "You killed")) &&
      !MATCH (msg, "* has left.") &&
      !MATCH (msg, "* brushes by you, heading *.") &&
      !MATCH (msg, "* has arrived."))
  { register char *s;

    for (s=msg; *s && !isspace (*s); s++) ;
    while (isspace (*s)) s++;

    if (!printedloc) print_locn ();
    fprintf (stderr, "Actn: [%s] %s\n", name, s);
    active_player (name, here, desc);
  }
  
  /*-------- If dead, ignore actions until reset --------*/
  msgtype = M_ACTION;

  /*-------- Accept donations --------*/
  if (MATCH (lcmsg, "* gives you * pennies.") ||
      MATCH (lcmsg, "* gives you * penny."))
  { long oldpennies = pennies;

    strcpy (speaker, name);
    speaktime = time (0);

    if (debug) fprintf (stderr, "Recv: [%s] %s pennies\n", res1, res2);

    /* Check for spoofing */
    psynch ();

    donate_player (name, atoi (res2));
    if ((killed_me_today (name) ||
	  streq (kshape, name) ||
	  streq (kname, name)) &&
	  pennies > oldpennies &&
	  randint (100) < (pennies - oldpennies))
    { player[add_player (name)].lastkill = 0;

      if (streq (kshape, name) || streq (kname, name))
      { command ("@lock me with me");
	strcpy (kname, "");
	strcpy (kshape, "");
      }

      reply ("\"Okay, %s, you're forgiven.", name);
    }
    else
    { reply ("\"Thanks for the donation, %s.", name); }
    return (1);
  }

  /*-------- Arrivals --------*/
  else if (MATCH (msg, "* has arrived."))
  { static long artime = 0;
    char buf[MSGSIZ];

    strcpy (buf, res1);
    if (!terse) fprintf (stderr, "Arrv: [%s]\n", buf);
    arrive_player (buf, here, desc);
    alone++;
        
    if (((pl = find_player (buf)) >= 0) &&
        (player[pl].lastlook + 600 < now))
    { look_at_thing (buf); }

    /* Special case if someone walks into robots home */
    if ((atdesk || !quiet && randint (100) < 20) && time (0) > artime + 30)
    { if ((pl = find_player (name)) < 0 ||
	   time (0) > player[pl].lastspoke + 300)
      { strcpy (speaker, name);
        spoke_player (name);
	reply ("\"Hello, %s.", name);
      }
      else
      { reply (":nods to %s.", name); }
      
      artime = time (0);
    }
    
    give_msgs (find_player (name));
    return (1);
  }

  /*-------- Depart --------*/
  else if (MATCH (msg, "* has left.") ||
	   MATCH (msg, "* has disconnected.") ||
	   MATCH (msg, "* brushes by you, heading *."))
  { if (!terse) fprintf (stderr, "Dprt: [%s]\n", res1);
    leave_player (res1, here, desc);
    --alone;
    if (streq (res1, speaker)) *speaker = '\0';
    return (1);
  }

  /*-------- Murder attempts --------*/
  else if (MATCH (msg, "* tried to kill you!"))
  { if (!printedloc) print_locn ();
    fprintf (stderr, "Kill: [%s] attempt\n", name);
    assault_player (name);
    reply ("\"Help! Help!  %s tried to kill me!", name);
    return (1);
  }

  /*-------- Murdered! --------*/
  else if (MATCH (msg, "* killed you!") ||
	   ismuck && MATCH (msg, "* killed Colin!"))
  { if (!printedloc) print_locn ();
    fprintf (stderr, "Kill: [%s] success\n", name);
    strcpy (kname, name);
    assault_player (name);
    return (1);
  }

  /*-------- Dispatch on first character --------*/
  else
  { switch (lcmsg[0])
    { case 'e':	if (!ismuck && 
		    MATCH (lcmsg, "either that player does not exist*"))
		{ fprintf (stderr, "Cannot connect to character\n");
		  sendmud ("QUIT");
		  quit_robot ();
		}
		break;
      case 'g':	if (MATCH (lcmsg, "give to whom*"))
		{ gave = 0; return (1); }
		else if (MATCH (lcmsg, "going down - bye*"))
		{ now = time (0);
	          fprintf (stderr, "\nGoing down message: %15.15s.\n",
			   ctime (&now) + 4);
		   if (ismuck)
		   { fprintf (stderr, "Ignr: ignoring going down message\n"); }
		   else
		   { lost_connect (NULL, 0); quit_robot (); }
		   return (1);
	        }
		break;

     case 'y':	if (MATCH (msg, "You sense that * is looking for you in *."))
		{ set_page (res1, res2); return (1); }

		else if (streq (msg, "You feel a wrenching sensation..."))
		{ fprintf (stderr, "Tele: from %s(%d)", here, hererm);
		  reset_teleported ();
		  return (1);
		}
		
		else if (MATCH (msg, "You sense that * is forcing you to *"))
		{ fprintf (stderr, "FORC: %s\n", msg);
		  msgtype = M_UNKNOWN;
		  reply (":was forced by %s to type \"%s\"", res1, res2);
		}

		else if (MATCH (lcmsg, "your insurance policy*"))
		{ strcpy (killer, kname);
		  strcpy (kshape, kname);
		  killed = now;
		  dead++;
		  
		  fprintf (stderr, "Dead: setting killer to '%s'\n", kname);

		  if (*kname) command ("@lock me with me & !*%s", kname);
		
		  if (MATCH (lcmsg, "your insurance policy pays * penn*"))
		  { pennies += atoi (res1); }

		  return (1);
		}

		break;
    }
  }

  /*-------- Actions involving Colin --------*/

  /*---- Colin wont talk to his last murderer ----*/
  if (!isowner (name) && (dur = killed_me_today (name)) &&
	sindex (lcmsg, lcstr (myname)))
  { switch (randint (4))
    { case 0: reply ("\"I don't talk to murderers!"); break;
      case 1: reply (":ignores %s.", name); break;
      case 2: reply (":steels himself for another attack from %s.", name);
		break;
      default: ; /* totally ignore them */
    }
    return (1);
  }

  /*---- Ping! ----*/
  if (MATCH (lcmsg, "* pings colin*") && !sindex (res2, "back"))
  { static long lastping = 0, lastpinger = 0;

    pl = find_player (name);

    strcpy (speaker, name);
    spoke_player (name);

    if (lastpinger != pl || lastping + randint (15) < now)
    { zinger (":pings %s back.", name);
      lastpinger = pl;
      lastping = now;
    }

    return (1);
  }
  
  /*---- Friendly greeting ----*/
  if (MATCH (lcmsg, "* waves to colin*") && (action = "waves to") ||
      MATCH (lcmsg, "* smiles at colin*") && (action = "smiles at") ||
      MATCH (lcmsg, "* smiles to colin*") && (action = "smiles at") ||
      MATCH (lcmsg, "* hugs colin*") && (action = "hugs") ||
      MATCH (lcmsg, "* nods to colin*") && (action = "nods to") ||
      MATCH (lcmsg, "* blinks at colin*") && (action = "blinks at") ||
      MATCH (lcmsg, "* kisses colin*") && (action = "kisses") &&
      	randint (100) < 50 ||
      MATCH (lcmsg, "* greets colin*") && (action = "greets"))
  { static long lasthug = 0, lasthugee = 0;

    pl = find_player (name);

    strcpy (speaker, name);
    spoke_player (name);

    if (lasthugee != pl || lasthug + randint (15) < now)
    { switch (randint (6))
      { case 0: reply ("\"Hi, %s!", name); break;
	case 1: reply (":hugs %s.", name); break;
	case 2: reply (":waves to %s.", name); break;
	case 3: reply (":nods to %s.", name); break;
	case 4: reply (":greets %s.", name); break;
	case 5: reply (":smiles at %s.", name); break;
      }
      lasthugee = pl;
      lasthug = now;
    }

    return (1);
  }
  
  /*---- Say ouch ----*/
  if (MATCH (lcmsg, "* assaults colin*") && (action = "assault") ||
      MATCH (lcmsg, "* baps colin*") && (action = "bap") ||
      MATCH (lcmsg, "* bashes colin*") && (action = "bash") ||
      MATCH (lcmsg, "* bonks colin*") && (action = "BONK") ||
      MATCH (lcmsg, "* bops colin*") && (action = "bop") ||
      MATCH (lcmsg, "* hits colin*") && (action = "hit") ||
      MATCH (lcmsg, "* kicks colin*") && (action = "kick") ||
      MATCH (lcmsg, "* prods colin*") && (action = "prod") ||
      MATCH (lcmsg, "* pushes colin*") && (action = "push") ||
      MATCH (lcmsg, "* reprogrames colin*") && (action = "reprogram"))
  { pokes++; }
  
  else if (
      MATCH (lcmsg, "* slaps colin*") && (action = "slap") ||
      MATCH (lcmsg, "* slashes colin*") && (action = "slash") ||
      MATCH (lcmsg, "* spanks colin*") && (action = "spank") ||
      MATCH (lcmsg, "* stabs colin*") && (action = "stab") ||
      MATCH (lcmsg, "* thumps colin*") && (action = "thump") ||
      MATCH (lcmsg, "* thwacks colin*") && (action = "thwack") ||
      MATCH (lcmsg, "* whacks colin*") && (action = "whack") ||
      MATCH (lcmsg, "* whaps colin*") && (action = "whap") ||
      MATCH (lcmsg, "* whips colin*") && (action = "whip"))
  { pokes++; }
      
  if (pokes)
  { strcpy (speaker, name);
    spoke_player (name);

    if (streq (action, "BONK") && randint (100) < 40)
    { zinger ("\"OIF, %s!", name); }
    
    else
    { switch (randint (5))
      { case 0: zinger ("\"Ouch, %s!", name); break;
	case 1: zinger ("\"Quit it, %s!", name); break;
	case 2: zinger ("\"Stop that, %s!", name); break;
	case 3: zinger ("\"Don't you dare, %s!", name); break;
	case 4: zinger ("\"Don't do that, %s!", name); break;
      }
    }

    return (1);
  }
  
  /* Message not matched by this rule set */
  return (0);
}

/****************************************************************
 * page_okay: True if we will allow caller to page us to loc
 * this function is not called for owner.
 ****************************************************************/

page_okay (caller, loc)
char *caller, *loc;
{
  if (killed_me_today (caller) && !isowner (caller))
  { if (!quiet || randint (100) < 10)
    { reply ("\"Hah! %s is paging me from %s. %s.",
	     caller, loc, "I don't answer pages from murderers");
    }
    return (0);
  }

  return (1);
}

/****************************************************************
 * home_hook: We went "home"
 ****************************************************************/

home_hook ()
{ havebook = 0; }

/****************************************************************
 * before_move_hook: things to do before moving out of current room
 ****************************************************************/

before_move_hook ()
{
  if (streq (here, home))
  { if (atdesk) send_pose ("gets up from his desk."); atdesk=0;
    command ("@describe %s as %s", DESK, AWAYMSG);
  }

  if (pagedto >= 0 && pagedfrom != hererm &&
      (!quiet || streq (pagedby, "<AUTORETURN>")))
  { msgtype = M_UNKNOWN;

    if (streq (pagedby, "<HOME>"))
    { unlogged (":strolls by on %s way home.", MALE ? "his" : "her"); }
    else if (streq (pagedby, "<unlogged>"))
    { unlogged (":goes by on %s way to %s.", 
	       MALE ? "his" : "her", room_name (pagedto)); }
    else if (streq (pagedby, "<WHIM>"))
    { unlogged (":wanders by on %s way to %s.",
	       MALE ? "his" : "her", room_name (pagedto)); }
    else if (streq (pagedby, "<EXPLORING>"))
    { command (":walks by on %s way to explore %s.",
	       MALE ? "his" : "her", room_name (pagedto)); }
    else
    { command (":rushes by on %s way to %s to see %s.",
	       MALE ? "his" : "her", room_name (pagedto), pagedby);
    }
  }
}

/****************************************************************
 * after_move_hook: things to do after moving out of current room
 ****************************************************************/

after_move_hook ()
{
}

/****************************************************************
 * new_room_hook: things to do after moving into a new room
 ****************************************************************/

new_room_hook ()
{
  strcpy (speaker, "");
  strcpy (pathto, "");
  
  /* Tell our caller where we are */
  if (*pagedby && *pagedby != '<' && hererm != pagedto && pennies > MINPEN)
  { pennies --;
    command ("page %s I'm now in %s", pagedby, room_name (hererm));
  }

  /* Check for home actions */
  if (hererm == homerm)
  { if (!atdesk && (pagedto < 0 || pagedto == homerm))
    { if (awake)
      { unlogged (":sits down at his desk."); atdesk++;
        command ("@descri %s as %s", DESK, OPENMSG);
      }
    }
  }
}

/****************************************************************
 * checkpoint: Write out files
 ****************************************************************/

checkpoint ()
{
  now = time (0);
  fprintf (stderr,
	   "Save: %15.15s, %ld of %ld rooms new, %ld of %ld exits new\n",
	   ctime (&now) + 4, newrooms, rooms, newexits, exits);
  write_players (plyfile);

  if (newrooms || newexits)
  { write_map (mapfile, 0);
    now = time (0);
    if (!terse) fprintf (stderr, "Save: done at %15.15s\n", ctime (&now) + 4);
  }
  lastcheck = time (0);
  newrooms = newexits = 0;
}


/****************************************************************
 * onintr: Got an interrupt, checkpoint and quit
 ****************************************************************/

onintr ()
{
  fprintf (stderr, "Quitting because of interrupt\n");
  sendmud ("\"Goodbye, all, I'm going home to sleep.");
  sendmud ("straighthome");
/*  sendmud ("@describe %s as %s", DESK, CLOSEDMSG);*/
  sendmud (":puts his head down and goes to sleep.");
  sendmud ("@describe me as %s", POWEROFF); strcpy (mydesc, POWEROFF);
  sendmud ("@quit");
  quit_robot ();
  exit (0);
}

/****************************************************************
 * isowner: Return true if the named person can give special orders
 ****************************************************************/

isowner (name)
{ return (streq (lcstr (name), OWNER)); }

/****************************************************************
 * amount_hook: Check amount given to player
 ****************************************************************/

amount_hook (amount, name, polite)
long amount, polite;
char *name;
{
  return (amount);
}

/****************************************************************
 * who_is_hook:
 ****************************************************************/

who_is_hook (pers, name)
char *pers, *name;
{ 
  if (streq (pers, "fuzzy"))
  { switch (randint (6))
    { case 0: zinger ("\"%s just this guy I met in a bar, %s.",
		      streq (name, "Fuzzy") ? "You're" : "He's", name);
		    break;
      case 1: zinger ("\"%s the reincarnation of Elvis Presley, %s.",
		     streq (name, "Fuzzy") ? "You're" : "He's", name); break;
      case 2: zinger ("\"%s the world's third greatest hacker, %s.",
		     streq (name, "Fuzzy") ? "You're" : "He's", name); break;
      case 3: zinger ("\"%s just this guy, you know, %s.",
		     streq (name, "Fuzzy") ? "You're" : "He's", name);
		    break;
      case 4: zinger ("\"%s my Amway representative, %s.",
		     streq (name, "Fuzzy") ? "You're" : "He's", name);
		    break;
      case 5: zinger ("\"I don't know, %s, I just follow %s around.",
		    name, streq (name, "Fuzzy") ? "you" : "him"); break;
    }
  }
  else if (streq (pers, "stewy"))
  { reply ("\"%s Clamen Airlines and the People Mover, %s.",
	  streq (name, "Stewy") ? "You run" : "He runs", name); }
  else if (streq (pers, "flexi"))
  { reply ("\"%s Puddle Blvd. and Emailboxes, %s.",
	   streq (name, "flexi") ? "You run" : "He runs", name); }
  else if (streq (pers, "wizard"))
  { reply ("\"%s used to own the world and everything in it, %s.",
	   streq (name, "Wizard") ? "You" : "He", name); }
  else if (streq (pers, "tinker"))
  { reply ("\"%s the world and everything in it, %s.",
	   streq (name, "Tinker") ? "You own" : "He owns", name); }
  else if (streq (pers, "nymph"))
  { reply ("\"%s the Rec Room, %s.",
	   streq (name, "Nymph") ? "You own" : "She owns", name); }
  else if (streq (pers, "zippy"))
  { reply ("\"%s the TFM room, %s.",
	   streq (name, "zippy") ? "You own" : "He owns", name); }
  else if (streq (pers, "danny"))
  { reply ("\"He's the chatty hermit who lives %s, %s.",
	    "in a dark cave north of Cirdan's",  name); }
  else if (streq (pers, "gloria"))
  { reply ("\"She was the assistant librarian of the original TinyMUD, %s.",
	   name); }
  else
  { return (0); }

  return (1);
}

/****************************************************************
 * do_number: Unused in Colin
 ****************************************************************/

do_number ()
{
  return (0);
}

/****************************************************************
 * lost_connect: Watchdog: send mail if Islandia goes down
 ****************************************************************/

lost_connect (pat, tim)
char *pat;
int tim;
{
}

/****************************************************************
 * give_tour
 ****************************************************************/

give_tour (query, name)
char *query, *name;
{ long from, to, town;
  char *answer = NULL;
  long i, numhelp, cnt=0;

  fprintf (stderr, "Tour: '%-32.32s', name '%s'\n", query, name);

  for (i=0; i<20; i++)
  { to = randint (rooms);
    if (room[to].name && room[to].name[0] && ROOM_GET (to, RM_REACH)) break;
  }

  if (i >= 20)
  { zinger ("\"I can't think of any good places to show you, %s.", name);
    return (1);
  }

  if (!((msgtype >= M_PAGE && (town = close_room ("town", NULL)) >= 0) &&
	(answer = find_path (town, to, SHORTPATH)) && (from = town) >= 0 ||
	(answer = find_path (hererm, to, SHORTPATH)) && (from = hererm) >= 0 ||
	(answer = find_path (homerm, to, SHORTPATH)) && (from = homerm) >=0))
  { zinger ("\"I can't think of any good places to show you, %s.", name);
    return (1);
  }

  switch (randint (3))
  { case 0: zinger ("\"Hmm...you should visit %s, %s.",
		   room_name (to), name);
	    break;
    case 1: zinger ("\"%s in %s, %s.",
		   "Perhaps you will find what you seek",
		   room_name (to), name);
	    break;
    case 2: zinger ("\"You should go see %s, %s.",
		   room_name (to), name);
	    break;
  }
  
  strcpy (speaker, name);

  reply ("\"From %s, go %s.",
	 (from == hererm) ? "here" : room_name (homerm), answer);

  return (1);
}


/****************************************************************
 * answer_what:
 ****************************************************************/

answer_what (name, det, thing)
char *name, *det, *thing;
{
  /* No public rules, yet */
}

long fasttype = 0;

fakeprint ()
{ fprintf (stderr, "fakeprint not avail in colin, exiting\n");
  exit (1);
}

hearts_msg () { return (0); }

reset_cnet () { return (0); }
