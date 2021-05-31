/**
 * \file mycrypt.c
 *
 * \brief Password encryption for PennMUSH
 *
 * This file defines the function mush_crypt(key) used for password
 * encryption, depending on the system. Actually, we pretty much
 * expect to use SHS.
 */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include "conf.h"
#if (CRYPT_SYSTEM > 0)
#include "shs.h"
#endif
#include "confmagic.h"

char *mush_crypt(const char *key);

/** Encrypt a password and return ciphertext.
 * \param key plaintext to encrypt.
 * \return encrypted password.
 */
char *
mush_crypt(const char *key)
{
#if (CRYPT_SYSTEM == 0)
  return (char *) key;
#else
  SHS_INFO shsInfo;
  static char crypt_buff[70];

  shsInfo.reverse_wanted = (BYTE) options.reverse_shs;
  shsInit(&shsInfo);
  shsUpdate(&shsInfo, (const BYTE *) key, strlen(key));
  shsFinal(&shsInfo);
  sprintf(crypt_buff, "XX%lu%lu", shsInfo.digest[0], shsInfo.digest[1]);
  return crypt_buff;
#endif
}
