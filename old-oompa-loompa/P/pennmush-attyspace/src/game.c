/* game.c */

#include "copyrite.h"
#include "config.h"

#include <ctype.h>
#include <fcntl.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <signal.h>
#ifdef I_SYS_WAIT
#include <sys/wait.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef WIN32
#include <process.h>
void Win32MUSH_setup _((void));
#endif
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#ifdef HAS_GETRUSAGE
#include <sys/resource.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#ifdef I_UNISTD
#include <unistd.h>
#endif
#include <stdio.h>

#include "conf.h"
#include "mushdb.h"
#include "game.h"
#include "externs.h"
#include "intrface.h"
#include "match.h"
#include "globals.h"
#ifdef USE_MAILER
#include "extmail.h"
#endif
#ifdef CHAT_SYSTEM
#include "extchat.h"
#endif
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "getpgsiz.h"
#include "parse.h"
#include "access.h"
#include "version.h"

#include "command.h"

#ifdef hpux
#include <sys/syscall.h>
#define getrusage(x,p)   syscall(SYS_GETRUSAGE,x,p)
#endif				/* fix to HP-UX getrusage() braindamage */

#include "confmagic.h"

#ifdef HAS_WAITPID
#define WAIT_TYPE int
#else
#ifdef UNION_WAIT
#define WAIT_TYPE union wait
#else
#define WAIT_TYPE int
#endif
#endif

/* declarations */
char dumpfile[200];
time_t start_time;		/* MUSH start time */
extern time_t mudtime;		/* MUSH current time */
static int epoch = 0;
int reserved;
int depth = 0;			/* excessive recursion prevention */
extern int invok_counter;	/* function recursion prevention */
extern dbref cplr;
extern char ccom[];

int paranoid_dump = 0;		/* if paranoid, scan before dumping */
int paranoid_checkpt = 0;	/* write out an okay message every x objs */
static void dump_database_internal _((void));
static FILE *db_open _((const char *filename));
static FILE *db_open_write _((const char *filename));
static void db_close _((FILE * f));
Signal_t reapear _((int));
#ifdef CHAT_SYSTEM
int parse_chat _((dbref player, char *command));
#endif
void do_readcache _((dbref player));
void set_interp _((dbref player, dbref cause, char const *obj,
		   char const *attrib, char const *val, int from_port));
int test_set _((dbref player, dbref cause, char const *command,
		char *arg1, char *arg2, int from_port));
char **argv_hack _((dbref player, dbref cause, char const *arg,
		    char *fargs[], int eflags));
int check_alias _((const char *command, const char *list));
int list_check _((dbref thing, dbref player, char type,
		  char end, char *str, int just_match));
int alias_list_check _((dbref thing, const char *command,
			const char *type));
int loc_alias_check _((dbref loc, const char *command,
		       const char *type));
void do_poor _((dbref player, char *arg1));
void do_writelog _((dbref player, char *str, int ltype));
void bind_and_queue _((dbref player, dbref cause,
		       char *action, char *arg));
void do_scan _((dbref player, char *command, int flag));
void do_list _((dbref player, char *arg));
void do_dolist _((dbref player, char *list, char *command,
		  dbref cause, int flag, int notify_flag));
void do_uptime _((dbref player));
char *make_new_epoch_file _((const char *basename, int the_epoch));
void fork_and_dump _((int forking));
void dest_info _((dbref thing, dbref tt));
#ifdef HAS_GETRUSAGE
void rusage_stats _((void));
#endif
void do_restart _((void));
extern void charge_action _((dbref player, dbref thing, const char *awhat));
extern int filter_found _((dbref thing, const char *msg, int flag));
extern void local_startup _((void));
extern void do_reboot _((dbref player, int flag));
extern void local_dump_database _((void));

extern dbref first_free;	/* head of free object list, destroy.c */

dbref orator = NOTHING;

/*
 * used to allocate storage for temporary stuff, cleared before command
 * execution
 */

