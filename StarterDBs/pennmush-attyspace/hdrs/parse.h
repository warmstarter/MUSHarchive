/* parse.h - parser declarations and macros

 * Written by T. Alexander Popiel, 13 May 1995
 * Last modified by T. Alexander Popiel, 26 May 1995
 *
 * Copyright (c) 1995 by T. Alexander Popiel
 * See copyrite.h for details.
 */

#ifndef _PARSE_H_
#define _PARSE_H_

#include "copyrite.h"

#include "config.h"
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#ifdef I_LIMITS
#include <limits.h>
#else
#ifdef I_VALUES
#include <values.h>
#endif
#endif
#include <math.h>
#include "dbdefs.h"
#include "confmagic.h"

/* This is the type to be used for numbers which may be non-integral. */

#ifdef FLOATING_POINTS
#define NVAL double
#define HUGE_NVAL	HUGE_DOUBLE
#else
#define NVAL int
#define HUGE_NVAL	HUGE_INT
#endif

/* These are some common error messages. */
extern char e_int[];		/* #-1 ARGUMENT MUST BE INTEGER */
extern char e_ints[];		/* #-1 ARGUMENTS MUST BE INTEGERS */
#ifdef FLOATING_POINTS
extern char e_num[];		/* #-1 ARGUMENT MUST BE NUMBER */
extern char e_nums[];		/* #-1 ARGUMENTS MUST BE NUMBERS */
#else
#define e_num	e_int		/* #-1 ARGUMENT MUST BE INTEGER */
#define e_nums	e_ints		/* #-1 ARGUMENTS MUST BE INTEGERS */
#endif

/* This type is used by process_expression().  In all but parse.c,
 * this should be left as an incompletely-specified type, making it
 * impossible to declare anything but pointers to it.
 */

typedef struct pe_info PE_Info;

/* The following routines all take strings as arguments, and return
 * data of the appropriate types.  
 */

int parse_boolean _((char const *str));

#define parse_integer(str) atoi(str)

#ifdef FLOATING_POINTS
#define parse_number(str) atof(str)
#else
#define parse_number(str) atoi(str)
#endif

/* The following routines all take varoius arguments, and return
 * string representations of same.  The string representations
 * are stored in static buffers, so the next call to each function
 * will destroy any old string that was there.
 */

#define unparse_boolean(x) ((x) ? "1" : "0")

char *unparse_dbref _((dbref num));
char *unparse_integer _((int num));
char *unparse_number _((NVAL num));

/* The following routines all take strings as arguments, and return
 * true iff the string is a valid representation of the appropriate type.
 */
int is_dbref _((char const *str));

int is_integer _((char const *str));
int is_boolean _((char const *str));


/* All function declarations follow the format: */

#ifdef CAN_NEWSTYLE
#define FUNCTION(fun_name) \
  /* ARGSUSED */ /* try to keep lint happy */ \
  void fun_name _((char *buff, char **bp, int nargs, char *args[], \
		   dbref executor, dbref caller, dbref enactor, \
		   char const *called_as, PE_Info *pe_info)); \
  void fun_name(char *buff, char **bp, int nargs, char *args[], \
		dbref executor, dbref caller, dbref enactor, \
		char const *called_as, PE_Info *pe_info)
#else
#define FUNCTION(fun_name) \
  /* ARGSUSED */ /* try to make lint happy */ \
  void fun_name _((char *buff, char **bp, int nargs, char *args[], \
		   dbref executor, dbref caller, dbref enactor, \
		   char const *called_as, PE_Info *pe_info)); \
  void fun_name(buff, bp, nargs, args, executor, caller, enactor, \
		called_as, pe_info) \
      char *buff; \
      char **bp; \
      int nargs; \
      char *args[]; \
      dbref executor; \
      dbref caller; \
      dbref enactor; \
      char const *called_as; \
      PE_Info *pe_info;
#endif

