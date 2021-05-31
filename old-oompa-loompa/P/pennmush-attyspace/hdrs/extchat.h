/*------------------------------------------------------------------
 * Header file for Javelin's extended @chat system
 * Based on the Battletech MUSE comsystem ported to PennMUSH by Kalkin
 *
 * Why:
 *  In the old system, channels were represented by bits set in a
 *  4-byte int on the db object. This had disadvantages - a limit
 *  of 32 channels, and players could find themselves on null channels.
 *  In addition, the old system required recompiles to permanently
 *  add channels, since the chaninfo was in the source.
 * How:
 *  Channels are a structure in a linked list.
 *  Each channel stores a whole bunch of info, including who's
 *  on it.
 *  We read/write this list using a chatdb file.
 *  We also maintain a linked list of channels that the user is
 *   connected to on the db object, which we set up at load time.
 *
 * User interface:
 * @chat channel = message
 * +channel message
 * @channel/on channel [= player] (or @channel channel = on)  do_channel()
 * @channel/off channel [= player] do_channel()
 * @channel/who channel do_channel()
 * @channel/title channel=title do_chan_title()
 * @channel/list do_chan_list()
 * @channel/add channel do_chan_admin()
 * @channel/priv channel = <privlist>  do_chan_admin()
 *  Privlist being: wizard, admin, private, moderated, etc.
 * @channel/joinlock channel = lock
 * @channel/speaklock channel = lock
 * @channel/modlock channel = lock
 * @channel/delete channel
 * @channel/quiet channel = yes/no
 * @channel/wipe channel
 *
 *------------------------------------------------------------------*/

#ifndef __EXTCHAT_H
#define __EXTCHAT_H

#ifdef CHAT_SYSTEM

#define CU_TITLE_LEN 80

/* This is a user - someone who's listening on a chat channel */
struct chanuser {
  dbref who;
  long int type;
  char title[CU_TITLE_LEN];
  struct chanuser *next;
};
typedef struct chanuser CHANUSER;

/* Flags and macros for channel users */
#define CU_QUIET    0x1		/* Do not hear connection messages */
#define CU_HIDE     0x2		/* Do not appear on the user list */
#define CU_GAG	    0x4		/* Do not hear any messages */
#define CU_DEFAULT_FLAGS 0x0

#define CUdbref(u) ((u)->who)
#define CUtype(u) ((u)->type)
#define CUtitle(u) ((u)->title)
#define CUnext(u) ((u)->next)
#define Chanuser_Quiet(u)	(CUtype(u) & CU_QUIET)
#define Chanuser_Hide(u) ((CUtype(u) & CU_HIDE) || hidden(CUdbref(u)))
#define Chanuser_Gag(u) (CUtype(u) & CU_GAG)

/* This is a chat channel */
#define CHAN_NAME_LEN 31
#define CHAN_TITLE_LEN 256
struct channel {
  char name[CHAN_NAME_LEN];	/* Channel name */
  char title[CHAN_TITLE_LEN];	/* Channel description */
  long int type;		/* Channel flags */
  long int cost;		/* Anyone can make a channel, but this is the cost */
  long int creator;		/* This is who paid the cost for the channel */
  long int num_users;		/* Number of connected users */
  long int max_users;		/* Maximum allocated users */
  struct chanuser *users;	/* Linked list of who is on */
  long int num_messages;	/* How many messages handled by this chan since startup */
  struct boolexp *joinlock;	/* Who may join */
  struct boolexp *speaklock;	/* Who may speak */
  struct boolexp *modifylock;	/* Who may change things and boot people */
  struct boolexp *seelock;	/* Who can see this in a list */
  struct boolexp *hidelock;	/* Who may hide from view */
  struct channel *next;		/* Next in linked list */
};
typedef struct channel CHAN;

struct chanlist {
  CHAN *chan;
  struct chanlist *next;
};
typedef struct chanlist CHANLIST;
#define Chanlist(x) db[(x)].channels

/* Channel type flags and macros */
#define CHANNEL_PLAYER  0x1	/* Players may join */
#define CHANNEL_OBJECT	0x2	/* Objects may join */
#define CHANNEL_DISABLED 0x4	/* Channel is turned off */
#define CHANNEL_QUIET	0x8	/* No broadcasts connect/disconnect */
#define CHANNEL_ADMIN   0x10	/* Wizard and royalty only ok */
#define CHANNEL_WIZARD  0x20	/* Wizard only ok */
#define CHANNEL_CANHIDE 0x40	/* Can non-DARK Wizards hide here? */
#define CHANNEL_DEFAULT_FLAGS	(CHANNEL_PLAYER)
#define CL_JOIN 0x1
#define CL_SPEAK 0x2
#define CL_MOD 0x4
#define CL_SEE 0x8
#define CL_HIDE 0x10
#define CHANNEL_COST (options.chan_cost)
#define MAX_PLAYER_CHANS (options.max_player_chans)
#define MAX_CHANNELS (options.max_channels)

