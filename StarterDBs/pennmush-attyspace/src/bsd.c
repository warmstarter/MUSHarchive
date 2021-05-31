/* bsd.c */

/* Windows NT users may uncomment this define to get the native network i/o
 * thread model instead of the bsd socket layer, for vastly better
 * performance. Doesn't work on Win 95/98. By Nick Gammon 
 */
/* #define NT_TCP */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#ifdef I_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#ifdef I_MEMORY
#include <memory.h>
#endif
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#ifdef WIN32
#define FD_SETSIZE 256
#include <winsock.h>
#include <io.h>
#undef OPAQUE			/* Clashes with flags.h */
#define EINTR WSAEINTR
#define EMFILE WSAEMFILE
#define EWOULDBLOCK WSAEWOULDBLOCK
#define MAXHOSTNAMELEN 32
void shutdown_checkpoint _((void));
#ifdef INFO_SLAVE
#undef INFO_SLAVE
#endif
#else				/* WIN32 */
#ifdef I_SYS_FILE
#include <sys/file.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#endif
#include <sys/ioctl.h>
#ifdef I_ERRNO
#include <errno.h>
#else
#ifdef I_SYS_ERRNO
#include <sys/errno.h>
#endif
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#ifdef I_SYS_PARAM
#include <sys/param.h>
#endif
#endif				/* WIN32 */
#ifdef I_TIME
#include <time.h>
#endif
#ifdef I_SYS_WAIT
#include <sys/wait.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_SYS_SELECT
#include <sys/select.h>
#endif
#ifdef I_UNISTD
#include <unistd.h>
#endif
#ifdef HAS_GETRLIMIT
#include <sys/resource.h>
#endif
#ifdef I_LIMITS
#include <limits.h>
#endif
#ifdef I_ARPA_INET
#include <arpa/inet.h>
#endif
#ifdef I_FLOATINGPOINT
#include <floatingpoint.h>
#endif

#include "conf.h"
#include "mushdb.h"
#include "intrface.h"
#include "externs.h"
#include "globals.h"
#include "help.h"
#include "match.h"
#include "ansi.h"
#include "pueblo.h"
#include "parse.h"
#include "access.h"
#include "version.h"
#include "ident.h"

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
#include "confmagic.h"

/* BSD 4.2 and maybe some others need these defined */
#ifndef FD_ZERO
#define fd_set int
#define FD_ZERO(p)       (*p = 0)
#define FD_SET(n,p)      (*p |= (1<<(n)))
#define FD_CLR(n,p)      (*p &= ~(1<<(n)))
#define FD_ISSET(n,p)    (*p & (1<<(n)))
#endif				/* defines for BSD 4.2 */

/* Win32 uses closesocket() to close a socket, and so will we */
#ifndef WIN32
#define closesocket(s)  close(s)
#endif

/* What htons expects */
#ifdef CAN_NEWSTYLE
#define Port_t unsigned short
#else
#define Port_t int
#endif

#ifdef HAS_GETRUSAGE
extern void rusage_stats _((void));
#endif
extern int que_next _((void));	/* from cque.c */
extern void dispatch _((void));	/* from timer.c */
extern dbref email_register_player _((const char *name, const char *email, const char *host, const char *ip));	/* from player.c */
static int how_many_fds _((void));
static void fillstate _((int state[], char **f));

extern time_t start_time;
extern dbref orator;
#ifndef WIN32
extern int errno;
#endif
extern int reserved;
static int extrafd;
int shutdown_flag = 0;
extern dbref db_top;

static int login_number = 0;
static int under_limit = 1;

char cf_motd_msg[BUFFER_LEN], cf_wizmotd_msg[BUFFER_LEN], cf_downmotd_msg[BUFFER_LEN],
 cf_fullmotd_msg[BUFFER_LEN];
static char poll_msg[DOING_LEN];
char confname[BUFFER_LEN];
char errlog[BUFFER_LEN];

struct text_block {
  int nchars;
  struct text_block *nxt;
  char *start;
  char *buf;
};

struct text_queue {
  struct text_block *head;
  struct text_block **tail;
};

struct descriptor_data {
  int descriptor;
  int connected;
  char addr[101];
  char ip[101];
  dbref player;
  char *output_prefix;
  char *output_suffix;
  int output_size;
  struct text_queue output;
  struct text_queue input;
  char *raw_input;
  char *raw_input_at;
  long connected_at;
  long last_time;
  int quota;
  int cmds;
  int hide;
  char doing[DOING_LEN];
#ifdef NT_TCP
  /* these are for the Windows NT TCP/IO */
  char input_buffer[512];	/* buffer for reading */
  char output_buffer[512];	/* buffer for writing */
  OVERLAPPED InboundOverlapped;	/* for asynchronous reading */
  OVERLAPPED OutboundOverlapped;	/* for asynchronous writing */
  BOOL bWritePending;		/* true if in process of writing */
  BOOL bConnectionDropped;	/* true if we cannot send to player */
  BOOL bConnectionShutdown;	/* true if connection has been shutdown */
#endif
  struct descriptor_data *next;
  struct descriptor_data *prev;
#ifdef USE_MAILER
  struct mail *mailp;
#endif
  int pueblo;
};

#define DESC_ITER_CONN(d) \
        for(d = descriptor_list;(d);d=(d)->next) \
	  if((d)->connected)

#define Hidden(d)        ((d->hide == 1) && Can_Hide(d->player))

/* log file pointers */
FILE *connlog_fp;
FILE *checklog_fp;
FILE *wizlog_fp;
FILE *tracelog_fp;
FILE *cmdlog_fp;
#ifdef TREKMUSH
FILE *spacelog_fp;
#endif /* TREKMUSH */

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

static const char *connect_fail = "Either that player does not exist, or has a different password.\r\n";
static const char *create_fail = "Either there is already a player with that name, or that name is illegal.\r\n";
static const char *register_fail = "Unable to register that player with that email address.\r\n";
static const char *register_success = "Registration successful! You will receive your password by email.\r\n";
#ifdef OLD_ANSI
static const char *flushed_message = "\r\n<Output Flushed>\033[0m\r\n";
#else
static const char *flushed_message = "\r\n<Output Flushed>\x1B[0m\r\n";
#endif
static const char *shutdown_message = "Going down - Bye\r\n";
static const char *asterisk_line =
"**********************************************************************";
#define REBOOTFILE		"reboot.db"

DESC *descriptor_list = 0;

static int sock;
static int ndescriptors = 0;
int restarting = 0;
static int maxd;
char ccom[BUFFER_LEN];
dbref cplr;
DESC *cdesc = 0;

#ifdef INFO_SLAVE
static fd_set info_pending;
static int info_slave;
static Pid_t info_slave_pid;
static int info_slave_state = 0;
static int info_query_spill, info_reap_spill;
static time_t info_queue_time = 0;
#endif

#ifdef HAS_GETRLIMIT
static void init_rlimit _((void));
#endif
#ifndef BOOLEXP_DEBUGGING
#ifdef WIN32
void mainthread _((int argc, char **argv));
#else
int main _((int argc, char **argv));
#endif
#endif
void set_signals _((void));
void raw_notify _((dbref player, const char *msg));
struct timeval timeval_sub _((struct timeval now, struct timeval then));
long int msec_diff _((struct timeval now, struct timeval then));
struct timeval msec_add _((struct timeval t, int x));
struct timeval update_quotas _((struct timeval last, struct timeval current));
void shovechars _((Port_t port));
DESC *new_connection _((int oldsock));
void clearstrings _((DESC *d));
void fcache_dump _((DESC *d, FBLOCK *fp[]));
int fcache_read _((FBLOCK **cp, char *filename));
void fcache_load _((dbref player));
void fcache_init _((void));
void logout_sock _((DESC *d));
void shutdownsock _((DESC *d));
DESC *initializesock _((int s, char *addr, char *ip));
int make_socket _((Port_t port));
struct text_block *make_text_block _((const char *s, int n));
void free_text_block _((struct text_block * t));
void add_to_queue _((struct text_queue * q, const char *b, int n));
int flush_queue _((struct text_queue * q, int n));
int queue_write _((DESC *d, const char *b, int n));
int queue_newwrite _((DESC *d, const char *b, int n));
int queue_string _((DESC *d, const char *s));
int process_output _((DESC *d));
void make_nonblocking _((int s));
void freeqs _((DESC *d));
void welcome_user _((DESC *d));
void do_new_spitfile _((dbref player, char *arg1, const char *index_file, const char *text_file, int restricted));
static void dump_info _((DESC *call_by));
void save_command _((DESC *d, const char *command));
int process_input _((DESC *d));
void process_input_helper _((DESC *d, char *tbuf1, int got));
void set_userstring _((char **userstring, const char *command));
void process_commands _((void));
int do_command _((DESC *d, char *command));
static int dump_messages _((DESC *d, dbref player, int new));
int check_connect _((DESC *d, const char *msg));
void parse_connect _((const char *msg, char *command, char *user, char *pass));
void close_sockets _((void));
void emergency_shutdown _((void));
dbref find_player_by_desc _((int port));
Signal_t bailout _((int sig));
void dump_users _((DESC *call_by, char *match, int doing));
const char *time_format_1 _((long int dt));
const char *time_format_2 _((long int dt));
void announce_connect _((dbref player));
void announce_disconnect _((dbref player));
void do_motd _((dbref player, int key, const char *message));
void do_doing _((dbref player, const char *message));
void do_poll _((dbref player, const char *message));
static const char *hostname_convert _((struct in_addr nums));
static const char *ip_convert _((struct in_addr nums));
dbref short_page _((const char *match));
void hide_player _((dbref player, int hide));
void inactivity_check _((void));
#ifdef CHAT_SYSTEM
void do_channel_who _((dbref player, CHAN *chan));
#endif
int hidden _((dbref player));
#ifdef USE_MAILER
struct mail *desc_mail _((dbref player));
void desc_mail_set _((dbref player, struct mail * mp));
#endif
void rwho_update _((void));
#ifdef INFO_SLAVE
static void make_info_slave _((void));
static void promote_info_slave _((void));
static void query_info_slave _((int fd));
static void reap_info_slave _((void));
void kill_info_slave _((void));
#endif
extern void local_shutdown _((void));
void reopen_logs _((void));
void dump_reboot_db _((void));
void load_reboot_db _((void));

#ifdef HAS_GETRLIMIT
static void
init_rlimit()
{
  /* Unlimit file descriptors. */
  /* Ultrix 4.4 and others may have getrlimit but may not be able to
   * change number of file descriptors 
   */
#ifdef RLIMIT_NOFILE
  struct rlimit *rlp;

  rlp = (struct rlimit *) malloc(sizeof(struct rlimit));
  if (getrlimit(RLIMIT_NOFILE, rlp)) {
    perror("init_rlimit: getrlimit()");
    free(rlp);
    return;
  }
  rlp->rlim_cur = rlp->rlim_max;
  if (setrlimit(RLIMIT_NOFILE, rlp))
    perror("init_rlimit: setrlimit()");
  free(rlp);
#endif
  return;
}
#endif				/* HAS_GETRLIMIT */

#ifdef NT_TCP
BOOL 
IsValidAddress(const void *lp, UINT nBytes, BOOL bReadWrite)
{
  return (lp != NULL &&
	  !IsBadReadPtr(lp, nBytes) &&
	  (!bReadWrite || !IsBadWritePtr((LPVOID) lp, nBytes)));

}

BOOL 
GetErrorMessage(const DWORD dwError, LPTSTR lpszError, const UINT nMaxError)
{

  LPTSTR lpBuffer;
  BOOL bRet = FormatMessage(
	     FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			     NULL,
			     dwError,
			     0,
			     (LPTSTR) & lpBuffer,
			     0,
			     NULL);

  if (bRet == FALSE)
    *lpszError = '\0';
  else {
    lstrcpyn(lpszError, lpBuffer, nMaxError);
    LocalFree(lpBuffer);
  }
  return bRet;
}

#endif

#ifdef macintosh
void GUSISetupFactories( void );
#endif

#ifndef BOOLEXP_DEBUGGING
#ifdef WIN32
/* Under WIN32, MUSH is a "service", so we just start a thread here.
 * The real "main" is in win32/services.c
 */
void
mainthread(argc, argv)
    int argc;
    char **argv;
#else
int
main(argc, argv)
    int argc;
    char **argv;
#endif				/* WIN32 */
{
#ifdef AUTORESTART
  FILE *id;
#endif
  FILE *newerr;

#ifdef macintosh
	GUSISetupFactories();
#endif

#ifndef macintosh
  /* read the configuration file */
  if (argc < 3) {
    fprintf(stderr, "ERROR: Usage: %s config_file error_log_file\n", argv[0]);
    exit(2);
  }
#endif
  
#ifdef WIN32
  {
    unsigned short wVersionRequested = MAKEWORD(1, 1);
    WSADATA wsadata;
    int err;

    /* Need to include library: wsock32.lib for Windows Sockets */
    err = WSAStartup(wVersionRequested, &wsadata);
    if (err) {
      printf("Error %i on WSAStartup\n", err);
      exit(1);
    }
  }
#endif				/* WIN32 */

#ifdef NT_TCP

/* Find which version of Windows we are using - Completion ports do */
/* not work with Windows 95/98 */

  {
    OSVERSIONINFO VersionInformation;

    VersionInformation.dwOSVersionInfoSize = sizeof(VersionInformation);
    GetVersionEx(&VersionInformation);
    platform = VersionInformation.dwPlatformId;
    printf("Running under Windows %s\n",
	   platform == VER_PLATFORM_WIN32_NT ? "NT" : "95/98");
  }
#endif

#ifdef HAS_GETRLIMIT
  init_rlimit();		/* unlimit file descriptors */
#endif

  /* These are FreeBSDisms to fix floating point exceptions */
#ifdef HAS_FPSETROUND
  fpsetround(FP_RN);
#endif
#ifdef HAS_FPSETMASK
  fpsetmask(0L);
#endif

  /* open the log files */
#ifndef macintosh
  strncpy(errlog, argv[2], BUFFER_LEN - 1);
  errlog[BUFFER_LEN - 1] = '\0';
  newerr = fopen(errlog, "a");
  if (!newerr) {
    fprintf(stderr, "Unable to open %s. Error output to stderr.\n", errlog);
  } else {
    fprintf(stderr, "Redirecting output to: %s\n", errlog);
    if (!freopen(errlog, "a", stderr)) {
      printf("Ack!  Failed reopening stderr!");
      exit(1);
    }
    setvbuf(stderr, NULL, _IOLBF, BUFSIZ);
    fclose(newerr);
  }
#endif
#ifndef SINGLE_LOGFILE
  start_log(&connlog_fp, CONNLOG);
  start_log(&checklog_fp, CHECKLOG);
  start_log(&wizlog_fp, WIZLOG);
  start_log(&tracelog_fp, TRACELOG);
  start_log(&cmdlog_fp, CMDLOG);
#ifdef TREKMUSH
  start_log(&spacelog_fp, SPACELOG);
#endif /* TREKMUSH */
#else
#ifdef TREKMUSH
  spacelog_fp = 
#endif /* TREKMUSH */
  connlog_fp = checklog_fp = wizlog_fp = tracelog_fp = cmdlog_fp = stderr;
#endif				/* SINGLE_LOGFILE */

/* this writes a file used by the restart script to check for active mush */
#ifdef AUTORESTART
  id = fopen("runid", "w");
  fprintf(id, "%d", getpid());
  fclose(id);
#endif

#ifdef macintosh
  strncpy(confname, "mush.cnf", BUFFER_LEN - 1);
#else
  strncpy(confname, argv[1], BUFFER_LEN - 1);
#endif
  confname[BUFFER_LEN - 1] = '\0';
  init_game_config(confname);

#ifdef INFO_SLAVE
  make_info_slave();
#endif

  /* save a file descriptor */
#ifndef macintosh
#ifndef WIN32
  reserved = open("/dev/null", O_RDWR);
  extrafd = open("/dev/null", O_RDWR);
#endif
#endif

  /* decide if we're in @shutdown/reboot */
  restarting = 0;
  newerr = fopen(REBOOTFILE, "r");
  if (newerr) {
    restarting = 1;
    fclose(newerr);
  }
  init_process_expression();

  if (init_game_dbs(confname) < 0) {
    fprintf(stderr, "ERROR: Couldn't load databases! Exiting.\n");
    exit(2);
  }
/* OFFLINE #ifdef TREKMUSH
  offline_init();
#endif */
  set_signals();
  if (USE_RWHO)
    rwhocli_setup(RWHOSERV, RWHOPASS, MUDNAME, SHORTVN);

  /* go do it */
#ifdef CSRI
#ifdef CSRI_DEBUG
  mal_verify(1);
#endif
#ifdef CSRI_TRACE
  mal_leaktrace(1);
#endif
#endif
  load_reboot_db();
#ifdef NT_TCP

  /* If we are running Windows NT we must create a completion port, */
  /* and start up a listening thread for new connections */

  if (platform == VER_PLATFORM_WIN32_NT) {
    int nRet;

    /* create initial IO completion port, so threads have something to wait on */

    CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
					    NULL,
					    0,
					    1);

    if (!CompletionPort) {
      char sMessage[200];
      GetErrorMessage(GetLastError(), sMessage, sizeof sMessage);
      printf("Error %ld (%s) on CreateIoCompletionPort\n",
	     GetLastError(),
	     sMessage);
      WSACleanup();		/* clean up */
      exit(1);
    }

    InitializeCriticalSection(&cs);

    /* Create a TCP/IP stream socket */
    MUDListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    /* Fill in the the address structure */
    saServer.sin_port = htons((u_short) TINYPORT);
    saServer.sin_family = AF_INET;
    saServer.sin_addr.s_addr = INADDR_ANY;

    /* bind our name to the socket */
    nRet = bind(MUDListenSocket, (LPSOCKADDR) & saServer, sizeof saServer);

    if (nRet) {
      printf("Error %ld on Win32: bind\n", WSAGetLastError());
      WSACleanup();		/* clean up */
      exit(1);
    }
    /* Set the socket to listen */
    nRet = listen(MUDListenSocket, SOMAXCONN);

    if (nRet) {
      printf("Error %ld on Win32: listen\n", WSAGetLastError());
      WSACleanup();		/* clean up */
      exit(1);
    }
    /* Create the MUD listening thread */
    dwMUDListenThread = _beginthread(MUDListenThread, 0,
				     (void *) (SOCKET) MUDListenSocket);

    if (dwMUDListenThread == -1) {
      printf("Error %ld on _beginthread\n", errno);
      WSACleanup();		/* clean up */
      exit(1);
    }
    fprintf(stderr, "Listening (NT-style) on port %d\n", TINYPORT);
  }
