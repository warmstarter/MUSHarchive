/* bsd.c */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#ifdef WIN32
#define FD_SETSIZE 256
#include <windows.h>
#include <winsock.h>
#include <io.h>
#define EINTR WSAEINTR
#define EWOULDBLOCK WSAEWOULDBLOCK
#define MAXHOSTNAMELEN 32
#define LC_MESSAGES 6
void shutdown_checkpoint(void);
#else				/* !WIN32 */
#ifdef I_SYS_FILE
#include <sys/file.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#endif
#include <sys/ioctl.h>
#include <errno.h>
#ifdef I_SYS_SOCKET
#include <sys/socket.h>
#endif
#ifdef I_NETINET_IN
#include <netinet/in.h>
#endif
#ifdef I_NETDB
#include <netdb.h>
#endif
#ifdef I_SYS_PARAM
#include <sys/param.h>
#endif
#ifdef I_SYS_STAT
#include <sys/stat.h>
#endif
#endif				/* !WIN32 */
#include <time.h>
#ifdef I_SYS_WAIT
#include <sys/wait.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#ifdef I_SYS_SELECT
#include <sys/select.h>
#endif
#ifdef I_UNISTD
#include <unistd.h>
#endif
#ifdef HAS_GETRLIMIT
#include <sys/resource.h>
#endif
#include <limits.h>
#ifdef I_FLOATINGPOINT
#include <floatingpoint.h>
#endif
#include <locale.h>
#ifdef __APPLE__
#define LC_MESSAGES     6
#define AUTORESTART
#endif
#include <setjmp.h>

#include "conf.h"

#if defined(WIN32) && defined(INFO_SLAVE)
#undef INFO_SLAVE
#endif

#ifdef INFO_SLAVE
#include <sys/uio.h>
#endif

#include "mushdb.h"
#include "externs.h"
#include "dbdefs.h"
#include "flags.h"
#include "lock.h"
#include "help.h"
#include "match.h"
#include "ansi.h"
#include "pueblo.h"
#include "parse.h"
#include "access.h"
#include "version.h"
#include "patches.h"
#include "mysocket.h"
#include "ident.h"
#include "strtree.h"
#include "log.h"

#ifdef MEM_CHECK
#include "memcheck.h"
#endif

#include "mymalloc.h"

#ifdef CHAT_SYSTEM
#include "extchat.h"
extern CHAN *channels;
struct na_cpass {
  CHANUSER *u;
  int checkquiet;
};

#endif
#include "extmail.h"
#include "attrib.h"
#include "game.h"
#include "confmagic.h"
#ifdef HAS_WAITPID
#define WAIT_TYPE int
#else
#ifdef UNION_WAIT
#define WAIT_TYPE union wait
#else
#define WAIT_TYPE int
#endif
#endif

/* BSD 4.2 and maybe some others need these defined */
#ifndef FD_ZERO
#define fd_set int
#define FD_ZERO(p)       (*p = 0)
#define FD_SET(n,p)      (*p |= (1<<(n)))
#define FD_CLR(n,p)      (*p &= ~(1<<(n)))
#define FD_ISSET(n,p)    (*p & (1<<(n)))
#endif				/* defines for BSD 4.2 */

#ifdef HAS_GETRUSAGE
extern void rusage_stats(void);
#endif
extern int que_next(void);	/* from cque.c */
extern int on_second;
extern void dispatch(void);	/* from timer.c */
extern dbref email_register_player(const char *name, const char *email, const char *host, const char *ip);	/* from player.c */

extern time_t start_time, first_start_time;
extern int reboot_count;
#ifndef WIN32
#ifndef errno
extern int errno;
#endif
#endif
extern int reserved;
extern int paranoid_dump;	/* from game.c */

static int under_limit = 1;

char cf_motd_msg[BUFFER_LEN], cf_wizmotd_msg[BUFFER_LEN],
  cf_downmotd_msg[BUFFER_LEN], cf_fullmotd_msg[BUFFER_LEN];
char confname[BUFFER_LEN];
char errlog[BUFFER_LEN];

#define CONN_DEFAULT 0
/* Using Pueblo, Smial, Mushclient, Simplemu, or some other
 *  pueblo-style HTML aware client */
#define CONN_HTML 0x1
/* Using a client that understands telnet options */
#define CONN_TELNET 0x2
/* Send a telnet option to test client */
#define CONN_TELNET_QUERY 0x4

#define TELNET_ABLE(d) ((d)->conn_flags & (CONN_TELNET | CONN_TELNET_QUERY))


/* When the mush gets a new connection, it tries sending a telnet
 * option negotiation code for setting client-side line-editing mode
 * to it. If it gets a reply, a flag in the descriptor struct is
 * turned on indicated telnet-awareness.
 * 
 * If the reply indicates that the client supports linemode, further
 * instructions as to what linemode options are to be used is sent.
 * Those options: Client-side line editing, and expanding literal
 * client-side-entered tabs into spaces.
 * 
 * Option negotation requests sent by the client are processed,
 * with the only one we confirm rather than refuse outright being
 * suppress-go-ahead, since a number of telnet clients try it.
 *
 * The character 255 is the telnet option escape character, so when it
 * is sent to a telnet-aware client by itself (Since it's also often y-umlaut)
 * it must be doubled to escape it for the client. This is done automatically,
 * and is the original purpose of adding telnet option support.
 */

/* Telnet codes */
#define IAC 255			/* interpret as command: */
#define DONT 254		/* you are not to use option */
#define DO      253		/* please, you use option */
#define WONT 252		/* I won't use option */
#define WILL    251		/* I will use option */
#define SB      250		/* interpret as subnegotiation */
#define SE      240		/* end sub negotiation */
#define TN_SGA 3		/* Suppress go-ahead */
#define TN_LINEMODE 34
#define TN_NAWS 31		/* Negotiate About Window Size */
void test_telnet(DESC *d);
void setup_telnet(DESC *d);
int handle_telnet(DESC *d, unsigned char **q, unsigned char *qend);

#define DESC_ITER_CONN(d) \
        for(d = descriptor_list;(d);d=(d)->next) \
          if((d)->connected)

#define Hidden(d)        ((d->hide == 1) && Can_Hide(d->player))

