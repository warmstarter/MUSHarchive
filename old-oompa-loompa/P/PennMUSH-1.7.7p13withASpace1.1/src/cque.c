/* cque.c */

#include "copyrite.h"
#include "config.h"

#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif

#include "conf.h"
#include "command.h"
#include "mushdb.h"
#include "match.h"
#include "externs.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "parse.h"
#include "strtree.h"
#include "mymalloc.h"
#include "game.h"
#include "attrib.h"
#include "flags.h"
#include "dbdefs.h"
#include "log.h"
#include "confmagic.h"


extern char ccom[];
extern dbref cplr;

char *wenv[10];			/* working environment (wptr/rptr equiv) */
char renv[NUMQ][BUFFER_LEN];
char *wnxt[10], *rnxt[NUMQ];	/* stuff to be shoved into the queue */
int break_called = 0;
typedef struct bque BQUE;

/** A queue entry.
 * This structure reprsents a queue entry on a linked list of queue
 * entries (a queue). It is used for all of the queues.
 */
struct bque {
  BQUE *next;			/**< pointer to next entry on queue */
  dbref player;			/**< player who will do command */
  dbref queued;			/**< object whose QUEUE gets incremented for this command */
  dbref cause;			/**< player causing command (for %N) */
  dbref sem;			/**< semaphore object to block on */
  char *semattr;		/**< semaphore attribute to block on */
  int left;			/**< seconds left until execution */
  char *env[10];		/**< environment, from wild match */
  char *rval[NUMQ];		/**< environment, from setq() */
  char *comm;			/**< command to be executed */
};

static BQUE *qfirst = NULL, *qlast = NULL, *qwait = NULL;
static BQUE *qlfirst = NULL, *qllast = NULL;
static BQUE *qsemfirst = NULL, *qsemlast = NULL;

static int add_to_generic(dbref player, int am, const char *name, int flags);
static int add_to(dbref player, int am);
static int add_to_sem(dbref player, int am, const char *name);
static int queue_limit(dbref player);
void free_qentry(BQUE *point);
static int pay_queue(dbref player, const char *command);
void wait_que(dbref player, int wait, char *command,
	      dbref cause, dbref sem, const char *semattr, int until);
int que_next(void);

static void show_queue(dbref player, dbref victim, int q_type,
		       int q_quiet, int q_all, BQUE *q_ptr, int *tot, int *self,
		       int *del);
static void do_raw_restart(dbref victim);
static int waitable_attr(dbref thing, const char *atr);

extern sig_atomic_t cpu_time_limit_hit;

#define SEMAPHORE_FLAGS (AF_LOCKED | AF_PRIVATE | AF_NOCOPY | AF_NODUMP)
/* Returns true if the attribute on thing can be used as a semaphore.
 * atr should be given in UPPERCASE. 
 */
static int
waitable_attr(dbref thing, const char *atr)
{
  ATTR *a;
  if (!atr || !*atr)
    return 0;
  a = atr_get_noparent(thing, atr);
  if (!a) {			/* Attribute isn't set */
    a = atr_match(atr);
    if (!a)			/* It's not a built in attribute */
      return 1;
    return !strcmp(AL_NAME(a), "SEMAPHORE");	/* Only allow SEMAPHORE for now */
  } else {			/* Attribute is set. Check for proper owner and flags and value */
    if ((AL_CREATOR(a) == GOD) && (AL_FLAGS(a) == SEMAPHORE_FLAGS)) {
      char *v = uncompress(AL_STR(a));
      if (!*v || is_integer(v))
	return 1;
      else
	return 0;
    } else {
      return 0;
    }
  }
  return 0;			/* Not reached */
}

static int
add_to_generic(dbref player, int am, const char *name, int flags)
{
  int num = 0;
  ATTR *a;
  char buff[MAX_COMMAND_LEN];
  a = atr_get_noparent(player, name);
  if (a)
    num = parse_integer(uncompress(AL_STR(a)));
  num += am;
  if (num) {
    sprintf(buff, "%d", num);
    (void) atr_add(player, name, buff, GOD, flags);
  } else {
    (void) atr_clr(player, name, GOD);
  }
  return (num);
}

static int
add_to(dbref player, int am)
{
  return (add_to_generic(player, am, "QUEUE", NOTHING));
}

static int
add_to_sem(dbref player, int am, const char *name)
{
  return (add_to_generic
	  (player, am, name ? name : "SEMAPHORE", SEMAPHORE_FLAGS));
}

