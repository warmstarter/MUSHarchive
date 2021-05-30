/****************************************************************
 * players.c: Common robot code for TinyMUD automata
 *
 * HISTORY
 * 01-May-91  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Don't page idle players msgs.
 *
 * 05-May-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Seventh sequential release.  Add the quote player
 *	command, Avoid saving duplicate player dialog.
 *	Mods for TinyHELL.
 *
 * 02-Apr-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Sixth experimental release.  Add time information to
 *	who_is queries.
 *
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
# include <time.h>
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

long find_player (name)
char *name;
{ register long i;
  char pat[MSGSIZ];

  if (!name && !*name) return (-1);

  strcpy (pat, lcstr (name));
  
  if (streq (pat, "you") || streq (pat, "yourself"))
  { strcpy (pat, lcstr (myname)); }

  for (i=0; i<players; i++)
  { if (strcmp (pat, lcstr (player[i].name)) == 0)
    { return (i); }
  }
  
  return (-1);
}

/****************************************************************
 * close_player: return player name, or speaker name if "me"
 ****************************************************************/

long close_player (str, who)
char *str, *who;
{ register long i;
  char pat[MSGSIZ];

  if (!str && !*str) return (-1);

  strcpy (pat, lcstr (str));
  
  if (streq (pat, "you") || streq (pat, "yourself"))
  { strcpy (pat, lcstr (myname)); }

  else if (streq (pat, "me") || streq (pat, "myself") || streq (pat, "i"))
  { strcpy (pat, lcstr (who)); }

  for (i=0; i<players; i++)
  { if (strcmp (pat, lcstr (player[i].name)) == 0)
    { return (i); }
  }
  
  return (-1);
}

/****************************************************************
 * add_player: return player name
 ****************************************************************/

