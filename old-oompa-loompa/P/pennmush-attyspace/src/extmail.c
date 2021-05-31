
/*---------------------------------------------------------------
 * extmail.c - Javelin's improved @mail system
 * Based on Amberyl's linked list mailer
 *
 * Summary of mail command syntax:
 * Sending:
 *   @mail[/sendswitch] player-list = message
 *     sendswitches: /silent, /urgent
 *     player-list is a space-separated list of players or msg#'s to reply
 *       to. Players can be names or dbrefs
 * Reading/Handling:
 *   @mail[/readswitch] [msg-list [= target]]
 *     With no readswitch, @mail reads msg-list (same as /read)
 *     With no readswitch and no msg-list, @mail lists all messages (/list)
 *     readswitches: /list, /read, /fwd (requires target list of players
 *	to forward messages to), /file (requires target folder to file
 *	to), /tag, /untag, /clear, /unclear, /purge (no msg-list),
 *	/count
 *     Assumes messages in current folder, set by @folder or @mail/folder
 *     msg-list can be one of: a message number, a message range,
 *     (2-3, 4-, -6), sender references (*player), date comparisons
 *     (~0, >2, <5), or the strings "urgent", "tagged", "cleared",
 *     "read", "unread", "all", or "folder"
 *     You can also use 1:2 (folder 1, message 2) and 1:2-3 for ranges.
 * Admin stuff:
 *   @mail[/switch] [player]
 *     Switches include: nuke (used to be "purge"), [efd]stats, debug
 *
 * THEORY OF OPERATION:
 *  Prior to pl11, mail was an unsorted linked list. When mail was sent,
 * it was added onto the end. To read mail, you scanned the whole list.
 * This is still how origmail.c works.
 *  As of pl11, extmail.c maintains mail as a sorted linked list, sorted
 * by recipient and order of receipt. This makes sending mail less
 * efficient (because you have to scan the list to figure out where to
 * insert), but reading/checking/deleting more efficient, 
 * because once you've found where the player's mail starts, you just
 * read from there. 
 *  That wouldn't be so exciting unless there was a fast way to find
 * where a player's mail chain started. Fortunately, there is. We
 * record that information for connected players when they connect,
 * on their descriptor. So, when connected players do reading/etc,
 * it's O(1). Sending to a connected player is O(1). Sending to an
 * unconnected player still requires scanning (O(n)), but you send once, 
 * and read/list/delete etc, multiple times.
 *  And just to make the sending to disconnected players faster, 
 * instead of scanning the whole maildb to find the insertion point,
 * we start the scan from the chain of the connected player with the
 * closest db# to the target player. This scales up very well.
 *--------------------------------------------------------------------*/

#include "config.h"
#include "copyrite.h"

#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <ctype.h>
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif

#include "conf.h"
#include "mushdb.h"
#include "dbdefs.h"
#include "intrface.h"
#include "externs.h"
#include "match.h"
#include "extmail.h"
#include "parse.h"
#include "mymalloc.h"
#include "pueblo.h"
#include "confmagic.h"

#ifdef USE_MAILER
extern void desc_mail_set _((dbref player, struct mail * mp));	/* bsd.c */
extern int do_convtime _((char *str, struct tm * ttm));		/* funtime.c */

void do_mail_change_folder _((dbref player, char *fld, char *newname));
void do_mail_unfolder _((dbref player, char *fld));
void do_mail_tag _((dbref player, char *msglist));
void do_mail_clear _((dbref player, char *msglist));
void do_mail_untag _((dbref player, char *msglist));
void do_mail_unclear _((dbref player, char *msglist));
static void do_mail_flags _((dbref player, char *msglist, mail_flag flag, int negate));
void do_mail_file _((dbref player, char *msglist, char *folder));
void do_mail_read _((dbref player, char *msglist));
void do_mail_list _((dbref player, const char *msglist));
static char *mail_list_time _((const char *the_time, int flag));
void do_mail_purge _((dbref player));
void do_mail_fwd _((dbref player, char *msglist, char *tolist));
void do_mail_send _((dbref player, char *tolist, char *message, mail_flag flags, int silent));
static struct mail *mail_fetch _((dbref player, int num));
static struct mail *real_mail_fetch _((dbref player, int num, int folder));
static struct mail *mailfun_fetch _((dbref player, int nargs, char *arg1, char *arg2));
static void count_mail _((dbref player, int folder, int *rcount, int *ucount, int *ccount));
#ifdef MAIL_SUBJECTS
static void send_mail _((dbref player, dbref target, unsigned char *subject, char *message, mail_flag flags, int silent));
#else
static void send_mail _((dbref player, dbref target, char *message, mail_flag flags, int silent));
#endif
void do_mail_nuke _((dbref player));
void do_mail_debug _((dbref player, char *action, char *victim));
void do_mail_stats _((dbref player, char *name, int full));
void do_mail _((dbref player, char *arg1, char *arg2));
int dump_mail _((FILE * fp));
struct mail *find_exact_starting_point _((dbref player));
static struct mail *find_insertion_point _((dbref player));
void mail_init _((void));
int load_mail _((FILE * fp));
static int get_folder_number _((dbref player, char *name));
static char *get_folder_name _((dbref player, int fld));
void add_folder_name _((dbref player, int fld, const char *name));
static int player_folder _((dbref player));
void set_player_folder _((dbref player, int fnum));
static int parse_folder _((dbref player, char *folder_string));
static int mail_match _((dbref player, struct mail * mp, struct mail_selector ms, int num));
static int parse_msglist _((const char *msglist, struct mail_selector * ms, dbref player));
static int parse_message_spec _((dbref player, const char *s, int *msglow, int *msghigh, int *folder));
static char *status_chars _((struct mail * mp));
static char *status_string _((struct mail * mp));
void check_mail _((dbref player, int folder, int silent));
static int sign _((int x));
#ifdef MAIL_SUBJECTS
static char *get_subject _((struct mail * mp));
#endif

struct mail *maildb;
struct mail *tail_ptr;

#define HEAD  maildb
#define TAIL  tail_ptr

#define DASH_LINE  \
  "-----------------------------------------------------------------------------"

int mdb_top = 0;		/* total number of messages */

/*-------------------------------------------------------------------------*
 *   User mail functions (these are called from game.c)
 *
 * do_mail - cases without a /switch.
 * do_mail_send - sending mail
 * do_mail_read - read messages
 * do_mail_list - list messages
 * do_mail_flags - tagging, untagging, clearing, unclearing of messages
 * do_mail_file - files messages into a new folder
 * do_mail_fwd - forward messages to another player(s)
 * do_mail_count - count messages
 * do_mail_purge - purge cleared messages
 * do_mail_change_folder - change current folder
 * do_mail_unfolder - remove a folder name from MAILFOLDERS
 * do_mail_subject - set the current mail subject
 *-------------------------------------------------------------------------*/

#ifdef MAIL_SUBJECTS
/* Return the subject of a mail message, or (no subject) */
static char *
get_subject(mp)
    struct mail *mp;
{
  static char sbuf[SUBJECT_LEN + 1];
  char *p;
  if (mp->subject) {
    strncpy(sbuf, uncompress(mp->subject), SUBJECT_LEN);
    sbuf[SUBJECT_LEN] = '\0';
    /* Stop at a return or a tab */
    for (p = sbuf; *p; p++) {
      if ((*p == '\r') || (*p == '\n') || (*p == '\t')) {
	*p = '\0';
	break;
      }
      if (!isprint(*p)) {
	*p = ' ';
      }
    }
  } else
    strcpy(sbuf, "(no subject)");
  return sbuf;
}
#endif


/* Change or rename a folder */
void
do_mail_change_folder(player, fld, newname)
    dbref player;
    char *fld;
    char *newname;
{
  int pfld;
  char *p;

  if (!fld || !*fld) {
    /* Check mail in all folders */
    for (pfld = MAX_FOLDERS; pfld >= 0; pfld--)
      check_mail(player, pfld, 1);
    pfld = player_folder(player);
    notify(player, tprintf("MAIL: Current folder is %d [%s].", pfld, get_folder_name(player, pfld)));
    return;
  }
  pfld = parse_folder(player, fld);
  if (pfld < 0) {
    notify(player, "MAIL: What folder is that?");
    return;
  }
  if (newname && *newname) {
    /* We're changing a folder name here */
    if (strlen(newname) > FOLDER_NAME_LEN) {
      notify(player, "MAIL: Folder name too long");
      return;
    }
    for (p = newname; p && *p; p++) {
      if (!isdigit(*p) && !isalpha(*p)) {
	notify(player, "MAIL: Illegal folder name");
	return;
      }
    }
    add_folder_name(player, pfld, newname);
    notify(player, tprintf("MAIL: Folder %d now named '%s'", pfld, newname));
  } else {
    /* Set a new folder */
    set_player_folder(player, pfld);
    notify(player, tprintf("MAIL: Current folder set to %d [%s].", pfld, get_folder_name(player, pfld)));
  }
}

