/* speech.c */

#include "copyrite.h"
#include "config.h"

/* Commands which involve speaking */
#include <ctype.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif

#include "conf.h"
#include "externs.h"
#include "mushdb.h"
#include "intrface.h"
#include "match.h"
#include "attrib.h"
#include "parse.h"
#include "confmagic.h"

static void oemit_notify_except _((dbref loc, dbref exc1, dbref exc2, const char *msg));
const char *reconstruct_message _((char *arg1, char *arg2));
int okay_pemit _((dbref player, dbref target));
static dbref speech_loc _((dbref thing));
void do_say _((dbref player, const char *tbuf1));
void do_oemit _((dbref player, const char *arg1, const char *arg2));
void do_whisper _((dbref player, const char *arg1, const char *arg2, int noisy));
void do_whisper_list _((dbref player, const char *arg1, const char *arg2, int noisy));
void do_pemit_list _((dbref player, char *list, const char *message));
void do_pemit _((dbref player, const char *arg1, const char *arg2, int silent));
void do_pose _((dbref player, const char *tbuf1, int space));
void do_wall _((dbref player, const char *message, dbref privs, int key));
void do_page _((dbref player, const char *arg1, const char *arg2));
void do_multipage _((dbref player, const char *arg1, const char *arg2));
int filter_found _((dbref thing, const char *msg, int flag));
void propagate_sound _((dbref thing, const char *msg));
void do_think _((dbref player, const char *message));
void do_emit _((dbref player, const char *tbuf1));
void do_remit _((dbref player, const char *arg1, const char *arg2));
void do_lemit _((dbref player, const char *tbuf1));
void do_zemit _((dbref player, const char *arg1, const char *arg2));
const char *spname _((dbref thing));
void ns_esnotify _((char *dest, dbref speaker, dbref (*func) (), void *fdata));
static void do_audible_stuff _((dbref loc, dbref exc1, dbref exc2, const char *msg));
dbref na_zemit _((dbref current, void *data));

const char *
spname(thing)
    dbref thing;
{
  if (!FULL_INVIS)
    return Name(thing);

  /* if FULL_INVIS is defined, dark objects and dark wizards will be
   * Something and Someone, respectively.
   */

  if (!DarkLegal(thing))
    return (Name(thing));
  else {
    if (!IsPlayer(thing))
      return ("Something");
    else
      return ("Someone");
  }
}

#ifdef TREKMUSH
extern int hidden _((dbref player)); /* from confmagic.h */
#endif /* TREKMUSH */

int
okay_pemit(player, target)
    dbref player;
    dbref target;
{
  if (Pemit_All(player))
    return 1;
  if ((player != target) &&
      (IsPlayer(target) &&
       (Haven(target) ||
	!eval_lock(player, target, Page_Lock)
       )))
    return 0;
  else
    return 1;
}

static dbref
speech_loc(thing)
    dbref thing;
{
  /* This is the place where speech, poses, and @emits by thing should be
   * heard. For things and players, it's the loc; For rooms, it's the room
   * itself; for exits, it's the source. */
  if (!GoodObject(thing))
    return NOTHING;
  switch (Typeof(thing)) {
  case TYPE_ROOM:
    return thing;
  case TYPE_EXIT:
    return Home(thing);
  default:
    return Location(thing);
  }
}


void
do_say(player, tbuf1)
    dbref player;
    const char *tbuf1;
{
  dbref loc;
  ATTR *a;
  char speak[BUFFER_LEN];
  char *sp;
  char *lang;
  char sep = '|';
  dbref thing;
  int flag;

  loc = speech_loc(player);
  if (!GoodObject(loc))
    return;

#ifdef SPEECH_LOCK
  if (!Hasprivs(player) && !eval_lock(player, loc, Speech_Lock)) {
    notify(player, "You may not speak here!");
    return;
  }
#endif

  /* notify everybody */
#ifdef TREKMUSH
  if (Typeof(player) == TYPE_PLAYER && !Ic(player)) {
    notify(player, tprintf("You <OOC> say, \"%s\"", tbuf1));
    notify_except(db[loc].contents, player, tprintf("%s <OOC> says, \"%s\"", spname(player), tbuf1));
    return;
  }
  a = atr_get_noparent(player, "SPEAKING");
  if (!a) {
    notify(player, tprintf("You say, \"%s\"", tbuf1));
    notify_except(db[loc].contents, player, tprintf("%s says, \"%s\"", spname(player), tbuf1));
    return;
  }
  sp = speak;
  safe_str(uncompress(a->value), speak, &sp);
  *sp = '\0';
  notify(player, tprintf("You say \"%s\" in %s.", tbuf1, speak));
  DOLIST(thing, db[loc].contents) {
    if (thing != player) {
      flag = 0;
      if (Typeof(thing) == TYPE_PLAYER) {
        if (!Ic(thing)) {
          flag = 1;
        } else
          a = atr_get_noparent(thing, "LANGUAGES");
      } else
        a = atr_get_noparent(Owner(thing), "LANGUAGES");
      if (a && !flag) {
        lang = trim_space_sep(uncompress(a->value), sep);
        while (lang) {
          if (!strcasecmp(speak, split_token(&lang, sep)))
            flag = 1;
        }
      }
      if (flag) {
        notify(thing, tprintf("%s says \"%s\" in %s.", spname(player), tbuf1, speak));
      } else
        notify(thing, tprintf("%s says something in %s.", spname(player), speak));
    }
  }
#else /* TREKMUSH */
  notify(player, tprintf("You say, \"%s\"", tbuf1));
  notify_except(Contents(loc), player,
		tprintf("%s says, \"%s\"", spname(player), tbuf1));
#endif /* TREKMUSH */
}

