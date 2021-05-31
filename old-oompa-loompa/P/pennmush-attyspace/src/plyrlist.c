
/* plyrlist.c */
#include "config.h"

#include <ctype.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#include "copyrite.h"

#include "conf.h"
#include "mushdb.h"
#include "intrface.h"
#include "globals.h"
#include "externs.h"
#include "htab.h"
#include "confmagic.h"


HASHTAB htab_player_list;

static int hft_initialized = 0;
static void init_hft _((void));
void clear_players _((void));
void add_player _((dbref player, char *alias));
void delete_player _((dbref player, char *alias));

static void
init_hft()
{
  hashinit(&htab_player_list, 256);
  hft_initialized = 1;
}

void
clear_players()
{
  hashflush(&htab_player_list, 256);
}


void
add_player(player, alias)
    dbref player;
    char *alias;
{
  if (!hft_initialized)
    init_hft();
  if (alias)
    hashadd(strupper(alias), (void *) player, &htab_player_list);
  else
    hashadd(strupper(Name(player)), (void *) player, &htab_player_list);
}

dbref
lookup_player(name)
    const char *name;
{
  int p;
  void *hval;

  if (*name == NUMBER_TOKEN) {
    name++;
    if (!is_strict_number(name))
      return NOTHING;
    p = atoi(name);
    if (!GoodObject(p))
      return NOTHING;
    return ((Typeof(p) == TYPE_PLAYER) ? p : NOTHING);
  }
  hval = hashfind(strupper(name), &htab_player_list);
  if (!hval)
    return NOTHING;
  return (dbref) hval;
  /* By the way, there's a flaw in this code. If #0 was a player, we'd
   * hash its name with a dbref of (void *)0, aka NULL, so we'd never
   * be able to retrieve that player. However, we assume that #0 will
   * always be the base room, and never a player, so that's ok.
   */
}

void
delete_player(player, alias)
    dbref player;
    char *alias;
{
  if (alias)
    hashdelete(strupper(alias), &htab_player_list);
  else
    hashdelete(strupper(Name(player)), &htab_player_list);
}
