/* comp_b.c */

/* Compression routines */
#include "copyrite.h"
#include "config.h"

#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#include "conf.h"
#include "mushdb.h"
#include "intrface.h"
#include "externs.h"
#include "confmagic.h"

/* These use a pathetically simple encoding that takes advantage of the */
/* eighth bit on a char; if you are using an international character set, */
/* they may need substantial patching. 
 * This is basically the old bigram compression from pl9, with
 * automatic table tuning at startup.
 */

#define TOKEN_BIT 0x80		/* if on, it's a token */
#define TOKEN_MASK 0x7f		/* for stripping out token value */
#define NUM_TOKENS 128
#define MAX_CHAR 128
#ifndef SAMPLE_SIZE
#define SAMPLE_SIZE	0
#endif


static unsigned char tokens[NUM_TOKENS][3];
static unsigned char token_table[MAX_CHAR][MAX_CHAR];
static int table_initialized = 0;


int init_compress _((FILE * f));
static void list_to_table _((void));
static int list_insert _((unsigned char c1, unsigned char c2, int count));
static int compressed _((const char *s));

/* Initialize the bigram table
 * 1. Read indb (up to SAMPLE_SIZE chars, if defined) and count
 *    the frequency of bigrams
 * 2. Cheat the relative frequency of some known special chars
 *    and upper-case letters
 * 3. Construct a bigram table
 */
int
init_compress(f)
    FILE *f;
{
  int counts[256][256];
  unsigned char last;
  unsigned char c, d, highc, highd;
  int i;
  int highest;
  int total;

  /* Initialize the table */
  for (c = 0; c < 255; c++)
    for (last = 0; last < 255; last++)
      if (!isprint(c) || !isprint(last))
	counts[c][last] = -1;
      else
	counts[c][last] = 0;


  /* Scan the db */
  total = 0;
  last = 255;
  while (!feof(f) && (!SAMPLE_SIZE || (total++ < SAMPLE_SIZE))) {
    c = fgetc(f);
    if (c == '\n')
      continue;
    if (last == 255) {
      last = c;
      continue;
    }
    if (counts[last][c] != -1)
      counts[last][c]++;
    last = c;
  }

  /* The ']' character is artificially raised by being the
   * start-of-attribute marker in indb.  Set it back to '[',
   * which it should be balancing...
   */
  for (c = 0; c < 255; c++) {
    counts[']'][c] = counts['['][c];
    counts[c][']'] = counts[c]['['];
  }

  /* Now run through the counts table and build the tokens table
   * by finding the 128 highest frequency bigrams from counts
   * When we add a bigram to the table, we also change its
   * frequency to -1 so it won't get picked again.
   */
  highc = highd = 0;
  for (i = 0; i < NUM_TOKENS; i++) {
    highest = 0;
    for (c = 0; c < 255; c++) {
      for (d = 0; d < 255; d++) {
	if (counts[c][d] >= highest) {
	  highc = c;
	  highd = d;
	  highest = counts[c][d];
	}
      }
    }
    tokens[i][0] = highc;
    tokens[i][1] = highd;
    token_table[tokens[i][0]][tokens[i][1]] = i | TOKEN_BIT;
    counts[highc][highd] = -1;
  }

  table_initialized = 1;
  return 0;
}


static int
compressed(s)
    const char *s;
{
  while (*s) {
    if (*s++ & TOKEN_BIT)
      return 1;
  }
  return 0;
}

unsigned char *
compress(s)
    char const *s;
{
  static unsigned char buf[BUFFER_LEN];
  unsigned char *to;
  unsigned char token;
  unsigned char *p;

  p = (unsigned char *) s;
  if (compressed(s))
    return p;			/* already compressed */

  /* tokenize the first characters */
  for (to = buf; p[0] && p[1]; to++) {
    token = token_table[p[0]][p[1]];
    if (token) {
      *to = token;
      p += 2;
    } else {
      *to = p[0];
      p++;
    }
  }

  /* copy the last character (if any) and null */
  while ((*to++ = *p++) != '\0') ;

  return buf;
}

char *
uncompress(s)
    unsigned char const *s;
{
  /* to avoid generating memory problems, this function should be
   * used with something of the format
   * char tbuf1[BUFFER_LEN];
   * strcpy(tbuf1, uncompress(a->value));
   * if you are using something of type char *buff, use the
   * safe_uncompress function instead.
   */

  static char buf[BUFFER_LEN];
  char *to;
  char *token;
  unsigned char *p = (unsigned char *) s;
  for (to = buf; *p; p++) {
    if (*p & TOKEN_BIT) {
      token = (char *) tokens[*p & TOKEN_MASK];
      *to++ = *token++;
      *to++ = *token;
    } else {
      *to++ = *p;
    }
  }

  *to++ = *p;

  return buf;
}

char *
safe_uncompress(s)
    unsigned char const *s;
{
  /* this function should be used when you're doing something like
   * char *attrib = safe_uncompress(a->value);
   * NEVER use it with something like 
   * char tbuf1[BUFFER_LEN]; strcpy(tbuf1, safe_uncompress(a->value));
   * or you will create a horrendous memory leak.
   */
  return strdup(uncompress(s));
}
