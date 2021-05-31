/**
 * \file game.c
 *
 * \brief The main game driver.
 *
 *
 */

#include "copyrite.h"
#include "config.h"

#include <ctype.h>
#include <fcntl.h>
#include <string.h>
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
#include <windows.h>
#undef OPAQUE			/* Clashes with flags.h */
void Win32MUSH_setup(void);
#endif
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#ifdef HAS_GETRUSAGE
#include <sys/resource.h>
#endif
#include <stdlib.h>
#ifdef I_UNISTD
#include <unistd.h>
#endif
#include <setjmp.h>

#ifdef macintosh
extern void PMSSetupDatabases(void);
#endif

#include "conf.h"
#include "mushdb.h"
#include "game.h"
#include "externs.h"
#include "attrib.h"
#include "match.h"
#include "case.h"
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
#include "strtree.h"
#include "command.h"
#include "htab.h"
#include "ptab.h"
#include "log.h"
#include "lock.h"
#include "dbdefs.h"
#include "flags.h"
#include "function.h"
#include "help.h"

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
time_t start_time;		/* MUSH start time (since process exec'd) */
time_t first_start_time = 0;	/* MUSH start time (since last shutdown) */
time_t last_dump_time = 0;	/* Time of last successful db save */
int reboot_count = 0;
static int epoch = 0;
int reserved;
int depth = 0;			/* excessive recursion prevention */
extern int invok_counter;	/* function recursion prevention */
extern dbref cplr;
extern char ccom[];

extern void initialize_mt(void);

int paranoid_dump = 0;		/* if paranoid, scan before dumping */
int paranoid_checkpt = 0;	/* write out an okay message every x objs */
extern long indb_flags;
extern void conf_default_set(void);
static int dump_database_internal(void);
static FILE *db_open(const char *filename);
static FILE *db_open_write(const char *filename);
static void db_close(FILE * f);
void do_readcache(dbref player);
char **argv_hack(dbref player, dbref cause, char const *arg,
		 char *fargs[], int eflags);
int check_alias(const char *command, const char *list);
int list_check(dbref thing, dbref player, char type,
	       char end, char *str, int just_match);
int alias_list_check(dbref thing, const char *command, const char *type);
int loc_alias_check(dbref loc, const char *command, const char *type);
void do_poor(dbref player, char *arg1);
void do_writelog(dbref player, char *str, int ltype);
void bind_and_queue(dbref player, dbref cause, char *action, const char *arg,
		    const char *placestr);
void do_scan(dbref player, char *command, int flag);
void do_list(dbref player, char *arg, int lc);
void do_dolist(dbref player, char *list, char *command,
	       dbref cause, unsigned int flags);
void do_uptime(dbref player, int mortal);
static char *make_new_epoch_file(const char *basename, int the_epoch);
void dest_info(dbref thing, dbref tt);
#ifdef HAS_GETRUSAGE
void rusage_stats(void);
#endif

void do_list_memstats(dbref player);
void st_stats_header(dbref player);
void st_stats(dbref player, StrTree *root, const char *name);
void do_timestring(char *buff, char **bp, const char *format,
		   unsigned long secs);

dbref orator = NOTHING;

#ifdef COMP_STATS
extern void compress_stats(long *entries,
			   long *mem_used,
			   long *total_uncompressed, long *total_compressed);
#endif



Pid_t forked_dump_pid = -1;

/** User command to dump the database.
 * \verbatim
 * This implements the @dump command.
 * \endverbatim
 * \param player the enactor, for permission checking.
 * \param num checkpoint interval, as a string.
 * \param flag type of dump.
 */
void
do_dump(dbref player, char *num, enum dump_type flag)
{
  if (Wizard(player)) {
#ifdef ALWAYS_PARANOID
    if (1) {
#else
    if (flag != DUMP_NORMAL) {
#endif
      /* want to do a scan before dumping each object */
      paranoid_dump = 1;
      if (num && *num) {
	/* checkpoint interval given */
	paranoid_checkpt = atoi(num);
	if ((paranoid_checkpt < 1) || (paranoid_checkpt >= db_top)) {
	  notify(player, T("Permission denied. Invalid checkpoint interval."));
	  paranoid_dump = 0;
	  return;
	}
      } else {
	/* use a default interval */
	paranoid_checkpt = db_top / 5;
	if (paranoid_checkpt < 1)
	  paranoid_checkpt = 1;
      }
      if (flag == DUMP_PARANOID) {
	notify_format(player, T("Paranoid dumping, checkpoint interval %d."),
		      paranoid_checkpt);
	do_rawlog(LT_CHECK,
		  "*** PARANOID DUMP *** done by %s(#%d),\n",
		  Name(player), player);
      } else {
	notify_format(player, T("Debug dumping, checkpoint interval %d."),
		      paranoid_checkpt);
	do_rawlog(LT_CHECK,
		  "*** DEBUG DUMP *** done by %s(#%d),\n",
		  Name(player), player);
      }
      do_rawlog(LT_CHECK, T("\tcheckpoint interval %d, at %s"),
		paranoid_checkpt, ctime(&mudtime));
    } else {
      /* normal dump */
      paranoid_dump = 0;	/* just to be safe */
      notify(player, "Dumping...");
      do_rawlog(LT_CHECK, "** DUMP ** done by %s(#%d) at %s",
		Name(player), player, ctime(&mudtime));
    }
    fork_and_dump(1);
    paranoid_dump = 0;
  } else {
    notify(player, T("Sorry, you are in a no dumping zone."));
  }
}


/** Print global variables to the trace log.
 * This function is used for error reporting.
 */
void
report(void)
{
  if (GoodObject(cplr))
    do_rawlog(LT_TRACE, T("TRACE: Cmd:%s\tdepth:%d\tby #%d at #%d"), ccom,
	      depth, cplr, Location(cplr));
  else
    do_rawlog(LT_TRACE, "TRACE: Cmd:%s\tdepth:%d\tby #%d", ccom, depth, cplr);
}

#ifdef HAS_GETRUSAGE
/** Log process statistics to the error log.
 */
void
rusage_stats(void)
{
  struct rusage usage;
  int pid;
  int psize;

  pid = getpid();
  psize = getpagesize();
  getrusage(RUSAGE_SELF, &usage);

  do_rawlog(LT_ERR, T("Process statistics:"));
  do_rawlog(LT_ERR, T("Time used:   %10ld user   %10ld sys"),
	    usage.ru_utime.tv_sec, usage.ru_stime.tv_sec);
  do_rawlog(LT_ERR, "Max res mem: %10ld pages  %10ld bytes",
	    usage.ru_maxrss, (usage.ru_maxrss * psize));
  do_rawlog(LT_ERR, "Integral mem:%10ld shared %10ld private %10ld stack",
	    usage.ru_ixrss, usage.ru_idrss, usage.ru_isrss);
  do_rawlog(LT_ERR,
	    T("Page faults: %10ld hard   %10ld soft    %10ld swapouts"),
	    usage.ru_majflt, usage.ru_minflt, usage.ru_nswap);
  do_rawlog(LT_ERR, T("Disk I/O:    %10ld reads  %10ld writes"),
	    usage.ru_inblock, usage.ru_oublock);
  do_rawlog(LT_ERR, T("Network I/O: %10ld in     %10ld out"), usage.ru_msgrcv,
	    usage.ru_msgsnd);
  do_rawlog(LT_ERR, T("Context swi: %10ld vol    %10ld forced"),
	    usage.ru_nvcsw, usage.ru_nivcsw);
  do_rawlog(LT_ERR, "Signals:     %10ld", usage.ru_nsignals);
}

#endif				/* HAS_GETRUSAGE */

/** User interface to shut down the MUSH.
 * \verbatim
 * This implements the @shutdown command.
 * \endverbatim
 * \param player the enactor, for permission checking.
 * \param flag type of shutdown to perform.
 */
void
do_shutdown(dbref player, enum shutdown_type flag)
{
  if (flag == SHUT_PANIC && !God(player)) {
    notify(player, T("It takes a God to make me panic."));
    return;
  }
  if (Wizard(player)) {
    flag_broadcast(0, 0, T("GAME: Shutdown by %s"), Name(player));
    do_log(LT_ERR, player, NOTHING, T("SHUTDOWN by %s(%s)\n"),
	   Name(player), unparse_dbref(player));

    /* This will create a file used to check if a restart should occur */
#ifdef AUTORESTART
    system("touch NORESTART");
#endif

    if (flag == SHUT_PANIC) {
      panic("@shutdown/panic");
    } else {
      if (flag == SHUT_PARANOID) {
	paranoid_checkpt = db_top / 5;
	if (paranoid_checkpt < 1)
	  paranoid_checkpt = 1;
	paranoid_dump = 1;
      }
      shutdown_flag = 1;
    }
  } else {
    notify(player, T("Your delusions of grandeur have been duly noted."));
  }
}

jmp_buf db_err;

static int
dump_database_internal(void)
{
  char realdumpfile[2048];
  char realtmpfl[2048];
  char tmpfl[2048];
  FILE *f = NULL;

#ifndef WIN32
  ignore_signal(SIGPROF);
#endif
#ifdef I_SETJMP
  if (setjmp(db_err)) {
    /* The dump failed. Disk might be full or something went bad with the
       compression slave. Boo! */
    do_rawlog(LT_ERR, T("ERROR! Database save failed."));
    flag_broadcast("WIZARD ROYALTY", 0,
		   T("GAME: ERROR! Database save failed!"));
#ifdef HAS_ITIMER
    install_sig_handler(SIGPROF, signal_cpu_limit);
#endif
    return 1;
  } else {
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
	db_write(f, 0);
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
      if (rename(realtmpfl, realdumpfile) < 0) {
	perror(realtmpfl);
	longjmp(db_err, 1);
      }
    } else {
      perror(realtmpfl);
      longjmp(db_err, 1);
    }
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
	if (rename(realtmpfl, realdumpfile) < 0) {
	  perror(realtmpfl);
	  longjmp(db_err, 1);
	}
      } else {
	perror(realtmpfl);
	longjmp(db_err, 1);
      }
    }