/* Remove a folder name from MAILFOLDERS */
void
do_mail_unfolder(player, fld)
    dbref player;
    char *fld;
{
  int pfld;

  if (!fld || !*fld) {
    notify(player, "MAIL: You must specify a folder name or number");
    return;
  }
  pfld = parse_folder(player, fld);
  if (pfld < 0) {
    notify(player, "MAIL: What folder is that?");
    return;
  }
  add_folder_name(player, pfld, NULL);
  notify(player, tprintf("MAIL: Folder %d now has no name", pfld));
}


void
do_mail_tag(player, msglist)
    dbref player;
    char *msglist;
{
  do_mail_flags(player, msglist, M_TAG, 0);
}

void
do_mail_clear(player, msglist)
    dbref player;
    char *msglist;
{
  do_mail_flags(player, msglist, M_CLEARED, 0);
}

void
do_mail_untag(player, msglist)
    dbref player;
    char *msglist;
{
  do_mail_flags(player, msglist, M_TAG, 1);
}

void
do_mail_unclear(player, msglist)
    dbref player;
    char *msglist;
{
  do_mail_flags(player, msglist, M_CLEARED, 1);
}


static void
do_mail_flags(player, msglist, flag, negate)
    dbref player;
    char *msglist;
    mail_flag flag;
    int negate;
{
  /* Set (negate=0) or clear (negate=1) a flag on a player's messages */

  struct mail *mp;
  struct mail_selector ms;
  int j, folder;
  folder_array i;
  int notified = 0;

  if (!parse_msglist(msglist, &ms, player)) {
    return;
  }
  FA_Init(i, j);
  j = 0;
  folder = MSFolder(ms) ? MSFolder(ms) : player_folder(player);
  for (mp = find_exact_starting_point(player);
       mp && (mp->to == player);
       mp = mp->next) {
    if ((mp->to == player) && (All(ms) || (Folder(mp) == folder))) {
      i[Folder(mp)]++;
      if (mail_match(player, mp, ms, i[Folder(mp)])) {
	j++;
	if (negate) {
	  mp->read &= ~flag;
	} else {
	  mp->read |= flag;
	}
	switch (flag) {
	case M_TAG:
	  if (All(ms)) {
	    if (!notified) {
	      notify(player, tprintf("MAIL: All messages in all folders %s.",
				     negate ? "untagged" : "tagged"));
	      notified++;
	    }
	  } else
	    notify(player, tprintf("MAIL: Msg #%d:%d %s.", Folder(mp), i[Folder(mp)],
				   negate ? "untagged" : "tagged"));
	  break;
	case M_CLEARED:
	  if (All(ms)) {
	    if (!notified) {
	      notify(player, tprintf("MAIL: All messages in all folders %s.",
				     negate ? "uncleared" : "cleared"));
	      notified++;
	    }
	  } else {
	    if (Unread(mp) && !negate) {
	      notify(player,
		     tprintf("MAIL: Unread Msg #%d:%d cleared! Use @mail/unclear %d:%d to recover.", Folder(mp), i[Folder(mp)], Folder(mp), i[Folder(mp)]));
	    } else {
	      notify(player, tprintf("MAIL: Msg #%d:%d %s.", Folder(mp), i[Folder(mp)],
				     negate ? "uncleared" : "cleared"));
	    }
	  }
	  break;
	}
      }
    }
  }
  if (!j) {
    /* ran off the end of the list without finding anything */
    notify(player, "MAIL: You don't have any matching messages!");
  }
  return;
}

void
do_mail_file(player, msglist, folder)
    dbref player;
    char *msglist;
    char *folder;
{
  /* Change a message's folder */
  struct mail *mp;
  struct mail_selector ms;
  int j, foldernum, origfold;
  folder_array i;
  int notified = 0;

  if (!parse_msglist(msglist, &ms, player)) {
    return;
  }
  if ((foldernum = parse_folder(player, folder)) == -1) {
    notify(player, "MAIL: Invalid folder specification");
    return;
  }
  FA_Init(i, j);
  j = 0;
  origfold = MSFolder(ms) ? MSFolder(ms) : player_folder(player);
  for (mp = find_exact_starting_point(player);
       mp && (mp->to == player);
       mp = mp->next) {
    if ((mp->to == player) && (All(ms) || (Folder(mp) == origfold))) {
      i[Folder(mp)]++;
      if (mail_match(player, mp, ms, i[Folder(mp)])) {
	j++;
	mp->read &= M_FMASK;	/* Clear the folder */
	mp->read |= FolderBit(foldernum);
	if (All(ms)) {
	  if (!notified) {
	    notify(player, tprintf("MAIL: All messages filed in folder %d [%s]", foldernum, get_folder_name(player, foldernum)));
	    notified++;
	  }
	} else
	  notify(player, tprintf("MAIL: Msg %d:%d filed in folder %d [%s]", origfold, i[origfold], foldernum, get_folder_name(player, foldernum)));
      }
    }
  }
  if (!j) {
    /* ran off the end of the list without finding anything */
    notify(player, "MAIL: You don't have any matching messages!");
  }
  return;
}

void
do_mail_read(player, msglist)
    dbref player;
    char *msglist;
{
  /* print a mail message(s) */

  struct mail *mp;
  char tbuf1[BUFFER_LEN];
  static char folderheader[BUFFER_LEN] = "Folder:";
  struct mail_selector ms;
  int j, folder;
  folder_array i;

  if (!parse_msglist(msglist, &ms, player)) {
    return;
  }
  folder = MSFolder(ms) ? MSFolder(ms) : player_folder(player);
  FA_Init(i, j);
  j = 0;
  for (mp = find_exact_starting_point(player);
       mp && (mp->to == player);
       mp = mp->next) {
    if ((mp->to == player) && (All(ms) || Folder(mp) == folder)) {
      i[Folder(mp)]++;
      if (mail_match(player, mp, ms, i[Folder(mp)])) {
	/* Read it */
	j++;
	if (SUPPORT_PUEBLO) {
	  notify_noenter(player, tprintf("%cPRE%c", TAG_START, TAG_END));
	  sprintf(folderheader, "%cA XCH_HINT=\"List messages in this folder\" XCH_CMD=\"@mail/list %d:1-\"%cFolder:%c/A%c",
		  TAG_START, Folder(mp), TAG_END, TAG_START, TAG_END);
	}
	notify(player, DASH_LINE);
	if (IsPlayer(mp->from))
	  sprintf(tbuf1, "%s", Name(mp->from));
	else
	  sprintf(tbuf1, "%s (owner: %s)", Name(mp->from), Name(Owner(mp->from)));
	notify(player, tprintf("From: %-55s %s\nDate: %-25s   %s %2d   Message: %d\nStatus: %s",
			       tbuf1,
			    (IsPlayer(mp->from) && Connected(mp->from) &&
			     (!hidden(mp->from) || Hasprivs(player))) ?
			       " (Conn)" : "      ",
			       uncompress(mp->time), folderheader,
			       Folder(mp), i[Folder(mp)],
			       status_string(mp)));
#ifdef MAIL_SUBJECTS
	notify(player, tprintf("Subject: %s", get_subject(mp)));
#endif
	notify(player, DASH_LINE);
	if (SUPPORT_PUEBLO)
	  notify_noenter(player, tprintf("%c/PRE%c", TAG_START, TAG_END));
	strcpy(tbuf1, uncompress(mp->message));
	notify(player, tbuf1);
	if (SUPPORT_PUEBLO)
	  notify(player, tprintf("%cPRE%c%s%c/PRE%c", TAG_START, TAG_END,
				 DASH_LINE, TAG_START, TAG_END));
	else
	  notify(player, DASH_LINE);
	if (Unread(mp))
	  mp->read |= M_READ;	/* mark message as read */
      }
    }
  }
  if (!j) {
    /* ran off the end of the list without finding anything */
    notify(player, "MAIL: You don't have that many matching messages!");
  }
  return;
}


