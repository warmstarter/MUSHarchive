#ifdef USE_SMALLOC
/*

 * * Satoria's Malloc Package 1.2
 *
 */

#include <stdio.h>
#include <memory.h>
#include "config.h"
#include "conf.h"
#include "externs.h"
#include "mymalloc.h"
#include "confmagic.h"

#define MTYPE unsigned int

#define SMALL_BLOCK_MAX_BYTES	32
#define SMALL_CHUNK_SIZE	0x4000
#define CHUNK_SIZE		0x40000

#define SINT sizeof(int)
#define SMALL_BLOCK_MAX (SMALL_BLOCK_MAX_BYTES/SINT)

#define PREV_BLOCK	0x80000000
#define THIS_BLOCK	0x40000000
#define MASK		0x0FFFFFFF

typedef unsigned int u;

/*
 * SMALL BLOCK data structures
 */

static u *sfltable[SMALL_BLOCK_MAX];	/* freed list */
static u *next_unused;
static u unused_size;		/* until we need a new chunk */

/*
 * LARGE BLOCK data structures
 */

static u *free_list;
static u *start_next_block;

/*
 * STATISTICS
 */

static int small_count[SMALL_BLOCK_MAX];
static int small_total[SMALL_BLOCK_MAX];

typedef struct {
  unsigned counter;
  unsigned long size;
} stat;

#ifndef SLOW_STATISTICS
#define COUNT(a,b)
#define START(a)
#else
#define COUNT(a,b) 			\
	{				\
		a.size+=(b);		\
		if ((b)<0)		\
			--a.counter;	\
		else			\
			++a.counter;	\
	}
#define START(a) a.size=0; a.counter=0;
#endif

#ifdef DEBUG
int debugmalloc = 1;		/* Only used when debuging malloc() */
#else
int debugmalloc = 0;
#endif

/********************************************************/
/*  SMALL BLOCK HANDLER                                 */
/********************************************************/

char *large_malloc();
static void large_free();

#define ptr_to_smallblock_size_field(p)	(p)
#define next_free_smallblock(p)	((u **) (p+1))

stat small_alloc_stat,		/* amount allocated */
 small_free_stat,		/* amount unused */
 small_total_stat,		/* total ever requested */
 small_chunk_stat;		/* small chunks allocated */

void *
malloc(sz)
    MTYPE sz;
{
  u *temp, i, size;

  if (sz == 0) {
    fprintf(stderr, "Malloc size 0.\n");
    return NULL;
  }
  if (sz > SMALL_BLOCK_MAX_BYTES)
    return large_malloc((u) sz, 0);

/* Compute the index into the small block table:
 * index                block size
 * 0              1-4 bytes
 * 1              5-8 bytes
 * 2              9-12 bytes
 */

  i = (sz - 1) >> 2;

/* Compute the actual size of the small block in ints,
 * including the one int overhead for the "header"
 */

  size = (sz + 3) >> 2;		/* block size in ints */
  ++size;			/* one int overhead */

/* Update statistics of small block allocations */

  COUNT(small_alloc_stat, size << 2);
  COUNT(small_total_stat, size << 2);

  small_count[i] += 1;		/* update statistics */
  small_total[i] += 1;

/* Is the free list for this size of block non-empty? */

  if (sfltable[i]) {

/* Update the free list stats */

    COUNT(small_free_stat, -(int) (size << 2));

    temp = sfltable[i];
    sfltable[i] = *(u **) (temp + 1);

    return (char *) (temp + 1);
  }
/* Free list was empty.  So allocate from the small-block chunk. */

  if (unused_size < size) {

/* There's not enough room in this chunk.  Blow it off (wasting the
 * space at the end of the chunk), and get a new one. */

    next_unused = (u *) large_malloc(SMALL_CHUNK_SIZE, 1);
    if (next_unused == 0)
      return 0;

    COUNT(small_chunk_stat, SMALL_CHUNK_SIZE + SINT);
    unused_size = SMALL_CHUNK_SIZE / SINT;
  }
  temp = (u *) next_free_smallblock(next_unused);

  *ptr_to_smallblock_size_field(next_unused) = size;
  next_unused += size;
  unused_size -= size;

  return (char *) temp;
}

void
free(ptr)
    char *ptr;
{
  u *block;
  u i;

/* Find the header for this block */

  block = (u *) ptr;
  block -= 1;

/* Test the size of this block to see if it's large or small */

  if ((*ptr_to_smallblock_size_field(block) & MASK)
      > SMALL_BLOCK_MAX + 1) {
    large_free(ptr);
    return;
  }
/* Get index for this size of block.  There's no & MASK because
 * small block headers only have the size
 */
  i = *block - 2;

/* Update the statistics */

  COUNT(small_alloc_stat, -(int) ((i + 2) << 2));
  COUNT(small_free_stat, (i + 2) << 2);

  small_count[i] -= 1;

  *next_free_smallblock(block) = sfltable[i];
  sfltable[i] = block;
}

