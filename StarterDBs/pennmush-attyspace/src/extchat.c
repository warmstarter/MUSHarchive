
#include "copyrite.h"
#include "config.h"
#include <ctype.h>
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif

#include "conf.h"
#include "mushdb.h"
#include "intrface.h"
#include "match.h"
#include "externs.h"
#include "extchat.h"
#include "ansi.h"
#include "privtab.h"
#include "mymalloc.h"
#include "pueblo.h"
#include "confmagic.h"

#ifdef CHAT_SYSTEM
static CHAN *new_channel _((void));
static CHANLIST *new_chanlist _((void));
static CHANUSER *new_user _((dbref who));
static void free_channel _((CHAN *c));
static void free_chanlist _((CHANLIST * cl));
static void free_user _((CHANUSER *u));
static int load_channel _((FILE * fp, CHAN *ch));
static int load_chanusers _((FILE * fp, CHAN *ch));
static void insert_channel _((CHAN **ch));
static void remove_channel _((CHAN *ch));
static void insert_obj_chan _((dbref who, CHAN **ch));
static void remove_obj_chan _((dbref who, CHAN *ch));
void remove_all_obj_chan _((dbref thing));
static void chan_chown _((CHAN *c, dbref victim));
void chan_chownall _((dbref old, dbref new));
static int insert_user _((CHANUSER *user, CHAN *ch));
static int remove_user _((CHANUSER *u, CHAN *ch));
static int save_channel _((FILE * fp, CHAN *ch));
static int save_chanuser _((FILE * fp, CHANUSER *user));
static void channel_wipe _((dbref player, CHAN *chan));
static int yesno _((const char *str));
static int canstilladd _((dbref player));
extern void do_channel_who _((dbref player, CHAN *chan));

#define YES 1
#define NO 0
#define ERR -1

#define insert_user_by_dbref(who,chan) \
	insert_user(new_user(who),chan)
#define remove_user_by_dbref(who,chan) \
	remove_user(onchannel(who,chan),chan)

int num_channels;

CHAN *channels;

static PRIV priv_table[] =
{
  {"Disabled", 'D', CHANNEL_DISABLED, CHANNEL_DISABLED},
  {"Admin", 'A', CHANNEL_ADMIN | CHANNEL_PLAYER, CHANNEL_ADMIN},
  {"Wizard", 'W', CHANNEL_WIZARD | CHANNEL_PLAYER, CHANNEL_WIZARD},
  {"Player", 'P', CHANNEL_PLAYER, CHANNEL_PLAYER},
  {"Object", 'O', CHANNEL_OBJECT, CHANNEL_OBJECT},
  {"Quiet", 'Q', CHANNEL_QUIET, CHANNEL_QUIET},
  {"Hide_Ok", 'H', CHANNEL_CANHIDE, CHANNEL_CANHIDE},
  {NULL, '\0', 0, 0}
};


/* Is the player on the channel? If so, return a pointer to player's
 * CHANUSER
 */
CHANUSER *
onchannel(who, ch)
    dbref who;
    CHAN *ch;
{
  static CHANUSER *u;
  for (u = ChanUsers(ch); u; u = u->next) {
    if (CUdbref(u) == who) {
      return u;
    }
  }
  return NULL;
}

/* A macro to test if a channel exists and, if not, to notify */
#define test_channel(player,name,chan) \
   do { \
    chan = NULL; \
    switch (find_channel(name,&chan)) { \
    case CMATCH_NONE: \
      notify(player, "CHAT: I don't recognize that channel."); \
      return; \
    case CMATCH_AMBIG: \
      notify(player, "CHAT: I don't know which channel you mean."); \
      return; \
     } \
    } while (0)

/*----------------------------------------------------------
 * Loading and saving the chatdb
 * The chatdb's format is pretty straightforward
 * Return 1 on success, 0 on failure
 */

void
init_chatdb()
{
  num_channels = 0;
  channels = NULL;
}

int
load_chatdb(fp)
    FILE *fp;
{
  int i;
  CHAN *ch;

  /* How many channels? */
  num_channels = getref(fp);
  if (num_channels > MAX_CHANNELS)
    return 0;

  /* Load all channels */
  for (i = 0; i < num_channels; i++) {
    if (feof(fp))
      break;
    ch = new_channel();
    if (!ch)
      break;
    if (!load_channel(fp, ch)) {
      fprintf(stderr, "Unable to load channel %d.", i);
      free_channel(ch);
      break;
    }
    insert_channel(&ch);
  }
  num_channels = i;

  return 1;
}

/* Malloc memory for a new channel, and initialize it */
static CHAN *
new_channel()
{
  CHAN *ch;

  ch = (CHAN *) mush_malloc(sizeof(CHAN), "CHAN");
  if (!ch)
    return NULL;
  ch->name[0] = '\0';
  ch->title[0] = '\0';
  ChanType(ch) = CHANNEL_DEFAULT_FLAGS;
  ChanCreator(ch) = NOTHING;
  ChanCost(ch) = CHANNEL_COST;
  ChanNext(ch) = NULL;
  ChanNumMsgs(ch) = 0;
  /* By default channels are public but mod-lock'd to the creator */
  ChanJoinLock(ch) = TRUE_BOOLEXP;
  ChanSpeakLock(ch) = TRUE_BOOLEXP;
  ChanSeeLock(ch) = TRUE_BOOLEXP;
  ChanHideLock(ch) = TRUE_BOOLEXP;
  ChanModLock(ch) = TRUE_BOOLEXP;
  ChanNumUsers(ch) = 0;
  ChanMaxUsers(ch) = 0;
  ChanUsers(ch) = NULL;
  return ch;
}

/* Malloc memory for a new user, and initialize it */
static CHANUSER *
new_user(who)
    dbref who;
{
  CHANUSER *u;
  u = (CHANUSER *) mush_malloc(sizeof(CHANUSER), "CHANUSER");
  if (!u)
    return NULL;
  CUdbref(u) = who;
  CUtype(u) = CU_DEFAULT_FLAGS;
  u->title[0] = '\0';
  CUnext(u) = NULL;
  return u;
}