void
do_dump(player, num, flag)
    dbref player;
    char *num;
    int flag;
{
  /* flag: 0 = normal, 1 = paranoid, 2 = debug */

  time_t tt;
  if (Wizard(player)) {
    if (options.daytime) {
      notify(player, "Sorry, CPU intensive commands are currently disabled.");
      return;
    }
    tt = time((time_t *) 0);
#ifdef ALWAYS_PARANOID
    if (1) {
#else
    if (flag) {
#endif
      /* want to do a scan before dumping each object */
      paranoid_dump = flag;
      if (num && *num) {
	/* checkpoint interval given */
	paranoid_checkpt = atoi(num);
	if ((paranoid_checkpt < 1) || (paranoid_checkpt >= db_top)) {
	  notify(player, "Permission denied. Invalid checkpoint interval.");
	  paranoid_dump = 0;
	  return;
	}
      } else {
	/* use a default interval */
	paranoid_checkpt = db_top / 5;
	if (paranoid_checkpt < 1)
	  paranoid_checkpt = 1;
      }
      if (flag == 1) {
	notify(player, tprintf("Paranoid dumping, checkpoint interval %d.",
			       paranoid_checkpt));
	fprintf(checklog_fp,
		"*** PARANOID DUMP *** done by %s(#%d),\n",
		Name(player), player);
      } else {
	notify(player, tprintf("Debug dumping, checkpoint interval %d.",
			       paranoid_checkpt));
	fprintf(checklog_fp,
		"*** DEBUG DUMP *** done by %s(#%d),\n",
		Name(player), player);
      }
      fprintf(checklog_fp, "\tcheckpoint interval %d, at %s",
	      paranoid_checkpt, ctime(&tt));
    } else {
      /* normal dump */
      paranoid_dump = 0;	/* just to be safe */
      notify(player, "Dumping...");
      fprintf(checklog_fp, "** DUMP ** done by %s(#%d) at %s",
	      Name(player), player, ctime(&tt));
    }
    fflush(checklog_fp);
    fork_and_dump(1);
    paranoid_dump = 0;
  } else {
    notify(player, "Sorry, you are in a no dumping zone.");
  }
}


/* print out stuff into error file */
void
report()
{
  if (GoodObject(cplr))
    do_rawlog(LT_TRACE, "TRACE: Cmd:%s\tdepth:%d\tby #%d at #%d", ccom,
	      depth, cplr, Location(cplr));
  else
    do_rawlog(LT_TRACE, "TRACE: Cmd:%s\tdepth:%d\tby #%d", ccom,
	      depth, cplr);
  fflush(tracelog_fp);
}

#ifdef HAS_GETRUSAGE
void
rusage_stats()
{
  struct rusage usage;
  int pid;
#ifndef hpux
  int psize;
#endif

  pid = getpid();
#ifndef hpux
  psize = getpagesize();
#endif
  getrusage(RUSAGE_SELF, &usage);

  fprintf(stderr, "\nProcess statistics:\n");
  fprintf(stderr, "Time used:   %10ld user   %10ld sys\n",
	  usage.ru_utime.tv_sec, usage.ru_stime.tv_sec);
#ifndef hpux
  fprintf(stderr, "Max res mem: %10ld pages  %10ld bytes\n",
	  usage.ru_maxrss, (usage.ru_maxrss * psize));
#else
  fprintf(stderr, "Max res mem: %10 pages\n", usage.ru_maxrss);
#endif				/* hpux */
  fprintf(stderr, "Integral mem:%10ld shared %10ld private %10ld stack\n",
	  usage.ru_ixrss, usage.ru_idrss, usage.ru_isrss);
  fprintf(stderr, "Page faults: %10ld hard   %10ld soft    %10ld swapouts\n",
	  usage.ru_majflt, usage.ru_minflt, usage.ru_nswap);
  fprintf(stderr, "Disk I/O:    %10ld reads  %10ld writes\n",
	  usage.ru_inblock, usage.ru_oublock);
  fprintf(stderr, "Network I/O: %10ld in     %10ld out\n",
	  usage.ru_msgrcv, usage.ru_msgsnd);
  fprintf(stderr, "Context swi: %10ld vol    %10ld forced\n",
	  usage.ru_nvcsw, usage.ru_nivcsw);
  fprintf(stderr, "Signals:     %10ld\n", usage.ru_nsignals);
}

#endif				/* HAS_GETRUSAGE */

void
do_shutdown(player, flag)
    dbref player;
    int flag;			/* -1 = panic shutdown, 0 = normal, 1 = paranoid */
{
  if (flag == -1 && !God(player)) {
    notify(player, "It takes a God to make me panic.");
    return;
  }
  if (Wizard(player)) {
    flag_broadcast(0, 0, "GAME: Shutdown by %s", Name(player));
    do_log(LT_ERR, player, NOTHING, "SHUTDOWN by %s\n",
	   real_unparse(player, player, 1));

    /* This will create a file used to check if a restart should occur */
#ifdef AUTORESTART
    system("touch NORESTART");
#endif

    if (flag == -1) {
      panic("@shutdown/panic");
    } else {
      if (flag == 1) {
	paranoid_checkpt = db_top / 5;
	if (paranoid_checkpt < 1)
	  paranoid_checkpt = 1;
	paranoid_dump = 1;
      }
      shutdown_flag = 1;
    }
  } else {
    notify(player, "Your delusions of grandeur have been duly noted.");
  }
}


static void
dump_database_internal()
{
  char realdumpfile[2048];
  char realtmpfl[2048];
  char tmpfl[2048];
  FILE *f;

  local_dump_database();

#ifdef ALWAYS_PARANOID
  paranoid_checkpt = db_top / 5;
  if (paranoid_checkpt < 1)
    paranoid_checkpt = 1;
#endif

  sprintf(realdumpfile, "%s%s", dumpfile, options.compresssuff);
  strcpy(tmpfl, make_new_epoch_file(dumpfile, epoch));
  sprintf(realtmpfl, "%s%s", tmpfl, options.compresssuff);

  if ((f = db_open_write(tmpfl)) != NULL) {
    switch (paranoid_dump) {
    case 0:
#ifdef ALWAYS_PARANOID
      db_paranoid_write(f, 0);
#else
      db_write(f);
#endif
      break;
    case 1:
      db_paranoid_write(f, 0);
      break;
    case 2:
      db_paranoid_write(f, 1);
      break;
    }
    db_close(f);
#ifdef WIN32
    /* Win32 systems can't rename over an existing file, so unlink first */
    unlink(realdumpfile);
#endif
    if (rename(realtmpfl, realdumpfile) < 0)
      perror(realtmpfl);
  } else
    perror(realtmpfl);
#ifdef USE_MAILER
  sprintf(realdumpfile, "%s%s", options.mail_db, options.compresssuff);
  strcpy(tmpfl, make_new_epoch_file(options.mail_db, epoch));
  sprintf(realtmpfl, "%s%s", tmpfl, options.compresssuff);
  if (mdb_top >= 0) {
    if ((f = db_open_write(tmpfl)) != NULL) {
      dump_mail(f);
      db_close(f);
#ifdef WIN32
      unlink(realdumpfile);
#endif
      if (rename(realtmpfl, realdumpfile) < 0)
	perror(realtmpfl);
    } else
      perror(realtmpfl);
  }
#endif				/* USE_MAILER */
#ifdef ALLOW_RPAGE
  strcpy(tmpfl, make_new_epoch_file("rpage.db.Z", epoch));
  if ((f = db_open_write(tmpfl)) != NULL) {
    dump_server_database(f);
    db_close(f);
#ifdef WIN32
    unlink("rpage.db.Z");
#endif
    if (rename(tmpfl, "rpage.db.Z") < 0)
      perror(tmpfl);
  } else
    perror(tmpfl);
#endif				/* ALLOW_RPAGE */
#ifdef CHAT_SYSTEM
  sprintf(realdumpfile, "%s%s", options.chatdb, options.compresssuff);
  strcpy(tmpfl, make_new_epoch_file(options.chatdb, epoch));
  sprintf(realtmpfl, "%s%s", tmpfl, options.compresssuff);
  if ((f = db_open_write(tmpfl)) != NULL) {
    save_chatdb(f);
    db_close(f);
#ifdef WIN32
    unlink(realdumpfile);
#endif
    if (rename(realtmpfl, realdumpfile) < 0)
      perror(realtmpfl);
  } else
    perror(realtmpfl);
#endif
}

void
panic(message)
    const char *message;
{
  const char *panicfile = options.crash_db;
  FILE *f;
  int i;

  fprintf(stderr, "PANIC: %s\n", message);
  report();
  flag_broadcast(0, 0, "EMERGENCY SHUTDOWN: %s", message);

  /* turn off signals */
  for (i = 0; i < NSIG; i++) {
    signal(i, SIG_IGN);
  }

  /* shut down interface */
  emergency_shutdown();

  /* dump panic file */
  if ((f = fopen(panicfile, "w")) == NULL) {
    perror("CANNOT OPEN PANIC FILE, YOU LOSE");
    _exit(135);
  } else {
    fprintf(stderr, "DUMPING: %s\n", panicfile);
    db_write(f);
    fclose(f);
    fprintf(stderr, "DUMPING: %s (done)\n", panicfile);
    _exit(136);
  }
}

void
dump_database()
{
  epoch++;

  fprintf(stderr, "DUMPING: %s.#%d#\n", dumpfile, epoch);
  dump_database_internal();
  fprintf(stderr, "DUMPING: %s.#%d# (done)\n", dumpfile, epoch);
}

#ifndef macintosh
#ifndef WIN32
/* Don't fear this function :) */
Signal_t reaper _((int sig));
Signal_t
reaper(sig)
    int sig;
{
  WAIT_TYPE my_stat;

#ifdef HAS_WAITPID
  while (waitpid(-1, &my_stat, WNOHANG) > 0) ;
#else
  while (wait3(&my_stat, WNOHANG, 0) > 0) ;
#endif

#ifndef HAS_SIGACTION
/* If we have sigaction, the signals are reliable */
#ifndef SIGNALS_KEPT
  signal(SIGCLD, (Sigfunc) reaper);	/* do this last or it'll loop */
#endif
#endif
#ifndef VOIDSIG
  return 0;
#endif
}
#endif				/* !WIN32 */
#endif              /* !macintosh */

void
fork_and_dump(forking)
    int forking;
{
  int child, nofork;
  epoch++;

  fprintf(checklog_fp, "CHECKPOINTING: %s.#%d#\n", dumpfile, epoch);
  fflush(checklog_fp);
  if (NO_FORK)
    nofork = 1;
  else
    nofork = !forking || (paranoid_dump == 2);	/* Don't fork for dump/debug */
#ifdef WIN32
  nofork = 1;
#endif
#ifdef macintosh
  nofork = 1;
#endif
  if (!nofork) {
#ifndef macintosh
#ifndef WIN32
    child = fork();
    if (child < 0) {
      /* Oops, fork failed. Let's do a nofork dump */
      do_log(LT_ERR, 0, 0, "fork_and_dump: fork() failed! Dumping nofork instead.");
      if (DUMP_NOFORK_MESSAGE && *DUMP_NOFORK_MESSAGE)
	flag_broadcast(0, 0, DUMP_NOFORK_MESSAGE);
      child = 0;
      nofork = 1;
    }
#ifdef HAS_SETPRIORITY
    else {
      /* Lower the priority of the child to make parent more responsive */
#ifdef HAS_GETPRIORITY
      setpriority(PRIO_PROCESS, child, getpriority(PRIO_PROCESS, child) + 4);
#else				/* HAS_GETPRIORITY */
      setpriority(PRIO_PROCESS, child, 8);
#endif				/* HAS_GETPRIORITY */
    }
#endif				/* HAS_SETPRIORITY */
#endif				/* WIN32 */
#endif				/* macintosh */
  } else {
    if (DUMP_NOFORK_MESSAGE && *DUMP_NOFORK_MESSAGE)
      flag_broadcast(0, 0, DUMP_NOFORK_MESSAGE);
    child = 0;
  }
  if (nofork || (!nofork && child == 0)) {
    /* in the child */
#ifndef WIN32
    close(reserved);		/* get that file descriptor back */
#endif
#ifdef CONCENTRATOR
    signal(SIGCLD, SIG_DFL);
#endif				/* CONCENTRATOR */
    dump_database_internal();
    if (!nofork) {
      _exit(0);			/* !!! */
    } else {
#ifndef WIN32
      reserved = open("/dev/null", O_RDWR);
#endif
#ifdef CONCENTRATOR
      signal(SIGCLD, (void *) reaper);
#endif				/* CONCENTRATOR */
      if (DUMP_NOFORK_COMPLETE && *DUMP_NOFORK_COMPLETE)
	flag_broadcast(0, 0, DUMP_NOFORK_COMPLETE);
    }
  }
}

void
do_restart()
{
  dbref thing;
  ATTR *s;
  char *r;
  char buf[SBUF_LEN];
  int j;

  /* Do stuff that needs to be done for players only: add stuff to the
   * alias table, and refund money from queued commands at shutdown.
   */
  for (thing = 0; thing < db_top; thing++) {
    if (IsPlayer(thing)) {
      if ((s = atr_get_noparent(thing, "ALIAS")) != NULL) {
	strcpy(buf, uncompress(s->value));
	add_player(thing, buf);
      }
      if ((s = atr_get_noparent(thing, "QUEUE")) != NULL) {
	giveto(thing, QUEUE_COST * atoi(uncompress(s->value)));
	atr_clr(thing, "QUEUE", GOD);
      }
      if ((s = atr_get_noparent(thing, "SEMAPHORE")) != NULL) {
	atr_clr(thing, "SEMAPHORE", GOD);
      }
    }
  }

  /* Once we load all that, then we can trigger the startups and 
   * begin queueing commands. Also, let's make sure that we get
   * rid of null names.
   */
  for (j = 0; j < 10; j++) {
    wnxt[j] = NULL;
    rnxt[j] = NULL;
  }
  for (thing = 0; thing < db_top; thing++) {
    if (Name(thing) == NULL) {
      if (IsGarbage(thing))
	SET(Name(thing), "Garbage");
      else {
	do_log(LT_ERR, NOTHING, NOTHING, "Null name on object #%d",
	       thing);
	SET(Name(thing), "XXXX");
      }
    }
    if (STARTUPS && !IsGarbage(thing) &&
	(Startup(thing) && !(Halted(thing)))) {
      s = atr_get_noparent(thing, "STARTUP");
      if (!s)
	continue;		/* just in case */
      r = safe_uncompress(s->value);
      parse_que(thing, r, thing);
      free((Malloc_t) r);
    }
  }
}

void
init_game_config(conf)
    const char *conf;
{
  int a;

  depth = 0;

  for (a = 0; a < 10; a++) {
    wenv[a] = NULL;
    renv[a][0] = '\0';
    wnxt[a] = NULL;
    rnxt[a] = NULL;
  }

  /* set MUSH start time */
  start_time = time((time_t *) 0);
  fprintf(stderr, "%s\n", VERSION);
  fprintf(stderr, "MUSH restarted, PID %d, at %s\n",
	  (int) getpid(), ctime(&start_time));

  /* initialize all the hash tables: flags, functions, and attributes. */
  init_flag_hashtab();
  init_func_hashtab();
  init_aname_hashtab();

  command_init_preconfig();
  config_file_startup(conf);
  command_init_postconfig();
}

int
init_game_dbs(conf)
    const char *conf;
{
  FILE *f;

  const char *infile, *outfile;
#ifdef USE_MAILER
  const char *mailfile;
#endif

#ifdef WIN32
  Win32MUSH_setup();		/* create index files, copy databases etc. */
#endif

  infile = restarting ? options.output_db : options.input_db;
  outfile = options.output_db;
#ifdef USE_MAILER
  mailfile = options.mail_db;
#endif

  /* read small text files into cache */
  fcache_init();

  f = db_open(infile);

  /* ok, read it in */
  fprintf(stderr, "ANALYZING: %s\n", infile);
  if (init_compress(f) < 0) {
    fprintf(stderr, "ERROR LOADING\n");
    return -1;
  }
  fprintf(stderr, "ANALYZING: %s (done)\n", infile);

  /* everything ok */
  db_close(f);

  f = db_open(infile);
  if (!f)
    return -1;

  /* ok, read it in */
  fprintf(stderr, "LOADING: %s\n", infile);
  if (db_read(f) < 0) {
    fprintf(stderr, "ERROR LOADING\n");
    return -1;
  }
  fprintf(stderr, "LOADING: %s (done)\n", infile);

  /* everything ok */
  db_close(f);

  /* complain about bad config options */
  if (!GoodObject(PLAYER_START) || (!IsRoom(PLAYER_START)))
    fprintf(stderr, "WARNING: Player_start (#%d) is NOT a room.\n",
	    PLAYER_START);
  if (DO_GLOBALS &&
      (!GoodObject(MASTER_ROOM) || (!IsRoom(MASTER_ROOM))))
    fprintf(stderr, "WARNING: Master room (#%d) is NOT a room.\n",
	    MASTER_ROOM);

#ifdef USE_MAILER
  /* read mail database */
  f = db_open(mailfile);

  /* okay, read it in */
  if (f == NULL) {
    mail_init();
  } else {
    fprintf(stderr, "LOADING: %s\n", mailfile);
    load_mail(f);
    fprintf(stderr, "LOADING: %s (done)\n", mailfile);
    db_close(f);
  }

#endif				/* USE_MAILER */

#ifdef CHAT_SYSTEM
  init_chatdb();
  f = db_open(options.chatdb);
  if (f) {
    fprintf(stderr, "LOADING: %s\n", options.chatdb);
    if (load_chatdb(f)) {
      fprintf(stderr, "LOADING: %s (done)\n", options.chatdb);
      db_close(f);
    } else {
      fprintf(stderr, "ERROR LOADING %s\n", options.chatdb);
      db_close(f);
      return -1;
    }
  }
#endif

  /* now do access file stuff */
  read_access_file();

  /* now do the rpage stuff */
#ifdef ALLOW_RPAGE
  rpage_init();
#endif				/* ALLOW_RPAGE */

  /* initialize random number generator */
  srandom(getpid());

  /* set up dumper */
  strcpy(dumpfile, outfile);
  init_timer();
#ifndef macintosh
#ifndef WIN32
  signal(SIGCLD, (void *) reaper);
#endif
#endif

  /* Call Local Startup */
  local_startup();

  /* everything else ok. Restart all objects. */
  do_restart();
  return 0;
}

void
do_readcache(player)
    dbref player;
{
  if (!Wizard(player)) {
    notify(player, "Permission denied.");
    return;
  }
  fcache_load(player);
}

#ifdef CHAT_SYSTEM
int
parse_chat(player, command)
    dbref player;
    char *command;
{
  /* function hacks up something of the form "+<channel> <message>",
   * finding the two args, and passes it to do_chat
   */

  char *arg1;
  char *arg2;
  char tbuf1[MAX_COMMAND_LEN];
  char *s;

  strncpy(tbuf1, command, MAX_COMMAND_LEN - 1);		/* don't hack it up */
  tbuf1[MAX_COMMAND_LEN - 1] = '\0';
  s = tbuf1;

  arg1 = s;
  while (*s && !isspace(*s))
    s++;

  if (*s) {
    *s++ = '\0';
    while (*s && isspace(*s))
      s++;
  }
  arg2 = s;

  return do_chat_by_name(player, arg1, arg2);
}
#endif				/* CHAT_SYSTEM */

#define list_match(x)        list_check(x, player, '$', ':', cptr, 0)
#define cmd_match(x)         atr_comm_match(x, player, '$', ':', cptr, 0);

void
process_command(player, command, cause, from_port)
    dbref player;
    char *command;
    dbref cause;
    int from_port;		/* 1 if this is direct input from a port
				 * (i.e. typed directly by a player).
				 * attrib sets don't get parsed then.
				 */
{
  int a;
  char *p;			/* utility */

  char unp[BUFFER_LEN];		/* unparsed command */
  /* general form command arg0=arg1,arg2...arg10 */
  int gagged = 0;
  char temp[BUFFER_LEN];	/* utility */
  int i;			/* utility */
  char *cptr;

  depth = 0;
  if (!command) {
    do_log(LT_ERR, NOTHING, NOTHING, "ERROR: No command!!!");
    return;
  }
  /* robustify player */
  if (!GoodObject(player)) {
    do_log(LT_ERR, NOTHING, NOTHING, "process_command bad player #%d",
	   player);
    return;
  }
  gagged = IS(Owner(player), TYPE_PLAYER, PLAYER_GAGGED);
  /* Access the player */
  Access(player);

  /* Destroyed objects shouldn't execute commands */
  if (IsGarbage(player))
    /* No message - nobody to tell, and it's too easy to do to log. */
    return;
  /* Halted objects can't execute commands */
  if (!IsPlayer(player) && Halted(player)) {
    notify(Owner(player),
	   tprintf("Attempt to execute command by halted object #%d",
		   player));
    return;
  }
  /* Players and things should not have invalid locations. This check
   * must be done _after_ the destroyed-object check.
   */
  if ((!GoodObject(Location(player)) ||
       (IsGarbage(Location(player)))) &&
      Mobile(player)) {
    notify(Owner(player),
	   tprintf("Invalid location on command execution: %s(#%d)",
		   Name(player), player));
    do_log(LT_ERR, NOTHING, NOTHING,
	   "Command attempted by %s(#%d) in invalid location #%d.",
	   Name(player), player, Location(player));
    moveto(player, PLAYER_START);	/* move it someplace valid */
  }
  /* The following check is removed due to a security hole it causes!
   * 'If player is an exit or room execute command as owner'
   */
  /* if (IsRoom(player) || IsExit(player))
   * player = Owner(player);  */
  orator = player;

  if (options.log_commands || Suspect(player))
    do_log(LT_CMD, player, 0, "%s", command);

  if Verbose
    (player)
      raw_notify(Owner(player), tprintf("#%d] %s", player, command));

  /* eat leading whitespace */
  while (*command && isspace(*command))
    command++;

  /* eat trailing whitespace */
  p = command + strlen(command) - 1;
  while (isspace(*p) && (p >= command))
    p--;
  *++p = '\0';

  /* ignore null commands that aren't from players */
  if ((!command || !*command) && !from_port)
    return;

  /* important home checking comes first! */
  if (strcmp(command, "home") == 0) {
    if (!Mobile(player))
      return;
    if (Fixed(player))
      notify(player, "You can't do that IC!");
    else
      do_move(player, command, 0);
    return;
  }
  strcpy(unp, command);

  cptr = command_parse(player, cause, command, from_port);
  if (cptr) {
    a = 0;
    if (!gagged && Mobile(player)) {

      /* if the "player" is an exit or room, no need to do these checks */

      /* try matching enter aliases */
      if (Location(player) != NOTHING &&
	  (i = alias_list_check(Contents(Location(player)),
				cptr, "EALIAS")) != -1) {

	sprintf(temp, "#%d", i);
	do_enter(player, temp, 1);
	goto done;
      }
      /* if that didn't work, try matching leave aliases */
      if (!IsRoom(Location(player)) &&
	  (loc_alias_check(Location(player), cptr, "LALIAS"))) {
	do_leave(player);
	goto done;
      }
      /* try matching user defined functions before chopping */

      /* try objects in the player's location, the location itself,
       * and objects in the player's inventory.
       */
      if (Location(player) != NOTHING) {
	a += list_match(Contents(Location(player)));
	if (Location(player) != player)
	  a += cmd_match(Location(player));
      }
      if (Location(player) != player)
	a += list_match(Contents(player));

      /* now do check on zones */
      if ((!a) && (Zone(Location(player)) != NOTHING)) {
	if (DO_GLOBALS && IsRoom(Zone(Location(player)))) {

	  /* zone of player's location is a zone master room */

	  if (Location(player) != Zone(player)) {

	    /* check zone master room exits */
	    if (remote_exit(player, cptr)) {
	      if (!Mobile(player))
		goto done;
	      else {
		do_move(player, cptr, 2);
		goto done;
	      }
	    } else
	      /* check commands in the zone master room if no exits
	       * can match more than one $command in zone master room
	       */
	      a += list_match(Contents(Zone(Location(player))));
	  }			/* end of zone master room check */
	} else
	  /* try matching commands on area zone object if GLOBALS
	   * aren't in use or zone object isn't a room
	   */
	if ((!a) && (Zone(Location(player)) != NOTHING))
	  a += cmd_match(Zone(Location(player)));
      }				/* end of matching on zone of player's location */
      /* if nothing matched with zone master room/zone object, try
         * matching zone commands on the player's personal zone
       */
      if ((!a) && (Zone(player) != NOTHING) &&
	  (Zone(Location(player)) != Zone(player))) {
	a += cmd_match(Zone(player));
      }
      /* end of zone stuff */
      if (DO_GLOBALS) {
	/* check global exits only if no other commands are matched */
	if ((!a) && (Location(player) != MASTER_ROOM)) {
	  if (global_exit(player, cptr)) {
	    if (!Mobile(player))
	      goto done;
	    else {
	      do_move(player, cptr, 1);
	      goto done;
	    }
	  } else
	    /* global user-defined commands checked if all else fails.
	     * May match more than one command in the master room.
	     */
	    a += list_match(Contents(MASTER_ROOM));
	}
	/* end of master room check */
      }
    }				/* end of special checks */
    if (!a) {
      notify(player, "Huh?  (Type \"help\" for help.)");
      if (options.log_huhs)
	do_log(LT_HUH, player, 0, "%s", unp);
    }
  }
  /* command has been executed. Free up memory. */

done:
  ;
}


/* now undef everything that needs to be */
#undef list_match
#undef cmd_match

int
check_alias(command, list)
    const char *command;
    const char *list;
{
  /* check if a string matches part of a semi-colon separated list */
  const char *p;
  while (*list) {
    for (p = command; (*p && DOWNCASE(*p) == DOWNCASE(*list)
		       && *list != EXIT_DELIMITER);
	 p++, list++) ;
    if (*p == '\0') {
      while (isspace(*list))
	list++;
      if (*list == '\0' || *list == EXIT_DELIMITER)
	return 1;		/* word matched */
    }
    /* didn't match. check next word in list */
    while (*list && *list++ != EXIT_DELIMITER) ;
    while (isspace(*list))
      list++;
  }
  /* reached the end of the list without matching anything */
  return 0;
}

/* match a list of things */
#ifdef CAN_NEWSTYLE
int
list_check(dbref thing, dbref player, char type, char end, char *str,
	   int just_match)
#else
int
list_check(thing, player, type, end, str, just_match)
    dbref thing, player;
    char type, end;
    char *str;
    int just_match;
#endif
{
  int match = 0;

  while (thing != NOTHING) {
    if (atr_comm_match(thing, player, type, end, str, just_match))
      match = 1;
    thing = Next(thing);
  }
  return (match);
}

int
alias_list_check(thing, command, type)
    dbref thing;
    const char *command;
    const char *type;
{
  ATTR *a;
  char alias[BUFFER_LEN];

  while (thing != NOTHING) {
    a = atr_get_noparent(thing, type);
    if (a) {
      strcpy(alias, uncompress(a->value));
      if (check_alias(command, alias) != 0)
	return thing;		/* matched an alias */
    }
    thing = Next(thing);
  }
  return -1;
}

int
loc_alias_check(loc, command, type)
    dbref loc;
    const char *command;
    const char *type;
{
  ATTR *a;
  char alias[BUFFER_LEN];
  a = atr_get_noparent(loc, type);
  if (a) {
    strcpy(alias, uncompress(a->value));
    return (check_alias(command, alias));
  } else
    return 0;
}

int
Hearer(thing)
    dbref thing;
{
  ALIST *ptr;

  if (Connected(thing) || Puppet(thing))
    return (1);
  for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr)) {
#ifndef VISIBLE_EMPTY_ATTRS
    if (!*AL_STR(ptr))
      continue;
#endif
    if (!strcmp(AL_NAME(ptr), "LISTEN"))
      return 1;
  }
  return (0);
}

