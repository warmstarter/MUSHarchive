#include "copyrite.h"

#include "config.h"
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#include "conf.h"
#include "externs.h"
#include "parse.h"
#include "confmagic.h"

extern time_t mudtime;
int do_convtime _((char *str, struct tm * ttm));

/* ARGSUSED */
FUNCTION(fun_time)
{
  char *s;

  s = (char *) ctime(&mudtime);
  s[strlen(s) - 1] = '\0';
  if (s[8] == ' ')
    s[8] = '0';
  safe_str(s, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_secs)
{
  safe_str(unparse_integer(mudtime), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_convsecs)
{
  /* converts seconds to a time string */

  time_t tt;
  char *s;

  if (!is_integer(args[0])) {
    safe_str(e_int, buff, bp);
    return;
  }
  tt = parse_integer(args[0]);
  if (tt < 0) {
    safe_str("#-1 ARGUMENT MUST BE POSITIVE", buff, bp);
    return;
  }
  s = (char *) ctime(&tt);
  s[strlen(s) - 1] = '\0';
  if (s[8] == ' ')
    s[8] = '0';
  safe_str(s, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_timestring)
{
  /* Convert seconds to #d #h #m #s
   * If pad > 0, pad with 0's (i.e. 0d 0h 5m 1s)
   */
  int secs, pad;
  int days, hours, mins;

  if (!is_integer(args[0])) {
    safe_str(e_ints, buff, bp);
    return;
  }
  if (nargs == 1)
    pad = 0;
  else {
    if (!is_integer(args[1])) {
      safe_str(e_ints, buff, bp);
      return;
    }
    pad = parse_integer(args[1]);
  }

  secs = parse_integer(args[0]);
  if (secs < 0) {
    safe_str("#-1 OUT OF RANGE", buff, bp);
    return;
  }
  days = secs / 86400;
  secs %= 86400;
  hours = secs / 3600;
  secs %= 3600;
  mins = secs / 60;
  secs %= 60;
  if (pad || (days > 0))
    safe_str(tprintf("%dd %2dh %2dm %2ds", days, hours, mins, secs), buff, bp);
  else if (hours > 0)
    safe_str(tprintf("%2dh %2dm %2ds", hours, mins, secs), buff, bp);
  else if (mins > 0)
    safe_str(tprintf("%2dm %2ds", mins, secs), buff, bp);
  else
    safe_str(tprintf("%2ds", secs), buff, bp);
}


static const char *month_table[] =
{
  "Jan",
  "Feb",
  "Mar",
  "Apr",
  "May",
  "Jun",
  "Jul",
  "Aug",
  "Sep",
  "Oct",
  "Nov",
  "Dec",
};

int
do_convtime(str, ttm)
    char *str;
    struct tm *ttm;
{
  /* converts time string to a struct tm. Returns 1 on success, 0 on fail.
   * Time string format is always 24 characters long, in format
   * Ddd Mmm DD HH:MM:SS YYYY
   */

  char *p, *q;
  int i;

  if (strlen(str) != 24)
    return 0;

  /* move over the day of week and truncate. Will always be 3 chars.
   * we don't need this, so we can ignore it.
   */
  if (!(p = strchr(str, ' ')))
    return 0;
  *p++ = '\0';
  if (strlen(str) != 3)
    return 0;

  /* get the month (3 chars), and convert it to a number */
  if (!(q = strchr(p, ' ')))
    return 0;
  *q++ = '\0';
  if (strlen(p) != 3)
    return 0;
  for (i = 0; (i < 12) && strcmp(month_table[i], p); i++) ;
  if (i == 12)			/* not found */
    return 0;
  else
    ttm->tm_mon = i;

  /* get the day of month */
  p = q;
  while (isspace(*p))		/* skip leading space */
    p++;
  if (!(q = strchr(p, ' ')))
    return 0;
  *q++ = '\0';
  ttm->tm_mday = atoi(p);

  /* get hours */
  if (!(p = (char *) strchr(q, ':')))
    return 0;
  *p++ = '\0';
  ttm->tm_hour = atoi(q);

  /* get minutes */
  if (!(q = strchr(p, ':')))
    return 0;
  *q++ = '\0';
  ttm->tm_min = atoi(p);

  /* get seconds */
  if (!(p = strchr(q, ' ')))
    return 0;
  *p++ = '\0';
  ttm->tm_sec = atoi(q);

  /* get year */
  ttm->tm_year = atoi(p) - 1900;

  ttm->tm_isdst = -1;

  return 1;
}

/* ARGSUSED */
FUNCTION(fun_convtime)
{
  /* converts time string to seconds */

  struct tm ttm;

  if (do_convtime(args[0], &ttm)) {
#ifdef SUN_OS
    safe_str(unparse_integer(timelocal(&ttm)), buff, bp);
#else
    safe_str(unparse_integer(mktime(&ttm)), buff, bp);
#endif				/* SUN_OS */
  } else {
    safe_str("-1", buff, bp);
  }
}

#ifdef WIN32
#pragma warning( disable : 4761)	/* NJG: disable warning re conversion */
#endif
/* ARGSUSED */
FUNCTION(fun_isdaylight)
{
  struct tm *ltime;
  time_t tt;

  time(&tt);
  ltime = localtime(&tt);

  safe_chr((ltime->tm_isdst > 0) ? '1' : '0', buff, bp);
}

#ifdef WIN32
#pragma warning( default : 4761)	/* NJG: enable warning re conversion */
#endif