#ifdef NT_TCP
/* for Windows NT IO-completion-port method of TCP/IP - NJG */

/* Windows NT TCP/IP routines written by Nick Gammon <nick@gammon.com.au> */

#include <process.h>
HANDLE CompletionPort;		/* IOs are queued up on this port */
SOCKET MUDListenSocket;		/* for our listening thread */
DWORD dwMUDListenThread;	/* thread handle for listening thread */
SOCKADDR_IN saServer;		/* for listening thread */
void __cdecl MUDListenThread(void *pVoid);	/* the listening thread */
DWORD platform;			/* which version of Windows are we using? */
OVERLAPPED lpo_aborted;		/* special to indicate a player has finished TCP IOs */
OVERLAPPED lpo_shutdown;	/* special to indicate a player should do a shutdown */
void ProcessWindowsTCP(void);	/* handle NT-style IOs */
CRITICAL_SECTION cs;		/* for thread synchronisation */
#endif


static const char *flushed_message = "\r\n<Output Flushed>\x1B[0m\r\n";
#define REBOOTFILE              "reboot.db"

#if 0
/* For translation */
static void dummy_msgs(void);
static void
dummy_msgs()
{
  char *temp;
  temp = T("Either that player does not exist, or has a different password.");
  temp =
    T
    ("Either there is already a player with that name, or that name is illegal.");
  temp = T("Unable to register that player with that email address.");
  temp = T("Registration successful! You will receive your password by email.");
  temp = T("Going down - Bye");
}

#endif

extern DESC *descriptor_list;
#ifdef WIN32
static WSADATA wsadata;
#endif
char ccom[BUFFER_LEN];
dbref cplr;


#ifndef BOOLEXP_DEBUGGING
#ifdef WIN32
void mainthread(int argc, char **argv);
#else
int main(int argc, char **argv);
#endif
#endif
void set_signals(void);
struct timeval *timeval_sub(struct timeval *now, struct timeval *then);
/* Windows doesn't have gettimeofday(). Implement it if on windows,
 * otherwise just call gettimeofday(). Cuts down on conditional code
 * elsewhere */
#ifdef WIN32
void win_gettimeofday(struct timeval *now);
#define our_gettimeofday(now) win_gettimeofday((now))
#else
#define our_gettimeofday(now) gettimeofday((now), (struct timezone *)NULL)
#endif
long int msec_diff(struct timeval *now, struct timeval *then);
struct timeval *msec_add(struct timeval *t, int x);
struct timeval *update_quotas(struct timeval *last, struct timeval *current);

void clearstrings(DESC *d);
typedef struct fblock {
  unsigned char *buff;
  size_t len;
} FBLOCK;

struct fcache_entries {
  FBLOCK connect_fcache[2];
  FBLOCK motd_fcache[2];
  FBLOCK wizmotd_fcache[2];
  FBLOCK newuser_fcache[2];
  FBLOCK register_fcache[2];
  FBLOCK quit_fcache[2];
  FBLOCK down_fcache[2];
  FBLOCK full_fcache[2];
  FBLOCK guest_fcache[2];
};
DESC *initializesock(int s, char *addr, char *ip);
struct text_block *make_text_block(const unsigned char *s, int n);
void free_text_block(struct text_block *t);
void add_to_queue(struct text_queue *q, const unsigned char *b, int n);
int flush_queue(struct text_queue *q, int n);
int queue_write(DESC *d, const unsigned char *b, int n);
int queue_newwrite(DESC *d, const unsigned char *b, int n);
int queue_string(DESC *d, const char *s);
int queue_string_eol(DESC *d, const char *s);
int queue_eol(DESC *d);
void freeqs(DESC *d);
void welcome_user(DESC *d);
void save_command(DESC *d, const unsigned char *command);
int process_input(DESC *d);
void process_input_helper(DESC *d, char *tbuf1, int got);
void set_userstring(unsigned char **userstring, const char *command);
int process_output(DESC *d);

enum na_type {
  NA_ASCII = 0,
  NA_ANSI,			/* ANSI flag */
  NA_COLOR,			/* ANSI and COLOR flags */
  NA_PUEBLO,			/* html */
  NA_PASCII,			/* Player without any of the above */
  NA_TANSI,			/* Like above with telnet-aware client */
  NA_TCOLOR,			/* Like above with telnet-aware client */
  NA_TPASCII,			/* Like above with telnet-aware client */
  NA_NANSI,			/* ANSI and NOACCENTS */
  NA_NCOLOR,			/* ANSI, COLOR, NOACCENTS */
  NA_NPUEBLO,			/* html & NOACCENTS */
  NA_NPASCII			/* 11. NOACCENTS */
};
#define TA_BGC 0
#define TA_FGC 1
#define TA_BOLD 2
#define TA_REV 3
#define TA_BLINK 4
#define TA_ULINE 5

static int na_depth = 0;

#define MESSAGE_TYPES 12

struct notify_strings {
  unsigned char *message;
  size_t len;
  int made;
};

static void fillstate(int state[], const unsigned char **f);
static void ansi_change_state(char *t, char **o, int color, int *state,
			      int *newstate);
static enum na_type notify_type(DESC *d);
static void free_strings(struct notify_strings messages[]);
static void zero_strings(struct notify_strings messages[]);
static unsigned char *notify_makestring(const char *message,
					struct notify_strings messages[],
					enum na_type type);


static void
fillstate(int state[6], const unsigned char **f)
{
  const unsigned char *p;
  int i;
  int n;
  p = *f;
  p++;
  if (*p != '[') {
    while (*p && *p != 'm')
      p++;
  } else {
    p++;
    while (*p && *p != 'm') {
      if ((*p > '9') || (*p < '0')) {
	/* Nada */
      } else if (!(*(p + 1)) || (*(p + 1) == 'm') || (*(p + 1) == ';')) {
	/* ShortCode */ ;
	switch (*p) {
	case '0':
	  for (i = 0; i < 6; i++)
	    state[i] = 0;
	  break;
	case '1':
	  state[TA_BOLD] = 1;
	  break;
	case '7':
	  state[TA_REV] = 1;
	  break;
	case '5':
	  state[TA_BLINK] = 1;
	  break;
	case '4':
	  state[TA_ULINE] = 1;
	  break;
	}
      } else {
	n = (*p - '0') * 10;
	p++;
	n += (*p - '0');
	if ((n >= 30) && (n <= 37))
	  state[TA_FGC] = n - 29;
	else if ((n >= 40) && (n <= 47))
	  state[TA_BGC] = n - 39;
      }
      p++;
    }
  }
  if ((p != *f) && (*p != 'm'))
    p--;
  *f = p;
}

