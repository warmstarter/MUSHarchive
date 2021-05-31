/* rob.c */

#include "config.h"
#include "copyrite.h"
#include <ctype.h>
#ifdef I_STDLIB
#include <stdlib.h>
#endif

/* rob and kill */

#include "conf.h"
#include "mushdb.h"
#include "intrface.h"
#include "match.h"
#include "externs.h"
#include "confmagic.h"


void do_kill _((dbref player, const char *what, int cost, int slay));
void do_give _((dbref player, char *recipient, char *amnt));
void do_rob _((dbref player, const char *what));

void
do_rob(player, what)
    dbref player;
    const char *what;
{
  dbref thing;
  long match_flags = MAT_ME | MAT_NEIGHBOR;

  if (Wizard(player))
    match_flags |= MAT_ABSOLUTE | MAT_PLAYER;
  thing = match_result(player, what, TYPE_PLAYER, match_flags);

  switch (thing) {
  case NOTHING:
    notify(player, "Rob whom?");
    break;
  case AMBIGUOUS:
    notify(player, "I don't know who you mean!");
    break;
  default:
    if (!IsPlayer(thing)) {
      notify(player, "Sorry, you can only rob other players.");
    } else if (Pennies(thing) < 1) {
      notify(player, tprintf("%s has no %s.", Name(thing), MONIES));
      notify(thing,
	     tprintf("%s tried to rob you, but you have no %s to take.",
		     Name(player), MONIES));
    } else if (could_doit(player, thing)) {
      /* steal a penny */
      giveto(player, 1);
      giveto(thing, -1);
      notify(player, tprintf("You stole a %s.", MONEY));
      notify(thing, tprintf("%s stole one of your %s!", Name(player),
			    MONIES));
      did_it(player, thing, "SUCCESS", NULL, "OSUCCESS", NULL, "ASUCCESS",
	     NOTHING);
    } else
      did_it(player, thing, "FAILURE", "Your conscience tells you not to.",
	     "OFAILURE", NULL, "AFAILURE", NOTHING);
    break;
  }
}

void
do_kill(player, what, cost, slay)
    dbref player;
    const char *what;
    int cost;
    int slay;
{
  dbref victim;
  char tbuf1[BUFFER_LEN], tbuf2[BUFFER_LEN];

  if (slay && !Wizard(player)) {
    notify(player, "You do not have such power.");
    return;
  }
  victim = match_result(player, what, TYPE_PLAYER, MAT_NEAR_THINGS);

  if (player == victim) {
    notify(player, "No suicide allowed.");
    return;
  }
  if (slay)
    do_log(LT_WIZ, player, victim, "SLAY");

  switch (victim) {
  case NOTHING:
    notify(player, "I don't see that here.");
    break;
  case AMBIGUOUS:
    notify(player, "I don't know what you mean!");
    break;
  default:
    if (Suspect(player))
      flag_broadcast(WIZARD, 0, "Broadcast: Suspect %s tried to kill %s(#%d).",
		     Name(player), Name(victim), victim);
    if (!Mobile(victim)) {
      notify(player, "Sorry, you can only kill players and objects.");
    } else if ((Haven(Location(victim)) &&
		!Wizard(player)) ||
	       (controls(victim, Location(victim)) &&
		!controls(player, Location(victim)))) {
      notify(player, "Sorry.");
    } else if (NoKill(victim) &&
	       !Wizard(player) && (Owner(victim) != player)) {
      notify(player, "That object cannot be killed.");
    } else {
      /* go for it */
      /* set cost */
      /* if this isn't called via slay */
      if (!slay) {
	if (cost < KILL_MIN_COST)
	  cost = KILL_MIN_COST;

	/* see if it works */
	if (!payfor(player, cost)) {
	  notify(player, tprintf("You don't have enough %s.", MONIES));
	  break;
	}
      }
      if (((getrandom(KILL_BASE_COST) < cost) || slay) &&
	  !Wizard(victim)) {
	/* you killed him */
	sprintf(tbuf1, "You killed %s!", Name(victim));
	sprintf(tbuf2, "killed %s!", Name(victim));
	do_halt(victim, "", victim);
	did_it(player, victim, "DEATH", tbuf1, "ODEATH", tbuf2, "ADEATH", NOTHING);

	/* notify victim */
	notify(victim, tprintf("%s killed you!", Name(player)));

	/* maybe pay off the bonus */
	/* if we were not called via slay */
	if (!slay) {
	  int payoff = cost * KILL_BONUS / 100;
	  if (payoff + Pennies(Owner(victim)) > MAX_PENNIES)
	    payoff = MAX_PENNIES - Pennies(Owner(victim));
	  if (payoff > 0) {
	    notify(victim, tprintf("Your insurance policy pays %d %s.",
			      payoff, ((payoff == 1) ? MONEY : MONIES)));
	    giveto(Owner(victim), payoff);
	  } else {
	    notify(victim, "Your insurance policy has been revoked.");
	  }
	}
	/* send him home */
	safe_tel(victim, HOME);
	/* if victim is object also dequeue all commands */
      } else {
	/* notify player and victim only */
	notify(player, "Your murder attempt failed.");
	notify(victim, tprintf("%s tried to kill you!", Name(player)));
      }
      break;
    }
  }
}

