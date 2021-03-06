/*
 * walkdb.c -- Support for commands that walk the entire db 
 */
/*
 * $Id: walkdb.c,v 1.1 2001/01/15 03:14:11 wenk Exp $ 
 */

#include "copyright.h"
#include "autoconf.h"

#include "mudconf.h"
#include "config.h"
#include "db.h"
#include "interface.h"
#include "match.h"
#include "externs.h"
#include "command.h"
#include "flags.h"
#include "powers.h"
#include "misc.h"
#include "alloc.h"

#ifdef MCHECK
#include "malloc.h"
#endif

/*
 * Bind occurances of the universal var in ACTION to ARG, then run ACTION.
 * * Cmds run in low-prio Q after a 1 sec delay for the first one. 
 */

static void bind_and_queue(player, cause, action, argstr, cargs, ncargs, number)
dbref player, cause;
int ncargs, number;
char *action, *argstr, *cargs[];
{
	char *command, *command2;	/*

					 * allocated by replace_string  
					 */

	command = replace_string(BOUND_VAR, argstr, action),
		command2 = replace_string(LISTPLACE_VAR, tprintf("%d", number),
					  command);
	wait_que(player, cause, 0, NOTHING, command2, cargs, ncargs,
		 mudstate.global_regs);
	free_lbuf(command);
	free_lbuf(command2);
}

/*
 * New @dolist.  i.e.:
 * * @dolist #12 #34 #45 #123 #34644=@emit [name(##)]
 * *
 * * New switches added 12/92, /space (default) delimits list using spaces,
 * * and /delimit allows specification of a delimiter.
 */

void do_dolist(player, cause, key, list, command, cargs, ncargs)
dbref player, cause;
int key, ncargs;
char *list, *command, *cargs[];
{
	char *curr, *objstring, delimiter = ' ';
	int number = 0;

	if (!list || *list == '\0') {
		notify(player,
		    "That's terrific, but what should I do with the list?");
		return;
	}
	curr = list;

	if (key == DOLIST_DELIMIT) {
		char *tempstr;

		if (strlen((tempstr = parse_to(&curr, ' ', EV_STRIP))) > 1) {
			notify(player, "The delimiter must be a single character!");
			return;
		}
		delimiter = *tempstr;
	}
	while (curr && *curr) {
		while (*curr == delimiter)
			curr++;
		if (*curr) {
			number++;
			objstring = parse_to(&curr, delimiter, EV_STRIP);
			bind_and_queue(player, cause, command, objstring,
				       cargs, ncargs, number);
		}
	}
}

/*
 * Regular @find command 
 */

void do_find(player, cause, key, name)
dbref player, cause;
int key;
char *name;
{
	dbref i, low_bound, high_bound;
	char *buff;

	if (!payfor(player, mudconf.searchcost)) {
		notify_quiet(player,
			     tprintf("You don't have enough %s.",
				     mudconf.many_coins));
		return;
	}
	parse_range(&name, &low_bound, &high_bound);
	for (i = low_bound; i <= high_bound; i++) {
#ifndef MEMORY_BASED
		if ((i % 100) == 0)
			cache_reset(0);
#endif /*
        * MEMORY_BASED 
        */
		if ((Typeof(i) != TYPE_EXIT) &&
		    controls(player, i) &&
		    (!*name || string_match(PureName(i), name))) {
			buff = unparse_object(player, i, 0);
			notify(player, buff);
			free_lbuf(buff);
		}
	}
	notify(player, "***End of List***");
}

/*
 * ---------------------------------------------------------------------------
 * * get_stats, do_stats: Get counts of items in the db.
 */

int get_stats(player, who, info)
dbref player, who;
STATS *info;
{
	dbref i;

	info->s_total = 0;
	info->s_rooms = 0;
	info->s_exits = 0;
	info->s_things = 0;
	info->s_players = 0;
	info->s_garbage = 0;

	/*
	 * Do we have permission? 
	 */

	if (Good_obj(who) && !Controls(player, who) && !Stat_Any(player)) {
		notify(player, "Permission denied.");
		return 0;
	}
	/*
	 * Can we afford it? 
	 */

	if (!payfor(player, mudconf.searchcost)) {
		notify(player, tprintf("You don't have enough %s.",
				       mudconf.many_coins));
		return 0;
	}
	DO_WHOLE_DB(i) {
		if ((who == NOTHING) || (who == Owner(i))) {
			info->s_total++;
			if (Going(i) && (Typeof(i) != TYPE_ROOM)) {
				info->s_garbage++;
				continue;
			}
			switch (Typeof(i)) {
			case TYPE_ROOM:
				info->s_rooms++;
				break;
			case TYPE_EXIT:
				info->s_exits++;
				break;
			case TYPE_THING:
				info->s_things++;
				break;
			case TYPE_PLAYER:
				info->s_players++;
				break;
			default:
				info->s_garbage++;
			}
		}
	}
	return 1;
}

