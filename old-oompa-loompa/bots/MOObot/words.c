/****************************************************************
 * words.c: Common lexical functions for TinyMUD automata
 *
 * HISTORY
 * 07-Jul-92  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Thirteenth prodigal release.  Added raw_car
 *
 * 04-Jun-91  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Twelfth surgical release.  Pulled these lexical functions
 *	from other files.
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

# define last_char(S) ((S)[strlen(S)-1])

/* Results from star matcher */
char res1[BUFSIZ], res2[BUFSIZ], res3[BUFSIZ], res4[BUFSIZ];
char res5[BUFSIZ], res6[BUFSIZ], res7[BUFSIZ], res8[BUFSIZ];
char *result[] = { res1, res2, res3, res4, res5, res6, res7, res8 };

char room1[BUFSIZ], room2[BUFSIZ], room3[BUFSIZ], room4[BUFSIZ];
char *roomstr[] = { room1, room2, room3, room4 };

char tmp1[BUFSIZ], tmp2[BUFSIZ], tmp3[BUFSIZ], tmp4[BUFSIZ];
char tmp5[BUFSIZ], tmp6[BUFSIZ], tmp7[BUFSIZ], tmp8[BUFSIZ];
char *tmpstr[] = { tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8 };

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
 * ematch: Match end of string
 ****************************************************************/

ematch (big, small)
register char *big, *small;
{ register long blen = strlen (big), slen = strlen (small);

  return (streq (big+blen-slen, small));
}

/****************************************************************
 * raw_car: Returns a pointer to a static containing the first
 * word of a string.
 ****************************************************************/

char *raw_car (str)
register char *str;
{ static char buf[MSGSIZ];
  register char *s = buf;

  if (!str) return (NULL);
  
  while (*str && isspace (*str)) str++;
  while (*str && (!isspace (*str) || !isspace (*str) && !isspace (str[1]))) 
  { *s++ = *str++; }
  *s = '\0';

  return (s = buf);
}

/****************************************************************
 * car: Returns a pointer to a static containing the first
 * word of a string.
 ****************************************************************/

# define isword(C) ((C) && (isalnum (C) || index ("'-_", (C))))

char *car (str)
register char *str;
{ static char buf[MSGSIZ];
  register char *s = buf;

  if (!str) return (NULL);
  
  if (stlmatch (str, "(>")) return ("(>");

  while (*str && !isword (*str)) str++;
  while (*str && (isword (*str) || !isspace (*str) && isword (str[1]))) 
  { *s++ = *str++; }
  *s = '\0';

  return (s = buf);
}

/****************************************************************
 * cdr: all but the first word
 ****************************************************************/

char *cdr (str)
register char *str;
{ register char *p = str;

  if (!str) return (NULL);

  if (stlmatch (str, "(>"))
  { str += 2;
    while (*str && !isword (*str))  str++;
    if (!*str) return (NULL);
    if (!*str) return (NULL);
    return (str);
  }
  
  else
  /* Skip initial white space */
  while (*str && !isword (*str))  str++;

  if (!*str) return (NULL);

  /* Skip over one word */  
  while (*str && (isword (*str) || !isspace (*str) && isword (str[1])))
  { str++; }

# ifndef TESTED_ONLY
  /* Skip white space */
  while (*str && !isword (*str)) str++;
# endif

  /* Anything left? */
  if (!*str) return (NULL);
  
  /* return pointer to second word */  
  return (str);
}

/****************************************************************
 * strip_robot_name: Strip robots name off the beginning and
 *	end of a string
 ****************************************************************/

char *strip_robot_name (str)
char *str;
{ static char sbuf[MSGSIZ];
  char buf[MSGSIZ];
  register char *s, *t;

  if (!str || !*str)
  { strcpy (sbuf, ""); return (s = sbuf); }

  /* Strip robots name off the front */
  if (strfoldeq (car (str), myname))
  { s = cdr (str);

    if (!s || !*s)
    { strcpy (sbuf, ""); return (s = sbuf); }

    while (*s && (isspace (*s) || index (",.:;'", *s))) s++;
    strcpy (buf, s);
  }
  else
  { strcpy (buf, str); }
  
  /* Strip robots name off the back */  
  if (strfoldeq (last (buf), myname))
  { s = buf + strlen (buf);
    while (s > buf && isspace (s[-1])) s--;
    while (s > buf && !isspace (s[-1])) s--;
    while (s > buf && (isspace (s[-1]) || index (".,:;'?!", s[-1]))) s--;
    *s = '\0';
  }
  
  strcpy (sbuf, buf);
  s = sbuf;

  return (s);  
}

/****************************************************************
 * pn_subs: Substitute first and second pronouns
 ****************************************************************/

char *pn_subs (str)
char *str;
{ register char *s, *t, *u;
  static char buf[BIGBUF], word[TOKSIZ];

  *buf = '\0';

  for (s=str; s && *s; s = cdr (s))
  { if (*buf) strcat (buf, " ");
    strcpy (word, lcstr (car (s)));
    
    if (streq (word, "you"))		strcat (buf, "I");
    else if  (streq (word, "your"))	strcat (buf, "my");
    else if  (streq (word, "you're"))	strcat (buf, "I am");
    else if  (streq (word, "you've"))	strcat (buf, "I've");
    else if  (streq (word, "yours"))	strcat (buf, "mine");
    else if  (streq (word, "yourself"))	strcat (buf, "myself");
    else if  (streq (word, "i"))	strcat (buf, "you");
    else if  (streq (word, "i'm"))	strcat (buf, "you're");
    else if  (streq (word, "i've"))	strcat (buf, "you've");
    else if  (streq (word, "am"))	strcat (buf, "are");
    else if  (streq (word, "me"))	strcat (buf, "you");
    else if  (streq (word, "my"))	strcat (buf, "your");
    else if  (streq (word, "his"))	strcat (buf, "your");
    else if  (streq (word, "her"))	strcat (buf, "your");
    else if  (streq (word, "mine"))	strcat (buf, "yours");
    else if  (streq (word, "myself"))	strcat (buf, "yourself");
    else if  (streq (word, "we"))	strcat (buf, "you");
    else if  (streq (word, "us"))	strcat (buf, "you");
    else if  (streq (word, "ours"))	strcat (buf, "yours");
    else if  (streq (word, "ourselves"))strcat (buf, "yourselves");
    else				strcat (buf, car (s));
  }
  
  if (smatch (buf, "*to I*", tmpstr) && !isalpha (*tmp2))
  { sprintf (buf, "%sto me%s", tmp1, tmp2); }

  if (smatch (buf, "*I were*", tmpstr))
  if (smatch (buf, "*I are*", tmpstr))
  { sprintf (buf, "%sI am%s", tmp1, tmp2); }

  if (smatch (buf, "*I were*", tmpstr))
  { sprintf (buf, "%sI was%s", tmp1, tmp2); }

  if (smatch (buf, "*you was*", tmpstr))
  { sprintf (buf, "%syou were%s", tmp1, tmp2); }

  if (smatch (buf, "*you am*", tmpstr))
  { sprintf (buf, "%syou are%s", tmp1, tmp2); }

  t = buf;
  return (t);
}

/****************************************************************
 * third_subs: Substitute first and second pronouns
 ****************************************************************/

