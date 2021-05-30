/****************************************************************
 * players.c: Common robot code for TinyMUD automata
 *
 * HISTORY
 * 19-Jan-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Sixth experimental (expand player data in player file)
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
# include "vars.h"

/****************************************************************
 * find_player: return player name
 ****************************************************************/

find_player (name)
char *name;
{ register long i;
  char pat[MSGSIZ];

  strcpy (pat, lcstr (name));

  for (i=0; i<players; i++)
  { if (strcmp (pat, lcstr (player[i].name)) == 0)
    { return (i); }
  }
  
  return (-1);
}

/****************************************************************
 * add_player: return player name
 ****************************************************************/

add_player (name)
char *name;
{ register long i;

  if (reserved (name)) return (-1);

  if ((i = find_player (name)) >= 0)
  { return (i); }
  
  /* Expand array if need be */
  if (players >= maxplayer)
  { PLAYER *oldplayer = player;

    maxplayer = maxplayer * 6 / 5 + 50;
    player_sp = maxplayer * sizeof (PLAYER);
    player = (PLAYER *) ralloc (player_sp);

    fprintf (stderr, "Util: expanding player array to %ld entries\n",
	     maxplayer);

    if (player == NULL)
    { crash_robot ("Malloc returns NULL in add_player"); }
  
    for (i=0; i<players; i++) player[i] = oldplayer[i];

    free (oldplayer);
  }

  fprintf (stderr, "Play: adding player %s, number %ld\n", name, players);

  player[players].name = makestring (name);
  player[players].number = 0;
  player[players].present = 0;
  player[players].active = 0;
  player[players].firstsaw = now;
  player[players].lastsaw = 0;
  player[players].lastspoke = 0;
  player[players].lastheard = 0;
  player[players].lastplace = -1;
  player[players].lastactive = 0;
  player[players].lastgave = 0;
  player[players].lastdona = 0;
  player[players].dontotal = 0;
  player[players].lastkill = 0;
  player[players].lastoffend = 0;
  player[players].flags = 0;
  player[players].lastlook = 0;
  player[players].user1 = 0;
  player[players].user2 = 0;
  player[players].desc = NULL;
  player[players].carry = NULL;
  player[players].dialog = NULL;
  player[players].email = NULL;
  
  return (players++);
}

/****************************************************************
 * clear_present
 ****************************************************************/

clear_present ()
{ register long i;

  for (i=0; i<players; i++)
  { player[i].present = 0; }

  player[me].active = 1;
  player[me].present = 1;
}

/****************************************************************
 * clear_active
 ****************************************************************/

clear_active ()
{ register long i;

  for (i=0; i<players; i++)
  { player[i].active = 0; }

  player[me].active = 1;
  player[me].present = 1;
}

/****************************************************************
 * saw_player: update player database
 ****************************************************************/

saw_player (name, place, desc)
char *name, *place, *desc;
{ long pl = -1, rm = -1, clock;

  if ((pl = add_player (name)) < 0) return (-1);
  if (place && *place) rm = add_room (place, desc);
  clock = now;
  
  player[pl].lastsaw = clock;
  if (rm >= 0) player[pl].lastplace = rm;
  
  player[pl].present = 1;
}

/****************************************************************
 * arrive_player: update player database
 ****************************************************************/

arrive_player (name, place, desc)
char *name, *place, *desc;
{ long pl = -1, rm = -1, clock;

  if ((pl = add_player (name)) < 0) return (-1);
  if (place && *place) rm = add_room (place, desc);
  clock = now;
  
  player[pl].lastsaw = clock;
  player[pl].lastactive = clock;
  if (rm >= 0) player[pl].lastplace = rm;
  
  player[pl].present = 1;
  player[pl].active = 1;
}

/****************************************************************
 * leave_player: update player database
 ****************************************************************/

leave_player (name, place, desc)
char *name, *place, *desc;
{ long pl = -1, rm = -1, clock;

  if ((pl = add_player (name)) < 0) return (-1);
  if (place && *place) rm = add_room (place, desc);
  clock = now;
  
  player[pl].lastsaw = clock;
  player[pl].lastactive = clock;
  if (rm >= 0) player[pl].lastplace = rm;
  
  player[pl].present = 0;
  player[pl].active = 1;
}

/****************************************************************
 * idle_player: update player database
 ****************************************************************/