#define ChanName(c) ((c)->name)
#define ChanType(c) ((c)->type)
#define ChanTitle(c) ((c)->title)
#define ChanCreator(c) ((c)->creator)
#define ChanCost(c) ((c)->cost)
#define ChanNumUsers(c) ((c)->num_users)
#define ChanMaxUsers(c) ((c)->max_users)
#define ChanUsers(c) ((c)->users)
#define ChanNext(c) ((c)->next)
#define ChanNumMsgs(c) ((c)->num_messages)
#define ChanJoinLock(c) ((c)->joinlock)
#define ChanSpeakLock(c) ((c)->speaklock)
#define ChanModLock(c) ((c)->modifylock)
#define ChanSeeLock(c) ((c)->seelock)
#define ChanHideLock(c) ((c)->hidelock)
#define Channel_Quiet(c)	(ChanType(c) & CHANNEL_QUIET)
#define Channel_Object(c) (ChanType(c) & CHANNEL_OBJECT)
#define Channel_Player(c) (ChanType(c) & CHANNEL_PLAYER)
#define Channel_Disabled(c) (ChanType(c) & CHANNEL_DISABLED)
#define Channel_Wizard(c) (ChanType(c) & CHANNEL_WIZARD)
#define Channel_Admin(c) (ChanType(c) & CHANNEL_ADMIN)
#define Channel_CanHide(c) (ChanType(c) & CHANNEL_CANHIDE)
#define Chan_Ok_Type(c,o) \
	((IsPlayer(o) && Channel_Player(c)) || \
	 (IsThing(o) && Channel_Object(c)))
#define Chan_Can(p,t) \
     (!(t & CHANNEL_DISABLED) && (!(t & CHANNEL_WIZARD) || Wizard(p)) && \
      (!(t & CHANNEL_ADMIN) || Hasprivs(p) || (Powers(p) & CHAT_PRIVS)))
/* Who can change channel privileges to type t */
#define Chan_Can_Priv(p,t) (Wizard(p) || Chan_Can(p,t))
#define Chan_Can_Access(c,p) (Chan_Can(p,ChanType(c)))
#define Chan_Can_Join(c,p) \
     (Chan_Can_Access(c,p) && \
     (eval_boolexp(p,ChanJoinLock(c),p,0,"ChanJoinlock")))
#define Chan_Can_Speak(c,p) \
     (Chan_Can_Access(c,p) && \
     (eval_boolexp(p,ChanSpeakLock(c),p,0,"ChanSpeaklock")))
#define Chan_Can_Modify(c,p) \
     (Wizard(p) || (ChanCreator(c) == (p)) || \
     (!Guest(p) && Chan_Can_Access(c,p) && \
     (eval_boolexp(p,ChanModLock(c),p,0,"ChanModlock"))))
#define Chan_Can_See(c,p) \
     (Wizard(p) || (Chan_Can_Access(c,p) && \
     (eval_boolexp(p,ChanSeeLock(c),p,0,"ChanSeelock"))))
#define Chan_Can_Hide(c,p) \
     (Can_Hide(p) || (Channel_CanHide(c) && Chan_Can_Access(c,p) && \
     (eval_boolexp(p,ChanHideLock(c),p,0,"ChanHidelock"))))
#define Chan_Can_Nuke(c,p) (Wizard(p) || (ChanCreator(c) == (p)))
#define Chan_Can_Decomp(c,p) (See_All(p) || (ChanCreator(c) == (p)))



     /* For use in channel matching */
#define CMATCH_NONE 0
#define CMATCH_EXACT 1
#define CMATCH_PARTIAL 2
#define CMATCH_AMBIG 3
#define CMATCHED(i) (((i) == CMATCH_EXACT) | ((i) == CMATCH_PARTIAL))

     /* Some globals */
extern int num_channels;
extern void channel_broadcast _((CHAN *channel, dbref player, int flags, const char *fmt,...));
extern CHANUSER *onchannel _((dbref who, CHAN *c));
extern void init_chatdb _((void));
extern int load_chatdb _((FILE * fp));
extern int save_chatdb _((FILE * fp));
extern void do_cemit _((dbref player, const char *name, const char *msg, int noisy));
extern void do_chan_user_flags _((dbref player, char *name, const char *isyn, int flag, int silent));
extern void do_chan_wipe _((dbref player, const char *name));
extern void do_chan_lock _((dbref player, const char *name, const char *lockstr, int whichlock));
extern void do_chan_what _((dbref player, const char *partname));
extern void do_chan_desc _((dbref player, const char *name, const char *title));
extern void do_chan_title _((dbref player, const char *name, const char *title));

#endif				/* CHAT_SYSTEM */
#endif				/* __EXTCHAT_H */