int
Commer(thing)
    dbref thing;
{
  ALIST *ptr;

  for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr)) {
#ifndef VISIBLE_EMPTY_ATTRS
    if (!*AL_STR(ptr))
      continue;
#endif
    if (AL_FLAGS(ptr) & AF_COMMAND && !(AL_FLAGS(ptr) & AF_NOPROG))
      return (1);
  }
  return (0);
}

int
Listener(thing)
    dbref thing;
{
  /* If a monitor flag is set on a room or thing, it's a listener.
   * Otherwise not (even if ^patterns are present)
   */
  return (IS(thing, TYPE_THING, THING_LISTEN) || IS(thing, TYPE_ROOM, ROOM_LISTEN));
}

void
do_poor(player, arg1)
    dbref player;
    char *arg1;
{
  int amt = atoi(arg1);
  dbref a;
  if (!God(player)) {
    notify(player, "Only God can cause financial ruin.");
    return;
  }
  for (a = 0; a < db_top; a++)
    if (IsPlayer(a))
      s_Pennies(a, amt);
  notify(player,
      tprintf("The money supply of all players has been reset to %d %s.",
	      amt, MONIES));
  do_log(LT_WIZ, player, NOTHING,
	 "** POOR done ** Money supply reset to %d %s.",
	 amt, MONIES);
  fflush(wizlog_fp);
}