void do_stats(player, cause, key, name)
dbref player, cause;
int key;
char *name;

/*
 * reworked by R'nice 
 */

{
	dbref owner;
	STATS statinfo;

	switch (key) {
	case STAT_ALL:
		owner = NOTHING;
		break;
	case STAT_ME:
		owner = Owner(player);
		break;
	case STAT_PLAYER:
		if (!(name && *name)) {
			notify(player,
			       tprintf("The universe contains %d objects.",
				       mudstate.db_top));
			return;
		}
		owner = lookup_player(player, name, 1);
		if (owner == NOTHING) {
			notify(player, "Not found.");
			return;
		}
		break;
	default:
		notify(player, "Illegal combination of switches.");
		return;
	}

	if (!get_stats(player, owner, &statinfo))
		return;
	notify(player,
	       tprintf("%d objects = %d rooms, %d exits, %d things, %d players. (%d garbage)",
		       statinfo.s_total, statinfo.s_rooms, statinfo.s_exits,
		       statinfo.s_things, statinfo.s_players,
		       statinfo.s_garbage));

#ifdef TEST_MALLOC
	if (Wizard(player))
		notify(player, tprintf("Malloc count = %d.", malloc_count));
#endif /*
        * TEST_MALLOC 
        */
#ifdef MCHECK
	if (Wizard(player)) {
		struct mstats mval;

		mval = mstats();
		notify(player, tprintf("Total size of the heap: %d",
				       mval.bytes_total));
		notify(player, tprintf("Chunks allocated: %d -- Total size of allocated chunks: %d",
				       mval.chunks_used, mval.bytes_used));
		notify(player, tprintf("Chunks free: %d -- Total size of free chunks: %d",
				       mval.chunks_free, mval.bytes_free));
	}
#endif /*
        * MCHECK 
        */
}

int chown_all(from_player, to_player)
dbref from_player, to_player;
{
	int i, count, quota_out, quota_in;

	if (Typeof(from_player) != TYPE_PLAYER)
		from_player = Owner(from_player);
	if (Typeof(to_player) != TYPE_PLAYER)
		to_player = Owner(to_player);
	count = 0;
	quota_out = 0;
	quota_in = 0;
	DO_WHOLE_DB(i) {
		if ((Owner(i) == from_player) && (Owner(i) != i)) {
			switch (Typeof(i)) {
			case TYPE_PLAYER:
				s_Owner(i, i);
				quota_out += mudconf.player_quota;
				break;
			case TYPE_THING:
				s_Owner(i, to_player);
				quota_out += mudconf.thing_quota;
				quota_in -= mudconf.thing_quota;
				break;
			case TYPE_ROOM:
				s_Owner(i, to_player);
				quota_out += mudconf.room_quota;
				quota_in -= mudconf.room_quota;
				break;
			case TYPE_EXIT:
				s_Owner(i, to_player);
				quota_out += mudconf.exit_quota;
				quota_in -= mudconf.exit_quota;
				break;
			default:
				s_Owner(i, to_player);
			}
			s_Flags(i, (Flags(i) & ~(CHOWN_OK | INHERIT)) | HALT);
			count++;
		}
	}
	add_quota(from_player, quota_out);
	add_quota(to_player, quota_in);
	return count;
}

void do_chownall(player, cause, key, from, to)
dbref player, cause;
int key;
char *from, *to;
{
	int count;
	dbref victim, recipient;

	init_match(player, from, TYPE_PLAYER);
	match_neighbor();
	match_absolute();
	match_player();
	if ((victim = noisy_match_result()) == NOTHING)
		return;

	if ((to != NULL) && *to) {
		init_match(player, to, TYPE_PLAYER);
		match_neighbor();
		match_absolute();
		match_player();
		if ((recipient = noisy_match_result()) == NOTHING)
			return;
	} else {
		recipient = player;
	}

	count = chown_all(victim, recipient);
	if (!Quiet(player)) {
		notify(player, tprintf("%d objects @chowned.", count));
	}
}