#endif				/* USE_MAILER */
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
      if (rename(realtmpfl, realdumpfile) < 0) {
	perror(realtmpfl);
	longjmp(db_err, 1);
      }
    } else {
      perror(realtmpfl);
      longjmp(db_err, 1);
    }
#endif				/* CHAT_SYSTEM */
    time(&last_dump_time);
  }

#endif
#ifdef HAS_ITIMER
  install_sig_handler(SIGPROF, signal_cpu_limit);
#endif

  return 0;
}

/** Crash gracefully.
 * This function is called when something disastrous happens - typically
 * a failure to malloc memory or a signal like segfault.
 * It logs the fault, does its best to dump a panic database, and
 * exits abruptly. This function does not return.
 * \param message message to log to the error log.
 */
void
panic(const char *message)
{
  const char *panicfile = options.crash_db;
  FILE *f = NULL;

  do_rawlog(LT_ERR, "PANIC: %s", message);
  report();
  flag_broadcast(0, 0, "EMERGENCY SHUTDOWN: %s", message);

  /* turn off signals */
  block_signals();

  /* shut down interface */
  emergency_shutdown();

  /* dump panic file */
  if (setjmp(db_err)) {
    /* Dump failed. We're in deep doo-doo */
    do_rawlog(LT_ERR, T("CANNOT DUMP PANIC DB. OOPS."));
    _exit(134);
  } else {
#ifdef macintosh
    if ((f = fopen(panicfile, "wb")) == NULL) {
#else
    if ((f = fopen(panicfile, "w")) == NULL) {
#endif
      do_rawlog(LT_ERR, T("CANNOT OPEN PANIC FILE, YOU LOSE"));
      _exit(135);
    } else {
      do_rawlog(LT_ERR, T("DUMPING: %s"), panicfile);
      db_write(f, DBF_PANIC);
#ifdef USE_MAILER
      dump_mail(f);
#endif
#ifdef CHAT_SYSTEM
      save_chatdb(f);
#endif
      fclose(f);
      do_rawlog(LT_ERR, T("DUMPING: %s (done)"), panicfile);
      _exit(136);
    }
  }
}

/** Dump the database.
 * This function is a wrapper for dump_database_internal() that does
 * a little logging before and after the dump.
 */
void
dump_database(void)
{
  epoch++;

  do_rawlog(LT_ERR, "DUMPING: %s.#%d#", dumpfile, epoch);
  dump_database_internal();
  do_rawlog(LT_ERR, "DUMPING: %s.#%d# (done)", dumpfile, epoch);
}

/** Dump a database, possibly by forking the process.
 * This function calls dump_database_internal() to dump the MUSH
 * databases. If we're configured to do so, it forks first, so that
 * the child process can perform the dump while the parent continues
 * to run the MUSH for the players. If we can't fork, this function
 * warns players online that a dump is taking place and the game
 * may pause.
 * \param forking if 1, attempt a forking dump.
 */
void
fork_and_dump(int forking)
{
  int child, nofork, status;
  epoch++;

  do_rawlog(LT_CHECK, "CHECKPOINTING: %s.#%d#\n", dumpfile, epoch);
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
      do_log(LT_ERR, 0, 0,
	     "fork_and_dump: fork() failed! Dumping nofork instead.");
      if (DUMP_NOFORK_MESSAGE && *DUMP_NOFORK_MESSAGE)
	flag_broadcast(0, 0, "%s", DUMP_NOFORK_MESSAGE);
      child = 0;
      nofork = 1;
    } else if (child > 0)
      forked_dump_pid = child;

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
      flag_broadcast(0, 0, "%s", DUMP_NOFORK_MESSAGE);
    child = 0;
  }
  if (nofork || (!nofork && child == 0)) {
    /* in the child */
#ifndef WIN32
    close(reserved);		/* get that file descriptor back */
#endif
    status = dump_database_internal();
    if (!nofork) {
      _exit(status);		/* !!! */
    } else {
#ifndef WIN32
      reserved = open("/dev/null", O_RDWR);
#endif
      if (DUMP_NOFORK_COMPLETE && *DUMP_NOFORK_COMPLETE)
	flag_broadcast(0, 0, "%s", DUMP_NOFORK_COMPLETE);
    }
  }
}

/** Start up the MUSH.
 * This function does all of the work that's necessary to start up
 * MUSH objects and code for the game. It sets up player aliases,
 * fixes null object names, and triggers all object startups.
 */
void
do_restart(void)
{
  dbref thing;
  ATTR *s;
  char buf[BUFFER_LEN];
  char *bp;
  int j;

  /* Do stuff that needs to be done for players only: add stuff to the
   * alias table, and refund money from queued commands at shutdown.
   */
  for (thing = 0; thing < db_top; thing++) {
    if (IsPlayer(thing)) {
      if ((s = atr_get_noparent(thing, "ALIAS")) != NULL) {
	bp = buf;
	safe_str(uncompress(s->value), buf, &bp);
	*bp = '\0';
	add_player(thing, buf);
      }
    }
  }

  /* Once we load all that, then we can trigger the startups and 
   * begin queueing commands. Also, let's make sure that we get
   * rid of null names.
   */
  for (j = 0; j < 10; j++)
    wnxt[j] = NULL;
  for (j = 0; j < NUMQ; j++)
    rnxt[j] = NULL;

  for (thing = 0; thing < db_top; thing++) {
    if (Name(thing) == NULL) {
      if (IsGarbage(thing))
	set_name(thing, "Garbage");
      else {
	do_log(LT_ERR, NOTHING, NOTHING, T("Null name on object #%d"), thing);
	set_name(thing, "XXXX");
      }
    }
    if (!IsGarbage(thing) && !(Halted(thing)))
      (void) queue_attribute_noparent(thing, "STARTUP", thing);
  }
}

