/* htab.h - Structures and declarations needed for table hashing */

#ifndef __HTAB_H
#define __HTAB_H

#include "config.h"
#include "mushdb.h"
#include "confmagic.h"

#define SOME_KEY_LEN 10

#define HTAB_UPSCALE   4
#define HTAB_DOWNSCALE 2

typedef struct hashentry HASHENT;
struct hashentry {
  struct hashentry *next;
  void *data;
  char key[SOME_KEY_LEN];
};

#define HASHENT_SIZE (sizeof(HASHENT)-SOME_KEY_LEN)

typedef struct hashtable HASHTAB;
struct hashtable {
  int hashsize;
  int mask;
  int entries;
  HASHENT **buckets;
  int last_hval;		/* Used for hashfirst & hashnext. */
  HASHENT *last_entry;		/* like last_hval */
};

#define hashval(x,y) hash_val(x,y)
#define get_hashmask(x) hash_getmask(x)
#define hashinit(x,y) hash_init(x,y)
#define hashfind(key,tab) hash_value(hash_find(tab,key))
#define hashadd(key,data,tab) hash_add(tab,key,data)
#define hashdelete(key,tab) hash_delete(tab,hash_find(tab,key))
#define hashflush(tab, size) hash_flush(tab,size)
#define hashfree(tab) hash_flush(tab, 0)
extern int hash_val _((const char *key, int hashmask));
extern int hash_getmask _((int *size));
extern void hash_init _((HASHTAB * htab, int size));
extern HASHENT *hash_find _((HASHTAB * htab, const char *key));
extern void *hash_value _((HASHENT * entry));
extern char *hash_key _((HASHENT * entry));
extern void hash_resize _((HASHTAB * htab, int size));
extern HASHENT *hash_new _((HASHTAB * htab, const char *key));
extern int hash_add _((HASHTAB * htab, const char *key, void *hashdata));
extern void hash_delete _((HASHTAB * htab, HASHENT * entry));
extern void hash_flush _((HASHTAB * htab, int size));
extern void *hash_firstentry _((HASHTAB * htab));
extern void *hash_nextentry _((HASHTAB * htab));

#endif
