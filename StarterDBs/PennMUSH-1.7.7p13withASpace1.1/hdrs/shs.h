#ifndef _SHS_H
#define _SHS_H

#include "config.h"
#include "confmagic.h"

/* -------------- SHS.H --------------------------- */

typedef unsigned char BYTE;
typedef unsigned long LONG;

#define SHS_BLOCKSIZE 64

#define SHS_DIGESTSIZE 20

/** An shs digest.
 */
typedef struct {
  LONG digest[5];		/**< message digest */
  LONG countLo, countHi;	/**< 64-bit bit count */
  LONG data[16];		/**< SHS data buffer */
  BYTE reverse_wanted;		/**< true to reverse (little_endian) false to not */
} SHS_INFO;

void shsInit(SHS_INFO *shsInfo);
void shsUpdate(SHS_INFO *shsInfo, const BYTE * buffer, int count);
void shsFinal(SHS_INFO *shsInfo);
#endif