char *third_subs (str, name)
char *str, *name;
{ register char *s, *t, *u;
  static char ibuf[BIGBUF],  buf[BIGBUF], word[TOKSIZ];

  strcpy (ibuf, str);
  *buf = '\0';

  if (smatch (buf, "*you are*", tmpstr))
  { sprintf (ibuf, "%s%s is%s", tmp1, male ? "he" : "she", tmp2); }

  if (smatch (buf, "*you were*", tmpstr))
  { sprintf (ibuf, "%s%s was%s", tmp1, male ? "he" : "she", tmp2); }

  if (smatch (buf, "*to you*", tmpstr))
  { sprintf (ibuf, "%sto %s%s", tmp1, male ? "him" : "her", tmp2); }

  for (s=ibuf; s && *s; s = cdr (s))
  { if (*buf) strcat (buf, " ");
    strcpy (word, lcstr (car (s)));
    
    if (streq (word, "you"))		strcat (buf, myname);
    else if  (streq (word, "your"))	strcat (buf, male ? "his" : "her");
    else if  (streq (word, "you're"))	{ strcat (buf, male ? "he" : "she");
					  strcat (buf, " is"); }
    else if  (streq (word, "you've"))	{ strcat (buf, male ? "he" : "she");
					  strcat (buf, " has"); }
    else if  (streq (word, "yours"))	strcat (buf, male ? "his" : "hers");
    else if  (streq (word, "yourself"))	strcat (buf, male ? "himself" :
							    "herself");
    else if  (streq (word, "i"))	strcat (buf, name);
    else if  (streq (word, "am"))	strcat (buf, "is");
    else if  (streq (word, "i'm"))	{ strcat (buf, name);
					  strcat (buf, " is"); }
    else if  (streq (word, "i've"))	{ strcat (buf, name);
					  strcat (buf, " has"); }
    else if  (streq (word, "me"))	strcat (buf, name);
    else if  (streq (word, "my"))	strcat (buf, malep(name) ? 
					        "his" : "her");
    else if  (streq (word, "mine"))	strcat (buf, malep(name) ? 
					        "his" : "hers");
    else if  (streq (word, "myself"))	strcat (buf, name);
    else				strcat (buf, car (s));
  }

  t = buf;
  return (t);
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
 * malep: Guess gender from name
 ****************************************************************/

char *male_fn[] = { "aaron", "abe", "abraham", "adam", "al", "alan",
"albert", "alec", "alex", "alexander", "alfred", "allan", "allen",
"alvin", "andre", "andrew", "andy", "anthony", "archie", "arnie",
"arnold", "art", "arthur", "artie", "barney", "bart", "bartholomew",
"ben", "benjamin", "bennie", "benny", "bernard", "bert", "bill", "billy",
"bob", "bobby", "brad", "bradford", "bradley", "brian", "bruce", "burt",
"cal", "calvin", "cam", "cameron", "carl", "carlos", "cary", "cecil",
"chad", "charles", "charley", "charlie", "chet", "christopher", "chuck",
"clarence", "clark", "claude", "clay", "clayton", "cliff", "clifford",
"craig", "curt", "cyrus", "dale", "dan", "daniel", "danny", "darrell",
"darren", "darryl", "dave", "david", "dean", "denis", "dennis", "denny",
"dick", "don", "donald", "donnie", "donny", "doug", "douglas", "drew",
"duane", "dustin", "dwayne", "dwight", "earl", "ed", "eddie", "eddy",
"edgar", "edward", "eli", "elliot", "elmer", "eric", "erick", "erik",
"ernest", "eugene", "felix", "frank", "fred", "freddie", "freddy",
"frederic", "frederick", "fredric", "gary", "gene", "geoffrey", "george",
"gerald", "gereld", "gerry", "glen", "glenn", "goeff", "gordon", "grant",
"greg", "gregory", "gus", "guy", "hal", "hank", "harold", "harry",
"harvey", "henry", "herb", "herbert", "herman", "hiram", "hubert", "hugh",
"irv", "irving", "isaac", "ishmael", "ivan", "jack", "jacob", "jake",
"james", "jason", "jay", "jeff", "jefferey", "jeffrey", "jeremy",
"jerold", "jerome", "jerry", "jesse", "jethro", "jim", "jimmie", "jimmy",
"joe", "joel", "joey", "john", "johnnie", "johnny", "jon", "jonathan",
"joseph", "karl", "keith", "ken", "kenneth", "kenny", "kent", "kevin",
"kim", "kirk", "kurt", "kyle", "larry", "laurence", "lawrence", "lennie",
"lenny", "leo", "leonard", "leroy", "les", "lester", "lew", "lewis",
"lloyd", "lou", "louis", "luke", "marc", "mark", "martin", "marv",
"marvin", "matt", "matthew", "merv", "mervin", "michael", "mike", "nate",
"nathan", "nathanial", "neal", "neale", "ned", "neil", "nicholas", "nick",
"nicolas", "norm", "norman", "oliver", "oscar", "otto", "patrick", "paul",
"pete", "peter", "phil", "philip", "phillip", "ralph", "randall",
"randolph", "ravi", "ray", "reuben", "rex", "richard", "rick", "ricky",
"ritch", "rob", "robby", "robert", "rod", "roger", "ron", "ronald",
"ronny", "ross", "roy", "ruben", "rudolph", "rudy", "russ", "russell",
"salem", "salvador", "sam", "samuel", "scot", "scott", "sean", "seth",
"shawn", "sherman", "sid", "sidney", "simon", "stan", "stanley",
"stephen", "steve", "steven", "stewart", "stu", "stuart", "sydney", "ted",
"teddy", "terence", "theodore", "thomas", "tim", "timothy", "todd", "tom",
"tommie", "tommy", "tony", "vic", "victor", "vincent", "wade", "wallace",
"wally", "walt", "walter", "warren", "wayne", "wilbur", "will", "willard",
"william", "willie", "willy", "zachary", "zack", NULL };

char *female_fn[] = { "abigail", "adele", "agatha", "agnes", "aileen",
"alice", "alicia", "alison", "amanda", "amelia", "amy", "anabelle",
"andrea", "angela", "anita", "ann", "anna", "anne", "annette", "annie",
"antoinette", "arlene", "ashne", "audrey", "autumn", "ava", "barb",
"barbara", "barbra", "bea", "beatrice", "becky", "belle", "bernice",
"bertha", "bess", "bessie", "beth", "betsy", "betty", "beulah", "bev",
"beverly", "bonnie", "brenda", "bridget", "brigette", "cami", "cammy",
"candace", "candy", "carla", "carol", "caroline", "carolyn", "carrie",
"catharine", "catherine", "cathie", "cathleen", "cathy", "cecilia",
"charlene", "charlotte", "cheryl", "christine", "cindy", "clara",
"clair", "claire", "claudia", "colleen", "connie", "cora", "cynthia",
"daphne", "darcie", "darcy", "darleen", "darlene", "dawn", "deanne",
"debbie", "debby", "debora", "deborah", "debra", "dee", "denice",
"denise", "diana", "diane", "dolly", "dolores", "donna", "doris",
"dorothy", "dorthy", "edith", "edna", "eileen", "elaine", "eleanor",
"eleanore", "elenore", "elisabeth", "elizabeth", "ellan", "ellen",
"elsa", "elsie", "emily", "emma", "emmy", "erica", "erika", "erma",
"estee", "esther", "ethel", "eve", "evelyn", "fay", "faye", "florence",
"fran", "frances", "gail", "gay", "geri", "gertrude", "gina", "ginny",
"gladys", "glenda", "gloria", "grace", "gretchen", "gwen", "gwendolen",
"gwendolyn", "gwyn", "hannah", "harriet", "hazel", "heather", "heidi",
"helen", "hilary", "hilda", "holly", "hope", "ida", "irene", "irma",
"isabel", "isabelle", "isobel", "ivy", "jacqueline", "jan", "jane",
"janet", "janice", "janis", "jean", "jeanette", "jeanne", "jennie",
"jennifer", "jenny", "jerrie", "jessica", "jessie", "jill", "jo",
"joan", "joann", "joanna", "joanne", "jody", "josephine", "joy",
"joyce", "juanita", "judith", "judy", "julia", "julie", "karen",
"karin", "karla", "kate", "katherine", "kathie", "kathleen",
"kathlene", "kathy", "katie", "kay", "kim", "kris", "kristine",
"laura", "laverne", "leanne", "lenore", "leslie", "lilian", "lillian",
"lillie", "lily", "linda", "lisa", "liz", "liza", "lois", "lora",
"loraine", "loralie", "loretta", "lori", "lorna", "lorraine", "louise",
"lucie", "lucille", "lucinda", "lucy", "lydia", "lynn", "lynne",
"mabel", "madeleine", "madeline", "mae", "maggie", "mandy", "marcia",
"marcie", "marcy", "margaret", "marge", "margery", "margo", "margot",
"maria", "marian", "marie", "marilyn", "marion", "marjorie", "marla",
"marleen", "marlene", "marsha", "martha", "mary", "mathilda", "maude",
"maureen", "meg", "megan", "melanie", "melissa", "meredith", "michele",
"michelle", "mildred", "millie", "milly", "minnie", "minny", "miranda",
"miriam", "mollie", "molly", "mona", "monica", "muriel", "myra",
"myrtle", "nadine", "nan", "nancy", "naomi", "natalie", "nellie",
"nicole", "niki", "nikki", "nona", "nora", "norah", "noreen", "norine",
"norma", "olga", "olivia", "pam", "pamela", "patrica", "patrice",
"patricia", "patsy", "patti", "patty", "paula", "pauline", "peggy",
"penelope", "penny", "phillis", "phylis", "phyllis", "polly",
"priscilla", "rachel", "rae", "rebecca", "renee", "rhoda", "rita",
"roberta", "robin", "rosa", "rosalie", "rosalyn", "rose", "roslyn",
"roxane", "roxanne", "ruth", "sallie", "sally", "samantha", "sandra",
"sara", "sarah", "sharon", "sheila", "shelley", "sherry", "sheryl",
"shirley", "sonja", "sonya", "sophia", "sophie", "stacey", "stefanie",
"stella", "stephanie", "stephenie", "sue", "susan", "susannah",
"susanne", "susie", "susy", "suzanna", "suzie", "suzy", "sybil",
"sylvia", "tabitha", "tammy", "tanya", "teresa", "theresa", "therese",
"tif", "tiff", "tiffany", "tina", "tracey", "tracy", "tricia", "trish",
"trudy", "tuesday", "val", "valerie", "vanessa", "veronica", "vicki",
"vickie", "vicky", "victoria", "virginia", "viv", "vivian", "wanda",
"wendy", "yvette", "yvonne", NULL };

char *both_fn[] = { "barry", "billie", "bobbie", "brett", "chris",
"francis", "jackie", "jamie", "kris", "lee", "lesley", "lou", "pat",
"randy", "robbie", "ronnie", "terry", NULL };

malep (name)
char *name;
{ char lname[MSGSIZ];
  register char **fn, *nm, *s, *desc;
  long pl;
  int m_cnt = 0, f_cnt = 0;

  strcpy (lname, lcstr (name));
  nm = lname;
  
  pl = find_player (name);

  if (!terse)
  fprintf (stderr, "Gend: player %s(%d) desc: %s\n",
	    name, pl, pl >= 0 ? player[pl].desc : "(no player found)");

  /* Check flags first */
  if (PLAYER_GET (pl, PL_MALE) || PLAYER_GET (pl, PL_ANDRO)) return (1);
  if (PLAYER_GET (pl, PL_FEMALE)) return (0);

  /* Check description for 'male' or 'female' clues */
  if (pl >= 0 && (desc = player[pl].desc))
  { for (; desc && *desc; desc = cdr (desc))
    { s = lcstr (car (desc));
    
      if (debug) fprintf (stderr, "Gend: words \"%s\"\n", s);

      if (streq (s, "she") ||
	  stlmatch (s, "she'") ||
	  streq (s, "her") || 
	  streq (s, "herself") ||
	  streq (s, "dress") ||
  	  streq (s, "female") ||
	  streq (s, "skirt") || 
	  streq (s, "woman") ||
	  streq (s, "girl") ||
	  streq (s, "lady") || 
	  streq (s, "gal"))
      { f_cnt++; }
      
      else if (streq (s, "he") ||
	       stlmatch (s, "he'") ||
	       streq (s, "his") ||
	       streq (s, "him") ||
       	       streq (s, "male") ||
	       streq (s, "fellow") ||
	       streq (s, "man") ||
	       streq (s, "boy") ||
	       streq (s, "gentleman") ||
	       streq (s, "guy"))
      { m_cnt++; }
    }
  }

  /* Female description: 2 or more gender words */
  if ((f_cnt - m_cnt) > 1)
  { if (!terse)
    { fprintf (stderr, "Gend: guess %s female, f_cnt %d, m_cnt %d\n",
	       nm, f_cnt, m_cnt);
    }
    return (0);
  }
  
  /* Male description: 2 or more gender words */
  if ((m_cnt - f_cnt) > 1)
  { if (!terse)
    { fprintf (stderr, "Gend: guess %s female, f_cnt %d, m_cnt %d\n",
	       nm, f_cnt, m_cnt);
    }
    return (1);
  }

  /* Check for explicit female name */
  for (fn = female_fn; *fn; fn++)
  { if (streq (nm, *fn))
    { if (!terse)
      { fprintf (stderr, "Gend: guess %s female, on female_fn list\n", nm); }
      return (0);
    }
  }

  /* Check for explicit male name */
  for (fn = male_fn; *fn; fn++)
  { if (streq (nm, *fn))
    { if (!terse)
      { fprintf (stderr, "Gend: guess %s male, on male_fn list\n", nm); }
      return (1);
    }
  }

  /* Check for explicit unknown name (assume male) */
  for (fn = both_fn; *fn; fn++)
  { if (streq (nm, *fn))
    { if (!terse)
      { fprintf (stderr, 
		"Gend: guess %s %s, on both_fn list, f_cnt %d, m_cnt %d\n",
		nm, m_cnt >= f_cnt ? "male" : "female", f_cnt, m_cnt);
      }
      return (m_cnt >= f_cnt);
    }
  }
  
  /* Allow a single gender word */
  if (f_cnt > m_cnt)
  { if (!terse)
    { fprintf (stderr, "Gend: guess %s female, f_cnt %d, m_cnt %d\n",
	       nm, f_cnt, m_cnt);
    }
    return (0);
  }

  if (m_cnt > f_cnt)
  { if (!terse)
    fprintf (stderr, "Gend: guess %s male, f_cnt %d, m_cnt %d\n",
	     nm, f_cnt, m_cnt);
    return (1);
  }

  /* A few heuristics for female names */
  if (ematch (name, "a") ||
      ematch (name, "i") ||
      ematch (name, "ie") ||
      ematch (name, "anne") ||
      ematch (name, "elle") ||
      ematch (name, "ette"))
  { if (!terse)
    { fprintf (stderr, "Gend: guess %s female, matches an ending\n", nm); }
    return (0);
  }

  /* Default: assume male */
  if (!terse)
  { fprintf (stderr, "Gend: guess %s male, default\n", nm); }
    
  return (1);
}

/****************************************************************
 * is_food: Return true if string is a food item, and return the
 * singular in *sp and the plural in *pp
 ****************************************************************/

is_food (str, sp, pp, number, liquid)
register char *str, **sp, **pp;
int *number, *liquid;
{ char *item = NULL, *plural = NULL;
  char s[MSGSIZ];
  register char *t = s;

  *sp = *pp = NULL;
  *number = 1;
  *liquid=0;
  
  if (!str || !*str) return (0);
  
  strcpy (s, str);

  if (MATCH (s, "*cookie*"))
  { item = "cookie"; plural = "cookies"; }

  else if (MATCH (s, "*chocolate chip*"))
  { item = "cookie"; plural = "cookies"; }

  else if (MATCH (s, "*cake*"))
  { item = "cake"; plural = "cake"; }

  else if (MATCH (s, "* pie*") && !isalpha (*res2))
  { item = "pie"; plural = "pie"; }

  else if ((MATCH (s, "*dough*nut*") || MATCH (s, "*donut*")))
  { item = "donut"; plural = "donuts"; }

  else if (MATCH (s, "*pizza*"))
  { item = "pizza"; plural = "pizza"; }

  else if (MATCH (s, "* za*") && !isalpha (*res2))
  { item = "pizza"; plural = "pizza"; }

  else if (MATCH (s, "*taco*"))
  { item = "taco"; plural = "tacos"; }

  else if (MATCH (s, "*rice*"))
  { item = "rice"; plural = "rice"; }

  else if (MATCH (s, "*corn*"))
  { item = "corn"; plural = "corn"; }

  else if (MATCH (s, "*wheat*"))
  { item = "wheat"; plural = "wheat"; }

  else if (MATCH (s, "*spinach*"))
  { item = "spinach"; plural = "spinach"; }

  else if (MATCH (s, "*broccoli*") || MATCH (s, "*brocoli*"))
  { item = "broccoli"; plural = "broccoli"; }

  else if (MATCH (s, "*beans*"))
  { item = "bean"; plural = "beans"; }

  else if (MATCH (s, "*nachos*"))
  { item = "nacho"; plural = "nachos"; }

  else if (MATCH (s, "*sandwich*"))
  { item = "sandwich"; 	plural = "sandwiches";}

  else if (MATCH (s, "*hamburger*"))
  { item = "hamburger"; plural = "hamburgers";}

  else if (MATCH (s, "*juice*"))
  { item = "juice"; plural = "juice"; *liquid=1; }

  else if (MATCH (s, "*milk shake*"))
  { item = "milk shake"; plural = "milk shakes"; *liquid=1; }

  else if (MATCH (s, "*milk*"))
  { item = "milk"; plural = "milk"; *liquid=1; }

  else if (MATCH (s, "*malted*"))
  { item = "malted"; plural = "malted's"; *liquid=1; }

  else if (MATCH (s, "*soft drink*"))
  { item = "soft drink"; plural = "soft drinks"; *liquid=1; }

  else if (MATCH (s, "*coke*"))
  { item = "Coke"; plural = "Cokes"; *liquid=1; }

  else if (MATCH (s, "*dr pepper*"))
  { item = "Dr Pepper"; plural = "Dr Peppers"; *liquid=1; }

  else if (MATCH (s, "*pepsi*"))
  { item = "Pepsi"; plural = "Pepsi's"; *liquid=1; }

  else if (MATCH (s, "*kool*aid*"))
  { item = "koolaid"; plural = "koolaid"; *liquid=1; }

  else if (MATCH (s, "*brownie*"))
  { item = "brownie"; plural = "brownies"; }

  else if (MATCH (s, "*cupcake*"))
  { item = "cupcake"; plural = "cupcakes"; }

  else if (MATCH (s, "*danish*"))
  { item = "danish"; plural = "danishs"; }

  else if (MATCH (s, "*candy*"))
  { item = "candy"; plural = "candy"; }

  else if (MATCH (s, "*ice cream*"))
  { item = "ice cream"; plural = "ice cream"; }

  else if (MATCH (s, "*nutroll*"))
  { item = "nutroll"; plural = "nutrolls"; }

  else if (MATCH (s, "*m&m*"))
  { item = "M&M"; plural = "M&M's"; }

  else if (MATCH (s, "*fudge*"))
  { item = "fudge"; plural = item; }

  else if (MATCH (s, "*banana*"))
  { item = "banana"; plural = "bananas"; }

  else if (MATCH (s, "*orange*"))
  { item = "orange"; plural = "oranges"; }

  else if (MATCH (s, "*prune*"))
  { item = "prune"; plural = "prunes"; }

  else if (MATCH (s, "*apple*"))
  { item = "apple"; plural = "apples"; }

  else if (MATCH (s, "*pear*"))
  { item = "pear"; plural = "pears"; }

  else if (MATCH (s, "*chocolate*"))
  { item = "chocolate"; plural=item; }

  if (!item)
  { return (0); }

  *number = get_quantifier (res1);
    
  *sp = item;
  *pp = plural;

  fprintf (stderr, "Food: input '%s', sing '%s', plur '%s', number %d\n",
	   s, item, plural, *number);

  return (1);
}

/****************************************************************
 * get_quantifier: Return a quantifier from a string
 *
 * -1: Every
 * -2: All
 * -3: Impossible number of cookies
 * -4: unknown plural
 * -5: some
 *  0: zero
 *  1: singular
 * 2+: definite number
 *
 ****************************************************************/

get_quantifier (s)
register char *s;
{ int number = 0;
  register char *t, *u;

  if (!s || !*s) return (0);

  /* Look for a number */
  for (t=s; *t && !isdigit (*t); t++) ;

  if (isdigit (*t)) 
  {
    /* Check for impossible quantifier */
    if (t[-1] == '-') return (-3);
    for (u=t; *u && isdigit (*u); u++) ;
    if ((int) (u-t) > 9) return (-3);
    
    number = atoi (t);
  }
  
  else if (stlmatch (s, "every ") || sindex (s, " every "))
  { number = -1; }

  else if (stlmatch (s, "all ") || sindex (s, " all "))
  { number = -2; }

  else if (stlmatch (s, "some ") || sindex (s, " some "))
  { number = -5; }

  else if (stlmatch (s, "a ") || sindex (s, " a "))
  { number = 1; }

  else if (stlmatch (s, "the ") || sindex (s, " the "))
  { number = 1; }

  else if (stlmatch (s, "zero ") || sindex (s, " zero "))
  { number = 0; }

  else
  { number = 0;

    if (stlmatch (s, "twenty ") || sindex (s, " twenty "))
    { number = 20; }
    else if (stlmatch (s, "thirty ") || sindex (s, " thirty "))
    { number = 30; }
    else if (stlmatch (s, "fourty ") || sindex (s, " fourty "))
    { number = 40; }
    else if (stlmatch (s, "fifty ") || sindex (s, " fifty "))
    { number = 50; }
    else if (stlmatch (s, "sixty ") || sindex (s, " sixty "))
    { number = 60; }
    else if (stlmatch (s, "seventy ") || sindex (s, " seventy "))
    { number = 70; }
    else if (stlmatch (s, "eighty ") || sindex (s, " eighty "))
    { number = 80; }
    else if (stlmatch (s, "ninety ") || sindex (s, " ninety "))
    { number = 90; }
  
    if (stlmatch (s, "one ") || sindex (s, " one "))
    { number += 1; }
    else if (stlmatch (s, "two ") || sindex (s, " two "))
    { number += 2; }
    else if (stlmatch (s, "three ") || sindex (s, " three "))
    { number += 3; }
    else if (stlmatch (s, "four ") || sindex (s, " four "))
    { number += 4; }
    else if (stlmatch (s, "five ") || sindex (s, " five "))
    { number += 5; }
    else if (stlmatch (s, "six ") || sindex (s, " six "))
    { number += 6; }
    else if (stlmatch (s, "seven ") || sindex (s, " seven "))
    { number += 7; }
    else if (stlmatch (s, "eight ") || sindex (s, " eight "))
    { number += 8; }
    else if (stlmatch (s, "nine ") || sindex (s, " nine "))
    { number += 9; }
    else if (stlmatch (s, "ten ") || sindex (s, " ten "))
    { number += 10; }
  
    if (stlmatch (s, "eleven ") || sindex (s, " eleven "))
    { number = 11; }
    else if (stlmatch (s, "twelve ") || sindex (s, " twelve "))
    { number = 12; }
    else if (stlmatch (s, "thirteen ") || sindex (s, " thirteen "))
    { number = 13; }
    else if (stlmatch (s, "fourteen ") || sindex (s, " fourteen "))
    { number = 14; }
    else if (stlmatch (s, "fifteen ") || sindex (s, " fifteen "))
    { number = 15; }
    else if (stlmatch (s, "sixteen ") || sindex (s, " sixteen "))
    { number = 16; }
    else if (stlmatch (s, "seventeen ") || sindex (s, " seventeen "))
    { number = 17; }
    else if (stlmatch (s, "eighteen ") || sindex (s, " eighteen "))
    { number = 18; }
    else if (stlmatch (s, "nineteen ") || sindex (s, " nineteen "))
    { number = 19; }
  
    if (stlmatch (s, "hundred ") || sindex (s, " hundred "))
    { if (number == 0) number = 1;
      number *= 100;
    }
    if (stlmatch (s, "thousand ") || sindex (s, " thousand "))
    { if (number == 0) number = 1;
      number *= 1000;
    }

    if (stlmatch (s, "million ") || sindex (s, " million "))
    { if (number == 0) number = 1;
      number *= 1000000; 
    }

    if (stlmatch (s, "billion ") || sindex (s, " billion "))
    { if (number == 0) number = 1;
      number *= 1000000000; 
    }
    
    if (number == 0) number = -4;
  }

  return (number);
}

/****************************************************************
 * inedible
 ****************************************************************/

inedible (thing)
char *thing;
{
  if (MATCH (thing, "*blow*nose*") ||
      MATCH (thing, "*wipe*nose*") ||
      MATCH (thing, "*wipe*ass*") ||
      MATCH (thing, "*ex*lax*") ||
      MATCH (thing, "*magic*brown*") ||
      sindex (thing, "penis") ||
      sindex (thing, "testicl") ||
      sindex (thing, " organ ") ||
      sindex (thing, " human ") ||
      sindex (thing, "dildo") ||
      sindex (thing, "laxat") ||
      sindex (thing, " laced ") ||
      sindex (thing, "underwear") ||
      sindex (thing, "lsd") ||
      sindex (thing, " acid ") ||
      sindex (thing, "pieces of ear") ||
      sindex (thing, "body part") ||
      sindex (thing, "fake") ||
      sindex (thing, "semen") ||
      sindex (thing, "spunk") ||
      sindex (thing, " cum ") ||
      sindex (thing, "with come") ||
      sindex (thing, "arsenic") ||
      sindex (thing, "cyanide") ||
      sindex (thing, "strychnine") ||
      sindex (thing, "mariju") ||
      sindex (thing, "cock") ||
      sindex (thing, " hash ") ||
      sindex (thing, "fuck") ||
      sindex (thing, "worm") ||
      sindex (thing, "maggot") ||
      sindex (thing, "enema") ||
      sindex (thing, "slime") ||
      sindex (thing, "disgust") ||
      sindex (thing, "shit") ||
      sindex (thing, "spit") ||
      sindex (thing, "expectorat") ||
      sindex (thing, "razor") ||
      sindex (thing, " drug ") ||
      sindex (thing, " drugged ") ||
      sindex (thing, "doctored") ||
      sindex (thing, "mickey") || sindex (thing, "finn") ||
      sindex (thing, "poison") ||
      sindex (thing, "piss") ||
      sindex (thing, "sperm") ||
      sindex (thing, "spunk") ||
      sindex (thing, "cunt") ||
      sindex (thing, "smegma") ||
      sindex (thing, "snot") ||
      sindex (thing, "of dirt") ||
      sindex (thing, "bomb") ||
      sindex (thing, "rock") ||
      sindex (thing, "glass") ||
      sindex (thing, "vomit") ||
      sindex (thing, "turd") ||
      sindex (thing, "dropping") ||
      sindex (thing, " rat ") ||
      sindex (thing, "on your face") ||
      sindex (thing, "in your face") ||
      sindex (thing, " soap") ||
      sindex (thing, "tumor") ||
      sindex (thing, "on your head") ||
      sindex (thing, "on your cloth") ||
      sindex (thing, "on the floor") ||
      sindex (thing, "dirt flav"))
  { return (1); }
  else
  { return (0); }
}

/****************************************************************
 * iscloth: True if string includes some kind of clothing
 ****************************************************************/

char *iscloth (str)
register char *str;
{
  if (sindex (str, "cloth"))	return ("clothes");
  if (sindex (str, "skirt"))	return ("skirt");
  if (sindex (str, "blouse"))	return ("blouse");
  if (sindex (str, " top"))	return ("top");
  if (sindex (str, "stocking"))	return ("stockings");
  if (sindex (str, "pants"))	return ("pants");
  if (sindex (str, "dress"))	return ("dress");
  if (sindex (str, " bra"))	return ("brassiere");
  if (sindex (str, "panties"))	return ("panties");
  if (sindex (str, "suit"))	return ("suit");

  return (NULL);
}

/****************************************************************
 * offensive_p: True if input is an offensive proposition
 ****************************************************************/

offensive_p (lcmsg)
char *lcmsg;
{
  if (MATCH (lcmsg, "*kiss* me*") ||
      MATCH (lcmsg, "*spank me*") ||
      MATCH (lcmsg, "*we*make out*") ||
      MATCH (lcmsg, "*make out*with me*") ||
      MATCH (lcmsg, "*bite me*") ||
      MATCH (lcmsg, "*bite my*") ||
      MATCH (lcmsg, "*how*about*kiss*") ||
      MATCH (lcmsg, "*kiss* my ass*") ||
      MATCH (lcmsg, "*kiss* my grits*") ||
      MATCH (lcmsg, "*i *have*kiss*") ||
      MATCH (lcmsg, "*marry* me*") ||
      MATCH (lcmsg, "*let*s fuck*") ||
      MATCH (lcmsg, "*fuck* me*") ||
      MATCH (lcmsg, "*give* me *fuck*") ||
      MATCH (lcmsg, "*fuck* my*") ||
      MATCH (lcmsg, "*fuck* off*") ||
      MATCH (lcmsg, "*fuck* you*") ||
      MATCH (lcmsg, "*screw* my*"))
  { return (1); }

  if (MATCH (lcmsg, "*screw* you*") ||
      MATCH (lcmsg, "*suck* me*") ||
      MATCH (lcmsg, "*suck* my*") ||
      MATCH (lcmsg, "*suck* dick*") ||
      MATCH (lcmsg, "*have sex *") ||
      MATCH (lcmsg, "* masturbate") ||
      MATCH (lcmsg, "*masturbate, *") ||
      MATCH (lcmsg, "*debase yourself, *") ||
      MATCH (lcmsg, "*on*your*knees*") ||
      MATCH (lcmsg, "*kneel") ||
      MATCH (lcmsg, "*sleep*with* me*") ||
      MATCH (lcmsg, "*spread*your*legs*") ||
      MATCH (lcmsg, "*spread em*") ||
      MATCH (lcmsg, "*spread 'em*") ||
      MATCH (lcmsg, "*blow* me*") ||
      MATCH (lcmsg, "*blow* my*"))
  { return (1); }

  if (MATCH (lcmsg, "* eat me*") ||
      MATCH (lcmsg, "* eat my*") ||
      MATCH (lcmsg, "eat me*") ||
      MATCH (lcmsg, "eat my*") ||
      MATCH (lcmsg, "*eat shit*") ||
      MATCH (lcmsg, "*give*me*head*") ||
      MATCH (lcmsg, "*give*blow*job*") ||
      MATCH (lcmsg, "* me*kiss") ||
      MATCH (lcmsg, "*make*love* me*") ||
      MATCH (lcmsg, "*do*make*love*") ||
      MATCH (lcmsg, "*sex*with* me*") ||
      MATCH (lcmsg, "*have*my*child*") ||
      MATCH (lcmsg, "*bear*my*child*") ||
      MATCH (lcmsg, "*have*my*baby*") ||
      MATCH (lcmsg, "*bear*my*baby*") ||
      MATCH (lcmsg, "*let*s*make*babies*") ||
      MATCH (lcmsg, "*let*s*have*sex*") ||
      MATCH (lcmsg, "*let*s*make*love*") ||
      MATCH (lcmsg, "*let*s*fuck*") ||
      MATCH (lcmsg, "*let*s*screw*") ||
      MATCH (lcmsg, "*do*wild*thing*") ||
      MATCH (lcmsg, "*want*mother*my*child*") ||
      MATCH (lcmsg, "*obey*me*") ||
      MATCH (lcmsg, "*submit*me*") ||
      MATCH (lcmsg, "*kneel*me*") ||
      (sindex (lcmsg, "you") || sindex (lcmsg, "lets")) &&
	MATCH (lcmsg, "*creat*with*back*") ||
      MATCH (lcmsg, "*boink* me*"))
  { return (1); }

  if ((sindex (lcmsg, "show me") ||
       sindex (lcmsg, "show us") ||
       sindex (lcmsg, "your") ||
       sindex (lcmsg, "touch my") ||
       sindex (lcmsg, "lick my")) &&
      (sindex (lcmsg, "nipple") ||
       sindex (lcmsg, " tit") ||
       sindex (lcmsg, "breast") ||
       sindex (lcmsg, "pussy") ||
       sindex (lcmsg, " ass") ||
       sindex (lcmsg, "cunt") ||
       sindex (lcmsg, "behind") ||
       sindex (lcmsg, "derrier") ||
       sindex (lcmsg, "cock") ||
       sindex (lcmsg, "dick")))
  { return (1); }


  if ((MATCH (lcmsg, "*fuck *, *") || MATCH (lcmsg, "*fuck *")) &&
      find_player (res2) >= 0)
  { return (1); }

  return (0);
}

/****************************************************************
 * doodify: Turn a name like Fuzzy into Fuzd00d with a given chance
 ****************************************************************/

char *doodify (name, chance)
char *name;
int chance;
{ static char buf[MSGSIZ];
  char lname[MSGSIZ];
  register char *s;
  int boy = 1;

  if (randint (100) >= chance) return (name);
  
  if (strfoldeq (name, "Priss")) return (name);
  
  strcpy (buf, name);
  strcpy (lname, lcstr (name));
  
  /* Guess gender */
  boy = malep (name);
  
  /* Start s at the end of the string */
  s = buf + strlen (buf) - 1;
  
  /* Special cases for some names */
  if (stlmatch (lname, "chup"))			strcpy (buf, "chup");
  else if (stlmatch (lname, "napo"))		strcpy (buf, "nap");
  else if (streq (lname, "explorer_bob"))	strcpy (buf, "ebob");
  else if (streq (lname, "mutant"))		strcpy (buf, "m00t");
  else if (streq (lname, "randomness"))		strcpy (buf, "ness");
  else if (streq (lname, "woodlock"))		strcpy (buf, "wood");
  else if (streq (lname, "hawkeye"))		strcpy (buf, "bird");
  else if (streq (lname, "satan"))		strcpy (buf, "devil");
  else if (stlmatch (lname, "smaras"))		strcpy (buf, "smar");
  else if (streq (lname, "blackbird"))		strcpy (buf, "beeb");
  else if (streq (lname, "rafael"))		strcpy (buf, "rafe");
  else if (streq (lname, "t.rev"))		strcpy (buf, "rev");
  else if (streq (lname, "bonehead"))		strcpy (buf, "bone");
  else if (stlmatch (lname, "evil"))		strcpy (buf, "bad");
  else if (streq (lname, "finrod"))		strcpy (buf, "fin");
  else if (streq (lname, "dirque"))		strcpy (buf, "durk");
  else if (streq (lname, "gregory"))		strcpy (buf, "greg");
  else if (streq (lname, "rhodesia"))		strcpy (buf, "rho");
  else if (streq (lname, "sidaria"))		strcpy (buf, "sid");
  else if (stlmatch (lname, "nihilist"))	strcpy (buf, "nehi");
  else if (streq (lname, "moonroach"))		strcpy (buf, "m00n");
  else if (streq (lname, "elthar"))		strcpy (buf, "thar");
  else if (streq (lname, "xibo"))		strcpy (buf, "zeeb");
  else if (streq (lname, "carneggy"))		strcpy (buf, "egg");
  else if (streq (lname, "snooze"))		strcpy (buf, "sn00z");
  else if (streq (lname, "moose"))		strcpy (buf, "m00se");
  else if (stlmatch (lname, "sgt."))		strcpy (buf, "sarge");
  else if (stlmatch (lname, "lucien"))		strcpy (buf, "l00cy");
  else if (stlmatch (lname, "nihilist"))	strcpy (buf, "nehi");
  else if (streq (lname, "belladonna"))		strcpy (buf, "bella");
  else if (streq (lname, "druid"))		strcpy (buf, "dr00d");
  else if (streq (lname, "random"))		strcpy (buf, "ran");
  
  else if (boy)
  { /* Remove and doubled letters */
    while (s > buf &&
	   ((vowelp (*s) || *s == 'y') && !vowelp (s[-1]) ||
	    *s == s[-1] ||
	    *s == 's' ||
	    *s == 'd' ||
	    *s == 'e' && s[-1] == 'i'))
    { s--; }
    s[1] = '\0';
  }
  
  /* Add cool suffix */
  if (!boy && randint (100) < 33)	if (last_char (buf) == 'i')
					{ strcat (buf, "kins"); }
					else if (vowelp (last_char (buf)))
					{ strcat (buf, "muffin"); }
					else
					{ strcat (buf, "ikins"); }
  else if (!boy && randint (100) < 50)	strcat (buf, "doll");
  else if (!boy)			strcat (buf, "babe");
  else if (randint (100) < 50)		strcat (buf, "d00d");
  else if (randint (100) < 16)		strcat (buf, "dude");
  else if (randint (100) < 20)		strcat (buf, "dood");
  else if (randint (100) < 25)		strcat (buf, "man");
  else if (randint (100) < 33)		strcat (buf, "master");
  else if (randint (100) < 50)		strcat (buf, "ster");
  else					strcat (buf, "meister");
  
  return (s=buf);
}

/****************************************************************
 * vowelp: Return true is something is a vowel
 ****************************************************************/

vowelp (ch)
int ch;
{
  return (index ("aeiouAEIOU", ch));
}

/****************************************************************
 * is_question: true if string is a question
 ****************************************************************/

is_question (lcmsg)
register char *lcmsg;
{
  if (!lcmsg || !*lcmsg) return (0);

  if (strfoldeq (car (lcmsg), myname))
  { lcmsg = cdr (lcmsg); }
  
  if (!lcmsg || !*lcmsg) return (0);

  if (stlmatch (lcmsg, "are you ") ||
      stlmatch (lcmsg, "can you ") ||
      stlmatch (lcmsg, "will you ") ||
      stlmatch (lcmsg, "would you ") ||
      sindex (lcmsg, "what ") ||
      sindex (lcmsg, "who ") ||
      sindex (lcmsg, "when ") ||
      sindex (lcmsg, "where ") ||
      sindex (lcmsg, "why ") ||
      sindex (lcmsg, "do you "))
  { return (1); }
  
  return (0);
}

/****************************************************************
 * is_negative: true if string is a negative reply
 ****************************************************************/

is_negative (lcmsg)
register char *lcmsg;
{ register char *s, *word;

  if (!lcmsg || !*lcmsg) return (0);

  for (s=lcmsg; s && *s; s = cdr (s))
  { word = car (s);
  
    if (streq (word, "no") ||
	streq (word, "nope") ||
	streq (word, "negat") ||
	streq (word, "not") ||
	streq (word, "negative") ||
	streq (word, "never") ||
	streq (word, "don't"))
    { return (1); }
  }
  
  return (0);
}

/****************************************************************
 * is_affirm: true if string is an affirmative reply
 ****************************************************************/

is_affirm (lcmsg)
register char *lcmsg;
{ register char *s, *word;

  if (!lcmsg || !*lcmsg) return (0);

  if (is_negative (lcmsg)) return (0);

  if (sindex (lcmsg, "sure do") ||
      sindex (lcmsg, "you bet") ||
      sindex (lcmsg, "affirm") ||
      sindex (lcmsg, "i think so") ||
      sindex (lcmsg, "i believe so")) return (1);

  for (s=lcmsg; s && *s; s = cdr (s))
  { word = car (s);
  
    if (streq (word, "yeah") ||
	streq (word, "yup") ||
	streq (word, "yes") ||
	streq (word, "yep") ||
	streq (word, "certainly") ||
	streq (word, "definitely") ||
	streq (word, "ok"))
    { return (1); }
  }
  
  return (0);
}

/****************************************************************
 * round_number: round a long integer the way a person might
 ****************************************************************/

# define round_to(R,N)	((R) * (((N)+((R)/2)) / (R)))

long round_number (n)
{
  if (n < 10)		return (n);
  else if (n < 20)	return (round_to (2, n));
  else if (n < 50)	return (round_to (5, n));
  else if (n < 100)	return (round_to (10, n));
  else if (n < 200)	return (round_to (20, n));
  else if (n < 500)	return (round_to (50, n));
  else if (n < 1000)	return (round_to (100, n));
  else if (n < 2000)	return (round_to (200, n));
  else if (n < 5000)	return (round_to (500, n));
  else			return (round_to (1000, n));
}

/****************************************************************
 * time_dur: 
 ****************************************************************/

# define OCT1989 623303940

char *time_dur (dur)
long dur;
{ long cnt = dur, yr = 0, rm = 0;
  char *units = "seconds", *s;
  char ybuf[SMABUF], dbuf[SMABUF];
  static char buf[2*SMABUF];
    
  if (!contest_mode && ((now-dur) < OCT1989))
  { strcpy (buf, "a long time");
    return (s = buf);
  }
  
  if (dur > (365 * DAYS + 20952))	
  { yr = dur / (365 * DAYS + 20952);	/* Years */
    dur -= yr * (365 * DAYS + 20952);	/* Portion of a year */
  }

  /* For small amounts of time */
  if (dur < 3)			sprintf (buf, "an instant");
  else if (dur < 10)		sprintf (buf, "a few seconds");
  else if (dur < 45)		sprintf (buf, "less than a minute");
  else if (dur < 75)		sprintf (buf, "a minute");
  else if (dur < 100)		sprintf (buf, "a minute and a half");
  else if (dur < 110)		sprintf (buf, "a minute or two");
  else if (dur < 135)		sprintf (buf, "a couple of minutes");
  else if (dur < 165)		sprintf (buf, "two and a half minutes");
  else if (dur < 200)		sprintf (buf, "three minutes");
  else if (dur < 500)		sprintf (buf, "a few minutes");

  else
  { if (dur < 101)		{ cnt = dur; units = "second"; }
    else if (dur < 6001)	{ cnt = dur/MINUTES; units = "minute"; }
    else if (dur < 120000)	{ cnt = dur/HOURS; units = "hour"; }
    else if (dur < 1200000)	{ cnt = dur/DAYS; units = "day"; }
    else if (dur < 13000000)	{ cnt = dur/(7*DAYS); units = "week"; }
    else			{ cnt = dur/((365/12)*DAYS); units = "month";}
    
    cnt = round_number (cnt);

    if (streq (units, "month") && cnt == 12)
    { cnt = 0; yr++; }

    /* Figure year string */
    if (yr == 0)
    { strcpy (ybuf, ""); }
    else
    { sprintf (ybuf, "%d year%s", yr, (yr == 1) ? "" : "s"); }
  
    /* Figure portion of a year string */
    if (cnt == 0)
    { strcpy (dbuf, ""); }
    else
    { sprintf (dbuf, "%ld %s%s", cnt, units, (cnt == 1) ? "" : "s"); }
    
    /* Combine the two strings */
    if (yr == 0 && cnt == 0)
    { sprintf (buf, "an instant"); }

    else if (yr > 0 && yr < 5 && cnt > 0)
    { sprintf (buf, "%s and %s", ybuf, dbuf); }

    else if (yr > 0)
    { strcpy (buf, ybuf); }

    else
    { strcpy (buf, dbuf); }
  }

  /* Return point to static memory */
  return (s = buf);
}

/****************************************************************
 * exact_dur: 
 ****************************************************************/

# define OCT1989 623303940

char *exact_dur (dur)
long dur;
{ long cnt = dur, yr = 0, rm = 0;
  char *units = "seconds", *s;
  char ybuf[SMABUF], dbuf[SMABUF];
  static char buf[2*SMABUF];
    
  if ((now-dur) < OCT1989)
  { strcpy (buf, "a long time");
    return (s = buf);
  }
  
  if (dur > (365 * DAYS + 20952))	
  { yr = dur / (365 * DAYS + 20952);	/* Years */
    dur -= yr * (365 * DAYS + 20952);	/* Portion of a year */
  }

  /* For small amounts of time */
  if (dur < 101)		{ cnt = dur; units = "second"; }
  else if (dur < 6001)		{ cnt = dur/MINUTES; units = "minute"; }
  else if (dur < 120000)	{ cnt = dur/HOURS; units = "hour"; }
  else if (dur < 1200000)	{ cnt = dur/DAYS; units = "day"; }
  else if (dur < 13000000)	{ cnt = dur/(7*DAYS); units = "week"; }
  else				{ cnt = dur/((365/12)*DAYS); units = "month";}

  /* Figure year string */
  if (yr == 0)
  { strcpy (ybuf, ""); }
  else
  { sprintf (ybuf, "%d year%s", yr, (yr == 1) ? "" : "s"); }
  
  /* Figure portion of a year string */
  if (cnt == 0)
  { strcpy (dbuf, ""); }
  else
  { sprintf (dbuf, "%ld %s%s", cnt, units, (cnt == 1) ? "" : "s"); }
    
  /* Combine the two strings */
  if (yr == 0 && cnt == 0)	{ sprintf (buf, "an instant"); }
  else if (yr > 0 && cnt > 0)	{ sprintf (buf, "%s and %s", ybuf, dbuf); }
  else if (yr > 0)		{ strcpy (buf, ybuf); }
  else				{ strcpy (buf, dbuf); }

  /* Return point to static memory */
  return (s = buf);
}

/****************************************************************
 * lcstr: lower case a string
 ****************************************************************/

char *lcstr (str)
char *str;
{ register char *s, *t;
  static char buf[BIGBUF];

  for (s=str, t=buf; *s; )
  { *t++ = isupper (*s) ? tolower (*s++) : *s++; }
  
  *t = '\0';

  return (t = buf);  
}

/****************************************************************
 * strfoldeq: True if two strings are the same except for case
 ****************************************************************/

# define fold_lower(C) (isupper (C) ? tolower (C) : (C))

strfoldeq (a, b)
register char *a, *b;
{
  while (*a && *b && fold_lower (*a) == fold_lower (*b)) { a++; b++; }
  return (*a == '\0' && *b == '\0');
}

/****************************************************************
 * timeofday: Return 'morning', 'afternoon' or 'evening'
 ****************************************************************/

char *timeofday (lunch)
int lunch;
{ struct tm *t, *localtime();
  
  now = time (0);
  t = localtime (&now);
  
  if (lunch && t->tm_hour > 11 && t->tm_hour < 13)	return ("lunch");
  else if (t->tm_hour < 12)				return ("morning");
  else if (t->tm_hour < 18)				return ("afternoon");
  else							return ("evening");
}

/****************************************************************
 * numberval: Return numeric value, or -1e9 for non-number
 ****************************************************************/

# define NONUMBER (1e9)

long numberval (str)
char *str;
{ long atol (), sign;
  char *s;

  if (isdigit (*str)) return (atol (str));
  if (*str == '-' && isdigit (str[1])) return (- atol (str+1));
  
  if (stlmatch (str, "negative ")) { sign = -1; s = str + 9; }
  else				   { sign = 1; s = str; }
  
  if (strfoldeq (s, "zero")) return (sign * 0);
  if (strfoldeq (s, "one")) return (sign * 1);
  if (strfoldeq (s, "two")) return (sign * 2);
  if (strfoldeq (s, "three")) return (sign * 3);
  if (strfoldeq (s, "four")) return (sign * 4);
  if (strfoldeq (s, "five")) return (sign * 5);
  if (strfoldeq (s, "six")) return (sign * 6);
  if (strfoldeq (s, "seven")) return (sign * 7);
  if (strfoldeq (s, "eight")) return (sign * 8);
  if (strfoldeq (s, "nine")) return (sign * 9);
  if (strfoldeq (s, "ten")) return (sign * 10);
  if (strfoldeq (s, "eleven")) return (sign * 11);
  if (strfoldeq (s, "twelve")) return (sign * 12);
  if (strfoldeq (s, "thirteen")) return (sign * 13);
  if (strfoldeq (s, "fourteen")) return (sign * 14);
  if (strfoldeq (s, "fifteen")) return (sign * 15);
  if (strfoldeq (s, "sixteen")) return (sign * 16);
  if (strfoldeq (s, "seventeen")) return (sign * 17);
  if (strfoldeq (s, "eighteen")) return (sign * 18);
  if (strfoldeq (s, "nineteen")) return (sign * 19);
  if (strfoldeq (s, "twenty")) return (sign * 20);

  return (sign * NONUMBER);
}

isnumber (str)
char *str;
{ return (numberval (str) != NONUMBER);  }

is_male_fn (str)
char *str;
{ register char **fn, *nm;

  /* Check for explicit male name */
  for (fn = male_fn; *fn; fn++)
  { if (streq (str, *fn))
    { return (1); }
  }
  
  return (0);
}

is_female_fn (str)
char *str;
{ register char **fn, *nm;

  /* Check for explicit female name */
  for (fn = female_fn; *fn; fn++)
  { if (streq (str, *fn))
    { return (1); }
  }
  
  return (0);
}

is_both_fn (str)
char *str;
{ register char **fn, *nm;

  /* Check for explicit both name */
  for (fn = both_fn; *fn; fn++)
  { if (streq (str, *fn))
    { return (1); }
  }
  
  return (0);
}

/****************************************************************
 * is_hearts: True if message is a hearts input command
 ****************************************************************/

is_hearts (lcmsg)
char *lcmsg;
{
  if (stlmatch (lcmsg, "drynn") ||
      streq (lcmsg, "illume") ||
      streq (lcmsg, "xrka") ||
      streq (lcmsg, "mei") ||
      streq (lcmsg, "plei") ||
      stlmatch (lcmsg, "nou'hou") ||
      stlmatch (lcmsg, "westray") ||
      stlmatch (lcmsg, "kirkard") ||
      stlmatch (lcmsg, "werplei") ||
      stlmatch (lcmsg, "auswahl") ||
      (index ("23456789jqka", lcmsg[0]) && index ("cdhs", lcmsg[1])) ||
      (lcmsg[0] == '1' && lcmsg[1] == '0' && index ("cdhs", lcmsg[2])))
  { return (1); }

  return (0);
}

/****************************************************************
 * malstr: Allocate a string and initialize it to a value
 ****************************************************************/

char *malstr (str)
char *str;
{ register char *s;

  if ((s = (char *) malloc (strlen (str) + 1)) == NULL)
  { perror ("in malstr"); exit (1); }

  strcpy (s, str);
  return (s);
}