#define ANY_OWNER -2

void er_mark_disabled(player)
dbref player;
{
	notify(player,
	 "The mark commands are not allowed while DB cleaning is enabled.");
	notify(player,
	       "Use the '@disable cleaning' command to disable automatic cleaning.");
	notify(player,
	"Remember to '@unmark_all' before re-enabling automatic cleaning.");
}


/*
 * ---------------------------------------------------------------------------
 * * do_search: Walk the db reporting various things (or setting/clearing
 * * mark bits)
 */

int search_setup(player, searchfor, parm)
dbref player;
char *searchfor;
SEARCH *parm;
{
	char *pname, *searchtype, *t;
	int err;

	/*
	 * Crack arg into <pname> <type>=<targ>,<low>,<high> 
	 */

	pname = parse_to(&searchfor, '=', EV_STRIP_TS);
	if (!pname || !*pname) {
		pname = (char *)"me";
	} else
		for (t = pname; *t; t++) {
			if (isupper(*t))
				*t = tolower(*t);
		}

	if (searchfor && *searchfor) {
		searchtype = (char *)rindex(pname, ' ');
		if (searchtype) {
			*searchtype++ = '\0';
		} else {
			searchtype = pname;
			pname = (char *)"";
		}
	} else {
		searchtype = (char *)"";
	}

	/*
	 * If the player name is quoted, strip the quotes 
	 */

	if (*pname == '\"') {
		err = strlen(pname) - 1;
		if (pname[err] == '"') {
			pname[err] = '\0';
			pname++;
		}
	}
	/*
	 * Strip any range arguments 
	 */

	parse_range(&searchfor, &parm->low_bound, &parm->high_bound);

	/*
	 * set limits on who we search 
	 */

	parm->s_owner = Owner(player);
	parm->s_wizard = Search(player);
	parm->s_rst_owner = NOTHING;
	if (!*pname) {
		parm->s_rst_owner = parm->s_wizard ? ANY_OWNER : player;
	} else if (pname[0] == '#') {
		parm->s_rst_owner = atoi(&pname[1]);
		if (!Good_obj(parm->s_rst_owner))
			parm->s_rst_owner = NOTHING;
		else if (Typeof(parm->s_rst_owner) != TYPE_PLAYER)
			parm->s_rst_owner = NOTHING;

	} else if (strcmp(pname, "me") == 0) {
		parm->s_rst_owner = player;
	} else {
		parm->s_rst_owner = lookup_player(player, pname, 1);
	}

	if (parm->s_rst_owner == NOTHING) {
		notify(player, tprintf("%s: No such player", pname));
		return 0;
	}
	/*
	 * set limits on what we search for 
	 */

	err = 0;
	parm->s_rst_name = NULL;
	parm->s_rst_eval = NULL;
	parm->s_rst_type = NOTYPE;
	parm->s_parent = NOTHING;
	parm->s_zone = NOTHING;
	parm->s_fset.word1 = 0;
	parm->s_fset.word2 = 0;
	parm->s_fset.word3 = 0;
	parm->s_pset.word1 = 0;
	parm->s_pset.word2 = 0;

	switch (searchtype[0]) {
	case '\0':		/*
				 * the no class requested class  :)  
				 */
		break;
	case 'e':
		if (string_prefix("exits", searchtype)) {
			parm->s_rst_name = searchfor;
			parm->s_rst_type = TYPE_EXIT;
		} else if (string_prefix("evaluate", searchtype)) {
			parm->s_rst_eval = searchfor;
		} else if (string_prefix("eplayer", searchtype)) {
			parm->s_rst_type = TYPE_PLAYER;
			parm->s_rst_eval = searchfor;
		} else if (string_prefix("eroom", searchtype)) {
			parm->s_rst_type = TYPE_ROOM;
			parm->s_rst_eval = searchfor;
		} else if (string_prefix("eobject", searchtype)) {
			parm->s_rst_type = TYPE_THING;
			parm->s_rst_eval = searchfor;
		} else if (string_prefix("ething", searchtype)) {
			parm->s_rst_type = TYPE_THING;
			parm->s_rst_eval = searchfor;
		} else if (string_prefix("eexit", searchtype)) {
			parm->s_rst_type = TYPE_EXIT;
			parm->s_rst_eval = searchfor;
		} else {
			err = 1;
		}
		break;
	case 'f':
		if (string_prefix("flags", searchtype)) {

			/*
			 * convert_flags ignores previous values of flag_mask
			 * * * * * and s_rst_type while setting them 
			 */

			if (!convert_flags(player, searchfor, &parm->s_fset,
					   &parm->s_rst_type))
				return 0;
		} else {
			err = 1;
		}
		break;
	case 'n':
		if (string_prefix("name", searchtype)) {
			parm->s_rst_name = searchfor;
		} else {
			err = 1;
		}
		break;
	case 'o':
		if (string_prefix("objects", searchtype)) {
			parm->s_rst_name = searchfor;
			parm->s_rst_type = TYPE_THING;
		} else {
			err = 1;
		}
		break;
	case 'p':
		if (string_prefix("players", searchtype)) {
			parm->s_rst_name = searchfor;
			parm->s_rst_type = TYPE_PLAYER;
			if (!*pname)
				parm->s_rst_owner = ANY_OWNER;
		} else if (string_prefix("parent", searchtype)) {
			parm->s_parent = match_controlled(player, searchfor);
			if (!Good_obj(parm->s_parent))
				return 0;
			if (!*pname)
				parm->s_rst_owner = ANY_OWNER;
		} else if (string_prefix("power", searchtype)) {
			if (!decode_power(player, searchfor, &parm->s_pset))
				return 0;
		} else {
			err = 1;

		}
		break;
	case 'r':
		if (string_prefix("rooms", searchtype)) {
			parm->s_rst_name = searchfor;
			parm->s_rst_type = TYPE_ROOM;
		} else {
			err = 1;
		}
		break;
	case 't':
		if (string_prefix("type", searchtype)) {
			if (searchfor[0] == '\0')
				break;
			if (string_prefix("rooms", searchfor))
				parm->s_rst_type = TYPE_ROOM;
			else if (string_prefix("exits", searchfor))
				parm->s_rst_type = TYPE_EXIT;
			else if (string_prefix("objects", searchfor))
				parm->s_rst_type = TYPE_THING;
			else if (string_prefix("things", searchfor))
				parm->s_rst_type = TYPE_THING;
			else if (string_prefix("garbage", searchfor))
				parm->s_rst_type = TYPE_GARBAGE;
			else if (string_prefix("players", searchfor)) {
				parm->s_rst_type = TYPE_PLAYER;
				if (!*pname)
					parm->s_rst_owner = ANY_OWNER;
			} else {
				notify(player,
				       tprintf("%s: unknown type",
					       searchfor));
				return 0;
			}
		} else if (string_prefix("things", searchtype)) {
			parm->s_rst_name = searchfor;
			parm->s_rst_type = TYPE_THING;
		} else {
			err = 1;
		}
		break;
	case 'z':
		if (string_prefix("zone", searchtype)) {
			parm->s_zone = match_controlled(player, searchfor);
			if (!Good_obj(parm->s_zone))
				return 0;
			if (!*pname)
				parm->s_rst_owner = ANY_OWNER;
		} else {
			err = 1;

		}
		break;
	default:
		err = 1;
	}

	if (err) {
		notify(player, tprintf("%s: unknown class", searchtype));
		return 0;
	}
	/*
	 * Make sure player is authorized to do the search 
	 */

	if (!parm->s_wizard &&
	    (parm->s_rst_type != TYPE_PLAYER) &&
	    (parm->s_rst_owner != player) &&
	    (parm->s_rst_owner != ANY_OWNER)) {
		notify(player, "You need a search warrant to do that!");
		return 0;
	}
	/*
	 * make sure player has money to do the search 
	 */

	if (!payfor(player, mudconf.searchcost)) {
		notify(player,
		tprintf("You don't have enough %s to search. (You need %d)",
			mudconf.many_coins, mudconf.searchcost));
		return 0;
	}
	return 1;
}