void
do_mail_list(player, msglist)
    dbref player;
    const char *msglist;
{
  /* print a mail message(s) */
#ifdef MAIL_SUBJECTS
  char subj[30];
#endif
  struct mail *mp;
  struct mail_selector ms;
  int j, folder;
  folder_array i;

  if (!parse_msglist(msglist, &ms, player)) {
    return;
  }
  FA_Init(i, j);
  j = 0;
  folder = MSFolder(ms) ? MSFolder(ms) : player_folder(player);
  if (SUPPORT_PUEBLO)
    notify_noenter(player, tprintf("%cPRE%c", TAG_START, TAG_END));
  notify(player, tprintf(
			  "---------------------------   MAIL (folder %d)  ------------------------------", folder));
  for (mp = find_exact_starting_point(player);
       mp && (mp->to == player);
       mp = mp->next) {
    if ((mp->to == player) && (All(ms) || Folder(mp) == folder)) {
      i[Folder(mp)]++;
      if (mail_match(player, mp, ms, i[Folder(mp)])) {
	/* list it */
	if (SUPPORT_PUEBLO)
	  notify_noenter(player,
			 tprintf("%cA XCH_CMD=\"@mail/read %d:%d\" XCH_HINT=\"Read message %d in folder %d\"%c", TAG_START, Folder(mp), i[Folder(mp)], i[Folder(mp)], Folder(mp), TAG_END));
#ifdef MAIL_SUBJECTS
	strcpy(subj, chopstr(get_subject(mp), 28));
	notify(player, tprintf("[%s] %2d:%-3d %c%-12s  %-*s %s",
			     status_chars(mp), Folder(mp), i[Folder(mp)],
			       ((Connected(mp->from) &&
				 (!hidden(mp->from) || Hasprivs(player)))
				? '*' : ' '),
			       chopstr(Name(mp->from), 12),
			       30, subj,
			       mail_list_time(uncompress(mp->time), 1)));
#else
	notify(player, tprintf("[%s] %2d:%-3d (%4d) From: %-*s At: %s %s",
			       status_chars(mp), Folder(mp),
			  i[Folder(mp)], strlen(uncompress(mp->message)),
			       PLAYER_NAME_LIMIT + 1, Name(mp->from),
			       mail_list_time(uncompress(mp->time), 0),
			       ((Connected(mp->from) &&
				 (!hidden(mp->from) || Hasprivs(player)))
				? "Conn" : " ")));
#endif
	if (SUPPORT_PUEBLO)
	  notify_noenter(player, tprintf("%c/A%c", TAG_START, TAG_END));
      }
    }
  }
  notify(player, DASH_LINE);
  if (SUPPORT_PUEBLO)
    notify(player, tprintf("%c/PRE%c", TAG_START, TAG_END));
  return;
}

static char *
mail_list_time(the_time, flag)
    const char *the_time;
    int flag;			/* 1 for no year */
{
  static char newtime[BUFFER_LEN];
  char *p, *q;
  int i;
  p = (char *) the_time;
  q = newtime;
  if (!p || !*p)
    return NULL;
  /* Format of the_time is: day mon dd hh:mm:ss yyyy */
  /* Chop out :ss */
  for (i = 0; i < 16; i++) {
    if (*p)
      *q++ = *p++;
  }
  if (!flag) {
    for (i = 0; i < 3; i++) {
      if (*p)
	p++;
    }
    for (i = 0; i < 5; i++) {
      if (*p)
	*q++ = *p++;
    }
  }
  *q = '\0';
  return newtime;
}


void
do_mail_purge(player)
    dbref player;
{
  struct mail *mp, *nextp;

  /* Go through player's mail, and remove anything marked cleared */
  for (mp = find_exact_starting_point(player);
       mp && (mp->to == player);
       mp = nextp) {
    if ((mp->to == player) && Cleared(mp)) {
      /* Delete this one */
      /* head and tail of the list are special */
      if (mp == HEAD)
	HEAD = mp->next;
      else if (mp == TAIL)
	TAIL = mp->prev;
      /* relink the list */
      if (mp->prev != NULL)
	mp->prev->next = mp->next;
      if (mp->next != NULL)
	mp->next->prev = mp->prev;
      /* save the pointer */
      nextp = mp->next;
      /* then wipe */
      mdb_top--;
#ifdef MAIL_SUBJECTS
      if (mp->subject != mp->message)
	free(mp->subject);
#endif
      free(mp->message);
      free(mp->time);
      free(mp);
    } else {
      nextp = mp->next;
    }
  }
  /* Clean up the player's mailp */
  if (Connected(player)) {
    desc_mail_set(player, NULL);
    desc_mail_set(player, find_exact_starting_point(player));
  }
  notify(player, "MAIL: Mailbox purged.");
  return;
}

void
do_mail_fwd(player, msglist, tolist)
    dbref player;
    char *msglist;
    char *tolist;
{
  struct mail *mp;
  struct mail *last;
  struct mail_selector ms;
  int j, num, folder;
  folder_array i;
  char *head, *tail, spot;
  struct mail *temp;
  dbref target;
  int num_recpts = 0;

  if (!parse_msglist(msglist, &ms, player)) {
    return;
  }
  if (!tolist || !*tolist) {
    notify(player, "MAIL: To whom should I forward?");
    return;
  }
  folder = MSFolder(ms) ? MSFolder(ms) : player_folder(player);
  /* Mark the player's last message. This prevents a loop if
   * the forwarding command happens to forward a message back
   * to the player itself 
   */
  last = mp = find_exact_starting_point(player);
  if (!last) {
    notify(player, "MAIL: You have no messages to forward.");
    return;
  }
  while (last->next && (last->next->to == player))
    last = last->next;

  FA_Init(i, j);
  while (mp && (mp->to == player) && (mp != last->next)) {
    if ((mp->to == player) && (All(ms) || (Folder(mp) == folder))) {
      i[Folder(mp)]++;
      if (mail_match(player, mp, ms, i[Folder(mp)])) {
	/* forward it to all players listed */
	head = (char *) tolist;
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
	  /* Now locate a target */
	  num = atoi(head);
	  if (num) {
	    /* reply to a mail message */
	    temp = mail_fetch(player, num);
	    if (!temp) {
	      notify(player, "MAIL: You can't reply to nonexistant mail.");
	    } else {
#ifdef MAIL_SUBJECTS
	      send_mail(player, temp->from, mp->subject, (char *) mp->message, M_FORWARD | M_REPLY, 1);
#else
	      send_mail(player, temp->from, (char *) mp->message, M_FORWARD | M_REPLY, 1);
#endif
	      num_recpts++;
	    }
	  } else {
	    /* forwarding to a player */
	    target = match_result(player, head, TYPE_PLAYER, MAT_ME | MAT_ABSOLUTE | MAT_PLAYER);
	    if (!GoodObject(target))
	      target = lookup_player(head);
	    if (!GoodObject(target))
	      target = short_page(head);
	    if (!GoodObject(target) || !IsPlayer(target)) {
	      notify(player, tprintf("No such unique player: %s.", head));
	    } else {
#ifdef MAIL_SUBJECTS
	      send_mail(player, target, mp->subject, (char *) mp->message, M_FORWARD, 1);
#else
	      send_mail(player, target, (char *) mp->message, M_FORWARD, 1);
#endif
	      num_recpts++;
	    }
	  }
	  /* Get the next recip */
	  *tail = spot;
	  head = tail;
	  if (*head == '"')
	    head++;
	}
      }
    }
    mp = mp->next;
  }
  notify(player, tprintf("MAIL: %d messages forwarded.", num_recpts));
}

void
do_mail_send(player, tolist, message, flags, silent)
    dbref player;
    char *tolist;
    char *message;
    mail_flag flags;
    int silent;
{
  char *head, *tail, spot;
  int num;
  dbref target;
  int mail_flags;
#ifdef MAIL_SUBJECTS
  char sbuf[SUBJECT_LEN + 1], *sb, *mb;
  int i = 0, subject_given = 0;
#endif

  if (!tolist || !*tolist) {
    notify(player, "MAIL: I can't figure out who you want to mail to.");
    return;
  }
  if (!message || !*message) {
    notify(player, "MAIL: I can't figure out what you want to send.");
    return;
  }
#ifdef MAIL_SUBJECTS
  sb = sbuf;
  mb = message;			/* Save the message pointer */
  while (*message && (i < SUBJECT_LEN) && *message != SUBJECT_COOKIE) {
    *sb++ = *message++;
    i++;
  }
  *sb = '\0';
  if (*message && (*message == SUBJECT_COOKIE)) {
    message++;
    subject_given = 1;
  } else
    message = mb;		/* Rewind the pointer to the beginning */
#ifdef ALLOW_NOSUBJECT
  if (!subject_given)
    strcpy(sbuf, "(no subject)");
#endif
#endif
  /* Parse the player list */
  head = (char *) tolist;
  while (head && *head) {
    mail_flags = flags;
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
    *tail = '\0';
    /* Now locate a target */
    num = atoi(head);
    if (num) {
      /* reply to a mail message */
      struct mail *temp;

      temp = mail_fetch(player, num);
      if (!temp) {
	notify(player, "MAIL: You can't reply to nonexistant mail.");
	return;
      }
#ifdef MAIL_SUBJECTS
      if (subject_given)
	send_mail(player, temp->from, compress(sbuf), message, mail_flags, silent);
      else
	send_mail(player, temp->from, temp->subject, message,
		  mail_flags | M_REPLY, silent);
#else
      send_mail(player, temp->from, message, mail_flags, silent);
#endif
    } else {
      /* send a new mail message */
      target = match_result(player, head, TYPE_PLAYER, MAT_ME | MAT_ABSOLUTE | MAT_PLAYER);
      if (!GoodObject(target))
	target = lookup_player(head);
      if (!GoodObject(target))
	target = short_page(head);
      if (!GoodObject(target) || !IsPlayer(target))
	notify(player, tprintf("No such unique player: %s.", head));
      else
#ifdef MAIL_SUBJECTS
	send_mail(player, target, compress(sbuf), message, mail_flags, silent);
#else
	send_mail(player, target, message, mail_flags, silent);
#endif
    }
    /* Get the next recip */
    *tail = spot;
    head = tail;
    if (*head == '"')
      head++;
  }
}