/* Free memory from a channel */
static void
free_channel(c)
    CHAN *c;
{
  CHANUSER *u, *unext;
  if (!c)
    return;
  if (ChanJoinLock(c))
    free(ChanJoinLock(c));
  if (ChanSpeakLock(c))
    free(ChanSpeakLock(c));
  if (ChanHideLock(c))
    free(ChanHideLock(c));
  if (ChanSeeLock(c))
    free(ChanSeeLock(c));
  if (ChanModLock(c))
    free(ChanModLock(c));
  u = ChanUsers(c);
  while (u) {
    unext = u->next;
    free_user(u);
    u = unext;
  }
  return;
}

/* Free memory from a channel user */
static void
free_user(u)
    CHANUSER *u;
{
  if (u)
    mush_free(u, "CHANUSER");
}


/* Load in a single channel into position i. Return 1 if
 * successful, 0 otherwise.
 */
static int
load_channel(fp, ch)
    FILE *fp;
    CHAN *ch;
{
  strcpy(ChanName(ch), getstring_noalloc(fp));
  if (feof(fp))
    return 0;
  strcpy(ChanTitle(ch), getstring_noalloc(fp));
  ChanType(ch) = getref(fp);
  ChanCreator(ch) = getref(fp);
  ChanCost(ch) = getref(fp);
  ChanNumMsgs(ch) = 0;
  ChanJoinLock(ch) = getboolexp(fp);
  ChanSpeakLock(ch) = getboolexp(fp);
  ChanModLock(ch) = getboolexp(fp);
  ChanSeeLock(ch) = getboolexp(fp);
  ChanHideLock(ch) = getboolexp(fp);
  ChanNumUsers(ch) = getref(fp);
  ChanMaxUsers(ch) = ChanNumUsers(ch);
  ChanUsers(ch) = NULL;
  if (ChanNumUsers(ch) > 0)
    return (ChanNumUsers(ch) = load_chanusers(fp, ch));
  return 1;
}

/* Load the *channel's user list. Return number of users on success, or 0 */
static int
load_chanusers(fp, ch)
    FILE *fp;
    CHAN *ch;
{
  int i, num = 0;
  CHANUSER *user;
  dbref player;
  for (i = 0; i < ChanNumUsers(ch); i++) {
    player = getref(fp);
    /* Don't bother if the player isn't a valid dbref or the wrong type */
    if (GoodObject(player) && Chan_Ok_Type(ch, player)) {
      user = new_user(player);
      if (!user)
	return 0;
      CUtype(user) = getref(fp);
      strcpy(CUtitle(user), getstring_noalloc(fp));
      CUnext(user) = NULL;
      if (insert_user(user, ch))
	num++;
    } else {
      /* But be sure to read (and discard) the player's info */
      do_log(LT_ERR, 0, 0, "Bad object #%d removed from channel %s",
	     player, ChanName(ch));
      (void) getref(fp);
      (void) getstring_noalloc(fp);
    }
  }
  return num;
}


/* Insert the channel onto the list of channels, sorted by name */
static void
insert_channel(ch)
    CHAN **ch;
{
  CHAN *p;

  if (!ch || !*ch)
    return;

  /* If there's no users on the list, or if the first user is already
   * alphabetically greater, user should be the first entry on the list */
  /* No channels? */
  if (!channels) {
    channels = *ch;
    channels->next = NULL;
    return;
  }
  p = channels;
  /* First channel? */
  if (strcasecmp(ChanName(p), ChanName(*ch)) > 0) {
    channels = *ch;
    channels->next = p;
    return;
  }
  /* Otherwise, find which user this user should be inserted after */
  for (;
       p->next && (strcasecmp(ChanName(p->next), ChanName(*ch)) < 0);
       p = p->next) ;
  (*ch)->next = p->next;
  p->next = *ch;
  return;
}

/* Remove a channel from the list, but don't free it */
static void
remove_channel(ch)
    CHAN *ch;
{
  CHAN *p;

  if (!ch)
    return;
  if (!channels)
    return;
  if (channels == ch) {
    /* First channel */
    channels = ch->next;
    return;
  }
  /* Otherwise, find the channel before this one */
  for (p = channels;
       p->next && (p->next != ch);
       p = p->next) ;

  if (p->next) {
    p->next = ch->next;
  }
  return;
}

/* Insert the channel onto the list of channels on a given object,
 * sorted by name
 */
static void
insert_obj_chan(who, ch)
    dbref who;
    CHAN **ch;
{
  CHANLIST *p;
  CHANLIST *tmp;

  if (!ch || !*ch)
    return;

  tmp = new_chanlist();
  if (!tmp)
    return;
  tmp->chan = *ch;
  /* If there's no channels on the list, or if the first channel is already
   * alphabetically greater, user should be the first entry on the list */
  /* No channels? */
  if (!Chanlist(who)) {
    Chanlist(who) = tmp;
    Chanlist(who)->next = NULL;
    return;
  }
  p = Chanlist(who);
  /* First channel? */
  if (strcasecmp(ChanName(p->chan), ChanName(*ch)) > 0) {
    Chanlist(who) = tmp;
    Chanlist(who)->next = p;
    return;
  } else if (!strcasecmp(ChanName(p->chan), ChanName(*ch))) {
    /* Don't add the same channel twice! */
    free_chanlist(tmp);
  } else {
    /* Otherwise, find which user this user should be inserted after */
    for (;
     p->next && (strcasecmp(ChanName(p->next->chan), ChanName(*ch)) < 0);
	 p = p->next) ;
    if (p->next && !strcasecmp(ChanName(p->next->chan), ChanName(*ch))) {
      /* Don't add the same channel twice! */
      free_chanlist(tmp);
    } else {
      tmp->next = p->next;
      p->next = tmp;
    }
  }
  return;
}

/* Remove a channel from the obj's chanlist, and free the chanlist ptr */
static void
remove_obj_chan(who, ch)
    dbref who;
    CHAN *ch;
{
  CHANLIST *p, *q;

  if (!ch)
    return;
  if (!Chanlist(who))
    return;
  p = Chanlist(who);
  if (p->chan == ch) {
    /* First channel */
    Chanlist(who) = p->next;
    free_chanlist(p);
    return;
  }
  /* Otherwise, find the channel before this one */
  for (;
       p->next && (p->next->chan != ch);
       p = p->next) ;

  if (p->next) {
    q = p->next;
    p->next = p->next->next;
    free_chanlist(q);
  }
  return;
}