void search_perform(player, cause, parm)
dbref player, cause;
SEARCH *parm;
{
	FLAG thing1flags, thing2flags, thing3flags;
	POWER thing1powers, thing2powers;
	dbref thing;
	char *buff, *buff2, *result, *bp, *str;
	int save_invk_ctr;

	buff = alloc_sbuf("search_perform.num");
	olist_init();
	save_invk_ctr = mudstate.func_invk_ctr;

	for (thing = parm->low_bound; thing <= parm->high_bound; thing++) {

#ifndef MEMORY_BASED
		if ((thing % 100) == 0) {
			cache_reset(0);
		}
#endif /*
        * MEMORY_BASED 
        */

		mudstate.func_invk_ctr = save_invk_ctr;

		/*
		 * Check for matching type 
		 */

		if ((parm->s_rst_type != NOTYPE) &&
		    (parm->s_rst_type != Typeof(thing)))
			continue;

		/*
		 * Check for matching owner 
		 */

		if ((parm->s_rst_owner != ANY_OWNER) &&
		    (parm->s_rst_owner != Owner(thing)))
			continue;

		/*
		 * Toss out destroyed things 
		 */

		thing1flags = Flags(thing);
		if ((Typeof(thing) == TYPE_THING) && (thing1flags & GOING))
			continue;

		/*
		 * Check for matching parent 
		 */

		if ((parm->s_parent != NOTHING) &&
		    (parm->s_parent != Parent(thing)))
			continue;

		/*
		 * Check for matching zone 
		 */

		if ((parm->s_zone != NOTHING) &&
		    (parm->s_zone != Zone(thing)))
			continue;

		/*
		 * Check for matching flags 
		 */

		thing3flags = Flags3(thing);
		thing2flags = Flags2(thing);
		if ((thing1flags & parm->s_fset.word1) != parm->s_fset.word1)
			continue;
		if ((thing2flags & parm->s_fset.word2) != parm->s_fset.word2)
			continue;
		if ((thing3flags & parm->s_fset.word3) != parm->s_fset.word3)
			continue;
			
		/*
		 * Check for matching power 
		 */

		thing1powers = Powers(thing);
		thing2powers = Powers2(thing);
		if ((thing1powers & parm->s_pset.word1) != parm->s_pset.word1)
			continue;
		if ((thing2powers & parm->s_pset.word2) != parm->s_pset.word2)
			continue;

		/*
		 * Check for matching name 
		 */

		if (parm->s_rst_name != NULL) {
			if (!string_prefix((char *)PureName(thing),
					   parm->s_rst_name))
				continue;
		}
		/*
		 * Check for successful evaluation 
		 */

		if (parm->s_rst_eval != NULL) {
			sprintf(buff, "#%d", thing);
			buff2 = replace_string(BOUND_VAR, buff,
					       parm->s_rst_eval);
			result = bp = alloc_lbuf("search_perform");
			str = buff2;
			exec(result, &bp, 0, player, cause,
			     EV_FCHECK | EV_EVAL | EV_NOTRACE, &str,
			     (char **)NULL, 0);
			*bp = '\0';
			free_lbuf(buff2);
			if (!*result || !xlate(result)) {
				free_lbuf(result);
				continue;
			}
			free_lbuf(result);
		}
		/*
		 * It passed everything.  Amazing. 
		 */

		olist_add(thing);
	}
	free_sbuf(buff);
	mudstate.func_invk_ctr = save_invk_ctr;
}

