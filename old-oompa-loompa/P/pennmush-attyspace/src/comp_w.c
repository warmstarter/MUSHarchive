/* Compression routines */

/*
   Table-lookup (word) compress, written by Nick Gammon. 21/Oct/95.

   This method maintains a table of 32768 words, where a "word" is defined
   as a sequence of alpha or numeric characters (such as "dog123").

   Compression
   -----------

   The text to be compressed is broken up into "words" and other symbols
   (e.g. commas, brackets, spaces and so on).

   Words are looked up in the word table by using a hash-table lookup. If
   found, the "index" into the table is emitted with the high order bit set.
   If not found, the word is added to the table, and is then emitted as
   described above.

   For example: "The dog and the other dog sat on the mat (eating fish)."
   would compress as:

   0x8001      The
   0x8002      dog
   0x8003      and
   0x8004      the
   0x8005      other
   0x8002      dog
   0x8007      sat
   0x8008      on
   0x8004      the
   0x8009      mat
   0x28        (
   0x800A      eating
   0x800B      fish)
   0x2E        .

   In the above example, the uncompressed text is 55 bytes, and the compressed 
   text is 26 bytes.

   For simplicity, the above example assumes that the words hash to consecutive
   table positions.

   Note that the trailing punctuation character (space, period or whatever) is 
   considered _part_ of the word. This is to save having to store multiple 
   spaces between each word, and is relying on the fact that a certain word is
   usually followed by the same punctuation. For example, the word "and" would
   normally be followed by a space.

   In the above example, the space following "mat" is stored in the table, and
   thus the "(" character had to be output separately. The ")" following the
   word "fish" is considered part of the word and is stored in the table, 
   however the last period was output as a separate character.

   Note how the high-order bit is turned on for words in the table, this is so
   the decompression routine can detect table lookup characters from ordinary
   ones. Also, any table index with a low-order byte of zero cannot be used,
   as this would cause the resulting "string" to be prematurely truncated (by
   strcpy, strcmp etc.). Thus the lookup routine skips any positions that hash
   to the value 0xXX00. (There are only 256 such indices).

   Also, to prevent characters which the user might type in with the high order
   bit set causing decompression confusion, all text is stripped of its high
   order bit before being added to the table or emitted.

   In the event of two words hashing to the same value, the compression routine
   scans forwards for COLLISION_LIMIT entries, looking for a match or a spare
   table entry. If none is found, the word is output "as is" (i.e.
   uncompressed). You might speed up compression slightly by lowering
   COLLISION_LIMIT, at the cost of slightly lower compression ratios. 

   The collision limit does not affect _decompression_ as that merely involves
   a direct table lookup.

   Decompression
   -------------

   Decompression involves a simple loop, in which each byte of the compressed
   text is examined. If it has the high-order bit clear, it is just emitted.
   If the high order bit is set it, along with the next byte in the compressed
   text, are used to index into the word lookup table. The word found there is
   then emitted.

   Speed
   -----

   Tests conducted on (hopefully typical) text show that decompression is about
   4 times as fast as Huffman decompression.

   Compression however is about 3.5 times as _slow_ as Huffman compression.

   The slower compression time is considered acceptable on the grounds that
   text is much more often _decompressed_ in a MUSH than compressed. Compression
   mainly takes part at database load time (say, once a week) whereas 
   decompression take part every hour, as the database is dumped to disk, and
   whenever an object description is displayed, or an attribute searched for,

   Compression ratio
   -----------------

   The "table compression" method has a poor compression ratio for small amounts
   of text, because of the overhead of the 32768 pointers, and the data stored
   in them. However, as the database size increases, the ratio improves because
   the table overhead becomes progressively less significant.

   The break-even points is with about 1.5 Mb of text, where both the table 
   compression and Huffman compress to about 63% of the size of the original.

   After that, the compression ratio gradually improves until reaching somewhere
   between 40% and 50% of the size of the original, as the amount of text to
   compress reaches 10 Mb.

   The nature of Huffman compression however is such that it will always be 
   fixed at about 63% regardless of the amount of data compressed.

 */

#include "copyrite.h"
#include "config.h"
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_STDLIB			/* NJG: added <stdlib.h> */
#include <stdlib.h>
#endif

#include <ctype.h>
#include "conf.h"
#include "externs.h"
#include "intrface.h"
#include "mushdb.h"
#include "mymalloc.h"
#include "confmagic.h"

#define MAXTABLE 32768
#define MAXWORD 100
#define COLLISION_LIMIT 20

#define COMPRESS_HASH_MASK 0x7FFF	/* 32767 */

#define TABLE_FLAG 0x80
#define TABLE_MASK 0x7F

/* Table of words */

static char *words[MAXTABLE];

/* The word we are currently compressing */