/* Remove all channels from the obj's chanlist, freeing them */
void
remove_all_obj_chan(thing)
    dbref thing;
{
  CHANLIST *p, *nextp;
  for (p = Chanlist(thing); p; p = nextp) {
    nextp = p->next;
    remove_user_by_dbref(thing, p->chan);
  }
  return;
}


static CHANLIST *
new_chanlist()
{
  CHANLIST *c;
  c = (CHANLIST *) mush_malloc(sizeof(CHANLIST), "CHANLIST");
  if (!c)
    return NULL;
  c->chan = NULL;
  c->next = NULL;
  return c;
}

static void
free_chanlist(cl)
    CHANLIST *cl;
{
  mush_free(cl, "CHANLIST");
}


/* Insert the user onto the channel's list, sorted by the user's name */
static int
insert_user(user, ch)
    CHANUSER *user;
    CHAN *ch;
{
  CHANUSER *p;

  if (!user || !ch)
    return 0;

  /* If there's no users on the list, or if the first user is already
   * alphabetically greater, user should be the first entry on the list */
  p = ChanUsers(ch);
  if (!p ||
      (strcasecmp(Name(CUdbref(p)), Name(CUdbref(user))) > 0)) {
    user->next = ChanUsers(ch);
    ChanUsers(ch) = user;
  } else {
    /* Otherwise, find which user this user should be inserted after */
    for (;
	 p->next && (strcasecmp(Name(CUdbref(p->next)), Name(CUdbref(user))) <= 0);
	 p = p->next) ;
    if (CUdbref(p) == CUdbref(user)) {
      /* Don't add the same user twice! */
      mush_free((Malloc_t) user, "CHANUSER");
      return 0;
    } else {
      user->next = p->next;
      p->next = user;
    }
  }
  insert_obj_chan(CUdbref(user), &ch);
  return 1;
}

/* Remove a user from a channel list, and free it */
static int
remove_user(u, ch)
    CHANUSER *u;
    CHAN *ch;
{
  CHANUSER *p;
  dbref who;

  if (!ch || !u)
    return 0;
  p = ChanUsers(ch);
  if (!p)
    return 0;
  who = CUdbref(u);
  if (p == u) {
    /* First user */
    ChanUsers(ch) = p->next;
    free_user(u);
  } else {
    /* Otherwise, find the user before this one */
    for (;
	 p->next && (p->next != u);
	 p = p->next) ;

    if (p->next) {
      p->next = u->next;
      free_user(u);
    } else
      return 0;
  }

  /* Now remove the channel from the user's chanlist */
  remove_obj_chan(who, ch);
  ChanNumUsers(ch)--;
  return 1;
}


int
save_chatdb(fp)
    FILE *fp;
{
  CHAN *ch;

  /* How many channels? */
  putref(fp, num_channels);

  for (ch = channels; ch; ch = ch->next) {
    save_channel(fp, ch);
  }
  return 1;
}

/* Save a single channel. Return 1 if  successful, 0 otherwise.
 */
static int
save_channel(fp, ch)
    FILE *fp;
    CHAN *ch;
{
  CHANUSER *cu;
  putstring(fp, ChanName(ch));
  putstring(fp, ChanTitle(ch));
  putref(fp, ChanType(ch));
  putref(fp, ChanCreator(ch));
  putref(fp, ChanCost(ch));
  putboolexp(fp, ChanJoinLock(ch));
  putboolexp(fp, ChanSpeakLock(ch));
  putboolexp(fp, ChanModLock(ch));
  putboolexp(fp, ChanSeeLock(ch));
  putboolexp(fp, ChanHideLock(ch));
  putref(fp, ChanNumUsers(ch));
  for (cu = ChanUsers(ch); cu; cu = cu->next)
    save_chanuser(fp, cu);
  return 1;
}

/* Save the channel's user list. Return 1 on success, 0 on failure */
static int
save_chanuser(fp, user)
    FILE *fp;
    CHANUSER *user;
{
  putref(fp, CUdbref(user));
  putref(fp, CUtype(user));
  putstring(fp, CUtitle(user));
  return 1;
}

/*-------------------------------------------------------------*
 * Some utility functions:
 *  find_channel - given a name, return a channel
 *  find_channel_partial - given a name and a player, return
 *    the first channel that matches name that player is on.
 *  onchannel - is player on channel?
 */

/* Given name and a chan pointer, set chan pointer to point to
 * channel if found (NULL otherwise), and return an indication
 * of how good the match was 
 */
int
find_channel(name, chan)
    const char *name;
    CHAN **chan;
{
  CHAN *p;
  int count = 0;

  *chan = NULL;
  if (!name || !*name)
    return CMATCH_NONE;
  for (p = channels; p; p = p->next) {
    if (!strcasecmp(name, ChanName(p))) {
      *chan = p;
      return CMATCH_EXACT;
    }
    if (string_prefix(ChanName(p), name)) {
      *chan = p;
      count++;
    }
  }
  switch (count) {
  case 0:
    return CMATCH_NONE;
  case 1:
    return CMATCH_PARTIAL;
  }
  return CMATCH_AMBIG;
}


/* Given a name and a chan pointer, set chan pointer to point
 * to channel if found. If the channel is ambiguous, return
 * the first channel matched that the player is on.
 */
int
find_channel_partial(name, chan, player)
    const char *name;
    CHAN **chan;
    dbref player;
{
  CHAN *p;
  int count = 0;

  *chan = NULL;
  if (!name || !*name)
    return CMATCH_NONE;
  for (p = channels; p; p = p->next) {
    if (!strcasecmp(name, ChanName(p))) {
      *chan = p;
      return CMATCH_EXACT;
    }
    if (string_prefix(ChanName(p), name)) {
      /* If we've already found an ambiguous match that the
       * player is on, keep using that one. Otherwise, this is
       * our best candidate so far.
       */
      if (!*chan || !onchannel(player, *chan))
	*chan = p;
      count++;
    }
  }
  switch (count) {
  case 0:
    return CMATCH_NONE;
  case 1:
    return CMATCH_PARTIAL;
  }
  return CMATCH_AMBIG;
}


