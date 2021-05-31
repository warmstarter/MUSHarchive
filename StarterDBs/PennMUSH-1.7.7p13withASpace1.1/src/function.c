/**
 * \file function.c
 *
 * \brief The function parser.
 *
 *
 */
#include "copyrite.h"

#include "config.h"
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "conf.h"
#include "attrib.h"
#include "dbdefs.h"
#include "externs.h"
#include "mushdb.h"
#include "function.h"
#include "match.h"
#include "htab.h"
#include "parse.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "lock.h"
#include "flags.h"
#include "game.h"
#include "mymalloc.h"
#include "funs.h"
#include "confmagic.h"

static void func_hash_insert(const char *name, FUN *func);
extern void local_functions(void);

USERFN_ENTRY *userfn_tab;
HASHTAB htab_function;
HASHTAB htab_user_function;

/* -------------------------------------------------------------------------*
 * Utilities.
 */

/** Save a copy of the q-registers.
 * \param funcname name of function calling (for memory leak testing)
 * \param preserve pointer to array to store the q-registers in.
 */
void
save_global_regs(const char *funcname, char *preserve[])
{
  int i;

  for (i = 0; i < NUMQ; i++) {
    if (!renv[i][0])
      preserve[i] = NULL;
    else {
      preserve[i] = (char *) mush_malloc(BUFFER_LEN, funcname);
      strcpy(preserve[i], renv[i]);
    }
  }
}

/** Restore the q-registers, freeing the storage array.
 * \param funcname name of function calling (for memory leak testing)
 * \param preserve pointer to array to restore the q-registers from.
 */
void
restore_global_regs(const char *funcname, char *preserve[])
{
  int i;
  for (i = 0; i < NUMQ; i++) {
    if (preserve[i]) {
      strcpy(renv[i], preserve[i]);
      mush_free(preserve[i], funcname);
    } else {
      renv[i][0] = '\0';
    }
  }
}

/** Restore the q-registers, without freeing the storage array.
 * \param preserve pointer to array to restore the q-registers from.
 */
void
load_global_regs(char *preserve[])
{
  int i;
  for (i = 0; i < NUMQ; i++) {
    if (preserve[i]) {
      strcpy(renv[i], preserve[i]);
    } else {
      renv[i][0] = '\0';
    }
  }
}

/** Save a copy of the environment (%0-%9)
 * \param funcname name of function calling (for memory leak testing)
 * \param preserve pointer to array to store %0-%9 in.
 */
void
save_global_env(const char *funcname __attribute__ ((__unused__)),
		char *preserve[])
{
  int i;
  for (i = 0; i < 10; i++)
    preserve[i] = wenv[i];
}

/** Restore the environment (%0-%9)
 * \param funcname name of function calling (for memory leak testing)
 * \param preserve pointer to array to restore %0-%9 from.
 */
void
restore_global_env(const char *funcname __attribute__ ((__unused__)),
		   char *preserve[])
{
  int i;
  for (i = 0; i < 10; i++)
    wenv[i] = preserve[i];
}

/** Check for a delimiter in an argument of a function call.
 * This function checks a given argument of a function call and sees
 * if it could be used as a delimiter. A delimiter must be a single
 * character. If the argument isn't present or is null, we return
 * the default delimiter, a space.
 * \param buff unused.
 * \param bp unused.
 * \param nfargs number of arguments to the function.
 * \param fargs array of function arguments.
 * \param sep_arg index of the argument to check for a delimiter.
 * \param set pointer to separator character, used to return separator.
 * \retval 0 illegal separator argument.
 * \retval 1 successfully returned a separator (maybe the default one).
 */
int
delim_check(char *buff, char **bp, int nfargs, char *fargs[], int sep_arg,
	    char *sep)
{
  /* Find a delimiter. */

  if (nfargs >= sep_arg) {
    if (!*fargs[sep_arg - 1])
      *sep = ' ';
    else if (strlen(fargs[sep_arg - 1]) != 1) {
      safe_str(T("#-1 SEPARATOR MUST BE ONE CHARACTER"), buff, bp);
      return 0;
    } else
      *sep = *fargs[sep_arg - 1];
  } else
    *sep = ' ';

  return 1;
}

/* --------------------------------------------------------------------------
 * The actual function handlers
 */

/** An entry in the function table.
 * This structure represents a function's entry in the function table.
 */
struct fun_tab {
  const char *name;	/**< Name of the function, uppercase. */
  function_func fun;	/**< Pointer to code to call for this function. */
  int minargs;	/**< Minimum args required. */
  int maxargs;	/**< Maximum args, or INT_MAX. If <0, last arg may have commas */
  int flags;	/**< Flags to control how the function is parsed. */
};

typedef struct fun_tab FUNTAB;

