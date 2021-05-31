/* timer.c */

/* Subroutines for timed events */
#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef WIN32
#include <windows.h>
#undef OPAQUE			/* conflict */
#endif
#include <signal.h>
#ifdef I_UNISTD
#include <unistd.h>
#endif

#include "conf.h"
#include "mushdb.h"
#include "intrface.h"
#include "match.h"
#include "externs.h"
#include "access.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "confmagic.h"


static int alarm_triggered = 0;
static int hup_triggered = 0;
static int usr1_triggered = 0;
extern time_t mudtime;
extern char ccom[BUFFER_LEN];
extern dbref cplr;

extern void inactivity_check _((void));
extern void do_dbck _((dbref player));
extern void fcache_load _((dbref player));
extern void fork_and_dump _((int forking));
extern int config_file_startup _((char *conf));
extern void reopen_logs _((void));
extern void do_reboot _((dbref player, int flag));
#ifndef WIN32
Signal_t alarm_handler _((void));
Signal_t hup_handler _((void));
Signal_t usr1_handler _((void));
/* OFFLINE #ifdef TREKMUSH
Signal_t usr2_handler _((void));
#endif */
#endif
void init_timer _((void));
void dispatch _((void));
extern void local_timer _((void));


#ifdef WIN32			/* NJG */
UINT timer_code;		/* needed to kill the timer */
/* Note: need to include: WINMM.LIB to link to timer functions */
void CALLBACK
alarm_handler(
	       UINT IDEvent,	/* identifies timer event */
	       UINT uReserved,	/* not used */
	       DWORD dwUser,	/* application-defined instance
				   data */
	       DWORD dwReserved1,	/* not used */
	       DWORD dwReserved2)
{				/* not used */
  alarm_triggered = 1;
}

void
kill_timer()
{
  timeKillEvent(timer_code);
}

#else				/* WIN32 */

Signal_t
alarm_handler()
{
  alarm_triggered = 1;
#ifndef SIGNALS_KEPT
  signal(SIGALRM, (void *) alarm_handler);
#endif
#ifndef VOIDSIG
  return 0;
#endif
}

Signal_t
hup_handler()
{
  hup_triggered = 1;
#ifndef SIGNALS_KEPT
  signal(SIGHUP, (void *) hup_handler);
#endif
#ifndef VOIDSIG
  return 0;
#endif
}

Signal_t
usr1_handler()
{
  usr1_triggered = 1;
#ifndef SIGNALS_KEPT
  signal(SIGHUP, (void *) usr1_handler);
#endif
#ifndef VOIDSIG
  return 0;
#endif
}

Signal_t
usr2_handler()
{
#ifndef SIGNALS_KEPT
  signal(SIGHUP, (void *) usr1_handler);
#endif
  _mcleanup();
#ifndef VOIDSIG
  return 0;
#endif
}

#endif				/* WIN32 */

void
init_timer()
{
#ifdef WIN32
  timer_code = timeSetEvent(1000, 1000, alarm_handler, 0, TIME_PERIODIC);
#else
  signal(SIGALRM, (void *) alarm_handler);
  signal(SIGHUP, (void *) hup_handler);
  signal(SIGUSR1, (void *) usr1_handler);
/* OFFLINE #ifdef TREKMUSH
  signal(SIGUSR2, (void *) usr2_handler);
#endif */
  alarm(1);
#endif
}


void
dispatch()
{
  /* this routine can be used to poll from intrface.c */
  if (hup_triggered) {
    do_rawlog(LT_ERR, "SIGHUP received: reloading .txt and .cnf files");
    fcache_load(NOTHING);
    config_file_startup(NULL);
    read_access_file();
    reopen_logs();
/* OFFLINE #ifdef TREKMUSH
    offline_shutdown();
    offline_init();
#endif */
    hup_triggered = 0;
  }
  if (usr1_triggered) {
    do_reboot(NOTHING, 0);	/* We don't return from this */
    usr1_triggered = 0;		/* But just in case */
  }
  if (!alarm_triggered)
    return;
  alarm_triggered = 0;

  mudtime = time(NULL);

  do_second();

  {
    static int purge_ticks = 0;
    if (--purge_ticks <= 0) {
      purge_ticks = PURGE_INTERVAL;
      /* Free list reconstruction */
      cplr = NOTHING;
      strcpy(ccom, "purge");
      purge();
    }
  }

  {
    static int dbck_ticks = 0;
    if (--dbck_ticks <= 0) {
      dbck_ticks = DBCK_INTERVAL;
      cplr = NOTHING;
      strcpy(ccom, "dbck");
      dbck();
      inactivity_check();
    }
  }

  /* Database dump routines */
  if (options.dump_counter <= mudtime) {
#ifdef MEM_CHECK
    log_mem_check();
#endif
    options.dump_counter = options.dump_interval + mudtime;
    strcpy(ccom, "dump");
    fork_and_dump(1);
#ifdef TREKMUSH
  } else if (NO_FORK &&
	     (options.dump_counter - 10 == mudtime) &&
	     *options.dump_warning_10sec) {
    flag_broadcast(0, 0, "GAME: Database will be dumped in 10 seconds.");
#endif /* TREKMUSH */
  } else if (NO_FORK &&
	     (options.dump_counter - 60 == mudtime) &&
	     *options.dump_warning_1min) {
    flag_broadcast(0, 0, options.dump_warning_1min);
  } else if (NO_FORK &&
	     (options.dump_counter - 300 == mudtime) &&
	     *options.dump_warning_5min) {
    flag_broadcast(0, 0, options.dump_warning_5min);
  }
  if (USE_RWHO && (options.rwho_counter <= mudtime)) {
    options.rwho_counter = options.rwho_interval + mudtime;
    strcpy(ccom, "update_rwho");
    rwho_update();
  }
#ifdef USE_WARNINGS
  if (options.warn_interval && (options.warn_counter <= mudtime)) {
    options.warn_counter = options.warn_interval + mudtime;
    strcpy(ccom, "warnings");
    run_topology();
  }
#endif

#ifndef WIN32
  /* reset alarm */
  alarm(1);
#endif

  local_timer();
}