static int
queue_limit(dbref player)
{
  /* returns 1 if player has exceeded his queue limit, and always
     increments QUEUE by one. */
  int nlimit;

  nlimit = add_to(player, 1);
  if (HugeQueue(player))
    return nlimit > (QUEUE_QUOTA + db_top);
  else
    return nlimit > QUEUE_QUOTA;
}

/** Free a queue entry.
 * \param point queue entry to free.
 */
void
free_qentry(BQUE *point)
{
  int a;
  for (a = 0; a < 10; a++)
    if (point->env[a]) {
      mush_free((Malloc_t) point->env[a], "bqueue_env");
    }
  for (a = 0; a < NUMQ; a++)
    if (point->rval[a]) {
      mush_free((Malloc_t) point->rval[a], "bqueue_rval");
    }
  if (point->semattr)
    mush_free((Malloc_t) point->semattr, "bqueue_semattr");
  if (point->comm)
    mush_free((Malloc_t) point->comm, "bqueue_comm");
  mush_free((Malloc_t) point, "BQUE");
}

static int
pay_queue(dbref player, const char *command)
{
  int estcost;
  estcost = QUEUE_COST + ((get_random_long(0, QUEUE_LOSS - 1) == 0) ? 1 : 0);
  if (!payfor(player, estcost)) {
    notify(Owner(player), T("Not enough money to queue command."));
    return 0;
  }
  if (queue_limit(QUEUE_PER_OWNER ? Owner(player) : player)) {
    notify_format(Owner(player),
		  T("Runaway object: %s(%s). Commands halted."),
		  Name(player), unparse_dbref(player));
    do_log(LT_TRACE, player, player, T("Runaway object %s executing: %s"),
	   unparse_dbref(player), command);
    /* Refund the queue costs */
    giveto(player, QUEUE_COST);
    add_to(QUEUE_PER_OWNER ? Owner(player) : player, -1);
    /* wipe out that object's queue and set it HALT */
    do_halt(Owner(player), "", player);
    set_flag_internal(player, "HALT");
    return 0;
  }
  return 1;
}

/** Add a new entry onto the player or object command queues.
 * This function adds a new entry to the back of the player or
 * object command queues (depending on whether the call was
 * caused by a player or an object).
 * \param player the enactor for the queued command.
 * \param command the command to enqueue.
 * \param cause the player or object causing the command to be queued.
 */
void
parse_que(dbref player, const char *command, dbref cause)
{
  int a;
  BQUE *tmp;
  if (!IsPlayer(player) && (Halted(player)))
    return;
  if (!pay_queue(player, command))	/* make sure player can afford to do it */
    return;
  tmp = (BQUE *) mush_malloc(sizeof(BQUE), "BQUE");
  tmp->comm = mush_strdup(command, "bqueue_comm");
  tmp->semattr = NULL;
  tmp->player = player;
  tmp->queued = QUEUE_PER_OWNER ? Owner(player) : player;
  tmp->next = NULL;
  tmp->left = 0;
  tmp->cause = cause;
  for (a = 0; a < 10; a++)
    if (!wnxt[a])
      tmp->env[a] = NULL;
    else {
      tmp->env[a] = mush_strdup(wnxt[a], "bqueue_env");
    }
  for (a = 0; a < NUMQ; a++)
    if (!rnxt[a] || !rnxt[a][0])
      tmp->rval[a] = NULL;
    else {
      tmp->rval[a] = mush_strdup(rnxt[a], "bqueue_rval");
    }
  if (IsPlayer(cause)) {
    if (qlast) {
      qlast->next = tmp;
      qlast = tmp;
    } else
      qlast = qfirst = tmp;
  } else {
    if (qllast) {
      qllast->next = tmp;
      qllast = tmp;
    } else
      qllast = qlfirst = tmp;
  }
}


/** Enqueue the action part of an attribute.
 * This function is a front-end to parse_que() that takes an attribute, 
 * removes ^....: or $....: from its value, and queues what's left.
 * \param executor object containing the attribute.
 * \param atrname attribute name.
 * \param enactor the enactor.
 * \param noparent if true, parents of executor are not checked for atrname.
 * \retval 0 failure.
 * \retval 1 success.
 */
int
queue_attribute_base(dbref executor, const char *atrname, dbref enactor,
		     int noparent)
{
  ATTR *a;
  char *start, *command;

  a = (noparent ? atr_get_noparent(executor, strupper(atrname)) :
       atr_get(executor, strupper(atrname)));
  if (!a)
    return 0;
  start = safe_uncompress(AL_STR(a));
  command = start;
  /* Trim off $-command or ^-command prefix */
  if (*command == '$' || *command == '^') {
    do {
      command = strchr(command + 1, ':');
    } while (command && command[-1] == '\\');
    if (!command)
      /* Oops, had '$' or '^', but no ':' */
      command = start;
    else
      /* Skip the ':' */
      command++;
  }
  parse_que(executor, command, enactor);
  free(start);
  return 1;
}

