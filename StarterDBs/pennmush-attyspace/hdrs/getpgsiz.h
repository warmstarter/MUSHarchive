#ifndef __GETPGSIZ_H
#define __GETPGSIZ_H

#ifndef HAS_GETPAGESIZE
#ifndef WIN32
#ifdef I_SYS_PARAM
#include <sys/param.h>
#endif
#endif
#ifdef I_SYS_PAGE
#include <sys/page.h>
#endif

#ifdef EXEC_PAGESIZE
#define getpagesize() EXEC_PAGESIZE
#else
#ifdef NBPG
#define getpagesize() NBPG * CLSIZE
#ifndef CLSIZE
#define CLSIZE 1
#endif				/* no CLSIZE */
#else				/* no NBPG */
#ifdef NBPC
#define getpagesize() NBPC
#else				/* no NBPC either? Bummer */
#define getpagesize() PAGESIZE
#endif				/* no NBPC */
#endif				/* no NBPG */
#endif				/* no EXEC_PAGESIZE */

#endif				/* not HAS_GETPAGESIZE */

#endif				/* __GETPGSIZ_H */
