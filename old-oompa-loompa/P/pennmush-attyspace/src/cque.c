/* cque.c */

#include "copyrite.h"
#include "config.h"

#include <ctype.h>
#include <fcntl.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif

#include "conf.h"
#include "mushdb.h"
#include "intrface.h"
#include "match.h"
#include "externs.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "parse.h"
#include "mymalloc.h"
#include "confmagic.h"


extern char ccom[];
extern dbref cplr;
extern time_t mudtime;

char *wenv[10];			/* working environment (wptr/rptr equiv) */
char renv[10][BUFFER_LEN];
char *wnxt[10], *rnxt[10];	/* stuff to be shoved into the queue */

typedef struct bque BQUE;

struct bque {
  BQUE *next;
  dbref player;			/* player who will do command */
  dbref cause;			/* player causing command (for %N) */
  dbref sem;			/* semaphore object to block on */
  int left;			/* seconds left until execution */
  char *env[10];		/* environment, from wild match */
  char *rval[10];		/* environment, from setq() */
  char *comm;			/* command to be executed */
};

static BQUE *qfirst = NULL, *qlast = NULL, *qwait = NULL;
static BQUE *qlfirst = NULL, *qllast = NULL;
static BQUE *qsemfirst = NULL, *qsemlast = NULL;

void parse_que _((dbref player, const char *command, dbref cause));
static int add_to_generic _((dbref player, int am, int key));
static int add_to _((dbref player, int am));
static int add_to_sem _((dbref player, int am));
static int queue_limit _((dbref player));
void free_qentry _((BQUE *point));
static int pay_queue _((dbref player));
void wait_que _((dbref player, int wait, char *command, dbref cause, dbref sem));
void do_second _((void));
int test_top _((void));
int do_top _((int ncom));
int que_next _((void));
int nfy_que _((dbref sem, int key, int count));
void do_notify _((dbref player, dbref cause, int key, char *what, char *count));
void do_wait _((dbref player, dbref cause, char *arg1, char *cmd));
static void show_queue _((dbref player, dbref victim, int q_type, int q_quiet, int q_all, BQUE *q_ptr, int *tot, int *self, int *del));
void do_queue _((dbref player, const char *what, int flag));
void do_halt _((dbref owner, const char *ncom, int victim));
void do_halt1 _((dbref player, const char *arg1, const char *arg2));
void do_allhalt _((dbref player));
void do_allrestart _((dbref player));
static void do_raw_restart _((dbref victim));
void do_restart_com _((dbref player, const char *arg1));

static int
add_to_generic(player, am, key)
    dbref player;
    int am;
    int key;


				/* 0 for queue, 1 for semaphore */
{
  int num = 0;
  ATTR *a;
  const char *atrname;
  char buff[MAX_COMMAND_LEN];

  if (key == 0) {
    atrname = "QUEUE";
    player = Owner(player);
  } else {
    atrname = "SEMAPHORE";
  }

  a = atr_get_noparent(player, atrname);	/* don't get from the parent! */
  if (a)
    num = atoi(uncompress(a->value));
  num += am;

  if (num)
    sprintf(buff, "%d", num);
  else
    *buff = '\0';

  (void) atr_add(player, atrname, buff, GOD, NOTHING);
  return (num);
}

static int
add_to(player, am)
    dbref player;
    int am;
{
  return (add_to_generic(player, am, 0));
}

static int
add_to_sem(player, am)
    dbref player;
    int am;
{
  return (add_to_generic(player, am, 1));
}

static int
queue_limit(player)
    dbref player;
{
  /* returns 1 if player has exceeded his queue limit */

  if (HugeQueue(player)) {
    if (add_to(player, 1) > (QUEUE_QUOTA + db_top))
      return 1;
    else
      return 0;
  } else {
    if (add_to(player, 1) > QUEUE_QUOTA)
      return 1;
    else
      return 0;
  }
  return 0;			/* NOTREACHED */
}