long add_player (name)
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
  player[players].lastheralded = 0;
  player[players].lastlook = 0;
  player[players].user1 = 0;
  player[players].user2 = 0;

  player[players].desc = NULL;
  player[players].carry = NULL;
  player[players].dialog = NULL;
  player[players].email = NULL;
  player[players].msgs = NULL;
  
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

  if (me >= 0)
  { player[me].active = 1;
    player[me].present = 1;
  }
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

  if (msgtype < M_PAGE)
  { player[pl].present = 1;
    if (hererm >= 0) player[pl].lastplace = hererm;
  }

  /* Track last few lines of dialog  - avoid duplicate sentences */
  if (str &&
      !is_hearts (lcstr (str)) &&
      !stlmatch (str, "something to ") &&
      !stlmatch (str, "I once heard  ") &&
      !sindex (str, "quote ") &&
      !sindex (lcstr (str), "give me pennies") &&
      !is_tell (lcstr (str)) &&
      (!isowner (name) ||
		!sindex (lcstr (str), "code") &&
		!sindex (lcstr (str), "shutdown") &&
		!sindex (lcstr (str), "terse") &&
		!sindex (lcstr (str), "debug")) &&
      (player[pl].dialog == NULL || !sindex (player[pl].dialog, str)))
  { if (player[pl].dialog == NULL)
    { player[pl].dialog = makefixstring ("", DIALOGSIZE); }

    /* Copy message into buf, removing vertical bars (|) */
    for (s=str, t=buf, bytes=0; *s && bytes < BIGBUF; s++)
    { if (*s != '|')
      { *t++ = *s; bytes++; }
    }

    /* Append older messages, separated by vertical bars (|) */
    *t++ = '|'; bytes++;
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

#define OLDEST(X,Y) ((X)?((X)<(Y)?(X):(Y)):(Y))
#define NEWEST(X,Y) ((X)>(Y)?(X):(Y))

read_players (fn)
char *fn;
{ char buf[BIGBUF], name[MSGSIZ], msgtxt[BIGBUF], wname[MSGSIZ];
  register char *s;
  FILE *mfile;
  long npl = 0, timestmp, cur = -1, nread = 0;
  PLAYER *pp;
  long t_number, t_dontotal, t_firstsaw, t_flags, t_lastactive, t_lastdona;
  long t_lastgave, t_lastheard, t_lastkill, t_lastlook, t_lastoffend;
  long t_lastplace, t_lastsaw, t_lastspoke, t_user1, t_user2;

  /* Open the players file */
  if ((mfile = fopen (fn, "r")) == NULL)
  { if (access (fn, 0) < 0)
    { fprintf (stderr, "Util: starting with blank players\n");
      realloc_players (200);
      return (1);
    }
    else
    { fatal ("Util: can't read %s.\n", fn); }
  }

  /* Read player file header */
  if (!fgets (buf, BIGBUF, mfile))
  { fclose (mfile);
    fprintf (stderr,
	     "Util: null players file, %s, will be over-written\n", fn);
    realloc_players (200);
    return (1);
  }
  
  /* Check for old style players file */  
  if (stlmatch (buf, "Gloria Players File"))
  { fclose (mfile);
    fatal ("Old style players file, use mn6cvt to convert it\n");
  }

  /* Check for new style header */
  if (!stlmatch (buf, "Maas-Neotek Players file"))
  { fclose (mfile);
    fatal ("Fatal, '%s' %s\nBad line: %s\n",
	   fn, "does not have a valid Maas-Neotek players file header", buf);
  }
  
  /* Now read room list */  
  while (fgets (buf, BIGBUF, mfile))
  { buf[strlen (buf) - 1] = '\0';

    if (*buf != '\0' && buf[1] != ':')
    { fprintf (stderr, "Warn: bad player line: %s\n", buf);
      continue;
    }

    /* Handle each line differently */
    switch (*buf)
    { case 'K':	if (npl > 0)
    		{ fprintf (stderr, "Warning, new K: line %s\n", buf); }
		else
		{ npl = atol (buf+2);
		  if (npl <= 0)
		  {   fclose (mfile);
		    fprintf (stderr, "Warning in %s, %s\n: %s\n",
			   fn, "number of players should be positive", buf);
		  }
		
		  if (npl < 200) npl = 200;
		
		  realloc_players (npl);
		}
		break;

      case 'G': if (sscanf (buf, "G:%ld %[^\n]", &heraldtime, herald) != 2)
		{ *herald = '\0'; heraldtime = 0; }
		break;

      case 'T': 
      case 'H': 
      case 'P': 
      case 'I': /* Ignore extra information for now */
		break;

      case 'W':	if (sscanf (buf, "W:%[^\n]", wname) == 1)
		{ if (!streq (world, wname))
		  { fprintf (stderr, "Wrld: new is %s, old was %s\n",
			     world, wname);
		  }
		}
		break;

      case 'M': if (cur >= 0)
		{ if (sscanf (buf, "M:%d %[^\n]", &timestmp, msgtxt) == 2)
		  { add_msg (cur, timestmp, msgtxt); }
		  else
		  { fprintf (stderr, "Warn: bogus M line: %s", buf); }
		}
		break;
      
      case 'N': 
#ifdef DEBUG_PLAYERS
		if ((++nread % 100) == 0)
		{ fprintf (stderr, "Read %d entries...\n", nread); }
#endif
		if (!valid_user_name (buf+2))
		{ cur = -1;
		  fprintf (stderr, "Warn: bogus player name '%s'\n", buf);
		}
		else if ((cur = find_player (buf+2)) >= 0)
		{ if (cur >= maxplayer)
		  { fprintf (stderr, "Woah! cur %d > maxplayer 5d.\n",
			     cur, maxplayer); }
		}
		else
		{ cur = players++;
#ifdef DEBUG_PLAYERS
		  if ((players % 500) == 0)
		  { fprintf (stderr, "Read %d unique players...\n", players); }
#endif
		  player[cur].name = makestring (buf+2);
		}
		break;

      case 'F':	if (cur >= 0)
		{ pp = &player[cur];
	          if (sscanf (buf,
			      "F:%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
			      &t_number, &t_dontotal, &t_firstsaw, &t_flags,
			      &t_lastactive, &t_lastdona, &t_lastgave,
			      &t_lastheard, &t_lastkill, &t_lastlook,
			      &t_lastoffend, &t_lastplace, &t_lastsaw,
			      &t_lastspoke, &t_user1, &t_user2) == 16)
	          { 
		    pp->number = OLDEST (pp->number, t_number);
		    pp->dontotal += t_dontotal;
		    pp->firstsaw = OLDEST (pp->firstsaw, t_firstsaw);
		    pp->flags |= t_flags;
		    pp->lastactive = NEWEST (pp->lastactive, t_lastactive);
		    pp->lastdona = NEWEST (pp->lastdona, t_lastdona);
		    pp->lastgave = NEWEST (pp->lastgave, t_lastgave);
		    pp->lastheard = NEWEST (pp->lastheard, t_lastheard);
		    pp->lastkill = NEWEST (pp->lastkill, t_lastkill);
		    pp->lastlook = NEWEST (pp->lastlook, t_lastlook);
		    pp->lastoffend = NEWEST (pp->lastoffend, t_lastoffend);
		    pp->lastplace = t_lastplace;
		    pp->lastsaw = NEWEST (pp->lastsaw, t_lastsaw);
		    pp->lastspoke = NEWEST (pp->lastspoke, t_lastspoke);
		    pp->user1 = t_user1;
		    pp->user2 = t_user2;
		  }
		  else
		  { fprintf (stderr, "Warn: Bogus F line: %s\n", buf); }
		}
		
		break;

      case 'S':	if (cur >= 0)
		{ if (player[cur].dialog)
		  { strncpy (player[cur].dialog, buf + 2, DIALOGSIZE);
		    player[cur].dialog[DIALOGSIZE-1] = '\0';
		  }
		  else
		  { player[cur].dialog = makefixstring (buf + 2, DIALOGSIZE); }
		}
		break;

      case 'D':	if (cur >= 0)
		{ freestring (player[cur].desc);
		  player[cur].desc = makestring (buf + 2);
		}
		break;

      case 'C':	if (cur >= 0)
		{ if (player[cur].carry)
		  { strncpy (player[cur].carry, buf + 2, DIALOGSIZE);
		    player[cur].carry[DIALOGSIZE-1] = '\0';
		  }
		  else
		  { player[cur].carry = makefixstring (buf + 2, DIALOGSIZE); }
		}
		break;

      case 'E':	if (cur >= 0)
		{ freestring (player[cur].email);
		  player[cur].email = makestring (buf + 2);
		}
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

  maxplayer = n+100;

  player_sp = maxplayer * sizeof (PLAYER);
  player = (PLAYER *) ralloc (player_sp);

  fprintf (stderr, "Util: allocating player array of %ld entries\n",
	   maxplayer);

  if (player == NULL)
  { crash_robot ("malloc returns NULL in realloc_players"); }

  /* If expanding, copy over old information */
  i = 0;
  if (oldplayer)  
  { fprintf (stderr, "Copying old array[%d..%d] to new array...\n",
	     i, oldmax-1);

    for (; i<oldmax; i++)
    { player[i] = oldplayer[i]; }
  
    free (oldplayer);
  }
  
  /* Now zero out the rest of the array */
  fprintf (stderr, "Util: clearing out player array from %d to %d...\n",
	   i, maxplayer);
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
    player[i].lastheralded = 0;
    player[i].lastlook = 0;
    player[i].user1 = 0;
    player[i].user2 = 0;

    player[i].name = NULL;
    player[i].desc = NULL;
    player[i].carry = NULL;
    player[i].dialog = NULL;
    player[i].email = NULL;
    player[i].msgs = NULL;
  }
}

/****************************************************************
 * write_players: Write current state of players
 ****************************************************************/

write_players (outname)
char *outname;
{ register long i;
  register O_EXIT *xp;
  long swap = 0, newplayers = 0;
  char tmpfile[MSGSIZ];
  MSGS *mp;
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
  
# ifndef NO_WEED_PLAYERS
  /* Weed out old players that havent been seen for a while */
  for (i=0; i<players; i++)
  { long age;

    if (player[i].lastactive > 0)
    { age = now - player[i].lastactive; }
    else if (player[i].lastsaw > 0)
    { age = now - player[i].lastsaw; }
    else
    { age = now - player[i].firstsaw; }

    /* Everyone hangs around for 2 months, plus we remember trusties & jerks */
    if (age < 60 * DAYS || PLAYER_GET (i, PL_JERK | PL_REMEMBER))
    { newplayers++; continue; }

    /* If there are non-entities (no messages or quotes), get rid of them */
    if (player[i].dialog == NULL && player[i].msgs == NULL)
    { PLAYER_SET (i, PL_OLD); continue; }
    
#ifdef WEED_500
    /* Other players hang around for 500 days */
    if (age < 500 * DAYS)
    { newplayers++; continue; }
#endif
    
    /* If we havent seen them for 500 days, good riddance */
    PLAYER_SET (i, PL_OLD); continue;
  }
#else
  /* Keep everyone */
  for (i=0; i<players; i++)
  { PLAYER_CLR (i, PL_OLD); }

  newplayers = players;
# endif

  fprintf (outfil, "Maas-Neotek Players file: %s\n\n", VERSION);

  fprintf (outfil, "K:%ld players\n", newplayers);
  fprintf (outfil, "T:%s", ctime (&now));
  fprintf (outfil, "W:%s\n", world);
  fprintf (outfil, "H:%s\n", mudhost);
  fprintf (outfil, "P:%ld\n", mudport);
  fprintf (outfil, "I:%s\n", myname);
  if (*herald && heraldtime > 0)
  { fprintf (outfil, "G:%ld %s\n", heraldtime, herald); }

  for (i=0; i<players; i++)
  { if (player[i].name && player[i].name[0] && !PLAYER_GET (i, PL_OLD))
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
      
      for (mp = player[i].msgs; mp; mp = mp->next)
      { fprintf (outfil, "M:%ld %s\n", mp->timestmp, mp->text); }
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
  char answer[BIGBUF];

  if (name && *name) strcpy (speaker, name);

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
  { sprintf (answer, "%s{, n}.", answer); }
  else
  { strcat (answer, "."); }
  
  reply ("%s", answer);
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
  int numpl;
  char who[MSGSIZ];
  
  strcpy (who, name);
  
  if (alone > 2 && msgtype < M_WHISPER) msgtype = M_WHISPER;

  limit = now - dur;
  recent = now - 300;
  
  numpl = players;

  /* Make a permutation array, keyed by "timeago" */  
  if ((order = (P_ORDER *) ralloc (numpl * sizeof (P_ORDER))) == NULL)
  { crash_robot ("malloc returns NULL in all_player_query"); }

  /* Fill the array */
  for (i=0; i<numpl; i++)
  { order[i].pl = i; order[i].ago = now - player[i].lastsaw; }

  /* Sort by time ago */
  qsort (order, numpl, sizeof (P_ORDER), cmp_saw);

  /* Now go through the players in order of recency */
  for (i = 0, cnt = 0; i < numpl && cnt < 20; i++)
  { pl = order[i].pl;

    if (player[pl].name && player[pl].name[0] &&
        !streq (player[pl].name, name) &&
	!streq (player[pl].name, myname) &&
	(player[pl].active || (now - player[pl].lastactive) < 12 * HOURS) &&
	((cnt < 10) || (player[pl].lastsaw > recent)))
    { strcpy (speaker, who);
      player_query (pl, NULL); cnt++;
    }
  }
  
  reply ("|done");

  /* Free the temporary array before returning */  
  free (order);
}

/****************************************************************
 * reserved: Words that cannot (or should not) be real users
 ****************************************************************/

char *rw[] = {
  "(", "a", "about", "an", "and", "any", "anything", "but", "damn", "few",
  "for", "fuck", "haha", "hello", "help", "hey", "hi", "how", "i", "in",
  "maybe", "more", "most", "no", "nope", "not", "of", "oh", "out", "piss",
  "right", "shit", "some", "test", "testing", "thanks", "that", "the",
  "these", "this", "those", "what", "when", "where", "who", "why", "with",
  "wrong", "yeah", "yep", "yes", "you", "your", "yup", NULL };

reserved (word)
char *word;
{ register char **s;

  for (s=rw; *s; s++)
  { if (strfoldeq (*s, word)) return (1); }
  
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

long look_up_player (name)
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

  if ((pl = find_player (name)) >= 0)
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
  int numpl;
  long printed = 0;

  if (alone > 2 && msgtype < M_WHISPER) msgtype = M_WHISPER;

  numpl = players;

  for (pl=0; pl<numpl; pl++)
  { if (sindex (lcstr (player[pl].name), obj) ||
	player[pl].desc  && sindex (lcstr (player[pl].desc), obj) ||
	player[pl].carry && sindex (lcstr (player[pl].carry), obj))
    { if (!printed++)
      { reply ("\"Here are the matches for %s{, n}:", obj); }
	
      if (printed > 10 && (alone > 1 || !isowner (name)))
      { reply ("\"There are more matches, but %s, %s.",
	       " it's too crowded here to go on", name);
	return;
      }

      strcpy (speaker, name);
      reply ("|");

      strcpy (speaker, name);
      if (player[pl].desc)
      { reply ("| %s's description is: %s", player[pl].name,player[pl].desc);}

      strcpy (speaker, name);
      if (player[pl].carry)
      { reply ("| %s carries: %s", player[pl].name, player[pl].carry); }
      
      if (!player[pl].desc && !player[pl].carry)
      { strcpy (speaker, name);
	reply ("| %s", player[pl].name);
      }
    }
  }
  
  strcpy (speaker, name);

  if (!printed)
  { reply ("\"I don't know anyone matching %s{, n}.", obj); }
  else
  { reply ("|done."); }
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
  { reply ("\"I don't see anyone asleep here{, n}."); }
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
  { reply ("\"I don't see anyone awake here{, n}."); }
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
  { reply ("\"I don't see anyone idle less than 30 minutes{, n}."); }
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
 * who_is: Who is a particular player
 ****************************************************************/

who_is (pers, name)
char *pers, *name;
{ long pl, msgcnt;
  long printed=0;
  char who[MSGSIZ];

  pl = find_player (pers);
  strcpy (who, name);

  if (msgtype < M_WHISPER) msgtype = M_WHISPER;

  /* Unknown player */
  if (pl < 0 && streq (name, "(>"))
  { switch (nrrint (189, 3))
    { case 0: zinger ("\"That's the heartsbot{, n}.");  break;
      case 1: zinger ("\"The hearts playing robot{, n}."); break;
      case 2: zinger ("\"(> moderates games of Hearts{, n}."); break;
    }

    return (1);
  }

  /* Unknown player */
  if (pl < 0)
  { switch (nrrint (190, 5))
    { case 0: reply ("\"I have never seen player %s{, n}.", pers);  break;
      case 1: reply ("\"I've never seen %s{, n}.", pers); break;
      case 2: reply ("\"%s who{, n}?", pers); break;
      case 3: reply ("\"I don't know any %s{, n}.", pers); break;
      case 4: reply ("\"Never met %s{, n}.",
      		     malep (pers) ? "him" : "her"); break;
    }

    return (1);
  }

  /* Check for person asking about robot */
  if (streq (pers, lcstr (myname)))
  { reply ("\"My description is %s.", mydesc);
    return (1);
  }

  if (debug)
  { fprintf (stderr, "Who:  gave dossier of %s(%ld)\n", pers, pl); }
  
  printed = who_is_hook (pers, name);
  
  if (printed && is_newbie (name)) return (1);

  /*
   * Need short description for newbies...something that does not sound
   * like it came from a computer (no X seconds ago, and no long repetition
   * of description
   */

  /* Give more info about non-owners */
  if (!streq (pers, "fuzzy") || paging)
  {
    /* Report players last description */
    if (!is_newbie (name) && player[pl].number > 0)
    { strcpy (speaker, who);
      if (streq (pers, lcstr (name)))
      { reply ("\"Your id number is %ld{, n}", player[pl].number); }
      else
      { reply ("\"%s's id number is %ld{, n}",
	          pers, player[pl].number);
      }
      printed++;
    }

    /* Report players last description */
    if (player[pl].desc && player[pl].desc[0])
    { strcpy (speaker, who);

      if (strfoldeq (pers, name))
      { reply ("\"As of %s ago, your description was: %s",
	          time_dur (now-player[pl].lastlook), player[pl].desc);
      }
      else
      { reply ("\"As of %s ago, %s's description was: %s",
	          time_dur (now-player[pl].lastlook), pers, player[pl].desc);
      }
      printed++;
    }

    /* Report players last Email address */
    if (player[pl].email && player[pl].email[0])
    { strcpy (speaker, who);

      if (strfoldeq (pers, name))
      { reply ("\"Your Email address: %s", player[pl].email); }
      else
      { reply ("\"%s's Email address: %s", pers, player[pl].email); }
      printed++;
    }
    
    if ((msgcnt = msg_count (pl)) > 0)
    { strcpy (speaker, who);
      reply ("\"I have %d message%s for %s.",
		msgcnt, msgcnt == 1 ? "" : "s", pers);
    }

    /* Report players last inventory */
    if (player[pl].carry && player[pl].carry[0])
    { strcpy (speaker, who);

      if (streq (pers, lcstr (name)))
      { reply ("\"You were carrying: %s", player[pl].carry); }
      else
      { reply ("\"%s was carrying: %s", pers, player[pl].carry); }
      printed++;
    }

    /* Report first time seen */
    if (player[pl].firstsaw > 0)
    { strcpy (speaker, who);

      reply ("\"I first saw %s logged in %s ago", pers,
		time_dur (now - player[pl].firstsaw));
    }

    /*----  Report various memory things about people ----*/
    
    /* Juicy quotes */
    if (player[pl].dialog && player[pl].dialog[0])
    { strcpy (speaker, who);
      quote_player (pers, name, 0); }

    /* Report last time assualted */
    if (player[pl].lastkill)
    { strcpy (speaker, who);
      reply ("\"%s last attacked me %s ago", pers,
		time_dur (now - player[pl].lastkill));
    }
    
    /* Report last time donated, and total amount */
    if (player[pl].lastdona)
    { strcpy (speaker, who);
      reply ("\"%s last gave me money %s ago, and %s %ld %s.", pers,
		time_dur (now - player[pl].lastdona),
		(player[pl].dontotal == 1) ?
			"had the generosity to give me" :
			"has given me a total of",
		player[pl].dontotal,
		(player[pl].dontotal == 1) ? "whole penny" : "pennies");
    }
    
    /* Report jerk status */
    if (PLAYER_GET (pl, PL_JERK))
    { strcpy (speaker, who);
      reply ("\"I will not obey commands from %s", pers); }
  }

  if (!printed)
  { strcpy (speaker, who);
    if (streq (pers, lcstr (name)))
    { reply ("\"I don't really know who you are{, n}."); }
    else
    { reply ("\"I don't really know who %s is{, n}.", pers); }
  }
}

/****************************************************************
 * quote_player: Repeat the words of another player
 ****************************************************************/

quote_player (pers, name, gossip)
char *pers, *name;
int gossip;
{ long pl;
  long printed=0;

  /* Check for person asking about robot */
  if (streq (pers, lcstr (myname)) || streq (pers, "yourself"))
  { switch (nrrint (191, 3))
    { case 0: reply ("\"I'm not very quotable{, n}."); break;
      case 1: reply ("\"I'll leave that to others{, n}."); break;
      case 2: reply ("\"Sorry, I don't do recursion{, n}."); break;
    }
    return (1);
  }

  /* Check for 'me' */
  if (streq (pers, "me"))
  { pers = name; }
  
  if (streq (pers, lcstr (name)) && randint (100) < 20)
  { reply ("\"A little vain today, aren't we{, n}?");
    return (1);
  }

  /* Now do lookup */
  pl = find_player (pers);
  if (debug) fprintf (stderr, "Quote query: %s(%ld)\n", pers, pl);

  /* Unknown player */
  if (pl < 0)
  { if (gossip) reply ("\"I have never heard player %s{, n}.", pers);
    return (1);
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
	    sindex (buf, "tell") ||
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
 
      if (streq (pers, lcstr (name)) || streq (pers, name))
      { reply ("\"I once heard you say, '%s'", buf); }
      else
      { reply ("\"I once heard %s say, '%s'", pers, buf); }
      printed++;
    }
  }
  
  if (!printed && gossip)
  { reply ("\"I haven't heard %s say anything quotable{, n}.", pers); }
  
  return (printed);
}

/****************************************************************
 * quote_random_player: Repeat the words of another player
 ****************************************************************/

quote_random_player (name)
char *name;
{ long pl;
  long printed=0;
  long cnt = 0;

  /* Loop through players, finding a quote */
  while (!printed && (pl = randint (players)) >= 0 && ++cnt < players)
  {
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
	      sindex (buf, "tell") ||
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
   
	reply ("\"I once heard %s say, '%s'", player[pl].name, buf);
	printed++;
      }
    }
  }
  
  if (!printed)
  { reply ("\"I haven't heard any good gossip lately{, n}."); }
  
  return (printed);
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
  
  if (reserved (str))
  { return (0); }
  
  while (*str)
  { if (*str < ' ' || *str > '~' || index (" \t=", *str)) return (0);
    str++;
  }
  
  return (1);
}

/****************************************************************
 * add_msg: Add a message to a players input queue
 ****************************************************************/

add_msg (pl, timestmp, msg)
long pl, timestmp;
char *msg;
{ MSGS *mp, *new;
  long mcnt = 0;

  if (!(new = (MSGS *) malloc (sizeof (MSGS))))
  { crash_robot ("malloc returns NULL in add_msg"); }

  new->text = makestring (msg);
  new->timestmp = timestmp;
  new->next = NULL;
  
  if ((mp = player[pl].msgs) == NULL)
  { player[pl].msgs = new; }
  else
  { for (mcnt=0; mp->next; mp = mp->next) mcnt++;
    mp->next = new;
  }
}

/***************************************************************
 * do_msgs: If someone is logged on or in the room, 
 *	check for their messages and repeat them
 ****************************************************************/

do_msgs ()
{ register long pl;
  MSGS *mp;
  char *who, *errmsg;
  static last_haven = 0;

  for (pl=0; pl<players; pl++)
  { if (player[pl].active &&
	(player[pl].msgs || *herald && !PLAYER_GET (pl, PL_HERALDED) ) &&
	(now - player[pl].lastactive <= 5 * MINUTES) &&
	(player[pl].present ||
	 pagedmsgs && (!PLAYER_GET(pl, PL_HAVEN) ||
	 	       (now - last_haven) > 10 * MINUTES)))
    { give_msgs (pl); }
  }
}

/****************************************************************
 * give_msgs:  Assume a player is active, send him/her any messages
 ****************************************************************/

give_msgs (pl)
long pl;
{ MSGS *mp;
  char *who, *errmsg, msgbuf[MSGSIZ], *unheralded();
  char recip[MSGSIZ];
  register char *s, *t;
  int is_herald, result = 0;
  static last_haven = 0;

  if (pl == me)
  { PLAYER_SET (pl, PL_HERALDED); return 0; }

  if (pl < 0) return 0;

  if (now - player[pl].lastactive > 5 * MINUTES) return 0;

  who = player[pl].name;
    
  /* last_haven tracks the last time we paged a non-Havened player */
  if (!player[pl].present && !PLAYER_GET(pl, PL_HAVEN))
  { last_haven = now; }
  

  /* Clear out recip */
  strcpy (recip, "");

  /* Deliver any private messages */
  while ((is_herald =
	   (*herald &&
	    !PLAYER_GET (pl, PL_HERALDED) &&
	    !PLAYER_GET (pl, PL_HAVEN))) ||
	 (mp = player[pl].msgs))
  { strcpy (speaker, who);

    /* Attempt to deliver herald message */
    if (is_herald)
    { strcpy (recip, "");
      msgtype = player[pl].present ? M_SPOKEN : M_PAGE;
      msgstat = 0;

      /* Use %n as a replacement for the name of the recipient */
      for (s=herald, t=msgbuf; *s; s++)
      { if (*s == '%')
        { switch (*(++s))
	  { 
	    /* Name of recipient(s) */
	    case 'n':	strcpy (recip, unheralded (msgtype, pl));
	    		strcpy (t, recip);
			while (*t) t++;
			break;
	    case 't':	strcpy (t, timeofday (0));
			while (*t) t++;
			break;
	    default:	*t++ = *s;
	    		break;
	  }
	}
	else
	{ *t++ = *s; }
      }
      *t = '\0';
      
      /* Find recipients */
      if (*recip == '\0')
      { strcpy (recip, player[pl].name); }

      /* Whisper long messages to a single recipient in a crowded
       * room, instead of saying them out loud */
      if (strlen (msgbuf) > 64 && 
	  alone > 2 &&
	  !sindex (recip, ", ") &&
	  !streq (recip, "everybody") &&
	  msgtype == M_SPOKEN)
      { msgtype = M_WHISPER; }

      fprintf (stderr, "Hrld: [%s to %s] \"%s\"\n",
	       (msgtype == M_WHISPER ? "whisper" :
	        msgtype == M_SPOKEN ? "say" : "page"),
	        recip, msgbuf);

      /* Deliver the message */
      unlogged ("\"%s", msgbuf);
    }

    /* Attempt to deliver private message */
    else
    { msgtype = player[pl].present ? M_WHISPER : M_PAGE;
      msgstat = 0;
      fprintf (stderr, "Dlvr: to %s, %s old\n",
	       player[pl].name, exact_dur (now - mp->timestmp));
      unlogged ("\"%s ago, %s", time_dur (now - mp->timestmp), mp->text);
    }

    /* Mark herald bits on recipients */
    if (msgstat >= 0 && is_herald)
    { if (msgtype == M_SPOKEN)
      { long i;
      
        for (i=0; i<players; i++)
	{ if (player[i].present && player[i].active)
	  { PLAYER_SET (i, PL_HERALDED);
	    player[i].lastheralded = now;
	  }
	}
      }
      else
      { PLAYER_SET (pl, PL_HERALDED);
	player[pl].lastheralded = now;
	result++;
      }
    }

    /* Remove message if delivery successful */
    else if (msgstat >= 0)
    { player[pl].msgs = mp->next;
      freestring (mp->text);
      free (mp);
      PLAYER_CLR(pl, PL_HAVEN);
      
      result++;
    }

    /* Error message to log if failed */
    else
    { switch (msgstat)
      { case  1:	errmsg = "success";			break;
	case  0:	errmsg = "not yet sent";		break;
	case -1:	player[pl].present = 0;
		    errmsg = "whisper to player not present";  break;
	case -2:	player[pl].active = 0;
		    errmsg = "recipient disconnected";	break;
	case -3:	player[pl].active = 0;
		    errmsg = "no such player";		break;
	case -4:	PLAYER_SET(pl, PL_HAVEN);
		    errmsg = "recipient set haven";		break;
	default:	errmsg = "unknown error";
      }

      fprintf (stderr,
	       "Warn: msg type %d to %s gave status %d (%s)\n",
	       msgtype, who, msgstat, errmsg);
      
      break;
    }
  }
  
  return (result);
}

/****************************************************************
 * unheralded: Names of people present who havent gotten the herald
 *	       if more than 3 names, use "everybody".  If msg is not
 *	       spoken, just return name.
 ****************************************************************/

char *unheralded (msgtype, pl)
int msgtype, pl;
{ static char buf[MSGSIZ];
  register char *s = buf;
  int total = 0, count = 0;

  if (msgtype != M_SPOKEN)
  { return (player[pl].name); }

  /* First count recipients */
  for (pl=0; pl<players; pl++)
  { if (player[pl].active && player[pl].present &&
	!PLAYER_GET (pl, PL_HERALDED))
    { total++; }
  }

  if (total > 3) return ("everybody");

  /* Now collect their names */
  strcpy (buf, "");
  for (pl=0; pl<players; pl++)
  { if (player[pl].active && player[pl].present &&
	!PLAYER_GET (pl, PL_HERALDED))
    { if (++count == 1)
      { strcpy (buf, player[pl].name); }
      else
      { strcat (buf, count == total ? " and " : ", ");
        strcat (buf, player[pl].name);
      }
    }
  }

  return (s);
}

/****************************************************************
 * msg_count: Returns number of messages for player
 ****************************************************************/

long msg_count (pl)
long pl;
{ MSGS *mp;
  long cnt=0;

  for (mp = player[pl].msgs; mp; mp = mp->next) cnt++;
  
  return (cnt);
}

/****************************************************************
 * msg_total: Returns number of messages for player
 ****************************************************************/

long msg_total (name)
char *name;
{ MSGS *mp;
  long cnt=0, pls=0, pl;
  

  for (pl=0; pl<players; pl++)
  { if (mp = player[pl].msgs)
    { pls++;
      for (; mp; mp = mp->next) cnt++;
    }
  }
  
  if (pl > 0)
  { reply ("\"I am holding %d message%s for %d player%s, %s.",
	   cnt, cnt==1 ? "" : "s", pls, pls == 1 ? "" : "s", name);
  }
  else
  { reply ("\"I don't have any messages for anyone right now{, n}."); }
}

/****************************************************************
 * most_query: Which room(s) have the most of something
 ****************************************************************/

# define QCNT 10

most_pl_query (name, type)
char *name;
int type;
{ long pl, b, i, j, best[QCNT], bcnt = 0;
  double val, bestval[QCNT];
  
  /* Search through every room */
  for (pl=0; pl<players; pl++)
   { if ((now - player[pl].lastspoke) > 7 * DAYS) continue;

    switch (type)
    { case FRIENDLY: val = (double) player[pl].user1; break;
    }
       
    /* Now find place to insert in sorted list */
    for (i=0; i<bcnt; i++)
    { if (bestval[i] < val)
      {
        /* Move the rest of the list down by 1 */
	for (j=QCNT-1; j>i; j--)
	{ best[j] = best[j-1]; bestval[j] = bestval[j-1]; }

	/* Now insert this room */
	best[i] = pl; bestval[i] = val;
	if (bcnt < QCNT) bcnt++;
	
	break;
      }
    }

    /* If we ran off the list, but it is short, add to the end */
    if (i == bcnt && bcnt < QCNT)
    { best[bcnt] = pl;
      bestval[bcnt] = val;
      bcnt++;
    }
  }
  
  /* best[0..bcnt-1] and bestval[0..bcnt-1]  now hold sorted list */
  switch (type)
  { case FRIENDLY:	reply ("\"The %d friendliest players are:",
			       QCNT);
  			for (b=0; b<bcnt; b++)
			{ strcpy (speaker, name);
			  reply ("| %s spoke to me %1.0lf times.",
				 player[best[b]].name, bestval[b]);
			}
			break;
  }
}