void
do_oemit(player, arg1, arg2)
    dbref player;
    const char *arg1;
    const char *arg2;
{
  dbref who;
  dbref loc;
  char *temp;

  if ((temp = (char *) index(arg1, '/')) == NULL) {
    who = match_result(player, arg1, NOTYPE, MAT_OBJECTS);
    if (!GoodObject(who)) {
      notify(player, "I can't find that object.");
      return;
    }
    loc = Location(who);
    if (!GoodObject(loc)) {
      notify(player, "You can only @oemit to neighboring objects.");
      return;
    }
  } else {
    *temp++ = '\0';

    /* first find the room */
    loc = noisy_match_result(player, arg1, NOTYPE, MAT_EVERYTHING);
    if (!GoodObject(loc)) {
      notify(player, "I can't find that room.");
      return;
    }
    /* then find the person to omit within that room */
    who = match_result(loc, temp, NOTYPE, MAT_POSSESSION | MAT_ABSOLUTE);
  }

#ifdef SPEECH_LOCK
  if (!Hasprivs(player) && !eval_lock(player, loc, Speech_Lock)) {
    notify(player, "You may not speak there!");
    return;
  }
#endif

  if (!GoodObject(who))
    oemit_notify_except(loc, NOTHING, loc, arg2);
  else
    oemit_notify_except(loc, who, loc, arg2);
}

void
do_whisper(player, arg1, arg2, noisy)
    dbref player;
    const char *arg1;
    const char *arg2;
    int noisy;



				/* 0 for silent, 1 for noisy */
{
  dbref who;
  int key;
  const char *gap;
  char tbuf1[BUFFER_LEN];

  switch (who = match_result(player, arg1, TYPE_PLAYER, MAT_NEAR_THINGS | MAT_CONTAINER)) {
  case NOTHING:
    notify(player, "Whisper to whom?");
    break;
  case AMBIGUOUS:
    notify(player, "I don't know who you mean!");
    break;
  default:
    gap = " ";
    switch (*arg2) {
    case SEMI_POSE_TOKEN:
      gap = "";
    case POSE_TOKEN:
      key = 1;
      arg2 = arg2 + 1;
      break;
    default:
      key = 2;
      break;
    }
    switch (key) {
    case 1:
      notify(player, tprintf("%s senses, \"%s%s%s\"", Name(who),
			     Name(player), gap, arg2));
      notify(who, tprintf("You sense: %s%s%s", Name(player), gap, arg2));
      break;
    case 2:
      notify(player,
	     tprintf("You whisper, \"%s\" to %s.", arg2, Name(who)));
      notify(who,
	     tprintf("%s whispers, \"%s\"", Name(player), arg2));
      break;
    }
    if (noisy && GoodObject(Location(player)) &&
	(getrandom(100) < WHISPER_LOUDNESS) &&
	(Location(player) == Location(who))) {
      sprintf(tbuf1, "%s whispers to %s.", Name(player), Name(who));
      notify_except2(Contents(Location(player)), player, who, tbuf1);
    }
    break;
  }
}

void
do_whisper_list(player, arg1, arg2, noisy)
    dbref player;
    const char *arg1;
    const char *arg2;
    int noisy;
{
  dbref who;
  int key;
  const char *gap;
  char *tbuf, *bp;
  char *tbuf2, *bp2;
  char *overbuf;
  dbref good[100];
  int gcount, bcount;
  char *bad, *bptr;
  char *head;
  char *tail;
  char spot;
  int overheard;

  if (!arg1 || !*arg1) {
    notify(player, "Whisper to whom?");
    return;
  }
  if (!arg2 || !*arg2) {
    notify(player, "Whisper what?");
    return;
  }
  tbuf = (char *) mush_malloc(BUFFER_LEN, "string");
  tbuf2 = (char *) mush_malloc(BUFFER_LEN, "string");
  overbuf = (char *) mush_malloc(BUFFER_LEN, "string");
  bad = (char *) mush_malloc(BUFFER_LEN, "string");
  if (!tbuf || !tbuf2 || !overbuf || !bad)
    panic("Unable to allocate memory in do_whisper_list");

  overheard = 0;
  head = (char *) arg1;
  /* Figure out what kind of message */
  gap = " ";
  switch (*arg2) {
  case SEMI_POSE_TOKEN:
    gap = "";
  case POSE_TOKEN:
    key = 1;
    break;
  default:
    key = 2;
    break;
  }

  /* Make up a list of good and bad names */
  gcount = 0;
  bcount = 0;
  bptr = bad;
  safe_str("Unable to whisper to:", bad, &bptr);
  while (head && *head) {
    while (*head == ' ')
      head++;
    tail = head;
    while (*tail && (*tail != ' ')) {
      if (*tail == '"') {
	head++;
	tail++;
	while (*tail && (*tail != '"'))
	  tail++;
      }
      if (*tail)
	tail++;
    }
    tail--;
    if (*tail != '"')
      tail++;
    spot = *tail;
    *tail = 0;
    who = match_result(player, head, TYPE_PLAYER, MAT_NEAR_THINGS | MAT_CONTAINER);
    if (!GoodObject(who)) {
      safe_chr(' ', bad, &bptr);
      safe_str(head, bad, &bptr);
      bcount++;
    } else {
      /* A good whisper */
      good[gcount++] = who;
      if (gcount >= 100) {
	notify(player, "Too many people to whisper to.");
	break;
      }
    }

    *tail = spot;
    head = tail;
    if (*head == '"')
      head++;
  }

  *bptr = '\0';
  /* Set up list of good names */
  bp = tbuf;
  for (who = 0; who < gcount; who++) {
    safe_str(Name(good[who]), tbuf, &bp);
    switch (gcount) {
    case 1:
      break;
    case 2:
      if (who == gcount - 2)
	safe_str(" and ", tbuf, &bp);
      break;
    default:
      if (who < gcount - 1)
	safe_str(", ", tbuf, &bp);
      if (who == gcount - 2)
	safe_str("and ", tbuf, &bp);
      break;
    }
  }
  *bp = '\0';

  bp2 = tbuf2;
  switch (key) {
  case 1:
    safe_str(tbuf, tbuf2, &bp2);
    safe_str(" senses, \"", tbuf2, &bp2);
    safe_str(Name(player), tbuf2, &bp2);
    safe_str(gap, tbuf2, &bp2);
    safe_str(arg2 + 1, tbuf2, &bp2);
    safe_chr('"', tbuf2, &bp2);
    break;
  case 2:
    safe_str("You whisper, \"", tbuf2, &bp2);
    safe_str(arg2, tbuf2, &bp2);
    safe_str("\" to ", tbuf2, &bp2);
    safe_str(tbuf, tbuf2, &bp2);
    break;
  }
  *bp2 = '\0';
  if (bcount)
    notify(player, bad);
  if (!gcount) {
    mush_free((Malloc_t) tbuf, "string");
    mush_free((Malloc_t) tbuf2, "string");
    mush_free((Malloc_t) bad, "string");
    mush_free((Malloc_t) overbuf, "string");
    return;
  }
  notify(player, tbuf2);

  /* Tell each recipient */
  switch (key) {
  case 1:
    for (who = 0; who < gcount; who++) {
      bp2 = tbuf2;
      safe_str("You sense: ", tbuf2, &bp2);
      safe_str(Name(player), tbuf2, &bp2);
      safe_str(gap, tbuf2, &bp2);
      safe_str(arg2 + 1, tbuf2, &bp2);
      *bp2 = '\0';
      notify(good[who], tbuf2);
      if (noisy && !overheard && GoodObject(Location(player)) &&
	  (getrandom(100) < WHISPER_LOUDNESS) &&
	  (Location(player) == Location(who))) {
	bp2 = overbuf;
	safe_str(Name(player), overbuf, &bp2);
	safe_str(" whispers to ", overbuf, &bp2);
	safe_str(Name(good[who]), overbuf, &bp2);
	safe_chr('.', overbuf, &bp2);
	*bp2 = '\0';
	overheard = 1;
      }
    }
    break;
  case 2:
    for (who = 0; who < gcount; who++) {
      bp2 = tbuf2;
      safe_str(Name(player), tbuf2, &bp2);
      safe_str(" whispers", tbuf2, &bp2);
      if (gcount > 1) {
	safe_str(" to ", tbuf2, &bp2);
	safe_str(tbuf, tbuf2, &bp2);
      }
      safe_str(": ", tbuf2, &bp2);
      safe_str(arg2, tbuf2, &bp2);
      *bp2 = '\0';
      notify(good[who], tbuf2);
      if (noisy && !overheard && GoodObject(Location(player)) &&
	  (getrandom(100) < WHISPER_LOUDNESS) &&
	  (Location(player) == Location(who))) {
	overheard = 1;
	bp2 = overbuf;
	safe_str(Name(player), tbuf2, &bp2);
	safe_str(" whispers", tbuf2, &bp2);
	if (gcount > 1) {
	  safe_str(" to ", tbuf2, &bp2);
	  safe_str(tbuf, tbuf2, &bp2);
	}
	safe_chr('.', overbuf, &bp2);
	*bp2 = '\0';
      }
    }
    break;
  }

  if (overheard) {
    dbref first = Contents(Location(player));
    if (!GoodObject(first))
      return;
    DOLIST(first, first) {
      overheard = 1;
      for (who = 0; who < gcount; who++) {
	if (first == good[who]) {
	  overheard = 0;
	  break;
	}
      }
      if (overheard)
	notify_noecho(first, overbuf);
    }
  }
  mush_free((Malloc_t) tbuf, "string");
  mush_free((Malloc_t) tbuf2, "string");
  mush_free((Malloc_t) bad, "string");
  mush_free((Malloc_t) overbuf, "string");
}