void
free_qentry(point)
    BQUE *point;
{
  int a;

  for (a = 0; a < 10; a++)
    if (point->env[a]) {
      mush_free((Malloc_t) point->env[a], "bqueue_env");
    }
  for (a = 0; a < 10; a++)
    if (point->rval[a]) {
      mush_free((Malloc_t) point->rval[a], "bqueue_rval");
    }
  if (point->comm)
    mush_free((Malloc_t) point->comm, "bqueue_comm");
  mush_free((Malloc_t) point, "BQUE");
}

static int
pay_queue(player)
    dbref player;
{
  if (!payfor(player,
	      QUEUE_COST + ((getrandom(QUEUE_LOSS) == 1) ? 1 : 0))) {
    notify(Owner(player), "Not enough money to queue command.");
    return 0;
  }
  if (queue_limit(QUEUE_PER_OWNER ? Owner(player) : player)) {
    notify(Owner(player),
	   tprintf("Runaway object: %s(#%d). Commands halted.",
		   Name(player), player));
    /* wipe out that object's queue and set it HALT */
    do_halt(Owner(player), "", player);
    Flags(player) |= HALT;
    return 0;
  }
  return 1;
}

void
parse_que(player, command, cause)
    dbref player;
    const char *command;
    dbref cause;
{
  int a;
  BQUE *tmp;

  if (!IsPlayer(player) && (Halted(player)))
    return;
  if (!pay_queue(player))	/* make sure player can afford to do it */
    return;
  tmp = (BQUE *) mush_malloc(sizeof(BQUE), "BQUE");
  tmp->comm = (char *) strdup(command);
#ifdef MEM_CHECK
  add_check("bqueue_comm");
#endif
  tmp->player = player;
  tmp->next = NULL;
  tmp->left = 0;
  tmp->cause = cause;
  for (a = 0; a < 10; a++)
    if (!wnxt[a])
      tmp->env[a] = NULL;
    else {
      tmp->env[a] = (char *) strdup(wnxt[a]);
#ifdef MEM_CHECK
      add_check("bqueue_env");
#endif
    }
  for (a = 0; a < 10; a++)
    if (!rnxt[a] || !rnxt[a][0])
      tmp->rval[a] = NULL;
    else {
      tmp->rval[a] = (char *) strdup(rnxt[a]);
#ifdef MEM_CHECK
      add_check("bqueue_rval");
#endif
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

void
wait_que(player, wait, command, cause, sem)
    dbref player;
    int wait;
    char *command;
    dbref cause;
    dbref sem;
{
  BQUE *tmp, *point, *trail;
  int a;

  if (wait == 0) {
    if (sem != NOTHING)
      add_to_sem(sem, -1);
    parse_que(player, command, cause);
    return;
  }
  if (!pay_queue(player))	/* make sure player can afford to do it */
    return;
  tmp = (BQUE *) mush_malloc(sizeof(BQUE), "BQUE");
  tmp->comm = (char *) strdup(command);
#ifdef MEM_CHECK
  add_check("bqueue_comm");
#endif
  tmp->player = player;
  tmp->cause = cause;

  for (a = 0; a < 10; a++) {
    if (!wnxt[a])
      tmp->env[a] = NULL;
    else {
      tmp->env[a] = (char *) strdup(wnxt[a]);
#ifdef MEM_CHECK
      add_check("bqueue_env");
#endif
    }
    if (!rnxt[a] || !rnxt[a][0])
      tmp->rval[a] = NULL;
    else {
      tmp->rval[a] = (char *) strdup(rnxt[a]);
#ifdef MEM_CHECK
      add_check("bqueue_rval");
#endif
    }
  }

  if (wait >= 0)
    tmp->left = mudtime + wait;
  else
    tmp->left = 0;		/* semaphore wait without a timeout */
  tmp->sem = sem;

  if (sem == NOTHING) {

    /* No semaphore, put on normal wait queue, sorted by time */

    for (point = qwait, trail = NULL;
	 point && (point->left <= tmp->left);
	 point = point->next)
      trail = point;

    tmp->next = point;
    if (trail != NULL)
      trail->next = tmp;
    else
      qwait = tmp;

  } else {

    /* Put it on the semaphore queue */

    tmp->next = NULL;
    if (qsemlast != NULL)
      qsemlast->next = tmp;
    else
      qsemfirst = tmp;
    qsemlast = tmp;
  }
}

void
do_second()
{
  /* call every second to check for wait queue commands */

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
    if (point->left == 0) {
      next = (trail = point)->next;
      continue;			/* skip non-timed waits */
    }
    if (point->left <= mudtime) {
      if (trail != NULL)
	trail->next = next = point->next;
      else
	qsemfirst = next = point->next;
      if (point == qsemlast)
	qsemlast = trail;
      add_to_sem(point->sem, -1);
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
    } else
      next = (trail = point)->next;
  }
}

int
test_top()
{
  return (qfirst ? 1 : 0);
}

int
do_top(ncom)
    int ncom;
{
  /* execute ncom commands off the top of the queue */

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
    if (GoodObject(entry->player) && !IsGarbage(entry->player)) {
      cplr = entry->player;
      giveto(cplr, QUEUE_COST);
      add_to(cplr, -1);
      entry->player = 0;
      if ((Typeof(cplr) == TYPE_PLAYER) || !Halted(cplr)) {
	for (a = 0; a < 10; a++) {
	  wenv[a] = entry->env[a];
	  if (entry->rval[a])
	    strcpy(renv[a], entry->rval[a]);
	  else
	    renv[a][0] = '\0';
	}
	s = entry->comm;
	while (*s) {
	  r = ccom;
	  process_expression(ccom, &r, &s, cplr, entry->cause, entry->cause,
			     PE_NOTHING, PT_SEMI, NULL);
	  *r = '\0';
	  if (*s == ';')
	    s++;
	  strcpy(tbuf, ccom);
	  process_command(cplr, tbuf, entry->cause, 0);
	}
      }
    }
    free_qentry(entry);
  }

  return i;
}