/** Queue an entry on the wait or semaphore queues.
 * This function creates and adds a queue entry to the wait queue
 * or the semaphore queue. Wait queue entries are sorted by when
 * they're due to expire; semaphore queue entries are just added
 * to the back of the queue.
 * \param player the enqueuing object.
 * \param wait time to wait, or 0.
 * \param command command to enqueue.
 * \param cause object that caused command to be enqueued.
 * \param sem object to serve as a semaphore, or NOTHING.
 * \param semattr attribute to serve as a semaphore, or NULL (to use SEMAPHORE).
 * \param until 1 if we wait until an absolute time.
 */
void
wait_que(dbref player, int wait, char *command, dbref cause, dbref sem,
	 const char *semattr, int until)
{
  BQUE *tmp;
  int a;
  if (wait == 0) {
    if (sem != NOTHING)
      add_to_sem(sem, -1, semattr);
    parse_que(player, command, cause);
    return;
  }
  if (!pay_queue(player, command))	/* make sure player can afford to do it */
    return;
  tmp = (BQUE *) mush_malloc(sizeof(BQUE), "BQUE");
  tmp->comm = mush_strdup(command, "bqueue_comm");
  tmp->player = player;
  tmp->queued = QUEUE_PER_OWNER ? Owner(player) : player;
  tmp->cause = cause;
  tmp->semattr = NULL;
  tmp->next = NULL;
  for (a = 0; a < 10; a++) {
    if (!wnxt[a])
      tmp->env[a] = NULL;
    else {
      tmp->env[a] = mush_strdup(wnxt[a], "bqueue_env");
    }
  }
  for (a = 0; a < NUMQ; a++) {
    if (!rnxt[a] || !rnxt[a][0])
      tmp->rval[a] = NULL;
    else {
      tmp->rval[a] = mush_strdup(rnxt[a], "bqueue_rval");
    }
  }

  if (until) {
    tmp->left = wait;
  } else {
    if (wait >= 0)
      tmp->left = mudtime + wait;
    else
      tmp->left = 0;		/* semaphore wait without a timeout */
  }
  tmp->sem = sem;
  if (sem == NOTHING) {
    /* No semaphore, put on normal wait queue, sorted by time */
    BQUE *point, *trail = NULL;

    for (point = qwait;
	 point && (point->left <= tmp->left); point = point->next)
      trail = point;

    tmp->next = point;
    if (trail != NULL)
      trail->next = tmp;
    else
      qwait = tmp;
  } else {

    /* Put it on the end of the semaphore queue */
    tmp->semattr =
      mush_strdup(semattr ? semattr : "SEMAPHORE", "bqueue_semattr");
    if (qsemlast != NULL) {
      qsemlast->next = tmp;
      qsemlast = tmp;
    } else {
      qsemfirst = qsemlast = tmp;
    }
  }
}

/** Once-a-second check for queued commands.
 * This function is called every second to check for commands
 * on the wait queue or semaphore queue, and to move a command
 * off the low priority object queue and onto the normal priority
 * player queue.
 */
void
do_second(void)
{
  BQUE *trail = NULL, *point, *next;
  /* move contents of low priority queue onto end of normal one 
   * this helps to keep objects from getting out of control since 
   * its effects on other objects happen only after one second 
   * this should allow @halt to be typed before getting blown away 
   * by scrolling text.
   */
  if (qlfirst) {
    if (qlast)
      qlast->next = qlfirst;
    else
      qfirst = qlfirst;
    qlast = qllast;
    qllast = qlfirst = NULL;
  }
  /* check regular wait queue */

  while (qwait && qwait->left <= mudtime) {
    point = qwait;
    qwait = point->next;
    point->next = NULL;
    point->left = 0;
    if (IsPlayer(point->cause)) {
      if (qlast) {
	qlast->next = point;
	qlast = point;
      } else
	qlast = qfirst = point;
    } else {
      if (qllast) {
	qllast->next = point;
	qllast = point;
      } else
	qllast = qlfirst = point;
    }
  }

  /* check for semaphore timeouts */

  for (point = qsemfirst, trail = NULL; point; point = next) {
    if (point->left == 0 || point->left > mudtime) {
      next = (trail = point)->next;
      continue;			/* skip non-timed and those that haven't gone off yet */
    }
    if (trail != NULL)
      trail->next = next = point->next;
    else
      qsemfirst = next = point->next;
    if (point == qsemlast)
      qsemlast = trail;
    add_to_sem(point->sem, -1, point->semattr);
    point->sem = NOTHING;
    point->next = NULL;
    if (IsPlayer(point->cause)) {
      if (qlast) {
	qlast->next = point;
	qlast = point;
      } else
	qlast = qfirst = point;
    } else {
      if (qllast) {
	qllast->next = point;
	qllast = point;
      } else
	qllast = qlfirst = point;
    }
  }
}

