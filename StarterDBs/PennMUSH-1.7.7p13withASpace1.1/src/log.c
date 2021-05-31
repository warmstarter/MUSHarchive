/**
 * \file log.c
 *
 * \brief Logging for PennMUSH.
 *
 *
 */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#ifdef I_UNISTD
#include <unistd.h>
#endif
#include <string.h>
#include <stdarg.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#endif
#include <time.h>
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif

#include "conf.h"
#include "externs.h"
#include "flags.h"
#include "dbdefs.h"
#include "log.h"
#include "confmagic.h"
#include "options.h"

static char *quick_unparse(dbref object);
static void start_log(FILE ** fp, const char *filename);
static void end_log(FILE * fp);

#ifdef macintosh
#include "PMInit.h"
#endif

/* log file pointers */
FILE *connlog_fp;
FILE *checklog_fp;
FILE *wizlog_fp;
FILE *tracelog_fp;
FILE *cmdlog_fp;
#ifdef ASPACE
FILE *spacelog_fp;
#endif

static char *
quick_unparse(dbref object)
{
  static char buff[BUFFER_LEN], *bp;

  switch (object) {
  case NOTHING:
    strcpy(buff, T("*NOTHING*"));
    break;
  case AMBIGUOUS:
    strcpy(buff, T("*VARIABLE*"));
    break;
  case HOME:
    strcpy(buff, T("*HOME*"));
    break;
  default:
    bp = buff;
    safe_format(buff, &bp, "%s(#%d%s)",
		Name(object), object, unparse_flags(object, GOD));
    *bp = '\0';
  }

  return buff;
}

static void
start_log(FILE ** fp, const char *filename)
{
  char newfilename[256] = "\0";

  /* Must use a buffer for MacOS file path conversion */
  strncpy(newfilename, filename, 256);

#ifdef macintosh
  /* Convert file path from a UNIX style one to a MacOS one */
  PMConvertPath((char *) newfilename, (char *) newfilename, 256);
  *fp = fopen(newfilename, "ab");
#else
  *fp = fopen(newfilename, "a");
#endif
  if (*fp == NULL) {
    fprintf(stderr, T("WARNING: cannot open log %s\n"), newfilename);
    *fp = stderr;
  }
  fprintf(*fp, "START OF LOG.\n");
  fflush(*fp);
}

/** Open all logfiles.
 */
void
start_all_logs(void)
{
#ifndef SINGLE_LOGFILE
  start_log(&connlog_fp, CONNLOG);
  start_log(&checklog_fp, CHECKLOG);
  start_log(&wizlog_fp, WIZLOG);
  start_log(&tracelog_fp, TRACELOG);
  start_log(&cmdlog_fp, CMDLOG);
#ifdef ASPACE
  start_log(&spacelog_fp, SPACELOG);
#endif
#else
#ifdef ASPACE
  spacelog_fp =
#endif
  connlog_fp = checklog_fp = wizlog_fp = tracelog_fp = cmdlog_fp = stderr;
#endif				/* SINGLE_LOGFILE */
}

static void
end_log(FILE * fp)
{
  if (fp != stderr) {
    fprintf(fp, "END OF LOG.\n");
    fflush(fp);
    fclose(fp);
  }
}

/** Close all logfiles.
 */
void
end_all_logs(void)
{
#ifndef SINGLE_LOGFILE
  /* close up the log files */
  end_log(connlog_fp);
  end_log(checklog_fp);
  end_log(wizlog_fp);
  end_log(tracelog_fp);
  end_log(cmdlog_fp);
#ifdef ASPACE
  end_log(spacelog_fp);
#endif
#endif				/* SINGLE_LOGFILE */
}


/** Log a raw message.
 * take a log type and format list and args, write to appropriate logfile.
 * log types are defined in log.h
 * \param logtype type of log to print message to.
 * \param fmt format string for message.
 */
void WIN32_CDECL
do_rawlog(int logtype, const char *fmt, ...)
{
  struct tm *ttm;
  char timebuf[18];
  char tbuf1[BUFFER_LEN + 50];
  va_list args;
  FILE *f = NULL;
  va_start(args, fmt);

#ifdef HAS_VSNPRINTF
  (void) vsnprintf(tbuf1, sizeof tbuf1, fmt, args);
#else
  (void) vsprintf(tbuf1, fmt, args);
#endif
  tbuf1[BUFFER_LEN - 1] = '\0';
  va_end(args);

  ttm = localtime(&mudtime);

  strftime(timebuf, sizeof timebuf, "[%m/%d %H:%M:%S]", ttm);

  switch (logtype) {
  case LT_ERR:
    f = stderr;
    break;
  case LT_CMD:
    f = cmdlog_fp;
    break;
  case LT_WIZ:
    f = wizlog_fp;
    break;
  case LT_CONN:
    f = connlog_fp;
    break;
  case LT_TRACE:
    f = tracelog_fp;
    break;
  case LT_CHECK:
    f = checklog_fp;
    break;
  case LT_HUH:
    f = cmdlog_fp;
    break;
#ifdef ASPACE
  case LT_SPACE:
    f = spacelog_fp;
    break;
#endif
  default:
    f = stderr;
    break;
  }
  fprintf(f, "%s %s\n", timebuf, tbuf1);
  fflush(f);
}

/** Log a message, with useful information.
 * take a log type and format list and args, write to appropriate logfile.
 * log types are defined in log.h. Unlike do_rawlog, this version
 * tags messages with prefixes, and uses dbref information passed to it.
 * \param logtype type of log to print message to.
 * \param player dbref that generated the log message.
 * \param object second dbref involved in log message (e.g. force logs)
 * \param fmt mesage format string.
 */
