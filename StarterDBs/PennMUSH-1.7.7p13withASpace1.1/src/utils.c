/**
 * \file utils.c
 *
 * \brief Utility functions for PennMUSH.
 *
 *
 */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#include <limits.h>
#ifdef sgi
#include <math.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#ifdef I_SYS_STAT
#include <sys/stat.h>
#endif
#define I_FCNTL
#ifdef I_FCNTL
#include <fcntl.h>
#endif
#ifdef I_UNISTD
#include <unistd.h>
#endif
#ifdef WIN32
#include <wtypes.h>
#include <winbase.h>		/* For GetCurrentProcessId() */
#endif
#include "conf.h"

#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "match.h"
#include "externs.h"
#include "mushdb.h"
#include "mymalloc.h"
#include "log.h"
#include "flags.h"
#include "dbdefs.h"
#include "attrib.h"
#include "confmagic.h"

dbref find_entrance(dbref door);
void initialize_mt(void);
static unsigned long genrand_int32(void);
static long genrand_int31(void);
static void init_genrand(unsigned long);
static void init_by_array(unsigned long *, int);
extern int local_can_interact_first(dbref from, dbref to, int type);
extern int local_can_interact_last(dbref from, dbref to, int type);

/** A malloc wrapper that tracks type of allocation.
 * This should be used in preference to malloc() when possible,
 * to enable memory leak tracing with MEM_CHECK.
 * \param size bytes to allocate.
 * \param check string to label allocation with.
 * \return allocated block of memory or NULL.
 */
Malloc_t
mush_malloc(size_t size, const char *check)
{
  Malloc_t ptr;
#ifdef MEM_CHECK
  add_check(check);
#endif
  ptr = malloc(size);
  if (ptr == NULL)
    do_log(LT_ERR, 0, 0, "mush_malloc failed to malloc %d bytes for %s",
	   size, check);
  return ptr;
}

/** A free wrapper that tracks type of allocation.
 * If mush_malloc() gets the memory, mush_free() should free it
 * to enable memory leak tracing with MEM_CHECK.
 * \param ptr pointer to block of member to free.
 * \param check string to label allocation with.
 */
void
mush_free(Malloc_t RESTRICT ptr, const char *RESTRICT check
	  __attribute__ ((__unused__)))
{
#ifdef MEM_CHECK
  del_check(check);
#endif
  free(ptr);
  return;
}


/** Parse object/attribute strings into components.
 * This function takes a string which is of the format obj/attr or attr,
 * and returns the dbref of the object, and a pointer to the attribute.
 * If no object is specified, then the dbref returned is the player's.
 * str is destructively modified. This function is probably underused.
 * \param player the default object.
 * \param str the string to parse.
 * \param thing pointer to dbref of object parsed out of string.
 * \param attrib pointer to pointer to attribute structure retrieved.
 */
void
parse_attrib(dbref player, char *str, dbref *thing, ATTR **attrib)
{
  char *name;

  /* find the object */

  if ((name = strchr(str, '/')) != NULL) {
    *name++ = '\0';
    *thing = noisy_match_result(player, str, NOTYPE, MAT_EVERYTHING);
  } else {
    name = str;
    *thing = player;
  }

  /* find the attribute */
  *attrib = (ATTR *) atr_get(*thing, upcasestr(name));
}


/** Given an exit, find the room that is its source through brute force.
 * This is used in pathological cases where the exit's own source
 * element is invalid.
 * \param door dbref of exit to find source of.
 * \return dbref of exit's source room, or NOTHING.
 */
dbref
find_entrance(dbref door)
{
  dbref room;
  dbref thing;
  for (room = 0; room < db_top; room++)
    if (IsRoom(room)) {
      thing = Exits(room);
      while (thing != NOTHING) {
	if (thing == door)
	  return room;
	thing = Next(thing);
      }
    }
  return NOTHING;
}

/** Remove the first occurence of what in chain headed by first.
 * This works for contents and exit chains.
 * \param first dbref of first object in chain.
 * \param what dbref of object to remove from chain.
 * \return new head of chain.
 */
dbref
remove_first(dbref first, dbref what)
{
  dbref prev;
  /* special case if it's the first one */
  if (first == what) {
    return Next(first);
  } else {
    /* have to find it */
    DOLIST(prev, first) {
      if (Next(prev) == what) {
	Next(prev) = Next(what);
	return first;
      }
    }
    return first;
  }
}

/** Is an object on a chain?
 * \param thing object to look for.
 * \param list head of chain to search.
 * \retval 1 found thing on list.
 * \retval 0 did not find thing on list.
 */
int
member(dbref thing, dbref list)
{
  DOLIST(list, list) {
    if (list == thing)
      return 1;
  }

  return 0;
}


/** Is an object inside another, at any level of depth?
 * That is, we check if disallow is inside of from, i.e., if 
 * loc(disallow) = from, or loc(loc(disallow)) = from, etc., with a 
 * depth limit of 50.
 * Despite the name of this function, it's not recursive any more.
 * \param disallow interior object to check.
 * \param from check if disallow is inside of this object.
 * \param count depths of nesting checked so far.
 * \retval 1 disallow is inside of from.
 * \retval 0 disallow is not inside of from.
 */