/************************************************/
/*      LARGE BLOCK HANDLER                     */
/************************************************/

#define BEST_FIT	0
#define FIRST_FIT	1
#define HYBRID		2
int fit_style = BEST_FIT;

#define ptr_to_largeblock_size_field(p)			(p)
#define next_free_largeblock(p)				(*((u **) (p+1)))
#define previous_free_largeblock(p)			(*((u **) (p+2)))
#define next_large_block(p)				(p + (*(p) & MASK))
#define previous_largeblock(p) 				(p - (*(p-1)))
#define is_previous_free(p)				(!(*p & PREV_BLOCK))
#define is_next_free(p)			(!(*next_large_block(p) & THIS_BLOCK))

#ifdef DEBUG
static void
show_block(ptr)
    u *ptr;
{
  fprintf(stderr, "MALLOC: [%c%d: %d]\n", (*ptr & THIS_BLOCK ? '+' : '-'),
	  (int) ptr, *ptr & MASK);
}

void
show_free_list()
{
  u *p;
  p = free_list;
  while (p) {
    show_block(p);
    p = next_free_largeblock(p);
  }
}
#endif

stat large_free_stat;
stat large_free_chain;

void
remove_from_free_list(ptr)
    u *ptr;
{
  COUNT(large_free_stat, -(int) ((*ptr & MASK) << 2));

  if (previous_free_largeblock(ptr))
    next_free_largeblock(previous_free_largeblock(ptr))
      = next_free_largeblock(ptr);
  else
    free_list
      = next_free_largeblock(ptr);

  if (next_free_largeblock(ptr))
    previous_free_largeblock(next_free_largeblock(ptr))
      = previous_free_largeblock(ptr);
}

void
add_to_free_list(ptr)
    u *ptr;
{
  COUNT(large_free_stat, (*ptr & MASK) << 2);

  if (free_list && previous_free_largeblock(free_list))
    fprintf(stderr, "ERROR: Free list consistency error.\n");

  next_free_largeblock(ptr) = free_list;
  if (free_list)
    previous_free_largeblock(free_list) = ptr;
  previous_free_largeblock(ptr) = 0;
  free_list = ptr;
}

/* build a properly annotated unallocated block */
static void
build_block(ptr, size)
    u *ptr, size;
{

/* Set the header to:  (old value of PREV_BLOCK) | !THIS_BLOCK | size */

  *(ptr) = (*ptr & PREV_BLOCK) | size;

/* Set the footer (immediately before the next blocks' header) to the size */

  *(ptr + size - 1) = size;

/* Clear PREV_BLOCK on the next block */

  *(ptr + size) &= (MASK | THIS_BLOCK);
}

static void
mark_block(ptr)
    u *ptr;
{
  *ptr |= THIS_BLOCK;
  *next_large_block(ptr) |= PREV_BLOCK;
}


/*
 * mod by Lars Pensj| (??):
 * It is system dependent how sbrk() aligns data, so we simply
 * use brk() to insure that we have enough.
 */
stat sbrk_stat;
static char *
esbrk(size)
    u size;
{
  extern char *sbrk();
  extern int brk();
  static char *current_break;

  if (current_break == 0)
    current_break = sbrk(0);
  if (brk(current_break + size) == -1)
    return 0;
  COUNT(sbrk_stat, size);
  current_break += size;
  return current_break - size;
}

