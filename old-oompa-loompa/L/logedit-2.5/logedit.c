/* logedit - automatic editing of MUSH logs
 * Syntax: logedit [switches] <filename>
 * Switches:
 * -p           remove pages
 * -w           remove whispers
 * -a           remove admin/broadcast messages
 * -c           remove @channel messages
 * -l           remove *has left, *has arrived, *goes home.
 * -x name      remove page/whisper/say/pose by name
 * -y name      convert "You say," to "name says,"
 * -s           produce attendance/other stats
 * -t           terse mode - eliminate room descs
 * -d           remove DOING polls
 * -O           aggressive anti-spam (for osuccs/odrops)
 * -n           no .logeditrc file.
 * -r regexp    remove all lines matching regexp
 * -I           Insert a blank line between every sentence
 *                 to improve readability (by CJS)
 * -m           Remove @mail
 * -f world     Include output from tf world "world" only
 * -f -world    Exclude output from tf world "world"
 * -A           Remove ANSI codes
 * -W [#[,#[,#]]]  Word wrap at column # (defaults to 72)
 *                Indent first line at #, rest of lines at #.
 * -F file      Use file as conf. file
 * -T           Remove timestamped messages (@idle, @away, @hide)
 * -o file      USe file as output file
 * Logedit always removes NOSPOOF markers, GAME: msgs, and anything between
 * a pair of #log's, and inserts an edit date/time header. And connecteds.
 * And MAIL: messages. And Huh? and Flag set/Doing set messages
 * And should also handle repeats of the same message (SPAM)
 * */

#include "logedit.h"

char confname[80];