/** Execute some commands from the top of the queue.
 * This function dequeues and executes commands on the normal
 * priority (player) queue.
 * \param ncom number of commands to execute.
 * \return number of commands executed.
 */
int
do_top(ncom)
    int ncom;
{
  int a, i;
  BQUE *entry;
  char tbuf[BUFFER_LEN];
  char *r;
  char const *s;

  for (i = 0; i < ncom; i++) {

    if (!qfirst)
      return i;
    /* We must dequeue before execution, so that things like
     * queued @kick or @ps get a sane queue image.
     */
    entry = qfirst;
    if (!(qfirst = entry->next))
      qlast = NULL;
    if (GoodObject(entry->player)
	&& !IsGarbage(entry->player)) {
      cplr = entry->player;
      giveto(cplr, QUEUE_COST);
      add_to(entry->queued, -1);
      entry->player = 0;
      if (IsPlayer(cplr) || !Halted(cplr)) {
	for (a = 0; a < 10; a++)
	  wenv[a] = entry->env[a];
	for (a = 0; a < NUMQ; a++) {
	  if (entry->rval[a])
	    strcpy(renv[a], entry->rval[a]);
	  else
	    renv[a][0] = '\0';
	}
	s = entry->comm;
	break_called = 0;
	start_cpu_timer();
	while (!break_called && !cpu_time_limit_hit && *s) {
	  r = ccom;
	  process_expression(ccom, &r, &s, cplr, entry->cause,
			     entry->cause, PE_NOTHING, PT_SEMI, NULL);
	  *r = '\0';
	  if (*s == ';')
	    s++;
	  strcpy(tbuf, ccom);
	  process_command(cplr, tbuf, entry->cause, 0);
	}
	reset_cpu_timer();
      }
    }
    free_qentry(entry);
  }

  return i;
}

/** Determine whether it's time to run a queued command.
 * This function returns the number of seconds we expect to wait
 * before it's time to run a queued command.
 * If there are commands in the player queue, that's 0.
 * If there are commands in the object queue, that's 1.
 * Otherwise, we check wait and semaphore queues to see what's next.
 * \return seconds left before a queue entry will be ready.
 */
int
que_next(void)
{
  int min, curr;
  BQUE *point;
  /* If there are commands in the player queue, they should be run
   * immediately.
   */
  if (qfirst != NULL)
    return 0;
  /* If there are commands in the object queue, they should be run in
   * one second.
   */
  if (qlfirst != NULL)
    return 1;
  /* Check out the wait and semaphore queues, looking for the smallest
   * wait value. Return that - 1, since commands get moved to the player
   * queue when they have one second to go.
   */
  min = 5;

  /* Wait queue is in sorted order so we only have to look at the first
     item on it. Anything else is wasted time. */
  if (qwait) {
    curr = qwait->left - mudtime;
    if (curr <= 2)
      return 1;
    if (curr < min)
      min = curr;
  }

  for (point = qsemfirst; point; point = point->next) {
    if (point->left == 0)	/* no timeout */
      continue;
    curr = point->left - mudtime;
    if (curr <= 2)
      return 1;
    if (curr < min) {
      min = curr;
    }
  }

  return (min - 1);
}

static int
drain_helper(dbref player __attribute__ ((__unused__)), dbref thing,
	     char const *pattern __attribute__ ((__unused__)), ATTR *atr,
	     void *args __attribute__ ((__unused__)))
{
  if (waitable_attr(thing, AL_NAME(atr)))
    (void) atr_clr(thing, AL_NAME(atr), GOD);
  return 0;
}

/** Drain or notify a semaphore.
 * This function dequeues an entry in the semaphore queue and either
 * discards it (drain) or executes it (notify). Maybe more than one.
 * \param thing object serving as semaphore.
 * \param aname attribute serving as semaphore.
 * \param count number of entries to dequeue.
 * \param all if 1, dequeue all entries.
 * \param drain if 1, drain rather than notify the entries.
 */
