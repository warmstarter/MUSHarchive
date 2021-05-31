/* sig.c */
/* Redefine signal() in an attempt to get consistent semantics.  This
   requires sigaction to work. Otherwise, we just live with it. */

#include "config.h"
#include "externs.h"
#include "confmagic.h"

#ifdef HAS_SIGACTION

/* Get SIG constants */
#include <signal.h>

/* If signal's a macro, undefine it */
#ifdef signal
#undef signal
#endif

#ifdef CAN_PROTOTYPE_SIGNAL
Sigfunc signal _((int signo, Sigfunc func));
#endif

/* We're going to rewrite the signal() function in terms of
 * sigaction, where available, to ensure consistent semantics.
 * We want signal handlers to remain installed, and we want
 * signals (except SIGALRM) to restart system calls which they
 * interrupt. This is how bsd signals work, and what we'd like.
 * This function is essentially example 10.12 from Stevens'
 * _Advanced Programming in the Unix Environment_
 */
Sigfunc
signal(signo, func)
    int signo;
    Sigfunc func;
{
  struct sigaction act, oact;
  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
#ifdef NEVER
  /* In Steven's original routine, he didn't want to restart
   * system calls after sigalarm. That seems wrong for our
   * application.
   */
  if (signo = SIGALRM) {
#ifdef SA_INTERRUPT
    /* SunOS restarts signals by default unless you add this */
    act.sa_flags |= SA_INTERRUPT;
#endif
  } else {
#endif				/* NEVER */
#ifdef SA_RESTART
    act.sa_flags |= SA_RESTART;
#endif
#ifdef NEVER
  }
#endif				/* NEVER */
  if (sigaction(signo, &act, &oact) < 0)
    return (SIG_ERR);
  return (oact.sa_handler);
}

#endif				/* HAS_SIGACTION */