int
 main(int argc, char *argv[])
{
  FILE *input, *output, *conf;
  char buffer[80];
  char inputname[30], outputname[33];
  char xname[MAX_EXCEPTS][30];
  char yname[30];
  char wname[MAX_WORLDS][80];
  byte switchlist = 0;
  int i, j, xcount = 0, wcount = 0, wrap = 72;
  int first_indent = 0, rest_indent = 0;
  boolean cmdline_worlds = FALSE;
  char *c, *p;

  if (argc == 1)
  {
    syntax(1);
    exit(EXIT_FAILURE);
  }
  /* parse into switches and filename */

  sprintf(confname, "%s/%s", CONFDIR, CONFFILE);
  strcpy(inputname, argv[argc - 1]);
  strcpy(outputname, inputname);
  strcat(outputname, ".log");
  for (i = 1; i < argc - 1; i++)
  {
    c = argv[i];
    if (*c != SWITCHCHAR)
    {
      syntax(1);
      exit(EXIT_FAILURE);
    }
    while (*++c)
    {
	    /****************************************
          	 * Parse special switches first:
          	 *   help, debug, tfworlds, outputfile
          	 ****************************************/
      if ((tolower(*c) == 'h') || (*c == '?'))
      {
	syntax(0);
	exit(EXIT_FAILURE);
      }
      if (*c == '-')
      {
	debug++;
	continue;
      }
      if (*c == 'o')
      {
	/* If it's just -o, or if -o is last, use stdout */
	if ((i == argc - 2) || (*argv[i + 1] == SWITCHCHAR))
	  strcpy(outputname, "-");
	else
	  strcpy(outputname, argv[++i]);
	continue;
      }
      if (*c == 'f')
      {
	cmdline_worlds = TRUE;
	if (argv[i + 1][0] == '-')
	{
	  if (switchlist & TFWORLDUSE)
	    fprintf(stderr, "Ignoring -f %s in favor of -f world\n", argv[++i]);
	  else
	  {
	    switchlist |= TFWORLDSKIP;
	    if (wcount < MAX_WORLDS)
	      strcpy(wname[wcount++], argv[++i] + 1);
	  }
	}
	else
	{
	  if (switchlist & TFWORLDSKIP)
	  {
	    wcount = 0;
	    fprintf(stderr, "Ignoring -f -world in favor of -f %s\n", argv[++i]);
	    switchlist &= ~TFWORLDSKIP;
	  }
	  switchlist |= TFWORLDUSE;
	  if (wcount < MAX_WORLDS)
	    strcpy(wname[wcount++], argv[++i]);
	}
	continue;
      }
	    /**********************************
          	 * Parse "normal" switches
          	 **********************************/
      j = 0;
      while (sw_array[j].sw_char)
      {
	if (*c == sw_array[j].sw_char)
	{
	  switchlist |= sw_array[j].sw_mask;
	  if (sw_array[j].sw_args > 0)
	  {
	    switch (sw_array[j].sw_mask)
	    {
	      case REGEXPS:
		add_regexp(argv[++i]);
		break;
	      case EXCEPT:
		if (xcount < MAX_EXCEPTS)
		  strcpy(xname[xcount++], argv[++i]);
		break;
	      case YOUCONVERT:
		strcpy(yname, argv[++i]);
		break;
	      case WRAP:
		parse_wrap(argv[++i], &wrap, &first_indent, &rest_indent);
		argv[i][2] = '\0';
		break;
	      case ALTCONF:
		strcpy(confname, argv[++i]);
		break;
	    }
	  }
	  break;
	}
	j++;
      }
      if (!sw_array[j].sw_char)
      {
	fprintf(stderr, "Ignoring unknown switch: -%c\n", *c);
      }
    }
  }

  /* Read from the configuration file */

  if (!(switchlist & NOCONF))
  {
    /* This file should contain one of these kinds of lines: 1) switches
     * = <list of switches> (only 0-arg switches) 2) # comments 3) remove
     * = <regexp> 4) exclude = <name> 5) you = <name> 6) world = <name>
     * 7) -world = <name> 8) wrap = <num> Report a warning if anything
     * else is encountered */
    if (NULL != (conf = fopen(confname, "r")))
    {
      fgets(buffer, 79, conf);
      while (!feof(conf))
      {
	/* Remove \n's */
	if (p = strchr(buffer, '\n'))
	  *p = '\0';
	if (match(buffer, "#")) ;	/* Comment - ignore it */
	else if (match(buffer, "switches"))
	{
	  p = buffer;
	  while (*p++ != '=') ;
	  while (*p == ' ')
	    p++;
	  /* Now parse the switches */
	  while (*p)
	  {
	    j = 0;
	    while (sw_array[j].sw_char)
	    {
	      if ((*p == sw_array[j].sw_char) && (sw_array[j].sw_args == 0))
	      {
		switchlist ^= sw_array[j].sw_mask;
		break;
	      }
	      j++;
	    }
	    if (!sw_array[j].sw_char)
	    {
	      fprintf(stderr, "Ignoring unknown switch: %c\n", *c);
	    }
	    p++;
	  }
	}
	else if (match(buffer, "remove"))
	{
	  switchlist |= REGEXPS;
	  p = buffer;
	  while (*p++ != '=') ;
	  while (*p == ' ')
	    p++;
	  add_regexp(p);
	}
	else if (match(buffer, "wrap"))
	{
	  if (!(switchlist & WRAP))
	  {
	    switchlist |= WRAP;
	    p = buffer;
	    while (*p++ != '=') ;
	    while (*p == ' ')
	      p++;
	    parse_wrap(p, &wrap, &first_indent, &rest_indent);
	  }
	}
	else if (match(buffer, "you"))
	{
	  if (!(switchlist & YOUCONVERT))
	  {
	    switchlist |= YOUCONVERT;
	    p = buffer;
	    while (*p++ != '=') ;
	    while (*p == ' ')
	      p++;
	    strcpy(yname, p);
	  }
	}
	else if (match(buffer, "world") && !cmdline_worlds)
	{
	  p = buffer;
	  while (*p++ != '=') ;
	  while (*p == ' ')
	    p++;
	  if (switchlist & TFWORLDSKIP)
	  {
	    fprintf(stderr, "Ignoring -world config directive in favor of world directive.\n");
	    switchlist &= ~TFWORLDSKIP;
	    wcount = 0;
	  }
	  switchlist |= TFWORLDUSE;
	  if (wcount < MAX_WORLDS)
	    strcpy(wname[wcount++], p);
	}
	else if (match(buffer, "-world") && !cmdline_worlds)
	{
	  if (switchlist & TFWORLDUSE)
	  {
	    fprintf(stderr, "Ignoring -world config directive in favor of world\n");
	  }
	  else
	  {
	    switchlist |= TFWORLDSKIP;
	    if (wcount < MAX_WORLDS)
	    {
	      p = buffer;
	      while (*p++ != '=') ;
	      while (*p == ' ')
		p++;
	      strcpy(wname[wcount++], p);
	    }
	  }
	}
	else if (match(buffer, "exclude"))
	{
	  switchlist |= EXCEPT;
	  p = buffer;
	  while (*p++ != '=') ;
	  while (*p == ' ')
	    p++;
	  if (xcount < MAX_EXCEPTS)
	    strcpy(xname[xcount++], p);
	}
	else
	  fprintf(stderr,
		  "Warning: Unrecognized config directive in %s:\n\t%s\n",
		  confname, buffer);

	fgets(buffer, 79, conf);
      }
      fclose(conf);
    }
  }
  /* make sure the filename exists */
  if (!strcmp(inputname, "-"))
  {
    input = stdin;
  }
  else if (NULL == (input = fopen(inputname, "r")))
  {
    fprintf(stderr, "Error opening file %s - aborted.\n", inputname);
    exit(EXIT_FAILURE);
  }
  /* Make up the output filename and try to create the file */
  if (*outputname == '-')
  {
    output = stdout;
  }
  else if (NULL == (output = fopen(outputname, "w")))
  {
    fprintf(stderr, "Error creating file %s - aborted.\n", outputname);
    exit(EXIT_FAILURE);
  }
  /* If we're doing stats, quick, or terse, we need to build a tree of
   * player names by scanning the file */
  if ((switchlist & TERSE) || (switchlist & STATS) || (switchlist & QUICK))
  {
    build_name_tree(input);
    if (switchlist & YOUCONVERT)
      add_name_tree(yname, name_tree);
  }
  if (switchlist & QUICK)
    switchlist |= PAGE | WHISPER | LEFT | DOING;
  /* call the edit function on the files */
  if (!(switchlist & WRAP))
    wrap = 0;
  edit(input, output, switchlist, xname, xcount - 1, yname, wname, wcount - 1,
       wrap, first_indent, rest_indent);
  /* Stats */
  if (switchlist & STATS)
    print_stats(output, inputname, outputname);
  fclose(input);
  fclose(output);
  return 0;
}

void
 syntax(int paged)