/*--------------------------------------------------------------*
 * User commands:
 *  do_channel - @channel/on,off,who
 *  do_chan_admin - @channel/add,delete,name,priv,quiet
 *  do_chan_desc
 *  do_chan_title
 *  do_chan_lock
 *  do_chan_boot
 *  do_chan_wipe
 */

void
do_channel(player, name, target, com)
    dbref player;
    const char *name;
    const char *target;
    const char *com;
{
  /* join, quit, wipe, or who a channel */

  CHAN *chan = NULL;
  dbref victim;

  if (!name && !*name) {
    notify(player, "You need to specify a channel.");
    return;
  }
  if (!com && !*com) {
    notify(player, "What do you want to do with that channel?");
    return;
  }
  test_channel(player, name, chan);
  if (!Chan_Can_See(chan, player)) {
    if (onchannel(player, chan))
      notify(player, tprintf("CHAT: You can't do that with channel <%s>.", ChanName(chan)));
    else
      notify(player, "CHAT: I don't recognize that channel.");
    return;
  }
  if (!strcasecmp(com, "who")) {
    do_channel_who(player, chan);
    return;
  } else if (!strcasecmp(com, "wipe")) {
    channel_wipe(player, chan);
    return;
  }
  /* It's on or off now */
  /* Determine who is getting added or deleted. If we don't have
   * an argument, we assume it's the player.
   */
  if (!target || !*target)
    victim = player;
  else if ((victim = lookup_player(target)) != NOTHING) ;
  else if (Channel_Object(chan))
    victim = match_result(player, target, TYPE_THING, MAT_OBJECTS);
  else
    victim = NOTHING;
  if (!GoodObject(victim)) {
    notify(player, "Invalid target.");
    return;
  }
  if (!Chan_Ok_Type(chan, victim)) {
    notify(player,
	   tprintf("Sorry, wrong type of thing for channel <%s>.", ChanName(chan)));
    return;
  }
  if (!strcasecmp("on", com)) {
    if (Guest(player)) {
      notify(player, "Guests are not allowed to join channels.");
      return;
    }
    if (!controls(player, victim)) {
      notify(player, "Invalid target.");
      return;
    }
    /* Is victim already on the channel? */
    if (onchannel(victim, chan)) {
      notify(player, tprintf("%s is already on channel <%s>.", Name(victim),
			     ChanName(chan)));
      return;
    }
    /* Does victim pass the joinlock? */
    if (!Chan_Can_Join(chan, victim)) {
      if (Wizard(player)) {
	/* Wizards can override join locks */
	notify(player, "CHAT: Warning: Player does not meet channel join permissions (joining anyway)");
      } else {
	notify(player, "Permission to join denied.");
	return;
      }
    }
    if (insert_user_by_dbref(victim, chan)) {
      if (victim != player) {
	notify(victim, tprintf("CHAT: %s joins you to channel <%s>.", Name(player), ChanName(chan)));
	notify(player, tprintf("CHAT: You join %s to channel <%s>.", Name(victim), ChanName(chan)));
      } else
	notify(victim, tprintf("CHAT: You join channel <%s>.", ChanName(chan)));
      if (!Channel_Quiet(chan) && !DarkLegal(victim))
	channel_broadcast(chan, victim, 1, "<%s> %s has joined this channel.",
			  ChanName(chan), Name(victim));
      ChanNumUsers(chan)++;
    } else {
      notify(player, tprintf("%s is already on channel <%s>.", Name(victim),
			     ChanName(chan)));
    }
    return;
  } else if (!strcasecmp("off", com)) {
    /* You must control either the victim or the channel */
    if (!controls(player, victim) && !Chan_Can_Modify(chan, player)) {
      notify(player, "Invalid target.");
      return;
    }
    if (Guest(player)) {
      notify(player, "Guests may not leave channels.");
      return;
    }
    if (remove_user_by_dbref(victim, chan)) {
      if (!Channel_Quiet(chan) && !DarkLegal(victim))
	channel_broadcast(chan, victim, 1, "<%s> %s has left this channel.",
			  ChanName(chan), Name(victim));
      if (victim != player) {
	notify(victim, tprintf("CHAT: %s removes you from channel <%s>.", Name(player), ChanName(chan)));
	notify(player, tprintf("CHAT: You remove %s from channel <%s>.", Name(victim), ChanName(chan)));
      } else
	notify(victim, tprintf("CHAT: You leave channel <%s>.", ChanName(chan)));
    } else {
      notify(player, tprintf("%s is not on channel <%s>.", Name(victim),
			     ChanName(chan)));
    }
    return;
  } else {
    notify(player, "I don't understand what you want to do.");
    return;
  }
}

int
do_chat_by_name(player, name, msg)
    dbref player;
    const char *name;
    const char *msg;
{
  /* Parse the name and call do_chat. If name fails,
   * return silently.
   */
  CHAN *c;
  c = NULL;
  if (!msg || !*msg)
    return 0;
  if (find_channel_partial(name, &c, player) == CMATCH_NONE)
    return 0;
  do_chat(player, c, msg);
  return 1;
}