void
do_pemit_list(player, list, message)
    dbref player;
    char *list;
    const char *message;
{
  /* Send a message to a list of dbrefs. To avoid repeated generation
   * of the NOSPOOF string, we set it up the first time we encounter
   * something Nospoof, and then check for it thereafter.
   * The list is destructively modified.
   */

  char tbuf[BUFFER_LEN], *bp, *p;
  char listbuff[BUFFER_LEN], *l;
  dbref who;
  char *msg;

  /* If no list or no message, further processing is pointless. */
  if (!message || !*message || !list || !*list)
    return;

  *tbuf = '\0';

  strncpy(listbuff, list, BUFFER_LEN - 1);
  listbuff[BUFFER_LEN - 1] = '\0';
  l = trim_space_sep(listbuff, ' ');

  while ((p = split_token(&l, ' '))) {
    who = noisy_match_result(player, p, NOTYPE, MAT_ABSOLUTE);
    if (GoodObject(who) && okay_pemit(player, who)) {
      msg = replace_string("##", tprintf("#%d", who), message);
      if (Nospoof(who)) {
	if ((*tbuf == '\0') || strstr(message, "##")) {
	  bp = tbuf;
	  if (PARANOID_NOSPOOF)
	    safe_str(tprintf("[%s(#%d)->] ",
			     Name(player), player), tbuf, &bp);
	  else
	    safe_str(tprintf("[%s->] ", Name(player)), tbuf, &bp);
	  safe_str(msg, tbuf, &bp);
	  *bp = '\0';
	}
	notify(who, tbuf);
      } else {
	notify(who, msg);
      }
      mush_free((Malloc_t) msg, "replace_string.buff");
    }
  }
}

void
do_pemit(player, arg1, arg2, silent)
    dbref player;
    const char *arg1;
    const char *arg2;
    int silent;
{
  dbref who;

  switch (who = match_result(player, arg1, NOTYPE, MAT_OBJECTS | MAT_HERE | MAT_CONTAINER)) {
  case NOTHING:
    notify(player, "I don't see that player here.");
    break;
  case AMBIGUOUS:
    notify(player, "I don't know who you mean!");
    break;
  default:
    if (!Mobile(who)) {
      notify(player, "Only players and things can hear @pemits.");
      break;
    }
    if (!okay_pemit(player, who)) {
      notify(player, tprintf("I'm sorry, but %s wishes to be left alone now.",
			     Name(who)));
      return;
    }
    if (!silent)
      notify(player,
	     tprintf("You pemit \"%s\" to %s.", arg2, Name(who)));
    if (Nospoof(who)) {
      if (PARANOID_NOSPOOF)
	notify(who, tprintf("[%s(#%d)->%s] %s", Name(player), player,
			    Name(who), arg2));
      else
	notify(who,
	       tprintf("[%s->%s] %s", Name(player), Name(who), arg2));
    } else {
      notify(who,
	     tprintf("%s", arg2));
    }
    break;
  }
}

