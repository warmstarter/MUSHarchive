/**
 * \file rob.c
 *
 * \brief Kill and give.
 *
 * This file is called rob.c for historical reasons, and one day it'll
 * probably get folded into some other file.
 *
 *
 */

#include "config.h"
#include "copyrite.h"
#include <ctype.h>
#include <stdlib.h>
#include "conf.h"
#include "mushdb.h"
#include "attrib.h"
#include "match.h"
#include "externs.h"
#include "flags.h"
#include "log.h"
#include "lock.h"
#include "dbdefs.h"
#include "game.h"
#include "confmagic.h"

/** The kill command - send an object back home.
 * \param player the enactor.
 * \param what name of object to kill.
 * \param cost amount to pay to kill.
 * \param slay if 1, this is the wizard 'slay' command instead.
 */
void
do_kill(dbref player, const char *what, int cost, int slay)
{
  dbref victim;
  char tbuf1[BUFFER_LEN], tbuf2[BUFFER_LEN], *tp;

  if (slay && !Wizard(player)) {
    notify(player, T("You do not have such power."));
    return;
  }
  victim = match_result(player, what, TYPE_PLAYER, MAT_NEAR_THINGS);

  if (player == victim) {
    notify(player, T("No suicide allowed."));
    return;
  }
  if (slay)
    do_log(LT_WIZ, player, victim, "SLAY");

  switch (victim) {
  case NOTHING:
    notify(player, T("I don't see that here."));
    break;
  case AMBIGUOUS:
    notify(player, T("I don't know what you mean!"));
    break;
  default:
    if (Suspect(player))
      flag_broadcast("WIZARD", 0,
		     T("Broadcast: Suspect %s tried to kill %s(#%d)."),
		     Name(player), Name(victim), victim);
    if (!Mobile(victim)) {
      notify(player, T("Sorry, you can only kill players and objects."));
    } else if ((Haven(Location(victim)) &&
		!Wizard(player)) ||
	       (controls(victim, Location(victim)) &&
		!controls(player, Location(victim)))) {
      notify(player, "Sorry.");
    } else if (NoKill(victim) && !Wizard(player) && (Owner(victim) != player)) {
      notify(player, T("That object cannot be killed."));
    } else {
      /* go for it */
      /* set cost */
      /* if this isn't called via slay */
      if (!slay) {
	if (cost < KILL_MIN_COST)
	  cost = KILL_MIN_COST;

	/* see if it works */
	if (!payfor(player, cost)) {
	  notify_format(player, T("You don't have enough %s."), MONIES);
	  break;
	}
      }
      if (((get_random_long(0, KILL_BASE_COST) < cost) || slay) &&
	  !Wizard(victim)) {
	/* you killed him */
	tp = tbuf1;
	safe_format(tbuf1, &tp, T("You killed %s!"), Name(victim));
	*tp = '\0';
	tp = tbuf2;
	safe_format(tbuf2, &tp, "killed %s!", Name(victim));
	*tp = '\0';
	do_halt(victim, "", victim);
	did_it(player, victim, "DEATH", tbuf1, "ODEATH", tbuf2, "ADEATH",
	       NOTHING);

	/* notify victim */
	notify_format(victim, T("%s killed you!"), Name(player));

	/* maybe pay off the bonus */
	/* if we were not called via slay */
	if (!slay) {
	  int payoff = cost * KILL_BONUS / 100;
	  if (payoff + Pennies(Owner(victim)) > Max_Pennies(Owner(victim)))
	    payoff = Max_Pennies(Owner(victim)) - Pennies(Owner(victim));
	  if (payoff > 0) {
	    notify_format(victim, T("Your insurance policy pays %d %s."),
			  payoff, ((payoff == 1) ? MONEY : MONIES));
	    giveto(Owner(victim), payoff);
	  } else {
	    notify(victim, T("Your insurance policy has been revoked."));
	  }
	}
	/* send him home */
	safe_tel(victim, HOME, 0);
	/* if victim is object also dequeue all commands */
      } else {
	/* notify player and victim only */
	notify(player, T("Your murder attempt failed."));
	notify_format(victim, T("%s tried to kill you!"), Name(player));
      }
      break;
    }
  }
}

/** The give command.
 * \param player the enactor/giver.
 * \param recipient name of object to receive.
 * \param amnt name of object to be transferred, or amount of pennies.
 * \param silent if 1, hush the usual messages.
 */
