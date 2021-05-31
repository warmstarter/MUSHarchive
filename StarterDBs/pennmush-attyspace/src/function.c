#include "copyrite.h"

#include "config.h"
#include <limits.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#include "conf.h"
#include "attrib.h"
#include "dbdefs.h"
#include "externs.h"
#include "function.h"
#include "intrface.h"
#include "match.h"
#include "htab.h"
#include "parse.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "mymalloc.h"
#include "confmagic.h"

void save_global_regs _((const char *funcname, char *preserve[]));
void restore_global_regs _((const char *funcname, char *preserve[]));
int delim_check _((char *buff, char **bp, int nfargs, char **fargs, int sep_arg, char *sep));
void do_list_functions _((dbref player));
FUN *func_hash_lookup _((char *name));
void func_hash_insert _((const char *name, FUN *func));
void init_func_hashtab _((void));
char *strip_braces _((const char *str));
void do_function _((dbref player, char *name, char **argv));
void do_function_delete _((dbref player, char *name));
extern void local_functions _((void));

HASHTAB htab_function;

/* -------------------------------------------------------------------------*
 * Utilities.
 */

void
save_global_regs(funcname, preserve)
    const char *funcname;
    char *preserve[];
{
  int i;

  for (i = 0; i < 10; i++) {
    if (!renv[i][0])
      preserve[i] = NULL;
    else {
      preserve[i] = (char *) mush_malloc(BUFFER_LEN, funcname);
      strcpy(preserve[i], renv[i]);
    }
  }
}

void
restore_global_regs(funcname, preserve)
    const char *funcname;
    char *preserve[];
{
  int i;

  for (i = 0; i < 10; i++) {
    if (preserve[i]) {
      strcpy(renv[i], preserve[i]);
      mush_free(preserve[i], funcname);
    } else {
      renv[i][0] = '\0';
    }
  }
}