#endif				/* NT_TCP */

  shovechars((Port_t) TINYPORT);
#ifdef CSRI
#ifdef CSRI_DEBUG
  mal_verify(1);
#endif
#endif

  /* someone has told us to shut down */

#ifdef ALLOW_RPAGE
  rpage_shutdown();		/* do this first */
#endif				/* ALLOW_RPAGE */

#ifdef WIN32
  /* Keep service manager happy */
  shutdown_checkpoint();
#endif

  close_sockets();

#ifdef WIN32
  /* Keep service manager happy */
  shutdown_checkpoint();
#endif

  dump_database();

  local_shutdown();

/* OFFLINE #ifdef TREKMUSH
offline_shutdown();
#endif */

#ifndef SINGLE_LOGFILE
  /* close up the log files */
  end_log(connlog_fp);
  end_log(checklog_fp);
  end_log(wizlog_fp);
  end_log(tracelog_fp);
  end_log(cmdlog_fp);
#ifdef TREKMUSH
  end_log(spacelog_fp);
#endif TREKMUSH
#endif				/* SINGLE_LOGFILE */

#ifdef CSRI
#ifdef CSRI_PROFILESIZES
  mal_statsdump(stderr);
#endif
#ifdef CSRI_TRACE
  mal_dumpleaktrace(stderr);
#endif
  fflush(stderr);
#endif

#ifdef WIN32
  /* Keep service manager happy */
  shutdown_checkpoint();
#endif

#ifdef HAS_GETRUSAGE
  rusage_stats();
#endif				/* HAS_RUSAGE */

  fprintf(stderr, "\nMUSH shutdown completed.\n");
  fflush(stderr);

#ifdef NT_TCP

  /* critical section not needed any more */
  if (platform == VER_PLATFORM_WIN32_NT)
    DeleteCriticalSection(&cs);
#endif

  closesocket(sock);
#ifdef WIN32
  shutdown_checkpoint();
  WSACleanup();			/* clean up */
  kill_timer();			/* stop timer thread */
#else
  exit(0);
#endif
}
#endif				/* BOOLEXP_DEBUGGING */

/* Close and reopen the logfiles - called on SIGHUP */
void
reopen_logs()
{
  FILE *newerr;
#ifndef SINGLE_LOGFILE
  /* close up the log files */
  end_log(connlog_fp);
  end_log(checklog_fp);
  end_log(wizlog_fp);
  end_log(tracelog_fp);
  end_log(cmdlog_fp);
#ifdef TREKMUSH
  end_log(spacelog_fp);
#endif /* TREKMUSH */
#endif				/* SINGLE_LOGFILE */
  newerr = fopen(errlog, "a");
  if (!newerr) {
    fprintf(stderr, "Unable to open %s. Error output continues to stderr.\n", errlog);
  } else {
    if (!freopen(errlog, "a", stderr)) {
      printf("Ack!  Failed reopening stderr!");
      exit(1);
    }
    setvbuf(stderr, NULL, _IOLBF, BUFSIZ);
    fclose(newerr);
  }
#ifndef SINGLE_LOGFILE
  start_log(&connlog_fp, CONNLOG);
  start_log(&checklog_fp, CHECKLOG);
  start_log(&wizlog_fp, WIZLOG);
  start_log(&tracelog_fp, TRACELOG);
  start_log(&cmdlog_fp, CMDLOG);
#ifdef TREKMUSH
  start_log(&spacelog_fp, SPACELOG);
#endif /* TREKMUSH */
#else
#ifdef TREKMUSH
  spacelog_fp = 
#endif /* TREKMUSH */
  connlog_fp = checklog_fp = wizlog_fp = tracelog_fp = cmdlog_fp = stderr;
#endif				/* SINGLE_LOGFILE */
}

void
set_signals()
{
/*#ifdef macintosh
  int	i;
  
  for ( i = 0; i < NSIG; i++ )
     signal( i, SIG_IGN );

  return;
#endif*/

#ifndef WIN32
  /* we don't care about SIGPIPE, we notice it in select() and write() */
  signal(SIGPIPE, SIG_IGN);
#endif

  /* standard termination signals */
  signal(SIGINT, (void *) bailout);
  signal(SIGTERM, (void *) bailout);
}

#define NA_RAW -1
#define NA_ASCII 0
#define NA_ANSI 1
#define NA_COLOR 2
#define NA_PUEBLO 3
#define NA_PASCII 4

#define TA_BGC 0
#define TA_FGC 1
#define TA_BOLD 2
#define TA_REV 3
#define TA_BLINK 4
#define TA_ULINE 5

static int na_depth = 0;

struct notify_strings {
  char *message;
  int made;
};

