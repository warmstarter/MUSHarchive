/**
 * \file extchat.c
 *
 * \brief The PennMUSH chat system
 *
 *
 */
#include "copyrite.h"
#include "config.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#include <setjmp.h>
#include <stdarg.h>
#include "conf.h"
#include "attrib.h"
#include "mushdb.h"
#include "match.h"
#include "externs.h"
#include "extchat.h"
#include "ansi.h"
#include "privtab.h"
#include "mymalloc.h"
#include "pueblo.h"
#include "parse.h"
#include "lock.h"
#include "log.h"
#include "flags.h"
#include "dbdefs.h"
#include "function.h"
#include "command.h"
#include "confmagic.h"

#ifdef CHAT_SYSTEM

extern CHAN *channels;
struct na_cpass {
  CHANUSER *u;
  int checkquiet;
};

extern jmp_buf db_err;
#define OUTPUT(fun) do { if ((fun) < 0) longjmp(db_err, 1); } while (0)

static CHAN *new_channel(void);
static CHANLIST *new_chanlist(void);
static CHANUSER *new_user(dbref who);
static void free_channel(CHAN *c);
static void free_chanlist(CHANLIST *cl);
static void free_user(CHANUSER *u);
static int load_channel(FILE * fp, CHAN *ch);
static int load_chanusers(FILE * fp, CHAN *ch);
static void insert_channel(CHAN **ch);
static void remove_channel(CHAN *ch);
static void insert_obj_chan(dbref who, CHAN **ch);
static void remove_obj_chan(dbref who, CHAN *ch);
void remove_all_obj_chan(dbref thing);
static void chan_chown(CHAN *c, dbref victim);
void chan_chownall(dbref old, dbref new);
static int insert_user(CHANUSER *user, CHAN *ch);
static int remove_user(CHANUSER *u, CHAN *ch);
static int save_channel(FILE * fp, CHAN *ch);
static int save_chanuser(FILE * fp, CHANUSER *user);
static void channel_wipe(dbref player, CHAN *chan);
static int yesno(const char *str);
static int canstilladd(dbref player);
static int find_channel_partial_on(const char *name, CHAN **chan, dbref player);
static int find_channel_partial_off
  (const char *name, CHAN **chan, dbref player);
static char *list_cflags(CHAN *c);
static char *list_cuflags(CHANUSER *u);
static void channel_join_self(dbref player, const char *name);
static void channel_leave_self(dbref player, const char *name);
static void do_channel_who(dbref player, CHAN *chan);
void chat_player_announce(dbref player, char *msg);
static int ok_channel_name(const char *n);
static void channel_allocate_buffer(CHAN *c, int lines);
static void channel_reallocate_buffer(CHAN *c, int lines);
static void channel_free_buffer(CHAN *c);
static void channel_push_buffer(CHAN *c, char *tbuf1);
static void channel_shift_buffer(CHAN *c, int space_needed);
static int channel_buffer_lines(CHAN *c);

const char *chan_speak_lock = "ChanSpeakLock";
const char *chan_join_lock = "ChanJoinLock";
const char *chan_mod_lock = "ChanModLock";
const char *chan_see_lock = "ChanSeeLock";
const char *chan_hide_lock = "ChanHideLock";

#define YES 1
#define NO 0
#define ERR -1

#define insert_user_by_dbref(who,chan) \
        insert_user(new_user(who),chan)
#define remove_user_by_dbref(who,chan) \
        remove_user(onchannel(who,chan),chan)

int num_channels;

CHAN *channels;

static PRIV priv_table[] = {
  {"Disabled", 'D', CHANNEL_DISABLED, CHANNEL_DISABLED},
  {"Admin", 'A', CHANNEL_ADMIN | CHANNEL_PLAYER, CHANNEL_ADMIN},
  {"Wizard", 'W', CHANNEL_WIZARD | CHANNEL_PLAYER, CHANNEL_WIZARD},
  {"Player", 'P', CHANNEL_PLAYER, CHANNEL_PLAYER},
  {"Object", 'O', CHANNEL_OBJECT, CHANNEL_OBJECT},
  {"Quiet", 'Q', CHANNEL_QUIET, CHANNEL_QUIET},
  {"Open", 'o', CHANNEL_OPEN, CHANNEL_OPEN},
  {"Hide_Ok", 'H', CHANNEL_CANHIDE, CHANNEL_CANHIDE},
  {NULL, '\0', 0, 0}
};


/** Get a player's CHANUSER entry if they're on a channel.
 * This function checks to see if a given player is on a given channel.
 * If so, it returns a pointer to their CHANUSER structure. If not,
 * returns NULL.
 * \param who player to test channel membership of.
 * \param ch pointer to channel to test membership on.
 * \return player's CHANUSER entry on the channel, or NULL.
 */
CHANUSER *
onchannel(dbref who, CHAN *ch)
{
  static CHANUSER *u;
  for (u = ChanUsers(ch); u; u = u->next) {
    if (CUdbref(u) == who) {
      return u;
    }
  }
  return NULL;
}

/** A macro to test if a channel exists and, if not, to notify. */
#define test_channel(player,name,chan) \
   do { \
    chan = NULL; \
    switch (find_channel(name,&chan,player)) { \
    case CMATCH_NONE: \
      notify(player, T ("CHAT: I don't recognize that channel.")); \
      return; \
    case CMATCH_AMBIG: \
      notify(player, T("CHAT: I don't know which channel you mean.")); \
      return; \
     } \
    } while (0)

/*----------------------------------------------------------
 * Loading and saving the chatdb
 * The chatdb's format is pretty straightforward
 * Return 1 on success, 0 on failure
 */

/** Initialize the chat database .*/
void
init_chatdb(void)
{
  num_channels = 0;
  channels = NULL;
}

/** Load the chat database from a file.
 * \param fp pointer to file to read from.
 * \retval 1 success
 * \retval 0 failure
 */
int
load_chatdb(FILE * fp)
{
  int i;
  CHAN *ch;
  char buff[20];

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
      return 0;
    if (!load_channel(fp, ch)) {
      do_rawlog(LT_ERR, T("Unable to load channel %d."), i);
      free_channel(ch);
      return 0;
    }
    insert_channel(&ch);
  }
  num_channels = i;

  /* Check for **END OF DUMP*** */
  fgets(buff, sizeof buff, fp);
  if (!buff)
    do_rawlog(LT_ERR, T("CHAT: No end-of-dump marker in the chat database."));
  else if (strcmp(buff, EOD) != 0)
    do_rawlog(LT_ERR, T("CHAT: Trailing garbage in the chat database."));

  return 1;
}

/* Malloc memory for a new channel, and initialize it */
static CHAN *
new_channel(void)
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
  ChanBuffer(ch) = NULL;
  ChanBufferSize(ch) = 0;
  return ch;
}


/* Malloc memory for a new user, and initialize it */
static CHANUSER *
new_user(dbref who)
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
free_channel(CHAN *c)
{
  CHANUSER *u, *unext;
  if (!c)
    return;
  free_boolexp(ChanJoinLock(c));
  free_boolexp(ChanSpeakLock(c));
  free_boolexp(ChanHideLock(c));
  free_boolexp(ChanSeeLock(c));
  free_boolexp(ChanModLock(c));
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
free_user(CHANUSER *u)
{
  if (u)
    mush_free(u, "CHANUSER");
}

/* Load in a single channel into position i. Return 1 if
 * successful, 0 otherwise.
 */
static int
load_channel(FILE * fp, CHAN *ch)
{
  strcpy(ChanName(ch), getstring_noalloc(fp));
  if (feof(fp))
    return 0;
  strcpy(ChanTitle(ch), getstring_noalloc(fp));
  ChanType(ch) = getref(fp);
  ChanCreator(ch) = getref(fp);
  ChanCost(ch) = getref(fp);
  ChanNumMsgs(ch) = 0;
  ChanJoinLock(ch) = getboolexp(fp, chan_join_lock);
  ChanSpeakLock(ch) = getboolexp(fp, chan_speak_lock);
  ChanModLock(ch) = getboolexp(fp, chan_mod_lock);
  ChanSeeLock(ch) = getboolexp(fp, chan_see_lock);
  ChanHideLock(ch) = getboolexp(fp, chan_hide_lock);
  ChanNumUsers(ch) = getref(fp);
  ChanMaxUsers(ch) = ChanNumUsers(ch);
  ChanUsers(ch) = NULL;
  if (ChanNumUsers(ch) > 0)
    return (ChanNumUsers(ch) = load_chanusers(fp, ch));
  return 1;
}

/* Load the *channel's user list. Return number of users on success, or 0 */
static int
load_chanusers(FILE * fp, CHAN *ch)
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
      do_log(LT_ERR, 0, 0, T("Bad object #%d removed from channel %s"),
	     player, ChanName(ch));
      (void) getref(fp);
      (void) getstring_noalloc(fp);
    }
  }
  return num;
}


/* Insert the channel onto the list of channels, sorted by name */
static void
insert_channel(CHAN **ch)
{
  CHAN *p;
  char cleanname[CHAN_NAME_LEN];
  char cleanp[CHAN_NAME_LEN];

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
  strcpy(cleanp, remove_markup(ChanName(p), NULL));
  strcpy(cleanname, remove_markup(ChanName(*ch), NULL));
  if (strcasecoll(cleanp, cleanname) > 0) {
    channels = *ch;
    channels->next = p;
    return;
  }
  /* Otherwise, find which user this user should be inserted after */
  while (p->next) {
    strcpy(cleanp, remove_markup(ChanName(p->next), NULL));
    if (strcasecoll(cleanp, cleanname) > 0)
      break;
    p = p->next;
  }
  (*ch)->next = p->next;
  p->next = *ch;
  return;
}

/* Remove a channel from the list, but don't free it */
static void
remove_channel(CHAN *ch)
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
  for (p = channels; p->next && (p->next != ch); p = p->next) ;

  if (p->next) {
    p->next = ch->next;
  }
  return;
}

/* Insert the channel onto the list of channels on a given object,
 * sorted by name
 */