int
delim_check(buff, bp, nfargs, fargs, sep_arg, sep)
    char *buff;
    char **bp;
    int nfargs;
    char *fargs[];
    int sep_arg;
    char *sep;
{
  /* Find a delimiter. */

  if (nfargs >= sep_arg) {
    if (!*fargs[sep_arg - 1])
      *sep = ' ';
    else if (strlen(fargs[sep_arg - 1]) != 1) {
      safe_str("#-1 SEPARATOR MUST BE ONE CHARACTER", buff, bp);
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

FUN flist[] =
{
  {"ABS", fun_abs, 1, 1, FN_REG},
  {"ADD", fun_add, 2, INT_MAX, FN_REG},
  {"AFTER", fun_after, 2, 2, FN_REG},
  {"ALPHAMAX", fun_alphamax, 1, INT_MAX, FN_REG},
  {"ALPHAMIN", fun_alphamin, 1, INT_MAX, FN_REG},
  {"AND", fun_and, 2, INT_MAX, FN_REG},
  {"ANDFLAGS", fun_andflags, 2, 2, FN_REG},
  {"ANSI", fun_ansi, 2, -2, FN_REG},
  {"APOSS", fun_aposs, 1, 1, FN_REG},
  {"ART", fun_art, 1, 1, FN_REG},
  {"ATRLOCK", fun_atrlock, 1, 2, FN_REG},
  {"BEEP", fun_beep, 0, 1, FN_REG},
  {"BEFORE", fun_before, 2, 2, FN_REG},
  {"CAND", fun_cand, 2, INT_MAX, FN_NOPARSE},
  {"CAPSTR", fun_capstr, 1, -1, FN_REG},
  {"CAT", fun_cat, 1, INT_MAX, FN_REG},
#ifdef CHAT_SYSTEM
  {"CEMIT", fun_cemit, 2, -2, FN_REG},
#endif
  {"CENTER", fun_center, 2, 3, FN_REG},
#ifdef CHAT_SYSTEM
  {"CHANNELS", fun_channels, 0, 1, FN_REG},
#endif
  {"CLONE", fun_clone, 1, 1, FN_REG},
  {"COMP", fun_comp, 2, 2, FN_REG},
  {"CON", fun_con, 1, 1, FN_REG},
  {"CONN", fun_conn, 1, 1, FN_REG},
  {"CONTROLS", fun_controls, 2, 2, FN_REG},
  {"CONVSECS", fun_convsecs, 1, 1, FN_REG},
  {"CONVTIME", fun_convtime, 1, 1, FN_REG},
  {"COR", fun_cor, 2, INT_MAX, FN_NOPARSE},
  {"CREATE", fun_create, 1, 2, FN_REG},
#ifdef CREATION_TIMES
  {"CTIME", fun_ctime, 1, 1, FN_REG},
#endif
#ifdef CHAT_SYSTEM
  {"CWHO", fun_cwho, 1, 1, FN_REG},
#endif
  {"DEC", fun_dec, 1, 1, FN_REG},
  {"DECRYPT", fun_decrypt, 2, 2, FN_REG},
  {"DEFAULT", fun_default, 2, 2, FN_NOPARSE},
  {"DELETE", fun_delete, 3, 3, FN_REG},
  {"DIE", fun_die, 2, 2, FN_REG},
  {"DIG", fun_dig, 1, 3, FN_REG},
  {"DIST2D", fun_dist2d, 4, 4, FN_REG},
  {"DIST3D", fun_dist3d, 6, 6, FN_REG},
  {"DIV", fun_div, 2, 2, FN_REG},
  {"DOING", fun_doing, 1, 1, FN_REG},
  {"EDEFAULT", fun_edefault, 2, 2, FN_NOPARSE},
  {"EDIT", fun_edit, 3, 3, FN_REG},
  {"ELEMENT", fun_element, 3, 3, FN_REG},
  {"ELEMENTS", fun_elements, 2, 3, FN_REG},
  {"ELOCK", fun_elock, 2, 2, FN_REG},
  {"EMIT", fun_emit, 1, -1, FN_REG},
  {"ENCRYPT", fun_encrypt, 2, 2, FN_REG},
  {"ENTRANCES", fun_entrances, 0, 4, FN_REG},
  {"EQ", fun_eq, 2, 2, FN_REG},
  {"EVAL", fun_eval, 2, 2, FN_REG},
  {"ESCAPE", fun_escape, 1, -1, FN_REG},
  {"EXIT", fun_exit, 1, 1, FN_REG},
  {"EXTRACT", fun_extract, 3, 4, FN_REG},
  {"FILTER", fun_filter, 2, 3, FN_REG},
  {"FINDABLE", fun_findable, 2, 2, FN_REG},
  {"FIRST", fun_first, 1, 2, FN_REG},
  {"FLAGS", fun_flags, 1, 1, FN_REG},
  {"FLIP", fun_flip, 1, 1, FN_REG},
  {"FOLD", fun_fold, 2, 4, FN_REG},
#ifdef USE_MAILER
  {"FOLDERSTATS", fun_folderstats, 0, 2, FN_REG},
#endif
  {"FOREACH", fun_foreach, 2, 2, FN_REG},
  {"FUNCTIONS", fun_functions, 0, 0, FN_REG},
  {"FULLNAME", fun_fullname, 1, 1, FN_REG},
  {"GET", fun_get, 1, 1, FN_REG},
  {"GET_EVAL", fun_get_eval, 1, 1, FN_REG},
  {"GRAB", fun_grab, 2, 3, FN_REG},
  {"GRABALL", fun_graball, 2, 3, FN_REG},
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
  {"HIDDEN", fun_hidden, 1, 1, FN_REG},
  {"HOME", fun_home, 1, 1, FN_REG},
  {"IDLE", fun_idlesecs, 1, 1, FN_REG},
  {"IDLESECS", fun_idlesecs, 1, 1, FN_REG},
#ifndef TREKMUSH
  {"IF", fun_if, 2, 3, FN_NOPARSE},
  {"IFELSE", fun_if, 3, 3, FN_NOPARSE},
#endif /* TREKMUSH */
  {"INC", fun_inc, 1, 1, FN_REG},
  {"INDEX", fun_index, 4, 4, FN_REG},
  {"INSERT", fun_insert, 3, 4, FN_REG},
  {"ISDAYLIGHT", fun_isdaylight, 0, 0, FN_REG},
  {"ISDBREF", fun_isdbref, 1, 1, FN_REG},
  {"ISNUM", fun_isnum, 1, 1, FN_REG},
  {"ISWORD", fun_isword, 1, 1, FN_REG},
  {"ITER", fun_iter, 2, 4, FN_NOPARSE},
  {"ITEMS", fun_items, 2, 2, FN_REG},
  {"LAST", fun_last, 1, 2, FN_REG},
  {"LATTR", fun_lattr, 1, 1, FN_REG},
  {"LCON", fun_lcon, 1, 1, FN_REG},
  {"LCSTR", fun_lcstr, 1, -1, FN_REG},
  {"LDELETE", fun_ldelete, 2, 3, FN_REG},
  {"LEFT", fun_left, 2, 2, FN_REG},
  {"LEMIT", fun_lemit, 1, -1, FN_REG},
  {"LEXITS", fun_lexits, 1, 1, FN_REG},
  {"LINK", fun_link, 2, 2, FN_REG},
  {"LIT", fun_lit, 1, -1, FN_LITERAL},
  {"LJUST", fun_ljust, 2, 3, FN_REG},
  {"LNUM", fun_lnum, 1, 3, FN_REG},
  {"LOC", fun_loc, 1, 1, FN_REG},
  {"LOCATE", fun_locate, 3, 3, FN_REG},
  {"LOCK", fun_lock, 1, 2, FN_REG},
  {"LPARENT", fun_lparent, 1, 1, FN_REG},
  {"LSEARCH", fun_lsearch, 3, 5, FN_REG},
  {"LSEARCHR", fun_lsearch, 3, 5, FN_REG},
  {"LSTATS", fun_lstats, 0, 1, FN_REG},
  {"LT", fun_lt, 2, 2, FN_REG},
  {"LTE", fun_lte, 2, 2, FN_REG},
  {"LWHO", fun_lwho, 0, 0, FN_REG},
#ifdef USE_MAILER
  {"MAIL", fun_mail, 0, 2, FN_REG},
  {"MAILFROM", fun_mailfrom, 1, 2, FN_REG},
  {"MAILSTATUS", fun_mailstatus, 1, 2, FN_REG},
#ifdef MAIL_SUBJECTS
  {"MAILSUBJECT", fun_mailsubject, 1, 2, FN_REG},
#endif
  {"MAILTIME", fun_mailtime, 1, 2, FN_REG},
#endif
  {"MAP", fun_map, 2, 3, FN_REG},
  {"MATCH", fun_match, 2, 3, FN_REG},
  {"MATCHALL", fun_matchall, 2, 3, FN_REG},
  {"MAX", fun_max, 1, INT_MAX, FN_REG},
  {"MEMBER", fun_member, 2, 3, FN_REG},
  {"MERGE", fun_merge, 3, 3, FN_REG},
  {"MID", fun_mid, 3, 3, FN_REG},
  {"MIN", fun_min, 1, INT_MAX, FN_REG},
  {"MIX", fun_mix, 3, 4, FN_REG},
  {"MOD", fun_mod, 2, 2, FN_REG},
  {"MONEY", fun_money, 1, 1, FN_REG},
#ifdef CREATION_TIMES
  {"MTIME", fun_mtime, 1, 1, FN_REG},
#endif
  {"MUDNAME", fun_mudname, 0, 0, FN_REG},
  {"MUL", fun_mul, 2, INT_MAX, FN_REG},
  {"MUNGE", fun_munge, 3, 4, FN_REG},
  {"MWHO", fun_lwho, 0, 0, FN_REG},
  {"NAME", fun_name, 1, 2, FN_REG},
  {"NEARBY", fun_nearby, 2, 2, FN_REG},
  {"NEQ", fun_neq, 2, 2, FN_REG},
  {"NEXT", fun_next, 1, 1, FN_REG},
  {"NOT", fun_not, 1, 1, FN_REG},
  {"NUM", fun_num, 1, 1, FN_REG},
  {"OBJ", fun_obj, 1, 1, FN_REG},
  {"OBJEVAL", fun_objeval, 2, -2, FN_NOPARSE},
  {"OBJMEM", fun_objmem, 1, 1, FN_REG},
  {"OEMIT", fun_oemit, 2, -2, FN_REG},
  {"OPEN", fun_open, 2, 2, FN_REG},
  {"OR", fun_or, 2, INT_MAX, FN_REG},
  {"ORFLAGS", fun_orflags, 2, 2, FN_REG},
  {"OWNER", fun_owner, 1, 1, FN_REG},
  {"PARENT", fun_parent, 1, 2, FN_REG},
  {"PARSE", fun_iter, 2, 4, FN_NOPARSE},
  {"PEMIT", fun_pemit, 2, -2, FN_REG},
  {"PLAYERMEM", fun_playermem, 1, 1, FN_REG},
  {"PMATCH", fun_pmatch, 1, 1, FN_REG},
  {"POLL", fun_poll, 0, 0, FN_REG},
  {"PORTS", fun_ports, 1, 1, FN_REG},
  {"POS", fun_pos, 2, 2, FN_REG},
  {"POSS", fun_poss, 1, 1, FN_REG},
  {"POWERS", fun_powers, 1, 1, FN_REG},
  {"PUEBLO", fun_pueblo, 1, 1, FN_REG},
#ifdef QUOTA
  {"QUOTA", fun_quota, 1, 1, FN_REG},
#endif
  {"R", fun_r, 1, 1, FN_REG},
  {"RAND", fun_rand, 1, 1, FN_REG},
  {"REGMATCH", fun_regmatch, 2, 3, FN_REG},
  {"REGMATCHI", fun_regmatchi, 2, 3, FN_REG},
  {"REMIT", fun_remit, 2, -2, FN_REG},
  {"REMOVE", fun_remove, 2, 3, FN_REG},
  {"REPEAT", fun_repeat, 2, 2, FN_REG},
  {"REPLACE", fun_replace, 3, 4, FN_REG},
  {"REST", fun_rest, 1, 2, FN_REG},
  {"REVERSE", fun_flip, 1, 1, FN_REG},
  {"REVWORDS", fun_revwords, 1, 2, FN_REG},
  {"RIGHT", fun_right, 2, 2, FN_REG},
  {"RJUST", fun_rjust, 2, 3, FN_REG},
  {"RLOC", fun_rloc, 2, 2, FN_REG},
  {"RNUM", fun_rnum, 2, 2, FN_REG},
  {"ROOM", fun_room, 1, 1, FN_REG},
  {"S", fun_s, 1, -1, FN_REG},
  {"SCRAMBLE", fun_scramble, 1, -1, FN_REG},
  {"SEARCH", fun_lsearch, 3, 5, FN_REG},
  {"SECS", fun_secs, 0, 0, FN_REG},
  {"SECURE", fun_secure, 1, -1, FN_REG},
  {"SET", fun_set, 2, 2, FN_REG},
  {"SETQ", fun_setq, 2, 2, FN_REG},
  {"SETR", fun_setq, 2, 2, FN_REG},
  {"SETDIFF", fun_setdiff, 2, 3, FN_REG},
  {"SETINTER", fun_setinter, 2, 3, FN_REG},
  {"SETUNION", fun_setunion, 2, 3, FN_REG},
  {"SHL", fun_shl, 2, 2, FN_REG},
  {"SHR", fun_shr, 2, 2, FN_REG},
  {"SHUFFLE", fun_shuffle, 1, 2, FN_REG},
  {"SIGN", fun_sign, 1, 1, FN_REG},
  {"SORT", fun_sort, 1, 3, FN_REG},
  {"SORTBY", fun_sortby, 2, 3, FN_REG},
  {"SOUNDEX", fun_soundex, 1, 1, FN_REG},
  {"SOUNDLIKE", fun_soundlike, 2, 2, FN_REG},
  {"SOUNDSLIKE", fun_soundlike, 2, 2, FN_REG},
  {"SPACE", fun_space, 1, 1, FN_REG},
  {"SPLICE", fun_splice, 3, 4, FN_REG},
  {"SQUISH", fun_squish, 1, 1, FN_REG},
  {"STARTTIME", fun_starttime, 0, 0, FN_REG},
  {"STATS", fun_lstats, 0, 1, FN_REG},
  {"STRCAT", fun_strcat, 1, INT_MAX, FN_REG},
  {"STRIPANSI", fun_stripansi, 1, -1, FN_REG},
  {"STRLEN", fun_strlen, 1, -1, FN_REG},
  {"STRMATCH", fun_strmatch, 2, 2, FN_REG},
  {"SUB", fun_sub, 2, 2, FN_REG},
  {"SUBJ", fun_subj, 1, 1, FN_REG},
  {"SWITCH", fun_switch, 3, INT_MAX, FN_NOPARSE},
  {"T", fun_t, 1, 1, FN_REG},
  {"TABLE", fun_table, 1, 5, FN_REG},
  {"TEL", fun_tel, 2, 2, FN_REG},
  {"TIME", fun_time, 0, 0, FN_REG},
  {"TIMESTRING", fun_timestring, 1, 2, FN_REG},
  {"TRIM", fun_trim, 1, 3, FN_REG},
  {"TRUNC", fun_trunc, 1, 1, FN_REG},
  {"TYPE", fun_type, 1, 1, FN_REG},
  {"UCSTR", fun_ucstr, 1, -1, FN_REG},
  {"UDEFAULT", fun_udefault, 2, 12, FN_NOPARSE},
  {"UFUN", fun_ufun, 1, 11, FN_REG},
  {"ULOCAL", fun_ulocal, 1, 11, FN_REG},
  {"U", fun_ufun, 1, 11, FN_REG},
  {"V", fun_v, 1, 1, FN_REG},
  {"VAL", fun_trunc, 1, 1, FN_REG},
  {"VALID", fun_valid, 2, 2, FN_REG},
  {"VERSION", fun_version, 0, 0, FN_REG},
  {"VISIBLE", fun_visible, 2, 2, FN_REG},
  {"WHERE", fun_where, 1, 1, FN_REG},
  {"WIPE", fun_wipe, 1, 1, FN_REG},
  {"WORDPOS", fun_wordpos, 2, 3, FN_REG},
  {"WORDS", fun_words, 1, 2, FN_REG},
  {"XGET", fun_xget, 2, 2, FN_REG},
  {"XOR", fun_xor, 2, INT_MAX, FN_REG},
  {"ZEMIT", fun_zemit, 2, -2, FN_REG},
  {"ZFUN", fun_zfun, 1, 11, FN_REG},
  {"ZONE", fun_zone, 1, 2, FN_REG},
  {"VADD", fun_vadd, 2, 3, FN_REG},
  {"VSUB", fun_vsub, 2, 3, FN_REG},
  {"VMUL", fun_vmul, 2, 3, FN_REG},
  {"VDOT", fun_vdot, 2, 3, FN_REG},
  {"VMAG", fun_vmag, 1, 2, FN_REG},
  {"VDIM", fun_words, 1, 2, FN_REG},
  {"VUNIT", fun_vunit, 1, 2, FN_REG},
#ifdef FLOATING_POINTS
  {"ACOS", fun_acos, 1, 1, FN_REG},
  {"ASIN", fun_asin, 1, 1, FN_REG},
  {"ATAN", fun_atan, 1, 1, FN_REG},
  {"CEIL", fun_ceil, 1, 1, FN_REG},
  {"COS", fun_cos, 1, 1, FN_REG},
  {"E", fun_e, 0, 0, FN_REG},
  {"EXP", fun_exp, 1, 1, FN_REG},
  {"FDIV", fun_fdiv, 2, 2, FN_REG},
  {"FLOOR", fun_floor, 1, 1, FN_REG},
  {"LOG", fun_log, 1, 1, FN_REG},
  {"LN", fun_ln, 1, 1, FN_REG},
  {"PI", fun_pi, 0, 0, FN_REG},
  {"POWER", fun_power, 2, 2, FN_REG},
  {"ROUND", fun_round, 2, 2, FN_REG},
  {"SIN", fun_sin, 1, 1, FN_REG},
  {"SQRT", fun_sqrt, 1, 1, FN_REG},
  {"TAN", fun_tan, 1, 1, FN_REG},
#endif				/* FLOATING_POINTS */
  {"HTML", fun_html, 1, 1, FN_REG},
  {"TAG", fun_tag, 1, INT_MAX, FN_REG},
  {"ENDTAG", fun_endtag, 1, 1, FN_REG},
  {"TAGWRAP", fun_tagwrap, 2, 3, FN_REG},
  {NULL, NULL, 0, 0, 0}
};

void
do_list_functions(player)
    dbref player;
{
  /* lists all built-in functions. */

  FUN *fp;
  char *ptrs[BUFFER_LEN / 2];
  char buff[BUFFER_LEN];
  char *bp;
  int nptrs = 0, i;
  fp = hash_firstentry(&htab_function);
  while (fp) {
    ptrs[nptrs++] = (char *) fp->name;
    fp = hash_nextentry(&htab_function);
  }
  do_gensort(ptrs, nptrs, 0);
  bp = buff;
  safe_str("Functions:", buff, &bp);
  for (i = 0; i < nptrs; i++) {
    safe_chr(' ', buff, &bp);
    safe_str(ptrs[i], buff, &bp);
  }
  *bp = '\0';

  notify(player, buff);
}

/*---------------------------------------------------------------------------
 * Hashed function table stuff
 */


FUN *
func_hash_lookup(name)
    char *name;
{
  return (FUN *) hashfind(strupper(name), &htab_function);
}

void
func_hash_insert(name, func)
    const char *name;
    FUN *func;
{
  hashadd(name, (void *) func, &htab_function);
}

void
init_func_hashtab()
{
  FUN *fp;

  hashinit(&htab_function, 256);
  for (fp = flist; fp->name; fp++)
    func_hash_insert((char *) fp->name, (FUN *) fp);
  local_functions();
}

void
function_add(name, fun, minargs, maxargs, ftype)
    const char *name;
    void (*fun) ();
    int minargs;
    int maxargs;
    int ftype;
{
  FUN *fp;
  fp = (FUN *) mush_malloc(sizeof(FUN), "function");
  memset(fp, 0, sizeof(FUN));
  fp->name = name;
  fp->fun = fun;
  fp->minargs = minargs;
  fp->maxargs = maxargs;
  fp->ftype = ftype;
  func_hash_insert(name, fp);
}

/*-------------------------------------------------------------------------
 * Function handlers and the other good stuff. Almost all this code is
 * a modification of TinyMUSH 2.0 code.
 */

char *
strip_braces(str)
    char const *str;
{
  /* this is a hack which just strips a level of braces. It malloc()s memory
   * which must be free()d later.
   */

  char *buff;
  char *bufc;

  buff = (char *) mush_malloc(BUFFER_LEN, "strip_braces.buff");
  bufc = buff;

  while (isspace(*str))		/* eat spaces at the beginning */
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

USERFN_ENTRY userfn_tab[MAX_GLOBAL_FNS];

static int userfn_count = 0;

/* ARGSUSED */
FUNCTION(fun_gfun)
{
  /* this function is a dummy. */
}

void
do_function(player, name, argv)
    dbref player;
    char *name;
    char *argv[];
{
  /* command of format: @function <function name>=<thing>,<attribute>
   * Adds a new user-defined function.
   */

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
      notify(player, "No global user-defined functions exist.");
      return;
    }
    if (Global_Funcs(player)) {
      /* if the player is privileged, display user-def'ed functions
       * with corresponding dbref number of thing and attribute name.
       */
      notify(player,
	     "Function Name                   Dbref #    Attrib");
      for (fp = (FUN *) hash_firstentry(&htab_function);
	   fp; fp = (FUN *) hash_nextentry(&htab_function)) {
	if (fp->ftype >= GLOBAL_OFFSET)
	  notify(player,
		 tprintf("%-32s %6d    %s", fp->name,
			 userfn_tab[GF_Index(fp->ftype)].thing,
			 userfn_tab[GF_Index(fp->ftype)].name));
      }
    } else {
      /* just print out the list of available functions */
      safe_str("User functions:", tbuf1, &bp);
      for (fp = (FUN *) hash_firstentry(&htab_function);
	   fp; fp = (FUN *) hash_nextentry(&htab_function)) {
	if (fp->ftype >= GLOBAL_OFFSET) {
	  safe_chr(' ', tbuf1, &bp);
	  safe_str(fp->name, tbuf1, &bp);
	}
      }
      *bp = '\0';
      notify(player, tbuf1);
    }
    return;
  }
  /* otherwise, we are adding a user function. There is NO deletion
   * mechanism. Only those with the Global_Funcs power may add stuff.
   * If you add a function that is already a user-defined function,
   * the old function gets over-written.
   */

  if (!Global_Funcs(player)) {
    notify(player, "Permission denied.");
    return;
  }
  if (userfn_count >= MAX_GLOBAL_FNS) {
    notify(player, "Function table full.");
    return;
  }
  if (!argv[1] || !*argv[1] || !argv[2] || !*argv[2]) {
    notify(player, "You must specify an object and an attribute.");
    return;
  }
  /* make sure the function name length is okay */
  if (strlen(name) >= SBUF_LEN) {
    notify(player, "Function name too long.");
    return;
  }
  /* find the object. For some measure of security, the player must
   * be able to examine it.
   */
  if ((thing = noisy_match_result(player, argv[1], NOTYPE, MAT_EVERYTHING)) == NOTHING)
    return;
  if (!Can_Examine(player, thing)) {
    notify(player, "No permission to examine object.");
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
    /* a completely new entry. First, insert it into general hash table */
    fp = (FUN *) mush_malloc(sizeof(FUN), "func_hash.FUN");
    fp->name = (char *) strdup(name);
#ifdef MEM_CHECK
    add_check("func_hash.name");
#endif
    fp->fun = fun_gfun;
    fp->minargs = 0;
    fp->maxargs = 10;
    fp->ftype = userfn_count + GLOBAL_OFFSET;
    func_hash_insert(name, (FUN *) fp);

    /* now add it to the user function table */
    userfn_tab[userfn_count].thing = thing;
    userfn_tab[userfn_count].name = (char *) strdup(upcasestr(argv[2]));
#ifdef MEM_CHECK
    add_check("userfn_tab.name");
#endif
    userfn_tab[userfn_count].fn = (char *) strdup(upcasestr(name));
#ifdef MEM_CHECK
    add_check("userfn_tab.fn");
#endif

    userfn_count++;

    notify(player, "Function added.");
    return;
  } else {

    /* we are modifying an old entry */
    if ((fp->ftype == FN_REG) || (fp->ftype == FN_NOPARSE)) {
      notify(player, "You cannot change a built-in function.");
      return;
    }
    userfn_tab[GF_Index(fp->ftype)].thing = thing;
    mush_free((Malloc_t) userfn_tab[GF_Index(fp->ftype)].name,
	      "userfn_tab.name");
    userfn_tab[GF_Index(fp->ftype)].name =
      (char *) strdup(upcasestr(argv[2]));
#ifdef MEM_CHECK
    add_check("userfn_tab.name");
#endif
    notify(player, "Function updated.");
  }
}

void
do_function_delete(player, name)
    dbref player;
    char *name;
{
  /* Deletes a user-defined function.
   * For security, you must control the object the function uses
   * to delete the function.
   */
  int table_index;
  int i;
  FUN *fp;

  if (!Global_Funcs(player)) {
    notify(player, "Permission denied.");
    return;
  }
  fp = func_hash_lookup(upcasestr(name));
  if (!fp) {
    notify(player, "No such function.");
    return;
  }
  if ((fp->ftype == FN_REG) || (fp->ftype == FN_NOPARSE)) {
    notify(player, "You cannot delete a built-in function.");
    return;
  }
  table_index = GF_Index(fp->ftype);
  if (!controls(player, userfn_tab[table_index].thing)) {
    notify(player, "You can't delete that @function.");
    return;
  }
  /* Remove it from the hash table */
  hashdelete((char *) fp->name, &htab_function);
  /* Free its memory */
  mush_free((Malloc_t) fp->name, "func_hash.name");
  mush_free((Malloc_t) fp, "func_hash.FUN");
  /* Fix up the user function table. Expensive, but how often will
   * we need to delete an @function anyway?
   */
  mush_free((Malloc_t) userfn_tab[table_index].name, "userfn_tab.name");
  mush_free((Malloc_t) userfn_tab[table_index].fn, "userfn_tab.fn");
  userfn_count--;
  for (i = table_index; i < userfn_count; i++) {
    fp = func_hash_lookup(userfn_tab[i + 1].fn);
    fp->ftype = i + GLOBAL_OFFSET;
    userfn_tab[i].thing = userfn_tab[i + 1].thing;
    userfn_tab[i].name = (char *) strdup(userfn_tab[i + 1].name);
#ifdef MEM_CHECK
    add_check("userfn_tab.name");
#endif
    mush_free((Malloc_t) userfn_tab[i + 1].name, "userfn_tab.name");
    userfn_tab[i].fn = (char *) strdup(userfn_tab[i + 1].fn);
#ifdef MEM_CHECK
    add_check("userfn_tab.fn");
#endif
    mush_free((Malloc_t) userfn_tab[i + 1].fn, "userfn_tab.fn");
  }
  notify(player, "Function deleted.");
}