#define add_ansi_if(x,c) \
  do { \
    if (newstate[(x)] && (newstate[(x)]!=state[(x)])) { \
      if (i) safe_chr(';',t,o); \
      safe_str((c),t,o); \
      i=1; \
    } \
  } while (0)

static void
ansi_change_state(char *t, char **o, int color, int *state, int *newstate)
{
  int i, n;
  if ((state[TA_BOLD] && !newstate[TA_BOLD]) ||
      (state[TA_REV] && !newstate[TA_REV]) ||
      (state[TA_BLINK] && !newstate[TA_BLINK]) ||
      (state[TA_ULINE] && !newstate[TA_ULINE]) ||
      (color && state[TA_FGC] && !newstate[TA_FGC]) ||
      (color && state[TA_BGC] && !newstate[TA_BGC])) {
    for (n = 0; n < 6; n++)
      state[n] = 0;
    safe_str(ANSI_NORMAL, t, o);
  }
  if ((newstate[TA_BOLD] && (newstate[TA_BOLD] != state[TA_BOLD])) ||
      (newstate[TA_REV] && (newstate[TA_REV] != state[TA_REV])) ||
      (newstate[TA_BLINK] && (newstate[TA_BLINK] != state[TA_BLINK])) ||
      (newstate[TA_ULINE] && (newstate[TA_ULINE] != state[TA_ULINE])) ||
      (color && newstate[TA_FGC] && (newstate[TA_FGC] != state[TA_FGC])) ||
      (color && newstate[TA_BGC] && (newstate[TA_BGC] != state[TA_BGC]))) {
    safe_chr(ESC_CHAR, t, o);
    safe_chr('[', t, o);
    i = 0;
    add_ansi_if(TA_BOLD, "1");
    add_ansi_if(TA_REV, "7");
    add_ansi_if(TA_BLINK, "5");
    add_ansi_if(TA_ULINE, "4");
    if (color) {
      add_ansi_if(TA_FGC, unparse_integer(newstate[TA_FGC] + 29));
      add_ansi_if(TA_BGC, unparse_integer(newstate[TA_BGC] + 39));
    }
    safe_chr('m', t, o);
  }
  for (n = 0; n < 6; n++)
    state[n] = newstate[n];
}

#undef add_ansi_if

int html_cols[] = {
  0xC0C0C0,			/* Default */
  0x000000,
  0xC00000,
  0x00C000,
  0xC0C000,
  0x0000C0,
  0xC000C0,
  0x00C0C0,
  0xC0C0C0
};

static void
zero_strings(struct notify_strings messages[])
{
  int n;
  for (n = 0; n < MESSAGE_TYPES; n++) {
    messages[n].message = NULL;
    messages[n].len = 0;
    messages[n].made = 0;
  }
}

static void
free_strings(struct notify_strings messages[])
{
  int n;
  for (n = 0; n < MESSAGE_TYPES; n++)
    if (messages[n].message)
      mush_free(messages[n].message, "string");
}

static unsigned char *
notify_makestring(const char *message, struct notify_strings messages[],
		  enum na_type type)
{
  char *o;
  const unsigned char *p;
  char *t;
  int state[6] = { 0, 0, 0, 0, 0, 0 };
  int newstate[6] = { 0, 0, 0, 0, 0, 0 };
  int n;
  int b, f;
  int changed = 0;
  int color = 0;
  int strip = 0;
  int fc, bc;
  char colbuff[128];
  static char tbuf[BUFFER_LEN];

  if (messages[type].made)
    return messages[type].message;
  messages[type].made = 1;

  p = (unsigned char *) message;
  o = tbuf;
  t = o;