static void
fillstate(state, f)
    int state[6];
    char **f;
{
  char *p;
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
ansi_change_state(t, o, color, state, newstate)
    char *t;
    char **o;
    int color;
    int *state;
    int *newstate;
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

int html_cols[] =
{
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

static char *
notify_makestring(message, messages, type)
    char *message;
    struct notify_strings messages[];	/* Must be allocated by caller! */
    int type;
{
  char *o;
  char *p;
  char *t;
  int state[6];
  int newstate[6];
  int n;
  int b, f;
  int changed = 0;
  int color = 0;
  int fc, bc;
  char colbuff[128];
  if (messages[type].made)
    return messages[type].message;
  messages[type].made = 1;

  p = message;
  o = messages[type].message;

  /* Since well over 50% is this type, we do it quick */
  if (type == NA_ASCII) {
    while (*p) {
      switch (*p) {
      case TAG_START:
	while (*p && *p != TAG_END)
	  p++;
	break;
      case '\r':
	break;
      case ESC_CHAR:
	while (*p && *p != 'm')
	  p++;
	break;
      default:
	*o++ = *p;
      }
      p++;
    }
    *o = '\0';
    return messages[type].message;
  } else if (type == NA_PASCII) {
    /* PLAYER Ascii. Different output. \n is \r\n */
    while (*p) {
      switch (*p) {
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
	*o++ = '\r';
	*o++ = '\n';
	break;
      default:
	*o++ = *p;
      }
      p++;
    }
    *o = '\0';
    return messages[type].message;
  }
  for (n = 0; n < 6; n++) {
    state[n] = 0;
    newstate[n] = 0;
  }

  t = o;
  switch (type) {
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
	  if (!state[TA_BOLD] && newstate[TA_BOLD])
	    safe_str("<B>", t, &o);
	  if (!state[TA_ULINE] && newstate[TA_ULINE])
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
	case '<':
	  safe_str("&lt;", t, &o);
	  break;
	case '>':
	  safe_str("&gt;", t, &o);
	  break;
	case '"':
	  safe_str("&quot;", t, &o);
	  break;
	case '&':
	  safe_str("&amp;", t, &o);
	  break;
	case '\r':
	  break;
	case '\n':
	  safe_str("<BR>\n", t, &o);
	  break;
	default:
	  safe_chr(*p, t, &o);
	  break;
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
    color = 1;
    /* FALLTHROUGH */
  case NA_ANSI:
    while (*p) {
      switch (*p) {
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
  return messages[type].message;
}

dbref
na_one(current, data)
    dbref current;
    void *data;
{
  if (current == NOTHING)
    return (dbref) data;
  else
    return NOTHING;
}

dbref
na_next(current, data)
    dbref current;
    void *data;
{
  if (current == NOTHING)
    return (dbref) data;
  else
    return Next(current);
}

dbref
na_loc(current, data)
    dbref current;
    void *data;
{
  if (current == NOTHING)
    return (dbref) data;
  else if (current == (dbref) data)
    return Contents(current);
  else
    return Next(current);
}

dbref
na_nextbut(current, data)
    dbref current;
    void *data;
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
na_except(current, data)
    dbref current;
    void *data;
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
na_except2(current, data)
    dbref current;
    void *data;
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


void
notify_anything(speaker, func, fdata, nsfunc, flags, message)
    dbref speaker;
    dbref (*func) ();
    void *fdata;
    void (*nsfunc) ();
    int flags;
    const char *message;
{
  dbref target;
  dbref passalong[3];
  struct notify_strings messages[5];
  struct notify_strings nospoofs[5];
  int i, j;
  DESC *d;
  int poutput;
  char *pstring;
  char *bp;
  ATTR *a;
  char const *asave, *ap;
  char *preserve[10];
  int havespoof = 0;
  char *wsave[10];
  char *tbuf1, *nospoof, *msgbuf;

  if (!message || *message == '\0' || !func)
    return;

  /* Depth check */
  if (na_depth > 7)
    return;
  na_depth++;

  /* Allocate memory */
  tbuf1 = (char *) mush_malloc(BUFFER_LEN, "string");
  nospoof = (char *) mush_malloc(BUFFER_LEN, "string");
  msgbuf = (char *) mush_malloc(BUFFER_LEN, "string");

  strcpy(msgbuf, message);

  target = NOTHING;

  for (i = 0; i < 5; i++) {
    nospoofs[i].message = (char *) mush_malloc(BUFFER_LEN, "string");
    messages[i].message = (char *) mush_malloc(BUFFER_LEN, "string");
    nospoofs[i].made = 0;
    messages[i].made = 0;
  }

  while ((target = func(target, fdata)) != NOTHING) {
    if ((flags & NA_PONLY) && !IsPlayer(target))
      continue;

    if (IsPlayer(target)) {
      if (!Connected(target) && options.login_allow && under_limit)
	continue;
      for (d = descriptor_list; d; d = d->next) {
	if (d->connected && d->player == target) {
	  if (d->pueblo) {
	    poutput = NA_PUEBLO;
	  } else if (ShowAnsi(target)) {
	    if (ShowAnsiColor(target))
	      poutput = NA_COLOR;
	    else
	      poutput = NA_ANSI;
	  } else {
	    poutput = NA_PASCII;
	  }

	  if ((flags & NA_PONLY) && (poutput != NA_PUEBLO))
	    continue;

	  if (nsfunc && Nospoof(target) && (target != speaker)) {
	    if (!havespoof) {
	      nsfunc(nospoof, speaker, func, fdata);
	      havespoof = 1;
	    }
	    pstring = notify_makestring(nospoof, nospoofs, poutput);
	    queue_newwrite(d, pstring, strlen(pstring));
	  }
	  pstring = notify_makestring(msgbuf, messages, poutput);

	  if (pstring && *pstring)
	    queue_newwrite(d, pstring, strlen(pstring));
	  if (!(flags & NA_NOENTER)) {
	    if (poutput == NA_PUEBLO) {
	      if (flags & NA_NOPENTER)
		queue_newwrite(d, "\n", 1);
	      else
		queue_newwrite(d, "<BR>\n", 5);
	    } else {
	      queue_newwrite(d, "\r\n", 2);
	    }
	  }
	}
      }
    } else if (Puppet(target) &&
	       (Location(target) != Location(Owner(target))) &&
	       ((flags & NA_PUPPET) || !(flags & NA_NORELAY))) {
      bp = tbuf1;
      safe_str(Name(target), tbuf1, &bp);
      safe_chr('>', tbuf1, &bp);
      safe_chr(' ', tbuf1, &bp);
      safe_str(msgbuf, tbuf1, &bp);
      *bp = 0;
      notify_anything(speaker, na_one, (void *) Owner(target), NULL, flags | NA_NORELAY, tbuf1);
    }
    if ((flags & NA_NOLISTEN)
	|| (!PLAYER_LISTEN && IsPlayer(target))
	|| IsExit(target))
      continue;

    /* do @listen stuff */
    a = atr_get_noparent(target, "LISTEN");
    if (a) {
      strcpy(tbuf1, uncompress(a->value));
      if (wild_match(tbuf1, notify_makestring(msgbuf, messages, NA_ASCII))) {
#ifdef LISTEN_LOCK
	if (eval_lock(speaker, target, Listen_Lock))
#endif
	  if (PLAYER_AHEAR || (!IsPlayer(target))) {
	    if (speaker != target)
	      charge_action(speaker, target, "AHEAR");
	    else
	      charge_action(speaker, target, "AMHEAR");
	    charge_action(speaker, target, "AAHEAR");
	  }
	if (!(flags & NA_NORELAY) && !member(speaker, Contents(target)) &&
	    !filter_found(target, notify_makestring(msgbuf, messages, NA_ASCII), 1)) {
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
	    free((Malloc_t) asave);
	    restore_global_regs("inprefix_save", preserve);
	    for (j = 0; j < 10; j++)
	      wenv[j] = wsave[j];
	  }
	  notify_anything(speaker, Puppet(target) ? na_except2 : na_except,
	      passalong, NULL, flags | NA_NORELAY, (a) ? tbuf1 : msgbuf);
	}
      }
    }
    /* if object is flagged LISTENER, check for ^ listen patterns
     *    * these are like AHEAR - object cannot trigger itself.
     *    * unlike normal @listen, don't pass the message on.
     *    */

    if ((speaker != target) && (ThingListen(target) || RoomListen(target))
#ifdef LISTEN_LOCK
	&& eval_lock(speaker, target, Listen_Lock)
#endif
      )
      atr_comm_match(target, speaker, '^', ':', notify_makestring(msgbuf, messages, NA_ASCII), 0);
  }
  for (i = 0; i < 5; i++) {
    mush_free((Malloc_t) nospoofs[i].message, "string");
    mush_free((Malloc_t) messages[i].message, "string");
  }
  mush_free((Malloc_t) tbuf1, "string");
  mush_free((Malloc_t) nospoof, "string");
  mush_free((Malloc_t) msgbuf, "string");
  na_depth--;
}

int
safe_tag(a_tag, buf, bp)
    char const *a_tag;
    char *buf;
    char **bp;
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
safe_tag_cancel(a_tag, buf, bp)
    char const *a_tag;
    char *buf;
    char **bp;
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
safe_tag_wrap(a_tag, params, data, buf, bp)
    char const *a_tag;
    char const *params;
    char const *data;
    char *buf;
    char **bp;
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
raw_notify(player, msg)
    dbref player;
    const char *msg;
{
  notify_anything(GOD, na_one, (void *) player, NULL, NA_NOLISTEN, msg);
}

#ifdef I_STDARG
void
flag_broadcast(object_flag_type inflags, object_flag_type intoggles, const char *fmt,...)
#else
void
flag_broadcast(inflags, intoggles, va_alist)
    object_flag_type inflags;
    object_flag_type intoggles;
    va_dcl
#endif				/* I_STDARG */
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
#ifndef I_STDARG
  char *fmt;

  va_start(args);
  fmt = va_arg(args, char *);
#else
  va_start(args, fmt);
#endif				/* I_STDARG */

  (void) vsprintf(tbuf1, fmt, args);
  va_end(args);

  DESC_ITER_CONN(d) {
    if (inflags) {
      if (intoggles) {
	if (!((Flags(d->player) & inflags) && (Toggles(d->player) & intoggles)))
	  continue;
      } else {
	if (!(Flags(d->player) & inflags))
	  continue;
      }
    } else {
      if (intoggles) {
	if (!(Toggles(d->player) & intoggles))
	  continue;
      }
    }
    queue_string(d, tbuf1);
    queue_write(d, "\r\n", 2);
    process_output(d);
  }
}


struct timeval
timeval_sub(now, then)
    struct timeval now;
    struct timeval then;
{
  now.tv_sec -= then.tv_sec;
  now.tv_usec -= then.tv_usec;
  if (now.tv_usec < 0) {
    now.tv_usec += 1000000;
    now.tv_sec--;
  }
  return now;
}

long
msec_diff(now, then)
    struct timeval now;
    struct timeval then;
{
  return ((now.tv_sec - then.tv_sec) * 1000
	  + (now.tv_usec - then.tv_usec) / 1000);
}

struct timeval
msec_add(t, x)
    struct timeval t;
    int x;
{
  t.tv_sec += x / 1000;
  t.tv_usec += (x % 1000) * 1000;
  if (t.tv_usec >= 1000000) {
    t.tv_sec += t.tv_usec / 1000000;
    t.tv_usec = t.tv_usec % 1000000;
  }
  return t;
}

struct timeval
update_quotas(last, current)
    struct timeval last;
    struct timeval current;
{
  int nslices;
  DESC *d;
  nslices = (int) msec_diff(current, last) / COMMAND_TIME_MSEC;

  if (nslices > 0) {
    for (d = descriptor_list; d; d = d->next) {
      d->quota += COMMANDS_PER_TIME * nslices;
      if (d->quota > COMMAND_BURST_SIZE)
	d->quota = COMMAND_BURST_SIZE;
    }
  }
  return msec_add(last, nslices * COMMAND_TIME_MSEC);
}

#ifdef CAN_NEWSTYLE
void
shovechars(Port_t port)
#else
void
shovechars(port)
    Port_t port;
#endif
{
  /* this is the main game loop */

  fd_set input_set, output_set;
  time_t now;
  struct timeval last_slice, current_time;
  struct timeval next_slice;
  struct timeval timeout, slice_timeout;
  int found;
  int queue_timeout;
  DESC *d, *dnext;
#ifndef INFO_SLAVE
  DESC *newd;
#endif
  int avail_descriptors;
#ifdef WIN32
  MMTIME win_time;
#endif
#ifdef INFO_SLAVE
  struct sockaddr_in addr;
  int addr_len;
  int newsock;
#endif

#ifdef NT_TCP
  if (platform != VER_PLATFORM_WIN32_NT)
#endif
    if (!restarting) {
      sock = make_socket(port);
      maxd = sock + 1;
    }
#ifdef WIN32
  timeGetSystemTime(&win_time, sizeof(win_time));
  last_slice.tv_sec = win_time.u.ms / 1000;
  last_slice.tv_usec = (win_time.u.ms % 1000) *1000;
#else
  gettimeofday(&last_slice, (struct timezone *) 0);
#endif

  avail_descriptors = how_many_fds() - 4;
#ifdef INFO_SLAVE
  avail_descriptors -= 2;	/* reserve some more for setting up the slave */
  FD_ZERO(&info_pending);
#endif

  /* done. print message to the log */
  fprintf(stderr, "%d file descriptors available.\n", avail_descriptors);
  fprintf(stderr, "RESTART FINISHED.\n");
  fflush(stderr);
  while (shutdown_flag == 0) {
#ifdef WIN32
    timeGetSystemTime(&win_time, sizeof(win_time));
    current_time.tv_sec = win_time.u.ms / 1000;
    current_time.tv_usec = (win_time.u.ms % 1000) *1000;
#else
    gettimeofday(&current_time, (struct timezone *) 0);
#endif
    last_slice = update_quotas(last_slice, current_time);

    process_commands();

    if (shutdown_flag)
      break;

    /* test for events */
    dispatch();

#ifdef ALLOW_RPAGE
    /* anything received on our datagram remote page? */
    recv_rpage();
#endif				/* ALLOW_RPAGE */

    /* any queued robot commands waiting? */
    /* timeout.tv_sec used to be set to que_next(), the number of
     * seconds before something on the queue needed to run, but this
     * caused a problem with stuff that had to be triggered by alarm
     * signal every second, so we're reduced to what's below:
     */
    queue_timeout = que_next();
    timeout.tv_sec = queue_timeout ? 1 : 0;
    timeout.tv_usec = 0;
    next_slice = msec_add(last_slice, COMMAND_TIME_MSEC);
    slice_timeout = timeval_sub(next_slice, current_time);

#ifdef NT_TCP

    /* for Windows NT, we handle IOs in a separate function for simplicity */
    if (platform == VER_PLATFORM_WIN32_NT) {
      ProcessWindowsTCP();
      continue;
    }				/* end of NT_TCP and Windows NT */
#endif

    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    if (ndescriptors < avail_descriptors)
      FD_SET(sock, &input_set);
#ifdef INFO_SLAVE
    if (info_slave_state > 0)
      FD_SET(info_slave, &input_set);
#endif
    for (d = descriptor_list; d; d = d->next) {
      if (d->input.head)
	timeout = slice_timeout;
      else
	FD_SET(d->descriptor, &input_set);
      if (d->output.head)
	FD_SET(d->descriptor, &output_set);
    }

    if ((found = select(maxd, &input_set, &output_set,
			(fd_set *) 0, &timeout)) < 0) {
#ifdef WIN32
      if (found == SOCKET_ERROR && WSAGetLastError() != WSAEINTR)
#else
      if (errno != EINTR)
#endif
      {
	perror("select");
	return;
      }
#ifdef INFO_SLAVE
      now = time((time_t *) 0);
      if (info_slave_state == 2 && now > info_queue_time + 30) {
	/* rerun any pending queries that got lost */
	info_queue_time = now;
	for (newsock = 0; newsock < maxd; newsock++)
	  if (FD_ISSET(newsock, &info_pending))
	    query_info_slave(newsock);
      }
#endif
    } else {
      /* if !found then time for robot commands */

      if (!found) {
	do_top(options.queue_chunk);
	continue;
      } else {
	do_top(options.active_q_chunk);
      }
      now = time((time_t *) 0);
#ifdef INFO_SLAVE
      if (info_slave_state > 0 && FD_ISSET(info_slave, &input_set)) {
	if (info_slave_state == 1)
	  promote_info_slave();
	else
	  reap_info_slave();
      } else {
	if (info_slave_state == 2 && now > info_queue_time + 30) {
	  /* rerun any pending queries that got lost */
	  info_queue_time = now;
	  for (newsock = 0; newsock < maxd; newsock++)
	    if (FD_ISSET(newsock, &info_pending))
	      query_info_slave(newsock);
	}
      }
      if (FD_ISSET(sock, &input_set)) {
	addr_len = sizeof(addr);
	newsock = accept(sock, (struct sockaddr *) &addr, &addr_len);
	if (newsock < 0) {
#ifdef WIN32
	  if (newsock == INVALID_SOCKET && WSAGetLastError() != WSAEINTR &&
	      WSAGetLastError() != WSAEMFILE)
#else
	  if (errno && errno != EINTR && errno != EMFILE && errno != ENFILE)
#endif
	  {
	    perror("new_connection");
	    continue;		/* this should _not_ be return. */
	  }
	}
	ndescriptors++;
	query_info_slave(newsock);
	if (newsock >= maxd)
	  maxd = newsock + 1;
      }
#else
      if (FD_ISSET(sock, &input_set)) {
	if (!(newd = new_connection(sock))) {
#ifdef WIN32
	  if (newd == INVALID_SOCKET && WSAGetLastError() != WSAEINTR &&
	      WSAGetLastError() != WSAEMFILE)
#else
	  if (errno && errno != EINTR && errno != EMFILE && errno != ENFILE)
#endif
	  {
	    perror("new_connection");
	    continue;		/* this should _not_ be return. */
	  }
	} else {
	  ndescriptors++;
	  if (newd->descriptor >= maxd)
	    maxd = newd->descriptor + 1;
	}
      }
#endif
      for (d = descriptor_list; d; d = dnext) {
	dnext = d->next;
	if (FD_ISSET(d->descriptor, &input_set)) {
	  d->last_time = now;
	  if (!process_input(d)) {
	    shutdownsock(d);
	    continue;
	  }
	}
	if (FD_ISSET(d->descriptor, &output_set)) {
	  if (!process_output(d)) {
	    shutdownsock(d);
	  }
	}
      }
    }
  }
}

DESC *
new_connection(oldsock)
    int oldsock;
{
  int newsock;
  struct sockaddr_in addr;
#ifdef macintosh
  unsigned int addr_len;
#else
  int addr_len;
#endif
  char tbuf1[BUFFER_LEN];
  char tbuf2[BUFFER_LEN];
  char *bp;
  char *socket_ident;
  char *chp;

  addr_len = sizeof(addr);
  newsock = accept(oldsock, (struct sockaddr *) &addr, &addr_len);
  if (newsock < 0) {
    return 0;
  }
  bp = tbuf2;
  safe_str((char *) ip_convert(addr.sin_addr), tbuf2, &bp);
  *bp = '\0';
  bp = tbuf1;
  if (USE_IDENT) {
    socket_ident = (char *) ident_id(newsock, options.ident_timeout);
    if (socket_ident) {
      /* Truncate at first non-printable character */
      for (chp = socket_ident; *chp && isprint(*chp); chp++) ;
      *chp = '\0';
      safe_str(socket_ident, tbuf1, &bp);
      safe_chr('@', tbuf1, &bp);
      free(socket_ident);
    }
  }
  safe_str((char *) hostname_convert(addr.sin_addr), tbuf1, &bp);
  *bp = '\0';
  if (Forbidden_Site(tbuf1) || Forbidden_Site(tbuf2)) {
    do_log(LT_CONN, 0, 0, "[%d/%s/%s] Refused connection (remote port %d)",
	   newsock, tbuf1, tbuf2, ntohs(addr.sin_port));
    shutdown(newsock, 2);
    closesocket(newsock);
#ifndef WIN32
    errno = 0;
#endif
    return 0;
  }
  do_log(LT_CONN, 0, 0, "[%d/%s/%s] Connection opened.", newsock, tbuf1, tbuf2);
  return initializesock(newsock, tbuf1, tbuf2);
}

void
clearstrings(d)
    DESC *d;
{
  if (d->output_prefix) {
    mush_free((Malloc_t) d->output_prefix, "userstring");
    d->output_prefix = 0;
  }
  if (d->output_suffix) {
    mush_free((Malloc_t) d->output_suffix, "userstring");
    d->output_suffix = 0;
  }
}

void
fcache_dump(d, fp)
    DESC *d;
    FBLOCK *fp[];
{
  FBLOCK *fb;
  if (d->pueblo && fp[1]) {
    fb = fp[1];
    while (fb) {
      queue_newwrite(d, fb->data, fb->hdr.nchars);
      fb = fb->hdr.nxt;
    }
  } else {
    fb = fp[0];
    while (fb) {
      queue_write(d, fb->data, fb->hdr.nchars);
      fb = fb->hdr.nxt;
    }
  }
}


int
fcache_read(cp, filename)
    FBLOCK **cp;
    char *filename;
{
  int n, nmax, fd, tchars;
  char *bufp;
  FBLOCK *fp, *tfp;

  /* Free prior buffer chain */
  fp = *cp;
  while (fp != NULL) {
    tfp = fp->hdr.nxt;
    free(fp);
    fp = tfp;
  }
  *cp = NULL;

  /* Read the text file into a new chain */
  close(reserved);
  if ((fd = open(filename, O_RDONLY, 0)) == -1) {
    do_log(LT_ERR, 0, 0, "couldn't open cached text file '%s'", filename);
    reserved = open("/dev/null", O_RDWR);
    return -1;
  }
  fp = (FBLOCK *) malloc(sizeof(char) * 256);
  fp->hdr.nxt = NULL;
  fp->hdr.nchars = 0;
  *cp = fp;
  tfp = NULL;
  tchars = 0;

  /* Read in the first chunk of the file */
  nmax = FBLOCK_SIZE;
  bufp = fp->data;
  n = read(fd, bufp, nmax);
  while (n > 0) {
    /* if we didn't read in all we wanted, update the pointers and try to
     * fill the current buffer.
     */
    fp->hdr.nchars += n;
    tchars += n;
    if ((Size_t) fp->hdr.nchars < FBLOCK_SIZE) {
      nmax -= n;
      bufp += n;
    } else {
      /* filled the current buffer. Get a new one. */
      tfp = fp;
      fp = (FBLOCK *) malloc(sizeof(char) * 256);
      fp->hdr.nxt = NULL;
      fp->hdr.nchars = 0;
      tfp->hdr.nxt = fp;
      nmax = FBLOCK_SIZE;
      bufp = fp->data;
    }
    /* read in the next chunk of the file */
    n = read(fd, bufp, nmax);
  }
  close(fd);
  reserved = open("/dev/null", O_RDWR);
  if (fp->hdr.nchars == 0) {
    free(fp);
    if (tfp == NULL)
      *cp = NULL;
    else
      tfp->hdr.nxt = NULL;
  }
  return tchars;
}

void
fcache_load(player)
    dbref player;
{
  int conn, motd, wiz, new, reg, quit, down, full;
  int guest;
  int i;

  for (i = 0; i < (SUPPORT_PUEBLO ? 2 : 1); i++) {
    conn = fcache_read(&options.connect_fcache[i], options.connect_file[i]);
    motd = fcache_read(&options.motd_fcache[i], options.motd_file[i]);
    wiz = fcache_read(&options.wizmotd_fcache[i], options.wizmotd_file[i]);
    new = fcache_read(&options.newuser_fcache[i], options.newuser_file[i]);
    reg = fcache_read(&options.register_fcache[i], options.register_file[i]);
    quit = fcache_read(&options.quit_fcache[i], options.quit_file[i]);
    down = fcache_read(&options.down_fcache[i], options.down_file[i]);
    full = fcache_read(&options.full_fcache[i], options.full_file[i]);
    guest = fcache_read(&options.guest_fcache[i], options.guest_file[i]);

    if (player != NOTHING) {
      notify(player,
	     tprintf("%s sizes:  NewUser...%d  Connect...%d  Guest...%d  Motd...%d  Wizmotd...%d  Quit...%d  Register...%d  Down...%d  Full...%d",
		     i ? "HTMLFile" : "File",
		     new, conn, guest, motd, wiz, quit, reg, down, full));
    }
  }

}

void
fcache_init()
{
  int i;
  for (i = 0; i < 2; i++) {
    options.connect_fcache[i] = NULL;
    options.motd_fcache[i] = NULL;
    options.wizmotd_fcache[i] = NULL;
    options.newuser_fcache[i] = NULL;
    options.register_fcache[i] = NULL;
    options.quit_fcache[i] = NULL;
    options.down_fcache[i] = NULL;
    options.full_fcache[i] = NULL;
    options.guest_fcache[i] = NULL;
  }
  fcache_load(NOTHING);
}

void
logout_sock(d)
    DESC *d;
{
  if (d->connected) {
    fcache_dump(d, options.quit_fcache);
    do_log(LT_CONN, 0, 0, "[%d/%s/%s] Logout by %s(#%d) <Connection not dropped>",
	   d->descriptor, d->addr, d->ip, Name(d->player), d->player);
    fflush(connlog_fp);
    announce_disconnect(d->player);
#ifdef TREKMUSH
     (void) atr_add(d->player, "CMDS", unparse_integer(d->cmds),
	   GOD, AF_LOCKED | AF_WIZARD | AF_ODARK | AF_NOPROG);
#endif /* TREKMUSH */
#ifdef USE_MAILER
    do_mail_purge(d->player);
#endif
    if (MAX_LOGINS) {
      login_number--;
      if (!under_limit && (login_number < MAX_LOGINS)) {
	under_limit = 1;
	do_log(LT_CONN, 0, 0,
	       "Below maximum player limit of %d. Logins enabled.",
	       MAX_LOGINS);
      }
    }
  } else {
    do_log(LT_CONN, 0, 0,
	   "[%d/%s/%s] Logout, never connected. <Connection not dropped>",
	   d->descriptor, d->addr, d->ip);
  }
  process_output(d);		/* flush our old output */
  /* pretend we have a new connection */
  d->connected = 0;
  d->output_prefix = 0;
  d->output_suffix = 0;
  d->output_size = 0;
  d->output.head = 0;
  d->player = 0;
  d->output.tail = &d->output.head;
  d->input.head = 0;
  d->input.tail = &d->input.head;
  d->raw_input = 0;
  d->raw_input_at = 0;
  d->quota = COMMAND_BURST_SIZE;
  d->last_time = 0;
  d->cmds = 0;
  d->hide = 0;
  d->doing[0] = '\0';
#ifdef USE_MAILER
  d->mailp = NULL;
#endif
  welcome_user(d);
}

void
shutdownsock(d)
    DESC *d;
{
#ifdef NT_TCP

  /* don't close down the socket twice */
  if (d->bConnectionShutdown)
    return;
  /* make sure we don't try to initiate or process any outstanding IOs */
  d->bConnectionShutdown = TRUE;
  d->bConnectionDropped = TRUE;
#endif

  if (d->connected) {
    do_log(LT_CONN, 0, 0, "[%d/%s/%s] Logout by %s(#%d)",
	   d->descriptor, d->addr, d->ip, Name(d->player), d->player);
    fflush(connlog_fp);
    if (d->connected != 2) {
      fcache_dump(d, options.quit_fcache);
      /* Player was not allowed to log in from the connect screen */
      announce_disconnect(d->player);
#ifdef TREKMUSH
      (void) atr_add(d->player, "CMDS", unparse_integer(d->cmds),
        GOD, AF_LOCKED|AF_WIZARD|AF_ODARK|AF_NOPROG);
#endif /* TREKMUSH */
#ifdef USE_MAILER
      do_mail_purge(d->player);
#endif
    }
    if (MAX_LOGINS) {
      login_number--;
      if (!under_limit && (login_number < MAX_LOGINS)) {
	under_limit = 1;
	do_log(LT_CONN, 0, 0,
	       "Below maximum player limit of %d. Logins enabled.",
	       MAX_LOGINS);
      }
    }
  } else {
    do_log(LT_CONN, 0, 0, "[%d/%s/%s] Connection closed, never connected.",
	   d->descriptor, d->addr, d->ip);
  }
  process_output(d);
  clearstrings(d);
#ifdef NT_TCP

  if (platform == VER_PLATFORM_WIN32_NT) {
    /* cancel any pending reads or writes on this socket */

    if (!CancelIo((HANDLE) d->descriptor)) {
      char sMessage[200];
      GetErrorMessage(GetLastError(), sMessage, sizeof sMessage);
      printf("Error %ld (%s) on CancelIo\n",
	     GetLastError(),
	     sMessage);
    }
    /* post a notification that it is safe to free the descriptor */
    /* we can't free the descriptor here (below) as there may be some */
    /* queued completed IOs that will crash when they refer to a descriptor */
    /* (d) that has been freed. */

    if (!PostQueuedCompletionStatus(CompletionPort, 0, (DWORD) d, &lpo_aborted)) {
      char sMessage[200];
      DWORD nError = GetLastError();
      GetErrorMessage(nError, sMessage, sizeof sMessage);
      printf("Error %ld (%s) on PostQueuedCompletionStatus in shutdownsock\n",
	     nError,
	     sMessage);
    }
  }
#endif
  shutdown(d->descriptor, 2);
  closesocket(d->descriptor);
#ifdef NT_TCP

  /* protect removing the descriptor from our linked list from */
  /* any interference from the listening thread */
  if (platform == VER_PLATFORM_WIN32_NT)
    EnterCriticalSection(&cs);
#endif

  if (d->prev)
    d->prev->next = d->next;
  else				/* d was the first one! */
    descriptor_list = d->next;
  if (d->next)
    d->next->prev = d->prev;

#ifdef NT_TCP
  /* safe to allow the listening thread to continue now */
  if (platform == VER_PLATFORM_WIN32_NT)
    LeaveCriticalSection(&cs);
  else
    /* we cannot free the strings or descriptor if we have queued IOs */
#endif
  {
    freeqs(d);
    mush_free((Malloc_t) d, "descriptor");
  }

  ndescriptors--;
}

DESC *
initializesock(s, addr, ip)
    int s;
    char *addr;
    char *ip;
{
  DESC *d;
  d = (DESC *) mush_malloc(sizeof(DESC), "descriptor");
  if (!d)
    panic("Out of memory.");
  d->descriptor = s;
  d->connected = 0;
  make_nonblocking(s);
  d->output_prefix = 0;
  d->output_suffix = 0;
  d->output_size = 0;
  d->output.head = 0;
  d->player = 0;
  d->output.tail = &d->output.head;
  d->input.head = 0;
  d->input.tail = &d->input.head;
  d->raw_input = 0;
  d->raw_input_at = 0;
  d->quota = COMMAND_BURST_SIZE;
  d->last_time = 0;
  d->cmds = 0;
  d->hide = 0;
  d->doing[0] = '\0';
#ifdef USE_MAILER
  /* Go find where this player's messages start */
  d->mailp = NULL;
#endif
  strncpy(d->addr, addr, 100);
  d->addr[99] = '\0';
  strncpy(d->ip, ip, 100);
  d->ip[99] = '\0';
  d->pueblo = 0;

#ifdef NT_TCP

  /* protect adding the descriptor from the linked list from */
  /* any interference from socket shutdowns */
  if (platform == VER_PLATFORM_WIN32_NT)
    EnterCriticalSection(&cs);
#endif

  if (descriptor_list)
    descriptor_list->prev = d;
  d->next = descriptor_list;
  d->prev = NULL;
  descriptor_list = d;

#ifdef NT_TCP

  /* ok to continue now */
  if (platform == VER_PLATFORM_WIN32_NT)
    LeaveCriticalSection(&cs);
  d->OutboundOverlapped.hEvent = NULL;
  d->InboundOverlapped.hEvent = NULL;
  d->InboundOverlapped.Offset = 0;
  d->InboundOverlapped.OffsetHigh = 0;
  d->bWritePending = FALSE;	/* no write pending yet */
  d->bConnectionShutdown = FALSE;	/* not shutdown yet */
  d->bConnectionDropped = FALSE;	/* not dropped yet */
#else
  welcome_user(d);
#endif

  return d;
}

#ifdef CAN_NEWSTYLE
int
make_socket(Port_t port)
#else
int
make_socket(port)
    Port_t port;
#endif
{
  int s;
  struct sockaddr_in server;
  int opt;
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror("creating stream socket");
    exit(3);
  }
#ifndef macintosh           /* MacOS and GUSI don't seem to like this */
  opt = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
		 (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt");
    exit(1);
  }
#endif     /* !macintosh */
  server.sin_family = AF_INET;
  if (*MUSH_IP_ADDR)
    server.sin_addr.s_addr = inet_addr(MUSH_IP_ADDR);
  else
    server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);
  if (bind(s, (struct sockaddr *) &server, sizeof(server))) {
    perror("binding stream socket");
    closesocket(s);
    exit(4);
  }
  fprintf(stderr, "Listening on port %d\n", port);
  listen(s, 5);
  return s;
}

#ifdef INFO_SLAVE
static void
make_info_slave()
{
  struct sockaddr_in addr;
  int opt;
  Pid_t child;
  char num[10];

  if (info_slave_state != 0) {
    closesocket(info_slave);
    kill(info_slave_pid, 15);
    info_slave_state = 0;
  }
  info_slave = socket(AF_INET, SOCK_STREAM, 0);
  if (info_slave < 0) {
    perror("creating slave ear stream socket");
    return;
  }
  opt = 1;
  if (setsockopt(info_slave, SOL_SOCKET, SO_REUSEADDR,
		 (char *) &opt, sizeof(opt)) < 0) {
    perror("setsockopt");
    return;
  }
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(0);
  if (bind(info_slave, (struct sockaddr *) &addr, sizeof(addr))) {
    perror("binding slave ear stream socket");
    closesocket(info_slave);
    return;
  }
  opt = sizeof(addr);
  if (getsockname(info_slave, (struct sockaddr *) &addr, &opt) < 0) {
    perror("getting address of slave ear stream socket");
    closesocket(info_slave);
    return;
  }
  listen(info_slave, 1);
  child = fork();
  if (child < 0) {
    perror("forking info slave");
    closesocket(info_slave);
    return;
  }
  if (child) {
    info_slave_state = 1;
    info_slave_pid = child;
    do_rawlog(LT_ERR, "Spawning info slave on port %d, pid %d, ident %d",
	      ntohs(addr.sin_port), child, USE_IDENT);
  } else {
    sprintf(num, "%d", ntohs(addr.sin_port));
    if (!USE_IDENT)
      execl("./info_slave", "./info_slave", num, "noident", (char *) NULL);
    else
      execl("./info_slave", "./info_slave", num, (char *) NULL);
    perror("execing info slave");
    exit(1);
  }
  info_query_spill = 0;
  info_reap_spill = 0;
  if (info_slave >= maxd)
    maxd = info_slave + 1;
}

static void
promote_info_slave()
{
  int newsock;
  struct sockaddr_in addr;
  int addr_len;
  int j;

  if (info_slave_state != 1) {
    make_info_slave();
    return;
  }
  addr_len = sizeof(addr);
  newsock = accept(info_slave, (struct sockaddr *) &addr, &addr_len);
  if (newsock < 0) {
    perror("accepting info slave connection");
    make_info_slave();
    return;
  }
  closesocket(info_slave);
  info_slave = newsock;
  make_nonblocking(info_slave);
  /* Do authentication here, if we care */
  info_slave_state = 2;
  do_rawlog(LT_ERR, "Accepted info slave from port %d", ntohs(addr.sin_port));
  for (j = 0; j < maxd; j++)
    if (FD_ISSET(j, &info_pending))
      query_info_slave(j);
  if (info_slave >= maxd)
    maxd = info_slave + 1;
}

static void
query_info_slave(fd)
    int fd;
{
  int len, size;
  static char buf[100];		/* overkill */
  struct sockaddr_in addr;

  FD_SET(fd, &info_pending);
  info_queue_time = time((time_t *) NULL);

  if (info_slave_state != 2) {
    make_info_slave();
    return;
  }
  /* THIS PACKET IS _NOT_ IN NETWORK ORDER */
  size = info_query_spill;	/* cleanup for truncated packet */
  memset(buf, 0, size);
  info_query_spill = 0;
  len = sizeof(addr);
  if (getpeername(fd, (struct sockaddr *) &addr, &len) < 0) {
    perror("socket peer vanished");
    shutdown(fd, 2);
    closesocket(fd);
    FD_CLR(fd, &info_pending);
    return;
  }
  memcpy(buf + size, &addr.sin_addr, sizeof(addr.sin_addr));
  size += sizeof(addr.sin_addr);
  memcpy(buf + size, &addr.sin_port, sizeof(addr.sin_port));
  size += sizeof(addr.sin_port);
  len = sizeof(addr);
  if (getsockname(fd, (struct sockaddr *) &addr, &len) < 0) {
    perror("socket self vanished");
    shutdown(fd, 2);
    closesocket(fd);
    FD_CLR(fd, &info_pending);
    return;
  }
  memcpy(buf + size, &addr.sin_addr, sizeof(addr.sin_addr));
  size += sizeof(addr.sin_addr);
  memcpy(buf + size, &addr.sin_port, sizeof(addr.sin_port));
  size += sizeof(addr.sin_port);
  memcpy(buf + size, &fd, sizeof(fd));
  size += sizeof(fd);

#ifdef WIN32
  len = send(info_slave, buf, size, 0);
#else
  len = write(info_slave, buf, size);
#endif
  if (len < size) {
    perror("info slave query");
#ifdef WIN32
    if (len == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
#else
    if (errno != EWOULDBLOCK)
#endif
      make_info_slave();
    else
      /* drop partial packet on floor.  cleanup later. */
      info_query_spill = size - len;
  }
}

static void
reap_info_slave()
{
  int fd, len, size;
  static char buf[10000];	/* overkill */
  char *bp;

  if (info_slave_state != 2) {
    make_info_slave();
    return;
  }
  if (info_reap_spill) {
    /* clean up some mess */
#ifdef WIN32
    len = recv(info_slave, buf, info_reap_spill, 0);
#else
    len = read(info_slave, buf, info_reap_spill);
#endif
    info_reap_spill -= len;
    if (len < 1) {
      /* crap.  lost the slave. */
      perror("info slave reap spill");
      make_info_slave();
      return;
    }
    if (info_reap_spill)
      /* can't read enough, come back later */
      return;
  }
  for (;;) {
    /* THIS PACKET IS _NOT_ IN NETWORK ORDER */
    /* grab the fd */
#ifdef WIN32
    len = recv(info_slave, (char *) &fd, sizeof(int), 0);
#else
    len = read(info_slave, (char *) &fd, sizeof(int));
#endif
#ifdef WIN32
    if (len != SOCKET_ERROR || WSAGetLastError() != WSAEWOULDBLOCK)
#else
    if (len < 0 && errno == EWOULDBLOCK)
#endif
      /* got all the data */
      return;
    if ((Size_t) len < sizeof(int)) {
      /* we're hosed now */
      perror("info slave reap fd");
      make_info_slave();
      return;
    }
    /* grab the string size */
#ifdef WIN32
    len = recv(info_slave, (char *) &size, sizeof(int), 0);
#else
    len = read(info_slave, (char *) &size, sizeof(int));
#endif
    if ((Size_t) len < sizeof(int) || (Size_t) size > sizeof(buf)) {
      /* we're still hosed */
      perror("info slave reap size");
      make_info_slave();
      return;
    }
    /* grab the actual string */
#ifdef WIN32
    len = recv(info_slave, buf, size, 0);
#else
    len = read(info_slave, buf, size);
#endif
    buf[len] = '\0';
    if (len < size) {
      /* crap... lost some.  clean up the mess and requery */
      info_reap_spill = size - len;
      query_info_slave(fd);
      return;
    }
    /* okay, now we have some info! */
    if (!FD_ISSET(fd, &info_pending))
      /* got a bloody duplicate */
      return;
    FD_CLR(fd, &info_pending);
    /* buf ideally contains ipaddr:ident-info */
    if ((bp = (char *) index(buf, ':'))) {
      *bp++ = '\0';
      /* buf is ip addr, bp is ident info */
    } else {
      /* Oops, just deal with buf */
    }
    if (Forbidden_Site(buf) || (bp && Forbidden_Site(bp))) {
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Refused connection.", fd,
	     bp ? bp : "", buf);
      shutdown(fd, 2);
      closesocket(fd);
#ifndef WIN32
      errno = 0;
#endif
      return;
    }
    do_log(LT_CONN, 0, 0, "[%d/%s/%s] Connection opened.", fd,
	   bp ? bp : "", buf);
    (void) initializesock(fd, bp ? bp : buf, buf);
  }
}

void
kill_info_slave()
{
  if (info_slave_state != 0) {
    closesocket(info_slave);
    kill(info_slave_pid, 15);
    info_slave_state = 0;
  }
}
#endif

struct text_block *
make_text_block(s, n)
    const char *s;
    int n;
{
  struct text_block *p;
  p = (struct text_block *) mush_malloc(sizeof(struct text_block), "text_block");
  if (!p)
    panic("Out of memory");
  p->buf = (char *) mush_malloc(sizeof(char) * n, "text_block_buff");
  if (!p->buf)
    panic("Out of memory");

  memcpy(p->buf, s, n);
  p->nchars = n;
  p->start = p->buf;
  p->nxt = 0;
  return p;
}

void
free_text_block(t)
    struct text_block *t;
{
  if (t) {
    if (t->buf)
      mush_free((Malloc_t) t->buf, "text_block_buff");
    mush_free((Malloc_t) t, "text_block");
  }
}

void
add_to_queue(q, b, n)
    struct text_queue *q;
    const char *b;
    int n;
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
flush_queue(q, n)
    struct text_queue *q;
    int n;
{
  struct text_block *p;
  int really_flushed = 0;
  n += strlen(flushed_message);

  while (n > 0 && (p = q->head)) {
    n -= p->nchars;
    really_flushed += p->nchars;
    q->head = p->nxt;
#ifdef DEBUG
    fprintf(stderr, "free_text_block(0x%x) at 1.\n", p);
#endif				/* DEBUG */
    free_text_block(p);
  }
  p = make_text_block(flushed_message, strlen(flushed_message));
  p->nxt = q->head;
  q->head = p;
  if (!p->nxt)
    q->tail = &p->nxt;
  really_flushed -= p->nchars;
  return really_flushed;
}

int
queue_write(d, b, n)
    DESC *d;
    const char *b;
    int n;
{
  char buff[BUFFER_LEN];
  struct notify_strings messages[5];
  char *s;
  PUEBLOBUFF;
  int i;

  if ((n == 2) && (b[0] == '\r') && (b[1] == '\n')) {
    if (d->pueblo)
      queue_newwrite(d, "<BR>\n", 5);
    else
      queue_newwrite(d, b, 2);
    return n;
  }
  if (n > BUFFER_LEN)
    n = BUFFER_LEN;

  strncpy(buff, b, n);
  buff[n] = '\0';

  for (i = 0; i < 5; i++) {
    messages[i].message = (char *) mush_malloc(BUFFER_LEN, "string");
  }
  if (d->pueblo) {
    PUSE;
    tag_wrap("PRE", NULL, buff);
    PEND;
    messages[NA_PUEBLO].made = 0;
    s = notify_makestring(pbuff, messages, NA_PUEBLO);
  } else {
    messages[NA_COLOR].made = 0;
    s = notify_makestring(buff, messages, NA_COLOR);
  }
  queue_newwrite(d, s, strlen(s));
  for (i = 0; i < 5; i++) {
    mush_free((Malloc_t) messages[i].message, "string");
  }
  return n;
}

int
queue_newwrite(d, b, n)
    DESC *d;
    const char *b;
    int n;
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
			d->output_buffer,
			nBytes,
			NULL,
			&d->OutboundOverlapped);

    d->bWritePending = FALSE;

    if (!bResult)
      if (GetLastError() == ERROR_IO_PENDING)
	d->bWritePending = TRUE;
      else {
	d->bConnectionDropped = TRUE;	/* do no more writes */
	/* post a notification that the descriptor should be shutdown */
	if (!PostQueuedCompletionStatus(CompletionPort, 0, (DWORD) d, &lpo_shutdown)) {
	  char sMessage[200];
	  DWORD nError = GetLastError();
	  GetErrorMessage(nError, sMessage, sizeof sMessage);
	  printf("Error %ld (%s) on PostQueuedCompletionStatus in queue_newwrite\n",
		 nError,
		 sMessage);
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
queue_string(d, s)
    DESC *d;
    const char *s;
{
  char *n;
  int poutput;
  struct notify_strings messages[5];
  dbref target;
  int ret;

  for (poutput = 0; poutput < 5; poutput++) {
    messages[poutput].made = 0;
    messages[poutput].message = (char *) mush_malloc(BUFFER_LEN, "string");
  }

  target = d->player;

  if (d->pueblo) {
    poutput = NA_PUEBLO;
  } else if (ShowAnsi(target)) {
    if (ShowAnsiColor(target))
      poutput = NA_COLOR;
    else
      poutput = NA_ANSI;
  } else {
    poutput = NA_PASCII;
  }
  n = notify_makestring(s, messages, poutput);
  ret = queue_newwrite(d, n, strlen(n));
  for (poutput = 0; poutput < 5; poutput++) {
    mush_free((Malloc_t) messages[poutput].message, "string");
  }
  return ret;
}

int
process_output(d)
    DESC *d;
{
  struct text_block **qp, *cur;
  int cnt;
  for (qp = &d->output.head; ((cur = *qp) != NULL);) {
#ifdef WIN32
    cnt = send(d->descriptor, cur->start, cur->nchars, 0);	/* NJG  */
#else
    cnt = write(d->descriptor, cur->start, cur->nchars);
#endif
    if (cnt < 0) {
#ifdef WIN32
      if (cnt == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
#else
#ifdef EAGAIN
      if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
#else
      if (errno == EWOULDBLOCK)
#endif
#endif
	return 1;
      return 0;
    }
    d->output_size -= cnt;
    if (cnt == cur->nchars) {
      if (!cur->nxt)
	d->output.tail = qp;
      *qp = cur->nxt;
#ifdef DEBUG
      fprintf(stderr, "free_text_block(0x%x) at 2.\n", cur);
#endif				/* DEBUG */
      free_text_block(cur);
      continue;			/* do not adv ptr */
    }
    cur->nchars -= cnt;
    cur->start += cnt;
    break;
  }
  return 1;
}

void
make_nonblocking(s)
    int s;
{
#ifdef WIN32
  unsigned long arg = 1;
  if (ioctlsocket(s, FIONBIO, &arg) == -1) {
#else
  if (fcntl(s, F_SETFL, O_NDELAY) == -1) {
#endif
    perror("make_nonblocking: fcntl");
    panic("O_NDELAY fcntl failed");
  }
}

void
freeqs(d)
    DESC *d;
{
  struct text_block *cur, *next;
  cur = d->output.head;
  while (cur) {
    next = cur->nxt;
#ifdef DEBUG
    fprintf(stderr, "free_text_block(0x%x) at 3.\n", cur);
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
    fprintf(stderr, "free_text_block(0x%x) at 4.\n", cur);
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

void
welcome_user(d)
    DESC *d;
{
  if (SUPPORT_PUEBLO && !d->pueblo)
    queue_newwrite(d, PUEBLO_HELLO, strlen(PUEBLO_HELLO));
  fcache_dump(d, options.connect_fcache);
}

void
do_new_spitfile(player, arg1, index_file, text_file, restricted)
    dbref player;
    char *arg1;
    const char *index_file;
    const char *text_file;
    int restricted;		/* 1 = admin help, 0 = regular help */
{
  int help_found;
  help_indx entry;
  FILE *fp;
  char *p, line[LINE_SIZE + 1];
  char the_topic[LINE_SIZE + 2];

  if (!text_file || !*text_file)
    return;
  if (*arg1 == '\0')
    arg1 = (char *) "help";
  else if (*arg1 == '&') {
    notify(player, "Help topics don't start with '&'.");
    return;
  }
  if (strlen(arg1) > LINE_SIZE)
    *(arg1 + LINE_SIZE) = '\0';

  if (restricted) {
    if (!Hasprivs(player)) {
      notify(player, "You don't look like an admin to me.");
      return;
    }
    sprintf(the_topic, "&%s", arg1);
  } else
    strcpy(the_topic, arg1);

  if ((fp = fopen(index_file, "rb")) == NULL) {
    notify(player, "Sorry, that function is temporarily unavailable.");
    do_log(LT_ERR, 0, 0, "Can't open index file %s for reading", index_file);
    return;
  }
  while ((help_found = fread(&entry, sizeof(help_indx), 1, fp)) == 1)
    if (string_prefix(entry.topic, the_topic))
       break;
  fclose(fp);
  if (!help_found) {
    notify(player, tprintf("No entry for '%s'.", arg1));
    return;
  }
  if ((fp = fopen(text_file, "rb")) == NULL) {
    notify(player, "Sorry, that function is temporarily unavailable.");
    do_log(LT_ERR, 0, 0, "Can't open text file %s for reading", text_file);
    return;
  }
  if (fseek(fp, entry.pos, 0) < 0L) {
    notify(player, "Sorry, that function is temporarily unavailable.");
    do_log(LT_ERR, 0, 0, "Seek error in file %s\n", text_file);
    return;
  }
  strcpy(the_topic, strupper(entry.topic + (*entry.topic == '&')));
  /* ANSI topics */
  if (ShowAnsi(player)) {
    char ansi_topic[LINE_SIZE + 10];
    sprintf(ansi_topic, "%s%s%s", ANSI_HILITE, the_topic, ANSI_NORMAL);
    notify(player, ansi_topic);
  } else
    notify(player, the_topic);

  if (SUPPORT_PUEBLO)
    notify_noenter(player, tprintf("%cPRE%c", TAG_START, TAG_END));
  for (;;) {
    if (fgets(line, LINE_SIZE, fp) == NULL)
      break;
    if (line[0] == '&')
      break;
    if (line[0] == '\n') {
      notify(player, " ");
    } else {
      for (p = line; *p != '\0'; p++)
	if (*p == '\n')
	  *p = '\0';
      notify(player, line);
    }
  }
  if (SUPPORT_PUEBLO)
    notify(player, tprintf("%c/PRE%c", TAG_START, TAG_END));
  fclose(fp);
}


void
save_command(d, command)
    DESC *d;
    const char *command;
{
  add_to_queue(&d->input, command, strlen(command) + 1);
}

void
process_input_helper(d, tbuf1, got)
    DESC *d;
    char *tbuf1;
    int got;
{
  char *p, *pend, *q, *qend;

  if (!d->raw_input) {
    d->raw_input = (char *) mush_malloc(sizeof(char) * MAX_COMMAND_LEN, "descriptor_raw_input");
    if (!d->raw_input)
      panic("Out of memory");
    d->raw_input_at = d->raw_input;
  }
  p = d->raw_input_at;
  pend = d->raw_input + MAX_COMMAND_LEN - 1;
  for (q = tbuf1, qend = tbuf1 + got; q < qend; q++) {
    if (*q == '\n') {
      *p = '\0';
      if (p > d->raw_input)
	save_command(d, d->raw_input);
      p = d->raw_input;
    } else if (*q == '\b') {
      if (p > d->raw_input)
	p--;
    } else if (p < pend &&
#if (COMPRESSION_TYPE > 1)
	       isascii(*q) && isprint(*q)
#else
	       (unsigned char) *q >= (unsigned char) ' '
#endif
      ) {
      *p++ = *q;
    }
  }
  if (p > d->raw_input) {
    d->raw_input_at = p;
  } else {
    mush_free((Malloc_t) d->raw_input, "descriptor_raw_input");
    d->raw_input = 0;
    d->raw_input_at = 0;
  }


}				/* end of process_input_helper */

int
process_input(d)
    DESC *d;
{
  int got;
  char tbuf1[BUFFER_LEN];

#ifdef WIN32
  got = recv(d->descriptor, tbuf1, sizeof tbuf1, 0);
#else
  got = read(d->descriptor, tbuf1, sizeof tbuf1);
#endif
  if (got <= 0)
    return 0;

  process_input_helper(d, tbuf1, got);

  return 1;
}

void
set_userstring(userstring, command)
    char **userstring;
    const char *command;
{
  if (*userstring) {
    mush_free((Malloc_t) * userstring, "userstring");
    *userstring = NULL;
  }
  while (*command && isascii(*command) && isspace(*command))
    command++;
  if (*command)
    *userstring = strdup(command);
}

void
process_commands()
{
  int nprocessed;
  DESC *dnext;
  struct text_block *t;
  int retval = 1;

  do {
    nprocessed = 0;
    for (cdesc = descriptor_list; cdesc;
	 cdesc = (nprocessed > 0 && retval > 0) ? cdesc->next : dnext) {
      dnext = cdesc->next;
      if (cdesc->quota > 0 && (t = cdesc->input.head)) {
	cdesc->quota--;
	nprocessed++;
	retval = do_command(cdesc, t->start);
	if (retval == 0) {
	  shutdownsock(cdesc);
	} else if (retval == -1) {
	  logout_sock(cdesc);
	} else {
	  cdesc->input.head = t->nxt;
	  if (!cdesc->input.head)
	    cdesc->input.tail = &cdesc->input.head;
	  if (t) {
#ifdef DEBUG
	    fprintf(stderr, "free_text_block(0x%x) at 5.\n", t);
#endif				/* DEBUG */
	    free_text_block(t);
	  }
	}
      }
    }
  } while (nprocessed > 0);
  cdesc = (DESC *) NULL;
}

#define send_prefix(d) \
  if (d->output_prefix) { \
    queue_newwrite(d, d->output_prefix, strlen(d->output_prefix)); \
    queue_newwrite(d, "\r\n", 2); \
  }

#define send_suffix(d) \
  if (d->output_suffix) { \
    queue_newwrite(d, d->output_suffix, strlen(d->output_suffix)); \
    queue_newwrite(d, "\r\n", 2); \
  }

int
do_command(d, command)
    DESC *d;
    char *command;
{
  int j;

  depth = 0;
  (d->cmds)++;

  if (!strcmp(command, QUIT_COMMAND)) {
    return 0;
  } else if (!strcmp(command, LOGOUT_COMMAND)) {
    return -1;
  } else if (!strcmp(command, INFO_COMMAND)) {
    send_prefix(d);
    dump_info(d);
    send_suffix(d);
  } else if (!strncmp(command, WHO_COMMAND, strlen(WHO_COMMAND))) {
    send_prefix(d);
    dump_users(d, command + strlen(WHO_COMMAND), 0);
    send_suffix(d);
  } else if (!strncmp(command, DOING_COMMAND, strlen(DOING_COMMAND))) {
    send_prefix(d);
    dump_users(d, command + strlen(DOING_COMMAND), 1);
    send_suffix(d);
  } else if (!strncmp(command, PREFIX_COMMAND, strlen(PREFIX_COMMAND))) {
    set_userstring(&d->output_prefix, command + strlen(PREFIX_COMMAND));
  } else if (!strncmp(command, SUFFIX_COMMAND, strlen(SUFFIX_COMMAND))) {
    set_userstring(&d->output_suffix, command + strlen(SUFFIX_COMMAND));
  } else if (SUPPORT_PUEBLO && !strncmp(command, PUEBLO_COMMAND, strlen(PUEBLO_COMMAND))) {
    if (!d->pueblo) {
      queue_newwrite(d, PUEBLO_SEND, strlen(PUEBLO_SEND));
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Switching to Pueblo mode.",
	     d->descriptor, d->addr, d->ip);
      d->pueblo = 1;
      if (!d->connected)
	welcome_user(d);
    }
  } else {
    if (d->connected) {
      send_prefix(d);
      cplr = d->player;
      strcpy(ccom, command);

      /* Clear %0-%9 and r(0) - r(9) */
      for (j = 0; j < 10; j++) {
	wenv[j] = (char *) NULL;
	renv[j][0] = '\0';
      }

      process_command(d->player, command, d->player, 1);
      send_suffix(d);
    } else {
      if (!check_connect(d, command))
	return 0;
    }
  }
  return 1;
}

static int
dump_messages(d, player, isnew)
    DESC *d;
    dbref player;
    int isnew;


  /* 0 if connect, 1 if create */
{
  d->connected = 1;
  d->connected_at = time((time_t *) 0);
  d->player = player;
  d->doing[0] = '\0';

  if (MAX_LOGINS) {
    /* check for exceeding max player limit */
    login_number++;
    if (under_limit && (login_number > MAX_LOGINS)) {
      under_limit = 0;
      fprintf(connlog_fp,
	      "Limit of %d players reached. Logins disabled.\n",
	      MAX_LOGINS);
      fflush(connlog_fp);
    }
  }
  /* give players a message on connection */
  if (!options.login_allow || !under_limit ||
      (Guest(player) && !options.guest_allow)) {
    if (!options.login_allow) {
      raw_notify(player, asterisk_line);
      fcache_dump(d, options.down_fcache);
      if (cf_downmotd_msg && *cf_downmotd_msg)
	raw_notify(player, cf_downmotd_msg);
    } else if (MAX_LOGINS && !under_limit) {
      raw_notify(player, asterisk_line);
      fcache_dump(d, options.full_fcache);
      if (cf_fullmotd_msg && *cf_fullmotd_msg)
	raw_notify(player, cf_fullmotd_msg);
    }
    if (!Can_Login(player)) {
      raw_notify(player, asterisk_line);
      /* when the connection has been refused, we want to update the
       * LASTFAILED info on the player
       */
      check_lastfailed(player, d->addr);
      return 0;
    } else
      raw_notify(player, asterisk_line);
  }
#ifdef USE_MAILER
  d->mailp = find_exact_starting_point(player);
#endif

  /* give permanent text messages */
  if (isnew)
    fcache_dump(d, options.newuser_fcache);
  fcache_dump(d, options.motd_fcache);
  if (Hasprivs(player))
    fcache_dump(d, options.wizmotd_fcache);
  if (Guest(player))
    fcache_dump(d, options.guest_fcache);

#ifdef CREATION_TIMES
  if (ModTime(player))
    notify(player, tprintf("%ld failed connections since last login.",
			   ModTime(player)));
  ModTime(player) = (time_t) 0;
#endif
  announce_connect(player);	/* broadcast connect message */
  check_last(player, d->addr, d->ip);	/* set Last, Lastsite, give paycheck */
#ifdef USE_MAILER
  /* Check folder 0, not silently (i.e. Report lack of mail, too) */
  queue_write(d, "\r\n", 2);
  check_mail(player, 0, 0);
  set_player_folder(player, 0);
#endif
  do_look_around(player);
  if (Haven(player))
    notify(player, "Your HAVEN flag is set. You cannot receive pages.");
#ifdef VACATION_FLAG
  if (Vacation(player)) {
    notify(player, "Welcome back from vacation!");
    set_flag(player, player, (char *) "ON-VACATION", 1, 0, 0);
  }
#endif
  return 1;
}

int
check_connect(d, msg)
    DESC *d;
    const char *msg;
{
  char command[MAX_COMMAND_LEN];
  char user[MAX_COMMAND_LEN];
  char password[MAX_COMMAND_LEN];
  dbref player;

  parse_connect(msg, command, user, password);

  if (string_prefix("connect", command)) {
    if ((player = connect_player(user, password, d->addr, d->ip)) == NOTHING) {
      queue_string(d, connect_fail);
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Failed connect to '%s'.",
	     d->descriptor, d->addr, d->ip, user);
    } else {
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Connected to %s(#%d) in %s(#%d)",
	     d->descriptor, d->addr, d->ip, Name(player), player,
	     Name(Location(player)), Location(player));
      if ((dump_messages(d, player, 0)) == 0) {
	d->connected = 2;
	return 0;
      }
    }

  } else if (!strcasecmp(command, "cd")) {
    if ((player = connect_player(user, password, d->addr, d->ip)) == NOTHING) {
      queue_string(d, connect_fail);
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Failed connect to '%s'.",
	     d->descriptor, d->addr, d->ip, user);
    } else {
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Connected dark to %s(#%d) in %s(#%d)",
	     d->descriptor, d->addr, d->ip, Name(player), player,
	     Name(Location(player)), Location(player));
      /* Set player dark */
      d->connected = 1;
      d->player = player;
      set_flag(player, player, (char *) "DARK", 0, 0, 0);
      if ((dump_messages(d, player, 0)) == 0) {
	d->connected = 2;
	return 0;
      }
    }

  } else if (!strcasecmp(command, "ch")) {
    if ((player = connect_player(user, password, d->addr, d->ip)) == NOTHING) {
      queue_string(d, connect_fail);
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Failed connect to '%s'.",
	     d->descriptor, d->addr, d->ip, user);
    } else {
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Connected hidden to %s(#%d) in %s(#%d)",
	     d->descriptor, d->addr, d->ip, Name(player), player,
	     Name(Location(player)), Location(player));
      /* Set player hidden */
      d->connected = 1;
      d->player = player;
      if (Can_Hide(player))
	d->hide = 1;
      if ((dump_messages(d, player, 0)) == 0) {
	d->connected = 2;
	return 0;
      }
    }

  } else if (string_prefix("create", command)) {
    if (!Site_Can_Create(d->addr) || !Site_Can_Create(d->ip)) {
      fcache_dump(d, options.register_fcache);
      if (!Deny_Silent_Site(d->addr) && !Deny_Silent_Site(d->ip)) {
	do_log(LT_CONN, 0, 0, "[%d/%s/%s] Refused create for '%s'.",
	       d->descriptor, d->addr, d->ip, user);
	fflush(connlog_fp);
      }
      return 0;
    }
    if (!options.login_allow || !options.create_allow) {
      fcache_dump(d, options.down_fcache);
      fprintf(connlog_fp,
	      "REFUSED CREATION for %s from %s on descriptor %d.\n",
	      user, d->addr, d->descriptor);
      fflush(connlog_fp);
      return 0;
    } else if (MAX_LOGINS && !under_limit) {
      fcache_dump(d, options.full_fcache);
      fprintf(connlog_fp,
	      "REFUSED CREATION for %s from %s on descriptor %d.\n",
	      user, d->addr, d->descriptor);
      fflush(connlog_fp);
      return 0;
    }
    player = create_player(user, password, d->addr, d->ip);
    if (player == NOTHING) {
      queue_string(d, create_fail);
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Failed create for '%s' (bad name).",
	     d->descriptor, d->addr, d->ip, user);
    } else if (player == AMBIGUOUS) {
      queue_string(d, create_fail);
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Failed create for '%s' (bad password).",
	     d->descriptor, d->addr, d->ip, user);
    } else {
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Created %s(#%d)",
	     d->descriptor, d->addr, d->ip, Name(player), player);
      if ((dump_messages(d, player, 1)) == 0) {
	d->connected = 2;
	return 0;
      }
    }				/* successful player creation */

  } else if (string_prefix("register", command)) {
    if (!Site_Can_Register(d->addr) || !Site_Can_Register(d->ip)) {
      fcache_dump(d, options.register_fcache);
      if (!Deny_Silent_Site(d->addr) && !Deny_Silent_Site(d->ip)) {
	do_log(LT_CONN, 0, 0, "[%d/%s/%s] Refused registration (bad site) for '%s'.",
	       d->descriptor, d->addr, d->ip, user);
	fflush(connlog_fp);
      }
      return 0;
    }
    if (!options.create_allow) {
      fcache_dump(d, options.down_fcache);
      fprintf(connlog_fp,
	      "Refused registration (creation disabled) for %s from %s on descriptor %d.\n",
	      user, d->addr, d->descriptor);
      fflush(connlog_fp);
      return 0;
    }
    if ((player = email_register_player(user, password, d->addr, d->ip)) == NOTHING) {
      queue_string(d, register_fail);
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Failed registration for '%s'.",
	     d->descriptor, d->addr, d->ip, user);
    } else {
      queue_string(d, register_success);
      do_log(LT_CONN, 0, 0, "[%d/%s/%s] Registered %s(#%d) to %s",
	  d->descriptor, d->addr, d->ip, Name(player), player, password);
    }
    /* Even when registration succeeds, they don't connect, so return 0 */
    return 0;

  } else {
    /* invalid command, just repeat login screen */
    welcome_user(d);
  }
  fflush(connlog_fp);
  return 1;
}

