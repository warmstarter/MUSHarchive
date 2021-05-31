/* magazine.c 1.7.0p10 */

#include <math.h>

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

#include "ansi.h"
#include "match.h"
#include "magazine.h"

/* ------------------------------------------------------------------------ */

int fn_match (char *list, char *word)
{
	char *s, *r;

	s = trim_space_sep(list, ' ');
	do {
		r = split_token(&s, ' ');
		if (local_wild_match(word, r))
			return 1;
	} while (s);
	return 0;
}

int fn_passlock (dbref thing, dbref magazine)
{
	ATTR *a, *b;
	
	a = atr_get_noparent(magazine, ATR_LOCKREAD);
	if (a == NULL) {
		a = atr_get_noparent(magazine, ATR_LOCKORG);
		if (a == NULL) {
			return 1;
		} else {
			b = atr_get_noparent(thing, ATR_MASTERORG);
			if (b != NULL)
				if (fn_match(uncompress(a->value), uncompress(b->value)))
					return 1;
			b = atr_get_noparent(thing, ATR_ORGANIZATION);
			if (b != NULL)
				if (fn_match(uncompress(a->value), uncompress(b->value)))
					return 1;
		}
		return 0;
	} else
		return fn_match(uncompress(a->value), unparse_dbref(thing));
}

int fn_writelock (dbref thing, dbref magazine)
{
	ATTR *a;
	
	if (Wizard(thing)) {
		return 1;
	} else {
		a = atr_get_noparent(magazine, ATR_LOCKPOST);
		if (a == NULL) {
			return 1;
		} else {
			a = atr_get_noparent(magazine, ATR_LOCKWRITE);
			if (a == NULL) {
				return fn_passlock(thing, magazine);
			} else
				return fn_match(uncompress(a->value), unparse_dbref(thing));
		}
	}
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_mag) /* mag (<function>[,<field>[,<field>[,<field>[,<field>[,...]]]]]) */
{
	dbref thing, magazine;

	switch (args[0][0]) {
		case 'p': /* check passlock */
			if (!Wizard(executor)) {
				safe_str("#-1 PERMISSION DENIED", buff, bp); return; }
			thing = match_thing(executor, args[1]);
			if (thing == NOTHING) {
				safe_str("#-1 NO SUCH OBJECT", buff, bp); return; }
			magazine = match_thing(executor, args[2]);
			if (magazine == NOTHING) {
				safe_str("#-1 NO SUCH MAGAZINE", buff, bp); return; }
			safe_str(unparse_integer(fn_passlock(thing, magazine)), buff, bp);
			break;
		case 'w': /* check writelock */
			if (!Wizard(executor)) {
				safe_str("#-1 PERMISSION DENIED", buff, bp); return; }
			thing = match_thing(executor, args[1]);
			if (thing == NOTHING) {
				safe_str("#-1 NO SUCH OBJECT", buff, bp); return; }
			magazine = match_thing(executor, args[2]);
			if (magazine == NOTHING) {
				safe_str("#-1 NO SUCH MAGAZINE", buff, bp); return; }
			safe_str(unparse_integer(fn_writelock(thing, magazine)), buff, bp);
			break;
		default:
			safe_str("#-1 NO SUCH FIELD SELECTION", buff, bp);
			break;
	}
	return;
}

/* ------------------------------------------------------------------------ */