/*-------------------------------------------------------------------------*
 *   Admin mail functions
 *
 * do_mail_nuke - clear & purge mail for a player, or all mail in db.
 * do_mail_stat - stats on mail for a player, or for all db.
 * do_mail_debug - fix mail with a sledgehammer
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 *   Basic mail functions
 *-------------------------------------------------------------------------*/
static struct mail *
mail_fetch(player, num)
    dbref player;
    int num;
{
  /* get an arbitrary mail message in the current folder */
  return real_mail_fetch(player, num, player_folder(player));
}

static struct mail *
real_mail_fetch(player, num, folder)
    dbref player;
    int num;
    int folder;
{
  struct mail *mp;
  int i = 0;

  for (mp = find_exact_starting_point(player); mp != NULL; mp = mp->next) {
    if (mp->to > player)
      break;
    if ((mp->to == player) && ((folder < 0) || (Folder(mp) == folder)))
      i++;
    if (i == num)
      return mp;
  }
  return NULL;
}


static void
count_mail(player, folder, rcount, ucount, ccount)
    dbref player;
    int folder;
    int *rcount;
    int *ucount;
    int *ccount;
{
  /* returns count of read, unread, & cleared messages as rcount, ucount,
   * ccount. folder=-1 returns for all folders */

  struct mail *mp;
  int rc, uc, cc;

  cc = rc = uc = 0;
  for (mp = find_exact_starting_point(player);
       mp && (mp->to == player);
       mp = mp->next) {
    if ((mp->to == player) && ((folder == -1) || (Folder(mp) == folder))) {
      if (Cleared(mp))
	cc++;
      else if (Read(mp))
	rc++;
      else
	uc++;
    }
  }
  *rcount = rc;
  *ucount = uc;
  *ccount = cc;
}


#ifdef MAIL_SUBJECTS
static void
send_mail(player, target, subject, message, flags, silent)
#else
static void
send_mail(player, target, message, flags, silent)
#endif
    dbref player;
    dbref target;
#ifdef MAIL_SUBJECTS
    unsigned char *subject;
#endif
    char *message;
    mail_flag flags;
    int silent;
{
  /* send a mail message */

  struct mail *newp, *mp;
  time_t tt;
  char tbuf1[30];
  int rc, uc, cc;
#ifdef MAIL_SUBJECTS
  char sbuf[BUFFER_LEN];
  ATTR *a;
#endif

  if (!IsPlayer(target)) {
    notify(player, "MAIL: You cannot send mail to non-existent people.");
    return;
  }
  if (!strcasecmp(message, "clear")) {
    notify(player, "MAIL: You probably don't wanna send mail saying 'clear'.");
    return;
  }
  if (!(Hasprivs(player) || eval_lock(player, target, Mail_Lock))) {
    notify(player, tprintf("MAIL: %s is not accepting mail from you right now.", Name(target)));
    return;
  }
  tt = time(NULL);
  strcpy(tbuf1, ctime(&tt));
  tbuf1[strlen(tbuf1) - 1] = '\0';	/* whack the newline */

  /* Append the MAILSIGNATURE to the mail - Cordin@Dune's idea */

  /* initialize the appropriate fields */
  newp = (struct mail *) mush_malloc(sizeof(struct mail), "mail");
  newp->to = target;
  newp->from = player;
#ifdef MAIL_SUBJECTS
  /* Deal with the subject */
  if (subject)
    strcpy(sbuf, uncompress(subject));
  else
    strcpy(sbuf, "(no subject)");
  if ((flags & M_FORWARD) && !string_prefix(sbuf, "Fwd:"))
    newp->subject = u_strdup(compress(chopstr(tprintf("Fwd: %s", sbuf), SUBJECT_LEN)));
  else if ((flags & M_REPLY) && !string_prefix(sbuf, "Re:"))
    newp->subject = u_strdup(compress(chopstr(tprintf("Re: %s", sbuf), SUBJECT_LEN)));
  else if ((a = atr_get_noparent(player, "MAILSUBJECT")) != NULL)
    /* Don't bother to uncompress a->value */
    newp->subject = u_strdup(a->value);
  else
    newp->subject = u_strdup(compress(sbuf));
#endif
  if (flags & M_FORWARD) {
    /* Forwarding passes the message already compressed */
    newp->message = u_strdup((unsigned char *) message);
  } else
    newp->message = u_strdup(compress(message));
  newp->time = u_strdup(compress(tbuf1));
  newp->read = flags & M_FMASK;	/* Send to folder 0 */

  /* Where do we insert it? After mp, wherever that is.
   * This can return NULL if there are no messages or
   * if we insert at the head of the list 
   */
  mp = find_insertion_point(target);

  if (mp) {
    newp->prev = mp;
    newp->next = mp->next;
    if (mp == TAIL)
      TAIL = newp;
    else
      mp->next->prev = newp;
    mp->next = newp;
  } else {
    if (HEAD) {
      /* Insert at the front */
      newp->next = HEAD;
      newp->prev = NULL;
      HEAD->prev = newp;
      HEAD = newp;
    } else {
      /* This is the first message in the maildb */
      HEAD = newp;
      TAIL = newp;
      newp->prev = NULL;
      newp->next = NULL;
    }
  }

  /* If the target's mailp isn't pointing to their list, we'd better
   * set it
   */
  if (Connected(target))
    desc_mail_set(target, find_exact_starting_point(target));

  mdb_top++;

  /* notify people */
  if (!silent)
    notify(player, tprintf("MAIL: You sent your message to %s.", Name(target)));
  count_mail(target, 0, &rc, &uc, &cc);
  notify(target, tprintf("MAIL: You have a new message (%d) from %s.",
			 rc + uc + cc, Name(player)));

  if (AMAIL_ATTR && (atr_get_noparent(target, "AMAIL")) && (player != target) &&
      Hasprivs(target))
    did_it(player, target, NULL, NULL, NULL, NULL, "AMAIL", NOTHING);

  return;
}


void
do_mail_nuke(player)
    dbref player;
{
  /* wipes the entire maildb */

  struct mail *mp, *nextp;

  if (!God(player)) {
    notify(player, "The postal service issues a warrant for your arrest.");
    return;
  }
  /* walk the list */
  for (mp = HEAD; mp != NULL; mp = nextp) {
    nextp = mp->next;
#ifdef MAIL_SUBJECTS
    if (mp->subject != mp->message)
      free(mp->subject);
#endif
    free(mp->message);
    free(mp->time);
    mush_free((Malloc_t) mp, "mail");
  }

  HEAD = TAIL = NULL;
  mdb_top = 0;

  do_log(LT_ERR, 0, 0, "** MAIL PURGE ** done by %s(#%d).",
	 Name(player), player);
  notify(player, "You annihilate the post office. All messages cleared.");
}

void
do_mail_debug(player, action, victim)
    dbref player;
    char *action;
    char *victim;
{
  /* how to fix mail with a sledgehammer */

  dbref target;
  struct mail *mp, *nextp;
  int i;

  if (!Wizard(player)) {
    notify(player, "Go get some bugspray.");
    return;
  }
  if (string_prefix("clear", action)) {
    target = lookup_player(victim);
    if (target == NOTHING) {
      target = match_result(player, victim, NOTYPE, MAT_ABSOLUTE);
    }
    if (target == NOTHING) {
      notify(player, tprintf("%s: no such player.", victim));
      return;
    }
    do_mail_clear(target, (char *) "ALL");
    do_mail_purge(target);
    notify(player, tprintf("Mail cleared for %s(#%d).", Name(target), target));
    return;
  } else if (string_prefix("sanity", action)) {
    for (i = 0, mp = HEAD; mp != NULL; i++, mp = mp->next) {
      if (!GoodObject(mp->to))
	notify(player, tprintf("Bad object #%d has mail.", mp->to));
      else if (!IsPlayer(mp->to))
	notify(player, tprintf("%s(#%d) has mail but is not a player.",
			       Name(mp->to), mp->to));
    }
    if (i != mdb_top) {
      notify(player,
	     tprintf(
	  "Mail database top is %d, actual message count is %d. Fixing.",
		      mdb_top, i));
      mdb_top = i;
    }
    notify(player, "Mail sanity check completed.");
  } else if (string_prefix("fix", action)) {
    for (i = 0, mp = HEAD; mp != NULL; i++, mp = nextp) {
      if (!GoodObject(mp->to) || !IsPlayer(mp->to)) {
	notify(player, tprintf("Fixing mail for #%d.", mp->to));
	/* Delete this one */
	/* head and tail of the list are special */
	if (mp == HEAD)
	  HEAD = mp->next;
	else if (mp == TAIL)
	  TAIL = mp->prev;
	/* relink the list */
	if (mp->prev != NULL)
	  mp->prev->next = mp->next;
	if (mp->next != NULL)
	  mp->next->prev = mp->prev;
	/* save the pointer */
	nextp = mp->next;
	/* then wipe */
	mdb_top--;
#ifdef MAIL_SUBJECTS
	if (mp->subject != mp->message)
	  free(mp->subject);
#endif
	free(mp->message);
	free(mp->time);
	mush_free((Malloc_t) mp, "mail");
      } else
	nextp = mp->next;
    }
    notify(player, "Mail sanity fix completed.");
  } else {
    notify(player, "That is not a debugging option.");
    return;
  }
}

