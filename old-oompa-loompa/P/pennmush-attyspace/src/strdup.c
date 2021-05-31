
/* strdup.c */

/* Strdup routine for systems without one. */
#include "config.h"

#include <ctype.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#ifdef I_MEMORY
#include <memory.h>
#endif
#include "conf.h"
#include "copyrite.h"
#include "mymalloc.h"
#include "confmagic.h"

#ifndef HAS_STRDUP
char *strdup _((const char *s));

char *
strdup(s)
    const char *s;
{
  int len;
  char *dup = NULL;

  len = strlen(s) + 1;
  if ((dup = (char *) malloc(len)) != NULL)
    memcpy(dup, s, len);
  return (dup);
}
#endif