  /* Since well over 50% is this type, we do it quick */
  switch (type) {
  case NA_ASCII:
    while (*p) {
      switch (*p) {
      case TAG_START:
	while (*p && *p != TAG_END)
	  p++;
	break;
      case '\r':
      case BEEP_CHAR:
	break;
      case ESC_CHAR:
	while (*p && *p != 'm')
	  p++;
	break;
      default:
	safe_chr(*p, t, &o);
      }
      p++;
    }
    *o = '\0';
    messages[type].message = (unsigned char *) mush_strdup(tbuf, "string");
    messages[type].len = o - tbuf;
    return messages[type].message;
  case NA_NPASCII:
    strip = 1;
  case NA_PASCII:
  case NA_TPASCII:
    /* PLAYER Ascii. Different output. \n is \r\n */
    if (type == NA_NPASCII)
      strip = 1;
    while (*p) {
      switch (*p) {
      case IAC:
	if (type == NA_TPASCII)
	  safe_str("\xFF\xFF", t, &o);
	else if (strip)
	  safe_str(accent_table[IAC].base, t, &o);
	else
	  safe_chr((char) IAC, t, &o);
	break;
      case TAG_START:
	while (*p && *p != TAG_END)
	  p++;
	break;
      case ESC_CHAR:
	while (*p && *p != 'm')
	  p++;
	break;
      case '\r':
	break;
      case '\n':
	safe_str("\r\n", t, &o);
	break;
      default:
	if (strip && accent_table[(unsigned char) *p].base)
	  safe_str(accent_table[(unsigned char) *p].base, t, &o);
	else
	  safe_chr(*p, t, &o);
      }
      p++;
    }
    *o = '\0';
    messages[type].message = (unsigned char *) mush_strdup(tbuf, "string");
    messages[type].len = o - tbuf;
    return messages[type].message;

  case NA_NPUEBLO:
    strip = 1;
  case NA_PUEBLO:
    while (*p) {
      if (*p == ESC_CHAR) {
	fillstate(newstate, &p);
	changed = 1;
      } else {
	if (changed) {
	  changed = 0;
	  /* Do we need to close any attribute tags? */
	  if (state[TA_BOLD] && !newstate[TA_BOLD])
	    safe_str("</B>", t, &o);
	  if (state[TA_ULINE] && !newstate[TA_ULINE])
	    safe_str("</U>", t, &o);
	  /* Are we going to start a new font? If so, temporarily
	   * close attribute tags */
	  if (!state[TA_BGC] && !state[TA_FGC] &&
	      (newstate[TA_FGC] || newstate[TA_FGC])) {
	    if (state[TA_BOLD] && newstate[TA_BOLD])
	      safe_str("</B>", t, &o);
	    if (state[TA_ULINE] && newstate[TA_ULINE])
	      safe_str("</U>", t, &o);
	  }
	  /* Are we changing fonts? If so, close the FONT tag and
	     any attribute tags */
	  if (((state[TA_BGC] || state[TA_FGC]) &&
	       ((state[TA_BGC] != newstate[TA_BGC]) ||
		(state[TA_FGC] != newstate[TA_FGC]))) ||
	      (state[TA_REV] != newstate[TA_REV])) {
	    /* If we're in bold, and staying bold, temporarily close the tag */
	    if (state[TA_BOLD] && newstate[TA_BOLD])
	      safe_str("</B>", t, &o);
	    if (state[TA_ULINE] && newstate[TA_ULINE])
	      safe_str("</U>", t, &o);
	    safe_str("</FONT>", t, &o);
	  }
	  /* Should we start a new FONT tag? */
	  if ((newstate[TA_BGC] && (state[TA_BGC] != newstate[TA_BGC]))
	      || (newstate[TA_FGC] && (state[TA_FGC] != newstate[TA_FGC]))
	      || (newstate[TA_REV] && (state[TA_REV] != newstate[TA_REV]))
	    ) {
	    fc = newstate[TA_FGC];
	    bc = newstate[TA_BGC];
	    if (newstate[TA_REV]) {
	      if (!fc)
		fc = 8;
	      if (!bc)
		bc = 1;
	      f = html_cols[bc];
	      b = html_cols[fc];
	    } else {
	      f = html_cols[fc];
	      b = html_cols[bc];
	    }
	    if (bc && fc) {
	      sprintf(colbuff, "<FONT FGCOLOR=#%x BGCOLOR=#%x>", f, b);
	    } else if (fc) {
	      sprintf(colbuff, "<FONT FGCOLOR=#%x>", f);
	    } else {
	      sprintf(colbuff, "<FONT BGCOLOR=#%x>", b);
	    }
	    safe_str(colbuff, t, &o);
	    /* Should we reopen attribute tags, too? */
	    if (state[TA_BOLD] && newstate[TA_BOLD])
	      safe_str("<B>", t, &o);
	    if (state[TA_ULINE] && newstate[TA_ULINE])
	      safe_str("<U>", t, &o);
	  }
	  /* If we weren't bold before, or if we were bold+font and now
	   * are just bold, we reopen bold tag if we're going to be bold
	   */
	  if ((!state[TA_BOLD] || (state[TA_BOLD] && (state[TA_FGC] ||
						      state[TA_BGC])
				   && !newstate[TA_FGC] && !newstate[TA_BGC]))
	      && newstate[TA_BOLD])
	    safe_str("<B>", t, &o);
	  if ((!state[TA_ULINE] || (state[TA_ULINE] && (state[TA_FGC] ||
							state[TA_BGC])
				    && !newstate[TA_FGC] && !newstate[TA_BGC]))
	      && newstate[TA_ULINE])
	    safe_str("<U>", t, &o);
	  for (n = 0; n < 6; n++)
	    state[n] = newstate[n];
	}
	switch (*p) {
	case TAG_START:
	  safe_chr('<', t, &o);
	  p++;
	  while ((*p) && (*p != TAG_END)) {
	    safe_chr(*p, t, &o);
	    p++;
	  }
	  safe_chr('>', t, &o);
	  break;
	case TAG_END:
	  /* Should never be seen alone */
	  break;
	case '\r':
	  break;
	default:
	  if (strip) {
	    /* Even if we're NOACCENTS, we must still translate a few things */
	    switch ((unsigned char) *p) {
	    case '\n':
	    case '&':
	    case '<':
	    case '>':
	      safe_str(accent_table[(unsigned char) *p].entity, t, &o);
	      break;
	    default:
	      if (accent_table[(unsigned char) *p].base)
		safe_str(accent_table[(unsigned char) *p].base, t, &o);
	      else
		safe_chr(*p, t, &o);
	      break;
	    }
	  } else if (accent_table[(unsigned char) *p].entity)
	    safe_str(accent_table[(unsigned char) *p].entity, t, &o);
	  else
	    safe_chr(*p, t, &o);
	}
      }
      p++;
    }
    if (state[TA_BGC] || state[TA_FGC] || state[TA_REV])
      safe_str("</FONT>", t, &o);
    if (state[TA_BOLD])
      safe_str("</B>", t, &o);
    if (state[TA_ULINE])
      safe_str("</U>", t, &o);
    break;
  case NA_COLOR:
  case NA_TCOLOR:
  case NA_NCOLOR:
    color = 1;
    /* FALLTHROUGH */
  case NA_ANSI:
  case NA_TANSI:
  case NA_NANSI:
    if (type == NA_NCOLOR || type == NA_NANSI)
      strip = 1;
    while (*p) {
      switch ((unsigned char) *p) {
      case IAC:
	if (changed) {
	  changed = 0;
	  ansi_change_state(t, &o, color, state, newstate);
	}
	if (type == NA_TANSI || type == NA_TCOLOR)
	  safe_str("\xFF\xFF", t, &o);
	else if (strip && accent_table[IAC].base)
	  safe_str(accent_table[IAC].base, t, &o);
	else
	  safe_chr((char) IAC, t, &o);
	break;
      case TAG_START:
	while (*p && *p != TAG_END)
	  p++;
	break;
      case '\r':
	break;
      case '\n':
	if (changed) {
	  changed = 0;
	  ansi_change_state(t, &o, color, state, newstate);
	}
	safe_str("\r\n", t, &o);

	break;
      case ESC_CHAR:
	fillstate(newstate, &p);
	changed = 1;
	break;
      default:
	if (changed) {
	  changed = 0;
	  ansi_change_state(t, &o, color, state, newstate);
	}
	if (strip && accent_table[(unsigned char) *p].base)
	  safe_str(accent_table[(unsigned char) *p].base, t, &o);
	else
	  safe_chr(*p, t, &o);
      }
      p++;
    }
    if (state[TA_BOLD] || state[TA_REV] ||
	state[TA_BLINK] || state[TA_ULINE] ||
	(color && (state[TA_FGC] || state[TA_BGC])))
      safe_str(ANSI_NORMAL, t, &o);

    break;
  }