idle_player (name, idle)
char *name;
long idle;
{ long pl = -1, clock;

  if ((pl = add_player (name)) < 0) return (-1);
  clock = now;
  
  player[pl].lastactive = clock - idle;
  player[pl].active = 1;
}

/****************************************************************
 * active_player: update player database
 ****************************************************************/

active_player (name, place, desc)
char *name, *place, *desc;
{ long pl = -1, rm = -1, clock;

  if ((pl = add_player (name)) < 0) return (-1);

  if (place && *place && desc && *desc)
  { if (find_room (place, desc) == hererm) player[pl].present = 1;
    rm = add_room (place, desc);
  }
  clock = now;
  
  player[pl].lastactive = clock;
  player[pl].lastsaw = clock;
  if (rm >= 0) player[pl].lastplace = rm;
  

  player[pl].active = 1;
}

/****************************************************************
 * gave_player: update player database
 ****************************************************************/

gave_player (name, amount)
char *name;
long amount;
{ long pl = -1, rm = -1, clock;

  if ((pl = add_player (name)) < 0) return (-1);
  rm = add_room (here, desc);
  clock = now;
  
  player[pl].lastactive = clock;
  player[pl].lastsaw = clock;
  if (rm >= 0) player[pl].lastplace = rm;
  
  player[pl].lastgave = clock;
}

/****************************************************************
 * donate_player: update player database
 ****************************************************************/

donate_player (name, amount)
char *name;
long amount;
{ long pl = -1, rm = -1, clock;

  if ((pl = add_player (name)) < 0) return (-1);
  rm = add_room (here, desc);
  clock = now;
  
  player[pl].lastactive = clock;
  player[pl].lastsaw = clock;
  if (rm >= 0) player[pl].lastplace = rm;
  
  player[pl].lastdona = clock;
  player[pl].dontotal += amount;
}

/****************************************************************
 * assault_player: update player database
 ****************************************************************/

assault_player (name)
char *name;
{ long pl = -1, rm = -1;

  if ((pl = add_player (name)) < 0) return (-1);
  rm = add_room (here, desc);
  
  player[pl].lastactive = now;
  player[pl].lastsaw = now;
  if (rm >= 0) player[pl].lastplace = rm;
  
  player[pl].lastkill = now;

  player[pl].present = 1;
  player[pl].active = 1;
  strcpy (killer, name);
}

/****************************************************************
 * heard_player: update player database
 ****************************************************************/

heard_player (name, str)
char *name, *str;
{ long pl = -1;
  char buf[(MSGSIZ+DIALOGSIZE+2)];
  register char *s, *t;
  long len, bytes;

  if ((pl = add_player (name)) < 0) return (-1);

  player[pl].lastheard = now;
  player[pl].lastactive = now;
  player[pl].active = 1;
  player[pl].present = 1;
  if (hererm >= 0) player[pl].lastplace = hererm;

  /* Track last few lines of dialog */
  if (str && !stlmatch (str, "something to "))
  { if (player[pl].dialog == NULL)
    { player[pl].dialog = makefixstring ("", DIALOGSIZE); }

    /* Copy message into buf, removing vertical bars (|) */
    for (s=str, t=buf, bytes=0; *s && bytes < BIGBUF; s++)
    { if (*s != '|')
      { *t++ = *s; bytes++; }
    }

    /* Append older messages, separated by vertical bars (|) */
    *t++ = '|';
    for (s=player[pl].dialog; *s && bytes < BIGBUF; )
    { *t++ = *s++; bytes++; }
    *t++ = '\0';

    if (bytes > (DIALOGSIZE + MSGSIZ))
    { fprintf (stderr, "Warn: heard_player, bytes moved %ld!\n", bytes); }
    
    buf[DIALOGSIZE-1] = '\0';
    
    if ((bytes = strlen (buf)) >= DIALOGSIZE)
    { crash_robot ("buf too long in heard_player, %ld bytes", bytes); }
    
    strcpy (player[pl].dialog, buf);
  }
}

/****************************************************************
 * spoke_player: update player database
 ****************************************************************/

spoke_player (name)
char *name;
{ long pl = -1;

  if ((pl = add_player (name)) < 0) return (-1);

  player[pl].lastspoke = now;
}

/****************************************************************
 * read_players: Read in a TinyMud players file
 ****************************************************************/

