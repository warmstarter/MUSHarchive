/****************************************************************
 * explore.c: Common robot code for TinyMUD automata
 *
 * HISTORY
 * 26-Feb-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Sixth experimental release. Changed file formats, added lots
 *	of info to room memory.  Found a memory allocation bug.
 *
 * 10-Feb-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Fifth special release.
 *
 * 04-Feb-90  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Fourth general release
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
 * read_map: Read in a TinyMud map file
 ****************************************************************/

read_map (fn)
char *fn;
{ char buf[BIGBUF], name[MSGSIZ], dir[MSGSIZ];
  register char *s;
  register O_EXIT *xp;
  FILE *mfile;
  long nrooms = 0, cur = 0, tmno = -1, xloc = -1;
  long tf, tv, cr, tr, as, ss, nm;

  /* Open the map file */
  if ((mfile = fopen (fn, "r")) == NULL)
  { if (access (fn, 0) < 0)
    { fprintf (stderr, "Util: starting with blank map\n");
      realloc_rooms (200);
      return (1);
    }
    else
    { fatal ("Map:  can't read %s.\n", fn); }
  }

  /* Read map file header */
  if (!fgets (buf, BIGBUF, mfile))
  { fclose (mfile);
    fprintf (stderr, "Util: null map file, %s, will be over-written\n", fn);
    realloc_rooms (200);
    return (1);
  }
  
  /* Check for old style map file */  
  if (stlmatch (buf, "Gloria Map File"))
  { fclose (mfile);
    fatal ("Old style map file, use mn6cvt to convert it\n");
  }

  /* Check for new style header */
  if (!stlmatch (buf, "Maas-Neotek Map file"))
  { fclose (mfile);
    fatal ("Fatal, '%s' does not have a valid Maas-Neotek map file header\n",
	   fn);
  }
  
  /* Now read room list */  
  while (fgets (buf, BUFSIZ, mfile))
  { buf[strlen (buf) - 1] = '\0';

    if (*buf != '\0' && buf[1] != ':')
    { fprintf (stderr, "Warn: bad map line: %s\n", buf);
      continue;
    }

    /* Handle each line differently */
    switch (*buf)
    { case 'K':	nrooms = atol (buf+2);
		if (nrooms <= 0)
		{ fclose (mfile);
		  fatal ("Fatal error in %s, %s\n: %s\n",
			 fn, "number of rooms must be positive", buf);
		}
		
		if (nrooms < 200) nrooms = 200;
		
		fprintf (stderr, "Util: allocating %ld rooms\n", nrooms);
		
		realloc_rooms (nrooms);
		break;

      case 'T': 
      case 'H': 
      case 'P': 
      case 'I': /* Ignore extra information for now */

      case 'R':	if (sscanf (buf, "R:%ld %[^\n]", &cur, name) != 2)
		{ cur = -1; }
		else if (cur < 0 || cur >= maxrooms)
		{ fatal ("Bogus room number %ld (max %ld) in read_map: %s\n",
			 cur, maxrooms, buf);
		}
		else
		{ room[cur].name = makestring (name); }
		break;

      case 'F':	if (cur >= 0)
		{ if (sscanf (buf, "F:%ld %ld %ld %ld %ld %ld %ld %ld",
			      &room[cur].number, &room[cur].firstin,
			      &room[cur].lastin, &room[cur].cntin,
			      &room[cur].totalin, &room[cur].awakesum,
			      &room[cur].sleepsum, &room[cur].msgsum) != 8)
		  { fprintf (stderr, "Bogus F line: %s\n", buf); }
		}
		
		if (rooms <= cur) rooms = cur+1;
		
		break;

      case 'D':	if (cur >= 0 && buf[2])
		{ room[cur].desc = makestring (buf + 2); }
		else
		{ fprintf (stderr, "Warn: cur %ld, ignoring '%s'\n", cur,buf);}
		break;

      case 'C':	if (cur >= 0 && buf[2])
		{ room[cur].contents = makestring (buf + 2); }
		else
		{ fprintf (stderr, "Warn: cur %ld, ignoring '%s'\n", cur,buf);}
		break;

      case 'B':	if (cur >= 0 && buf[2])
		{ add_exit (cur, exitstring  (buf+2), cur); }
		else
		{ fprintf (stderr, "Warn: cur %ld, ignoring '%s'\n", cur,buf);}
		break;

      case 'E':	if (cur >= 0)
		{ if (sscanf (buf+2, "%ld %[^\n]", &xloc, dir) == 2 && *dir)
		  { add_exit (cur, exitstring (dir), xloc); }
		  else
		  { fprintf (stderr, "Warn: bogus exit line: %s\n", buf); }
		}
		else
		{ fprintf (stderr, "Warn: ignoring exit, cur %ld: %s\n",
			   cur, buf);
		}
		break;

      case '#':
      case '\0': break;

      default:	fprintf (stderr, "Warn: bad map file line: %s\n", buf);
    }
  }
  
  fclose (mfile);

  reach_added++;
  newexits = newrooms = 0;
  lastcheck = now = time (0);

  fprintf (stderr, "Util: read %ld rooms from %s\n", rooms, fn);

  return (1);
}

/****************************************************************
 * realloc_rooms:
 ****************************************************************/

realloc_rooms (n)
long n;
{ O_ROOM *oldroom = room;
  long oldmax = maxrooms;
  register long i;

  if (path != NULL) free (path);

  maxrooms = n;

  room_sp = maxrooms * sizeof (O_ROOM);
  room = (O_ROOM *) ralloc (room_sp);
  path_sp = maxrooms * sizeof (PATH);
  path = (PATH *) ralloc (path_sp);

  fprintf (stderr, "Util: allocating room array of %ld entries\n", maxrooms);

  if (room == NULL)
  { crash_robot ("malloc returns NULL in realloc_rooms"); }

  /* If expanding, copy over old information */
  i = 0;
  if (oldroom)  
  { for (; i<oldmax; i++)
    { room[i] = oldroom[i]; }
  
    free (oldroom);
  }
  
  /* Now zero out the rest of the array */
  for (; i<maxrooms; i++)
  { room[i].number = -1;
    room[i].firstin = 0;
    room[i].lastin = 0;
    room[i].cntin = 0;
    room[i].totalin = 0;
    room[i].awakesum = 0;
    room[i].sleepsum = 0;
    room[i].msgsum = 0;
    room[i].name = NULL;
    room[i].desc = NULL;
    room[i].contents = NULL;
    room[i].exits = NULL;
  }
}

