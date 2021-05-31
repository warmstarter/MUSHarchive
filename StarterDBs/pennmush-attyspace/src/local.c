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

/* Called after all MUSH init is done.
 */
void
local_startup()
{
}

/* Called when the database will be saved
 * This is called JUST before we dump the
 * database to disk
 * Use to save any in-memory structures
 * back to disk
 */
void
local_dump_database()
{
#ifdef TREKMUSH
  (void) dump_space(GOD);
#endif /* TREKMUSH */
}

/* Called when the MUSH is shutting down.
 * The DB has been saved and descriptors closed
 * The log files are still open though.
 */
void
local_shutdown()
{
}

/* This is called exactly once a second
 * After the MUSH has done all it's stuff
 */
void
local_timer()
{
#ifdef TREKMUSH
  (void) do_space_db_iterate();
#endif /* TREKMUSH */
}

#ifdef LOCAL_DATA
/* For serious hackers only */

/* Those who are depraved enough to do so (Like me), can always 
 * abuse this as a new and better way of Always Doing Stuff
 * to objects.
 * Like, say you want to put out a message on the wizard
 * channel every time an object is destroyed, do so in the
 * local_data_destroy() routine.
 */

/* Called when a object is created with @create (or @dig, @link) 
 * This is done AFTER object-specific setup, so the types
 * etc will already be set, and object-specific initialization
 * will be done.
 * Note that the game will ALWAYS set the LocData to NULL before
 * this routine is called.
 */

/* For a well-commented example of how to use this code,
 * see: ftp://bimbo.hive.no/pub/PennMUSH/coins.tar.gz
 */

void
local_data_create(object)
    dbref object;
{
}

/* Called when an object is cloned. Since clone is a rather
 * specific form of creation, it has it's own function.
 * Note that local_data_create() is NOT called for this object
 * first, but the system will always set LocData to NULL first.
 * Clone is the 'new' object, while source is the one it's
 * being copied from.
 */

void
local_data_clone(clone, source)
    dbref clone;
    dbref source;
{
}

/* Called when a object is REALLY destroyed, not just set
 * Going.
 */

void
local_data_free(object)
    dbref object;
{
}

/* Initiation of objects after a reload or dumping to disk should
 * be handled in local_dump_database() and local_startup().
 */

#endif
