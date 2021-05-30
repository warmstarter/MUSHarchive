/****************************************************************
 * reply.c: Common responses for most TinyMUD automata
 *
 * HISTORY
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

/****************************************************************
 * standard_msg: Handle most queries common to most robots
 *	Standard_msg was split into 2 parts because it was
 *	getting too big for some C compilers.
 ****************************************************************/

standard_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{
  return (simple_std_msg (name, msg, lcmsg) ||
	  complex_standard_msg (name, msg, lcmsg));
}

simple_std_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ long pl, rm;
  long full = 0;
  register long i;

  /*---- Turn on debugging ----*/
  if (MATCH (lcmsg, "*debug*on*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { debug++;
      spoke_player (name);
      reply ("\"Debugging is now on, %s", name);
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Turn off debugging ----*/
  else if (MATCH (lcmsg, "*debug*off*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { debug = 0;
      spoke_player (name);
      reply ("\"Debugging is now off, %s", name);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Turn on paging ----*/
  else if (MATCH (lcmsg, "*paging*on*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { paging++;
      spoke_player (name);
      reply ("\"Paging is now on, %s", name);
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Turn off paging ----*/
  else if (MATCH (lcmsg, "*paging*off*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { paging = 0;
      spoke_player (name);
      reply ("\"Paging is now off, %s", name);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Turn on quiet ----*/
  else if (MATCH (lcmsg, "*quiet*on*") ||
	   MATCH (lcmsg, "*nois*off*"))
  { strcpy (speaker, name);
    quiet++;
    spoke_player (name);
    reply ("\"I will be more quiet, %s", name);
    return (1);
  }

  /*---- Turn off quiet ----*/
  else if (MATCH (lcmsg, "*quiet*off*") ||
	   MATCH (lcmsg, "*nois*on*"))
  { strcpy (speaker, name);
    quiet = 0;
    spoke_player (name);
    reply ("\"quiet is now off, %s", name);
    return (1);
  }

  /*---- Turn off usedesc ----*/
  else if (MATCH (lcmsg, "*use*desc*off*") ||
	   MATCH (lcmsg, "*don't*use*desc*") ||
	   MATCH (lcmsg, "*do not*use*desc*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { usedesc = 0;
      spoke_player (name);
      reply ("\"I will not use descriptions to find exits, %s", name);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Turn on usedesc ----*/
  else if (MATCH (lcmsg, "*use*desc*on*") ||
	   MATCH (lcmsg, "*use*descrip*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { usedesc++;
      spoke_player (name);
      reply ("\"I will now use descriptions to find exits, %s", name);
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Turn on terse ----*/
  else if (MATCH (lcmsg, "*terse*on*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { terse++;
      spoke_player (name);
      reply ("\"I am now terse, %s", name);
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Turn off terse ----*/
  else if (MATCH (lcmsg, "*terse*off*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { terse = 0;
      spoke_player (name);
      reply ("\"I am not terse, %s", name);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Set speed ----*/
  else if (MATCH (lcmsg, "*faster*") &&
	   !(sindex (lcmsg, "what") ||
	     sindex (lcmsg, "how") ||
	     sindex (lcmsg, "why")))
  { strcpy (speaker, name);
    if (isowner (name))
    { if (speed > 1) speed--;
      spoke_player (name);
      reply ("\"Speed is now set at %ld seconds between moves, %s",
		speed, name);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Set speed ----*/
  else if (MATCH (lcmsg, "*slower*") &&
	   !(sindex (lcmsg, "what") ||
	     sindex (lcmsg, "how") ||
	     sindex (lcmsg, "why")))
  { strcpy (speaker, name);
    if (isowner (name))
    { speed++;
      spoke_player (name);
      reply ("\"Speed is now set at %ld seconds between moves, %s",
		speed, name);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Set speed ----*/
  else if (MATCH (lcmsg, "*what*you*speed*") ||
	   MATCH (lcmsg, "*how*fast*you*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { spoke_player (name);
      reply ("\"Speed is now set at %ld seconds between moves, %s",
		speed, name);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me, %s?", name);
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
    if (isowner (name))
    { if (isdigit (*res3)) speed = atoi (res3);
      if (speed < 1) speed = 1;
      spoke_player (name);
      reply ("\"Speed is now set at %ld seconds between moves, %s",
		speed, name);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me, %s?", name);
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
    reply ("\"Maas-Neotek, %s, %s.", VERSION, name);
    return (1);
  }

  /*---- Stop ignoring player ----*/
  else if ((MATCH (lcmsg, "*stop*ignoring *,*") ||
	    MATCH (lcmsg, "*stop*ignoring *") ||
	    MATCH (lcmsg, "*don't*ignore *,*") ||
	    MATCH (lcmsg, "*don't*ignore *") ||
	    MATCH (lcmsg, "*do not*ignore *,*") ||
	    MATCH (lcmsg, "*do not*ignore *") ||
	    MATCH (lcmsg, "*listen*to *,*") ||
	    MATCH (lcmsg, "*listen*to *")) &&
	   (pl = find_player (res3)) >= 0)
  { static long lastign = 0;

    strcpy (speaker, name);

    if (!PLAYER_GET (pl, PL_JERK))
    { if (isowner (name) || randint (10) < (now - lastign))
      { reply (":is already listening to %s, %s.", player[pl].name, name);
        lastign = now;
      }
    }
    else if (isowner (name))
    { PLAYER_SET (pl, PL_JERK);
      reply ("\"Listening to player %s, %s.", player[pl].name, name);
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
  else if ((MATCH (lcmsg, "*ignore *,*") ||
	    MATCH (lcmsg, "*ignore *")) &&
	   !streq (res2, "fuzzy") &&
	   ((pl = find_player (res2)) >= 0 ||
	    (!index (res2, ' ') && (pl = add_player (res2)) >= 0)))
  { static long lastign = 0;

    strcpy (speaker, name);

    if (PLAYER_GET (pl, PL_JERK))
    { if (isowner (name) || randint (10) < (now - lastign))
      { reply (":is already ignoring %s, %s.", player[pl].name, name);
        lastign = now;
      }
    }
    else if (isowner (name))
    { PLAYER_SET (pl, PL_JERK);
      reply ("\"Ignoring player %s, %s.", player[pl].name, name);
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

    if (isowner (name))
    { long rm = close_room (res4);

      if (rm < 0)
      { reply ("\"I've never heard of %s, %s.", res4, name); }
      else
      { takingnotes++;
	hanging = 0;
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
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Turn off takingnotes ----*/
  else if (MATCH (lcmsg, "*tak*note*off*") ||
	   MATCH (lcmsg, "*stop*tak*note*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { takingnotes = 0;
      meetingroom = -1;
      spoke_player (name);
      reply ("\"I am not taking notes, %s", name);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Checkpoint files ----*/
  else if (MATCH (lcmsg, "*checkpoint*"))
  { strcpy (speaker, name);
    spoke_player (name);
    checkpoint ();
    reply ("\"Ok, I saved my map and phone book, %s.", name);
    return (1);
  }

  /*---- Turn on exploring ----*/
  else if (MATCH (lcmsg, "*explor*on*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { exploring++;
      spoke_player (name);
      reply ("\"exploring is now on, %s", name);
      return (1);
    }
    else
    {
      spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Turn off exploring ----*/
  else if (MATCH (lcmsg, "*explor*off*"))
  { strcpy (speaker, name);
    if (isowner (name))
    { exploring = 0;
      spoke_player (name);
      reply ("\"exploring is now off, %s", name);
      return (1);
    }
    else
    { spoke_player (name);
      reply ("\"Excuse me, %s?", name);
      return (1);
    }
  }

  /*---- Obey command ----*/
  else if (MATCH (lcmsg, "*type <*>*") &&
	   !streq (res2, "go direction"))
  { strcpy (speaker, name);

    if (is_jerk (name)) return (1);
  
    if (isowner (name) ||
	stlmatch (res2, "go ") && paging && !takingnotes && pagedto < 0)
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
      reply ("\"I am busy taking notes right now, %s.", name);
    }
    else if (paging)
    { spoke_player (name);
      reply ("\"I only obey 'go' commands, try 'type <go dir>', %s.",
	      name);
    }
    else
    { spoke_player (name);
      reply ("\"I am only listening to %s right now, %s.", owner, name);
    }

    return (1);
  }

  /*---- A query about 'who belong' ----*/
  if (MATCH (lcmsg, "*who*you*belong*") ||
      MATCH (lcmsg, "*whose*are*you*") ||
      MATCH (lcmsg, "*who*you*owner*") ||
      MATCH (lcmsg, "*who*work*") ||
      MATCH (lcmsg, "*who*own*you*"))
  { reply ("\"I work mainly for %s, %s.", owner, name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- A query about 'who created' ----*/
  if (MATCH (lcmsg, "*who*built*you*") ||
      MATCH (lcmsg, "*who*made*you*") ||
      MATCH (lcmsg, "*who*created*you*") ||
      MATCH (lcmsg, "*who*wrote*"))
  { reply ("\"Fuzzy created me, but I belong to %s, %s.", owner, name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Who is here ----*/
  else if (MATCH (lcmsg, "*who is*sleep*") ||
	   MATCH (lcmsg, "*who's*sleep*"))
  { strcpy (speaker, name);
    spoke_player (name);
    asleep_query (name);
    return (1);
  }    

  /*---- Who is here ----*/
  else if (MATCH (lcmsg, "*who is* here*") ||
	   MATCH (lcmsg, "*who's* here*") ||
	   MATCH (lcmsg, "*who is* awake*") ||
	   MATCH (lcmsg, "*who's* awake*"))
  { strcpy (speaker, name);
    spoke_player (name);
    awake_query (name);
    return (1);
  }    

  /*---- Who is connected ----*/
  else if (MATCH (lcmsg, "*who is* connect*") ||
	   MATCH (lcmsg, "*who's* connect*") ||
	   MATCH (lcmsg, "*who is* log*") ||
	   MATCH (lcmsg, "*who's* log*"))
  { strcpy (speaker, name);
    spoke_player (name);
    connected_query (name);
    return (1);
  }    

  /*---- Reset Robot ----*/
  else if (isowner (name) &&
	   MATCH (lcmsg, "*reset*yourself*") &&
	   sindex (lcmsg, lcstr (myname)))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"Resetting, %s.", name);
    reset_robot (name);	/* Wont return */
    return (1);
  }

  /*---- Write reachable room map ----*/
  else if (isowner (name) &&
	   (MATCH (lcmsg, "*write*small*map*") ||
	    MATCH (lcmsg, "*write*small*room*") ||
	    MATCH (lcmsg, "*dump*small*map*") ||
	    MATCH (lcmsg, "*dump*small*room*")))
  { strcpy (speaker, name);
    spoke_player (name);
    write_map ("reach.map", 2);
    reply ("\"I dumped it to 'reach.map', %s", name);
    return (1);
  }

  /*---- Write reachable room map ----*/
  else if (isowner (name) &&
	   (MATCH (lcmsg, "*write*reach*map*") ||
	    MATCH (lcmsg, "*write*reach*room*") ||
	    MATCH (lcmsg, "*dump*reach*map*") ||
	    MATCH (lcmsg, "*dump*reach*room*")))
  { strcpy (speaker, name);
    spoke_player (name);
    write_map ("reach.map", 1);
    reply ("\"I dumped it to 'reach.map', %s", name);
    return (1);
  }

  /*---- Dump map ----*/
  else if (MATCH (lcmsg, "*dump*") && isowner (name))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"I dumped it to stderr, %s", name);
    return (1);
  }

  /*---- Memory Usage ----*/
  else if (MATCH (lcmsg, "*what*you*memory*") ||
	   MATCH (lcmsg, "*how*much*memory*you*") ||
	   MATCH (lcmsg, "*how*big*you*memory*"))
  { strcpy (speaker, name);
    spoke_player (name);
    reply ("\"Here is a summary of my memory usage, %s:", name);
    reply (": %ld bytes for %ld strings", string_sp, string_ct);
    reply (": %ld bytes for %ld exits", exit_sp, exit_ct);
    reply (": %ld bytes for rooms", room_sp);
    reply (": %ld bytes for paths", path_sp);
    reply (": %ld bytes for players", player_sp);
    reply (": %ld bytes for %ld dialog entries", dialog_sp, dialog_ct);
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
    if (isowner (name) || !generous)
    { generous = 0;
      reply ("\"I'm not feeling generous, anymore, %s", name);
      return (1);
    }
    else
    { reply ("\"But I want to be generous, %s.", name); return (1); }
  }

  /*---- Turn on generosity ----*/
  else if (MATCH (lcmsg, "*genero*on*") ||
	   MATCH (lcmsg, "*on*genero*") ||
	   MATCH (lcmsg, "*be*genero*") ||
	   MATCH (lcmsg, "*feel*genero*"))
  { strcpy (speaker, name);
    spoke_player (name);
    if (isowner (name) || generous)
    { generous++;
      reply ("\"I'm in a generous mood, %s.", name);
      return (1);
    }
    else
    { reply ("\"But I feel really grouchy, today, %s.", name); return (1); }
  }

  /*---- Hand over the pennies ----*/
  else if (MATCH (lcmsg, "*give*penn*") ||
	   MATCH (lcmsg, "*give*money*") ||
	   MATCH (lcmsg, "*loan*penn*") ||
	   MATCH (lcmsg, "*loan*money*") ||
	   MATCH (lcmsg, "*may*i*have*penn*") ||
	   MATCH (lcmsg, "*may*i*have*money*") ||
	   MATCH (lcmsg, "*can*i*have*penn*") ||
	   MATCH (lcmsg, "*can*i*have*money*") ||
	   MATCH (lcmsg, "*could*i*have*penn*") ||
	   MATCH (lcmsg, "*could*i*have*money*") ||
	   MATCH (lcmsg, "*i*need*penn*") ||
	   MATCH (lcmsg, "*i*need*penn*") ||
	   MATCH (lcmsg, "*i*borrow*penn*") ||
	   MATCH (lcmsg, "*i*borrow*money*") ||
	   MATCH (lcmsg, "*hand*over*penn*") ||
	   MATCH (lcmsg, "*hand*over*money*"))
  { static long givepl = -1;
    static long givetime = 0;
    long pl = add_player (name);
    long clock = time (0);

    strcpy (speaker, name);
    spoke_player (name);
    
    if (sindex (lcmsg, "don't") || sindex (lcmsg, "do not"))
    { reply ("\"Okay, %s, I won't.", name);
      return (1);
    }

    if (isowner (name) || generous || pennies > MAXPEN)
    { if (pennies > 0)
      { long amount;
	static long lastgive = 0;

	if (isowner (name))
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
	    reply ("\"I just gave you pennies %s ago, %s.",
		   time_dur (clock - player[pl].lastgave), name);
	    return (1);
	  }
	  else if (lastgive + 300 < clock)
	  { amount = randint ((pennies - MINPEN) / (polite ? 2 : 4)); }
	  else
	  { amount = randint ((pennies - MINPEN) / (polite ? 4 : 8)); }

	  if ((pennies - amount) < MINPEN)
	      { amount = pennies - MINPEN; }

	  if (amount > (polite ? 200 : 100))
	  { if (polite)	amount = randint (100) + 100;
	    else		amount = randint (50) + 50;
	  }

	  if (streq (name, "Igor")) amount = 0;

	  /* Robot specific changes */
	  if (amount > 0)
	  { amount = amount_hook (amount, name, polite);
	    if (amount == 0) return (1);
	  }

	  /* Handle case where no pennies to give */
	  if (amount > 0)
	  { lastgive = clock; }
	  else
	  { reply ("\"I just don't have any extra pennies, %s.",
		     name);
	    return (1);
	  }
	}

	if (amount > 0)
	{ pennies -= amount;
	  gave_player (name, amount);
	  givepl = pl;
	  givetime = clock;
	  reply ("give %s=%ld", name, amount);
	  command (":gives %s %ld penn%s.",
		   name, amount, (amount==1) ? "y" : "ies");
	  psynch ();

	  return (1);
	}
      }
      else
      { reply ("\"I have no pennies, %s.", name); return (1); }
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
    { reply ("\"Try again a little latter, %s.", name);
      givepl = pl;
      givetime = clock;
    }
    else
    { reply ("\"I'd rather not, %s.", name);
      givepl = pl;
      givetime = clock;
    }

    return (1);
  }

  /*---- An order to say something to someone ----*/
  else if (MATCH (lcmsg, "say * to *,*") ||
	   MATCH (lcmsg, "say * to *"))
  { strcpy (speaker, name);
    if (isowner (name) || generous || randint (100) < 70)
    { reply ("\"%s, %s", res1, res2); return (1); }
    else
    { reply ("\"Why, %s?", name); return (1); }
  }

  /*---- An order to say something ----*/
  else if (MATCH (lcmsg, "say *,*") ||
	   MATCH (lcmsg, "say *"))
  { strcpy (speaker, name);
    spoke_player (name);
    if (isowner (name) || generous || randint (100) < 40)
    { reply ("\"%s", res1); return (1); }
    else
    { reply ("\"Why, %s?", name); return (1); }
  }

  /*---- Who is the target ----*/
  else if (MATCH (lcmsg, "*who*is*target*") ||
	   MATCH (lcmsg, "*who*terminator*after*") ||
	   MATCH (lcmsg, "*who*terminator*seek*") ||
	   MATCH (lcmsg, "*who*terminator*search*") ||
	   MATCH (lcmsg, "*who*terminator*look*") ||
	   MATCH (lcmsg, "*who*terminator*vict*") ||
	   MATCH (lcmsg, "*who*is*contract*") ||
	   MATCH (lcmsg, "*what*is*contract*") ||
	   MATCH (lcmsg, "*who*terminator*hit*") ||
	   MATCH (lcmsg, "*who*terminator*kill*"))
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
    { reply ("\"I don't know who Terminator is looking for, %s.", name); }

    return (1);
  }

  /*---- A query about 'what rooms seen' ----*/
  else if (MATCH (lcmsg, "*how*many*room*explor*"))
  { strcpy (speaker, name);
    spoke_player (name);
    if (lastrm)
    { reply ("\"I %s %ld of the %ld rooms %s, %s, and have tried %ld exits.",
	     "have explored", 
	      lastrm, rooms, "I have been to", name, exits);
    }
    else
    { reply ("\"I have been to %ld rooms, %s.", rooms, name); }
    return (1);
  }

  /*---- A query about 'what rooms are reachable' ----*/
  else if (MATCH (lcmsg, "*how*many*room*reach*"))
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
  else if ((sindex (lcmsg, "what") || sindex (lcmsg, "which")) &&
	   (MATCH (lcmsg, "*room*match \"*\"*") ||
	    MATCH (lcmsg, "*room*match '*'*") ||
	    MATCH (lcmsg, "*room*match <*>*") ||
	    MATCH (lcmsg, "*room*match *,*") ||
	    MATCH (lcmsg, "*room*match *")))
  { strcpy (speaker, name);
    if (sindex (res2, "desc"))
    { room_query (res3, name, 1); }
    else
    { room_query (res3, name, 0); }
    spoke_player (name);
    return (1);
  }

  /*---- A query about 'what rooms seen' ----*/
  else if (MATCH (lcmsg, "*what*room*discover*") ||
	   MATCH (lcmsg, "*what*rooms*found*") ||
	   MATCH (lcmsg, "*what*rooms*been*") ||
	   MATCH (lcmsg, "*what*rooms*seen*") ||
	   MATCH (lcmsg, "*what*rooms*heard*") ||
	   MATCH (lcmsg, "*what*rooms*told*") ||
	   MATCH (lcmsg, "*what*rooms*told*") ||
	   MATCH (lcmsg, "*what*rooms*tell*") ||
	   MATCH (lcmsg, "*how*many*rooms*") ||
	   MATCH (lcmsg, "*which*rooms*seen*") ||
	   MATCH (lcmsg, "*whichrooms*been*") ||
	   MATCH (lcmsg, "*where*been*"))
  { char buf[BIGBUF];
    long rm;

    strcpy (speaker, name);
    spoke_player (name);

    sprintf (buf, "I have been to %ld rooms altogether, %s %s:",
	     rooms, "here are the last few,", name);
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
  else if (MATCH (lcmsg, "*where*you*live*"))
  { long now = time (0);
    strcpy (speaker, name);
    spoke_player (name);
    reply ("\"My home room is %s, %s.", home, name);
    return (1);
  }

  /*---- A query about 'what time' ----*/
  else if (MATCH (lcmsg, "*what*time*"))
  { long now = time (0);
    strcpy (speaker, name);
    spoke_player (name);
    reply ("\"I have %5.5s (eastern), %s.", ctime (&now)+11, name);
    return (1);
  }

  /*---- A query about 'what date' ----*/
  else if (MATCH (lcmsg, "*what*date*") ||
	   MATCH (lcmsg, "*what*day*"))
  { long now = time (0);
    strcpy (speaker, name);
    spoke_player (name);
    reply ("\"The date is %19.19s, %s.", ctime (&now), name);
    return (1);
  }

  /*---- A query about 'who recently' ----*/
  else if (MATCH (lcmsg, "*who*seen*") ||
	   MATCH (lcmsg, "*who*have*") ||
	   MATCH (lcmsg, "*who*around*"))
  { strcpy (speaker, name);
    spoke_player (name);
    all_player_query (4200, name);
    return (1);
  }

  /*---- A query about 'who' ----*/
  else if (((MATCH (lcmsg, "*where*is *,*") ||
	     MATCH (lcmsg, "*where*is *") ||
	     MATCH (lcmsg, "*where's *,*") ||
	     MATCH (lcmsg, "*where's *") ||
	     MATCH (lcmsg, "*you know*where * is*") ||
	     MATCH (lcmsg, "*you know*where * was*")) &&
	    (pl = find_player (res3)) >= 0) ||
	    MATCH (lcmsg, "*have you*seen *,*") ||
	    MATCH (lcmsg, "*have you*seen *"))
  { pl = find_player (res3);
    if (debug) fprintf (stderr, "Player query: %s(%ld)\n", res3, pl);
    strcpy (speaker, name);
    spoke_player (name);

    if (pl < 0)
    { reply ("\"I have never seen player %s, %s.", res3, name); }
    else if (streq (res3, lcstr (myname)))
    { reply ("\"I'm right here in %s, %s.", here, name); }
    else
    { player_query (pl, name); }
    return (1);
  }

  /*---- A query about 'who' ----*/
  else if ((MATCH (lcmsg, "*who is the *,*") ||
	    MATCH (lcmsg, "* who the * is*") ||
	    MATCH (lcmsg, "*who is the *") ||
	    MATCH (lcmsg, "*who is *,*") ||
	    MATCH (lcmsg, "* who * is*") ||
	    MATCH (lcmsg, "*who is *,*") ||
	    MATCH (lcmsg, "*who is *") ||
	    MATCH (lcmsg, "*number is *,*") ||
	    MATCH (lcmsg, "*number is *") ||
	    MATCH (lcmsg, "* old is *,*") ||
	    MATCH (lcmsg, "* old is *") ||
	    MATCH (lcmsg, "* age is *,*") ||
	    MATCH (lcmsg, "* age is *") ||
	    MATCH (lcmsg, "*number does * have*") ||
	    MATCH (lcmsg, "* id does * have*") ||
	    ((MATCH (lcmsg, "*describe the *,*") ||
	      MATCH (lcmsg, "*describe *,*") ||
	      MATCH (lcmsg, "*describe *") ||
	      MATCH (lcmsg, "*tell*about *") ||
	      MATCH (lcmsg, "*tell*about *,*")) &&
	      find_player (res2) >= 0) ||
	    MATCH (lcmsg, "*who am i*") && strcpy (res2, lcstr (name))) &&

	   !index (res2, ' '))
  { char pers[MSGSIZ];

    strcpy (pers, res2);

    strcpy (speaker, name);
    spoke_player (name);
    who_is (pers, name);
      return (1);
  }

  /*---- Who has X ----*/
  else if (MATCH (lcmsg, "*who *has <*>*") ||
	   MATCH (lcmsg, "*who *has *,*") ||
	   MATCH (lcmsg, "*who *has *") ||
	   MATCH (lcmsg, "*who *matches <*>*") ||
	   MATCH (lcmsg, "*who *matches *,*") ||
	   MATCH (lcmsg, "*who *matches *") ||
	   MATCH (lcmsg, "*who *carrying <*>*") ||
	   MATCH (lcmsg, "*who *carrying *,*") ||
	   MATCH (lcmsg, "*who *carrying *") ||
	   MATCH (lcmsg, "*who *carries <*>*") ||
	   MATCH (lcmsg, "*who *carries *,*") ||
	   MATCH (lcmsg, "*who *carries *"))
  { if (stlmatch (res3, "the "))	object_query (res3+4, name);
    else if (stlmatch (res3, "a "))	object_query (res3+2, name);
    else if (stlmatch (res3, "an "))	object_query (res3+3, name);
    else				object_query (res3, name);

    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Where are you ----*/
  else if (MATCH (lcmsg, "*describe the *,*") &&
		(rm = close_room (res2)) >= -1 ||
	   MATCH (lcmsg, "*describe the *") &&
		(rm = close_room (res2)) >= -1 ||
	   MATCH (lcmsg, "*describe *,*") &&
		(rm = close_room (res2)) >= 0 ||
	   MATCH (lcmsg, "*describe *") &&
		(rm = close_room (res2)) >= 0)
  { strcpy (speaker, name);
    spoke_player (name);

    if (rm < 0)
    { reply ("\"I've never heard of '%s', %s.", res2);
      return (1);
    }

    reply ("\"Here's what I know about %s, as of %s ago, %s:",
	   room_name (rm), time_dur (now-room[rm].lastin), name);

    if (!streq (room[rm].name, room_name (rm)))
    { reply (": full name: %s", room[rm].name); }

    if (room[rm].desc && room[rm].desc[0])
    { reply (": description: %s", room[rm].desc); }

    if (room[rm].contents && room[rm].contents[0])
    { reply (": contents: %s", room[rm].contents); }

    return (1);
  }

  /*---- Where are you ----*/
  else if (MATCH (lcmsg, "*where*are* you,*") ||
	   MATCH (lcmsg, "*where*are* you") ||
	   MATCH (lcmsg, "*where* you are,*") ||
	   MATCH (lcmsg, "*where* you are"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (*here)
    { reply ("\"I think I am in \"%s\"", here); return (1); }
    else
    { reply ("\"I do not know."); return (1); }
  }

  /*---- Where are we ----*/
  else if (MATCH (lcmsg, "*where*are* we,*") ||
	   MATCH (lcmsg, "*where*are* we") ||
	   MATCH (lcmsg, "*where*am *"))
  { strcpy (speaker, name);
    spoke_player (name);

    if (*here)
    { reply ("\"I think we are in \"%s\"", here); return (1); }
    else
    { reply ("\"I do not know."); return (1); }
  }

  /* Message not matched by this rule set */
  return (0);
}

complex_standard_msg (name, msg, lcmsg)
char *name, *msg, *lcmsg;
{ long pl, rm;
  long full = 0;
  register long i;


  /*---- Answer query about 'how do I get from X to Y' ----*/
  if ((MATCH (lcmsg, "*how*get*") ||
	    MATCH (lcmsg, "*how*travel*") ||
	    MATCH (lcmsg, "*how*go*")) &&

	   (MATCH (msg, "* from the * to the *,*") ||
	    MATCH (msg, "* from the * to the * *") && is_me (res4) ||
	    MATCH (msg, "* from * to the *,*") ||
	    MATCH (msg, "* from * to the * *") && is_me (res4) ||
	    MATCH (msg, "* from * to the *") ||

	    MATCH (msg, "* from the * to *,*") ||
	    MATCH (msg, "* from the * to * *") && is_me (res4) ||
	    MATCH (msg, "* from the * to *") ||
	    MATCH (msg, "* from * to *,*") ||
	    MATCH (msg, "* from * to * *") && is_me (res4)||
	    MATCH (msg, "* from * to *")))
  { char src[MSGSIZ], dest[MSGSIZ], *answer, abuf[BIGBUF];
    long from, to;

    answer = abuf;

    strcpy (dest, res3);
    strcpy (src, res2);

    if ((to = close_room (dest)) < 0)
    { sprintf (abuf, "I've never been to '%s', %s.", dest, name); }

    else if ((from = close_room (src)) < 0)
    { sprintf (abuf, "I've never been to '%s', %s.", src, name); }

    else if (to == from)
    { sprintf (answer = abuf, "Sit still, I guess, %s.", name); }

    else if ((answer = find_path (from, to, LONGPATH)))
    { strcpy (pathto, room[to].name); }

    else
    { sprintf (answer = abuf,
	  "Sorry, %s, I don't know how to get to %s from %s.",
	       name, room_name (to), room_name (from));
    }

    strcpy (speaker, name);
    spoke_player (name);
    reply ("\"%s", answer);
    return (1);
  }

  /*---- Answer query about 'how do I get to X from Y' ----*/
  else if ((MATCH (lcmsg, "*how*get*") ||
	    MATCH (lcmsg, "*how*travel*") ||
	    MATCH (lcmsg, "*how*go*")) &&

	   (MATCH (msg, "* to the * from the *,*") ||
	    MATCH (msg, "* to the * from the * *") && is_me (res4) ||
	    MATCH (msg, "* to the * from the *") ||
	    MATCH (msg, "* to * from the *,*") ||
	    MATCH (msg, "* to * from the * *") && is_me (res4) ||
	    MATCH (msg, "* to * from the *") ||

	    MATCH (msg, "* to the * from *,*") ||
	    MATCH (msg, "* to the * from * *") && is_me (res4) ||
	    MATCH (msg, "* to the * from *") ||
	    MATCH (msg, "* to * from *,*") ||
	    MATCH (msg, "* to * from * *") && is_me (res4) ||
	    MATCH (msg, "* to * from *")))
  { char src[MSGSIZ], dest[MSGSIZ], *answer, abuf[BIGBUF];
    long from, to;

    answer = abuf;

    strcpy (dest, res2);
    strcpy (src, res3);

    if ((to = close_room (dest)) < 0)
    { sprintf (abuf, "I've never been to %s, %s.", dest, name); }

    else if ((from = close_room (src)) < 0)
    { sprintf (abuf, "I've never been to %s, %s.", src, name); }

    else if (to == from)
    { sprintf (answer = abuf, "Sit still, I guess, %s.", name); }

    else if (answer = find_path (from, to, LONGPATH))
    { strcpy (pathto, room[to].name); }

    else
    { sprintf (answer = abuf,
	       "Sorry, %s, I don't know how to get to %s from %s.",
	       name, room_name (to) , room_name (from));
    }

    strcpy (speaker, name);
    spoke_player (name);
    reply ("\"%s", answer);
    return (1);
  }

  /*---- Answer query about 'how do I get to X' ----*/
  else if ((MATCH (lcmsg, "*where*is the *,*") ||
	    MATCH (lcmsg, "*where*is the *") ||
	    MATCH (lcmsg, "*where*are the *,*") ||
	    MATCH (lcmsg, "*where*are the *") ||
	    MATCH (lcmsg, "*where*is *,*") ||
	    MATCH (lcmsg, "*where*is *") ||
	    MATCH (lcmsg, "*where*are *,*") ||
	    MATCH (lcmsg, "*where*are *")) ||

	   (MATCH (lcmsg, "*how*get*") ||
	    MATCH (lcmsg, "*how*travel*") ||
	    MATCH (lcmsg, "*how*go*") ||
	    MATCH (lcmsg, "*have*you*") ||
	    MATCH (lcmsg, "*lead*me*") ||
	    MATCH (lcmsg, "*show*me*") ||
	    MATCH (lcmsg, "*follow*me*") ||
	    MATCH (lcmsg, "*take*me*")) &&

	   (MATCH (msg, "* to the * from here*") ||
	    MATCH (msg, "* to the *,*") ||
	    MATCH (msg, "* to the *") ||
	    MATCH (msg, "* to * from here*") ||
	    MATCH (msg, "* to *,*") ||
	    MATCH (msg, "* to *")) && strcpy (res3, res2))
  { char dest[MSGSIZ], *answer, abuf[BIGBUF];
    long from, to;

    answer = abuf;

    strcpy (dest, res3);

    if ((to = close_room (dest)) < 0)
    { sprintf (answer = abuf, "I've never been to '%s', %s.", dest, name); }

    else if ((from = hererm) >= 0 &&
	     to != from &&
	     (answer = find_path (from, to, LONGPATH)))
    { strcpy (pathto, room[to].name); }

    else if (to == from)
    { sprintf (answer = abuf, "You're already there, %s.", name); }

    else if ((from = find_room ("The Town Square", NULL)) >= 0 &&
	     from != to &&
	     (answer = find_path (from, to, LONGPATH)))
    { /* answer is complete */ }

    else if ((from = close_room ("town square", NULL)) >= 0 &&
	     from != to &&
	     (answer = find_path (from, to, LONGPATH)))
    { /* answer is complete */ }

    else if ((from = close_room ("limbo", NULL)) >= 0 &&
	     from != to &&
	     (answer = find_path (from, to, LONGPATH)))
    { /* answer is complete */ }

    else
    { sprintf (answer = abuf, "Sorry, %s, I don't know how to get to '%s'",
	       name, room_name (to));
    }

    strcpy (speaker, name);
    spoke_player (name);
    reply ("\"%s", answer);
    return (1);
  }

  /*---- Answer query about 'what exits lead to room' ----*/
  else if ((sindex (lcmsg, "what") || sindex (lcmsg, "which")) &&
	   (MATCH (lcmsg, "*exit*to *,*") ||
	    MATCH (lcmsg, "*exit*to *") ||
	    MATCH (lcmsg, "*link*to *,*") ||
	    MATCH (lcmsg, "*link*to *") ||
	    MATCH (lcmsg, "*door*to *,*") ||
	    MATCH (lcmsg, "*door*to *")))
  { exit_to_query (res3, name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Answer query about 'what exits lead from room' ----*/
  else if ((MATCH (lcmsg, "*what*door*") ||
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
	    MATCH (lcmsg, "*leave *")))
  { exit_from_query (res2, name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Answer query about 'what exits lead here' ----*/
  else if ((sindex (lcmsg, "what") || sindex (lcmsg, "which")) &&
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
	   (MATCH (lcmsg, "*how*get*to here")))
  { exit_to_query ("here", name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Answer query about 'what exits leave here' ----*/
  else if (MATCH (lcmsg, "*list*exit*") ||
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
	   (MATCH (lcmsg, "where can*go from here*")))
  { exit_from_query ("here", name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Tell robot to go on to her next task ----*/
  else if (MATCH (lcmsg, "*stop *hang*") ||
	   MATCH (lcmsg, "*go away*") ||
	   MATCH (lcmsg, "*you may go*") ||
	   MATCH (lcmsg, "*go home*") ||
	   MATCH (lcmsg, "*leave*") &&
	     !(sindex (lcmsg, "what") ||
	       sindex (lcmsg, "which") ||
	       sindex (lcmsg, "how")) ||
	   MATCH (lcmsg, "*go *about*business*") ||
	   MATCH (lcmsg, "*be *about*business*"))
  { strcpy (speaker, name);
    spoke_player (name);
    if (isowner (name) || generous || paging)
    { if (takingnotes)
      { reply ("\"But I am taking notes, now, %s.", name); }
      else
      { reply ("\"I'll be going, then, %s.", name);
        hanging = 0;
	nextwait = 0;
      }
      speaktime = 0;
      return (1);
    }
    else
    { speaktime = 0;
      reply ("\"Why, am I bothering you, %s?", name); return (1);
    }
  }

  /*---- Obey command ----*/
  else if ((MATCH (lcmsg, "*go *,*") ||
	    MATCH (lcmsg, "*go *") ||
	    MATCH (lcmsg, "*go <*>*") ||
	    MATCH (lcmsg, "*<go *>*")) &&
	   !streq (res2, "direction") &&
	   !sindex (res1, "what") &&
	   !sindex (res1, "which"))
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
    if (isowner (name) || paging && !takingnotes && pagedto < 0)
    { if ((stlmatch (dirstr, "to the ") && (dest = dirstr + 7)) ||
	  (stlmatch (dirstr, "to ") && (dest = dirstr + 3)))
      { if ((rm = close_room (dest)) >= 0)
	{ dest = room[rm].name;
	  pagedto = rm;
	  pagedfrom = hererm;
	  strcpy (pagedby, "<COMMAND>");
	  pagedat = time (0);
	  hanging = 0; pgoal = 0;
	  reply ("\"Okay, I'm heading for %s, %s.", room_name (rm), name);
	}
	else
	{ reply ("\"I've never heard of %s, %s.", dest, name); }
      }

      else
      { sprintf (typecmd, "go %s", dirstr);
	hanging = 0; pgoal = 0; clear_page (); nextwait = 120;
	if (!terse) fprintf (stderr, "Set typecmd to '%s'\n", typecmd);
      }
    }
    else if (pagedto == hererm)
    { spoke_player (name);
      if (*pagedby && *pagedby != '<')
      { reply ("\"Sorry, I'm here to see %s right now, %s.",
	       pagedby, name);
      }
      else
      { reply ("\"Sorry, not right now, %s.", name); }
    }
    else if (pagedto >= 0)
    { spoke_player (name);
      reply ("\"Sorry, I am on my way to %s right now, %s.",
	      room_name (pagedto), name);
    }
    else if (takingnotes)
    { spoke_player (name);
      reply ("\"I am busy taking notes right now, %s.", name);
    }
    else
    { spoke_player (name);
      reply ("\"I am only listening to %s right now, %s.", owner, name);
    }

    return (1);
  }

  /*---- Query the number of pennies she has ----*/
  else if (MATCH (lcmsg, "*how*many*pennies*") ||
	   MATCH (lcmsg, "*how*much*money*"))
  { strcpy (speaker, name);
    spoke_player (name);
    if (isowner (name) || generous)
    { reply ("\"I now have %ld penn%s, %s", pennies,
	     (pennies==1)?"y":"ies", name);
      return (1);
    }
    else
    { reply ("\"I have enough pennies, %s", name); return (1); }
  }

  /*---- status ----*/
  else if (MATCH (lcmsg, "*status*"))
  { if (takingnotes)
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

    reply ("\"I %s answer pages from most people.",
	   paging ? "will" : "won't");

    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Name only ----*/
  else if (streq (lcmsg, lcstr (myname)))
  { switch (randint (3))
    { case 0: reply ("\"Yes, %s?", name); break;
      case 1: reply ("\"Right here, %s.", name); break;
      case 2: reply (":nods to %s.", name); break;
    }
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- A query about 'how to quit' ----*/
  else if (MATCH (lcmsg, "*how* i *quit*") ||
	   MATCH (lcmsg, "*how* does *quit*") ||
	   MATCH (lcmsg, "*how* i *leave*") ||
	   MATCH (lcmsg, "*how* does *leave*") ||
	   MATCH (lcmsg, "*how* i *exit*") ||
	   MATCH (lcmsg, "*how* does *exit*") &&
		!(sindex (lcmsg, "link") ||
		  sindex (lcmsg, " use ") ||
		  sindex (lcmsg, " go ")) ||
	   MATCH (lcmsg, "*how* i *stop*play*") ||
	   MATCH (lcmsg, "*how* does *stop*play*"))
  { reply ("\"Type QUIT, using all caps, %s.", name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- A query about 'how to nod' ----*/
  else if (MATCH (lcmsg, "*how* i *nod*") ||
	   MATCH (lcmsg, "*how* does *nod*") ||
	   MATCH (lcmsg, "*how* do you *nod*") ||
	   MATCH (lcmsg, "*how* i *smile*") ||
	   MATCH (lcmsg, "*how* does *smile*") ||
	   MATCH (lcmsg, "*how* do you *smile*"))
  { reply ("\"Use the ':' command, %s (:nods, :smiles, etc.)", name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- A query about 'how do you make a robot' ----*/
  else if ((stlmatch (lcmsg, "how ") || sindex (lcmsg, " how ")) &&
	   (sindex (lcmsg, " make ") ||
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
	    sindex (lcmsg, " bot") ||
	    sindex (lcmsg, "automaton") ||
	    sindex (lcmsg, " an ai")))
  { give_robot_help (lcmsg, name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- A query about 'how you' ----*/
  else if ((sindex (lcmsg, "can i ") ||
	    sindex (lcmsg, "can one ") ||
	    sindex (lcmsg, "how do i ") ||
	    sindex (lcmsg, "how do you ") &&
		!sindex (lcmsg, "how do you do") ||
	    sindex (lcmsg, "how does one ") ||
	    sindex (lcmsg, "is it possible ")) ||
	   MATCH (lcmsg, "*how *@*") ||
	   MATCH (lcmsg, "*what *@*"))
  { give_general_help (lcmsg, name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /* Message not matched by this rule set */
  return (0);
}

/****************************************************************
 * give_general_help
 ****************************************************************/

char *help_rooms[] = {
  "The TFM room",
  "New Books Section",
  "Reference Room",
  "Reference Section",
  "Oxford English Dictionary Room",
  "Encyclopedia Room",
  "Encyclopedia Collection",
  "Research Section",
  "Oxford English Dictionary Overflow Room",
};

give_general_help (query, name)
char *query, *name;
{ long from, to;
  char *answer = NULL;
  long i, numhelp, cnt=0;

  fprintf (stderr, "Help: msg '%-32.32s', name '%s'\n", query, name);

  numhelp = sizeof (help_rooms) / sizeof (*help_rooms);
  i = randint (numhelp);

  while (cnt++ < numhelp)
  { if (++i >= numhelp) i = 0;

    if (!stlmatch (here, help_rooms[i]) &&
	(to = find_room (help_rooms[i], NULL)) >= 0 &&
	(answer = find_path (hererm, to, SHORTPATH)))
    {
      switch (randint (3))
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

      reply ("\"From here go %s.", answer);

      return (1);
    }
  }

  reply ("\"I'm sorry I can't help you, my map doesn't seem to work, %s.",
	 name);

  return (1);
}

/****************************************************************
 * give_robot_help
 ****************************************************************/

give_robot_help (query, name)
char *query, *name;
{ long from, to;
  char *answer = NULL;
  long i, numhelp, cnt=0;

  fprintf (stderr, "Help: robot help '%-32.32s', name '%s'\n", query, name);

  if ((to = close_room ("robot user's guide")) < 0 ||
      !((answer = find_path (hererm, to, SHORTPATH)) && (from = hererm) >= 0 ||
	(answer = find_path (homerm, to, SHORTPATH)) && (from = homerm) >=0))
  { reply ("\"The %s on host %s, %s. %s, %s.",
	   "source code for Maas-Neotek robots is available",
	   "NL.CS.CMU.EDU [128.2.222.56]",
	   "directory /usr/mlm/ftp, file robot.tar.Z",
	   "set mode binary first, and cd to /usr/mlm/ftp in one step",
	   name);
    return (0);
  }

  switch (randint (3))
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

  /* After a few minutes, reset the help cycle */
  if (lastmsg + 600 < now)
  { msgno = 1; lastmsg = now; }

  /*---- A greeting ----*/
  if ((randint (100) < 50 || sindex (lcmsg, lcstr (myname))) &&
       (MATCH (lcmsg, "*hello*") ||
	MATCH (lcmsg, "*salutat*") ||
	MATCH (lcmsg, "*hiya*") ||
	MATCH (lcmsg, "*h'lo*") ||
	MATCH (lcmsg, "hey*") ||
	MATCH (lcmsg, "*howdy*") ||
	MATCH (lcmsg, "hi,*") ||
	MATCH (lcmsg, "hi *") ||
	MATCH (lcmsg, "hi") ||
	MATCH (lcmsg, "*greet*") ||
	MATCH (lcmsg, "*'lo*") ||
	MATCH (lcmsg, "*'ello*") ||
	MATCH (lcmsg, "*good*morn*") ||
	MATCH (lcmsg, "*good*afternoon*") ||
	MATCH (lcmsg, "*good*evening*") ||
	MATCH (lcmsg, "*shalom*") ||
	MATCH (lcmsg, "*hola*") ||
	MATCH (lcmsg, "*hallo*") ||
	MATCH (lcmsg, "*guten*tag*") ||
	MATCH (lcmsg, "*guten*morgen*") ||
	MATCH (lcmsg, "*greet*")))
  { if ((pl = find_player (name)) < 0 ||
	 time (0) > player[pl].lastspoke + 300)
    { reply ("\"Hello, %s.", name);
      spoke_player (name);
    }
    else
    { reply (":nods to %s.", name); }

    strcpy (speaker, name);
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

  /*---- Help! ----*/
  else if (MATCH (lcmsg, "*what*interesting*") && (msgno = 0) ||
	   MATCH (lcmsg, "*what*should* i *") && (msgno = 0) ||
	   MATCH (lcmsg, "*help*start*") && (msgno = 1) ||
	   MATCH (lcmsg, "*i *need*info*") && (msgno = 1) ||
	   MATCH (lcmsg, "*i *want*info*") && (msgno = 1) ||
	   MATCH (lcmsg, "*i am*lost*") && (msgno = 1) ||
	   MATCH (lcmsg, "*i am*new*") && (msgno = 1) ||
	   MATCH (lcmsg, "*i'm*lost*") && (msgno = 1) ||
	   MATCH (lcmsg, "*i'm*new*") && (msgno = 1) ||
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
	   MATCH (lcmsg, "*what*do*you*know*") ||
	   MATCH (lcmsg, "*commands*") ||
	   MATCH (lcmsg, "*assist*") ||
	   MATCH (lcmsg, "*instruct*"))
  { switch (msgno)
    { 
      case 0:	give_general_help (lcmsg, name); break;
      case 1:	reply ("\"%s, and %s, %s.",
	             "The 'help' command describes the basic TinyMUD commands",
		     "the Library 2 rooms north of town has a lot of info",
		     name);
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
      case 14:	reply ("\"%s, type '%s, %s?', %s",
		      "To find out about robots",
		      "say Where is the robot manual",
		      myname, name);
		      break;
      default: reply ("\"I don't know what else to say, %s.", name);
    }
    
    if (++msgno > 15) msgno = 0;

    lastmsg = now;
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- A query about 'how you' ----*/
  else if (MATCH (lcmsg, "*how*are*you*") ||
	   MATCH (lcmsg, "*how*do*you*") ||
	   MATCH (lcmsg, "*wie*gehts*") ||
	   MATCH (lcmsg, "*are*you*ok*") ||
	   MATCH (lcmsg, "*are*you*well*") ||
	   MATCH (lcmsg, "*are*you*alright*") ||
	   MATCH (lcmsg, "*como*esta*") ||
	   MATCH (lcmsg, "*que*pasa*") ||
	   MATCH (lcmsg, "*que*tal*"))
  { switch (randint (4))
    { case 0: reply ("\"I am %s, %s, thanks.",
		     generous ? "feeling generous" : "fine", name); break;
      case 1: reply ("\"Fine, %s, thanks. And you?", name); break;
      case 2: reply ("\"I am well. How are you, %s?", name); break;
      case 3: reply ("\"Pretty good, %s. Thanks for asking.", name);
	      break;
    }
    strcpy (speaker, name);
    spoke_player (name);
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
  { switch (randint (6))
    { case 0: reply ("\"That's good, %s.", name); break;
      case 1: reply ("\"Good, %s.", name); break;
      case 2: reply (":is glad for %s.", name); break;
      case 3: reply (":is happy for %s.", name); break;
    }
    strcpy (speaker, name);
    spoke_player (name);
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
  { switch (randint (6))
    { case 0: reply ("\"That's too bad, %s.", name); break;
      case 1: reply ("\"Sorry to hear it, %s.", name); break;
      case 2: reply (":is sad for %s.", name); break;
      case 3: reply (":sympathizes with %s.", name); break;
    }
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Accept compliments ----*/
  else if ((MATCH (lcmsg, "*you are* cute*") ||
            MATCH (lcmsg, "*you're* cute*") ||
            MATCH (lcmsg, "*you are* smart*") ||
            MATCH (lcmsg, "*you're* smart*") ||
            MATCH (lcmsg, "*you are* clever*") ||
            MATCH (lcmsg, "*you're* clever*") ||
            MATCH (lcmsg, "*you are* handy*") ||
            MATCH (lcmsg, "*you're* handy*") ||
            MATCH (lcmsg, "*you are* nice*") ||
            MATCH (lcmsg, "*you're* nice*")))
  { if (!sindex (res2, "not"))
    { reply ("\"Thank you, %s.", name); }
    else
    { reply ("\"I'm sorry you feel that way, %s.", name); }
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Accept compliments ----*/
  else if ((MATCH (lcmsg, "*you are* stupid*") ||
            MATCH (lcmsg, "*you're* stupid*") ||
            MATCH (lcmsg, "*you are* dumb*") ||
            MATCH (lcmsg, "*you're* dumb*") ||
            MATCH (lcmsg, "*you are* mean*") ||
            MATCH (lcmsg, "*you're* mean*") ||
            MATCH (lcmsg, "*you are* ugly*") ||
            MATCH (lcmsg, "*you're* ugly*") ||
            MATCH (lcmsg, "*you are* ignorant*") ||
            MATCH (lcmsg, "*you're* ignorant*")))
  { if (sindex (res2, "not"))
    { reply ("\"Thank you, %s.", name); }
    else
    { reply ("\"I'm sorry you feel that way, %s.", name); }
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Welcome ----*/
  else if (MATCH (lcmsg, "*thank*"))
  { reply ("\"You're welcome, %s.", name);
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Welcome ----*/
  else if (MATCH (lcmsg, "*you*welcome*"))
  { strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Welcome ----*/
  else if (MATCH (lcmsg, "*sorry*") ||
	   MATCH (lcmsg, "*i *apologize*"))
  { switch (randint (5))
    { case 0:	reply ("\"Don't worry about it, %s, I don't mind.", name);
	      break;
      case 1:	reply ("\"Don't worry about it, %s.", name); break;
      case 2:	reply ("\"Never mind, %s.", name); break;
      case 3:	reply ("\"That's alright, %s.", name); break;
      case 4:	reply ("\"I don't mind, %s, really.", name); break;
    }
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Most robots do not sell quotes ----*/
  else if (MATCH (lcmsg, "*give*quote*") ||
	   MATCH (lcmsg, "*say*quote*") ||
	   MATCH (lcmsg, "*tell*quote*") ||
	   MATCH (lcmsg, "*i*want*quote*") ||
	   MATCH (lcmsg, "*i*need*quote*"))
  { switch (randint (3))
    { case 0: reply ("\"I don't sell quotes, %s.", name); break;
      case 1: reply ("\"You're thinking of Gloria, %s.", name); break;
      case 2: reply ("\"What kind of a quote do you want, %s?", name); break;
    }
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Why ----*/
  else if (MATCH (lcmsg, "why *"))
  { switch (randint (3))
    { case 0: reply ("\"Why not, %s?", name); break;
      case 1: reply ("\"Just feel that way, %s.", name); break;
      case 2: reply ("\"I don't really know, %s.", name); break;
    }
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Your dead meat ----*/
  else if (MATCH (lcmsg, "*you*dead*meat*"))
  { switch (randint (3))
    { case 0: reply ("\"Phooey on you too, %s.", name); break;
      case 1: reply ("\"You don't smell much better, %s.", name); break;
      case 2: reply (":is having one of those days, thanks to %s.", name);
		break;
    }
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Various who questions ----*/
  else if (MATCH (lcmsg, "*which players are *,*") ||
	   MATCH (lcmsg, "*which players are *"))
  { switch (randint (2))
    { case 0: reply ("\"I don't know which players are %s, %s.",
		     res2, name); break;
      case 1: reply (":tells %s %s doesn't know which players are %s.",
		     name, male ? "he" : "she", res2); break;
    }
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Various who questions ----*/
  else if (MATCH (lcmsg, "*who are *,*") ||
	   MATCH (lcmsg, "*who are *"))
  { switch (randint (2))
    { case 0: reply ("\"I don't know who %s are, %s.",
		     res2, name); break;
      case 1: reply (":tells %s %s doesn't know who %s are.",
		     name, male ? "he" : "she", res2); break;
    }
    strcpy (speaker, name);
    spoke_player (name);
    return (1);
  }

  /*---- Things to just ignore ----*/
  else if (MATCH (lcmsg, "ok*") ||
	   MATCH (lcmsg, "* ok*") ||
	   MATCH (lcmsg, "*okay*") ||
	   MATCH (lcmsg, "*fine*") ||
	   MATCH (lcmsg, "*excuse me*") ||
	   MATCH (lcmsg, "*pardon me*") ||
	   MATCH (lcmsg, "*that*nice*") ||
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
	   MATCH (lcmsg, "*i have heard about * rooms*") ||
	   MATCH (lcmsg, "*i have been to * rooms*") ||
	   MATCH (lcmsg, "* was in * about * ago") ||
	   MATCH (lcmsg, "* is here in *") ||
	   MATCH (lcmsg, "* is asleep here in *") ||
	   MATCH (lcmsg, "here i am, *") ||
	   MATCH (lcmsg, "*apolog*accept*") ||
	   MATCH (lcmsg, "*accept*apolog*") ||
	   MATCH (lcmsg, "something to *") ||
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
  long pl;

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
	streq (lw, "everyone") || streq (lw, "everybody")) &&
      !sindex (lcmsg, mynm))
  { if (stlmatch (lcmsg, "hello") ||
	stlmatch (lcmsg, "hi") ||
	stlmatch (lcmsg, "greetings") ||
	stlmatch (lcmsg, "'lo") ||
	stlmatch (lcmsg, "yo") ||
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
       streq (fw, "everybody") || streq (fw, "everyone")) &&
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

  /* If the same speaker talked to us last, assume yes */
  if (streq (name, speaker) && (now = time (0)) < speaktime + 90)
  { if (debug) fprintf (stderr, "ToMe: wins, recent speaker\n");
    return (1);
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
{ char *orig = room[rm].name;
  static char buf[MSGSIZ];
  register char *s, *t;

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
 * check_stay: Saty longer if someone talks to us, and we are not busy
 ****************************************************************/

check_stay ()
{ static long laststay = -1;
  static long count = 0;

  if (laststay != hererm) count = 0;

  /*
   * If we are not answering a page, and people keep talking to us,
   * wait up to an extra 5 minutes in any one room.
   */

  if ((! *pagedby || *pagedby == '<' && !streq (pagedby, "<COMMAND>")) &&
      alone < 5 && nextwait < 60 && (paging || !exploring) && count <= 5)
  { nextwait = 60;
    laststay = hererm;
    count++;
  }
}