/****************************************************************
 * write_map: Write current state of map
 ****************************************************************/

write_map (outname, reach_only)
char *outname;
long reach_only;
{ register long i;
  register O_EXIT *xp;
  long swap = 0;
  char tmpfile[MSGSIZ], buf[BUFSIZ];
  FILE *outfil;

  if (rooms <= 0) return (0);

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

  now = time (0);

  fprintf (outfil, "Maas-Neotek Map file: %s\n\n", VERSION);

  fprintf (outfil, "K:%ld rooms\n", rooms);
  fprintf (outfil, "T:%s", ctime (&now));
  fprintf (outfil, "H:%s\n", mudhost);
  fprintf (outfil, "P:%ld\n", mudport);
  fprintf (outfil, "I:%s\n", myname);

  /* Each iteration of loop writes one room */
  for (i=0; i<rooms; i++)
  { if (room[i].name && (!reach_only || can_reach (i)))
    { fprintf (outfil, "\nR:%ld %s\n", i, room[i].name);
    
      fprintf (outfil, "F:%ld %ld %ld %ld %ld %ld %ld %ld\n",
	       room[i].number, room[i].firstin, room[i].lastin,
	       room[i].cntin, room[i].totalin, room[i].awakesum,
	       room[i].sleepsum, room[i].msgsum);

      if (room[i].desc && room[i].desc[0])
      { strncpy (buf, room[i].desc, BUFSIZ);
        buf[BUFSIZ-3] = '\0';
        fprintf (outfil, "D:%s\n", buf);
      }

      if (room[i].contents && room[i].contents[0])
      { strncpy (buf, room[i].contents, BUFSIZ);
        buf[BUFSIZ-3] = '\0';
        fprintf (outfil, "C:%s\n", buf);
      }

      /* Each iteration of loop writes one exit from this room */
      for (xp = room[i].exits; xp; xp = xp->next)
      { if (!reach_only ||
            reach_only == 1 && can_reach (xp->room) ||
	    reach_only == 2 && can_reach (xp->room) && xp->room != i)
        { if (xp->room == i)
	  { fprintf (outfil, "B:%s\n", xp->dir); }
	  else
	  { fprintf (outfil, "E:%ld\t%s\n", xp->room, xp->dir); }
	}
      }
    }
  }
  
  fclose (outfil);
  
  if (swap)
  { return (swap_files (tmpfile, outname)); }
  else
  { return (1); }
}

/****************************************************************
 * dump_map: Dump current state of map
 ****************************************************************/

dump_map ()
{ register long i;
  register O_EXIT *xp;

  fprintf (stderr, "\n\nMaas-Neotek Map Dump: %ld rooms\n\n", rooms);

  for (i=0; i<rooms; i++)
  { if (room[i].name)
    { print_room (&room[i], i); fprintf (stderr, "\n");

      if (room[i].exits)
      { fprintf (stderr, "\tAlso tried:");
	
        for (xp = room[i].exits; xp; xp = xp->next)
        { if (xp->room == i) fprintf (stderr, " %s", xp->dir); }
        fprintf (stderr, "\n");
      }

      for (xp = room[i].exits; xp; xp = xp->next)
      { if (xp->room != i)
        { fprintf (stderr, "%15.15s ", xp->dir);

          if (xp->room < maxrooms)
	  { print_room (&room[xp->room], xp->room); }
	  else
	  { fprintf (stderr, "(undefined)"); }

	  fprintf (stderr, "\n");
	}
      }

      fprintf (stderr, "\n");
    }
  }
}

print_room (rm, num)
O_ROOM *rm;
long num;
{
  fprintf (stderr, "\"%s\"", rm->name);

  if (rm->number >= 0)
  { fprintf (stderr, " (#%ld)", rm->number); }

# ifdef DEBUG
  if (num >= 0)
  { fprintf (stderr, " [%ld]", num); }
# endif
}


/****************************************************************
 * exitstring: Return standard pointers for common exits, and
 * use makestring for uncommon exits.
 ****************************************************************/

char *exitstring (str)
register char *str;
{ register char **ep;

  for (ep=stexits; *ep; ep++)
  { if (strcmp (*ep, str) == 0) return (*ep); }
  
  return (makestring (str));
}

/****************************************************************
 * find_room: Find a room number given the name
 * modification: find last room in list, not first
 ****************************************************************/

find_room (roomnam, desc)
char *roomnam, *desc;
{ register long i;
  long best = -1, bestscore = 0, matches = 0, score;
  long roomnum;

  if (desc && !*desc) desc = NULL;

  if (smatch (roomnam, "* (#*)", roomstr) && isdigit (*room2))
  { roomnum = atoi (room2); }
  else if (smatch (roomnam, "*(#*)", roomstr) && isdigit (*room2))
  { roomnum = atoi (room2); }
  else
  { strcpy (room1, roomnam); roomnum = -1; }

  for (i=0; i<rooms; i++)
  { if (room[i].name)
    { if (score = same_room (i, roomnum, room1, desc))
      { 
	/* Prefer reachable rooms, and rooms with shorter names */
	if (score > bestscore ||
	    score == bestscore &&
	    (can_reach (i) && ! can_reach (best) ||
	     strlen (room[i].name) < strlen (room[best].name)))
        { if (best >= 0 && can_reach (best) && !can_reach (i)) continue;

	  best = i; bestscore = score;
        }
	
	if (debug && desc && room[i].desc)
	{ fprintf (stderr, "Find: matching %s[%16.16s] to %s[%16.16s](%ld)\n",
		   roomnam, desc, room[i].name, room[i].desc, i);
	}
	
	matches++;
      }
      else if (debug && streq (room[i].name, room1))
      { fprintf (stderr, "Find: failing to match %s, desc '%s', number %ld\n",
		 room1, desc ? desc : "(no desc)", roomnum);
	fprintf (stderr, "Find: room[%ld].desc '%s', room[%ld].number %ld\n",
		 i, room[i].desc ? room[i].desc : "(no desc)",
		 i, room[i].number);
      }
    }
  }

  /* Print result of find_room */
  if (!debug)
  { /* No message */ }
  else if (matches > 1)
  { if (desc)
    { fprintf (stderr,
	   "Find: find_room, multiple matches for %s[%16.16s], first %s(%ld)\n",
	       roomnam, desc, room[best].name, best);
    }
    else
    { fprintf (stderr,
	       "Find: find_room, multiple matches for %s, first %s(%ld)\n",
	       roomnam, room[best].name, best);
    }
  }
  else if (matches == 1)
  { if (desc)
    { fprintf (stderr, "Find: find_room, match for %s[%16.16s] => %s(%ld)\n",
	       roomnam, desc, room[best].name, best);
    }
    else
    { fprintf (stderr, "Find: find_room, match for %s => %s(%ld)\n",
	       roomnam, room[best].name, best); }
  }
  else if (matches == 0)
  { if (desc)
    { fprintf (stderr, "Find: find_room, no matches for %s[%16.16s]\n",
	       roomnam, desc);
    }
    else
    { fprintf (stderr, "Find: find_room, no matches for %s\n", roomnam); }
  }

  /* If room name/description changed, make the change */
  if (best >= 0)
  { if (!streq (room[best].name, room1))
    { fprintf (stderr, "Warn: Changing room name '%s' to '%s' (%ld)\n",
		room[best].name, room1, best);
      freestring (room[best].name);
      room[best].name = makestring (room1);
    }

    if (*desc && (!room[best].desc || !streq (room[best].desc, desc)))
    { fprintf (stderr,
	    "Warn: Changing room desc for %s from '%s' to '%s' (%ld)\n",
		room[best].name, room[best].desc, desc, best);
      freestring (room[best].desc);
      room[best].desc = makestring (desc);
    }
    
    if (room[best].number < 0 && roomnum >= 0) room[best].number = roomnum;
  }
  
  return (best);
}

