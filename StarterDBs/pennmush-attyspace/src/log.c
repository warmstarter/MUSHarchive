/* log.c */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#ifdef I_UNISTD
#include <unistd.h>
#endif
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif

#include "conf.h"
#include "externs.h"
#include "intrface.h"
#include "confmagic.h"

char *quick_unparse _((dbref object));
void start_log _((FILE ** fp, const char *filename));
void end_log _((FILE * fp));
void do_rawlog _((int logtype, const char *fmt,...));


char *
quick_unparse(object)
    dbref object;
{
  static char buff[BUFFER_LEN];

  switch (object) {
  case NOTHING:
    sprintf(buff, "*NOTHING*");
    break;
  case AMBIGUOUS:
    sprintf(buff, "*VARIABLE*");
    break;
  case HOME:
    sprintf(buff, "*HOME*");
    break;
  default:
    sprintf(buff, "%s(#%d%s)",
	    Name(object), object, unparse_flags(object, GOD));
  }

  return buff;
}

void
start_log(fp, filename)
    FILE **fp;
    const char *filename;
{
  *fp = fopen(filename, "a");
  if (*fp == NULL) {
    fprintf(stderr, "WARNING: cannot open log %s\n", filename);
    *fp = stderr;
  }
  fprintf(*fp, "START OF LOG.\n");
  fflush(*fp);
}

void
end_log(fp)
    FILE *fp;
{
  if (fp != stderr) {
    fprintf(fp, "END OF LOG.\n");
    fflush(fp);
    fclose(fp);
  }
}

#ifdef I_STDARG
void
do_rawlog(int logtype, const char *fmt,...)
#else
void
do_rawlog(logtype, va_alist)
    int logtype;
    va_dcl
#endif
{
  /* take a log type and format list and args, write to appropriate logfile.
   * log types are defined in mushdb.h
   */

  time_t tt;
  struct tm *ttm;
  char timebuf[16];
  char tbuf1[BUFFER_LEN + 50];
  va_list args;
  FILE *f = NULL;
#ifndef I_STDARG
  char *fmt;

  va_start(args);
  fmt = va_arg(args, char *);
#else
  va_start(args, fmt);
#endif
  (void) vsprintf(tbuf1, fmt, args);
  va_end(args);

  time(&tt);
  ttm = localtime(&tt);
  sprintf(timebuf, "%d%d/%d%d %d%d:%d%d:%d%d",
	  (((ttm->tm_mon) + 1) / 10), (((ttm->tm_mon) + 1) % 10),
	  (ttm->tm_mday / 10), (ttm->tm_mday % 10),
	  (ttm->tm_hour / 10), (ttm->tm_hour % 10),
	  (ttm->tm_min / 10), (ttm->tm_min % 10),
	  (ttm->tm_sec / 10), (ttm->tm_sec % 10));

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
  case LT_RPAGE:
    break;
  case LT_CHECK:
    f = checklog_fp;
    break;
  case LT_HUH:
    f = cmdlog_fp;
    break;
#ifdef TREKMUSH
  case LT_SPACE:
    f = spacelog_fp;
    break;
#endif /* TREKMUSH */
  default:
    f = stderr;
    break;
  }
  fprintf(f, "%s  %s\n", timebuf, tbuf1);
  fflush(f);
}

#ifdef I_STDARG
void
do_log(int logtype, dbref player, dbref object, const char *fmt,...)
#else
void
do_log(logtype, player, object, va_alist)
    int logtype;
    dbref player;
    dbref object;
    va_dcl