void
do_writelog(player, str, ltype)
    dbref player;
    char *str;
    int ltype;
{
  if (!Wizard(player)) {
    notify(player, "Permission denied.");
    return;
  }
  do_rawlog(ltype, "LOG: %s(#%d%s): %s", Name(player), player,
	    unparse_flags(player, GOD), str);

  notify(player, "Logged.");
}

/* Bind occurences of '##' in "action" to "arg", then run "action" */

void
bind_and_queue(player, cause, action, arg)
    dbref player;
    dbref cause;
    char *action;
    char *arg;
{
  char *repl, *command;

  repl = replace_string("##", arg, action);
  command = strip_braces(repl);

  if (repl)
    free(repl);
#ifdef MEM_CHECK
  del_check("replace_string.buff");
#endif

  parse_que(player, command, cause);

  if (command)
    free(command);
#ifdef MEM_CHECK
  del_check("strip_braces.buff");
#endif
}

void
do_scan(player, command, flag)
    dbref player;
    char *command;
    int flag;
{
  /* scan for possible matches of user-def'ed commands */

#define ScanFind(p,x)  \
  (Can_Examine(p,x) && \
      ((num = atr_comm_match(x, p, '$', ':', command, 1)) != 0))


  dbref thing;
  int num;

  if (!GoodObject(Location(player))) {
    notify(player, "Sorry, you are in an invalid location.");
    return;
  }
  if (!command || !*command) {
    notify(player, "What command do you want to scan for?");
    return;
  }
  if (flag & CHECK_NEIGHBORS) {
    notify(player, "Matches on contents of this room:");
    DOLIST(thing, Contents(Location(player))) {
      if (ScanFind(player, thing))
	notify(player,
	       tprintf("%s  [%d]", unparse_object(player, thing), num));
    }
  }
  if (flag & CHECK_HERE) {
    if (ScanFind(player, Location(player)))
      notify(player, tprintf("Matched here: %s  [%d]",
			 unparse_object(player, Location(player)), num));
  }
  if (flag & CHECK_INVENTORY) {
    notify(player, "Matches on carried objects:");
    DOLIST(thing, Contents(player)) {
      if (ScanFind(player, thing))
	notify(player, tprintf("%s  [%d]",
			       unparse_object(player, thing), num));
    }
  }
  if (flag & CHECK_SELF) {
    if (ScanFind(player, player))
      notify(player, tprintf("Matched self: %s  [%d]",
			     unparse_object(player, player), num));
  }
  if (flag & CHECK_ZONE) {
    /* zone checks */
    if (Zone(Location(player)) != NOTHING) {
      if (IsRoom(Zone(Location(player)))) {
	/* zone of player's location is a zone master room */
	if (Location(player) != Zone(player)) {
	  notify(player, "Matches on zone master room of location:");
	  DOLIST(thing, Contents(Zone(Location(player)))) {
	    if (ScanFind(player, thing))
	      notify(player, tprintf("%s  [%d]",
				     unparse_object(player, thing), num));
	  }
	}
      } else {
	/* regular zone object */
	if (ScanFind(player, Zone(Location(player))))
	  notify(player,
		 tprintf("Matched zone of location: %s  [%d]",
		   unparse_object(player, Zone(Location(player))), num));
      }
    }
    if ((Zone(player) != NOTHING) &&
	(Zone(player) != Zone(Location(player)))) {
      /* check the player's personal zone */
      if (ScanFind(player, Zone(player)))
	notify(player, tprintf("Matched personal zone: %s  [%d]",
			     unparse_object(player, Zone(player)), num));
    }
  }
  if (DO_GLOBALS && (flag & CHECK_GLOBAL) &&
      (Location(player) != MASTER_ROOM) &&
      (Zone(Location(player)) != MASTER_ROOM) &&
      (Zone(player) != MASTER_ROOM)) {
    /* try Master Room stuff */
    notify(player, "Matches on objects in the Master Room:");
    DOLIST(thing, Contents(MASTER_ROOM)) {
      if (ScanFind(player, thing))
	notify(player, tprintf("%s  [%d]",
			       unparse_object(player, thing), num));
    }
  }
}