void
do_give(dbref player, char *recipient, char *amnt, int silent)
{
  dbref who;
  int amount;
  char *s;
  char tbuf1[BUFFER_LEN];
  char *bp;
  char *myenv[10];
  int i;

  /* check recipient */
  switch (who =
	  match_result(player, recipient, TYPE_PLAYER,
		       MAT_NEAR_THINGS | MAT_ENGLISH)) {
  case NOTHING:
    notify(player, T("Give to whom?"));
    return;
  case AMBIGUOUS:
    notify(player, T("I don't know who you mean!"));
    return;
  }

  /* Can't give to garbage... */
  if (IsGarbage(who)) {
    notify(player, T("Give to whome?"));
    return;
  }

  /* make sure amount is all digits */
  for (s = amnt; *s && ((isdigit((unsigned char) *s)) || (*s == '-')); s++) ;
  /* must be giving object */
  if (*s) {
    dbref thing;
    switch (thing =
	    match_result(player, amnt, TYPE_THING,
			 MAT_POSSESSION | MAT_ENGLISH)) {
    case NOTHING:
      notify(player, T("You don't have that!"));
      return;
    case AMBIGUOUS:
      notify(player, T("I don't know which you mean!"));
      return;
    default:
      /* if you can give yourself, that's like "enter". since we
       * do no lock check with give, we shouldn't be able to
       * do this.
       */
      if (thing == player) {
	notify(player, T("You can't give yourself away!"));
	return;
      }
      if (!eval_lock(player, thing, Give_Lock)) {
	notify(player, T("You can't give that away."));
	return;
      }
      if (Mobile(thing) && (EnterOk(who) || controls(player, who))) {
	moveto(thing, who);

	/* Notify the giver with their GIVE message */
	myenv[0] = (char *) mush_malloc(BUFFER_LEN, "dbref");
	myenv[1] = (char *) mush_malloc(BUFFER_LEN, "dbref");
	sprintf(myenv[0], "#%d", thing);
	sprintf(myenv[1], "#%d", who);
	for (i = 2; i < 10; i++)
	  myenv[i] = NULL;
	bp = tbuf1;
	safe_format(tbuf1, &bp, T("You gave %s to %s."), Name(thing),
		    Name(who));
	*bp = '\0';
	real_did_it(player, player, "GIVE", tbuf1, "OGIVE", NULL,
		    "AGIVE", NOTHING, myenv);

	/* Notify the object that it's been given */
	notify_format(thing, T("%s gave you to %s."), Name(player), Name(who));

	/* Recipient gets success message on thing and receive on self */
	did_it(who, thing, "SUCCESS", NULL, "OSUCCESS", NULL, "ASUCCESS",
	       NOTHING);
	bp = tbuf1;
	safe_format(tbuf1, &bp, T("%s gave you %s."), Name(player),
		    Name(thing));
	*bp = '\0';
	sprintf(myenv[1], "#%d", player);
	real_did_it(who, who, "RECEIVE", tbuf1, "ORECEIVE", NULL,
		    "ARECEIVE", NOTHING, myenv);

	mush_free(myenv[0], "dbref");
	mush_free(myenv[1], "dbref");
      } else
	notify(player, T("Permission denied."));
    }
    return;
  }
  amount = atoi(amnt);
  /* do amount consistency check */
  if (amount < 0 && !Wizard(player)) {
    notify(player, T("What is this, a holdup?"));
    return;
  } else if (amount == 0) {
    notify_format(player,
		  T("You must specify a positive number of %s."), MONIES);
    return;
  }
  if (Pennies(who) + amount > Max_Pennies(who)) {
    notify(player, T("You are not a bank!"));
    return;
  }
  if (Wizard(player))
    if (amount < 0 && (abs(Pennies(who) + amount) > Max_Pennies(who))) {
      notify(player, T("It's cruel to reduce anyone to such abject poverty."));
      return;
    }
  /* try to do the give */
  if (!payfor(player, amount)) {
    notify_format(player, T("You don't have that many %s to give!"), MONIES);
  } else {
    /* objects work differently */
    if (IsThing(who)) {
      int cost = 0;
      ATTR *a;
      a = atr_get(who, "COST");
      if (a && (amount < (cost = atoi(uncompress(a->value))))) {
	notify(player, T("Feeling poor today?"));
	giveto(player, amount);
	return;
      }
      if (cost < 0)
	return;
      if ((amount - cost) > 0) {
	notify_format(player, T("You get %d in change."), amount - cost);
      } else {
	notify_format(player, T("You paid %d %s."), amount,
		      ((amount == 1) ? MONEY : MONIES));
      }
      giveto(player, amount - cost);
      giveto(who, cost);
      did_it(player, who, "PAYMENT", NULL, "OPAYMENT", NULL, "APAYMENT",
	     NOTHING);
      return;
    } else {
      /* he can do it */
      if (amount > 0) {
	notify_format(player,
		      T("You give %d %s to %s."), amount,
		      ((amount == 1) ? MONEY : MONIES), Name(who));
      } else {
	notify_format(player, T("You took %d %s from %s!"), abs(amount),
		      ((abs(amount) == 1) ? MONEY : MONIES), Name(who));
      }
      if (IsPlayer(who) && !silent) {
	if (amount > 0) {
	  notify_format(who, T("%s gives you %d %s."), Name(player),
			amount, ((amount == 1) ? MONEY : MONIES));
	} else {
	  notify_format(who, T("%s took %d %s from you!"), Name(player),
			abs(amount), ((abs(amount) == 1) ? MONEY : MONIES));
	}
      }
      giveto(who, amount);
      did_it(player, who, "PAYMENT", NULL, "OPAYMENT", NULL, "APAYMENT",
	     NOTHING);
    }
  }
}