/****************************************************************
 * same_room: Determine if a room is the "same", but allow for the room
 * name/description to change is there is a room number.
 ****************************************************************/

same_room (rm, number, name, desc)
long rm, number;
char *name, *desc;
{
  /* Perfect match, number, name, and description */
  if (number >= 0 && room[rm].number == number &&
      streq (room[rm].name, name) &&
      (!desc || !*desc || !room[rm].desc || streq (room[rm].desc, desc)))
  { return (3); }

  /* If numbers dont match, rooms cant be the same */
  if ((number >= 0 && room[rm].number >= 0) && room[rm].number != number)
  { return (0); }

  /* If name && description match, assume same */  
  if (streq (room[rm].name, name) &&
      (!desc || !*desc || !room[rm].desc || streq (desc, room[rm].desc)))
  { return (2); }
  
  /* If no numbers, assume not the same */
  if (number < 0 || room[rm].number < 0)
  { return (0); } 

  /*
   * If numbers present and match, and either name or description match,
   * then assume same room.  This is to handle the Rec Room, which is
   * link_ok, and has a stable description, but whose name changes
   * frequently.
   */

  if (streq (room[rm].name, name) ||
      *desc && room[rm].desc && streq (room[rm].desc, desc))
  { return (1); }

  /* All cases fail, assume different */
  return (0);
}

/****************************************************************
 * close_room: find a room number given an approximate name
 ****************************************************************/

# define POTSDESC \
"This would appear to be the waiting room for POTSMASTER's office. His office is one door further in. It appears to have a cardkey-style lock."

close_room (roomnam)
char *roomnam;
{ register long i, phone, result = -1;
  char pat[TOKSIZ], dat[TOKSIZ];

  /* Special case for here */
  if (result < 0 &&
      (streq (roomnam, "here") ||
       streq (roomnam, "this place") ||
       streq (roomnam, "this spot") ||
       streq (roomnam, "this room")))
  { result = hererm; }

  /*---- Try the exact match next ----*/
  if (result < 0 && (i = find_room (roomnam, NULL)) >= 0) result = i;

  /* Strip off 'the' */
  if (result < 0 && stlmatch (roomnam, "the "))
  { roomnam += 4;
    if ((i = find_room (roomnam, NULL)) >= 0) result = i;
  }

  /*---- Special case room names ----*/

  /* Special case for robots home */
  if (result < 0 &&
      ((sindex (roomnam, lcstr (myname)) ||
	sindex (roomnam, "your")) &&
       (sindex (roomnam, "office") ||
        sindex (roomnam, "place") ||
        sindex (roomnam, "pad") ||
        sindex (roomnam, "desk") ||
        sindex (roomnam, "home"))) &&
      (i = find_room (home, NULL)) >= 0)
  { result = i; }

  /* Special case for robots manual */
  if (result < 0 &&
      ((sindex (roomnam, lcstr (myname)) ||
	sindex (roomnam, "robot") ||
	sindex (roomnam, "your")) &&
       (sindex (roomnam, "manual") ||
        sindex (roomnam, "guide") ||
        sindex (roomnam, "instruction") ||
        sindex (roomnam, "program") ||
        sindex (roomnam, "source"))) &&
      (i = find_room ("Maas-Neotek Robot User's Guide", NULL)) >= 0)
  { result = i; }

  /* pattern for rec room without space */
  if (result < 0 &&
      (streq (roomnam, "recroom") ||
       streq (roomnam, "rec room") ||
       streq (roomnam, "rec")) &&
      (((phone = find_room ("Rec room phone booth", NULL)) >= 0 &&
	(i = leads_to (phone, "out")) >= 0) ||
       (i = find_room ("rec room", NULL))))
  { result = i; }

  /* pattern for town */
  if (result < 0 &&
      streq (roomnam, "town") &&
      (i = find_room ("The Town Square", NULL)) >= 0)
  { result = i; }

  /* pattern for library */
  if (result < 0 &&
      streq (roomnam, "library") &&
      (i = find_room ("Library lobby", NULL)) >= 0)
  { result = i; }

  /* pattern for potsmaster */
  if (result < 0 &&
      (stlmatch (roomnam, "potsmaster")) &&
      (i = find_room ("Anteroom", POTSDESC)) >= 0)
  { result = i; }

  /*---- Lower case the input name, and look for substrings ----*/
  if (result < 0)
  { strcpy (pat, lcstr (roomnam));
  
    for (i=0; i<rooms; i++)
    { 
      /* Match names, but prefer reachable rooms and shorter names */
      if (room[i].name && sindex (lcstr (room[i].name), pat))
      { if (result < 0 ||
            can_reach (i) && !can_reach (result))
	{ result = i; continue; }

	if (can_reach (result) && !can_reach (i))
	{ continue; }

	if (strlen (room[i].name) < strlen (room[result].name))
	{ result = i; }
      }
    }
  }

  /*---- Return result ----*/  
  if (result >= 0 && !terse)
  { fprintf (stderr, "Map:  close_room maps '%s' to '%s'\n",
	     roomnam, room[result].name);
  }

  return (result);
}

