/* lock.h */

#include "copyrite.h"

#ifndef __LOCK_H
#define __LOCK_H

#include "conf.h"
#include "dbdefs.h"

/* I'm using a string for a lock type instead of a magic-cookie int for
 * several reasons:
 * 1) I don't think it will hurt efficiency that much. I'll profile it
 * to check.
 * 2) It will make debugging much easier to see lock types that can easily
 * be interpreted by a human.
 * 3) It allows the possibility of having arbitrary user-defined locks.
 */

typedef const char *lock_type;

typedef struct lock_list lock_list;
struct lock_list {
  lock_type type;
  struct boolexp *key;
  struct lock_list *next;
};



/* The actual magic cookies. */
extern const lock_type Basic_Lock;
extern const lock_type Enter_Lock;
extern const lock_type Use_Lock;
extern const lock_type Zone_Lock;
extern const lock_type Page_Lock;
extern const lock_type Tport_Lock;
extern const lock_type Speech_Lock;	/* Who can speak aloud in me */
extern const lock_type Listen_Lock;	/* Who can trigger ^s/ahears on me */
extern const lock_type Parent_Lock;	/* Who can @parent to me */
extern const lock_type Link_Lock;	/* Who can @link to me */
extern const lock_type Leave_Lock;	/* Who can leave me */
extern const lock_type Drop_Lock;	/* Who can drop me */
extern const lock_type Give_Lock;	/* Who can give me */
extern const lock_type Mail_Lock;	/* Who can @mail me */
/* Declare new lock types here! */
/* OFFLINE #ifdef TREKMUSH
extern const lock_type DBRead_Lock;     Who can read an offline DB
extern const lock_type DBWrite_Lock;    Who can write to an offline DB
#endif */


/* For the sake of a quick proof-of-concept, we redefine these macros
 * for simplicity.
 */
#define Key(x)      (getlock((x), Basic_Lock))
#define Enterkey(x) (getlock((x), Enter_Lock))
#define Usekey(x)   (getlock((x), Use_Lock))

#endif				/* __LOCK_H */