/* Print out a syntax help */
{
  byte switchlist = 0;
  char confname[200], buffer[128];
  FILE *conf;
  char *p;
  int j;
  char c;

  sprintf(confname, "%s/%s", CONFDIR, CONFFILE);
  if (NULL != (conf = fopen(confname, "r")))
  {
    fgets(buffer, 79, conf);
    while (!feof(conf))
    {
      /* Remove \n's */
      if (p = strchr(buffer, '\n'))
	*p = '\0';
      if (match(buffer, "#")) ;	/* Comment - ignore it */
      else if (match(buffer, "switches"))
      {
	p = buffer;
	while (*p++ != '=') ;
	while (*p == ' ')
	  p++;
	/* Now parse the switches */
	while (*p)
	{
	  j = 0;
	  while (sw_array[j].sw_char)
	  {
	    if ((*p == sw_array[j].sw_char) && (sw_array[j].sw_args == 0))
	    {
	      switchlist ^= sw_array[j].sw_mask;
	      break;
	    }
	    j++;
	  }
	  p++;
	}
      }
      else if (match(buffer, "remove"))
      {
	switchlist |= REGEXPS;
      }
      else if (match(buffer, "wrap"))
      {
	switchlist |= WRAP;
      }
      else if (match(buffer, "world"))
      {
	switchlist |= TFWORLDUSE;
      }
      else if (match(buffer, "-world") && !(switchlist & TFWORLDUSE))
      {
	switchlist |= TFWORLDSKIP;
      }
      else if (match(buffer, "you"))
      {
	switchlist |= YOUCONVERT;
      }
      else if (match(buffer, "exclude"))
      {
	switchlist |= EXCEPT;
      }
      fgets(buffer, 79, conf);
    }
    fclose(conf);
  }
#if  (defined(CURSES) || defined(CURSESX))
  initscr();
#endif
  printf("Logedit %s - by Alan Schwartz\n", VER);
  printf("Syntax: logedit [-acdilmnopstwA:] [-o [file]] [-r regexp] [-x name]\n");
  printf("                [-y name] [-F filename] [-W [#[,#[,#]]]] filename\n");
  printf("%s -a\tRemove @wall/@rwall/@wizwall\n", STATUS(ADMIN));
  printf("%s -c\tRemove @channels\n", STATUS(CHANNEL));
  printf("%s -d\tRemove DOING/WHO\n", STATUS(DOING));
  printf("%s -f <world>\tUse only output from <world>\n", STATUS(TFWORLDUSE));
  printf("%s -f !<world>\tSkip output from <world>\n", STATUS(TFWORLDSKIP));
  printf("%s -l\tRemove * has arrived and * has left\n", STATUS(LEFT));
  printf("%s -m\tRemove @mail read\n", STATUS(MAILNIX));
  printf(" (off) -n\tIgnore configuration file (%s/%s)\n", CONFDIR, CONFFILE);
  printf("       -o [<file>]\tUse <file> as output file, instead of filename.log\n\t\t\tIf no <file> is specified, write to stdout\n");
  printf("%s -p\tRemove pages to you and from you\n", STATUS(PAGE));
  printf("%s -r\tRemove lines containing the given Unix regexp\n",
	 STATUS(REGEXPS));
  printf("%s -s\tInclude stats on attendance and such\n", STATUS(STATS));
  printf("%s -t\tRemove room descriptions/contents (admin/see_all log req.)\n",
	 STATUS(TERSE));
  printf("%s -w\tRemove whispers to you and from you\n", STATUS(WHISPER));
  printf("%s -x\tRemove anything by <name>\n", STATUS(EXCEPT));
  printf("%s -y\tConvert 'You say,' to '<name> says,'\n",
	 STATUS(YOUCONVERT));
  if (paged)
  {
#if (defined(CURSES) || defined(CURSESX))
    printf("  ---Press any key for more---");
    fflush(stdout);
    fflush(stdin);
    nonl();
    noecho();
#ifdef CURSES
    crmode();
#else
    cbreak();
#endif
    getch();
    nl();
#ifdef CURSES
    nocrmode();
#else
    nocbreak();
#endif
    echo();
    printf("\n");
#else /* no CURSES or CURSESX */
    printf("  ---Press return for more---");
    fflush(stdout);
    fflush(stdin);
    getchar();
#endif /* CURSES stuff */
  }
  printf("%s -A\tStrip ANSI codes\n", STATUS(ANSI));
  printf(" (off) -F\tUse alternate configuration file <filename>\n");
  printf("%s -I\tInsert a blank line between each sentence\n",
	 STATUS(BLANKLINE));
  printf("%s -O\tAggressively remove osucc/odrop-type SPAM\n",
	 STATUS(NOSPAM));
  printf("%s -T\tRemove timestamp messages (@idle, @away, @haven)\n",
	 STATUS(TIMEMSG));
  printf("%s -W\tWrap words (column, first line indent, other lines indent)\n", STATUS(WRAP));
  printf("%s -:\tRemove lines starting with : and \"\n", STATUS(USERPOSE));
  printf("%s -%%\tRemove lines starting with %% (tf messages)\n", STATUS(TFMSG));
  printf("\nUsing a filename of '-' will read from the stdin\n");
  printf("and write to the stdout (unless -o is used to redirect)\n");
#if (defined(CURSES) || defined(CURSESX))
  fflush(stdout);
  endwin();
#endif
  return;
}

/* Do the real work, line by line. Ignore lines which begin with
 * lower-case letters. Analyze first word of the line to guess
 * at what it is. Sometimes will need to match more. */