void
do_chat(player, chan, arg1)
    dbref player;
    CHAN *chan;
    const char *arg1;
{
  /* send a message to a channel */
  CHANUSER *u;
  int key;
  const char *gap;
  char *title;

  if (!Chan_Ok_Type(chan, player)) {
    notify(player,
       tprintf("Sorry, you're not the right type to be on channel <%s>.",
	       ChanName(chan)));
    return;
  }
  if (!Chan_Can_Speak(chan, player)) {
    notify(player,
	   tprintf("Sorry, you're not allowed to speak on channel <%s>.", ChanName(chan)));
    return;
  }
  /* figure out what kind of message we have */
  gap = " ";
  switch (*arg1) {
  case SEMI_POSE_TOKEN:
    gap = "";
    /* FALLTHRU */
  case POSE_TOKEN:
    key = 1;
    arg1 = arg1 + 1;
    break;
  case SAY_TOKEN:
    key = 2;
    arg1 = arg1 + 1;
    break;
  case '\0':
    key = 3;
    break;
  default:
    key = 2;
    break;
  }

  if ((u = onchannel(player, chan)) &&CUtitle(u) && *CUtitle(u))
     title = CUtitle(u);
  else
    title = NULL;

  /* now send out the message. If the player isn't on that channel, tell
   * him what he said.
   */
  switch (key) {
  case 1:
    channel_broadcast(chan, player, 0, "<%s> %s%s%s%s%s", ChanName(chan),
		      title ? title : "", title ? " " : "",
		      Name(player), gap, arg1);
    if (!onchannel(player, chan))
      notify(player, tprintf("To channel %s: %s%s%s", ChanName(chan),
			     Name(player), gap, arg1));
    break;
  case 2:
    channel_broadcast(chan, player, 0, "<%s> %s%s%s says, \"%s\"", ChanName(chan),
		      title ? title : "", title ? " " : "",
		      Name(player), arg1);
    if (!onchannel(player, chan))
      notify(player, tprintf("To channel %s: %s says, \"%s\"",
			     ChanName(chan), Name(player), arg1));
    break;
  case 3:
    notify(player, "What do you want to say to that channel?");
    break;
  }
  ChanNumMsgs(chan)++;
}

void
do_cemit(player, name, msg, noisy)
    dbref player;
    const char *name;
    const char *msg;
    int noisy;			/* Prepend chan name? */
{
  /* Send a message to a channel, no prefix. */
  CHAN *chan = NULL;

  if (!Can_Cemit(player)) {
    notify(player, "You can't channel-surf that well.");
    return;
  }
  if (!name || !*name) {
    notify(player, "That is not a valid channel.");
    return;
  }
  switch (find_channel(name, &chan)) {
  case CMATCH_NONE:
    notify(player, "I don't recognize that channel.");
    return;
  case CMATCH_AMBIG:
    notify(player, "I don't know which channel you mean.");
    return;
  }
  if (!Chan_Can_See(chan, player)) {
    notify(player, "CHAT: I don't recognize that channel.");
    return;
  }
  if (!msg || !*msg) {
    notify(player, "What do you want to emit?");
    return;
  }
  if (noisy)
    channel_broadcast(chan, player, 2, "<%s> %s", ChanName(chan), msg);
  else
    channel_broadcast(chan, player, 2, "%s", msg);
  if (!onchannel(player, chan))
    notify(player, tprintf("Cemit to channel %s: %s",
			   ChanName(chan), msg));
  ChanNumMsgs(chan)++;
  return;
}

void
do_chan_admin(player, name, perms, flag)
    dbref player;
    char *name;
    const char *perms;
    int flag;
{
  /* Based on flag:
   * 0: add name=perms
   * 1: delete name
   * 2: rename name=newname
   * 3: priv name=newpriv
   * 4: quiet name=on/off
   */

  CHAN *chan = NULL, *temp = NULL;
  long int type;
  int res;
  struct boolexp *key;
  char old[CHAN_NAME_LEN];

  if (!name || !*name) {
    notify(player, "You must specify a channel.");
    return;
  }
  if (Guest(player)) {
    notify(player, "Guests may not modify channels.");
    return;
  }
  if ((flag > 1) && (!perms || !*perms)) {
    notify(player, "What do you want to do with the channel?");
    return;
  }
  /* Make sure we've got a unique channel name unless we're
   * adding a channel */
  if (flag)
    test_channel(player, name, chan);
  switch (flag) {
  case 0:
    /* add a channel */
    if (num_channels == MAX_CHANNELS) {
      notify(player, "No more room for channels.");
      return;
    }
    if (strlen(name) > CHAN_NAME_LEN - 1) {
      notify(player, "The channel needs a shorter name.");
      return;
    }
    if (!Hasprivs(player) && !canstilladd(player)) {
      notify(player, "You already own too many channels.");
      return;
    }
    res = find_channel(name, &chan);
    if (res != CMATCH_NONE) {
      notify(player, "CHAT: The channel needs a more unique name.");
      return;
    }
    /* get the permissions. Invalid specs default to the default */
    type = string_to_privs(priv_table, perms, 0);
    if (!Chan_Can(player, type)) {
      notify(player, "You can't create channels of that type.");
      return;
    }
    if (type & CHANNEL_DISABLED)
      notify(player, "Warning: channel will be created disabled.");
    /* Can the player afford it? There's a cost */
    if (!payfor(Owner(player), CHANNEL_COST)) {
      notify(player, tprintf("You can't afford the %d %s.", CHANNEL_COST,
			     MONIES));
      return;
    }
    /* Ok, let's do it */
    chan = new_channel();
    if (!chan) {
      notify(player, "CHAT: No more memory for channels!");
      giveto(Owner(player), CHANNEL_COST);
      return;
    }
    key = parse_boolexp(player, tprintf("=#%d", player));
    if (!key) {
      mush_free(chan, "CHAN");
      notify(player, "CHAT: No more memory for channels!");
      giveto(Owner(player), CHANNEL_COST);
      return;
    }
    ChanModLock(chan) = key;
    num_channels++;
    if (type)
      ChanType(chan) = type;
    ChanCreator(chan) = Owner(player);
    strcpy(ChanName(chan), name);
    insert_channel(&chan);
    notify(player, tprintf("CHAT: Channel <%s> created.", ChanName(chan)));
    break;
  case 1:
    /* remove a channel */
    /* Check permissions. Wizards and owners can remove */
    if (!Chan_Can_Nuke(chan, player)) {
      notify(player, "Permission denied.");
      return;
    }
    /* remove everyone from the channel */
    channel_wipe(player, chan);
    /* refund the owner's money */
    giveto(ChanCreator(chan), ChanCost(chan));
    /* zap the channel */
    remove_channel(chan);
    free_channel(chan);
    num_channels--;
    notify(player, "Channel removed.");
    break;
  case 2:
    /* rename a channel */
    /* Can the player do this? */
    if (!Chan_Can_Modify(chan, player)) {
      notify(player, "Permission denied.");
      return;
    }
    /* make sure the channel name is unique */
    if (find_channel(perms, &temp)) {
      /* But allow renaming a channel to a differently-cased version of
       * itself 
       */
      if (temp != chan) {
	notify(player, "The channel needs a more unique new name.");
	return;
      }
    }
    if (strlen(perms) > CHAN_NAME_LEN - 1) {
      notify(player, "That name is too long.");
      return;
    }
    /* When we rename a channel, we actually remove it and re-insert it */
    strcpy(old, ChanName(chan));
    remove_channel(chan);
    strcpy(ChanName(chan), perms);
    insert_channel(&chan);
    channel_broadcast(chan, player, 1, "<%s> %s has renamed channel %s to %s.",
		      ChanName(chan), Name(player), old, ChanName(chan));
    notify(player, "Channel renamed.");
    break;
  case 3:
    /* change the permissions on a channel */
    if (!Chan_Can_Modify(chan, player)) {
      notify(player, "Permission denied.");
      return;
    }
    /* get the permissions. Invalid specs default to no change */
    type = string_to_privs(priv_table, perms, ChanType(chan));
    if (!Chan_Can_Priv(player, type)) {
      notify(player, "You can't make channels that type.");
      return;
    }
    if (type & CHANNEL_DISABLED)
      notify(player, "Warning: channel will be disabled.");
    ChanType(chan) = type;
    notify(player, tprintf("Permissions on channel <%s> changed.", ChanName(chan)));
    break;
  case 4:
    /* Quiet a channel */
    if (!Chan_Can_Modify(chan, player)) {
      notify(player, "Permission denied. Use @channel/mute <chan>=<y/n>");
      return;
    }
    switch (yesno(perms)) {
    case YES:
      ChanType(chan) |= CHANNEL_QUIET;
      break;
    case NO:
      ChanType(chan) &= ~CHANNEL_QUIET;
      break;
    default:
      notify(player, "Quiet status must be 'yes' or 'no'.");
      return;
    }
    notify(player,
       tprintf("Quiet status on channel <%s> changed.", ChanName(chan)));
    break;
  }
}

