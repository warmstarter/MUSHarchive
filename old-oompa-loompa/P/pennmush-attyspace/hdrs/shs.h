#ifndef _SHS_H
#define _SHS_H

#include "config.h"
#include "confmagic.h"

/* -------------- SHS.H --------------------------- */

typedef unsigned char BYTE;
typedef unsigned long LONG;

#define SHS_BLOCKSIZE 64

#define SHS_DIGESTSIZE 20

typedef struct {
  LONG digest[5];		/* message digest */
  LONG countLo, countHi;	/* 64-bit bit count */
  LONG data[16];		/* SHS data buffer */
  BYTE reverse_wanted;  /* true to reverse (little_endian) false to not */
} SHS_INFO;

void shsInit _((SHS_INFO * shsInfo));
void shsUpdate _((SHS_INFO * shsInfo, BYTE * buffer, int count));
void shsFinal _((SHS_INFO * shsInfo));
#endif