int
que_next()
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

  for (point = qwait; point; point = point->next) {
    curr = point->left - mudtime;
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
    if (curr < min)
      min = curr;
  }

  return (min - 1);
}

int
nfy_que(sem, key, count)
    dbref sem;
    int key;
    int count;

				/* 0 is normal, 1 is all, 2 is drain */

{
  BQUE *point, *trail, *next;
  int num;
  ATTR *a;

  if ((a = atr_get_noparent(sem, "SEMAPHORE")) != NULL)
    num = atoi(uncompress(a->value));
  else
    num = 0;

  if (num > 0) {
    num = 0;
    for (point = qsemfirst, trail = NULL; point; point = next) {
      if (point->sem == sem) {
	num++;
	if (trail)
	  trail->next = next = point->next;
	else
	  qsemfirst = next = point->next;
	if (point == qsemlast)
	  qsemlast = trail;

	/* run or discard the command */
	if (key != 2) {
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
	} else {
	  giveto(point->player, QUEUE_COST);
	  add_to(point->player, -1);
	  free_qentry(point);
	}
      } else {
	next = (trail = point)->next;
      }

      /* if we've notified enough, exit. Note that we don't have to check
       * for the case of @notify/all, since we don't break out of this
       * loop "manually" unless the key is standard @notify.
       */
      if ((key == 0) && (num >= count))
	next = NULL;
    }
  } else {
    num = 0;
  }

  /* update the semaphore waiter's count */
  if (key == 0)
    add_to_sem(sem, -count);
  else
    atr_clr(sem, "SEMAPHORE", GOD);

  return num;
}