read_players (fn)
char *fn;
{ char buf[BIGBUF], name[MSGSIZ];
  register char *s;
  FILE *mfile;
  long npl, cur = -1;

  /* Open the map file */
  if ((mfile = fopen (fn, "r")) == NULL)
  { if (access (fn, 0) < 0)
    { fprintf (stderr, "Util: starting with blank players\n");
      realloc_players (200);
      return (1);
    }
    else
    { fatal ("Util: can't read %s.\n", fn); }
  }

  /* Read map file header */
  if (!fgets (buf, BIGBUF, mfile))
  { fclose (mfile);
    fprintf (stderr, "Util: null map file, %s, will be over-written\n", fn);
    realloc_players (200);
    return (1);
  }
  
  /* Check for old style map file */  
  if (stlmatch (buf, "Gloria Players File"))
  { fclose (mfile);
    fatal ("Old style players file, use mn6cvt to convert it\n");
  }

  /* Check for new style header */
  if (!stlmatch (buf, "Maas-Neotek Players file"))
  { fclose (mfile);
    fatal ("Fatal, '%s' does not have a valid Maas-Neotek map file header\n",
	   fn);
  }
  
  /* Now read room list */  
  while (fgets (buf, BUFSIZ, mfile))
  { buf[strlen (buf) - 1] = '\0';

    if (*buf != '\0' && buf[1] != ':')
    { fprintf (stderr, "Warn: bad player line: %s\n", buf);
      continue;
    }

    /* Handle each line differently */
    switch (*buf)
    { case 'K':	npl = atol (buf+2);
		if (npl <= 0)
		{ fclose (mfile);
		  fatal ("Fatal error in %s, %s\n: %s\n",
			 fn, "number of players must be positive", buf);
		}
		
		if (npl < 200) npl = 200;
		
		fprintf (stderr, "Util: allocating %ld players\n", npl);
		
		realloc_players (npl);
		break;

      case 'T': 
      case 'H': 
      case 'P': 
      case 'I': /* Ignore extra information for now */
		break;

      case 'N': if (!valid_user_name (buf+2))
		{ cur = -1;
		  fprintf (stderr, "Warn: bogus player name '%s'\n", buf);
		}
		else
		{ cur = players++;
		  player[cur].name = makestring (buf+2);
		}
		break;

      case 'F':	if (cur >= 0)
		{ if (sscanf (buf,
	   "F:%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
		      &player[cur].number, &player[cur].dontotal,
		      &player[cur].firstsaw, &player[cur].flags,
		      &player[cur].lastactive, &player[cur].lastdona,
		      &player[cur].lastgave, &player[cur].lastheard,
		      &player[cur].lastkill, &player[cur].lastlook,
		      &player[cur].lastoffend, &player[cur].lastplace,
		      &player[cur].lastsaw, &player[cur].lastspoke,
		      &player[cur].user1, &player[cur].user2) != 16)
		  { fprintf (stderr, "Bogus F line: %s\n", buf); }
		}
		
		break;

      case 'S':	if (cur >= 0)
		{ player[cur].dialog = makefixstring (buf + 2, DIALOGSIZE); }
		break;

      case 'D':	if (cur >= 0)
		{ player[cur].desc = makestring (buf + 2); }
		break;

      case 'C':	if (cur >= 0)
		{ player[cur].carry = makefixstring (buf + 2, DIALOGSIZE); }
		break;

      case 'E':	if (cur >= 0)
		{ player[cur].email = makestring (buf + 2); }
		break;

      case '#':
      case '\0': break;

      default:	fprintf (stderr, "Warn: bad player file line: %s\n", buf);
    }
  }
  
  fclose (mfile);

  fprintf (stderr, "Util: read %ld players from %s\n", players, fn);

  return (1);
}

/****************************************************************
 * realloc_players:
 ****************************************************************/

