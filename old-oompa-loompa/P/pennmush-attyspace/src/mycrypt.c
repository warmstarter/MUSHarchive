/* This file defines the function mush_crypt(key) used for password
 * encryption, depending on the system.
 */

#include "config.h"
#include <stdio.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include "conf.h"
#ifdef I_CRYPT
#include <crypt.h>
#else
#if (CRYPT_SYSTEM == 1)
extern char *crypt _((const char *, const char *));
#endif
#endif
#if (CRYPT_SYSTEM == 2)
#include "shs.h"
#endif
#include "confmagic.h"

#if (CRYPT_SYSTEM == 2)
void shsInit _((SHS_INFO * shsInfo));
void shsUpdate _((SHS_INFO * shsInfo, BYTE * buffer, int count));
void shsFinal _((SHS_INFO * shsInfo));
#include "shs.c"
#endif

char *mush_crypt _((const char *key));
char *
mush_crypt(key)
    const char *key;
{
#if (CRYPT_SYSTEM == 2)
  SHS_INFO shsInfo;
  static char crypt_buff[70];
#endif

#if (CRYPT_SYSTEM == 0)
  return (char *) key;
#endif
#if (CRYPT_SYSTEM == 1)
#ifdef HAS_CRYPT
  return crypt(key, "XX");
#else
  return (char *) key;
#endif
#endif
#if (CRYPT_SYSTEM == 2)
  shsInfo.reverse_wanted = (BYTE) options.reverse_shs;
  shsInit(&shsInfo);
  shsUpdate(&shsInfo, (BYTE *) key, strlen(key));
  shsFinal(&shsInfo);
  sprintf(crypt_buff, "XX%lu%lu", shsInfo.digest[0],
	  shsInfo.digest[1]);
  return crypt_buff;
#endif
}