  *o = '\0';
  messages[type].message = (unsigned char *) mush_strdup(tbuf, "string");
  messages[type].len = o - tbuf;
  return messages[type].message;
}

dbref
na_one(dbref current, void *data)
{
  if (current == NOTHING)
    return *((dbref *) data);
  else
    return NOTHING;
}

dbref
na_next(dbref current, void *data)
{
  if (current == NOTHING)
    return *((dbref *) data);
  else
    return Next(current);
}

dbref
na_loc(dbref current, void *data)
{
  dbref loc = *((dbref *) data);
  if (current == NOTHING)
    return loc;
  else if (current == loc)
    return Contents(current);
  else
    return Next(current);
}

dbref
na_nextbut(dbref current, void *data)
{
  dbref *dbrefs = data;

  do {
    if (current == NOTHING)
      current = dbrefs[0];
    else
      current = Next(current);
  } while (current == dbrefs[1]);
  return current;
}

dbref
na_except(dbref current, void *data)
{
  dbref *dbrefs = data;

  do {
    if (current == NOTHING)
      current = dbrefs[0];
    else if (current == dbrefs[0])
      current = Contents(current);
    else
      current = Next(current);
  } while (current == dbrefs[1]);
  return current;
}

dbref
na_except2(dbref current, void *data)
{
  dbref *dbrefs = data;

  do {
    if (current == NOTHING)
      current = dbrefs[0];
    else if (current == dbrefs[0])
      current = Contents(current);
    else
      current = Next(current);
  } while ((current == dbrefs[1]) || (current == dbrefs[2]));
  return current;
}

/* Works like na_except2, but with multiple exceptions.
 * dbrefs[0] should indicate the number of exceptions.
 * dbrefs[1] is the dbref of the room the message was emitted in.
 * dbrefs[2] and up are the exceptions.
 */
dbref
na_exceptN(dbref current, void *data)
{
  dbref *dbrefs = data;
  int i, check;

  do {
    if (current == NOTHING)
      current = dbrefs[1];
    else if (current == dbrefs[1])
      current = Contents(current);
    else
      current = Next(current);
    check = 0;
    for (i = 2; i < dbrefs[0] + 2; i++)
      if (current == dbrefs[i])
	check = 1;
  } while (check);
  return current;
}


static int nohtml_strlen(const char *s, dbref puppet);
static int
nohtml_strlen(const char *s, dbref puppet)
{
  const char *p = s;
  int n = 0;

  if (GoodObject(puppet)) {
    char *name = strstr(s, tprintf("%s> ", Name(puppet)));
    if (name == s) {
      p += strlen(Name(puppet)) + 2;
    }
  }

  for (; *p != '\0'; p++) {
    switch (*p) {
    case TAG_START:
      do {
	p++;
      } while (*p != '\0' && *p != TAG_END);
      break;
    case TAG_END:		/* Just in case */
      break;
    default:
      n++;
    }
  }
  return n;
}

static enum na_type
notify_type(DESC *d)
{
  enum na_type poutput;
  int strip = 0;

  if (!d->connected || IS(d->player, TYPE_PLAYER, "NOACCENTS"))
    strip = 1;

  if (d->conn_flags & CONN_HTML) {
    poutput = strip ? NA_NPUEBLO : NA_PUEBLO;
  } else if (ShowAnsi(d->player)) {
    if (ShowAnsiColor(d->player)) {
      if (strip)
	poutput = NA_NCOLOR;
      else
	poutput = (d->conn_flags & CONN_TELNET) ? NA_TCOLOR : NA_COLOR;
    } else {
      if (strip)
	poutput = NA_NANSI;
      else
	poutput = (d->conn_flags & CONN_TELNET) ? NA_TANSI : NA_ANSI;
    }
  } else {
    if (strip)
      poutput = NA_NPASCII;
    else
      poutput = (d->conn_flags & CONN_TELNET) ? NA_TPASCII : NA_PASCII;
  }
  return poutput;
}