realloc_players (n)
long n;
{ PLAYER *oldplayer = player;
  long oldmax = maxplayer;
  register long i;

  maxplayer = n;

  player_sp = maxplayer * sizeof (PLAYER);
  player = (PLAYER *) ralloc (player_sp);

  fprintf (stderr, "Util: allocating player array of %ld entries\n",
	   maxplayer);

  if (player == NULL)
  { crash_robot ("malloc returns NULL in realloc_players"); }

  /* If expanding, copy over old information */
  i = 0;
  if (oldplayer)  
  { for (; i<oldmax; i++)
    { player[i] = oldplayer[i]; }
  
    free (oldplayer);
  }
  
  /* Now zero out the rest of the array */
  for (; i<maxplayer; i++)
  { player[i].number = -1;
    player[i].present = 0;
    player[i].active = 0;
    player[i].firstsaw = 0;
    player[i].lastsaw = 0;
    player[i].lastspoke = 0;
    player[i].lastheard = 0;
    player[i].lastplace = 0;
    player[i].lastactive = 0;
    player[i].lastgave = 0;
    player[i].lastdona = 0;
    player[i].dontotal = 0;
    player[i].lastkill = 0;
    player[i].lastoffend = 0;
    player[i].flags = 0;
    player[i].lastlook = 0;
    player[i].user1 = 0;
    player[i].user2 = 0;

    player[i].name = NULL;
    player[i].desc = NULL;
    player[i].carry = NULL;
    player[i].dialog = NULL;
    player[i].email = NULL;
  }
}

/****************************************************************
 * write_players: Write current state of players
 ****************************************************************/

write_players (outname)
char *outname;
{ register long i;
  register O_EXIT *xp;
  long swap = 0;
  char tmpfile[MSGSIZ];
  FILE *outfil;

  if (players <= 0) return (0);

  sprintf (tmpfile, "%s.NEW", outname);
  if (access (outname, 0) == 0)
  { swap++;
    if ((outfil = fopen (tmpfile, "w")) == 0)
    { perror (tmpfile); return (0); }
  }
  else
  { if ((outfil = fopen (outname, "w")) == 0)
    { perror (outname); return (0); }
  }

  fprintf (outfil, "Maas-Neotek Players file: %s\n\n", VERSION);

  fprintf (outfil, "K:%ld players\n", players);
  fprintf (outfil, "T:%s", ctime (&now));
  fprintf (outfil, "H:%s\n", mudhost);
  fprintf (outfil, "P:%ld\n", mudport);
  fprintf (outfil, "I:%s\n", myname);

  for (i=0; i<players; i++)
  { if (player[i].name && player[i].name[0])
    { fprintf (outfil, "\nN:%s\n", player[i].name);
      fprintf (outfil,
	 "F:%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
	       player[i].number, player[i].dontotal, player[i].firstsaw,
	       player[i].flags, player[i].lastactive, player[i].lastdona,
	       player[i].lastgave, player[i].lastheard, player[i].lastkill,
	       player[i].lastlook, player[i].lastoffend, player[i].lastplace,
	       player[i].lastsaw, player[i].lastspoke, player[i].user1,
	       player[i].user2);

      if (player[i].desc && player[i].desc[0])
      { fprintf (outfil, "D:%s\n", player[i].desc); }

      if (player[i].carry && player[i].carry[0])
      { fprintf (outfil, "C:%s\n", player[i].carry); }

      if (player[i].email && player[i].email[0])
      { fprintf (outfil, "E:%s\n", player[i].email); }

      if (player[i].dialog && player[i].dialog[0])
      { fprintf (outfil, "S:%s\n", player[i].dialog); }
    }
  }
  
  fclose (outfil);
  
  if (swap)
  { return (swap_files (tmpfile, outname)); }
  else
  { return (1); }
}

/****************************************************************
 * player_query
 ****************************************************************/

player_query (pl, name)
long pl;
char *name;
{ long dur = now - player[pl].lastsaw;
  char *answer[BIGBUF];

  /* Four cases: here, seen known place, seen unknown place, not seen */
  if (player[pl].present)
  { if (name && streq (name, player[pl].name))
    { sprintf (answer, "\"You are here in %s", here); }
    else
    { if (player[pl].active)
      { sprintf (answer, "\"%s is right here in %s",
		 player[pl].name, here);
      }
      else
      { sprintf (answer, "\"%s has been asleep here in %s for about %s",
		player[pl].name, here,
		time_dur (now - player[pl].lastactive));
      }
    }
  }

  else if (player[pl].lastplace >= 0 &&
	   player[pl].lastplace < rooms &&
	   player[player[pl].lastplace].name &&
	   player[pl].lastsaw > 0)
  { sprintf (answer, "\"%s was %sin %s about %s ago",
	     player[pl].name,
	     (((now - player[pl].lastsaw) > 600 ||
	      player[pl].active) ? "" : "asleep "),
	     room_name (player[pl].lastplace), time_dur (dur));
  }
  else if (player[pl].lastsaw > 0)
  { sprintf (answer, "\"I saw %s about %s ago",
	     player[pl].name, time_dur (dur));
  }
  else
  { sprintf (answer, "\"I haven't seen %s", player[pl].name); }

  if (name)
  { sprintf (answer, "%s, %s.", answer, name); }
  else
  { strcat (answer, "."); }
  
  reply (answer);
}