void
do_pose(player, tbuf1, space)
    dbref player;
    const char *tbuf1;
    int space;
{
  dbref loc;

  loc = speech_loc(player);
  if (!GoodObject(loc))
    return;

#ifdef SPEECH_LOCK
  if (!Hasprivs(player) && !eval_lock(player, loc, Speech_Lock)) {
    notify(player, "You may not speak here!");
    return;
  }
#endif

  /* notify everybody */
#ifdef TREKMUSH
  if (!Ic(player) && Typeof(player) == TYPE_PLAYER) {
    if (!space) {
      notify_except(db[loc].contents, NOTHING,
		  tprintf("%s <OOC> %s", spname(player), tbuf1));
    } else
      notify_except(db[loc].contents, NOTHING,
		  tprintf("%s<OOC>%s", spname(player), tbuf1));
  } else
#endif /* TREKMUSH */
  if (!space)
    notify_except(Contents(loc), NOTHING,
		  tprintf("%s %s", spname(player), tbuf1));
  else
    notify_except(Contents(loc), NOTHING,
		  tprintf("%s%s", spname(player), tbuf1));
}

void
do_wall(player, message, privs, key)
    dbref player;
    const char *message;
    int privs;
    int key;
{
  /* privs is 0 for wizard wizwall, 1 for royalty-wizard wizwall,
   * 2 is for general wall
   */
  char tbuf[BUFFER_LEN];
  char *bp;
  const char *gap, *prefix;
  int mask;

  /* Only @wall is available to those with the announce power.
   * Only @rwall is available to royalty.
   */
  if (!(Wizard(player) ||
	((privs == 2) && Can_Announce(player)) ||
	((privs == 1) && Royalty(player)))) {
    notify(player,
	   "Posing as a wizard could be hazardous to your health.");
    return;
  }
  /* put together the message and figure out what type it is */
  gap = " ";
  switch (*message) {
  case SAY_TOKEN:
    key = 1;
    message = message + 1;
    break;
  case SEMI_POSE_TOKEN:
    gap = "";
  case POSE_TOKEN:
    key = 2;
    message = message + 1;
    break;
  }

  if (!*message) {
    notify(player, "What did you want to say?");
    return;
  }
  if (privs == 0) {
    /* to wizards only */
    mask = WIZARD;
    prefix = WIZWALL_PREFIX;
  }
#ifdef ROYALTY_FLAG
  else if (privs == 1) {
    /* to wizards and royalty */
    mask = WIZARD | ROYALTY;
    prefix = RWALL_PREFIX;
  }
#endif				/* ROYALTY_FLAG */
  else {
    /* to everyone */
    mask = 0;
    prefix = WALL_PREFIX;
  }

  bp = tbuf;
  switch (key) {
  case 2:
    safe_str(prefix,tbuf,&bp);
    safe_chr(' ',tbuf,&bp);
    safe_str(Name(player),tbuf,&bp);
    safe_str(gap,tbuf,&bp);
    safe_str(message,tbuf,&bp);
    break;
  case 3:
    safe_str(prefix,tbuf,&bp);
    safe_str(" [",tbuf,&bp);
    safe_str(Name(player),tbuf,&bp);
    safe_str("]: ",tbuf,&bp);
    safe_str(message,tbuf,&bp);
    break;
  default:
    safe_str(prefix,tbuf,&bp);
    safe_chr(' ',tbuf,&bp);
    safe_str(Name(player),tbuf,&bp);
    if (privs != 2)
	safe_str(" says, \"",tbuf,&bp);
    else
	safe_str(" shouts, \"",tbuf,&bp);
    safe_str(message,tbuf,&bp);
    safe_chr('"', tbuf, &bp);
  }
  *bp = '\0';
  /* broadcast the message */
  flag_broadcast(mask, 0, "%s", tbuf);

  /* log it if necessary */
  if ((privs == 2) || (options.log_walls && (privs == 0))) {
    bp = tbuf;
    safe_str("(MSG/",tbuf,&bp);
    safe_str(privs ? "all" : "wiz", tbuf, &bp);
    safe_str(") ", tbuf, &bp);
    if (key == 2) {
      safe_str(Name(player), tbuf, &bp);
      safe_str(gap, tbuf, &bp);
    }
    safe_str(message, tbuf, &bp);
    *bp = '\0';
    do_log(LT_WIZ, player, NOTHING, "%s", tbuf);
  }
}