void
 edit(FILE * input, FILE * output, byte switchlist, char
      xname[MAX_EXCEPTS][30], int xcount, char yname[],
      char wname[MAX_WORLDS][80], int wcount, int wrap,
      int first_indent, int rest_indent)
{
  char buffer[BUFFER_LEN];
  char lastbuff[BUFFER_LEN];
  char nextbuff[BUFFER_LEN];
  char tbuf[BUFFER_LEN];
  char *p;
  boolean skip = FALSE;
  boolean inlog = FALSE;
  boolean indoing = FALSE;
  boolean inmail = FALSE, inbadworld = FALSE;
  boolean inroom = FALSE, incontents = FALSE, exitline = FALSE;
  struct regexp_entry *regptr;
  time_t bintime;
  int count = 0;
  int is_blank_line = FALSE;
  int CJSnoblank = FALSE;
  int CJSleft = 0;
  int j;

  /* File header */
  time(&bintime);
  fprintf(output, "Log edited with Logedit %s, on %s\n", VER,
	  asctime(localtime(&bintime)));
  if (switchlist & ALTCONF)
    fprintf(output, "Using alternate config file: %s\n", confname);
  fprintf(output, "Editing out: GAME ");
  j = 0;
  while (sw_array[j].sw_char)
  {
    if ((sw_array[j].sw_args == 0) && (switchlist & sw_array[j].sw_mask))
      fprintf(output, "%s ", sw_array[j].sw_name);
    j++;
  }
  fprintf(output, "\n");
  if (switchlist & EXCEPT)
  {
    fprintf(output, "All statements by: ");
    for (count = 0; count <= xcount; count++)
      fprintf(output, "%s ", xname[count]);
    fprintf(output, "are removed.\n");
  }
  if ((switchlist & TFWORLDSKIP) || (switchlist & TFWORLDUSE))
  {
    if (switchlist & TFWORLDSKIP)
      fprintf(output, "Skipping output from worlds: ");
    else
      fprintf(output, "Using only output from worlds: ");
    for (count = 0; count <= wcount; count++)
    {
      fprintf(output, "%s ", wname[count]);
      sprintf(buffer, "*%s*", wname[count]);
      strcpy(wname[count], buffer);
    }
    fprintf(output, "\n");
  }
  if (switchlist & REGEXPS)
  {
    fprintf(output, "Regexps: ");
    regptr = regexp_list;
    while (regptr)
    {
      fprintf(output, "%s ", regptr->str);
      regptr = regptr->next;
    }
    fprintf(output, "are removed.\n");
  }
  if (switchlist & YOUCONVERT)
    fprintf(output, "Logged by: %s\n", yname);
  if (switchlist & WRAP)
  {
    fprintf(output, "Wrapping words at column %d.\n", wrap);
    if (first_indent)
      fprintf(output, "  First line indented to column %d.\n", first_indent);
    if (rest_indent)
      fprintf(output, "  All lines but first indented to column %d.\n",
	      rest_indent);
  }
  if (switchlist & STATS)
    fprintf(output, "Statistics appear at end of log\n");
  fprintf(output, "-----------log begins-------------\n\n");

  if (switchlist & USERPOSE)
  {
    add_regexp("^:");
    add_regexp("^\"");
  }

  strcpy(buffer, "");
  strcpy(nextbuff, "");

  while (!feof(input))
  {
    strcpy(lastbuff, buffer);
    strcpy(buffer, nextbuff);
    fgets(nextbuff, BUFFER_LEN - 1, input);
    if (buffer == "")
      continue;

    /* Always remove these, but they can't occur in room descs */
    if (match(buffer, GAMEPREF) ||
#ifdef MAILPREF
	match(buffer, MAILPREF) ||
#endif
	match(buffer, "Huh?") ||
	match(buffer, "Flag set") ||
#ifdef MAILDELIM
	(!strcmp(lastbuff, buffer) && !match(buffer, MAILDELIM)) ||
#endif
	match(buffer, "Doing set"))
    {
      inroom = FALSE;
      incontents = FALSE;
      continue;
    }
    /* Remove anything between #log's */
    if (match(buffer, DELIMITER))
    {
      inlog = !inlog;
      continue;
    }
    if (inlog)
      continue;

    /* Deal with worlds. Tf's world change string is in TFWORLDCHANGE */
    if ((switchlist & TFWORLDUSE) || (switchlist & TFWORLDSKIP))
    {
      if (match(buffer, TFWORLDCHANGE))
      {
	if (debug)
	  fprintf(stderr, "World change\n");
	if (switchlist & TFWORLDUSE)
	{
	  inbadworld = TRUE;
	  for (count = 0; count <= wcount; count++)
	  {
	    if (match(buffer, wname[count]))
	      inbadworld = FALSE;
	  }
	  /* If only 1 world, don't show the world change */
	  if (!inbadworld && (wcount == 0))
	    continue;
	}
	if (switchlist & TFWORLDSKIP)
	{
	  inbadworld = FALSE;
	  for (count = 0; count <= wcount; count++)
	  {
	    if (match(buffer, wname[count]))
	      inbadworld = TRUE;
	  }
	}
      }
      if (inbadworld)
	continue;
    }
    /* Remove DOING/WHO, if instructed */
    if (switchlist & DOING)
    {
      if (match(buffer, DOING_END))
      {
	indoing = FALSE;
	continue;
      }
      if (match(buffer, DOING_START))
      {
	indoing = TRUE;
	continue;
      }
      if (indoing)
	continue;
    }
    /* Remove @mail */
#ifdef MAILDELIM
    if ((switchlist & MAILNIX) && nextbuff && *nextbuff)
    {
      if (match(buffer, MAILDELIM) && (inmail == 2))
      {
	inmail = FALSE;
	continue;
      }
      if (match(buffer, MAILDELIM) && match(nextbuff, "From:"))
      {
	/* This is a mail message */
	inmail = 1;
	continue;
      }
      if (match(buffer, MAILDELIM))
	inmail = 2;
      if (inmail)
	continue;
    }
#endif

    /* We can edit room descs here. In a room desc once we see (#*R*).
     * Out of it once we see a line that starts with "Contents:",
     * "Exits:", "Obvious exits:", or a player takes an action (that is,
     * we see a line starting with a name */
    if (switchlist & TERSE)
    {
      /* Handle room descs */
      if (isplayer(firstword(buffer)) ||
	  match(buffer, "Contents:") ||
	  match(buffer, "Exits:") ||
	  match(buffer, "Obvious exits:"))
      {
	inroom = FALSE;
      }
      if (match(buffer, "*(#*R*)*"))
      {
	inroom = TRUE;
      }
      else if (inroom)
	continue;

      /* Handle Contents */
      if (incontents &&
	  (isplayer(firstword(buffer)) ||
	   match(buffer, "Exits:") ||
	   match(buffer, "Obvious exits:")))
      {
	incontents = FALSE;
	fprintf(output, "\n");
      }
      if (match(buffer, "Contents:"))
      {
	incontents = TRUE;
      }
      else if (incontents)
      {
	/* This is something in the contents list. We want * * to
	 * strip the \n and replace it with a space */
	p = strchr(buffer, '\n');
	if (p)
	  *p = ' ';
      }
    }
    /* If the first char is lower-case, we're continuing from a previous
     * line. */
    if (islower(buffer[0]))
    {
      if (!skip)
      {
	if (switchlist & ANSI)
	  safe_fputs(buffer, output, wrap, first_indent, rest_indent);
	else if (switchlist & WRAP)
	  wrap_fputs(buffer, output, wrap, first_indent, rest_indent);
	else
	  fputs(buffer, output);
      }
      continue;
    }
    skip = FALSE;

    if (strstr(buffer, "has connected.") ||
	strstr(buffer, "has disconnected.") ||
	strstr(buffer, "has reconnected.") ||
	strstr(buffer, "has partially disconnected."))
    {
      skip = TRUE;
      continue;
    }
    if (switchlist & QUICK)
      if (!isplayer(firstword(buffer)) && !match(buffer, "You say"))
      {
	skip = TRUE;
	continue;
      }
    if (switchlist & ADMIN)
      if (match(buffer, WALL) ||
#ifdef PENN150
	  match(buffer, RWALL) ||
#endif
	  match(buffer, WIZWALL))
      {
	skip = TRUE;
	continue;
      }
    if (switchlist & CHANNEL)
      if (buffer[0] == '<' && isupper(buffer[1]) &&
	  strchr(buffer, '>'))
      {
	skip = TRUE;
	continue;
      }
    if (switchlist & LEFT)
      if (strstr(buffer, "has left.") ||
	  strstr(buffer, "has arrived.") ||
	  strstr(buffer, "goes home."))
      {
	skip = TRUE;
	continue;
      }
    if (switchlist & TIMEMSG)
      if (match(buffer, TIMEMESSAGE))
      {
	skip = TRUE;
	continue;
      }
    if (switchlist & PAGE)
      if (match(buffer, PAGETOPREF) || match(buffer, PPTOPREF) ||
	  match(buffer, PPFROMPREF) || strstr(buffer, "pages:"))
      {
	skip = TRUE;
	continue;
      }
    if (switchlist & WHISPER)
      if (match(buffer, WHISTOPREF) || match(buffer, WPFROMPREF) ||
	  strstr(buffer, "senses:") || strstr(buffer, "whispers,"))
      {
	skip = TRUE;
	continue;
      }
    if (switchlist & TFMSG)
      if (*buffer == '%')
      {
	skip = TRUE;
	continue;
      }
    if (switchlist & EXCEPT)
      for (count = 0; count <= xcount; count++)
	if (match(buffer, xname[count]))
	{
	  skip = TRUE;
	  continue;
	}
    if (switchlist & NOSPAM)
      if (spam_match(lastbuff, buffer))
      {
	skip = TRUE;
	continue;
      }
    if (switchlist & REGEXPS)
      if (match_regexp(buffer))
      {
	skip = TRUE;
	continue;
      }

    /* Check for blanks or lines containing just spaces - by CJS */
    is_blank_line = FALSE;
    count = -1;
    if (switchlist & BLANKLINE)
    {
      while (buffer[++count] != 0x0a)
      {
	if (buffer[count] != ' ')
	{
	  is_blank_line = TRUE;
	  break;
	}
      }

      exitline = FALSE;
      /* We don't want blank lines if we are inserting them - CJS */
      if (!is_blank_line)
	skip = TRUE;

      /* Don't stick blanks in the room description - CJS */
      else if (incontents &&
	       (isplayer(firstword(buffer)) ||
		(exitline = match(buffer, "Exits:") ||
		 match(buffer, "Obvious exits:"))))
      {

	incontents = FALSE;
	if (exitline)
	  CJSleft = 2;
      }
      else if (match(buffer, "Contents:"))
	incontents = TRUE;

      CJSnoblank = !incontents;

      /* Make sure the exits line, and the one following it - CJS */
      /* do not have spaces inserted.  Multiple lines of    - CJS */
      /* exits are handled incorrectly, but there's no way  - CJS */
      /* to accurately tell how many lines there are.       - CJS */
      if (CJSleft)
      {
	CJSnoblank = FALSE;
	CJSleft--;
      }
    }
    if (skip)
      continue;

    /* Insert a blank line - CJS */
    if ((switchlist & BLANKLINE) && CJSnoblank)
      fputs("\n", output);

    p = buffer;

    if ((switchlist & YOUCONVERT) && match(buffer, "You say,"))
    {
      while (*p++ != ',') ;
      sprintf(tbuf, "%s says, ", yname);
      p = strcat(tbuf, p);
    }
    else if (*p == '[')
    {
      while (*p++ != ']') ;
      p++;
      fprintf(output, "*");
    }
    if (switchlist & STATS)
      add_mention(firstword(p), match(p, "*says*"));

    if (switchlist & ANSI)
      safe_fputs(p, output, wrap, first_indent, rest_indent);
    else if (switchlist & WRAP)
      wrap_fputs(p, output, wrap, first_indent, rest_indent);
    else
      fputs(p, output);

    skip = FALSE;
  }
  fprintf(output, "\n");
}