static void search_mark(player, key)
dbref player;
int key;
{
	dbref thing;
	int nchanged, is_marked;

	nchanged = 0;
	for (thing = olist_first(); thing != NOTHING; thing = olist_next()) {
		is_marked = Marked(thing);

		/*
		 * Don't bother checking if marking and already marked * (or
		 * * * * if unmarking and not marked) 
		 */

		if (((key == SRCH_MARK) && is_marked) ||
		    ((key == SRCH_UNMARK) && !is_marked))
			continue;
		/*
		 * Toggle the mark bit and update the counters 
		 */
		if (key == SRCH_MARK) {
			Mark(thing);
			nchanged++;
		} else {
			Unmark(thing);
			nchanged++;
		}
	}
	notify(player,
	       tprintf("%d objects %smarked",
		       nchanged, ((key == SRCH_MARK) ? "" : "un")));
	return;
}

void do_search(player, cause, key, arg)
dbref player, cause;
int key;
char *arg;
{
	int flag, destitute;
	char *buff, *outbuf, *bp;
	dbref thing, from, to;
	SEARCH searchparm;

	if ((key != SRCH_SEARCH) && (mudconf.control_flags & CF_DBCHECK)) {
		er_mark_disabled(player);
		return;
	}
	if (!search_setup(player, arg, &searchparm))
		return;
	search_perform(player, cause, &searchparm);
	destitute = 1;

	/*
	 * If we are doing a @mark command, handle that here. 
	 */

	if (key != SRCH_SEARCH) {
		search_mark(player, key);
		return;
	}
	outbuf = alloc_lbuf("do_search.outbuf");

	/*
	 * room search 
	 */
	if (searchparm.s_rst_type == TYPE_ROOM ||
	    searchparm.s_rst_type == NOTYPE) {
		flag = 1;
		for (thing = olist_first(); thing != NOTHING; thing = olist_next()) {
			if (Typeof(thing) != TYPE_ROOM)
				continue;
			if (flag) {
				flag = 0;
				destitute = 0;
				notify(player, "\nROOMS:");
			}
			buff = unparse_object(player, thing, 0);
			notify(player, buff);
			free_lbuf(buff);
		}
	}
	/*
	 * exit search 
	 */
	if (searchparm.s_rst_type == TYPE_EXIT ||
	    searchparm.s_rst_type == NOTYPE) {
		flag = 1;
		for (thing = olist_first(); thing != NOTHING; thing = olist_next()) {
			if (Typeof(thing) != TYPE_EXIT)
				continue;
			if (flag) {
				flag = 0;
				destitute = 0;
				notify(player, "\nEXITS:");
			}
			from = Exits(thing);
			to = Location(thing);

			bp = outbuf;
			buff = unparse_object(player, thing, 0);
			safe_str(buff, outbuf, &bp);
			free_lbuf(buff);

			safe_str((char *)" [from ", outbuf, &bp);
			buff = unparse_object(player, from, 0);
			safe_str(((from == NOTHING) ? "NOWHERE" : buff),
				 outbuf, &bp);
			free_lbuf(buff);

			safe_str((char *)" to ", outbuf, &bp);
			buff = unparse_object(player, to, 0);
			safe_str(((to == NOTHING) ? "NOWHERE" : buff),
				 outbuf, &bp);
			free_lbuf(buff);

			safe_chr(']', outbuf, &bp);
			*bp = '\0';
			notify(player, outbuf);
		}
	}
	/*
	 * object search 
	 */
	if (searchparm.s_rst_type == TYPE_THING ||
	    searchparm.s_rst_type == NOTYPE) {
		flag = 1;
		for (thing = olist_first(); thing != NOTHING; thing = olist_next()) {
			if (Typeof(thing) != TYPE_THING)
				continue;
			if (flag) {
				flag = 0;
				destitute = 0;
				notify(player, "\nOBJECTS:");
			}
			bp = outbuf;
			buff = unparse_object(player, thing, 0);
			safe_str(buff, outbuf, &bp);
			free_lbuf(buff);

			safe_str((char *)" [owner: ", outbuf, &bp);
			buff = unparse_object(player, Owner(thing), 0);
			safe_str(buff, outbuf, &bp);
			free_lbuf(buff);

			safe_chr(']', outbuf, &bp);
			*bp = '\0';
			notify(player, outbuf);
		}
	}
	/*
	 * garbage search 
	 */
	if (searchparm.s_rst_type == TYPE_GARBAGE ||
	    searchparm.s_rst_type == NOTYPE) {
		flag = 1;
		for (thing = olist_first(); thing != NOTHING; thing = olist_next()) {
			if (Typeof(thing) != TYPE_GARBAGE)
				continue;
			if (flag) {
				flag = 0;
				destitute = 0;
				notify(player, "\nGARBAGE:");
			}
			bp = outbuf;
			buff = unparse_object(player, thing, 0);
			safe_str(buff, outbuf, &bp);
			free_lbuf(buff);

			safe_str((char *)" [owner: ", outbuf, &bp);
			buff = unparse_object(player, Owner(thing), 0);
			safe_str(buff, outbuf, &bp);
			free_lbuf(buff);

			safe_chr(']', outbuf, &bp);
			*bp = '\0';
			notify(player, outbuf);
		}
	}
	/*
	 * player search 
	 */
	if (searchparm.s_rst_type == TYPE_PLAYER ||
	    searchparm.s_rst_type == NOTYPE) {
		flag = 1;
		for (thing = olist_first(); thing != NOTHING; thing = olist_next()) {
			if (Typeof(thing) != TYPE_PLAYER)
				continue;
			if (flag) {
				flag = 0;
				destitute = 0;
				notify(player, "\nPLAYERS:");
			}
			bp = outbuf;
			buff = unparse_object(player, thing, 0);
			safe_str(buff, outbuf, &bp);
			free_lbuf(buff);
			if (searchparm.s_wizard) {
				safe_str((char *)" [location: ",
					 outbuf, &bp);
				buff = unparse_object(player,
						      Location(thing), 0);
				safe_str(buff, outbuf, &bp);
				free_lbuf(buff);
				safe_chr(']', outbuf, &bp);
			}
			*bp = '\0';
			notify(player, outbuf);
		}
	}
	/*
	 * if nothing found matching search criteria 
	 */

	if (destitute) {
		notify(player, "Nothing found.");
	}
	free_lbuf(outbuf);
	olist_init();
}

