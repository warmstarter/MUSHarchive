/* htab.c - table hashing routines */

/* This code is ripped out of TinyMUSH 2.2. */
/* Minor tweaking to make in Penn-compatible by Trivian (xeno@mix.hive.no) */


#include "config.h"
#include "copyrite.h"
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include "conf.h"
#include "externs.h"
#include "intrface.h"
#include "htab.h"
#include "mymalloc.h"
#include "confmagic.h"

/* ---------------------------------------------------------------------------
 * hash_val: Compute hash value of a string for a hash table.
 */

int
hash_val(key, hashmask)
    const char *key;
    int hashmask;
{
  int hash = 0;
  const char *sp;

  /*
   * If the string pointer is null, return 0.  If not, add up the
   * numeric value of all the characters and return the sum,
   * modulo the size of the hash table.
   */

  if (!key || !*key)
    return 0;
  for (sp = key; *sp; sp++)
    hash = (hash << 5) + hash + *sp;
  return (hash & hashmask);
}

/* ----------------------------------------------------------------------
 * hash_getmask: Get hash mask for mask-style hashing.
 */

int
hash_getmask(size)
    int *size;
{
  int tsize;

  if (!size || !*size)
    return 0;

  /* Get next power-of-two >= size, return power-1 as the mask
   * for ANDing
   */

  for (tsize = 1; tsize < *size; tsize = tsize << 1) ;
  *size = tsize;
  return tsize - 1;
}

void
hash_init(htab, size)
    HASHTAB *htab;
    int size;
{
  htab->mask = get_hashmask(&size);
  htab->hashsize = size;
  htab->entries = 0;
  htab->buckets = NULL;
}

HASHENT *
hash_find(htab, key)
    HASHTAB *htab;
    const char *key;
{
  int hval;
  HASHENT *hptr;

  if (!htab->buckets)
    return NULL;

  hval = hash_val(key, htab->mask);
  for (hptr = htab->buckets[hval]; hptr != NULL; hptr = hptr->next) {
    if (strcmp(key, hptr->key) == 0) {
      return hptr;
    }
  }
  return NULL;
}

void *
hash_value(entry)
    HASHENT *entry;
{
  if (entry)
    return entry->data;
  else
    return NULL;
}

char *
hash_key(entry)
    HASHENT *entry;
{
  if (entry)
    return entry->key;
  else
    return NULL;
}

void
hash_resize(htab, size)
    HASHTAB *htab;
    int size;
{
  int i;
  HASHENT **oldarr;
  HASHENT **newarr;
  HASHENT *hent, *nent;
  int hval;
  int osize;
  int mask;

  /* We don't want hashes outside these limits */
  if ((size < (1 << 4)) || (size > (1 << 20)))
    return;

  /* Save the old data we need */
  osize = htab->hashsize;
  oldarr = htab->buckets;

  mask = htab->mask = get_hashmask(&size);

  if (size == htab->hashsize)
    return;

  htab->hashsize = size;
  newarr = (HASHENT **) mush_malloc(size * sizeof(struct hashentry *), "hash_buckets");
  htab->buckets = newarr;
  for (i = 0; i < size; i++)
    newarr[i] = NULL;

  for (i = 0; i < osize; i++) {
    hent = oldarr[i];
    while (hent) {
      nent = hent->next;
      hval = hash_val(hent->key, mask);
      hent->next = newarr[hval];
      newarr[hval] = hent;
      hent = nent;
    }
  }
  mush_free(oldarr, "hash_buckets");

  return;
}

HASHENT *
hash_new(htab, key)
    HASHTAB *htab;
    const char *key;
{
  int hval, i;
  HASHENT *hptr;

  if (!htab->buckets) {
    htab->buckets = (HASHENT **) mush_malloc(htab->hashsize * sizeof(HASHENT *), "hash_buckets");
    for (i = 0; i < htab->hashsize; i++)
      htab->buckets[i] = NULL;
  } else {
    hptr = hash_find(htab, key);
    if (hptr)
      return hptr;
  }

  if (htab->entries > (htab->hashsize * HTAB_UPSCALE))
    hash_resize(htab, htab->hashsize << 1);

  hval = hash_val(key, htab->mask);
  htab->entries++;
  hptr = (HASHENT *) mush_malloc(HASHENT_SIZE + strlen(key) + 1, "hash_entry");
  strcpy(hptr->key, key);
  hptr->data = NULL;
  hptr->next = htab->buckets[hval];
  htab->buckets[hval] = hptr;
  return hptr;
}

int
hash_add(htab, key, hashdata)
    HASHTAB *htab;
    const char *key;
    void *hashdata;
{
  HASHENT *hptr;

  if (hash_find(htab, key) != NULL) {
    return -1;
  }
  hptr = hash_new(htab, key);

  if (!hptr)
    return -1;

  hptr->data = hashdata;
  return 0;
}

void
hash_delete(htab, entry)
    HASHTAB *htab;
    HASHENT *entry;
{
  int hval;
  HASHENT *hptr, *last;

  if (!entry)
    return;

  hval = hash_val(entry->key, htab->mask);
  last = NULL;
  for (hptr = htab->buckets[hval]; hptr; last = hptr, hptr = hptr->next) {
    if (entry == hptr) {
      if (last == NULL)
	htab->buckets[hval] = hptr->next;
      else
	last->next = hptr->next;
      mush_free(hptr, "hash_entry");
      htab->entries--;
      return;
    }
  }

  if (htab->entries < (htab->hashsize * HTAB_DOWNSCALE))
    hash_resize(htab, htab->hashsize >> 1);
}

void
hash_flush(htab, size)
    HASHTAB *htab;
    int size;
{
  HASHENT *hent, *thent;
  int i;

  if (htab->buckets) {
    for (i = 0; i < htab->hashsize; i++) {
      hent = htab->buckets[i];
      while (hent != NULL) {
	thent = hent;
	hent = hent->next;
	mush_free(thent, "hash_entry");
      }
      htab->buckets[i] = NULL;
    }
  }
  if (size == 0) {
    mush_free(htab->buckets, "hash_buckets");
    htab->buckets = NULL;
  } else if (size != htab->hashsize) {
    if (htab->buckets)
      mush_free(htab->buckets, "hash_buckets");
    hashinit(htab, size);
  } else {
    htab->entries = 0;
  }
}

void *
hash_firstentry(htab)
    HASHTAB *htab;
{
  int hval;

  for (hval = 0; hval < htab->hashsize; hval++)
    if (htab->buckets[hval]) {
      htab->last_hval = hval;
      htab->last_entry = htab->buckets[hval];
      return htab->buckets[hval]->data;
    }
  return NULL;
}

void *
hash_nextentry(htab)
    HASHTAB *htab;
{
  int hval;
  HASHENT *hptr;

  hval = htab->last_hval;
  hptr = htab->last_entry;
  if (hptr->next) {
    htab->last_entry = hptr->next;
    return hptr->next->data;
  }
  hval++;
  while (hval < htab->hashsize) {
    if (htab->buckets[hval]) {
      htab->last_hval = hval;
      htab->last_entry = htab->buckets[hval];
      return htab->buckets[hval]->data;
    }
    hval++;
  }
  return NULL;
}
