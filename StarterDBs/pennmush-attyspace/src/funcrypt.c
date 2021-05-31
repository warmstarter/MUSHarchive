/* This is the USA/Canadian version of the PennMUSH 1.60
 * encrypt/decrypt functions. Export of this source code
 * outside of the United States or Canada is restricted,
 * and probably not legal. Don't do it.
 *
 * This file replaces the src/funcrypt.c that's distributed.
 */

#include "ansi.h"

#ifdef TREKMUSH
extern char *crunch_code _((char *code)); 
extern char *crypt_code _((char *code, char *text, int type));
#else /* TREKMUSH */
static char *crunch_code _((char *code)); 
static char *crypt_code _((char *code, char *text, int type));
#endif /* TREKMUSH */

/* Copy over only alphanumeric chars */
#ifdef TREKMUSH
extern char *
#else /* TREKMUSH */
static char *
#endif /* TREKMUSH */
crunch_code(code)
    char *code;
{
  char *in;
  char *out;
  static char output[BUFFER_LEN];

  out = output;
  in = code;
  while (*in) {
    while (*in == ESC_CHAR) {
       while (*in && *in != 'm') in++;
       in++; /* skip 'm' */
    }
    if ((*in >= 32) || (*in <= 126)) {
      printf("%c", *in);
      *out++ = *in;
    }
    in++;
  }
  *out = '\0';
  return (output);
}

#ifdef TREKMUSH
extern char *
#else /* TREKMUSH */
static char *
#endif /* TREKMUSH */
crypt_code(code, text, type)
    char *code;
    char *text;
    int type;
{
  static char textbuff[BUFFER_LEN];
  char codebuff[BUFFER_LEN];
  int start = 32;
  int end = 126;
  int mod = end - start + 1;
  char *p, *q, *r;

  if (!text && !*text)
    return ((char *) "");
  strcpy(codebuff, crunch_code(code));
  if (!code || !*code || !codebuff || !*codebuff)
    return (text);
  strcpy(textbuff, "");

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
  return (textbuff);
}

FUNCTION(fun_encrypt)
{
  safe_str(crypt_code(args[1], args[0], 1), buff, bp);
}

FUNCTION(fun_decrypt)
{
  safe_str(crypt_code(args[1], args[0], 0), buff, bp);
}