void
do_dolist(player, list, command, cause, flag, notify_flag)
    dbref player;
    char *list, *command;
    dbref cause;
    int flag;			/* 0 for @dolist, 1 for @map */
    int notify_flag;		/* execute '@notify me' at end? */
{
  char *curr, *objstring;
  char outbuf[BUFFER_LEN];
  char *ebuf1, *ebuf2, *bp;
  int place;
  char placestr[10];
  int j;

  if (!command || !*command) {
    notify(player, "What do you want to do with the list?");
    if (notify_flag)
      parse_que(player, "@notify me", cause);
    return;
  }
  /* set up environment for any spawned commands */
  for (j = 0; j < 10; j++) {
    wnxt[j] = wenv[j];
    rnxt[j] = renv[j];
  }

  bp = outbuf;
  curr = list;
  place = 0;
  while (curr && *curr) {
    while (*curr == ' ')
      curr++;
    if (*curr) {
      place++;
      sprintf(placestr, "%d", place);
      objstring = ebuf1 = curr;
      process_expression(objstring, &ebuf1, (char const **) &curr,
			 player, cause, cause,
			 PE_NOTHING, PT_SPACE, NULL);
      if (*curr == ' ')
	*curr++ = '\0';
      if (!flag) {
	/* @dolist, queue command */
	ebuf1 = replace_string("#@", placestr, command);
	bind_and_queue(player, cause, ebuf1, objstring);
#ifdef MEM_CHECK
	del_check("replace_string.buff");
#endif
	free(ebuf1);
      } else {
	/* it's @map, add to the output list */
	if (bp != outbuf)
	  safe_chr(' ', outbuf, &bp);
	ebuf1 = replace_string("##", objstring, command);
	ebuf2 = replace_string("#@", placestr, ebuf1);
	free(ebuf1);
#ifdef MEM_CHECK
	del_check("replace_string.buff");
#endif
	ebuf1 = ebuf2;
	process_expression(outbuf, &bp, (char const **) &ebuf1,
			   player, cause, cause,
			   PE_DEFAULT, PT_DEFAULT, NULL);
	free(ebuf2);
#ifdef MEM_CHECK
	del_check("replace_string.buff");
#endif
      }
    }
  }

  *bp = '\0';

  if (flag) {
    /* if we're doing a @map, copy the list to an attribute */
    atr_add(player, "MAPLIST", outbuf, GOD, NOTHING);
    notify(player, "Function mapped onto list.");
  }
  if (notify_flag) {
    /*  Execute a '@notify me' so the object knows we're done
     *  with the list execution. We don't execute nfy_que()
     *  directly, since we want the command to be queued
     *  _after_ the list has executed.
     */
    parse_que(player, "@notify me", cause);
  }
}

