
#include "copyrite.h"

#include "config.h"
#include "conf.h"
#include "externs.h"
#include "intrface.h"
#include "match.h"
#include "parse.h"
#include "mymalloc.h"
#include "confmagic.h"

void do_userfn _((char *buff, char **bp, dbref obj, ATTR *attrib, int nargs, char **args, dbref executor, dbref caller, dbref enactor, PE_Info * pe_info));

/* ARGSUSED */
FUNCTION(fun_s)
{
  char const *p;
  p = args[0];
  process_expression(buff, bp, &p, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  return;
}

/* ARGSUSED */
FUNCTION(fun_objeval)
{
  char name[BUFFER_LEN];
  char *s;
  char const *p;
  dbref obj;

  /* First, we evaluate our first argument so people can use 
   * functions on it.
   */
  s = name;
  p = args[0];
  process_expression(name, &s, &p, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  *s = '\0';

#ifdef FUNCTION_SIDE_EFFECTS
  /* The security hole created by function side effects is too great
   * to allow a see_all player to evaluate functions from someone else's
   * standpoint. We require control.
   */
  if (((obj = match_thing(executor, name)) == NOTHING) ||
      !controls(executor, obj))
    obj = executor;
#else
  /* In order to evaluate from something else's viewpoint, you
   * must control it, or be able to see_all.
   */
  if (((obj = match_thing(executor, name)) == NOTHING) ||
      (!controls(executor, obj) && !See_All(executor)))
    obj = executor;
#endif

  p = args[1];
  process_expression(buff, bp, &p, obj, executor, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
}

void
do_userfn(buff, bp, obj, attrib, nargs, args,
	  executor, caller, enactor, pe_info)
    char *buff, **bp;
    dbref obj;
    ATTR *attrib;
    int nargs;
    char **args;
    dbref executor, caller, enactor;
    PE_Info *pe_info;
{
  int j;
  char *tptr[10];
  char *tbuf;
  char const *tp;

  /* save our stack */
  for (j = 0; j < 10; j++)
    tptr[j] = wenv[j];

  /* copy the appropriate args into the stack */
  if (nargs > 10)
    nargs = 10;			/* maximum ten args */
  for (j = 0; j < nargs; j++)
    wenv[j] = args[j];
  for (; j < 10; j++)
    wenv[j] = NULL;

  tp = tbuf = safe_uncompress(attrib->value);
  process_expression(buff, bp, &tp, obj, executor, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  free(tbuf);

  /* restore the stack */
  for (j = 0; j < 10; j++)
    wenv[j] = tptr[j];
}

/* ARGSUSED */
FUNCTION(fun_ufun)
{
  ATTR *attrib;
  dbref obj;

  /* find the user function attribute */
  parse_attrib(executor, args[0], &obj, &attrib);
  if (!GoodObject(obj)) {
    safe_str("#-1 INVALID OBJECT", buff, bp);
    return;
  }
  if (attrib && Can_Read_Attr(executor, obj, attrib)) {
    if (!CanEval(executor, obj)) {
      safe_str("#-1 PERMISSION DENIED", buff, bp);
      return;
    }
    do_userfn(buff, bp, obj, attrib, nargs - 1, args + 1,
	      executor, caller, enactor, pe_info);
    return;
  } else if (attrib || !Can_Examine(executor, obj)) {
    safe_str("#-1 NO PERMISSION TO GET ATTRIBUTE", buff, bp);
    return;
  }
  return;
}

/* ARGSUSED */
FUNCTION(fun_ulocal)
{
  /* Like fun_ufun, but saves the state of the q0-q9 registers
   * when called
   */
  ATTR *attrib;
  dbref obj;
  char *preserve[10];

  /* find the user function attribute */
  parse_attrib(executor, args[0], &obj, &attrib);
  if (!GoodObject(obj)) {
    safe_str("#-1 INVALID OBJECT", buff, bp);
    return;
  }
  if (attrib && Can_Read_Attr(executor, obj, attrib)) {
    if (!CanEval(executor, obj)) {
      safe_str("#-1 PERMISSION DENIED", buff, bp);
      return;
    }
    save_global_regs("ulocal.save", preserve);
    do_userfn(buff, bp, obj, attrib, nargs - 1, args + 1,
	      executor, caller, enactor, pe_info);
    restore_global_regs("ulocal.save", preserve);
    return;
  } else if (attrib || !Can_Examine(executor, obj)) {
    safe_str("#-1 NO PERMISSION TO GET ATTRIBUTE", buff, bp);
    return;
  }
  return;
}

/* Like fun_ufun, but takes as second argument a default message
 * to use if the attribute isn't there
 */
/* ARGSUSED */
FUNCTION(fun_udefault)
{
  dbref thing;
  ATTR *attrib;
  char *dp;
  char const *sp;
  char mstr[BUFFER_LEN];
  char **xargs;
  int i;

  /* find our object and attribute */
  dp = mstr;
  sp = args[0];
  process_expression(mstr, &dp, &sp, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  *dp = '\0';
  parse_attrib(executor, mstr, &thing, &attrib);
  if (GoodObject(thing) && attrib && CanEval(executor, thing) &&
      Can_Read_Attr(executor, thing, attrib)) {
    /* Ok, we've got it */
    /* We must now evaluate all the arguments from args[2] on and
     * pass them to the function */
    xargs = NULL;
    if (nargs > 2) {
      xargs = (char **) mush_malloc((nargs - 2) * sizeof(char *), "udefault.xargs");
      for (i = 0; i < nargs - 2; i++) {
	xargs[i] = (char *) mush_malloc(BUFFER_LEN, "udefault");
	dp = xargs[i];
	sp = args[i + 2];
	process_expression(xargs[i], &dp, &sp, executor, caller, enactor,
			   PE_DEFAULT, PT_DEFAULT, pe_info);
	*dp = '\0';
      }
    }
    do_userfn(buff, bp, thing, attrib, nargs - 2, xargs,
	      executor, caller, enactor, pe_info);

    /* Free the xargs */
    if (nargs > 2) {
      for (i = 0; i < nargs - 2; i++)
	mush_free(xargs[i], "udefault");
      mush_free(xargs, "udefault.xargs");
    }
    return;
  }
  /* We couldn't get it. Evaluate args[1] and return it */
  sp = args[1];
  process_expression(buff, bp, &sp, executor, caller, enactor,
		     PE_DEFAULT, PT_DEFAULT, pe_info);
  return;
}


/* ARGSUSED */
FUNCTION(fun_zfun)
{
  ATTR *attrib;
  dbref zone;

  zone = Zone(executor);

  if (zone == NOTHING) {
    safe_str("#-1 INVALID ZONE", buff, bp);
    return;
  }
  /* find the user function attribute */
  attrib = atr_get(zone, upcasestr(args[0]));
  if (attrib && Can_Read_Attr(executor, zone, attrib)) {
    if (!CanEval(executor, zone)) {
      safe_str("#-1 PERMISSION DENIED", buff, bp);
      return;
    }
    do_userfn(buff, bp, zone, attrib, nargs - 1, args + 1,
	      executor, caller, enactor, pe_info);
    return;
  } else if (attrib || !Can_Examine(executor, zone)) {
    safe_str("#-1 NO PERMISSION TO GET ATTRIBUTE", buff, bp);
    return;
  }
  safe_str("#-1 NO SUCH USER FUNCTION", buff, bp);
  return;
}