stat large_alloc_stat;
stat large_total_stat;
char *
large_malloc(size, force_more)
    u size;
    int force_more;
{
  u best_size, real_size;
  u *first, *best, *ptr;

/* Round the block size up to a multiple of 4, then divide by four, and add
 * 1, to get the actual block size in ints, including room for the header
 */
  size = (size + 7) >> 2;

  first = best = 0;
  best_size = MASK;

/* If we are being forced to allocate a big block, ignore the fit attempts */

  if (force_more)
    ptr = 0;
  else {
    ptr = free_list;
#ifdef SLOW_STATISTICS
    large_free_chain.counter++;
#endif
  }

/* run through the free list, looking for a perfect, first, or best fit */

  while (ptr) {
    u tempsize;
#ifdef SLOW_STATISTICS
    large_free_chain.size++;
#endif
    /* Perfect fit? */
    tempsize = *ptr & MASK;
    if (tempsize == size) {
      best = first = ptr;
      break;			/* always accept perfect fit */
    }
/* If allocating this block for this malloc call would result in a hole
 * that's smaller than the minimum large block size, that is, smaller
 * than than SMALL_BLOCK_MAX, then it'd never get allocated.  So refuse
 * those cases.
 */
#ifndef WASTE_SMALL
    if (tempsize >= size + SMALL_BLOCK_MAX + 1) {
#else
    if (tempsize >= size) {
#endif
      if (!first) {
	first = ptr;
	if (fit_style == FIRST_FIT)
/* If we're using first fit, just take this one! */
	  break;
      }
/* Find out how well this one fits */

      tempsize -= size;
      if (tempsize > 0 && tempsize <= best_size) {
	best = ptr;
	best_size = tempsize;
      }
    }
    ptr = next_free_largeblock(ptr);

  }				/* end while */

  if (fit_style == BEST_FIT)
    ptr = best;
  else
    ptr = first;		/* Hybrid says use first if no perfect */

  if (!ptr) {

/* There was no match, or we're forced, so allocate more memory */

    u chunk_size, block_size;

    block_size = size * SINT;

    if (force_more || (block_size > CHUNK_SIZE))
      chunk_size = block_size;
    else
      chunk_size = CHUNK_SIZE;

    if (!start_next_block) {

/* This is the first allocation */

      start_next_block = (u *) esbrk(SINT);
      if (!start_next_block)
	return 0;

/* The first block header must mark the previous block as used */

      *(start_next_block) = PREV_BLOCK;

/* Initialize statistics */

      START(large_alloc_stat)
	START(large_total_stat)
	START(large_free_stat)
	START(sbrk_stat)
	START(small_alloc_stat)
	START(small_free_stat)
	START(small_total_stat)
	START(small_chunk_stat)
	START(large_free_chain)
    }
    ptr = (u *) esbrk(chunk_size);
    if (ptr == 0)
      return 0;

/* Old block at end of memory had an extra trailing word. Overwrite it. */

    ptr -= 1;

    block_size = chunk_size / SINT;

/* Configure the header information for this new bit. */

    build_block(ptr, block_size);

/* Set up the bogus header at the end of memory */

    *next_large_block(ptr) = THIS_BLOCK;

/* Stick this new block in the free list, so it's exactly like a real
 * free block, so it looks the same as one found in the free list
 */
    add_to_free_list(ptr);
  }
/* Ok, we got the free block of appropriate size from somewhere */

  remove_from_free_list(ptr);
  real_size = *ptr & MASK;

  if (real_size != size) {
    /* split block pointed to by ptr into two blocks */
    build_block(ptr + size, real_size - size);
    add_to_free_list(ptr + size);
    build_block(ptr, size);
  }
  mark_block(ptr);

/* Update statistics */

  COUNT(large_alloc_stat, size << 2);
  COUNT(large_total_stat, size << 2);

  return (char *) (ptr + 1);
}

static void
large_free(ptr)
    char *ptr;
{
  u size, *p;

/* Find the header again */

  p = (u *) ptr;
  p -= 1;

/* Update statistics */

  size = *p & MASK;
  COUNT(large_alloc_stat, -(int) (size << 2));

/* Since this is about to become a hole, check memory before and after
 * for holes.  If they are holes, merge them with this one.
 */
  if (is_next_free(p)) {
    remove_from_free_list(next_large_block(p));
    size += (*next_large_block(p) & MASK);
    *p = (*p & PREV_BLOCK) | size;
  }
  if (is_previous_free(p)) {
    remove_from_free_list(previous_largeblock(p));
    size += (*previous_largeblock(p) & MASK);
    p = previous_largeblock(p);
  }
  build_block(p, size);
  add_to_free_list(p);
}

void *
realloc(p, size)
    char *p;
    unsigned size;
{
  unsigned *q, old_size;
  char *t;
  q = ((unsigned *) p) - 1;
  old_size = ((*q & MASK) - 1) * SINT;

  if (old_size >= size)
    return p;

  t = malloc(size);
  if (t == 0)
    return (char *) 0;

  memcpy(t, p, old_size);
  free(p);
  return t;
}

int
resort_free_list()
{
  return 0;
}

#define d(str,stat) p(player, tprintf(str,stat.counter,stat.size))
#define p notify
#ifdef SLOW_STATISTICS
void
dump_malloc_data(player)
    dbref player;
{
  p(player, "TOTAL ALLOCATIONS  (all requests ever made)");
  p(player, " Type               Count      Space (bytes)");
  d(" large blocks:  %9d      %12ld", large_total_stat);
  d(" small blocks:  %9d      %12ld", small_total_stat);
  d(" sbrk requests:  %8d        %10ld (a)", sbrk_stat);
  if (large_free_chain.counter)
    p(player, tprintf(" large mallocs: %9d      average search length: %7.2f",
		      large_free_chain.counter,
	     (double) large_free_chain.size / large_free_chain.counter));
  p(player, "CURRENT USAGE");
  p(player, " Type               Count      Space (bytes)");
  d(" large blocks:   %8d        %10ld (b)", large_alloc_stat);
  d(" large holes:    %8d        %10ld (c)", large_free_stat);
  d(" small chunks:   %8d        %10ld (d)", small_chunk_stat);
  d(" small blocks:   %8d        %10ld (e)", small_alloc_stat);
  d(" small holes:    %8d        %10ld (f)", small_free_stat);
  p(player, tprintf(" unused from current chunk       %10d (g)",
		    unused_size << 2));
  p(player, "NOTES");
  p(player, "  Small blocks are stored in small chunks, which are allocated as");
  p(player, "large blocks.  Therefore, the total large blocks allocated (b) plus");
  p(player, "the large free blocks (c) should equal total storage from sbrk (a).");
  p(player, "Similarly, (e) + (f) + (g) equals (d).  The total amount of storage");
  p(player, "wasted is (c) + (f) + (g); the amount allocated is (b) - (f) - (g).");
}

#endif
#undef p
#undef d

/*
 * Modified by Lars Pensj| (??)
 * calloc() is provided because some stdio packages uses it.
 */
void *
calloc(nelem, sizel)
    unsigned nelem, sizel;
{
  char *p;

  if (nelem == 0 || sizel == 0)
    return 0;
  p = malloc(nelem * sizel);
  if (p == 0)
    return 0;
  (void) memset(p, 0, nelem * sizel);
  return p;
}

/*
 * Functions below can be used to debug malloc.
 */

#ifdef DEBUG

unsigned int
memused()
{
  return sbrk_stat.size;
}

/*
 * Verify that the free list is correct. The upper limit compared to
 * is very machine dependant.
 */
verify_sfltable(player)
    dbref player;
{
  u *p;
  int i, j;
  extern int end;

  if (!debugmalloc)
    return;
  if (unused_size > SMALL_CHUNK_SIZE / SINT) {
    fprintf(stderr, "ERROR: free list.  SMALL_CHUNK/SINT < unused");
    notify(player, "ERROR: free list.  SMALL_CHUNK/SINT < unused");
  }
  for (i = 0; i < SMALL_BLOCK_MAX; i++) {
    for (j = 0, p = sfltable[i]; p; p = *(u **) (p + 1), j++) {
      if (p < (u *) &end || p > (u *) 0xfffff) {
	fprintf(stderr, "ERROR: free list. pointer out of range.");
	notify(player, "ERROR: free list. pointer out of range.");
      }
      if (*p - 2 != i) {
	fprintf(stderr, "ERROR: free list. pointer corrupt.");
	notify(player, "ERROR: free list. pointer corrupt.");
      }
    }
    if (p >= next_unused && p < next_unused + unused_size) {
      notify(player, "ERROR: free list.  pointer in bad place.");
      fprintf(stderr, "ERROR: free list.  pointer in bad place.");
    }
  }
  p = free_list;
  while (p) {
    if (p >= next_unused && p < next_unused + unused_size) {
      notify(player, "ERROR: free list.  pointer in bad place.");
      fprintf(stderr, "ERROR: free list.  pointer in bad place.");
    }
    p = next_free_largeblock(p);
  }
}

verify_free(ptr)
    u *ptr;
{
  u *p;
  int i, j;

  if (!debugmalloc)
    return;
  for (i = 0; i < SMALL_BLOCK_MAX; i++) {
    for (j = 0, p = sfltable[i]; p; p = *(u **) (p + 1), j++) {
      if (*p - 2 != i)
	fprintf(stderr, "ERROR: error in free list");
      if (ptr >= p && ptr < p + *p)
	fprintf(stderr, "ERROR: error in free list");
      if (p >= ptr && p < ptr + *ptr)
	fprintf(stderr, "ERROR: error in free list");
      if (p >= next_unused && p < next_unused + unused_size)
	fprintf(stderr, "ERROR: error in free list");
    }
  }

  p = free_list;
  while (p) {
    if (ptr >= p && ptr < p + (*p & MASK))
      fprintf(stderr, "ERROR: error in free list");
    if (p >= ptr && p < ptr + (*ptr & MASK))
      fprintf(stderr, "ERROR: error in free list");
    if (p >= next_unused && p < next_unused + unused_size)
      fprintf(stderr, "ERROR: error in free list");
    p = next_free_largeblock(p);
  }
  if (ptr >= next_unused && ptr < next_unused + unused_size)
    fprintf(stderr, "ERROR: error in free list");
}
#endif				/* DEBUG */
#endif				/* USE_SMALLOC */