void do_page(player, arg1, arg2)
    dbref player;
    const char *arg1;
    const char *arg2;
{
  dbref target;
  char *message;
  const char *gap;
  int key;
  char *tbuf, *tbuf2, *tbuf3;
  char *bp3;
  char *head;
  char *tail;
  char spot;
  ATTR *a;
#ifdef TREKMUSH
  ATTR *b;
#endif /* TREKMUSH */

  tbuf = (char *) mush_malloc(BUFFER_LEN, "string");
  tbuf2 = (char *) mush_malloc(BUFFER_LEN, "string");
  tbuf3 = (char *) mush_malloc(BUFFER_LEN, "string");
  if (!tbuf || !tbuf2 || !tbuf3)
    panic("Unable to allocate memory in do_page");

  if (arg2 && *arg2) {
    head = (char *) arg1;
    message = (char *) arg2;
  } else {
    head = NULL;
    message = (char *) arg1;
  }

  if (!head || !*head) {
    a = atr_get_noparent(player, "LASTPAGED");
    if (!a) {
      notify(player, "You haven't paged anyone since connecting.");
      mush_free((Malloc_t) tbuf, "string");
      mush_free((Malloc_t) tbuf2, "string");
      mush_free((Malloc_t) tbuf3, "string");
      return;
    }
    strcpy(tbuf2, uncompress(a->value));
    if (!message || !*message) {
      notify(player, tprintf("You last paged %s.", tbuf2));
      mush_free((Malloc_t) tbuf, "string");
      mush_free((Malloc_t) tbuf2, "string");
      mush_free((Malloc_t) tbuf3, "string");
      return;
    }
    head = (char *) tbuf2;
  }
  if (Haven(player))
    notify(player, "You are set HAVEN and cannot receive pages.");

  bp3 = tbuf3;
  while (head && *head) {
    while (*head == ' ')
      head++;
    tail = head;
    while (*tail && (*tail != ' ')) {
      if (*tail == '"') {
	head++;
	tail++;
	while (*tail && (*tail != '"'))
	  tail++;
      }
      if (*tail)
	tail++;
    }
    tail--;
    if (*tail != '"')
      tail++;
    spot = *tail;
    *tail = 0;
    target = lookup_player(head);
    if (!GoodObject(target))
      target = short_page(head);
#ifdef TREKMUSH
    if (!strcasecmp(head, "me"))
      if (Typeof(player) == TYPE_PLAYER)
  	    target = player;
#endif /* TREKMUSH */
    *tail = spot;
    head = tail;
    if (*head == '"')
      head++;

    if (target == NOTHING) {
      notify(player, "I can't find who you're trying to page.");
      continue;
    } else if (target == AMBIGUOUS) {
      notify(player, "I'm not sure who you want to page!");
      continue;
    } else if (!GoodObject(target)) {
      notify(player, "I can't figure out who you want to page.");
      continue;
    } else if (!Connected(target) ||
#ifdef TREKMUSH
	       ((Dark(target) || hidden(target)) && Haven(target))) {
#else /* TREKMUSH */
	       (Dark(target) && Haven(target))) {
#endif /* TREKMUSH */
      page_return(player, target, "Away", "AWAY",
		  "That player is not connected.");
      continue;
    } else if (Haven(target)) {
      page_return(player, target, "Haven", "HAVEN",
		  "That player is not accepting any pages.");
      continue;
    } else if (!eval_lock(player, target, Page_Lock)) {
      if (Dark(target))
	page_return(player, target, "Away", "AWAY",
		    "That player is not connected.");
      else
	page_return(player, target, "Haven", "HAVEN",
		    "That player is not accepting your pages.");
      continue;
    }
    if (!payfor(player, PAGE_COST)) {
      notify(player, tprintf("You don't have enough %s.", MONIES));
      continue;
    }
    gap = " ";
    switch (*message) {
    case SEMI_POSE_TOKEN:
      gap = "";
    case POSE_TOKEN:
      key = 1;
      break;
    default:
      key = 3;
      break;
    }

    /* Add this person to the list of people last paged */
    if (bp3 != tbuf3)
      safe_chr(' ', tbuf3, &bp3);
    if (strchr(Name(target), ' ')) {
      safe_chr('"', tbuf3, &bp3);
      safe_str(Name(target), tbuf3, &bp3);
      safe_chr('"', tbuf3, &bp3);
    } else {
      safe_str(Name(target), tbuf3, &bp3);
    }

    /* this is a hack: truncate the message if it's going to overflow
     * the tprintf buffer.
     */
    if (message && *message && (strlen(message) > BUFFER_LEN - 32))
      message[BUFFER_LEN - 32] = '\0';

    switch (key) {
    case 1:
      sprintf(tbuf, "From afar, %s%s%s", Name(player), gap, message + 1);
      notify(player, tprintf("Long distance to %s: %s%s%s", Name(target),
			     Name(player), gap, message + 1));
      break;
    case 3:
#ifdef TREKMUSH
      b = atr_get(player, "ALIAS");
      if (b == NULL) {
        sprintf(tbuf, "%s pages: %s", Name(player), message);
      } else
        sprintf(tbuf, "%s (%s) pages: %s", Name(player), uncompress(b->value), message);
#else /* TREKMUSH */
      sprintf(tbuf, "%s pages: %s", Name(player), message);
#endif /* TREKMUSH */
      notify(player, tprintf("You paged %s with '%s'.", Name(target),
			     message));
      break;
    }
    if (!IsPlayer(player) && Nospoof(target))
      notify(target, tprintf("[#%d] %s", player, tbuf));
    else
      notify(target, tbuf);
    page_return(player, target, "Idle", "IDLE", NULL);
  }
  *bp3 = '\0';
  if (tbuf3 && *tbuf3)
    atr_add(player, "LASTPAGED", tbuf3, GOD, NOTHING);
  mush_free((Malloc_t) tbuf, "string");
  mush_free((Malloc_t) tbuf2, "string");
  mush_free((Malloc_t) tbuf3, "string");
}

/* Like page, but combine the names of the pagees. Less private, more
 * esthetic. Limited to 100 recipients.
 */
