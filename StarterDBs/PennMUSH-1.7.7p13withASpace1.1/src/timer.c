/**
 * \file timer.c
 *
 * \brief Timed events in PennMUSH.
 *
 *
 */
#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif
#ifdef I_UNISTD
#include <unistd.h>
#endif

#include "conf.h"
#include "match.h"
#include "flags.h"
#include "externs.h"
#include "access.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "log.h"
#include "game.h"
#include "help.h"
#include "parse.h"
#include "confmagic.h"


int on_second = 0;
static sig_atomic_t hup_triggered = 0;
static sig_atomic_t usr1_triggered = 0;
extern char ccom[BUFFER_LEN];
extern dbref cplr;

extern void inactivity_check(void);
extern void reopen_logs(void);

#ifndef WIN32
void hup_handler(int);
void usr1_handler(int);

#endif
void dispatch(void);

#ifdef HAS_ITIMER
void signal_cpu_limit(int signo);
#endif

#ifndef WIN32

/** Handler for HUP signal.
 * Do the minimal work here - set a global variable and reload the handler.
 * \param x unused.
 */
void
hup_handler(int x __attribute__ ((__unused__)))
{
  hup_triggered = 1;
  reload_sig_handler(SIGHUP, hup_handler);
}

/** Handler for USR1 signal.
 * Do the minimal work here - set a global variable and reload the handler.
 * \param x unused.
 */
void
usr1_handler(int x __attribute__ ((__unused__)))
{
  usr1_triggered = 1;
  reload_sig_handler(SIGUSR1, usr1_handler);
}

#endif				/* WIN32 */

/** Set up signal handlers.
 */
void
init_timer(void)
{
#ifndef WIN32
  install_sig_handler(SIGHUP, hup_handler);
  install_sig_handler(SIGUSR1, usr1_handler);
#endif
#ifdef HAS_ITIMER
  install_sig_handler(SIGPROF, signal_cpu_limit);
#endif
}


/** Handle events that may need handling.
 * This routine is polled from bsd.c. At any call, it can handle
 * the HUP and USR1 signals. At calls that are 'on the second',
 * it goes on to perform regular every-second processing and to
 * check whether it's time to do other periodic processes like
 * purge, dump, or inactivity checks.
 */
void
dispatch(void)
{
  static int idle_counter = 0;

  /* A HUP reloads configuration and reopens logs */
  if (hup_triggered) {
    do_rawlog(LT_ERR, T("SIGHUP received: reloading .txt and .cnf files"));
    config_file_startup(NULL, 0);
    config_file_startup(NULL, 1);
    fcache_load(NOTHING);
    help_reindex(NOTHING);
    read_access_file();
    reopen_logs();
    hup_triggered = 0;
  }
  /* A USR1 does a shutdown/reboot */
  if (usr1_triggered) {
    do_reboot(NOTHING, 0);	/* We don't return from this */
    usr1_triggered = 0;		/* But just in case */
  }
  if (!on_second)
    return;
  on_second = 0;

  mudtime = time(NULL);

  do_second();

  if (options.purge_counter <= mudtime) {
    /* Free list reconstruction */
    options.purge_counter = options.purge_interval + mudtime;
    cplr = NOTHING;
    strcpy(ccom, "purge");
    purge();
  }

  if (options.dbck_counter <= mudtime) {
    /* Database consistency check */
    options.dbck_counter = options.dbck_interval + mudtime;
    cplr = NOTHING;
    strcpy(ccom, "dbck");
    dbck();
  }

  if (idle_counter <= mudtime) {
    /* Inactivity check */
    idle_counter = 60 + mudtime;
    inactivity_check();
  }

  /* Database dump routines */
  if (options.dump_counter <= mudtime) {
#ifdef MEM_CHECK
    log_mem_check();
#endif
    options.dump_counter = options.dump_interval + mudtime;
    strcpy(ccom, "dump");
    fork_and_dump(1);
#ifdef VACATION_FLAG
    flag_broadcast(0, "ON-VACATION", "%s",
		   T
		   ("Your ON-VACATION flag is set! If you're back, clear it."));
#endif
  } else if (NO_FORK &&
	     (options.dump_counter - 60 == mudtime) &&
	     *options.dump_warning_1min) {
    flag_broadcast(0, 0, "%s", options.dump_warning_1min);
  } else if (NO_FORK &&
	     (options.dump_counter - 300 == mudtime) &&
	     *options.dump_warning_5min) {
    flag_broadcast(0, 0, "%s", options.dump_warning_5min);
  }
  if (options.warn_interval && (options.warn_counter <= mudtime)) {
    options.warn_counter = options.warn_interval + mudtime;
    strcpy(ccom, "warnings");
    run_topology();
  }

  local_timer();
}

sig_atomic_t cpu_time_limit_hit = 0;
int cpu_limit_warning_sent = 0;

#if defined(HAS_ITIMER)
/** Handler for PROF signal.
 * Do the minimal work here - set a global variable and reload the handler.
 * \param signo unused.
 */
void
signal_cpu_limit(int signo __attribute__ ((__unused__)))
{
  cpu_time_limit_hit = 1;
  reload_sig_handler(SIGPROF, signal_cpu_limit);
}
#elif defined(WIN32)
UINT_PTR timer_id;
VOID CALLBACK
win32_timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
  cpu_time_limit_hit = 1;
}
#endif
int timer_set = 0;

/** Start the cpu timer (before running a command).
 */
void
start_cpu_timer(void)
{
  cpu_time_limit_hit = 0;
  cpu_limit_warning_sent = 0;
  timer_set = 1;
#if defined(HAS_ITIMER)		/* UNIX way */
  {
    struct itimerval time_limit;
    if (options.queue_entry_cpu_time > 0) {
      ldiv_t t;
      /* Convert from milliseconds */
      t = ldiv(options.queue_entry_cpu_time, 1000);
      time_limit.it_value.tv_sec = t.quot;
      time_limit.it_value.tv_usec = t.rem * 1000;
      time_limit.it_interval.tv_sec = 0;
      time_limit.it_interval.tv_usec = 0;
      if (setitimer(ITIMER_PROF, &time_limit, NULL)) {
	perror("setitimer");
	timer_set = 0;
      }
    } else
      timer_set = 0;
  }
#elif defined(WIN32)		/* Windoze way */
  if (options.queue_entry_cpu_time > 0)
    timer_id = SetTimer(NULL, 0, (unsigned) options.queue_entry_cpu_time,
			(TIMERPROC) win32_timer);
  else
    timer_set = 0;
#endif
}

/** Reset the cpu timer (after running a command).
 */
void
reset_cpu_timer(void)
{
  if (timer_set) {
#if defined(HAS_ITIMER)
    struct itimerval time_limit, time_left;
    time_limit.it_value.tv_sec = 0;
    time_limit.it_value.tv_usec = 0;
    time_limit.it_interval.tv_sec = 0;
    time_limit.it_interval.tv_usec = 0;
    if (setitimer(ITIMER_PROF, &time_limit, &time_left))
      perror("setitimer");
#elif defined(WIN32)
    KillTimer(NULL, timer_id);
#endif
  }
  cpu_time_limit_hit = 0;
  cpu_limit_warning_sent = 0;
  timer_set = 0;
}
