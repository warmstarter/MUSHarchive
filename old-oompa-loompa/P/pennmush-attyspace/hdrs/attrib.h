#ifndef _ATTRIB_H
#define _ATTRIB_H

typedef int dbref;		/* offset into db */

/* new attribute foo */
typedef struct attr ATTR;

/* the attribute structure */
struct attr {
  char const *name;		/* name of attribute */
  int flags;
  unsigned char *value;
  dbref creator;
  ATTR *next;
};

struct boolatr {
  char *name;			/* which attribute? */
  unsigned char *text;
};

/* possible attribute flags */
#define AF_ODARK	0x1	/* OBSOLETE! Leave here but don't use */
#define AF_INTERNAL	0x2	/* no one can see it or set it */
#define AF_WIZARD	0x4	/* Wizard only can change it */
#define AF_NUKED	0x8	/* OBSOLETE! Leave here but don't use */
#define AF_LOCKED	0x10	/* Only creator of attrib can change it. */
#define AF_NOPROG	0x20	/* won't be searched for $ commands. */
#define AF_MDARK        0x40	/* Only wizards can see it */
#define AF_PRIVATE      0x80	/* Children don't inherit it */
#define AF_NOCOPY       0x100	/* atr_cpy (for @clone) doesn't copy it */
#define AF_VISUAL	0x200	/* Everyone can see this attribute */
#define AF_REGEXP	0x400	/* Match $/^ patterns using regexps */
#define AF_CASE		0x800	/* Match $/^ patterns case-sensitive */
#define AF_STATIC	0x10000	/* INTERNAL: name statically allocated */
#define AF_COMMAND	0x20000	/* INTERNAL: value starts with $ */
#define AF_LISTEN	0x40000	/* INTERNAL: value starts with ^ */

/* external predefined attributes. */
extern ATTR attr[];

/* easy access macros for attributes */
#ifdef NEVER
#define s_Osucc(thing,s) atr_add(thing, "OSUCCESS", (s), (thing), NOTHING)
#define s_Ofail(thing,s) atr_add(thing, "OFAILURE", (s), (thing), NOTHING)
#define s_Fail(thing,s) atr_add(thing, "FAILURE", (s), (thing), NOTHING)
#define s_Succ(thing,s) atr_add(thing, "SUCCESS", (s), (thing), NOTHING)
#define s_Pass(thing,s) atr_add(thing, "XYXXY", (s), GOD, NOTHING)
#define s_Desc(thing,s) atr_add(thing, "DESCRIBE", (s), (thing), NOTHING)

#define Astr(attrib) ((attrib)->value)
#endif

typedef ATTR ALIST;

#define AL_ATTR(alist)		(alist)
#define AL_NAME(alist)		((alist)->name)
#define AL_STR(alist)		((alist)->value)
#define AL_NEXT(alist)		((alist)->next)
#define AL_CREATOR(alist)	((alist)->creator)
#define AL_FLAGS(alist)		((alist)->flags)

#endif				/* __ATTRIB_H */
