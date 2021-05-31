/* parse.c */

#include "copyrite.h"

#include "config.h"
#include <math.h>
#ifdef I_LIMITS
#include <limits.h>
#endif
#ifdef I_VALUES
#include <values.h>
#endif
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>

#include "conf.h"
#include "ansi.h"
#include "dbdefs.h"
#include "externs.h"
#include "function.h"
#include "globals.h"
#include "intrface.h"
#include "match.h"
#include "mushdb.h"
#include "parse.h"

#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "mymalloc.h"
#include "confmagic.h"

extern char ccom[];		/* bsd.c */
extern char *absp[], *obj[], *poss[], *subj[];	/* fundb.c */

int global_fun_invocations;
int global_fun_recursions;

typedef struct debug_info Debug_Info;
struct debug_info {
  char *string;
  Debug_Info *prev;
  Debug_Info *next;
};

struct pe_info {
  int fun_invocations;
  int fun_depth;
  int nest_depth;
  Debug_Info *debug_strings;
};

/* Common error messages */
char e_int[] = "#-1 ARGUMENT MUST BE INTEGER";
char e_ints[] = "#-1 ARGUMENTS MUST BE INTEGERS";
#ifdef FLOATING_POINTS
char e_num[] = "#-1 ARGUMENT MUST BE NUMBER";
char e_nums[] = "#-1 ARGUMENTS MUST BE NUMBERS";
#endif
char e_invoke[] = "#-1 FUNCTION INVOCATION LIMIT EXCEEDED";

dbref
parse_dbref(str)
    char const *str;
{
  /* Make sure string is strictly in format "#nnn".
   * Otherwise, possesives will be fouled up.
   */
  char const *p;
  dbref num;

  if (!str || (*str != NUMBER_TOKEN))
    return NOTHING;
  for (p = str + 1; isdigit(*p); p++) {
  }
  if (*p)
    return NOTHING;

  num = atoi(str + 1);
  if (!GoodObject(num))
    return NOTHING;
  return num;
}

int
parse_boolean(str)
    char const *str;
{
  if (TINY_BOOLEANS) {
    return (atoi(str) ? 1 : 0);
  } else {
    /* Turn a string into a boolean value.
     * All negative dbrefs are false, all non-negative dbrefs are true.
     * Zero is false, all other numbers are true.
     * Empty (or space only) strings are false, all other strings are true.
     */
    if (!str)
      return 0;
    if (str[0] == '#' && str[1] == '-')		/* check like this so error msgs */
      return 0;			/* are false */
    if (is_strict_number(str))
      return parse_number(str) != 0;	/* avoid rounding problems */
    while (*str == ' ')
      str++;
    return *str != '\0';	/* force to 1 or 0 */
  }
}

int
is_boolean(str)
    char const *str;
{
  if (TINY_BOOLEANS)
    return is_integer(str);
  else
    return 1;
}

int
is_dbref(str)
    char const *str;
{
  if (!str || (*str != NUMBER_TOKEN))
    return 0;
  if (*(str + 1) == '-') {
    str++;
  }
  for (str++; isdigit(*str); str++) {
  }
  return !*str;
}

int
is_integer(str)
    char const *str;
{
  char const *p = str;
  int val;
  /* If we're emulating Tiny, anything is an integer */
  if (options.tiny_math)
    return 1;
  if (!str)
    return 0;
  while (isspace(*str))
    str++;
  if (!*str)
    return (NULL_EQ_ZERO ? 1 : 0);
  if ((*str == '-') && isdigit(str[1]))
    str += 2;
  while (isdigit(*str))
    str++;
  while (isspace(*str))
    str++;
  if (*str)
    return 0;
  val = parse_integer(p);
  if ((val > HUGE_INT) || (-val > HUGE_INT))
    return 0;
  return 1;
}