void
do_notify(player, cause, key, what, count)
    dbref player;
    dbref cause;
    int key;			/* 0 is normal, 1 is all, 2 is drain */
    char *what;
    char *count;
{
  dbref thing;
  int i;

  /* find it */
  if ((thing = noisy_match_result(player, what, NOTYPE, MAT_EVERYTHING)) == NOTHING)
    return;

  /* must control something or have it link_ok in order to use it as 
   * as a semaphore.
   */
  if (!controls(player, thing) && !LinkOk(thing)) {
    notify(player, "Permission denied.");
    return;
  }
  /* find how many times to notify */
  if (count && *count)
    i = atoi(count);
  else
    i = 1;

  if (i > 0)
    nfy_que(thing, key, i);

  if (key != 2) {
    quiet_notify(player, "Notified.");
  } else {
    quiet_notify(player, "Drained.");
  }
}

void
do_wait(player, cause, arg1, cmd)
    dbref player;
    dbref cause;
    char *arg1;
    char *cmd;
{
  dbref thing;
  char *tcount;
  int waitfor, num;
  ATTR *a;
  char *arg2;
  int j;

  for (j = 0; j < 10; j++) {
    wnxt[j] = wenv[j];
    rnxt[j] = renv[j];
  }

  arg2 = strip_braces(cmd);

  if (is_strict_number(arg1)) {
    /* normal wait */
    wait_que(player, atoi(arg1), arg2, cause, NOTHING);
    mush_free(arg2, "strip_braces.buff");
    return;
  }
  /* semaphore wait with optional timeout */

  /* find the thing to wait on */
  tcount = (char *) index(arg1, '/');
  if (tcount)
    *tcount++ = '\0';
  if ((thing = noisy_match_result(player, arg1, NOTYPE, MAT_EVERYTHING)) == NOTHING) {
    mush_free(arg2, "strip_braces.buff");
    return;
  }
  if (!controls(player, thing) && !LinkOk(thing)) {
    notify(player, "Permission denied.");
    mush_free(arg2, "strip_braces.buff");
    return;
  }
  /* get timeout, default of -1 */
  if (tcount && *tcount)
    waitfor = atol(tcount);
  else
    waitfor = -1;

  add_to_sem(thing, 1);
  a = atr_get_noparent(thing, "SEMAPHORE");
  if (a)
    num = atoi(uncompress(a->value));
  else
    num = 0;
  if (num <= 0) {
    thing = NOTHING;
    waitfor = -1;		/* just in case there was a timeout given */
  }
  wait_que(player, waitfor, arg2, cause, thing);
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
      if (!q_quiet &&
	  (LookQueue(player) || Owns(tmp->player, player))) {
	switch (q_type) {
	case 1:		/* wait queue */
	  notify(player, tprintf("[%ld]%s:%s", tmp->left - mudtime,
				 unparse_object(player, tmp->player),
				 tmp->comm));
	  break;
	case 2:		/* semaphore queue */
	  if (tmp->left != 0) {
	    notify(player,
		   tprintf("[#%d/%ld]%s:%s", tmp->sem,
			   tmp->left - mudtime,
			   unparse_object(player, tmp->player),
			   tmp->comm));
	  } else {
	    notify(player,
		   tprintf("[#%d]%s:%s", tmp->sem,
			   unparse_object(player, tmp->player),
			   tmp->comm));
	  }
	  break;
	default:		/* player or object queue */
	  notify(player, tprintf("%s:%s",
				 unparse_object(player, tmp->player),
				 tmp->comm));
	}
      }
    }
  }
}

