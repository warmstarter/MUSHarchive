#ifndef _ATTRIB_H
#define _ATTRIB_H

/** An attribute on an object.
 * This structure represents an attribute set on an object.
 * Attributes form a linked list on an object, sorted alphabetically.
 */
struct attr {
  char const *name;		/**< Name of attribute */
  int flags;			/**< Attribute flags */
  unsigned char *value;		/**< The attribute's value, compressed */
  dbref creator;		/**< The attribute's creator's dbref */
  ATTR *next;			/**< Pointer to next attribute in list */
};


/* Stuff that's actually in atr_tab.c */
extern ATTR *aname_hash_lookup(const char *name);
extern int alias_attribute(const char *atr, const char *alias);
extern void do_attribute_access
  (dbref player, char *name, char *perms, int retroactive);
extern void do_attribute_delete(dbref player, char *name);
extern void do_attribute_rename(dbref player, char *old, char *newname);
extern void do_attribute_info(dbref player, char *name);
extern void do_list_attribs(dbref player, int lc);
extern char *list_attribs(void);

/* From attrib.c */
extern int good_atr_name(char const *s);
extern ATTR *atr_match(char const *string);
extern void atr_new_add(dbref thing, char const *RESTRICT atr,
			char const *RESTRICT s, dbref player, int flags);
extern int atr_add(dbref thing, char const *RESTRICT atr,
		   char const *RESTRICT s, dbref player, int flags);
extern int atr_clr(dbref thing, char const *atr, dbref player);
extern ATTR *atr_get(dbref thing, char const *atr);
extern ATTR *atr_get_noparent(dbref thing, char const *atr);
typedef int (*aig_func) (dbref, dbref, const char *, ATTR *, void *);
extern int atr_iter_get(dbref player, dbref thing, char const *name,
			aig_func func, void *args);
extern ATTR *atr_complete_match(dbref player, char const *atr, dbref privs);
extern void atr_free(dbref thing);
extern void atr_cpy(dbref dest, dbref source);
extern char const *convert_atr(int oldatr);
extern int atr_comm_match(dbref thing, dbref player, int type, int end,
			  char const *str, int just_match, char *atrname,
			  char **abp);
extern int do_set_atr
  (dbref thing, char const *RESTRICT atr, char const *RESTRICT s, dbref player,
   int flags);
extern void do_atrlock(dbref player, char const *arg1, char const *arg2);
extern void do_atrchown(dbref player, char const *arg1, char const *arg2);
extern int string_to_atrflag(dbref player, const char *p);
extern void init_atr_name_tree(void);


/* possible attribute flags */
#define AF_ODARK        0x1	/* OBSOLETE! Leave here but don't use */
#define AF_INTERNAL     0x2	/* no one can see it or set it */
#define AF_WIZARD       0x4	/* Wizard only can change it */
#define AF_NUKED        0x8	/* OBSOLETE! Leave here but don't use */
#define AF_LOCKED       0x10	/* Only creator of attrib can change it. */
#define AF_NOPROG       0x20	/* won't be searched for $ commands. */
#define AF_MDARK        0x40	/* Only wizards can see it */
#define AF_PRIVATE      0x80	/* Children don't inherit it */
#define AF_NOCOPY       0x100	/* atr_cpy (for @clone) doesn't copy it */
#define AF_VISUAL       0x200	/* Everyone can see this attribute */
#define AF_REGEXP       0x400	/* Match $/^ patterns using regexps */
#define AF_CASE         0x800	/* Match $/^ patterns case-sensitive */
#define AF_SAFE         0x1000	/* This attribute may not be modified */
#define AF_STATIC       0x10000	/* OBSOLETE! Leave here but don't use */
#define AF_COMMAND      0x20000	/* INTERNAL: value starts with $ */
#define AF_LISTEN       0x40000	/* INTERNAL: value starts with ^ */
#define AF_NODUMP       0x80000	/* INTERNAL: attribute is not saved */
#define AF_LISTED       0x100000	/* INTERNAL: Used in @list attribs */
#define AF_PREFIXMATCH  0x200000	/* Subject to prefix-matching */
#define AF_VEILED       0x400000	/* On ex, show presence, not value */

/* external predefined attributes. */
extern ATTR attr[];

#define AL_ATTR(alist)          (alist)
#define AL_NAME(alist)          ((alist)->name)
#define AL_STR(alist)           ((alist)->value)
#define AL_NEXT(alist)          ((alist)->next)
#define AL_CREATOR(alist)       ((alist)->creator)
#define AL_FLAGS(alist)         ((alist)->flags)

#endif				/* __ATTRIB_H */