void
do_chan_user_flags(player, name, isyn, flag, silent)
    dbref player;
    char *name;
    const char *isyn;
    int flag;			/* 0 = mute, 1 = hide, 2 = gag, 3 = notitles */
    int silent;
{
  CHAN *c = NULL;
  CHANUSER *u;

  if (!name || !*name) {
    notify(player, "You must specify a channel.");
    return;
  }
  test_channel(player, name, c);
  u = onchannel(player, c);
  if (!u) {
    if (!silent)
      notify(player, tprintf("You are not on channel <%s>.", ChanName(c)));
    return;
  }
  switch (flag) {
  case 0:
    /* Mute */
    switch (yesno(isyn)) {
    case YES:
      CUtype(u) |= CU_QUIET;
      if (!silent)
	notify(player, tprintf(
	  "You will no longer hear connection messages on channel <%s>.",
				ChanName(c)));
      break;
    case NO:
      CUtype(u) &= ~CU_QUIET;
      if (!silent)
	notify(player, tprintf(
		"You will now hear connection messages on channel <%s>.",
				ChanName(c)));
      break;
    default:
      if (!silent)
	notify(player, "Mute status must be 'yes' or 'no'.");
      return;
    }
    return;

  case 1:
    /* Hide */
    switch (yesno(isyn)) {
    case YES:
      if (!Chan_Can_Hide(c, player) && !Wizard(player)) {
	if (!silent)
	  notify(player, tprintf("You are not permitted to hide on channel <%s>.", ChanName(c)));
	return;
      }
      CUtype(u) |=CU_HIDE;
      if (!silent)
	notify(player,
	       tprintf("You no longer appear on channel <%s>'s who list.",
		       ChanName(c)));
      break;
    case NO:
      CUtype(u) &= ~CU_HIDE;
      if (!silent)
	notify(player,
	       tprintf("You now appear on channel <%s>'s who list.", ChanName(c)));
      break;
    default:
      if (!silent)
	notify(player, "Hide status must be 'yes' or 'no'.");
      return;
    }
    return;

  case 2:
    /* Gag */
    switch (yesno(isyn)) {
    case YES:
      CUtype(u) |= CU_GAG;
      if (!silent)
	notify(player,
	       tprintf("You will no longer hear messages on channel <%s>.", ChanName(c)));
      break;
    case NO:
      CUtype(u) &= ~CU_GAG;
      if (!silent)
	notify(player,
	       tprintf("You will now hear messages on channel <%s>.", ChanName(c)));
      break;
    default:
      if (!silent)
	notify(player, "Gag status must be 'yes' or 'no'.");
      return;
    }
    return;
  }
  /* NOTREACHED */
  return;
}

/* Set a user's title for the channel */
void
do_chan_title(player, name, title)
    dbref player;
    const char *name;
    const char *title;
{
  CHAN *c = NULL;
  CHANUSER *u;

  if (!name || !*name) {
    notify(player, "You must specify a channel.");
    return;
  }
  if (strlen(title) >= CU_TITLE_LEN) {
    notify(player, "Title too long.");
    return;
  }
  test_channel(player, name, c);
  u = onchannel(player, c);
  if (!u) {
    notify(player, tprintf("You are not on channel <%s>.", ChanName(c)));
    return;
  }
  strcpy(CUtitle(u), title);
  if (!Quiet(player))
    notify(player, "Title set.");
  return;
}