void
do_uptime(player)
    dbref player;
{
  char tbuf1[BUFFER_LEN];
  char *p;

#ifdef HAS_UPTIME
  FILE *fp;
  char c;
  int i;
#endif
  int pid;
#ifndef hpux
  int psize;
#endif
  int sec, min;
#ifdef HAS_GETRUSAGE
  struct rusage usage;
#endif				/* HAS_GETRUSAGE */

  /* calculate time until next dump */
  min = (options.dump_counter - mudtime) / 60;
  sec = (options.dump_counter - mudtime) % 60;

  sprintf(tbuf1, "Up since: %s", ctime(&start_time));
  if ((p = strchr(tbuf1, '\n')))
    *p = '\0';
  notify(player, tbuf1);

  sprintf(tbuf1, "Time now: %s", ctime(&mudtime));
  if ((p = strchr(tbuf1, '\n')))
    *p = '\0';
  notify(player, tbuf1);

  if (!Wizard(player)) {
    notify(player,
	 tprintf("Time until next database save: %d minutes %d seconds.",
		 min, sec));
    return;
  }
#ifdef HAS_UPTIME
  fp = popen(UPTIME_PATH, "r");

  /* just in case the system is screwy */
  if (fp == NULL) {
    notify(player, "Error -- cannot execute uptime.");
    fprintf(stderr, "** ERROR ** popen for @uptime returned NULL.");
    return;
  }
  /* print system uptime */
  for (i = 0; (c = getc(fp)) != '\n'; i++)
    tbuf1[i] = c;
  tbuf1[i] = '\0';
  pclose(fp);

  notify(player, tbuf1);
#endif

  /* do process stats */

  pid = getpid();
#ifndef hpux
#ifdef WIN32
  psize = 1024;			/* NJG: a guess! */
#else
#ifdef macintosh
  psize = 1024;
#else
  psize = getpagesize();
#endif                  /* macintosh */
#endif                  /* WIN32 */
  notify(player, tprintf("\nProcess ID:  %10d        %10d bytes per page",
			 pid, psize));
#else
  notify(player, tprintf("\nProcess ID:  %10d", pid));
#endif				/* hpux  */

#ifdef HAS_GETRUSAGE
  getrusage(RUSAGE_SELF, &usage);
  notify(player, tprintf("Time used:   %10ld user   %10ld sys",
			 usage.ru_utime.tv_sec, usage.ru_stime.tv_sec));
#ifndef hpux
  notify(player, tprintf("Max res mem: %10ld pages  %10ld bytes",
			 usage.ru_maxrss, (usage.ru_maxrss * psize)));
#else
  notify(player, tprintf("Max res mem: %10ld pages", usage.ru_maxrss));
#endif				/* hpux */
  notify(player, tprintf("Integral mem:%10ld shared %10ld private %10ld stack",
			 usage.ru_ixrss, usage.ru_idrss, usage.ru_isrss));
  notify(player, tprintf("Page faults: %10ld hard   %10ld soft    %10ld swapouts",
		      usage.ru_majflt, usage.ru_minflt, usage.ru_nswap));
  notify(player, tprintf("Disk I/O:    %10ld reads  %10ld writes",
			 usage.ru_inblock, usage.ru_oublock));
  notify(player, tprintf("Network I/O: %10ld in     %10ld out",
			 usage.ru_msgrcv, usage.ru_msgsnd));
  notify(player, tprintf("Context swi: %10ld vol    %10ld forced",
			 usage.ru_nvcsw, usage.ru_nivcsw));
  notify(player, tprintf("Signals:     %10ld", usage.ru_nsignals));
#endif				/* HAS_GETRUSAGE */

  notify(player, tprintf("The head of the object free list is #%d.",
			 first_free));
  notify(player,
	 tprintf("Time until next database save: %d minutes %d seconds.",
		 min, sec));
}