void WIN32_CDECL
do_log(int logtype, dbref player, dbref object, const char *fmt, ...)
{
  /* tbuf1 had 50 extra chars because we might pass this function
   * both a label string and a command which could be up to BUFFER_LEN
   * in length - for example, when logging @forces
   */
  char tbuf1[BUFFER_LEN + 50];
  va_list args;
  char unp1[BUFFER_LEN], unp2[BUFFER_LEN];

  va_start(args, fmt);

#ifdef HAS_VSNPRINTF
  (void) vsnprintf(tbuf1, sizeof tbuf1, fmt, args);
#else
  (void) vsprintf(tbuf1, fmt, args);
#endif
  va_end(args);

  switch (logtype) {
  case LT_ERR:
    do_rawlog(logtype, "RPT: %s", tbuf1);
    break;
  case LT_CMD:
    if (options.log_commands) {
      strcpy(unp1, quick_unparse(player));
      strcpy(unp2, quick_unparse(Location(player)));
      if (Suspect(player))
	do_rawlog(logtype, T("CMD: SUSPECT %s in %s: %s"), unp1, unp2, tbuf1);
      else
	do_rawlog(logtype, T("CMD: %s in %s: %s"), unp1, unp2, tbuf1);
    } else {
      if (Suspect(player)) {
	strcpy(unp1, quick_unparse(player));
	strcpy(unp2, quick_unparse(Location(player)));
	do_rawlog(logtype, T("CMD: SUSPECT %s in %s: %s"), unp1, unp2, tbuf1);
      }
    }
    break;
  case LT_WIZ:
    strcpy(unp1, quick_unparse(player));
    if (GoodObject(object)) {
      strcpy(unp2, quick_unparse(object));
      do_rawlog(logtype, "WIZ: %s --> %s: %s", unp1, unp2, tbuf1);
    } else {
      do_rawlog(logtype, "WIZ: %s: %s", unp1, tbuf1);
    }
    break;
  case LT_CONN:
    do_rawlog(logtype, "NET: %s", tbuf1);
    break;
  case LT_TRACE:
    do_rawlog(logtype, "TRC: %s", tbuf1);
    break;
  case LT_CHECK:
    do_rawlog(logtype, "%s", tbuf1);
    break;
  case LT_HUH:
    if (!controls(player, Location(player))) {
      strcpy(unp1, quick_unparse(player));
      strcpy(unp2, quick_unparse(Location(player)));
      do_rawlog(logtype, T("HUH: %s in %s [%s]: %s"),
		unp1, unp2,
		(GoodObject(Location(player))) ?
		Name(Owner(Location(player))) : T("bad object"), tbuf1);
    }
    break;
#ifdef ASPACE
  case LT_SPACE:
    strcpy(unp1, quick_unparse(player));
    strcpy(unp2, quick_unparse(object));
    do_rawlog(logtype, "SPACE: %s %s %s", unp1, unp2, tbuf1);
    break;
#endif /* ASPACE */
  default:
    do_rawlog(LT_ERR, "ERR: %s", tbuf1);
  }
}

/** Wipe out a game log. This is intended for those emergencies where
 * the log has grown out of bounds, overflowing the disk quota, etc.
 * Because someone with the god password can use this command to wipe
 * out 'intrusion' traces, we also require the log_wipe_passwd given
 * in mush.cnf
 * \param player the enactor.
 * \param logtype type of log to wipe.
 * \param str password for wiping logs.
 */
void
do_logwipe(dbref player, int logtype, char *str)
{
  if (strcmp(str, LOG_WIPE_PASSWD)) {
    const char *lname;
    switch (logtype) {
    case LT_CONN:
      lname = "connection";
      break;
    case LT_CHECK:
      lname = "checkpoint";
      break;
    case LT_CMD:
      lname = "command";
      break;
    case LT_TRACE:
      lname = "trace";
      break;
    case LT_WIZ:
      lname = "wizard";
      break;
#ifdef ASPACE
    case LT_SPACE:
      lname = "space";
      break;
#endif
    default:
      lname = "unspecified";
    }
    notify(player, T("Wrong password."));
    do_log(LT_WIZ, player, NOTHING,
	   T("Invalid attempt to wipe the %s log, password %s"), lname, str);
    return;
  }
  switch (logtype) {
  case LT_CONN:
    end_log(connlog_fp);
    unlink(CONNLOG);
    start_log(&connlog_fp, CONNLOG);
    do_log(LT_ERR, player, NOTHING, T("Connect log wiped."));
    break;
  case LT_CHECK:
    end_log(checklog_fp);
    unlink(CHECKLOG);
    start_log(&checklog_fp, CHECKLOG);
    do_log(LT_ERR, player, NOTHING, T("Checkpoint log wiped."));
    break;
  case LT_CMD:
    end_log(cmdlog_fp);
    unlink(CMDLOG);
    start_log(&cmdlog_fp, CMDLOG);
    do_log(LT_ERR, player, NOTHING, T("Command log wiped."));
    break;
  case LT_TRACE:
    end_log(tracelog_fp);
    unlink(TRACELOG);
    start_log(&tracelog_fp, TRACELOG);
    do_log(LT_ERR, player, NOTHING, T("Trace log wiped."));
    break;
  case LT_WIZ:
    end_log(wizlog_fp);
    unlink(WIZLOG);
    start_log(&wizlog_fp, WIZLOG);
    do_log(LT_ERR, player, NOTHING, T("Wizard log wiped."));
    break;
#ifdef ASPACE
  case LT_SPACE:
    end_log(spacelog_fp);
    unlink(SPACELOG);
    start_log(&spacelog_fp, SPACELOG);
    do_log(LT_ERR, player, NOTHING, T("Space log wiped."));
    break;
#endif
  default:
    notify(player, T("That is not a valid log."));
    return;
  }
  notify(player, T("Log wiped."));
}
