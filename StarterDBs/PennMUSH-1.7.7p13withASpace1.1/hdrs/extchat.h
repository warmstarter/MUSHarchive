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
 * @channel/buffer channel = <maxlines>
 * @channel/recall channel [= <lines>]
 *
 *------------------------------------------------------------------*/

#ifndef __EXTCHAT_H
#define __EXTCHAT_H

#ifdef CHAT_SYSTEM

#define CU_TITLE_LEN 80

/** A channel user.
 * This structure represents an object joined to a chat channel.
 * Each chat channel maintains a linked list of users.
 */
struct chanuser {
  dbref who;			/**< Dbref of joined object */
  long int type;		/**< Bitflags for this user */
  char title[CU_TITLE_LEN];	/**< User's channel title */
  struct chanuser *next;	/**< Pointer to next user in list */
};

/* Flags and macros for channel users */
#define CU_QUIET    0x1		/* Do not hear connection messages */
#define CU_HIDE     0x2		/* Do not appear on the user list */
#define CU_GAG      0x4		/* Do not hear any messages */
#define CU_DEFAULT_FLAGS 0x0

#define CUdbref(u) ((u)->who)
#define CUtype(u) ((u)->type)
#define CUtitle(u) ((u)->title)
#define CUnext(u) ((u)->next)
#define Chanuser_Quiet(u)       (CUtype(u) & CU_QUIET)
#define Chanuser_Hide(u) ((CUtype(u) & CU_HIDE) || (IsPlayer(CUdbref(u)) && hidden(CUdbref(u))))
#define Chanuser_Gag(u) (CUtype(u) & CU_GAG)

/* This is a chat channel */
#define CHAN_NAME_LEN 31
#define CHAN_TITLE_LEN 256
/** A chat channel.
 * This structure represents a MUSH chat channel. Channels are organized
 * into a sorted linked list.
 */
struct channel {
  char name[CHAN_NAME_LEN];	/**< Channel name */
  char title[CHAN_TITLE_LEN];	/**< Channel description */
  long int type;		/**< Channel flags */
  long int cost;		/**< What it cost to make this channel */
  long int creator;		/**< This is who paid the cost for the channel */
  long int num_users;		/**< Number of connected users */
  long int max_users;		/**< Maximum allocated users */
  struct chanuser *users;	/**< Linked list of current users */
  long int num_messages;	/**< How many messages handled by this chan since startup */
  struct boolexp *joinlock;	/**< Who may join */
  struct boolexp *speaklock;	/**< Who may speak */
  struct boolexp *modifylock;	/**< Who may change things and boot people */
  struct boolexp *seelock;	/**< Who can see this in a list */
  struct boolexp *hidelock;	/**< Who may hide from view */
  struct channel *next;		/**< Next channel in linked list */
  char *buffer;			/**< Pointer to channel recall buffer start */
  char *buffer_end;		/**< Pointer to channel recall buffer end */
  int buffersize;		/**< Size of channel recall buffer */
  int num_buffered;		/**< Number of lines in the buffer */
};

/** A list of channels on an object.
 * This structure is a linked list of channels that is associated
 * with each object
 */
struct chanlist {
  CHAN *chan;			/**< Channel data */
  struct chanlist *next;	/**< Next channel in list */
};

#define Chanlist(x) ((struct chanlist *)get_objdata(x, "CHANNELS"))
#define s_Chanlist(x, y) set_objdata(x, "CHANNELS", (void *)y)

/* Channel type flags and macros */
#define CHANNEL_PLAYER  0x1	/* Players may join */
#define CHANNEL_OBJECT  0x2	/* Objects may join */
#define CHANNEL_DISABLED 0x4	/* Channel is turned off */
#define CHANNEL_QUIET   0x8	/* No broadcasts connect/disconnect */
#define CHANNEL_ADMIN   0x10	/* Wizard and royalty only ok */
#define CHANNEL_WIZARD  0x20	/* Wizard only ok */
#define CHANNEL_CANHIDE 0x40	/* Can non-DARK Wizards hide here? */
#define CHANNEL_OPEN    0x80	/* Can you speak if you're not joined? */
#define CHANNEL_DEFAULT_FLAGS   (CHANNEL_PLAYER)
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
#define ChanBuffer(c) ((c)->buffer)
#define ChanBufferSize(c) ((c)->buffersize)
#define ChanBufferEnd(c) ((c)->buffer_end)
#define ChanNumBuffered(c) ((c)->num_buffered)
#define Channel_Quiet(c)        (ChanType(c) & CHANNEL_QUIET)
#define Channel_Open(c) (ChanType(c) & CHANNEL_OPEN)
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
     (eval_boolexp(p,ChanJoinLock(c),p)))
#define Chan_Can_Speak(c,p) \
     (Chan_Can_Access(c,p) && \
     (eval_boolexp(p,ChanSpeakLock(c),p)))
#define Chan_Can_Modify(c,p) \
     (Wizard(p) || (ChanCreator(c) == (p)) || \
     (!Guest(p) && Chan_Can_Access(c,p) && \
     (eval_boolexp(p,ChanModLock(c),p))))
#define Chan_Can_See(c,p) \
     (Hasprivs(p) || See_All(p) || (Chan_Can_Access(c,p) && \
     (eval_boolexp(p,ChanSeeLock(c),p))))
#define Chan_Can_Hide(c,p) \
     (Can_Hide(p) || (Channel_CanHide(c) && Chan_Can_Access(c,p) && \
     (eval_boolexp(p,ChanHideLock(c),p))))
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
extern void WIN32_CDECL channel_broadcast
  (CHAN *channel, dbref player, int flags, const char *fmt, ...)
  __attribute__ ((__format__(__printf__, 4, 5)));
extern CHANUSER *onchannel(dbref who, CHAN *c);
extern void init_chatdb(void);
extern int load_chatdb(FILE * fp);
extern int save_chatdb(FILE * fp);
extern void do_cemit
  (dbref player, const char *name, const char *msg, int noisy);
extern void do_chan_user_flags
  (dbref player, char *name, const char *isyn, int flag, int silent);
extern void do_chan_wipe(dbref player, const char *name);
extern void do_chan_lock
  (dbref player, const char *name, const char *lockstr, int whichlock);
extern void do_chan_what(dbref player, const char *partname);
extern void do_chan_desc(dbref player, const char *name, const char *title);
extern void do_chan_title(dbref player, const char *name, const char *title);
extern void do_chan_recall(dbref player, const char *name, const char *lines);
extern void do_chan_buffer(dbref player, const char *name, const char *lines);
extern void init_chat(void);
extern void do_channel
  (dbref player, const char *name, const char *target, const char *com);
extern void do_chat(dbref player, CHAN *chan, const char *arg1);
extern void do_chan_admin
  (dbref player, char *name, const char *perms, int flag);
extern int find_channel(const char *p, CHAN **chan, dbref player);
extern int find_channel_partial(const char *p, CHAN **chan, dbref player);
extern void do_channel_list(dbref player, const char *partname);
extern int do_chat_by_name
  (dbref player, const char *name, const char *msg, int source);
extern void do_chan_decompile(dbref player, const char *name, int brief);
extern void do_chan_chown(dbref player, const char *name, const char *newowner);
extern const char *channel_description(dbref player);


#endif				/* CHAT_SYSTEM */
#endif				/* __EXTCHAT_H */