FUNTAB flist[] = {
  {"@@", fun_atat, 1, -1, FN_NOPARSE},
  {"ABS", fun_abs, 1, 1, FN_REG},
  {"ACCENT", fun_accent, 2, 2, FN_REG},
  {"ACCNAME", fun_accname, 1, 1, FN_REG},
  {"ADD", fun_add, 2, INT_MAX, FN_REG},
  {"AFTER", fun_after, 2, 2, FN_REG},
  {"ALPHAMAX", fun_alphamax, 1, INT_MAX, FN_REG},
  {"ALPHAMIN", fun_alphamin, 1, INT_MAX, FN_REG},
  {"AND", fun_and, 2, INT_MAX, FN_REG},
  {"ANDFLAGS", fun_andflags, 2, 2, FN_REG},
  {"ANDLFLAGS", fun_andlflags, 2, 2, FN_REG},
  {"ANSI", fun_ansi, 2, -2, FN_NOPARSE},
  {"APOSS", fun_aposs, 1, 1, FN_REG},
  {"ART", fun_art, 1, 1, FN_REG},
  {"ATRLOCK", fun_atrlock, 1, 2, FN_REG},
  {"BAND", fun_band, 1, INT_MAX, FN_REG},
  {"BEEP", fun_beep, 0, 1, FN_REG},
  {"BEFORE", fun_before, 2, 2, FN_REG},
  {"BNAND", fun_bnand, 2, 2, FN_REG},
  {"BNOT", fun_bnot, 1, 1, FN_REG},
  {"BOR", fun_bor, 1, INT_MAX, FN_REG},
  {"BOUND", fun_bound, 2, 3, FN_REG},
  {"BRACKETS", fun_brackets, 1, 1, FN_REG},
  {"BXOR", fun_bxor, 1, INT_MAX, FN_REG},
  {"CAND", fun_cand, 2, INT_MAX, FN_NOPARSE},
  {"CAPSTR", fun_capstr, 1, -1, FN_REG},
  {"CASE", fun_switch, 3, INT_MAX, FN_NOPARSE},
  {"CASEALL", fun_switch, 3, INT_MAX, FN_NOPARSE},
  {"CAT", fun_cat, 1, INT_MAX, FN_REG},
#ifdef CHAT_SYSTEM
  {"CEMIT", fun_cemit, 2, 3, FN_REG},
#endif
  {"CENTER", fun_center, 2, 3, FN_REG},
  {"CHILDREN", fun_lsearch, 1, 1, FN_REG},
  {"CHR", fun_chr, 1, 1, FN_REG},
#ifdef CHAT_SYSTEM
  {"CFLAGS", fun_cflags, 1, 2, FN_REG},
  {"CHANNELS", fun_channels, 0, 2, FN_REG},
#endif
  {"CHECKPASS", fun_checkpass, 2, 2, FN_REG | FN_WIZARD},
#ifdef CHAT_SYSTEM
  {"CLOCK", fun_clock, 1, 2, FN_REG},
#endif
  {"CLONE", fun_clone, 1, 1, FN_REG},
  {"COMP", fun_comp, 2, 3, FN_REG},
  {"CON", fun_con, 1, 1, FN_REG},
  {"CONFIG", fun_config, 1, 1, FN_REG},
  {"CONN", fun_conn, 1, 1, FN_REG},
  {"CONTROLS", fun_controls, 2, 2, FN_REG},
  {"CONVSECS", fun_convsecs, 1, 2, FN_REG},
  {"CONVUTCSECS", fun_convsecs, 1, 1, FN_REG},
  {"CONVTIME", fun_convtime, 1, 1, FN_REG},
  {"COR", fun_cor, 2, INT_MAX, FN_NOPARSE},
  {"CREATE", fun_create, 1, 2, FN_REG},
  {"CTIME", fun_ctime, 1, 1, FN_REG},
#ifdef CHAT_SYSTEM
  {"CTITLE", fun_ctitle, 2, 2, FN_REG},
  {"CWHO", fun_cwho, 1, 1, FN_REG},
#endif
  {"DEC", fun_dec, 1, 1, FN_REG},
  {"DECRYPT", fun_decrypt, 2, 2, FN_REG},
  {"DEFAULT", fun_default, 2, 2, FN_NOPARSE},
  {"DELETE", fun_delete, 3, 3, FN_REG},
  {"DIE", fun_die, 2, 3, FN_REG},
  {"DIG", fun_dig, 1, 3, FN_REG},
  {"DIST2D", fun_dist2d, 4, 4, FN_REG},
  {"DIST3D", fun_dist3d, 6, 6, FN_REG},
  {"DIV", fun_div, 2, 2, FN_REG},
  {"DOING", fun_doing, 1, 1, FN_REG},
  {"EDEFAULT", fun_edefault, 2, 2, FN_NOPARSE},
  {"EDIT", fun_edit, 3, INT_MAX, FN_REG},
  {"ELEMENT", fun_element, 3, 3, FN_REG},
  {"ELEMENTS", fun_elements, 2, 4, FN_REG},
  {"ELIST", fun_itemize, 1, 5, FN_REG},
  {"ELOCK", fun_elock, 2, 2, FN_REG},
  {"EMIT", fun_emit, 1, -1, FN_REG},
  {"ENCRYPT", fun_encrypt, 2, 2, FN_REG},
  {"ENTRANCES", fun_entrances, 0, 4, FN_REG},
  {"ETIMEFMT", fun_etimefmt, 2, 2, FN_REG},
  {"EQ", fun_eq, 2, 2, FN_REG},
  {"EVAL", fun_eval, 2, 2, FN_REG},
  {"ESCAPE", fun_escape, 1, -1, FN_REG},
  {"EXIT", fun_exit, 1, 1, FN_REG},
  {"EXTRACT", fun_extract, 3, 4, FN_REG},
  {"FILTER", fun_filter, 2, 4, FN_REG},
  {"FILTERBOOL", fun_filter, 2, 4, FN_REG},
  {"FINDABLE", fun_findable, 2, 2, FN_REG},
  {"FIRST", fun_first, 1, 2, FN_REG},
  {"FLAGS", fun_flags, 0, 1, FN_REG},
  {"FLIP", fun_flip, 1, 1, FN_REG},
  {"FLOORDIV", fun_floordiv, 2, 2, FN_REG},
  {"FOLD", fun_fold, 2, 4, FN_REG},
#ifdef USE_MAILER
  {"FOLDERSTATS", fun_folderstats, 0, 2, FN_REG},
#endif
  {"FOLLOWERS", fun_followers, 1, 1, FN_REG},
  {"FOLLOWING", fun_following, 1, 1, FN_REG},
  {"FOREACH", fun_foreach, 2, 4, FN_REG},
  {"FUNCTIONS", fun_functions, 0, 0, FN_REG},
  {"FULLNAME", fun_fullname, 1, 1, FN_REG},
  {"GET", fun_get, 1, 1, FN_REG},
  {"GET_EVAL", fun_get_eval, 1, 1, FN_REG},
  {"GRAB", fun_grab, 2, 3, FN_REG},
  {"GRABALL", fun_graball, 2, 4, FN_REG},
  {"GREP", fun_grep, 3, 3, FN_REG},
  {"GREPI", fun_grep, 3, 3, FN_REG},
  {"GT", fun_gt, 2, 2, FN_REG},
  {"GTE", fun_gte, 2, 2, FN_REG},
  {"HASATTR", fun_hasattr, 2, 2, FN_REG},
  {"HASATTRP", fun_hasattr, 2, 2, FN_REG},
  {"HASATTRPVAL", fun_hasattr, 2, 2, FN_REG},
  {"HASATTRVAL", fun_hasattr, 2, 2, FN_REG},
  {"HASFLAG", fun_hasflag, 2, 2, FN_REG},
  {"HASPOWER", fun_haspower, 2, 2, FN_REG},
  {"HASTYPE", fun_hastype, 2, 2, FN_REG},
  {"HEIGHT", fun_height, 1, 1, FN_REG},
  {"HIDDEN", fun_hidden, 1, 1, FN_REG},
  {"HOME", fun_home, 1, 1, FN_REG},
  {"IDLE", fun_idlesecs, 1, 1, FN_REG},
  {"IDLESECS", fun_idlesecs, 1, 1, FN_REG},
  {"IF", fun_if, 2, 3, FN_NOPARSE},
  {"IFELSE", fun_if, 3, 3, FN_NOPARSE},
  {"ILEV", fun_ilev, 0, 0, FN_REG},
  {"INAME", fun_iname, 1, 1, FN_REG},
  {"INC", fun_inc, 1, 1, FN_REG},
  {"INDEX", fun_index, 4, 4, FN_REG},
  {"INSERT", fun_insert, 3, 4, FN_REG},
  {"INUM", fun_inum, 1, 1, FN_REG},
  {"ISDAYLIGHT", fun_isdaylight, 0, 0, FN_REG},
  {"ISDBREF", fun_isdbref, 1, 1, FN_REG},
  {"ISINT", fun_isint, 1, 1, FN_REG},
  {"ISNUM", fun_isnum, 1, 1, FN_REG},
  {"ISWORD", fun_isword, 1, 1, FN_REG},
  {"ITER", fun_iter, 2, 4, FN_NOPARSE},
  {"ITEMS", fun_items, 2, 2, FN_REG},
  {"ITEMIZE", fun_itemize, 1, 4, FN_REG},
  {"ITEXT", fun_itext, 1, 1, FN_REG},
  {"LAST", fun_last, 1, 2, FN_REG},
  {"LATTR", fun_lattr, 1, 1, FN_REG},
  {"LCON", fun_lcon, 1, 1, FN_REG},
  {"LCSTR", fun_lcstr, 1, -1, FN_REG},
  {"LDELETE", fun_ldelete, 2, 3, FN_REG},
  {"LEFT", fun_left, 2, 2, FN_REG},
  {"LEMIT", fun_lemit, 1, -1, FN_REG},
  {"LEXITS", fun_lexits, 1, 1, FN_REG},
  {"LFLAGS", fun_lflags, 0, 1, FN_REG},
  {"LINK", fun_link, 2, 2, FN_REG},
  {"LIST", fun_list, 1, 1, FN_REG},
  {"LIT", fun_lit, 1, -1, FN_LITERAL},
  {"LJUST", fun_ljust, 2, 3, FN_REG},
  {"LMATH", fun_lmath, 2, 3, FN_REG},
  {"LNUM", fun_lnum, 1, 3, FN_REG},
  {"LOC", fun_loc, 1, 1, FN_REG},
  {"LOCALIZE", fun_localize, 1, 1, FN_NOPARSE},
  {"LOCATE", fun_locate, 3, 3, FN_REG},
  {"LOCK", fun_lock, 1, 2, FN_REG},
  {"LPARENT", fun_lparent, 1, 1, FN_REG},
  {"LPLAYERS", fun_lplayers, 1, 1, FN_REG},
  {"LPOS", fun_lpos, 2, 2, FN_REG},
  {"LSEARCH", fun_lsearch, 1, 5, FN_REG},
  {"LSEARCHR", fun_lsearch, 1, 5, FN_REG},
  {"LSTATS", fun_lstats, 0, 1, FN_REG},
  {"LT", fun_lt, 2, 2, FN_REG},
  {"LTE", fun_lte, 2, 2, FN_REG},
  {"LVCON", fun_lvcon, 1, 1, FN_REG},
  {"LVEXITS", fun_lvexits, 1, 1, FN_REG},
  {"LVPLAYERS", fun_lvplayers, 1, 1, FN_REG},
  {"LWHO", fun_lwho, 0, 0, FN_REG},
#ifdef USE_MAILER
  {"MAIL", fun_mail, 0, 2, FN_REG},
  {"MAILFROM", fun_mailfrom, 1, 2, FN_REG},
  {"MAILSTATS", fun_mailstats, 1, 1, FN_REG},
  {"MAILDSTATS", fun_mailstats, 1, 1, FN_REG},
  {"MAILFSTATS", fun_mailstats, 1, 1, FN_REG},
  {"MAILSTATUS", fun_mailstatus, 1, 2, FN_REG},
  {"MAILSUBJECT", fun_mailsubject, 1, 2, FN_REG},
  {"MAILTIME", fun_mailtime, 1, 2, FN_REG},
#endif
  {"MAP", fun_map, 2, 4, FN_REG},
  {"MATCH", fun_match, 2, 3, FN_REG},
  {"MATCHALL", fun_matchall, 2, 4, FN_REG},
  {"MAX", fun_max, 1, INT_MAX, FN_REG},
  {"MEAN", fun_mean, 1, INT_MAX, FN_REG},
  {"MEDIAN", fun_median, 1, INT_MAX, FN_REG},
  {"MEMBER", fun_member, 2, 3, FN_REG},
  {"MERGE", fun_merge, 3, 3, FN_REG},
  {"MID", fun_mid, 3, 3, FN_REG},
  {"MIN", fun_min, 1, INT_MAX, FN_REG},
  {"MIX", fun_mix, 3, 12, FN_REG},
  {"MODULO", fun_modulo, 2, 2, FN_REG},
  {"MONEY", fun_money, 1, 1, FN_REG},
  {"MTIME", fun_mtime, 1, 1, FN_REG},
  {"MUDNAME", fun_mudname, 0, 0, FN_REG},
  {"MUL", fun_mul, 2, INT_MAX, FN_REG},
  {"MUNGE", fun_munge, 3, 5, FN_REG},
  {"MWHO", fun_lwho, 0, 0, FN_REG},
  {"NAME", fun_name, 0, 2, FN_REG},
  {"NAND", fun_nand, 1, INT_MAX, FN_REG},
  {"NATTR", fun_nattr, 1, 1, FN_REG},
  {"NEARBY", fun_nearby, 2, 2, FN_REG},
  {"NEQ", fun_neq, 2, 2, FN_REG},
  {"NEXT", fun_next, 1, 1, FN_REG},
  {"NOR", fun_nor, 1, INT_MAX, FN_REG},
  {"NOT", fun_not, 1, 1, FN_REG},
  {"NSPEMIT", fun_nspemit, 2, 2, FN_REG},
  {"NUM", fun_num, 1, 1, FN_REG},
  {"NULL", fun_null, 1, 1, FN_REG},
  {"OBJ", fun_obj, 1, 1, FN_REG},
  {"OBJEVAL", fun_objeval, 2, -2, FN_NOPARSE},
  {"OBJMEM", fun_objmem, 1, 1, FN_REG},
  {"OEMIT", fun_oemit, 2, -2, FN_REG},
  {"OPEN", fun_open, 2, 2, FN_REG},
  {"OR", fun_or, 2, INT_MAX, FN_REG},
  {"ORD", fun_ord, 1, 1, FN_REG},
  {"ORFLAGS", fun_orflags, 2, 2, FN_REG},
  {"ORLFLAGS", fun_orlflags, 2, 2, FN_REG},
  {"OWNER", fun_owner, 1, 1, FN_REG},
  {"PARENT", fun_parent, 1, 2, FN_REG},
  {"PCREATE", fun_pcreate, 1, 2, FN_REG},
  {"PEMIT", fun_pemit, 2, -2, FN_REG},
  {"PLAYERMEM", fun_playermem, 1, 1, FN_REG},
  {"PMATCH", fun_pmatch, 1, 1, FN_REG},
  {"POLL", fun_poll, 0, 0, FN_REG},
  {"PORTS", fun_ports, 1, 1, FN_REG},
  {"POS", fun_pos, 2, 2, FN_REG},
  {"POSS", fun_poss, 1, 1, FN_REG},
  {"POWERS", fun_powers, 1, 2, FN_REG},
  {"PUEBLO", fun_pueblo, 1, 1, FN_REG},
#ifdef QUOTA
  {"QUOTA", fun_quota, 1, 1, FN_REG},
#endif
  {"R", fun_r, 1, 1, FN_REG},
  {"RAND", fun_rand, 1, 1, FN_REG},
  {"REGEDIT", fun_regreplace, 3, INT_MAX, FN_NOPARSE},
  {"REGEDITALL", fun_regreplace, 3, INT_MAX, FN_NOPARSE},
  {"REGEDITALLI", fun_regreplace, 3, INT_MAX, FN_NOPARSE},
  {"REGEDITI", fun_regreplace, 3, INT_MAX, FN_NOPARSE},
  {"REGMATCH", fun_regmatch, 2, 3, FN_REG},
  {"REGMATCHI", fun_regmatch, 2, 3, FN_REG},
  {"REGRAB", fun_regrab, 2, 4, FN_REG},
  {"REGRABALL", fun_regrab, 2, 4, FN_REG},
  {"REGRABALLI", fun_regrab, 2, 3, FN_REG},
  {"REGRABI", fun_regrab, 2, 3, FN_REG},
  {"REGREP", fun_regrep, 3, 3, FN_REG},
  {"REGREPI", fun_regrep, 3, 3, FN_REG},
  {"REMAINDER", fun_remainder, 2, 2, FN_REG},
  {"REMIT", fun_remit, 2, -2, FN_REG},
  {"REMOVE", fun_remove, 2, 3, FN_REG},
  {"REPEAT", fun_repeat, 2, 2, FN_REG},
  {"REPLACE", fun_replace, 3, 4, FN_REG},
  {"REST", fun_rest, 1, 2, FN_REG},
  {"RESTARTS", fun_restarts, 0, 0, FN_REG},
  {"RESTARTTIME", fun_restarttime, 0, 0, FN_REG},
  {"REVERSE", fun_flip, 1, 1, FN_REG},
  {"REVWORDS", fun_revwords, 1, 3, FN_REG},
  {"RIGHT", fun_right, 2, 2, FN_REG},
  {"RJUST", fun_rjust, 2, 3, FN_REG},
  {"RLOC", fun_rloc, 2, 2, FN_REG},
  {"RNUM", fun_rnum, 2, 2, FN_REG},
  {"ROOM", fun_room, 1, 1, FN_REG},
  {"S", fun_s, 1, -1, FN_REG},
  {"SCAN", fun_scan, 2, -2, FN_REG},
  {"SCRAMBLE", fun_scramble, 1, -1, FN_REG},
  {"SECS", fun_secs, 0, 0, FN_REG},
  {"SECURE", fun_secure, 1, -1, FN_REG},
  {"SET", fun_set, 2, 2, FN_REG},
  {"SETQ", fun_setq, 2, 2, FN_REG},
  {"SETR", fun_setq, 2, 2, FN_REG},
  {"SETDIFF", fun_setdiff, 2, 5, FN_REG},
  {"SETINTER", fun_setinter, 2, 5, FN_REG},
  {"SETUNION", fun_setunion, 2, 5, FN_REG},
  {"SHA1", fun_sha1, 1, 1, FN_REG},
  {"SHL", fun_shl, 2, 2, FN_REG},
  {"SHR", fun_shr, 2, 2, FN_REG},
  {"SHUFFLE", fun_shuffle, 1, 3, FN_REG},
  {"SIGN", fun_sign, 1, 1, FN_REG},
  {"SORT", fun_sort, 1, 4, FN_REG},
  {"SORTBY", fun_sortby, 2, 4, FN_REG},
  {"SOUNDEX", fun_soundex, 1, 1, FN_REG},
  {"SOUNDSLIKE", fun_soundlike, 2, 2, FN_REG},
  {"SPACE", fun_space, 1, 1, FN_REG},
  {"SPELLNUM", fun_spellnum, 1, 1, FN_REG},
  {"SPLICE", fun_splice, 3, 4, FN_REG},
  {"SQUISH", fun_squish, 1, 2, FN_REG},
  {"STARTTIME", fun_starttime, 0, 0, FN_REG},
  {"STEP", fun_step, 3, 5, FN_REG},
  {"STRCAT", fun_strcat, 1, INT_MAX, FN_REG},
  {"STRINSERT", fun_strinsert, 3, -3, FN_REG},
  {"STRIPACCENTS", fun_stripaccents, 1, 1, FN_REG},
  {"STRIPANSI", fun_stripansi, 1, -1, FN_REG},
  {"STRLEN", fun_strlen, 1, -1, FN_REG},
  {"STRMATCH", fun_strmatch, 2, 2, FN_REG},
  {"SUB", fun_sub, 2, 2, FN_REG},
  {"SUBJ", fun_subj, 1, 1, FN_REG},
  {"SWITCH", fun_switch, 3, INT_MAX, FN_NOPARSE},
  {"SWITCHALL", fun_switch, 3, INT_MAX, FN_NOPARSE},
  {"T", fun_t, 1, 1, FN_REG},
  {"TABLE", fun_table, 1, 5, FN_REG},
  {"TEL", fun_tel, 2, 3, FN_REG},
  {"TEXTFILE", fun_textfile, 2, 2, FN_REG},
  {"TIME", fun_time, 0, 1, FN_REG},
  {"TIMEFMT", fun_timefmt, 1, 2, FN_REG},
  {"TIMESTRING", fun_timestring, 1, 2, FN_REG},
  {"TRIM", fun_trim, 1, 3, FN_REG},
  {"TRUNC", fun_trunc, 1, 1, FN_REG},
  {"TYPE", fun_type, 1, 1, FN_REG},
  {"UCSTR", fun_ucstr, 1, -1, FN_REG},
  {"UDEFAULT", fun_uldefault, 2, 12, FN_NOPARSE},
  {"UFUN", fun_ufun, 1, 11, FN_REG},
  {"ULDEFAULT", fun_uldefault, 1, 12, FN_NOPARSE},
  {"ULOCAL", fun_ulocal, 1, 11, FN_REG},
  {"UTCTIME", fun_time, 0, 0, FN_REG},
  {"U", fun_ufun, 1, 11, FN_REG},
  {"V", fun_v, 1, 1, FN_REG},
  {"VALID", fun_valid, 2, 2, FN_REG},
  {"VERSION", fun_version, 0, 0, FN_REG},
  {"VISIBLE", fun_visible, 2, 2, FN_REG},
  {"WHERE", fun_where, 1, 1, FN_REG},
  {"WIDTH", fun_width, 1, 1, FN_REG},
  {"WIPE", fun_wipe, 1, 1, FN_REG},
  {"WORDPOS", fun_wordpos, 2, 3, FN_REG},
  {"WORDS", fun_words, 1, 2, FN_REG},
  {"WRAP", fun_wrap, 2, 4, FN_REG},
  {"XGET", fun_xget, 2, 2, FN_REG},
  {"XOR", fun_xor, 2, INT_MAX, FN_REG},
  {"ZEMIT", fun_zemit, 2, -2, FN_REG},
  {"ZFUN", fun_zfun, 1, 11, FN_REG},
  {"ZONE", fun_zone, 1, 2, FN_REG},
  {"VADD", fun_vadd, 2, 3, FN_REG},
  {"VSUB", fun_vsub, 2, 3, FN_REG},
  {"VMAX", fun_vmax, 2, 3, FN_REG},
  {"VMIN", fun_vmin, 2, 3, FN_REG},
  {"VMUL", fun_vmul, 2, 3, FN_REG},
  {"VDOT", fun_vdot, 2, 3, FN_REG},
  {"VMAG", fun_vmag, 1, 2, FN_REG},
  {"VDIM", fun_words, 1, 2, FN_REG},
  {"VUNIT", fun_vunit, 1, 2, FN_REG},
  {"ACOS", fun_acos, 1, 2, FN_REG},
  {"ASIN", fun_asin, 1, 2, FN_REG},
  {"ATAN", fun_atan, 1, 2, FN_REG},
  {"ATAN2", fun_atan2, 2, 3, FN_REG},
  {"CEIL", fun_ceil, 1, 1, FN_REG},
  {"COS", fun_cos, 1, 2, FN_REG},
  {"CTU", fun_ctu, 3, 3, FN_REG},
  {"E", fun_e, 0, 0, FN_REG},
  {"EXP", fun_exp, 1, 1, FN_REG},
  {"FDIV", fun_fdiv, 2, 2, FN_REG},
  {"FMOD", fun_fmod, 2, 2, FN_REG},
  {"FLOOR", fun_floor, 1, 1, FN_REG},
  {"LOG", fun_log, 1, 2, FN_REG},
  {"LN", fun_ln, 1, 1, FN_REG},
  {"PI", fun_pi, 0, 0, FN_REG},
  {"POWER", fun_power, 2, 2, FN_REG},
  {"ROUND", fun_round, 2, 2, FN_REG},
  {"SIN", fun_sin, 1, 2, FN_REG},
  {"SQRT", fun_sqrt, 1, 1, FN_REG},
  {"STDDEV", fun_stddev, 1, INT_MAX, FN_REG},
  {"TAN", fun_tan, 1, 2, FN_REG},
  {"HTML", fun_html, 1, 1, FN_REG},
  {"TAG", fun_tag, 1, INT_MAX, FN_REG},
  {"ENDTAG", fun_endtag, 1, 1, FN_REG},
  {"TAGWRAP", fun_tagwrap, 2, 3, FN_REG},
  {NULL, NULL, 0, 0, 0}
};

