/****************************************************************
 * mn6cvt.c: Convert Pre-6.0 Maas-Neotek robot data files to
 *	version 6.0 format.
 *
 * USAGE
 *	% mn6cvt < old.map > new.map
 *	% mn6cvt < old.players > new.players
 *
 * HISTORY
 * 18-Feb-90  Michael Mauldin (mlm) at Carnegie-Mellon University
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

/****************************************************************
 * main:
 ****************************************************************/

main ()
{ char buf[BUFSIZ];

  if (fgets (buf, BUFSIZ, stdin))
  { if (stlmatch (buf, "Gloria Map File: "))
    { convert_map (buf); }

    else if (stlmatch (buf, "Gloria Players File: "))
    { convert_players (buf); }

    else
    { fprintf (stderr,
	       "Error: input file is not a pre-6.0 Maas-Netotek file.\n");
      exit (1);
    }
  }
  
  else
  { perror ("stdin");
    exit (1);
  }
  
  exit (0);
}

/****************************************************************
 * write_map: Write current state of map
 ****************************************************************/

convert_map (head)
char *head;
{ long rooms, now, rn, number, dst;
  char name[MSGSIZ], buf[BUFSIZ], dir[MSGSIZ];

  if (sscanf (head, "Gloria Map File: %ld rooms", &rooms) == 1)
  { printf ("Maas-Neotek Map file: %s\n\n", VERSION); }
  else
  { fprintf (stderr, "Bogus map header: head", head);
    exit (1);
  }

  now = time (0);

  printf ("K:%d rooms\n", rooms);
  printf ("T:%s", ctime (&now));
  printf ("H:unknown\n");
  printf ("P:4201\n");
  printf ("I:unknown\n");

  while (fgets (buf, BUFSIZ, stdin))
  { if (isdigit (*buf) &&
        sscanf (buf, "%ld %ld %[^\n]", &rn, &number, name)) 
    { printf ("\nR:%ld %s\n", rn, name);
      printf ("F:%ld 0 0 0 0 0 0 0\n", number);
    }
    else if (stlmatch (buf, "Desc: ") && buf[6] != '\n')
    { printf ("D:%s", buf+6); }
    else if (stlmatch (buf, "Cont: ") && buf[6] != '\n')
    { printf ("C:%s", buf+6); }
    else if (*buf == '\t' && sscanf (buf, "\t%[^\t\n]\t%ld", dir, &dst))
    { if (dst == rn)
      { printf ("B:%s\n", dir); }
      else
      { printf ("E:%ld %s\n", dst, dir);
      }
    }
    else if (*buf && *buf != '\n')
    { fprintf (stderr, "Warning, ignoring: %s", buf);
    }
  }
}

/****************************************************************
 * write_players: Write current state of players
 ****************************************************************/

convert_players (head)
char *head;
{ long players, now, number;
  long saw, spoke, heard, place, active, gave, dona, total, kill, flags;
  int okay = 0;

  char name[MSGSIZ], buf[BUFSIZ], dir[MSGSIZ], dialog[1024];

  if (sscanf (head, "Gloria Players File: %ld players", &players) == 1)
  { printf ("Maas-Neotek Players file: %s\n\n", VERSION); }
  else
  { fprintf (stderr, "Bogus map header: head", head);
    exit (1);
  }

  now = time (0);

  printf ("K:%d players\n", players);
  printf ("T:%s", ctime (&now));
  printf ("H:unknown\n");
  printf ("P:4201\n");
  printf ("I:unknown\n");

  while (fgets (buf, BUFSIZ, stdin))
  { if (*buf == '"' &&
    	sscanf (buf,
	     "\"%[^\"]\" %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %lx %[^\n]",
		name, &number, &saw, &spoke, &heard, &place, &active,
		&gave, &dona, &total, &kill, &flags, dialog) == 13)
    { printf ("\nN:%s\n", name);
      printf
	("F:%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
	  number, total, 0,  flags, active, dona,  gave, heard, kill, 
	  0, 0, place,  saw, spoke, 0, 0);
      if (dialog[1])
      { printf ("S:%s\n", dialog+1);
        okay = 1;
      }
    }
    else if (*buf == '"')
    { okay = 0;
      fprintf (stderr, "Error, bad player name: %s", buf);
    }
    else if (okay && stlmatch (buf, "Desc: ") && buf[6] != '\n')
    { printf ("D:%s", buf+6); }
    else if (okay && stlmatch (buf, "Inve: ") && buf[6] != '\n')
    { printf ("C:%s", buf+6); }
    else if (*buf && *buf != '\n')
    { fprintf (stderr, "Warning, ignoring: %s", buf);
    }
  }
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
