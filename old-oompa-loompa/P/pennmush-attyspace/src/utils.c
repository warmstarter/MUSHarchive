/* utils.c */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#include <limits.h>
#ifdef sgi
#include <math.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>


#include "conf.h"
#include "intrface.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "match.h"
#include "externs.h"
#include "mushdb.h"
#include "mymalloc.h"
#include "confmagic.h"


extern ATTR *atr_get _((dbref thing, const char *atr));
extern char *upcasestr _((char *s));

void parse_attrib _((dbref player, char *str, dbref *thing, ATTR **attrib));
dbref find_entrance _((dbref door));
dbref remove_first _((dbref first, dbref what));
int member _((dbref thing, dbref list));
int recursive_member _((dbref disallow, dbref from, int count));
dbref reverse _((dbref list));
struct dblist *listcreate _((dbref ref));
void listadd _((struct dblist * head, dbref ref));
void listfree _((struct dblist * head));
int getrandom _((int x));
Malloc_t mush_malloc _((int size, const char *check));
void mush_free _((Malloc_t ptr, const char *check));
char *shortname _((dbref it));

Malloc_t
mush_malloc(size, check)
    int size;
    const char *check;
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

void
mush_free(ptr, check)
    Malloc_t ptr;
    const char *check;
{
#ifdef MEM_CHECK
  del_check(check);
#endif
  free(ptr);
  ptr = NULL;
  return;
}


void
parse_attrib(player, str, thing, attrib)
    dbref player;
    char *str;
    dbref *thing;
    ATTR **attrib;
{
  /* takes a string which is of the format <obj>/<attr> or <attr>,
   * and returns the dbref of the object, and a pointer to the attribute.
   * If no object is specified, then the dbref returned is the player's.
   * str is destructively modified.
   */

  char *name;

  /* find the object */

  if ((name = (char *) index(str, '/')) != NULL) {
    *name++ = '\0';
    *thing = noisy_match_result(player, str, NOTYPE, MAT_EVERYTHING);
  } else {
    name = str;
    *thing = player;
  }

  /* find the attribute */
  *attrib = (ATTR *) atr_get(*thing, upcasestr(name));
}


dbref
find_entrance(door)
    dbref door;
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

/* remove the first occurence of what in list headed by first */
dbref
remove_first(first, what)
    dbref first;
    dbref what;
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

int
member(thing, list)
    dbref thing;
    dbref list;
{
  DOLIST(list, list) {
    if (list == thing)
      return 1;
  }

  return 0;
}


/* Return 1 if disallow is inside of from, i.e., if loc(disallow) = from,
 * or loc(loc(disallow)) = from, etc.
 * Actually, it's not recursive any more.
 */
int
recursive_member(disallow, from, count)
    dbref disallow;
    dbref from;
    int count;
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

dbref
reverse(list)
    dbref list;
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

/* takes a dbref and returns a pointer to the head of a dblist */
struct dblist *
listcreate(ref)
    dbref ref;
{
  struct dblist *ptr;

  ptr = (struct dblist *) mush_malloc((unsigned) sizeof(struct dblist), "dblist");
  if (!ptr)
    panic("Out of memory.");
  ptr->obj = ref;
  ptr->next = NULL;
  return (ptr);
}

/*
 * takes a pointer to a dblist and a dbref and adds the dbref to the
 * end of the list.
 */
void
listadd(head, ref)
    struct dblist *head;
    dbref ref;
{
  struct dblist *ptr = head;


  while (ptr->next)
    ptr = ptr->next;

  ptr->next = listcreate(ref);
}

/* takes a pointer to a dblist and recursively frees it */
void
listfree(head)
    struct dblist *head;
{
  struct dblist *ptra = head, *ptrb;

  while (ptra->next) {
    ptrb = ptra->next;
    mush_free((Malloc_t) ptra, "dblist");
    ptra = ptrb;
  }
}


int
getrandom(x)
    int x;
{
  /* In order to be perfectly anal about not introducing any further sources
   * of statistical bias, we're going to call random() until we get a number
   * less than the greatest representable multiple of x. We'll then return
   * n mod x.
   */
  long n;
  if (x <= 0)
    return -1;
  do {
    n = random();
  } while (LONG_MAX - n < LONG_MAX % x);
  /* N.B. This loop happens in randomized constant time, and pretty damn
   * fast randomized constant time too, since P(LONG_MAX - n < LONG_MAX % x)
   * < 0.5 for any x, so for any X, the average number of times we should
   * have to call random() is less than 2.
   */
  return (n % x);
}




/* Return an object's name, but for exits, return just the first
 * component. We expect a valid object.
 */
char *
shortname(it)
    dbref it;
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