/** List all functions.
 * \verbatim
 * This is the mail interface to @list functions.
 * \endverbatim
 * \param player the enactor.
 * \param lc if 1, return functions in lowercase.
 */
void
do_list_functions(dbref player, int lc)
{
  /* lists all built-in functions. */
  char *b = list_functions();
  notify_format(player, "Functions: %s", lc ? strlower(b) : b);
}

/** Return a list of function names.
 * This function returns the list of function names as a string.
 * \return list of function names as a static string.
 */
char *
list_functions(void)
{
  FUN *fp;
  const char *ptrs[BUFFER_LEN / 2];
  static char buff[BUFFER_LEN];
  char *bp;
  int nptrs = 0, i;
  for (fp = (FUN *) hash_firstentry(&htab_function);
       fp; fp = (FUN *) hash_nextentry(&htab_function)) {
    if (fp->flags & FN_OVERRIDE)
      continue;
    ptrs[nptrs++] = fp->name;
  }
  fp = (FUN *) hash_firstentry(&htab_user_function);
  while (fp) {
    ptrs[nptrs++] = fp->name;
    fp = (FUN *) hash_nextentry(&htab_user_function);
  }
  do_gensort((char **) ptrs, nptrs, 0);
  bp = buff;
  safe_str(ptrs[0], buff, &bp);
  for (i = 1; i < nptrs; i++) {
    safe_chr(' ', buff, &bp);
    safe_str(ptrs[i], buff, &bp);
  }
  *bp = '\0';
  return buff;
}