void
dequeue_semaphores(dbref thing, char const *aname, int count, int all,
		   int drain)
{
  BQUE **point;
  BQUE *entry;

  if (all)
    count = INT_MAX;

  /* Go through the semaphore queue and do it */
  point = &qsemfirst;
  while (*point && count > 0) {
    entry = *point;
    if (entry->sem != thing || (aname && strcmp(entry->semattr, aname))) {
      point = &(entry->next);
      continue;
    }

    /* Remove the queue entry from the semaphore list */
    *point = entry->next;
    entry->next = NULL;
    if (qsemlast == entry) {
      qsemlast = qsemfirst;
      if (qsemlast)
	while (qsemlast->next)
	  qsemlast = qsemlast->next;
    }

    /* Update bookkeeping */
    count--;
    add_to_sem(entry->sem, -1, entry->semattr);

    /* Dispose of the entry as appropriate: discard if @drain, or put
     * into either the player or the object queue. */
    if (drain) {
      giveto(entry->player, QUEUE_COST);
      add_to(entry->queued, -1);
      free_qentry(entry);
    } else if (IsPlayer(entry->cause)) {
      if (qlast) {
	qlast->next = entry;
	qlast = entry;
      } else {
	qlast = qfirst = entry;
      }
    } else {
      if (qllast) {
	qllast->next = entry;
	qllast = entry;
      } else {
	qllast = qlfirst = entry;
      }
    }
  }

  /* If @drain/all, clear the relevant attribute(s) */
  if (drain && all) {
    if (aname)
      (void) atr_clr(thing, aname, GOD);
    else
      atr_iter_get(GOD, thing, "*", drain_helper, NULL);
  }

  /* If @notify and count was higher than the number of queue entries,
   * make the semaphore go negative.  This does not apply to
   * @notify/any or @notify/all. */
  if (!drain && aname && !all && count > 0)
    add_to_sem(thing, -count, aname);
}

COMMAND (cmd_notify_drain) {
  int drain;
  char *pos;
  char const *aname;
  dbref thing;
  int count;
  int all;

  /* Figure out whether we're in notify or drain */
  drain = (cmd->name[1] == 'D');

  /* Make sure they gave an object ref */
  if (!arg_left || !*arg_left) {
    notify(player, T("You must specify an object to use for the semaphore."));
    return;
  }

  /* Figure out which attribute we're using */
  pos = strchr(arg_left, '/');
  if (pos) {
    if (SW_ISSET(sw, SWITCH_ANY)) {
      notify(player,
	     T
	     ("You may not specify a semaphore attribute with the ANY switch."));
      return;
    }
    *pos++ = '\0';
    upcasestr(pos);
    aname = pos;
  } else {
    if (SW_ISSET(sw, SWITCH_ANY)) {
      aname = NULL;
    } else {
      aname = "SEMAPHORE";
    }
  }

  /* Figure out which object we're using */
  thing = noisy_match_result(player, arg_left, NOTYPE, MAT_EVERYTHING);
  if (!GoodObject(thing))
    return;
  /* must control something or have it link_ok in order to use it as 
   * as a semaphore.
   */
  if ((!controls(player, thing) && !LinkOk(thing))
      || (aname && !waitable_attr(thing, aname))) {
    notify(player, T("Permission denied."));
    return;
  }

  /* Figure out how many times to notify */
  all = SW_ISSET(sw, SWITCH_ALL);
  if (arg_right && *arg_right) {
    if (all) {
      notify(player,
	     T("You may not specify a semaphore count with the ALL switch."));
      return;
    }
    if (!is_uinteger(arg_right)) {
      notify(player, T("The semaphore count must be an integer."));
      return;
    }
    count = parse_integer(arg_right);
  } else {
    if (drain)
      all = 1;
    if (all)
      count = INT_MAX;
    else
      count = 1;
  }

  dequeue_semaphores(thing, aname, count, all, drain);

  if (drain) {
    quiet_notify(player, T("Drained."));
  } else {
    quiet_notify(player, T("Notified."));
  }
}

/** Softcode interface to add a command to the wait or semaphore queue.
 * \verbatim
 * This is the top-level function for @wait.
 * \endverbatim
 * \param player the enactor
 * \param cause the object causing the command to be added.
 * \param arg1 the wait time, semaphore object/attribute, or both.
 * \param cmd command to queue.
 * \param until if 1, wait until an absolute time.
 */
