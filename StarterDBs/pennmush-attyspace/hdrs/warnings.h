#ifndef __WARNINGS_H
#define __WARNINGS_H

#ifdef USE_WARNINGS

/* Header file for MUSH warning system. Adapted from the MUSE @winhibit
 * system, which was ported to MUSH by Al Brown (Kalkin), and now
 * totally rewritten by Javelin
 */
typedef long int warn_type;

static int warning_lock_type _((dbref i, const struct boolexp * l));
static void complain _((dbref player, dbref i, const char *name,
			const char *desc));
static void ct_room _((dbref player, dbref i, warn_type flags));
static void ct_exit _((dbref player, dbref i, warn_type flags));
static void ct_player _((dbref player, dbref i, warn_type flags));
static void ct_thing _((dbref player, dbref i, warn_type flags));
static void check_topology_on _((dbref player, dbref i));
#endif

#endif				/* __WARNINGS_H */