/*---------------------------------------------------------------------------
 * Hashed function table stuff
 */


/** Look up a function by name.
 * \param name name of function to look up.
 * \return pointer to function data, or NULL.
 */
FUN *
func_hash_lookup(const char *name)
{
  FUN *f;
  f = (FUN *) hashfind(strupper(name), &htab_function);
  if (!f || f->flags & FN_OVERRIDE)
    f = (FUN *) hashfind(strupper(name), &htab_user_function);
  return f;
}

static void
func_hash_insert(name, func)
    const char *name;
    FUN *func;
{
  hashadd(name, (void *) func, &htab_function);
}

/** Initialize the function hash table.
 */
void
init_func_hashtab(void)
{
  FUNTAB *ftp;

  hashinit(&htab_function, 512, sizeof(FUN));
  hashinit(&htab_user_function, 32, sizeof(FUN));
  for (ftp = flist; ftp->name; ftp++) {
    function_add(ftp->name, ftp->fun, ftp->minargs, ftp->maxargs, ftp->flags);
  }
  local_functions();
}

/** Function initization to perform after reading the config file.
 * This function performs post-config initialization. Specifically,
 * we need the max_globals value from the config file before we
 * can allocate the global user function table here.
 */
void
function_init_postconfig(void)
{
  userfn_tab =
    (USERFN_ENTRY *) mush_malloc(MAX_GLOBAL_FNS * sizeof(USERFN_ENTRY),
				 "userfn_tab");
}