/*
 * ---------------------------------------------------------------------------
 * * do_markall: set or clear the mark bits of all objects in the db.
 */

void do_markall(player, cause, key)
dbref player, cause;
int key;
{
	int i;

	if (mudconf.control_flags & CF_DBCHECK) {
		er_mark_disabled(player);
		return;
	}
	if (key == MARK_SET)
		Mark_all(i);
	else if (key == MARK_CLEAR)
		Unmark_all(i);
	if (!Quiet(player))
		notify(player, "Done.");
}

/*
 * ---------------------------------------------------------------------------
 * * do_apply_marked: Perform a command for each marked obj in the db.
 */

void do_apply_marked(player, cause, key, command, cargs, ncargs)
dbref player, cause;
int key, ncargs;
char *command, *cargs[];
{
	char *buff;
	int i;

	if (mudconf.control_flags & CF_DBCHECK) {
		er_mark_disabled(player);
		return;
	}
	buff = alloc_sbuf("do_apply_marked");
	DO_WHOLE_DB(i) {
		if (Marked(i)) {
			sprintf(buff, "#%d", i);
			bind_and_queue(player, cause, command, buff,
				       cargs, ncargs);
		}
	}
	free_sbuf(buff);
	if (!Quiet(player))
		notify(player, "Done.");
}