void
do_mail_stats(player, name, full)
    dbref player;
    char *name;
    int full;
{
  /* mail database statistics */

  dbref target;
  int fc, fr, fu, tc, tr, tu, fchars, tchars, cchars;
  char last[50];
  struct mail *mp;

  fc = fr = fu = tc = tr = tu = cchars = fchars = tchars = 0;

  /* find player */

  if (*name == '\0') {
    if Wizard
      (player)
	target = AMBIGUOUS;
    else
      target = player;
  } else if (*name == NUMBER_TOKEN) {
    target = atoi(&name[1]);
    if (!GoodObject(target) || !IsPlayer(target))
      target = NOTHING;
  } else if (!strcasecmp(name, "me")) {
    target = player;
  } else {
    target = lookup_player(name);
  }

  if (!GoodObject(target)) {
    target = match_result(player, name, NOTYPE, MAT_ABSOLUTE);
  }
  if (!GoodObject(target)) {
    notify(player, tprintf("%s: No such player.", name));
    return;
  }
  if (!Wizard(player) && (target != player)) {
    notify(player, "The post office protects privacy!");
    return;
  }
  /* this comand is computationally expensive */

  if (options.daytime) {
    notify(player, "Sorry, that command has been temporarily disabled.");
    return;
  }
  if (!payfor(player, FIND_COST)) {
    notify(player, tprintf("Finding mail stats costs %d %s.", FIND_COST,
			   (FIND_COST == 1) ? MONEY : MONIES));
    return;
  }
  if (target == AMBIGUOUS) {	/* stats for all */
    if (full == 0) {
      notify(player,
	   tprintf("There are %d messages in the mail spool.", mdb_top));
      return;
    } else if (full == 1) {
      for (mp = HEAD; mp != NULL; mp = mp->next) {
	if (Cleared(mp))
	  fc++;
	else if (Read(mp))
	  fr++;
	else
	  fu++;
      }
      notify(player,
	     tprintf("MAIL: There are %d msgs in the mail spool, %d unread, %d cleared.",
		     fc + fr + fu, fu, fc));
      return;
    } else {
      for (mp = HEAD; mp != NULL; mp = mp->next) {
	if (Cleared(mp)) {
	  fc++;
	  cchars += strlen((char *) uncompress(mp->message));
	} else if (Read(mp)) {
	  fr++;
	  fchars += strlen((char *) uncompress(mp->message));
	} else {
	  fu++;
	  tchars += strlen((char *) uncompress(mp->message));
	}
      }
      notify(player,
	     tprintf(
		      "MAIL: There are %d old msgs in the mail spool, totalling %d characters.",
		      fr, fchars));
      notify(player,
	     tprintf(
		      "MAIL: There are %d new msgs in the mail spool, totalling %d characters.",
		      fu, tchars));
      notify(player,
	     tprintf(
		      "MAIL: There are %d cleared msgs in the mail spool, totalling %d characters.",
		      fc, cchars));
      return;
    }
  }
  /* individual stats */

  if (full == 0) {
    /* just count number of messages */
    for (mp = HEAD; mp != NULL; mp = mp->next) {
      if (mp->from == target)
	fr++;
      if (mp->to == target)
	tr++;
    }
    notify(player, tprintf("%s sent %d messages.",
			   Name(target), fr));
    notify(player, tprintf("%s has %d messages.", Name(target), tr));
    return;
  }
  /* more detailed message count */
  for (mp = HEAD; mp != NULL; mp = mp->next) {
    if (mp->from == target) {
      if (Cleared(mp))
	fc++;
      else if (Read(mp))
	fr++;
      else
	fu++;
      if (full == 2)
	fchars += strlen((char *) uncompress(mp->message));
    }
    if (mp->to == target) {
      if (!tr && !tu)
	strcpy(last, (char *) uncompress(mp->time));
      if (Cleared(mp))
	tc++;
      else if (Read(mp))
	tr++;
      else
	tu++;
      if (full == 2)
	tchars += strlen((char *) uncompress(mp->message));
    }
  }

  notify(player, tprintf("Mail statistics for %s:", Name(target)));

  if (full == 1) {
    notify(player, tprintf("%d messages sent, %d unread, %d cleared.",
			   fc + fr + fu, fu, fc));
    notify(player, tprintf("%d messages received, %d unread, %d cleared.",
			   tc + tr + tu, tu, tc));
  } else {
    notify(player,
	   tprintf("%d messages sent, %d unread, %d cleared, totalling %d characters.",
		   fc + fr + fu, fu, fc, fchars));
    notify(player,
	   tprintf("%d messages received, %d unread, %d cleared, totalling %d characters.",
		   tc + tr + tu, tu, tc, tchars));
  }

  if (tc + tr + tu > 0)
    notify(player, tprintf("Last is dated %s", last));
  return;
}


/*-------------------------------------------------------------------------*
 *   Main mail routine for @mail w/o a switch
 *-------------------------------------------------------------------------*/

void
do_mail(player, arg1, arg2)
    dbref player;
    char *arg1;
    char *arg2;
{
  dbref sender;
  /* Force player to be a real player, but keep track of the
   * enactor in case we're sending mail, which objects can do
   */
  sender = player;
  player = Owner(player);
  if (!arg1 || !*arg1) {
    if (arg2 && *arg2) {
      notify(player, "MAIL: Invalid mail command.");
      return;
    }
    /* just the "@mail" command */
    do_mail_list(player, "");
    return;
  }
  /* purge a player's mailbox */
  if (!strcasecmp(arg1, "purge")) {
    do_mail_purge(player);
    return;
  }
  /* clear message */
  if (!strcasecmp(arg1, "clear")) {
    do_mail_clear(player, arg2);
    return;
  }
  if (!strcasecmp(arg1, "unclear")) {
    do_mail_unclear(player, arg2);
    return;
  }
  if (arg2 && *arg2) {
    /* Sending mail */
    do_mail_send(sender, arg1, arg2, 0, 0);
  } else {
    /* Must be reading or listing mail - no arg2 */
    if (isdigit(*arg1) && !index(arg1, '-'))
      do_mail_read(player, arg1);
    else
      do_mail_list(player, arg1);
  }
  return;
}

/*-------------------------------------------------------------------------*
 *   Auxiliary functions
 *-------------------------------------------------------------------------*/
/* ARGSUSED */
FUNCTION(fun_folderstats)
{
  /* This function can take one of four formats:
   * folderstats() -> returns stats for my current folder
   * folderstats(folder#) -> returns stats for my folder folder#
   * folderstats(player) -> returns stats for player's current folder
   * folderstats(player,folder#) -> returns stats for player's folder folder#
   */
  dbref player;
  int rc, uc, cc;

  if (nargs == 0) {
    count_mail(executor, player_folder(executor), &rc, &uc, &cc);
    safe_str(unparse_integer(rc + uc), buff, bp);
    return;
  }
  if (nargs == 1) {
    if (!is_integer(args[0])) {
      /* handle the case of wanting to count the number of messages */
      if ((player = noisy_match_result(executor, args[0], TYPE_PLAYER, MAT_OBJECTS)) == NOTHING) {
	safe_str("#-1 NO SUCH PLAYER", buff, bp);
	return;
      } else if ((executor != player) && !Wizard(executor)) {
	safe_str("#-1 PERMISSION DENIED", buff, bp);
	return;
      } else {
	count_mail(player, player_folder(player), &rc, &uc, &cc);
	safe_str(unparse_integer(rc), buff, bp);
	safe_chr(' ', buff, bp);
	safe_str(unparse_integer(uc), buff, bp);
	safe_chr(' ', buff, bp);
	safe_str(unparse_integer(cc), buff, bp);
	return;
      }
    } else {
      count_mail(executor, parse_integer(args[0]), &rc, &uc, &cc);
      safe_str(unparse_integer(rc + uc), buff, bp);
      return;
    }
  } else {
    if ((player = lookup_player(args[0])) == NOTHING) {
      safe_str("#-1 NO SUCH PLAYER", buff, bp);
      return;
    } else if ((executor != player) && !Wizard(executor)) {
      safe_str("#-1 PERMISSION DENIED", buff, bp);
      return;
    }
    if (!is_integer(args[1])) {
      safe_str("#-1 FOLDER MUST BE INTEGER", buff, bp);
      return;
    }
    count_mail(player, parse_integer(args[1]), &rc, &uc, &cc);
    safe_str(unparse_integer(rc), buff, bp);
    safe_chr(' ', buff, bp);
    safe_str(unparse_integer(uc), buff, bp);
    safe_chr(' ', buff, bp);
    safe_str(unparse_integer(cc), buff, bp);
    return;
  }
}