/* All results are returned in buff, at the point *bp.  *bp is likely
 * not equal to buff, so make no assumptions about writing at the
 * start of the buffer.  *bp must be updated to point at the next
 * place to be filled (ala safe_str() and safe_chr()).  Be very
 * careful about not overflowing buff; use of safe_str() and safe_chr()
 * for all writes into buff is highly recommended.
 *
 * nargs is the count of the number of arguments passed to the function,
 * and args is an array of pointers to them.  args will have at least
 * nargs elements, or 10 elements, whichever is greater.  The first ten
 * elements are initialized to NULL for ease of porting functions from
 * the old style, but relying on such is considered bad form.
 * The argument strings are stored in BUFFER_LEN buffers, but reliance
 * on that size is also considered bad form.  The argument strings may
 * be modified, but modifying the pointers to the argument strings will
 * cause crashes. 
 *
 * executor corresponds to %!, the object invoking the function.
 * caller   corresponds to %@, the last object to do a U() or similar.
 * enactor  corresponds to %#, the object that started the whole mess.
 * Note that fun_ufun() and similar must swap around these parameters
 * in calling process_expression(); no checks are made in the parser
 * itself to maintain these values.
 *
 * called_as contains a pointer to the name of the function called
 * (taken from the function table).  This may be used to distinguish
 * multiple functions which use the same C function for implementation.
 *
 * pe_info holds context information used by the parser.  It should
 * be passed untouched to process_expression(), if it is called.
 * pe_info should be treated as a black box; its structure and contents
 * may change without notice.
 */

/* process_expression() evaluates expressions.  What a concept. */

void process_expression _((char *buff, char **bp, char const **str,
			   dbref executor, dbref caller, dbref enactor,
			   int eflags, int tflags, PE_Info * pe_info));

/* buff is a pointer to a BUFFER_LEN string to contain the expression
 * result.  *bp is the point in buff at which the result should be written.
 * *bp will be updated to point one past the result of the expression,
 * and the result will _NOT_ be null-terminated.
 * For top-level calls to process_expression(), *bp should probably equal
 * buff.  For calls to process_expression() inside function implementations,
 * buff and bp should probably be the values passed into the implementation.
 *
 * *str is a pointer to a string containing the expression to evaluate.
 * *str will be updated to point at the terminator which caused return
 * from process_expression().  The string pointed to by *str will not
 * be modified.
 *
 * executor, caller, and enactor represent %!, %@, and %#, respectively.
 * No validity checking of any sort is done on these parameters, so please
 * be careful with them.
 *
 * eflags consists of one or more of the following evaluation flags:
 */

#define PE_NOTHING		0
#define PE_COMPRESS_SPACES	0x00000001
#define PE_STRIP_BRACES		0x00000002
#define PE_EVALUATE		0x00000010
#define PE_FUNCTION_CHECK	0x00000020
#define PE_FUNCTION_MANDATORY	0x00000040
#define PE_LITERAL		0x00000100

#define PE_DEFAULT (PE_COMPRESS_SPACES | PE_STRIP_BRACES | \
		    PE_EVALUATE | PE_FUNCTION_CHECK)

/* PE_COMPRESS_SPACES strips leading and trailing spaces, and reduces sets
 * of internal spaces to one space.
 *
 * PE_STRIP_BRACES strips off top-level braces.
 *
 * PE_EVALUATE allows %-substitutions, []-evaluation, function evaluation,
 * and \-stripping.
 *
 * PE_FUNCTION_CHECK allows function evaluation.  Note that both PE_EVALUATE
 * and PE_FUNCTION_CHECK must be active for function evaluation to occur.
 *
 * PE_FUNCTION_MANDATORY causes an error to be reported if a function call
 * is attempted for a non-existant function.  Otherwise, the function call
 * is not evaluated, but rather treated as normal text.
 *
 * PE_LITERAL prevents { and [ from being recognized and causing recursion.
 *
 * PE_DEFAULT is the most commonly used set of flags, normally sufficient
 * for calls to process_expression().
 *
 *
 * tflags consists of one or more of the following termination flags:
 */

#define PT_NOTHING	0
#define PT_BRACE	0x00000001
#define PT_BRACKET	0x00000002
#define PT_PAREN	0x00000004
#define PT_COMMA	0x00000008
#define PT_SEMI		0x00000010
#define PT_EQUALS	0x00000020
#define PT_SPACE	0x00000040

/* These represent '\0', '}', ']', ')', ',', ';', '=', and ' ', respectively.
 * If the character corresponding to a set flag is encountered, then
 * process_expression() will exit, with *str pointing at the terminating
 * charater.  '\0' is always a terminating character.
 *
 * PT_DEFAULT, below, is provided as syntactic sugar.
 */

#define PT_DEFAULT PT_NOTHING

/* pe_info is a pointer to a structure of internal state information
 * for process_expression().  Top-level calls to process_expression()
 * should pass a NULL as pe_info.  Calls to process_expression() from
 * function implementations should pass their pe_info as pe_info.
 * In no case should any other pe_info be passed to process_expression().
 */

/* Miscellany */
void do_userfn _((char *buff, char **bp, dbref obj, ATTR *attrib,
		  int nargs, char **args, dbref executor, dbref caller,
		  dbref enactor, PE_Info * pe_info));
void init_process_expression _((void));

#endif				/* !_PARSE_H_ */