void
parse_connect(msg, command, user, pass)
    const char *msg;
    char *command;
    char *user;
    char *pass;
{
  char *p;

  while (*msg && isascii(*msg) && isspace(*msg))
    msg++;
  p = command;
  while (*msg && isascii(*msg) && !isspace(*msg))
    *p++ = *msg++;
  *p = '\0';
  while (*msg && isascii(*msg) && isspace(*msg))
    msg++;
  p = user;

  if (PLAYER_NAME_SPACES && *msg == '\"') {
    for (; *msg && ((*msg == '\"') || isspace(*msg)); msg++) ;
    while (*msg && (*msg != '\"')) {
      while (*msg && !isspace(*msg) && (*msg != '\"'))
	*p++ = *msg++;
      if (*msg == '\"') {
	msg++;
	while (*msg && isspace(*msg))
	  msg++;
	break;
      }
      while (*msg && isspace(*msg))
	msg++;
      if (*msg && (*msg != '\"'))
	*p++ = ' ';
    }
  } else
    while (*msg && isascii(*msg) && !isspace(*msg))
      *p++ = *msg++;

  *p = '\0';
  while (*msg && isascii(*msg) && isspace(*msg))
    msg++;
  p = pass;
  while (*msg && isascii(*msg) && !isspace(*msg))
    *p++ = *msg++;
  *p = '\0';
}