void
notify_anything(dbref speaker, na_lookup func,
		void *fdata, char *(*nsfunc) (dbref, na_lookup func, void *,
					      int), int flags,
		const char *message)
{
  dbref target;
  dbref passalong[3];
  struct notify_strings messages[MESSAGE_TYPES];
  struct notify_strings nospoofs[MESSAGE_TYPES];
  struct notify_strings paranoids[MESSAGE_TYPES];
  int i, j;
  DESC *d;
  enum na_type poutput;
  unsigned char *pstring;
  size_t plen;
  char *bp;
  ATTR *a;
  char *asave;
  char const *ap;
  char *preserve[NUMQ];
  int havespoof = 0;
  int havepara = 0;
  char *wsave[10];
  char *tbuf1 = NULL, *nospoof = NULL, *paranoid = NULL, *msgbuf;
  static dbref puppet = NOTHING;

  if (!message || *message == '\0' || !func)
    return;

  /* Depth check */
  if (na_depth > 7)
    return;
  na_depth++;

  /* Only allocate these buffers when needed */
  for (i = 0; i < MESSAGE_TYPES; i++) {
    messages[i].message = NULL;
    messages[i].made = 0;
    nospoofs[i].message = NULL;
    nospoofs[i].made = 0;
    paranoids[i].message = NULL;
    paranoids[i].made = 0;
  }

  msgbuf = mush_strdup(message, "string");

  target = NOTHING;

  while ((target = func(target, fdata)) != NOTHING) {
    if ((flags & NA_PONLY) && !IsPlayer(target))
      continue;

    if (IsPlayer(target)) {
      if (!Connected(target) && options.login_allow && under_limit)
	continue;

      if (flags & NA_INTERACTION) {
	if ((flags & NA_INTER_DIDIT) &&
	    !can_interact(speaker, target, INTERACT_SEE))
	  continue;
	else if (!(flags & NA_INTER_DIDIT)
		 && !can_interact(speaker, target, INTERACT_HEAR))
	  continue;
      }

      for (d = descriptor_list; d; d = d->next) {
	if (d->connected && d->player == target) {
	  poutput = notify_type(d);

	  if ((flags & NA_PONLY) && (poutput != NA_PUEBLO))
	    continue;

	  if (nsfunc && Nospoof(target) && (target != speaker)) {
	    if (Paranoid(target)) {
	      if (!havepara) {
		paranoid = nsfunc(speaker, func, fdata, 1);
		havepara = 1;
	      }
	      pstring = notify_makestring(paranoid, paranoids, poutput);
	      plen = paranoids[poutput].len;
	    } else {
	      if (!havespoof) {
		nospoof = nsfunc(speaker, func, fdata, 0);
		havespoof = 1;
	      }
	      pstring = notify_makestring(nospoof, nospoofs, poutput);
	      plen = nospoofs[poutput].len;
	    }
	    queue_newwrite(d, pstring, plen);
	  }
	  pstring = notify_makestring(msgbuf, messages, poutput);
	  plen = messages[poutput].len;

	  if (pstring && *pstring
	      && ((d->conn_flags & CONN_HTML) || !(flags & NA_NOENTER)
		  || ((flags & NA_PUPPET2)
		      && nohtml_strlen(msgbuf, puppet) > 0))) {
	    queue_newwrite(d, pstring, plen);
	  }
	  if (!(flags & NA_NOENTER)) {
	    if ((poutput == NA_PUEBLO) || (poutput == NA_NPUEBLO)) {
	      if (flags & NA_NOPENTER)
		queue_newwrite(d, (unsigned char *) "\n", 1);
	      else
		queue_newwrite(d, (unsigned char *) "<BR>\n", 5);
	    } else {
	      queue_newwrite(d, (unsigned char *) "\r\n", 2);
	    }
	  }
	}
      }
    } else if (Puppet(target) &&
	       ((Location(target) != Location(Owner(target))) ||
		has_flag_by_name(target, "VERBOSE", NOTYPE) ||
		(flags & NA_MUST_PUPPET)) &&
	       ((flags & NA_PUPPET) || !(flags & NA_NORELAY))) {
      dbref last = puppet;
      puppet = target;
      if (!tbuf1)
	tbuf1 = (char *) mush_malloc(BUFFER_LEN, "string");
      bp = tbuf1;
      safe_str(Name(target), tbuf1, &bp);
      safe_str("> ", tbuf1, &bp);
      safe_str(msgbuf, tbuf1, &bp);
      *bp = '\0';
      notify_anything(speaker, na_one, &Owner(target), NULL,
		      flags | NA_NORELAY | NA_PUPPET2, tbuf1);
      puppet = last;
    }
    if ((flags & NA_NOLISTEN)
	|| (!PLAYER_LISTEN && IsPlayer(target))
	|| IsExit(target))
      continue;

    /* do @listen stuff */
    a = atr_get_noparent(target, "LISTEN");
    if (a) {
      if (!tbuf1)
	tbuf1 = (char *) mush_malloc(BUFFER_LEN, "string");
      strcpy(tbuf1, uncompress(a->value));
      if ((AL_FLAGS(a) & AF_REGEXP)
	  ? regexp_match_case(tbuf1,
			      (char *) notify_makestring(msgbuf, messages,
							 NA_ASCII),
			      AL_FLAGS(a) & AF_CASE)
	  : wild_match_case(tbuf1,
			    (char *) notify_makestring(msgbuf, messages,
						       NA_ASCII),
			    AL_FLAGS(a) & AF_CASE)) {
	if (eval_lock(speaker, target, Listen_Lock))
	  if (PLAYER_AHEAR || (!IsPlayer(target))) {
	    if (speaker != target)
	      charge_action(speaker, target, "AHEAR");
	    else
	      charge_action(speaker, target, "AMHEAR");
	    charge_action(speaker, target, "AAHEAR");
	  }
	if (!(flags & NA_NORELAY) && !member(speaker, Contents(target)) &&
	    !filter_found(target,
			  (char *) notify_makestring(msgbuf, messages,
						     NA_ASCII), 1)) {
	  passalong[0] = target;
	  passalong[1] = target;
	  passalong[2] = Owner(target);
	  a = atr_get(target, "INPREFIX");
	  if (a) {
	    for (j = 0; j < 10; j++)
	      wsave[j] = wenv[j];
	    wenv[0] = (char *) msgbuf;
	    for (j = 1; j < 10; j++)
	      wenv[j] = NULL;
	    save_global_regs("inprefix_save", preserve);
	    asave = safe_uncompress(a->value);
	    ap = asave;
	    bp = tbuf1;
	    process_expression(tbuf1, &bp, &ap, target, speaker, speaker,
			       PE_DEFAULT, PT_DEFAULT, NULL);
	    if (bp != tbuf1)
	      safe_chr(' ', tbuf1, &bp);
	    safe_str(msgbuf, tbuf1, &bp);
	    *bp = 0;
	    free(asave);
	    restore_global_regs("inprefix_save", preserve);
	    for (j = 0; j < 10; j++)
	      wenv[j] = wsave[j];
	  }
	  notify_anything(speaker, Puppet(target) ? na_except2 : na_except,
			  passalong, NULL, flags | NA_NORELAY,
			  (a) ? tbuf1 : msgbuf);
	}
      }
    }
    /* if object is flagged LISTENER, check for ^ listen patterns
     *    * these are like AHEAR - object cannot trigger itself.
     *    * unlike normal @listen, don't pass the message on.
     *    */

    if ((speaker != target) && (ThingListen(target) || RoomListen(target))
	&& eval_lock(speaker, target, Listen_Lock)
      )
      atr_comm_match(target, speaker, '^', ':',
		     (char *) notify_makestring(msgbuf, messages, NA_ASCII), 0,
		     NULL, NULL);

    /* If object is flagged AUDIBLE and has a @FORWARDLIST, send
     *  stuff on */
    if (!(flags & NA_NORELAY) && Audible(target)
	&& ((a = atr_get_noparent(target, "FORWARDLIST")) != NULL)
	&& !filter_found(target, msgbuf, 0)) {
      char *fwdstr, *orig, *curr;
      dbref fwd;

      orig = safe_uncompress(AL_STR(a));
      fwdstr = trim_space_sep(orig, ' ');

      if (!tbuf1)
	tbuf1 = mush_malloc(BUFFER_LEN, "string");
      bp = tbuf1;
      make_prefixstr(target, msgbuf, tbuf1);

      while ((curr = split_token(&fwdstr, ' ')) != NULL) {
	if (is_dbref(curr)) {
	  fwd = parse_dbref(curr);
	  if (GoodObject(fwd) && !IsGarbage(fwd) && Can_Forward(target, fwd)) {
	    if (IsRoom(fwd)) {
	      notify_anything(speaker, na_loc, &fwd, NULL,
			      flags | NA_NORELAY, tbuf1);
	    } else {
	      notify_anything(speaker, na_one, &fwd, NULL,
			      flags | NA_NORELAY, tbuf1);
	    }
	  }
	}
      }
      free((Malloc_t) orig);
    }
  }

  for (i = 0; i < MESSAGE_TYPES; i++) {
    if (messages[i].message)
      mush_free((Malloc_t) messages[i].message, "string");
    if (nospoofs[i].message)
      mush_free((Malloc_t) nospoofs[i].message, "string");
    if (paranoids[i].message)
      mush_free((Malloc_t) paranoids[i].message, "string");
  }
  if (nospoof)
    mush_free((Malloc_t) nospoof, "string");
  if (paranoid)
    mush_free((Malloc_t) paranoid, "string");
  if (tbuf1)
    mush_free((Malloc_t) tbuf1, "string");
  mush_free((Malloc_t) msgbuf, "string");
  na_depth--;
}