/** Check permissions to run a function.
 * \param player the executor.
 * \param fp pointer to function data.
 * \retval 1 executor may use the function.
 * \retval 0 permission denied.
 */
int
check_func(dbref player, FUN *fp)
{
  if (!fp)
    return 0;
  if ((fp->flags & (~FN_ARG_MASK)) == 0)
    return 1;
  if (fp->flags & FN_DISABLED)
    return 0;
  if ((fp->flags & FN_GOD) && !God(player))
    return 0;
  if ((fp->flags & FN_WIZARD) && !Wizard(player))
    return 0;
  if ((fp->flags & FN_ADMIN) && !Hasprivs(player))
    return 0;
  if ((fp->flags & FN_NOGAGGED) && Gagged(player))
    return 0;
  if ((fp->flags & FN_NOFIXED) && Fixed(player))
    return 0;
  if ((fp->flags & FN_NOGUEST) && Guest(player))
    return 0;
  return 1;
}

/** Add an alias to a function.
 * This function adds an alias to a function in the hash table.
 * \param function name of function to alias.
 * \param function name of the alias to add.
 * \retval 0 failure (alias exists, or function doesn't, or is a user fun).
 * \retval 1 success.
 */
int
alias_function(const char *function, const char *alias)
{
  FUN *fp;

  /* Make sure the alias doesn't exist already */
  if (func_hash_lookup(alias))
    return 0;

  /* Look up the original */
  fp = func_hash_lookup(function);
  if (!fp)
    return 0;

  /* We can't alias @functions. Just use another @function for these */
  if (!(fp->flags & FN_BUILTIN))
    return 0;

  function_add(strdup(strupper(alias)), fp->where.fun,
	       fp->minargs, fp->maxargs, fp->flags);
  return 1;
}