boolean
match(char *str, char *pattern)
/* If there's no wildcard, simple matching. Otherwise use wild_match */
{
  if (!str || !*str || !pattern || !*pattern)
    return (FALSE);
  if (strchr(pattern, '*'))
    return (wild_match(str, pattern));
  else
  {
    while (*pattern != '\0')
    {
      if (*pattern++ != *str++)
	return (FALSE);
    }
    return (TRUE);
  }
}

boolean
wild_match(char *str, char *pattern)
/* This time we allow the * wildcard to match */
{
  boolean wild = FALSE;
  char nextword[80];
  char *nextwordptr, *temp;

  while (*pattern != '\0')
  {
    if (!wild)
    {
      if (*pattern == '*')
      {
	wild = TRUE;
	pattern++;
      }
      else
      {
	if (*pattern++ != *str++)
	  return (FALSE);
      }
    }
    else
    {
      /* We're in a wildcard match now. That means that we can skip
       * ahead in the str until we reach the next occurence of the next
       * word in the pattern. If there is no next word in the pattern
       * (ended with *), we're already handled. If the str runs out, we
       * must return false. */
      temp = pattern;
      nextwordptr = nextword;
      while ((*temp != '\0') && (*temp != ' ') && (*temp != '*'))
	*nextwordptr++ = *temp++;
      *nextwordptr = '\0';
      if ((str = strstr(str, nextword)) == NULL)
      {
	/* Didn't find the next word */
	return (FALSE);
      }
      else
      {
	/* Ok, str points to the beginning of the word, and * * so
	 * does pattern, so let's just continue normally */
	wild = FALSE;
      }
    }
  }
  return (TRUE);
}

