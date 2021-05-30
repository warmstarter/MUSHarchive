/****************************************************************
 * Colin, the Mass-Neotek exploration robot
 *
 * HISTORY
 * 26-Feb-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Sixth experimental release. Changed file formats, added lots
 *	of info to room memory.  Found a memory allocation bug.
 *
 * 10-Feb-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Fifth special release.
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
# include <signal.h>
# include <ctype.h>
# include <varargs.h>

# include "robot.h"
# include "vars.h"
# include "colin.h"

/****************************************************************
 * setconfig: These values are defined in colin.h
 ****************************************************************/

setconfig ()
{
  male = MALE;
  whoami = WHOAMI;
  owner = OWNER;
  if (myname == NULL)	myname = MYNAME;
  if (mudhost == NULL)	mudhost = MUDHOST;
  if (mudport == 0)	mudport = MUDPORT;
  if (mapfile == NULL)	mapfile = MAPFILE;
  if (plyfile == NULL)	plyfile = PLYFILE;
}

/****************************************************************
 * robot main loop
 ****************************************************************/

robot ()
{ 
  waitfor ("Welcome to");
  
  now = time (0);
  fprintf (stderr, "\n-------- %s log starts at %15.15s --------\n\n",
	   MYNAME, ctime (&now) + 4);
  
  read_players (plyfile);
  read_map (mapfile);
  check_players ();

  /* Set up signal handling */
  signal (SIGHUP, onintr);
  signal (SIGINT, onintr);

  /* Log in to TinyMUD */
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

    /* Hi priority actions */
    if (do_warn ())	continue;
    if (do_typecmd ())	continue;
    if (do_page ())	continue;
    if (do_notes ())	continue;
    if (do_explore ())	continue;

    /* Long wait */
    do_twiddle ();

    /* Low priority actions */
    if (do_wander ())	continue;

    /* Reset if dead */
    if (dead)		reset_dead ();
  }

  sendmud ("QUIT");
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
{
  sendmud ("%s %s", opre, outpre);
  sendmud ("%s %s", osuf, outsuf);
  command ("@lock me = me");
  command ("@describe me = %s.", WHOAMI); strcpy (mydesc, WHOAMI);
  command ("@fail me = You masher!");
  command ("@ofail me = %s", ROBFAIL);
  wsynch ();
  movemud ("home");
  strcpy (home, here);
  strcpy (homedesc, desc);
  homerm = add_room (home, desc);
  me = add_player (MYNAME);
  psynch ();

  command (":activates and appears as a shadowy figure sitting on the desk.");
  atdesk++;
  command ("@describe %s = %s", DESK, OPENMSG);

  awake++;
  lastcheck = now = time (0);
}

/****************************************************************
 * do_special_check ()
 ****************************************************************/