/** Add a function.
 * \param name name of the function to add.
 * \param fun pointer to compiled function code.
 * \param minargs minimum arguments to function.
 * \param maxargs maximum arguments to function.
 * \param ftype function evaluation flags.
 */
void
function_add(const char *name, function_func fun, int minargs, int maxargs,
	     int ftype)
{
  FUN *fp;
  fp = (FUN *) mush_malloc(sizeof(FUN), "function");
  memset(fp, 0, sizeof(FUN));
  fp->name = name;
  fp->where.fun = fun;
  fp->minargs = minargs;
  fp->maxargs = maxargs;
  fp->flags = FN_BUILTIN | ftype;
  func_hash_insert(name, fp);
}

/*-------------------------------------------------------------------------
 * Function handlers and the other good stuff. Almost all this code is
 * a modification of TinyMUSH 2.0 code.
 */

/** Strip a level of braces.
 * this is a hack which just strips a level of braces. It malloc()s memory
 * which must be free()d later.
 * \param str string to strip braces from.
 * \return newly allocated string with the first level of braces stripped.
 */
char *
strip_braces(const char *str)
{
  char *buff;
  char *bufc;

  buff = (char *) mush_malloc(BUFFER_LEN, "strip_braces.buff");
  bufc = buff;

  while (isspace((unsigned char) *str))	/* eat spaces at the beginning */
    str++;

  switch (*str) {
  case '{':
    str++;
    process_expression(buff, &bufc, &str, 0, 0, 0, PE_NOTHING, PT_BRACE, NULL);
    *bufc = '\0';
    return buff;
    break;			/* NOT REACHED */
  default:
    strcpy(buff, str);
    return buff;
  }
}

/*------------------------------------------------------------------------
 * User-defined global function handlers 
 */


static Size_t userfn_count = 0;

/** Given a function name and a restriction, apply the restriction to the
 * function in addition to whatever its usual restrictions are.
 * This is used by the configuration file startup in conf.c
 * \verbatim
 * Valid restrictions are:
 *   nobody     disable the command
 *   nogagged   can't be used by gagged players
 *   nofixed    can't be used by fixed players
 *   noguest    can't be used by guests
 *   admin      can only be used by royalty or wizards
 *   wizard     can only be used by wizards
 *   god        can only be used by god
 *   noplayer   can't be used by players, just objects/rooms/exits
 *   nosidefx   can't be used to do side-effect thingies
 * \endverbatim
 * \param name name of function to restrict.
 * \param restriction name of restriction to apply to function.
 * \retval 1 success.
 * \retval 0 failure (invalid function or restriction name).
 */
int
restrict_function(const char *name, const char *restriction)
{
  FUN *fp;
  int clear = 0;
  unsigned int flags = 0;

  if (!name || !*name)
    return 0;
  fp = func_hash_lookup(name);
  if (!fp)
    return 0;
  if (!restriction || !*restriction)
    return 0;
  if (*restriction == '!') {
    restriction++;
    clear = 1;
  }
  if (!strcasecmp(restriction, "nobody")) {
    flags |= FN_DISABLED;
  } else if (string_prefix(restriction, "nogag")) {
    flags |= FN_NOGAGGED;
  } else if (string_prefix(restriction, "nofix")) {
    flags |= FN_NOFIXED;
  } else if (!strcasecmp(restriction, "noguest")) {
    flags |= FN_NOGUEST;
  } else if (!strcasecmp(restriction, "admin")) {
    flags |= FN_ADMIN;
  } else if (!strcasecmp(restriction, "wizard")) {
    flags |= FN_WIZARD;
  } else if (!strcasecmp(restriction, "god")) {
    flags |= FN_GOD;
  } else if (!strcasecmp(restriction, "nosidefx")) {
    flags |= FN_NOSIDEFX;
  } else {
    return 0;
  }
  if (clear)
    fp->flags &= ~flags;
  else
    fp->flags |= flags;
  return 1;
}

/** Softcode interface to restrict a function.
 * \verbatim
 * This is the implementation of @function/restrict.
 * \endverbatim
 * \param player the enactor.
 * \param name name of function to restrict.
 * \param restriction name of restriction to add.
 */
void
do_function_restrict(dbref player, const char *name, const char *restriction)
{
  if (!Wizard(player)) {
    notify(player, T("Permission denied."));
    return;
  }
  if (!name) {
    notify(player, T("Restrict what function?"));
    return;
  }
  if (!restriction) {
    notify(player, T("Do what with the function?"));
    return;
  }
  if (restrict_function(name, restriction))
    notify(player, T("Restrictions modified."));
  else
    notify(player, T("Restrict attempt failed."));
}


/** Add a user-defined function.
 * \verbatim
 * This is the implementation of the @function command. If no arguments
 * are given, it lists all @functions defined. Otherwise, this adds
 * an @function.
 * \endverbatim
 * \param player the enactor.
 * \param name name of function to add.
 * \param argv array of arguments.
 */