/****************************************************************
 * add_room: Add a room
 ****************************************************************/

add_room (roomnam, roomdsc)
char *roomnam, *roomdsc;
{ long rm;
  register long i;
  long roomnum;

  if (! *roomnam) return (-1);

  if (roomdsc && !*roomdsc) roomdsc = NULL;

  if (room == NULL)
  { crash_robot ("room is NULL in add_room"); }

  if ((rm = find_room (roomnam, roomdsc)) >= 0)
  { if (roomdsc && room[rm].desc == NULL)
    { room[rm].desc = makestring (roomdsc); }
    else if (roomdsc && !streq (roomdsc, room[rm].desc))
    { fprintf (stderr,
	       "Warn: changing description of %s(%ld) from '%s' to '%s'\n",
	       room[rm].name, rm, room[rm].desc, roomdsc);
      freestring (room[rm].desc);
      room[rm].desc = makestring (roomdsc);
    }

    return (rm);
  }

  if (roomdsc)
  { fprintf (stderr, "Map:  add_room (%s, '%16.16s') ==> %ld\n",
	     roomnam, roomdsc, rooms);
  }
  else
  { fprintf (stderr, "Map:  add_room (%s, NULL) ==> %ld\n", roomnam, rooms);
  }
  
  newrooms++;
  
  /* Grow array if necessary */
  if (rooms >= maxrooms) realloc_rooms (maxrooms * 6 / 5 + 50);
  
  if (smatch (roomnam, "* (#*)", roomstr) && isdigit (*room2))
  { roomnum = atoi (room2); }
  else if (smatch (roomnam, "*(#*)", roomstr) && isdigit (*room2))
  { roomnum = atoi (room2); }
  else
  { strcpy (room1, roomnam); roomnum = -1; }

  room[rooms].name = makestring (room1);

  if (desc)
  { room[rooms].desc = makestring (desc); }
  else
  { room[rooms].desc = NULL; }

  room[rooms].contents = NULL;
  room[rooms].number = roomnum;
  room[rooms].firstin = now;
  room[rooms].exits = NULL;

  return (rooms++);
}

/****************************************************************
 * add_exit: Add an exit from a room into another room
 ****************************************************************/

add_exit (from, dir, to)
long from, to;
char *dir;
{ register O_EXIT *xp;

  if (streq (dir, "home")) return (0);

  if (!isprint (*dir))
  { crash_robot ("non-printing direction \\%03o in add_exit\n", *dir); }

  /*
   * Dont learn paths to our home unless the direction is NOT home
   * and there is a direct exit from the home room to here.  Instead
   * mark the exit as a NOP.
   */

  if (to == homerm && homerm >= 0)
  { if (!exit_to (to, from)) to = from; }

  /*-------- First see if this is an old exit --------*/ 
  for (xp = room[from].exits; xp; xp = xp->next)
  { if (strcmp (dir, xp->dir) == 0)
    { if (xp->room != to && xp->room >= 0)
      { fprintf (stderr,
	         "Exit: dir '%s' from %s(%ld) now goes to %s(%ld), was %s(%ld)\n",
		 dir, room[from].name, from, room[to].name, to,
		 room[xp->room].name, xp->room);
        xp->room = to;
	newexits++;
	reach_changed++;
      }
      else if (xp->room < 0)
      { fprintf (stderr,
	        "Exit: dir '%s' from %s(%ld) now goes to %s(%ld), was unknown\n",
		 dir, room[from].name, from, room[to].name, to,
		 room[xp->room].name, xp->room);
        xp->room = to;
	newexits++;
	reach_added++;
      }
     
      return;
    }
  }
  
  newexits++;
  exits++;
  reach_added++;

  /*-------- Okay, allocate a new exit, and link it into the list --------*/
  xp = (O_EXIT *) ralloc (sizeof (O_EXIT));
  exit_sp += sizeof (O_EXIT);
  exit_ct ++;
  
  if (xp == NULL)
  { crash_robot ("malloc returns NULL in add_exit"); }
  
  xp->dir = exitstring (dir);
  xp->room = to;
  xp->next = room[from].exits;
  room[from].exits = xp;

  if (awake && !terse && from != to)
  { fprintf (stderr,
	     "Exit: dir '%s' from %s(%ld) now goes to %s(%ld)\n",
	     dir, room_name (from), from, room_name (to), to);
  }
}

/****************************************************************
 * find_path: Find a path from one room to another
 ****************************************************************/

