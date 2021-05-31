/*-----------------------------------------------------------------
 * Local stuff
 *
 * This file contains custom stuff, and some of the items here are
 * called from within PennMUSH at specific times.
 */

/* Here are some includes you're likely to need or want.
 */
#include "copyrite.h"
#include "config.h"
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include "externs.h"
#include "intrface.h"
#include "parse.h"
#include "htab.h"
#include "command.h"
#include "confmagic.h"

extern HASHTAB htab_reserved_aliases;
void reserve_aliases _((void));
void local_commands _((void));

/* Called during the command init sequence before any commands are
 * added (including local_commands, below). This is where you
 * want to reserve any strings that you *don't* want any command
 * to alias to (because you want to preserve it for matching exits
 * or globals)
 */
void
reserve_aliases()
{
  static char placeholder[2] = "x";
  /* Example: Don't alias any commands to cardinal directions.
   * Remove the #ifdef EXAMPLE and #endif to use this code
   */
#ifdef EXAMPLE
  hashadd("W", (void *) placeholder, &htab_reserved_aliases);
  hashadd("E", (void *) placeholder, &htab_reserved_aliases);
  hashadd("S", (void *) placeholder, &htab_reserved_aliases);
  hashadd("N", (void *) placeholder, &htab_reserved_aliases);
  hashadd("NE", (void *) placeholder, &htab_reserved_aliases);
  hashadd("SE", (void *) placeholder, &htab_reserved_aliases);
  hashadd("NW", (void *) placeholder, &htab_reserved_aliases);
  hashadd("SW", (void *) placeholder, &htab_reserved_aliases);
  hashadd("U", (void *) placeholder, &htab_reserved_aliases);
  hashadd("D", (void *) placeholder, &htab_reserved_aliases);
#endif
}

#ifdef EXAMPLE
COMMAND (cmd_local_silly) {
  if (SW_ISSET(sw, SWITCH_NOISY))
    notify(player, tprintf("Noisy silly with %s", arg_left));
  notify(player, tprintf("SillyCommand %s", arg_left));
}
#endif


/* Called during the command init sequence.
 * This is where you'd put calls to add_command to insert a local
 * command into the command hash table. Any command you add here
 * will be auto-aliased for you.
 */
void
local_commands()
{
#ifdef EXAMPLE
  command_add("@SILLY", CMD_T_ANY, 0, 0, 0, switchmask("NOISY NOEVAL"), cmd_local_silly);
#endif
}