/* ARGSUSED */
FUNCTION(fun_mail)
{
  /* mail([<player>,] [<folder #>:]<message #>)
   * mail() --> return total # of messages for executor
   * mail(<player>) --> return total # of messages for player
   */

  struct mail *mp;
  dbref player;
  int rc, uc, cc;

  if (nargs == 0) {
    count_mail(executor, -1, &rc, &uc, &cc);
    safe_str(unparse_integer(rc + uc + cc), buff, bp);
    return;
  }
  /* Try mail(<player>) */
  if (nargs == 1) {
    player = match_result(executor, args[0], TYPE_PLAYER, MAT_OBJECTS);
    if (GoodObject(player)) {
      if ((executor != player) && !Wizard(executor)) {
	safe_str("#-1 PERMISSION DENIED", buff, bp);
      } else {
	count_mail(player, -1, &rc, &uc, &cc);
	safe_str(unparse_integer(rc), buff, bp);
	safe_chr(' ', buff, bp);
	safe_str(unparse_integer(uc), buff, bp);
	safe_chr(' ', buff, bp);
	safe_str(unparse_integer(cc), buff, bp);
      }
      return;
    }
  }
  /* That didn't work. Ok, try mailfun_fetch */
  mp = mailfun_fetch(executor, nargs, args[0], args[1]);
  if (mp) {
    safe_str(uncompress(mp->message), buff, bp);
    return;
  }
  safe_str("#-1 INVALID MESSAGE OR PLAYER", buff, bp);
  return;
}

/* A helper routine used by all the mail*() functions
 * We parse the following format:
 *  func([<player>,] [<folder #>:]<message #>)
 * and return the matching message or NULL
 */
static struct mail *
mailfun_fetch(player, nargs, arg1, arg2)
    dbref player;
    int nargs;
    char *arg1;
    char *arg2;
{
  dbref target;
  int msg;
  int folder;

  if (nargs == 1) {
    /* Simply a message number or folder:message */
    if (parse_message_spec(player, arg1, &msg, NULL, &folder))
      return real_mail_fetch(player, msg, folder);
    else {
      return NULL;
    }
  } else {
    /* Both a target and a message */
    if ((target = noisy_match_result(player, arg1, TYPE_PLAYER, MAT_OBJECTS)) == NOTHING) {
      return NULL;
    } else if ((player != target) && !Wizard(player)) {
      notify(player, "Permission denied");
      return NULL;
    }
    if (parse_message_spec(target, arg2, &msg, NULL, &folder))
      return real_mail_fetch(target, msg, folder);
    else {
      notify(player, "Invalid message specification");
      return NULL;
    }
  }
  /* NOTREACHED */
  return NULL;
}


/* ARGSUSED */
FUNCTION(fun_mailfrom)
{
  struct mail *mp;

  mp = mailfun_fetch(executor, nargs, args[0], args[1]);
  if (!mp)
    safe_str("#-1", buff, bp);
  else
    safe_str(unparse_dbref(mp->from), buff, bp);
  return;
}


/* ARGSUSED */
FUNCTION(fun_mailtime)
{
  struct mail *mp;

  mp = mailfun_fetch(executor, nargs, args[0], args[1]);
  if (!mp)
    safe_str("#-1", buff, bp);
  else
    safe_str(uncompress(mp->time), buff, bp);
  return;
}

/* ARGSUSED */
FUNCTION(fun_mailstatus)
{
  struct mail *mp;

  mp = mailfun_fetch(executor, nargs, args[0], args[1]);
  if (!mp)
    safe_str("#-1", buff, bp);
  else
    safe_str(status_chars(mp), buff, bp);
  return;
}


#ifdef MAIL_SUBJECTS
/* ARGSUSED */
FUNCTION(fun_mailsubject)
{
  struct mail *mp;

  mp = mailfun_fetch(executor, nargs, args[0], args[1]);
  if (!mp)
    safe_str("#-1", buff, bp);
  else
    safe_str(uncompress(mp->subject), buff, bp);
  return;
}
#endif

int
dump_mail(fp)
    FILE *fp;
{
  struct mail *mp;
  int count = 0;
  int mail_flags = 0;

#ifdef MAIL_SUBJECTS
  mail_flags += MDBF_SUBJECT;
#endif

  if (mail_flags)
    fprintf(fp, "+%d\n", mail_flags);

  fprintf(fp, "%d\n", mdb_top);

  for (mp = HEAD; mp != NULL; mp = mp->next) {
    putref(fp, mp->to);
    putref(fp, mp->from);
    putstring(fp, uncompress(mp->time));
#ifdef MAIL_SUBJECTS
    if (mp->subject)
      putstring(fp, uncompress(mp->subject));
    else
      putstring(fp, "");
#endif
    putstring(fp, uncompress(mp->message));
    putref(fp, mp->read);
    count++;
  }

  fprintf(fp, "*** END OF DUMP ***\n");
  fflush(fp);

  if (count != mdb_top) {
    do_log(LT_ERR, 0, 0, "MAIL: Count of messages is %d, mdb_top is %d.",
	   count, mdb_top);
/*    mdb_top = count;    */
/*  Removed since it won't make a difference unless the process isn't forked */
  }
  return (count);
}



/* Find the first message in a player's mail chain, or NULL if none */
struct mail *
find_exact_starting_point(player)
    dbref player;
{
  static struct mail *mp;

  if (!HEAD)
    return NULL;
  mp = desc_mail(player);
  if (!mp) {
    /* Player is connected and has no mail, or nobody's connected who
     * has mail - we have to scan the maildb.
     */
    if (HEAD->to > player)
      return NULL;		/* No mail chain */
    for (mp = HEAD; mp && (mp->to < player); mp = mp->next) ;
  } else {
    while (mp && (mp->to >= player))
      mp = mp->prev;
    if (!mp)
      mp = HEAD;
    while (mp && (mp->to < player))
      mp = mp->next;
  }
  if (mp && (mp->to == player))
    return mp;
  return NULL;
}


/* Find the place where new mail to this player should go (after):
 *  1. The last message in the player's mail chain, or
 *  2. The last message before where the player's chain should start, or
 *  3. NULL (meaning TAIL)
 */
static struct mail *
find_insertion_point(player)
    dbref player;
{
  static struct mail *mp;

  if (!HEAD)
    return NULL;
  mp = desc_mail(player);
  if (!mp) {
    /* Player is connected and has no mail, or nobody's connected who
     * has mail - we have to scan the maildb.
     */
    if (HEAD->to > player)
      return NULL;		/* No mail chain */
    for (mp = TAIL; mp && (mp->to > player); mp = mp->prev) ;
  } else {
    while (mp && (mp->to <= player))
      mp = mp->next;
    if (!mp)
      mp = TAIL;
    while (mp && (mp->to > player))
      mp = mp->prev;
  }
  return mp;
}



void
mail_init()
{
  mdb_top = 0;
  HEAD = NULL;
  TAIL = NULL;
}