char *find_path (from, to, desc)
long from, to;
long desc;
{ register long i;
  long extended = 0, found = 0, cnt = 0, depth = 0;
  char *result = NULL;
  static char buf[BIGBUF];
  static long maxcnt = 0;
  register O_EXIT *xp;

  if (debug)
  { fprintf (stderr, "Path: from: %s(%ld), to: %s(%ld), %s\n",
	     room[from].name, from, room[to].name, to,
	     desc ? "whole" : "next move");
  }

  if (from == to)
  { return (NULL); }

  for (i=0; i<rooms; i++) path[i].depth = -1;

  path[to].dir = NULL;  
  path[to].loc = to;
  path[to].depth = 0;
  
  extended = 1;
  for (depth=0; extended && !found && depth < rooms; depth++)
  { extended = 0;

# ifdef DEBUGBFS
    if (debug)
    { fprintf (stderr, "\nSearching depth %ld\n\n", depth); }
# endif

    /* Check each room, each edge, to extend search */
    for (i=0; i < rooms && !found; i++)
    { if (room[i].name && path[i].depth < 0)
      { for (xp = room[i].exits; xp && !found; xp = xp->next)
        { if (xp->room != i && xp->room >= 0 && xp->room < rooms &&
	      path[xp->room].depth == depth)
	  { extended++;
	    path[i].dir = xp->dir;
	    path[i].loc = xp->room;
	    path[i].depth = path[xp->room].depth + 1;
	    if (i == from) found++;
	    
# ifdef DEBUGBFS
	    if (debug)
	    { fprintf (stderr, "Adding %s(%ld), depth %ld, dir %s, to %s(%ld)\n",
			room[i].name, i, path[i].depth, path[i].dir,
			room[path[i].loc].name, path[i].loc);
	    }
# endif
	  }
	}
      }
    }
  }
  
  if (depth > maxcnt && !terse)
  { fprintf (stderr, "Find: find_path (%s) %s, max depth %ld (was %ld)\n",
	     room[to].name, found ? "wins" : "loses", depth, maxcnt);
    maxcnt = depth;
  }
  else if (debug)
  { fprintf (stderr, "Path: find_path search %s\n",
		found ? "wins" : "loses");
  }

  if (found && desc != NEXTMOVE)
  { 
    *buf = '\0';
  
    if (desc == LONGPATH)
    { sprintf (buf, "Path from %s to %s, type",
	       room[from].name, room[to].name);

      i = from;
      while (i != to && path[i].dir && i< rooms)
      { sprintf (buf, "%s 'go %s' to get to %s%s", buf, 
	           path[i].dir, room[path[i].loc].name,
	           (path[i].loc) == to ? "." : ",");

        i = path[i].loc;
      }
    }

    if (desc == MEDIUMPATH || strlen (buf) > 250)
    { sprintf (buf, "Path from %s to %s, type",
	       room[from].name, room[to].name);

      i = from;
      while (i != to && path[i].dir && i< rooms)
      { if (islandmark (room[path[i].loc].name) || path[i].loc == to)
	{ sprintf (buf, "%s %s (to %s)%s", buf, 
	           path[i].dir, room[path[i].loc].name,
	           (path[i].loc) == to ? "." : ",");
	}
	else
	{ sprintf (buf, "%s %s,", buf, path[i].dir); }

        i = path[i].loc;
      }
    }
    
    if (! *buf || strlen (buf) > 250)
    { *buf = '\0';

      i = from;
      while (i != to && path[i].dir && i< rooms)
      { sprintf (buf, "%s %s%s", buf, path[i].dir,
		 (path[i].loc) == to ? "" : ",");

        i = path[i].loc;
      }
    }
    
    result = buf;
  }
  else if (found)
  { result = path[from].dir; }    
  
  if (debug)
  { fprintf (stderr,
	     "Path: find_path returns %s\n", result ? result : "(nil)");
  }

  return (result);
}

/****************************************************************
 * explore_exit: Find an exit that is either untried or that
 * goes to a different room.  There is a 1% chance of trying an exit
 * even if it has been tried before.  This gives a small hope that
 * we will discover new exits created after a room has been explored.
 ****************************************************************/

char *explore_exit ()
{ register char *dir = NULL;
  register O_EXIT *xp;
  register long cnt=0;

  if (debug)
  { fprintf (stderr, "Expl: in explore_exit, %s(%ld)\n", here, hererm); }

  if (hererm < 0)
  { dir = stexits[randint (numexits)]; }

  else if (dir = unexp_exit (hererm, RANDOM))
  { if (debug)
    { fprintf (stderr, "Expl: unexp_exit '%s'(%ld) => <%s>\n",
	       room_name (hererm), hererm, dir);
    }
  }
  else
  { for (xp = room[hererm].exits; xp; xp = xp->next)
    { if (xp->room != hererm)
      { if (randint (++cnt) == 0) dir = xp->dir; }
    }
      
    if (cnt && debug)
    { fprintf (stderr, "Expl: old exit '%s'(%ld) => <%s>\n",
	       room_name (hererm), hererm, dir);
    }
  }

  if (debug)
  { fprintf (stderr, "Expl: explore exit from %s returns <%s>\n",
	     here, dir ? dir : "(nil)");
  }

# ifndef CRAZY
  if (dir && stlmatch (dir, "OUTPUT")) crash_robot ("bad exit in\n");
# endif

  return (dir);
}

/****************************************************************
 * unexplored_room: Return the name of an unexplored room
 ****************************************************************/

unexplored_room ()
{ register long rm;
  static long lastreset = 0;
  register char *dir;

  if (debug)
  { fprintf (stderr, "Expl: in unexplored_room, here %s, lastrm %ld\n",
		here, lastrm);
  }

  if (now > (lastreset + 36 * HOURS))
  { lastreset = now; lastrm = 0;
    fprintf (stderr, "Expl: resetting lastrm to 0 at %15.15s\n",
	     ctime (&now)+4);
  }

  /* Main loop, check rooms in order until we find an unexplored one */
  for (rm = lastrm; rm < rooms; rm++)
  { if (room[rm].name && (rm == hererm || can_reach (rm)))
    { if (dir = unexp_exit (rm, FIRST))
      { if (!terse)
	{ fprintf (stderr, "Expl: unexplored_room returns %s(%ld), <%s>.\n",
		   room_name (rm), rm, dir);
	}

	lastrm = rm;
	return (rm);
      }
    }

    /* Check for pending IO, bail out if necessary */	  
    if (rm > lastrm && charsavail (fmud))
    { fprintf (stderr, "Expl: %s out at %ld (of %ld) to service IO\n",
		 "unexplored_room bails", rm, rooms);
      lastrm = rm;
      return (-1);
    }
  }

  if (!terse)
  { fprintf (stderr, "Expl: unexplored_room loses\n"); }

  return (-1);
}

/****************************************************************
 * unexp_exit: Return the name of an unexplored exit.  If pickrandom
 * is true, chose randomly from all unexplorted exits in the room.
 ****************************************************************/

char *unexp_exit (rm, pickrandom)
register long rm;
long pickrandom;
{ register long i, cnt=0;
  register char *dir = NULL, *ep, *w;
  register O_EXIT *xp;
  long type = 0;
  char buf[MSGSIZ];

  /* Check all standard exits exit for one that we have not tried */
  for (i=0; i<numexits; i++)
  { if (leads_to (rm, stexits[i]) < 0)
    { if (pickrandom)
      { if (randint (++cnt) == 0) { dir = stexits[i]; type=1; } }
      else
      { return (stexits[i]); }
      
    }
  }

  /* If using room descriptions, check all unusual words in desc */
  if (usedesc && room[rm].desc && room[rm].desc[0])
  { strcpy (buf, lcstr (room[rm].desc));

    /* Check every exit for one that we have not tried */
    for (ep=buf; ep && *ep && (w = car (ep)) && *w; ep = cdr (ep))
    { check_id (w);

      if (not_exit (w)) continue;

      /* Okay, this room has an unexplored exit */
      if (leads_to (rm, w) < 0)
      { if (pickrandom)
        { if (randint (++cnt) == 0) { dir = exitstring (w); type=2; } }
	else
	{ return (exitstring (w)); }
      }
    }
  }

  /* Check for exits in list that are unmarked */
  for (xp = room[rm].exits; xp; xp = xp->next)
  { if (xp->room < 0)
    { if (pickrandom)
      { if (randint (++cnt) == 0) { dir = xp->dir; type=3; } }
      else
      { return (xp->dir); }
    }
  }

  if (debug)
  { fprintf (stderr, "Expl: unexp_exit type %ld '%s'(%ld) => <%s>\n",
		type, room_name (hererm), hererm, dir);
  }

  check_id (dir);

  return (dir);
}