void
do_wait(dbref player, dbref cause, char *arg1, char *cmd, int until)
{
  dbref thing;
  char *tcount = NULL, *aname = NULL;
  int waitfor, num;
  ATTR *a;
  char *arg2;
  int j;
  for (j = 0; j < 10; j++)
    wnxt[j] = wenv[j];
  for (j = 0; j < NUMQ; j++)
    rnxt[j] = renv[j];

  arg2 = strip_braces(cmd);
  if (is_strict_integer(arg1)) {
    /* normal wait */
    wait_que(player, parse_integer(arg1), arg2, cause, NOTHING, NULL, until);
    mush_free(arg2, "strip_braces.buff");
    return;
  }
  /* semaphore wait with optional timeout */

  /* find the thing to wait on */
  aname = strchr(arg1, '/');
  if (aname)
    *aname++ = '\0';
  if ((thing =
       noisy_match_result(player, arg1, NOTYPE, MAT_EVERYTHING)) == NOTHING) {
    mush_free(arg2, "strip_braces.buff");
    return;
  }

  /* aname is either time, attribute or attribute/time.
   * After this:
   * tcount will hold string timeout or NULL for none
   * aname will hold attribute name.
   */
  if (aname) {
    tcount = strchr(aname, '/');
    if (!tcount) {
      if (is_strict_integer(aname)) {	/* Timeout */
	tcount = aname;
	aname = (char *) "SEMAPHORE";
      } else {			/* Attribute */
	upcasestr(aname);
      }
    } else {			/* attribute/timeout */
      *tcount++ = '\0';
      upcasestr(aname);
    }
  } else {
    aname = (char *) "SEMAPHORE";
  }

  if ((!controls(player, thing) && !LinkOk(thing))
      || (aname && !waitable_attr(thing, aname))) {
    notify(player, T("Permission denied."));
    mush_free(arg2, "strip_braces.buff");
    return;
  }
  /* get timeout, default of -1 */
  if (tcount && *tcount)
    waitfor = atol(tcount);
  else
    waitfor = -1;
  add_to_sem(thing, 1, aname);
  a = atr_get_noparent(thing, aname);
  if (a)
    num = parse_integer(uncompress(a->value));
  else
    num = 0;
  if (num <= 0) {
    thing = NOTHING;
    waitfor = -1;		/* just in case there was a timeout given */
  }
  wait_que(player, waitfor, arg2, cause, thing, aname, until);
  mush_free(arg2, "strip_braces.buff");
}

static void
show_queue(player, victim, q_type, q_quiet, q_all, q_ptr, tot, self, del)
    dbref player;
    dbref victim;
    int q_type;
    int q_quiet;
    int q_all;
    BQUE *q_ptr;
    int *tot;
    int *self;
    int *del;
{
  BQUE *tmp;
  for (tmp = q_ptr; tmp; tmp = tmp->next) {
    (*tot)++;
    if (!GoodObject(tmp->player))
      (*del)++;
    else if (q_all || (Owner(tmp->player) == victim)) {
      (*self)++;
      if (!q_quiet && (LookQueue(player)
		       || Owns(tmp->player, player))) {
	switch (q_type) {
	case 1:		/* wait queue */
	  notify_format(player, "[%ld]%s:%s", tmp->left - mudtime,
			unparse_object(player, tmp->player), tmp->comm);
	  break;
	case 2:		/* semaphore queue */
	  if (tmp->left != 0) {
	    notify_format(player, "[#%d/%s/%ld]%s:%s", tmp->sem,
			  tmp->semattr, tmp->left - mudtime,
			  unparse_object(player, tmp->player), tmp->comm);
	  } else {
	    notify_format(player, "[#%d/%s]%s:%s", tmp->sem,
			  tmp->semattr, unparse_object(player, tmp->player),
			  tmp->comm);
	  }
	  break;
	default:		/* player or object queue */
	  notify_format(player, "%s:%s", unparse_object(player,
							tmp->player),
			tmp->comm);
	}
      }
    }
  }
}

/** Display a player's queued commands.
 * \verbatim
 * This is the top-level function for @ps.
 * \endverbatim
 * \param player the enactor.
 * \param what name of player whose queue is to be displayed.
 * \param flag type of display. 0 - normal, 1 - all, 2 - summary, 3 - quick
 */