extern void init_names(void);
extern struct db_stat_info current_state;

/** Initialize game structures and read the most of the configuration file.
 * This function runs before we read in the databases. It is responsible
 * for recording the MUSH start time, setting up all the hash and 
 * prefix tables and similar structures, and reading the portions of the
 * config file that don't require database load.
 * \param conf file name of the configuration file.
 */
void
init_game_config(const char *conf)
{
  int a;

  depth = 0;

  for (a = 0; a < 10; a++) {
    wenv[a] = NULL;
    wnxt[a] = NULL;
  }
  for (a = 0; a < NUMQ; a++) {
    renv[a][0] = '\0';
    rnxt[a] = NULL;
  }

  /* set MUSH start time */
  start_time = time((time_t *) 0);
  if (!first_start_time)
    first_start_time = start_time;
  do_rawlog(LT_ERR, "%s", VERSION);
  do_rawlog(LT_ERR, T("MUSH restarted, PID %d, at %s"),
	    (int) getpid(), ctime(&start_time));

  /* initialize all the hash and prefix tables */
  init_flag_table();
  init_func_hashtab();
  init_math_hashtab();
  init_tag_hashtab();
  init_aname_table();
  init_atr_name_tree();
  init_locks();
  init_names();
  init_pronouns();

  memset(&current_state, 0, sizeof current_state);

  /* Load all the config file stuff except restrict_* */
  conf_default_set();
  config_file_startup(conf, 0);
}

/** Post-db-load configuration.
 * This function contains code that should be run after dbs are loaded 
 * (usually because we need to have the flag table loaded, or because they 
 * run last). It reads in the portions of the config file that rely
 * on flags being defined.
 * \param conf file name of the configuration file.
 */
void
init_game_postdb(const char *conf)
{
  /* access file stuff */
  read_access_file();
  /* initialize random number generator */
  initialize_mt();
  /* set up signal handlers for the timer */
  init_timer();
  /* Commands and functions require the flag table for restrictions */
  command_init_preconfig();
  command_init_postconfig();
  function_init_postconfig();
  /* Load further restrictions from config file */
  config_file_startup(conf, 1);
  /* Call Local Startup */
  local_startup();
  /* everything else ok. Restart all objects. */
  do_restart();
}


/** Read the game databases.
 * This function reads in the object, mail, and chat databases.
 * \retval -1 error.
 * \retval 0 success.
 */
int
init_game_dbs(void)
{
  FILE *f;

  const char *infile, *outfile;
#ifdef USE_MAILER
  const char *mailfile;
#endif
  int panicdb;

#ifdef WIN32
  Win32MUSH_setup();		/* create index files, copy databases etc. */
#endif
#ifdef macintosh
  PMSSetupDatabases();
#endif

  infile = restarting ? options.output_db : options.input_db;
  outfile = options.output_db;
#ifdef USE_MAILER
  mailfile = options.mail_db;
#endif

  /* read small text files into cache */
  fcache_init();

  f = db_open(infile);

  if (!f)
    return -1;

  /* ok, read it in */
  do_rawlog(LT_ERR, "ANALYZING: %s", infile);
  if (init_compress(f) < 0) {
    do_rawlog(LT_ERR, "ERROR LOADING");
    return -1;
  }
  do_rawlog(LT_ERR, "ANALYZING: %s (done)", infile);

  /* everything ok */
  db_close(f);

  f = db_open(infile);
  if (!f)
    return -1;

  /* ok, read it in */
  do_rawlog(LT_ERR, "LOADING: %s", infile);
  if (db_read(f) < 0) {
    do_rawlog(LT_ERR, "ERROR LOADING");
    return -1;
  }
  do_rawlog(LT_ERR, "LOADING: %s (done)", infile);

  /* If there's stuff at the end of the db, we may have a panic
   * format db, with everything shoved together. In that case,
   * don't close the file
   */
  panicdb = ((indb_flags & DBF_PANIC) && !feof(f));

  /* everything ok */
  if (!panicdb)
    db_close(f);

  /* complain about bad config options */
  if (!GoodObject(PLAYER_START) || (!IsRoom(PLAYER_START)))
    do_rawlog(LT_ERR, T("WARNING: Player_start (#%d) is NOT a room."),
	      PLAYER_START);
  if (!GoodObject(MASTER_ROOM) || (!IsRoom(MASTER_ROOM)))
    do_rawlog(LT_ERR, T("WARNING: Master room (#%d) is NOT a room."),
	      MASTER_ROOM);
  if (!GoodObject(GOD) || (!IsPlayer(GOD)))
    do_rawlog(LT_ERR, T("WARNING: God (#%d) is NOT a player."), GOD);

#ifdef USE_MAILER
  /* read mail database */
  if (panicdb) {
    do_rawlog(LT_ERR, T("LOADING: Trying to get mail from %s"), infile);
    if (load_mail(f) <= 0) {
      do_rawlog(LT_ERR, T("FAILED: Reverting to normal maildb"));
      db_close(f);
      panicdb = 0;
    }
  }
  if (!panicdb) {
    f = db_open(mailfile);

    /* okay, read it in */
    if (f == NULL) {
      mail_init();
    } else {
      do_rawlog(LT_ERR, "LOADING: %s", mailfile);
      load_mail(f);
      do_rawlog(LT_ERR, "LOADING: %s (done)", mailfile);
      db_close(f);
    }
  }
#endif				/* USE_MAILER */

#ifdef CHAT_SYSTEM
  init_chatdb();

  if (panicdb) {
    do_rawlog(LT_ERR, T("LOADING: Trying to get chat from %s"), infile);
    if (load_chatdb(f) <= 0) {
      do_rawlog(LT_ERR, T("FAILED: Reverting to normal chatdb"));
      db_close(f);
      panicdb = 0;
    }
  }
  if (!panicdb) {
    f = db_open(options.chatdb);
    if (f) {
      do_rawlog(LT_ERR, "LOADING: %s", options.chatdb);
      if (load_chatdb(f)) {
	do_rawlog(LT_ERR, "LOADING: %s (done)", options.chatdb);
	db_close(f);
      } else {
	do_rawlog(LT_ERR, "ERROR LOADING %s", options.chatdb);
	db_close(f);
	return -1;
      }
    }
  }
#endif

  if (panicdb)
    db_close(f);

  /* set up dumper */
  strcpy(dumpfile, outfile);

  return 0;
}

/** Read cached text files.
 * \verbatim
 * This implements the @readcache function.
 * \endverbatim
 * \param player the enactor, for permission checking.
 */
void
do_readcache(dbref player)
{
  if (!Wizard(player)) {
    notify(player, T("Permission denied."));
    return;
  }
  fcache_load(player);
  help_reindex(player);
}

#define list_match(x)        list_check(x, player, '$', ':', cptr, 0)
#define cmd_match(x)         atr_comm_match(x, player, '$', ':', cptr, 0, NULL, NULL);