/****************************************************************
 * leads_to: Where would 'dir' takes us from from?
 ****************************************************************/

leads_to (from, dir)
long from;
char *dir;
{ register O_EXIT *xp;

  for (xp = room[from].exits; xp; xp = xp->next)
  {
    /* Check for bogus map */
    if (xp == xp->next ||
	!isprint (xp->dir[0]) ||
	xp->dir[1] && !isprint (xp->dir[1]))
    { fprintf (stderr, "Dir[0] = '%03o', Dir[1] = '%03o'\n",
		xp->dir[0] & 255, xp->dir[1] & 255);
      write_players (plyfile);
      crash_robot ("bogus direction entry in leads_to (%ld, %s)", from, dir);
    }

    if (streq (dir, xp->dir))
    { if (debug > 1)
      { fprintf (stderr, "Lead: from %s(%ld), '%s' --> %s(%ld)\n",
		room[from].name, from, dir, room[xp->room].name, xp->room);
      }
      return (xp->room);
    }
  }

  return (-1);
}

/****************************************************************
 * exit_to: Is there an exit from one room into the other?
 ****************************************************************/

char *exit_to (from, to)
register long from, to;
{ register O_EXIT *xp;

  for (xp = room[from].exits; xp; xp = xp->next)
  { if (xp->room == to)
    { if (debug)
      { fprintf (stderr, "Exit: from %s(%ld), '%s' --> %s(%ld)\n",
		room[from].name, from, xp->dir, room[to].name, to);
      }
      return (xp->dir);
    }
  }

  return (NULL);
}

/****************************************************************
 * hashf: Compress a string down to an 6 character identifier
 ****************************************************************/

char *fivebit = "abcdefghijklmnopqrstuvwxyz012345";
long modulus = 1073741789;
long mult = 269;

char *hashf (str)
char *str;
{ unsigned long result = 0;
  static char buf[8];
  char *t = buf;

  while (*str)
  { result = ((result * mult) + *str++) % modulus; }
  
  do
  {
    *t++ = fivebit[result & 037];
    result >>= 5;
  } while (result > 0);
  
  *t = '\0';
  
  return (t = buf);
}

/****************************************************************
 * room_match: returns T if two roomnames are the same room
 ****************************************************************/

room_match (rm1, rm2)
char *rm1, *rm2;
{ long r1, r2;

  if (strcmp (rm1, rm2) == 0)
  { return (1); }

  if ((r1 = find_room (rm1, NULL)) >= 0 &&
      (r2 = find_room (rm2, NULL)) >= 0 &&
       r1 == r2)
  { return (1); }

  return (0);
}

/****************************************************************
 * islandmark: A list of landmark rooms
 ****************************************************************/

char *landmarks[] = {
  "Library lobby", 
  "The Town Square",
  "Rec Room",
  "Emailboxes Unlimited lobby",
  "Amusement Park Entrance",
  "Mine Entrance",
  "cyberware lobby",
  "Heather Park Station",
  "College Row",
  "Boston Common",
  "Building 3 (First Floor, SouthEast corner)",
  "Central Station, Inbound",
  "Central Station, Outbound",
  "Cirdan's Pass",
  "Corner of Forbes and Murray",
  "MIT building",
  "Harvard Square",
  "Harvard Station Turnstiles",
  "Lobby 1 (First Floor)",
  "Main Academic Quad",
  "The @ commands",
  NULL
};

islandmark (loc)
char *loc;
{ register char **rp;

  for (rp=landmarks; *rp; rp++)
  { if (streq (loc, *rp)) return (1); }

  return (0);
}

/****************************************************************
 * do_explore: Explore the universe
 ****************************************************************/

do_explore ()
{ long oldplace;
  char *dir;
  long tries = 0;
  long eroom;

  /* Explore */
  if (exploring &&
      !nextwait &&
      ! *pagedby &&
      ! *typecmd &&
      !dead &&
      !takingnotes)
  { 
    /* Always try the 'out' exit first */
    if (hererm >= 0 && leads_to (hererm, "out") < 0)
    { movemud ("out"); hangaround (1); return (1); }
    
    /* If there is an unexplored exit here, just try it */
    if (dir = unexp_exit (hererm, RANDOM))
    { if (debug)
      { fprintf (stderr, "Expl: unexp_exit '%s'(%ld) => <%s>\n",
		 room_name (hererm), hererm, dir);
      }
      movemud (dir); hangaround (speed); return (1);
    }
    
    /* If not at home, go there with some probability */
    if (hererm != homerm &&
        (now - room[homerm].lastin) > (randint (10) + 5) * MINUTES)
    { if (!terse) fprintf (stderr, "Expl: getting homesick after %ld seconds\n",
			   now - room[homerm].lastin);
      movemud ("home"); hangaround (speed); return (1);
    }

    /* 66% chance we will pick earliest unexplored room and go explore it */
    if (randint (100) < 66)
    { 
      if ((eroom = unexplored_room ()) < 0)
      { do { eroom = randint (rooms); } while (!room[eroom].name); }

      /* Give a chance to process messages */
      hangaround (speed);
      if (*pagedby || nextwait || *typecmd) return (1);
      
      /* Now make a goal to get to the next room to explore */
      if (eroom != hererm &&
	  (find_path (hererm, eroom, NEXTMOVE) ||
	   homerm == eroom ||
	   find_path (homerm, eroom, NEXTMOVE)))
      { strcpy (pagedby, "<EXPLORING>");
	pagedfrom = hererm;
	pagedto = eroom;
	pagedat = now;
      
	disengage (room[pagedto].name);
      
        if (!terse)
	{ fprintf (stderr, "\n---- %15.15s ---- Heading off to %s ----\n\n",
		   ctime (&pagedat) + 4, room[pagedto].name);
        }
	else
	{ fprintf (stderr, "Expl: %15.15s, heading to %s(%ld)\n",
		   ctime (&pagedat) + 4, room[pagedto].name, pagedto);
        }

	return (1);
      }
    }

    /* Else just random walk from here */
    if (dir = explore_exit ())
    { if (!terse)
      { fprintf (stderr, "Expl: unexp_exit '%s'(%ld) => <%s>\n",
		 room_name (hererm), hererm, dir);
      }
      movemud (dir); hangaround (speed); return (1);
    }
    
    /* Stuck, go home */
    if (hererm != homerm)
    { fprintf (stderr, "Expl: stuck in '%s'(%ld), going home\n",
		room_name (hererm), hererm);
      movemud ("home"); hangaround (speed); return (1);
    }
    
    /* Stuck in home??? Fail */
    fprintf (stderr, "Warn: stuck in home: '%s'(%ld)\n", here, hererm);
    return (0);
  }

  return (0);
}