/* List all the channels and their flags */
void
do_channel_list(player, partname)
    dbref player;
    const char *partname;
{
  CHAN *c;
  CHANUSER *u;
  char numusers[BUFFER_LEN];

  if (SUPPORT_PUEBLO)
    notify_noenter(player, tprintf("%cPRE%c", TAG_START, TAG_END));
  notify(player, tprintf("%-*s %-5s %8s %-15s %-8s",
			 CHAN_NAME_LEN,
			 "Name", "Users", "Msgs", "Chan Type", "Status"));
  for (c = channels; c; c = c->next) {
    if (Chan_Can_See(c, player) && string_prefix(ChanName(c), partname)) {
      u = onchannel(player, c);
      if (SUPPORT_PUEBLO)
	sprintf(numusers, "%cA XCH_CMD=\"@channel/who %s\" XCH_HINT=\"See who's on this channel now\"%c%5ld%c/A%c", TAG_START, ChanName(c), TAG_END, ChanNumUsers(c), TAG_START, TAG_END);
      else
	sprintf(numusers, "%5ld", ChanNumUsers(c));
      notify(player,
	  tprintf("%-30s %s %8ld [%c%c%c%c%c%c %c%c%c%c%c%c] [%-3s %c%c]",
		  ChanName(c),
		  numusers,
		  ChanNumMsgs(c),
		  Channel_Disabled(c) ? 'D' : '-',
		  Channel_Player(c) ? 'P' : '-',
		  Channel_Object(c) ? 'O' : '-',
		  Channel_Admin(c) ? 'A' :
		  (Channel_Wizard(c) ? 'W' : '-'),
		  Channel_Quiet(c) ? 'Q' : '-',
		  Channel_CanHide(c) ? 'H' : '-',
      /* Locks */
		  ChanJoinLock(c) != TRUE_BOOLEXP ? 'j' : '-',
		  ChanSpeakLock(c) != TRUE_BOOLEXP ? 's' : '-',
		  ChanModLock(c) != TRUE_BOOLEXP ? 'm' : '-',
		  ChanSeeLock(c) != TRUE_BOOLEXP ? 'v' : '-',
		  ChanHideLock(c) != TRUE_BOOLEXP ? 'h' : '-',
      /* Does the player own it? */
		  ChanCreator(c) == player ? '*' : '-',
      /* User status */
		  u ? "On" : "Off",
		   (u &&Chanuser_Quiet(u)) ? 'Q' : ' ',
		   (u &&Chanuser_Hide(u)) ? 'H' : ' '
	     ));
    }
  }
  if (SUPPORT_PUEBLO)
    notify_noenter(player, tprintf("%c/PRE%c", TAG_START, TAG_END));
}

/* Remove all players from a channel, notifying them. This is the 
 * utility routine for handling it. The command @channel/wipe
 * calls do_chan_wipe, below
 */
static void
channel_wipe(player, chan)
    dbref player;
    CHAN *chan;
{
  CHANUSER *u, *nextu;
  dbref victim;
  /* This is easy. Just call remove_user on each user in the list */
  if (!chan)
    return;
  for (u = ChanUsers(chan); u; u = nextu) {
    nextu = u->next;
    victim = CUdbref(u);
    if (remove_user(u, chan))
       notify(victim, tprintf("CHAT: %s has removed all users from <%s>.",
			      Name(player), ChanName(chan)));
  }
  ChanNumUsers(chan) = 0;
  return;
}

/* This is the user function for wiping a channel */
void
do_chan_wipe(player, name)
    dbref player;
    const char *name;
{
  CHAN *c;
  /* Find the channel */
  test_channel(player, name, c);
  /* Check permissions */
  if (!Chan_Can_Modify(c, player)) {
    notify(player, "CHAT: Wipe that silly grin off your face instead.");
    return;
  }
  /* Wipe it */
  channel_wipe(player, c);
  notify(player, tprintf("CHAT: Channel <%s> wiped.", ChanName(c)));
  return;
}

/* Changing the owner of a channel */
void
do_chan_chown(player, name, newowner)
    dbref player;
    const char *name;
    const char *newowner;
{
  CHAN *c;
  dbref victim;
  /* Only a Wizard can do this */
  if (!Wizard(player)) {
    notify(player, "CHAT: Only a Wizard can do that.");
    return;
  }
  /* Find the channel */
  test_channel(player, name, c);
  /* Find the victim */
  if ((victim = lookup_player(newowner)) == NOTHING) {
    notify(player, "CHAT: Invalid owner.");
    return;
  }
  /* We refund the original owner's money, but don't charge the
   * new owner. 
   */
  chan_chown(c, victim);
  notify(player, tprintf("CHAT: Channel <%s> now owned by %s.", ChanName(c),
			 Name(ChanCreator(c))));
  return;
}

/* Chown all of a player's channels. Usually done before destruction */
void
chan_chownall(old, newowner)
    dbref old;
    dbref newowner;
{
  CHAN *c;

  /* Run the channel list. If a channel is owned by old, chown it
     silently to newowner */
  for (c = channels; c; c = c->next) {
    if (ChanCreator(c) == old)
      chan_chown(c, newowner);
  }
}

/* The actual chowning of a channel */
static void
chan_chown(c, victim)
    CHAN *c;
    dbref victim;
{
  giveto(ChanCreator(c), ChanCost(c));
  ChanCreator(c) = victim;
  ChanCost(c) = 0;
  return;
}

/* Lock one of the channel's locks */
void
do_chan_lock(player, name, lockstr, whichlock)
    dbref player;
    const char *name;
    const char *lockstr;
    int whichlock;
{
  CHAN *c;
  struct boolexp *key;

  /* Make sure the channel exists */
  test_channel(player, name, c);
  /* Make sure the player has permission */
  if (!Chan_Can_Modify(c, player)) {
    notify(player, tprintf("CHAT: Channel <%s> resists.", ChanName(c)));
    return;
  }
  /* Ok, let's do it */
  if (!lockstr || !*lockstr) {
    /* Unlock it */
    key = TRUE_BOOLEXP;
  } else {
    key = parse_boolexp(player, lockstr);
    if (key == TRUE_BOOLEXP) {
      notify(player, "CHAT: I don't understand that key.");
      return;
    }
  }
  switch (whichlock) {
  case CL_JOIN:
    if (ChanJoinLock(c))
      free(ChanJoinLock(c));
    ChanJoinLock(c) = key;
    notify(player, tprintf("CHAT: Joinlock on <%s> %s.",
			   ChanName(c),
			   (key == TRUE_BOOLEXP) ? "reset" : "set"));
    break;
  case CL_SPEAK:
    if (ChanSpeakLock(c))
      free(ChanSpeakLock(c));
    ChanSpeakLock(c) = key;
    notify(player, tprintf("CHAT: Speaklock on <%s> %s.",
			   ChanName(c),
			   (key == TRUE_BOOLEXP) ? "reset" : "set"));
    break;
  case CL_SEE:
    if (ChanSeeLock(c))
      free(ChanSeeLock(c));
    ChanSeeLock(c) = key;
    notify(player, tprintf("CHAT: Seelock on <%s> %s.",
			   ChanName(c),
			   (key == TRUE_BOOLEXP) ? "reset" : "set"));
    break;
  case CL_HIDE:
    if (ChanHideLock(c))
      free(ChanHideLock(c));
    ChanHideLock(c) = key;
    notify(player, tprintf("CHAT: Hidelock on <%s> %s.",
			   ChanName(c),
			   (key == TRUE_BOOLEXP) ? "reset" : "set"));
    break;
  case CL_MOD:
    if (ChanModLock(c))
      free(ChanModLock(c));
    ChanModLock(c) = key;
    notify(player, tprintf("CHAT: Modlock on <%s> %s.",
			   ChanName(c),
			   (key == TRUE_BOOLEXP) ? "reset" : "set"));
    break;
  }
  return;
}