void
close_sockets()
{
  DESC *d, *dnext;

  if (USE_RWHO)
    rwhocli_shutdown();
  for (d = descriptor_list; d; d = dnext) {
    dnext = d->next;
#ifdef WIN32
    send(d->descriptor, shutdown_message, strlen(shutdown_message), 0);
#else
    write(d->descriptor, shutdown_message, strlen(shutdown_message));
#endif
    if (shutdown(d->descriptor, 2) < 0)
      perror("shutdown");
    closesocket(d->descriptor);
  }
#ifdef NT_TCP
  /* shutdown listening thread */
  if (platform == VER_PLATFORM_WIN32_NT)
    closesocket(MUDListenSocket);
#endif

}
void
emergency_shutdown()
{
  close_sockets();
}

void
boot_desc(d)
    DESC *d;
{
  shutdownsock(d);
}

DESC *
player_desc(player)
    dbref player;
{
  DESC *d;
  for (d = descriptor_list; d; d = d->next) {
    if (d->connected && (d->player == player)) {
      return d;
    }
  }
  return (DESC *) NULL;
}

DESC *
inactive_desc(player)
    dbref player;
{
  DESC *d;
  time_t now;
  now = time((time_t *) 0);
  DESC_ITER_CONN(d) {
    if ((d->player == player) && (now - d->last_time > 60)) {
      return d;
    }
  }
  return (DESC *) NULL;
}