int
is_strict_number(str)
    char const *str;
{
  char const *p = str;
  NVAL val;

  if (!str)
    return 0;
  while (isspace(*str))
    str++;
  if (!*str)
    return (NULL_EQ_ZERO ? 1 : 0);
  if (((*str == '-') || (*str == '+')) && str[1])
    str++;
  while (isdigit(*str))
    str++;
  if (*str == '.')
    str++;
  while (isdigit(*str))
    str++;
  while (isspace(*str))
    str++;
  if (*str)
    return 0;
  val = parse_number(p);
  if ((val > HUGE_NVAL) || (-val > HUGE_NVAL))
    return 0;
  return 1;
}

int
is_number(str)
    char const *str;
{
  /* If we're emulating Tiny, anything is a number */
  if (options.tiny_math)
    return 1;
  return is_strict_number(str);
}

/* Table of interesting characters for process_expression() */
static char active_table[UCHAR_MAX + 1];

/* Function to initialize table above */
void
init_process_expression()
{
  active_table['\0'] = 1;
  active_table['%'] = 1;
  active_table['{'] = 1;
  active_table['['] = 1;
  active_table['('] = 1;
  active_table['\\'] = 1;
  active_table[' '] = 1;
  active_table['}'] = 1;
  active_table[']'] = 1;
  active_table[')'] = 1;
  active_table[','] = 1;
  active_table[';'] = 1;
  active_table['='] = 1;
}

