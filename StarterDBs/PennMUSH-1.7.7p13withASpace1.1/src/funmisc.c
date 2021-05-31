/**
 * \file funmisc.c
 *
 * \brief Miscellaneous functions for mushcode.
 *
 *
 */
#include "copyrite.h"

#include "config.h"
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "conf.h"
#include "case.h"
#include "externs.h"
#include "version.h"
#include "htab.h"
#include "flags.h"
#include "match.h"
#include "mushdb.h"
#include "dbdefs.h"
#include "parse.h"
#include "function.h"
#include "command.h"
#include "game.h"
#include "attrib.h"
#include "confmagic.h"

#ifdef WIN32
#pragma warning( disable : 4761)	/* NJG: disable warning re conversion */
#endif

extern time_t start_time, first_start_time;
extern int reboot_count;
extern FUN flist[];
static char *soundex(char *str);
extern char cf_motd_msg[BUFFER_LEN], cf_wizmotd_msg[BUFFER_LEN],
  cf_downmotd_msg[BUFFER_LEN], cf_fullmotd_msg[BUFFER_LEN];
extern HASHTAB htab_function;

/* ARGSUSED */
FUNCTION(fun_valid)
{
  /* Checks to see if a given <something> is valid as a parameter of a
   * given type (such as an object name.
   */

  if (!args[0] || !*args[0])
    safe_str("#-1", buff, bp);
  else if (!strcasecmp(args[0], "name"))
    safe_boolean(ok_name(args[1]), buff, bp);
  else if (!strcasecmp(args[0], "attrname"))
    safe_boolean(good_atr_name(upcasestr(args[1])), buff, bp);
  else if (!strcasecmp(args[0], "playername"))
    safe_boolean(ok_player_name(args[1]), buff, bp);
  else
    safe_str("#-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_pemit)
{
  if (!command_check_byname(executor, "@pemit") || fun->flags & FN_NOSIDEFX) {
    safe_str(T(e_perm), buff, bp);
    return;
  }
  orator = executor;
  do_pemit_list(executor, args[0], args[1], PEMIT_LIST);
}

/* ARGSUSED */
FUNCTION(fun_nspemit)
{
  if (!command_check_byname(executor, "@nspemit") || fun->flags & FN_NOSIDEFX) {
    safe_str(T(e_perm), buff, bp);
    return;
  }
  orator = executor;
  do_pemit_list(executor, args[0], args[1], 0);
}

/* ARGSUSED */
FUNCTION(fun_oemit)
{
  if (!command_check_byname(executor, "@oemit") || fun->flags & FN_NOSIDEFX) {
    safe_str(T(e_perm), buff, bp);
    return;
  }
  orator = executor;
  do_oemit_list(executor, args[0], args[1]);
}

/* ARGSUSED */
FUNCTION(fun_emit)
{
  if (!command_check_byname(executor, "@emit") || fun->flags & FN_NOSIDEFX) {
    safe_str(T(e_perm), buff, bp);
    return;
  }
  orator = executor;
  do_emit(executor, args[0]);
}

/* ARGSUSED */
FUNCTION(fun_remit)
{
  if (!command_check_byname(executor, "@remit") || fun->flags & FN_NOSIDEFX) {
    safe_str(T(e_perm), buff, bp);
    return;
  }
  orator = executor;
  do_remit(executor, args[0], args[1], 0);
}

/* ARGSUSED */
FUNCTION(fun_lemit)
{
  if (!command_check_byname(executor, "@lemit") || fun->flags & FN_NOSIDEFX) {
    safe_str(T(e_perm), buff, bp);
    return;
  }
  orator = executor;
  do_lemit(executor, args[0], 0);
}

/* ARGSUSED */
FUNCTION(fun_zemit)
{
  if (!command_check_byname(executor, "@zemit") || fun->flags & FN_NOSIDEFX) {
    safe_str(T(e_perm), buff, bp);
    return;
  }
  orator = executor;
  do_zemit(executor, args[0], args[1]);
}



extern signed char qreg_indexes[UCHAR_MAX + 1];
/* ARGSUSED */
FUNCTION(fun_setq)
{
  /* sets a variable into a local register */
  int qindex;

  if (*args[0] && (*(args[0] + 1) == '\0') &&
      ((qindex = qreg_indexes[(unsigned char) args[0][0]]) != -1)
      && renv[qindex]) {
    strcpy(renv[qindex], args[1]);
    if (!strcmp(called_as, "SETR"))
      safe_strl(args[1], arglens[1], buff, bp);
  } else
    safe_str(T("#-1 REGISTER OUT OF RANGE"), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_r)
{
  /* returns a local register */
  int qindex;

  if (*args[0] && (*(args[0] + 1) == '\0') &&
      ((qindex = qreg_indexes[(unsigned char) args[0][0]]) != -1)
      && renv[qindex])
    safe_str(renv[qindex], buff, bp);
  else
    safe_str(T("#-1 REGISTER OUT OF RANGE"), buff, bp);
}

/* --------------------------------------------------------------------------
 * Utility functions: RAND, DIE, SECURE, SPACE, BEEP, SWITCH, EDIT,
 *      ESCAPE, SQUISH, ENCRYPT, DECRYPT, LIT
 */

/* ARGSUSED */
FUNCTION(fun_rand)
{
  /*
   * Uses Sh'dow's random number generator, found in utils.c.  Better
   * distribution than original, w/ minimal speed losses.
   */
  int i;
  if (!is_integer(args[0])) {
    safe_str(T(e_int), buff, bp);
    return;
  }
  i = get_random_long(0, parse_integer(args[0]) - 1);
  safe_integer(i, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_die)
{
  unsigned int n;
  unsigned int die;
  unsigned int count;
  unsigned int total = 0;
  int show_all = 0, first = 1;

  if (!is_uinteger(args[0]) || !is_uinteger(args[1])) {
    safe_str(T(e_uints), buff, bp);
    return;
  }
  n = parse_uinteger(args[0]);
  die = parse_uinteger(args[1]);
  if (nargs == 3)
    show_all = parse_boolean(args[2]);

  if (n == 0 || n > 20) {
    safe_str(T("#-1 NUMBER OUT OF RANGE"), buff, bp);
    return;
  }
  if (show_all) {
    for (count = 0; count < n; count++) {
      if (first)
	first = 0;
      else
	safe_chr(' ', buff, bp);
      safe_uinteger(get_random_long(1, die), buff, bp);
    }
  } else {
    for (count = 0; count < n; count++)
      total += get_random_long(1, die);

    safe_uinteger(total, buff, bp);
  }
}

/* ARGSUSED */
FUNCTION(fun_switch)
{
  /* this works a bit like the @switch command: it returns the string
   * appropriate to the match. It picks the first match, like @select
   * does, though.
   * Args to this function are passed unparsed. Args are not evaluated
   * until they are needed.
   */

  int j, per;
  char mstr[BUFFER_LEN], pstr[BUFFER_LEN], *dp;
  char const *sp;
  char *tbuf1;
  int first = 1, found = 0, exact = 0;

  if (strstr(called_as, "ALL"))
    first = 0;

  if (string_prefix(called_as, "CASE"))
    exact = 1;

  dp = mstr;
  sp = args[0];
  process_expression(mstr, &dp, &sp, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  *dp = '\0';

  /* try matching, return match immediately when found */

  for (j = 1; j < (nargs - 1); j += 2) {
    dp = pstr;
    sp = args[j];
    process_expression(pstr, &dp, &sp, executor, caller, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
    *dp = '\0';

    if ((!exact)
	? local_wild_match(pstr, mstr)
	: (strcmp(pstr, mstr) == 0)) {
      /* If there's a #$ in a switch's action-part, replace it with
       * the value of the conditional (mstr) before evaluating it.
       */
      if (!exact)
	tbuf1 = replace_string("#$", mstr, args[j + 1]);
      else
	tbuf1 = args[j + 1];

      sp = tbuf1;

      per = process_expression(buff, bp, &sp,
			       executor, caller, enactor,
			       PE_DEFAULT, PT_DEFAULT, pe_info);
      if (!exact)
	mush_free((Malloc_t) tbuf1, "replace_string.buff");
      found = 1;
      if (per || first)
	return;
    }
  }

  if (!(nargs & 1) && !found) {
    /* Default case */
    tbuf1 = replace_string("#$", mstr, args[nargs - 1]);
    sp = tbuf1;
    process_expression(buff, bp, &sp, executor, caller, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
    mush_free((Malloc_t) tbuf1, "replace_string.buff");
  }
}

/* ARGSUSED */
FUNCTION(fun_if)
{
  char tbuf[BUFFER_LEN], *tp;
  char const *sp;

  tp = tbuf;
  sp = args[0];
  process_expression(tbuf, &tp, &sp, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  *tp = '\0';
  if (parse_boolean(tbuf)) {
    sp = args[1];
    process_expression(buff, bp, &sp, executor, caller, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
  } else if (nargs > 2) {
    sp = args[2];
    process_expression(buff, bp, &sp, executor, caller, enactor,
		       PE_DEFAULT, PT_DEFAULT, pe_info);
  }
}

/* ARGSUSED */
FUNCTION(fun_mudname)
{
  safe_str(MUDNAME, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_version)
{
  safe_str(VERSION, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_starttime)
{
  char *s;
  s = (char *) ctime(&first_start_time);
  s[strlen(s) - 1] = '\0';
  if (s[8] == ' ')
    s[8] = '0';
  safe_str(s, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_restarttime)
{
  char *s;
  s = (char *) ctime(&start_time);
  s[strlen(s) - 1] = '\0';
  if (s[8] == ' ')
    s[8] = '0';
  safe_str(s, buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_restarts)
{
  safe_integer(reboot_count, buff, bp);
}

/* Data for soundex functions */
static char soundex_val[26] = {
  0, 1, 2, 3, 0, 1, 2, 0, 0,
  2, 2, 4, 5, 5, 0, 1, 2, 6,
  2, 3, 0, 1, 0, 2, 0, 2
};

/* The actual soundex routine */
static char *
soundex(str)
    char *str;
{
  static char tbuf1[BUFFER_LEN];
  char *p, *q;

  tbuf1[0] = '\0';
  tbuf1[1] = '\0';
  tbuf1[2] = '\0';
  tbuf1[3] = '\0';

  p = tbuf1;
  q = upcasestr(remove_markup(str, NULL));
  /* First character is just copied */
  *p = *q++;
  /* Special case for PH->F */
  if ((*p == 'P') && *q && (*q == 'H')) {
    *p = 'F';
    q++;
  }
  p++;
  /* Convert letters to soundex values, squash duplicates */
  while (*q) {
    if (!isalpha((unsigned char) *q) || !isascii((unsigned char) *q)) {
      q++;
      continue;
    }
    *p = soundex_val[*q++ - 'A'] + '0';
    if (*p != *(p - 1))
      p++;
  }
  *p = '\0';
  /* Remove zeros */
  p = q = tbuf1;
  while (*q) {
    if (*q != '0')
      *p++ = *q;
    q++;
  }
  *p = '\0';
  /* Pad/truncate to 4 chars */
  if (tbuf1[1] == '\0')
    tbuf1[1] = '0';
  if (tbuf1[2] == '\0')
    tbuf1[2] = '0';
  if (tbuf1[3] == '\0')
    tbuf1[3] = '0';
  tbuf1[4] = '\0';
  return tbuf1;
}

/* ARGSUSED */
FUNCTION(fun_soundex)
{
  /* Returns the soundex code for a word. This 4-letter code is:
   * 1. The first letter of the word (exception: ph -> f)
   * 2. Replace each letter with a numeric code from the soundex table
   * 3. Remove consecutive numbers that are the same
   * 4. Remove 0's
   * 5. Truncate to 4 characters or pad with 0's.
   * It's actually a bit messier than that to make it faster.
   */
  if (!args[0] || !*args[0] || !isalpha((unsigned char) *args[0])
      || strchr(args[0], ' ')) {
    safe_str(T("#-1 FUNCTION (SOUNDEX) REQUIRES A SINGLE WORD ARGUMENT"), buff,
	     bp);
    return;
  }
  safe_str(soundex(args[0]), buff, bp);
  return;
}

/* ARGSUSED */
FUNCTION(fun_soundlike)
{
  /* Return 1 if the two arguments have the same soundex.
   * This can be optimized to go character-by-character, but
   * I deem the modularity to be more important. So there.
   */
  char tbuf1[5];
  if (!*args[0] || !*args[1] || !isalpha(*args[0]) || !isalpha(*args[1]) ||
      strchr(args[0], ' ') || strchr(args[1], ' ')) {
    safe_str(T("#-1 FUNCTION (SOUNDLIKE) REQUIRES TWO ONE-WORD ARGUMENTS"),
	     buff, bp);
    return;
  }
  /* soundex uses a static buffer, so we need to save it */
  strcpy(tbuf1, soundex(args[0]));
  safe_boolean(!strcmp(tbuf1, soundex(args[1])), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_functions)
{
  safe_str(list_functions(), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_null)
{
  return;
}

/* ARGSUSED */
FUNCTION(fun_atat)
{
  return;
}

/* ARGSUSED */
FUNCTION(fun_list)
{
  if (!args[0] || !*args[0])
    safe_str("#-1", buff, bp);
  else if (string_prefix("motd", args[0]))
    safe_str(cf_motd_msg, buff, bp);
  else if (string_prefix("wizmotd", args[0]) && Hasprivs(executor))
    safe_str(cf_wizmotd_msg, buff, bp);
  else if (string_prefix("downmotd", args[0]) && Hasprivs(executor))
    safe_str(cf_downmotd_msg, buff, bp);
  else if (string_prefix("fullmotd", args[0]) && Hasprivs(executor))
    safe_str(cf_fullmotd_msg, buff, bp);
  else if (string_prefix("functions", args[0]))
    safe_str(list_functions(), buff, bp);
  else if (string_prefix("commands", args[0]))
    safe_str(list_commands(), buff, bp);
  else if (string_prefix("attribs", args[0]))
    safe_str(list_attribs(), buff, bp);
  else
    safe_str("#-1", buff, bp);
  return;
}

/* ARGSUSED */
FUNCTION(fun_scan)
{
  dbref thing;

  thing = match_thing(executor, args[0]);
  if (!GoodObject(thing)) {
    safe_str(T(e_notvis), buff, bp);
    return;
  }
  if (!See_All(executor) && !controls(executor, thing)) {
    notify(executor, T("Permission denied."));
    safe_str("#-1", buff, bp);
    return;
  }
  safe_str(scan_list(thing, args[1]), buff, bp);
}
