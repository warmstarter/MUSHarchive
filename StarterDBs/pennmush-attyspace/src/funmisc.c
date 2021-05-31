#include "copyrite.h"

#include "config.h"
#include <time.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#include "conf.h"
#include "externs.h"
#include "version.h"
#include "intrface.h"
#include "htab.h"
#include "parse.h"
#include "command.h"
#include "function.h"
#include "confmagic.h"

#ifdef WIN32
#pragma warning( disable : 4761)	/* NJG: disable warning re conversion */
#endif

extern time_t start_time;
extern FUN flist[];
static char *soundex _((char *str));
extern void do_emit _((dbref player, const char *tbuf1));
extern void do_remit _((dbref player, const char *arg1, const char *arg2));
extern void do_lemit _((dbref player, const char *tbuf1));
extern void do_zemit _((dbref player, const char *arg1, const char *arg2));
extern void do_oemit _((dbref player, const char *arg1, const char *arg2));

extern HASHTAB htab_function;

#include "funcrypt.c"

/* ARGSUSED */
FUNCTION(fun_valid)
{
  /* Checks to see if a given <something> is valid as a parameter of a
   * given type (such as an object name.
   */
  /* This is an undocumented function.  If the first arg is "name",
   * then check to see if the second arg is a legal object name.
   */

  if (!args[0] || !*args[0])
    safe_str("#-1", buff, bp);
  else if (!strcasecmp(args[0], "name"))
    safe_chr(ok_name(args[1]) ? '1' : '0', buff, bp);
  else
    safe_str("#-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_pemit)
{
  if (!command_check_byname(executor, "@pemit")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  if (Gagged(executor))
    return;
  do_pemit_list(executor, args[0], args[1]);
}

/* ARGSUSED */
FUNCTION(fun_oemit)
{
  if (!command_check_byname(executor, "@oemit")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  if (Gagged(executor))
    return;
  do_oemit(executor, args[0], args[1]);
}

/* ARGSUSED */
FUNCTION(fun_emit)
{
  if (!command_check_byname(executor, "@emit")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  if (Gagged(executor))
    return;
  do_emit(executor, args[0]);
}

/* ARGSUSED */
FUNCTION(fun_remit)
{
  if (!command_check_byname(executor, "@remit")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  if (Gagged(executor))
    return;
  do_remit(executor, args[0], args[1]);
}

/* ARGSUSED */
FUNCTION(fun_lemit)
{
  if (!command_check_byname(executor, "@lemit")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  if (Gagged(executor))
    return;
  do_lemit(executor, args[0]);
}

/* ARGSUSED */
FUNCTION(fun_zemit)
{
  if (!command_check_byname(executor, "@zemit")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  if (Gagged(executor))
    return;
  do_zemit(executor, args[0], args[1]);
}

#ifdef CHAT_SYSTEM
/* ARGSUSED */
FUNCTION(fun_cemit)
{
  if (!command_check_byname(executor, "@cemit")) {
    safe_str("#-1 PERMISSION DENIED", buff, bp);
    return;
  }
  if (Gagged(executor))
    return;
  do_cemit(executor, args[0], args[1], 0);
}
#endif


/* ARGSUSED */
FUNCTION(fun_setq)
{
  /* sets a variable into a local register */
  int r;

  if (!is_integer(args[0])) {
    safe_str(e_int, buff, bp);
    return;
  }
  r = parse_integer(args[0]);
  if ((r < 0) || (r > 9)) {
    safe_str("#-1 REGISTER OUT OF RANGE", buff, bp);
    return;
  }
  strcpy(renv[r], args[1]);
  if (!strcmp(called_as, "SETR"))
    safe_str(args[1], buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_r)
{
  /* returns a local register */
  int r;

  if (!is_integer(args[0])) {
    safe_str(e_int, buff, bp);
    return;
  }
  r = parse_integer(args[0]);
  if ((r < 0) || (r > 9)) {
    safe_str("#-1 REGISTER OUT OF RANGE", buff, bp);
    return;
  }
  if (renv[r])
    safe_str(renv[r], buff, bp);
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
  if (!is_integer(args[0])) {
    safe_str(e_int, buff, bp);
    return;
  }
  safe_str(unparse_integer(getrandom(parse_integer(args[0]))), buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_die)
{
  int n;
  int die;
  int count;
  int total = 0;

  if (!is_integer(args[0]) || !is_integer(args[1])) {
    safe_str(e_ints, buff, bp);
    return;
  }
  n = parse_integer(args[0]);
  die = parse_integer(args[1]);

  if ((n < 0) || (n > 20)) {
    safe_str("#-1 NUMBER OUT OF RANGE", buff, bp);
    return;
  }
  for (count = 0; count < n; count++)
    total += getrandom(die) + 1;

  safe_str(unparse_integer(total), buff, bp);
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

  int j;
  char mstr[BUFFER_LEN], pstr[BUFFER_LEN], *dp;
  char const *sp;
  char *tbuf1;

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

    if (local_wild_match(pstr, mstr)) {
      /* If there's a #$ in a switch's action-part, replace it with
       * the value of the conditional (mstr) before evaluating it.
       */
      tbuf1 = replace_string("#$", mstr, args[j + 1]);
      sp = tbuf1;
      process_expression(buff, bp, &sp,
			 executor, caller, enactor,
			 PE_DEFAULT, PT_DEFAULT, pe_info);
      mush_free((Malloc_t) tbuf1, "replace_string.buff");
      return;
    }
  }

  if (!(nargs & 1)) {
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
  char tbuf1[BUFFER_LEN];
  strcpy(tbuf1, ctime(&start_time));
  tbuf1[strlen(tbuf1) - 1] = '\0';
  safe_str(tbuf1, buff, bp);
}

/* Data for soundex functions */
static char soundex_val[26] =
{
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
  q = upcasestr(str);
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
    if (!isalpha(*q)) {
      q++;
      continue;
    }
    *p = soundex_val[*q++ - 0x41] + '0';
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
  if (!args[0] || !*args[0] || !isalpha(*args[0]) || strchr(args[0], ' ')) {
    safe_str("#-1 FUNCTION (SOUNDEX) REQUIRES A SINGLE WORD ARGUMENT", buff, bp);
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
    safe_str("#-1 FUNCTION (SOUNDLIKE) REQUIRES TWO ONE-WORD ARGUMENTS", buff, bp);
    return;
  }
  /* soundex uses a static buffer, so we need to save it */
  strcpy(tbuf1, soundex(args[0]));
  if (!strcmp(tbuf1, soundex(args[1]))) {
    safe_str("1", buff, bp);
  } else {
    safe_str("0", buff, bp);
  }
  return;
}

/* ARGSUSED */
FUNCTION(fun_functions)
{
  FUN *fp;
  char *ptrs[BUFFER_LEN / 2];
  int nptrs = 0, i;
  fp = hash_firstentry(&htab_function);
  while (fp) {
    ptrs[nptrs++] = (char *) fp->name;
    fp = hash_nextentry(&htab_function);
  }
  do_gensort(ptrs, nptrs, 0);
  for (i = 0; i < nptrs; i++) {
    if (i)
      safe_chr(' ', buff, bp);
    safe_str(ptrs[i], buff, bp);
  }
}