DESC *
port_desc(port)
    int port;
{
  DESC *d;
  for (d = descriptor_list; (d); d = d->next) {
    if (d->descriptor == port) {
      return d;
    }
  }
  return (DESC *) NULL;
}

dbref
find_player_by_desc(port)
    int port;
{
  /* given a descriptor, find the matching player dbref */

  DESC *d;
  for (d = descriptor_list; (d); d = d->next) {
    if (d->connected && (d->descriptor == port)) {
      return d->player;
    }
  }

  /* didn't find anything */
  return NOTHING;
}

Signal_t
bailout(sig)
    int sig;
{
  char tbuf1[BUFFER_LEN];

  sprintf(tbuf1, "BAILOUT: caught signal %d", sig);
  panic(tbuf1);
  _exit(7);
#ifndef VOIDSIG
  return 0;
#endif
}

static void
dump_info(call_by)
    DESC *call_by;
{
  int count = 0;
  DESC *d;
  queue_string(call_by, tprintf("### Begin INFO %s\r\n", INFO_VERSION));
  /* Count connected players */
  for (d = descriptor_list; d; d = d->next) {
    if (d->connected) {
      if (!GoodObject(d->player))
	continue;
      if (COUNT_ALL || !Hidden(d))
	count++;
    }
  }
  queue_string(call_by, tprintf("Name: %s\r\n", options.mud_name));
  queue_string(call_by, tprintf("Uptime: %s\r", ctime(&start_time)));
  queue_string(call_by, tprintf("Connected: %d\r\n", count));
  queue_string(call_by, tprintf("Size: %d\r\n", db_top));
  queue_string(call_by, tprintf("Version: %s\r\n", SHORTVN));
  queue_string(call_by, "### End INFO\r\n");
}

void
dump_users(call_by, match, doing)
    DESC *call_by;
    char *match;
    int doing;


  /* 0 if normal WHO, 1 if DOING */
{
  DESC *d;
  int count = 0;
  time_t now;
  char tbuf1[BUFFER_LEN];
  char tbuf2[BUFFER_LEN];

  if (!GoodObject(call_by->player)) {
    do_log(LT_ERR, 0, 0, "Bogus caller #%d of dump_users", call_by->player);
    return;
  }
  while (*match && *match == ' ')
    match++;
  now = time((time_t *) 0);

/* If a wizard/royal types "DOING" it gives him the normal player WHO,
 * BUT flags are not shown. Wizard/royal WHO does not show @doings.
 */

  if (SUPPORT_PUEBLO && call_by->pueblo)
    queue_newwrite(call_by, "<PRE>", 5);

  if ((doing) || !call_by->player || !Priv_Who(call_by->player)) {
    if (poll_msg[0] == '\0')
      strcpy(poll_msg, "Doing");
    if (ShowAnsi(call_by->player))
      sprintf(tbuf2, "Player Name          On For   Idle  %s%s\r\n",
	      poll_msg, ANSI_NORMAL);
    else
      sprintf(tbuf2, "Player Name          On For   Idle  %s\r\n",
	      poll_msg);
    queue_string(call_by, tbuf2);
  } else {
    queue_string(call_by,
	  "Player Name      Loc #    On For  Idle  Cmds  Des  Host\r\n");
  }

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected) {
      if (!GoodObject(d->player))
	continue;
      if (COUNT_ALL || (!Hidden(d) || (call_by->player && Priv_Who(call_by->player))))
	count++;
      if (match && !(string_prefix(Name(d->player), match)))
	continue;

      if (call_by->connected && !(doing) && call_by->player && Priv_Who(call_by->player)) {
	sprintf(tbuf1, "%-16s %5d %9s %5s  %4d  %3d  %s",
		Name(d->player),
		Location(d->player),
		time_format_1(now - d->connected_at),
		time_format_2(now - d->last_time),
		d->cmds,
		d->descriptor,
		d->addr);
	tbuf1[78] = '\0';
	if (Dark(d->player)) {
	  tbuf1[71] = '\0';
	  strcat(tbuf1, " (Dark)");
	} else if (Hidden(d)) {
	  tbuf1[71] = '\0';
	  strcat(tbuf1, " (Hide)");
	}
      } else {
	if (!Hidden(d) || (call_by->player && Priv_Who(call_by->player) && (doing))) {
	  sprintf(tbuf1, "%-16s %10s   %4s%c %s",
		  Name(d->player),
		  time_format_1(now - d->connected_at),
		  time_format_2(now - d->last_time)
		  ,(Dark(d->player) ? 'D' :
		    (Hidden(d) ? 'H' : ' '))
		  ,d->doing
	    );
	}
      }

      if (!Hidden(d) || (call_by->player && Priv_Who(call_by->player))) {
	queue_string(call_by, tbuf1);
	if (SUPPORT_PUEBLO && call_by->pueblo)
	  queue_newwrite(call_by, "\n", 1);
	else
	  queue_newwrite(call_by, "\r\n", 2);
      }
    }
  }
  if (SUPPORT_PUEBLO && call_by->pueblo)
    queue_newwrite(call_by, "</PRE>\n", 7);
  switch (count) {
  case 0:
    strcpy(tbuf1, "There are no players connected.\r\n");
    break;
  case 1:
    strcpy(tbuf1, "There is 1 player connected.\r\n");
    break;
  default:
    sprintf(tbuf1, "There are %d players connected.\r\n", count);
    break;
  }
  queue_string(call_by, tbuf1);
}

const char *
time_format_1(dt)
    long int dt;
{
  register struct tm *delta;
  time_t holder;		/* A hack for 64bit SGI */
  static char buf[64];
  if (dt < 0)
    dt = 0;
  holder = (time_t) dt;
  delta = gmtime(&holder);
  if (delta->tm_yday > 0) {
    sprintf(buf, "%dd %02d:%02d",
/*   sprintf(buf, "%d:%02d:%02d", */
	    delta->tm_yday, delta->tm_hour, delta->tm_min);
  } else {
    sprintf(buf, "%02d:%02d",
	    delta->tm_hour, delta->tm_min);
  }
  return buf;
}

const char *
time_format_2(dt)
    long int dt;
{
  register struct tm *delta;
  static char buf[64];
  if (dt < 0)
    dt = 0;

  delta = gmtime((time_t *) & dt);
  if (delta->tm_yday > 0) {
    sprintf(buf, "%dd", delta->tm_yday);
  } else if (delta->tm_hour > 0) {
    sprintf(buf, "%dh", delta->tm_hour);
  } else if (delta->tm_min > 0) {
    sprintf(buf, "%dm", delta->tm_min);
  } else {
    sprintf(buf, "%ds", delta->tm_sec);
  }
  return buf;
}

void
announce_connect(player)
    dbref player;
{
  dbref loc;
  ATTR *temp;
  char tbuf1[BUFFER_LEN];
  dbref zone;
  dbref obj;
  char *s;
  DESC *d;
  int num = 0;
  int is_hidden;
  int j;

#ifdef CHAT_SYSTEM
  CHAN *c;
  CHANUSER *u;
#endif

  Toggles(player) |= PLAYER_CONNECT;

  is_hidden = Can_Hide(player) && Dark(player);

  /* send out RWHO stuff. We're going to use tbuf1 in a moment again */
  if (USE_RWHO) {
    sprintf(tbuf1, "%d@%s", player, MUDNAME);
    rwhocli_userlogin(tbuf1, Name(player), time((time_t *) 0));
  }
  /* check to see if this is a reconnect and also set DARK status */
  DESC_ITER_CONN(d) {
    if (d->player == player) {
      num++;
      if (is_hidden)
	d->hide = 1;
    }
  }

  sprintf(tbuf1, "%s has %s%sconnected.", Name(player),
	  Dark(player) ? "DARK-" : (hidden(player) ? "HIDDEN-" : ""),
	  (num > 1) ? "re" : "");

  /* send out messages */
  if (Suspect(player))
    flag_broadcast(WIZARD, 0, "Broadcast: Suspect %s", tbuf1);

  if (Dark(player)) {
#ifdef ROYALTY_FLAG
    flag_broadcast(WIZARD | ROYALTY, PLAYER_MONITOR, "GAME: %s", tbuf1);
#else
    flag_broadcast(WIZARD, PLAYER_MONITOR, "GAME: %s", tbuf1);
#endif
  } else
    flag_broadcast(0, PLAYER_MONITOR, "GAME: %s", tbuf1);

  /* tell players on a channel when someone connects */
#ifdef CHAT_SYSTEM
  for (c = channels; c; c = c->next) {
    u = onchannel(player, c);
    if (u &&!Channel_Quiet(c) && (Channel_Admin(c) || Channel_Wizard(c)
				|| (!Chanuser_Hide(u) && !Dark(player))))
       channel_broadcast(c, player, 1, "<%s> %s", ChanName(c), tbuf1);
  }
#endif				/* CHAT_SYSTEM */

  loc = Location(player);
  if (!GoodObject(loc)) {
    notify(player, "You are nowhere!");
    return;
  }
  orator = player;

  raw_notify(player, asterisk_line);
  if (cf_motd_msg && *cf_motd_msg)
    raw_notify(player, cf_motd_msg);
  raw_notify(player, " ");
  if (Hasprivs(player) && cf_wizmotd_msg && *cf_wizmotd_msg)
    raw_notify(player, cf_wizmotd_msg);
  raw_notify(player, asterisk_line);

  notify_except(Contents(player), player, tbuf1);
  /* added to allow player's inventory to hear a player connect */

  if (!Dark(player))
    notify_except(Contents(loc), player, tbuf1);

  /* clear the environment for possible actions */
  for (j = 0; j < 10; j++) {
    wnxt[j] = NULL;
    rnxt[j] = NULL;
  }

  /* do the person's personal connect action */
  temp = atr_get(player, "ACONNECT");
  if (temp) {
    s = safe_uncompress(temp->value);
    parse_que(player, s, player);
    free((Malloc_t) s);
  }
  if (ROOM_CONNECTS) {
    /* Do the room the player connected into */
    if (IsRoom(loc)) {
      temp = atr_get(loc, "ACONNECT");
      if (temp) {
	s = safe_uncompress(temp->value);
	parse_que(loc, s, player);
	free((Malloc_t) s);
      }
    }
  }
  if (GLOBAL_CONNECTS) {
    /* do the zone of the player's location's possible aconnect */
    if ((zone = Zone(loc)) != NOTHING) {
      switch (Typeof(zone)) {
      case TYPE_THING:
	temp = atr_get(zone, "ACONNECT");
	if (temp) {
	  s = safe_uncompress(temp->value);
	  parse_que(zone, s, player);
	  free((Malloc_t) s);
	}
	break;
      case TYPE_ROOM:
	/* check every object in the room for a connect action */
	DOLIST(obj, Contents(zone)) {
	  temp = atr_get(obj, "ACONNECT");
	  if (temp) {
	    s = safe_uncompress(temp->value);
	    parse_que(obj, s, player);
	    free((Malloc_t) s);
	  }
	}
	break;
      default:
	do_log(LT_ERR, 0, 0, "Invalid zone #%d for %s(#%d) has bad type %d",
	       zone, Name(player), player, Typeof(zone));
      }
    }
    /* now try the master room */
    if (DO_GLOBALS) {
      DOLIST(obj, Contents(MASTER_ROOM)) {
	temp = atr_get(obj, "ACONNECT");
	if (temp) {
	  s = safe_uncompress(temp->value);
	  parse_que(obj, s, player);
	  free((Malloc_t) s);
	}
      }
    }
  }
}

void
announce_disconnect(player)
    dbref player;
{
  dbref loc;
  int num;
  ATTR *temp;
  DESC *d;
  char tbuf1[BUFFER_LEN];
  dbref zone, obj;
  char *s;
  char *p;
  time_t tt;
  int j;

#ifdef CHAT_SYSTEM
  CHAN *c;
  CHANUSER *u;
#endif

  tt = time((time_t *) 0);
  p = ctime(&tt);
  p[strlen(p) - 1] = 0;

  loc = Location(player);
  if (!GoodObject(loc))
    return;

  orator = player;

  for (num = 0, d = descriptor_list; d; d = d->next)
    if (d->connected && (d->player == player))
      num++;
  if (num < 2) {

    if (USE_RWHO) {
      sprintf(tbuf1, "%d@%s", player, MUDNAME);
      rwhocli_userlogout(tbuf1);
    }
    sprintf(tbuf1, "%s has disconnected.", Name(player));

    if (!Dark(player))
      notify_except(Contents(loc), player, tbuf1);
    /* notify contents */
    notify_except(Contents(player), player, tbuf1);

    /* clear the environment for possible actions */
    for (j = 0; j < 10; j++) {
      wnxt[j] = NULL;
      rnxt[j] = NULL;
    }

    temp = atr_get(player, "ADISCONNECT");
    if (temp) {
      s = safe_uncompress(temp->value);
      parse_que(player, s, player);
      free((Malloc_t) s);
    }
    if (ROOM_CONNECTS)
      if (IsRoom(loc)) {
	temp = atr_get(loc, "ADISCONNECT");
	if (temp) {
	  s = safe_uncompress(temp->value);
	  parse_que(loc, s, player);
	  free((Malloc_t) s);
	}
      }
    if (GLOBAL_CONNECTS) {
      /* do the zone of the player's location's possible adisconnect */
      if ((zone = Zone(loc)) != NOTHING) {
	switch (Typeof(zone)) {
	case TYPE_THING:
	  temp = atr_get(zone, "ADISCONNECT");
	  if (temp) {
	    s = safe_uncompress(temp->value);
	    parse_que(zone, s, player);
	    free((Malloc_t) s);
	  }
	  break;
	case TYPE_ROOM:
	  /* check every object in the room for a connect action */
	  DOLIST(obj, Contents(zone)) {
	    temp = atr_get(obj, "ADISCONNECT");
	    if (temp) {
	      s = safe_uncompress(temp->value);
	      parse_que(obj, s, player);
	      free((Malloc_t) s);
	    }
	  }
	  break;
	default:
	  do_log(LT_ERR, 0, 0, "Invalid zone #%d for %s(#%d) has bad type %d",
		 zone, Name(player), player, Typeof(zone));
	}
      }
      /* now try the master room */
      if (DO_GLOBALS) {
	DOLIST(obj, Contents(MASTER_ROOM)) {
	  temp = atr_get(obj, "ADISCONNECT");
	  if (temp) {
	    s = safe_uncompress(temp->value);
	    parse_que(obj, s, player);
	    free((Malloc_t) s);
	  }
	}
      }
    }
    Toggles(player) &= ~PLAYER_CONNECT;

    sprintf(tbuf1, "%s has %sdisconnected.", Name(player),
	    Dark(player) ? "DARK-" : "");
  } else {
    /* note: when you partially disconnect, ADISCONNECTS are not executed */
    sprintf(tbuf1, "%s has partially disconnected.", Name(player));

    if (!Dark(player))
      notify_except(Contents(loc), player, tbuf1);
    /* notify contents */
    notify_except(Contents(player), player, tbuf1);
  }
  /* now print messages */
  if (Suspect(player))
    flag_broadcast(WIZARD, 0, "Broadcast: Suspect %s", tbuf1);

  if (Dark(player)) {
#ifdef ROYALTY_FLAG
    flag_broadcast(WIZARD | ROYALTY, PLAYER_MONITOR, "GAME: %s", tbuf1);
#else
    flag_broadcast(WIZARD, PLAYER_MONITOR, "GAME: %s", tbuf1);
#endif
  } else
    flag_broadcast(0, PLAYER_MONITOR, "GAME: %s", tbuf1);

  /* tell players on channel that someone's left */
#ifdef CHAT_SYSTEM
  for (c = channels; c; c = c->next) {
    u = onchannel(player, c);
    if (u) {
      if (((!Dark(player) && !Chanuser_Hide(u))
	   || Channel_Admin(c) || Channel_Wizard(c)) &&
	  !Channel_Quiet(c))
	 channel_broadcast(c, player, 1, "<%s> %s", ChanName(c), tbuf1);
      /* Ungag them from the channel */
      do_chan_user_flags(player, ChanName(c), "no", 2, 1);
    }
  }
#endif				/* CHAT_SYSTEM */
}

void
do_motd(player, key, message)
    dbref player;
    int key;
    const char *message;
{

  if (!Wizard(player) && (key != 3)) {
    notify(player,
	   "You may get 15 minutes of fame and glory in life, but not right now.");
    return;
  }
  switch (key) {
  case 1:
    strcpy(cf_motd_msg, message);
    notify(player, "Motd set.");
    break;
  case 2:
    strcpy(cf_wizmotd_msg, message);
    notify(player, "Wizard motd set.");
    break;
  case 4:
    strcpy(cf_downmotd_msg, message);
    notify(player, "Down motd set.");
    break;
  case 5:
    strcpy(cf_fullmotd_msg, message);
    notify(player, "Full motd set.");
    break;
  default:
    notify(player, tprintf("MOTD: %s", cf_motd_msg));
    if (Hasprivs(player)) {
      notify(player, tprintf("Wiz MOTD: %s", cf_wizmotd_msg));
      notify(player, tprintf("Down MOTD: %s", cf_downmotd_msg));
      notify(player, tprintf("Full MOTD: %s", cf_fullmotd_msg));
    }
  }
}