#endif
{
  /* take a log type and format list and args, write to appropriate logfile.
   * log types are defined in mushdb.h
   */

  /* tbuf1 had 50 extra chars because we might pass this function
   * both a label string and a command which could be up to BUFFER_LEN
   * in length - for example, when logging @forces
   */
  char tbuf1[BUFFER_LEN + 50];
  va_list args;
  char unp1[BUFFER_LEN], unp2[BUFFER_LEN];
#ifndef I_STDARG
  char *fmt;

  va_start(args);
  fmt = va_arg(args, char *);
#else
  va_start(args, fmt);
#endif
  (void) vsprintf(tbuf1, fmt, args);
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
	do_rawlog(logtype, "CMD: SUSPECT %s in %s: %s", unp1, unp2, tbuf1);
      else
	do_rawlog(logtype, "CMD: %s in %s: %s", unp1, unp2, tbuf1);
    } else {
      if (Suspect(player)) {
	strcpy(unp1, quick_unparse(player));
	strcpy(unp2, quick_unparse(Location(player)));
	do_rawlog(logtype, "CMD: SUSPECT %s in %s: %s", unp1, unp2, tbuf1);
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
  case LT_RPAGE:
    break;
  case LT_CHECK:
    do_rawlog(logtype, "%s", tbuf1);
    break;
  case LT_HUH:
    if (!controls(player, Location(player))) {
      strcpy(unp1, quick_unparse(player));
      strcpy(unp2, quick_unparse(Location(player)));
      do_rawlog(logtype, "HUH: %s in %s [%s]: %s",
		unp1, unp2,
	 (GoodObject(Location(player))) ? Name(Owner(Location(player))) :
		"bad object", tbuf1);
    }
    break;
#ifdef TREKMUSH
  case LT_SPACE:
    strcpy(unp1, quick_unparse(player));
    strcpy(unp2, quick_unparse(object));
    do_rawlog(logtype, "SPACE: %s %s %s", unp1, unp2, tbuf1);
    break;
#endif /* TREKMUSH */
  default:
    do_rawlog(LT_ERR, "ERR: %s", tbuf1);
  }
}

void
do_logwipe(player, logtype, str)
    dbref player;
    int logtype;
    char *str;
{
  /* Wipe out a game log. This is intended for those emergencies where
   * the log has grown out of bounds, overflowing the disk quota, etc.
   * Because someone with the god password can use this command to wipe
   * out 'intrusion' traces, we also require the log_wipe_passwd given
   * in mush.cnf
   */

  if (!God(player)) {
    notify(player, "Permission denied.");
    return;
  }
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
    default:
      lname = "unspecified";
    }
    do_log(LT_WIZ, player, NOTHING,
	   "Invalid attempt to wipe the %s log, password %s", lname, str);
    return;
  }
  switch (logtype) {
  case LT_CONN:
    end_log(connlog_fp);
    unlink(CONNLOG);
    start_log(&connlog_fp, CONNLOG);
    do_log(LT_ERR, player, NOTHING, "Connect log wiped.");
    break;
  case LT_CHECK:
    end_log(checklog_fp);
    unlink(CHECKLOG);
    start_log(&checklog_fp, CHECKLOG);
    do_log(LT_ERR, player, NOTHING, "Checkpoint log wiped.");
    break;
  case LT_CMD:
    end_log(cmdlog_fp);
    unlink(CMDLOG);
    start_log(&cmdlog_fp, CMDLOG);
    do_log(LT_ERR, player, NOTHING, "Command log wiped.");
    break;
  case LT_TRACE:
    end_log(tracelog_fp);
    unlink(TRACELOG);
    start_log(&tracelog_fp, TRACELOG);
    do_log(LT_ERR, player, NOTHING, "Trace log wiped.");
    break;
  case LT_WIZ:
    end_log(wizlog_fp);
    unlink(WIZLOG);
    start_log(&wizlog_fp, WIZLOG);
    do_log(LT_ERR, player, NOTHING, "Wizard log wiped.");
    break;
#ifdef TREKMUSH /* TrekMUSH Patch */
  case LT_SPACE:
    end_log(spacelog_fp);
    unlink(SPACELOG);
    start_log(&spacelog_fp, SPACELOG);
    do_log(LT_ERR, player, NOTHING, "Space log wiped.");
    break;
#endif /* TrekMUSH Patch */
  default:
    notify(player, "That is not a valid log.");
    return;
  }
  notify(player, "Log wiped.");
}
