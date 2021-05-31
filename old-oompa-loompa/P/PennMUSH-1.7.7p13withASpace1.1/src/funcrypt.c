/**
 * \file funcrypt.c
 *
 * \brief Functions for cryptographic stuff in softcode
 *
 *
 */
#include "copyrite.h"

#include "config.h"
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "conf.h"
#include "case.h"
#include "externs.h"
#include "version.h"
#include "extchat.h"
#include "htab.h"
#include "flags.h"
#include "dbdefs.h"
#include "parse.h"
#include "function.h"
#include "command.h"
#include "game.h"
#include "attrib.h"
#include "ansi.h"
#include "match.h"
#include "shs.h"
#include "confmagic.h"
#include "options.h"
  
#ifdef ASPACE
extern char *crunch_code _((char *code));
extern char *crypt_code _((char *code, char *text, int type));
#else
static char *crunch_code _((char *code));
static char *crypt_code _((char *code, char *text, int type));
#endif

/* Copy over only alphanumeric chars */
#ifdef ASPACE
extern char *
#else
static char *
#endif
crunch_code(char *code)
{
  char *in;
  char *out;
  static char output[BUFFER_LEN];

  out = output;
  in = code;
  while (*in) {
    while (*in == ESC_CHAR) {
      while (*in && *in != 'm')
	in++;
      in++;			/* skip 'm' */
    }
    if ((*in >= 32) && (*in <= 126)) {
      *out++ = *in;
    }
    in++;
  }
  *out = '\0';
  return output;
}

#ifdef ASPACE
extern char *
#else
static char *
#endif
crypt_code(char *code, char *text, int type)
{
  static char textbuff[BUFFER_LEN];
  char codebuff[BUFFER_LEN];
  int start = 32;
  int end = 126;
  int mod = end - start + 1;
  char *p, *q, *r;

  if (!text && !*text)
    return (char *) "";
  strcpy(codebuff, crunch_code(code));
  if (!code || !*code || !codebuff || !*codebuff)
    return text;
  textbuff[0] = '\0';

  p = text;
  q = codebuff;
  r = textbuff;
  /* Encryption: Simply go through each character of the text, get its ascii
   * value, subtract start, add the ascii value (less start) of the
   * code, mod the result, add start. Continue  */
  while (*p) {
    if ((*p < start) || (*p > end)) {
      p++;
      continue;
    }
    if (type)
      *r++ = (((*p++ - start) + (*q++ - start)) % mod) + start;
    else
      *r++ = (((*p++ - *q++) + 2 * mod) % mod) + start;
    if (!*q)
      q = codebuff;
  }
  *r = '\0';
  return textbuff;
}

FUNCTION(fun_encrypt)
{
  safe_str(crypt_code(args[1], args[0], 1), buff, bp);
}

FUNCTION(fun_decrypt)
{
  safe_str(crypt_code(args[1], args[0], 0), buff, bp);
}

FUNCTION(fun_checkpass)
{
  dbref it = match_thing(executor, args[0]);
  if (!(GoodObject(it) && IsPlayer(it))) {
    safe_str(T("#-1 NO SUCH PLAYER"), buff, bp);
    return;
  }
  safe_boolean(password_check(it, args[1]), buff, bp);
}

FUNCTION(fun_sha1)
{
  SHS_INFO shsInfo;
  shsInfo.reverse_wanted = (BYTE) options.reverse_shs;
  shsInit(&shsInfo);
  shsUpdate(&shsInfo, (const BYTE *) args[0], arglens[0]);
  shsFinal(&shsInfo);
  safe_format(buff, bp, "%0lx%0lx%0lx%0lx%0lx", shsInfo.digest[0],
	      shsInfo.digest[1], shsInfo.digest[2], shsInfo.digest[3],
	      shsInfo.digest[4]);
}