void
do_doing(player, message)
    dbref player;
    const char *message;
{
  char buf[MAX_COMMAND_LEN];
  DESC *d;
  int i;

  if (!Connected(player)) {
    /* non-connected things have no need for a doing */
    notify(player, "Why would you want to do that?");
    return;
  }
  strncpy(buf, message, DOING_LEN - 1);

  /* now smash undesirable characters and truncate */
  for (i = 0; i < DOING_LEN; i++) {
    if ((buf[i] == '\r') || (buf[i] == '\n') || (buf[i] == 27) ||
	(buf[i] == '\t') || (buf[i] == BEEP_CHAR))
      buf[i] = ' ';
  }
  buf[DOING_LEN - 1] = '\0';

  /* set it */
  for (d = descriptor_list; d; d = d->next)
    if (d->connected && (d->player == player))
      strcpy(d->doing, buf);
  if (strlen(message) >= DOING_LEN) {
    notify(player,
	   tprintf("Doing set. %d characters lost.", strlen(message) - (DOING_LEN - 1)));
  } else
    notify(player, "Doing set.");
}

/* this sets the message which replaces "Doing" */
void
do_poll(player, message)
    dbref player;
    const char *message;
{

  if (!Change_Poll(player)) {
    notify(player, "Who do you think you are, Gallup?");
    return;
  }
  strncpy(poll_msg, message, DOING_LEN - 1);
  if (strlen(message) >= DOING_LEN) {
    poll_msg[DOING_LEN - 1] = 0;
    notify(player,
	   tprintf("Poll set. %d characters lost.", strlen(message) - (DOING_LEN - 1)));
  } else
    notify(player, "Poll set.");
  do_log(LT_WIZ, player, NOTHING, "Poll Set to '%s'.", poll_msg);
  fflush(wizlog_fp);
}

void
rwho_update()
{
  DESC *d;
  char tbuf1[BUFFER_LEN];

  rwhocli_pingalive();
  for (d = descriptor_list; d; d = d->next) {
    if (d->connected && !Hidden(d)) {
      sprintf(tbuf1, "%d@%s", d->player, MUDNAME);
      rwhocli_userlogin(tbuf1, Name(d->player), d->connected_at);
    }
  }
}

static const char *
hostname_convert(nums)
    struct in_addr nums;
{
/* given an address, convert it to either IP numbers or a hostname */

  struct hostent *he;

  if (options.use_dns) {
    he = gethostbyaddr((char *) &nums.s_addr, sizeof(nums.s_addr), AF_INET);
    if (he == NULL)
      return (ip_convert(nums));	/* IP numbers */
    else
      return (he->h_name);	/* hostname */
  } else
    return (ip_convert(nums));
}

static const char *
ip_convert(nums)
    struct in_addr nums;
{
  /* given an address, convert it to IP numbers */
  return inet_ntoa(nums);
}

dbref
short_page(match)
    const char *match;
{
  /* attempts to match to the partial name of a connected player */

  DESC *d;
  dbref who1 = NOTHING;
  int count = 0;

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected) {
      if (match && !string_prefix(Name(d->player), match))
	continue;
      if (!strcasecmp(Name(d->player), match)) {
	count = 1;
	who1 = d->player;
	break;
      }
      if (who1 == NOTHING || d->player != who1) {
	who1 = d->player;
	count++;
      }
    }
  }

  if (count > 1)
    return AMBIGUOUS;
  else if (count == 0)
    return NOTHING;

  return who1;
}


/* LWHO() function - really belongs in eval.c but needs stuff declared here */

/* ARGSUSED */
FUNCTION(fun_lwho)
{
  DESC *d;
  int first;
  int powered = (*called_as == 'L');

  first = 1;
  DESC_ITER_CONN(d) {
    if (!Hidden(d) || (powered && Priv_Who(executor))) {
      if (first)
	first = 0;
      else
	safe_chr(' ', buff, bp);
      safe_str(unparse_dbref(d->player), buff, bp);
    }
  }
}

/* ARGSUSED */
FUNCTION(fun_doing)
{
  /* Gets a player's @doing */
  DESC *d;
  dbref target;

  target = lookup_player(args[0]);
  if (target == NOTHING) {
    target = match_result(executor, args[0], TYPE_PLAYER,
			  MAT_ABSOLUTE | MAT_PLAYER | MAT_ME);
  }
  if ((target == NOTHING) || !Connected(target)) {
    safe_str("#-1", buff, bp);
    return;
  }
  /* else walk the descriptor list looking for a match */
  DESC_ITER_CONN(d) {
    if (d->player == target) {
      if (!Hidden(d) || Priv_Who(executor))
	safe_str(d->doing, buff, bp);
      else
	safe_str("#-1", buff, bp);
      return;
    }
  }

  /* if we hit this point we are in trouble */
  safe_str("#-1", buff, bp);
  do_log(LT_ERR, 0, 0,
	 "Whoa. doing() can't find player #%d on call by #%d\n",
	 target, executor);
}

FUNCTION(fun_poll)
{
  /* Gets the current poll */
  if (poll_msg[0] == '\0')
    strcpy(poll_msg, "Doing");

  safe_str(poll_msg, buff, bp);
}

