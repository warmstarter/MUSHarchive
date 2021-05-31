#ifndef MUSH_TYPES_H
#define MUSH_TYPES_H
#include "copyrite.h"
#include "options.h"
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#endif
#ifdef HAS_OPENSSL
#include <openssl/ssl.h>
#endif

/* Function number type */
typedef double NVAL;

/* Dbref type */
typedef int dbref;

/* special dbref's */
#define NOTHING (-1)		/* null dbref */
#define AMBIGUOUS (-2)		/* multiple possibilities, for matchers */
#define HOME (-3)		/* virtual room, represents mover's home */
#define ANY_OWNER (-2)		/* For lstats and @stat */


#define INTERACT_SEE 0x1
#define INTERACT_HEAR 0x2
#define INTERACT_MATCH 0x4

typedef unsigned char *object_flag_type;

/* Boolexps and locks */
struct boolexp;
typedef const char *lock_type;
typedef struct lock_list lock_list;

typedef struct pe_info PE_Info;
typedef struct debug_info Debug_Info;
/** process_expression() info
 * This type is used by process_expression().  In all but parse.c,
 * this should be left as an incompletely-specified type, making it
 * impossible to declare anything but pointers to it.
 *
 * Unfortunately, we need to know what it is in funlist.c, too,
 * to prevent denial-of-service attacks.  ARGH!  Don't look at
 * this struct unless you _really_ want to get your hands dirty.
 */
struct pe_info {
  int fun_invocations;		/**< Invocation count */
  int fun_depth;		/**< Recursion count */
  int nest_depth;		/**< Depth of function nesting, for DEBUG */
  int call_depth;		/**< Function call counter */
  Debug_Info *debug_strings;	/**< DEBUG infromation */
};


/* new attribute foo */
typedef struct attr ATTR;
typedef ATTR ALIST;

/** A text block
 */
struct text_block {
  int nchars;			/**< Number of characters in the block */
  struct text_block *nxt;	/**< Pointer to next block in queue */
  unsigned char *start;		/**< Start of text */
  unsigned char *buf;		/**< Current position in text */
};
/** A queue of text blocks.
 */
struct text_queue {
  struct text_block *head;	/**< Pointer to the head of the queue */
  struct text_block **tail;	/**< Pointer to pointer to tail of the queue */
};

/* Descriptor foo */
#define DOING_LEN 40
typedef struct descriptor_data DESC;
/** A player descriptor's data.
 * This structure associates a connection's socket (file descriptor)
 * with a lot of other relevant information.
 */
struct descriptor_data {
  int descriptor;	/**< Connection socket (fd) */
  int connected;	/**< Connection status */
  char addr[101];	/**< Hostname of connection source */
  char ip[101];		/**< IP address of connection source */
  dbref player;		/**< Dbref of player associated with connection */
  unsigned char *output_prefix;	/**< Text to show before output */
  unsigned char *output_suffix;	/**< Text to show after output */
  int output_size;		/**< Size of output left to send */
  struct text_queue output;	/**< Output text queue */
  struct text_queue input;	/**< Input text queue */
  unsigned char *raw_input;	/**< Pointer to start of next raw input */
  unsigned char *raw_input_at;	/**< Pointer to position in raw input */
  long connected_at;	/**< Time of connection */
  long last_time;	/**< Time of last activity */
  int quota;		/**< Quota of commands allowed */
  int cmds;		/**< Number of commands sent */
  int hide;		/**< Hide status */
  char doing[DOING_LEN];	/**< Player's doing string */
#ifdef NT_TCP
  /* these are for the Windows NT TCP/IO */
  char input_buffer[512];	/**< WinNT: buffer for reading */
  char output_buffer[512];	/**< WinNT: buffer for writing */
  OVERLAPPED InboundOverlapped;	/**< WinNT: for asynchronous reading */
  OVERLAPPED OutboundOverlapped;	/**< WinNT: for asynchronous writing */
  BOOL bWritePending;		/**< WinNT: true if in process of writing */
  BOOL bConnectionDropped;	/**< WinNT: true if we cannot send to player */
  BOOL bConnectionShutdown;	/**< WinNT: true if connection has been shutdown */
#endif
  struct descriptor_data *next;	/**< Next descriptor in linked list */
  struct descriptor_data *prev;	/**< Previous descriptor in linked list */
#ifdef USE_MAILER
  struct mail *mailp;	/**< Pointer to start of player's mail chain */
#endif
  int conn_flags;	/**< Flags of connection (telnet status, etc.) */
  unsigned long input_chars;	/**< Characters received */
  unsigned long output_chars;	/**< Characters sent */
  int width;			/**< Screen width */
  int height;			/**< Screen height */
#ifdef HAS_OPENSSL
  BIO *bio;
#endif
};

/* max length of command argument to process_command */
#define MAX_COMMAND_LEN 4096
#define BUFFER_LEN ((MAX_COMMAND_LEN)*2)

#ifdef CHAT_SYSTEM
/* Channel stuff */
typedef struct chanuser CHANUSER;
typedef struct chanlist CHANLIST;
typedef struct channel CHAN;
#endif

#endif