/* A channel list with names and descriptions only. */
void
do_chan_what(player, partname)
    dbref player;
    const char *partname;
{
  CHAN *c;
  int found = 0;
  if (ShowAnsi(player)) {
    for (c = channels; c; c = c->next) {
      if (Chan_Can_See(c, player) && string_prefix(ChanName(c), partname)) {
	notify(player, tprintf("%s<%s>%s",
			       ANSI_HILITE, ChanName(c), ANSI_NORMAL));
	notify(player, tprintf("Creator: %s", Name(ChanCreator(c))));
	notify(player, privs_to_string(priv_table, ChanType(c)));
	notify(player, ChanTitle(c));
	found++;
      }
    }
  } else {
    for (c = channels; c; c = c->next) {
      if (Chan_Can_See(c, player) && string_prefix(ChanName(c), partname)) {
	notify(player, tprintf("<%s>", ChanName(c)));
	notify(player, tprintf("Creator: %s", Name(ChanCreator(c))));
	notify(player, privs_to_string(priv_table, ChanType(c)));
	notify(player, ChanTitle(c));
	found++;
      }
    }
  }
  if (!found)
    notify(player, "CHAT: I don't recognize that channel.");
}


/* A decompilation which one could recreate a channel with */
void
do_chan_decompile(player, name)
    dbref player;
    const char *name;
{
  CHAN *c;
  CHANUSER *u;
  int found;

  found = 0;
  for (c = channels; c; c = c->next) {
    if (string_prefix(ChanName(c), name)) {
      found++;
      if (!(Chan_Can_Modify(c, player) || (ChanCreator(c) == player))) {
	notify(player, tprintf("CHAT: No permission to decompile <%s>", ChanName(c)));
	continue;
      }
      notify(player, tprintf("@channel/add %s = %s", ChanName(c), privs_to_string(priv_table, ChanType(c))));
      notify(player, tprintf("@channel/chown %s = %s", ChanName(c), Name(ChanCreator(c))));
      if (ChanModLock(c) != TRUE_BOOLEXP)
	notify(player, tprintf("@clock/mod %s = %s", ChanName(c), unparse_boolexp(player, ChanModLock(c), 1)));
      if (ChanHideLock(c) != TRUE_BOOLEXP)
	notify(player, tprintf("@clock/hide %s = %s", ChanName(c), unparse_boolexp(player, ChanHideLock(c), 1)));
      if (ChanJoinLock(c) != TRUE_BOOLEXP)
	notify(player, tprintf("@clock/join %s = %s", ChanName(c), unparse_boolexp(player, ChanJoinLock(c), 1)));
      if (ChanSpeakLock(c) != TRUE_BOOLEXP)
	notify(player, tprintf("@clock/speak %s = %s", ChanName(c), unparse_boolexp(player, ChanSpeakLock(c), 1)));
      if (ChanSeeLock(c) != TRUE_BOOLEXP)
	notify(player, tprintf("@clock/see %s = %s", ChanName(c), unparse_boolexp(player, ChanSeeLock(c), 1)));

      for (u = ChanUsers(c); u; u = u->next) {
	if (!Chanuser_Hide(u) || Priv_Who(player))
	   notify(player, tprintf("@channel/on %s = %s", ChanName(c), Name(CUdbref(u))));
      }
    }
  }
  if (!found)
    notify(player, "CHAT: No channel matches that string.");
}


/* Modify a channel's description */
void
do_chan_desc(player, name, title)
    dbref player;
    const char *name;
    const char *title;
{
  CHAN *c;

  /* Check new title length */
  if (title && strlen(title) > CHAN_TITLE_LEN - 1) {
    notify(player, "CHAT: New title too long.");
    return;
  }
  /* Make sure the channel exists */
  test_channel(player, name, c);
  /* Make sure the player has permission */
  if (!Chan_Can_Modify(c, player)) {
    notify(player, "CHAT: Yeah, right.");
    return;
  }
  /* Ok, let's do it */
  if (!title || !*title) {
    strcpy(ChanTitle(c), "");
    notify(player, tprintf("CHAT: Channel <%s> title cleared.", ChanName(c)));
  } else {
    strcpy(ChanTitle(c), title);
    notify(player, tprintf("CHAT: Channel <%s> title set.", ChanName(c)));
  }
}



static int
yesno(str)
    const char *str;
{
  if (!str || !*str)
    return ERR;
  switch (str[0]) {
  case 'y':
  case 'Y':
    return YES;
  case 'n':
  case 'N':
    return NO;
  case 'o':
  case 'O':
    switch (str[1]) {
    case 'n':
    case 'N':
      return YES;
    case 'f':
    case 'F':
      return NO;
    default:
      return ERR;
    }
  default:
    return ERR;
  }
}

/* Can this player still add channels, or have they created their
 * limit already?
 */
static int
canstilladd(player)
    dbref player;
{
  CHAN *c;
  int num = 0;
  for (c = channels; c; c = c->next) {
    if (ChanCreator(c) == player)
      num++;
  }
  return (num < MAX_PLAYER_CHANS);
}

#endif				/* CHAT_SYSTEM */