static void
insert_obj_chan(dbref who, CHAN **ch)
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
   * alphabetically greater, chan should be the first entry on the list */
  /* No channels? */
  if (!Chanlist(who)) {
    tmp->next = NULL;
    s_Chanlist(who, tmp);
    return;
  }
  p = Chanlist(who);
  /* First channel? */
  if (strcasecoll(ChanName(p->chan), ChanName(*ch)) > 0) {
    tmp->next = p;
    s_Chanlist(who, tmp);
    return;
  } else if (!strcasecmp(ChanName(p->chan), ChanName(*ch))) {
    /* Don't add the same channel twice! */
    free_chanlist(tmp);
  } else {
    /* Otherwise, find which channel this channel should be inserted after */
    for (;
	 p->next
	 && (strcasecoll(ChanName(p->next->chan), ChanName(*ch)) < 0);
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
remove_obj_chan(dbref who, CHAN *ch)
{
  CHANLIST *p, *q;

  if (!ch)
    return;
  p = Chanlist(who);
  if (!p)
    return;
  if (p->chan == ch) {
    /* First channel */
    s_Chanlist(who, p->next);
    free_chanlist(p);
    return;
  }
  /* Otherwise, find the channel before this one */
  for (; p->next && (p->next->chan != ch); p = p->next) ;

  if (p->next) {
    q = p->next;
    p->next = p->next->next;
    free_chanlist(q);
  }
  return;
}

/** Remove all channels from the obj's chanlist, freeing them.
 * \param thing object to have channels removed from.
 */
void
remove_all_obj_chan(dbref thing)
{
  CHANLIST *p, *nextp;
  for (p = Chanlist(thing); p; p = nextp) {
    nextp = p->next;
    remove_user_by_dbref(thing, p->chan);
  }
  return;
}


static CHANLIST *
new_chanlist(void)
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
free_chanlist(CHANLIST *cl)
{
  mush_free(cl, "CHANLIST");
}


/* Insert the user onto the channel's list, sorted by the user's name */
static int
insert_user(CHANUSER *user, CHAN *ch)
{
  CHANUSER *p;

  if (!user || !ch)
    return 0;

  /* If there's no users on the list, or if the first user is already
   * alphabetically greater, user should be the first entry on the list */
  p = ChanUsers(ch);
  if (!p || (strcasecoll(Name(CUdbref(p)), Name(CUdbref(user))) > 0)) {
    user->next = ChanUsers(ch);
    ChanUsers(ch) = user;
  } else {
    /* Otherwise, find which user this user should be inserted after */
    for (;
	 p->next
	 && (strcasecoll(Name(CUdbref(p->next)), Name(CUdbref(user))) <= 0);
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
remove_user(CHANUSER *u, CHAN *ch)
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
    for (; p->next && (p->next != u); p = p->next) ;

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


/** Write the chat database to disk.
 * \param fp pointer to file to write to.
 * \retval 1 success
 * \retval 0 failure
 */
int
save_chatdb(FILE * fp)
{
  CHAN *ch;

  /* How many channels? */
  putref(fp, num_channels);

  for (ch = channels; ch; ch = ch->next) {
    save_channel(fp, ch);
  }
  OUTPUT(fputs(EOD, fp));
  return 1;
}

/* Save a single channel. Return 1 if  successful, 0 otherwise.
 */
static int
save_channel(FILE * fp, CHAN *ch)
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
save_chanuser(FILE * fp, CHANUSER *user)
{
  putref(fp, CUdbref(user));
  putref(fp, CUtype(user));
  putstring(fp, CUtitle(user));
  return 1;
}

/*-------------------------------------------------------------*
 * Some utility functions:
 *  find_channel - given a name and a player, return a channel
 *  find_channel_partial - given a name and a player, return
 *    the first channel that matches name 
 *  find_channel_partial_on - given a name and a player, return
 *    the first channel that matches name that player is on.
 *  onchannel - is player on channel?
 */

/** Attempt to match a channel name for a player.
 * Given name and a chan pointer, set chan pointer to point to
 * channel if found (NULL otherwise), and return an indication
 * of how good the match was. If the player is not able to see
 * the channel, fail to match.
 * \param name name of channel to find.
 * \param chan pointer to address of channel structure to return.
 * \param player dbref to use for permission checks.
 * \retval CMATCH_EXACT exact match of channel name.
 * \retval CMATCH_PARTIAL partial match of channel name.
 * \retval CMATCH_AMBIG multiple match of channel name.
 * \retval CMATCH_NONE no match for channel name.
 */
int
find_channel(const char *name, CHAN **chan, dbref player)
{
  CHAN *p;
  int count = 0;
  char cleanname[BUFFER_LEN];
  char cleanp[CHAN_NAME_LEN];

  *chan = NULL;
  if (!name || !*name)
    return CMATCH_NONE;
  strcpy(cleanname, remove_markup(name, NULL));
  for (p = channels; p; p = p->next) {
    strcpy(cleanp, remove_markup(ChanName(p), NULL));
    if (!strcasecmp(cleanname, cleanp)) {
      *chan = p;
      if (Chan_Can_See(*chan, player) || onchannel(player, *chan))
	return CMATCH_EXACT;
      else
	return CMATCH_NONE;
    }
    if (string_prefix(cleanp, name)) {
      /* Keep the alphabetically first channel if we've got one */
      if (Chan_Can_See(p, player) || onchannel(player, p)) {
	if (!*chan)
	  *chan = p;
	count++;
      }
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


/** Attempt to match a channel name for a player.
 * Given name and a chan pointer, set chan pointer to point to
 * channel if found (NULL otherwise), and return an indication
 * of how good the match was. If the player is not able to see
 * the channel, fail to match. If the match is ambiguous, return
 * the first channel matched.
 * \param name name of channel to find.
 * \param chan pointer to address of channel structure to return.
 * \param player dbref to use for permission checks.
 * \retval CMATCH_EXACT exact match of channel name.
 * \retval CMATCH_PARTIAL partial match of channel name.
 * \retval CMATCH_AMBIG multiple match of channel name.
 * \retval CMATCH_NONE no match for channel name.
 */
int
find_channel_partial(const char *name, CHAN **chan, dbref player)
{
  CHAN *p;
  int count = 0;
  char cleanname[BUFFER_LEN];
  char cleanp[CHAN_NAME_LEN];

  *chan = NULL;
  if (!name || !*name)
    return CMATCH_NONE;
  strcpy(cleanname, remove_markup(name, NULL));
  for (p = channels; p; p = p->next) {
    strcpy(cleanp, remove_markup(ChanName(p), NULL));
    if (!strcasecmp(cleanname, cleanp)) {
      *chan = p;
      return CMATCH_EXACT;
    }
    if (string_prefix(cleanp, cleanname)) {
      /* If we've already found an ambiguous match that the
       * player is on, keep using that one. Otherwise, this is
       * our best candidate so far.
       */
      if (!*chan || (!onchannel(player, *chan) && onchannel(player, p)))
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


/** Attempt to match a channel name for a player.
 * Given name and a chan pointer, set chan pointer to point to
 * channel if found and player is on the channel (NULL otherwise), 
 * and return an indication of how good the match was. If the player is 
 * not able to see the channel, fail to match. If the match is ambiguous, 
 * return the first channel matched.
 * \param name name of channel to find.
 * \param chan pointer to address of channel structure to return.
 * \param player dbref to use for permission checks.
 * \retval CMATCH_EXACT exact match of channel name.
 * \retval CMATCH_PARTIAL partial match of channel name.
 * \retval CMATCH_AMBIG multiple match of channel name.
 * \retval CMATCH_NONE no match for channel name.
 */
static int
find_channel_partial_on(const char *name, CHAN **chan, dbref player)
{
  CHAN *p;
  int count = 0;
  char cleanname[BUFFER_LEN];
  char cleanp[CHAN_NAME_LEN];

  *chan = NULL;
  if (!name || !*name)
    return CMATCH_NONE;
  strcpy(cleanname, remove_markup(name, NULL));
  for (p = channels; p; p = p->next) {
    if (onchannel(player, p)) {
      strcpy(cleanp, remove_markup(ChanName(p), NULL));
      if (!strcasecmp(cleanname, cleanp)) {
	*chan = p;
	return CMATCH_EXACT;
      }
      if (string_prefix(cleanp, cleanname) && onchannel(player, p)) {
	if (!*chan)
	  *chan = p;
	count++;
      }
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

/** Attempt to match a channel name for a player.
 * Given name and a chan pointer, set chan pointer to point to
 * channel if found and player is NOT on the channel (NULL otherwise), 
 * and return an indication of how good the match was. If the player is 
 * not able to see the channel, fail to match. If the match is ambiguous, 
 * return the first channel matched.
 * \param name name of channel to find.
 * \param chan pointer to address of channel structure to return.
 * \param player dbref to use for permission checks.
 * \retval CMATCH_EXACT exact match of channel name.
 * \retval CMATCH_PARTIAL partial match of channel name.
 * \retval CMATCH_AMBIG multiple match of channel name.
 * \retval CMATCH_NONE no match for channel name.
 */
static int
find_channel_partial_off(const char *name, CHAN **chan, dbref player)
{
  CHAN *p;
  int count = 0;
  char cleanname[BUFFER_LEN];
  char cleanp[CHAN_NAME_LEN];

  *chan = NULL;
  if (!name || !*name)
    return CMATCH_NONE;
  strcpy(cleanname, remove_markup(name, NULL));
  for (p = channels; p; p = p->next) {
    if (!onchannel(player, p)) {
      strcpy(cleanp, remove_markup(ChanName(p), NULL));
      if (!strcasecmp(cleanname, cleanp)) {
	*chan = p;
	return CMATCH_EXACT;
      }
      if (string_prefix(cleanp, cleanname)) {
	if (!*chan)
	  *chan = p;
	count++;
      }
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

/** User interface to channels.
 * \verbatim
 * This is one of the top-level functions for @channel.
 * It handles the /on, /off and /who switches. It also
 * parses and handles the older @channel <channel>=<command>
 * format, for the on, off, who, and wipe commands.
 * \endverbatim
 * \param player the enactor.
 * \param name name of channel.
 * \param target name of player to add/remove (NULL for self)
 * \param com channel command switch.
 */
void
do_channel(dbref player, const char *name, const char *target, const char *com)
{
  CHAN *chan = NULL;
  dbref victim;

  if (!name && !*name) {
    notify(player, T("You need to specify a channel."));
    return;
  }
  if (!com && !*com) {
    notify(player, T("What do you want to do with that channel?"));
    return;
  }

  /* Quickly catch two common cases and handle separately */
  if (!target || !*target) {
    if (!strcasecmp(com, "on") || !strcasecmp(com, "join")) {
      channel_join_self(player, name);
      return;
    } else if (!strcasecmp(com, "off") || !strcasecmp(com, "leave")) {
      channel_leave_self(player, name);
      return;
    }
  }

  test_channel(player, name, chan);
  if (!Chan_Can_See(chan, player)) {
    if (onchannel(player, chan))
      notify_format(player,
		    T("CHAT: You can't do that with channel <%s>."),
		    ChanName(chan));
    else
      notify(player, T("CHAT: I don't recognize that channel."));
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
   * an argument, we return, because we should've caught those above,
   * and this shouldn't happen.
   */
  if (!target || !*target) {
    notify(player, T("I don't understand what you want to do."));
    return;
  } else if ((victim = lookup_player(target)) != NOTHING) ;
  else if (Channel_Object(chan))
    victim = match_result(player, target, TYPE_THING, MAT_OBJECTS);
  else
    victim = NOTHING;
  if (!GoodObject(victim)) {
    notify(player, T("Invalid target."));
    return;
  }
  if (!strcasecmp("on", com) || !strcasecmp("join", com)) {
    if (!Chan_Ok_Type(chan, victim)) {
      notify_format(player,
		    T("Sorry, wrong type of thing for channel <%s>."),
		    ChanName(chan));
      return;
    }
    if (Guest(player)) {
      notify(player, T("Guests are not allowed to join channels."));
      return;
    }
    if (!controls(player, victim)) {
      notify(player, T("Invalid target."));
      return;
    }
    /* Is victim already on the channel? */
    if (onchannel(victim, chan)) {
      notify_format(player,
		    T("%s is already on channel <%s>."), Name(victim),
		    ChanName(chan));
      return;
    }
    /* Does victim pass the joinlock? */
    if (!Chan_Can_Join(chan, victim)) {
      if (Wizard(player)) {
	/* Wizards can override join locks */
	notify(player,
	       T
	       ("CHAT: Warning: Target does not meet channel join permissions (joining anyway)"));
      } else {
	notify(player, T("Permission to join denied."));
	return;
      }
    }
    if (insert_user_by_dbref(victim, chan)) {
      notify_format(victim,
		    T("CHAT: %s joins you to channel <%s>."), Name(player),
		    ChanName(chan));
      notify_format(player,
		    T("CHAT: You join %s to channel <%s>."), Name(victim),
		    ChanName(chan));
      if (!Channel_Quiet(chan) && !DarkLegal(victim))
	channel_broadcast(chan, victim, 1,
			  T("<%s> %s has joined this channel."), ChanName(chan),
			  Name(victim));
      ChanNumUsers(chan)++;
    } else {
      notify_format(player,
		    T("%s is already on channel <%s>."), Name(victim),
		    ChanName(chan));
    }
    return;
  } else if (!strcasecmp("off", com) || !strcasecmp("leave", com)) {
    /* You must control either the victim or the channel */
    if (!controls(player, victim) && !Chan_Can_Modify(chan, player)) {
      notify(player, T("Invalid target."));
      return;
    }
    if (Guest(player)) {
      notify(player, T("Guests may not leave channels."));
      return;
    }
    if (remove_user_by_dbref(victim, chan)) {
      if (!Channel_Quiet(chan) && !DarkLegal(victim))
	channel_broadcast(chan, victim, 1,
			  T("<%s> %s has left this channel."), ChanName(chan),
			  Name(victim));
      notify_format(victim,
		    T("CHAT: %s removes you from channel <%s>."),
		    Name(player), ChanName(chan));
      notify_format(player,
		    T("CHAT: You remove %s from channel <%s>."),
		    Name(victim), ChanName(chan));
    } else {
      notify_format(player, T("%s is not on channel <%s>."), Name(victim),
		    ChanName(chan));
    }
    return;
  } else {
    notify(player, T("I don't understand what you want to do."));
    return;
  }
}

static void
channel_join_self(dbref player, const char *name)
{
  CHAN *chan = NULL;
  if (Guest(player)) {
    notify(player, T("Guests are not allowed to join channels."));
    return;
  }


  switch (find_channel_partial_off(name, &chan, player)) {
  case CMATCH_NONE:
    if (find_channel_partial_on(name, &chan, player))
      notify_format(player, T("CHAT: You are already on channel <%s>"),
		    ChanName(chan));
    else
      notify(player, T("CHAT: I don't recognize that channel."));
    return;
  case CMATCH_AMBIG:
    notify(player, T("CHAT: I don't know which channel you mean."));
    return;
  }
  if (!Chan_Can_See(chan, player)) {
    notify(player, T("CHAT: I don't recognize that channel."));
    return;
  }
  if (!Chan_Ok_Type(chan, player)) {
    notify_format(player,
		  T("Sorry, wrong type of thing for channel <%s>."),
		  ChanName(chan));
    return;
  }
  /* Does victim pass the joinlock? */
  if (!Chan_Can_Join(chan, player)) {
    if (Wizard(player)) {
      /* Wizards can override join locks */
      notify(player,
	     T
	     ("CHAT: Warning: You don't meet channel join permissions (joining anyway)"));
    } else {
      notify(player, T("Permission to join denied."));
      return;
    }
  }
  if (insert_user_by_dbref(player, chan)) {
    notify_format(player, T("CHAT: You join channel <%s>."), ChanName(chan));
    if (!Channel_Quiet(chan) && !DarkLegal(player))
      channel_broadcast(chan, player, 1,
			T("<%s> %s has joined this channel."), ChanName(chan),
			Name(player));
    ChanNumUsers(chan)++;
  } else {
    /* Should never happen */
    notify_format(player,
		  T("%s is already on channel <%s>."), Name(player),
		  ChanName(chan));
  }
}

static void
channel_leave_self(dbref player, const char *name)
{
  CHAN *chan = NULL;
  if (Guest(player)) {
    notify(player, T("Guests are not allowed to leave channels."));
    return;
  }
  switch (find_channel_partial_on(name, &chan, player)) {
  case CMATCH_NONE:
    if (find_channel_partial_off(name, &chan, player)
	&& Chan_Can_See(chan, player))
      notify_format(player, T("CHAT: You are not on channel <%s>"),
		    ChanName(chan));
    else
      notify(player, T("CHAT: I don't recognize that channel."));
    return;
  case CMATCH_AMBIG:
    notify(player, T("CHAT: I don't know which channel you mean."));
    return;
  }
  if (remove_user_by_dbref(player, chan)) {
    if (!Channel_Quiet(chan) && !DarkLegal(player))
      channel_broadcast(chan, player, 1,
			T("<%s> %s has left this channel."), ChanName(chan),
			Name(player));
    notify_format(player, T("CHAT: You leave channel <%s>."), ChanName(chan));
  } else {
    /* Should never happen */
    notify_format(player, T("%s is not on channel <%s>."), Name(player),
		  ChanName(chan));
  }
}

/** Parse a chat token command.
 * This function hacks up something of the form "+<channel> <message>",
 * finding the two args, and passes it to do_chat
 * \param player the enactor.
 * \param command the command to parse.
 * \retval 1 chat was successful.
 * \retval 0 chat failed (no such channel, etc.)
 */
int
parse_chat(dbref player, char *command)
{
  char *arg1;
  char *arg2;
  char tbuf1[MAX_COMMAND_LEN];
  char *s;

  strncpy(tbuf1, command, MAX_COMMAND_LEN - 1);	/* don't hack it up */
  tbuf1[MAX_COMMAND_LEN - 1] = '\0';
  s = tbuf1;

  arg1 = s;
  while (*s && !isspace((unsigned char) *s))
    s++;

  if (*s) {
    *s++ = '\0';
    while (*s && isspace((unsigned char) *s))
      s++;
  }
  arg2 = s;

  return do_chat_by_name(player, arg1, arg2, 0);
}


/** Chat on a channel, given its name.
 * \verbatim
 * This function parses a channel name and then calls do_chat()
 * to do the actual work of chatting. If it was called through
 * @chat, it fails noisily, but if it was called through +<channel>,
 * it fails silently so that the command can be matched against $commands.
 * \endverbatim
 * \param player the enactor.
 * \param name name of channel to speak on.
 * \param msg message to send to channel.
 * \param source 0 if from +<channel>, 1 if from the chat command.
 * \retval 1 successful chat.
 * \retval 0 failed chat.
 */
int
do_chat_by_name(dbref player, const char *name, const char *msg, int source)
{
  CHAN *c;
  c = NULL;
  if (!msg || !*msg) {
    if (source)
      notify(player, T("Don't you have anything to say?"));
    return 0;
  }
  /* First try to find a channel that the player's on. If that fails,
   * look for one that the player's not on.
   */
  if (find_channel_partial_on(name, &c, player) == CMATCH_NONE) {
    if (find_channel(name, &c, player) == CMATCH_NONE) {
      if (source)
	notify(player, T("No such channel."));
      return 0;
    }
  }
  do_chat(player, c, msg);
  return 1;
}

/** Send a message to a channel.
 * This function does the real work of putting together a message
 * to send to a chat channel (which it then does via channel_broadcast()).
 * \param player the enactor.
 * \param chan pointer to the channel to speak on.
 * \param arg1 message to send.
 */
void
do_chat(dbref player, CHAN *chan, const char *arg1)
{
  CHANUSER *u;
  const char *gap;
  char *title;
  int canhear;

  if (!Chan_Ok_Type(chan, player)) {
    notify_format(player,
		  T("Sorry, you're not the right type to be on channel <%s>."),
		  ChanName(chan));
    return;
  }
  if (!Chan_Can_Speak(chan, player)) {
    if (Chan_Can_See(chan, player))
      notify_format(player,
		    T("Sorry, you're not allowed to speak on channel <%s>."),
		    ChanName(chan));
    else
      notify(player, T("No such channel."));
    return;
  }
  u = onchannel(player, chan);
  canhear = u ? !Chanuser_Gag(u) : 0;
  /* If the channel isn't open, you must hear it in order to speak */
  if (!Channel_Open(chan)) {
    if (!u) {
      notify(player, T("You must be on that channel to speak on it."));
      return;
    } else if (!canhear) {
      notify(player, T("You must stop gagging that channel to speak on it."));
      return;
    }
  }

  if (!*arg1) {
    notify(player, T("What do you want to say to that channel?"));
    return;
  }

  if (u &&CUtitle(u) && *CUtitle(u))
     title = CUtitle(u);
  else
    title = NULL;

  /* figure out what kind of message we have */
  gap = " ";
  switch (*arg1) {
  case SEMI_POSE_TOKEN:
    gap = "";
    /* FALLTHRU */
  case POSE_TOKEN:
    arg1 = arg1 + 1;
    channel_broadcast(chan, player, 0, "<%s> %s%s%s%s%s%s", ChanName(chan),
		      title ? title : "", title ? ANSI_NORMAL : "",
		      title ? " " : "", accented_name(player), gap, arg1);
    if (!canhear)
      notify_format(player, T("To channel %s: %s%s%s"), ChanName(chan),
		    accented_name(player), gap, arg1);
    break;
  default:
    if (CHAT_STRIP_QUOTE && (*arg1 == SAY_TOKEN))
      arg1 = arg1 + 1;
    channel_broadcast(chan, player, 0, T("<%s> %s%s%s%s says, \"%s\""),
		      ChanName(chan), title ? title : "",
		      title ? ANSI_NORMAL : "", title ? " " : "",
		      accented_name(player), arg1);
    if (!canhear)
      notify_format(player,
		    T("To channel %s: %s says, \"%s\""), ChanName(chan),
		    accented_name(player), arg1);
    break;
  }

  ChanNumMsgs(chan)++;
}

/** Emit on a channel.
 * \verbatim
 * This is the top-level function for @cemit.
 * \endverbatim
 * \param player the enactor.
 * \param name name of the channel.
 * \param msg message to emit.
 * \param noisy if 1, prepend the channel name to the message.
 */
void
do_cemit(dbref player, const char *name, const char *msg, int noisy)
{
  CHAN *chan = NULL;
  CHANUSER *u;
  int canhear;

  if (!name || !*name) {
    notify(player, T("That is not a valid channel."));
    return;
  }
  switch (find_channel(name, &chan, player)) {
  case CMATCH_NONE:
    notify(player, T("I don't recognize that channel."));
    return;
  case CMATCH_AMBIG:
    notify(player, T("I don't know which channel you mean."));
    return;
  }
  if (!Chan_Can_See(chan, player)) {
    notify(player, T("CHAT: I don't recognize that channel."));
    return;
  }
  if (!Chan_Ok_Type(chan, player)) {
    notify_format(player,
		  T("Sorry, you're not the right type to be on channel <%s>."),
		  ChanName(chan));
    return;
  }
  if (!Chan_Can_Speak(chan, player)) {
    notify_format(player,
		  T("Sorry, you're not allowed to speak on channel <%s>."),
		  ChanName(chan));
    return;
  }
  u = onchannel(player, chan);
  canhear = u ? !Chanuser_Gag(u) : 0;
  /* If the channel isn't open, you must hear it in order to speak */
  if (!Channel_Open(chan)) {
    if (!u) {
      notify(player, T("You must be on that channel to speak on it."));
      return;
    } else if (!canhear) {
      notify(player, T("You must stop gagging that channel to speak on it."));
      return;
    }
  }

  if (!msg || !*msg) {
    notify(player, T("What do you want to emit?"));
    return;
  }
  if (noisy)
    channel_broadcast(chan, player, 2, "<%s> %s", ChanName(chan), msg);
  else
    channel_broadcast(chan, player, 2, "%s", msg);
  if (!canhear)
    notify_format(player, T("Cemit to channel %s: %s"), ChanName(chan), msg);
  ChanNumMsgs(chan)++;
  return;
}

/** Administrative channel commands.
 * \verbatim
 * This is one of top-level functions for @channel. This one handles
 * the /add, /delete, /rename, /priv, and /quiet switches.
 * \endverbatim
 * \param player the enactor.
 * \param name the name of the channel.
 * \param perms the permissions to set on an added/priv'd channel, the newname for a renamed channel, or on/off for a quieted channel.
 * \param flag switch indicator: 0=add, 1=delete, 2=rename, 3=priv, 4=quiet
 */
void
do_chan_admin(dbref player, char *name, const char *perms, int flag)
{
  CHAN *chan = NULL, *temp = NULL;
  long int type;
  int res;
  struct boolexp *key;
  char old[CHAN_NAME_LEN];

  if (!name || !*name) {
    notify(player, T("You must specify a channel."));
    return;
  }
  if (Guest(player)) {
    notify(player, T("Guests may not modify channels."));
    return;
  }
  if ((flag > 1) && (!perms || !*perms)) {
    notify(player, T("What do you want to do with the channel?"));
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
      notify(player, T("No more room for channels."));
      return;
    }
    if (strlen(name) > CHAN_NAME_LEN - 1) {
      notify(player, T("The channel needs a shorter name."));
      return;
    }
    if (!ok_channel_name(name)) {
      notify(player, T("Invalid name for a channel."));
      return;
    }
    if (!Hasprivs(player) && !canstilladd(player)) {
      notify(player, T("You already own too many channels."));
      return;
    }
    res = find_channel(name, &chan, GOD);
    if (res != CMATCH_NONE) {
      notify(player, T("CHAT: The channel needs a more unique name."));
      return;
    }
    /* get the permissions. Invalid specs default to the default */
    type = string_to_privs(priv_table, perms, 0);
    if (!Chan_Can(player, type)) {
      notify(player, T("You can't create channels of that type."));
      return;
    }
    if (type & CHANNEL_DISABLED)
      notify(player, T("Warning: channel will be created disabled."));
    /* Can the player afford it? There's a cost */
    if (!payfor(Owner(player), CHANNEL_COST)) {
      notify_format(player, T("You can't afford the %d %s."), CHANNEL_COST,
		    MONIES);
      return;
    }
    /* Ok, let's do it */
    chan = new_channel();
    if (!chan) {
      notify(player, T("CHAT: No more memory for channels!"));
      giveto(Owner(player), CHANNEL_COST);
      return;
    }
    key = parse_boolexp(player, tprintf("=#%d", player), chan_mod_lock);
    if (!key) {
      mush_free(chan, "CHAN");
      notify(player, T("CHAT: No more memory for channels!"));
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
    notify_format(player, T("CHAT: Channel <%s> created."), ChanName(chan));
    break;
  case 1:
    /* remove a channel */
    /* Check permissions. Wizards and owners can remove */
    if (!Chan_Can_Nuke(chan, player)) {
      notify(player, T("Permission denied."));
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
    notify(player, T("Channel removed."));
    break;
  case 2:
    /* rename a channel */
    /* Can the player do this? */
    if (!Chan_Can_Modify(chan, player)) {
      notify(player, T("Permission denied."));
      return;
    }
    /* make sure the channel name is unique */
    if (find_channel(perms, &temp, GOD)) {
      /* But allow renaming a channel to a differently-cased version of
       * itself 
       */
      if (temp != chan) {
	notify(player, T("The channel needs a more unique new name."));
	return;
      }
    }
    if (strlen(perms) > CHAN_NAME_LEN - 1) {
      notify(player, T("That name is too long."));
      return;
    }
    /* When we rename a channel, we actually remove it and re-insert it */
    strcpy(old, ChanName(chan));
    remove_channel(chan);
    strcpy(ChanName(chan), perms);
    insert_channel(&chan);
    channel_broadcast(chan, player, 1,
		      "<%s> %s has renamed channel %s to %s.",
		      ChanName(chan), Name(player), old, ChanName(chan));
    notify(player, T("Channel renamed."));
    break;
  case 3:
    /* change the permissions on a channel */
    if (!Chan_Can_Modify(chan, player)) {
      notify(player, T("Permission denied."));
      return;
    }
    /* get the permissions. Invalid specs default to no change */
    type = string_to_privs(priv_table, perms, ChanType(chan));
    if (!Chan_Can_Priv(player, type)) {
      notify(player, T("You can't make channels that type."));
      return;
    }
    if (type & CHANNEL_DISABLED)
      notify(player, T("Warning: channel will be disabled."));
    if (type == ChanType(chan)) {
      notify_format(player,
		    T
		    ("Invalid or same permissions on channel <%s>. No changes made."),
		    ChanName(chan));
    } else {
      ChanType(chan) = type;
      notify_format(player,
		    T("Permissions on channel <%s> changed."), ChanName(chan));
    }
    break;
  case 4:
    /* Quiet a channel */
    if (!Chan_Can_Modify(chan, player)) {
      notify(player, T("Permission denied. Use @channel/mute <chan>=<y/n>"));
      return;
    }
    if (abs(yesno(perms)))
      ChanType(chan) |= CHANNEL_QUIET;
    else
      ChanType(chan) &= ~CHANNEL_QUIET;
    notify_format(player,
		  T("Quiet status on channel <%s> changed."), ChanName(chan));
    break;
  }
}

static int
ok_channel_name(const char *n)
{
  /* is name valid for a channel? */
  const char *p;
  char name[BUFFER_LEN];

  strcpy(name, remove_markup(n, NULL));

  if (!name || !*name)
    return 0;

  /* No leading spaces */
  if (isspace((unsigned char) *name))
    return 0;

  /* only printable characters */
  for (p = name; p && *p; p++) {
    if (!isprint((unsigned char) *p))
      return 0;
  }

  /* No trailing spaces */
  p--;
  if (isspace((unsigned char) *p))
    return 0;

  return 1;
}


/** Modify a user's settings on a channel.
 * \verbatim
 * This is one of top-level functions for @channel. This one
 * handles the /mute, /hide, and /gag switches, which control an
 * individual user's settings on a single channel.
 * \endverbatim
 * \param player the enactor.
 * \param name the name of the channel.
 * \param isyn a yes/no string.
 * \param flag switch indicator: 0=mute, 1=hide, 2=gag
 * \param silent if 1, no notification of actions.
 */
void
do_chan_user_flags(dbref player, char *name, const char *isyn, int flag,
		   int silent)
{
  CHAN *c = NULL;
  CHANUSER *u;
  CHANLIST *p;
  int setting = abs(yesno(isyn));
  p = NULL;

  if (!name || !*name) {
    p = Chanlist(player);
    if (!p) {
      notify(player, T("You are not on any channels."));
      return;
    }
    silent = 1;
    switch (flag) {
    case 0:
      notify(player, setting ? T("All channels have been muted.")
	     : T("All channels have been unmuted."));
      break;
    case 1:
      notify(player, setting ? T("You hide on all the channels you can.")
	     : T("You unhide on all channels."));
      break;
    case 2:
      notify(player, setting ? T("All channels have been gagged.")
	     : T("All channels have been ungagged."));
      break;
    }
  } else {
    test_channel(player, name, c);
  }

  /* channel loop */
  do {
    /* If we have a channel list at the start, 
     * that means they didn't gave us a channel name,
     * so we now figure out c. */
    if (p != NULL) {
      c = p->chan;
      p = p->next;
    }

    u = onchannel(player, c);
    if (!u) {
      /* This should only happen if they gave us a bad name */
      if (!silent)
	notify_format(player, T("You are not on channel <%s>."), ChanName(c));
      return;
    }

    switch (flag) {
    case 0:
      /* Mute */
      if (setting) {
	CUtype(u) |= CU_QUIET;
	if (!silent)
	  notify_format(player,
			T
			("You will no longer hear connection messages on channel <%s>."),
			ChanName(c));
      } else {
	CUtype(u) &= ~CU_QUIET;
	if (!silent)
	  notify_format(player,
			T
			("You will now hear connection messages on channel <%s>."),
			ChanName(c));
      }
      break;

    case 1:
      /* Hide */
      if (setting) {
	if (!Chan_Can_Hide(c, player) && !Wizard(player)) {
	  if (!silent)
	    notify_format(player,
			  T("You are not permitted to hide on channel <%s>."),
			  ChanName(c));
	} else {
	  CUtype(u) |= CU_HIDE;
	  if (!silent)
	    notify_format(player,
			  T("You no longer appear on channel <%s>'s who list."),
			  ChanName(c));
	}
      } else {
	CUtype(u) &= ~CU_HIDE;
	if (!silent)
	  notify_format(player,
			T("You now appear on channel <%s>'s who list."),
			ChanName(c));
      }
      break;
    case 2:
      /* Gag */
      if (setting) {
	CUtype(u) |= CU_GAG;
	if (!silent)
	  notify_format(player,
			T("You will no longer hear messages on channel <%s>."),
			ChanName(c));
      } else {
	CUtype(u) &= ~CU_GAG;
	if (!silent)
	  notify_format(player,
			T("You will now hear messages on channel <%s>."),
			ChanName(c));
      }
      break;
    }
  } while (p != NULL);

  return;
}

/** Set a user's title for the channel.
 * \verbatim
 * This is one of the top-level functions for @channel. It handles
 * the /title switch.
 * \param player the enactor.
 * \param name the name of the channel.
 * \param title the player's new channel title.
 */
void
do_chan_title(dbref player, const char *name, const char *title)
{
  CHAN *c = NULL;
  CHANUSER *u;
  const char *scan;

  if (!name || !*name) {
    notify(player, T("You must specify a channel."));
    return;
  }
  if (strlen(title) >= CU_TITLE_LEN) {
    notify(player, T("Title too long."));
    return;
  }
  /* Stomp newlines and other weird whitespace */
  for (scan = title; *scan; scan++) {
    if ((isspace((unsigned char) *scan) && (*scan != ' '))
	|| (*scan == BEEP_CHAR)) {
      notify(player, T("Invalid character in title."));
      return;
    }
  }
  test_channel(player, name, c);
  u = onchannel(player, c);
  if (!u) {
    notify_format(player, T("You are not on channel <%s>."), ChanName(c));
    return;
  }
  strcpy(CUtitle(u), title);
  if (!Quiet(player))
    notify_format(player, T("Title %s for channel <%s>."),
		  *title ? T("set") : T("cleared"), ChanName(c));
  return;
}

/** List all the channels and their flags.
 * \verbatim
 * This is one of the top-level functions for @channel. It handles the
 * /list switch.
 * \endverbatim
 * \param player the enactor.
 * \param partname a partial channel name to match.
 */
void
do_channel_list(dbref player, const char *partname)
{
  CHAN *c;
  CHANUSER *u;
  char numusers[BUFFER_LEN];
  char cleanname[CHAN_NAME_LEN];
  const char thirtyblanks[31] = "                              ";
  char blanks[31];
  int numblanks;

  if (SUPPORT_PUEBLO)
    notify_noenter(player, tprintf("%cSAMP%c", TAG_START, TAG_END));
  notify_format(player, "%-30s %-5s %8s %-16s %-8s %-3s",
		"Name", "Users", "Msgs", T("Chan Type"), "Status", "Buf");
  for (c = channels; c; c = c->next) {
    strcpy(cleanname, remove_markup(ChanName(c), NULL));
    if (Chan_Can_See(c, player) && string_prefix(cleanname, partname)) {
      u = onchannel(player, c);
      if (SUPPORT_PUEBLO)
	sprintf(numusers,
		"%cA XCH_CMD=\"@channel/who %s\" XCH_HINT=\"See who's on this channel now\"%c%5ld%c/A%c",
		TAG_START, cleanname, TAG_END, ChanNumUsers(c),
		TAG_START, TAG_END);
      else
	sprintf(numusers, "%5ld", ChanNumUsers(c));
      numblanks = strlen(ChanName(c)) - strlen(cleanname);
      if (numblanks > 0 && numblanks < 31) {
	strcpy(blanks, thirtyblanks);
	blanks[numblanks] = '\0';
      } else {
	blanks[0] = '\0';
      }
      notify_format(player,
		    "%-30s%s %s %8ld [%c%c%c%c%c%c%c %c%c%c%c%c%c] [%-3s %c%c] %3d",
		    ChanName(c), blanks, numusers, ChanNumMsgs(c),
		    Channel_Disabled(c) ? 'D' : '-',
		    Channel_Player(c) ? 'P' : '-',
		    Channel_Object(c) ? 'O' : '-',
		    Channel_Admin(c) ? 'A' : (Channel_Wizard(c) ? 'W' : '-'),
		    Channel_Quiet(c) ? 'Q' : '-',
		    Channel_CanHide(c) ? 'H' : '-', Channel_Open(c) ? 'o' : '-',
		    /* Locks */
		    ChanJoinLock(c) != TRUE_BOOLEXP ? 'j' : '-',
		    ChanSpeakLock(c) != TRUE_BOOLEXP ? 's' : '-',
		    ChanModLock(c) != TRUE_BOOLEXP ? 'm' : '-',
		    ChanSeeLock(c) != TRUE_BOOLEXP ? 'v' : '-',
		    ChanHideLock(c) != TRUE_BOOLEXP ? 'h' : '-',
		    /* Does the player own it? */
		    ChanCreator(c) == player ? '*' : '-',
		    /* User status */
		    u ? (Chanuser_Gag(u) ? "Gag" : "On") : "Off",
		    (u &&Chanuser_Quiet(u)) ? 'Q' : ' ',
		    (u &&Chanuser_Hide(u)) ? 'H' : ' ',
		    channel_buffer_lines(c));
    }
  }
  if (SUPPORT_PUEBLO)
    notify_noenter(player, tprintf("%c/SAMP%c", TAG_START, TAG_END));
}

static char *
list_cflags(CHAN *c)
{
  static char tbuf1[BUFFER_LEN];
  char *bp;

  bp = tbuf1;
  if (Channel_Disabled(c))
    safe_chr('D', tbuf1, &bp);
  if (Channel_Player(c))
    safe_chr('P', tbuf1, &bp);
  if (Channel_Object(c))
    safe_chr('O', tbuf1, &bp);
  if (Channel_Admin(c))
    safe_chr('A', tbuf1, &bp);
  if (Channel_Wizard(c))
    safe_chr('W', tbuf1, &bp);
  if (Channel_Quiet(c))
    safe_chr('Q', tbuf1, &bp);
  if (Channel_CanHide(c))
    safe_chr('H', tbuf1, &bp);
  if (Channel_Open(c))
    safe_chr('o', tbuf1, &bp);
  *bp = '\0';
  return tbuf1;
}

static char *
list_cuflags(CHANUSER *u)
{
  static char tbuf1[BUFFER_LEN];
  char *bp;

  bp = tbuf1;
  if (Chanuser_Gag(u))
     safe_chr('G', tbuf1, &bp);
  if (Chanuser_Quiet(u))
     safe_chr('Q', tbuf1, &bp);
  if (Chanuser_Hide(u))
     safe_chr('H', tbuf1, &bp);
  *bp = '\0';
  return tbuf1;
}

/* ARGSUSED */
FUNCTION(fun_cflags)
{
  /* With one channel arg, returns list of set flags, as per 
   * do_channel_list. Sample output: PQ, Oo, etc.
   * With two args (channel,object) return channel-user flags
   * for that object on that channel (a subset of GQH).
   * You must pass channel's @clock/see, and, in second case,
   * must be able to examine the object.
   */
  CHAN *c;
  CHANUSER *u;
  dbref thing;

  if (!args[0] || !*args[0]) {
    safe_str(T("#-1 NO CHANNEL GIVEN"), buff, bp);
    return;
  }
  switch (find_channel(args[0], &c, executor)) {
  case CMATCH_NONE:
    safe_str(T("#-1 NO SUCH CHANNEL"), buff, bp);
    return;
  case CMATCH_AMBIG:
    safe_str(T("#-1 AMBIGUOUS CHANNEL NAME"), buff, bp);
    return;
  default:
    if (!Chan_Can_See(c, executor)) {
      safe_str(T("#-1 NO SUCH CHANNEL"), buff, bp);
      return;
    }
    if (nargs == 1) {
      safe_str(list_cflags(c), buff, bp);
      return;
    }
    thing = match_thing(executor, args[1]);
    if (thing == NOTHING) {
      safe_str(T(e_match), buff, bp);
      return;
    }
    if (!Can_Examine(executor, thing)) {
      safe_str(T(e_perm), buff, bp);
      return;
    }
    u = onchannel(thing, c);
    if (!u) {
      safe_str(T("#-1 NOT ON CHANNEL"), buff, bp);
      return;
    }
    safe_str(list_cuflags(u), buff, bp);
    break;
  }
}


/* ARGSUSED */
FUNCTION(fun_ctitle)
{
  /* ctitle(<channel>,<object>) returns the object's chantitle on that chan.
   * You must pass the channel's see-lock, and
   * either you must either be able to examine <object>, or
   * <object> must not be hidden, and either
   *   a) You must be on <channel>, or
   *   b) You must pass the join-lock 
   */
  CHAN *c;
  CHANUSER *u;
  dbref thing;
  int ok;
  int can_ex;

  if (!args[0] || !*args[0]) {
    safe_str(T("#-1 NO CHANNEL GIVEN"), buff, bp);
    return;
  }
  switch (find_channel(args[0], &c, executor)) {
  case CMATCH_NONE:
    safe_str(T("#-1 NO SUCH CHANNEL"), buff, bp);
    return;
  case CMATCH_AMBIG:
    safe_str(T("#-1 AMBIGUOUS CHANNEL NAME"), buff, bp);
    return;
  default:
    thing = match_thing(executor, args[1]);
    if (thing == NOTHING) {
      safe_str(T(e_match), buff, bp);
      return;
    }
    if (!Chan_Can_See(c, executor)) {
      safe_str(T("#-1 NO SUCH CHANNEL"), buff, bp);
      return;
    }
    can_ex = Can_Examine(executor, thing);
    ok = (onchannel(executor, c) || Chan_Can_Join(c, executor));
    u = onchannel(thing, c);
    if (!u) {
      if (can_ex || ok)
	safe_str(T("#-1 NOT ON CHANNEL"), buff, bp);
      else
	safe_str(T("#-1 PERMISSION DENIED"), buff, bp);
      return;
    }
    ok &= !Chanuser_Hide(u);
    if (!(can_ex || ok)) {
      safe_str(T("#-1 PERMISSION DENIED"), buff, bp);
      return;
    }
    if (CUtitle(u))
       safe_str(CUtitle(u), buff, bp);
    break;
  }
}

/* Remove all players from a channel, notifying them. This is the 
 * utility routine for handling it. The command @channel/wipe
 * calls do_chan_wipe, below
 */
static void
channel_wipe(dbref player, CHAN *chan)
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
       notify_format(victim, T("CHAT: %s has removed all users from <%s>."),
		     Name(player), ChanName(chan));
  }
  ChanNumUsers(chan) = 0;
  return;
}

/** Remove all players from a channel.
 * \verbatim
 * This is the top-level function for @channel/wipe, which removes all
 * players from a channel.
 * \endverbatim
 * \param player the enactor.
 * \param name name of channel to wipe.
 */
void
do_chan_wipe(dbref player, const char *name)
{
  CHAN *c;
  /* Find the channel */
  test_channel(player, name, c);
  /* Check permissions */
  if (!Chan_Can_Modify(c, player)) {
    notify(player, T("CHAT: Wipe that silly grin off your face instead."));
    return;
  }
  /* Wipe it */
  channel_wipe(player, c);
  notify_format(player, T("CHAT: Channel <%s> wiped."), ChanName(c));
  return;
}

/** Change the owner of a channel.
 * \verbatim
 * This is the top-level function for @channel/chown, which changes
 * ownership of a channel.
 * \endverbatim
 * \param player the enactor.
 * \param name name of the channel.
 * \param newowner name of the new owner for the channel.
 */
void
do_chan_chown(dbref player, const char *name, const char *newowner)
{
  CHAN *c;
  dbref victim;
  /* Only a Wizard can do this */
  if (!Wizard(player)) {
    notify(player, T("CHAT: Only a Wizard can do that."));
    return;
  }
  /* Find the channel */
  test_channel(player, name, c);
  /* Find the victim */
  if ((victim = lookup_player(newowner)) == NOTHING) {
    notify(player, T("CHAT: Invalid owner."));
    return;
  }
  /* We refund the original owner's money, but don't charge the
   * new owner. 
   */
  chan_chown(c, victim);
  notify_format(player,
		T("CHAT: Channel <%s> now owned by %s."), ChanName(c),
		Name(ChanCreator(c)));
  return;
}

/** Chown all of a player's channels. 
 * This function changes ownership of all of a player's channels. It's
 * usually used before destroying the player.
 * \param old dbref of old channel owner.
 * \param newowner dbref of new channel owner.
 */
void
chan_chownall(dbref old, dbref newowner)
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

/** Lock one of the channel's locks.
 * \verbatim
 * This is the top-level function for @clock.
 * \endverbatim
 * \param player the enactor.
 * \param name the name of the channel.
 * \param lockstr string representation of the lock value.
 * \param whichlock which lock is to be set.
 */
void
do_chan_lock(dbref player, const char *name, const char *lockstr, int whichlock)
{
  CHAN *c;
  struct boolexp *key;
  const char *ltype;

  /* Make sure the channel exists */
  test_channel(player, name, c);
  /* Make sure the player has permission */
  if (!Chan_Can_Modify(c, player)) {
    notify_format(player, T("CHAT: Channel <%s> resists."), ChanName(c));
    return;
  }
  /* Ok, let's do it */
  switch (whichlock) {
  case CL_JOIN:
    ltype = chan_join_lock;
    break;
  case CL_MOD:
    ltype = chan_mod_lock;
    break;
  case CL_SEE:
    ltype = chan_see_lock;
    break;
  case CL_HIDE:
    ltype = chan_hide_lock;
    break;
  case CL_SPEAK:
    ltype = chan_speak_lock;
    break;
  default:
    ltype = "ChanUnknownLock";
  }

  if (!lockstr || !*lockstr) {
    /* Unlock it */
    key = TRUE_BOOLEXP;
  } else {
    key = parse_boolexp(player, lockstr, ltype);
    if (key == TRUE_BOOLEXP) {
      notify(player, T("CHAT: I don't understand that key."));
      return;
    }
  }
  switch (whichlock) {
  case CL_JOIN:
    free_boolexp(ChanJoinLock(c));
    ChanJoinLock(c) = key;
    notify_format(player, (key == TRUE_BOOLEXP) ?
		  T("CHAT: Joinlock on <%s> reset.") :
		  T("CHAT: Joinlock on <%s> set."), ChanName(c));
    break;
  case CL_SPEAK:
    free_boolexp(ChanSpeakLock(c));
    ChanSpeakLock(c) = key;
    notify_format(player, (key == TRUE_BOOLEXP) ?
		  T("CHAT: Speaklock on <%s> reset.") :
		  T("CHAT: Speaklock on <%s> set."), ChanName(c));
    break;
  case CL_SEE:
    free_boolexp(ChanSeeLock(c));
    ChanSeeLock(c) = key;
    notify_format(player, (key == TRUE_BOOLEXP) ?
		  T("CHAT: Seelock on <%s> reset.") :
		  T("CHAT: Seelock on <%s> set."), ChanName(c));
    break;
  case CL_HIDE:
    free_boolexp(ChanHideLock(c));
    ChanHideLock(c) = key;
    notify_format(player, (key == TRUE_BOOLEXP) ?
		  T("CHAT: Hidelock on <%s> reset.") :
		  T("CHAT: Hidelock on <%s> set."), ChanName(c));
    break;
  case CL_MOD:
    free_boolexp(ChanModLock(c));
    ChanModLock(c) = key;
    notify_format(player, (key == TRUE_BOOLEXP) ?
		  T("CHAT: Modlock on <%s> reset.") :
		  T("CHAT: Modlock on <%s> set."), ChanName(c));
    break;
  }
  return;
}


/** A channel list with names and descriptions only.
 * \verbatim
 * This is the top-level function for @channel/what.
 * \endverbatim
 * \param player the enactor.
 * \param partname a partial name of channels to match.
 */
void
do_chan_what(dbref player, const char *partname)
{
  CHAN *c;
  int found = 0;
  char cleanname[BUFFER_LEN];
  char cleanp[CHAN_NAME_LEN];

  strcpy(cleanname, remove_markup(partname, NULL));
  if (ShowAnsi(player)) {
    for (c = channels; c; c = c->next) {
      strcpy(cleanp, remove_markup(ChanName(c), NULL));
      if (Chan_Can_See(c, player) && string_prefix(cleanp, cleanname)) {
	notify_format(player, "%s<%s>%s",
		      ANSI_HILITE, ChanName(c), ANSI_NORMAL);
	notify_format(player, "%s: %s", T("Creator"), Name(ChanCreator(c)));
	notify(player, privs_to_string(priv_table, ChanType(c)));
	notify(player, ChanTitle(c));
	found++;
      }
    }
  } else {
    for (c = channels; c; c = c->next) {
      strcpy(cleanp, remove_markup(ChanName(c), NULL));
      if (Chan_Can_See(c, player) && string_prefix(cleanp, cleanname)) {
	notify_format(player, "<%s>", ChanName(c));
	notify_format(player, "%s: %s", T("Creator"), Name(ChanCreator(c)));
	notify(player, privs_to_string(priv_table, ChanType(c)));
	notify(player, ChanTitle(c));
	found++;
      }
    }
  }
  if (!found)
    notify(player, T("CHAT: I don't recognize that channel."));
}


/** A decompile of a channel.
 * \verbatim
 * This is the top-level function for @channel/decompile, which attempts
 * to produce all the MUSHcode necessary to recreate a channel and its
 * membership.
 * \param player the enactor.
 * \param name name of the channel.
 * \param brief if 1, don't include channel membership.
 */
void
do_chan_decompile(dbref player, const char *name, int brief)
{
  CHAN *c;
  CHANUSER *u;
  int found;
  char cleanname[BUFFER_LEN];
  char cleanp[CHAN_NAME_LEN];

  found = 0;
  strcpy(cleanname, remove_markup(name, NULL));
  for (c = channels; c; c = c->next) {
    strcpy(cleanp, remove_markup(ChanName(c), NULL));
    if (string_prefix(cleanp, cleanname)) {
      found++;
      if (!(See_All(player) || Chan_Can_Modify(c, player)
	    || (ChanCreator(c) == player))) {
	if (Chan_Can_See(c, player))
	  notify_format(player, T("CHAT: No permission to decompile <%s>"),
			ChanName(c));
	continue;
      }
      notify_format(player, "@channel/add %s = %s", ChanName(c),
		    privs_to_string(priv_table, ChanType(c)));
      notify_format(player, "@channel/chown %s = %s", ChanName(c),
		    Name(ChanCreator(c)));
      if (ChanModLock(c) != TRUE_BOOLEXP)
	notify_format(player, "@clock/mod %s = %s", ChanName(c),
		      unparse_boolexp(player, ChanModLock(c), UB_DBREF));
      if (ChanHideLock(c) != TRUE_BOOLEXP)
	notify_format(player, "@clock/hide %s = %s", ChanName(c),
		      unparse_boolexp(player, ChanHideLock(c), UB_DBREF));
      if (ChanJoinLock(c) != TRUE_BOOLEXP)
	notify_format(player, "@clock/join %s = %s", ChanName(c),
		      unparse_boolexp(player, ChanJoinLock(c), UB_DBREF));
      if (ChanSpeakLock(c) != TRUE_BOOLEXP)
	notify_format(player, "@clock/speak %s = %s", ChanName(c),
		      unparse_boolexp(player, ChanSpeakLock(c), UB_DBREF));
      if (ChanSeeLock(c) != TRUE_BOOLEXP)
	notify_format(player, "@clock/see %s = %s", ChanName(c),
		      unparse_boolexp(player, ChanSeeLock(c), UB_DBREF));
      if (ChanTitle(c))
	notify_format(player, "@channel/desc %s = %s", ChanName(c),
		      ChanTitle(c));
      if (ChanBuffer(c))
	notify_format(player, "@channel/buffer %s = %d", ChanName(c),
		      channel_buffer_lines(c));
      if (!brief) {
	for (u = ChanUsers(c); u; u = u->next) {
	  if (!Chanuser_Hide(u) || Priv_Who(player))

	     notify_format(player, "@channel/on %s = %s", ChanName(c),
			   Name(CUdbref(u)));
	}
      }
    }
  }
  if (!found)
    notify(player, T("CHAT: No channel matches that string."));
}

static void
do_channel_who(dbref player, CHAN *chan)
{
  char tbuf1[BUFFER_LEN];
  char *bp;
  CHANUSER *u;
  dbref who;
  int i = 0;
  bp = tbuf1;
  for (u = ChanUsers(chan); u; u = u->next) {
    who = CUdbref(u);
    if ((IsThing(who) || Connected(who)) &&
	(!Chanuser_Hide(u) || Priv_Who(player))) {
      i++;
      safe_itemizer(i, !(u->next), ",", T("and"), " ", tbuf1, &bp);
      safe_str(Name(who), tbuf1, &bp);
      if (IsThing(who))
	safe_format(tbuf1, &bp, "(#%d)", who);
      if (Chanuser_Hide(u) && Chanuser_Gag(u))
	 safe_str(" (hidden,gagging)", tbuf1, &bp);
      else if (Chanuser_Hide(u))
	 safe_str(" (hidden)", tbuf1, &bp);
      else if (Chanuser_Gag(u))
	 safe_str(" (gagging)", tbuf1, &bp);
    }
  }
  *bp = '\0';
  if (!*tbuf1)
    notify(player, T("There are no connected players on that channel."));
  else {
    notify_format(player, T("Members of channel <%s> are:"), ChanName(chan));
    notify(player, tbuf1);
  }
}

/* ARGSUSED */
FUNCTION(fun_cwho)
{
  int first = 1;
  CHAN *chan = NULL;
  CHANUSER *u;
  int i;
  dbref who;
  i = find_channel(args[0], &chan, executor);
  switch (i) {
  case CMATCH_NONE:
    notify(executor, T("No such channel."));
    return;
  case CMATCH_AMBIG:
    notify(executor, T("I can't tell which channel you mean."));
    return;
  }

  /* Feh. We need to do some sort of privilege checking, so that
   * if mortals can't do '@channel/who wizard', they can't do
   * 'think cwho(wizard)' either. The first approach that comes to
   * mind is the following:
   * if (!ChannelPermit(privs,chan)) ...
   * Unfortunately, we also want objects to be able to check cwho()
   * on channels.
   * So, we check the owner, instead, because most uses of cwho()
   * are either in the Master Room owned by a wizard, or on people's
   * quicktypers.
   */

  if (!Chan_Can_See(chan, Owner(executor))
      && !Chan_Can_See(chan, executor)) {
    safe_str(T("#-1 NO PERMISSIONS FOR CHANNEL"), buff, bp);
    return;
  }
  for (u = ChanUsers(chan); u; u = u->next) {
    who = CUdbref(u);
    if ((IsThing(who) || Connected(who)) &&
	(!Chanuser_Hide(u) || Priv_Who(executor))) {
      if (first)
	first = 0;
      else
	safe_chr(' ', buff, bp);
      safe_dbref(who, buff, bp);
    }
  }
}


/** Modify a channel's description.
 * \verbatim
 * This is the top-level function for @channel/desc, which sets a channel's
 * description.
 * \endverbatim
 * \param player the enactor.
 * \param name name of the channel.
 * \param title description of the channel.
 */
void
do_chan_desc(dbref player, const char *name, const char *title)
{
  CHAN *c;
  /* Check new title length */
  if (title && strlen(title) > CHAN_TITLE_LEN - 1) {
    notify(player, T("CHAT: New description too long."));
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
    ChanTitle(c)[0] = '\0';
    notify_format(player, T("CHAT: Channel <%s> description cleared."),
		  ChanName(c));
  } else {
    strcpy(ChanTitle(c), title);
    notify_format(player, T("CHAT: Channel <%s> description set."),
		  ChanName(c));
  }
}



static int
yesno(const char *str)
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
canstilladd(dbref player)
{
  CHAN *c;
  int num = 0;
  for (c = channels; c; c = c->next) {
    if (ChanCreator(c) == player)
      num++;
  }
  return (num < MAX_PLAYER_CHANS);
}



/** Tell players on a channel when someone connects or disconnects.
 * \param player player to announce to.
 * \param msg message to announce.
 */
void
chat_player_announce(dbref player, char *msg)
{
  CHAN *c;
  CHANUSER *u;

  for (c = channels; c; c = c->next) {
    u = onchannel(player, c);
    if (u) {
      if (!Channel_Quiet(c) && (Channel_Admin(c) || Channel_Wizard(c)
				|| (!Chanuser_Hide(u) && !Dark(player))))
	 channel_broadcast(c, player, 1, "<%s> %s", ChanName(c), msg);
      CUtype(u) &= ~CU_GAG;
    }
  }
}

/** Return a list of channels that the player is on.
 * \param player player whose channels are to be shown.
 * \return string listing player's channels, prefixed with Channels:
 */
const char *
channel_description(player)
    dbref player;
{
  static char buf[BUFFER_LEN];
  CHANLIST *c;

  *buf = '\0';

  if (Chanlist(player)) {
    strcpy(buf, T("Channels:"));
    for (c = Chanlist(player); c; c = c->next)
      sprintf(buf, "%s %s", buf, ChanName(c->chan));
  } else if (IsPlayer(player))
    strcpy(buf, T("Channels: *NONE*"));
  return buf;
}


FUNCTION(fun_channels)
{
  dbref it;
  char sep = ' ';
  CHAN *c;
  CHANLIST *cl;
  CHANUSER *u;
  int can_ex;

  /* There are these possibilities:
   *  no args - just a list of all channels
   *   2 args - object, delimiter
   *   1 arg - either object or delimiter. If it's longer than 1 char,
   *           we treat it as an object.
   * You can see an object's channels if you can examine it.
   * Otherwise you can see only channels that you share with
   * it where it's not hidden.
   */
  if (nargs >= 1) {
    /* Given an argument, return list of channels it's on */
    it = match_result(executor, args[0], NOTYPE, MAT_EVERYTHING);
    if (GoodObject(it)) {
      int first = 1;
      if (!delim_check(buff, bp, nargs, args, 2, &sep))
	return;
      can_ex = Can_Examine(executor, it);
      for (cl = Chanlist(it); cl; cl = cl->next) {
	if (can_ex || ((u = onchannel(it, cl->chan)) &&!Chanuser_Hide(u)
		       && onchannel(executor, cl->chan))) {
	  if (!first)
	    safe_chr(sep, buff, bp);
	  safe_str(ChanName(cl->chan), buff, bp);
	  first = 0;
	}
      }
      return;
    } else {
      /* args[0] didn't match. Maybe it's a delimiter? */
      if (arglens[0] > 1) {
	if (it == NOTHING)
	  notify(executor, T("I can't see that here."));
	else if (it == AMBIGUOUS)
	  notify(executor, T("I don't know which thing you mean."));
	return;
      } else if (!delim_check(buff, bp, nargs, args, 1, &sep))
	return;
    }
  }
  /* No arguments (except maybe delimiter) - return list of all channels */
  for (c = channels; c; c = c->next) {
    if (Chan_Can_See(c, executor)) {
      if (c != channels)
	safe_chr(sep, buff, bp);
      safe_str(ChanName(c), buff, bp);
    }
  }
  return;
}

FUNCTION(fun_clock)
{
  CHAN *c = NULL;
  char *p = NULL;
  struct boolexp *lock_ptr = NULL;
  int which_lock = 0;

  if ((p = strchr(args[0], '/'))) {
    *p++ = '\0';
  } else {
    p = (char *) "JOIN";
  }

  switch (find_channel(args[0], &c, executor)) {
  case CMATCH_NONE:
    safe_str(T("#-1 NO SUCH CHANNEL"), buff, bp);
    return;
  case CMATCH_AMBIG:
    safe_str("#-2 AMBIGUOUS CHANNEL MATCH", buff, bp);
    return;
  }

  if (!strcasecmp(p, "JOIN")) {
    which_lock = CL_JOIN;
    lock_ptr = ChanJoinLock(c);
  } else if (!strcasecmp(p, "SPEAK")) {
    which_lock = CL_SPEAK;
    lock_ptr = ChanSpeakLock(c);
  } else if (!strcasecmp(p, "MOD")) {
    which_lock = CL_MOD;
    lock_ptr = ChanModLock(c);
  } else if (!strcasecmp(p, "SEE")) {
    which_lock = CL_SEE;
    lock_ptr = ChanSeeLock(c);
  } else if (!strcasecmp(p, "HIDE")) {
    which_lock = CL_HIDE;
    lock_ptr = ChanHideLock(c);
  } else {
    safe_str(T("#-1 NO SUCH LOCK TYPE"), buff, bp);
    return;
  }

  if (nargs == 2) {
#ifdef FUNCTION_SIDE_EFFECTS
    if (!command_check_byname(executor, "@clock") || fun->flags & FN_NOSIDEFX) {
      safe_str(T(e_perm), buff, bp);
      return;
    }
    do_chan_lock(executor, args[0], args[1], which_lock);
    return;
#else
    safe_str(T(e_disabled), buff, bp);
#endif
  }

  if (Chan_Can_Decomp(c, executor)) {
    safe_str(unparse_boolexp(executor, lock_ptr, 1), buff, bp);
    return;
  } else {
    safe_str(T(e_perm), buff, bp);
    return;
  }
}

/* ARGSUSED */
FUNCTION(fun_cemit)
{
  int noisy = 0;

  if (!command_check_byname(executor, "@cemit") || fun->flags & FN_NOSIDEFX) {
    safe_str(T(e_perm), buff, bp);
    return;
  }
  if (nargs == 3 && parse_boolean(args[2]))
    noisy = 1;
  orator = executor;
  do_cemit(executor, args[0], args[1], noisy);
}

COMMAND (cmd_cemit) {
  do_cemit(player, arg_left, arg_right, SW_ISSET(sw, SWITCH_NOISY));
}

COMMAND (cmd_channel) {
  if (switches)
    do_channel(player, arg_left, arg_right, switches);
  else if (SW_ISSET(sw, SWITCH_LIST))
    do_channel_list(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_ADD))
    do_chan_admin(player, arg_left, arg_right, 0);
  else if (SW_ISSET(sw, SWITCH_DELETE))
    do_chan_admin(player, arg_left, arg_right, 1);
  else if (SW_ISSET(sw, SWITCH_NAME))
    do_chan_admin(player, arg_left, arg_right, 2);
  else if (SW_ISSET(sw, SWITCH_RENAME))
    do_chan_admin(player, arg_left, arg_right, 2);
  else if (SW_ISSET(sw, SWITCH_PRIVS))
    do_chan_admin(player, arg_left, arg_right, 3);
  else if (SW_ISSET(sw, SWITCH_QUIET))
    do_chan_admin(player, arg_left, arg_right, 4);
  else if (SW_ISSET(sw, SWITCH_NOISY))
    do_chan_admin(player, arg_left, "n", 4);
  else if (SW_ISSET(sw, SWITCH_DECOMPILE))
    do_chan_decompile(player, arg_left, SW_ISSET(sw, SWITCH_BRIEF));
  else if (SW_ISSET(sw, SWITCH_DESCRIBE))
    do_chan_desc(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_TITLE))
    do_chan_title(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_CHOWN))
    do_chan_chown(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_WIPE))
    do_chan_wipe(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_MUTE))
    do_chan_user_flags(player, arg_left, arg_right, 0, 0);
  else if (SW_ISSET(sw, SWITCH_UNMUTE))
    do_chan_user_flags(player, arg_left, "n", 0, 0);
  else if (SW_ISSET(sw, SWITCH_HIDE))
    do_chan_user_flags(player, arg_left, arg_right, 1, 0);
  else if (SW_ISSET(sw, SWITCH_UNHIDE))
    do_chan_user_flags(player, arg_left, "n", 1, 0);
  else if (SW_ISSET(sw, SWITCH_GAG))
    do_chan_user_flags(player, arg_left, arg_right, 2, 0);
  else if (SW_ISSET(sw, SWITCH_UNGAG))
    do_chan_user_flags(player, arg_left, "n", 2, 0);
  else if (SW_ISSET(sw, SWITCH_WHAT))
    do_chan_what(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_RECALL))
    do_chan_recall(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_BUFFER))
    do_chan_buffer(player, arg_left, arg_right);
  else
    do_channel(player, arg_left, NULL, arg_right);
}

COMMAND (cmd_chat) {
  do_chat_by_name(player, arg_left, arg_right, 1);
}

COMMAND (cmd_clock) {
  if (SW_ISSET(sw, SWITCH_JOIN))
    do_chan_lock(player, arg_left, arg_right, CL_JOIN);
  else if (SW_ISSET(sw, SWITCH_SPEAK))
    do_chan_lock(player, arg_left, arg_right, CL_SPEAK);
  else if (SW_ISSET(sw, SWITCH_MOD))
    do_chan_lock(player, arg_left, arg_right, CL_MOD);
  else if (SW_ISSET(sw, SWITCH_SEE))
    do_chan_lock(player, arg_left, arg_right, CL_SEE);
  else if (SW_ISSET(sw, SWITCH_HIDE))
    do_chan_lock(player, arg_left, arg_right, CL_HIDE);
  else
    notify(player, T("You must specify a type of lock"));
}

/** Find the next player on a channel to notify.
 * This function is a helper for notify_anything that is used to
 * notify all players on a channel.
 * \param current next dbref to notify (not used).
 * \param data pointer to structure containing channel and chanuser data.
 * \return next dbref to notify.
 */
dbref
na_channel(current, data)
    dbref current;
    void *data;
{
  struct na_cpass *nac = data;
  CHANUSER *u, *nu;
  int cont;

  nu = nac->u;
  do {
    u = nu;
    if (!u)
      return NOTHING;
    current = CUdbref(u);
    nu = u->next;
    cont = (!GoodObject(current) ||
	    (nac->checkquiet && Chanuser_Quiet(u)) ||
	    Chanuser_Gag(u) || (IsPlayer(current) && !Connected(current)));
  } while (cont);
  nac->u = nu;
  return current;
}

/** Broadcast a message to a channel.
 * \param channel pointer to channel to broadcast to.
 * \param player message speaker.
 * \param flags broadcast flag mask (0x1 = checkquiet, 0x2 = nospoof)
 * \param fmt message format string.
 */
void WIN32_CDECL
channel_broadcast(CHAN *channel, dbref player, int flags, const char *fmt, ...)
/* flags: 0x1 = checkquiet, 0x2 = nospoof */
{
  va_list args;
#ifdef HAS_VSNPRINTF
  char tbuf1[BUFFER_LEN];
#else
  char tbuf1[BUFFER_LEN * 2];	/* Safety margin as per tprintf */
#endif
  struct na_cpass nac;

  /* Make sure we can write to the channel before doing so */
  if (Channel_Disabled(channel))
    return;

  va_start(args, fmt);

#ifdef HAS_VSNPRINTF
  (void) vsnprintf(tbuf1, sizeof tbuf1, fmt, args);
#else
  (void) vsprintf(tbuf1, fmt, args);
#endif
  va_end(args);
  tbuf1[BUFFER_LEN - 1] = '\0';

  nac.u = ChanUsers(channel);
  nac.checkquiet = (flags & CU_QUIET) ? 1 : 0;
  notify_anything(player, na_channel, &nac,
		  (flags & CU_HIDE) ? ns_esnotify : NULL, 0, tbuf1);
  channel_push_buffer(channel, tbuf1);
}

static void
channel_push_buffer(CHAN *c, char *tbuf1)
{
  int len = strlen(tbuf1);
  int room = len + sizeof(len) + sizeof(time_t) + 1;
  if (!ChanBuffer(c))
    return;
  if ((ChanBufferEnd(c) > ChanBuffer(c)) &&
      ((ChanBufferSize(c) - (ChanBufferEnd(c) - ChanBuffer(c))) < room))
    channel_shift_buffer(c, room);
  memcpy(ChanBufferEnd(c), &len, sizeof(len));
  ChanBufferEnd(c) += sizeof(len);
  memcpy(ChanBufferEnd(c), &mudtime, sizeof(time_t));
  ChanBufferEnd(c) += sizeof(time_t);
  memcpy(ChanBufferEnd(c), tbuf1, len + 1);
  ChanBufferEnd(c) += len + 1;
  ChanNumBuffered(c)++;
}

static void
channel_shift_buffer(CHAN *c, int space_needed)
{
  char *p = ChanBuffer(c);
  int size, jump;
  int skipped = 0;

  while (*p && space_needed > 0) {
    /* First 4 bytes is the size of the first string, not including \0 */
    memcpy(&size, p, sizeof(size));
    /* Jump to the start of the next string */
    jump = size + sizeof(size) + sizeof(time_t) + 1;
    p += jump;
    space_needed -= jump;
    skipped++;
  }

  if (*p) {
    /* Shift everything here and after up to the front */
    memmove(ChanBuffer(c), p, ChanBufferEnd(c) - p);
    ChanBufferEnd(c) -= (p - ChanBuffer(c));
    ChanNumBuffered(c) -= skipped;
  }
}

/* Size of buffer in lines */
static int
channel_buffer_lines(CHAN *c)
{
  if (ChanBuffer(c))
    return ChanBufferSize(c) / (BUFFER_LEN + sizeof(int) + sizeof(time_t));
  else
    return 0;
}

/* Allocate a channel buffer and return the size in bytes.
 * The channel buffer should hold enough space for the given
 * number of lines (in full) plus an initial int for each line
 * giving its size
 */
static void
channel_allocate_buffer(CHAN *c, int lines)
{
  int bytes = lines * (BUFFER_LEN + sizeof(int) + sizeof(time_t));
  ChanBuffer(c) = (char *) malloc(bytes);
  *ChanBuffer(c) = '\0';
  ChanBufferEnd(c) = ChanBuffer(c);
  ChanNumBuffered(c) = 0;
  ChanBufferSize(c) = bytes;
}

static void
channel_reallocate_buffer(CHAN *c, int lines)
{
  int bytes = lines * (BUFFER_LEN + sizeof(int) + sizeof(time_t));
  /* If we were accidentally called without a buffer, deal */
  if (!ChanBuffer(c)) {
    channel_allocate_buffer(c, lines);
    return;
  }
  /* Are we not changing size? */
  if (ChanBufferSize(c) == bytes)
    return;
  if (ChanBufferSize(c) > bytes) {
    /* Shrinking the buffer */
    if ((ChanBufferEnd(c) - ChanBuffer(c)) >= bytes)
      channel_shift_buffer(c, ChanBufferEnd(c) - ChanBuffer(c) - bytes);
  }
  ChanBuffer(c) = (char *) realloc(ChanBuffer(c), bytes);
  ChanBufferSize(c) = bytes;
}

static void
channel_free_buffer(CHAN *c)
{
  if (ChanBuffer(c))
    free(ChanBuffer(c));
  ChanBuffer(c) = ChanBufferEnd(c) = NULL;
  ChanBufferSize(c) = 0;
  ChanNumBuffered(c) = 0;
}


/** Recall past lines from the channel's buffer.
 * We try to recall no more lines that are requested by the player,
 * but sometimes we may have fewer to recall.
 * \verbatim
 * This is the top-level function for @chan/recall.
 * \endverbatim
 * \param player the enactor.
 * \param name the name of the channel.
 * \param lines a string given the number of lines to recall (default all).
 */
void
do_chan_recall(dbref player, const char *name, const char *lines)
{
  CHAN *chan;
  CHANUSER *u;
  int num_lines = INT_MAX;
  char *p;
  int size;
  time_t timestamp;
  char tbuf1[BUFFER_LEN], *stamp;

  if (!name || !*name) {
    notify(player, T("You need to specify a channel."));
    return;
  }
  if (lines && *lines) {
    if (is_integer(lines))
      num_lines = parse_integer(lines);
    else {
      notify(player, T("How many lines did you want to recall?"));
      return;
    }
  }
  if (num_lines < 1) {
    notify(player, T("How many lines did you want to recall?"));
    return;
  }

  test_channel(player, name, chan);
  if (!Chan_Can_See(chan, player)) {
    if (onchannel(player, chan))
      notify_format(player,
		    T("CHAT: You can't do that with channel <%s>."),
		    ChanName(chan));
    else
      notify(player, T("CHAT: I don't recognize that channel."));
    return;
  }
  u = onchannel(player, chan);
  if (!u) {
    notify(player, T("CHAT: You must join a channel to recall from it."));
    return;
  }
  if (!ChanBuffer(chan)) {
    notify(player, T("CHAT: That channel doesn't have a recall buffer."));
    return;
  }
  if (ChanBuffer(chan) == ChanBufferEnd(chan)) {
    notify(player, T("CHAT: Nothing to recall."));
    return;
  }

  p = ChanBuffer(chan);
  /* Do we need to skip any lines? */
  if (num_lines < ChanNumBuffered(chan)) {
    int skip = ChanNumBuffered(chan) - num_lines;
    while (skip) {
      memcpy(&size, p, sizeof(size));
      p += sizeof(size) + sizeof(time_t) + size + 1;
      skip--;
    }
  }
  /* Now show everything from this point on */
  while (p < ChanBufferEnd(chan)) {
    memcpy(&size, p, sizeof(size));
    p += sizeof(size);
    memcpy(&timestamp, p, sizeof(time_t));
    stamp = (char *) ctime(&timestamp);
    stamp[strlen(stamp) - 1] = '\0';
    p += sizeof(time_t);
    memcpy(tbuf1, p, size + 1);
    notify_format(player, "%s %s", stamp, tbuf1);
    p += size + 1;
  }
}

/** Set the size of a channel's buffer in maximum lines.
 * \verbatim
 * This is the top-level function for @chan/buffer.
 * \endverbatim
 * \param player the enactor.
 * \param name the name of the channel.
 * \param lines a string given the number of lines to buffer.
 */
void
do_chan_buffer(dbref player, const char *name, const char *lines)
{
  CHAN *chan;
  int size;

  if (!name || !*name) {
    notify(player, T("You need to specify a channel."));
    return;
  }
  if (!lines || !*lines || !is_integer(lines)) {
    notify(player, T("You need to specify the number of lines to buffer."));
    return;
  }
  size = parse_integer(lines);
  if (size < 0 || size > 10) {
    notify(player, T("Invalid buffer size."));
    return;
  }
  test_channel(player, name, chan);
  if (!Chan_Can_Modify(chan, player)) {
    notify(player, T("Permission denied."));
    return;
  }
  if (!size) {
    /* Remove a channel's buffer */
    if (ChanBuffer(chan)) {
      channel_free_buffer(chan);
      notify_format(player,
		    T("CHAT: Channel buffering disabled for channel <%s>."),
		    ChanName(chan));
    } else {
      notify_format(player,
		    T
		    ("CHAT: Channel buffering already disabled for channel <%s>."),
		    ChanName(chan));
    }
  } else {
    if (ChanBuffer(chan)) {
      /* Resize a buffer */
      channel_reallocate_buffer(chan, size);
    } else {
      /* Start a new buffer */
      channel_allocate_buffer(chan, size);
      notify_format(player,
		    T("CHAT: Buffering enabled on channel <%s>."),
		    ChanName(chan));
    }
  }
}



#endif				/* CHAT_SYSTEM */