int
load_mail(fp)
    FILE *fp;
{
  char nbuf1[8];
  unsigned char *tbuf = NULL;
  int mail_top = 0;
  int mail_flags = 0;
  int i = 0;
  struct mail *mp, *tmpmp;
  int done;
#ifdef MAIL_SUBJECTS
  char sbuf[BUFFER_LEN];
#endif

  mail_init();

  /* find out how many messages we should be loading */
  fgets(nbuf1, sizeof(nbuf1), fp);
  /* If it starts with +, it's telling us the mail db flags */
  if (*nbuf1 == '+') {
    mail_flags = atoi(nbuf1 + 1);
    /* If the flags indicates aliases, we'll read them now */
    fgets(nbuf1, sizeof(nbuf1), fp);
  }
  mail_top = atoi(nbuf1);
  if (!mail_top)
    return 0;

  /* first one is a special case */
  mp = (struct mail *) mush_malloc(sizeof(struct mail), "mail");
  mp->to = getref(fp);
  mp->from = getref(fp);
  mp->time = u_strdup(compress((char *) getstring_noalloc(fp)));
  if (mail_flags & MDBF_SUBJECT) {
    tbuf = u_strdup(compress((char *) getstring_noalloc(fp)));
  }
  mp->message = u_strdup(compress((char *) getstring_noalloc(fp)));
#ifdef MAIL_SUBJECTS
  if (mail_flags & MDBF_SUBJECT)
    mp->subject = tbuf;
  else {
    strcpy(sbuf, uncompress(mp->message));
    mp->subject = u_strdup(compress(chopstr(sbuf, SUBJECT_LEN)));
  }
#endif
  mp->read = getref(fp);
  mp->next = NULL;
  mp->prev = NULL;
  HEAD = mp;
  TAIL = mp;
  i++;

  /* now loop through */
  for (; i < mail_top; i++) {
    mp = (struct mail *) mush_malloc(sizeof(struct mail), "mail");
    mp->to = getref(fp);
    mp->from = getref(fp);
    mp->time = u_strdup(compress((char *) getstring_noalloc(fp)));
    if (mail_flags & MDBF_SUBJECT)
      tbuf = u_strdup(compress((char *) getstring_noalloc(fp)));
    else
      tbuf = NULL;
    mp->message = u_strdup(compress((char *) getstring_noalloc(fp)));
#ifdef MAIL_SUBJECTS
    if (tbuf)
      mp->subject = tbuf;
    else {
      strcpy(sbuf, uncompress(mp->message));
      mp->subject = u_strdup(compress(chopstr(sbuf, SUBJECT_LEN)));
    }
#endif
    mp->read = getref(fp);

    /* We now to a sorted insertion, sorted by recipient db# */
    if (mp->to >= TAIL->to) {
      /* Pop it onto the end */
      mp->next = NULL;
      mp->prev = TAIL;
      TAIL->next = mp;
      TAIL = mp;
    } else {
      /* Search for where to put it */
      mp->prev = NULL;
      for (done = 0, tmpmp = HEAD; tmpmp && !done; tmpmp = tmpmp->next) {
	if (tmpmp->to > mp->to) {
	  /* Insert before tmpmp */
	  mp->next = tmpmp;
	  mp->prev = tmpmp->prev;
	  if (tmpmp->prev) {
	    /* tmpmp isn't HEAD */
	    tmpmp->prev->next = mp;
	  } else {
	    /* tmpmp is HEAD */
	    HEAD = mp;
	  }
	  tmpmp->prev = mp;
	  done = 1;
	}
      }
      if (!done) {
	/* This is bad */
	fprintf(stderr, "MAIL: bad code.\n");
      }
    }
  }


  mdb_top = i;

  if (i != mail_top) {
    fprintf(stderr, "MAIL: mail_top is %d, only read in %d messages.\n",
	    mail_top, i);
  }
  do_mail_debug(GOD, (char *) "fix", (char *) "");
  return (mdb_top);
}

static int
get_folder_number(player, name)
    dbref player;
    char *name;
{
  ATTR *a;
  char str[BUFFER_LEN], pat[BUFFER_LEN], *res, *p;
  /* Look up a folder name and return the appopriate number */
  a = (ATTR *) atr_get_noparent(player, "MAILFOLDERS");
  if (!a)
    return -1;
  strcpy(str, uncompress(a->value));
  sprintf(pat, ":%s:", strupper(name));
  res = strstr(str, pat);
  if (!res)
    return -1;
  res += 2 + strlen(name);
  p = res;
  while (!isspace(*p))
    p++;
  p = '\0';
  return atoi(res);
}

static char *
get_folder_name(player, fld)
    dbref player;
    int fld;
{
  static char str[BUFFER_LEN];
  char pat[BUFFER_LEN];
  static char *old;
  char *r;
  ATTR *a;

  /* Get the name of the folder, or "nameless" */
  sprintf(pat, "%d:", fld);
  old = NULL;
  a = (ATTR *) atr_get_noparent(player, "MAILFOLDERS");
  if (!a) {
    strcpy(str, "unnamed");
    return str;
  }
  strcpy(str, (char *) uncompress(a->value));
  old = (char *) string_match(str, pat);
  if (old) {
    r = old + strlen(pat);
    while (*r != ':')
      r++;
    *r = '\0';
    return old + strlen(pat);
  } else {
    strcpy(str, "unnamed");
    return str;
  }
}

void
add_folder_name(player, fld, name)
    dbref player;
    int fld;
    const char *name;
{
  char *old, *res, *r;
  char *new, *pat, *str, *tbuf;
  ATTR *a;

  /* Muck with the player's MAILFOLDERS attrib to add a string of the form:
   * number:name:number to it, replacing any such string with a matching
   * number.
   */
  new = (char *) mush_malloc(BUFFER_LEN, "string");
  pat = (char *) mush_malloc(BUFFER_LEN, "string");
  str = (char *) mush_malloc(BUFFER_LEN, "string");
  tbuf = (char *) mush_malloc(BUFFER_LEN, "string");
  if (!new || !pat || !str || !tbuf)
    panic("Failed to allocate strings in add_folder_name");

  if (name && *name)
    sprintf(new, "%d:%s:%d ", fld, strupper(name), fld);
  else
    strcpy(new, " ");
  sprintf(pat, "%d:", fld);
  /* get the attrib and the old string, if any */
  old = NULL;
  a = (ATTR *) atr_get_noparent(player, "MAILFOLDERS");
  if (a) {
    strcpy(str, (char *) uncompress(a->value));
    old = (char *) string_match(str, pat);
  }
  if (old && *old) {
    strcpy(tbuf, str);
    r = old;
    while (!isspace(*r))
      r++;
    *r = '\0';
    res = (char *) replace_string(old, new, tbuf);	/* mallocs mem! */
  } else {
    r = res = (char *) malloc(BUFFER_LEN + 1);
    if (a)
      safe_str(str, res, &r);
    safe_str(new, res, &r);
    *r = '\0';
  }
  /* put the attrib back */
  atr_add(player, "MAILFOLDERS", res, GOD, AF_WIZARD | AF_NOPROG | AF_LOCKED);
  free(res);
  mush_free((Malloc_t) new, "string");
  mush_free((Malloc_t) pat, "string");
  mush_free((Malloc_t) str, "string");
  mush_free((Malloc_t) tbuf, "string");
}

static int
player_folder(player)
    dbref player;
{
  /* Return the player's current folder number. If they don't have one, set
   * it to 0 */
  ATTR *a;

  a = (ATTR *) atr_get_noparent(player, "MAILCURF");
  if (!a) {
    set_player_folder(player, 0);
    return 0;
  }
  return atoi(uncompress(a->value));
}

void
set_player_folder(player, fnum)
    dbref player;
    int fnum;
{
  /* Set a player's folder to fnum */
  ATTR *a;
  char tbuf1[BUFFER_LEN];

  sprintf(tbuf1, "%d", fnum);
  a = (ATTR *) atr_match("MAILCURF");
  if (a)
    atr_add(player, a->name, tbuf1, GOD, a->flags);
  else				/* Shouldn't happen, but... */
    atr_add(player, "MAILCURF", tbuf1, GOD, AF_WIZARD | AF_NOPROG | AF_LOCKED);
}

static int
parse_folder(player, folder_string)
    dbref player;
    char *folder_string;
{
  int fnum;

  /* Given a string, return a folder #, or -1 The string is just a number,
   * for now. Later, this will be where named folders are handled */
  if (!folder_string || !*folder_string)
    return -1;
  if (isdigit(*folder_string)) {
    fnum = atoi(folder_string);
    if ((fnum < 0) || (fnum > MAX_FOLDERS))
      return -1;
    else
      return fnum;
  }
  /* Handle named folders here */
  return get_folder_number(player, folder_string);
}

static int
mail_match(player, mp, ms, num)
    dbref player;
    struct mail *mp;
    struct mail_selector ms;
    int num;
{
  time_t now, msgtime, diffdays;
  char msgtimestr[BUFFER_LEN];
  struct tm *msgtm;
  mail_flag mpflag;

  /* Does a piece of mail match the mail_selector? */
  if (ms.low && num < ms.low)
    return 0;
  if (ms.high && num > ms.high)
    return 0;
  if (ms.player && mp->from != ms.player)
    return 0;
  if (AllInFolder(ms) && (Folder(mp) == player_folder(player)))
    return 1;
  mpflag = Read(mp) ? mp->read : (mp->read | M_MSUNREAD);
  if (!All(ms) && !(ms.flags & mpflag))
    return 0;
  if (ms.days != -1) {
    /* Get the time now, subtract mp->time, and compare the results with
     * ms.days (in manner of ms.day_comp) */
    time(&now);
    msgtm = (struct tm *) malloc(sizeof(struct tm));
    if (!msgtm) {
      /* Ugly malloc failure */
      do_log(LT_ERR, 0, 0, "MAIL: Couldn't malloc struct tm!");
      return 0;
    }
    strcpy(msgtimestr, (char *) uncompress(mp->time));
    if (do_convtime(msgtimestr, msgtm)) {
#ifdef HAS_TIMELOCAL
      msgtime = timelocal(msgtm);
#else
      msgtime = mktime(msgtm);
#endif				/* HAS_TIMELOCAL */
      free(msgtm);
      diffdays = (now - msgtime) / 86400;
      if (sign(diffdays - ms.days) != ms.day_comp)
	return 0;
    } else {
      free(msgtm);
      return 0;
    }
  }
  return 1;
}