do_special_check ()
{
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
  { reply ("\"Oh by the way, %s, %s in %s about %s ago.",
	   player[termtarg].name,
	   "Terminator was looking for you", 
	   termloc, time_dur (time (0) - termat));
    strcpy (speaker, player[termtarg].name);
    spoke_player (player[termtarg].name);
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
    else if (exploring)		dur = randint (30) + 5;
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
  /* If not exploring, head for home */
  if (! exploring && !nextwait && ! *pagedby &&
      ! *typecmd && ! dead && !takingnotes)
  { long dest;
    long dice;

    do
    { dice = randint (100);
      dest = -1;

      if (dice < 60)	  dest = find_room ("The Town Square", NULL);
      else if (dice < 80) dest = close_room ("Rec Room", NULL);
      

      if (dest < 0)
      { do { dest = randint (rooms); } while (!room[dest].name); }
    } while (dest < 0 || (dest == hererm || randint (100) < 90));

    now = time (0);
    
    if (!terse)
    { fprintf (stderr, "\n---- %15.15s ---- Wandering over to %s ----\n\n",
	      ctime (&now) + 4, room_name (dest));
    }

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

  /* Save lower case copy of message */
  strcpy (lcmsg, lcstr (msg));

  /* Save name for replies */
  for (s=msg, t=name; *s && !isspace (*s); ) *t++ = *s++;
  *t = '\0';

  /* Standard message handling.  Try each batch of replies in turn */
  if (special_mode   (name, msg, lcmsg))
  { return; }

  /* Ignore most messages until we are really awake */
  if (!awake) return;

  if (says_msg       (name, msg, lcmsg) ||
      action_msg     (name, msg, lcmsg))
  { /* Replies were handled by the call that succeeded */
    return;
  }

  /* Ignore message altogether, then */
  return;
}

/****************************************************************
 * says_msg: Handle spoken messages from other players (that is
 *	     messages like 'Foo says "Hello."').
 ****************************************************************/

says_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ char buf[TOKSIZ], lcbuf[TOKSIZ];
  register char *s;

  /* skip over first word (should be the name) */
  for (s=msg; *s && !isspace (*s); s++);
  while (isspace (*s)) s++;
  
  /* Verify that this is a speech action */
  if (!awake)			     { return (0); }
  else if (stlmatch (s, "says"))     { s += 4; /* strlen ("says") */ }
  else if (stlmatch (s, "whispers")) { s += 8; /* strlen ("whispers") */ }
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
  fprintf (stderr, "Says: [%s] \"%s\"\n", name, buf);

  if (awake && find_player (name) >= 0 && !reserved (name))
  { saw_player (name, here, desc);
    heard_player (name, buf);
  }

  speaktime = time (0);

  /*---- Verify that this message was meant for the robot ----*/
  if (to_me (name, buf, lcbuf))
  { check_stay ();

    /* Now try various speech rule sets */
    if (hi_priority  (name, buf, lcbuf) ||
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
{ long dur;

  /*---- Colin wont talk to his last murderer ----*/
  if (!isowner (name) && (dur = killed_me_today (name)))
  { switch (randint (3))
    { case 0: reply ("\"I don't talk to murderers!"); break;
      case 1: reply (":ignores %s.", name); break;
      default: ; /* totally ignore them */
    }
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
	   MATCH (lcmsg, "*be*quiet*") ||
	   MATCH (lcmsg, "*stop*talk*") ||
	   MATCH (lcmsg, "*have a*nice*day*") ||
	   MATCH (lcmsg, "*stop*talk*") ||
	   MATCH (lcmsg, "*hasta*vista*"))
  { if (isowner (name) && MATCH (lcmsg, "*colin*"))
    { fprintf (stderr, "Quitting on command of %s\n", name);
      sendmud ("\"Goodbye, %s, I'm going home to sleep.", name);
      sendmud ("home");
      sendmud ("@describe %s = %s", DESK, CLOSEDMSG);
      sendmud (":deactivates and vanishes without a sound.");
      sendmud ("@describe me = %s", POWEROFF); strcpy (mydesc, POWEROFF);
      sendmud ("QUIT");
      spoke_player (name);
      quit_robot ();
    }
    else /* They probably mean they are leaving */
    { reply ("\"Goodbye, %s.", name);
      spoke_player (name);
      strcpy (speaker, "");
      speaktime = 0;
      return (1);
    }
  }

  /*---- Checkpoint files ----*/
  else if (MATCH (lcmsg, "*core*dump*colin*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner (name))
    { crash_robot ("Core dumped by %s: %s", name, msg); }
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

lo_priority (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ long count;
  static long msgno = 1;


  /*---- A query about 'who' ----*/
  if (MATCH (lcmsg, "*who*you*") && MATCH (lcmsg, "*are*") ||
      MATCH (lcmsg, "*what*you*mission*") ||
      MATCH (lcmsg, "*what*do*you*do*") ||
      MATCH (lcmsg, "*what*you*function*") ||
      MATCH (lcmsg, "*what*you*purpose*") ||
      MATCH (lcmsg, "*what*you*goal*") ||
      MATCH (lcmsg, "*what*you*objectiv*") ||
      MATCH (lcmsg, "*explain*you*mission*") ||
      MATCH (lcmsg, "*explain*you*function*") ||
      MATCH (lcmsg, "*explain*you*purpose*") ||
      MATCH (lcmsg, "*explain*you*goal*") ||
      MATCH (lcmsg, "*explain*you*objectiv*") ||
      MATCH (lcmsg, "*describe*you*mission*") ||
      MATCH (lcmsg, "*describe*you*function*") ||
      MATCH (lcmsg, "*describe*you*purpose*") ||
      MATCH (lcmsg, "*describe*you*goal*") ||
      MATCH (lcmsg, "*describe*you*objectiv*"))
  { if (takingnotes && randint (100) < 50)
    { reply ("\"I am taking notes, %s.", name); }
    else
    { reply ("\"I explore the world and help people, %s.", name); }
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- A query about 'goals' ----*/
  else if (MATCH (lcmsg, "*what is *tinymud*") ||
	   MATCH (lcmsg, "*what's *tinymud*") ||
	   MATCH (lcmsg, "what*all*this*"))
  { switch (randint (4))
    { case 0: reply ("\"This game has no objective, %s.", name); break;
      case 1: reply ("\"A way to meet wonderful new people, %s.", name);
		break;
      case 2: reply ("\"This game has no objective, %s.", name); break;
      case 3: reply ("\"What do you think TInyMUD is, %s?", name); break;
    }
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Pick commands ----*/
  else if (MATCH (lcmsg, "*take *") ||
	   MATCH (lcmsg, "*get *") ||
	   MATCH (lcmsg, "*pick *"))
  { reply ("\"I'm sorry, %s, I don't pick things up.", name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }    

  /*---- Pick commands ----*/
  else if (MATCH (lcmsg, "*kill*") ||
	   MATCH (lcmsg, "*murder*") ||
	   MATCH (lcmsg, "*fight*"))
  { reply ("\"I'm sorry, %s, I don't like violence.", name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }    

  /*---- Follow me, listen to me ----*/
  else if (MATCH (lcmsg, "*follow*") ||
	   MATCH (lcmsg, "*listen to*") ||
	   MATCH (lcmsg, "*, listen*") ||
	   MATCH (lcmsg, "*: listen*"))
  { reply ("\"I'm sorry, %s, I can't right now.", name);
    strcpy (speaker, name);
    spoke_player (name);
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

  /*---- Else ----*/
  if (randint (100) < 50 ||
      MATCH (lcmsg, "*, colin*") ||
      MATCH (lcmsg, "*colin: *") ||
      MATCH (lcmsg, "*colin, *"))
  { switch (randint (3))
    { case 0: reply (":says I'm sorry, %s, I couldn't understand that.",
		     name); break;
      case 1: reply (":doesn't completely understand %s.", name); break;
      case 2: reply (":could not understand %s's statement.", name); break;
    }
    strcpy (speaker, name);
    spoke_player (name);
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
{ long pl, rm;
  char *action;

  /* Count messages/actions in this room */
  room[hererm].msgsum++;

  /*-------- Log players actions --------*/
  if (awake && find_player (name) >= 0 && !reserved (name))
  { register char *s;

    for (s=msg; *s && !isspace (*s); s++) ;
    while (isspace (*s)) s++;

    if (!printedloc) print_locn ();
    fprintf (stderr, "Actn: [%s] %s\n", name, s);
    active_player (name, here, desc);
    return (1);
  }

  /*-------- Accept donations --------*/
  if (MATCH (lcmsg, "* gives you * pennies.") ||
      MATCH (lcmsg, "* gives you * penny."))
  { long oldpennies = pennies;

    strcpy (speaker, name);
    speaktime = time (0);

    if (debug) fprintf (stderr, "Recv: [%s] %s pennies\n", res1, res2);

    /* Check for spoofing */
    psynch ();

    if ((pennies <= oldpennies) && !inmove)
    { if (!terse)
      { fprintf (stderr, "Was cheated, old %ld, new %ld\n",
		 oldpennies, pennies); }
      reply ("\"%s is a cheat and a thief.  --%s", name, MYNAME);
    }
    else
    { donate_player (name, atoi (res2));
      if (killed_me_today (name) &&
	  randint (100) < (pennies - oldpennies))
      { player[add_player (name)].lastkill = 0;
	reply ("\"Okay, %s, you're forgiven.", name);
      }
      else
      { reply ("\"Thank you for your donation, %s.", name);
        if (!terse)
	{ fprintf (stderr, "Got a donation, now have %ld pennies\n",
		   pennies); }
      }
    }
    
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
      { reply ("\"Hello, %s.", name);
	strcpy (speaker, name);
        spoke_player (name);
      }
      else
      { reply (":nods to %s.", name); }
      
      artime = time (0);
    }
    return (1);
  }

  /*-------- Depart --------*/
  else if (MATCH (msg, "* has left.") ||
	   MATCH (msg, "* brushes by you, heading *."))
  { if (!terse) fprintf (stderr, "Dprt: [%s]\n", res1);
    leave_player (res1, here, desc);
    if (--alone <= 0 && exploring) hanging = 0;
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
  else if (MATCH (msg, "* killed you!"))
  { if (!printedloc) print_locn ();
    fprintf (stderr, "Kill: [%s] success\n", name);
    assault_player (name);
    return (1);
  }

  /*-------- Dispatch on first character --------*/
  else
  { switch (lcmsg[0])
    { case 'e':	if (MATCH (lcmsg, "either that player does not exist*"))
		{ fprintf (stderr, "Cannot connect to character\n");
		  sendmud ("QUIT");
		  quit_robot ();
		}
		break;
      case 'g':	if (MATCH (lcmsg, "give to whom*"))
		{ gave = 0; return (1); }
		break;

      case 'y':	if (MATCH (msg, "You sense that * is looking for you in *."))
		{ set_page (res1, res2); return (1); }

		else if (MATCH (lcmsg, "your insurance policy pays * penn*."))
		{ dead++; pennies += atoi (res1); return (1); }
		break;
    }
  }
  
  /*---- Say ouch ----*/
  if (MATCH (lcmsg, "* kicks colin*") && (action = "kick") ||
      MATCH (lcmsg, "* hits colin*") && (action = "hit")||
      MATCH (lcmsg, "* hurts colin*") && (action = "hurt") ||
      MATCH (lcmsg, "* pokes colin*") && (action = "poke") ||
      MATCH (lcmsg, "* nudges colin*") && (action = "nudge") ||
      MATCH (lcmsg, "* bites colin*") && (action = "bite") ||
      MATCH (lcmsg, "* bonks colin*") && (action = "bonk") ||
      MATCH (lcmsg, "* strikes colin*") && (action = "strike") ||
      MATCH (lcmsg, "* tweaks colin*") && (action = "tweak") ||
      MATCH (lcmsg, "* beats colin*") && (action = "beat"))
  { switch (randint (5))
    { case 0: reply ("\"Ouch, %s!", name); break;
      case 1: reply ("\"Don't %s me, %s!", action, name); break;
      case 2: reply (":%ss %s back.", action, name); break;
      case 3: reply (":would like to %s %s.", action, name); break;
      default: /* ignore */ ;
    }

    strcpy (speaker, name);
    spoke_player (name);
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
  if (killed_me_today (caller))
  { reply ("\"Hah! %s is paging me from %s. %s.",
	   caller, loc, "I don't answer pages from murderers");
    return (0);
  }

  return (1);
}

/****************************************************************
 * before_move_hook: things to do before moving out of current room
 ****************************************************************/

before_move_hook ()
{
  if (streq (here, home))
  { if (atdesk) command (":gets up from %s desk.",
			 MALE ? "his" : "her");
    atdesk=0;
    command ("@describe %s = %s", DESK, AWAYMSG);
  }
  
  if (pagedto >= 0 && !quiet && pagedfrom != hererm)
  { if (streq (pagedby, "<HOME>"))
    { command (":strolls by on %s way home.", MALE ? "his" : "her"); }
    else if (streq (pagedby, "<COMMAND>"))
    { command (":goes by on %s way to %s.", 
	       MALE ? "his" : "her", room_name (pagedto)); }
    else if (streq (pagedby, "<WHIM>"))
    { command (":wanders by on %s way to %s.",
	       MALE ? "his" : "her", room_name (pagedto)); }
    else if (streq (pagedby, "<EXPLORING>"))
    { command (":walks by on %s way to explore %s.",
	       MALE ? "his" : "her", room_name (pagedto)); }
    else if (pagedfrom != hererm)
    { command (":rushes by on %s way to %s to see %s.",
	       MALE ? "his" : "her", room_name (pagedto), pagedby);
    }
  }
	  
  if (*typecmd)
  { command (":types <%s>", typecmd); }
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
    command ("page %s = 0", pagedby);
  }

  /* Check for home actions */
  if (hererm == homerm)
  { if (!atdesk && (pagedto < 0 || pagedto == homerm))
    { if (awake)
      { command (":sits on the edge of %s the desk.", MALE ? "his" : "her");
	atdesk++;
        command ("@describe %s = %s", DESK, OPENMSG);
      }
    }
  }

  if (exploring && !*pagedby && !*typecmd && !dead && !takingnotes)
  { check_time ();
    do_special_check ();
    if (awake) hangaround (randint (10) + randint (10));
  }
}

/****************************************************************
 * checkpoint: Write out files
 ****************************************************************/

checkpoint ()
{ 
  now = time (0);
  fprintf (stderr,
	   "Save: checkpointing at %15.15s, %ld new rooms, %ld new exits\n",
	   ctime (&now) + 4, newrooms, newexits);
  write_players (plyfile);
  if (newrooms || newexits) write_map (mapfile, 0);
  lastcheck = time (0);
  newrooms = newexits = 0;
  now = time (0);
  if (!terse) fprintf (stderr, "Save: done at %15.15s\n", ctime (&now) + 4);
}

/****************************************************************
 * onintr: Got an interrupt, checkpoint and quit
 ****************************************************************/

onintr ()
{
  fprintf (stderr, "Quitting because of interrupt\n");
  sendmud ("\"Goodbye, all, I'm going home to sleep.");
  sendmud ("home");
  sendmud ("@describe %s = %s", DESK, CLOSEDMSG);
  sendmud (":deactivates and vanishes without a sound.");
  sendmud ("@describe me = %s", POWEROFF); strcpy (mydesc, POWEROFF);
  sendmud ("QUIT");
  quit_robot ();
  exit (0);
}

/****************************************************************
 * isowner: Return true if the named person can give special orders
 ****************************************************************/

isowner (name)
{ return (streq (lcstr (name), OWNER)); }

/****************************************************************
 * home_hook: Called after going home, before after_move_hook()
 ****************************************************************/

home_hook () {}

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
  { switch (randint (5))
    { case 0: reply ("\"%s just this guy I met in a bar, %s.",
		      isowner (name) ? "You're" : "He's", name);
		    break;
      case 1: reply ("\"%s the reincarnation of Elvis Presley, %s.",
		     isowner (name) ? "You're" : "He's", name); break;
      case 2: reply ("\"%s the world's third greatest hacker, %s.",
		     isowner (name) ? "You're" : "He's", name); break;
      case 3: reply ("\"%s just this guy, you know, %s.",
		     isowner (name) ? "You're" : "He's", name);
		    break;
      case 4: reply ("\"%s my Amway representative, %s.",
		     isowner (name) ? "You're" : "He's", name);
		    break;
    }
  }
  else if (streq (pers, "stewy"))
  { reply ("\"%s Clamen Airlines and the People Mover, %s.",
	  streq (name, "Stewy") ? "You run" : "He runs", name); }
  else if (streq (pers, "flexi"))
  { reply ("\"%s Puddle Blvd. and Emailboxes, %s.",
	   streq (name, "flexi") ? "You run" : "He runs", name); }
  else if (streq (pers, "wizard"))
  { reply ("\"%s the world and everything in it, %s.",
	   streq (name, "Wizard") ? "You own" : "He owns", name); }
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
  { reply ("\"She's the assistant librarian of TinyMUD, %s.", name); }
  else if (streq (pers, "julia"))
  { reply ("\"She's the chief librarian of TinyMUD, %s.", name); }
  else
  { return (0); }

  return (1);
}