int
safe_tag(char const *a_tag, char *buf, char **bp)
{
  int result = 0;

  if (SUPPORT_PUEBLO) {
    result += safe_chr(TAG_START, buf, bp);
    result += safe_str(a_tag, buf, bp);
    result += safe_chr(TAG_END, buf, bp);
  }
  return result;
}

int
safe_tag_cancel(char const *a_tag, char *buf, char **bp)
{
  int result = 0;
  if (SUPPORT_PUEBLO) {
    result += safe_chr(TAG_START, buf, bp);
    result += safe_chr('/', buf, bp);
    result += safe_str(a_tag, buf, bp);
    result += safe_chr(TAG_END, buf, bp);
  }
  return result;
}

int
safe_tag_wrap(char const *a_tag, char const *params, char const *data,
	      char *buf, char **bp)
{
  int result = 0;
  if (SUPPORT_PUEBLO) {
    result += safe_chr(TAG_START, buf, bp);
    result += safe_str(a_tag, buf, bp);
    if (params && *params) {
      result += safe_chr(' ', buf, bp);
      result += safe_str(params, buf, bp);
    }
    result += safe_chr(TAG_END, buf, bp);
  }
  result += safe_str(data, buf, bp);
  if (SUPPORT_PUEBLO) {
    result += safe_tag_cancel(a_tag, buf, bp);
  }
  return result;
}

void
raw_notify(dbref player, const char *msg)
{
  notify_anything(GOD, na_one, &player, NULL, NA_NOLISTEN, msg);
}

/* Rewrite this to compute a flag mask from a list of flags names */
/* You must have flag1 and flag2 to get the broadcast */
void
 WIN32_CDECL
flag_broadcast(const char *flag1, const char *flag2, const char *fmt, ...)
{
  /* takes flag/toggle masks, format string, and format args, and notifies
   * all connected players with that flag mask of something.
   * If inflags=intoggles=0, broadcast to all.
   * If one of them is 0, and the other isn't, then players
   * with _at least_ one of the bits in the flag mask are notified,
   * rather than players with that _entire_ flag mask. The former
   * behavior is more useful.
   * If both are nonzero, the player must have at least one bit in each.
   */

  va_list args;
  char tbuf1[BUFFER_LEN];
  DESC *d;
  int ok;

  va_start(args, fmt);

#ifdef HAS_VSNPRINTF
  (void) vsnprintf(tbuf1, sizeof tbuf1, fmt, args);
#else
  (void) vsprintf(tbuf1, fmt, args);
#endif
  va_end(args);
  tbuf1[BUFFER_LEN - 1] = '\0';

  DESC_ITER_CONN(d) {
    ok = 1;
    if (flag1)
      ok = ok && flaglist_check_long(GOD, d->player, flag1, 0);
    if (flag2)
      ok = ok && flaglist_check_long(GOD, d->player, flag2, 0);
    if (ok) {
      queue_string_eol(d, tbuf1);
      process_output(d);
    }
  }
}



struct text_block *
make_text_block(const unsigned char *s, int n)
{
  struct text_block *p;
  p =
    (struct text_block *) mush_malloc(sizeof(struct text_block), "text_block");
  if (!p)
    panic("Out of memory");
  p->buf =
    (unsigned char *) mush_malloc(sizeof(unsigned char) * n, "text_block_buff");
  if (!p->buf)
    panic("Out of memory");

  memcpy(p->buf, s, n);
  p->nchars = n;
  p->start = p->buf;
  p->nxt = 0;
  return p;
}

void
free_text_block(struct text_block *t)
{
  if (t) {
    if (t->buf)
      mush_free((Malloc_t) t->buf, "text_block_buff");
    mush_free((Malloc_t) t, "text_block");
  }
}

void
add_to_queue(struct text_queue *q, const unsigned char *b, int n)
{
  struct text_block *p;

  if (n == 0)
    return;

  p = make_text_block(b, n);
  p->nxt = 0;
  *q->tail = p;
  q->tail = &p->nxt;
}

int
flush_queue(struct text_queue *q, int n)
{
  struct text_block *p;
  int really_flushed = 0;
  n += strlen(flushed_message);

  while (n > 0 && (p = q->head)) {
    n -= p->nchars;
    really_flushed += p->nchars;
    q->head = p->nxt;
#ifdef DEBUG
    do_rawlog(LT_ERR, "free_text_block(0x%x) at 1.", p);
#endif				/* DEBUG */
    free_text_block(p);
  }
  p =
    make_text_block((unsigned char *) flushed_message, strlen(flushed_message));
  p->nxt = q->head;
  q->head = p;
  if (!p->nxt)
    q->tail = &p->nxt;
  really_flushed -= p->nchars;
  return really_flushed;
}

