/*-----------------------------------------------------------------
 * Local functions
 *
 * This file is reserved for local functions that you may wish
 * to hack into PennMUSH. Read parse.h for information on adding
 * functions. This file will not be overwritten when you update
 * to a new distribution, so it's preferable to add new functions
 * here and leave the other fun*.c files alone.
 *
 */

/* Here are some includes you're likely to need or want.
 * If your functions are doing math, include <math.h>, too.
 */
 
#include "copyrite.h"
#include "config.h"
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include "externs.h"
#include "intrface.h"
#include "parse.h"
#include "confmagic.h"
#include "function.h"

#ifdef TREKMUSH
#include <math.h>
#include "ansi.h"
#include "match.h"
#include "space.h"
#endif /* TREKMUSH */

void local_functions _((void));

/* Here you can use the new add_function instead of hacking into function.c
 * Example included :)
 */

#ifdef EXAMPLE
FUNCTION(local_fun_silly)
{
  safe_str(tprintf("Silly%sSilly", args[0]), buff, bp);
}

#endif

#ifdef TREKMUSH

/* ------------------------------------------------------------------------ */

dbref lookup_org (const char *name)
{
	dbref org, thing;
	
	if (is_dbref(name)) {
		org = parse_dbref(name);
		if (OrgObj(org) && GoodObject(org))
			return org;
	} else
		for (thing = 0; thing < db_top; thing++)
			if (OrgObj(thing) && GoodObject(thing))
				if (local_wild_match(name, Name(thing)))
      					return thing;
	
	return NOTHING;
}

dbref lookup_space (const char *name)
{
	dbref space, thing;
	
	if (is_dbref(name)) {
		space = parse_dbref(name);
		if (SpaceObj(space) && GoodObject(space))
			return space;
	} else
		for (thing = 0; thing < db_top; thing++)
			if (SpaceObj(thing) && GoodObject(thing))
				if (local_wild_match(name, Name(thing)))
      					return thing;
	
	return NOTHING;
}

return_t
  crack_list(clist, min_list, max_list, actual, array)
char *clist;
int min_list;
int max_list;
int *actual;
array_t array;
{
  int i = 0;
  int j, p;
  char list[MAX_LIST * MAX_NAME];
  
  /* validate parameters */

  *actual = 0;
  if (min_list < 1)
    return MIN_LIST_TOO_SMALL;
  
  if (max_list > MAX_LIST)
    return MAX_LIST_TOO_LARGE;
  
  strcpy (list, uncompress(clist));

  /* skip to first non-whitespace */
  
  p = 0;
  while  ((isspace(list[p])) && (list[p] != '\0'))
    p++;
 
  /* Split apart the 'words' */
  
  if (list[p] != '\0') {
    for (i=0; i < MAX_LIST; i++) {
      
      /* copy word entry */
      
      if (list[p] != '\0') {
	for (j=0; j < MAX_NAME; j++) {
	  if ((!isspace(list[p])) && (list[p] != '\0')) {
	    array[i][j] = list[p];
	    p++;
	  } else {
	    array[i][j] = '\0';
	    break;
	  }	  
	}
      } else {
	break;
      }
     
      /* truncate a word that is too long */

      if (!isspace(list[p]))
	array[i][MAX_NAME] = '\0';
	while ((!isspace(list[p])) && (list[p] != '\0'))
	  p++;
      
      /* skip to next non-whitespace */
      
      while  ((isspace(list[p])) && (list[p] != '\0'))
	p++;
    }  
  }

  /* Validate result */

  *actual = i;
  if (i < min_list)
    return MIN_LIST_NOT_MET;
  else if (i > max_list)
    return MAX_LIST_EXCEEDED;
  else
    return SUCCESS;
}

return_t
  convert_double (input, min, max, deflt, output)
char *input;
double min;
double max;
double deflt;
double *output;
{
  if (is_number(input)) {
    *output = atof(input);
/*  if (*output < min) {
      *output = deflt;   
      return CONVERSION_TOO_SMALL;
    } else if (*output > max) {
      *output = deflt;
      return CONVERSION_TOO_LARGE;
    } else {
      return SUCCESS;
    } */
    return SUCCESS;
  } else {
    *output = deflt;
    return CONVERSION_ERROR;
  }
}

return_t
  convert_long (input, min, max, deflt, output)