void
do_queue(player, what, flag)
    dbref player;
    const char *what;
    int flag;			/* 0 - normal, 1 - all, 2 - summary, 3 - quick */
{
  /* tell player what commands they have pending in the queue (@ps) */

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
    notify(player, "I couldn't find that player.");
    break;
  case AMBIGUOUS:
    notify(player, "I don't know who you mean!");
    break;
  default:

    if (!quick) {
      if (all == 1)
	notify(player, "Queue for : all");
      else
	notify(player,
	       tprintf("Queue for : %s", Name(victim)));
    }
    victim = Owner(victim);

    if (!quick)
      notify(player, "Player Queue:");
    show_queue(player, victim, 0, quick, all, qfirst, &tpq, &pq, &dpq);

    if (!quick)
      notify(player, "Object Queue:");
    show_queue(player, victim, 0, quick, all, qlfirst, &toq, &oq, &doq);

    if (!quick)
      notify(player, "Wait Queue:");
    show_queue(player, victim, 1, quick, all, qwait, &twq, &wq, &dwq);

    if (!quick)
      notify(player, "Semaphore Queue:");
    show_queue(player, victim, 2, quick, all, qsemfirst, &tsq, &sq, &dsq);

    if (!quick)
      notify(player, "------------  Queue Done  ------------");

    notify(player,
	   tprintf(
		    "Totals: Player...%d/%d[%ddel]  Object...%d/%d[%ddel]  Wait...%d/%d  Semaphore...%d/%d",
		    pq, tpq, dpq, oq, toq, doq, wq, twq, sq, tsq));
  }
}