void
 build_name_tree(FILE * input)
/* Build a binary tree, each node containing what we believe is
 * a player or puppet name, culled from these sources:
 * Anybody who says anything
 * Anybody who does certain actions
 * Anybody who connects/disconnects/leaves/arrives
 * Anybody who appears on a WHO list
 * Anybody with (#*P*) or (#*p*) after their name 
 * Beware of ANSI!
 */
{
  char buffer[BUFFER_LEN], rbuf[BUFFER_LEN];
  char *ptr, *ptr2, *ptr3;
  boolean indoing = FALSE;
  struct action
  {
    char pat[30];
  };
  static struct action action_list[20] =
  {
    {"*smile*"},
    {"*frown*"},
    {"*chuckle*"},
    {"*ROTF*"},
    {"*rotf*"},
    {"*grin*"},
    {"*wave*"},
    {"*laugh*"},
    {"*nod*"},
    {"*giggle*"},
    {"\0"}
  };
  struct action *actptr;

  if (debug)
    msg("Building name tree.\n");

  while (!feof(input))
  {
    fgets(rbuf, BUFFER_LEN - 1, input);
    strcpy(buffer, nixansi((char *) rbuf));
    ptr = buffer;
    /* Strip any prefixes off buffer */
    if (match(buffer, WALL) ||
#ifdef PENN150
	match(buffer, RWALL) ||
	match(buffer, "<") ||
#endif
	match(buffer, WIZWALL) ||
	match(buffer, GAMEPREF))
    {
      while (*ptr != ' ')
	ptr++;
      if (match(ptr, "Suspect"))
	while (*ptr != ' ')
	  ptr++;
    }

    /* Handle DOING/WHO */
    if (match(ptr, DOING_END))
    {
      indoing = FALSE;
      continue;
    }
    if (match(ptr, DOING_START))
    {
      indoing = TRUE;
      continue;
    }
    if (indoing)
    {
      /* First word is a player name, for sure */
      if (debug)
	msg("Match in doing.\n");
      add_name_tree(firstword(ptr), name_tree);
      continue;
    }
    /* Handle say/connect/disconnect/arrive/leave Be careful about GAME:,
     * Announcement, Admin, Broadcast messages. */
    if (match(ptr, "*says,*"))
    {
      /* Use the first word */
      add_name_tree(firstword(ptr), name_tree);
      continue;
    }
    if (match(ptr, "*has arrived.") ||
	match(ptr, "*has left."))
    {
      if (debug)
	msg("Match in say/arrive/leave\n");
      add_name_tree(firstword(ptr), name_tree);
      continue;
    }
    if (match(ptr, "*has*connected."))
    {
      /* Handle chat channels and suspects */
      if (debug)
	msg("Found a connect/disconnect!\n");
      add_name_tree(firstword(ptr), name_tree);
      continue;
    }

    /* Look for (#*Pp*) in the line */
    if (match(ptr, "*(#*P*)*") || match(ptr, "*(#*p*)*"))
    {
      /* The word before the (# is a player */
      ptr3 = strstr(ptr, "(#");
      *ptr3 = '\0';
      ptr2 = buffer;
      while ((ptr3 != ptr2) && (*ptr3 != ' '))
	ptr3--;
      if (*ptr3 == ' ')
	ptr3++;
      if (debug)
	msg("Match in P/p-flag\n");
      add_name_tree(ptr3, name_tree);
      continue;
    }

    /* Look for actions */
    for (actptr = action_list; *(actptr->pat); actptr++)
    {
      if (match(ptr, actptr->pat) && !match(restword(restword(ptr)), actptr->pat))
      {
	if (debug)
	  msg("Match in action.\n");
	add_name_tree(firstword(ptr), name_tree);
	break;
      }
    }
  }
  rewind(input);
}

