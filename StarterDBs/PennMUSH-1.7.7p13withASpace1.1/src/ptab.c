/**
 * \file ptab.c
 *
 * \brief Prefix tables for PennMUSH.
 *
 *
 */
#include "config.h"
#include "copyrite.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "externs.h"
#include "ptab.h"
#include "memcheck.h"
#include "confmagic.h"

static int ptab_find_exact_nun(PTAB *tab, const char *key);
static int WIN32_CDECL ptab_cmp(const void *, const void *);

/** A ptab entry. */
typedef struct ptab_entry {
  void *data;			/**< pointer to data */
  char key[BUFFER_LEN];		/**< the index key */
} ptab_entry;

/** The memory usage of a ptab entry, not including data. */
#define PTAB_SIZE (sizeof(struct ptab_entry) - BUFFER_LEN)

/** Initialize a ptab.
 * \param tab pointer to a ptab.
 */
void
ptab_init(PTAB *tab)
{
  if (!tab)
    return;
  tab->state = tab->maxlen = tab->len = tab->current = 0;
  tab->tab = NULL;
}

/** Free all entries in a ptab.
 * \param tab pointer to a ptab.
 */
void
ptab_free(PTAB *tab)
{
  if (!tab)
    return;
  if (tab->tab) {
    int n;
    for (n = 0; n < tab->len; n++)
      mush_free(tab->tab[n], "ptab.entry");
    mush_free(tab->tab, "ptab");
  }
  tab->tab = NULL;
  tab->state = tab->maxlen = tab->len = tab->current = 0;
}

/** Search a ptab for an entry that prefix-matches a given key.
 * We search through unique prefixes of keys in the table to try
 * to match the key we're looking for.
 * \param tab pointer to a ptab.
 * \param key key to search for.
 * \return void pointer to ptab data indexed by key, or NULL if none.
 */
void *
ptab_find(PTAB *tab, const char *key)
{
  int nun;

  if (!tab || !key || !*key || tab->state)
    return NULL;

  if (tab->len < 10) {		/* Just do a linear search for small tables */
    for (nun = 0; nun < tab->len; nun++) {
      if (string_prefix(tab->tab[nun]->key, key)) {
	if (nun + 1 < tab->len && string_prefix(tab->tab[nun + 1]->key, key))
	  return NULL;
	else
	  return tab->tab[nun]->data;
      }
    }
  } else {			/* Binary search of the index */
    int left = 0;
    int cmp;
    int right = tab->len - 1;

    while (1) {
      nun = (left + right) / 2;

      if (left > right)
	break;

      cmp = strcasecmp(key, tab->tab[nun]->key);

      if (cmp == 0) {
	return tab->tab[nun]->data;
      } else if (cmp < 0) {
	int mem;
	/* We need to catch the first unique prefix */
	if (string_prefix(tab->tab[nun]->key, key)) {
	  for (mem = nun - 1; mem >= 0; mem--) {
	    if (string_prefix(tab->tab[mem]->key, key)) {
	      if (strcasecmp(tab->tab[mem]->key, key) == 0)
		return tab->tab[mem]->data;
	    } else
	      break;
	  }
	  /* Non-unique prefix */
	  if (mem != nun - 1)
	    return NULL;
	  for (mem = nun + 1; mem < tab->len; mem++) {
	    if (string_prefix(tab->tab[mem]->key, key)) {
	      if (strcasecmp(tab->tab[mem]->key, key) == 0)
		return tab->tab[mem]->data;
	    } else
	      break;
	  }
	  if (mem != nun + 1)
	    return NULL;
	  return tab->tab[nun]->data;
	}
	if (left == right)
	  break;
	right = nun - 1;
      } else {			/* cmp > 0 */
	if (left == right)
	  break;
	left = nun + 1;
      }
    }
  }
  return NULL;
}

/** Search a ptab for an entry that exactly matches a given key.
 * We search through full names of keys in the table to try
 * to match the key we're looking for.
 * \param tab pointer to a ptab.
 * \param key key to search for.
 * \return void pointer to ptab data indexed by key, or NULL if none.
 */
void *
ptab_find_exact(PTAB *tab, const char *key)
{
  int nun;
  nun = ptab_find_exact_nun(tab, key);
  if (nun < 0)
    return NULL;
  else
    return tab->tab[nun]->data;
}

static int
ptab_find_exact_nun(PTAB *tab, const char *key)
{
  int nun;

  if (!tab || !key || tab->state)
    return -1;

  if (tab->len < 10) {		/* Just do a linear search for small tables */
    int cmp;
    for (nun = 0; nun < tab->len; nun++) {
      cmp = strcasecmp(tab->tab[nun]->key, key);
      if (cmp == 0)
	return nun;
      else if (cmp > 0)
	return -1;
    }
  } else {			/* Binary search of the index */
    int left = 0;
    int cmp;
    int right = tab->len - 1;

    while (1) {
      nun = (left + right) / 2;

      if (left > right)
	break;

      cmp = strcasecmp(key, tab->tab[nun]->key);

      if (cmp == 0)
	return nun;
      if (left == right)
	break;
      if (cmp < 0)
	right = nun - 1;
      else			/* cmp > 0 */
	left = nun + 1;
    }
  }
  return -1;
}

