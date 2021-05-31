#ifndef LOG_H
#define LOG_H
/* Added for Aspace */
#include "options.h"
/* End Aspace */
/* log types */
#define LT_ERR    0
#define LT_CMD    1
#define LT_WIZ    2
#define LT_CONN   3
#define LT_TRACE  4
#define LT_RPAGE  5		/* Obsolete */
#define LT_CHECK  6
#define LT_HUH    7
/* Added for Aspace */
#ifdef ASPACE
#define LT_SPACE  8
#endif
/* End Aspace */
/* From log.c */
extern void start_all_logs(void);
extern void end_all_logs(void);
extern void WIN32_CDECL do_log
  (int logtype, dbref player, dbref object, const char *fmt, ...)
  __attribute__ ((__format__(__printf__, 4, 5)));
extern void WIN32_CDECL do_rawlog(int logtype, const char *fmt, ...)
  __attribute__ ((__format__(__printf__, 2, 3)));
extern void do_logwipe(dbref player, int logtype, char *str);


#endif				/* LOG_H */