void
 add_name_tree(char *name, struct name_entry *top)
/* Add a name to the tree */
{
  struct name_entry *ptr, *nextptr;

  /* If sent a null name, ignore quietly */
  if (!name || !*name)
    return;
  /* If it starts with < or [, probably not a name */
  if ((*name == '<') || (*name == '['))
  {
    if (debug)
      msg("Rejecting name for </[\n");
    return;
  }

  if (debug)
    fprintf(stdout, "Adding: %s to tree\n", name);

  if (!name_tree)
  {
    name_tree = new_entry(name);
    return;
  }
  ptr = name_tree;
  while (TRUE)
  {
    if (!strcmp(ptr->name, name))
      return;
    nextptr = (strcmp(ptr->name, name) > 0) ? ptr->left : ptr->right;
    if (nextptr)
      ptr = nextptr;
    else
    {
      nextptr = new_entry(name);
      if (strcmp(ptr->name, name) > 0)
	ptr->left = nextptr;
      else
	ptr->right = nextptr;
      return;
    }
  }
}

struct name_entry *
 new_entry(char *name)
/* Malloc a new entry node */
{
  struct name_entry *ptr;

  ptr = (struct name_entry *) malloc(sizeof(struct name_entry));

  ptr->left = NULL;
  ptr->right = NULL;
  ptr->says = 0;
  ptr->mentions = 0;
  strcpy(ptr->name, name);
  return (ptr);
}

boolean
add_mention(char *name, int issay)
/* update a player's mention counter, and say counter (if issay == 1)
 * Return TRUE if successful, FALSE if for some reason we couldn't
 * locate that player. I suppose we could add 'em on to the list,
 * too, but it probably means it's an emit or something, so we won't
 * bother for now */
{
  struct name_entry *ptr;

  /* Search throught name_tree to locate the player's record. */
  ptr = name_tree;
  while (ptr)
  {
    if (!strcmp(ptr->name, name))
    {
      /* Success */
      ptr->mentions++;
      if (issay)
	ptr->says++;
      return (TRUE);
    }
    else
      ptr = (strcmp(ptr->name, name) > 0) ? ptr->left : ptr->right;
  }

  /* We failed */
  return (FALSE);
}

boolean
isplayer(char *name)
/* Return true if name is in the player tree */
{
  struct name_entry *ptr;
  char aname[30];
  char *p;

  strcpy(aname, name);
  p = aname;
  while (p && *p && (*p != ' ') && !ispunct(*p))
    p++;
  *p = '\0';
  ptr = name_tree;
  while (ptr)
  {
    if (!strcmp(ptr->name, aname))	/* hit! */
      return (TRUE);
    else
      ptr = (strcmp(ptr->name, aname) > 0) ? ptr->left : ptr->right;
  }
  /* miss. */
  return (FALSE);
}

char *
 firstword(char *line)
/* Return first word of a string */
{
  static char firstword[30];
  char *lineptr;
  int index = 0;

  lineptr = line;
  /* Strip leading spaces */
  while (lineptr && *lineptr && (*lineptr == ' '))
    lineptr++;
  /* Copy to next space or 28 chars */
  while (lineptr && *lineptr && (*lineptr != ' ') && (index < 29))
    firstword[index++] = *lineptr++;
  firstword[index] = '\0';
  return (firstword);
}

char *restword(char *line)
/* Return all words after the first */
{
  char *lineptr;

  lineptr = line;
  while (lineptr && *lineptr && (*lineptr == ' '))
    lineptr++;
  while (lineptr && *lineptr && (*lineptr != ' '))
    lineptr++;
  while (lineptr && *lineptr && (*lineptr == ' '))
    lineptr++;
  if (lineptr && *lineptr)
    return (lineptr);
  else
    return (NULL);
}

void
 print_stats(FILE * output, char *infile, char *outfile)
/* Print stats for player pose/says and word/line counts */
{
  int words, lines, chars;
  char pipestr[80];
  char buffer[80];
  FILE *pipe;

  fflush(output);
  fprintf(output,
	  "---------------------LOG ENDS - STATISTICS----------------\n");

  /* Word counts */
  sprintf(pipestr, "wc < %s", infile);
  if ((pipe = popen(pipestr, "r")) != NULL)
  {
    fgets(buffer, 79, pipe);
    pclose(pipe);
    sscanf(buffer, "%d %d %d", &lines, &words, &chars);
    fprintf(output, "Original file (%s): %d lines, %d words\n",
	    infile, lines, words);
  }
  sprintf(pipestr, "wc < %s", outfile);
  if ((pipe = popen(pipestr, "r")) != NULL)
  {
    fgets(buffer, 79, pipe);
    pclose(pipe);
    sscanf(buffer, "%d %d %d", &lines, &words, &chars);
    fprintf(output, "This file (%s): %d lines, %d words\n",
	    outfile, lines, words);
  }
  /* Player stats - Search down the tree */
  fprintf(output, "Player Statistics:\n");
  fprintf(output, "%-30s%10s%10s\n", "PLAYER", "VERBOSITY", "ACTION");
  print_tree_stats(output, name_tree, 0);

  fprintf(output, "\nQuiet players:\n");
  print_tree_stats(output, name_tree, 1);

  fprintf(output, "\n");
}