void
do_halt(owner, ncom, victim)
    dbref owner;
    const char *ncom;
    dbref victim;
{
  BQUE *tmp, *trail = NULL, *point, *next;
  int num = 0;
  dbref player;

  if (victim == NOTHING)
    player = owner;
  else
    player = victim;

  quiet_notify(Owner(player), tprintf("Halted: %s(#%d).", Name(player), player));

  for (tmp = qfirst; tmp; tmp = tmp->next)
    if (GoodObject(tmp->player) &&
	((tmp->player == player) || (Owner(tmp->player) == player))) {
      num--;
      giveto(player, QUEUE_COST);
      tmp->player = NOTHING;
    }
  for (tmp = qlfirst; tmp; tmp = tmp->next)
    if (GoodObject(tmp->player) &&
	((tmp->player == player) || (Owner(tmp->player) == player))) {
      num--;
      giveto(player, QUEUE_COST);
      tmp->player = NOTHING;
    }
  /* remove wait q stuff */
  for (point = qwait; point; point = next) {
    if (((point->player == player) || (Owner(point->player) == player))) {
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
    if (((point->player == player) || (Owner(point->player) == player))) {
      num--;
      giveto(player, QUEUE_COST);
      if (trail)
	trail->next = next = point->next;
      else
	qsemfirst = next = point->next;
      if (point == qsemlast)
	qsemlast = trail;
      add_to_sem(point->sem, -1);
      free_qentry(point);
    } else
      next = (trail = point)->next;
  }

  if (Owner(player) == player)
    (void) atr_clr(player, "QUEUE", GOD);
  else
    add_to(player, num);

  if (ncom && *ncom) {
    int j;

    for (j = 0; j < 10; j++) {
      wnxt[j] = wenv[j];
      rnxt[j] = renv[j];
    }
    parse_que(player, ncom, player);
  }
}

void
do_halt1(player, arg1, arg2)
    dbref player;
    const char *arg1;
    const char *arg2;
{
  dbref victim;
  if (*arg1 == '\0')
    do_halt(player, "", player);
  else {
    if ((victim = noisy_match_result(player, arg1, NOTYPE, MAT_OBJECTS)) == NOTHING)
      return;
    if (!Owns(player, victim) && !HaltAny(player)) {
      notify(player, "Permission denied.");
      return;
    }
    if (arg2 && *arg2 && !controls(player, victim)) {
      notify(player, "You may not use @halt obj=command on this object.");
      return;
    }
    /* If victim's a player, we halt all of their objects */
    /* If not, we halt victim and set the HALT flag if no new command */
    /* was given */
    do_halt(player, arg2, victim);
    if (IsPlayer(victim)) {
      if (victim == player) {
	notify(player, "All of your objects have been halted.");
      } else {
	notify(player, tprintf("All objects for %s have been halted.",
			       Name(victim)));
	notify(victim, tprintf("All of your objects have been halted by %s.",
			       Name(player)));
      }
    } else {
      if (Owner(victim) != player) {
	notify(player, tprintf("Halted: %s's %s(#%d)",
			       Name(Owner(victim)), Name(victim),
			       victim));
	notify(Owner(victim),
	       tprintf("Halted: %s(#%d), by %s", Name(victim),
		       victim, Name(player)));
      }
      if (*arg2 == '\0')
	Flags(victim) |= HALT;
    }
  }
}

void
do_allhalt(player)
    dbref player;
{
  dbref victim;
  if (!HaltAny(player)) {
    notify(player, "You do not have the power to bring the world to a halt.");
    return;
  }
  for (victim = 0; victim < db_top; victim++) {
    if (IsPlayer(victim)) {
      notify(victim, tprintf("Your objects have been globally halted by %s",
			     Name(player)));
      do_halt(victim, "", victim);
    }
  }
}

void
do_allrestart(player)
    dbref player;
{
  dbref thing;
  ATTR *s;
  char *r;
  if (!HaltAny(player)) {
    notify(player, "You do not have the power to restart the world.");
    return;
  }
  do_allhalt(player);
  for (thing = 0; thing < db_top; thing++) {
    if (!IsGarbage(thing) &&
	(Startup(thing)) && !(Halted(thing))) {
      s = atr_get_noparent(thing, "STARTUP");
      if (!s)
	continue;
      r = safe_uncompress(s->value);
      parse_que(thing, r, thing);
      free((Malloc_t) r);
    }
    if (IsPlayer(thing)) {
      notify(thing, tprintf("Your objects are being globally restarted by %s",
			    Name(player)));
    }
  }
}

static void
do_raw_restart(victim)
    dbref victim;
{
  ATTR *s;
  char *r;
  dbref thing;
  if (IsPlayer(victim)) {
    for (thing = 0; thing < db_top; thing++) {
      if ((Owner(thing) == victim) && !IsGarbage(thing) &&
	  (Startup(thing)) && !(Halted(thing))) {
	s = atr_get_noparent(thing, "STARTUP");
	if (!s)
	  continue;
	r = safe_uncompress(s->value);
	parse_que(thing, r, thing);
	free((Malloc_t) r);
      }
    }
  } else {
    /* A single object */
    if (!IsGarbage(victim) &&
	(Startup(victim)) && !(Halted(victim))) {
      s = atr_get_noparent(victim, "STARTUP");
      if (!s)
	return;
      r = safe_uncompress(s->value);
      parse_que(victim, r, victim);
      free((Malloc_t) r);
    }
  }
}

void
do_restart_com(player, arg1)
    dbref player;
    const char *arg1;
{
  dbref victim;
  if (*arg1 == '\0') {
    do_halt(player, "", player);
    do_raw_restart(player);
  } else {
    if ((victim = noisy_match_result(player, arg1, NOTYPE, MAT_OBJECTS)) == NOTHING)
      return;
    if (!Owns(player, victim) && !HaltAny(player)) {
      notify(player, "Permission denied.");
      return;
    }
    if (Owner(victim) != player) {
      if (IsPlayer(victim)) {
	notify(player, tprintf("All objects for %s are being restarted.",
			       Name(victim)));
	notify(victim, tprintf("All of your objects are being restarted by %s.",
			       Name(player)));
      } else {
	notify(player, tprintf("Restarting: %s's %s(#%d)",
			       Name(Owner(victim)), Name(victim),
			       victim));
	notify(Owner(victim),
	       tprintf("Restarting: %s(#%d), by %s", Name(victim),
		       victim, Name(player)));
      }
    } else {
      if (victim == player)
	notify(player, "All of your objects are being restarted.");
      else
	notify(player, tprintf("Restarting: %s(#%d)", Name(victim), victim));
    }
    do_halt(player, "", victim);
    do_raw_restart(victim);
  }
}