/****************************************************************
 * disengage: We are leaving, break off any converstation
 ****************************************************************/

disengage (dest)
char *dest;
{ long pl;

  /* Count present */
  alone = 0;
  player[me].active = player[me].present = 0;

  for (pl = 0; pl<players; pl++)
  { if (player[pl].active && player[pl].present) alone++; }


  /* Must disengage from a conversation */    
  if (dest && ((now = time (0)) < speaktime + 60) && *speaker)
  { switch (randint (4))
    { case 0: command ("\"Excuse me, %s, I feel like exploring %s.",
			speaker, dest);
	      break;
      case 1: command ("\"Pardon me, %s, I must be going to %s",
			speaker, dest);
	      break;
      case 2: command ("\"Goodbye, %s, I'm heading to %s",
			speaker, dest);
	      break;
      case 3: command ("\"Goodbye, %s, I feel like taking a walk to %s",
			speaker, dest);
	      break;
    }
  }

  /* Might say good bye just to be sociable */    
  else if (dest && (hererm == homerm || !quiet && dest && randint (100) < 50))
  { switch (randint (4))
    { case 0: command ("\"I feel like exploring %s.", dest); break;
      case 1: command ("\"Pardon me, I must be going to %s.", dest); break;
      case 2: command ("\"Goodbye, I'm heading off to %s.", dest); break;
      case 3: command ("\"I feel like taking a walk to %s", dest); break;
    }
  }

  /* Must disengage from a conversation */    
  else if (((now = time (0)) < speaktime + 60) && *speaker)
  { switch (randint (4))
    { case 0: command ("\"Excuse me, %s, %s.",
		       speaker, "I feel like doing some exploring");
	      break;
      case 1: command ("\"Pardon me, %s, I must be going.", speaker);
	      break;
      case 2: command ("\"Bye, %s.", speaker);
	      break;
      case 3: command ("\"Bye, %s, I feel like taking a walk.", speaker);
	      break;
    }
  }

  /* Might say good bye just to be sociable */    
  else if (!quiet && randint (100) < 50)
  { switch (randint (4))
    { case 0: command ("\"I feel like doing some exploring."); break;
      case 1: command ("\"Pardon me, I must be going."); break;
      case 2: command ("\"Goodbye."); break;
      case 3: command ("\"I feel like taking a walk."); break;
    }
  }
}

/****************************************************************
 * inlibrary: A list of valid locations to page Julia
 ****************************************************************/

char *librooms[] = {
  "Library lobby", 
  "Library Reception Desk", 
  "Circulating Room", 
  "Wizard's Room", 
  "Reference Room", 
  "Short Hallway in the Library", 
  "Library trash bin", 
  "Landing on Stairway", 
  "Oxford English Dictionary Room", 
  "Encyclopedia Collection", 
  "Library Computer Services Office", 
  "Library Interactive Fiction Office", 
  "Library Research Office", 
  "Gloria's Office", 
  NULL
};

inlibrary (loc)
char *loc;
{ register char **rp;

  for (rp=librooms; *rp; rp++)
  { if (streq (loc, *rp)) return (1); }

  return (0);
}

/****************************************************************
 * not_exit: A stop list of exit words from descriptions
 ****************************************************************/

char *exit_stop[] = {
  "a", "about", "against", "all", "an", "and", "are", "as", "at", "be",
  "but", "by", "can", "continues", "each", "exits", "few", "filled",
  "for", "from", "go", "goes", "has", "have", "here", "if", "in",
  "include", "into", "is", "it", "it's", "just", "large", "lead",
  "leading", "leads", "like", "looking", "looks", "lots", "many", "now",
  "of", "on", "one", "other", "runs", "see", "seems", "several", "small",
  "so", "some", "standing", "surrounded", "that", "the", "them", "there",
  "there's", "this", "to", "was", "where", "with", "you", "your", "home",
  NULL
};

not_exit (loc)
char *loc;
{ register char **ep;

  for (ep=exit_stop; *ep; ep++)
  { if (streq (loc, *ep)) return (1); }

  return (0);
}

/****************************************************************
 * exit_to_query: Print rooms and exits that lead directly to room
 ****************************************************************/

exit_to_query (roomnam, name)
char *roomnam, *name;
{ register long rm, i;
  long printed = 0;
  register O_EXIT *xp;


  fprintf (stderr, "Exit: query_to (%s), for %s\n", roomnam, name);

  if ((rm = close_room (roomnam, NULL)) < 0)
  { reply ("\"I have never heard of %s, %s.", roomnam, name);
    return;
  }
  
  for (i=0; i<rooms; i++)
  { if (room[i].name)
    { for (xp = room[i].exits; xp; xp = xp->next)
      { if (xp->room == rm)
        { if (!printed++)
	  {  reply ("\"Here are the exits to %s, %s:",
		    room_name (rm), name);
	  }
	
	  if (printed > 15 && alone > 2)
	  { reply ("\"There are more exits, but %s, %s.",
		   " it's too crowded here to go on", name);
	    return;
	  }
	
	  reply (":<%s> from %s.", xp->dir, room_name (i));
	  break;
	}
      }
    }
  }
  
  if (!printed)
  { reply ("\"I don't know any exits to %s, %s.", room_name (rm), name); }
  else
  { reply (":done."); }
}

/****************************************************************
 * exit_from_query: Print exits and their rooms for a given room
 ****************************************************************/