void
do_queue(dbref player, const char *what, int flag)
{
  dbref victim = NOTHING;
  int all = 0;
  int quick = 0;
  int dpq = 0, doq = 0, dwq = 0, dsq = 0;
  int pq = 0, oq = 0, wq = 0, sq = 0;
  int tpq = 0, toq = 0, twq = 0, tsq = 0;
  if (flag == 2 || flag == 3)
    quick = 1;
  if (flag == 1 || flag == 2) {
    all = 1;
    victim = player;
  } else if (LookQueue(player)) {
    if (!what || !*what)
      victim = player;
    else {
      victim = match_result(player, what, TYPE_PLAYER,
			    MAT_PLAYER | MAT_ABSOLUTE | MAT_ME);
    }
  } else {
    victim = player;
  }

  switch (victim) {
  case NOTHING:
    notify(player, T("I couldn't find that player."));
    break;
  case AMBIGUOUS:
    notify(player, T("I don't know who you mean!"));
    break;
  default:

    if (!quick) {
      if (all == 1)
	notify(player, T("Queue for : all"));
      else
	notify_format(player, T("Queue for : %s"), Name(victim));
    }
    victim = Owner(victim);
    if (!quick)
      notify(player, T("Player Queue:"));
    show_queue(player, victim, 0, quick, all, qfirst, &tpq, &pq, &dpq);
    if (!quick)
      notify(player, T("Object Queue:"));
    show_queue(player, victim, 0, quick, all, qlfirst, &toq, &oq, &doq);
    if (!quick)
      notify(player, T("Wait Queue:"));
    show_queue(player, victim, 1, quick, all, qwait, &twq, &wq, &dwq);
    if (!quick)
      notify(player, T("Semaphore Queue:"));
    show_queue(player, victim, 2, quick, all, qsemfirst, &tsq, &sq, &dsq);
    if (!quick)
      notify(player, T("------------  Queue Done  ------------"));
    notify_format(player,
		  "Totals: Player...%d/%d[%ddel]  Object...%d/%d[%ddel]  Wait...%d/%d  Semaphore...%d/%d",
		  pq, tpq, dpq, oq, toq, doq, wq, twq, sq, tsq);
  }
}

/** Halt an object, internal use.
 * This function is used to halt objects by other hardcode.
 * See do_halt1() for the softcode interface.
 * \param owner the enactor.
 * \param ncom command to queue after halting.
 * \param victim object to halt.
 */
void
do_halt(dbref owner, const char *ncom, dbref victim)
{
  BQUE *tmp, *trail = NULL, *point, *next;
  int num = 0;
  dbref player;
  if (victim == NOTHING)
    player = owner;
  else
    player = victim;
  quiet_notify(Owner(player),
	       tprintf("%s: %s(#%d).", T("Halted"), Name(player), player));
  for (tmp = qfirst; tmp; tmp = tmp->next)
    if (GoodObject(tmp->player)
	&& ((tmp->player == player)
	    || (Owner(tmp->player) == player))) {
      num--;
      giveto(player, QUEUE_COST);
      tmp->player = NOTHING;
    }
  for (tmp = qlfirst; tmp; tmp = tmp->next)
    if (GoodObject(tmp->player)
	&& ((tmp->player == player)
	    || (Owner(tmp->player) == player))) {
      num--;
      giveto(player, QUEUE_COST);
      tmp->player = NOTHING;
    }
  /* remove wait q stuff */
  for (point = qwait; point; point = next) {
    if (((point->player == player)
	 || (Owner(point->player) == player))) {
      num--;
      giveto(player, QUEUE_COST);
      if (trail)
	trail->next = next = point->next;
      else
	qwait = next = point->next;
      free_qentry(point);
    } else
      next = (trail = point)->next;
  }

  /* clear semaphore queue */

  for (point = qsemfirst, trail = NULL; point; point = next) {
    if (((point->player == player)
	 || (Owner(point->player) == player))) {
      num--;
      giveto(player, QUEUE_COST);
      if (trail)
	trail->next = next = point->next;
      else
	qsemfirst = next = point->next;
      if (point == qsemlast)
	qsemlast = trail;
      add_to_sem(point->sem, -1, point->semattr);
      free_qentry(point);
    } else
      next = (trail = point)->next;
  }

  add_to(QUEUE_PER_OWNER ? Owner(player) : player, num);

  if (ncom && *ncom) {
    int j;
    for (j = 0; j < 10; j++)
      wnxt[j] = wenv[j];
    for (j = 0; j < NUMQ; j++)
      rnxt[j] = renv[j];
    parse_que(player, ncom, player);
  }
}

/** Halt an object, softcode interface.
 * \verbatim
 * This is the top-level function for @halt.
 * \endverbatim
 * \param player the enactor.
 * \param arg1 string representing object to halt.
 * \param arg2 option string representing command to queue after halting.
 */
