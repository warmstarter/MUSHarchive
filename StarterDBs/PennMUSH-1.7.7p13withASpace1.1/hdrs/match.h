/* match.h */

#ifndef __MATCH_H
#define __MATCH_H

#include "copyrite.h"

/* match constants */
  /* match modifiers */
#define MAT_CHECK_KEYS   0x000001
#define MAT_GLOBAL       0x000002
#define MAT_REMOTES      0x000004
#define MAT_CONTROL      0x000008
  /* individual things to match */
#define MAT_ME           0x000010
#define MAT_HERE         0x000020
#define MAT_ABSOLUTE     0x000040
#define MAT_PLAYER       0x000080
#define MAT_NEIGHBOR     0x000100
#define MAT_POSSESSION   0x000200
#define MAT_EXIT         0x004000
  /* special things to match */
#define MAT_CARRIED_EXIT     0x010000
#define MAT_CONTAINER        0x020000
#define MAT_REMOTE_CONTENTS  0x040000
#define MAT_NEAR             0x080000
#define MAT_ENGLISH          0x100000
  /* types of match results - used internally */
#define MAT_NOISY                0x1000000
#define MAT_LAST                 0x2000000
  /* groups of things to match */
#define MAT_EVERYTHING   (MAT_ME|MAT_HERE|MAT_ABSOLUTE|MAT_PLAYER| \
                          MAT_NEIGHBOR|MAT_POSSESSION|MAT_EXIT|MAT_ENGLISH)
#define MAT_NEARBY       (MAT_EVERYTHING|MAT_NEAR)
#define MAT_OBJECTS      (MAT_ME|MAT_ABSOLUTE|MAT_PLAYER|MAT_NEIGHBOR| \
                          MAT_POSSESSION)
#define MAT_NEAR_THINGS  (MAT_OBJECTS|MAT_NEAR)
#define MAT_LIMITED      (MAT_ABSOLUTE|MAT_PLAYER|MAT_NEIGHBOR)
#define MAT_REMOTE       (MAT_ABSOLUTE|MAT_PLAYER|MAT_REMOTE_CONTENTS|MAT_EXIT)


/* Functions we can call */
/* These functions do the matching and return the result:
 * match_result - returns match, NOTHING, or AMBIGUOUS
 * noisy_match_result - notifies player, returns match or NOTHING
 * last_match_result - returns a match or NOTHING
 * match_controlled - returns match if player controls, or NOTHING
 */
extern dbref match_result
  (const dbref who, const char *name, const int type, const long int flags);
extern dbref noisy_match_result
  (const dbref who, const char *name, const int type, const long int flags);
extern dbref last_match_result
  (const dbref who, const char *name, const int type, const long int flags);
extern dbref match_controlled(dbref player, const char *name);

#define match_thing(player,name) \
  noisy_match_result((player), (name), NOTYPE, MAT_EVERYTHING)

#endif				/* __MATCH_H */
