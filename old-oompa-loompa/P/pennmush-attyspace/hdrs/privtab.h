/* privtab.h */
/* Defines a privilege table entry for general use */

#ifndef __PRIVTAB_H
#define __PRIVTAB_H

#include "copyrite.h"
#include "config.h"
#include "confmagic.h"


typedef struct priv_info PRIV;
struct priv_info {
  const char *name;
  char letter;
  long int bits_to_set;
  long int bits_to_show;
};

#define PrivName(x)	((x)->name)
#define PrivChar(x)	((x)->letter)
#define PrivSetBits(x)	((x)->bits_to_set)
#define PrivShowBits(x)	((x)->bits_to_show)

extern int string_to_privs _((PRIV * table, const char *str, long int origprivs));
extern int letter_to_privs _((PRIV * table, const char *str, long int origprivs));
extern const char *privs_to_string _((PRIV * table, int privs));
extern const char *privs_to_letters _((PRIV * table, int privs));

#endif				/* __PRIVTAB_H */