int
recursive_member(dbref disallow, dbref from, int count)
{
  do {
    /* The end of the location chain. This is a room. */
    if (!GoodObject(disallow) || IsRoom(disallow))
      return 0;

    if (from == disallow)
      return 1;

    disallow = Location(disallow);
    count++;
  } while (count <= 50);

  return 1;
}

/** Is an object or its location unfindable?
 * \param thing object to check.
 * \retval 1 object or location is unfindable.
 * \retval 0 neither object nor location is unfindable.
 */
int
unfindable(dbref thing)
{
  int count = 0;
  do {
    if (!GoodObject(thing))
      return 0;
    if (Unfind(thing))
      return 1;
    if (IsRoom(thing))
      return 0;
    thing = Location(thing);
    count++;
  } while (count <= 50);
  return 0;
}


/** Reverse the order of a dbref chain.
 * \param list dbref at the head of the chain.
 * \return dbref at the head of the reversed chain.
 */
dbref
reverse(dbref list)
{
  dbref newlist;
  dbref rest;
  newlist = NOTHING;
  while (list != NOTHING) {
    rest = Next(list);
    PUSH(list, newlist);
    list = rest;
  }
  return newlist;
}


#define N 624

/* We use the Mersenne Twister PRNG. It's quite good as PRNGS go,
 * much better than the typical ones provided in system libc's.
 *
 * The following two functions are based on the reference implementation,
 * with changes in the seeding function to use /dev/urandom as a seed
 * if possible.
 *
 * The Mersenne Twister homepage is:
 *  http://www.math.keio.ac.jp/~matumoto/emt.html
 *
 * You can get the reference code there.
 */


/** Wrapper to choose a seed and initialize the Mersenne Twister PRNG. */
void
initialize_mt(void)
{
#ifdef HAS_DEV_URANDOM
  int fd;
  unsigned long buf[N];

  fd = open("/dev/urandom", O_RDONLY);
  if (fd >= 0) {
    int r = read(fd, buf, sizeof buf);
    close(fd);
    if (r <= 0) {
      do_rawlog(LT_ERR,
		"Couldn't read from /dev/urandom! Resorting to normal seeding method.");
    } else {
      do_rawlog(LT_ERR, "Seeded RNG from /dev/urandom");
      init_by_array(buf, r / sizeof(unsigned long));
      return;
    }
  } else
    do_rawlog(LT_ERR,
	      "Couldn't open /dev/urandom to seed random number generator. Resorting to normal seeding method.");

#endif
  /* Default seeder. Pick a seed that's fairly random */
#ifdef WIN32
  init_genrand(GetCurrentProcessId() | (time(NULL) << 16));
#else
  init_genrand(getpid() | (time(NULL) << 16));
#endif
}


/* A C-program for MT19937, with initialization improved 2002/1/26.*/
/* Coded by Takuji Nishimura and Makoto Matsumoto.                 */

/* Before using, initialize the state by using init_genrand(seed)  */
/* or init_by_array(init_key, key_length).                         */

/* This library is free software.                                  */
/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            */

/* Copyright (C) 1997, 2002 Makoto Matsumoto and Takuji Nishimura. */
/* Any feedback is very welcome.                                   */
/* http://www.math.keio.ac.jp/matumoto/emt.html                    */
/* email: matumoto@math.keio.ac.jp                                 */

/* Period parameters */
#define M 397
#define MATRIX_A 0x9908b0dfUL	/* constant vector a */
#define UPPER_MASK 0x80000000UL	/* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL	/* least significant r bits */

static unsigned long mt[N];	/* the array for the state vector  */
static int mti = N + 1;		/* mti==N+1 means mt[N] is not initialized */

/** initializes mt[N] with a seed.
 * \param a seed value.
 */
static void
init_genrand(unsigned long s)
{
  mt[0] = s & 0xffffffffUL;
  for (mti = 1; mti < N; mti++) {
    mt[mti] = (1812433253UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
    /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
    /* In the previous versions, MSBs of the seed affect   */
    /* only MSBs of the array mt[].                        */
    /* 2002/01/09 modified by Makoto Matsumoto             */
    mt[mti] &= 0xffffffffUL;
    /* for >32 bit machines */
  }
}

/** initialize by an array with array-length
 * \param init_key the array for initializing keys 
 * \param key_length the array's length 
 */
static void
init_by_array(unsigned long init_key[], int key_length)
{
  int i, j, k;
  init_genrand(19650218UL);
  i = 1;
  j = 0;
  k = (N > key_length ? N : key_length);
  for (; k; k--) {
    mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 30)) * 1664525UL))
      + init_key[j] + j;	/* non linear */
    mt[i] &= 0xffffffffUL;	/* for WORDSIZE > 32 machines */
    i++;
    j++;
    if (i >= N) {
      mt[0] = mt[N - 1];
      i = 1;
    }
    if (j >= key_length)
      j = 0;
  }
  for (k = N - 1; k; k--) {
    mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 30)) * 1566083941UL))
      - i;			/* non linear */
    mt[i] &= 0xffffffffUL;	/* for WORDSIZE > 32 machines */
    i++;
    if (i >= N) {
      mt[0] = mt[N - 1];
      i = 1;
    }
  }

  mt[0] = 0x80000000UL;		/* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0,0xffffffff]-interval */
