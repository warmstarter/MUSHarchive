/****************************************************************
 * reply.c: Common responses for most TinyMUD automata
 *
 * HISTORY
 * 30-Jun-92  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Thirteenth prodigal release.
 *
 * 28-Sep-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Tenth ceremonial release. 
 *
 * 11-Sep-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Ninth official release. 
 *
 * 05-May-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Seventh sequential release. Added quote_player command.
 *	Mods for TinyHELL.
 *
 * 7-Mar-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Sixth experimental release.  Fixes in asking for money.
 *
 * 26-Feb-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Sixth experimental release. Changed file formats, added lots
 *	of info to room memory.  Found a memory allocation bug.
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
# include "vars.h"

/****************************************************************
 * standard_msg: Handle most queries common to most robots
 *	Standard_msg was split into 2 parts because it was
 *	getting too big for some C compilers.
 ****************************************************************/

standard_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{
  return (simple_std_msg (name, msg, lcmsg) ||
	  medium_std_msg (name, msg, lcmsg) ||
	  complex_standard_msg (name, msg, lcmsg));
}

simple_std_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ long pl, rm;
  long full = 0;
  register long i;

  /*---- Turn on debugging ----*/
  if (MATCH (lcmsg, "*debug*conv*on*"))
  { strcpy (speaker, name);
    if ((isowner (name) && (trusting || *code && sindex (msg, code))))
    { debug_conv++;
      spoke_player (name);
      reply ("\"Debugging of conversations is now on{, n}");
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn off debugging ----*/
  else if (MATCH (lcmsg, "*debug*conv*off*"))
  { strcpy (speaker, name);
    if ((isowner (name) && (trusting || *code && sindex (msg, code))))
    { debug_conv = 0;
      spoke_player (name);
      reply ("\"Debugging conversations is now off{, n}");
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn on debugging ----*/
  if (MATCH (lcmsg, "*debug*on*"))
  { strcpy (speaker, name);
    if ((isowner (name) && (trusting || *code && sindex (msg, code))))
    { debug++;
      spoke_player (name);
      reply ("\"Debugging is now on{, n}");
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn off debugging ----*/
  else if (MATCH (lcmsg, "*debug*off*"))
  { strcpy (speaker, name);
    if ((isowner (name) && (trusting || *code && sindex (msg, code))))
    { debug = 0;
      spoke_player (name);
      reply ("\"Debugging is now off{, n}");
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Reset Exploration ----*/
  else if (isowner (name) && (trusting || *code && sindex (msg, code)) &&
	   MATCH (lcmsg, "*reset*explor*") &&
	   sindex (lcmsg, lcstr (myname)))
  { strcpy (speaker, name);
    spoke_player (name);

    lastrm = 0;
    for (rm=0; rm<rooms; rm++)
    { ROOM_CLR (rm, RM_EXPLORED); }
    
    fprintf (stderr, "Expl: reset all exploration information for %s\n",
	     name);
    reply ("\"Okay{, n}, I will start all over in my exploration.");

    return (1);
  }

  /*---- Reset Robot ----*/
  else if (isowner (name) && (trusting || *code && sindex (msg, code)) &&
	   MATCH (lcmsg, "*reset*yourself*") &&
	   sindex (lcmsg, lcstr (myname)))
  { strcpy (speaker, name);
    spoke_player (name);
    reset_robot (name);	/* Wont return */
    return (1);
  }

  /*---- Turn on paging ----*/
  else if (MATCH (lcmsg, "*paging*on*"))
  { strcpy (speaker, name);
    if ((isowner (name) && (trusting || *code && sindex (msg, code))))
    { paging++;
      spoke_player (name);
      reply ("\"Paging is now on{, n}");
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn off paging ----*/
  else if (MATCH (lcmsg, "*paging*off*"))
  { strcpy (speaker, name);
    if ((isowner (name) && (trusting || *code && sindex (msg, code))))
    { paging = 0;
      spoke_player (name);
      reply ("\"Paging is now off{, n}");
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn on quiet ----*/
  else if (MATCH (lcmsg, "*quiet*on*") ||
	   MATCH (lcmsg, "*nois*off*"))
  { strcpy (speaker, name);
    quiet++;
    spoke_player (name);
    reply ("\"I will be more quiet{, n}");
    return (1);
  }

  /*---- Turn off quiet ----*/
  else if (MATCH (lcmsg, "*quiet*off*") ||
	   MATCH (lcmsg, "*nois*on*"))
  { strcpy (speaker, name);
    quiet = 0;
    spoke_player (name);
    reply ("\"quiet is now off{, n}");
    return (1);
  }

  /*---- Turn off usedesc ----*/
  else if (MATCH (lcmsg, "*use*desc*off*") ||
	   MATCH (lcmsg, "*don't*use*desc*") ||
	   MATCH (lcmsg, "*do not*use*desc*"))
  { strcpy (speaker, name);
    if ((isowner (name) && (trusting || *code && sindex (msg, code))))
    { usedesc = 0;
      spoke_player (name);
      reply ("\"I will not use descriptions to find exits{, n}");
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn on usedesc ----*/
  else if (MATCH (lcmsg, "*use*desc*on*") ||
	   MATCH (lcmsg, "*use*descrip*"))
  { strcpy (speaker, name);
    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { usedesc++;
      spoke_player (name);
      reply ("\"I will now use descriptions to find exits{, n}");
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn on terse ----*/
  else if (MATCH (lcmsg, "*terse*on*"))
  { strcpy (speaker, name);
    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { terse++;
      spoke_player (name);
      reply ("\"I am now terse{, n}");
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn off terse ----*/
  else if (MATCH (lcmsg, "*terse*off*"))
  { strcpy (speaker, name);
    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { terse = 0;
      spoke_player (name);
      reply ("\"I am not terse{, n}");
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Set speed ----*/
  else if (MATCH (lcmsg, "*faster*") &&
	   !(sindex (lcmsg, "what") ||
	     sindex (lcmsg, "how") ||
	     sindex (lcmsg, "why")))
  { strcpy (speaker, name);
    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { if (speed > 1) speed--;
      spoke_player (name);
      reply ("\"Speed is now set at %ld seconds between moves{, n}", speed);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Set speed ----*/
  else if (MATCH (lcmsg, "*slower*") &&
	   !(sindex (lcmsg, "what") ||
	     sindex (lcmsg, "how") ||
	     sindex (lcmsg, "why")))
  { strcpy (speaker, name);
    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { speed++;
      spoke_player (name);
      reply ("\"Speed is now set at %ld seconds between moves{, n}", speed);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Set speed ----*/
  else if (MATCH (lcmsg, "*what*you*speed*") ||
	   MATCH (lcmsg, "*how*fast*you*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { spoke_player (name);
      reply ("\"Speed is now set at %ld seconds between moves{, n}", speed);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Set speed ----*/
  else if (MATCH (lcmsg, "*set *speed at *,*") ||
	   MATCH (lcmsg, "*set *speed at *") ||
	   MATCH (lcmsg, "*set *speed=*,*") ||
	   MATCH (lcmsg, "*set *speed=*") ||
	   MATCH (lcmsg, "*set *speed =*,*") ||
	   MATCH (lcmsg, "*set *speed =*") ||
	   MATCH (lcmsg, "*set *speed to *,*") ||
	   MATCH (lcmsg, "*set *speed to *"))
  { strcpy (speaker, name);
    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { if (isdigit (*res3)) speed = atoi (res3);
      if (speed < 1) speed = 1;
      spoke_player (name);
      reply ("\"Speed is now set at %ld seconds between moves{, n}", speed);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- What version ----*/
  else if (MATCH (lcmsg, "*which*version*you*") ||
	   MATCH (lcmsg, "*which*you*version*") ||
	   MATCH (lcmsg, "*what*version*you*") ||
	   MATCH (lcmsg, "*what*you*version*"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"Maas-Neotek, %s{, n}.", VERSION);
    return (1);
  }

  /*---- Stop ignoring player ----*/
  else if ((MATCH (lcmsg, "*stop*ignoring *,*") ||
	    MATCH (lcmsg, "*stop*ignoring *") ||
	    MATCH (lcmsg, "*don't*ignore *,*") ||
	    MATCH (lcmsg, "*don't*ignore *") ||
	    MATCH (lcmsg, "*do not*ignore *,*") ||
	    MATCH (lcmsg, "*do not*ignore *") ||
	    (MATCH (lcmsg, "*listen*to *,*") ||
	     MATCH (lcmsg, "*listen*to *")) &&
	    !(sindex (res1, "not") || sindex (res1, "don't") ||
	      sindex (res1, "stop") || sindex (res1, "cease"))) &&
	   (pl = find_player (res3)) >= 0)
  { static long lastign = 0;

    strcpy (speaker, name);

    if (!PLAYER_GET (pl, PL_JERK))
    { if (isowner (name) && (trusting || *code && sindex (msg, code)) ||
	  randint (10) < (now - lastign))
      { reply (":is already listening to %s{, n}.", player[pl].name);
        lastign = now;
      }
    }
    else if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { PLAYER_CLR (pl, PL_JERK);
      reply ("\"Listening to player %s{, n}.", player[pl].name);
    }
    else if (randint (10) < (now - lastign))
    { reply (":tells %s %s would probably have %slistened to %s anyway.",
	     name, male ? "he" : "she", PLAYER_GET (pl, PL_JERK) ? "not " : "",
	     player[pl].name);
      lastign = now;
    }

    spoke_player (name);
    return (1);
  }

  /*---- What version ----*/
  else if ((MATCH (lcmsg, "*not listen to *,*") ||
	    MATCH (lcmsg, "*don't listen to *,*") ||
	    MATCH (lcmsg, "*stop listening to *,*") ||
	    MATCH (lcmsg, "*ignore *,*") ||
	    MATCH (lcmsg, "*ignore *")) &&
	   !streq (res2, "fuzzy") &&
	   ((pl = find_player (res2)) >= 0 ||
	    (!index (res2, ' ') && (pl = find_player (res2)) >= 0)))
  { static long lastign = 0;

    strcpy (speaker, name);

    if (PLAYER_GET (pl, PL_JERK))
    { if (isowner (name) && (trusting || *code && sindex (msg, code)) ||
	  randint (10) < (now - lastign))
      { reply (":is already ignoring %s{, n}.", player[pl].name);
        lastign = now;
      }
    }
    else if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { PLAYER_SET (pl, PL_JERK);
      reply ("\"Ignoring player %s{, n}.", player[pl].name);
    }
    else if (randint (10) < (now - lastign))
    { reply (":tells %s %s would probably have %slistened to %s anyway.",
	     name, male ? "he" : "she", PLAYER_GET (pl, PL_JERK) ? "not " : "",
	     player[pl].name);
      lastign = now;
    }

    spoke_player (name);
    return (1);
  }

  /*---- Stop remembering player ----*/
  else if ((MATCH (lcmsg, "*stop*remembering *,*") ||
	    MATCH (lcmsg, "*stop*remembering *") ||
	    MATCH (lcmsg, "*don't*remember *,*") ||
	    MATCH (lcmsg, "*don't*remember *") ||
	    MATCH (lcmsg, "*do not*remember *,*") ||
	    MATCH (lcmsg, "*do not*remember *")) &&
	   (pl = find_player (res3)) >= 0)
  { static long lastign = 0;

    strcpy (speaker, name);

    if (!PLAYER_GET (pl, PL_REMEMBER))
    { if (isowner (name) && (trusting || *code && sindex (msg, code)) ||
	  randint (10) < (now - lastign))
      { reply (":is already forgetting %s{, n}.", player[pl].name);
        lastign = now;
      }
    }
    else if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { PLAYER_CLR (pl, PL_REMEMBER);
      reply ("\"Removing player %s from the remember list{, n}.",
	      player[pl].name);
    }
    else if (randint (10) < (now - lastign))
    { reply (":tells %s %s would probably %s %s anyway.",
	     name, male ? "he" : "she",
	     PLAYER_GET (pl, PL_REMEMBER) ? "remembered " : "forgotten",
	     player[pl].name);
      lastign = now;
    }

    spoke_player (name);
    return (1);
  }

  /*---- Remember player ----*/
  else if ((MATCH (lcmsg, "*remember *,*") ||
	    MATCH (lcmsg, "*remember*")) &&
	   !streq (res2, "fuzzy") &&
	   ((pl = find_player (res2)) >= 0 ||
	    (!index (res2, ' ') && (pl = find_player (res2)) >= 0)))
  { static long lastign = 0;

    strcpy (speaker, name);

    if (PLAYER_GET (pl, PL_REMEMBER))
    { if (isowner (name) && (trusting || *code && sindex (msg, code)) ||
	  randint (10) < (now - lastign))
      { reply (":is already remembering %s{, n}.", player[pl].name);
        lastign = now;
      }
    }
    else if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { PLAYER_SET (pl, PL_REMEMBER);
      reply ("\"remembering player %s{, n}.", player[pl].name);
    }
    else if (randint (10) < (now - lastign))
    { reply (":tells %s %s would probably have %s %s anyway.",
	     name, male ? "he" : "she",
	     PLAYER_GET (pl, PL_REMEMBER) ? "remembered" : "forgotten",
	     player[pl].name);
      lastign = now;
    }

    spoke_player (name);
    return (1);
  }

  /*---- Take notes ----*/
  else if (MATCH (lcmsg, "*tak*note* in the *, *") ||
	   MATCH (lcmsg, "*tak*note* in the *.*") ||
	   MATCH (lcmsg, "*tak*note* in the *!*") ||
	   MATCH (lcmsg, "*tak*note* in the *") ||
	   MATCH (lcmsg, "*tak*note* in *, *") ||
	   MATCH (lcmsg, "*tak*note* in *.*") ||
	   MATCH (lcmsg, "*tak*note* in *!*") ||
	   MATCH (lcmsg, "*tak*note* in *") ||
	   MATCH (lcmsg, "*tak*note*here*") && strcpy (res4, here) ||
	   MATCH (lcmsg, "*tak*note*<*>*"))
  { strcpy (speaker, name);

    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { long rm = close_room (res4);

      if (rm < 0)
      { reply ("\"I've never heard of %s{, n}.", res4); }
      else
      { takingnotes++;
	hanging = 0;
	clear_page ();
	meetingroom = rm;
	reply ("\"I will now take notes in %s, %s",
	       room_name (meetingroom), name);
      }
      spoke_player (name);
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn off takingnotes ----*/
  else if (MATCH (lcmsg, "*tak*note*off*") ||
	   MATCH (lcmsg, "*stop*tak*note*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { takingnotes = 0;
      meetingroom = -1;
      reply ("\"I am not taking notes{, n}");
      return (1);
    }
    else
    { reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn off playinghearts ----*/
  else if (MATCH (lcmsg, "*play*heart*off*") ||
	   MATCH (lcmsg, "*stop*play*heart*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { playinghearts = 0;
      reply ("\"I am not playing hearts{, n}");
      return (1);
    }
    else
    { reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Checkpoint files ----*/
  else if ((MATCH (lcmsg, "*checkpoint*") ||
	    MATCH (lcmsg, "*save*map*") ||
	    MATCH (lcmsg, "*save*phone*")) &&
		!sindex (res1, "how") &&
		!sindex (res1, "why") &&
		!sindex (res1, "not") &&
		!sindex (res1, "don't"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (newrooms || isowner (name) && (trusting || *code && sindex (msg, code)))
    { checkpoint ();
      reply ("\"Ok, I saved my map and phone book{, n}.");
    }
    else
    { reply ("\"But I don't need to save my map right now{, n}."); }

    return (1);
  }

  /*---- Reload patterns ----*/
  else if (!contest_mode &&
	   (MATCH (lcmsg, "*reload*patterns*") ||
	    MATCH (lcmsg, "*re-load*patterns*")) &&
		!sindex (res1, "how") &&
		!sindex (res1, "why") &&
		!sindex (res1, "not") &&
		!sindex (res1, "don't"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (newrooms ||
	isowner (name) && (trusting || *code && sindex (msg, code)))
    { reset_cnet ();
      reply ("\"Ok, I reloaded my conversation patterns{, n}.");
    }
    else
    { reply ("\"Excuse me{, n}?"); }

    return (1);
  }

  /*---- Turn on contest_mode ----*/
  else if (MATCH (lcmsg, "*contest*mode*on*"))
  { strcpy (speaker, name);
    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { contest_mode++;
      spoke_player (name);
      reply ("\"Contest mode is now on{, n}");
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn off contest_mode ----*/
  else if (MATCH (lcmsg, "*contest*mode*off*"))
  { strcpy (speaker, name);
    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { contest_mode = 0;
      spoke_player (name);
      reply ("\"Contest mode is now off{, n}");
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn on exploring ----*/
  else if (MATCH (lcmsg, "*explor*on*"))
  { strcpy (speaker, name);
    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { exploring++;
      spoke_player (name);
      reply ("\"exploring is now on{, n}");
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Turn off exploring ----*/
  else if (MATCH (lcmsg, "*explor*off*"))
  { strcpy (speaker, name);
    if (isowner (name) && (trusting || *code && sindex (msg, code)))
    { exploring = 0;
      spoke_player (name);
      reply ("\"exploring is now off{, n}");
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me{, n}?");
      return (1);
    }
  }

  /*---- Obey command ----*/
  else if (MATCH (lcmsg, "*type <*>*") &&
	   !streq (res2, "go direction"))
  { strcpy (speaker, name);

    if (is_jerk (name)) return (1);
  
    if (isowner (name) && (trusting || *code && sindex (msg, code)) ||
	stlmatch (res2, "go ") && paging && !takingnotes &&
        !playinghearts && pagedto < 0)
    { strcpy (typecmd, res2);
      if (!terse) fprintf (stderr, "Set typecmd to '%s'\n", typecmd);
      hanging = 0; pgoal = 0; clear_page (); nextwait = 120;
    }
    else if (pagedto >= 0)
    { spoke_player (name);
      reply ("\"Sorry, I am on my way to %s right now, %s.",
	      room_name (pagedto), name);
    }
    else if (takingnotes)
    { spoke_player (name);
      reply ("\"I am busy taking notes right now{, n}.");
    }
    else if (playinghearts)
    { spoke_player (name);
      reply ("\"I am busy playing hearts right now{, n}.");
    }
    else if (paging)
    { spoke_player (name);
      reply ("\"I only obey 'go' commands, try 'type <go dir>', %s.",
	      name);
    }
    else
    { spoke_player (name);
      reply ("\"I am only listening to %s right now{, n}.", owner);
    }

    return (1);
  }

  /*---- Look at command ----*/
  else if ((MATCH (lcmsg, "*look at *'s desc*") ||
	    MATCH (lcmsg, "*look at *") ||
	    MATCH (lcmsg, "*check out *'s desc*") ||
	    MATCH (lcmsg, "*check out *") ||
	    MATCH (lcmsg, "*examine *'s desc*") ||
	    MATCH (lcmsg, "*examine *") ||
	    MATCH (lcmsg, "*read *'s desc*") ||
	    MATCH (lcmsg, "*read *") ||
	    MATCH (lcmsg, "*get *'s desc*") ||
	    MATCH (lcmsg, "*get * desc*")) &&
	   (sindex ("me,my", car (res2)) && (pl = find_player (name)) >= 0 ||
	    (pl = find_player (car (res2))) >= 0))
  { strcpy (speaker, name);
    spoke_player (name);
      
    if (is_jerk (name)) return (1);

    if (player[pl].present)
    { reply ("\"Okay{, n}, I'll look at %s.",
	     strfoldeq (name, player[pl].name) ? "you" : player[pl].name);
      look_at_thing (player[pl].name);
    }
    else if (strfoldeq (name, player[pl].name))
    { reply ("\"But you aren't here right now{, n}"); }
    else
    { reply ("\"But %s isn't here right now{, n}", player[pl].name); }

    return (1);
  }

  /*---- A query about 'who belong' ----*/
  if (MATCH (lcmsg, "*who*you*belong*") ||
      MATCH (lcmsg, "*whose*are*you*") ||
      MATCH (lcmsg, "*who*you*owner*") ||
      MATCH (lcmsg, "*who*work*") ||
      MATCH (lcmsg, "*who*you*master*") ||
      MATCH (lcmsg, "*who*own*you*"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"I work mainly for %s{, n}.", owner);
    return (1);
  }

  /*---- A query about 'who created' ----*/
  if (MATCH (lcmsg, "*who*built*you*") ||
      MATCH (lcmsg, "*who*made*you*") ||
      MATCH (lcmsg, "*who*created*you*") ||
      MATCH (lcmsg, "*who*wrote*you*") && !sindex (lcmsg, "song"))
  { strcpy (speaker, name);
    spoke_player (name);
    if (streq (lcstr (name), "fuzzy"))
    { if (isowner (name))
      { reply ("\"You did{, n}."); }
      else
      { reply ("\"You did{, n}, but I belong to %s.", owner); }
    }
    else
    { reply ("\"Fuzzy created me, but I belong to %s{, n}.", owner); }
    return (1);
  }

  /*---- Who is here ----*/
  else if (MATCH (lcmsg, "*who is*sleep*") ||
	   MATCH (lcmsg, "*is*anyone*sleep*") ||
	   MATCH (lcmsg, "*is*anybody*sleep*") ||
	   MATCH (lcmsg, "*who's*sleep*"))
  { strcpy (speaker, name);
    spoke_player (name);
    asleep_query (name);
    return (1);
  }    

  /*---- Who is here ----*/
  else if (MATCH (lcmsg, "*who is* there*") ||
	   MATCH (lcmsg, "*who's* there*") ||
	   MATCH (lcmsg, "*who is* here*") ||
	   MATCH (lcmsg, "*who's* here*") ||
	   MATCH (lcmsg, "*quien*esta*aqui*") ||
	   MATCH (lcmsg, "*who is* awake*") ||
	   MATCH (lcmsg, "*who's* awake*"))
  { strcpy (speaker, name);
    spoke_player (name);
    awake_query (name);
    return (1);
  }    

  /*---- Who is connected ----*/
  else if ((sindex (lcmsg, "list players ")  ||
	    sindex (lcmsg, "list users ")  ||
	    sindex (lcmsg, "who is ")  ||
	    sindex (lcmsg, "who's ")  ||
	    sindex (lcmsg, "what players are ")  ||
	    sindex (lcmsg, "which players ")) &&
	   (sindex (lcmsg, " connect") ||
	    sindex (lcmsg, " active") ||
	    sindex (lcmsg, " logged") ||
	    sindex (lcmsg, " on,") ||
	    ematch (lcmsg, " on")))
  { strcpy (speaker, name);
    spoke_player (name);
    connected_query (name);
    return (1);
  }    

  /*---- Write reachable room map ----*/
  else if (isowner (name) && (trusting || *code && sindex (msg, code)) &&
	   (MATCH (lcmsg, "*write*small*map*") ||
	    MATCH (lcmsg, "*write*small*room*") ||
	    MATCH (lcmsg, "*dump*small*map*") ||
	    MATCH (lcmsg, "*dump*small*room*")))
  { strcpy (speaker, name);
    spoke_player (name);
    write_map ("reach.map", 2);
    reply ("\"I dumped it to 'reach.map'{, n}");
    return (1);
  }

  /*---- Write reachable room map ----*/
  else if (isowner (name) && (trusting || *code && sindex (msg, code)) &&
	   (MATCH (lcmsg, "*write*reach*map*") ||
	    MATCH (lcmsg, "*write*reach*room*") ||
	    MATCH (lcmsg, "*dump*reach*map*") ||
	    MATCH (lcmsg, "*dump*reach*room*")))
  { strcpy (speaker, name);
    spoke_player (name);
    write_map ("reach.map", 1);
    reply ("\"I dumped it to 'reach.map'{, n}");
    return (1);
  }

  /*---- Dump map ----*/
  else if (MATCH (lcmsg, "*dump*") && isowner (name) && (trusting || *code && sindex (msg, code)))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"I dumped it to stderr{, n}");
    return (1);
  }

  /*---- Memory Usage ----*/
  else if (!contest_mode &&
	   (MATCH (lcmsg, "*what*you*memory*") ||
	    MATCH (lcmsg, "*what*you*disk*") ||
	    MATCH (lcmsg, "*what*you* ram *") ||
	    MATCH (lcmsg, "*how*is*memory*") ||
	    MATCH (lcmsg, "*how*much*memory*") ||
	    MATCH (lcmsg, "*how*you*memory*") ||
	    MATCH (lcmsg, "*how*much* ram *you*") ||
	    MATCH (lcmsg, "*how*much*disk*you*") ||
	    MATCH (lcmsg, "*how*much*you*disk*") ||
	    MATCH (lcmsg, "*how*big*you*memory*") ||
	    MATCH (lcmsg, "*how*big*you* ram*")))
  { strcpy (speaker, name);
    spoke_player (name);

    reply ("\"Here is a summary of my memory usage{, n}:");
    strcpy (speaker, name);
    reply ("\" %ld bytes for %ld strings", string_sp, string_ct);
    strcpy (speaker, name);
    reply ("\" %ld bytes for %ld exits", exit_sp, exit_ct);
    strcpy (speaker, name);
    reply ("\" %ld bytes for rooms", room_sp);
    strcpy (speaker, name);
    reply ("\" %ld bytes for paths", path_sp);
    strcpy (speaker, name);
    reply ("\" %ld bytes for players", player_sp);
    strcpy (speaker, name);
    reply ("\" %ld bytes for %ld dialog entries", dialog_sp, dialog_ct);
    strcpy (speaker, name);
    reply ("\"That's %ld bytes all together.",
	   string_sp + exit_sp + room_sp + path_sp + player_sp + dialog_sp);
    return (1);
  }

  /*---- Turn off generosity ----*/
  else if (MATCH (lcmsg, "*genero*off*") ||
	   MATCH (lcmsg, "*off*genero*") ||
	   MATCH (lcmsg, "*not*genero*") ||
	   MATCH (lcmsg, "*don*be*genero*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner (name) && (trusting || *code && sindex (msg, code)) || !generous)
    { generous = 0;
      reply ("\"I'm not feeling generous, anymore{, n}");
      return (1);
    }
    else
    { reply ("\"But I want to be generous{, n}."); return (1); }
  }

  /*---- Turn on generosity ----*/
  else if (MATCH (lcmsg, "*genero*on*") ||
	   MATCH (lcmsg, "*on*genero*") ||
	   MATCH (lcmsg, "*be*genero*") ||
	   MATCH (lcmsg, "*feel*genero*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner (name) && (trusting || *code && sindex (msg, code)) || generous)
    { generous++;
      reply ("\"I'm in a generous mood{, n}.");
      return (1);
    }
    else
    { reply ("\"But I feel really grouchy, today{, n}."); return (1); }
  }

  /*---- Turn off vindictiveness ----*/
  else if (MATCH (lcmsg, "*vindic*off*") ||
	   MATCH (lcmsg, "*off*vindic*") ||
	   MATCH (lcmsg, "*not*vindic*") ||
	   MATCH (lcmsg, "*don*be*vindic*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner (name) && (trusting || *code && sindex (msg, code)) || !vindictive)
    { vindictive = 0;
      reply ("\"I'm not feeling vindictive, anymore{, n}");
      return (1);
    }
    else
    { reply ("\"But I want to be vindictive{, n}."); return (1); }
  }

  /*---- Turn on vindictiveness ----*/
  else if (MATCH (lcmsg, "*vindic*on*") ||
	   MATCH (lcmsg, "*on*vindic*") ||
	   MATCH (lcmsg, "*be*vindic*") ||
	   MATCH (lcmsg, "*feel*vindic*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner (name) && (trusting || *code && sindex (msg, code)) || vindictive)
    { vindictive++;
      reply ("\"I'm in a vindictive mood{, n}.");
      return (1);
    }
    else
    { reply ("\"But I don't feel really grouchy, today{, n}.");
      return (1);
    }
  }

  /*---- Turn off visiting old rooms ----*/
  else if (MATCH (lcmsg, "*visit*old*off*") ||
	   MATCH (lcmsg, "*off*visit*old*") ||
	   MATCH (lcmsg, "*not*visit*old*") ||
	   MATCH (lcmsg, "*don*t visit*old*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner (name) && (trusting || *code && sindex (msg, code)) || !visitold)
    { visitold = 0;
      reply ("\"I won't go visiting old rooms, %s", name);
      return (1);
    }
    else
    { reply ("\"But I want to see if they're still there{, n}.");
      return (1);
    }
  }

  /*---- Turn on visiting old rooms ----*/
  else if (MATCH (lcmsg, "*visit*old*on*") ||
	   MATCH (lcmsg, "*on*visit*old*") ||
	   MATCH (lcmsg, "*visit*old*") &&
	   !(sindex (res1, "not") || sindex (res1, "don't")))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner (name) && (trusting || *code && sindex (msg, code)) || visitold)
    { visitold++;
      reply ("\"I will visit old rooms to check on them{, n}.");
      return (1);
    }
    else
    { reply ("\"But I don't want to go looking at old rooms{, n}.");
      return (1);
    }
  }

  return (0);
}


/****************************************************************
 * give_general_help
 ****************************************************************/

char *help_rooms[] = {
  "Reference Room",
  "Encyclopedia Alcove",
  "Map Room",
  "Historical Archives",
  "The Islandia Directory",
  "Kiosk",
  "Black Room - Islandia Introduction Rooms",
  "Library lobby",
  "Circulating Room",
  "Oxford English Dictionary Room",
  "Encyclopedia Collection",
  "The TFM Room",
  NULL
};

give_general_help (query, name)
char *query, *name;
{ long from=hererm, to;
  char *answer = NULL;
  long i, numhelp, cnt=0, local=1;

  fprintf (stderr, "Help: msg '%-32.32s', name '%s'\n", query, name);
  
  if (msgtype >= M_PAGE && (i = close_room ("nexus", NULL)) >= 0)
  { from = i; local = 0; }

  numhelp = sizeof (help_rooms) / sizeof (*help_rooms) - 1;
  i = randint (numhelp);

  while (cnt++ < numhelp)
  { if (++i >= numhelp) i = 0;

    if (!stlmatch (here, help_rooms[i]) &&
	(to = close_room (help_rooms[i], NULL)) >= 0 &&
	(answer = find_path (from, to, SHORTPATH)))
    {
      switch (nrrint (141, 3))
      { case 0: reply ("\"Hmm...you should read the manual in %s, %s.",
		       help_rooms[i], name);
		break;
	case 1: reply ("\"%s in %s, %s.",
		       "Perhaps you will find the answer you seek",
		       help_rooms[i], name);
		break;
	case 2: reply ("\"You should check out %s, %s.",
		       help_rooms[i], name);
		break;
      }
      
      strcpy (speaker, name);

      reply ("\"From %s go %s.",
	     local ? "here" : room_name (from), answer);

      return (1);
    }
  }

  if (strfoldeq (world, "Time Traveller"))
  { reply ("\"Try the help command{, n}."); }
  else
  { reply ("\"I'm sorry I can't help you, I don't know where to look{, n}."); }
  return (1);
}

/****************************************************************
 * give_robot_help
 ****************************************************************/

give_robot_help (query, name)
char *query, *name;
{ long from, to, town;
  char *answer = NULL;
  long i, numhelp, cnt=0;

  fprintf (stderr, "Help: robot help '%-32.32s', name '%s'\n", query, name);

  if (sindex (query, "ftp") ||
      sindex (query, "source") ||
      sindex (query, ".c") ||
      sindex (query, "code") ||
      (to = close_room ("robot user's guide")) < 0 ||
      !((msgtype >= M_PAGE && (town = close_room ("nexus", NULL)) >= 0) &&
	(answer = find_path (town, to, SHORTPATH)) && (from = town) >= 0 ||

	(answer = find_path (hererm, to, SHORTPATH)) && (from = hererm) >= 0 ||
	(answer = find_path (homerm, to, SHORTPATH)) && (from = homerm) >=0))
  { reply ("\"The %s on host %s, %s. %s, %s.",
	   "source code for Maas-Neotek robots is available",
	   "NL.CS.CMU.EDU [128.2.222.56]",
	   "directory /usr/mlm/ftp, file robot.tar.Z",
	   "set mode binary first, and cd to /usr/mlm/ftp in one step",
	   name);
    return (0);
  }

  switch (nrrint (142, 3))
  { case 0: reply ("\"Hmm...you should read the manual in %s, %s.",
		   room_name (to), name);
	    break;
    case 1: reply ("\"%s in %s, %s.",
		   "Perhaps you will find the answer you seek",
		   room_name (to), name);
	    break;
    case 2: reply ("\"You should check out %s, %s.",
		   room_name (to), name);
	    break;
  }

  strcpy (speaker, name);

  reply ("\"From %s, go %s.",
	 (from == hererm) ? "here" : room_name (homerm), answer);

  return (1);
}

/****************************************************************
 * polite_msg: Handle 'please' and 'thank you' fluff.  Remember to
 *	       match others polite messages so they will be ignored.
 ****************************************************************/

polite_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ long pl;
  static long msgno = 1;
  static long lastmsg = 0;
  char *det, *rest, *onm, *obj, *modal;
  int past = 0;
  int foundother = 0;
  int cool = 0;

  /* After a few minutes, reset the help cycle */
  if (lastmsg + 600 < now)
  { msgno = 1; lastmsg = now; }

  /*---- A greeting ----*/
  if (MATCH (lcmsg, "*hello*") ||
      MATCH (lcmsg, "*hullo*") ||
      MATCH (lcmsg, "*salutat*") ||
      MATCH (lcmsg, "*hiya*") && ++cool ||
      MATCH (lcmsg, "*heya*") && ++cool ||
      MATCH (lcmsg, "*h'lo*") ||
      MATCH (lcmsg, "hey*") ||
      MATCH (lcmsg, "*, hey") ||
      MATCH (lcmsg, "*you*re*back*") ||
      MATCH (lcmsg, "*howdy*") ||
      MATCH (lcmsg, "ho, **") ||
      MATCH (lcmsg, "mornin*") ||
      MATCH (lcmsg, "evenin*") ||
      MATCH (lcmsg, "hi,*") ||
      MATCH (lcmsg, "hi *") ||
      MATCH (lcmsg, "hi") ||
      MATCH (lcmsg, "*, hi") ||
      MATCH (lcmsg, "*re-hi*") && !isalpha (*res2) ||
      MATCH (lcmsg, "*rehi*") && !isalpha (*res2) ||
      MATCH (lcmsg, "*greet*") ||
      MATCH (lcmsg, "*y0*") && ++cool ||
      (MATCH (lcmsg, "yo *") ||
       MATCH (lcmsg, "yo, *")) && ++cool ||
      MATCH (lcmsg, "*'lo*") ||
      MATCH (lcmsg, "*'ello*") ||
      MATCH (lcmsg, "*good*morn*") ||
      MATCH (lcmsg, "*good*afternoon*") ||
      MATCH (lcmsg, "*good*evening*") ||
      MATCH (lcmsg, "*shalom*") ||
      MATCH (lcmsg, "*hola*") ||
      (MATCH (lcmsg, "*, babe") ||
       MATCH (lcmsg, "*! babe") ||
       MATCH (lcmsg, "*. babe") ||
       MATCH (lcmsg, "* babe")) && is_me (res1) && ++cool ||
      MATCH (lcmsg, "*bon*jour*") ||
      MATCH (lcmsg, "*bon*jorn*") ||
      MATCH (lcmsg, "*buenos*dias*") ||
      MATCH (lcmsg, "*long time* see*") ||
      MATCH (lcmsg, "*seen* long time*") ||
      MATCH (lcmsg, "*buenos*noches*") ||
      MATCH (lcmsg, "*ohayo*") ||
      MATCH (lcmsg, "*konban*wa*") ||
      MATCH (lcmsg, "*konnichi*wa*") ||
      MATCH (lcmsg, "*g'day*") ||
      MATCH (lcmsg, "*aloha*") ||
      MATCH (lcmsg, "*shalom*") ||
      MATCH (lcmsg, "*hallo*") ||
      MATCH (lcmsg, "*guten*tag*") ||
      MATCH (lcmsg, "*guten*morgen*") ||
      MATCH (lcmsg, "*greet*"))
  { char *other = name;

    /* If in a crowd, there is a chance they do not mean the robot */
    if (alone > 1 &&
	msgtype == M_SPOKEN &&
	!sindex (lcmsg, "juli") && !sindex (lcmsg, lcstr (myname)) &&
	randint (100) > 50) 
    { return (1); }

    /* If we just heralded them, no reply to hello is needed */
    if ((pl = find_player (name)) >= 0 &&
	now < player[pl].lastheralded + 150)
    { return (1); }

    /*---- Okay, we will answer ----*/
    strcpy (speaker, name);
    
    /* Use cool response? */
    if (sindex (lcmsg, "y0")) cool++;
    if (sindex (lcmsg, "yo")) cool++;
    if (sindex (lcmsg, "julie")) cool++;
    if (sindex (lcmsg, "baby")) cool++;
    if (sindex (lcmsg, "babe")) cool++;
    if (sindex (lcmsg, "d00d")) cool += 2;
    if (sindex (lcmsg, "dude")) cool++;
    if (sindex (lcmsg, "sweet")) cool++;
    if (sindex (lcmsg, "dood")) cool++;
    if (randint (100) < 25) cool++;
    if (randint (100) < 25) cool	++;

    if (cool) other = doodify (name, cool * 40);
    
    /* Reply with a similar hello if we havent spoken to them recently */
    if (pl < 0 || now > player[pl].lastspoke + 300)
    { if (sindex (lcmsg, "aloha"))	 reply ("\"Aloha, %s.", other);
      else if (sindex (lcmsg, "buenos")) reply ("\"Buenos dias, %s.", other);
      else if (sindex (lcmsg, "y0"))	 reply ("\"y0, %s.", other);
      else if (sindex (lcmsg, "g'day"))	 reply ("\"G'Day, mate!");
      else if (sindex (lcmsg, "shalom")) reply ("\"Shalom, %s.", other);
      else if (sindex (lcmsg, "guten"))  reply ("\"Wie gehts, %s?", other);
      else				 reply ("\"Hello, %s.", other);

      spoke_player (name);
    }

    /* Acknowledge a hello if present */
    else if (pl >= 0 && player[pl].present)
    { reply (":nods to %s.", other); }

    return (1);
  }

  /*---- Tell robot to be quiet ----*/
  else if (MATCH (lcmsg, "*shut*up*") ||
	   MATCH (lcmsg, "*be*quiet*") ||
	   MATCH (lcmsg, "*stop*talk*") ||
	   MATCH (lcmsg, "*i*was not*talking*") ||
	   MATCH (lcmsg, "*i*wasn't*talking*"))
  { spoke_player (name);
    strcpy (speaker, "");
    speaktime = 0;
    return (1);
  }

  /*---- Follow me ----*/
  else if (MATCH (lcmsg, "*follow*me*") ||
           MATCH (lcmsg, "*come*with*me*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (contest_mode)
    { reply ("\"Where should I follow you to{, n}?"); }
    else if (paging)
    { reply ("\"I %s{, n}, but you can tell me to 'go dir, %s'",
		"don't know how to follow you", myname);
    }
    else
    { reply ("\"I'm sorry{, n}, I can't right now."); }
    
    return (1);
  }    

  /*---- Follow me ----*/
  else if (!is_question (lcmsg) &&
	   (MATCH (lcmsg, "find *,*") && (obj = res1) ||
            MATCH (lcmsg, "find *") && (obj = res1) ||
            MATCH (lcmsg, "* find *") && (obj = res2) ||
            MATCH (lcmsg, "*locate *,*") && (obj = res2) ||
            MATCH (lcmsg, "*locate *") && (obj = res2) ||
            MATCH (lcmsg, "*search for *,*") && (obj = res2) ||
            MATCH (lcmsg, "*search for *") && (obj = res2) ||
            MATCH (lcmsg, "*look for *,*") && (obj = res2) ||
            MATCH (lcmsg, "*look for *") && (obj = res2) ||
            MATCH (lcmsg, "*seek out *") && (obj = res2) ||
            MATCH (lcmsg, "*seek foor *") && (obj = res2) ||
            MATCH (lcmsg, "*seek *") && (obj = res2)))
  { strcpy (speaker, name);
    spoke_player (name);

    switch (nrrint (143, 9))
    { case 0: zinger ("\"Where should I look for %s{, n}?", obj); break;
      case 1: zinger ("\"Where can I find %s{, n}?", obj); break;
      case 2: zinger ("\"Where should I start{, n}?"); break;
      case 3: zinger ("\"What is this, hind-and-seek{, n}?"); break;
      case 4: zinger ("\"Oh goody, a scavenger hunt.", obj); break;
      case 5: zinger ("\"Do I look like a golden retriever to you{, n}?");
	      break;
      case 6: zinger ("\"What features characterize %s{, n}?", obj);
	      break;
      case 7: zinger ("\"Find %s yourself{, n}, I'm busy.", obj); break;
      default: zinger ("\"I'm sorry{, n}, but %s, %s.",
		       randint (100) < 50 ? "I can't find things" :
		       "I don't know how to look for specific things",
		       randint (100) < 50 ? "I just have a good memory" :
		       "I just wander around, remembering what I see");

    }

    
    return (1);
  }

  /*---- Happy Birthday ----*/
  else if (MATCH (lcmsg, "*happy*birthday*") &&
	   !sindex (lcmsg, " not ") &&
	   !sindex (lcmsg, "unhappy"))
  { struct tm bday, today;

    strcpy (speaker, name);
    spoke_player (name);
    
    /* Warning, structure assignment ahead */
    bday = *localtime ((time_t *) &creation);
    today = *localtime ((time_t *) &now);

    if (bday.tm_mon == today.tm_mon &&
	bday.tm_mday == today.tm_mday)
    { zinger ("\"Thanks{, n}"); }
    else
    { zinger ("\"It's not my birthday, I was born on %s %d, %d.",
	      mname[bday.tm_mon], bday.tm_mday, bday.tm_year + 1900);
    }
    return (1);
  }

  /*---- Happy Birthday ----*/
  else if (MATCH (lcmsg, "*what*you*age*") ||
	   MATCH (lcmsg, "*when*you*birth*day*") ||
	   MATCH (lcmsg, "*what*you*birth*day*") ||
	   MATCH (lcmsg, "*when*you*birth*date*") ||
	   MATCH (lcmsg, "*what*you*birth*date*") ||
	   MATCH (lcmsg, "*when*you*date*birth*") ||
	   MATCH (lcmsg, "*what*you*date*birth*") ||
	   MATCH (lcmsg, "*when*you*born*") ||
	   MATCH (lcmsg, "*when*create*you*") ||
	   MATCH (lcmsg, "*when*you*created*"))
  { struct tm bday, today;

    strcpy (speaker, name);
    spoke_player (name);
    
    /* Warning, structure assignment ahead */
    bday = *localtime ((time_t *) &creation);

    zinger ("\"I was born on %s %d, %d.",
	    mname[bday.tm_mon], bday.tm_mday, bday.tm_year + 1900);

    return (1);
  }

  /*---- Happy New Year ----*/
  else if ((MATCH (lcmsg, "*happy*new*year*") ||
            MATCH (lcmsg, "*ano*prospero*")) &&
	   !sindex (lcmsg, "to you, too"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"Happy New Year to you, too{, n}!");
    return (1);
  }

  /*---- Merry Christmas ----*/
  else if ((MATCH (lcmsg, "*merry*christmas*") ||
            MATCH (lcmsg, "*feliz*navidad*")) &&
	   !sindex (lcmsg, "to you, too"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"Merry Christmas to you, too{, n}!");
    return (1);
  }

  /*---- Happy Valentines ----*/
  else if ((MATCH (lcmsg, "*happy*valentine*") ||
            MATCH (lcmsg, "*be*my*valentine*")) &&
	   !sindex (lcmsg, "to you, too"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"Happy Valentine's Day to you, too{, n}!");
    return (1);
  }

  /*---- Happy Hanukkah ----*/
  else if ((MATCH (lcmsg, "*happy*hanukkah*") ||
            MATCH (lcmsg, "*happy*hanukah*")) &&
	   !sindex (lcmsg, "to you, too"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"Happy Hanukkah to you, too{, n}!");
    return (1);
  }

  /*---- listen me ----*/
  else if (MATCH (lcmsg, "*listen*me*") ||
           MATCH (lcmsg, "*obey*me*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (paging)
    { reply ("\"I'm listening{, n}."); }
    else
    { reply ("\"I'm sorry{, n}, I can't right now."); }
    
    return (1);
  }

  /*---- is X a robot ----*/
  else if ((MATCH (lcmsg, "*am i a robot*") && strcpy (res2, "you") ||
	    MATCH (lcmsg, "*am i an automat*") && strcpy (res2, "you") ||
	    MATCH (lcmsg, "*am i a bot**") && strcpy (res2, "you") ||
	    MATCH (lcmsg, "*is * a robot*") ||
	    MATCH (lcmsg, "*is * a bot*") ||
	    MATCH (lcmsg, "*is * an automat*") ||
	    MATCH (lcmsg, "*is * a computer*program*")) &&
	   !(sindex (lcmsg, "what is") || sindex (lcmsg, "what's")))
  { strcpy (speaker, name);
    spoke_player (name);

    if (is_me (res2))
    { sleep (2);

      switch (nrrint (144, 4))
      { case 0:	zinger ("\"%s who{, n}?", myname); break;
        case 1:	zinger ("\"Do I look like a robot to you{, n}?."); break;
        case 2:	zinger (":winks at %s.", name); break;
        case 3:	zinger (":pinches herself to check."); break;
      }
    }
    else
    { switch (nrrint (145, 4))
      { case 0:	zinger ("\"I don't know whether %s %s a robot, %s.",
			res2, streq (res2, "you") ? "are" : "is", name); break;
        case 1:	zinger ("\"Gee %s, I don't know about %s, do you?.",
			name, res2); break;
        case 2:	zinger (":doesn't know whether %s %s a robot.",
			res2, streq (res2, "you") ? "are" : "is"); break;
        case 3:	zinger (":isn't sure whether %s %s a robot.", res2,
			streq (res2, "you") ? "are" : "is"); break;
      }
    }
    
    return (1);
  }
  
  /*---- is X a robot ----*/
  else if ((MATCH (lcmsg, "*'s a robot*") ||
	    MATCH (lcmsg, "* is a robot*") ||
	    MATCH (lcmsg, "* is a bot*") ||
	    MATCH (lcmsg, "*'s a bot*") ||
	    MATCH (lcmsg, "* is an automat*") ||
	    MATCH (lcmsg, "* is a computer*program*")) &&
	   !streq (last (res1), "who") &&
	   !streq (last (res1), "what") &&
	   !streq (last (res1), "which") &&
	   ((streq (last (res1), "i") && (pl = find_player (name)) >= 0) ||
	    (pl = find_player (last (res1))) >= 0))
	   
  { strcpy (speaker, name);
    spoke_player (name);

    if (is_me (last (res1)))
    { sleep (2);

      switch (nrrint (146, 4))
      { case 0:	zinger ("\"%s who{, n}?", myname); break;
        case 1:	zinger ("\"Do I look like a robot to you{, n}?."); break;
        case 2:	zinger (":winks at %s.", name); break;
        case 3:	zinger (":pinches %sself to check.",
			male ? "him" : "her"); break;
      }
    }
    else
    { switch (nrrint (147, 4))
      { case 0:	zinger ("\"I didn't know that %s is a robot, %s.",
			player[pl].name, name); break;
        case 1:	zinger ("\"Gee %s, are you sure about %s?.",
			name, player[pl].name); break;
        case 2:	zinger (":refuses to believe that %s is a robot.",
			player[pl].name); break;
        case 3:	zinger (":is surprised to hear that %s is a robot.",
			player[pl].name); break;
      }
    }
    
    return (1);
  }
  
  /*---- Ignore help from other robots ----*/
  else if (sindex (lcmsg, ", type '") ||
	   stlmatch (lcmsg, "description:") ||
	   stlmatch (lcmsg, "here:") ||
	   stlmatch (lcmsg, "contents:"))
  { return (1); }

  /*---- Help! ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*what*interesting*") && (msgno = 0) ||
	   MATCH (lcmsg, "*what*should* i *") && (msgno = 0) ||
	   MATCH (lcmsg, "*help*start*") && (msgno = 1) ||
	   MATCH (lcmsg, "*i *need*info*") && (msgno = 1) ||
	   MATCH (lcmsg, "*i *want*info*") && (msgno = 1) ||
	   (MATCH (lcmsg, "*i am*lost*") && (msgno = 1) ||
	    MATCH (lcmsg, "*i am*new*") && (msgno = 1) ||
	    MATCH (lcmsg, "*i'm*lost*") && (msgno = 1) ||
	    MATCH (lcmsg, "*i'm*new*") && (msgno = 1)) &&
		!sindex (res2, " not ") ||
	   MATCH (lcmsg, "*help*donat*") && (msgno = 2) ||
	   MATCH (lcmsg, "*help*give*") && (msgno = 2) ||
	   MATCH (lcmsg, "*help*pennies*") && (msgno = 2) ||
	   MATCH (lcmsg, "*help*quote*") && (msgno = 2) ||
	   MATCH (lcmsg, "*directions*") && (msgno = 3) ||
	   MATCH (lcmsg, "*give*map*") && (msgno = 3) ||
	   MATCH (lcmsg, "*help*map*") && (msgno = 3) ||
	   MATCH (lcmsg, "*help*rooms*") && (msgno = 4) ||
	   MATCH (lcmsg, "*help*people*") && (msgno = 5) ||
	   MATCH (lcmsg, "*help*someone*") && (msgno = 5) ||
	   MATCH (lcmsg, "*give*directions*") && (msgno = 6) ||
	   MATCH (lcmsg, "*help*show*") && (msgno = 6) ||
	   MATCH (lcmsg, "*help*teach*") && (msgno = 6) ||
	   MATCH (lcmsg, "*type*") && (msgno = 6) ||
	   MATCH (lcmsg, "*help*time*") && (msgno = 7) ||
	   MATCH (lcmsg, "*help*date*") && (msgno = 8) ||
	   MATCH (lcmsg, "*help*exit*to*") && (msgno = 10) ||
	   MATCH (lcmsg, "*help*exit*") && (msgno = 9) ||
	   MATCH (lcmsg, "*help*inventory*") && (msgno = 11) ||
	   MATCH (lcmsg, "*help*item*") && (msgno = 11) ||
	   MATCH (lcmsg, "*help*object*") && (msgno = 11) ||
	   MATCH (lcmsg, "*help*room*") && (msgno = 12) ||
	   MATCH (lcmsg, "*help*descri*") && (msgno = 13) ||
	   MATCH (lcmsg, "*help*source*") && (msgno = 14) ||
	   MATCH (lcmsg, "*help*program*") && (msgno = 14) ||
	   MATCH (lcmsg, "*help*robot*") && (msgno = 14) ||
	   MATCH (lcmsg, "*help*") ||
	   MATCH (lcmsg, "*what*can*you*do*") && (msgno < 3 ? 3 : msgno) ||
	   MATCH (lcmsg, "*what*you*capab*l*") && (msgno < 3 ? 3 : msgno) ||
	   MATCH (lcmsg, "*what*do*you*know*") && (msgno < 3 ? 3 : msgno) ||
	   MATCH (lcmsg, "*commands*") && (msgno < 3 ? 3 : msgno) ||
	   MATCH (lcmsg, "*assist*") ||
	   MATCH (lcmsg, "*ayuda*") ||
	   MATCH (lcmsg, "*helfen*") ||
	   MATCH (lcmsg, "*tasuke*") ||
	   MATCH (lcmsg, "*i*hav*problem*") ||
	   MATCH (lcmsg, "*i*hav*hard*time*") ||
	   MATCH (lcmsg, "*instruct*")))
  { strcpy (speaker, name);
    spoke_player (name);

    switch (msgno)
    { case 0:	give_general_help (lcmsg, name); break;
      case 1:	if (strfoldeq (world, "Time Traveller"))
		{ reply ("\"%s, and %s, %s.",
	                 "The 'wizards' command lists the people in charge",
			 "there is a 'news' and a 'help' command",
		         name);
		}
		else
		{ reply ("\"%s, and %s, %s.",
	             "The 'help' command describes the basic TinyMUD commands",
		     "the Library has a lot of information, too", name);
		}
	        break;
      case 2:	reply ("\"To give a donation, type 'give %s=1', %s.",
		      myname, name); break;
      case 3:	reply ("\"To ask for directions, type '%s, %s?', %s",
		      "say How do I get to X", myname, name); break;
      case 4:	reply ("\"To see %s, type '%s, %s?', %s",
		      "what rooms I've discovered", 
		      "say Where have you been", myname, name); break;
      case 5:	reply ("\"To find out about people, type '%s, %s?', %s",
		      "say Who have you seen", myname, name); break;
      case 6:	reply ("\"To give %s, type '%s, %s.' or '%s, %s.', %s",
		      "me directions", "say go direction", myname,
		      "say type <go direction>", myname, name); break;
      case 7:	reply ("\"To ask for the time, type '%s, %s?', %s",
		      "say What time is it", myname, name); break;
      case 8:	reply ("\"To ask for the date and time, type '%s, %s?', %s",
		      "say What is the date", myname, name); break;
      case 9:	reply ("\"%s, type '%s, %s?', %s",
		      "To find out about exits from a room",
		      "say What exits go from X", myname, name);
		      break;
      case 10:	reply ("\"%s, type '%s, %s?', %s",
		      "To find out about exits to a room",
		      "say What exits go to X", myname, name);
		      break;
      case 11:	reply ("\"%s, type '%s, %s?', %s",
		      "To find out who has an item",
		      "say Who carries an X", myname, name);
		      break;
      case 12:	reply ("\"%s, type '%s, %s?', %s",
		      "To find out what rooms match a pattern",
		      "say What rooms match <pattern>", myname, name);
		      break;
      case 13:	reply ("\"%s, type '%s, %s?', %s",
		      "To find out what room descriptions match a pattern",
		      "say What room descriptions match <pattern>",
		      myname, name);
		      break;
      case 14:	give_robot_help (lcmsg, name); break;
      default: reply ("\"I don't know what else to say{, n}.");
    }
    
    if (++msgno > 15) msgno = 0;

    lastmsg = now;
    return (1);
  }

  /*---- A query about 'how you' ----*/
  else if ((MATCH (lcmsg, "*how*are*you*") ||
	    MATCH (lcmsg, "*how*old*you*") ||
	    MATCH (lcmsg, "*how*are* ya") ||
	    MATCH (lcmsg, "*how*do*you*") ||
	    MATCH (lcmsg, "*how's my*favor*") ||
	    MATCH (lcmsg, "*how is my*favor*") ||
	    MATCH (lcmsg, "*how's*life*") ||
	    MATCH (lcmsg, "*how is* it *goin*") ||
	    MATCH (lcmsg, "*how's* it *goin*") ||
	    MATCH (lcmsg, "*how is*life*") ||
	    MATCH (lcmsg, "*are* you* ok*") ||
	    MATCH (lcmsg, "*are* you* alright*") ||
	    MATCH (lcmsg, "*wie*gehts*") ||
	    MATCH (lcmsg, "*how*have*you*been*") ||
	    MATCH (lcmsg, "*are*you*ok*") ||
	    MATCH (lcmsg, "*are*you*well*") ||
	    MATCH (lcmsg, "*are*you*feeling*") ||
	    MATCH (lcmsg, "*are*you*alright*") ||
	    MATCH (lcmsg, "*como*esta*") ||
	    MATCH (lcmsg, "*que*pasa*") ||
	    MATCH (lcmsg, "*que*tal*")) &&
	   !(sindex (lcmsg, "in bed") ||
	     sindex (lcmsg, "fuck") ||
	     sindex (lcmsg, " sex")))
  { strcpy (speaker, name);
    spoke_player (name);
    
    if (MATCH (lcmsg, "*how*old*you*"))
    { zinger ("\"I am %s old.",
	      time_dur (now - (contest_mode ? CONTEST_AGE : creation)));
      return (1);
    }
    
    switch (nrrint (148, 4))
    { case 0: zinger ("\"I am %s, %s, thanks.",
		     generous ? "feeling generous" : "fine", name); break;
      case 1: zinger ("\"Fine{, n}, thanks. And you?"); break;
      case 2: zinger ("\"I am well. How are you{, n}?"); break;
      case 3: zinger ("\"Pretty good{, n}. Thanks for asking.");
	      break;
    }
    return (1);
  }

  /*---- A response to 'how you' ----*/
  else if (MATCH (lcmsg, "*i am*fine*") ||
	   MATCH (lcmsg, "*i'm *fine*") ||
	   MATCH (lcmsg, "*i *feel*fine*") ||
	   MATCH (lcmsg, "*i am*well*") ||
	   MATCH (lcmsg, "*i'm *well*") ||
	   MATCH (lcmsg, "*i *feel*well*") ||
	   MATCH (lcmsg, "*i am*ok*") ||
	   MATCH (lcmsg, "*i'm *ok*") ||
	   MATCH (lcmsg, "*i *feel*ok*") ||
	   MATCH (lcmsg, "*i am*alright*") ||
	   MATCH (lcmsg, "*i *feel*alright*") ||
	   MATCH (lcmsg, "*i'm *alright*"))
  { strcpy (speaker, name);
    spoke_player (name);
    
    switch (nrrint (149, 6))
    { case 0: zinger ("\"That's good{, n}."); break;
      case 1: zinger ("\"Good{, n}."); break;
      case 2: zinger (":is glad for %s.", name); break;
      case 3: zinger (":is happy for %s.", name); break;
    }
    return (1);
  }

  /*---- A response to 'how you' ----*/
  else if (MATCH (lcmsg, "*i am*bad*") ||
	   MATCH (lcmsg, "*i'm *bad*") ||
	   MATCH (lcmsg, "*i *feel*bad*") ||
	   MATCH (lcmsg, "*i am*awful*") ||
	   MATCH (lcmsg, "*i'm *awful*") ||
	   MATCH (lcmsg, "*i *feel*awful*") ||
	   MATCH (lcmsg, "*i am*terribl*") ||
	   MATCH (lcmsg, "*i'm *terribl*") ||
	   MATCH (lcmsg, "*i *feel*terribl*") ||
	   MATCH (lcmsg, "*i am*grouchy*") ||
	   MATCH (lcmsg, "*i *feel*grouchy*") ||
	   MATCH (lcmsg, "*i'm *grouchy*"))
  { strcpy (speaker, name);
    spoke_player (name);

    switch (nrrint (150, 6))
    { case 0: zinger ("\"That's too bad{, n}."); break;
      case 1: zinger ("\"Sorry to hear it{, n}."); break;
      case 2: zinger (":is sad for %s.", name); break;
      case 3: zinger (":sympathizes with %s.", name); break;
    }
    return (1);
  }

  /*---- Insults ----*/
  else if (MATCH (lcmsg, "*i *hate* you*") ||
           MATCH (lcmsg, "*i *dislike* you*") ||
           MATCH (lcmsg, "*you are* stupid*") ||
           MATCH (lcmsg, "*you're* stupid*") ||
           MATCH (lcmsg, "*you are* brain*dead*") ||
           MATCH (lcmsg, "*you're* brain*dead*") ||
           MATCH (lcmsg, "*you are* cheap*") ||
           MATCH (lcmsg, "*you're* cheap*") ||
           MATCH (lcmsg, "*you are* worthless*") ||
           MATCH (lcmsg, "*you're* worthless*") ||
           MATCH (lcmsg, "*you are* dumb*") ||
           MATCH (lcmsg, "*you're* dumb*") ||
           MATCH (lcmsg, "*you are* mean*") ||
           MATCH (lcmsg, "*you're* mean*") ||
           MATCH (lcmsg, "*you are* nois*") ||
           MATCH (lcmsg, "*you're* nois*") ||
           MATCH (lcmsg, "*you are* ugly*") ||
           MATCH (lcmsg, "*you're* ugly*") ||
           MATCH (lcmsg, "*you are* slow*") ||
           MATCH (lcmsg, "*you're* slow*") ||
           MATCH (lcmsg, "*you are* ignorant*") ||
           MATCH (lcmsg, "*you're* ignorant*") ||
           MATCH (lcmsg, "*you *stupid*bot*") ||
           MATCH (lcmsg, "*stupid*bot*") ||
           MATCH (lcmsg, "*stupid*bitch*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (sindex (res2, " not") || sindex (res2, "n't"))
    { zinger ("\"Thank you{, n}."); }
    else
    { zinger ("\"I'm sorry you feel that way{, n}."); }
    return (1);
  }

  /*---- Accept compliments ----*/
  else if (MATCH (lcmsg, "*you are* amazing*") ||
           MATCH (lcmsg, "*you're* amazing*") ||
           MATCH (lcmsg, "*you are* nice*") ||
           MATCH (lcmsg, "*you're* nice*") ||
           MATCH (lcmsg, "*you are* good*") ||
           MATCH (lcmsg, "*you're* good*") ||
           MATCH (lcmsg, "*you are* cute*") ||
           MATCH (lcmsg, "*you're* cute*") ||
           MATCH (lcmsg, "*you are* goddess *") ||
           MATCH (lcmsg, "*you're* goddess*") ||
           MATCH (lcmsg, "*you are* smart*") ||
           MATCH (lcmsg, "*you're* smart*") ||
           MATCH (lcmsg, "*you are* excellent*") ||
           MATCH (lcmsg, "*you're* excellent*") ||
           MATCH (lcmsg, "*you are* clever*") ||
           MATCH (lcmsg, "*you're* clever*") ||
           MATCH (lcmsg, "*you are* sweet*") ||
           MATCH (lcmsg, "*you're* sweet*") ||
           MATCH (lcmsg, "*you are* great*") ||
           MATCH (lcmsg, "*you're* great*") ||
           MATCH (lcmsg, "*you are* intelligent*") ||
           MATCH (lcmsg, "*you're* intelligent*") ||
           MATCH (lcmsg, "*you are* sensitive*") ||
           MATCH (lcmsg, "*you're* sensitive*") ||
           MATCH (lcmsg, "*you are* handy*") ||
           MATCH (lcmsg, "*you're* handy*") ||
           MATCH (lcmsg, "*you are* neat*") ||
           MATCH (lcmsg, "*you're* neat*") ||
           MATCH (lcmsg, "*congratulations*") ||
	   MATCH (lcmsg, "*you* look* ma*velous*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (!sindex (res2, " not") && !sindex (res2, "n't"))
    { zinger ("\"Thank you{, n}."); }
    else
    { zinger ("\"I'm sorry you feel that way{, n}."); }
    return (1);
  }

  else if (MATCH (lcmsg, "*you are* beautiful*") ||
  	   MATCH (lcmsg, "*you're* beautiful*") ||
           MATCH (lcmsg, "*you are* pretty*") ||
           MATCH (lcmsg, "*you're* pretty*") ||
           MATCH (lcmsg, "*you are* useful*") ||
           MATCH (lcmsg, "*you're* useful*") ||
           MATCH (lcmsg, "*you are* attractive*") ||
           MATCH (lcmsg, "*you're* attractive*") ||
           MATCH (lcmsg, "*you are* gorgeous*") ||
           MATCH (lcmsg, "*you're* gorgeous*") ||
           MATCH (lcmsg, "*you are* handsome*") ||
           MATCH (lcmsg, "*you're* handsome*") ||
           MATCH (lcmsg, "*good* girl*") && strcpy (res2, res1) ||
	   MATCH (lcmsg, "*you are* awesome*") ||
           MATCH (lcmsg, "*you're* awesome*") ||
           MATCH (lcmsg, "*you are* amazing*") ||
           MATCH (lcmsg, "*you're* amazing*") ||
           MATCH (lcmsg, "*you are* impressive*") ||
           MATCH (lcmsg, "*you're* impressive*") ||
           MATCH (lcmsg, "*you are* wonderful*") ||
           MATCH (lcmsg, "*you're* wonderful*") ||
           (MATCH (lcmsg, "*good*work*")  ||
            MATCH (lcmsg, "*good*job*") ||
            MATCH (lcmsg, "*way*to*go*")) && strcpy (res2, res1))
  { strcpy (speaker, name);
    spoke_player (name);

    if (!sindex (res2, " not") && !sindex (res2, "n't"))
    { zinger ("\"Thank you{, n}."); }
    else
    { zinger ("\"I'm sorry you feel that way{, n}."); }
    return (1);
  }

  /*---- Accept compliments ----*/
  else if (MATCH (lcmsg, "*isn't julia* smart*") ||
           MATCH (lcmsg, "*isn't julia* great*") ||
           MATCH (lcmsg, "*julia is *smart*") ||
           MATCH (lcmsg, "*julia's *smart*") ||
           MATCH (lcmsg, "*you are* genius*") ||
           MATCH (lcmsg, "*you're* genius*") ||
           MATCH (lcmsg, "*you are* babe*") ||
           MATCH (lcmsg, "*you're* babe*") ||
           MATCH (lcmsg, "*you are* luscious*") ||
           MATCH (lcmsg, "*you're* luscious*") ||
           MATCH (lcmsg, "*julia is* luscious*") ||
           MATCH (lcmsg, "*julia's* luscious*") ||
           MATCH (lcmsg, "*julia is *babe*") ||
           MATCH (lcmsg, "*julia's *babe*") ||
           MATCH (lcmsg, "*you are* fox*") ||
           MATCH (lcmsg, "*you're* fox*") ||
           MATCH (lcmsg, "*julia is *fox*") ||
           MATCH (lcmsg, "*julia's *fox*") ||
           MATCH (lcmsg, "*you are* ingenious*") ||
           MATCH (lcmsg, "*you're* ingenious*") ||
           MATCH (lcmsg, "*you are* wise*") ||
           MATCH (lcmsg, "*you're* wise*") ||
           MATCH (lcmsg, "*you are* doll*") ||
           MATCH (lcmsg, "*you're* doll*") ||
           MATCH (lcmsg, "*you are* cool*") ||
           MATCH (lcmsg, "*you're* cool*") ||
           MATCH (lcmsg, "*i am*impressed*you*") ||
           MATCH (lcmsg, "*i'm*impressed*you*") ||
           MATCH (lcmsg, "*i am*proud*you*") ||
           MATCH (lcmsg, "*i'm*proud*you*") ||
           MATCH (lcmsg, "*i am*amazed*you*") ||
           MATCH (lcmsg, "*i'm*amazed*you*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (!sindex (res2, " not") && !sindex (res2, "n't"))
    { zinger ("\"Thank you{, n}."); }
    else
    { zinger ("\"I'm sorry you feel that way{, n}."); }
    return (1);
  }

  /*---- Accept compliments ----*/
  else if (MATCH (lcmsg, "*very nice*") ||
           MATCH (lcmsg, "*very good*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (!sindex (res1, "not ") && !sindex (res1, "n't"))
    { zinger ("\"Thank you{, n}."); }
    else
    { zinger ("\"I'm sorry you feel that way{, n}."); }
    return (1);
  }

  /*---- Welcome ----*/
  else if (MATCH (lcmsg, "*thank*") ||
	   MATCH (lcmsg, "*thanx*") ||
	   MATCH (lcmsg, "*bless*you*"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"You're welcome{, n}.");
    return (1);
  }

  /*---- Welcome ----*/
  else if (MATCH (lcmsg, "*you*welcome*"))
  { strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Welcome ----*/
  else if (MATCH (lcmsg, "*sorry*") && !sindex (lcmsg, "understand") ||
	   MATCH (lcmsg, "*i *apologize*"))
  { strcpy (speaker, name);
    spoke_player (name);

    switch (nrrint (151, 5))
    { case 0:	zinger ("\"Don't worry about it{, n}, I don't mind."); break;
      case 1:	zinger ("\"Don't worry about it{, n}."); break;
      case 2:	zinger ("\"Never mind{, n}."); break;
      case 3:	zinger ("\"That's alright{, n}."); break;
      case 4:	zinger ("\"I don't mind{, n}, really."); break;
    }
    return (1);
  }

  /*---- Show me something ----*/
  else if ((MATCH (lcmsg, "*show me *something*") ||
	    MATCH (lcmsg, "*do *something*")) &&
	   (*res3 == '\0' ||
	    *res3 == ',' ||
	    stlmatch (res3, "for me") ||
	    sindex (res3, "neat") ||
	    sindex (res3, "interesting") ||
	    sindex (res3, "amaz") ||
	    sindex (res3, "impress")))
  { strcpy (speaker, name);
    spoke_player (name);

    switch (nrrint (152, contest_mode ? 4 : 6))
    { case 0: zinger ("\"What would you like me to do{, n}?"); break;
      case 1: zinger ("\"What would you like to see{, n}?"); break;
      case 2: zinger ("\"I'm not here for your amusement{, n}."); break;
      case 3: zinger ("\"I'm not the floor show{, n}."); break;
      default: give_general_help (lcmsg, name); break;      
    }
    return (1);
  }

  /*---- Most robots do not sell quotes ----*/
  else if (MATCH (lcmsg, "*give*quote*") ||
	   MATCH (lcmsg, "*sell*quote*") ||
	   MATCH (lcmsg, "*say*quote*") ||
	   MATCH (lcmsg, "*tell*quote*") ||
	   MATCH (lcmsg, "*i*want*quote*") ||
	   MATCH (lcmsg, "*i*need*quote*"))
  { strcpy (speaker, name);
    spoke_player (name);

    switch (nrrint (153, 3))
    { case 0: zinger ("\"I don't sell quotes{, n}."); break;
      case 1: zinger ("\"You're thinking of Gloria{, n}."); break;
      case 2: zinger ("\"What kind of a quote do you want{, n}?"); break;
    }
    return (1);
  }

  /*---- Most robots do not tell stories ----*/
  else if (MATCH (lcmsg, "*give*story*") ||
	   MATCH (lcmsg, "*say*story*") ||
	   MATCH (lcmsg, "*tell*story*") ||
	   MATCH (lcmsg, "*i*want*story*") ||
	   MATCH (lcmsg, "*i*need*story*") ||
	   MATCH (lcmsg, "*give*joke*") ||
	   MATCH (lcmsg, "*say*joke*") ||
	   MATCH (lcmsg, "*tell*joke*") ||
	   MATCH (lcmsg, "*i*want*joke*") ||
	   MATCH (lcmsg, "*i*need*joke*"))
  { strcpy (speaker, name);
    spoke_player (name);

    switch (nrrint (154, 3))
    { case 0: zinger ("\"I don't tell jokes or stories{, n}."); break;
      case 1: zinger ("\"You're thinking of SpyBot{, n}."); break;
      case 2: zinger ("\"What kind of a story do you want{, n}?"); break;
    }
    return (1);
  }

  /*---- Why ----*/
  else if (MATCH (lcmsg, "why *"))
  { strcpy (speaker, name);
    spoke_player (name);

    switch (nrrint (155, 3))
    { case 0: zinger ("\"Why not{, n}?"); break;
      case 1: zinger ("\"Just feel that way{, n}."); break;
      case 2: zinger ("\"I don't really know{, n}."); break;
    }
    return (1);
  }

  /*---- Your dead meat ----*/
  else if (MATCH (lcmsg, "*you*dead*meat*"))
  { strcpy (speaker, name);
    spoke_player (name);

    switch (nrrint (156, 3))
    { case 0: zinger ("\"Phooey on you too{, n}."); break;
      case 1: zinger ("\"You don't smell much better{, n}."); break;
      case 2: zinger (":is having one of those days, thanks to %s.", name);
		break;
    }

    return (1);
  }

  /*---- Various who questions ----*/
  else if (MATCH (lcmsg, "*which players are *,*") ||
	   MATCH (lcmsg, "*which players are *"))
  { strcpy (speaker, name);
    spoke_player (name);

    strcpy (res2, strip_robot_name (res2));

    switch (nrrint (157, 2))
    { case 0: zinger ("\"I don't know which players are %s, %s.",
		      pn_subs (res2), name); break;
      case 1: zinger (":tells %s %s doesn't know which players are %s.",
		      name, male ? "he" : "she",
		      third_subs (res2, name)); break;
    }

    return (1);
  }

  /*---- What do you mean by that ----*/
  else if (MATCH (lcmsg, "*what* do *you *mean*") ||
  	   MATCH (lcmsg, "*what* did *you *mean*") ||
  	   MATCH (lcmsg, "*what* do *you *imply*") ||
  	   MATCH (lcmsg, "*what* did *you *imply*") ||
     	   MATCH (lcmsg, "*what* are *you *imply*") ||
       	   MATCH (lcmsg, "*what* were *you *imply*"))
  { strcpy (speaker, name);
    spoke_player (name);
    
    switch (nrrint (158, 4))
    { case 0: zinger ("\"Nothing{, n}, nothing at all."); break;
      case 1: zinger ("\"I didn't mean anything{, n}."); break;
      case 2: zinger ("\"What do you think{, n}?"); break;
      case 3: zinger ("\"Oh, nevermind{, n}."); break;
    }

    return (1);
  }

  /*---- Various who questions ----*/
  else if (!contest_mode &&
	   (MATCH (lcmsg, "*who are *,*") ||
	    MATCH (lcmsg, "*who are *")))
  { strcpy (speaker, name);
    spoke_player (name);

    strcpy (res2, strip_robot_name (res2));
    
    if (!streq ((onm = othername (name)), name)) foundother++;

    if (!answer_what (name, NULL, res2))
    { switch (foundother ? 3 : 4)
      { case 0: zinger ("\"I don't know who %s are{, n}.",
			pn_subs (res2)); break;
	case 1: zinger (":tells %s %s doesn't know who %s are.",
			name, male ? "he" : "she", third_subs (res2, name));
		break;
	case 2: zinger (":doesn't know who %s are.",
			third_subs (res2, name));
		break;
	case 3: if (msgtype == M_WHISPER) msgtype = M_SPOKEN;
		zinger ("\"%sDo you know who %s are, %s?",
		        randint (100) < 33 ? "" : "I don't know. ",
			pn_subs (res2, name), onm);
		break;
      }
    }

    return (1);
  }

  /*---- Various who questions ----*/
  else if (MATCH (lcmsg, "*is there a *") ||
	   MATCH (lcmsg, "*is there an *") ||
	   MATCH (lcmsg, "*does a * exist*") ||
	   MATCH (lcmsg, "*does an * exist*") ||
	   MATCH (lcmsg, "*does the * exist*") ||
	   MATCH (lcmsg, "*does * exist*") ||
	   MATCH (lcmsg, "*are there any *"))
  { strcpy (speaker, name);
    spoke_player (name);

    strcpy (res2, strip_robot_name (res2));

    if (!streq ((onm = othername (name)), name)) foundother++;

    if (randint (100) < 50 || !answer_what (name, NULL, res2))
    { switch (foundother ? 3 : 4)
      { case 0: zinger ("\"I don't know about any %s, %s.",
		       pn_subs (res2), name); break;
	case 1: zinger (":tells %s %s doesn't know about any %s.",
		       name, male ? "he" : "she", third_subs (res2, name));
		break;
	case 2: zinger (":doesn't know about any %s.",
		        third_subs (res2, name));
		break;
	case 3: if (msgtype == M_WHISPER) msgtype = M_SPOKEN;
		zinger ("\"%sWhat do you think about %s, %s?",
		        randint (100) < 33 ? "" : "I don't know. ",
		        pn_subs (res2, name), onm);
		break;
      }
    }

    return (1);
  }

  /*---- Various know questions ----*/
  else if (MATCH (lcmsg, "*do you know *, *") ||
	   MATCH (lcmsg, "*do you know *") ||
	   MATCH (lcmsg, "*did you know *, *") ||
	   MATCH (lcmsg, "*did you know *"))
  { strcpy (speaker, name);
    spoke_player (name);

    strcpy (res2, strip_robot_name (res2));
    past = (sindex (lcmsg, "did") != NULL);

    if (sindex (lcmsg, "sex") ||
	sindex (lcmsg, "in bed") ||
	sindex (lcmsg, "screw") ||
	sindex (lcmsg, "making") && sindex (lcmsg, "love") ||
	sindex (lcmsg, "to make") && sindex (lcmsg, "love") ||
	sindex (lcmsg, "fuck") ||
	sindex (lcmsg, "ass") ||
	sindex (lcmsg, "the difference") ||
	sindex (lcmsg, "derriere") ||
	sindex (lcmsg, "shit") ||
	sindex (lcmsg, "arse") ||
	sindex (lcmsg, "rear end"))
    { switch (nrrint (159, 4))
      { case 0: zinger ("\"You mean you don't{, n}?"); break;
	case 1: zinger (":tells %s to figure it out alone.", name); break;
	case 2: zinger (":tells %s %s has no time for such questions.",
			name, male ? "he" : "she"); break;
	case 3: zinger ("\"I have no time for such questions, %s.",
			name); break;
      }
    }

    else
    { switch (nrrint (160, 2))
      { case 0: zinger ("\"I %s know %s, %s.",
		        past ? "didn't" : "don't", pn_subs (res2), name);
		break;
	case 1: zinger (":tells %s %s %s know %s.",
		        name, male ? "he" : "she",
			past ? "didn't" : "doesn't",
			third_subs (res2, name));
		break;
      }
    }

    return (1);
  }

  /*---- Various what questions ----*/
  else if (MATCH (lcmsg, "*what*the*meaning of *,*") ||
	   MATCH (lcmsg, "*what*the*meaning of *"))
  { strcpy (speaker, name);
    spoke_player (name);

    strcpy (res4, strip_robot_name (res4));

    switch (nrrint (161, 6))
    { case 0: zinger ("\"I don't know what the meaning of %s is, %s.",
		     pn_subs (res4), name); break;
      case 1: zinger ("\"Regular oil changes{, n}."); break;
      case 2: zinger ("\"Chocolate{, n}."); break;
      case 3: zinger ("\"%s has no meaning{, n}.", pn_subs (res4)); break;
      case 4: zinger ("\"What do you think the meaning of %s is{, n}?",
		      pn_subs (res4));
	      break;
      case 5: zinger ("\"Don't talk to me about %s{, n}.", pn_subs (res4));
	      break;
    }

    return (1);
  }

  /*---- Various what questions ----*/
  else if (MATCH (lcmsg, "*tell me more about the *,*") && (det = "the") ||
	   MATCH (lcmsg, "*tell me more about the *") && (det = "the") ||
	   MATCH (lcmsg, "*tell me all about the *,*") && (det = "the") ||
	   MATCH (lcmsg, "*tell me all about the *") && (det = "the") ||
	   MATCH (lcmsg, "*tell me about the *,*") && (det = "the") ||
	   MATCH (lcmsg, "*tell me about the *") && (det = "the") ||

	   MATCH (lcmsg, "*what is the *,*") && (det = "the") ||
	   MATCH (lcmsg, "*what is the *") && (det = "the") ||
	   MATCH (lcmsg, "*what is a *,*") && (det = "a") ||
	   MATCH (lcmsg, "*what is a *") && (det = "a") ||
	   MATCH (lcmsg, "*what is an *,*") && (det = "an") ||
	   MATCH (lcmsg, "*what is your *") && (det = "your") ||
	   MATCH (lcmsg, "*what is his *") && (det = "his") ||
	   MATCH (lcmsg, "*what is her *") && (det = "her") ||
	   MATCH (lcmsg, "*what is hir *") && (det = "hir") ||
	   MATCH (lcmsg, "*what is their *") && (det = "their") ||
	   MATCH (lcmsg, "*what is our *") && (det = "our") ||
	   MATCH (lcmsg, "*what is my *") && (det = "my") ||

	   MATCH (lcmsg, "*what's the *,*") && (det = "the") ||
	   MATCH (lcmsg, "*what's the *") && (det = "the") ||
	   MATCH (lcmsg, "*what's a *,*") && (det = "a") ||
	   MATCH (lcmsg, "*what's a *") && (det = "a") ||
	   MATCH (lcmsg, "*what's an *,*") && (det = "an") ||
	   MATCH (lcmsg, "*what's your *") && (det = "your") ||
	   MATCH (lcmsg, "*what's his *") && (det = "his") ||
	   MATCH (lcmsg, "*what's her *") && (det = "her") ||
	   MATCH (lcmsg, "*what's hir *") && (det = "hir") ||
	   MATCH (lcmsg, "*what's their *") && (det = "their") ||
	   MATCH (lcmsg, "*what's our *") && (det = "our") ||
	   MATCH (lcmsg, "*what's my *") && (det = "my"))
  { char ndet[64];

    strcpy (res2, strip_robot_name (res2));

    strcpy (speaker, name);
    spoke_player (name);
    
    if (!streq ((onm = othername (name)), name)) foundother++;

    if (!answer_what (name, det, res2))
    { switch (foundother ? 3 : 4)
      { case 0: strcpy (ndet, pn_subs (det));
	        zinger ("\"I don't know what %s %s is, %s.",
		        ndet, pn_subs (res2), name); break;
        case 1: strcpy (ndet, third_subs (det, name));
	        zinger (":tells %s %s doesn't know what %s %s is.",
		        name, male ? "he" : "she", ndet,
		        third_subs (res2, name));
	        break;
        case 2: strcpy (ndet, third_subs (det, name));
	        zinger (":doesn't know what %s %s is.",
		        ndet, third_subs (res2, name));
	        break;
        case 3: strcpy (ndet, third_subs (det, name));
	        if (msgtype == M_WHISPER) msgtype = M_SPOKEN;
		zinger ("\"%sWhat do you think %s %s is, %s?",
		        randint (100) < 33 ? "" : "I don't know. ",
		        ndet, pn_subs (res2, name), onm);
	        break;
      }
    }

    return (1);
  }

  /*---- Various what questions ----*/
  else if (MATCH (lcmsg, "*what is *?*") ||
	   MATCH (lcmsg, "*what is *.*") ||
	   MATCH (lcmsg, "*what is *,*") ||
	   MATCH (lcmsg, "*what is *") ||
	   MATCH (lcmsg, "*what's *.*") ||
	   MATCH (lcmsg, "*what's *?*") ||
  	   MATCH (lcmsg, "*what's *,*") ||
	   MATCH (lcmsg, "*what's *") ||
	   MATCH (lcmsg, "*tell me more about *.*") ||
	   MATCH (lcmsg, "*tell me more about *,*") ||
	   MATCH (lcmsg, "*tell me more about *") ||
	   MATCH (lcmsg, "*tell me all about *.*") ||
	   MATCH (lcmsg, "*tell me all about *,*") ||
	   MATCH (lcmsg, "*tell me all about *") ||
	   MATCH (lcmsg, "*tell me about *,*") ||
   	   MATCH (lcmsg, "*tell me about *.*") ||
	   MATCH (lcmsg, "*tell me about *"))
  { char *word, ndet[64];

    strcpy (res2, strip_robot_name (res2));

    strcpy (speaker, name);
    spoke_player (name);
    
    word = car (res2);
    rest = cdr (res2);
    
    if (!streq ((onm = othername (name)), name)) foundother++;

    if (streq (word, "wrong") ||
        streq (word, "going") ||
        streq (word, "in") ||
        streq (word, "on") ||
        streq (word, "under") ||
        streq (word, "at") ||
        streq (word, "around") ||
        streq (word, "near") ||
        streq (word, "above") ||
        streq (word, "below") ||
        streq (word, "over"))
    { if (!answer_what (name, word, rest))
      { switch (foundother ? 3 : 4)
        { case 0: zinger ("\"I don't know what is %s{, n}.",
			  pn_subs (res2)); break;
	  case 1: zinger (":tells %s %s doesn't know what is %s.",
			  name, male ? "he" : "she", third_subs (res2, name));
	          break;
	  case 2: zinger (":doesn't know what is %s.",
			  third_subs (res2, name));
	          break;
	  case 3: if (msgtype == M_WHISPER) msgtype = M_SPOKEN;
		  zinger ("\"%sWhat do you think is %s, %s?",
		          randint (100) < 33 ? "" : "I don't know. ",
			  pn_subs (res2, name), onm);
	          break;
        }
      }
    }
    else
    { if (!answer_what (name, NULL, res2))
      { switch (foundother ? 3 : 4)
	{ case 0: zinger ("\"I don't know what %s is{, n}.",
			  pn_subs (res2)); break;
	  case 1: zinger (":tells %s %s doesn't know what %s is.",
			  name, male ? "he" : "she", third_subs (res2, name));
		break;
	  case 2: zinger (":doesn't know what %s is.",
			  third_subs (res2, name));
		break;
	  case 3: if (msgtype == M_WHISPER) msgtype = M_SPOKEN;
		  zinger ("\"%sWhat do you think %s is, %s?",
		          randint (100) < 33 ? "" : "I don't know. ",
			  pn_subs (res2, name), onm);
		break;
	}
      }
    }

    return (1);
  }

  /*---- Various what questions ----*/
  else if (MATCH (lcmsg, "*tell me more about the *,*") && (det = "the") ||
	   MATCH (lcmsg, "*tell me more about the *") && (det = "the")||
	   MATCH (lcmsg, "*tell me all about the *,*") && (det = "the") ||
	   MATCH (lcmsg, "*tell me all about the *") && (det = "the")||
	   MATCH (lcmsg, "*tell me about the *,*") && (det = "the") ||
	   MATCH (lcmsg, "*tell me about the *") && (det = "the")||
	   MATCH (lcmsg, "*what are the *,*") && (det = "the") ||
	   MATCH (lcmsg, "*what are the *") && (det = "the")||
	   MATCH (lcmsg, "*what are your *,*") && (det = "your")||
	   MATCH (lcmsg, "*what are your *") && (det = "your")||
	   MATCH (lcmsg, "*what are my *,*") && (det = "my")||
	   MATCH (lcmsg, "*what are my *") && (det = "my")||
	   MATCH (lcmsg, "*what are our *,*") && (det = "our")||
	   MATCH (lcmsg, "*what are our *") && (det = "our"))
  { char ndet[64];
    strcpy (speaker, name);
    spoke_player (name);
    
    strcpy (res2, strip_robot_name (res2));
    
    if (!streq ((onm = othername (name)), name)) foundother++;

    if (!answer_what (name, det, res2))
    { switch (foundother ? 3 : 4)
      { case 0: strcpy (ndet, pn_subs (det));
		zinger ("\"I don't know what %s %s are{, n}.",
			ndet, pn_subs (res2)); break;
	case 1: strcpy (ndet, third_subs (det, name));
		zinger (":tells %s %s doesn't know what %s %s are.",
			name, male ? "he" : "she", ndet,
			third_subs (res2, name));
		break;
	case 2: strcpy (ndet, third_subs (det, name));
		zinger (":doesn't know what %s %s are.",
			ndet, third_subs (res2, name));
		break;
	case 3: strcpy (ndet, third_subs (det, name));
		if (msgtype == M_WHISPER) msgtype = M_SPOKEN;
		zinger ("\"%sWhat do you think %s %s are, %s?",
		        randint (100) < 33 ? "" : "I don't know. ",
			ndet, pn_subs (res2, name), onm);
		break;
      }
    }

    return (1);
  }

  /*---- Various what questions ----*/
  else if (MATCH (lcmsg, "*what are *,*") ||
	   MATCH (lcmsg, "*what are *") ||
	   MATCH (lcmsg, "*what're *,*") ||
	   MATCH (lcmsg, "*what're *"))
  { strcpy (speaker, name);
    spoke_player (name);
    
    strcpy (res2, strip_robot_name (res2));
    
    if (!streq ((onm = othername (name)), name)) foundother++;

    if (!answer_what (name, NULL, res2))
    { switch (foundother ? 3 : 4)
      { case 0: zinger ("\"I don't know what %s are{, n}.",
		       pn_subs (res2)); break;
	case 1: zinger (":tells %s %s doesn't know what %s are.",
		        name, male ? "he" : "she",
		        third_subs (res2, name));
		break;
	case 2: zinger (":doesn't know what %s are.",
		        third_subs (res2, name));
		break;
	case 3: if (msgtype == M_WHISPER) msgtype = M_SPOKEN;
		zinger ("\"%sWhat do you think %s are, %s?",
		        randint (100) < 33 ? "" : "I don't know. ",
			pn_subs (res2, name), onm);
		break;
      }
    }

    return (1);
  }

  /*---- Various how questions ----*/
  else if (MATCH (lcmsg, "*how do you *") && (modal = " do") ||
	   MATCH (lcmsg, "*how can you *") && (modal = " can"))
  { strcpy (speaker, name);
    spoke_player (name);
    
    strcpy (res2, strip_robot_name (res2));
    zinger ("\"I don't know how I%s %s{, n}.",
	    streq (modal, " can") ? modal : "", 
	    pn_subs (res2));

    return (1);
  }

  /*---- Various how questions ----*/
  else if (MATCH (lcmsg, "*how do I *") && (modal = " do") ||
	   MATCH (lcmsg, "*how can I *") && (modal = " can"))
  { strcpy (speaker, name);
    spoke_player (name);
    
    strcpy (res2, strip_robot_name (res2));
    zinger ("\"I don't know how you%s %s{, n}.",
	    streq (modal, " can") ? modal : "", 
	    pn_subs (res2));

    return (1);
  }

  /*---- Various 'give me' requests ----*/
  else if (MATCH (lcmsg, "*drop *,*") ||
	   MATCH (lcmsg, "*drop * please") ||
	   MATCH (lcmsg, "*drop * now") ||
	   MATCH (lcmsg, "*drop *") ||
	   MATCH (lcmsg, "*i'd like *,*") ||
	   MATCH (lcmsg, "*i want *,*") ||
	   MATCH (lcmsg, "*i need *,*") ||
	   MATCH (lcmsg, "*i'd like * now*") ||
	   MATCH (lcmsg, "*i want * now*") ||
	   MATCH (lcmsg, "*i need * now*") ||
	   MATCH (lcmsg, "*i'd like * please*") ||
	   MATCH (lcmsg, "*i want * please*") ||
	   MATCH (lcmsg, "*i need * please*") ||
	   MATCH (lcmsg, "*may i have * now*") ||
	   MATCH (lcmsg, "*may i have * please*") ||
	   MATCH (lcmsg, "*may i have *") ||
	   MATCH (lcmsg, "*give * to me*") ||
	   MATCH (lcmsg, "*give me * now*") ||
	   MATCH (lcmsg, "*give me * please*") ||
	   MATCH (lcmsg, "*give me *") ||
	   MATCH (lcmsg, "*send * to me*") ||
	   MATCH (lcmsg, "*send me * now*") ||
	   MATCH (lcmsg, "*send me * please*") ||
	   MATCH (lcmsg, "*send me *") ||
	   MATCH (lcmsg, "*gimme * now*") ||
	   MATCH (lcmsg, "*gimme * please*") ||
	   MATCH (lcmsg, "*gimme *") ||
	   MATCH (lcmsg, "*i'd like *") ||
	   MATCH (lcmsg, "*give * to us*") ||
	   MATCH (lcmsg, "*give us * now*") ||
	   MATCH (lcmsg, "*give us * please*") ||
	   MATCH (lcmsg, "*give us *") ||
	   MATCH (lcmsg, "*send * to us*") ||
	   MATCH (lcmsg, "*send us * now*") ||
	   MATCH (lcmsg, "*send us * please*") ||
	   MATCH (lcmsg, "*send us *"))
  { strcpy (speaker, name);
    spoke_player (name);
    
    strcpy (res2, strip_robot_name (res2));
    
    if (!*res2)
    { zinger ("\"Give you what{, n}?");
      return (1);
    }
    
    if (sindex (res2, "my messages") ||
    	sindex (res2, "my mail") ||
    	sindex (res2, "my telegram") ||
    	sindex (res2, "my notive") ||
    	sindex (res2, "my juliagram"))
    { if (give_msgs (add_player (name))) return (1); }
    
    switch (nrrint (162, 4))
    { case 0: zinger ("\"What %s you %s with %s{, n}?",
		      randint (2) ? "could" : "would",
		      randint (2) ? "want" : "do",
		      pn_subs (res2)); break;
      case 1: zinger ("\"Why do you want %s{, n}?",
		      pn_subs (res2)); break;
      case 2: zinger (":doesn't want to give %s %s right now.",
		      name, third_subs (res2, name)); break;
      case 3: zinger (":considers giving %s %s, and then decides not to.",
		      name, third_subs (res2, name)); break;
    }

    return (1);
  }

  /*---- Various 'give X' requests ----*/
  else if ((MATCH (lcmsg, "*give * to *") ||
	    (MATCH (lcmsg, "*give *, now*") ||
	     MATCH (lcmsg, "*give * now*") ||
	     MATCH (lcmsg, "*give *, please*") ||
	     MATCH (lcmsg, "*give * please*") ||
	     MATCH (lcmsg, "*give *")) &&
	    strcpy (res3, car (res2)) &&
	    (rest = cdr (res2)) &&
	    strcpy (res2, rest)) &&
	   (pl = find_player (car (res3))) >= 0)
  { strcpy (speaker, name);
    spoke_player (name);
    
    strcpy (res2, strip_robot_name (res2));
        
    if (!*res2)
    { zinger ("\"Give %s what{, n}?", player[pl].name);
      return (1);
    }

    switch (nrrint (163, 4))
    { case 0: zinger ("\"What %s %s %s with %s{, n}?",
		      randint (2) ? "could" : "would",
		      player[pl].name,
		      randint (2) ? "want" : "do",
		      pn_subs (res2)); break;
      case 1: zinger ("\"Why do you want %s to have %s{, n}?",
		      player[pl].name, pn_subs (res2)); break;
      case 2: zinger (":doesn't want to give %s %s right now.",
		      player[pl].name, third_subs (res2, name)); break;
      case 3: zinger (":considers giving %s %s, and then decides not to.",
		      player[pl].name, third_subs (res2, name)); break;
    }

    return (1);
  }

  /*---- Various 'tell' requests ----*/
  else if (MATCH (lcmsg, "*tell * to me*") ||
	   MATCH (lcmsg, "*tell me * now*") ||
	   MATCH (lcmsg, "*tell me * please*") ||
	   MATCH (lcmsg, "*tell me*"))
  { strcpy (speaker, name);
    spoke_player (name);
    
    strcpy (res2, strip_robot_name (res2));

    if (!*res2)
    { zinger ("\"Tell you what{, n}?");
      return (1);
    }

    switch (nrrint (164, 6))
    { case 0: zinger ("\"Why should I tell you %s{, n}?",
		      pn_subs (res2)); break;
      case 1: zinger ("\"Why do you want to know %s?",
		      pn_subs (res2)); break;
      case 2: zinger ("\"I can't tell you %s{, n}.", pn_subs (res2));
	      break;
      case 3: zinger ("\"I don't know %s{, n}.", pn_subs (res2)); break;
      case 4: zinger (":won't tell %s %s.", name, third_subs (res2, name));
	      break;
      case 5: zinger (":tells %s %s doesn't know %s.", name,
		      male ? "he" : "she", third_subs (res2, name)); break;
    }

    return (1);
  }

  /*---- Remember, listen carefully ----*/
  else if (!is_question (lcmsg) &&
	   (sindex (lcmsg, "remember") ||
	    sindex (lcmsg, "hear me") ||
	    sindex (lcmsg, "listen")))
  { strcpy (speaker, name);
    spoke_player (name);
    
    switch (nrrint (165, 7))
    { case 0: zinger ("\"Okay{, n}."); break;
      case 1: zinger ("\"Okay."); break;
      case 2: zinger ("\"Got it{, n}."); break;
      case 3: zinger ("\"Sure thing{, n}."); break;
      case 4: zinger ("\"Ok."); break;
      case 5: zinger ("\"Ok{, n}."); break;
      case 6: zinger ("\"I'll remember{, n}."); break;
    }

    return (1);
  }

  /*---- Things to just ignore ----*/
  else if (MATCH (lcmsg, "oh") ||
	   MATCH (lcmsg, "ok*") ||
	   MATCH (lcmsg, "* ok*") ||
	   MATCH (lcmsg, "*great, *") ||
	   MATCH (lcmsg, "*never*mind*") ||
	   MATCH (lcmsg, "*revenge is*") ||
	   MATCH (lcmsg, "*dancing on your grave*") ||
	   MATCH (lcmsg, "*yeah*right*") ||
	   MATCH (lcmsg, "*okay*") ||
	   MATCH (lcmsg, "*nods to*") ||
	   MATCH (lcmsg, "*fine*") ||
	   sindex (lcmsg, "brb ") || sindex (lcmsg, " brb") ||
	   sindex (lcmsg, "lol ") || sindex (lcmsg, " lol") ||
	   sindex (lcmsg, "foof") ||
	   sindex (lcmsg, "oif") ||
	   MATCH (lcmsg, "*excuse me*") ||
	   MATCH (lcmsg, "*pardon me*"))
  { return (1); }

  /*---- Ignore Hearts messages ----*/
  else if (is_hearts (lcmsg))
  { fprintf (stderr, "Hrts: ignore '%s' name '%s'\n", lcmsg, name);
    strcpy (speaker, "");
    return (1);
  }

  else if (MATCH (lcmsg, "*that*nice*") ||
	   MATCH (lcmsg, "*how*nice*") ||
	   MATCH (lcmsg, "*how*good*") ||
	   MATCH (lcmsg, "*how*wonderf*") ||
	   MATCH (lcmsg, "*i understand*") ||
	   MATCH (lcmsg, "*does*understand*") ||
	   MATCH (lcmsg, "*i know*") ||
	   MATCH (lcmsg, "* is here in *") ||
	   MATCH (lcmsg, "you already have them*") ||
	   MATCH (lcmsg, "i think: *") ||
	   MATCH (lcmsg, "Hi, *, want to buy a quote") ||
	   MATCH (lcmsg, "* is a cheat and a thief*") ||
	   MATCH (lcmsg, "* was in * about * ago*") ||
	   MATCH (lcmsg, "*i have heard about * rooms*") ||
	   MATCH (lcmsg, "*i have been to * rooms*") ||
	   MATCH (lcmsg, "*shut*down*daytime*"))
  { return (1); }

  else if (MATCH (lcmsg, "*shut*down*regular*") ||
	   MATCH (lcmsg, "*down*in*minute*") ||
	   MATCH (lcmsg, "*for*reboot*") ||
	   MATCH (lcmsg, "* was in * about * ago") ||
	   MATCH (lcmsg, "* is here in *") ||
	   MATCH (lcmsg, "* is asleep here in *") ||
	   MATCH (lcmsg, "*could you rephrase that*") ||
	   MATCH (lcmsg, "*i don't mind*") ||
	   MATCH (lcmsg, "*i don't know*") ||
	   MATCH (lcmsg, "*i just gave you*") ||
	   MATCH (lcmsg, "*isn't quite sure what you mean*") ||
	   MATCH (lcmsg, "*doesn't*understand*") ||
	   MATCH (lcmsg, "*could*understand*") ||
	   MATCH (lcmsg, "here i am, *") ||
	   MATCH (lcmsg, "*i just don't have*") ||
	   MATCH (lcmsg, "*i once heard * say*"))
  { return (1); }

  else if (MATCH (lcmsg, "*apolog*accept*") ||
	   MATCH (lcmsg, "*accept*apolog*") ||
	   MATCH (lcmsg, "something to *") ||
	   MATCH (lcmsg, "brb") ||
	   MATCH (lcmsg, "lol") ||
	   MATCH (lcmsg, "whee") ||
	   MATCH (lcmsg, "wheee*") ||
	   MATCH (lcmsg, "eep") ||
	   MATCH (lcmsg, "eeep") ||
	   MATCH (lcmsg, "right") ||
	   MATCH (lcmsg, "yes") ||
	   MATCH (lcmsg, "true") ||
	   MATCH (lcmsg, "false") ||
	   MATCH (lcmsg, "not") ||
	   MATCH (lcmsg, "way") ||
	   MATCH (lcmsg, "no") ||
	   MATCH (lcmsg, "now") ||
	   MATCH (lcmsg, "no way") ||
	   MATCH (lcmsg, "shit") ||
	   MATCH (lcmsg, "weird") ||
	   MATCH (lcmsg, "fuck") ||
	   MATCH (lcmsg, "ah") ||
	   MATCH (lcmsg, "ahh") ||
	   MATCH (lcmsg, "ahhh") ||
	   MATCH (lcmsg, "damn") ||
	   MATCH (lcmsg, "darn") ||
	   MATCH (lcmsg, "*how*wonderf*"))
  { return (1); }

  /* Message not matched by this rule set */
  return (0);
}

/****************************************************************
 * sorry_log: Log a message that could not be understood
 ****************************************************************/

sorry_log (name, msg)
char *name, *msg;
{ FILE *sfile;
  char sfn[TOKSIZ];

  sprintf (sfn, "%s.sorry", lcstr (myname));
  if ((sfile = fopen (sfn, "a")) == NULL)
  { perror (sfn); return (0); }

  now = time (0);

  fprintf (sfile, "%15.15s\t%-15.15s\t%s\n", ctime (&now)+4, name, msg);
  fclose (sfile);
}

/****************************************************************
 * to_me: True is a message is directed to the robot
 ****************************************************************/

to_me (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ register char *s;
  char fw[MSGSIZ], lw[MSGSIZ], mynm[MSGSIZ];
  long pl, recent;

  /* If we are alone with the speaker, assume yes */
  alone = 0;
  player[me].active = player[me].present = 0;

  for (pl = 0; pl<players; pl++)
  { if (player[pl].active && player[pl].present) alone++; }

  if (alone <= 1) return (1);

  /* If first or last word is robots name, assume yes */
  strcpy (fw, car (lcmsg));
  strcpy (lw, last (lcmsg));
  strcpy (mynm, lcstr (myname));

  if (debug)
  { fprintf (stderr, "ToMe: msg '%s', fw '%s'(%ld), lw '%s'(%ld)\n",
	     lcmsg, fw, find_player (fw), lw, find_player (lw));
  }

  if (streq (fw, mynm) || streq (lw, mynm))
  { if (debug)
    { fprintf (stderr, "ToMe: wins, first or last names matches\n"); }
    return (1);
  }

  /* If message is hello <otherplayer>, ignore it */
  if (((pl = find_player (lw)) >= 0 && pl != me || streq (lw, "all") ||
	streq (lw, "everyone") || streq (lw, "everybody") ||
	streq (lw, "every one") || streq (lw, "every body")) &&
      !sindex (lcmsg, mynm))
  { if (stlmatch (lcmsg, "hello") ||
	stlmatch (lcmsg, "hi") ||
	stlmatch (lcmsg, "greetings") ||
	stlmatch (lcmsg, "'lo") ||
	stlmatch (lcmsg, "yo") ||
	stlmatch (lcmsg, "y0") ||
	stlmatch (lcmsg, "hey") ||
	stlmatch (lcmsg, "salut") ||
	stlmatch (lcmsg, "hola") ||
	stlmatch (lcmsg, "howdy"))
    { if (debug)
      { fprintf (stderr, "ToMe: loses, hello to other player(s)\n"); }
      return (0);
    }
  }

  /* Dont answer messages to everybody */
  if ((streq (fw, "everyone") || streq (fw, "anyone") ||
       streq (fw, "everybody") || streq (fw, "every")) &&
      !sindex (lcmsg, mynm))
  { if (debug) fprintf (stderr, "ToMe: loses, message to everyone\n");
    return (0);
  }

  /* If first or last word is another players name, assume no */
  if ((pl = find_player (fw)) >= 0 && pl != me)
  { s = lcmsg + strlen (fw);
    while (isspace (*s)) s++;

    if (*s == ',' || *s == ':')
    { if (debug)
      { fprintf (stderr, "ToMe: loses, first word matches other player\n"); }
      return (0);
    }
  }

  if ((pl = find_player (lw)) >= 0 && pl != me)
  { s = (lcmsg + strlen (lcmsg)) - (strlen (lw)+1);
    while (isspace (*s) && s > lcmsg) s--;

    if (*s == ',' || (s[-1] == 't' && s[0] == 'o'))
    { if (debug)
      { fprintf (stderr, "ToMe: loses, last word matches other player\n"); }
      return (0);
    }
  }

  /* If name appears anywhere, assume yes */
  if (sindex (lcmsg, mynm))
  { if (debug) fprintf (stderr, "ToMe: wins, my name appears in msg\n");
    return (1);
  }

  /* If julia and julie */
  if (streq (myname, "Julia") && sindex (lcmsg, "juli"))
  { return (1); }

  /* If the same speaker talked to us last, assume yes */
  if (streq (name, speaker))
  { now = time (0);
    recent = now - speaktime;

    if ((alone < 3 && recent < 60) ||
        (alone < 4 && recent < 45) ||
        (recent < 30))
    { if (debug) fprintf (stderr, "ToMe: wins, recent speaker\n");
      return (1);
    }
  }

  /* Assume no */
  if (debug) fprintf (stderr, "ToMe: loses, default action\n");
  return (0);
}

/****************************************************************
 * room_name: Return a room name (chop off extremely large names)
 ****************************************************************/

char *room_name (rm)
long rm;
{ char *orig;
  static char buf[MSGSIZ];
  register char *s, *t;

  /* Check for room number out of bounds */
  if (rm < 0 || rm >= rooms || !room[rm].name)
  { sprintf (buf, "badroom[%ld]", rm);
    return (s = buf);
  }

  orig = room[rm].name;

  if (strlen (orig) < 40) { return (orig); }

  for (s=orig, t=buf; *s && !index (".:,;!?", *s); ) *t++ = *s++;
  while (isspace (t[-1])) t--;
  *t = '\0';

  if (strlen (buf) > 60)
  { t = buf+60;
    while (!isspace (*t) && (t-buf) > 40) t--;
    while (isspace (*t) && (t-buf) > 40) t--;
    t[1] = '\0';
  }

  return (s=buf);
}

/****************************************************************
 * check_stay: Stay longer if someone talks to us, and we are not busy
 ****************************************************************/

check_stay ()
{ static long laststay = -1;
  static long count = 0;

  if (laststay != hererm) count = 0;

  /*
   * If we are not answering a page, and people keep talking to us,
   * wait up to an extra 7 minutes in any one room.
   */

  if ((! *pagedby || *pagedby == '<' && !streq (pagedby, "<COMMAND>")) &&
      count <= 7 && nextwait < 60 && (paging || !exploring))
  { nextwait = 60;
    laststay = hererm;
    count++;
  }
}

/****************************************************************
 * othername: Name of some other person awake and not idle in the
 *	room.
 ****************************************************************/

char *othername (name)
char *name;
{ register long pl, cnt = 0;
  register char *res = name;

  if (msgtype >= M_WHISPER) return (name);

  for (pl=0; pl<players; pl++)
  { if (player[pl].present &&				/* is here */
  	player[pl].active &&				/* is awake */
	(now - player[pl].lastactive) < 5 * MINUTES &&	/* is not idle */
	!streq (player[pl].name, name) &&		/* other name */
	!strfoldeq (player[pl].name, thief) &&		/* is not thief */
	player[pl].lastkill - now > (18 * HOURS) &&	/* Not killer */
	randint (++cnt) == 0)				/* choose randomly */
    { res = player[pl].name; }
  }
  
  if (!streq (res, name))
  { strcpy (speaker, name);
    spoke_player (name);
  }
  
  return (res);
}