static void
delete_entry(PTAB *tab, int n)
{
  mush_free(tab->tab[n], "ptab.entry");

  /* If we're deleting the last item in the list, just decrement the length.
   *  Otherwise, we have to fill in the hole
   */
  if (n < tab->len - 1) {
    int i;
    for (i = n + 1; i < tab->len; i++)
      tab->tab[i - 1] = tab->tab[i];
  }
  tab->len--;
}

/** Delete a ptab entry indexed by key.
 * \param tab pointer to a ptab.
 * \param key key to search for.
 */
void
ptab_delete(PTAB *tab, const char *key)
{
  int nun;
  nun = ptab_find_exact_nun(tab, key);
  if (nun >= 0)
    delete_entry(tab, nun);
  return;
}

/** Put a ptab into insertion state.
 * \param tab pointer to a ptab.
 */
void
ptab_start_inserts(PTAB *tab)
{
  if (!tab)
    return;
  tab->state = 1;
}

static int WIN32_CDECL
ptab_cmp(const void *a, const void *b)
{
  const struct ptab_entry *const *ra = a;
  const struct ptab_entry *const *rb = b;

  return strcasecmp((*ra)->key, (*rb)->key);
}

/** Complete the ptab insertion process by re-sorting the entries.
 * \param tab pointer to a ptab.
 */
void
ptab_end_inserts(PTAB *tab)
{
  struct ptab_entry **tmp;
  if (!tab)
    return;
  tab->state = 0;
  qsort(tab->tab, tab->len, sizeof(struct ptab_entry *), ptab_cmp);

  tmp = realloc(tab->tab, (tab->len + 10) * sizeof(struct ptab_entry *));
  if (!tmp)
    return;
  tab->tab = tmp;
  tab->maxlen = tab->len + 10;
}

/** Insert an entry into a ptab.
 * \param tab pointer to a ptab.
 * \param key key to insert entry under.
 * \param data pointer to entry data.
 */
void
ptab_insert(PTAB *tab, const char *key, void *data)
{
  int lamed;

  if (!tab || tab->state != 1)
    return;

  if (tab->len == tab->maxlen) {
    struct ptab_entry **tmp;
    if (tab->maxlen == 0)
      tab->maxlen = 200;
    else
      tab->maxlen *= 2;
    tmp = realloc(tab->tab, tab->maxlen * sizeof(struct ptab_entry **));
#ifdef MEM_CHECK
    if (tab->tab == NULL)
      add_check("ptab");
#endif
    if (!tmp)
      return;
    tab->tab = tmp;
  }

  lamed = strlen(key) + 1;

  tab->tab[tab->len] = mush_malloc(PTAB_SIZE + lamed, "ptab.entry");

  tab->tab[tab->len]->data = data;
  memcpy(tab->tab[tab->len]->key, key, lamed);
  tab->len++;

}

/** Return the data (and optionally the key) of the first entry in a ptab.
 * This function resets the 'current' index in the ptab to the start
 * of the table.
 * \param tab pointer to a ptab.
 * \param key memory location to store first key unless NULL is passed in.
 * \return void pointer to data from first entry, or NULL if none.
 */
void *
ptab_firstentry_new(PTAB *tab, char *key)
{
  if (!tab || tab->len == 0)
    return NULL;
  tab->current = 1;
  if (key)
    strcpy(key, tab->tab[0]->key);
  return tab->tab[0]->data;
}

/** Return the data (and optionally the key) of the next entry in a ptab.
 * This function increments the 'current' index in the ptab.
 * \param tab pointer to a ptab.
 * \param key memory location to store next key unless NULL is passed in.
 * \return void pointer to data from next entry, or NULL if none.
 */
void *
ptab_nextentry_new(PTAB *tab, char *key)
{
  if (!tab || tab->current >= tab->len)
    return NULL;
  if (key)
    strcpy(key, tab->tab[tab->current]->key);
  return tab->tab[tab->current++]->data;
}

/** Header for report of ptab stats.
 * \param player player to notify with table header.
 */
void
ptab_stats_header(dbref player)
{
  notify_format(player, "Table      Entries AvgComparisons %39s", "~Memory");
}

/** Data for one line of report of ptab stats.
 * \param player player to notify with table.
 * \param tab pointer to ptab to summarize stats for.
 * \param pname name of ptab, for row header.
 */
void
ptab_stats(dbref player, PTAB *tab, const char *pname)
{
  int mem, nun;

  mem = sizeof(struct ptab_entry *) * tab->maxlen;

  for (nun = 0; nun < tab->len; nun++)
    mem += PTAB_SIZE + strlen(tab->tab[nun]->key) + 1;

  notify_format(player, "%-10s %7d %14.3f %39d", pname, tab->len, log(tab->len),
		mem);
}