/** Attempt to match and execute a command.
 * This function performs some sanity checks and then attempts to
 * run a command. It checks, in order: home, built-in commands,
 * enter aliases, leave aliases, $commands on neighboring objects or
 * the player, $commands on the container, $commands on inventory,
 * exits in the zone master room, $commands on objects in the ZMR,
 * $commands on the ZMO, $commands on the player's zone, exits in the
 * master room, and $commands on objectrs in the master room.
 *
 * When a command is directly input from a socket, we don't parse
 * the value in attribute sets.
 *
 * \param player the enactor.
 * \param command command to match and execute.
 * \param cause object which caused the command to be executed.
 * \param from_port if 1, the command was direct input from a socket.
 */
void
process_command(dbref player, char *command, dbref cause, int from_port)
{
  int a;
  char *p;			/* utility */

  char unp[BUFFER_LEN];		/* unparsed command */
  /* general form command arg0=arg1,arg2...arg10 */
  char temp[BUFFER_LEN];	/* utility */
  int i;			/* utility */
  char *cptr;

  depth = 0;
  if (!command) {
    do_log(LT_ERR, NOTHING, NOTHING, T("ERROR: No command!!!"));
    return;
  }
  /* robustify player */
  if (!GoodObject(player)) {
    do_log(LT_ERR, NOTHING, NOTHING, T("process_command bad player #%d"),
	   player);
    return;
  }

  /* Destroyed objects shouldn't execute commands */
  if (IsGarbage(player))
    /* No message - nobody to tell, and it's too easy to do to log. */
    return;
  /* Halted objects can't execute commands */
  /* And neither can halted players if the command isn't from_port */
  if (Halted(player) && (!IsPlayer(player) || !from_port)) {
    notify_format(Owner(player),
		  T("Attempt to execute command by halted object #%d"), player);
    return;
  }
  /* Players and things should not have invalid locations. This check
   * must be done _after_ the destroyed-object check.
   */
  if ((!GoodObject(Location(player)) ||
       (IsGarbage(Location(player)))) && Mobile(player)) {
    notify_format(Owner(player),
		  T("Invalid location on command execution: %s(#%d)"),
		  Name(player), player);
    do_log(LT_ERR, NOTHING, NOTHING,
	   T("Command attempted by %s(#%d) in invalid location #%d."),
	   Name(player), player, Location(player));
    moveto(player, PLAYER_START);	/* move it someplace valid */
  }
  orator = player;

  if (options.log_commands || Suspect(player))
    do_log(LT_CMD, player, 0, "%s", command);

  if Verbose
    (player)
      raw_notify(Owner(player), tprintf("#%d] %s", player, command));

  /* eat leading whitespace */
  while (*command && isspace((unsigned char) *command))
    command++;

  /* eat trailing whitespace */
  p = command + strlen(command) - 1;
  while (isspace((unsigned char) *p) && (p >= command))
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
      notify(player, T("You can't do that IC!"));
    else
      do_move(player, command, 0);
    return;
  }
  strcpy(unp, command);

  cptr = command_parse(player, cause, command, from_port);
  if (cptr) {
    a = 0;
    if (!Gagged(player) && Mobile(player)) {

      /* if the "player" is an exit or room, no need to do these checks */

      /* try matching enter aliases */
      if (Location(player) != NOTHING &&
	  (i = alias_list_check(Contents(Location(player)),
				cptr, "EALIAS")) != -1) {

	sprintf(temp, "#%d", i);
	do_enter(player, temp);
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
      if (GoodObject(Location(player))) {
	a += list_match(Contents(Location(player)));
	if (Location(player) != player)
	  a += cmd_match(Location(player));
      }
      if (Location(player) != player)
	a += list_match(Contents(player));

      /* now do check on zones */
      if ((!a) && (Zone(Location(player)) != NOTHING)) {
	if (IsRoom(Zone(Location(player)))) {
	  /* zone of player's location is a zone master room,
	   * so we check for exits and commands
	   */
	  /* check zone master room exits */
	  if (remote_exit(player, cptr)) {
	    if (!Mobile(player))
	      goto done;
	    else {
	      do_move(player, cptr, 2);
	      goto done;
	    }
	  } else
	    a += list_match(Contents(Zone(Location(player))));
	} else
	  a += cmd_match(Zone(Location(player)));
      }
      /* if nothing matched with zone master room/zone object, try
       * matching zone commands on the player's personal zone
       */
      if ((!a) && (Zone(player) != NOTHING) &&
	  (Zone(Location(player)) != Zone(player))) {
	if (IsRoom(Zone(player)))
	  /* Player's personal zone is a zone master room, so we
	   * also check commands on objects in that room
	   */
	  a += list_match(Contents(Zone(player)));
	else
	  a += cmd_match(Zone(player));
      }
      /* end of zone stuff */
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
    }				/* end of special checks */
    if (!a) {
      notify(player, T("Huh?  (Type \"help\" for help.)"));
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

/** Check to see if a string matches part of a semicolon-separated list.
 * \param command string to match.
 * \param list semicolon-separated list of aliases to match against.
 * \retval 1 string matched an alias.
 * \retval 0 string failed to match an alias.
 */
int
check_alias(const char *command, const char *list)
{
  /* check if a string matches part of a semi-colon separated list */
  const char *p;
  while (*list) {
    for (p = command; (*p && DOWNCASE(*p) == DOWNCASE(*list)
		       && *list != EXIT_DELIMITER); p++, list++) ;
    if (*p == '\0') {
      while (isspace((unsigned char) *list))
	list++;
      if (*list == '\0' || *list == EXIT_DELIMITER)
	return 1;		/* word matched */
    }
    /* didn't match. check next word in list */
    while (*list && *list++ != EXIT_DELIMITER) ;
    while (isspace((unsigned char) *list))
      list++;
  }
  /* reached the end of the list without matching anything */
  return 0;
}

/** Match a command or listen pattern against a list of things.
 * This function iterates through a list of things (using the object
 * Next pointer, so typically this is a list of contents of an object),
 * and checks each for an attribute matching a command/listen pattern.
 * \param thing first object on list.
 * \param player the enactor.
 * \param type type of attribute to match ('$' or '^')
 * \param end character that signals the end of the matchable portion (':')
 * \param str string to match against the attributes.
 * \param just_match if 1, don't execute the command on match.
 * \retval 1 a match was made.
 * \retval 0 no match was made.
 */
int
list_check(dbref thing, dbref player, char type, char end, char *str,
	   int just_match)
{
  int match = 0;

  while (thing != NOTHING) {
    if (atr_comm_match(thing, player, type, end, str, just_match, NULL, NULL))
      match = 1;
    thing = Next(thing);
  }
  return (match);
}

/** Match a command against an attribute of aliases.
 * This function iterates through a list of things (using the object
 * Next pointer, so typically this is a list of contents of an object),
 * and checks each for an attribute of aliases that might be matched
 * (as in EALIAS).
 * \param thing first object on list.
 * \param command command to attempt to match.
 * \param type name of attribute of aliases to match against.
 * \return dbref of first matching object, or -1 if none.
 */
int
alias_list_check(dbref thing, const char *command, const char *type)
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

/** Check a command against a list of aliases on a location
 * (as for LALIAS).
 * \param loc location with attribute of aliases.
 * \param command command to attempt to match.
 * \param type name of attribute of aliases to match against.
 * \retval 1 successful match.
 * \retval 0 failure.
 */
int
loc_alias_check(dbref loc, const char *command, const char *type)
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

/** Can an object hear?
 * This function determines if a given object can hear. A Hearer is:
 * a connected player, a puppet, an AUDIBLE object with a FORWARDLIST
 * attribute, or an object with a LISTEN attribute.
 * \param thing object to check.
 * \retval 1 object can hear.
 * \retval 0 object can't hear.
 */
int
Hearer(dbref thing)
{
  ALIST *ptr;
  int cmp;

  if (Connected(thing) || Puppet(thing))
    return 1;
  for (ptr = List(thing); ptr; ptr = AL_NEXT(ptr)) {
#ifndef VISIBLE_EMPTY_ATTRS
    if (!*AL_STR(ptr))
      continue;
#endif
    if (Audible(thing) && (strcmp(AL_NAME(ptr), "FORWARDLIST") == 0))
      return 1;
    cmp = strcoll(AL_NAME(ptr), "LISTEN");
    if (cmp == 0)
      return 1;
    if (cmp > 0)
      break;
  }
  return 0;
}

/** Might an object be responsive to commands?
 * This function determines if a given object might pick up a $command.
 * That is, if it has any attributes with $commands on them that are
 * not set no_command.
 * \param thing object to check.
 * \retval 1 object responds to commands. 
 * \retval 0 object doesn't respond to commands.
 */
int
Commer(dbref thing)
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

/** Is an object listening?
 * This function determines if a given object is a Listener. A Listener
 * is a thing or room that has the MONITOR flag set.
 * \param thing object to check.
 * \retval 1 object is a Listener.
 * \retval 0 object isn't listening with ^patterns.
 */
int
Listener(dbref thing)
{
  /* If a monitor flag is set on a room or thing, it's a listener.
   * Otherwise not (even if ^patterns are present)
   */
  return (ThingListen(thing) || RoomListen(thing));
}

/** Reset all players' money.
 * \verbatim
 * This function implements the @poor command. It probably belongs in 
 * rob.c, though.
 * \endverbatim
 * \param player the enactor, for permission checking.
 * \param arg1 the amount of money to reset all players to.
 */
void
do_poor(dbref player, char *arg1)
{
  int amt = atoi(arg1);
  dbref a;
  if (!God(player)) {
    notify(player, T("Only God can cause financial ruin."));
    return;
  }
  for (a = 0; a < db_top; a++)
    if (IsPlayer(a))
      s_Pennies(a, amt);
  notify_format(player,
		T("The money supply of all players has been reset to %d %s."),
		amt, MONIES);
  do_log(LT_WIZ, player, NOTHING,
	 T("** POOR done ** Money supply reset to %d %s."), amt, MONIES);
}


/** User interface to write a message to a log.
 * \verbatim
 * This function implements @log.
 * \endverbatim
 * \param player the enactor.
 * \param str message to write to the log.
 * \param ltype type of log to write to.
 */
void
do_writelog(dbref player, char *str, int ltype)
{
  if (!Wizard(player)) {
    notify(player, T("Permission denied."));
    return;
  }
  do_rawlog(ltype, "LOG: %s(#%d%s): %s", Name(player), player,
	    unparse_flags(player, GOD), str);

  notify(player, "Logged.");
}

/** Bind occurences of '##' in "action" to "arg", then run "action".
 * \param player the enactor.
 * \param cause object that caused command to run.
 * \param action command string which may contain tokens.
 * \param arg value for ## token.
 * \param placestr value for #@ token.
 */
void
bind_and_queue(dbref player, dbref cause, char *action, const char *arg,
	       const char *placestr)
{
  char *repl, *command;
  const char *replace[2];

  replace[0] = arg;
  replace[1] = placestr;

  repl = replace_string2(standard_tokens, replace, action);

  command = strip_braces(repl);

  mush_free(repl, "replace_string.buff");

  parse_que(player, command, cause);

  mush_free(command, "strip_braces.buff");
}

#define ScanFind(p,x)  \
  (Can_Examine(p,x) && \
      ((num = atr_comm_match(x, p, '$', ':', command, 1, atrname, &ptr)) != 0))

/** Scan for matches of $commands.
 * This function scans for possible matches of user-def'd commands from the
 * viewpoint of player, and return as a string.
 * It assumes that atr_comm_match() returns atrname with a leading space.
 * \param player the object from whose viewpoint to scan.
 * \param command the command to scan for matches to.
 * \return string of obj/attrib pairs with matching $commands.
 */
char *
scan_list(dbref player, char *command)
{
  static char tbuf[BUFFER_LEN];
  char *tp;
  dbref thing;
  char atrname[BUFFER_LEN];
  char *ptr;
  int num;

  if (!GoodObject(Location(player))) {
    strcpy(tbuf, T("#-1 INVALID LOCATION"));
    return tbuf;
  }
  if (!command || !*command) {
    strcpy(tbuf, T("#-1 NO COMMAND"));
    return tbuf;
  }
  tp = tbuf;
  ptr = atrname;
  DOLIST(thing, Contents(Location(player))) {
    if (ScanFind(player, thing)) {
      *ptr = '\0';
      safe_str(atrname, tbuf, &tp);
      ptr = atrname;
    }
  }
  ptr = atrname;
  if (ScanFind(player, Location(player))) {
    *ptr = '\0';
    safe_str(atrname, tbuf, &tp);
  }
  ptr = atrname;
  DOLIST(thing, Contents(player)) {
    if (ScanFind(player, thing)) {
      *ptr = '\0';
      safe_str(atrname, tbuf, &tp);
      ptr = atrname;
    }
  }
  /* zone checks */
  ptr = atrname;
  if (Zone(Location(player)) != NOTHING) {
    if (IsRoom(Zone(Location(player)))) {
      /* zone of player's location is a zone master room */
      if (Location(player) != Zone(player)) {
	DOLIST(thing, Contents(Zone(Location(player)))) {
	  if (ScanFind(player, thing)) {
	    *ptr = '\0';
	    safe_str(atrname, tbuf, &tp);
	    ptr = atrname;
	  }
	}
      }
    } else {
      /* regular zone object */
      if (ScanFind(player, Zone(Location(player)))) {
	*ptr = '\0';
	safe_str(atrname, tbuf, &tp);
      }
    }
  }
  ptr = atrname;
  if ((Zone(player) != NOTHING) && (Zone(player) != Zone(Location(player)))) {
    /* check the player's personal zone */
    if (IsRoom(Zone(player))) {
      if (Location(player) != Zone(player)) {
	DOLIST(thing, Contents(Zone(player))) {
	  if (ScanFind(player, thing)) {
	    *ptr = '\0';
	    safe_str(atrname, tbuf, &tp);
	    ptr = atrname;
	  }
	}
      }
    } else if (ScanFind(player, Zone(player))) {
      *ptr = '\0';
      safe_str(atrname, tbuf, &tp);
    }
  }
  ptr = atrname;
  if ((Location(player) != MASTER_ROOM)
      && (Zone(Location(player)) != MASTER_ROOM)
      && (Zone(player) != MASTER_ROOM)) {
    /* try Master Room stuff */
    DOLIST(thing, Contents(MASTER_ROOM)) {
      if (ScanFind(player, thing)) {
	*ptr = '\0';
	safe_str(atrname, tbuf, &tp);
	ptr = atrname;
      }
    }
  }
  *tp = '\0';
  if (*tbuf && *tbuf == ' ')
    return tbuf + 1;		/* atrname comes with leading spaces */
  return tbuf;
}

/** User interface to scan for $command matches.
 * \verbatim
 * This function implements @scan.
 * \endverbatim
 * \param player the enactor.
 * \param command command to scan for matches to.
 * \param flag bitflags for where to scan.
 */
void
do_scan(dbref player, char *command, int flag)
{
  /* scan for possible matches of user-def'ed commands */
  char atrname[BUFFER_LEN];
  char *ptr;
  dbref thing;
  int num;

  ptr = atrname;
  if (!GoodObject(Location(player))) {
    notify(player, T("Sorry, you are in an invalid location."));
    return;
  }
  if (!command || !*command) {
    notify(player, T("What command do you want to scan for?"));
    return;
  }
  if (flag & CHECK_NEIGHBORS) {
    notify(player, T("Matches on contents of this room:"));
    DOLIST(thing, Contents(Location(player))) {
      if (ScanFind(player, thing)) {
	*ptr = '\0';
	notify_format(player,
		      "%s  [%d:%s]", unparse_object(player, thing), num,
		      atrname);
	ptr = atrname;
      }
    }
  }
  ptr = atrname;
  if (flag & CHECK_HERE) {
    if (ScanFind(player, Location(player))) {
      *ptr = '\0';
      notify_format(player, T("Matched here: %s  [%d:%s]"),
		    unparse_object(player, Location(player)), num, atrname);
    }
  }
  ptr = atrname;
  if (flag & CHECK_INVENTORY) {
    notify(player, T("Matches on carried objects:"));
    DOLIST(thing, Contents(player)) {
      if (ScanFind(player, thing)) {
	*ptr = '\0';
	notify_format(player, "%s  [%d:%s]",
		      unparse_object(player, thing), num, atrname);
	ptr = atrname;
      }
    }
  }
  ptr = atrname;
  if (flag & CHECK_SELF) {
    if (ScanFind(player, player)) {
      *ptr = '\0';
      notify_format(player, T("Matched self: %s  [%d:%s]"),
		    unparse_object(player, player), num, atrname);
    }
  }
  ptr = atrname;
  if (flag & CHECK_ZONE) {
    /* zone checks */
    if (Zone(Location(player)) != NOTHING) {
      if (IsRoom(Zone(Location(player)))) {
	/* zone of player's location is a zone master room */
	if (Location(player) != Zone(player)) {
	  notify(player, T("Matches on zone master room of location:"));
	  DOLIST(thing, Contents(Zone(Location(player)))) {
	    if (ScanFind(player, thing)) {
	      *ptr = '\0';
	      notify_format(player, "%s  [%d:%s]",
			    unparse_object(player, thing), num, atrname);
	      ptr = atrname;
	    }
	  }
	}
      } else {
	/* regular zone object */
	if (ScanFind(player, Zone(Location(player)))) {
	  *ptr = '\0';
	  notify_format(player,
			T("Matched zone of location: %s  [%d:%s]"),
			unparse_object(player, Zone(Location(player))),
			num, atrname);
	}
      }
    }
    ptr = atrname;
    if ((Zone(player) != NOTHING) && (Zone(player) != Zone(Location(player)))) {
      /* check the player's personal zone */
      if (IsRoom(Zone(player))) {
	if (Location(player) != Zone(player)) {
	  notify(player, T("Matches on personal zone master room:"));
	  DOLIST(thing, Contents(Zone(player))) {
	    if (ScanFind(player, thing)) {
	      *ptr = '\0';
	      notify_format(player, "%s  [%d:%s]",
			    unparse_object(player, thing), num, atrname);
	      ptr = atrname;
	    }
	  }
	}
      } else if (ScanFind(player, Zone(player))) {
	*ptr = '\0';
	notify_format(player, T("Matched personal zone: %s  [%d:%s]"),
		      unparse_object(player, Zone(player)), num, atrname);
      }
    }
  }
  ptr = atrname;
  if ((flag & CHECK_GLOBAL)
      && (Location(player) != MASTER_ROOM)
      && (Zone(Location(player)) != MASTER_ROOM)
      && (Zone(player) != MASTER_ROOM)) {
    /* try Master Room stuff */
    notify(player, T("Matches on objects in the Master Room:"));
    DOLIST(thing, Contents(MASTER_ROOM)) {
      if (ScanFind(player, thing)) {
	*ptr = '\0';
	notify_format(player, "%s  [%d:%s]",
		      unparse_object(player, thing), num, atrname);
	ptr = atrname;
      }
    }
  }
}

#define DOL_MAP 1
#define DOL_NOTIFY 2
#define DOL_DELIM 4

/** Execute a command for each element of a list.
 * \verbatim
 * This function implements @dolist.
 * \endverbatim
 * \param player the enactor.
 * \param list string containing the list to iterate over.
 * \param command command to run for each list element.
 * \param cause object which caused this command to be run.
 * \param flags command switch flags.
 */
void
do_dolist(dbref player, char *list, char *command, dbref cause,
	  unsigned int flags)
{
  char *curr, *objstring;
  char outbuf[BUFFER_LEN];
  char *bp;
  int place;
  char placestr[10];
  int j;
  char delim = ' ';
  if (!command || !*command) {
    notify(player, T("What do you want to do with the list?"));
    if (flags & DOL_NOTIFY)
      parse_que(player, "@notify me", cause);
    return;
  }

  if (flags & DOL_DELIM) {
    if (list[1] != ' ') {
      notify(player, T("Separator must be one character."));
      if (flags & DOL_NOTIFY)
	parse_que(player, "@notify me", cause);
      return;
    }
    delim = list[0];
  }

  /* set up environment for any spawned commands */
  for (j = 0; j < 10; j++)
    wnxt[j] = wenv[j];
  for (j = 0; j < NUMQ; j++)
    rnxt[j] = renv[j];
  bp = outbuf;
  if (flags & DOL_DELIM)
    list += 2;
  place = 0;
  objstring = trim_space_sep(list, delim);
  if (objstring && !*objstring) {
    /* Blank list */
    if (flags & DOL_NOTIFY)
      parse_que(player, "@notify me", cause);
    return;
  }

  while (objstring) {
    curr = split_token(&objstring, delim);
    place++;
    sprintf(placestr, "%d", place);
    if (!(flags & DOL_MAP)) {
      /* @dolist, queue command */
      bind_and_queue(player, cause, command, curr, placestr);
    } else {
      const char *replace[2];
      char *ebuf, *ebufptr;
      /* it's @map, add to the output list */
      if (bp != outbuf)
	safe_chr(delim, outbuf, &bp);
      replace[0] = curr;
      replace[1] = placestr;
      ebufptr = ebuf = replace_string2(standard_tokens, replace, command);
      process_expression(outbuf, &bp, (char const **) &ebuf,
			 player, cause, cause, PE_DEFAULT, PT_DEFAULT, NULL);
      mush_free(ebufptr, "replace_string.buff");
    }
  }

  *bp = '\0';
  if (flags & DOL_MAP) {
    /* if we're doing a @map, copy the list to an attribute */
    (void) atr_add(player, "MAPLIST", outbuf, GOD, NOTHING);
    notify(player, T("Function mapped onto list."));
  }
  if (flags & DOL_NOTIFY) {
    /*  Execute a '@notify me' so the object knows we're done
     *  with the list execution. We don't execute dequeue_semaphores()
     *  directly, since we want the command to be queued
     *  _after_ the list has executed.
     */
    parse_que(player, "@notify me", cause);
  }
}

static void linux_uptime(dbref player) __attribute__ ((__unused__));
static void unix_uptime(dbref player) __attribute__ ((__unused__));
static void win32_uptime(dbref player) __attribute__ ((__unused__));

static void
linux_uptime(dbref player __attribute__ ((__unused__)))
{
#ifdef linux
  /* Use /proc files instead of calling the external uptime program on linux */
  char tbuf1[BUFFER_LEN];
  FILE *fp;
  char line[128];		/* Overkill */
  char *nl;
  Pid_t pid;
  int psize;
#ifdef HAS_GETRUSAGE
  struct rusage usage;
#endif

  /* Current time */
  {
    struct tm *t;
    t = localtime(&mudtime);
    strftime(tbuf1, sizeof tbuf1, "%I:%M%p ", t);
    nl = tbuf1 + strlen(tbuf1);
  }
  /* System uptime */
  fp = fopen("/proc/uptime", "r");
  if (fp) {
    time_t uptime;
    const char *fmt;
    if (fgets(line, sizeof line, fp)) {
      /* First part of this line is uptime in seconds.milliseconds. We
         only care about seconds. */
      uptime = strtol(line, NULL, 10);
      if (uptime > 86400)
	fmt = "up $d days, $2h:$2M,";
      else
	fmt = "up $2h:$2M,";
      do_timestring(tbuf1, &nl, fmt, uptime);
    } else {
      safe_str("Unknown uptime,", tbuf1, &nl);
    }
    fclose(fp);
  } else {
    safe_str("Unknown uptime,", tbuf1, &nl);
  }

  /* Now load averages */
  fp = fopen("/proc/loadavg", "r");
  if (fp) {
    if (fgets(line, sizeof line, fp)) {
      double load[3];
      char *x, *l = line;
      load[0] = strtod(l, &x);
      l = x;
      load[1] = strtod(l, &x);
      l = x;
      load[2] = strtod(l, NULL);
      safe_format(tbuf1, &nl, " load average: %.2f, %.2f, %.2f",
		  load[0], load[1], load[2]);
    } else {
      safe_str("Unknown load", tbuf1, &nl);
    }
    fclose(fp);
  } else {
    safe_str("Unknown load", tbuf1, &nl);
  }

  *nl = '\0';
  notify(player, tbuf1);

  /* do process stats */
  pid = getpid();
  psize = getpagesize();
  notify_format(player,
		T("\nProcess ID:  %10u        %10d bytes per page"),
		pid, psize);

  /* Linux's getrusage() is mostly unimplemented. Just has times, page faults
     and swapouts. We use /proc/self/status */
#ifdef HAS_GETRUSAGE
  getrusage(RUSAGE_SELF, &usage);
  notify_format(player, T("Time used:   %10ld user   %10ld sys"),
		usage.ru_utime.tv_sec, usage.ru_stime.tv_sec);
  notify_format(player,
		T
		("Page faults: %10ld hard   %10ld soft    %10ld swapouts"),
		usage.ru_majflt, usage.ru_minflt, usage.ru_nswap);
#endif
  fp = fopen("/proc/self/status", "r");
  if (!fp)
    return;
  /* Skip lines we don't care about. */
  while (fgets(line, sizeof line, fp) != NULL) {
    static const char *fields[] = {
      "VmSize:", "VmRSS:", "VmData:", "VmStk:", "VmExe:", "VmLib:",
      "SigPnd:", "SigBlk:", "SigIgn:", "SigCgt:", NULL
    };
    int n;
    for (n = 0; fields[n]; n++) {
      size_t len = strlen(fields[n]);
      if (strncmp(line, fields[n], len) == 0) {
	if ((nl = strchr(line, '\n')) != NULL)
	  *nl = '\0';
	notify(player, line);
      }
    }
  }

  fclose(fp);

#endif
}

static void
unix_uptime(dbref player __attribute__ ((__unused__)))
{
#ifdef HAS_UPTIME
  FILE *fp;
  char c;
  int i;
#endif
#ifdef HAS_GETRUSAGE
  struct rusage usage;
#endif
#ifndef WIN32
  char tbuf1[BUFFER_LEN];
#endif
  Pid_t pid;
  int psize;

#ifdef HAS_UPTIME
  fp =
#ifdef __LCC__
    (FILE *)
#endif
    popen(UPTIME_PATH, "r");

  /* just in case the system is screwy */
  if (fp == NULL) {
    notify(player, T("Error -- cannot execute uptime."));
    do_rawlog(LT_ERR, T("** ERROR ** popen for @uptime returned NULL."));
    return;
  }
  /* print system uptime */
  for (i = 0; (c = getc(fp)) != '\n'; i++)
    tbuf1[i] = c;
  tbuf1[i] = '\0';
  pclose(fp);

  notify(player, tbuf1);
#endif				/* HAS_UPTIME */

  /* do process stats */

  pid = getpid();
  psize = getpagesize();
  notify_format(player, T("\nProcess ID:  %10u        %10d bytes per page"),
		pid, psize);


#ifdef HAS_GETRUSAGE
  getrusage(RUSAGE_SELF, &usage);
  notify_format(player, T("Time used:   %10ld user   %10ld sys"),
		usage.ru_utime.tv_sec, usage.ru_stime.tv_sec);
  notify_format(player, "Max res mem: %10ld pages  %10ld bytes",
		usage.ru_maxrss, (usage.ru_maxrss * psize));
  notify_format(player,
		"Integral mem:%10ld shared %10ld private %10ld stack",
		usage.ru_ixrss, usage.ru_idrss, usage.ru_isrss);
  notify_format(player,
		T
		("Page faults: %10ld hard   %10ld soft    %10ld swapouts"),
		usage.ru_majflt, usage.ru_minflt, usage.ru_nswap);
  notify_format(player, T("Disk I/O:    %10ld reads  %10ld writes"),
		usage.ru_inblock, usage.ru_oublock);
  notify_format(player, T("Network I/O: %10ld in     %10ld out"),
		usage.ru_msgrcv, usage.ru_msgsnd);
  notify_format(player, T("Context swi: %10ld vol    %10ld forced"),
		usage.ru_nvcsw, usage.ru_nivcsw);
  notify_format(player, "Signals:     %10ld", usage.ru_nsignals);
#endif				/* HAS_GETRUSAGE */

}

static void
win32_uptime(dbref player __attribute__ ((__unused__)))
{				/* written by NJG */
#ifdef WIN32
  MEMORYSTATUS memstat;
  double mem;
  memstat.dwLength = sizeof(memstat);
  GlobalMemoryStatus(&memstat);
  notify(player, "---------- Windows memory usage ------------");
  notify_format(player, "%10ld %% memory in use", memstat.dwMemoryLoad);
  mem = memstat.dwAvailPhys / 1024.0 / 1024.0;
  notify_format(player, "%10.3f Mb free physical memory", mem);
  mem = memstat.dwTotalPhys / 1024.0 / 1024.0;
  notify_format(player, "%10.3f Mb total physical memory", mem);
  mem = memstat.dwAvailPageFile / 1024.0 / 1024.0;
  notify_format(player, "%10.3f Mb available in the paging file ", mem);
  mem = memstat.dwTotalPageFile / 1024.0 / 1024.0;
  notify_format(player, "%10.3f Mb total paging file size", mem);
#endif
}


/** Report on server uptime.
 * \verbatim
 * This command implements @uptime.
 * \endverbatim
 * \param player the enactor.
 * \param mortal if 1, show mortal display, even if player is privileged.
 */
void
do_uptime(dbref player, int mortal)
{
  char tbuf1[BUFFER_LEN];
  struct tm *when;

  when = localtime(&first_start_time);
  strftime(tbuf1, sizeof tbuf1, T("     Up since %a %b %d %X %Z %Y"), when);
  notify(player, tbuf1);

  when = localtime(&start_time);
  strftime(tbuf1, sizeof tbuf1, T("  Last reboot: %a %b %d %X %Z %Y"), when);
  notify(player, tbuf1);

  notify_format(player, T("Total reboots: %d"), reboot_count);

  when = localtime(&mudtime);
  strftime(tbuf1, sizeof tbuf1, T("     Time now: %a %b %d %X %Z %Y"), when);
  notify(player, tbuf1);

  if (last_dump_time > 0) {
    when = localtime(&last_dump_time);
    strftime(tbuf1, sizeof tbuf1,
	     T("   Time of last database save: %a %b %d %X %Z %Y"), when);
    notify(player, tbuf1);
  }

  /* calculate times until various events */
  when = localtime(&options.dump_counter);
  strftime(tbuf1, sizeof tbuf1, "%X", when);
  notify_format(player,
		T
		("Time until next database save: %ld minutes %ld seconds, at %s"),
		(options.dump_counter - mudtime) / 60,
		(options.dump_counter - mudtime) % 60, tbuf1);

  when = localtime(&options.dbck_counter);
  strftime(tbuf1, sizeof tbuf1, "%X", when);
  notify_format(player,
		T
		("   Time until next dbck check: %ld minutes %ld seconds, at %s."),
		(options.dbck_counter - mudtime) / 60,
		(options.dbck_counter - mudtime) % 60, tbuf1);

  when = localtime(&options.purge_counter);
  strftime(tbuf1, sizeof tbuf1, "%X", when);
  notify_format(player,
		T
		("        Time until next purge: %ld minutes %ld seconds, at %s."),
		(options.purge_counter - mudtime) / 60,
		(options.purge_counter - mudtime) % 60, tbuf1);

  if (options.warn_interval) {
    when = localtime(&options.warn_counter);
    strftime(tbuf1, sizeof tbuf1, "%X", when);
    notify_format(player,
		  T
		  ("    Time until next @warnings: %ld minutes %ld seconds, at %s."),
		  (options.warn_counter - mudtime) / 60,
		  (options.warn_counter - mudtime) % 60, tbuf1);
  }

  /* Mortals, go no further! */
  if (!Wizard(player) || mortal)
    return;
#if defined(linux)
  linux_uptime(player);
#elif defined(WIN32)
  win32_uptime(player);
#else
  unix_uptime(player);
#endif
}


/* Open a db file, which may be compressed, and return a file pointer */
static FILE *
db_open(const char *filename)
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
      f =
#ifdef __LCC__
	(FILE *)
#endif
	popen(tprintf
	      ("%s < %s%s", options.uncompressprog, filename,
	       options.compresssuff), "r");
      /* Force the pipe to be fully buffered */
      if (f)
	setvbuf(f, NULL, _IOFBF, BUFSIZ);
    }
  } else
#endif				/* WIN32 */
#endif				/* macintosh */
  {
#ifdef macintosh
    f = fopen(filename, "rb");
#else
    f = fopen(filename, "r");
#endif
  }
  return f;
}

