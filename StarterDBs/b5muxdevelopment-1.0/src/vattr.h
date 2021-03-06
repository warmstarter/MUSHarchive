/*
 *  Definitions for user-defined attributes
 *
 *  $Id: vattr.h,v 1.1 2001/01/15 03:22:20 wenk Exp $
 */

#define VHASH_SIZE	256		/* Must be a power of 2 */
#define VNHASH_SIZE	256
#define VHASH_MASK	0xff		/* AND mask to get 0..hsize-1 */
#define VNHASH_MASK	0xff
/* This number will allocate entries 32k bytes at a time. */
#define VALLOC_SIZE	630
#define VNAME_SIZE	32

typedef struct user_attribute VATTR;
struct user_attribute {
	char	*name; /* Name of user attribute */
	int	number;		/* Assigned attribute number */
	int	flags;		/* Attribute flags */
	struct user_attribute *next;	/* name hash chain. */
};

extern void	NDECL(vattr_init);
extern VATTR *	FDECL(vattr_rename, (char *, char *));
extern VATTR *	FDECL(vattr_find, (char *));
extern VATTR *	FDECL(vattr_nfind, (int));
extern VATTR *	FDECL(vattr_alloc, (char *, int));
extern VATTR *	FDECL(vattr_define, (char *, int, int));
extern void	FDECL(vattr_delete, (char *));
extern VATTR *	FDECL(attr_rename, (char *, char *));
extern VATTR *	NDECL(vattr_first);
extern VATTR *	FDECL(vattr_next, (VATTR *));
extern void	FDECL(list_vhashstats, (dbref));
