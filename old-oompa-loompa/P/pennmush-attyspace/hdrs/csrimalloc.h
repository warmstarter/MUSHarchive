/*  Author: Mark Moraes <moraes@csri.toronto.edu> */

/* Modified by Alan Schwartz for PennMUSH.
 * Should be included after config.h
 */

#ifndef __CSRIMALLOC_H
#define __CSRIMALLOC_H

#include "config.h"
#define univptr_t		Malloc_t
#define memSize_t		Size_t
#include "confmagic.h"

/*
 *  defined so users of new features of this malloc can #ifdef
 *  invocations of those features.
 */
#define CSRIMALLOC

#ifdef CSRI_TRACE
/* Tracing malloc definitions - helps find leaks */

extern univptr_t __malloc _((Size_t nbytes, const char *fname, int linenum));
extern univptr_t __calloc _((Size_t nelem, Size_t elsize, const char *fname, int linenum));
extern univptr_t __realloc _((univptr_t cp, Size_t nbytes, const char *fname, int linenum));
extern univptr_t __valloc _((Size_t size, const char *fname, int linenum));
extern univptr_t __memalign _((Size_t alignment, Size_t size, const char *fname, int linenum));
extern univptr_t __emalloc _((Size_t nbytes, const char *fname, int linenum));
extern univptr_t __ecalloc _((Size_t nelem, Size_t sz, const char *fname, int linenum));
extern univptr_t __erealloc _((univptr_t ptr, Size_t nbytes, const char *fname, int linenum));
extern char *__strdup _((const char *s, const char *fname, int linenum));
extern char *__strsave _((const char *s, const char *fname, int linenum));
extern void __free _((univptr_t cp, const char *fname, int linenum));
extern void __cfree _((univptr_t cp, const char *fname, int linenum));

#define malloc(x)		__malloc((x), __FILE__, __LINE__)
#define calloc(x, n)		__calloc((x), (n), __FILE__, __LINE__)
#define realloc(p, x)		__realloc((p), (x), __FILE__, __LINE__)
#define memalign(x, n)		__memalign((x), (n), __FILE__, __LINE__)
#define valloc(x)		__valloc((x), __FILE__, __LINE__)
#define emalloc(x)		__emalloc((x), __FILE__, __LINE__)
#define ecalloc(x, n)		__ecalloc((x), (n), __FILE__, __LINE__)
#define erealloc(p, x)		__erealloc((p), (x), __FILE__, __LINE__)
#define strdup(p)		__strdup((p), __FILE__, __LINE__)
#define strsave(p)		__strsave((p), __FILE__, __LINE__)
/* cfree and free are identical */
#define cfree(p)		__free((p), __FILE__, __LINE__)
#define free(p)			__free((p), __FILE__, __LINE__)

#else				/* CSRI_TRACE */

extern univptr_t malloc _((Size_t nbytes));
extern univptr_t calloc _((Size_t nelem, Size_t elsize));
extern univptr_t realloc _((univptr_t cp, Size_t nbytes));
extern univptr_t valloc _((Size_t size));
extern univptr_t memalign _((Size_t alignment, Size_t size));
extern univptr_t emalloc _((Size_t nbytes));
extern univptr_t ecalloc _((Size_t nelem, Size_t sz));
extern univptr_t erealloc _((univptr_t ptr, Size_t nbytes));
extern char *strdup _((const char *s));
extern char *strsave _((const char *s));
extern Free_t free _((univptr_t cp));
extern Free_t cfree _((univptr_t cp));

#endif				/* CSRI_TRACE */

extern void mal_debug _((int level));
extern void mal_dumpleaktrace _((FILE * fp));
extern void mal_heapdump _((FILE * fp));
extern void mal_leaktrace _((int value));
extern void mal_sbrkset _((int n));
extern void mal_slopset _((int n));
extern void mal_statsdump _((FILE * fp));
extern void mal_setstatsfile _((FILE * fp));
extern void mal_trace _((int value));
extern int mal_verify _((int fullcheck));
extern void mal_mmap _((char *fname));


/*
 *  You may or may not want this - In gcc version 1.30, on Sun3s running
 *  SunOS3.5, this works fine.
 */
#ifdef __GNUC__
#define alloca(n) __builtin_alloca(n)
#endif				/* __GNUC__ */
#ifdef sparc
#define alloca(n) __builtin_alloca(n)
#endif				/* sparc */


#endif	/* __CSRIMALLOC_H__ */	/* Do not add anything after this line */