void
do_give(player, recipient, amnt)
    dbref player;
    char *recipient;
    char *amnt;
{
  dbref who;
  int amount;
  char *s;
  char tbuf1[BUFFER_LEN];

  /* check recipient */
  switch (who = match_result(player, recipient, TYPE_PLAYER, MAT_NEAR_THINGS)) {
  case NOTHING:
    notify(player, "Give to whom?");
    return;
  case AMBIGUOUS:
    notify(player, "I don't know who you mean!");
    return;
  }

  /* make sure amount is all digits */
  for (s = amnt; *s && ((isdigit(*s)) || (*s == '-')); s++) ;
  /* must be giving object */
  if (*s) {
    dbref thing;
    switch (thing = match_result(player, amnt, TYPE_THING, MAT_POSSESSION)) {
    case NOTHING:
      notify(player, "You don't have that!");
      return;
    case AMBIGUOUS:
      notify(player, "I don't know which you mean!");
      return;
    default:
      /* if you can give yourself, that's like "enter". since we
       * do no lock check with give, we shouldn't be able to
       * do this.
       */
      if (thing == player) {
	notify(player, "You can't give yourself away!");
	return;
      }
#ifdef GIVE_LOCK
      if (!eval_lock(player, thing, Give_Lock)) {
	notify(player, "You can't give that away.");
	return;
      }
#endif
      if (Mobile(thing) &&
	  (EnterOk(who) || controls(player, who))) {
	moveto(thing, who);
	notify(who,
	       tprintf("%s gave you %s.", Name(player), Name(thing)));
	notify(player, "Given.");
	notify(thing, tprintf("%s gave you to %s.", Name(player),
			      Name(who)));
	did_it(who, thing, "SUCCESS", NULL, "OSUCCESS", NULL, "ASUCCESS",
	       NOTHING);
      } else
	notify(player, "Permission denied.");
    }
    return;
  }
  amount = atoi(amnt);
  /* do amount consistency check */
  if (amount < 0 && !Wizard(player)) {
    notify(player, "Try using the \"rob\" command.");
    return;
  } else if (amount == 0) {
    notify(player, tprintf("You must specify a positive number of %s.", MONIES));
    return;
  }
  if (Pennies(who) + amount > MAX_PENNIES) {
    notify(player, "You are not a bank!");
    return;
  }
  if (Wizard(player))
    if (amount < 0 && (abs(Pennies(who) + amount) > MAX_PENNIES)) {
      notify(player,
	 "Why not just have that person wake up on a lilypad tomorrow?");
      notify(player, "It's cruel to reduce anyone to such abject poverty.");
      return;
    }
  /* try to do the give */
  if (!payfor(player, amount)) {
    notify(player, tprintf("You don't have that many %s to give!", MONIES));
  } else {
    /* objects work differently */
    if (IsThing(who)) {
      int cost = 0;
      ATTR *a;
      a = atr_get(who, "COST");
      if (a && (amount < (cost = atoi(uncompress(a->value))))) {
	notify(player, "Feeling poor today?");
	giveto(player, amount);
	return;
      }
      if (cost < 0)
	return;
      if ((amount - cost) > 0) {
	sprintf(tbuf1, "You get %d in change.", amount - cost);
      } else {
	sprintf(tbuf1, "You paid %d %s.", amount,
		((amount == 1) ? MONEY : MONIES));
      }
      notify(player, tbuf1);
      giveto(player, amount - cost);
      giveto(who, cost);
      did_it(player, who, "PAYMENT", NULL, "OPAYMENT", NULL, "APAYMENT",
	     NOTHING);
      return;
    } else {
      /* he can do it */
      if (amount > 0) {
	notify(player,
	       tprintf("You give %d %s to %s.", amount,
		       ((amount == 1) ? MONEY : MONIES), Name(who)));
      } else {
	notify(player, tprintf("You took %d %s from %s!", abs(amount),
			       ((abs(amount) == 1) ? MONEY : MONIES),
			       Name(who)));
      }
      if (IsPlayer(who)) {
	if (amount > 0) {
	  notify(who, tprintf("%s gives you %d %s.", Name(player),
			      amount, ((amount == 1) ? MONEY : MONIES)));
	} else {
	  notify(who, tprintf("%s took %d %s from you!", Name(player),
			      abs(amount),
			      ((abs(amount) == 1) ? MONEY : MONIES)));
	}
      }
      giveto(who, amount);
      did_it(player, who, "PAYMENT", NULL, "OPAYMENT", NULL, "APAYMENT",
	     NOTHING);
    }
  }
}