int
queue_write(DESC *d, const unsigned char *b, int n)
{
  char buff[BUFFER_LEN];
  struct notify_strings messages[MESSAGE_TYPES];
  unsigned char *s;
  PUEBLOBUFF;
  size_t len;

  if ((n == 2) && (b[0] == '\r') && (b[1] == '\n')) {
    if ((d->conn_flags & CONN_HTML))
      queue_newwrite(d, (unsigned char *) "<BR>\n", 5);
    else
      queue_newwrite(d, b, 2);
    return n;
  }
  if (n > BUFFER_LEN)
    n = BUFFER_LEN;

  memcpy(buff, b, n);
  buff[n] = '\0';

  zero_strings(messages);

  if (d->conn_flags & CONN_HTML) {
    PUSE;
    tag_wrap("SAMP", NULL, buff);
    PEND;
    s = notify_makestring(pbuff, messages, NA_PUEBLO);
    len = messages[NA_PUEBLO].len;
  } else {
    s = notify_makestring(buff, messages, notify_type(d));
    len = messages[notify_type(d)].len;
  }
  queue_newwrite(d, s, len);
  free_strings(messages);
  return n;
}

int
queue_newwrite(DESC *d, const unsigned char *b, int n)
{
  int space;

#ifdef NT_TCP

  /* queue up an asynchronous write if using Windows NT */
  if (platform == VER_PLATFORM_WIN32_NT) {

    struct text_block **qp, *cur;
    DWORD nBytes;
    BOOL bResult;

    /* don't write if connection dropped */
    if (d->bConnectionDropped)
      return n;

    /* add to queue - this makes sure output arrives in the right order */
    add_to_queue(&d->output, b, n);
    d->output_size += n;

    /* if we are already writing, exit and wait for IO completion */
    if (d->bWritePending)
      return n;

    /* pull head item out of queue - not necessarily the one we just put in */
    qp = &d->output.head;

    if ((cur = *qp) == NULL)
      return n;			/* nothing in queue - rather strange, but oh well. */


    /* if buffer too long, write what we can and queue up the rest */

    if (cur->nchars <= sizeof(d->output_buffer)) {
      nBytes = cur->nchars;
      memcpy(d->output_buffer, cur->start, nBytes);
      if (!cur->nxt)
	d->output.tail = qp;
      *qp = cur->nxt;
      free_text_block(cur);
    }
    /* end of able to write the lot */
    else {
      nBytes = sizeof(d->output_buffer);
      memcpy(d->output_buffer, cur->start, nBytes);
      cur->nchars -= nBytes;
      cur->start += nBytes;
    }				/* end of buffer too long */

    d->OutboundOverlapped.Offset = 0;
    d->OutboundOverlapped.OffsetHigh = 0;

    bResult = WriteFile((HANDLE) d->descriptor,
			d->output_buffer, nBytes, NULL, &d->OutboundOverlapped);

    d->bWritePending = FALSE;

    if (!bResult)
      if (GetLastError() == ERROR_IO_PENDING)
	d->bWritePending = TRUE;
      else {
	d->bConnectionDropped = TRUE;	/* do no more writes */
	/* post a notification that the descriptor should be shutdown */
	if (!PostQueuedCompletionStatus
	    (CompletionPort, 0, (DWORD) d, &lpo_shutdown)) {
	  char sMessage[200];
	  DWORD nError = GetLastError();
	  GetErrorMessage(nError, sMessage, sizeof sMessage);
	  printf
	    ("Error %ld (%s) on PostQueuedCompletionStatus in queue_newwrite\n",
	     nError, sMessage);
	  shutdownsock(d);
	}
      }				/* end of had write */
    return n;

  }				/* end of NT TCP/IP way of doing it */
#endif

  space = MAX_OUTPUT - d->output_size - n;
  if (space < SPILLOVER_THRESHOLD) {
    process_output(d);
    space = MAX_OUTPUT - d->output_size - n;
    if (space < 0)
      d->output_size -= flush_queue(&d->output, -space);
  }
  add_to_queue(&d->output, b, n);
  d->output_size += n;
  return n;
}

int
queue_eol(DESC *d)
{
  if (SUPPORT_PUEBLO && (d->conn_flags & CONN_HTML))
    return queue_newwrite(d, (unsigned char *) "<BR>\n", 5);
  else
    return queue_newwrite(d, (unsigned char *) "\r\n", 2);
}

int
queue_string_eol(DESC *d, const char *s)
{
  queue_string(d, s);
  return queue_eol(d);
}

int
queue_string(DESC *d, const char *s)
{
  unsigned char *n;
  enum na_type poutput;
  struct notify_strings messages[MESSAGE_TYPES];
  dbref target;
  int ret;

  zero_strings(messages);

  target = d->player;

  poutput = notify_type(d);

  n = notify_makestring(s, messages, poutput);
  ret = queue_newwrite(d, n, messages[poutput].len);
  free_strings(messages);
  return ret;
}


void
freeqs(DESC *d)
{
  struct text_block *cur, *next;
  cur = d->output.head;
  while (cur) {
    next = cur->nxt;
#ifdef DEBUG
    do_rawlog(LT_ERR, "free_text_block(0x%x) at 3.", cur);
#endif				/* DEBUG */
    free_text_block(cur);
    cur = next;
  }
  d->output.head = 0;
  d->output.tail = &d->output.head;

  cur = d->input.head;
  while (cur) {
    next = cur->nxt;
#ifdef DEBUG
    do_rawlog(LT_ERR, "free_text_block(0x%x) at 4.", cur);
#endif				/* DEBUG */
    free_text_block(cur);
    cur = next;
  }
  d->input.head = 0;
  d->input.tail = &d->input.head;

  if (d->raw_input) {
    mush_free((Malloc_t) d->raw_input, "descriptor_raw_input");
  }
  d->raw_input = 0;
  d->raw_input_at = 0;
}