/****************************************************************
 * all_player_query
 ****************************************************************/

typedef struct astruct { long pl, ago; } P_ORDER;

cmp_saw (a, b)
register P_ORDER *a, *b;
{ return (a->ago - b->ago); }

all_player_query (dur, name)
long dur;
char *name;
{ long limit, recent;
  P_ORDER *order;
  register long pl, i, cnt;

  limit = now - dur;
  recent = now - 300;

  /* Make a permutation array, keyed by "timeago" */  
  if ((order = (P_ORDER *) ralloc (players * sizeof (P_ORDER))) == NULL)
  { crash_robot ("malloc returns NULL in all_player_query"); }

  /* Fill the array */
  for (i=0; i<players; i++)
  { order[i].pl = i; order[i].ago = now - player[i].lastsaw; }

  /* Sort by time ago */
  qsort (order, players, sizeof (P_ORDER), cmp_saw);

  /* Now go through the players in order of recency */
  for (i = 0, cnt = 0; i < players && cnt < 20; i++)
  { pl = order[i].pl;

    if (player[pl].name && player[pl].name[0] &&
        !streq (player[pl].name, name) &&
	!streq (player[pl].name, myname) &&
	((cnt < 10) || (player[pl].lastsaw > recent)))
    { player_query (pl, NULL); cnt++; }
  }
  
  command (":done");

  /* Free the temporary array before returning */  
  free (order);
}

/****************************************************************
 * reserved: Words that cannot (or should not) be real users
 ****************************************************************/

char *rw[] = { "a", "an", "the", "you", "some", NULL };

reserved (word)
char *word;
{ register char **s, *t;

  t = lcstr (word);
  
  for (s=rw; *s; s++)
  { if (strcmp (*s, t) == 0) return (1); }
  
  return (0);
}

/****************************************************************
 * check_players: If we start a new map, clear lastpace for each
 *		  player
 ****************************************************************/

check_players ()
{ register long pl;

  if (rooms == 0 && players > 0)
  { for (pl=0; pl<players; pl++)
    { player[pl].lastplace  = -1; }
    
    fprintf (stderr, "Play: cleared %ld player's lastplace flags\n", players);
  }
}

/****************************************************************
 * look_up_player: Find a player's tinymud ID number
 ****************************************************************/

look_up_player (name)
char *name;
{ long pl;
  static long inlook = 0;

  if ((pl = find_player (name)) < 0)	return (0);
  if (player[pl].number > 0)		return (player[pl].number);

# ifdef OLD_EXAMINE
  if (inlook)
  { fprintf (stderr, "Warn: got recursive look_up_players, dropping one\n");
    return (0);
  }

  lastlock = 0;

  inlook++;
  sendmud ("%s %s\n@lock me = *%s\nexamin me\n%s %s\n@lock me = me",
	   opre, numpre, name, opre, outpre);
  waitfor (outsuf);	/* Eat lock output */
  waitfor (outsuf);	/* Eat examine output */
  waitfor (outsuf);	/* Eat relock output */
  inlook--;
  
  if (lastlock > 0)
  { player[pl].number = lastlock; }
  else
  { lastlock = 0; }
# else
  lastlock = 0;
# endif
  
  return (lastlock);
}

/****************************************************************
 * look_at_thing: Find a thing's description
 ****************************************************************/

look_at_thing (name)
char *name;
{ long pl;

  if ((pl = find_player (name)) >= 0)
  { player[pl].lastlook = now; }
  
  if (debug)
  { fprintf (stderr, "Look: issuing look at '%s'(%ld)\n", name, pl); }

  sendmud ("%s %s %s\nlook %s\n%s %s",
	   opre, plypre, name, name, opre, outpre);
  waitfor (outsuf);	/* Eat look output output */
  
  return (1);
}

/****************************************************************
 * killed_me_today: Return true is 'name' attempted to kill us
 ****************************************************************/

killed_me_today (name)
char *name;
{ long pl, dur;

  if (pl = find_player (name))
  { dur = now - player[pl].lastkill;
  
    if (dur < 18 * HOURS)
    { return (dur); }
  }
  
  return (0);
}