exit_from_query (roomnam, name)
char *roomnam, *name;
{ register long rm, i;
  long printed = 0, tried=0, total=0, unknown=0;
  register O_EXIT *xp;

  if ((rm = close_room (roomnam, NULL)) < 0)
  { reply ("\"I have never heard of %s, %s.", roomnam, name);
    return;
  }
  
  /* Count exits */
  for  (xp = room[rm].exits; xp; xp = xp->next)
  { if (xp->room == rm)		{ tried++; }
    else if (xp->room < 0)	{ unknown++; }
    else			{ total++; }
  }

  /* If too many exits, just give count */
  if ((total + unknown) > 50 || ((total + unknown) > 15 && alone > 2))
  { if (unknown > 0)
    { reply("\"There are %ld exits from %s, plus %ld more I've heard about, %s.",
	    total, room_name (rm), unknown, name);
    }
    else
    { reply ("\"There are %ld exits from %s, %s.",
	     total, room_name (rm), name); }
    
    return;
  }

  /* List them all (should really sort by destination) */
  for  (xp = room[rm].exits; xp; xp = xp->next)
  { if (xp->room != rm)
    { if (!printed++)
      { reply ("\"Here are the %ld exits for %s, %s:",
		total+unknown, room_name (rm), name);
      }

      if (printed > 15 && alone > 2)
      { reply ("\"There are more exits, but %s, %s.",
	       " it's too crowded here to go on", name);
	break;
      }

      if (xp->room >= 0 && xp->room < rooms)	
      { reply (":<%s> goes to %s.", xp->dir, room_name (xp->room)); }
      else
      { reply (":<%s> is an exit I've heard about.", xp->dir); }
    }
  }
  
  if (!printed)
  { reply ("\"I don't know any exits from %s, %s.", room_name (rm), name); }
  else
  { reply (":done."); }
}

/****************************************************************
 * room_query: Print all rooms matching some pattern
 ****************************************************************/

room_query (pat, name, full)
char *pat, *name;
long full;
{ register long rm;
  long printed = 0;

  /* First do rooms */
  if (!full)
  { for (rm = 0; rm < rooms; rm++)
    { if (room[rm].name && sindex (lcstr (room[rm].name), pat))
      { if (!printed++)
        { reply ("\"Here are the rooms that match '%s', %s:", pat, name); }
	
        if (printed > 15 && alone > 2)
        { reply ("\"There are more rooms, but %s, %s.",
	         " it's too crowded here to go on", name);
  	  break;
        }

	if (can_reach (rm))	
	{ reply (": %s", room_name (rm)); }
	else
	{ reply (": %s (unreachable)", room_name (rm)); }
      }
    }
  }

  /* If no rooms, check descriptions */
  if (!printed || full)
  { for (rm = 0; rm < rooms; rm++)
    { if (room[rm].name && sindex (lcstr (room[rm].name), pat) ||
	  room[rm].desc && sindex (lcstr (room[rm].desc), pat) ||
	  room[rm].contents && sindex (lcstr (room[rm].contents), pat))
      { if (!printed++)
        { reply ("\"Here are the rooms whose descriptions match '%s', %s:",
		 pat, name); }
	
          if (printed > 15 && alone > 2)
          { reply ("\"There are more rooms, but %s, %s.",
	           " it's too crowded here to go on", name);
	    break;
          }
	
        reply (":");
        reply (": %s", room_name (rm));

        if (room[rm].desc && room[rm].desc[0])
        { reply (": description: %s", room[rm].desc); }

        if (room[rm].contents && room[rm].contents[0])
        { reply (": contents: %s", room[rm].contents); }
      }
    }
  }

  
  if (!printed)
  { reply ("\"I don't know any rooms that match '%s', %s.",
	   pat, name);
  }
  else
  { reply (":done."); }
}

/****************************************************************
 * can_reach: Returns true if there is a path from the robots
 *	home to that room.
 ****************************************************************/

can_reach (rm)
long rm;
{ register long i;
  long extended = 0, found = 0, cnt = 0, depth = 0;
  static long maxcnt = 0;
  register O_EXIT *xp;

  /* if no new exits or changed exits, then look up bit */
  if (!reach_added && !reach_changed)
  { return (ROOM_GET (rm, RM_REACH)); }

  /* If no exits have been changed, then reachable rooms are valid */
  if (!reach_changed && ROOM_GET (rm, RM_REACH))
  { return (RM_REACH); }

  /*---- No more easy cases, have to search graph ----*/

  /* If exits have changed, must clear all bits first */
  if (reach_changed)
  { for (i=0; i<rooms; i++) ROOM_CLR (i, RM_REACH|RM_SEARCHED);
    reach_changed = 0;
  }

  /* If exits have been added, must search rooms again */
  for (i=0; i<rooms; i++) ROOM_CLR (i, RM_SEARCHED);

  /* Can always reach home */
  ROOM_SET (homerm, RM_REACH);

  extended = 1;
  for (depth=0; extended && !found && depth < rooms; depth++)
  { extended = 0;

# ifndef REAL
    if (debug)
    { fprintf (stderr, "\nReach, searching depth %ld\n\n", depth); }
# endif

    /* Check each room, each edge, to extend search */
    for (i=0; i < rooms && !found; i++)
    { 
      /* If a room that we can reach but have not searched */
      if (ROOM_GET (i, RM_REACH|RM_SEARCHED) == RM_REACH)
      { ROOM_SET (i, RM_SEARCHED);

	for (xp = room[i].exits; xp; xp = xp->next)
        { if (xp->room != i && !ROOM_GET (xp->room, RM_REACH))
	  { extended++;
	    ROOM_SET (xp->room, RM_REACH);
	    if (xp->room == rm) found++;
	    
# ifndef REAL
	    if (debug)
	    { fprintf (stderr, "Adding %s(%ld), depth %ld\n",
			room[xp->room].name, xp->room, depth);
	    }
# endif
	  }
	}
      }
    }
  }
  
  /* If we searched exhaustively, remember that */
  if (!extended)
  { reach_added = 0;
    if (debug)
    { fprintf (stderr, "Reac: search exhausted for %s(%ld)\n",
		room_name (rm), rm);
    }
  }
  
  /* The RM_REACH bit for room 'rm' is now valid */
  return (ROOM_GET (rm, RM_REACH));
}

/****************************************************************
 * check_id: If not all characters in string are printing, crash
 ****************************************************************/

check_id (str)
register char *str;
{
  while (*str)
  { if (!isprint (*str)) crash_robot ("bad char '\\%03o' in str\n", *str);
    str++;
  }
}
