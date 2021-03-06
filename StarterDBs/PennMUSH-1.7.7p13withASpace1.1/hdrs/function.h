#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include "copyrite.h"

#define FN_REG 0x0
/* Function arguments aren't parsed */
#define FN_NOPARSE      0x1
#define FN_LITERAL      0x2
#define FN_ARG_MASK     0x3
/* Function is disabled */
#define FN_DISABLED     0x4
/* Function will fail if object is gagged */
#define FN_NOGAGGED  0x8
/* Function will fail if object is a guest */
#define FN_NOGUEST   0x10
/* Function will fail if object is fixed */
#define FN_NOFIXED   0x20
/* Function is wizard-only */
#define FN_WIZARD 0x40
/* Function is royalty or wizard */
#define FN_ADMIN  0x80
/* Function is god-only */
#define FN_GOD    0x100
/* Function is builtin */
#define FN_BUILTIN 0x200
/* Function can be overridden with a @function */
#define FN_OVERRIDE 0x400
/* Side-effect version of function no workie */
#define FN_NOSIDEFX 0x800


#ifndef HAVE_FUN_DEFINED
typedef struct fun FUN;
#define HAVE_FUN_DEFINED
#endif

typedef void (*function_func) (FUN *, char *, char **, int, char *[], int[],
			       dbref, dbref, dbref, const char *, PE_Info *);

/** A calling pointer to a function.
 * This union holds either a pointer to a function's code or
 * the offset of the function in the user-defined function table.
 */
union fun_call {
  function_func fun;	/**< Pointer to compiled function code */
  size_t offset;	/**< Offset into user-defined function table */
};

/** A function.
 * This structure represents a mushcode function.
 */
struct fun {
  const char *name;	/**< Function name */
  union fun_call where;	/**< Where to find the function to call it */
  int minargs;		/**< Minimum arguments required, or 0 */
  /** Maximum arguments allowed.
   * Maximum arguments allowed. If there is no limit, this is INT_MAX.
   * If this is negatve, the final argument to the function can contain
   * commas that won't be parsed, and the maximum number of arguments
   * is the absolute value of this variable.
   */
  int maxargs;
  unsigned int flags;	/**< Bitflags of function */
};

typedef struct userfn_entry USERFN_ENTRY;

/** A user-defined function
 * This structure represents an entry in the user-defined function table.
 */
struct userfn_entry {
  char *fn;		/**< Name of the function */
  dbref thing;		/**< Dbref of object where the function is defined */
  char *name;		/**< Name of attribute where the function is defined */
  unsigned int flags;	/**< Bitflags of function */
};

extern USERFN_ENTRY *userfn_tab;

extern void do_userfn(char *buff, char **bp,
		      dbref obj, ATTR *attrib,
		      int nargs, char **args,
		      dbref executor, dbref caller, dbref enactor,
		      PE_Info * pe_info);

extern FUN *func_hash_lookup(const char *name);
extern int check_func(dbref player, FUN *fp);
extern int restrict_function(const char *name, const char *restrict);
extern int alias_function(const char *function, const char *alias);
extern void do_function_restrict(dbref player, const char *name,
				 const char *restrict);
extern void do_function_restore(dbref player, const char *name);
extern void do_list_functions(dbref player, int lc);
extern char *list_functions(void);
extern void do_function(dbref player, char *name, char **argv);
extern void do_function_toggle(dbref player, char *name, int toggle);
extern void do_function_report(dbref player, char *name);
extern void do_function_delete(dbref player, char *name);
extern void function_init_postconfig(void);


#define FUNCTION_PROTO(fun_name) \
  extern void fun_name (FUN *fun, char *buff, char **bp, int nargs, char *args[], \
                   int arglen[], dbref executor, dbref caller, dbref enactor, \
                   char const *called_as, PE_Info *pe_info)
extern void function_add(const char *name, function_func fun, int minargs,
			 int maxargs, int ftype);

#endif