/****************************************************************
 * object_query: Look at player names, descriptions, and inventories.
 ****************************************************************/

object_query (obj, name)
char *obj;
{ register long pl;
  long printed = 0;

  for (pl=0; pl<players; pl++)
  { if (sindex (lcstr (player[pl].name), obj) ||
	player[pl].desc  && sindex (lcstr (player[pl].desc), obj) ||
	player[pl].carry && sindex (lcstr (player[pl].carry), obj))
    { if (!printed++)
      { reply ("\"Here are the matches for %s, %s:", obj, name); }
	
      if (printed > 10 && alone > 1)
      { reply ("\"There are more matches, but %s, %s.",
	       " it's too crowded here to go on", name);
	return;
      }

      reply (":");

      if (player[pl].desc)
      { reply (":%s's description is: %s", player[pl].name, player[pl].desc); }

      if (player[pl].carry)
      { reply (":%s carries: %s", player[pl].name, player[pl].carry); }
    }
  }
  
  if (!printed)
  { reply ("\"I don't know anyone matching %s, %s.", obj, name); }
  else
  { reply (":done."); }
}

/****************************************************************
 * asleep_query: Who is asleep in this room
 ****************************************************************/

asleep_query (name)
char *name;
{ char buf[BUFSIZ];
  register long pl;
  long printed = 0;

  strcpy (buf, "");
  for (pl=0; pl<players; pl++)
  { if (player[pl].present && !player[pl].active)
    { if (printed) strcat (buf, " ");
      strcat (buf, player[pl].name);
      printed++;
    }
  }

  if (!printed)
  { reply ("\"I don't see anyone asleep here, %s.", name); }
  else
  { reply ("\"Well %s, I see %ld player%s asleep here: %s", name,
	   printed, printed == 1 ? "" : "s", buf);
  }
}

/****************************************************************
 * awake_query: Who is awake in this room
 ****************************************************************/

awake_query (name)
char *name;
{ char buf[BUFSIZ];
  register long pl;
  long printed = 0;

  strcpy (buf, "");
  for (pl=0; pl<players; pl++)
  { if (player[pl].present && player[pl].active)
    { if (printed) strcat (buf, " ");
      strcat (buf, player[pl].name);
      printed++;
    }
  }

  if (!printed)
  { reply ("\"I don't see anyone awake here, %s.", name); }
  else
  { reply ("\"Well %s, I see %ld player%s awake here: %s", name,
	   printed, printed == 1 ? "" : "s", buf);
  }
}

/****************************************************************
 * connected_query: Who is connected
 ****************************************************************/

connected_query (name)
char *name;
{ char buf[BIGBUF];
  register long pl;
  long printed = 0, lurk = 0;
  
  wsynch ();

  strcpy (buf, "");
  for (pl=0; pl<players; pl++)
  { if (player[pl].active && (player[pl].lastactive + 30 * MINUTES > now))
    { if (printed) strcat (buf, " ");
      strcat (buf, player[pl].name);
      printed++;
    }
  }

  if (!printed)
  { reply ("\"I don't see anyone idle less than 30 minutes, %s.", name); }
  else
  { reply ("\"Well %s, I see %ld player%s idle less than 30 minutes: %s", name,
	   printed, printed == 1 ? "" : "s", buf);
  }

  strcpy (buf, "");
  for (pl=0; pl<players; pl++)
  { if (player[pl].active && (player[pl].lastactive + 30 * MINUTES <= now))
    { if (lurk) strcat (buf, " ");
      strcat (buf, player[pl].name);
      lurk++;
    }
  }

  if (lurk)
  { reply ("\"I %ssee %ld player%s idle more than 30 minutes: %s",
	   printed ? "also " : "", lurk, lurk == 1 ? "" : "s", buf);
  }
}

/****************************************************************
 * who_is: Whi is a particular player
 ****************************************************************/