static int
parse_msglist(msglist, ms, player)
    const char *msglist;
    struct mail_selector *ms;
    dbref player;
{
  char *p;
  char tbuf1[BUFFER_LEN];
  int folder;

  /* Take a message list, and return the appropriate mail_selector setup. For
   * now, msglists are quite restricted. That'll change once all this is
   * working. Returns 0 if couldn't parse, and also notifys player of why. */
  /* Initialize the mail selector - this matches all messages */
  ms->low = 0;
  ms->high = 0;
  ms->flags = 0x00FF | M_MSUNREAD;
  ms->player = 0;
  ms->days = -1;
  ms->day_comp = 0;
  /* Now, parse the message list */
  if (!msglist || !*msglist) {
    /* All messages */
    return 1;
  }
  /* Don't mess with msglist itself */
  strncpy(tbuf1, msglist, BUFFER_LEN - 1);
  p = tbuf1;
  while (p && *p && isspace(*p))
    p++;
  if (!p || !*p) {
    ms->flags |= M_FOLDER;
    return 1;			/* all messages in current folder */
  }
  if (isdigit(*p) || *p == '-') {
    if (!parse_message_spec(player, p, &ms->low, &ms->high, &folder)) {
      notify(player, "MAIL: Invalid message specification");
      return 0;
    }
    ms->flags |= FolderBit(folder);
  } else if (*p == '~') {
    /* exact # of days old */
    p++;
    if (!p || !*p) {
      notify(player, "MAIL: Invalid age");
      return 0;
    }
    if (!is_integer(p)) {
      notify(player, "MAIL: Message ages must be integers");
      return 0;
    }
    ms->day_comp = 0;
    ms->days = atoi(p);
    if (ms->days < 0) {
      notify(player, "MAIL: Invalid age");
      return 0;
    }
  } else if (*p == '<') {
    /* less than # of days old */
    p++;
    if (!p || !*p) {
      notify(player, "MAIL: Invalid age");
      return 0;
    }
    if (!is_integer(p)) {
      notify(player, "MAIL: Message ages must be integers");
      return 0;
    }
    ms->day_comp = -1;
    ms->days = atoi(p);
    if (ms->days < 0) {
      notify(player, "MAIL: Invalid age");
      return 0;
    }
  } else if (*p == '>') {
    /* greater than # of days old */
    p++;
    if (!p || !*p) {
      notify(player, "MAIL: Invalid age");
      return 0;
    }
    if (!is_integer(p)) {
      notify(player, "MAIL: Message ages must be integers");
      return 0;
    }
    ms->day_comp = 1;
    ms->days = atoi(p);
    if (ms->days < 0) {
      notify(player, "MAIL: Invalid age");
      return 0;
    }
  } else if (*p == '#') {
    /* From db# */
    if (!is_dbref(p)) {
      notify(player, "MAIL: Invalid dbref #");
      return 0;
    }
    ms->player = parse_dbref(p);
    if (!GoodObject(ms->player) || !(ms->player)) {
      notify(player, "MAIL: Invalid dbref #");
      return 0;
    }
  } else if (*p == '*') {
    /* From player name */
    p++;
    if (!p || !*p) {
      notify(player, "MAIL: Invalid player");
      return 0;
    }
    ms->player = lookup_player(p);
    if (ms->player == NOTHING) {
      notify(player, "MAIL: Invalid player");
      return 0;
    }
  } else if (!strcasecmp(p, "all")) {
    ms->flags = M_ALL;
  } else if (!strcasecmp(p, "folder")) {
    ms->flags = M_FOLDER;
  } else if (!strcasecmp(p, "urgent")) {
    ms->flags = M_URGENT;
  } else if (!strcasecmp(p, "unread")) {
    ms->flags = M_MSUNREAD;
  } else if (!strcasecmp(p, "read")) {
    ms->flags = M_READ;
  } else if (!strcasecmp(p, "clear") || !strcasecmp(p, "cleared")) {
    ms->flags = M_CLEARED;
  } else if (!strcasecmp(p, "tag") || !strcasecmp(p, "tagged")) {
    ms->flags = M_TAG;
  } else if (!strcasecmp(p, "mass")) {
    ms->flags = M_MASS;
  } else if (!strcasecmp(p, "me")) {
    ms->player = player;
  } else {
    notify(player, "MAIL: Invalid message specification");
    return 0;
  }
  return 1;
}

static char *
status_chars(mp)
    struct mail *mp;
{
  /* Return a short description of message flags */
  static char res[10];
  char *p;

  strcpy(res, "");
  p = res;
  *p++ = Read(mp) ? '-' : 'N';
  *p++ = Cleared(mp) ? 'C' : '-';
  *p++ = Urgent(mp) ? 'U' : '-';
  /* *p++ = Mass(mp) ? 'M' : '-'; */
  *p++ = Forward(mp) ? 'F' : '-';
  *p++ = Tagged(mp) ? '+' : '-';
  *p = '\0';
  return res;
}

static char *
status_string(mp)
    struct mail *mp;
{
  /* Return a longer description of message flags */
  static char tbuf1[BUFFER_LEN];

  strcpy(tbuf1, "");
  if (Read(mp))
    strcat(tbuf1, "Read ");
  else
    strcat(tbuf1, "Unread ");
  if (Cleared(mp))
    strcat(tbuf1, "Cleared ");
  if (Urgent(mp))
    strcat(tbuf1, "Urgent ");
  if (Mass(mp))
    strcat(tbuf1, "Mass ");
  if (Forward(mp))
    strcat(tbuf1, "Fwd ");
  if (Tagged(mp))
    strcat(tbuf1, "Tagged");
  return tbuf1;
}

void
check_mail(player, folder, silent)
    dbref player;
    int folder;
    int silent;
{

  /* Check for new @mail */
  int rc;			/* read messages */
  int uc;			/* unread messages */
  int cc;			/* cleared messages */

  /* just count messages */
  count_mail(player, folder, &rc, &uc, &cc);
  if (rc + uc + cc > 0)
    notify(player,
	   tprintf("MAIL: %d messages in folder %d [%s] (%d unread, %d cleared).",
	 rc + uc + cc, folder, get_folder_name(player, folder), uc, cc));
  else if (!silent)
    notify(player, tprintf("\nMAIL: You have no mail.\n"));
  return;
}

static int
sign(x)
    int x;
{
  if (x == 0) {
    return 0;
  } else if (x < 0) {
    return -1;
  } else {
    return 1;
  }
}

/* See if we've been given something of the form [f:]m1[-m2]
 * If so, return 1 and set f and mlow and mhigh
 * If not, return 0
 * If msghigh is given as NULL, don't allow ranges 
 * Used in parse_msglist, fun_mail and relatives.
 */
static int
parse_message_spec(player, s, msglow, msghigh, folder)
    dbref player;
    const char *s;
    int *msglow;
    int *msghigh;
    int *folder;
{
  char buf[BUFFER_LEN];
  char *p, *q;
  if (!s || !*s)
    return 0;
  strcpy(buf, s);
  if ((p = strchr(buf, ':'))) {
    *p++ = '\0';
    if (!is_integer(buf))
      return 0;
    *folder = parse_integer(buf);
    if (msghigh && (q = strchr(p, '-'))) {
      /* f:low-high */
      *q++ = '\0';
      if (!*p)
	*msglow = 0;
      else if (!is_integer(p))
	return 0;
      else {
	*msglow = parse_integer(p);
	if (*msglow == 0)
	  *msglow = -1;
      }
      if (!*q)
	*msghigh = 0;
      else if (!is_integer(q))
	return 0;
      else {
	*msghigh = parse_integer(q);
	if (*msghigh == 0)
	  *msghigh = -1;
      }
    } else {
      /* f:m */
      if (!is_integer(p))
	return 0;
      *msglow = parse_integer(p);
      if (*msglow == 0)
	*msglow = -1;
      if (msghigh)
	*msghigh = *msglow;
    }
    if (*msglow < 0 || (msghigh && *msghigh < 0) || *folder < 0 || *folder > MAX_FOLDERS)
      return 0;
  } else {
    /* No folder spec */
    *folder = player_folder(player);
    if (msghigh && (q = strchr(buf, '-'))) {
      /* low-high */
      *q++ = '\0';
      if (!*buf)
	*msglow = 0;
      else if (!is_integer(buf))
	return 0;
      else {
	*msglow = parse_integer(buf);
	if (*msglow == 0)
	  *msglow = -1;
      }
      if (!*q)
	*msghigh = 0;
      else if (!is_integer(q))
	return 0;
      else {
	*msghigh = parse_integer(q);
	if (*msghigh == 0)
	  *msghigh = -1;
      }
    } else {
      /* m */
      if (!is_integer(buf))
	return 0;
      *msglow = parse_integer(buf);
      if (*msglow == 0)
	*msglow = -1;
      if (msghigh)
	*msghigh = *msglow;
    }
    if (*msglow < 0 || (msghigh && *msghigh < 0))
      return 0;
  }
  return 1;
}

#endif				/* USE_MAILER */