void
do_function(dbref player, char *name, char *argv[])
{
  char tbuf1[BUFFER_LEN];
  char *bp = tbuf1;
  dbref thing;
  FUN *fp;

  /* if no arguments, just give the list of user functions, by walking
   * the function hash table, and looking up all functions marked
   * as user-defined.
   */

  if (!name || !*name) {
    if (userfn_count == 0) {
      notify(player, T("No global user-defined functions exist."));
      return;
    }
    if (Global_Funcs(player)) {
      /* if the player is privileged, display user-def'ed functions
       * with corresponding dbref number of thing and attribute name.
       */
      notify(player, T("Function Name                   Dbref #    Attrib"));
      for (fp = (FUN *) hash_firstentry(&htab_user_function);
	   fp; fp = (FUN *) hash_nextentry(&htab_user_function)) {
	notify_format(player,
		      "%-32s %6d    %s", fp->name,
		      userfn_tab[fp->where.offset].thing,
		      userfn_tab[fp->where.offset].name);
      }
    } else {
      /* just print out the list of available functions */
      safe_str(T("User functions:"), tbuf1, &bp);
      for (fp = (FUN *) hash_firstentry(&htab_user_function);
	   fp; fp = (FUN *) hash_nextentry(&htab_user_function)) {
	safe_chr(' ', tbuf1, &bp);
	safe_str(fp->name, tbuf1, &bp);
      }
      *bp = '\0';
      notify(player, tbuf1);
    }
    return;
  }
  /* otherwise, we are adding a user function. 
   * Only those with the Global_Funcs power may add stuff.
   * If you add a function that is already a user-defined function,
   * the old function gets over-written.
   */

  if (!Global_Funcs(player)) {
    notify(player, T("Permission denied."));
    return;
  }
  if (!argv[1] || !*argv[1] || !argv[2] || !*argv[2]) {
    notify(player, T("You must specify an object and an attribute."));
    return;
  }
  /* make sure the function name length is okay */
  if (strlen(name) >= SBUF_LEN) {
    notify(player, T("Function name too long."));
    return;
  }
  /* find the object. For some measure of security, the player must
   * be able to examine it.
   */
  if ((thing = noisy_match_result(player, argv[1], NOTYPE, MAT_EVERYTHING))
      == NOTHING)
    return;
  if (!Can_Examine(player, thing)) {
    notify(player, T("No permission to examine object."));
    return;
  }
  /* we don't need to check if the attribute exists. If it doesn't,
   * it's not our problem - it's the user's responsibility to make
   * sure that the attribute exists (if it doesn't, invoking the
   * function will return a #-1 NO SUCH ATTRIBUTE error).
   * We do, however, need to make sure that the user isn't trying
   * to replace a built-in function.
   */

  fp = func_hash_lookup(upcasestr(name));
  if (!fp) {
    if (userfn_count >= (Size_t) MAX_GLOBAL_FNS) {
      notify(player, T("Function table full."));
      return;
    }
    /* a completely new entry. First, insert it into general hash table */
    fp = (FUN *) mush_malloc(sizeof(FUN), "func_hash.FUN");
    fp->name = mush_strdup(name, "func_hash.name");
    fp->where.offset = userfn_count;
    if (argv[3] && *argv[3]) {
      fp->minargs = parse_integer(argv[3]);
      if (fp->minargs < 0)
	fp->minargs = 0;
      else if (fp->minargs > 10)
	fp->minargs = 10;
    } else
      fp->minargs = 0;

    if (argv[4] && *argv[4]) {
      fp->maxargs = parse_integer(argv[4]);
      if (fp->maxargs < -10)
	fp->maxargs = -10;
      else if (fp->maxargs > 10)
	fp->maxargs = 10;
    } else
      fp->maxargs = 10;
    fp->flags = 0;
    hashadd(name, fp, &htab_user_function);

    /* now add it to the user function table */
    userfn_tab[userfn_count].thing = thing;
    userfn_tab[userfn_count].name =
      mush_strdup(upcasestr(argv[2]), "userfn_tab.name");
    userfn_tab[userfn_count].fn = mush_strdup(name, "usrfn_tab.fn");
    userfn_count++;

    notify(player, T("Function added."));
    return;
  } else {

    /* we are modifying an old entry */
    if ((fp->flags & FN_BUILTIN) && !(fp->flags & FN_OVERRIDE)) {
      notify(player, T("You cannot change that built-in function."));
      return;
    }
    if (fp->flags & FN_BUILTIN) {	/* Overriding a built in function */
      if (userfn_count >= (Size_t) MAX_GLOBAL_FNS) {
	notify(player, T("Function table full."));
	return;
      }
      fp = (FUN *) mush_malloc(sizeof(FUN), "func_hash.FUN");
      fp->name = mush_strdup(name, "func_hash.name");
      fp->where.offset = userfn_count;
      fp->flags = 0;
      userfn_count++;
      hashadd(name, fp, &htab_user_function);
    }
    userfn_tab[fp->where.offset].thing = thing;
    if (userfn_tab[fp->where.offset].name)
      mush_free((Malloc_t) userfn_tab[fp->where.offset].name,
		"userfn_tab.name");
    userfn_tab[fp->where.offset].name =
      mush_strdup(upcasestr(argv[2]), "userfn_tab.name");
    if (argv[3] && *argv[3]) {
      fp->minargs = parse_integer(argv[3]);
      if (fp->minargs < 0)
	fp->minargs = 0;
      else if (fp->minargs > 10)
	fp->minargs = 10;
    } else
      fp->minargs = 0;

    if (argv[4] && *argv[4]) {
      fp->maxargs = parse_integer(argv[4]);
      if (fp->maxargs < -10)
	fp->maxargs = -10;
      else if (fp->maxargs > 10)
	fp->maxargs = 10;
    } else
      fp->maxargs = 10;
    notify(player, T("Function updated."));
  }
}


/** Restore an overridden built-in function.
 * \verbatim
 * If a built-in function is deleted with @function/delete, it can be
 * restored with @function/restore. This implements @function/restore.
 * If a user-defined function has been added, it will be removed by
 * this function.
 * \endverbatim
 * \param player the enactor.
 * \param name name of function to restore.
 */
void
do_function_restore(dbref player, const char *name)
{
  FUN *fp;
  Size_t table_index, i;

  if (!Wizard(player)) {
    notify(player, T("Permission denied."));
    return;
  }

  if (!name || !*name) {
    notify(player, T("Restore what?"));
    return;
  }

  fp = (FUN *) hashfind(strupper(name), &htab_function);

  if (!fp) {
    notify(player, T("That's not a builtin function."));
    return;
  }

  if (!(fp->flags & FN_OVERRIDE)) {
    notify(player, T("That function isn't deleted!"));
    return;
  }

  fp->flags &= ~FN_OVERRIDE;
  notify(player, T("Restored."));

  /* Delete any @function with the same name */
  fp = (FUN *) hashfind(strupper(name), &htab_user_function);
  if (!fp)
    return;
  /* Remove it from the hash table */
  hashdelete(fp->name, &htab_user_function);
  /* Free its memory */
  table_index = fp->where.offset;
  mush_free((void *) fp->name, "func_hash.name");
  mush_free(fp, "func_hash.FUN");
  /* Fix up the user function table. Expensive, but how often will
   * we need to delete an @function anyway?
   */
  mush_free((Malloc_t) userfn_tab[table_index].name, "userfn_tab.name");
  mush_free((Malloc_t) userfn_tab[table_index].fn, "userfn_tab.fn");
  userfn_count--;
  for (i = table_index; i < userfn_count; i++) {
    fp = (FUN *) hashfind(userfn_tab[i + 1].fn, &htab_user_function);
    fp->where.offset = i;
    userfn_tab[i].thing = userfn_tab[i + 1].thing;
    userfn_tab[i].name = mush_strdup(userfn_tab[i + 1].name, "userfn_tab.name");
    mush_free((Malloc_t) userfn_tab[i + 1].name, "userfn_tab.name");
    userfn_tab[i].fn = mush_strdup(userfn_tab[i + 1].fn, "userfn_tab.fn");
    mush_free((Malloc_t) userfn_tab[i + 1].fn, "userfn_tab.fn");
  }
}