who_is (pers, name)
char *pers, *name;
{ long pl;
  long printed=0;

  pl = find_player (res2);
  if (debug) fprintf (stderr, "Who is query: %s(%ld)\n", pers, pl);

  /* Unknown player */
  if (pl < 0)
  { reply ("\"I have never seen player %s, %s.", res2, name);
    return (1);
  }

  /* Check for person asking about robot */
  if (streq (pers, lcstr (myname)))
  { reply ("\"I'm %s, %s.", whoami, name);
    return (1);
  }

  printed = who_is_hook (pers, name);

  /* Give more info about non-owners */
  if (!streq (pers, "fuzzy") || paging)
  {
    /* Report players last description */
    if (player[pl].number > 0)
    { if (streq (pers, lcstr (name)))
      { reply ("\"Your id number is %ld, %s", player[pl].number, name); }
      else
      { reply ("\"%s's id number is %ld, %s",
	       pers, player[pl].number, name);
      }
      printed++;
    }

    /* Report players last description */
    if (player[pl].desc && player[pl].desc[0])
    { if (streq (pers, lcstr (name)))
      { reply ("\"Your description reads: %s", player[pl].desc); }
      else
      { reply ("\"%s's description reads: %s", pers, player[pl].desc); }
      printed++;
    }

    /* Report players last Email address */
    if (player[pl].email && player[pl].email[0])
    { if (streq (pers, lcstr (name)))
      { reply ("\"Your Email address: %s", player[pl].email); }
      else
      { reply ("\"%s's Email address: %s", pers, player[pl].email); }
      printed++;
    }

    /* Report players last inventory */
    if (player[pl].carry && player[pl].carry[0])
    { if (streq (pers, lcstr (name)))
      { reply ("\"You're carrying: %s", player[pl].carry); }
      else
      { reply ("\"%s is carrying: %s", pers, player[pl].carry); }
      printed++;
    }

    /* Report various memory things about people */
    if (player[pl].dialog && player[pl].dialog[0])
    { register char *s, *t, *head, *tail;
      char *sent = NULL;
      long cnt=0;
      char buf[DIALOGSIZE];

      head = player[pl].dialog;
      tail = head + strlen (head);
      
      /* Now each '|' (or s == head) is start of sentence */
      for (s=head; *s; s++)
      { if (s == head || s[-1] == '|')
	{ for (t=s; *t && *t != '|'; t++) ;
	  if (*t == '|')
	  { strcpy (buf, lcstr (s));
	    buf[t-s] = '\0';
	  }
	  else
	  { break; }


	  /* Dont report players queries */
	  if (sindex (buf, "where is") ||
	      sindex (buf, "how do i g") ||
	      sindex (buf, "how do you g") ||
	      stlmatch (buf, "something to") ||
	      sindex (buf, "who is"))
	  { if (debug) fprintf (stderr, "Play: skipping '%s'\n", buf);
	    continue;
	  }
  
	  if (debug) fprintf (stderr, "Play: using    '%s'\n", buf);
  
	  if (randint (++cnt) == 0) sent = s;
        }
      }
      
      /* If we chose a sentence, repeat it */
      if (sent)
      { strcpy (buf, sent);
	for (s=buf; *s && *s != '|'; s++) ;
	*s = '\0';
   
	if (streq (pers, lcstr (name)))
	{ reply ("\"I once heard you say, '%s'", buf); }
	else
	{ reply ("\"I once heard %s say, '%s'", pers, buf); }
	printed++;
      }
    }
    
    /* Report last time assualted */
    if (player[pl].lastkill)
    { reply ("\"%s last attacked me %s ago", pers,
	     time_dur (now - player[pl].lastkill));
    }
    
    /* Report last time donated, and total amount */
    if (player[pl].lastdona)
    { reply ("\"%s last gave me money %s ago, and %s of %ld pennies.", pers,
	     time_dur (now - player[pl].lastdona),
	     "has given me a total", player[pl].dontotal);
    }
    
    /* Report jerk status */
    if (PLAYER_GET (pl, PL_JERK))
    { reply ("\"I will not obey commands from %s", pers); }
  }

  if (!printed)
  { if (streq (pers, lcstr (name)))
    { reply ("\"I don't really know who you are, %s.", name); }
    else
    { reply ("\"I don't really know who %s is, %s.", pers, name); }
  }
}

/****************************************************************
 * is_jerk: True is player is a jerk
 ****************************************************************/

is_jerk (name)
char *name;
{ register long pl;

  if ((pl = find_player (name)) < 0) return (0);
  
  if (PLAYER_GET (pl, PL_JERK)) return (1);
  
  return (0);
}

/****************************************************************
 * valid_user_name:
 ****************************************************************/

valid_user_name (str)
register char *str;
{
  if (*str == '\0') return (0);
  
  while (*str)
  { if (*str < ' ' || *str > '~' || index (" \t=", *str)) return (0);
    str++;
  }
  
  return (1);
}