/* Open a file or pipe (if compressing) for writing */
static FILE *
db_open_write(const char *filename)
{
  FILE *f;
#ifndef macintosh
#ifndef WIN32
  if (options.compressprog && *options.compressprog) {
    f =
#ifdef __LCC__
      (FILE *)
#endif
      popen(tprintf
	    ("%s >%s%s", options.compressprog, filename,
	     options.compresssuff), "w");
    /* Force the pipe to be fully buffered */
    if (f)
      setvbuf(f, NULL, _IOFBF, BUFSIZ);
  } else
#endif				/* WIN32 */
#endif				/* macintosh */
  {
#ifdef macintosh
    f = fopen(filename, "wb");
#else
    f = fopen(filename, "w");
#endif
  }
  if (!f)
    longjmp(db_err, 1);
  return f;
}


/* Close a db file, which may really be a pipe */
static void
db_close(FILE * f)
{
#ifndef macintosh
#ifndef WIN32
  if (options.compressprog && *options.compressprog) {
    pclose(f);
  } else
#endif				/* WIN32 */
#endif				/* macintosh */
  {
    fclose(f);
  }
}

/* List various goodies.
 * \verbatim
 * This function implements @list.
 * \endverbatim
 * \param player the enactor.
 * \param arg what to list.
 * \param lc if 1, list in lowercase.
 */