#ifdef WIN32
#pragma warning( disable : 4761)	/* NJG: disable warning re conversion */
#endif
/* Function and other substitution evaluation. */
void
process_expression(buff, bp, str, executor, caller, enactor,
		   eflags, tflags, pe_info)
    char *buff;
    char **bp;
    char const **str;
    dbref executor;
    dbref caller;
    dbref enactor;
    int eflags;
    int tflags;
    PE_Info *pe_info;
{
  int debugging = 0, made_info = 0;
  char *debugstr = NULL, *sourcestr = NULL;
  char *realbuff = NULL, *realbp = NULL;
  int gender = -1;
  char *startpos = *bp;
  int had_space = 0;
  char temp[3];
  int temp_eflags;

  if (!buff || !bp || !str || !*str)
    return;
  if (Halted(executor))
    eflags = PE_NOTHING;
  if (eflags & PE_COMPRESS_SPACES)
    while (**str == ' ')
      (*str)++;
  if (!*str)
    return;

  if (eflags != PE_NOTHING) {
    if (((*bp) - buff) > (BUFFER_LEN - SBUF_LEN)) {
      realbuff = buff;
      realbp = *bp;
      buff = (char *) mush_malloc(BUFFER_LEN,
				  "process_expression.buffer_extension");
      *bp = buff;
      startpos = buff;
    }
    debugging = Debug(executor) && Connected(Owner(executor));
    if (debugging) {
      int j;
      char *debugp;
      char const *mark;
      Debug_Info *node;

      if (!pe_info) {
	made_info = 1;
	pe_info = (PE_Info *) mush_malloc(sizeof(PE_Info),
					  "process_expression.pe_info");
	pe_info->fun_invocations = 0;
	pe_info->fun_depth = 0;
	pe_info->nest_depth = 0;
	pe_info->debug_strings = NULL;
      }
      debugstr = (char *) mush_malloc(BUFFER_LEN,
				      "process_expression.debug_source");
      debugp = debugstr;
      safe_str(unparse_dbref(executor), debugstr, &debugp);
      safe_chr('!', debugstr, &debugp);
      for (j = 0; j <= pe_info->nest_depth; j++)
	safe_chr(' ', debugstr, &debugp);
      sourcestr = debugp;
      mark = *str;
      process_expression(debugstr, &debugp, str,
			 executor, caller, enactor,
			 PE_NOTHING, tflags, pe_info);
      *str = mark;
      if (eflags & PE_COMPRESS_SPACES)
	while ((debugp > sourcestr) && (debugp[-1] == ' '))
	  debugp--;
      *debugp = '\0';
      node = (Debug_Info *) mush_malloc(sizeof(Debug_Info),
					"process_expression.debug_node");
      node->string = debugstr;
      node->prev = pe_info->debug_strings;
      node->next = NULL;
      if (node->prev)
	node->prev->next = node;
      pe_info->debug_strings = node;
      pe_info->nest_depth++;
    }
  }
  for (;;) {
    /* Find the first "interesting" character */
    {
      char const *pos;
      int len, len2;
      /* Inlined strcspn() equivalent, to save on overhead and portability */
      pos = *str;
      while (!active_table[*(unsigned char const *) *str])
	(*str)++;
      /* Inlined safe_str(), since the source string
       * may not be null terminated */
      len = *str - pos;
      len2 = BUFFER_LEN - 1 - (*bp - buff);
      if (len > len2)
	len = len2;
      if (len >= 0) {
	memcpy(*bp, pos, len);
	*bp += len;
      }
    }

    switch (**str) {
      /* Possible terminators */
    case '}':
      if (tflags & PT_BRACE)
	goto exit_sequence;
      break;
    case ']':
      if (tflags & PT_BRACKET)
	goto exit_sequence;
      break;
    case ')':
      if (tflags & PT_PAREN)
	goto exit_sequence;
      break;
    case ',':
      if (tflags & PT_COMMA)
	goto exit_sequence;
      break;
    case ';':
      if (tflags & PT_SEMI)
	goto exit_sequence;
      break;
    case '=':
      if (tflags & PT_EQUALS)
	goto exit_sequence;
      break;
    case ' ':
      if (tflags & PT_SPACE)
	goto exit_sequence;
      break;
    case '\0':
      goto exit_sequence;
    }

    switch (**str) {
    case '%':			/* Percent substitutions */
      if (!(eflags & PE_EVALUATE)) {
	/* peak -- % escapes (at least) one character */
	char savec;

	safe_chr('%', buff, bp);
	(*str)++;
	savec = **str;
	if (!savec)
	  goto exit_sequence;
	safe_chr(savec, buff, bp);
	(*str)++;
	break;
      } else {
	char savec, nextc;
	char *savepos;
	ATTR *attrib;

	(*str)++;
	savec = **str;
	if (!savec)
	  goto exit_sequence;
	savepos = *bp;
	(*str)++;

	switch (savec) {
	case '%':		/* %% - a real % */
	  safe_chr('%', buff, bp);
	  break;
	case '!':		/* executor dbref */
	  safe_str(unparse_dbref(executor), buff, bp);
	  break;
	case '@':		/* caller dbref */
	  safe_str(unparse_dbref(caller), buff, bp);
	  break;
	case '#':		/* enactor dbref */
	  safe_str(unparse_dbref(enactor), buff, bp);
	  break;
	case '?':		/* function limits */
	  if (pe_info) {
	    safe_str(unparse_integer(pe_info->fun_invocations), buff, bp);
	    safe_chr(' ', buff, bp);
	    safe_str(unparse_integer(pe_info->fun_depth), buff, bp);
	  } else {
	    safe_str("0 0", buff, bp);
	  }
	  break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':		/* positional argument */
	  if (wenv[savec - '0'])
	    safe_str(wenv[savec - '0'], buff, bp);
	  break;
	case 'A':
	case 'a':		/* enactor absolute possessive pronoun */
	  if (gender < 0)
	    gender = get_gender(enactor);
	  safe_str(absp[gender], buff, bp);
	  break;
	case 'B':
	case 'b':		/* blank space */
	  safe_chr(' ', buff, bp);
	  break;
	case 'C':
	case 'c':		/* command line */
	  safe_str(ccom, buff, bp);
	  break;
	case 'L':
	case 'l':		/* enactor location dbref */
	  /* The security implications of this have
	   * already been talked to death.  Deal. */
	  safe_str(unparse_dbref(Location(enactor)), buff, bp);
	  break;
	case 'N':
	case 'n':		/* enactor name */
	  safe_str(Name(enactor), buff, bp);
	  break;
	case 'O':
	case 'o':		/* enactor objective pronoun */
	  if (gender < 0)
	    gender = get_gender(enactor);
	  safe_str(obj[gender], buff, bp);
	  break;
	case 'P':
	case 'p':		/* enactor possessive pronoun */
	  if (gender < 0)
	    gender = get_gender(enactor);
	  safe_str(poss[gender], buff, bp);
	  break;
	case 'Q':
	case 'q':		/* temporary storage */
	  nextc = **str;
	  if (!nextc)
	    goto exit_sequence;
	  (*str)++;
	  if (!isdigit(nextc))
	    break;
	  if (renv[nextc - '0'])
	    safe_str(renv[nextc - '0'], buff, bp);
	  break;
	case 'R':
	case 'r':		/* newline */
	  safe_str("\r\n", buff, bp);
	  break;
	case 'S':
	case 's':		/* enactor subjective pronoun */
	  if (gender < 0)
	    gender = get_gender(enactor);
	  safe_str(subj[gender], buff, bp);
	  break;
	case 'T':
	case 't':		/* tab */
	  safe_chr('\t', buff, bp);
	  break;
	case 'V':
	case 'v':
	case 'W':
	case 'w':
	case 'X':
	case 'x':		/* attribute substitution */
	  nextc = **str;
	  if (!nextc)
	    goto exit_sequence;
	  (*str)++;
	  temp[0] = UPCASE(savec);
	  temp[1] = UPCASE(nextc);
	  temp[2] = '\0';
	  attrib = atr_get(executor, temp);
	  if (attrib)
	    safe_str(uncompress(attrib->value), buff, bp);
	  break;
	default:		/* just copy */
	  safe_chr(savec, buff, bp);
	}

	if (isupper(savec))
	  *savepos = UPCASE(*savepos);
      }
      break;

    case '{':			/* "{}" parse group; recurse with no function check */
      if (!pe_info && eflags != PE_NOTHING) {
	made_info = 1;
	pe_info = (PE_Info *) mush_malloc(sizeof(PE_Info),
					  "process_expression.pe_info");
	pe_info->fun_invocations = 0;
	pe_info->fun_depth = 0;
	pe_info->nest_depth = 0;
	pe_info->debug_strings = NULL;
      }
      if (eflags & PE_LITERAL) {
	safe_chr('{', buff, bp);
	(*str)++;
	break;
      }
      if (!(eflags & PE_STRIP_BRACES))
	safe_chr('{', buff, bp);
      (*str)++;
      process_expression(buff, bp, str,
			 executor, caller, enactor,
			 eflags & ~(PE_STRIP_BRACES | PE_FUNCTION_CHECK),
			 PT_BRACE, pe_info);
      if (**str == '}') {
	if (!(eflags & PE_STRIP_BRACES))
	  safe_chr('}', buff, bp);
	(*str)++;
      }
      break;

    case '[':			/* "[]" parse group; recurse with mandatory function check */
      if (!pe_info && eflags != PE_NOTHING) {
	made_info = 1;
	pe_info = (PE_Info *) mush_malloc(sizeof(PE_Info),
					  "process_expression.pe_info");
	pe_info->fun_invocations = 0;
	pe_info->fun_depth = 0;
	pe_info->nest_depth = 0;
	pe_info->debug_strings = NULL;
      }
      if (eflags & PE_LITERAL) {
	safe_chr('[', buff, bp);
	(*str)++;
	break;
      }
      if (!(eflags & PE_EVALUATE)) {
	safe_chr('[', buff, bp);
	temp_eflags = eflags & ~PE_STRIP_BRACES;
      } else
	temp_eflags = eflags | PE_FUNCTION_CHECK | PE_FUNCTION_MANDATORY;
      (*str)++;
      process_expression(buff, bp, str,
			 executor, caller, enactor,
			 temp_eflags, PT_BRACKET, pe_info);
      if (**str == ']') {
	if (!(eflags & PE_EVALUATE))
	  safe_chr(']', buff, bp);
	(*str)++;
      }
      break;

    case '(':			/* Function call */
      if (!pe_info && eflags != PE_NOTHING) {
	made_info = 1;
	pe_info = (PE_Info *) mush_malloc(sizeof(PE_Info),
					  "process_expression.pe_info");
	pe_info->fun_invocations = 0;
	pe_info->fun_depth = 0;
	pe_info->nest_depth = 0;
	pe_info->debug_strings = NULL;
      }
      (*str)++;
      if (!(eflags & PE_EVALUATE) || !(eflags & PE_FUNCTION_CHECK)) {
	safe_chr('(', buff, bp);
	process_expression(buff, bp, str,
			   executor, caller, enactor,
			   eflags & ~PE_STRIP_BRACES, PT_PAREN, pe_info);
	if (**str == ')') {
	  safe_chr(')', buff, bp);
	  (*str)++;
	}
	break;
      } else {
	char *sargs[10];
	char **fargs;
	int args_alloced;
	int nfargs;
	int j;
	char *name;
	char *sp, *tp;
	FUN *fp;
	int temp_tflags;

	fargs = sargs;
	for (j = 0; j < 10; j++)
	  fargs[j] = NULL;
	args_alloced = 10;

	eflags &= ~PE_FUNCTION_CHECK;

	/* Get the function name */
	name = (char *) mush_malloc(BUFFER_LEN,
				    "process_expression.function_name");
	for (sp = startpos, tp = name;
	     sp < *bp;
	     sp++)
	  safe_chr(UPCASE(*sp), name, &tp);
	*tp = '\0';

	fp = func_hash_lookup(name);
	if (!fp) {
	  if (eflags & PE_FUNCTION_MANDATORY) {
	    *bp = startpos;
	    safe_str("#-1 FUNCTION (", buff, bp);
	    safe_str(name, buff, bp);
	    safe_str(") NOT FOUND", buff, bp);
	    process_expression(name, &tp, str,
			       executor, caller, enactor,
			       PT_NOTHING, PT_PAREN, pe_info);
	    if (**str == ')')
	      (*str)++;
	    mush_free((Malloc_t) name, "process_expression.function_name");
	    goto exit_sequence;
	  }
	  mush_free((Malloc_t) name, "process_expression.function_name");
	  safe_chr('(', buff, bp);
	  if (**str == ' ')
	    safe_chr(*(*str)++, buff, bp);
	  process_expression(buff, bp, str,
			     executor, caller, enactor,
			     eflags, PT_PAREN, pe_info);
	  if (**str == ')') {
	    if (eflags & PE_COMPRESS_SPACES && (*str)[-1] == ' ')
	      safe_chr(' ', buff, bp);
	    safe_chr(')', buff, bp);
	    (*str)++;
	  }
	  break;
	}
	mush_free((Malloc_t) name, "process_expression.function_name");
	*bp = startpos;

	/* Get the arguments */
	temp_eflags = (eflags & ~PE_FUNCTION_MANDATORY)
	  | PE_COMPRESS_SPACES | PE_EVALUATE | PE_FUNCTION_CHECK;
	if (fp->fun != fun_gfun) {
	  switch (fp->ftype) {
	  case FN_LITERAL:
	    temp_eflags |= PE_LITERAL;
	    /* FALL THROUGH */
	  case FN_NOPARSE:
	    temp_eflags &= ~(PE_COMPRESS_SPACES | PE_EVALUATE |
			     PE_FUNCTION_CHECK);
	    break;
	  }
	}
	temp_tflags = PT_COMMA | PT_PAREN;
	nfargs = 0;
	do {
	  char *argp;

	  if ((fp->maxargs < 0) && ((nfargs + 1) >= -fp->maxargs))
	    temp_tflags = PT_PAREN;

	  if (nfargs >= args_alloced) {
	    char **nargs;
	    nargs = (char **) mush_malloc((nfargs + 10) * sizeof(char *),
				  "process_expression.function_arglist");
	    for (j = 0; j < nfargs; j++)
	      nargs[j] = fargs[j];
	    if (fargs != sargs)
	      mush_free((Malloc_t) fargs,
			"process_expression.function_arglist");
	    fargs = nargs;
	    args_alloced += 10;
	  }
	  fargs[nfargs] = (char *) mush_malloc(BUFFER_LEN,
				 "process_expression.function_argument");
	  argp = fargs[nfargs];
	  process_expression(fargs[nfargs], &argp, str,
			     executor, caller, enactor,
			     temp_eflags, temp_tflags, pe_info);
	  *argp = '\0';
	  (*str)++;
	  nfargs++;
	} while ((*str)[-1] == ',');
	if ((*str)[-1] != ')')
	  (*str)--;

	/* If we have the right number of args, eval the function.
	 * Otherwise, return an error message.
	 * Special case: zero args is recognized as one null arg.
	 */
	if ((fp->minargs == 0) && (nfargs == 1) && !*fargs[0]) {
	  mush_free((Malloc_t) fargs[0],
		    "process_expression.function_argument");
	  fargs[0] = NULL;
	  nfargs = 0;
	}
	if ((nfargs < fp->minargs) || (nfargs > abs(fp->maxargs))) {
	  *bp = buff;
	  safe_str("#-1 FUNCTION (", buff, bp);
	  safe_str(fp->name, buff, bp);
	  safe_str(") EXPECTS ", buff, bp);
	  if (fp->minargs == abs(fp->maxargs)) {
	    safe_str(unparse_integer(fp->minargs), buff, bp);
	  } else if ((fp->minargs + 1) == abs(fp->maxargs)) {
	    safe_str(unparse_integer(fp->minargs), buff, bp);
	    safe_str(" OR ", buff, bp);
	    safe_str(unparse_integer(abs(fp->maxargs)), buff, bp);
	  } else if (fp->maxargs == INT_MAX) {
	    safe_str("AT LEAST ", buff, bp);
	    safe_str(unparse_integer(fp->minargs), buff, bp);
	  } else {
	    safe_str("BETWEEN ", buff, bp);
	    safe_str(unparse_integer(fp->minargs), buff, bp);
	    safe_str(" AND ", buff, bp);
	    safe_str(unparse_integer(abs(fp->maxargs)), buff, bp);
	  }
	  safe_str(" ARGUMENTS", buff, bp);
	} else {
	  global_fun_invocations++;
	  global_fun_recursions++;
	  pe_info->fun_invocations++;
	  pe_info->fun_depth++;
	  if ((pe_info->fun_invocations >= options.func_invk_lim) ||
	      (global_fun_invocations >= options.func_invk_lim)) {
	    int e_len = strlen(e_invoke);
	    if ((buff + e_len > *bp) || strcmp(e_invoke, *bp - e_len))
	      safe_str(e_invoke, buff, bp);
	  } else if ((pe_info->fun_depth >= options.func_nest_lim) ||
		     (global_fun_recursions >= options.func_nest_lim)) {
	    safe_str("#-1 FUNCTION RECURSION LIMIT EXCEEDED", buff, bp);
	  } else if (fp->fun != fun_gfun)
	    fp->fun(buff, bp, nfargs, fargs, executor, caller, enactor,
		    fp->name, pe_info);
	  else {
	    dbref thing;
	    ATTR *attrib;
	    thing = userfn_tab[GF_Index(fp->ftype)].thing;
	    attrib = atr_get(thing, userfn_tab[GF_Index(fp->ftype)].name);
	    if (!attrib) {
	      do_rawlog(LT_ERR,
		      "ERROR: @function (%s) without attribute (#%d/%s)",
		  fp->name, thing, userfn_tab[GF_Index(fp->ftype)].name);
	      safe_str("#-1 @FUNCTION (", buff, bp);
	      safe_str(fp->name, buff, bp);
	      safe_str(") MISSING ATTRIBUTE (", buff, bp);
	      safe_str(unparse_dbref(thing), buff, bp);
	      safe_chr('/', buff, bp);
	      safe_str(userfn_tab[GF_Index(fp->ftype)].name, buff, bp);
	      safe_chr(')', buff, bp);
	    } else
	      do_userfn(buff, bp, thing, attrib, nfargs, fargs,
			executor, caller, enactor, pe_info);
	  }
	  pe_info->fun_depth--;
	  global_fun_recursions--;
	}

	/* Free up the space allocated for the args */
	for (j = 0; j < nfargs; j++)
	  if (fargs[j])
	    mush_free((Malloc_t) fargs[j],
		      "process_expression.function_argument");
	if (fargs != sargs)
	  mush_free((Malloc_t) fargs, "process_expression.function_arglist");
      }
      break;

      /* Space compression */
    case ' ':
      had_space = 1;
      safe_chr(' ', buff, bp);
      (*str)++;
      if (eflags & PE_COMPRESS_SPACES) {
	while (**str == ' ')
	  (*str)++;
      } else
	while (**str == ' ') {
	  safe_chr(' ', buff, bp);
	  (*str)++;
	}
      break;

      /* Escape charater */
    case '\\':
      if (!(eflags & PE_EVALUATE))
	safe_chr('\\', buff, bp);
      (*str)++;
      if (!**str)
	goto exit_sequence;
      /* FALL THROUGH */

      /* Basic character */
    default:
      safe_chr(**str, buff, bp);
      (*str)++;
      break;
    }
  }

exit_sequence:
  if (eflags != PE_NOTHING) {
    if ((eflags & PE_COMPRESS_SPACES) && had_space &&
	((*str)[-1] == ' ') && ((*bp)[-1] == ' '))
      (*bp)--;

    if (debugging) {
      pe_info->nest_depth--;
      **bp = '\0';
      if (strcmp(sourcestr, startpos)) {
	if (pe_info->debug_strings) {
	  while (pe_info->debug_strings->prev)
	    pe_info->debug_strings = pe_info->debug_strings->prev;
	  while (pe_info->debug_strings->next) {
	    raw_notify(Owner(executor),
		       tprintf("%s :", pe_info->debug_strings->string));
	    pe_info->debug_strings = pe_info->debug_strings->next;
	    mush_free((Malloc_t) pe_info->debug_strings->prev,
		      "process_expression.debug_node");
	  }
	  mush_free((Malloc_t) pe_info->debug_strings,
		    "process_expression.debug_node");
	  pe_info->debug_strings = NULL;
	}
	raw_notify(Owner(executor), tprintf("%s => %s", debugstr, startpos));
      } else {
	Debug_Info *node;
	node = pe_info->debug_strings;
	if (node) {
	  pe_info->debug_strings = node->prev;
	  if (node->prev)
	    node->prev->next = NULL;
	  mush_free((Malloc_t) node, "process_expression.debug_node");
	}
      }
      mush_free((Malloc_t) debugstr, "process_expression.debug_source");
    }
    if (made_info)
      mush_free((Malloc_t) pe_info, "process_expression.pe_info");
    if (realbuff) {
      **bp = '\0';
      *bp = realbp;
      safe_str(buff, realbuff, bp);
      mush_free((Malloc_t) buff, "process_expression.buffer_extension");
    }
  }
}
#ifdef WIN32
#pragma warning( default : 4761)	/* NJG: enable warning re conversion */
#endif