void
do_halt1(dbref player, const char *arg1, const char *arg2)
{
  dbref victim;
  if (*arg1 == '\0')
    do_halt(player, "", player);
  else {
    if ((victim =
	 noisy_match_result(player, arg1, NOTYPE, MAT_OBJECTS)) == NOTHING)
      return;
    if (!Owns(player, victim) && !HaltAny(player)) {
      notify(player, T("Permission denied."));
      return;
    }
    if (arg2 && *arg2 && !controls(player, victim)) {
      notify(player, T("You may not use @halt obj=command on this object."));
      return;
    }
    /* If victim's a player, we halt all of their objects */
    /* If not, we halt victim and set the HALT flag if no new command */
    /* was given */
    do_halt(player, arg2, victim);
    if (IsPlayer(victim)) {
      if (victim == player) {
	notify(player, T("All of your objects have been halted."));
      } else {
	notify_format(player,
		      T("All objects for %s have been halted."), Name(victim));
	notify_format(victim,
		      T("All of your objects have been halted by %s."),
		      Name(player));
      }
    } else {
      if (Owner(victim) != player) {
	notify_format(player, "%s: %s's %s(%s)", T("Halted"),
		      Name(Owner(victim)), Name(victim), unparse_dbref(victim));
	notify_format(Owner(victim),
		      "%s: %s(%s), by %s", T("Halted"),
		      Name(victim), unparse_dbref(victim), Name(player));
      }
      if (*arg2 == '\0')
	set_flag_internal(victim, "HALT");
    }
  }
}

/** Halt all objects in the database.
 * \param player the enactor.
 */
void
do_allhalt(dbref player)
{
  dbref victim;
  if (!HaltAny(player)) {
    notify(player,
	   T("You do not have the power to bring the world to a halt."));
    return;
  }
  for (victim = 0; victim < db_top; victim++) {
    if (IsPlayer(victim)) {
      notify_format(victim,
		    T("Your objects have been globally halted by %s"),
		    Name(player));
      do_halt(victim, "", victim);
    }
  }
}

/** Restart all objects in the database.
 * \verbatim
 * A restart is a halt and then triggering the @startup.
 * \endverbatim
 * \param player the enactor.
 */
void
do_allrestart(dbref player)
{
  dbref thing;

  if (!HaltAny(player)) {
    notify(player, T("You do not have the power to restart the world."));
    return;
  }
  do_allhalt(player);
  for (thing = 0; thing < db_top; thing++) {
    if (!IsGarbage(thing) && !(Halted(thing)))
      (void) queue_attribute_noparent(thing, "STARTUP", thing);
    if (IsPlayer(thing)) {
      notify_format(thing,
		    T("Your objects are being globally restarted by %s"),
		    Name(player));
    }
  }
}

static void
do_raw_restart(victim)
    dbref victim;
{
  dbref thing;

  if (IsPlayer(victim)) {
    for (thing = 0; thing < db_top; thing++) {
      if ((Owner(thing) == victim) && !IsGarbage(thing)
	  && !(Halted(thing)))
	(void) queue_attribute_noparent(thing, "STARTUP", thing);
    }
  } else {
    /* A single object */
    if (!IsGarbage(victim) && !(Halted(victim)))
      (void) queue_attribute_noparent(victim, "STARTUP", victim);
  }
}

/** Restart an object.
 * \param player the enactor.
 * \param arg1 string representing the object to restart.
 */
void
do_restart_com(dbref player, const char *arg1)
{
  dbref victim;
  if (*arg1 == '\0') {
    do_halt(player, "", player);
    do_raw_restart(player);
  } else {
    if ((victim =
	 noisy_match_result(player, arg1, NOTYPE, MAT_OBJECTS)) == NOTHING)
      return;
    if (!Owns(player, victim) && !HaltAny(player)) {
      notify(player, T("Permission denied."));
      return;
    }
    if (Owner(victim) != player) {
      if (IsPlayer(victim)) {
	notify_format(player,
		      T("All objects for %s are being restarted."),
		      Name(victim));
	notify_format(victim,
		      T("All of your objects are being restarted by %s."),
		      Name(player));
      } else {
	notify_format(player,
		      "Restarting: %s's %s(%s)",
		      Name(Owner(victim)), Name(victim), unparse_dbref(victim));
	notify_format(Owner(victim),
		      "Restarting: %s(%s), by %s",
		      Name(victim), unparse_dbref(victim), Name(player));
      }
    } else {
      if (victim == player)
	notify(player, T("All of your objects are being restarted."));
      else
	notify_format(player, "Restarting: %s(%s)", Name(victim),
		      unparse_dbref(victim));
    }
    do_halt(player, "", victim);
    do_raw_restart(victim);
  }
}