void
 print_tree_stats(FILE * output, struct name_entry *ptr, int quiet)
/* Depth-first search of tree. If quiet, print players where mentions
 * is 0. If not, print players where mentions != 0 */
{
  if (debug)
    msg("Print_tree_stats\n");
  if (!ptr)
  {
    if (debug)
      msg("Null pointer return\n");
    return;
  }
  if (ptr->left)
    print_tree_stats(output, ptr->left, quiet);
  if (!quiet && (ptr->mentions > 0))
    fprintf(output, "%-30s%10d%10d\n", ptr->name, ptr->says, ptr->mentions);
  if (quiet && (ptr->mentions == 0))
    fprintf(output, "%s ", ptr->name);
  if (ptr->right)
    print_tree_stats(output, ptr->right, quiet);
}

void
 msg(char *line)
/* Debug messages - Use stdout 'cuz it's easily redirectable */
{
  fprintf(stdout, "%s", line);
}

struct regexp_entry *
 new_regexp(char *name)
/* Malloc a new regexp_entry */
{
  struct regexp_entry *ptr;

  ptr = (struct regexp_entry *) malloc(sizeof(struct regexp_entry));

  ptr->next = NULL;
  strcpy(ptr->str, name);
  return (ptr);
}

void
 add_regexp(char *re)
/* Add re to the end of the regexp_list */
{
  struct regexp_entry *ptr;

  if (!regexp_list)
  {
    /* First element */
    regexp_list = new_regexp(re);
  }
  else
  {
    ptr = regexp_list;
    while (ptr->next)
      ptr = ptr->next;
    ptr->next = new_regexp(re);
  }
}

boolean
match_regexp(char *buffer)
/* Return TRUE if buffer matches any regexp in the list, FALSE otherwise */
{
  struct regexp_entry *ptr;

  ptr = regexp_list;
  while (ptr)
  {
    regexp_comp(ptr->str);
    if (regexp_exec(buffer))
      return (TRUE);
    else
      ptr = ptr->next;
  }
  return (FALSE);
}

boolean
spam_match(char *buf1, char *buf2)
/* Return true if buf1 and buf2 bear some similarities. Specifically,
 * 1) Ignore the first word in each (convert to '*')
 * 2) Ignore all pronouns (convert to '*')
 * 3) Run a match on the converted strings
 */
{
  char newbuf1[BUFFER_LEN];
  char *p;

  /* Simple version - no pronoun zapping */
  return (match(restword(buf1), restword(buf2)));
}

/* fputs, but nix ansi codes (anything from an ESC to the first letter) */
void
 safe_fputs(char *str, FILE * output, int wrap, int first_indent,
	    int rest_indent)
{
  char buffer[BUFFER_LEN];

  strcpy(buffer, nixansi(str));
  if (wrap)
    wrap_fputs(buffer, output, wrap, first_indent, rest_indent);
  else
    fputs(buffer, output);
}

char *nixansi(char *str)
{
  char *p1, *p2;
  char buffer[BUFFER_LEN];

  if (strchr(str, 27) == NULL)
  {
    return str;
  }
  else
  {
    p2 = buffer;
    p1 = str;
    while (*p1 != '\0')
    {
      if (*p1 == 27)
      {
	do
	{
	  p1++;
	}
	while (*p1 && !isalpha(*p1));
	if (*p1)
	  p1++;
      }
      else
	*p2++ = *p1++;
    }
    *p2 = '\0';
    return buffer;
  }
}

/* Put out a string, word-wrapping it at a column */
void
 wrap_fputs(char *str, FILE * output, int wrap,
	    int first_indent, int rest_indent)
{
  int col;
  int indent;
  char *wrd, *buf;

  buf = str;
  while (*buf == ' ')
    buf++;
  for (col = 0; col < first_indent; col++)
    fputc(' ', output);
  wrd = strtok(buf, "\t ");
  while (wrd != NULL)
  {
    if ((col + strlen(wrd)) > wrap)
    {
      fprintf(output, "\n");
      for (col = 0; col < rest_indent; col++)
	fputc(' ', output);
    }
    if (strchr(wrd, '\n'))
    {
      fprintf(output, "%s", wrd);
    }
    else
    {
      fprintf(output, "%s ", wrd);
      col += strlen(wrd) + 1;
    }
    wrd = strtok(NULL, "\t ");
  }
}

/* Parse a wrap string of the form [<col>][,[<first>][,[<rest>]]] */
void
 parse_wrap(char *str, int *wrap, int *first_indent, int *rest_indent)
{
  char buffer[80];
  int count = 1, i, last, len;

  if (!str || !*str)
    return;
  strcpy(buffer, str);
  len = strlen(buffer);
  last = 0;
  for (i = 0; i <= len; i++)
  {
    if ((buffer[i] == ',') || (buffer[i] == '\0'))
    {
      if (last != i)
      {
	buffer[i] = '\0';
	switch (count)
	{
	  case 1:
	    *wrap = atoi(buffer + last);
	    break;
	  case 2:
	    *first_indent = atoi(buffer + last);
	    break;
	  case 3:
	    *rest_indent = atoi(buffer + last);
	    break;
	}
      }
      last = i + 1;
      count++;
    }
  }
}