FUNCTION(fun_pueblo)
{
  /* Return the status of the pueblo flag on the least idle descriptor we
   * find that matches the player's dbref. 
   */
  DESC *d;
  dbref target;
  DESC *match;

  target = lookup_player(args[0]);
  if (target == NOTHING) {
    target = match_result(executor, args[0], TYPE_PLAYER,
			  MAT_ABSOLUTE | MAT_PLAYER | MAT_ME);
  }
  /* non-connected players return error #-1 */
  if ((target == NOTHING) || !Connected(target)) {
    safe_str("#-1 NOT CONNECTED", buff, bp);
    return;
  }
  /* else walk the descriptor list looking for a match */
  match = NULL;
  DESC_ITER_CONN(d) {
    if ((d->player == target) &&
	(!Hidden(d) || Priv_Who(executor)) &&
	(!match || (d->last_time > match->last_time)))
      match = d;
  }
  if (match)
    safe_str(unparse_integer(match->pueblo), buff, bp);
  else
    safe_str("#-1 NOT CONNECTED", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_idlesecs)
{
  /* returns the number of seconds a player has been idle, using
   * their least idle connection
   */

  time_t now;
  DESC *d;
  DESC *match;
  dbref target;

  now = time((time_t *) 0);

  target = lookup_player(args[0]);
  if (target == NOTHING) {
    target = match_result(executor, args[0], TYPE_PLAYER,
			  MAT_ABSOLUTE | MAT_PLAYER | MAT_ME);
  }
  /* non-connected players return error -1 */
  if ((target == NOTHING) || !Connected(target)) {
    safe_str("-1", buff, bp);
    return;
  }
  /* else walk the descriptor list looking for a match */
  match = NULL;
  DESC_ITER_CONN(d) {
    if ((d->player == target) &&
	(!Hidden(d) || Priv_Who(executor)) &&
	(!match || (d->last_time > match->last_time)))
      match = d;
  }

  if (match)
    safe_str(unparse_integer(now - match->last_time), buff, bp);
  else
    safe_str("-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_conn)
{
  /* returns the number of seconds a player has been connected, using
   * their longest-connected descriptor
   */

  time_t now;
  DESC *d;
  DESC *match;
  dbref target;

  now = time((time_t *) 0);

  target = lookup_player(args[0]);
  if (target == NOTHING) {
    target = match_result(executor, args[0], TYPE_PLAYER,
			  MAT_ABSOLUTE | MAT_PLAYER);
  }
  /* non-connected players and dark wizards return error, -1 */
  if ((target == NOTHING) || !Connected(target)) {
    safe_str("-1", buff, bp);
    return;
  }
  /* else walk the descriptor list looking for a match */
  match = NULL;
  DESC_ITER_CONN(d) {
    if ((d->player == target) &&
	(!Hidden(d) || Priv_Who(executor)) &&
	(!match || (d->connected_at < match->connected_at)))
      match = d;
  }

  if (match)
    safe_str(unparse_integer(now - match->connected_at), buff, bp);
  else
    safe_str("-1", buff, bp);
}

/* ARGSUSED */
FUNCTION(fun_ports)
{
  /* returns a list of the network descriptors that a player is 
   * connected to (wizard-only)
   */

  dbref target;
  DESC *d;
  int first;

  if (!Hasprivs(executor)) {
    notify(executor, "Permission denied.");
    return;
  }
  target = lookup_player(args[0]);
  if (target == NOTHING) {
    target = match_result(executor, args[0], TYPE_PLAYER,
			  MAT_ABSOLUTE | MAT_PLAYER);
  }
  if ((target == NOTHING) || !Connected(target)) {
    return;
  }
  /* Walk descriptor chain. Don't worry about buffer length limits. */
  first = 1;
  DESC_ITER_CONN(d) {
    if (d->player == target) {
      if (first)
	first = 0;
      else
	safe_chr(' ', buff, bp);
      safe_str(unparse_integer(d->descriptor), buff, bp);
    }
  }
}

#ifdef TREKMUSH
/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_cmds)
{
  /* returns the number of commands a player has entered */
  
  DESC *d;
  dbref target;
  
  target = lookup_player(args[0]);
  if (target == NOTHING) {
    target = match_result(executor, args[0], TYPE_PLAYER,
                          MAT_ABSOLUTE | MAT_PLAYER);
  }
  /* non-connected players and dark wizards return error, -1 */
  if ((target == NOTHING) || !Connected(target)) {
    safe_str("-1", buff, bp);
    return;
  }
  /* else walk the descriptor list looking for a match */
  DESC_ITER_CONN(d) {
    if (d->player == target) {
      if (!Hidden(d) || Priv_Who(executor))
        safe_str(unparse_integer(d->cmds), buff, bp);
      else
        safe_str("-1", buff, bp);
      return;
    }
  }

  /* if we hit this point we are in trouble */
  safe_str("-1", buff, bp);
  do_log(LT_ERR, 0, 0, "Whoa. cmds() can't find player #%d on call by #%d\n",
         target, executor);
}

const char *time_format_3 (long int dt)
{
	register struct tm *delta;
	static char buf[64];
	
	if (dt < 0)
		dt = 0;
	delta = gmtime((time_t *) & dt);
	if (delta->tm_yday > 0) {
		sprintf(buf, "%2d %02d:%02d", delta->tm_yday, delta->tm_hour, delta->tm_min);
	} else
		sprintf(buf, "%02d:%02d:%02d", delta->tm_hour, delta->tm_min, delta->tm_sec);
	
	return buf;
}

void do_who(dbref player, dbref masterorg)
{
	ATTR *a;
	struct descriptor_data *d;
	dbref org;
	int count = 0, hcount = 0, hop = 0;
	time_t now = time(NULL);
	char tbuf[BUFFER_LEN];
	char nbuf[21];
	char obuf[41];
	char *tp = tbuf;
	char *np = nbuf;
	char *op = obuf;
	int delimit = 0;
	
	if (OrgObj(masterorg) && GoodObject(masterorg))
		delimit = 1;
	
#ifdef EXTENDED_ANSI

safe_str(tprintf("%s%s~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~%s
%s%sPlayer Name      OOC Affiliation Name                         Online   Idle    %s
%s%s~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~%s
", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_WHITE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL), tbuf, &tp);

#else /* EXTENDED_ANSI */

safe_str(tprintf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Player Name      OOC Affiliation Name                         Online   Idle    
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
"), tbuf, &tp);

#endif /* EXTENDED_ANSI */

	for(d = descriptor_list; (d); d=(d)->next)
	if ((d)->connected) {
		if (!GoodObject(d->player))
			continue;
		if (!Hasprivs(player))
			if (Dark(d->player)) {
				continue;
			} else if (Hidden(d)) {
				++hcount;
				++count;
				continue;
			}
		
		np = nbuf;
		safe_str(Name(d->player), nbuf, &np);
		if (Dark(d->player)) {
			safe_str(" (D)", nbuf, &np);
		} else if (Hidden(d)) {
			safe_str(" (H)", nbuf, &np);
			++hcount;
			++count;
		} else
			++count;
		*np = '\0';
		
		op = obuf;
		a = atr_get(d->player, "MASTERORG");
		if (a != NULL) {
			org = parse_dbref(uncompress(a->value));
			if (delimit && (org != masterorg))
				continue;
			if (OrgObj(org) && GoodObject(org)) {
				safe_str(Name(org), obuf, &op);
			} else
				safe_str("Unaffilliated                           ", obuf, &op);
		} else
			if (delimit) {
				continue;
			} else
				safe_str("Unaffilliated                           ", obuf, &op);
		*op = '\0';
		
		if (Ic(d->player)) {
			safe_str(tprintf("%-16.16s     %-40.40s ", nbuf, obuf), tbuf, &tp);
		} else
			safe_str(tprintf("%-16.16s OOC %-40.40s ", nbuf, obuf), tbuf, &tp);
		safe_str(tprintf("%8.8s ", time_format_3(now - d->connected_at)), tbuf, &tp);
		safe_str(tprintf("%8.8s", time_format_3(now - d->last_time)), tbuf, &tp);
		
		++hop;
		if (hop == 50) {
			*tp = '\0';
			notify(player, tbuf);
			tp = tbuf;
		} else
			safe_chr('\n', tbuf, &tp);
	}

#ifdef EXTENDED_ANSI
	safe_str(tprintf("%s%s~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~%s
", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL), tbuf, &tp);
#else /* EXTENDED_ANSI */
	safe_str(tprintf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
"), tbuf, &tp);
#endif /* EXTENDED_ANSI */
	safe_str(tprintf("%s current time is %s", MUDNAME, ctime(&now)), tbuf, &tp);
	switch (count) {
		case 0:
			safe_str(tprintf("There are no players connected (%d hidden).", hcount), tbuf, &tp);
			break;
		case 1:
			safe_str(tprintf("There is 1 player connected (%d hidden).", hcount), tbuf, &tp);
			break;
		default:
			safe_str(tprintf("There are %d players connected (%d hidden).", count, hcount), tbuf, &tp);
			break;
		}
	*tp = '\0';
	notify(player, tbuf);
	return;
}

/* ARGSUSED */
FUNCTION(fun_who)
{
	dbref masterorg;
	
	if (nargs == 1) {
		masterorg = lookup_org(args[0]);
		if (masterorg == NOTHING) {
			safe_str("#-1 NO SUCH MASTERORG", buff, bp);
		} else
			(void) do_who(enactor, masterorg);
	} else
		(void) do_who(enactor, -1);
	return;
}

/* ------------------------------------------------------------------------ */
#endif /* TREKMUSH */

void
hide_player(player, hide)
    dbref player;
    int hide;

  /* hide player? */
{
  DESC *d;
  char buf[BUFFER_LEN];

  if (!Connected(player))
    return;

  if (!Can_Hide(player)) {
    notify(player, "Permission denied.");
    return;
  }
  /* change status on WHO */

  if (Can_Hide(player)) {
    DESC_ITER_CONN(d) {
      if (d->player == player)
	d->hide = hide;
    }
  }
  if (hide)
    notify(player, "You no longer appear on the WHO list.");
  else
    notify(player, "You now appear on the WHO list.");

  /* change status on RWHO if necessary */
  if (USE_RWHO) {
    sprintf(buf, "%d@%s", player, MUDNAME);
    if (hide)
      rwhocli_userlogout(buf);
    else
      rwhocli_userlogin(buf, Name(player), time((time_t *) NULL));
  }
}

void
inactivity_check()
{
  DESC *d, *nextd;
  register struct tm *idle;
  time_t now;
  int check, hrs, mns;
  char buf[BUFFER_LEN];

  if (!INACTIVITY_LIMIT)
    return;

  check = hrs = mns = 0;
  now = time((time_t *) 0);

  for (mns = INACTIVITY_LIMIT; mns > 60; mns -= 60, hrs++) ;

  for (d = descriptor_list; d; d = nextd) {
    nextd = d->next;
    check = (now - d->last_time);
    idle = gmtime((time_t *) & check);

    if ((idle->tm_hour > hrs) ||
	((idle->tm_hour == hrs) && (idle->tm_min >= mns))) {

      if (!d->connected)
	shutdownsock(d);

      else if (!Can_Idle(d->player)) {

	notify(d->player, "\n*** Inactivity timeout ***\n");
	do_log(LT_CONN, 0, 0, "[%d/%s/%s] Logout by %s(#%d) <Inactivity Timeout>",
	       d->descriptor, d->addr, d->ip, Name(d->player), d->player);
	fflush(connlog_fp);
	boot_desc(d);

      } else if (Unfind(d->player)) {

	if ((Can_Hide(d->player)) && (!Hidden(d))) {
	  notify(d->player,
	    "\n*** Inactivity limit reached. You are now HIDDEN. ***\n");
	  d->hide = 1;
	  if (USE_RWHO) {
	    sprintf(buf, "%d@%s", d->player, MUDNAME);
	    rwhocli_userlogout(buf);
	  }
	}
      }
    }
  }
}


#ifdef CHAT_SYSTEM


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
	    Chanuser_Gag(u) ||
	     (IsPlayer(current) && !Connected(current)));
  } while (cont);
  nac->u = nu;
  return current;
}

#ifdef I_STDARG
void
channel_broadcast(CHAN *channel, dbref player, int flags, const char *fmt,...)
#else
void
channel_broadcast(channel, player, flags, va_alist)
    CHAN *channel;
    dbref player;
    int flags;			/* 0x1 = checkquiet, 0x2 = nospoof */
    va_dcl
#endif
{
  va_list args;
  char tbuf1[BUFFER_LEN * 2];	/* Safety margin as per tprintf */
#ifndef I_STDARG
  char *fmt;
#endif
  struct na_cpass nac;

  /* Make sure we can write to the channel before doing so */
  if (Channel_Disabled(channel))
    return;

#ifndef I_STDARG
  va_start(args);
  fmt = va_arg(args, char *);
#else
  va_start(args, fmt);
#endif

  (void) vsprintf(tbuf1, fmt, args);
  va_end(args);
  tbuf1[BUFFER_LEN - 1] = '\0';

  nac.u = ChanUsers(channel);
  nac.checkquiet = (flags & 0x1) ? 1 : 0;
  notify_anything(player, na_channel, &nac, (flags & 0x2) ? ns_esnotify : NULL, 0, tbuf1);
}

void
do_channel_who(player, chan)
    dbref player;
    CHAN *chan;
{
  DESC *d;
  char tbuf1[BUFFER_LEN];
  char *bp;
  CHANUSER *u;

  bp = tbuf1;
  DESC_ITER_CONN(d) {
    u = onchannel(d->player, chan);
    if (u &&(!Chanuser_Hide(u) || Priv_Who(player))) {
      if (bp != tbuf1)
	safe_str(", ", tbuf1, &bp);
      safe_str(Name(d->player), tbuf1, &bp);
      if (Chanuser_Gag(u))
	 safe_str(" (gagging)", tbuf1, &bp);
    }
  }
  *bp = '\0';

  if (!*tbuf1)
    notify(player, "There are no connected players on that channel.");
  else {
    notify(player, "Connected players on that channel are:");
    notify(player, tbuf1);
  }
}

/* ARGSUSED */
FUNCTION(fun_cwho)
{
  DESC *d;
  int first = 1;
  CHAN *chan = NULL;
  CHANUSER *u;
  int i;

  i = find_channel(args[0], &chan);
  switch (i) {
  case CMATCH_NONE:
    notify(executor, "No such channel.");
    return;
  case CMATCH_AMBIG:
    notify(executor, "I can't tell which channel you mean.");
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

  if (!Chan_Can_See(chan, Owner(executor)) &&
      !Chan_Can_See(chan, executor)) {
    safe_str("#-1 NO PERMISSIONS FOR CHANNEL", buff, bp);
    return;
  }
  DESC_ITER_CONN(d) {
    if ((u = onchannel(d->player, chan)) &&
	(!Chanuser_Hide(u) || Priv_Who(executor))) {
      if (first)
	first = 0;
      else
	safe_chr(' ', buff, bp);
      safe_str(unparse_dbref(d->player), buff, bp);
    }
  }
}

#endif				/* CHAT_SYSTEM */

int
hidden(player)
    dbref player;
{
  DESC *d;

  DESC_ITER_CONN(d) {
    if (d->player == player) {
      if (Hidden(d))
	return 1;
      else
	return 0;
    }
  }
  return 0;
}


#ifdef USE_MAILER
/* Return the mailp of the player closest in db# to player,
 * or NULL if there's nobody on-line
 */
struct mail *
desc_mail(player)
    dbref player;
{
  DESC *d;
  int i;
  int diff = db_top;
  static struct mail *mp;

  mp = NULL;
  DESC_ITER_CONN(d) {
    i = abs(d->player - player);
    if (i == 0)
      return d->mailp;
    if ((i < diff) && d->mailp) {
      diff = i;
      mp = d->mailp;
    }
  }
  return mp;
}

void
desc_mail_set(player, mp)
    dbref player;
    struct mail *mp;
{
  DESC *d;
  DESC_ITER_CONN(d) {
    if (d->player == player)
      d->mailp = mp;
  }
}

#endif				/* USE_MAILER */



#ifdef SUN_OS
/* SunOS's implementation of stdio breaks when you get a file descriptor
 * greater than 128. Brain damage, brain damage, brain damage!
 *
 * Our objective, therefore, is not to fix stdio, but to work around it,
 * so that performance degrades semi-gracefully when you are using a lot
 * of file descriptors.
 * Therefore, we'll save a file descriptor when we start up that is less
 * than 128, so that if we get a file descriptor that is >= 128, we can
 * use our own saved file descriptor instead. This is only one level of
 * defense; if you have more than 128 fd's in use, and you try two fopen's
 * before doing an fclose(), the second will fail.
 */

FILE *
fopen(file, mode)
    const char *file;
    const char *mode;
{
/* FILE *f; */
  int fd, rw, oflags = 0;
/* char tbchar; */

  rw = (mode[1] == '+') || (mode[1] && (mode[2] == '+'));

  switch (*mode) {
  case 'a':
    oflags = O_CREAT | (rw ? O_RDWR : O_WRONLY);
    break;
  case 'r':
    oflags = rw ? O_RDWR : O_RDONLY;
    break;
  case 'w':
    oflags = O_TRUNC | O_CREAT | (rw ? O_RDWR : O_WRONLY);
    break;
  default:
    return (NULL);
  }
/* SunOS fopen() doesn't use the 't' or 'b' flags. */


  fd = open(file, oflags, 0666);
  if (fd < 0)
    return NULL;

  /* My addition, to cope with SunOS brain damage! */
  if (fd >= 128) {
    close(fd);
    if ((extrafd < 128) && (extrafd >= 0)) {
      close(extrafd);
      fd = open(file, oflags, 0666);
      extrafd = -1;
    } else {
      return NULL;
    }
  }
  /* End addition. */

  return fdopen(fd, mode);
}


#undef fclose(x)
int
f_close(stream)
    FILE *stream;
{
  int fd = fileno(stream);
  /* if extrafd is bad, and the fd we're closing is good, recycle the
   * fd into extrafd.
   */
  fclose(stream);
  if (((extrafd < 0)) &&
      (fd >= 0) && (fd < 128)) {
    extrafd = open("/dev/null", O_RDWR);
    if (extrafd >= 128) {
      /* To our surprise, we didn't get a usable fd. */
      close(extrafd);
      extrafd = -1;
    }
  }
  return 0;
}

#define fclose(x) f_close(x)

#endif				/* SUN_OS */

static int
how_many_fds()
{
  /* Determine how many open file descriptors we're allowed
   * In order, we'll try:
   * 1. sysconf(_SC_OPEN_MAX) - POSIX.1
   * 2. OPEN_MAX constant - POSIX.1 limits.h
   * 3. getdtablesize - BSD (which Config maps to ulimit or NOFILE if needed)
   */
  static int open_max = 0;

  if (open_max)
    return open_max;

#ifdef WIN32
  open_max = 60;
  return open_max;
#else				/* ! WIN32 */
#ifdef HAS_SYSCONF
  errno = 0;
  if ((open_max = sysconf(_SC_OPEN_MAX)) < 0) {
    if (errno == 0)		/* Value was indeterminate */
      open_max = 0;
  }
  if (open_max)
    return open_max;
#endif
#ifdef OPEN_MAX
  open_max = OPEN_MAX;
  return open_max;
#endif
  /* Caching getdtablesize is dangerous, since it's affected by
   * getrlimit, so we don't.
   */
  open_max = 0;
  return getdtablesize();
#endif				/* WIN 32 */
}

/*
 * ---------------------------------------------------------------------------
 * * dump_reboot_db: Dumps descriptor_list to REBOOTFILE
 */
void
dump_reboot_db()
{
  FILE *f;
  DESC *d;

  f = fopen(REBOOTFILE, "w");

  /* This shouldn't happen */
  if (!f) {
    flag_broadcast(0, 0, "GAME: Error writing reboot database!");
    exit(0);
  }
  putref(f, sock);
  putref(f, maxd);

  /* First, iterate through all descriptors to get to the end 
   * we do this so the descriptor_list isn't reversed on reboot 
   */
  for (d = descriptor_list; d && d->next; d = d->next) ;

  /* Second, we iterate backwards from the end of descriptor_list
   * which is now in the d variable.
   */
  for (; d != NULL; d = d->prev) {

    putref(f, d->descriptor);
    putref(f, d->connected_at);
    putref(f, d->hide);
    putref(f, d->cmds);
    if (GoodObject(d->player))
      putref(f, d->player);
    else
      putref(f, -1);
    putref(f, d->last_time);
    if (d->output_prefix)
      putstring(f, d->output_prefix);
    else
      putstring(f, "__NONE__");
    if (d->output_suffix)
      putstring(f, d->output_suffix);
    else
      putstring(f, "__NONE__");
    putstring(f, d->addr);
    putstring(f, d->ip);
    putstring(f, d->doing);
    putref(f, d->pueblo);
  }				/* for loop */

  putref(f, 0);
  fclose(f);
}

void
load_reboot_db()
{
  FILE *f;
  DESC *d = NULL;
  int val;
  char *temp;

  f = fopen(REBOOTFILE, "r");

  if (!f) {
    restarting = 0;
    return;
  }
  restarting = 1;

  sock = getref(f);
  maxd = getref(f);

  while ((val = getref(f)) != 0) {
    ndescriptors++;
    d = (DESC *) mush_malloc(sizeof(DESC), "restart.descriptor");
    d->descriptor = val;
    d->connected_at = getref(f);
    d->hide = getref(f);
    d->cmds = getref(f);
    d->player = getref(f);
    d->last_time = getref(f);
    d->connected = GoodObject(d->player) ? 1 : 0;
    temp = (char *) getstring_noalloc(f);

    d->output_prefix = NULL;
    if (strcmp(temp, "__NONE__"))
      set_userstring(&d->output_prefix, temp);

    temp = (char *) getstring_noalloc(f);
    d->output_suffix = NULL;
    if (strcmp(temp, "__NONE__"))
      set_userstring(&d->output_suffix, temp);

    strcpy(d->addr, getstring_noalloc(f));
    strcpy(d->ip, getstring_noalloc(f));
    strcpy(d->doing, getstring_noalloc(f));

    d->pueblo = getref(f);

    d->output_size = 0;
    d->output.head = 0;
    d->output.tail = &d->output.head;
    d->input.head = 0;
    d->input.tail = &d->input.head;
    d->raw_input = NULL;
    d->raw_input_at = NULL;
    d->quota = options.starting_quota;
#ifdef USE_MAILER
    d->mailp = NULL;
#endif

    if (descriptor_list)
      descriptor_list->prev = d;
    d->next = descriptor_list;
    d->prev = NULL;
    descriptor_list = d;

    if (d->connected && d->player && GoodObject(d->player) &&
	IsPlayer(d->player))
      Toggles(d->player) |= PLAYER_CONNECT;
    else if ((!d->player || !GoodObject(d->player)) && d->connected) {
      d->connected = 0;
      d->player = 0;
    }
  }				/* while loop */

#ifdef USE_MAILER
  DESC_ITER_CONN(d) {
    d->mailp = find_exact_starting_point(d->player);
  }
#endif

  fclose(f);
  remove(REBOOTFILE);
  flag_broadcast(0, 0, "GAME: Reboot finished.");
}


#ifdef NT_TCP

/* --------------------------------------------------------------------------- */
/* Thread to listen on MUD port - for Windows NT */
/* --------------------------------------------------------------------------- */

void __cdecl 
MUDListenThread(void *pVoid)
{
  SOCKET MUDListenSocket = (SOCKET) pVoid;

  SOCKET socketClient;
  SOCKADDR_IN SockAddr;
  int nLen;
/*  int     i; */
  BOOL b;

  char tbuf1[BUFFER_LEN];
  char tbuf2[BUFFER_LEN];
  char *bp;
  DESC *d;

  printf("Starting MUD listening thread ...\n");

  /* Loop forever accepting connections */
  while (TRUE) {

    /* Block on accept() */
    nLen = sizeof(SOCKADDR_IN);
    socketClient = accept(MUDListenSocket,
			  (LPSOCKADDR) & SockAddr,
			  &nLen);

    if (socketClient == INVALID_SOCKET) {
      /* parent thread closes the listening socket */
      /* when it wants this thread to stop. */
      break;
    }
    /* We have a connection */
    /*  */

    bp = tbuf2;
    safe_str((char *) ip_convert(SockAddr.sin_addr), tbuf2, &bp);
    *bp = '\0';
    bp = tbuf1;
    safe_str((char *) hostname_convert(SockAddr.sin_addr), tbuf1, &bp);
    *bp = '\0';
    if (Forbidden_Site(tbuf1)) {
      do_log(LT_CONN, 0, 0, "[%d/%s] Refused connection (remote port %d)",
	     socketClient, tbuf1, ntohs(SockAddr.sin_port));
      shutdown(socketClient, 2);
      closesocket(socketClient);
      continue;
    }
    do_log(LT_CONN, 0, 0, "[%d/%s] Connection opened.", socketClient, tbuf1);

    d = initializesock(socketClient, tbuf1, tbuf2);

    printf("[%d/%s] Connection opened.\n", socketClient, tbuf1);

/* add this socket to the IO completion port */

    CompletionPort = CreateIoCompletionPort((HANDLE) socketClient,
					    CompletionPort,
					    (DWORD) d,
					    1);

    if (!CompletionPort) {
      char sMessage[200];
      GetErrorMessage(GetLastError(), sMessage, sizeof sMessage);
      printf("Error %ld (%s) on CreateIoCompletionPort for socket %ld\n",
	     GetLastError(),
	     sMessage,
	     socketClient);
      shutdownsock(d);
      continue;
    }
/* welcome the user - we can't do this until the completion port is created */

    welcome_user(d);

/* do the first read */

    b = ReadFile((HANDLE) socketClient,
		 d->input_buffer,
		 sizeof(d->input_buffer) - 1,
		 NULL,
		 &d->InboundOverlapped);

    if (!b && GetLastError() != ERROR_IO_PENDING) {
      shutdownsock(d);
      continue;
    }
  }				/* end of while loop */

  printf("End of MUD listening thread ...\n");

}				/* end of MUDListenThread */


/*
   This is called from within shovechars when it needs to see if any IOs have
   completed for the Windows NT version.

   The 4 sorts of IO completions are:

   1. Outstanding read completing (there should always be an outstanding read)
   2. Outstanding write completing
   3. A special "shutdown" message to tell us to shutdown the socket
   4. A special "aborted" message to tell us the socket has shut down, and we
   can now free the descriptor.

   The latter 2 are posted by the application by PostQueuedCompletionStatus
   when it is necessary to signal these "events".

   The reason for posting the special messages is to shut down sockets in an
   orderly way.

 */

void 
ProcessWindowsTCP(void)
{
  LPOVERLAPPED lpo;
  DWORD nbytes;
  BOOL b;
  DWORD nError;
  DESC *d;
  time_t now;

  /* pull out the next completed IO, waiting half-a-second for it if necessary */

  b = GetQueuedCompletionStatus(CompletionPort,
				&nbytes,
				(LPDWORD) & d,
				&lpo,
				500);	/* half-second timeout */

  nError = GetLastError();

  /* ignore timeouts and cancelled IOs */
  if (!b &&
      (nError == WAIT_TIMEOUT ||
       nError == ERROR_OPERATION_ABORTED))
    return;


  /* shutdown this descriptor if wanted */
  if (lpo == &lpo_shutdown) {
    shutdownsock(d);		/* shut him down */
    return;
  }
  /* *now* we can free the descriptor (posted by shutdownsock) */
  if (lpo == &lpo_aborted) {
    freeqs(d);
    mush_free((Malloc_t) d, "descriptor");
    return;
  }
  /* if address of descriptor is bad - don't try using it */

  if (!IsValidAddress(d, sizeof(DESC), TRUE)) {
    printf("Invalid descriptor %08lX on GetQueuedCompletionStatus\n",
	   d);
    return;
  }
  /* bad IO - shut down this client */

  if (!b) {
    shutdownsock(d);
    return;
  }
  /* see if read completed */

  if (lpo == &d->InboundOverlapped && !d->bConnectionDropped) {

    /* zero length IO completion means connection dropped by client */

    if (nbytes == 0) {
      shutdownsock(d);
      return;
    }
    now = time((time_t *) 0);
    d->last_time = now;

    /* process the player's input */

    process_input_helper(d, d->input_buffer, nbytes);

    /* now fire off another read */

    b = ReadFile((HANDLE) d->descriptor,
		 d->input_buffer,
		 sizeof(d->input_buffer) - 1,
		 NULL,
		 &d->InboundOverlapped);

    if (!b && GetLastError() != ERROR_IO_PENDING) {
      d->bConnectionDropped = TRUE;	/* do no more reads */
      /* post a notification that the descriptor should be shutdown */
      if (!PostQueuedCompletionStatus(CompletionPort, 0, (DWORD) d, &lpo_shutdown)) {
	char sMessage[200];
	DWORD nError = GetLastError();
	GetErrorMessage(nError, sMessage, sizeof sMessage);
	printf("Error %ld (%s) on PostQueuedCompletionStatus in ProcessWindowsTCP (read)\n",
	       nError,
	       sMessage);
	shutdownsock(d);
      }
      return;
    }
    return;
  }				/* end of read completing */
  /* see if write completed */
  if (lpo == &d->OutboundOverlapped && !d->bConnectionDropped) {
    struct text_block **qp, *cur;
    DWORD nBytes;

    qp = &d->output.head;

    if ((cur = *qp) == NULL)
      d->bWritePending = FALSE;
    else {			/* here if there is more to write */

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

      b = WriteFile((HANDLE) d->descriptor,
		    d->output_buffer,
		    nBytes,
		    NULL,
		    &d->OutboundOverlapped);

      d->bWritePending = FALSE;

      if (!b)
	if (GetLastError() == ERROR_IO_PENDING)
	  d->bWritePending = TRUE;
	else {
	  d->bConnectionDropped = TRUE;		/* do no more reads */
	  /* post a notification that the descriptor should be shutdown */
	  if (!PostQueuedCompletionStatus(CompletionPort, 0, (DWORD) d, &lpo_shutdown)) {
	    char sMessage[200];
	    DWORD nError = GetLastError();
	    GetErrorMessage(nError, sMessage, sizeof sMessage);
	    printf("Error %ld (%s) on PostQueuedCompletionStatus in ProcessWindowsTCP (write)\n",
		   nError,
		   sMessage);
	    shutdownsock(d);
	  }
	  return;
	}
    }				/* end of more to write */

  }				/* end of write completing */
}
#endif
