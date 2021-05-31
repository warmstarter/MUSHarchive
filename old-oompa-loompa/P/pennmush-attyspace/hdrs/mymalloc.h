/* A wrapper header that sets up the proper defines based on
 * options.h's MALLOC_PACKAGE
 */

#ifndef _MYMALLOC_H
#define _MYMALLOC_H

#ifdef WIN32
#undef malloc
#undef calloc
#undef realloc
#undef free
#endif

#ifdef I_STDLIB
#include <stdlib.h>
#else
/* If you're using gmalloc on some linux kernels, and have trouble
 * with the compile, consider uncommenting this line: */
/*#undef I_MALLOC */
#ifdef I_MALLOC
#include <malloc.h>
#endif
#endif

#include "options.h"

#if (MALLOC_PACKAGE == 1)
#define CSRI
#elif (MALLOC_PACKAGE == 2)
#define CSRI
#define CSRI_TRACE
#define CSRI_PROFILESIZES
#define CSRI_DEBUG
#elif (MALLOC_PACKAGE == 3)
#define USE_SMALLOC
#elif (MALLOC_PACKAGE == 4)
#define USE_SMALLOC
#define SLOW_STATISTICS
#endif

#endif				/* _MYMALLOC_H */