static unsigned long
genrand_int32(void)
{
  unsigned long y;
  static unsigned long mag01[2] = { 0x0UL, MATRIX_A };
  /* mag01[x] = x * MATRIX_A  for x=0,1 */

  if (mti >= N) {		/* generate N words at one time */
    int kk;

    if (mti == N + 1)		/* if init_genrand() has not been called, */
      init_genrand(5489UL);	/* a default initial seed is used */

    for (kk = 0; kk < N - M; kk++) {
      y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
      mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    for (; kk < N - 1; kk++) {
      y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
      mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
    mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

    mti = 0;
  }

  y = mt[mti++];

  /* Tempering */
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  y ^= (y >> 18);

  return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
static long
genrand_int31(void)
{
  return (long) (genrand_int32() >> 1);
}

/** Get a uniform random long between low and high values, inclusive.
 * Based on MUX's RandomINT32()
 * \param low lower bound for random number.
 * \param high upper bound for random number.
 * \return random number between low and high, or 0 or -1 for error.
 */
long
get_random_long(long low, long high)
{
  unsigned long x, n, n_limit;

  /* Validate parameters */
  if (high < low) {
    return 0;
  } else if (high == low) {
    return low;
  }

  x = high - low;
  if (LONG_MAX < x) {
    return -1;
  }
  x++;

  /* We can now look for an random number on the interval [0,x-1].
     //

     // In order to be perfectly conservative about not introducing any
     // further sources of statistical bias, we're going to call getrand()
     // until we get a number less than the greatest representable
     // multiple of x. We'll then return n mod x.
     //
     // N.B. This loop happens in randomized constant time, and pretty
     // damn fast randomized constant time too, since
     //
     //      P(UINT32_MAX_VALUE - n < UINT32_MAX_VALUE % x) < 0.5, for any x.
     //
     // So even for the least desireable x, the average number of times
     // we will call getrand() is less than 2.
   */

  n_limit = ULONG_MAX - (ULONG_MAX % x);

  do {
    n = genrand_int31();
  } while (n >= n_limit);

  return low + (n % x);
}

/* Return an object's name, but for exits, return just the first
 * component. We expect a valid object.
 * \param it dbref of object.
 * \return object's short name.
 */
char *
shortname(dbref it)
{
  static char n[BUFFER_LEN];	/* STATIC */
  char *s;

  strncpy(n, Name(it), BUFFER_LEN - 1);
  n[BUFFER_LEN - 1] = '\0';
  if (IsExit(it)) {
    if ((s = strchr(n, ';')))
      *s = '\0';
  }
  return n;
}

/** Return the absolute room (outermost container) of an object.
 * Return  NOTHING if it's in an invalid object or in an invalid
 * location or AMBIGUOUS if there are too many containers.
 * \param it dbref of object.
 * \return absolute room of object, NOTHING, or AMBIGUOUS.
 */
dbref
absolute_room(dbref it)
{
  int rec = 0;
  dbref room;
  if (!GoodObject(it))
    return NOTHING;
  room = Location(it);
  if (!GoodObject(room))
    return NOTHING;
  while (!IsRoom(room)) {
    room = Location(room);
    rec++;
    if (rec > 20)
      return AMBIGUOUS;
  }
  return room;
}


/** Can one object interact with/perceive another in a given way?
 * This funtion checks to see if 'to' can perceive something from
 * 'from'. The types of interactions currently supported include:
 * INTERACT_SEE (will light rays from from reach to?), INTERACT_HEAR
 * (will sound from from reach to?), and INTERACT_MATCH (can 'to'
 * match the name of 'from').
 * \param from object of interaction.
 * \param to subject of interaction, attempting to interact with from.
 * \param type type of interaction.
 * \retval 1 to can interact with from in this way.
 * \retval 0 to can not interact with from in this way.
 */
int
can_interact(dbref from, dbref to, int type)
{
  int lci;

  /* This shouldn't even be checked for rooms and garbage, but we're
   * paranoid. Trying to stop interaction with yourself will not work 99%
   * of the time, so we don't allow it anyway. */
  if (IsGarbage(from) || IsGarbage(to))
    return 0;

  if ((from == to) || IsRoom(from) || IsRoom(to))
    return 1;

  /* This function can override standard checks! */
  lci = local_can_interact_first(from, to, type);
  if (lci != NOTHING)
    return lci;

  /* Standard checks */

  /* You can interact with the object you are in or any objects
   * you're holding.
   * You can interact with objects you control, but not
   * specifically the other way around
   */
  if ((from == Location(to)) || (to == Location(from)) || controls(to, from))
    return 1;

  lci = local_can_interact_last(from, to, type);
  if (lci != NOTHING)
    return lci;

  return 1;
}