char *input;
long min;
long max;
long deflt;
long *output;
{
  if (is_number(input)) {
    *output = atoi(input);
/*  if (*output < min) {
      *output = deflt;   
      return CONVERSION_TOO_SMALL;
    } else if (*output > max) {
      *output = deflt;
      return CONVERSION_TOO_LARGE;
    } else {
      return SUCCESS;
    } */
    return SUCCESS;
  } else {
    *output = deflt;
    return CONVERSION_ERROR;
  }
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
#ifdef EXTENDED_ANSI
FUNCTION(fun_ansip)
{
	int row, col;
	
	if (!is_integer(args[0]) || !is_integer(args[1])) {
		safe_str(e_int, buff, bp);
	} else {
		row = parse_integer(args[0]);
		col = parse_integer(args[1]);
		if (row < 1 || row > 24 || col < 1 || col > 80) {
			safe_str("#-1 POSITION OUT OF RANGE", buff, bp);
		} else {
			safe_str(ANSI_POS_1, buff, bp);
			safe_str(unparse_integer(row), buff, bp);
			safe_str(ANSI_POS_2, buff, bp);
			safe_str(unparse_integer(col), buff, bp);
			safe_str(ANSI_POS_3, buff, bp);
			safe_str(args[2], buff, bp);
		}
	}
	return;
}
#else /* EXTENDED_ANSI */
FUNCTION(fun_ansip)
{
	safe_str("#-1 FUNCTION DISABLED", buff, bp);
	return;
}
#endif /* EXTENDED_ANSI */

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_cash)
{
	dbref thing;
	ATTR *a;
	
	thing = match_thing(executor, args[0]);
	if (thing == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, thing)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else {
		if (nargs == 2) {
#ifdef FUNCTION_SIDE_EFFECTS
			if (!Wizard(executor)) {
				safe_str("#-1 PERMISSION DENIED", buff, bp);
			} else if (!is_number(args[1])) {
				safe_str(e_num, buff, bp);
			} else {
				(void) atr_add(thing, "MONEY", args[1], GOD, AF_LOCKED|AF_WIZARD|AF_ODARK|AF_NOPROG);
			}
#else
			safe_str("#-1 FUNCTION DISABLED", buff, bp);
#endif
		} else {
			a = atr_get_noparent(thing, "MONEY");
			if (a == NULL) {
				safe_str("0", buff, bp);
			} else if (!is_number(uncompress(a->value))) {
				safe_str("0", buff, bp);
			} else {
				safe_str(uncompress(a->value), buff, bp);
			}
		}
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_deg2rad)
{
	if (!is_number(args[0])) {
		safe_str(e_num, buff, bp);
	} else
		safe_str(unparse_number(parse_number(args[0]) / 57.29577951), buff, bp);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_fmod)
{
	NVAL bot;
	
	if (!is_number(args[0]) || !is_number(args[1])) {
		safe_str(e_num, buff, bp);
	} else {
		bot = parse_number(args[1]);
		if (bot == 0.0) {
			safe_str("#-1 DIVISION BY ZERO", buff, bp);
		} else 
			safe_str(unparse_number(fmod(parse_number(args[0]), bot)), buff, bp);
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_ifelse)
{
	char mstr[BUFFER_LEN], *dp;
	char const *sp;
	char *tbuf1;
	
	dp = mstr;
	sp = args[0];
	process_expression(mstr, &dp, &sp, executor, caller, enactor,
		PE_DEFAULT, PT_DEFAULT, pe_info);
	*dp = '\0';
	
	if (parse_boolean(mstr)) {
		tbuf1 = replace_string("#$", mstr, args[1]);
	} else {
		tbuf1 = replace_string("#$", mstr, args[2]);
	}
	
	sp = tbuf1;
	process_expression(buff, bp, &sp,
		executor, caller, enactor,
		PE_DEFAULT, PT_DEFAULT, pe_info);
	mush_free((Malloc_t) tbuf1, "replace_string.buff");
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_ifthen)
{
	char mstr[BUFFER_LEN], *dp;
	char const *sp;
	char *tbuf1;

	dp = mstr;
	sp = args[0];
	process_expression(mstr, &dp, &sp, executor, caller, enactor,
		PE_DEFAULT, PT_DEFAULT, pe_info);
	*dp = '\0';
	
	if (parse_boolean(mstr)) {
		tbuf1 = replace_string("#$", mstr, args[1]);
		sp = tbuf1;
		process_expression(buff, bp, &sp,
			executor, caller, enactor,
			PE_DEFAULT, PT_DEFAULT, pe_info);
		mush_free((Malloc_t) tbuf1, "replace_string.buff");
	}
	
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_inv)
{
	NVAL bot;
	
	if (!is_number(args[0])) {
		safe_str(e_num, buff, bp);
	} else {
		bot = parse_number(args[0]);
		if (bot == 0.0) {
			safe_str("#-1 DIVISION BY ZERO", buff, bp);
		} else 
			safe_str(unparse_number(1 / parse_number(args[0])), buff, bp);
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_inzone)
{
	dbref thing, room;
	dbref it = match_thing(executor, args[0]);
	int type;
	int first = 0;
	
	if (it == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, it)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else {
		switch (args[1][0]) {
			case 't': case 'T':
				type = TYPE_THING;
				break;
			case 'p': case 'P':
				type = TYPE_PLAYER;
				break;
			default:
				safe_str("#-1 INVALID TYPE", buff, bp);
				return;
				break;
		}
		for (thing = 0; thing < db_top; thing++) {
			if (Typeof(thing) == type) {
				room = Location(thing);
				if (GoodObject(room)) {
					while (Typeof(room) != TYPE_ROOM) {
						room = Location(room);
						if (!GoodObject(room)) {
							break;
						}
					}
					room = Zone(room);
					if (GoodObject(room)) {
						if (room == it) {
							if (first) {
								safe_chr(' ', buff, bp);
							} else
								first = 1;
							safe_str(unparse_dbref(thing), buff, bp);
						}
					}
				}
			}
		}
	}	
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_isa)
{
	dbref thing, parent;
	
	thing = match_thing(executor, args[0]);
	parent = match_thing(executor, args[1]);
	
	if (thing == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (parent == NOTHING) {
		safe_str("#-1 PARENT NOT FOUND", buff, bp);
	} else {
		if (Parent(thing) == parent) {
			safe_chr('1', buff, bp);
		} else {
			safe_chr('0', buff, bp);
		}
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_lkids)
{
	dbref thing;
	dbref it = match_thing(executor, args[0]);
	int first = 1;
	
	if (it == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, it)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else
		for (thing = 0; thing < db_top; thing++)
			if (Parent(thing) == it) {
				if (first) {
					first = 0;
				} else
					safe_chr(' ', buff, bp);
				safe_str(unparse_dbref(thing), buff, bp);
			}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_lnuma)
{
	int j, lim, first;
	
	if (!is_integer(args[0])) {
		safe_str(e_int, buff, bp);
		return;
	}
	lim = parse_integer(args[0]);
	if (lim < 1) {
		safe_str("#-1 NUMBER OUT OF RANGE", buff, bp);
		return;
	}
	first = 1;
	for (j = 1; j <= lim; j++) {
		if (first) {
			first = 0;
		} else
			safe_chr(' ', buff, bp);
		if (safe_str(unparse_integer(j), buff, bp))
			break;
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_logb)
{
	NVAL num, base;

	if (!is_number(args[0]) || !is_number(args[1])) {
		safe_str(e_num, buff, bp);
	} else {
		num = parse_number(args[0]);
		base = parse_number(args[1]);
		if ((num <= 0.0) || (base <= 0.0)) {
			safe_str("#-1 OUT OF RANGE", buff, bp);
		} else 
			safe_str(unparse_number(log10(num) / log10(base)), buff, bp);
	}
    return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_ly2pc)
{
	if (!is_number(args[0])) {
		safe_str(e_num, buff, bp);
	} else
		safe_str(unparse_number(parse_number(args[0]) * LIGHTYEAR / PARSEC), buff, bp);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_ly2su)
{
	if (!is_number(args[0])) {
		safe_str(e_num, buff, bp);
	} else
		safe_str(unparse_number(parse_number(args[0]) * LIGHTYEAR), buff, bp);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_lzone)
{
	dbref thing;
	dbref it = match_thing(executor, args[0]);
	int type;
	int first = 1;
	
	if (it == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, it)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else if (nargs == 2) {
		switch (args[1][0]) {
			case 't': case 'T':
				type = TYPE_THING;
				break;
			case 'p': case 'P':
				type = TYPE_PLAYER;
				break;
			case 'r': case 'R':
				type = TYPE_ROOM;
				break;
			case 'e': case 'E':
				type = TYPE_EXIT;
				break;
			default:
				safe_str("#-1 INVALID TYPE", buff, bp);
				return;
				break;
		}
		for (thing = 0; thing < db_top; thing++)
			if (Zone(thing) == it)
				if (Typeof(thing) == type) {
					if (first) {
						first = 0;
					} else
						safe_chr(' ', buff, bp);
					safe_str(unparse_dbref(thing), buff, bp);
				}
	} else
		for (thing = 0; thing < db_top; thing++)
			if (Zone(thing) == it) {
				if (first) {
					first = 0;
				} else
					safe_chr(' ', buff, bp);
				safe_str(unparse_dbref(thing), buff, bp);
			}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_masterorg)
{
	dbref org;
	dbref thing;
	ATTR *a;
	
	thing = match_thing(executor, args[0]);
	if (thing == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, thing)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else {
		if (nargs == 2) {
#ifdef FUNCTION_SIDE_EFFECTS
			org = match_thing(executor, args[1]);
			if (!Wizard(executor)) {
				safe_str("#-1 PERMISSION DENIED", buff, bp);
			} else if (org == NOTHING) {
				safe_str("#-1 ORG OBJECT NOT FOUND", buff, bp);
			} else if (!OrgObj(org)) {
				safe_str("#-1 INVALID ORG OBJECT", buff, bp);
			} else {
				(void) atr_add(thing, "MASTERORG", unparse_dbref(org), GOD, AF_LOCKED|AF_WIZARD|AF_ODARK|AF_NOPROG);
			}
#else
			safe_str("#-1 FUNCTION DISABLED", buff, bp);
#endif
		} else {
			a = atr_get(thing, "MASTERORG");
			if (a == NULL) {
				safe_str("#-1", buff, bp);
			} else {
				org = parse_dbref(uncompress(a->value));
				if (!GoodObject(org) || !OrgObj(org)) {
					safe_str("#-1", buff, bp);
				} else {
					safe_str(unparse_dbref(org), buff, bp);
				}
			}
		}
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_neg)
{
	if (!is_number(args[0])) {
		safe_str(e_num, buff, bp);
	} else
		safe_str(unparse_number(parse_number(args[0]) * -1.0), buff, bp);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_org)
{
	dbref org;
	dbref thing;
	ATTR *a;
	
	thing = match_thing(executor, args[0]);
	if (thing == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, thing)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else {
		if (nargs == 2) {
#ifdef FUNCTION_SIDE_EFFECTS
			org = match_thing(executor, args[1]);
			if (!Wizard(executor)) {
				safe_str("#-1 PERMISSION DENIED", buff, bp);
			} else if (org == NOTHING) {
				safe_str("#-1 ORG OBJECT NOT FOUND", buff, bp);
			} else if (!OrgObj(org)) {
				safe_str("#-1 INVALID ORG OBJECT", buff, bp);
			} else {
				(void) atr_add(thing, "ORGANIZATION", unparse_dbref(org), GOD, AF_LOCKED|AF_WIZARD|AF_ODARK|AF_NOPROG);
			}
#else
			safe_str("#-1 FUNCTION DISABLED", buff, bp);
#endif
		} else {
			a = atr_get(thing, "ORGANIZATION");
			if (a == NULL) {
				safe_str("#-1", buff, bp);
			} else {
				org = parse_dbref(uncompress(a->value));
				if (!GoodObject(org) || !OrgObj(org)) {
					safe_str("#-1", buff, bp);
				} else {
					safe_str(unparse_dbref(org), buff, bp);
				}
			}
		}
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_orgnum)
{
	dbref org;
	if (Hasprivs(executor)) {
		org = lookup_org(args[0]);
		if (org == NOTHING) {
			safe_str("#-1 ORG OBJECT NOT FOUND", buff, bp);
		} else
			safe_str(unparse_dbref(org), buff, bp);
	} else
	    safe_str("#-1 PERMISSION DENIED", buff, bp);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_pc2ly)
{
	if (!is_number(args[0])) {
		safe_str(e_num, buff, bp);
	} else
		safe_str(unparse_number(parse_number(args[0]) * PARSEC / LIGHTYEAR), buff, bp);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_pc2su)
{
	if (!is_number(args[0])) {
		safe_str(e_num, buff, bp);
	} else
		safe_str(unparse_number(parse_number(args[0]) * PARSEC), buff, bp);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_position)
{
	dbref thing;
	ATTR *a;
	
	thing = match_thing(executor, args[0]);
	if (thing == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, thing)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else {
		if (nargs == 2) {
#ifdef FUNCTION_SIDE_EFFECTS
			if (!Wizard(executor)) {
				safe_str("#-1 PERMISSION DENIED", buff, bp);
			} else {
				(void) atr_add(thing, "POSITION", args[1], GOD, AF_LOCKED|AF_WIZARD|AF_ODARK|AF_NOPROG);
			}
#else
			safe_str("#-1 FUNCTION DISABLED", buff, bp);
#endif
		} else {
			a = atr_get(thing, "POSITION");
			if (a == NULL) {
				return;
			} else {
				safe_str(uncompress(a->value), buff, bp);
			}
		}
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_race)
{
	dbref race;
	dbref thing;
	ATTR *a;
	
	thing = match_thing(executor, args[0]);
	if (thing == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, thing)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else {
		if (nargs == 2) {
#ifdef FUNCTION_SIDE_EFFECTS
			race = match_thing(executor, args[1]);
			if (!Wizard(executor)) {
				safe_str("#-1 PERMISSION DENIED", buff, bp);
			} else if (race == NOTHING) {
				safe_str("#-1 RACE OBJECT NOT FOUND", buff, bp);
			} else {
				(void) atr_add(thing, "RACE", unparse_dbref(race), GOD, AF_LOCKED|AF_WIZARD|AF_MDARK|AF_NOPROG);
			}
#else
			safe_str("#-1 FUNCTION DISABLED", buff, bp);
#endif
		} else {
			a = atr_get(thing, "RACE");
			if (a == NULL) {
				safe_str("#-1", buff, bp);
			} else {
				race = parse_dbref(uncompress(a->value));
				if (!GoodObject(race)) {
					safe_str("#-1", buff, bp);
				} else {
					safe_str(unparse_dbref(race), buff, bp);
				}
			}
		}
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_racename)
{
	dbref race;
	dbref thing;
	ATTR *a;
	
	thing = match_thing(executor, args[0]);
	if (thing == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, thing)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else {
		a = atr_get(thing, "RACE");
		if (a == NULL) {
			safe_str("None", buff, bp);
		} else {
			race = parse_dbref(uncompress(a->value));
			if (!GoodObject(race)) {
				safe_str("None", buff, bp);
			} else {
				safe_str(Name(race), buff, bp);
			}
		}
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_rad2deg)
{
	if (!is_number(args[0])) {
		safe_str(e_num, buff, bp);
	} else
		safe_str(unparse_number(parse_number(args[0]) * 57.29577951), buff, bp);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_rank)
{
	int rank;
	dbref thing;
	ATTR *a;
	
	thing = match_thing(executor, args[0]);
	if (thing == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, thing)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else {
		if (nargs == 2) {
#ifdef FUNCTION_SIDE_EFFECTS
			rank = parse_integer(args[1]);
			if (!Wizard(executor)) {
				safe_str("#-1 PERMISSION DENIED", buff, bp);
			} else if (!is_integer(args[1])) {
				safe_str(e_ints, buff, bp);
			} else {
				rank = parse_integer(args[1]);
				if (rank < 0 || rank > 100) {
					safe_str("#-1 RANK NUMBER OF OF RANGE", buff, bp);
				} else {
					(void) atr_add(thing, "RANK", unparse_integer(rank), GOD, AF_LOCKED|AF_WIZARD|AF_MDARK|AF_NOPROG);
				}
			}
#else
			safe_str("#-1 FUNCTION DISABLED", buff, bp);
#endif
		} else {
			a = atr_get(thing, "RANK");
			if (a == NULL) {
				safe_str("0", buff, bp);
			} else {
				rank = parse_integer(uncompress(a->value));
				if (rank < 0 || rank > 100) {
					safe_str("0", buff, bp);
				} else {
					safe_str(unparse_integer(rank), buff, bp);
				}
			}
		}
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_rankname)
{
	int rank, sex;
	dbref thing, org;
	ATTR *a;
	
	thing = match_thing(executor, args[0]);
	if (thing == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, thing)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else {
		a = atr_get(thing, "RANK");
		if (a != NULL) {
			rank = parse_integer(uncompress(a->value));
			if (rank >= 0 || rank <= 100) {
				a = atr_get(thing, "ORGANIZATION");
				if (a != NULL) {
					org = parse_dbref(uncompress(a->value));
					if (GoodObject(org) && OrgObj(org)) {
						sex = get_gender(thing);
						if (sex == 3) {
							a = atr_get(org, tprintf("RANK_%d_M", rank));
						} else
							a = atr_get(org, tprintf("RANK_%d_F", rank));
						if (a != NULL) {
							safe_str(uncompress(a->value), buff, bp);
						}
					}
				}
			}
		}
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_secs2sd)
{
	if (!is_integer(args[0])) {
		safe_str(e_ints, buff, bp);
	} else
		safe_str(unparse_number(((parse_integer(args[0]) - 558964800) / 31557.6) + 61153.7), buff, bp);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_sd2secs)
{
	double sd;
	
	if (!is_number(args[0])) {
		safe_str(e_num, buff, bp);
	} else {
		sd = parse_number(args[0]);
		if (sd > 80000.0) {
			safe_str("#-1 STARDATE TOO LARGE", buff, bp);
		} else if (sd < 0.0) {
			safe_str("#-1 STARDATE TOO SMALL", buff, bp);
		} else
			safe_str(unparse_integer((int) ((sd - 61153.7) * 31557.6) + 558964800), buff, bp);
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_spacenum)
{
	dbref space;
	if (Hasprivs(executor)) {
		space = lookup_space(args[0]);
		if (space == NOTHING) {
			safe_str("#-1 SPACE OBJECT NOT FOUND", buff, bp);
		} else
			safe_str(unparse_dbref(space), buff, bp);
	} else
	    safe_str("#-1 PERMISSION DENIED", buff, bp);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_sph2xyz)
{
	NVAL b, e, r;
	
	if (!is_number(args[0]) || !is_number(args[1]) || !is_number(args[2])) {
		safe_str(e_num, buff, bp);
	} else {
		b = parse_number(args[0]) * PI / 180;
		e = parse_number(args[1]) * PI / 180;
		r = parse_number(args[2]) ;
		safe_str(unparse_number(cos(b) * cos(e) * r), buff, bp);
		safe_chr(' ', buff, bp);
		safe_str(unparse_number(sin(b) * cos(e) * r), buff, bp);
		safe_chr(' ', buff, bp);
		safe_str(unparse_number(sin(e) * r), buff, bp);
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_sum)
{
  NVAL num, sum;
  char *p1;
  char sep;

  /* return if a list is empty */
  if (!args[0])
    return;

  if (!delim_check(buff, bp, nargs, args, 2, &sep))
    return;
  p1 = trim_space_sep(args[0], sep);

  /* return if a list is empty */
  if (!*p1)
    return;

  /* sum the squares */
  num = parse_number(split_token(&p1, sep));
  sum = num;
  while (p1) {
    num = parse_number(split_token(&p1, sep));
    sum += num;
  }

#ifdef FLOATING_POINTS
  safe_str(unparse_number(sum), buff, bp);
#else
  safe_str(unparse_number((int) (sum + 0.5)), buff, bp);
#endif                          /* FLOATING_POINTS */
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_su2ly)
{
	if (!is_number(args[0])) {
		safe_str(e_num, buff, bp);
	} else
		safe_str(unparse_number(parse_number(args[0]) / LIGHTYEAR), buff, bp);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_su2pc)
{
	if (!is_number(args[0])) {
		safe_str(e_num, buff, bp);
	} else
		safe_str(unparse_number(parse_number(args[0]) / PARSEC), buff, bp);
	return;
}


/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_truerace)
{
	dbref race;
	dbref thing;
	ATTR *a;
	
	thing = match_thing(executor, args[0]);
	if (thing == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, thing)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else {
		if (nargs == 2) {
#ifdef FUNCTION_SIDE_EFFECTS
			race = match_thing(executor, args[1]);
			if (!Wizard(executor)) {
				safe_str("#-1 PERMISSION DENIED", buff, bp);
			} else if (race == NOTHING) {
				safe_str("#-1 RACE OBJECT NOT FOUND", buff, bp);
			} else {
				(void) atr_add(thing, "TRUERACE", unparse_dbref(race), GOD, AF_LOCKED|AF_WIZARD|AF_MDARK|AF_NOPROG);
			}
#else
			safe_str("#-1 FUNCTION DISABLED", buff, bp);
#endif
		} else {
			a = atr_get(thing, "TRUERACE");
			if (a == NULL) {
				safe_str("#-1", buff, bp);
			} else {
				race = parse_dbref(uncompress(a->value));
				if (!GoodObject(race)) {
					safe_str("#-1", buff, bp);
				} else {
					safe_str(unparse_dbref(race), buff, bp);
				}
			}
		}
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_xyz2sph)
{
	NVAL b, e, r, x, y, z, xy;
	
	if (!is_number(args[0]) || !is_number(args[1]) || !is_number(args[2])) {
		safe_str(e_num, buff, bp);
	} else {
		x = parse_number(args[0]);
		y = parse_number(args[1]);
		z = parse_number(args[2]);
		xy = sqrt(x * x + y * y);
		r = sqrt(x * x + y * y + z * z);
#ifndef HAS_IEEE_MATH
  		/* You can overflow, which is bad. */
		if ((r < 0.0) || (xy < 0.0)) {
			safe_str("#-1 OVERFLOW ERROR", buff, bp);
			return;
  		}
#endif /* HAS_IEEE_MATH */
		if (y == 0.0) {
			if (x == 0.0) {
				b = 0.0;
			} else if (x > 0.0) {
				b = 0.0;
			} else
				b = 180.0;
		} else if (x == 0.0) {
			if (y > 0.0) {
				b = 90.0;
			} else
				b = 270.0;
		} else if (x > 0.0) {
			if (y > 0.0) {
				b = atan(y / x) * 180.0 / PI;
			} else
				b = atan(y / x) * 180.0 / PI + 360.0;
		} else if (x < 0.0) {
			b = atan(y / x) * 180.0 / PI + 180.0;
		} else 
			b = 0.0;
		
		if (xy == 0.0) {
			if (z == 0.0) {
				e = 0.0;
			} else if (z > 0.0) {
				e = 90.0;
			} else
				e = 270.0;
		} else if (z > 0.0) {
			e = atan(z / xy) * 180.0 / PI;
		} else if (z < 0.0) {
			e = atan(z / xy) * 180.0 / PI + 360;
		} else
			e = 0.0;
		
		safe_str(unparse_number(b), buff, bp);
		safe_chr(' ', buff, bp);
		safe_str(unparse_number(e), buff, bp);
		safe_chr(' ', buff, bp);
		safe_str(unparse_number(r), buff, bp);
	}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_zwho)
{
	dbref thing;
	dbref it = match_thing(executor, args[0]);
	int first = 1;
	
	if (it == NOTHING) {
		safe_str("#-1 OBJECT NOT FOUND", buff, bp);
	} else if (!Can_Examine(executor, it)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else
		for (thing = 0; thing < db_top; thing++)
			if (Typeof(thing) == TYPE_PLAYER)
				if (Zone(thing) == it) {
					if (first) {
						first = 0;
					} else
						safe_chr(' ', buff, bp);
					safe_str(unparse_dbref(thing), buff, bp);
				}
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_vcross)
{
  int i;
  NVAL a[3],b[3],c[3];
  char *p1, *p2;
  char sep;

  /* return if a list is empty */
  if (!args[0] || !args[1])
    return;
        
  if (!delim_check(buff, bp, nargs, args, 3, &sep))
    return;
  p1 = trim_space_sep(args[0], sep);
  p2 = trim_space_sep(args[1], sep);

  /* return if a list is empty */
  if (!*p1 || !*p2) 
    return;
  
  /* get vectors */   
  for (i=0;(i<3) && p1 && p2;i++){
    a[i] = parse_number(split_token(&p1,sep));
    b[i] = parse_number(split_token(&p2,sep));
  }
  if (p1 || p2 || (i<3) ) {
    safe_str("#-1 INVALID VECTOR SIZES", buff, bp);
    return;
  }
  /* magic vcross for loop */
  for (i=0;i<3;i++) {
    if(i) safe_chr(' ', buff, bp);  
    safe_str(unparse_number(((i%2)?-1:1)*(a[(i+1)%3]*b[(i+2)%3] - a[(i+2)%3]*b[(i+1)%3])),
                 buff, bp);
  }
}


/* ------------------------------------------------------------------------ */
#endif /* TREKMUSH */

void
local_functions()
{
#ifdef EXAMPLE
  function_add("SILLY", local_fun_silly, 1, 1, FN_REG);
#endif
#ifdef TREKMUSH
  function_add((char *) "ANSIP", fun_ansip, 3, -3, FN_REG);
  function_add((char *) "CASH", fun_cash, 1, 2, FN_REG);
  function_add((char *) "CDB", fun_cdb, 1, 5, FN_REG);
  function_add((char *) "CHILDREN", fun_lkids, 1, 1, FN_REG);
  function_add((char *) "CMDS", fun_cmds, 1, 1, FN_REG);
/* OFFLINE function_add((char *) "DBEVAL", fun_dbeval, 2, 2, FN_REG);
  function_add((char *) "DBGET", fun_dbget, 2, 2, FN_REG);
  function_add((char *) "DBLATTR", fun_dblattr, 1, 2, FN_REG);
  function_add((char *) "DBHASATTR", fun_dbhasattr, 2, 2, FN_REG);
  function_add((char *) "DBQUOTA", fun_dbquota, 1, 2, FN_REG);
  function_add((char *) "DBSET", fun_dbset, 2, 3, FN_REG);
  function_add((char *) "DBU", fun_dbufun, 1, 11, FN_REG);
  function_add((char *) "DBUFUN", fun_dbufun, 1, 11, FN_REG);
  function_add((char *) "DBUSAGE", fun_dbusage, 1, 1, FN_REG);
  function_add((char *) "DBV", fun_dbv, 1, 1, FN_REG), */
  function_add((char *) "DEG2RAD", fun_deg2rad, 1, 1, FN_REG);
  function_add((char *) "FABS", fun_abs, 1, 1, FN_REG);
  function_add((char *) "FADD", fun_add, 2, INT_MAX, FN_REG);
  function_add((char *) "FALSE", fun_not, 1, 1, FN_REG);
  function_add((char *) "FCOMP", fun_comp, 2, 2, FN_REG);
  function_add((char *) "FMOD", fun_fmod, 2, 2, FN_REG);
  function_add((char *) "FMUL", fun_mul, 2, INT_MAX, FN_REG);
  function_add((char *) "FSGN", fun_sign, 1, 1, FN_REG);
  function_add((char *) "FSQRT", fun_sqrt, 1, 1, FN_REG);
  function_add((char *) "FSUB", fun_sub, 2, 2, FN_REG);
  function_add((char *) "IF", fun_t, 1, 1, FN_REG);
  function_add((char *) "IFELSE", fun_ifelse, 3, 3, FN_NOPARSE);
  function_add((char *) "IFTHEN", fun_ifthen, 2, 2, FN_NOPARSE);
  function_add((char *) "INV", fun_inv, 1, 1, FN_REG);
  function_add((char *) "INZONE", fun_inzone, 2, 2, FN_REG);
  function_add((char *) "ISA", fun_isa, 2, 2, FN_REG);
  function_add((char *) "LKIDS", fun_lkids, 1, 1, FN_REG);
  function_add((char *) "LNUMA", fun_lnuma, 1, 1, FN_REG);
  function_add((char *) "LOGB", fun_logb, 2, 2, FN_REG);
  function_add((char *) "LY2PC", fun_ly2pc, 1, 1, FN_REG);
  function_add((char *) "LY2SU", fun_ly2su, 1, 1, FN_REG);
  function_add((char *) "LZONE", fun_lzone, 1, 2, FN_REG);
  function_add((char *) "MAG", fun_mag, 3, 3, FN_REG);
  function_add((char *) "MAGAZINE", fun_mag, 3, 3, FN_REG);
  function_add((char *) "MASTERORG", fun_masterorg, 1, 2, FN_REG);
  function_add((char *) "NEG", fun_neg, 1, 1, FN_REG);
  function_add((char *) "ORG", fun_org, 1, 2, FN_REG);
  function_add((char *) "ORGNUM", fun_orgnum, 1, 1, FN_REG);
  function_add((char *) "PC2LY", fun_pc2ly, 1, 1, FN_REG);
  function_add((char *) "PC2SU", fun_pc2su, 1, 1, FN_REG);
  function_add((char *) "POSITION", fun_position, 1, 2, FN_REG);
  function_add((char *) "RACE", fun_race, 1, 2, FN_REG);
  function_add((char *) "RACENAME", fun_racename, 1, 1, FN_REG);
  function_add((char *) "RAD2DEG", fun_rad2deg, 1, 1, FN_REG);
  function_add((char *) "RANK", fun_rank, 1, 2, FN_REG);
  function_add((char *) "RANKNAME", fun_rankname, 1, 1, FN_REG);
  function_add((char *) "SD2SECS", fun_sd2secs, 1, 1, FN_REG);
  function_add((char *) "SDB", fun_sdb, 1, 7, FN_REG);
  function_add((char *) "SECS2SD", fun_secs2sd, 1, 1, FN_REG);
  function_add((char *) "SPACENUM", fun_spacenum, 1, 1, FN_REG);
  function_add((char *) "SPH2XYZ", fun_sph2xyz, 3, 3, FN_REG);
  function_add((char *) "SUM", fun_sum, 1, 2, FN_REG);
  function_add((char *) "SU2LY", fun_su2ly, 1, 1, FN_REG);
  function_add((char *) "SU2PC", fun_su2pc, 1, 1, FN_REG);
  function_add((char *) "TRUE", fun_t, 1, 1, FN_REG);
  function_add((char *) "TRUERACE", fun_truerace, 1, 2, FN_REG);
  function_add((char *) "VCROSS", fun_vcross, 2, 2, FN_REG);
  function_add((char *) "XYZ2SPH", fun_xyz2sph, 3, 3, FN_REG);
  function_add((char *) "ZWHO", fun_zwho, 1, 1, FN_REG);
#endif /* TREKMUSH */
}
