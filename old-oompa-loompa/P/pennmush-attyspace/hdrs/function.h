#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include "copyrite.h"

#include "attrib.h"
#include "dbdefs.h"
#include "parse.h"

#define FN_REG		0
#define FN_NOPARSE	1
#define FN_LITERAL	2

#define MAX_GLOBAL_FNS   50
#define GLOBAL_OFFSET    100

#define GF_Index(x)      (x - GLOBAL_OFFSET)

typedef struct fun FUN;

struct fun {
  const char *name;
/* Sigh. This should be:
   void (*fun) _((char *buff, char **bp, int nargs, char *args[],
   dbref executor, dbref caller, dbref enactor,
   char const *called_as, PE_Info * pe_info));
   * But some compilers (e.g. ultrix 4.2) barf
 */
  void (*fun) ();
  int minargs;
  int maxargs;
  int ftype;
};

typedef struct userfn_entry USERFN_ENTRY;

struct userfn_entry {
  char *fn;
  dbref thing;
  char *name;
};

extern USERFN_ENTRY userfn_tab[MAX_GLOBAL_FNS];

extern void do_userfn _((char *buff, char **bp,
			 dbref obj, ATTR *attrib,
			 int nargs, char **args,
			 dbref executor, dbref caller, dbref enactor,
			 PE_Info * pe_info));

extern FUN *func_hash_lookup _((char *name));

#define FUNCTION_PROTO(fun_name) \
  extern void fun_name _((char *buff, char **bp, int nargs, char *args[], \
			  dbref executor, dbref caller, dbref enactor, \
			  char const *called_as, PE_Info *pe_info))

extern void function_add _((const char *name, void (*fun) (), int minargs,
			    int maxargs, int ftype));

#include "funs.h"

#endif