void
do_list(dbref player, char *arg, int lc)
{
  if (!arg || !*arg)
    notify(player, T("I don't understand what you want to @list."));
  else if (string_prefix("commands", arg))
    do_list_commands(player, lc);
  else if (string_prefix("functions", arg))
    do_list_functions(player, lc);
  else if (string_prefix("motd", arg))
    do_motd(player, 3, "");
  else if (string_prefix("attribs", arg))
    do_list_attribs(player, lc);
  else
    notify(player, T("I don't understand what you want to @list."));
}

extern HASHTAB htab_function;
extern HASHTAB htab_user_function;
extern HASHTAB htab_math;
extern HASHTAB htab_tag;
extern HASHTAB htab_player_list;
extern HASHTAB htab_reserved_aliases;
extern HASHTAB help_files;
extern HASHTAB htab_objdata;
extern StrTree atr_names;
extern StrTree lock_names;
extern StrTree object_names;
extern PTAB ptab_command;
extern PTAB ptab_attrib;
extern PTAB ptab_flag;

/** Reports stats on various in-memory data structures.
 * \param player the enactor.
 */
void
do_list_memstats(dbref player)
{
  notify(player, "Hash Tables:");
  hash_stats_header(player);
  hash_stats(player, &htab_function, "Functions");
  hash_stats(player, &htab_user_function, "@Functions");
  hash_stats(player, &htab_math, "Math funs");
  hash_stats(player, &htab_tag, "HTML tags");
  hash_stats(player, &htab_player_list, "Players");
  hash_stats(player, &htab_reserved_aliases, "Aliases");
  hash_stats(player, &help_files, "HelpFiles");
  hash_stats(player, &htab_objdata, "ObjData");
  notify(player, "Prefix Trees:");
  ptab_stats_header(player);
  ptab_stats(player, &ptab_attrib, "AttrPerms");
  ptab_stats(player, &ptab_command, "Commands");
  ptab_stats(player, &ptab_flag, "Flags");
  notify(player, "String Trees:");
  st_stats_header(player);
  st_stats(player, &atr_names, "AttrNames");
  st_stats(player, &object_names, "ObjNames");
  st_stats(player, &lock_names, "LockNames");
#if (COMPRESSION_TYPE >= 3) && defined(COMP_STATS)
  if (Wizard(player)) {
    long items, used, total_comp, total_uncomp;
    double percent;
    compress_stats(&items, &used, &total_uncomp, &total_comp);
    notify(player, "---------- Internal attribute compression  ----------");
    notify_format(player,
		  "%10ld compression table items used, "
		  "taking %ld bytes.", items, used);
    notify_format(player, "%10ld bytes in text before compression. ",
		  total_uncomp);
    notify_format(player, "%10ld bytes in text AFTER  compression. ",
		  total_comp);
    percent = ((float) (total_comp)) / ((float) total_uncomp) * 100.0;
    notify_format(player,
		  "%10.0f %% text    compression ratio (lower is better). ",
		  percent);
    percent =
      ((float) (total_comp + used + (32768L * sizeof(char *)))) /
      ((float) total_uncomp) * 100.0;
    notify_format(player,
		  "%10.0f %% OVERALL compression ratio (lower is better). ",
		  percent);
    notify_format(player,
		  T
		  ("          (Includes table items, and table of words pointers of %ld bytes)"),
		  32768L * sizeof(char *));
    if (percent >= 100.0)
      notify(player,
	     "          " "(Compression ratio improves with larger database)");
  }
#endif

#ifdef MEM_CHECK
  if (God(player))
    list_mem_check(player);
#endif
}

static char *
make_new_epoch_file(const char *basename, int the_epoch)
{
  static char result[BUFFER_LEN];	/* STATIC! */
  /* Unlink the last the_epoch and create a new one */
  sprintf(result, "%s.#%d#", basename, the_epoch - 1);
  unlink(result);
  sprintf(result, "%s.#%d#", basename, the_epoch);
  return result;
}