/* Open a db file, which may be compressed, and return a file pointer */
static FILE *
db_open(filename)
    const char *filename;
{
  FILE *f;
#ifndef macintosh
#ifndef WIN32
  if (options.uncompressprog && *options.uncompressprog) {
    /* We do this because on some machines (SGI Irix, for example),
     * the popen will not return NULL if the mailfile isn't there.
     */
    f = fopen(tprintf("%s%s", filename, options.compresssuff), "r");
    if (f) {
      fclose(f);
      f = popen(tprintf("%s < %s%s", options.uncompressprog, filename, options.compresssuff), "r");
    }
  } else
#endif  /* WIN32 */
#endif  /* macintosh */
  {
    f = fopen(filename, "r");
  }
  return f;
}

/* Open a file or pipe (if compressing) for writing */
static FILE *
db_open_write(filename)
    const char *filename;
{
  FILE *f;
#ifndef macintosh
#ifndef WIN32
  if (options.compressprog && *options.compressprog) {
    f = popen(tprintf("%s >%s%s", options.compressprog, filename, options.compresssuff), "w");
  } else
#endif  /* WIN32 */
#endif  /* macintosh */
  {
    f = fopen(filename, "w");
  }
  return f;
}


/* Close a db file, which may really be a pipe */
static void
db_close(f)
    FILE *f;
{
#ifndef macintosh
#ifndef WIN32
  if (options.compressprog && *options.compressprog) {
    pclose(f);
  } else
#endif  /* WIN32 */
#endif  /* macintosh */
  {
    fclose(f);
  }
}

void
do_list(player, arg)
    dbref player;
    char *arg;
{
  if (!arg || !*arg)
    notify(player, "I don't understand what you want to @list.");
  else if (string_prefix("commands", arg))
    do_list_commands(player);
  else if (string_prefix("functions", arg))
    do_list_functions(player);
  else if (string_prefix("motd", arg))
    do_motd(player, 3, "");
  else if (string_prefix("attribs", arg))
    do_list_attribs(player);
  else
    notify(player, "I don't understand what you want to @list.");
}

char *
make_new_epoch_file(basename, the_epoch)
    const char *basename;
    int the_epoch;
{
  static char result[BUFFER_LEN];	/* STATIC! */
  /* Unlink the last the_epoch and create a new one */

  sprintf(result, "%s.#%d#", basename, the_epoch - 1);
  unlink(result);
  sprintf(result, "%s.#%d#", basename, the_epoch);
  return result;
}