void
do_multipage(player, arg1, arg2)
    dbref player;
    const char *arg1;
    const char *arg2;
{
  dbref target;
  char *message;
  const char *gap;
  int key;
  char *tbuf, *tbuf2, *tbuf3, *bad;
  char *bp, *bp2, *bp3, *bptr;
  dbref good[100];
  int gcount, bcount;
  char *head;
  char *tail;
  char spot;
  ATTR *a;
#ifdef TREKMUSH
  ATTR *b;
#endif /* TREKMUSH */

  tbuf = (char *) mush_malloc(BUFFER_LEN, "string");
  tbuf2 = (char *) mush_malloc(BUFFER_LEN, "string");
  tbuf3 = (char *) mush_malloc(BUFFER_LEN, "string");
  bad = (char *) mush_malloc(BUFFER_LEN, "string");
  if (arg2 && *arg2) {
    head = (char *) arg1;
    message = (char *) arg2;
  } else {
    head = NULL;
    message = (char *) arg1;
  }

  if (!head || !*head) {
    a = atr_get_noparent(player, "LASTPAGED");
    if (!a) {
      notify(player, "You haven't paged anyone since connecting.");
      mush_free((Malloc_t) tbuf, "string");
      mush_free((Malloc_t) tbuf2, "string");
      mush_free((Malloc_t) tbuf3, "string");
      mush_free((Malloc_t) bad, "string");
      return;
    }
    strcpy(tbuf2, uncompress(a->value));
    if (!message || !*message) {
      notify(player, tprintf("You last paged %s.", tbuf2));
      mush_free((Malloc_t) tbuf, "string");
      mush_free((Malloc_t) tbuf2, "string");
      mush_free((Malloc_t) tbuf3, "string");
      mush_free((Malloc_t) bad, "string");
      return;
    }
    head = (char *) tbuf2;
  }
  if (Haven(player))
    notify(player, "You are set HAVEN and cannot receive pages.");

  /* Figure out what kind of message */
  gap = " ";
  switch (*message) {
  case SEMI_POSE_TOKEN:
    gap = "";
  case POSE_TOKEN:
    key = 1;
    break;
  default:
    key = 3;
    break;
  }

  /* Make up a list of good and bad names */
  gcount = 0;
  bcount = 0;
  bptr = bad;
  safe_str("Unable to page:", bad, &bptr);
  while (head && *head) {
    if (!payfor(player, PAGE_COST)) {
      notify(player, tprintf("You don't have enough %s.", MONIES));
      safe_str(head, bad, &bptr);
      break;
    }
    while (*head == ' ')
      head++;
    tail = head;
    while (*tail && (*tail != ' ')) {
      if (*tail == '"') {
	head++;
	tail++;
	while (*tail && (*tail != '"'))
	  tail++;
      }
      if (*tail)
	tail++;
    }
    tail--;
    if (*tail != '"')
      tail++;
    spot = *tail;
    *tail = 0;
    target = lookup_player(head);
    if (!GoodObject(target))
      target = short_page(head);
#ifdef TREKMUSH
    if (!strcasecmp(head, "me"))
      if (Typeof(player) == TYPE_PLAYER)
  	    target = player;
#endif /* TREKMUSH */
    if (!GoodObject(target)) {
      safe_chr(' ', bad, &bptr);
      safe_str(head, bad, &bptr);
      bcount++;
    } else if (!Connected(target) ||
#ifdef TREKMUSH
	       ((Dark(target) || hidden(target)) && Haven(target))) {
#else /* TREKMUSH */
	       (Dark(target) && Haven(target))) {
#endif /* TREKMUSH */
      page_return(player, target, "Away", "AWAY",
		  "That player is not connected.");
      safe_chr(' ', bad, &bptr);
      safe_str(head, bad, &bptr);
      bcount++;
    } else if (Haven(target)) {
      page_return(player, target, "Haven", "HAVEN",
		  "That player is not accepting any pages.");
      safe_chr(' ', bad, &bptr);
      safe_str(head, bad, &bptr);
      bcount++;
    } else if (!eval_lock(player, target, Page_Lock)) {
      page_return(player, target, "Haven", "HAVEN",
		  "That player is not accepting your pages.");
      safe_chr(' ', bad, &bptr);
      safe_str(head, bad, &bptr);
      bcount++;
    } else {
      /* This is a good page */
      page_return(player, target, "Idle", "IDLE", NULL);
      good[gcount++] = target;
      if (gcount >= 100) {
	notify(player, "Too many page-recipients.");
	break;
      }
    }

    *tail = spot;
    head = tail;
    if (*head == '"')
      head++;
  }

  *bptr = '\0';
  /* Set up list of good names */
  bp = tbuf;
  bp3 = tbuf3;
  for (target = 0; target < gcount; target++) {
    safe_str(Name(good[target]), tbuf, &bp);
    safe_chr(' ', tbuf3, &bp3);
    if (strchr(Name(good[target]), ' ')) {
      safe_chr('"', tbuf3, &bp3);
      safe_str(Name(good[target]), tbuf3, &bp3);
      safe_chr('"', tbuf3, &bp3);
    } else {
      safe_str(Name(good[target]), tbuf3, &bp3);
    }
    switch (gcount) {
    case 1:
      break;
    case 2:
      if (target == gcount - 2)
	safe_str(" and ", tbuf, &bp);
      break;
    default:
      if (target < gcount - 1)
	safe_str(", ", tbuf, &bp);
      if (target == gcount - 2)
	safe_str("and ", tbuf, &bp);
      break;
    }
  }
  *bp = '\0';
  *bp3 = '\0';
  if (tbuf3 && *tbuf3)
    atr_add(player, "LASTPAGED", tbuf3, GOD, NOTHING);

  /* Tell player who s/he paged what */
  bp2 = tbuf2;
  switch (key) {
  case 1:
    safe_str("Long distance to ", tbuf2, &bp2);
    safe_str(tbuf, tbuf2, &bp2);
    safe_str(": ", tbuf2, &bp2);
    safe_str(Name(player), tbuf2, &bp2);
    safe_str(gap, tbuf2, &bp2);
    safe_str(message + 1, tbuf2, &bp2);
    break;
  case 3:
    safe_str("You paged ", tbuf2, &bp2);
    safe_str(tbuf, tbuf2, &bp2);
    safe_str(" with '", tbuf2, &bp2);
    safe_str(message, tbuf2, &bp2);
    safe_str("'.", tbuf2, &bp2);
    break;
  }
  *bp2 = '\0';

  if (gcount)
    notify(player, tbuf2);
  if (bcount)
    notify(player, bad);

  /* Tell each page recipient */
  switch (key) {
  case 1:
    for (target = 0; target < gcount; target++) {
      bp2 = tbuf2;
      if (!IsPlayer(player) && Nospoof(target))
	safe_str(tprintf("[#%d] ", player), tbuf2, &bp);
      safe_str("From afar", tbuf2, &bp2);
      if (gcount > 1) {
	safe_str(" (to ", tbuf2, &bp2);
	safe_str(tbuf, tbuf2, &bp2);
	safe_chr(')', tbuf2, &bp2);
      }
      safe_str(", ", tbuf2, &bp2);
      safe_str(Name(player), tbuf2, &bp2);
      safe_str(gap, tbuf2, &bp2);
      safe_str(message + 1, tbuf2, &bp2);
      *bp2 = '\0';
      notify(good[target], tbuf2);
    }
    break;
  case 3:
    for (target = 0; target < gcount; target++) {
      bp2 = tbuf2;
      if (!IsPlayer(player) && Nospoof(target))
	safe_str(tprintf("[#%d] ", player), tbuf2, &bp);
      safe_str(Name(player), tbuf2, &bp2);
#ifdef TREKMUSH
      b = atr_get(player, "ALIAS");
      if (b != NULL) {
        safe_str(" (", tbuf2, &bp2);
        safe_str(uncompress(b->value), tbuf2, &bp2);
        safe_chr(')', tbuf2, &bp2);
      }
#endif /* TREKMUSH */
      safe_str(" pages", tbuf2, &bp2);
      if (gcount > 1) {
	safe_chr(' ', tbuf2, &bp2);
	safe_str(tbuf, tbuf2, &bp2);
      }
      safe_str(": ", tbuf2, &bp2);
      safe_str(message, tbuf2, &bp2);
      *bp2 = '\0';
      notify(good[target], tbuf2);
    }
    break;
  }
  mush_free((Malloc_t) tbuf, "string");
  mush_free((Malloc_t) tbuf2, "string");
  mush_free((Malloc_t) tbuf3, "string");
  mush_free((Malloc_t) bad, "string");
}