static char word[MAXWORD + 2];
static int wordpos = 0;

/* Stats */

static long total_mallocs = 0;
static long total_uncomp = 0;
static long total_comp = 0;
static long total_entries = 0;

/* Work pointer for compression */

static unsigned char *b;

static void output_previous_word _((void));
int init_compress _((FILE * f));
void compress_stats _((long *entries, long *mem_used, long *total_uncompressed,
		       long *total_compressed));
unsigned int hash_fn _((const char *s, int hashtab_mask));

static void
output_previous_word()
{
  char *p;
  int i, j;

  word[wordpos++] = 0;		/* word's trailing null */

/* Don't bother putting single or double letter words in the table */

  if (wordpos <= 3) {
    p = word;
    while (*p)
      *b++ = *p++;
    return;
  }
/* search table to see if word is already in it; */

  for (i = hash_fn(word, COMPRESS_HASH_MASK), j = 0;
       i < MAXTABLE &&
       (words[i] || (i & 0xFF) == 0) &&
       j < COLLISION_LIMIT;
       i++, j++)
    if (words[i])
      if (strcmp(word, words[i]) == 0) {
	*b++ = (i >> 8) | TABLE_FLAG;
	*b++ = i & 0xFF;
	return;
      }
/* not in table, add to it */

  if ((i & 0xFF) == 0) {
    i++;			/* make sure we don't have a null in the message */
    j++;
  }
/* Can't add to table if full */

  if (i >= MAXTABLE || j >= COLLISION_LIMIT) {
    p = word;
    while (*p)
      *b++ = *p++;
    return;
  }
  words[i] = malloc(wordpos);

  if (!words[i])
    panic("Out of memory in string compression routine");

  total_mallocs += wordpos;
  total_entries++;

  strcpy(words[i], word);

  *b++ = (i >> 8) | TABLE_FLAG;
  *b++ = i & 0xFF;

}				/* end of output_previous_word */

unsigned char *
compress(s)
    char const *s;
{
  const unsigned char *p;
  static char buf[BUFFER_LEN];

  p = (unsigned char *) s;
  b = buf;

  wordpos = 0;

/* break up input into words */
  while (*p) {
    if (!(isdigit(*p) || isalpha(*p)) || wordpos >= MAXWORD) {
      if (wordpos) {
	word[wordpos++] = *p & 0x7F;	/* add trailing punctuation */
	output_previous_word();
	wordpos = 0;
      } else
	*b++ = *p & 0x7F;
    } else
      word[wordpos++] = *p & 0x7F;
    p++;
  }

  if (wordpos)
    output_previous_word();

  *b = 0;			/* trailing null */

  total_comp += strlen(buf);	/* calculate size of compressed   text */
  total_uncomp += strlen(s);	/* calculate size of uncompressed text */

  return buf;
}				/* end of compress; */


char *
uncompress(s)
    unsigned char const *s;
{

  const unsigned char *p;
  char c;
  int i;
  static char buf[BUFFER_LEN];

  buf[0] = '\0';
  if (!s || !*s)
    return buf;
  p = (unsigned char *) s;
  b = buf;

  while (*p) {
    c = *p;
    if (c & TABLE_FLAG) {
      i = ((c & TABLE_MASK) << 8) | *(++p);
      if (i >= MAXTABLE || words[i] == NULL) {
	static int panicking = 0;
	if (panicking) {
	  fprintf(stderr,
	  "Error in string decompression occurred during panic dump.\n");
	  exit(1);
	} else {
	  panicking = 1;	/* don't panic from within panic */
	  fprintf(stderr, "Error in string decompression, i = %i\n", i);
	  panic("Fatal error in decompression");
	}
      }
      strcpy(b, words[i]);
      b += strlen(words[i]);
    } else
      *b++ = c;
    p++;
  }

  *b++ = 0;			/* trailing null */

  return buf;

}				/* end of uncompress; */


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

  return (char *) strdup((char *) uncompress(s));
}


int
init_compress(f)
    FILE *f;
{

/* Clear the words table the first time through */

  memset(words, 0, sizeof(words));

  return 0;
}

void
compress_stats(entries, mem_used, total_uncompressed, total_compressed)
    long *entries;
    long *mem_used;
    long *total_uncompressed;
    long *total_compressed;
{

  *entries = total_entries;
  *mem_used = total_mallocs;
  *total_uncompressed = total_uncomp;
  *total_compressed = total_comp;

}

unsigned
hash_fn(s, hashtab_mask)
    const char *s;
    int hashtab_mask;
{
  /* hash function, using masks (based on TinyMUSH 2.0) */

  unsigned hashval;
  const char *p;

  for (hashval = 0, p = s; *p; p++)
    hashval = (hashval << 5) + hashval + *p;
  return (hashval & hashtab_mask);
}