/** Delete a function.
 * \verbatim
 * This code implements @function/delete, which deletes a function -
 * either a built-in or a user-defined one.
 * \endverbatim
 * \param player the enactor.
 * \param name name of the function to delete.
 */
void
do_function_delete(dbref player, char *name)
{
  /* Deletes a user-defined function. 
   * For security, you must control the object the function uses
   * to delete the function.
   */
  Size_t table_index, i;
  FUN *fp;

  if (!Global_Funcs(player)) {
    notify(player, T("Permission denied."));
    return;
  }
  fp = func_hash_lookup(name);
  if (!fp) {
    notify(player, T("No such function."));
    return;
  }
  if (fp->flags & FN_BUILTIN) {
    if (!Wizard(player)) {
      notify(player, T("You can't delete that @function."));
      return;
    }
    fp->flags |= FN_OVERRIDE;
    notify(player, T("Function deleted."));
    return;
  }

  table_index = fp->where.offset;
  if (!controls(player, userfn_tab[table_index].thing)) {
    notify(player, T("You can't delete that @function."));
    return;
  }
  /* Remove it from the hash table */
  hashdelete(fp->name, &htab_user_function);
  /* Free its memory */
  mush_free((void *) fp->name, "func_hash.name");
  mush_free(fp, "func_hash.FUN");
  /* Fix up the user function table. Expensive, but how often will
   * we need to delete an @function anyway?
   */
  mush_free((Malloc_t) userfn_tab[table_index].name, "userfn_tab.name");
  mush_free((Malloc_t) userfn_tab[table_index].fn, "userfn_tab.fn");
  userfn_count--;
  for (i = table_index; i < userfn_count; i++) {
    fp = (FUN *) hashfind(userfn_tab[i + 1].fn, &htab_user_function);
    fp->where.offset = i;
    userfn_tab[i].thing = userfn_tab[i + 1].thing;
    userfn_tab[i].name = mush_strdup(userfn_tab[i + 1].name, "userfn_tab.name");
    mush_free((Malloc_t) userfn_tab[i + 1].name, "userfn_tab.name");
    userfn_tab[i].fn = mush_strdup(userfn_tab[i + 1].fn, "userfn_tab.fn");
    mush_free((Malloc_t) userfn_tab[i + 1].fn, "userfn_tab.fn");
  }
  notify(player, T("Function deleted."));
}

/** Enable or disable a function.
 * \verbatim
 * This implements @function/disable and @function/enable.
 * \endverbatim
 * \param player the enactor.
 * \param name name of the function to enable or disable.
 * \param toggle if 1, enable; if 0, disable.
 */
void
do_function_toggle(dbref player, char *name, int toggle)
{
  FUN *fp;

  if (!Wizard(player)) {
    notify(player, T("Permission denied."));
    return;
  }

  fp = func_hash_lookup(name);
  if (!fp) {
    notify(player, T("No such function."));
    return;
  }

  if (toggle) {
    fp->flags &= ~FN_DISABLED;
    notify(player, T("Enabled."));
  } else {
    fp->flags |= FN_DISABLED;
    notify(player, T("Disabled."));
  }
}

/** Get information about a function.
 * \verbatim
 * This implements the @function <function> command, which reports function
 * details to the enactor.
 * \endverbatim
 * \param player the enactor.
 * \param name name of the function.
 */
void
do_function_report(dbref player, char *name)
{
  FUN *fp;
  char tbuf[BUFFER_LEN];
  char *tp;
  const char *state, *state2;
  int first = 1;
  int maxargs;

  fp = func_hash_lookup(name);
  if (!fp) {
    notify(player, T("No such function."));
    return;
  }

  if (fp->flags & FN_BUILTIN)
    state2 = "";
  else
    state2 = " @function";

  if (fp->flags & FN_DISABLED)
    state = "Disabled";
  else
    state = "Enabled";

  notify_format(player, T("Name      : %s() (%s%s)"), fp->name, state, state2);

  tp = tbuf;
  tbuf[0] = '\0';
  if (fp->flags & FN_NOPARSE) {
    safe_str("Noparse", tbuf, &tp);
    if (first)
      first = 0;
  }

  if (fp->flags & FN_LITERAL) {
    if (first == 0)
      safe_strl(", ", 2, tbuf, &tp);
    safe_str("Literal", tbuf, &tp);
    first = 0;
  }

  if (fp->flags & FN_NOSIDEFX) {
    if (first == 0)
      safe_strl(", ", 2, tbuf, &tp);
    safe_str("Nosidefx", tbuf, &tp);
    first = 0;
  }


  if (fp->flags & FN_NOGAGGED) {
    if (first == 0)
      safe_strl(", ", 2, tbuf, &tp);
    safe_str("Nogagged", tbuf, &tp);
    first = 0;
  }

  if (fp->flags & FN_NOGUEST) {
    if (first == 0)
      safe_strl(", ", 2, tbuf, &tp);
    safe_str("Noguest", tbuf, &tp);
    first = 0;
  }

  if (fp->flags & FN_NOFIXED) {
    if (first == 0)
      safe_strl(", ", 2, tbuf, &tp);
    safe_str("Nofixed", tbuf, &tp);
    first = 0;
  }

  if (fp->flags & FN_WIZARD) {
    if (first == 0)
      safe_strl(", ", 2, tbuf, &tp);
    safe_str("Wizard", tbuf, &tp);
    first = 0;
  }

  if (fp->flags & FN_ADMIN) {
    if (first == 0)
      safe_strl(", ", 2, tbuf, &tp);
    safe_str("Admin", tbuf, &tp);
    first = 0;
  }

  if (fp->flags & FN_GOD) {
    if (first == 0)
      safe_strl(", ", 2, tbuf, &tp);
    safe_str("God", tbuf, &tp);
    first = 0;
  }

  *tp = '\0';
  notify_format(player, T("Flags     : %s"), tbuf);

  if (!(fp->flags & FN_BUILTIN) && Global_Funcs(player)) {
    notify_format(player, T("Location  : #%d/%s"),
		  userfn_tab[fp->where.offset].thing,
		  userfn_tab[fp->where.offset].name);
  }

  maxargs = abs(fp->maxargs);

  tp = tbuf;

  if (fp->maxargs < 0) {
    safe_str(T("(Commas okay in last argument)"), tbuf, &tp);
    *tp = '\0';
  } else
    tbuf[0] = '\0';

  if (fp->minargs == maxargs)
    notify_format(player, T("Arguments : %d %s"), fp->minargs, tbuf);
  else if (fp->maxargs == INT_MAX)
    notify_format(player, T("Arguments : At least %d %s"), fp->minargs, tbuf);
  else
    notify_format(player,
		  T("Arguments : %d to %d %s"), fp->minargs, maxargs, tbuf);
}