void
ns_esnotify(dest, speaker, func, fdata)
    char *dest;
    dbref speaker;
    dbref (*func) ();
    void *fdata;
{
  if (!GoodObject(speaker))
    *dest = '\0';
  else if (PARANOID_NOSPOOF)
    sprintf(dest, "[%s(#%d)] ", spname(speaker), speaker);
  else
    sprintf(dest, "[%s:] ", spname(speaker));
}


int
filter_found(thing, msg, flag)
    dbref thing;
    const char *msg;
    int flag;


				/* 0 for @filter, 1 for @infilter */
{
  /* check to see if the message matches the filter pattern on thing */

  char *filter;
  ATTR *a;
  char *p, *bp;
  char *temp;			/* need this so we don't leak memory
				 * by failing to free the storage
				 * allocated by safe_uncompress
				 */

  int i;
  int matched = 0;

  if (!flag)
    a = atr_get(thing, "FILTER");
  else
    a = atr_get(thing, "INFILTER");
  if (!a)
    return matched;

  filter = safe_uncompress(a->value);
  temp = filter;

  for (i = 0; (i < MAX_ARG) && !matched; i++) {
    p = bp = filter;
    process_expression(p, &bp, (char const **) &filter, 0, 0, 0,
		       PE_NOTHING, PT_COMMA, NULL);
    if (*filter == ',')
      *filter++ = '\0';
    matched = local_wild_match(p, msg);
  }

  free((Malloc_t) temp);
  return matched;
}

void
propagate_sound(thing, msg)
    dbref thing;
    const char *msg;
{
  /* pass a message on, for AUDIBLE, prepending a prefix, unless the
   * message matches a filter pattern.
   */

  char *bp;
  char const *asave, *ap;
  char tbuf1[BUFFER_LEN];
  ATTR *a;
  dbref loc = Location(thing);
  char *wsave[10], *preserve[10];
  int j;
  dbref pass[2];

  if (!GoodObject(loc))
    return;

  /* check to see that filter doesn't suppress message */
  if (filter_found(thing, msg, 0))
    return;

  /* figure out the prefix */

  a = atr_get(thing, "PREFIX");

  bp = tbuf1;

  if (!a) {
    safe_str("From ", tbuf1, &bp);
    if (IsExit(thing))
      safe_str(Name(Source(thing)), tbuf1, &bp);
    else
      safe_str(Name(thing), tbuf1, &bp);
    safe_chr(',', tbuf1, &bp);
  } else {
    for (j = 0; j < 10; j++) {
      wsave[j] = wenv[j];
      wenv[j] = NULL;
    }
    wenv[0] = (char *) msg;
    save_global_regs("prefix_save", preserve);
    asave = safe_uncompress(a->value);
    ap = asave;
    process_expression(tbuf1, &bp, &ap, thing, orator, orator,
		       PE_DEFAULT, PT_DEFAULT, NULL);
    free((Malloc_t) asave);
    restore_global_regs("prefix_save", preserve);
    for (j = 0; j < 10; j++)
      wenv[j] = wsave[j];
  }
  if (bp != tbuf1)
    safe_chr(' ', tbuf1, &bp);
  safe_str(msg, tbuf1, &bp);
  *bp = '\0';

  /* Exits pass the message on to things in the next room.
   * Objects pass the message on to the things outside.
   * Don't tell yourself your own message.
   */

  if (IsExit(thing)) {
    notify_anything(orator, na_next, (void *) Contents(loc), NULL, 0, tbuf1);
  } else {
    pass[0] = Contents(loc);
    pass[1] = thing;
    notify_anything(orator, na_nextbut, pass, NULL, 0, tbuf1);
  }
}

static void
do_audible_stuff(loc, exc1, exc2, msg)
    dbref loc;
    dbref exc1;
    dbref exc2;
    const char *msg;
{
  char tbuf1[BUFFER_LEN];
  dbref e;
  int did = 0;
  if (Audible(loc)) {
    if (IsRoom(loc)) {
      /* the strcpy is necessary to prevent choking in propagate_sound
       * when msg is an array of char and not a char * in the calling
       * function. Yes, this is ugly and stupid. It works. Kinda.
       */
      if (!did) {
	strcpy(tbuf1, msg);
	did = 1;
      }
      DOLIST(e, Exits(loc)) {
	if (Audible(e))
	  propagate_sound(e, tbuf1);
      }
    } else if ((loc != exc1) && (loc != exc2)) {
      if (!did) {
	strcpy(tbuf1, msg);
	did = 1;
      }
      propagate_sound(loc, tbuf1);
    }
  }
}

void
notify_except(first, exception, msg)
    dbref first;
    dbref exception;
    const char *msg;
{
  dbref loc;
  dbref pass[2];

  if (!GoodObject(first))
    return;
  loc = Location(first);
  if (!GoodObject(loc))
    return;

  if (exception == NOTHING)
    exception = AMBIGUOUS;

  pass[0] = loc;
  pass[1] = exception;
  notify_anything(orator, na_except, pass, &ns_esnotify, 0, msg);

  do_audible_stuff(loc, exception, NOTHING, msg);
}