/*
 * ---------------------------------------------------------------------------
 * * olist_init, olist_add, olist_first, olist_next: Object list management
 * * routines.
 */

/*
 * olist_init: Clear and initialize the object list 
 */

void NDECL(olist_init)
{
	OBLOCK *op, *onext;

	for (op = mudstate.olist_head; op != NULL; op = onext) {
		onext = op->next;
		free_lbuf(op);
	}
	mudstate.olist_head = NULL;
	mudstate.olist_tail = NULL;
	mudstate.olist_cblock = NULL;
	mudstate.olist_count = 0;
	mudstate.olist_citm = 0;
}

/*
 * olist_add: Add an entry to the object list 
 */

void olist_add(item)
dbref item;
{
	OBLOCK *op;

	if (!mudstate.olist_head) {
		op = (OBLOCK *) alloc_lbuf("olist_add.first");
		mudstate.olist_head = mudstate.olist_tail = op;
		mudstate.olist_count = 0;
		op->next = NULL;
	} else if (mudstate.olist_count >= OBLOCK_SIZE) {
		op = (OBLOCK *) alloc_lbuf("olist_add.next");
		mudstate.olist_tail->next = op;
		mudstate.olist_tail = op;
		mudstate.olist_count = 0;
		op->next = NULL;
	} else {
		op = mudstate.olist_tail;
	}
	op->data[mudstate.olist_count++] = item;
}

/*
 * olist_first: Return the first entry in the object list 
 */

dbref NDECL(olist_first)
{
	if (!mudstate.olist_head)
		return NOTHING;
	if ((mudstate.olist_head == mudstate.olist_tail) &&
	    (mudstate.olist_count == 0))
		return NOTHING;
	mudstate.olist_cblock = mudstate.olist_head;
	mudstate.olist_citm = 0;
	return mudstate.olist_cblock->data[mudstate.olist_citm++];
}

dbref NDECL(olist_next)
{
	dbref thing;

	if (!mudstate.olist_cblock)
		return NOTHING;
	if ((mudstate.olist_cblock == mudstate.olist_tail) &&
	    (mudstate.olist_citm >= mudstate.olist_count))
		return NOTHING;
	thing = mudstate.olist_cblock->data[mudstate.olist_citm++];
	if (mudstate.olist_citm >= OBLOCK_SIZE) {
		mudstate.olist_cblock = mudstate.olist_cblock->next;
		mudstate.olist_citm = 0;
	}
	return thing;
}
