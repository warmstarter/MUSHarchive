/* malias.h - header file for global mailing aliases/lists */

#ifndef _MALIAS_H
#define _MALIAS_H

#ifdef MAIL_ALIASES

#define ALIAS_MEMBERS	0x1	/* Only those on the alias */
#define ALIAS_ADMIN	0x2	/* Only admin/powered */
#define ALIAS_OWNER	0x4	/* Only the owner */

struct mail_alias {
  char *name;
  char *desc;			/* Description */
  int size;			/* Size of the members array */
  dbref **members;		/* Pointer to an array of dbrefs */
  int nflags;			/* Who can use/see alias name */
  int mflags;			/* Who can list alias members */
  dbref owner;			/* Who controls this alias */
};

#endif

#endif