void
notify_except2(first, exc1, exc2, msg)
    dbref first;
    dbref exc1;
    dbref exc2;
    const char *msg;
{
  dbref loc;
  dbref pass[3];

  if (!GoodObject(first))
    return;
  loc = Location(first);
  if (!GoodObject(loc))
    return;

  if (exc1 == NOTHING)
    exc1 = AMBIGUOUS;
  if (exc2 == NOTHING)
    exc2 = AMBIGUOUS;

  pass[0] = loc;
  pass[1] = exc1;
  pass[2] = exc2;

  notify_anything(orator, na_except2, pass, NULL, 0, msg);

  do_audible_stuff(loc, exc1, exc2, msg);
}

static void
oemit_notify_except(loc, exc1, exc2, msg)
    dbref loc;
    dbref exc1;
    dbref exc2;
    const char *msg;
{
  dbref pass[3];

  if (!GoodObject(loc))
    return;

  if (exc1 == NOTHING)
    exc1 = AMBIGUOUS;
  if (exc2 == NOTHING)
    exc2 = AMBIGUOUS;

  pass[0] = loc;
  pass[1] = exc1;
  pass[2] = exc2;

  notify_anything(orator, na_except2, pass, ns_esnotify, 0, msg);

  do_audible_stuff(loc, exc1, exc2, msg);
}

void
do_think(player, message)
    dbref player;
    const char *message;
{
  /* privately tell yourself a message */

  /* notify the player only, with no special fanfare */
  /* Why was this tprintf'd? */
  notify(player, message);
}


void
do_emit(player, tbuf1)
    dbref player;
    const char *tbuf1;
{
  dbref loc;

  loc = speech_loc(player);
  if (!GoodObject(loc))
    return;

#ifdef SPEECH_LOCK
  if (!Hasprivs(player) && !eval_lock(player, loc, Speech_Lock)) {
    notify(player, "You may not speak here!");
    return;
  }
#endif

  /* notify everybody */
  notify_anything(player, na_loc, (void *) loc, ns_esnotify, 0, tbuf1);

  do_audible_stuff(loc, NOTHING, NOTHING, tbuf1);
}

void
do_remit(player, arg1, arg2)
    dbref player;
    const char *arg1;
    const char *arg2;
{
  dbref room;
  const char *rmno;

  room = match_result(player, arg1, NOTYPE, MAT_EVERYTHING);
  if (!GoodObject(room)) {
    notify(player, "I can't find that.");
  } else {
    if (IsExit(room)) {
      notify(player, "There can't be anything in that!");
    } else if (!okay_pemit(player, room)) {
      notify(player, tprintf("I'm sorry, but %s wishes to be left alone now.",
			     Name(room)));
#ifdef SPEECH_LOCK
    } else if (!Hasprivs(player) && !eval_lock(player, room, Speech_Lock)) {
      notify(player, "You may not speak there!");
#endif
    } else {

      rmno = unparse_object(player, room);
      if (Location(player) == room) {
	notify(player, arg2);
      } else if (!SILENT_PEMIT) {
	notify(player,
	       tprintf("You remit, \"%s\" in %s", arg2, rmno));
      }
      oemit_notify_except(room, player, room, arg2);
    }
  }
}

void
do_lemit(player, tbuf1)
    dbref player;
    const char *tbuf1;
{
  /* give a message to the "absolute" location of an object */

  dbref room;
  int rec = 0;

  /* only players and things may use this command */
  if (!Mobile(player))
    return;

  /* prevent infinite loop if player is inside himself */
  if (((room = Location(player)) == player) || !GoodObject(room)) {
    notify(player, "Invalid container object.");
    fprintf(stderr, "** BAD CONTAINER **  #%d is inside #%d.\n",
	    player, room);
    return;
  }
  while (!IsRoom(room) && (rec < 15)) {
    room = Location(room);
    rec++;
  }
  if (rec > 15) {
    notify(player, "Too many containers.");
    return;
#ifdef SPEECH_LOCK
  } else if (!Hasprivs(player) && !eval_lock(player, room, Speech_Lock)) {
    notify(player, "You may not speak there!");
#endif
  } else {
    notify(player, tprintf("You lemit: \"%s\"", tbuf1));
    oemit_notify_except(room, player, room, tbuf1);
  }
}

dbref
na_zemit(current, data)
    dbref current;
    void *data;
{
  dbref this;
  dbref room;
  dbref *dbrefs = data;
  this = dbrefs[0];
  do {
    if (this == NOTHING) {
      for (room = dbrefs[1]; room < db_top; room++) {
	if (IsRoom(room) && (Zone(room) == dbrefs[2])
#ifdef SPEECH_LOCK
	    && (Hasprivs(dbrefs[3]) || eval_lock(dbrefs[3], room, Speech_Lock))
#endif
	  )
	  break;
      }
      if (!(room < db_top))
	return NOTHING;
      this = room;
      dbrefs[1] = room + 1;
    } else if (IsRoom(this)) {
      this = Contents(this);
    } else {
      this = Next(this);
    }
  } while ((this == NOTHING) || (this == dbrefs[4]));
  dbrefs[0] = this;
  return this;
}

void
do_zemit(player, arg1, arg2)
    dbref player;
    const char *arg1;
    const char *arg2;
{
  const char *where;
  dbref zone;
  dbref pass[5];

  zone = match_result(player, arg1, NOTYPE, MAT_ABSOLUTE);
  if (!GoodObject(zone)) {
    notify(player, "Invalid zone.");
    return;
  }
  if (!controls(player, zone)) {
    notify(player, "Permission denied.");
    return;
  }
  /* this command is computationally expensive */
  if (!payfor(player, FIND_COST)) {
    notify(player, "Sorry, you don't have enough money to do that.");
    return;
  }
  where = unparse_object(player, zone);
  notify(player,
	 tprintf("You zemit, \"%s\" in zone %s", arg2, where));

  pass[0] = NOTHING;
  pass[1] = 0;
  pass[2] = zone;
  pass[3] = player;
  pass[4] = player;
  notify_anything(player, na_zemit, pass, ns_esnotify, 0, arg2);
}
