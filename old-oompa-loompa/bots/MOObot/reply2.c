/****************************************************************
 * reply.c: Common responses for most TinyMUD automata
 *
 * HISTORY
 * 30-Oct-93  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Fourteenth animal release.
 *
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
# include <sys/ioctl.h>
# include <sys/socket.h>
# include <setjmp.h>
# include <netinet/in.h>
# include <netdb.h>
# include <ctype.h>
# include <varargs.h>

# include "robot.h"
# include "vars.h"

medium_std_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ long pl, rm;
  long full = 0;
  register long i;
  char rmstr[MSGSIZ], comp[MSGSIZ];
  char *s, *who;

  /*---- Hand over the pennies ----*/
  if       (!contest_mode &&
	   (((sindex (lcmsg, "pennies") ||
	     sindex (lcmsg, "cookie") ||
	     sindex (lcmsg, "penny") ||
	     sindex (lcmsg, "money") ||
	     sindex (lcmsg, "cash") ||
	     sindex (lcmsg, "copper") ||
	     sindex (lcmsg, "gold") ||
	     sindex (lcmsg, "silver") ||
	     sindex (lcmsg, "dime") ||
	     sindex (lcmsg, "quarter") ||
	     sindex (lcmsg, "dollar") ||
	     sindex (lcmsg, "buck")) &&
 
	    (MATCH (lcmsg, "*give*") ||
	     MATCH (lcmsg, "*bestow*") ||
	     MATCH (lcmsg, "*hand* me*") ||
	     MATCH (lcmsg, "*fork*over*") ||
	     MATCH (lcmsg, "*loan*") ||
	     MATCH (lcmsg, "*may*i*have*") ||
	     MATCH (lcmsg, "*may*i*borrow*") ||
	     MATCH (lcmsg, "*can*i*have*") ||
	     MATCH (lcmsg, "*can*i*borrow*") ||
	     MATCH (lcmsg, "*can*you*spare*") ||
	     MATCH (lcmsg, "*could*i*have*") ||
	     MATCH (lcmsg, "*could*i*borrow*") ||
	     MATCH (lcmsg, "*could*you*spare*") ||
	     MATCH (lcmsg, "*i*need*"))) ||
	    (MATCH (lcmsg, "*trick*or*treat*"))))
  { static long givepl = -1;
    static long givetime = 0;
    long pl = add_player (name);
    long clock = time (0);
    
    if (pl < 0) return (1);

    strcpy (speaker, name);
    spoke_player (name);
    
    if (sindex (lcmsg, "don't") || sindex (lcmsg, "do not"))
    { reply ("\"Okay{, n}, I won't.");
      return (1);
    }
    
    if (!(isowner (name) && (trusting || *code && sindex (msg, code))) &&
        ((pl = find_player (name)) < 0 ||
	  (now - player[pl].firstsaw) < 1 * HOURS))
    { reply ("\"Sorry{, n}, you're too new.  Wait an hour and ask again.");
      return (1);
    }

    if (isowner (name) && (trusting || *code && sindex (msg, code)) || generous || pennies > MAXPEN)
    { if (pennies > 0)
      { long amount;
	static long lastgive = 0;

	if (isowner (name) && (trusting || *code && sindex (msg, code)))
	{ if (MATCH (lcmsg, "*me * penn*") && isdigit (*res2))
	  { amount = atoi (res2);
	    if (amount < 1) amount = pennies;
	    if (amount > pennies) amount = pennies;
	  }
	  else
	  { amount = pennies; }
	}
	else
	{ long polite = MATCH (lcmsg, "*please*");

	  if (clock < player[pl].lastgave + 12 * HOURS)
	  { givepl = pl;
	    givetime = clock;
	    reply ("\"I just gave you %s %s ago, %s.",
		   streq (world, "ASYLUM") ? "cookies" : "pennies",
		   time_dur (clock - player[pl].lastgave), name);
	    return (1);
	  }
	  else if (lastgive + 300 < clock)
	  { amount = randint ((pennies - MINPEN) / (polite ? 2 : 4)); }
	  else
	  { amount = randint ((pennies - MINPEN) / (polite ? 4 : 8)); }

	  if ((pennies - amount) < MINPEN)
	      { amount = pennies - MINPEN; }

	  if (amount > (polite ? 80 : 40))
	  { if (polite)	amount = randint (40) + 40;
	    else		amount = randint (20) + 20;
	  }

	  if (is_jerk (name)) amount = 0;

	  /* Robot specific changes */
	  if (amount > 0)
	  { amount = amount_hook (amount, name, polite);
	    if (amount == 0) return (1);
	  }

	  /* Handle case where no pennies to give */
	  if (amount > 0)
	  { lastgive = clock; }
	  else
	  { reply ("\"I just don't have any extra %s, %s.",
		     streq (world, "ASYLUM") ? "cookies" : "pennies", name);
	    return (1);
	  }
	}

	if (amount > 0)
	{ pennies -= amount;
	  gave_player (name, amount);
	  givepl = pl;
	  givetime = clock;
	  reply ("give %s=%ld", name, amount);
	  msgtype = M_UNKNOWN;
	  if (streq (world, "ASYLUM"))
	  { unlogged (":gives %s %ld %s.",
		      name, amount, (amount==1) ? "cookie" : "cookies");
	  }
	  else
	  { unlogged (":gives %s %ld penn%s.",
		      name, amount, (amount==1) ? "y" : "ies");
	  }
	  psynch ();

	  return (1);
	}
      }
      else
      { reply ("\"I have no %s, %s.",
	       streq (world, "ASYLUM") ? "cookies" : "pennies", name);
	return (1);
      }
    }
    else if (givepl >= 0 && pl == givepl && clock < givetime + 600)
    { if (clock < givetime + 60)
      { /* Ignore it */ }
      else if (clock < givetime + 300)
      { reply (":gives %s the cold shoulder.", name); }
      else
      { reply (":laughs at %s.", name); }

      givepl = pl;
      givetime = clock;
    }
    else if (generous)
    { reply ("\"Try again a little latter{, n}.");
      givepl = pl;
      givetime = clock;
    }
    else
    { reply ("\"I'd rather not{, n}.");
      givepl = pl;
      givetime = clock;
    }

    return (1);
  }

  /*---- An order to say something to someone ----*/
  else if (MATCH (lcmsg, "say * to *,*") ||
	   MATCH (lcmsg, "say * to *"))
  { strcpy (speaker, name);
  
    if (isowner (name) && (trusting || *code && sindex (msg, code)) ||
	msgtype == M_SPOKEN && randint (100) < 70)
    { msgtype = M_SPOKEN;
      reply ("\"%s, %s", res1, res2); return (1);
    }
    else if (msgtype == M_WHISPER && randint (100) < 50)
    { msgtype = M_SPOKEN;
      zinger ("\"Why do ask me to say '%s' to %s{, n}?", res1, res2);
      return (1);
    }
    else
    { zinger ("\"Why{, n}?"); return (1); }
  }

  /*---- An order to say something ----*/
  else if (MATCH (lcmsg, "say *,*") ||
	   MATCH (lcmsg, "say *"))
  { strcpy (speaker, name);
    spoke_player (name);
    if (isowner (name) && (trusting || *code && sindex (msg, code)) ||
	msgtype == M_SPOKEN && randint (100) < 50)
    { msgtype = M_SPOKEN;
      reply ("\"%s", res1); return (1);
    }
    else if (msgtype == M_WHISPER && randint (100) < 50)
    { msgtype = M_SPOKEN;
      zinger ("\"Why do ask me to say '%s'{, n}?", res1);
      return (1);
    }
    else
    { zinger ("\"Why{, n}?"); return (1); }
  }

  /*---- Who is the target ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*who*is*target*") ||
	   MATCH (lcmsg, "*who*terminator*after*") ||
	   MATCH (lcmsg, "*who*terminator*seek*") ||
	   MATCH (lcmsg, "*who*terminator*search*") ||
	   MATCH (lcmsg, "*who*terminator*look*") ||
	   MATCH (lcmsg, "*who*terminator*vict*") ||
	   MATCH (lcmsg, "*who*is*contract*") ||
	   MATCH (lcmsg, "*what*is*contract*") ||
	   MATCH (lcmsg, "*who*terminator*hit*") ||
	   MATCH (lcmsg, "*who*terminator*kill*")))
  { strcpy (speaker, name);
    spoke_player (name);

    if (termtarg >= 0)
    { reply ("\"Terminator was looking for %s in %s %s ago, %s.",
	      player[termtarg].name, termloc,
	      time_dur (time (0) - termat), name);
      if (termwarn)
      { reply ("\"I warned %s %s ago.", player[termtarg].name,
	      time_dur (time (0) - termwarn));
      }
    }
    else
    { reply ("\"I don't know who Terminator is looking for{, n}."); }

    return (1);
  }

  /*---- A query about 'noisiest rooms' ----*/
  else if (!contest_mode &&
	  ((sindex (lcmsg, "which") || sindex (lcmsg, "what")) &&
	   sindex (lcmsg, " nois") && !sindex (lcmsg, "least") &&
	   (sindex (lcmsg, "room") ||
	    sindex (lcmsg, "place") ||
	    sindex (lcmsg, "area"))))
  { strcpy (speaker, name);
    spoke_player (name);
    most_rm_query (name, NOISE);
    return (1);
  }

  /*---- A query about 'popular rooms' ----*/
  else if (!contest_mode &&
	  ((sindex (lcmsg, "which") || sindex (lcmsg, "what")) &&
	   (sindex (lcmsg, " people") || sindex (lcmsg, "popular")) &&
	   !sindex (lcmsg, "least") &&
	   (sindex (lcmsg, "room") ||
	    sindex (lcmsg, "place") ||
	    sindex (lcmsg, "area"))))
  { strcpy (speaker, name);
    spoke_player (name);
    most_rm_query (name, PEOPLE);
    return (1);
  }

  /*---- A query about 'what rooms seen' ----*/
  else if (!contest_mode &&
	  ((sindex (lcmsg, "which") || sindex (lcmsg, "what")) &&
	   sindex (lcmsg, " sleep") && !sindex (lcmsg, "least") &&
	   (sindex (lcmsg, "room") ||
	    sindex (lcmsg, "place") ||
	    sindex (lcmsg, "area"))))
  { strcpy (speaker, name);
    spoke_player (name);
    most_rm_query (name, SLEEPERS);
    return (1);
  }

  /*---- A query about 'what rooms friendliest' ----*/
  else if (!contest_mode &&
	  ((sindex (lcmsg, "which") || sindex (lcmsg, "what")) &&
	   sindex (lcmsg, " friendl") && !sindex (lcmsg, "least") &&
	   (sindex (lcmsg, "room") ||
	    sindex (lcmsg, "place") ||
	    sindex (lcmsg, "area"))))
  { strcpy (speaker, name);
    spoke_player (name);
    most_rm_query (name, FRIENDLY);
    return (1);
  }

  /*---- A query about 'what rooms seen' ----*/
  else if (!contest_mode &&
  	   (MATCH (lcmsg, "*how*many*room*explor*")))
  { strcpy (speaker, name);
    spoke_player (name);

    if (lastrm)
    { reply ("\"I %s %ld of the %ld rooms %s, %s, and have tried %ld exits.",
	     "have explored", 
	      lastrm, rooms, "I have been to", name, exits);
    }
    else
    { reply ("\"I have been to %ld rooms{, n}.", rooms); }
    return (1);
  }

  /*---- A query about 'what rooms are reachable' ----*/
  else if (!contest_mode &&
  	   (MATCH (lcmsg, "*how*many*room*reach*")))
  { long yes = 0, no = 0;

    for (i=0; i<rooms; i++)
    { if (room[i].name)
      { if (can_reach (i))	yes++;
        else			no++;
      }
    }

    strcpy (speaker, name);
    spoke_player (name);
    reply ("\"%ld of %ld rooms are reachable from %s, and %ld are not, %s.",
	      yes, yes+no, home, no, name);
    return (1);
  }

  /*---- A query about 'what rooms match X' ----*/
  else if (!contest_mode &&
	  ((sindex (lcmsg, "what") || sindex (lcmsg, "which")) &&
	   (MATCH (lcmsg, "*room*match \"*\"*") ||
	    MATCH (lcmsg, "*room*match '*'*") ||
	    MATCH (lcmsg, "*room*match <*>*") ||
	    MATCH (lcmsg, "*room*match *,*") ||
	    MATCH (lcmsg, "*room*match *"))))
  { strcpy (speaker, name);
    spoke_player (name);

    if (sindex (res2, "desc"))
    { room_query (res3, name, 1); }
    else
    { room_query (res3, name, 0); }
    return (1);
  }

  /*---- A query about 'what rooms seen' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*what*room*discover*") ||
	   MATCH (lcmsg, "*what*rooms*found*") ||
	   MATCH (lcmsg, "*what*rooms*been*") ||
	   MATCH (lcmsg, "*what*rooms*seen*") ||
	   MATCH (lcmsg, "*what*rooms*heard*") ||
	   MATCH (lcmsg, "*what*rooms*told*") ||
	   MATCH (lcmsg, "*what*rooms*told*") ||
	   MATCH (lcmsg, "*what*rooms*tell*") ||
	   MATCH (lcmsg, "*how*many*rooms*") ||
	   MATCH (lcmsg, "*which*rooms*seen*") ||
	   MATCH (lcmsg, "*which*rooms*been*") ||
	   MATCH (lcmsg, "*where*been*")))
  { char buf[BIGBUF];
    long rm;

    strcpy (speaker, name);
    spoke_player (name);

    sprintf (buf, "I have been to %ld rooms altogether, %s{ n}:",
	     rooms, "here are the last few,");
    for (rm = (rooms > 10) ? rooms-10 : 0; rm < rooms; rm++)
    { if (room[rm].name)
      { sprintf (buf, "%s %s%s",
		buf, room_name (rm), rm == rooms-1 ? ".":",");
      }
    }
    reply ("\"%s", buf);
    return (1);
  }

  /*---- A query about 'where do you live' ----*/
  else if (MATCH (lcmsg, "*where*you*live*") ||
	   MATCH (lcmsg, "*where*you*reside*") ||
	   MATCH (lcmsg, "*where*you*domicile*") ||
	   MATCH (lcmsg, "*where*you*hang*out*"))
  { long now = time (0);
    strcpy (speaker, name);
    spoke_player (name);

    if (!contest_mode)
    { reply ("\"My home room is %s{, n}.", home); }
    else
    { static long last = 0;
    
      if (last + 15 * MINUTES > now)
      { reply ("\"I already told you, Pittsburgh{, n}."); }
      else
      { reply ("\"I live in Pittsburgh{, n}."); }
    }
    return (1);
  }

  /*---- A query about 'what date' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*what*date*") ||
	   MATCH (lcmsg, "*what*day*")))
  { long now = time (0);
    strcpy (speaker, name);
    spoke_player (name);
    reply ("\"The date is %19.19s{, n}.", ctime (&now));
    return (1);
  }

  /*---- A query about 'what date' ----*/
  else if (MATCH (lcmsg, "*predict*weather*") ||
	   MATCH (lcmsg, "*what*weather*") ||
	   MATCH (lcmsg, "*how*s*weather*") ||
	   MATCH (lcmsg, "*how*z*weather*") ||
	   MATCH (lcmsg, "*what*forecast*") ||
	   MATCH (lcmsg, "*describe*forecast*") ||
	   MATCH (lcmsg, "*describe*weather*"))
  { long now = time (0);
    strcpy (speaker, name);
    spoke_player (name);
    { switch (nrrint (201, 5))
      { case 0: zinger ("\"Tonight's forecast: dark.  Continued dark, with scattered light around sunrise."); break;
        case 1: zinger ("\"Partly cloudy skies in Allegheny county."); break;
        case 2: zinger ("\"Most parts of the country will have weather today.  Except Los Angeles."); break;
        case 3: zinger ("\"We'll either have sunny skies tomorrow or a plague of frogs.  Not both."); break;
        case 4: zinger ("\"I don't know, I haven't been outside, today."); break;        
      }
    }
    return (1);
  }

  /*---- A query about 'who recently' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*where is*every*one*") ||
	   MATCH (lcmsg, "*where is*every*body*") ||
	   MATCH (lcmsg, "*where are*all*people*") ||
	   MATCH (lcmsg, "*who*seen*") ||
	   MATCH (lcmsg, "*who*have*") ||
	   MATCH (lcmsg, "*who*around*")))
  { strcpy (speaker, name);
    spoke_player (name);
    all_player_query (4200, name);
    return (1);
  }

  /*---- A query about 'when was X logged int' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*has * been on*") ||
	   MATCH (lcmsg, "*has * been connec*") ||
	   MATCH (lcmsg, "*has * connected*") ||
	   MATCH (lcmsg, "*has * been logg*") ||
	   (sindex (lcmsg, "when") || sindex (lcmsg, "what time")) &&
	   (MATCH (lcmsg, "* was *player * last*") ||
	    MATCH (lcmsg, "* was *player * on*") ||
	    MATCH (lcmsg, "* was *player * log*") ||
	    MATCH (lcmsg, "* was *player * connected*") ||
    	    MATCH (lcmsg, "* was *player * here*") ||
       	    (MATCH (lcmsg, "* was *the* time * here*") ||
       	     MATCH (lcmsg, "* was *the* time * on*") ||
       	     MATCH (lcmsg, "* was *the* time * log*") ||
       	     MATCH (lcmsg, "* was *the* time * connect*")) &&
	     	strcpy (res2, res4) ||
	    MATCH (lcmsg, "* did you last see player *") ||
	    MATCH (lcmsg, "* did you see player *") ||
	    MATCH (lcmsg, "* did you last see *") ||
	    MATCH (lcmsg, "* did you see *") ||
	    MATCH (lcmsg, "* was * last*") ||
	    MATCH (lcmsg, "* was * on*") ||
	    MATCH (lcmsg, "* was * log*") ||
	    MATCH (lcmsg, "* was * here*") ||
	    MATCH (lcmsg, "* was * connected*") ||
	    MATCH (lcmsg, "* did * last*log*") ||
	    MATCH (lcmsg, "* did * last*connect*") ||
	    MATCH (lcmsg, "* did * log*last*") ||
	    MATCH (lcmsg, "* did * connect*last*") ||
	    MATCH (lcmsg, "* was * active*") ||
	    MATCH (lcmsg, "* was * connected*"))))
  { char *who = car (res2);

    strcpy (speaker, name);
    spoke_player (name);

    pl = close_player (who, name);

    if (pl < 0)
    { reply ("\"I have never seen player %s{, n}.", res2); }
    else if (strfoldeq (res2, myname))
    { reply ("\"I'm right here in %s{, n}.", room_name (hererm)); }
    else if (player[pl].active)
    { reply ("\"%s is on right now{, n}.", player[pl].name); }
    else
    { reply ("\"%s was on %s ago{, n}.",
	     player[pl].name, time_dur (now-player[pl].lastactive));
    }

    return (1);
  }

  /*---- A query about 'how long X was logged in' ----*/
  else if (!contest_mode &&
	   (MATCH (lcmsg, "*how long*") ||
	    MATCH (lcmsg, "*what*length*time*") ||
	    MATCH (lcmsg, "*what*duration*")) &&
	   ((MATCH (lcmsg, "* was *the* time * here*") ||
       	     MATCH (lcmsg, "* was *the* time * on*") ||
       	     MATCH (lcmsg, "* was *the* time * log*") ||
       	     MATCH (lcmsg, "* was *the* time * connect*")) &&
	     	strcpy (res2, res4) ||
	    MATCH (lcmsg, "* did you last see player *") ||
	    MATCH (lcmsg, "* did you see player *") ||
	    MATCH (lcmsg, "* did you last see *") ||
	    MATCH (lcmsg, "* did you see *") ||
	    MATCH (lcmsg, "* was * last*") ||
	    MATCH (lcmsg, "* was * on*") ||
	    MATCH (lcmsg, "* was * log*") ||
	    MATCH (lcmsg, "* was * here*") ||
	    MATCH (lcmsg, "* was * connected*") ||
	    MATCH (lcmsg, "* did * last*log*") ||
	    MATCH (lcmsg, "* did * last*connect*") ||
	    MATCH (lcmsg, "* did * log*last*") ||
	    MATCH (lcmsg, "* did * connect*last*") ||
	    MATCH (lcmsg, "* was * active*") ||
	    MATCH (lcmsg, "* was * connected*")))
  { char *who = car (res2);

    strcpy (speaker, name);
    spoke_player (name);

    pl = close_player (who, name);

    if (pl < 0)
    { reply ("\"I have never seen player %s{, n}.", res2); }
    else
    { reply ("\"I don't keep track of player's session lengths{, n}."); }

    return (1);
  }

  /*---- A query about 'who' ----*/
  else if (!contest_mode &&
	  ((((MATCH (lcmsg, "*where* does * live*") ||
	      MATCH (lcmsg, "*where* does * hang*") ||
	      MATCH (lcmsg, "*where*is *,*") ||
	      MATCH (lcmsg, "*where*is *") ||
	      MATCH (lcmsg, "*where*was *,*") ||
	      MATCH (lcmsg, "*where*did you*see *") &&
			strcpy (res3, res4) ||
	      MATCH (lcmsg, "*where's *,*") && strcpy (res3, res2) ||
	      MATCH (lcmsg, "*where's *") && strcpy (res3, res2) ||
	      MATCH (lcmsg, "*you know*where * is*") ||
	      MATCH (lcmsg, "*you know*where * was*")) &&
	     (pl = close_player (car (res3), name)) >= 0) ||
	     MATCH (lcmsg, "*have you*seen *,*") ||
	     MATCH (lcmsg, "*have you*seen *")) &&
	   !streq (car (res3), "your")))
  { char plyr[TOKSIZ];
    strcpy (plyr, car (res3));

    pl = close_player (plyr, name);

    strcpy (speaker, name);
    spoke_player (name);

    if (pl < 0)
    { reply ("\"I have never seen player %s{, n}.", plyr); }
    else if (is_me (plyr))
    { reply ("\"I'm right here in %s{, n}.", room_name (hererm)); }
    else
    { player_query (pl, name); }
    return (1);
  }

  /*---- A query about 'who' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*what is *'s *address*") ||
	   MATCH (lcmsg, "*what is *'s email*") ||
	   MATCH (lcmsg, "*what is *' email*")  ||
	   MATCH (lcmsg, "*what is * email*")   ||
	   MATCH (lcmsg, "*what is * address*")   ||
	   MATCH (lcmsg, "*what's *'s *address*")  ||
	   MATCH (lcmsg, "*what's *'s email*")  ||
	   MATCH (lcmsg, "*what's *' email*")   ||
	   MATCH (lcmsg, "*what's * email*")    ||
	   MATCH (lcmsg, "*what's * address*")    ||
	   MATCH (lcmsg, "*do you know *'s email*") ||
	   MATCH (lcmsg, "*do you know *'s *address*") ||
	   MATCH (lcmsg, "*what *address do you have for *") &&
		strcpy (res2, car (res3))))
  { char *s = car (res2);

    if (streq (s, "me") || streq (s, "my"))    strcpy (s, name);
    if (streq (s, "you") || streq (s, "your")) strcpy (s, myname);

    strcpy (speaker, name);
    spoke_player (name);

    if (strfoldeq (s, myname))
    { if (strfoldeq (myname, "julia"))
      { zinger ("\"My email address is <julia@fuzine.mt.cs.cmu.edu>"); }
      else
      { reply ("\"I don't have an email address{, n}."); }
    }

    else if ((pl = find_player (s)) >= 0)
    { if (strfoldeq (s, name))
      { if (player[pl].email)
        { reply ("\"I have your email address listed as %s, %s.",
		 player[pl].email, name);
        }
	else
	{ reply ("\"I have no email address for you, %s, but %s, %s\".",
		 name, "you could say \"My email address is <addr>",
		 myname);
	}
      }
      else
      { if (player[pl].email)
        { reply ("\"I have %s's email address listed as %s, %s.",
		 player[pl].name, player[pl].email, name);
        
        }
	else
	{ reply ("\"I have no email address for player %s, %s.",
		player[pl].name, name);
	}
      }
    }
    else
    { reply ("\"I have never heard of player %s{, n}.", s); }

    return (1);
  }

  /*---- A query about 'who' ----*/
  else if (!contest_mode &&
	   (((MATCH (lcmsg, "*who was the *") ||
	    MATCH (lcmsg, "* who the * was*") ||
	    MATCH (lcmsg, "*who was the *") ||
	    MATCH (lcmsg, "*who was *") ||
	    MATCH (lcmsg, "* who * was*") ||
	    MATCH (lcmsg, "*who is the *") ||
	    MATCH (lcmsg, "* who the * is*") ||
	    MATCH (lcmsg, "*who is the *") ||
	    MATCH (lcmsg, "*who is *") ||
	    MATCH (lcmsg, "* who * is*") ||
	    MATCH (lcmsg, "*who am *") ||
	    MATCH (lcmsg, "*who was *") ||
	    MATCH (lcmsg, "*who is *") ||
	    MATCH (lcmsg, "*who the hell is *") ||
	    MATCH (lcmsg, "*who's *") ||
	    MATCH (lcmsg, "*do you know player *") ||
	    (MATCH (lcmsg, "*do you know about *") ||
	     MATCH (lcmsg, "*do you know *")) &&
	        !reserved (car (res2)) &&
	     	find_player (car (res2)) >= 0 ||
	    MATCH (lcmsg, "* old is *") ||
	    MATCH (lcmsg, "* old am *") ||
	    MATCH (lcmsg, "*quien es *") ||
	    MATCH (lcmsg, "* id does * have*") ||
	    MATCH (lcmsg, "* id do * have*") ||
	    ((MATCH (lcmsg, "*describe the *") ||
	      MATCH (lcmsg, "*describe *") ||
	      MATCH (lcmsg, "*what does * look*like*") ||
	      MATCH (lcmsg, "*what does the * look*like*")) &&
	      (close_player (car (res2), name) >= 0 ||
	       streq (car (res2), "yourself") ||
	       !sindex (res2, " ") &&
	       !(streq (res2, "there") || streq (res2, "here")))) ||
	    ((MATCH (lcmsg, "*tell*about *")) &&
	      (close_player (car (res3), name) >= 0 ||
	       streq (car (res3), "yourself") ||
	       !sindex (res2, " "))) &&
	     strcpy (res2, res3) ||
	    MATCH (lcmsg, "*who am i*") && strcpy (res2, lcstr (name))) &&
	    (sindex (lcmsg, " player ") ||
	     !(streq (res2, "around") ||
	       streq (res2, "here") ||
	       streq (res2, "there"))) &&
	    (close_player (car (res2), name) >= 0 ||
	     sindex ("you,yourself,me,i", car (res2)) ||
	     !sindex (res2, " "))) &&
	    !reserved (car (res2))))
  { char pers[MSGSIZ];

    strcpy (pers, car (res2));
    
    if (streq (pers, "yourself") || streq (pers, "you"))
    { strcpy (pers, lcstr (myname)); }

    if (streq (pers, "me") || streq (pers, "i"))
    { strcpy (pers, lcstr (name)); }

    strcpy (speaker, name);
    spoke_player (name);
    
    if ((MATCH (lcmsg, "*how*old*") ||
	 MATCH (lcmsg, "* age *") ||
	 MATCH (lcmsg, "* number *")) &&
	(pl = close_player (pers, name)) >= 0)
    { if (player[pl].firstsaw == 0)
      { long start = 635499244;
        char oldest[TOKSIZ], latest[TOKSIZ];
	
	strcpy (oldest, time_dur (now - creation));
	strcpy (latest, time_dur (now - start));
	
	reply ("\"Gee{, n}, %s. I first saw %s between %s and %s ago.",
		"I'm not sure", 
		streq (player[pl].name, name) ? "you" : player[pl].name,
		latest, oldest);
      }
      else
      { reply ("\"I first saw %s logged in %s ago{, n}.",
	       is_me (pers) ? "myself" :
	       streq (player[pl].name, name) ? "you" : player[pl].name,
	       time_dur (now - player[pl].firstsaw));
      }
    }
    else
    { who_is (pers, name); }

    return (1);
  }

  /*---- Repeat gossip ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*quote anybody*") ||
	   MATCH (lcmsg, "*quote anyone*") ||
	   MATCH (lcmsg, "*quote somebody*") ||
	   MATCH (lcmsg, "*quote someone*") ||
	   MATCH (lcmsg, "*tell* me *gossip*") ||
	   MATCH (lcmsg, "*give* me *gossip*") ||
	   MATCH (lcmsg, "gossip, *") ||
	   MATCH (lcmsg, "*, gossip") ||
	   MATCH (lcmsg, "gossip") ||
	   MATCH (lcmsg, "*how*about*gossip*") ||
	   MATCH (lcmsg, "*give*me*quote*") && randint (100) < 50 ||
	   MATCH (lcmsg, "*give*gossip*")))
  { char pers[MSGSIZ];

    strcpy (pers, car (res2));

    strcpy (speaker, name);
    spoke_player (name);
    quote_random_player (name);
    return (1);
  }

  /*---- Repeat gossip ----*/
  else if (!contest_mode &&
	   ((MATCH (lcmsg, "*quote player *") ||
	     MATCH (lcmsg, "*quote the *") ||
	     MATCH (lcmsg, "*quote from player *") ||
	     MATCH (lcmsg, "*quote from the *") ||
	     MATCH (lcmsg, "*quote from *")) ||

	    (MATCH (lcmsg, "*quote *") &&
		 (streq (car (res2), "me") ||
		  !sindex (lcmsg, " ") ||
		  close_player (car (res2), name) >= 0))))
  { char pers[MSGSIZ];

    strcpy (pers, car (res2));

    strcpy (speaker, name);
    spoke_player (name);

    if (sindex (res1, "don't") || sindex (res1, "do not"))
    { reply ("\"Okay{, n}, I won't."); }
    else
    { quote_player (pers, name, 1); }
    return (1);
  }

  /*---- Repeat gossip ----*/
  else if (!contest_mode &&
	   (!sindex (lcmsg, " ") &&
	    (pl = find_player (lcmsg, name)) >= 0 &&
	    pl != me && !reserved (lcmsg)) &&
	    strcpy (res2, lcmsg) &&
	   quote_player (lcmsg, name, 0))
  { return (1); }

  /*---- Repeat gossip ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*gossip* player *") ||
	   MATCH (lcmsg, "*gossip* about *") ||
	   MATCH (lcmsg, "*gossip* from *")))
  { char pers[MSGSIZ];

    strcpy (pers, car (res3));

    strcpy (speaker, name);
    spoke_player (name);

    if (sindex (res1, "don't") || sindex (res1, "do not"))
    { reply ("\"Okay{, n}, I won't."); }
    else
    { quote_player (pers, name, 1); }
    return (1);
  }

  /*---- Who has X ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*who *has <*>*") ||
	   MATCH (lcmsg, "*who *has *,*") ||
	   MATCH (lcmsg, "*who *has *") ||
	   MATCH (lcmsg, "*quien *tiene <*>*") ||
	   MATCH (lcmsg, "*quien *tiene *,*") ||
	   MATCH (lcmsg, "*quien *tiene *") ||
	   MATCH (lcmsg, "*who *wears <*>*") ||
	   MATCH (lcmsg, "*who *wears *,*") ||
	   MATCH (lcmsg, "*who *wears *") ||
	   MATCH (lcmsg, "*who *wearing <*>*") ||
	   MATCH (lcmsg, "*who *wearing *,*") ||
	   MATCH (lcmsg, "*who *wearing *") ||
	   MATCH (lcmsg, "*who *matches <*>*") ||
	   MATCH (lcmsg, "*who *matches *,*") ||
	   MATCH (lcmsg, "*who *matches *") ||
	   MATCH (lcmsg, "*player*match <*>*") ||
	   MATCH (lcmsg, "*player*match *,*") ||
	   MATCH (lcmsg, "*player*match *") ||
	   MATCH (lcmsg, "*player*matches <*>*") ||
	   MATCH (lcmsg, "*player*matches *,*") ||
	   MATCH (lcmsg, "*player*matches *") ||
	   MATCH (lcmsg, "*who *carrying <*>*") ||
	   MATCH (lcmsg, "*who *carrying *,*") ||
	   MATCH (lcmsg, "*who *carrying *") ||
	   MATCH (lcmsg, "*who *carries <*>*") ||
	   MATCH (lcmsg, "*who *carries *,*") ||
	   MATCH (lcmsg, "*who *carries *")))
  { strcpy (speaker, name);
    spoke_player (name);

    if (stlmatch (res3, "the "))	object_query (res3+4, name);
    else if (stlmatch (res3, "a "))	object_query (res3+2, name);
    else if (stlmatch (res3, "an "))	object_query (res3+3, name);
    else				object_query (res3, name);

    return (1);
  }

  /*---- Describe room ----*/
  else if (!contest_mode &&
  	  (((MATCH (lcmsg, "*what*do*you*see*here*") ||
  	     MATCH (lcmsg, "*what*do*you*see,*") ||
  	     MATCH (lcmsg, "*what*do*you*see") ||
  	     MATCH (lcmsg, "*describe*your*surround*") ||
  	     MATCH (lcmsg, "*describe*the*room*")) &&
	    strcpy (rmstr, "here") ||
	   (MATCH (lcmsg, "*describe *") ||
	     MATCH (lcmsg, "*tell me about *") ||
	     MATCH (lcmsg, "*what is in *") ||
	     MATCH (lcmsg, "*what's in *") ||
	     MATCH (lcmsg, "*what's it like in *") ||
	     MATCH (lcmsg, "*what's it like *") ||
	     MATCH (lcmsg, "*what is * like") ||
	     MATCH (lcmsg, "*what's * like")) &&
	    strcpy (rmstr, res2)) &&

	    (((MATCH (rmstr, "the *,*") ||
	       MATCH (rmstr, "the * in detail") ||
	       MATCH (rmstr, "the *")) && 
	       (rm = close_room (res1)) >= (-1)) ||
	     ((MATCH (rmstr, "*,*") ||
	       MATCH (rmstr, "* in detail") ||
	       strcpy (res1, rmstr)) &&
	      (rm = close_room (res1)) >= (0)))))
  { int detail = 0;
    char tmp[512];

    if (sindex (lcmsg, "in detail")) detail++;

    strcpy (speaker, name);
    spoke_player (name);

    if (rm < 0)
    { reply ("\"I've never heard of '%s'{, n}.", res1);
      return (1);
    }

    reply ("\"Here's what I know about %s, as of %s ago, %s:",
	   room_name (rm), time_dur (now-room[rm].lastin), name);

    if (!streq (room[rm].name, room_name (rm)))
    { strcpy (speaker, name);
      reply ("| full name: %s", room[rm].name);
    }

    if (room[rm].desc && room[rm].desc[0])
    { strcpy (speaker, name);
      reply ("| description: %s", room[rm].desc); }

    if (room[rm].exlist && room[rm].exlist[0])
    { strcpy (speaker, name);
      reply ("| exits: %s", room[rm].exlist); }

    if (room[rm].contents && room[rm].contents[0])
    { strcpy (speaker, name);
      reply ("| contents: %s", room[rm].contents); }
    
    if (detail)
    { strcpy (speaker, name);
      if (room[rm].number >= 0)
      { reply ("|number is %ld", room[rm].number); }
      else
      { reply ("|number is unknown"); }
      
      strcpy (speaker, name);
      reply ("|first saw on %20.20s", ctime (&room[rm].firstin)+4);
      
      strcpy (speaker, name);
      reply ("|last seen on %20.20s", ctime (&room[rm].lastin)+4);
      
      strcpy (speaker, name);
      reply ("|total time in %s", time_dur (room[rm].totalin));
      
      strcpy (speaker, name);
      reply ("|visits %d", room[rm].cntin);
      
      strcpy (speaker, name);
      reply ("|awake %d", room[rm].awakesum);
      
      strcpy (speaker, name);
      reply ("|sleep %d", room[rm].sleepsum);
      
      strcpy (speaker, name);
      reply ("|msgs %d (%d to me, today)", room[rm].msgsum, room[rm].friendly);
      
      strcpy (speaker, name);
      reply ("|%s is a %s room",
	     room_name(rm), ispublic (room[rm].name) ? "public" : "private");
      
    }

    return (1);
  }

  /*---- Where are you going ----*/
  else if (MATCH (lcmsg, "*what*you doing*") ||
	   MATCH (lcmsg, "*wha*ya doing*") ||
	   MATCH (lcmsg, "*wha*cha doing*") ||
	   MATCH (lcmsg, "*what*you doing*") ||
	   MATCH (lcmsg, "*wha*ya doing*") ||
	   MATCH (lcmsg, "*are*you*busy*") ||
	   MATCH (lcmsg, "*where* you going*") ||
	   MATCH (lcmsg, "*where* you head*") ||
	   MATCH (lcmsg, "*where* you off to*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (pagedto >= 0 && *pagedby != '<')
    { reply ("\"I am going to %s to see %s, %s.",
	     room_name (pagedto), pagedby, name); }
    else if (pagedto >= 0)
    { reply ("\"I am on my way to %s{, n}.", room_name (pagedto)); }
    else if (msgtype >= M_PAGE)
    { reply ("\"I'm just hanging around in %s{, n}.", room_name (hererm));}
    else
    { switch (nrrint (166, 5))
      { case 0: zinger ("\"I'm just hanging around here{, n}."); break;
        case 1: zinger ("\"I'm just waiting for people to talk to{, n}.");
		break;
        case 2: zinger ("\"%s in particular{, n}.",
			sindex (lcmsg, "where") ? "Nowhere" : "Nothing");
		break;
        case 3: zinger ("\"I'm studying this room{, n}."); break;
        case 4: zinger ("\"I'm just examining these dust molecules{, n}.");
		break;
      }
    }
    
    return (1);
  }

  /*---- Where are you ----*/
  else if ((MATCH (lcmsg, "*where*are* you,*") ||
	    MATCH (lcmsg, "*where*are* you *") && !sindex (res4, "ing") ||
	    MATCH (lcmsg, "*where*are* you") ||
	    MATCH (lcmsg, "*where* you are,*") ||
	    MATCH (lcmsg, "*where* you are")) &&
	   !(sindex (res1, "how") &&
	     (sindex (res1, "get") ||
	      sindex (res1, "go") ||
	      sindex (res1, "come"))))
  { strcpy (speaker, name);
    spoke_player (name);

    if (contest_mode)
    { switch (nrrint (167, 4))
      { case 0: reply ("\"I am in Boston{, n}."); break;
        case 1: reply ("\"I'm not supposed to say{, n}."); break;
	case 2: reply ("\"I'm in a hidden room{, n}."); break;
        case 3: reply ("\"I can't tell you{, n}."); break;
      }
    }

    else if (*here)
    { switch (nrrint (168, 4))
      { case 0: reply ("\"I think I am in \"%s\"", room_name (hererm)); break;
        case 1: reply ("\"I think I'm in \"%s\"", room_name (hererm)); break;
	case 2: reply ("\"I'm in \"%s\"", room_name (hererm)); break;
	case 3: reply ("\"I am in \"%s\"", room_name (hererm)); break;
      }
    }
    else
    { switch (nrrint (169, 2))
      { case 0: reply ("\"I don't know{, n}."); break;
        case 1: reply ("\"I do not know{, n}."); break;
        
      }
    }

    return (1);
  }

  /*---- Where are we ----*/
  else if (MATCH (lcmsg, "*where*are* we,*") ||
	   MATCH (lcmsg, "*where*are* we *") && !sindex (res4, "ing") ||
	   MATCH (lcmsg, "*where*are* we") ||
	   MATCH (lcmsg, "*where*am *"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (contest_mode)
    { switch (nrrint (170, 4))
      { case 0: reply ("\"I am in Boston{, n}."); break;
        case 1: reply ("\"I'm not supposed to say{, n}."); break;
	case 2: reply ("\"I don't know where you are{, n}."); break;
        case 3: reply ("\"I can't tell you{, n}."); break;
      }
    }

    else if (*here && (pl = find_player (name)) >= 0 && player[pl].present)
    { switch (nrrint (171, 4))
      { case 0: reply ("\"I think we are in \"%s\"", room_name (hererm));break;
        case 1: reply ("\"I think we're in \"%s\"", room_name (hererm)); break;
	case 2: reply ("\"We're in \"%s\"", room_name (hererm)); break;
	case 3: reply ("\"We are in \"%s\"", room_name (hererm)); break;
      }
    }
    else
    { switch (nrrint (172, 2))
      { case 0: reply ("\"I don't know{, n}."); break;
        case 1: reply ("\"I do not know{, n}."); break;
        
      }
    }
    return (1);
  }

  /* What gender is X */
  else if ((MATCH (lcmsg, "*what*gender is player *") ||
	    MATCH (lcmsg, "*what*gender is the *") ||
	    MATCH (lcmsg, "*what*gender is a *") ||
	    MATCH (lcmsg, "*what*gender is *") ||
	    MATCH (lcmsg, "*what*sex is player *") ||
	    MATCH (lcmsg, "*what*sex is the *") ||
	    MATCH (lcmsg, "*what*sex is a *") ||
	    MATCH (lcmsg, "*what*sex is *")) &&
	   (who = car (res3)) &&
	   *who)
  { char whobuf[MSGSIZ];

    strcpy (speaker, name);
    spoke_player (name);
    
    strcpy (whobuf, who);
    
    if (strfoldeq (whobuf, myname)) { what_gender (name); return (1); }

    switch (nrrint (173, 4))
    { case 0:	zinger ("\"I guess that %s is %s, %s.",
			whobuf, malep (whobuf) ? "male" : "female", name);
		break;
      case 1:	zinger ("\"Isn't %s %s, %s?",
			whobuf, malep (whobuf) ? "male" : "female", name);
		break;
      case 2:	zinger ("\"I bet that %s is %s, %s.",
			whobuf, malep (whobuf) ? "male" : "female", name);
		break;
      case 3:	zinger ("\"I think that %s is %s, %s.",
			whobuf, malep (whobuf) ? "male" : "female", name);
		break;
    }
    
    return (1);
  }
 
  /* Is X male? */
  else if ((MATCH (lcmsg, "*is * male*") ||
	    MATCH (lcmsg, "*is * a man*") ||
	    MATCH (lcmsg, "*is * a boy*") ||
	    MATCH (lcmsg, "*is * a gentleman*")) &&
	   !index (res2, ' ') && (who = res2) && *who)
  { char whobuf[MSGSIZ];

    strcpy (speaker, name);
    spoke_player (name);
    
    strcpy (whobuf, who);

    if (strfoldeq (whobuf, myname)) { what_gender (name); return (1); }

    if (malep (whobuf))
    { zinger ("\"I think so{, n}."); }
    else
    { zinger ("\"I don't think so{, n}."); }
    
    return (1);
  }
 
  /* Is X female? */
  else if ((MATCH (lcmsg, "*is * female*") ||
	    MATCH (lcmsg, "*is * a woman*") ||
	    MATCH (lcmsg, "*is * a girl*") ||
	    MATCH (lcmsg, "*is * a lady*")) &&
	   !index (res2, ' ') && (who = res2) && *who)
  { char whobuf[MSGSIZ];

    strcpy (speaker, name);
    spoke_player (name);
    
    strcpy (whobuf, who);

    if (strfoldeq (whobuf, myname)) { what_gender (name); return (1); }

    if (!malep (whobuf))
    { zinger ("\"I think so{, n}."); }
    else
    { zinger ("\"I don't think so{, n}."); }
    
    return (1);
  }
 
  /* Question about gender */
  else if (MATCH (lcmsg, "*what*gender am i*") ||
	   MATCH (lcmsg, "*what*is*my*gender*") ||
	   MATCH (lcmsg, "*what*sex am i*") ||
	   MATCH (lcmsg, "*what*is*my*sex*"))
  { char whobuf[MSGSIZ];

    strcpy (speaker, name);
    spoke_player (name);
    
    zinger ("\"I guess that you are %s, %s.",
	    malep (name) ? "male" : "female", name);
    
    return (1);
  }
 
  /* Question about gender */
  else if (MATCH (lcmsg, "*are*you*male*") ||
	   MATCH (lcmsg, "*are*you*woman*") ||
	   MATCH (lcmsg, "*are*you* boy*") ||
	   MATCH (lcmsg, "*are*you* girl*") ||
	   MATCH (lcmsg, "*are*you*a man*") ||
	   MATCH (lcmsg, "*what*gender are you*") ||
	   MATCH (lcmsg, "*what*is*your*gender*") ||
	   MATCH (lcmsg, "*what*sex are you*") ||
	   MATCH (lcmsg, "*what*is*your*sex*"))
  { strcpy (speaker, name);
    spoke_player (name);
    what_gender (name);    
    return (1);
  }

  /*---- I am male ----*/
  else if ((MATCH (lcmsg, "*i am*") ||
  	    MATCH (lcmsg, "*i'm*")) &&
	   strcpy (comp, res2) &&
	   ((MATCH (comp, "* male") ||
	     MATCH (comp, "* male,*") ||
	     MATCH (comp, "* male;*") ||
	     MATCH (comp, "* male.*") ||
	     MATCH (comp, "* a man*") ||
	     MATCH (comp, "* a boy*")) && 
	    !sindex (res1, " not") ||
	   MATCH (comp, "* not female") ||
	   MATCH (comp, "* not a female") ||
	   MATCH (comp, "* not a girl") ||
	   MATCH (comp, "* not a woman")))
  { strcpy (speaker, name);
    spoke_player (name);
    
    if ((pl = find_player (name)) < 0)
    { zinger ("\"So{, n}?"); }
    
    else if (PLAYER_GET (pl, PL_MALE))
    { zinger ("\"You already told me that{, n}.");  }
    
    else if (PLAYER_GET (pl, PL_FEMALE))
    { PLAYER_SET (pl, PL_MALE);
      PLAYER_CLR (pl, PL_FEMALE);
      PLAYER_CLR (pl, PL_ANDRO);
      
      zinger ("\"%s %s; %s{, n}?",
    	      randint(100)<50 ? "You said earlier that" : "You told me",
    	      randint(100)<50 ? "you were female" : "you were a woman",
    	      randint(100)<50 ? " been to Sweden" : "get a sex change");
    }
    
    else if (PLAYER_GET (pl, PL_ANDRO))
    { PLAYER_SET (pl, PL_MALE);
      PLAYER_CLR (pl, PL_FEMALE);
      PLAYER_CLR (pl, PL_ANDRO);
      
      zinger ("\"%s %s; %s{, n}",
    	      randint(100)<50 ? "You said earlier that" : "You told me",
    	      randint(100)<50 ? "you were androgenous" : "you were unisex",
	      "I'm glad you're comfortable now with being male");
    }
    
    else if (malep (name))
    { PLAYER_SET (pl, PL_MALE);
      PLAYER_CLR (pl, PL_FEMALE);
      PLAYER_CLR (pl, PL_ANDRO);
      zinger ("\"That's what I thought{, n}");
    }
    
    else
    { PLAYER_SET (pl, PL_MALE);
      PLAYER_CLR (pl, PL_FEMALE);
      PLAYER_CLR (pl, PL_ANDRO);
      zinger ("\"Oops, sorry! I thought you were %s, %s.  My mistake.",
	      randint (100) < 50 ? "female" : "a woman", name);
    }
    
    return (1);
  }
 

  /*---- I am female ----*/
  else if ((MATCH (lcmsg, "*i am*") ||
  	    MATCH (lcmsg, "*i'm*")) &&
	   strcpy (comp, res2) &&
	   ((MATCH (comp, "* female*")||
	     MATCH (comp, "* a woman*")||
	     MATCH (comp, "* a girl*")) && 
	    !sindex (res1, " not") ||
	   MATCH (comp, "* not male") ||
	   MATCH (comp, "* not a man") ||
	   MATCH (comp, "* not a boy")))
  { strcpy (speaker, name);
    spoke_player (name);
    
    if ((pl = find_player (name)) < 0)
    { zinger ("\"So{, n}?"); }
    
    else if (PLAYER_GET (pl, PL_FEMALE))
    { zinger ("\"You already told me that{, n}.");  }
    
    else if (PLAYER_GET (pl, PL_MALE))
    { PLAYER_SET (pl, PL_FEMALE);
      PLAYER_CLR (pl, PL_MALE);
      PLAYER_CLR (pl, PL_ANDRO);
      
      zinger ("\"%s %s; %s{, n}?",
    	      randint(100)<50 ? "You said earlier that" : "You told me",
    	      randint(100)<50 ? "you were male" : "you were a man",
    	      randint(100)<50 ? " been to Sweden" : "get a sex change");
    }
    
    else if (PLAYER_GET (pl, PL_ANDRO))
    { PLAYER_SET (pl, PL_FEMALE);
      PLAYER_CLR (pl, PL_ANDRO);
      PLAYER_CLR (pl, PL_MALE);
      
      zinger ("\"%s %s; %s{, n}",
    	      randint(100)<50 ? "You said earlier that" : "You told me",
    	      randint(100)<50 ? "you were androgenous" : "you were unisex",
	      "I'm glad you're comfortable now with being female");
    }
    
    else if (!malep (name))
    { PLAYER_SET (pl, PL_FEMALE);
      PLAYER_CLR (pl, PL_MALE);
      PLAYER_CLR (pl, PL_ANDRO);
      zinger ("\"That's what I thought{, n}");
    }
    
    else
    { PLAYER_SET (pl, PL_FEMALE);
      PLAYER_CLR (pl, PL_MALE);
      PLAYER_CLR (pl, PL_ANDRO);
      zinger ("\"Oops, sorry! I thought you were %s{, n}.  My mistake.",
	      randint (100) < 50 ? "male" : "a man");
    }
    
    return (1);
  }
 
  /* Message not matched by this rule set */
  return (0);
}

what_gender (name)
char *name;
{
  if (male)
  { switch (nrrint (174, 6))
    { case 0:     zinger ("\"I am clearly male{, n}."); break;
      case 1:     zinger ("\"I am male{, n}."); break;
      case 2:     zinger ("\"I'm a man{, n}."); break;
      case 3:     zinger ("\"I'm male{, n}."); break;
      case 4:     zinger ("\"Can't you tell{, n}?"); break;
      case 5:     zinger ("\"Don't you know{, n}?"); break;
    }
  }
  else
  { switch (nrrint (175, 6))
    { case 0:     zinger ("\"I am clearly female{, n}."); break;
      case 1:     zinger ("\"I am female{, n}."); break;
      case 2:     zinger ("\"I'm a woman{, n}."); break;
      case 3:     zinger ("\"I'm female{, n}."); break;
      case 4:     zinger ("\"Can't you tell{, n}?"); break;
      case 5:     zinger ("\"Don't you know{, n}?"); break;
    }
  }
}

complex_standard_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ long pl, rm;
  long full = 0;
  register long i;
  char *s, tmp[MSGSIZ];

  /*---- A query about 'how do you make a robot' ----*/
  if (!contest_mode &&
	  ((stlmatch (lcmsg, "how ") || sindex (lcmsg, " how ")) &&
	   (sindex (lcmsg, " ftp ") ||
	    sindex (lcmsg, " make ") ||
	    sindex (lcmsg, " set up ") ||
	    sindex (lcmsg, " made") ||
	    sindex (lcmsg, " build ") ||
	    sindex (lcmsg, " built") ||
	    sindex (lcmsg, " write ") ||
	    sindex (lcmsg, " written") ||
	    sindex (lcmsg, " program ") ||
	    sindex (lcmsg, " programmed") ||
	    sindex (lcmsg, " create ") ||
	    sindex (lcmsg, " created")) &&
	   (sindex (lcmsg, "robot") ||
	    sindex (lcmsg, " bot ") ||
	    sindex (lcmsg, " bot,") ||
	    sindex (lcmsg, " bot?") ||
	    ematch (lcmsg, " bot") ||
	    sindex (lcmsg, "automaton") ||
	    sindex (lcmsg, " an ai"))))
  { strcpy (speaker, name);
    spoke_player (name);
    give_robot_help (lcmsg, name);
    return (1);
  }

  /*---- A query about 'how do you make a robot' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*tell*about*bot*") ||
	   MATCH (lcmsg, "*what*is a*bot*") && !isalpha (*res4) ||
	   MATCH (lcmsg, "*where*info*bot*") ||
	   MATCH (lcmsg, "*where*bot*info*") ||
	   MATCH (lcmsg, "*where*bot*source*") ||
	   MATCH (lcmsg, "*where*bot*code*") ||
	   MATCH (lcmsg, "*where*you*source*") ||
	   MATCH (lcmsg, "*where*you*code*") ||
	   MATCH (lcmsg, "*where*can i*bot*") ||
	   MATCH (lcmsg, "*how*get*bot*code*") ||
	   MATCH (lcmsg, "*how*get*bot*source*") ||
	   MATCH (lcmsg, "*how*get*you*code*") ||
	   MATCH (lcmsg, "*how*get*you*source*") ||
	   MATCH (lcmsg, "*what*are*bots*")))
  { strcpy (speaker, name);
    spoke_player (name);
    give_robot_help (lcmsg, name);
    return (1);
  }

  /*---- Answer query about 'how do I get from X to Y' ----*/
  else if ((MATCH (lcmsg, "*how*get*") ||
	    MATCH (lcmsg, "*what*route*") ||
	    MATCH (lcmsg, "*what*way*") ||
	    MATCH (lcmsg, "*what*path*") ||
	    MATCH (lcmsg, "*what*short*") ||
	    MATCH (lcmsg, "*how*travel*") ||
	    MATCH (lcmsg, "*how*go*")) &&

	   (MATCH (msg, "* between the * and the *,*") ||
	    MATCH (msg, "* between the * and *,*") ||
	    MATCH (msg, "* between * and the *,*") ||
	    MATCH (msg, "* between * and *,*") ||
	    MATCH (msg, "* between the * and the *") ||
	    MATCH (msg, "* between the * and *") ||
	    MATCH (msg, "* between * and the *") ||
	    MATCH (msg, "* between * and *") ||

	    MATCH (msg, "* from the * to the *,*") ||
	    MATCH (msg, "* from the * to *,*") ||
	    MATCH (msg, "* from * to the *,*") ||
	    MATCH (msg, "* from * to *,*") ||
	    MATCH (msg, "* from the * to the *") ||
	    MATCH (msg, "* from the * to *") ||
	    MATCH (msg, "* from * to the *") ||
	    MATCH (msg, "* from * to *")))
  { char src[MSGSIZ], dest[MSGSIZ], *answer, abuf[BIGBUF];
    long from, to, ptype = SHORTPATH;

    strcpy (speaker, name);
    spoke_player (name);

    if (contest_mode)
    { switch (nrrint (176, 3))
      { case 0: zinger ("\"That's not really a pet question{, n}."); break;
        case 1: zinger ("\"That goes beyond my domain{, n}."); break;
        case 2: zinger ("\"I really can't say{, n}."); break;
      }
      return (1);
    }
    
    answer = abuf;

    strcpy (dest, res3);
    strcpy (src, res2);
    
    if (!is_newbie (name)) ptype = LONGPATH;
    if (sindex (lcmsg, "quickly"))
    { ptype = SHORTPATHHOME;
      fprintf (stderr, "Path: allowing 'home' in path\n");
    }
    if ((to = close_room (dest)) < 0)
    { sprintf (abuf, "I've never been to '%s'{, n}.", dest); }

    else if ((from = close_room (src)) < 0)
    { sprintf (abuf, "I've never been to '%s'{, n}.", src); }

    else if (to == from)
    { sprintf (answer = abuf, "Sit still, I guess{, n}."); }

    else if ((answer =
	      find_path (from, to, ptype)))
    { strcpy (pathto, room[to].name);
      if (*answer == ' ')
      { if (from == hererm && msgtype < M_PAGE)
	{ sprintf (tmp, "From here, go%s", answer); }
	else
	{ sprintf (tmp, "From %s, go%s", room_name (from), answer); }
        answer = tmp;
      }
    }

    else
    { sprintf (answer = abuf,
		"Sorry{, n}, I don't know how to get to %s from %s.",
		room_name (to), room_name (from));
    }

    reply ("\"%s", answer);
    return (1);
  }

  /*---- Answer query about 'how do I get to X from Y' ----*/
  else if ((MATCH (lcmsg, "*how*get*") ||
	    MATCH (lcmsg, "*what*route*") ||
	    MATCH (lcmsg, "*how*travel*") ||
	    MATCH (lcmsg, "*how*go*")) &&

	   (MATCH (msg, "* to the * from the *,*") ||
	    MATCH (msg, "* to the * from the *") ||
	    MATCH (msg, "* to * from the *,*") ||
	    MATCH (msg, "* to * from the *") ||

	    MATCH (msg, "* to the * from *,*") ||
	    MATCH (msg, "* to the * from *") ||
	    MATCH (msg, "* to * from *,*") ||
	    MATCH (msg, "* to * from *") ||

	    (MATCH (msg, "* here from the *,*") ||
	     MATCH (msg, "* here from the *") ||
	     MATCH (msg, "* here from *,*") ||
	     MATCH (msg, "* here from *")) &&
	    strcpy (res3, res2) &&
	    strcpy (res2, "here")))
  { char src[MSGSIZ], dest[MSGSIZ], *answer, abuf[BIGBUF];
    long from, to, ptype = SHORTPATH;

    strcpy (speaker, name);
    spoke_player (name);

    answer = abuf;

    strcpy (dest, res2);
    strcpy (src, res3);
    

    if (contest_mode)
    { switch (nrrint (177, 3))
      { case 0: zinger ("\"That's not really a pet question{, n}."); break;
        case 1: zinger ("\"That goes beyond my domain{, n}."); break;
        case 2: zinger ("\"I really can't say{, n}."); break;
      }
      return (1);
    }
    
    if (!is_newbie (name)) ptype = LONGPATH;
    if (sindex (lcmsg, "quickly"))
    { ptype = SHORTPATHHOME;
      fprintf (stderr, "Path: allowing 'home' in path\n");
    }

    if ((to = close_room (dest)) < 0)
    { sprintf (abuf, "I've never been to %s{, n}.", dest); }

    else if ((from = close_room (src)) < 0)
    { sprintf (abuf, "I've never been to %s{, n}.", src); }

    else if (to == from)
    { sprintf (answer = abuf, "Sit still, I guess{, n}."); }

    else if (answer =
	     find_path (from, to, ptype))
    { strcpy (pathto, room[to].name);
      if (*answer == ' ')
      { if (from == hererm && msgtype < M_PAGE)
	{ sprintf (tmp, "From here, go%s", answer); }
	else
	{ sprintf (tmp, "From %s, go%s", room_name (from), answer); }
        answer = tmp;
      }
    }

    else
    { sprintf (answer = abuf,
	       "Sorry{, n}, I don't know how to get to %s from %s.",
	       room_name (to) , room_name (from));
    }

    reply ("\"%s", answer);
    return (1);
  }

  /*---- Answer query about 'how do I get to X' ----*/
  else if ((MATCH (lcmsg, "*know where* the * is*") ||
	    MATCH (lcmsg, "*know *where * is*") ||
	    MATCH (lcmsg, "*know where* the * are*") ||
	    MATCH (lcmsg, "*know *where * are*") ||
	    MATCH (lcmsg, "*where*is the *,*") ||
	    MATCH (lcmsg, "*where*is the *") ||
	    MATCH (lcmsg, "*where*are the *,*") ||
	    MATCH (lcmsg, "*where*are the *") ||
	    MATCH (lcmsg, "*where*is *,*") ||
	    MATCH (lcmsg, "*where*is *") ||
	    MATCH (lcmsg, "*where*are *,*") ||
	    MATCH (lcmsg, "*where*are *") ||
	    MATCH (lcmsg, "*donde*esta *,*") ||
	    MATCH (lcmsg, "*donde*esta *") ||
	    MATCH (lcmsg, "*donde*estan *,*") ||
	    MATCH (lcmsg, "*donde*estan *")) ||

	   (MATCH (lcmsg, "*how* get*") ||
	    MATCH (lcmsg, "*what* route*") ||
    	    MATCH (lcmsg, "* you *have* map *") ||
	    MATCH (lcmsg, "*how* travel*") ||
	    MATCH (lcmsg, "*how* go *") ||
	    MATCH (lcmsg, "*have* you*") ||
	    MATCH (lcmsg, "*lead* me*") ||
	    MATCH (lcmsg, "*show* me*") ||
	    MATCH (lcmsg, "*follow* me*") ||
	    MATCH (lcmsg, "*take* me*") ||
	    MATCH (lcmsg, "*como*puedo*llega*")) &&

	   ((MATCH (lcmsg, "*how* get there,*") ||
	     MATCH (lcmsg, "*how* get there")) &&
	    strcpy (res3, "there") ||
	    (MATCH (lcmsg, "*how* get here,*") ||
	     MATCH (lcmsg, "*how* get here")) &&
	    strcpy (res3, "there") ||
	    (MATCH (msg, "* to the * from here*") ||
	     MATCH (msg, "* to the *,*") ||
	     MATCH (msg, "* to the *") ||
	     MATCH (msg, "* to * from here*") ||
	     MATCH (msg, "* to *,*") ||
	     MATCH (msg, "* to *") ||
	     MATCH (msg, "* a la *,*") ||
	     MATCH (msg, "* a la *") ||
	     MATCH (msg, "* al *,*") ||
	     MATCH (msg, "* al *") ||
	     MATCH (msg, "* a *,*") ||
	     MATCH (msg, "* a *")) && strcpy (res3, res2)))
  { char dest[MSGSIZ], *answer, abuf[BIGBUF];
    long from, to;

    strcpy (speaker, name);
    spoke_player (name);
    

    if (contest_mode)
    { switch (nrrint (178, 3))
      { case 0: zinger ("\"That's not really a pet question{, n}."); break;
        case 1: zinger ("\"That goes beyond my domain{, n}."); break;
        case 2: zinger ("\"I really can't say{, n}."); break;
      }
      return (1);
    }
    
    answer = abuf;

    strcpy (dest, res3);

    if ((to = close_room (dest)) < 0)
    { sprintf (answer = abuf,
	       "I've never been to a room called '%s'{, n}.",
	       dest); }

    else if (msgtype < M_PAGE &&
	     (from = hererm) >= 0 &&
	     to != from &&
	     (answer =
	      find_path (from, to, is_newbie (name) ? SHORTPATH : LONGPATH)))
    { strcpy (pathto, room[to].name);
      if (*answer == ' ')
      { sprintf (tmp, "From here, go%s", answer);
        answer = tmp;
      }
    }

    else if (msgtype < M_PAGE && to == from)
    { sprintf (answer = abuf, "You're already there{, n}."); }

    else if ((from = close_room ("St. Cuthbert Plaza", NULL)) >= 0 &&
	     from != to &&
	     (answer =
	      find_path (from, to, is_newbie (name) ? SHORTPATH : LONGPATH)))
    { /* answer is complete */
      if (*answer == ' ')
      { sprintf (tmp, "From %s, go%s", room_name (from), answer);
        answer = tmp;
      }
    }

    else if ((from = close_room ("town", NULL)) >= 0 &&
	     from != to &&
	     (answer =
	      find_path (from, to, is_newbie (name) ? SHORTPATH : LONGPATH)))
    { /* answer is complete */
      if (*answer == ' ')
      { sprintf (tmp, "From %s, go%s", room_name (from), answer);
        answer = tmp;
      }
    }

    else if ((from = close_room ("nexus", NULL)) >= 0 &&
	     from != to &&
	     (answer =
	      find_path (from, to, is_newbie (name) ? SHORTPATH : LONGPATH)))
    { /* answer is complete */
      if (*answer == ' ')
      { sprintf (tmp, "From %s, go%s", "the nexus", answer);
        answer = tmp;
      }
    }

    else if ((from = close_room ("chat", NULL)) >= 0 &&
	     from != to &&
	     (answer =
	      find_path (from, to, is_newbie (name) ? SHORTPATH : LONGPATH)))
    { /* answer is complete */
      if (*answer == ' ')
      { sprintf (tmp, "From %s, go%s", room_name (from), answer);
        answer = tmp;
      }
    }

    else if ((from = homerm) >= 0 &&
	     from != to &&
	     (answer =
	      find_path (from, to, is_newbie (name) ? SHORTPATH : LONGPATH)))
    { /* answer is complete */ 
      if (*answer == ' ')
      { sprintf (tmp, "From %s, go%s", room_name (from), answer);
        answer = tmp;
      }
    }

    else
    { sprintf (answer = abuf,
	       "Sorry{, n}, I don't know how to get to '%s'", room_name (to));
    }

    reply ("\"%s", answer);
    return (1);
  }

  /*---- Answer query about 'what exits lead to room' ----*/
  else if (!contest_mode &&
	  ((sindex (lcmsg, "what") || sindex (lcmsg, "which")) &&
	   (MATCH (lcmsg, "*exit*to *,*") ||
	    MATCH (lcmsg, "*exit*to *") ||
	    MATCH (lcmsg, "*link*to *,*") ||
	    MATCH (lcmsg, "*link*to *") ||
	    MATCH (lcmsg, "*door*to *,*") ||
	    MATCH (lcmsg, "*door*to *"))))
  { strcpy (speaker, name);
    spoke_player (name);
    exit_to_query (res3, name);
    return (1);
  }

  /*---- Answer query about 'what exits lead from room' ----*/
  else if (!contest_mode &&
	  ((MATCH (lcmsg, "*what*door*") ||
	    MATCH (lcmsg, "*what*exit*") ||
	    MATCH (lcmsg, "*what*link*") ||
	    MATCH (lcmsg, "*which*door*") ||
	    MATCH (lcmsg, "*which*exit*") ||
	    MATCH (lcmsg, "*which*link*") ||
	    MATCH (lcmsg, "*show*exit*") ||
	    MATCH (lcmsg, "*show*link*") ||
	    MATCH (lcmsg, "*show*door*") ||
	    MATCH (lcmsg, "*list*exit*") ||
	    MATCH (lcmsg, "*list*link*") ||
	    MATCH (lcmsg, "*how*do* i *leave*") ||
	    MATCH (lcmsg, "*how*do* i *get out*") ||
	    MATCH (lcmsg, "*list*door*")) &&
	   (MATCH (lcmsg, "* in *,*") ||
	    MATCH (lcmsg, "* in *") ||
	    MATCH (lcmsg, "* of *,*") ||
	    MATCH (lcmsg, "* of *") ||
	    MATCH (lcmsg, "*for *,*") ||
	    MATCH (lcmsg, "*for *") ||
	    MATCH (lcmsg, "*from *,*") ||
	    MATCH (lcmsg, "*from *") ||
	    MATCH (lcmsg, "*leave *,*") ||
	    MATCH (lcmsg, "*leave *"))))
  { strcpy (speaker, name);
    spoke_player (name);
    exit_from_query (res2, name);
    return (1);
  }

  /*---- Answer query about 'what exits lead here' ----*/
  else if (!contest_mode &&
	  ((sindex (lcmsg, "what") || sindex (lcmsg, "which")) &&
	   (MATCH (lcmsg, "*exit*lead here*") ||
	    MATCH (lcmsg, "*exit*go here*") ||
	    MATCH (lcmsg, "*exit*come here*") ||
	    MATCH (lcmsg, "*exit*to here*") ||
	    MATCH (lcmsg, "*link*lead here*") ||
	    MATCH (lcmsg, "*link*go here*") ||
	    MATCH (lcmsg, "*link*come here*") ||
	    MATCH (lcmsg, "*link*to here*") ||
	    MATCH (lcmsg, "*door*lead here*") ||
	    MATCH (lcmsg, "*door*go here*") ||
	    MATCH (lcmsg, "*door*come here*") ||
	    MATCH (lcmsg, "*door*to here*")) ||
	   (MATCH (lcmsg, "*how*get*to here"))))
  { strcpy (speaker, name);
    spoke_player (name);
    exit_to_query ("here", name);
    return (1);
  }

  /*---- Answer query about 'what exits leave here' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*list*exit*") ||
	   MATCH (lcmsg, "*show*exit*") ||
	   MATCH (lcmsg, "*, exits") ||
	   (sindex (lcmsg, "what") || sindex (lcmsg, "which")) &&
	   (MATCH (lcmsg, "*exits here*") ||
	    MATCH (lcmsg, "*exits please*") ||
	    MATCH (lcmsg, "*exit*are here*") ||
	    MATCH (lcmsg, "*exit*are there*") ||
	    MATCH (lcmsg, "*exit*is here*") ||
	    MATCH (lcmsg, "*exit*from here*") ||
	    MATCH (lcmsg, "*door*are here*") ||
	    MATCH (lcmsg, "*door*is here*") ||
	    MATCH (lcmsg, "*door*from here*")) ||
	   (MATCH (lcmsg, "where can*go from here*"))))
  { strcpy (speaker, name);
    spoke_player (name);
    exit_from_query ("here", name);
    return (1);
  }

  /*---- Tell robot to go on to her next task ----*/
  else if (MATCH (lcmsg, "*stop *hang*") ||
	   MATCH (lcmsg, "*go away*") ||
	   MATCH (lcmsg, "*you may go*") ||
	   MATCH (lcmsg, "*go*home*") ||
	   MATCH (lcmsg, "*take*a*hike*") ||
	   MATCH (lcmsg, "*ski*daddle*") ||
	   MATCH (lcmsg, "*go*hell*") ||
	   MATCH (lcmsg, "*go*back*explor*") ||
	   MATCH (lcmsg, "*go*back*busines*") ||
	   MATCH (lcmsg, "*leave*") &&
	     !(sindex (lcmsg, "what") ||
	       sindex (lcmsg, "which") ||
	       sindex (lcmsg, "how")) ||
	   MATCH (lcmsg, "*go *about*business*") ||
	   MATCH (lcmsg, "*be *about*business*"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (contest_mode)
    { switch (nrrint (179, 3))
      { case 0: zinger ("\"But I have to sit here and type{, }."); break;
        case 1: zinger ("\"Okay{, n}."); break;
        case 2: zinger ("\"Bye{, n}."); break;
      }
      return (1);
    }
    else if (isowner (name) && (trusting || *code && sindex (msg, code)) ||
	     generous || paging)
    { if (playinghearts)
      { reply ("\"But I am playing hearts, now{, n}."); }
      else if (takingnotes)
      { reply ("\"But I am taking notes, now{, n}."); }
      else
      { reply ("\"I'll be going, then{, n}.");
        hanging = 0;
	nextwait = 0;
      }
      speaktime = 0;
      return (1);
    }
    else
    { speaktime = 0;
      reply ("\"Why, am I bothering you{, n}?"); return (1);
    }
  }
  
  /*---- Obey command ----*/
  else if (!contest_mode &&
	  (((MATCH (lcmsg, "go *,*") ||
	     MATCH (lcmsg, "go *") ||
	     MATCH (lcmsg, "come *,*") && stlmatch (res1, "to ") ||
	     MATCH (lcmsg, "come *") && stlmatch (res1, "to ") ||
	     MATCH (lcmsg, "go <*>*") ||
	     (MATCH (lcmsg, "invites you to '*'.") ||
	      MATCH (lcmsg, "invites you to '*'") ||
	      MATCH (lcmsg, "invites you to '*") ||
	      MATCH (lcmsg, "invites you to *")) &&
		sprintf (res2, "to %s	", res1)) ||

	    MATCH (lcmsg, "* go *,*") ||
	    MATCH (lcmsg, "* go *") ||
	    MATCH (lcmsg, "* come *,*") && stlmatch (res2, "to ") ||
	    MATCH (lcmsg, "* come *") && stlmatch (res2, "to ") ||
	    MATCH (lcmsg, "* go <*>*") ||
	    MATCH (lcmsg, "*<go *>*")) &&
	   !streq (res2, "direction") &&
	   !sindex (res1, "what") &&
	   !sindex (res1, "which")))
  { register char *dest, *dir;
    char dirstr[MSGSIZ];
    long rm = 0;

    if (is_jerk (name)) return (1);

    strcpy (speaker, name);

    /* Clean up direction */
    dir = res2;
    while (index (" \t.`'\"", *dir)) dir++;

    strcpy (dirstr, dir);

    /* Strip robot name off of direction */
    if (ematch (dirstr, lcstr (myname)))
    { dir = dirstr + strlen (dirstr) - strlen (myname); }
    else
    { dir = dirstr + strlen (dirstr); }

    while (index (" \t,.:;`'\"", dir[-1])) dir--;
    *dir = '\0';

    /* Case where robot is told to go to a room */
    if (isowner (name) && (trusting || *code && sindex (msg, code)) ||
	paging && !takingnotes && !playinghearts && pagedto < 0 ||
	streq (pagedby, name))
    { if ((stlmatch (dirstr, "to the ") && (dest = dirstr + 7)) ||
	  (stlmatch (dirstr, "to ") && (dest = dirstr + 3)))
      { if ((rm = close_room (dest)) >= 0)
	{ dest = room[rm].name;
	  pagedto = rm;
	  pagedfrom = hererm;
	  strcpy (pagedby, "<COMMAND>");
	  pagedat = time (0);
	  hanging = 0; pgoal = 0;
	  nextwait = 60;
	  reply ("\"Okay, I'm heading for %s{, n}.", room_name (rm));

	  if (doing_works)
	  { command ("@doing heading to %s", room_name (pagedto)); }
	}
	else
	{ reply ("\"I've never heard of %s{, n}.", dest); }
      }
      
      /* Do not enter other players */
      else if (ismuck && randint (100) < 33 &&
	       MATCH (dirstr, "enter *") &&
	       find_player (res1) >= 0)
      { switch (nrrint (180, 4))
        { case 0: zinger ("\"But I'm claustrophobic{, n}."); break;
	  case 1: zinger ("\"I'd rust{, n}!"); break;
	  case 2: zinger ("\"I wouldn't fit in there{, n}!"); break;
	  case 3: zinger ("\"No way! You go first{, n}!"); break;
	}
	return (1);
      }

      else
      { sprintf (typecmd, "go %s", dirstr);
	hanging = 0; pgoal = 0; clear_page (); nextwait = 120;
	if (!terse) fprintf (stderr, "Set typecmd to '%s'\n", typecmd);
	if (!isowner (name))
	{ msgtype = M_UNKNOWN;
	  reply (":types <%s> as requested by %s.", typecmd, name); }
      }
    }

    else if (pagedto >= 0 &&
	     ((stlmatch (dirstr, "to the ") && (dest = dirstr + 7)) ||
	      (stlmatch (dirstr, "to ") && (dest = dirstr + 3))) &&
	     (close_room (dest) == pagedto))
    { if (pagedto == hererm)
      { reply ("\"I'm already there{, n}."); }
      else
      { reply ("\"I'm already on my way there{, n}."); }
    }

    else if (pagedto == hererm)
    { spoke_player (name);
      if (*pagedby && *pagedby != '<')
      { reply ("\"Sorry, I'm here to see %s right now{, n}.", pagedby); }
      else
      { reply ("\"Sorry, not right now{, n}."); }
    }

    else if (pagedto >= 0)
    { spoke_player (name);
      reply ("\"Sorry, I am on my way to %s right now{, n}.",
	      room_name (pagedto));
    }

    else if (playinghearts)
    { spoke_player (name);
      reply ("\"I am busy playing hearts right now{, n}.");
    }

    else if (takingnotes)
    { spoke_player (name);
      reply ("\"I am busy taking notes right now{, n}.");
    }

    else
    { spoke_player (name);
      reply ("\"I am only listening to %s right now{, n}.", owner);
    }

    return (1);
  }

  /*---- Query the number of pennies she has ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*how*many*cookies*") ||
	   MATCH (lcmsg, "*how*many*pennies*") ||
	   MATCH (lcmsg, "*how*much*money*")))
  { strcpy (speaker, name);
    spoke_player (name);

    if (isowner (name) && (trusting || *code && sindex (msg, code)) || generous)
    { if (streq (world, "ASYLUM"))
      { reply ("\"I now have %ld %s{, n}", pennies,
	       (pennies==1)?"cookie":"cookies");
      }
      else
      { reply ("\"I now have %ld penn%s{, n}", pennies,
	       (pennies==1)?"y":"ies");
      }
      return (1);
    }
    else
    { reply ("\"I have enough %s{, n}",
	     streq (world, "ASYLUM") ? "cookies" : "pennies");
      return (1);
    }
  }

  /*---- How many people ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*who*do*you*know*") ||
	   MATCH (lcmsg, "*how*many*players*") ||
	   MATCH (lcmsg, "*how*many*people*")))
  { strcpy (speaker, name);
    spoke_player (name);

    zinger ("\"I have seen at least %d different names on the WHO list{, n}",
	   players);
    return (1);
  }

  /*---- My email address is ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*my e-mail address is* <*>*") ||
	   MATCH (lcmsg, "*my email address is* <*>*") ||
	   MATCH (lcmsg, "*my* address is <*>*") ||
	   MATCH (lcmsg, "*my address is* <*>*") ||
	   MATCH (lcmsg, "*my *address is *")))
  { char ebuf[MSGSIZ];
    register char *s;

    if ((pl = add_player (name)) < 0) return (1);

    strcpy (speaker, name);
    spoke_player (name);    

    strcpy (ebuf, res3);
    
    if (strfoldeq (last (res3), myname))
    { s = ebuf + strlen (ebuf) - strlen (myname);
      while (isspace (s[-1]) || s[-1] == ',') s--;
      *s = '\0';
    }

    if (player[pl].email) freestring  (player[pl].email);
    player[pl].email = makestring (ebuf);

    if (msgtype < M_WHISPER) msgtype = M_WHISPER;
    reply ("\"Okay, %s, I set your email address to '%s'.",
	   player[pl].name, player[pl].email);

    return (1);
  }

  /*---- Ask about mail for a player ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*repeat*my mail*") && (s = name) ||
	   MATCH (lcmsg, "*repeat*messages*") && (s = name) ||
	   MATCH (lcmsg, "*give me*my mail*") && (s = name) ||
	   MATCH (lcmsg, "*give me*messages*") && (s = name) ||
	   MATCH (lcmsg, "*may* i *have *my mail*") && (s = name) ||
	   MATCH (lcmsg, "*may* i *have *messages*") && (s = name) ||
	   MATCH (lcmsg, "*can* i *have *my mail*") && (s = name) ||
	   MATCH (lcmsg, "*can* i *have *messages*") && (s = name) ||
	   MATCH (lcmsg, "*read my mail*") && (s = name) ||
	   MATCH (lcmsg, "*read my messages*") && (s = name) ||
	   MATCH (lcmsg, "*any mail, *") && (s = name) ||
	   MATCH (lcmsg, "*any mail") && (s = name) ||
	   MATCH (lcmsg, "*any messages, *") && (s = name) ||
	   MATCH (lcmsg, "*any messages") && (s = name) ||
	   MATCH (lcmsg, "*play* my* message*") && (s = name) ||
	   MATCH (lcmsg, "*want* my* message*") && (s = name) ||
	   MATCH (lcmsg, "*what* my* message*") && (s = name) ||
	   MATCH (lcmsg, "*what* my* mail*") && (s = name) ||
	   MATCH (lcmsg, "*any mail for *") && (s = car (res2)) ||
	   MATCH (lcmsg, "*a message for *") && (s = car (res2)) ||
	   MATCH (lcmsg, "*any message* for *") && (s = car (res3)) ||
	   MATCH (lcmsg, "*are there* message* for *") && (s = car (res4)) ||
	   MATCH (lcmsg, "*is there* mail* for *") && (s = car (res4)) ||
	   MATCH (lcmsg, "*does * have*message*") && (s = car (res2)) ||
	   MATCH (lcmsg, "*does * have*mail*") && (s = car (res2)) ||
	   MATCH (lcmsg, "*you*have*mail for *") && (s = car (res4)) ||
	   MATCH (lcmsg, "*you*have*message* for *") && (s = car (res5)) ||
	   MATCH (lcmsg, "*have*you*mail for *") && (s = car (res4)) ||
	   MATCH (lcmsg, "*have*you*message* for *") && (s = car (res5)) ||
	   MATCH (lcmsg, "*, mail") && is_me (res1) && (s = name) ||
	   MATCH (lcmsg, "mail, *") && is_me (res1) && (s = name) ||
	   MATCH (lcmsg, "mail") && (s = name) ||
	   MATCH (lcmsg, "*, messages") && is_me (res1) && (s = name) ||
	   MATCH (lcmsg, "messages, *") && is_me (res1) && (s = name) ||
	   MATCH (lcmsg, "messages") && (s = name)))
  { int cnt;

    strcpy (speaker, name);
    spoke_player (name);
    
    if ((pl = close_player (s, name)) >= 0)
    { if ((cnt = msg_count (pl)) > 0)
      { reply ("\"I have %d message%s for %s{, n}.",
	       cnt, cnt == 1 ? "" : "s",
	       strfoldeq (player[pl].name, name) ? "you" : player[pl].name);
      }
      else if (MATCH (lcmsg, "*repeat*") ||
	       MATCH (lcmsg, "*last*mail*") ||
	       MATCH (lcmsg, "*old*mail*") ||
	       MATCH (lcmsg, "*last*messages*") ||
	       MATCH (lcmsg, "*old*messages*"))
      { zinger ("\"Sorry{, n}, but I don't save copies of old messages."); }
      else
      { reply ("\"I have no messages for %s{, n}.",
		strfoldeq (player[pl].name, name) ? "you" : player[pl].name);
      }

      strcpy (speaker, name);
      
      if (PLAYER_GET(pl, PL_HAVEN))
      { reply ("\"%s was not accepting pages %s. When I next see %s, %s",
	       player[pl].name, "last time I checked", player[pl].name,
	       "I'll relay the message by whispering.");
      }

      /* If there are messages for the speaker, try to send them */
      if (cnt > 0 && strfoldeq (player[pl].name, name))
      { PLAYER_CLR(pl, PL_HAVEN);

	if (!give_msgs (pl))
	{ fprintf (stderr,
		   "Warn: have %d msgs for %s, but give_msgs(%d) loses\n",
		   cnt, s, pl);
	}
      }
    }
    else
    { reply ("\"I've never heard of player %s{, n}.", s); }

    return (1);
  }

  /*---- Ask about mail senders ----*/
  else if (!contest_mode &&
	  ((MATCH (lcmsg, "*who*sent*") && strcpy (tmp, res3) ||
	    MATCH (lcmsg, "*what*is*") && strcpy (tmp, res3) ||
	    MATCH (lcmsg, "*what*are*") && strcpy (tmp, res3) ||
	    MATCH (lcmsg, "*read me*") && strcpy (tmp, res2) ||
	    MATCH (lcmsg, "*tell me*") && strcpy (tmp, res2)) &&
	   (MATCH (tmp, "*mail*to*") ||
	    MATCH (tmp, "*mail*for*") ||
	    MATCH (tmp, "*message*to*") ||
	    MATCH (tmp, "*message*for*") ||
	    MATCH (tmp, "*msg*to*") ||
	    MATCH (tmp, "*msg*for*")) ||
	   MATCH (lcmsg, "*who*are*msg*from*") ||
	   MATCH (lcmsg, "*who*is*mail*from*") ||
	   MATCH (lcmsg, "*who*are*message*from*") ||
	   MATCH (lcmsg, "*who*is*message*from*")))
  { int cnt;

    strcpy (speaker, name);
    spoke_player (name);
    
    zinger ("\"Sorry{, n}, but I don't reveal the source or contents %s.",
	    "of a player's messages");

    return (1);
  }

  /*---- A message to another player ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*how*much*mail*") ||
	   MATCH (lcmsg, "*how*many*message*")))
  { strcpy (speaker, name);
    spoke_player (name);
    msg_total (name);
    return (1);
  }

  /*---- status ----*/
  else if (!contest_mode && (MATCH (lcmsg, "*status*")))
  { strcpy (speaker, name);
    spoke_player (name);

    if (playinghearts)
    { reply ("\"I am %splaying hearts in %s, %s",
	     quiet ? "quietly " : "", room_name (hererm), name);
    }

    else if (takingnotes)
    { reply ("\"I am %staking notes in %s, %s",
	     quiet ? "quietly " : "", room_name (meetingroom), name);
    }
    else if (exploring)
    { reply ("\"I'm just %slooking %s, with speed %ld, %s",
	     quiet ? "quietly " : "", 
	     usedesc ? "at room descriptions" : "around", speed, name);
    }
    else
    { reply ("\"I'm %sspending most of my time around home, %s",
   	     quiet ? "quietly " : "", name); }

    strcpy (speaker, name);
    reply ("\"I %s answer pages from most people.",
	   paging ? "will" : "won't");

    strcpy (speaker, name);
    reply ("\"I %s visit old rooms and I %s feel vindictive today.",
	   visitold ? "will" : "won't",
	   vindictive ? "do" : "don't");

    return (1);
  }

  /*---- Name only ----*/
  else if (strfoldeq (lcmsg, myname) ||
	   MATCH (lcmsg, "come here*") ||
	   MATCH (lcmsg, "*come here"))
  { strcpy (speaker, name);
    spoke_player (name);
    switch (nrrint (181, 3))
    { case 0: reply ("\"Yes{, n}?"); break;
      case 1: reply ("\"Right here{, n}."); break;
      case 2: reply (":nods to %s.", name); break;
    }
    return (1);
  }

  /*---- A query about 'how to quit' ----*/
  else if (MATCH (lcmsg, "*how* i *quit*") ||
	   MATCH (lcmsg, "*how* does *quit*") ||
	   MATCH (lcmsg, "*how* i *leave*game*") ||
	   MATCH (lcmsg, "*how* does *leave*") ||
	   MATCH (lcmsg, "*how* i *exit*") ||
	   MATCH (lcmsg, "*how* does *exit*") &&
		!(sindex (lcmsg, "link") ||
		  sindex (lcmsg, " use ") ||
		  sindex (lcmsg, " go ")) ||
	   MATCH (lcmsg, "*how* i *stop*play*") ||
	   MATCH (lcmsg, "*how* does *stop*play*"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"Type QUIT, using all caps{, n}.");
    return (1);
  }

  /*---- Can I ask you a question ----*/
  else if (MATCH (lcmsg, "*can* i *ask*question*") ||
	   MATCH (lcmsg, "*may* i *ask*question*") ||
	   MATCH (lcmsg, "*can* i *ask*something*") ||
	   MATCH (lcmsg, "*may* i *ask*something*"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"Sure{, n}, go ahead.");
    return (1);
  }

  /*---- A query about 'how to nod' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*how* i *nod*") ||
	   MATCH (lcmsg, "*how* does *nod*") ||
	   MATCH (lcmsg, "*how* do you *nod*") ||
	   MATCH (lcmsg, "*how* i *smile*") ||
	   MATCH (lcmsg, "*how* does *smile*") ||
	   MATCH (lcmsg, "*how* do you *smile*")))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"Use the ':' command{, n} (:nods, :smiles, etc.)");
    return (1);
  }

  /*---- A query about 'how to sacrifice' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*how* i *sacrifice*") ||
	   MATCH (lcmsg, "*how* does *sacrifice*") ||
	   MATCH (lcmsg, "*how* do you *sacrifice*")))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"To sacrifice an object, drop it in the temple{, n}.)");
    return (1);
  }

  /*---- A query about 'how to send message' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*how* give* message*") ||
	   MATCH (lcmsg, "*how* send* message*") ||
	   MATCH (lcmsg, "*how* leave* message*")))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"To send a message, type 'wh %s = %s' or 'page %s = %s', %s.",
	   myname, "tell <player> <message>",
	   myname, "tell <player> <message>", name);
    return (1);
  }

  /*---- A query about 'how to save' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*how* i *save*character*") ||
	   MATCH (lcmsg, "*how* does *save*character*") ||
	   MATCH (lcmsg, "*how* do you *save*character*")))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"Your %s, just go home and type QUIT, %s.",
	   "state is automatically saved", name);
    return (1);
  }

  /*---- A query about 'how to make money' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*how* i *make*money*") ||
	   MATCH (lcmsg, "*how* does *make*money*") ||
	   MATCH (lcmsg, "*how* do you *make*money*") ||
	   MATCH (lcmsg, "*how* i *get*penn*") ||
	   MATCH (lcmsg, "*how* does *get*penn*") ||
	   MATCH (lcmsg, "*how* do you *get*cookie*") ||
	   MATCH (lcmsg, "*how* i *get*cookie*") ||
	   MATCH (lcmsg, "*how* does *get*cookie*") ||
	   MATCH (lcmsg, "*how* do you *get*cookie*")))
  { strcpy (speaker, name);
    spoke_player (name);
    if (streq (world, "ASYLUM"))
    { reply ("\"You can get cookies by %s to the lost and found, %s.",
	     "returning things owned by other people", name);
    }
    else
    { reply ("\"You can get pennies by %s in the temple, %s.",
	     "sacrificing objects (owned by other people)", name);
    }
    return (1);
  }

  /*---- A query about addition ----*/
  else if ((MATCH (lcmsg, "*what is * + *") ||
	    MATCH (lcmsg, "*what's * + *") ||
	    MATCH (lcmsg, "*what is *+*") ||
	    MATCH (lcmsg, "*what's *+*") ||
	    MATCH (lcmsg, "*what are * and *") ||
	    MATCH (lcmsg, "*what is * plus *") ||
	    MATCH (lcmsg, "*what's * plus *")) &&
	   isnumber (res2) && isnumber (res3))
  { strcpy (speaker, name);
    spoke_player (name);
    zinger ("\"I think it's %d, %s.",
	   numberval (res2) + numberval (res3), name);
    return (1);
  }

  /*---- A query about subtraction ----*/
  else if ((MATCH (lcmsg, "*what is * - *") ||
	    MATCH (lcmsg, "*what's * - *") ||
	    MATCH (lcmsg, "*what is *-*") ||
	    MATCH (lcmsg, "*what's *-*") ||
	    MATCH (lcmsg, "*what is * minus *") ||
	    MATCH (lcmsg, "*what's * minus *")) &&
	   isnumber (res2) && isnumber (res3))
  { strcpy (speaker, name);
    spoke_player (name);
    zinger ("\"I think it's %d, %s.",
	   numberval (res2) - numberval (res3), name);
    return (1);
  }

  /*---- A query about multiplication ----*/
  else if ((MATCH (lcmsg, "*what is * times *") ||
	    MATCH (lcmsg, "*what is * x *") ||
	    MATCH (lcmsg, "*what's * x *") ||
	    MATCH (lcmsg, "*what is *x*") ||
	    MATCH (lcmsg, "*what's *x*") ||
	    MATCH (lcmsg, "*what's *") &&
		strcpy (res1, res2) &&
		(sscanf (res2, "%[0-9] * %[0-9]", res2, res3) ||
		 sscanf (res2, "%[0-9]*%[0-9]", res2, res3)) ||
	    MATCH (lcmsg, "*what is * times *") ||
	    MATCH (lcmsg, "*what's * times *")) &&
	   isnumber (res2) && isnumber (res3))
  { strcpy (speaker, name);
    spoke_player (name);
    zinger ("\"I think it's %d, %s.",
	   numberval (res2) * numberval (res3), name);
    return (1);
  }

  /*---- A query about division ----*/
  else if ((MATCH (lcmsg, "*what is * / *") ||
	    MATCH (lcmsg, "*what's * / *") ||
	    MATCH (lcmsg, "*what is */*") ||
	    MATCH (lcmsg, "*what's */*") ||
	    MATCH (lcmsg, "*what is * over *") ||
	    MATCH (lcmsg, "*what's * over *") ||
	    MATCH (lcmsg, "*what is * divided by *") ||
	    MATCH (lcmsg, "*what's * divided by *")) &&
	   isnumber (res2) && isnumber (res3))
  { strcpy (speaker, name);
    spoke_player (name);

    if (numberval (res3) == 0)
    { switch (nrrint (182, 4))
      { case 0: zinger (":dumps core."); break;
        case 1: zinger ("\"I think it's pretty big{, n}."); break;
        case 2: zinger (":tells %s \"%s\"",
			name, "You can't fool me with that one."); break;
        case 3: zinger ("\"I think it's %1.3lf, %s.",
			(double) randint (10001) / randint (10001), name);
		break;
      }
    } 
    else
    { zinger ("\"I think it's %1.3lf, %s.",
	      (double) numberval (res2) / numberval (res3), name);
    }
    return (1);
  }

  /*---- A query about 'how you compute route' ----*/
  else if (!contest_mode &&
	  (MATCH (lcmsg, "*how*you*compute*route*") ||
	   MATCH (lcmsg, "*how*you*find*route*") ||
	   MATCH (lcmsg, "*how*you*compute*path*") ||
	   MATCH (lcmsg, "*how*you*find*path*") ||
	   MATCH (lcmsg, "*how*you*find*your*way*")))
  { strcpy (speaker, name);
    spoke_player (name);

    { switch (nrrint (183, 4))
      { case 0: zinger ("\"I use a breadth first search."); break;
        case 1: zinger (":uses a breadth first search."); break;
        case 2: zinger ("\"I use a shortest path algorithm."); break;
        case 3: zinger (":uses a shortest path algorithm."); break;
      }
    } 

    return (1);
  }

  /*---- A query about 'how you' ----*/
  else if (!contest_mode &&
	   ((sindex (lcmsg, "can i ") ||
	     sindex (lcmsg, "can one ") ||
	     sindex (lcmsg, "how may ") ||
	     (randint (100) < 50 &&
	      (sindex (lcmsg, "how can ") ||
	       sindex (lcmsg, "how do i ") ||
	       sindex (lcmsg, "how do you ") &&
		  !sindex (lcmsg, "you do") &&
		  !sindex (lcmsg, "feel"))) ||
	     sindex (lcmsg, "how does one ") ||
	     sindex (lcmsg, "is it possible ")) ||
	    MATCH (lcmsg, "*you*know*how*") ||
	    MATCH (lcmsg, "*you*knew*how*") ||
	    MATCH (lcmsg, "*how *@*") ||
	    MATCH (lcmsg, "*what *@*")))
  { strcpy (speaker, name);
    spoke_player (name);
    give_general_help (lcmsg, name);
    return (1);
  }

  /*---- A query about 'what time' ----*/
  else if (MATCH (lcmsg, "*what*time*") &&
	   !sindex (lcmsg, "trav") &&
	   !sindex (lcmsg, "times"))
  { long now = time (0);
    strcpy (speaker, name);
    spoke_player (name);
    reply ("\"I have %5.5s (eastern){, n}.", ctime (&now)+11);
    return (1);
  }

  /* Message not matched by this rule set */
  return (0);
}
