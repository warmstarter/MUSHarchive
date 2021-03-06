/* conf.h */

#ifndef __CONF_H
#define __CONF_H

#include "copyrite.h"
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#include "options.h"
#include "mushtype.h"


/* limit on player name length */
#define PLAYER_NAME_LIMIT (options.player_name_len)
/* limit on object name length */
#define OBJECT_NAME_LIMIT 256
/* Limit on attribute name length */
#define ATTRIBUTE_NAME_LIMIT 1024
/* Loose limit on command/function name length */
#define COMMAND_NAME_LIMIT 64

/* magic cookies */
#define LOOKUP_TOKEN '*'
#define NUMBER_TOKEN '#'
#define ARG_DELIMITER '='

/* magic command cookies */
#define SAY_TOKEN '"'
#define POSE_TOKEN ':'
#define SEMI_POSE_TOKEN ';'
#define EMIT_TOKEN '\\'
#define CHAT_TOKEN '+'

/* delimiter for lists of exit aliases */
#define EXIT_DELIMITER ';'

#define QUIT_COMMAND "QUIT"
#define WHO_COMMAND "WHO"
#define LOGOUT_COMMAND "LOGOUT"
#define INFO_COMMAND "INFO"
#define INFO_VERSION "1.1"
#define DOING_COMMAND "DOING"
#define SESSION_COMMAND "SESSION"
#define IDLE_COMMAND "IDLE"

#define PREFIX_COMMAND "OUTPUTPREFIX"
#define SUFFIX_COMMAND "OUTPUTSUFFIX"
#define PUEBLO_COMMAND "PUEBLOCLIENT "

/* These CAN be modified, but it's heavily NOT suggested */
#define PUEBLO_SEND "</xch_mudtext><img xch_mode=purehtml><xch_page clear=text><body bgcolor=#000000 fgcolor=#C0C0C0>\n"
#define PUEBLO_HELLO "This world is Pueblo 1.10 Enhanced.\r\n"


#define MAX_OUTPUT 16384
/* How much output buffer space must be left before we flush the
 * buffer? Reportedly, using '0' fixes problems with Win32 port,
 * and may be more efficient in network use. Using (MAX_OUTPUT / 2)
 * is how it's been done in the past. You get to pick.
 */
#define SPILLOVER_THRESHOLD     0
/* #define SPILLOVER_THRESHOLD  (MAX_OUTPUT / 2) */
#define COMMAND_TIME_MSEC 1000	/* time slice length in milliseconds */
#define COMMAND_BURST_SIZE 100	/* commands allowed per user in a burst */
#define COMMANDS_PER_TIME 1	/* commands per time slice after burst */


/* Set this somewhere near the recursion limit */
#define MAX_ITERS 100

/* From conf.c */
extern void do_config_list(dbref player, const char *type, int lc);

typedef struct options_table OPTTAB;

/** Runtime configuration options.
 * This large structure stores all of the runtime configuration options
 * that are typically set in mush.cnf.
 */
struct options_table {
  char mud_name[128];	/**< The name of the mush */
  int port;		/**< The port to listen for connections */
  int sslport;		/**< Future: The port to listen for SSL connections */
  char input_db[256];	/**< Name of the input database file */
  char output_db[256];	/**< Name of the output database file */
  char crash_db[256];	/**< Name of the panic database file */
  char mail_db[256];	/**< Name of the mail database file */
  dbref player_start;	/**< The room in which new players are created */
  dbref master_room;	/**< The master room for global commands/exits */
  int idle_timeout;	/**< Maximum idle time allowed, in minutes */
  int unconnected_idle_timeout;	/**< Maximum idle time for connections without dbrefs, in minutes */
  int dump_interval;	/**< Interval between database dumps, in seconds */
  char dump_message[256]; /**< Message shown at start of nonforking dump */
  char dump_complete[256]; /**< Message shown at end of nonforking dump */
  time_t dump_counter;	/**< Time since last dump */
  int ident_timeout;	/**< Timeout for ident lookups */
  int max_logins;	/**< Maximum total logins allowed at once */
  int max_guests;	/**< Maximum guests logins allowed at once */
  int whisper_loudness;	/**< % chance that a noisy whisper is overheard */
  int blind_page;	/**< Does page default to page/blind? */
  int page_aliases;	/**< Does page include aliases? */
  int paycheck;		/**< Number of pennies awarded each day of connection */
  int guest_paycheck;	/**< Paycheck for guest connections */
  int starting_money;	/**< Number of pennies for newly created players */
  int starting_quota;	/**< Object quota for newly created players */
  int player_queue_limit; /**< Maximum commands a player can queue at once */
  int queue_chunk;	/**< Number of commands run from queue when no input from sockets is waiting */
  int active_q_chunk;	/**< Number of commands run from queue when input from sockets is waiting */
  int func_nest_lim;	/**< Maximum function recursion depth */
  int func_invk_lim;	/**< Maximum number of function invocations */
  int call_lim;		/**< Maximum parser calls allowed in a queue cycle */
  char log_wipe_passwd[256];	/**< Password for logwipe command */
  char money_singular[32];	/**< Currency unit name, singular */
  char money_plural[32];	/**< Currency unit name, plural */
  char compressprog[256];	/**< Program to compress database dumps */
  char uncompressprog[256];	/**< Program to uncompress database dumps */
  char compresssuff[256];	/**< Suffix for compressed dump files */
#ifdef CHAT_SYSTEM
  char chatdb[256];		/**< Name of the chat database file */
  int max_player_chans;		/**< Number of channels a player can create */
  int max_channels;		/**< Total maximum allowed channels */
  int chan_cost;		/**< Cost to create a channel */
#endif
  char connect_file[2][256];	/**< Names of text and html connection files */
  char motd_file[2][256];	/**< Names of text and html motd files */
  char wizmotd_file[2][256];	/**< Names of text and html wizmotd files */
  char newuser_file[2][256];	/**< Names of text and html new user files */
  char register_file[2][256];	/**< Names of text and html registration files */
  char quit_file[2][256];	/**< Names of text and html disconnection files */
  char down_file[2][256];	/**< Names of text and html server down files */
  char full_file[2][256];	/**< Names of text and html server full files */
  char guest_file[2][256];	/**< Names of text and html guest files */
  int log_commands;	/**< Should we log all commands? */
  int log_huhs;		/**< Should we log commands that produce a Huh? */
  int log_forces;	/**< Should we log force commands? */
  int log_walls;	/**< Should we log wall commands? */
  int support_pueblo;	/**< Should the MUSH send Pueblo tags? */
  int login_allow;	/**< Are mortals allowed to log in? */
  int guest_allow;	/**< Are guests allowed to log in? */
  int create_allow;	/**< Can new players be created? */
  int reverse_shs;	/**< Should the SHS routines assume little-endian byte order? */
  char player_flags[BUFFER_LEN];	/**< Space-separated list of flags to set on newly created players. */
  char room_flags[BUFFER_LEN];		/**< Space-separated list of flags to set on newly created rooms. */
  char exit_flags[BUFFER_LEN];		/**< Space-separated list of flags to set on newly created exits. */
  char thing_flags[BUFFER_LEN];		/**< Space-separated list of flags to set on newly created things. */
  int warn_interval;	/**< Interval between warning checks */
  time_t warn_counter;	/**< Time since last warning check */
  dbref base_room;	/**< Room which floating checks consider as the base */
  int use_dns;		/**< Should we use DNS lookups? */
  int haspower_restricted;	/**< Should haspower() be restricted to see_all? */
  int safer_ufun;	/**< Should we require security for ufun calls? */
  char dump_warning_1min[256];	/**< 1 minute nonforking dump warning message */
  char dump_warning_5min[256];	/**< 5 minute nonforking dump warning message */
  int noisy_whisper;	/**< Does whisper default to whisper/noisy? */
  int possessive_get;	/**< Can possessive get be used? */
  int possessive_get_d;	/**< Can possessive get be used on disconnected players? */
  int really_safe;	/**< Does the SAFE flag protect objects from nuke */
  int destroy_possessions;	/**< Are the possessions of a nuked player nuked? */
  int null_eq_zero;	/**< Is null string treated as 0 in math functions? */
  int tiny_booleans;	/**< Do strings and db#'s evaluate as false, like TinyMUSH? */
  int tiny_trim_fun;	/**< Does the trim function take arguments in TinyMUSH order? */
  int tiny_math;	/**< Can you use strings in math functions, like TinyMUSH? */
  int adestroy;		/**< Is the adestroy attribute available? */
  int amail;		/**< Is the amail attribute available? */
  int mail_limit;	/**< Maximum number of mail messages per player */
  int player_listen;	/**< Does listen work on players? */
  int player_ahear;	/**< Does ahear work on players? */
  int startups;		/**< Is startup run on startups? */
  int room_connects;	/**< Do players trigger aconnect/adisconnect on their location? */
  int ansi_names;	/**< Are object names shown in bold? */
  int comma_exit_list;	/**< Should exit lists be itemized? */
  int count_all;	/**< Are hidden players included in total player counts? */
  int exits_connect_rooms;	/**< Does the presence of an exit make a room connected? */
  int zone_control;	/**< Are only ZMPs allowed to determine zone-based control? */
  int link_to_object;	/**< Can exits be linked to objects? */
  int owner_queues;	/**< Are queues tracked by owner or individual object? */
  int wiz_noaenter;	/**< Do DARK wizards trigger aenters? */
  int use_ident;	/**< Should we do ident checks on connections? */
  char ip_addr[64];	/**< What ip address should the server bind to? */
  int player_name_spaces;	/**< Can players have multiword names? */
  int forking_dump;	/**< Should we fork to dump? */
  int restrict_building;	/**< Is the builder power required to build? */
  int free_objects;	/**< If builder power is required, can you create without it? */
  int flags_on_examine;	/**< Are object flags shown when it's examined? */
  int ex_public_attribs;	/**< Are visual attributes shown on examine? */
  int full_invis;	/**< Are DARK wizards anonymous? */
  int silent_pemit;	/**< Does pemit default to pemit/silent? */
  dbref max_dbref;	/**< Maximum allowable database size */
  int chat_strip_quote;	/**< Should we strip initial quotes in chat? */
  char wizwall_prefix[256];	/**< Prefix for wizwall announcements */
  char rwall_prefix[256];	/**< Prefix for rwall announcements */
  char wall_prefix[256];	/**< Prefix for wall announcements */
  char access_file[256];	/**< Name of file of access control rules */
  char names_file[256];	/**< Name of file of forbidden player names */
  int object_cost;	/**< Cost to create an object */
  int exit_cost;	/**< Cost to create an exit */
  int link_cost;	/**< Cost to link an exit */
  int room_cost;	/**< Cost to dig a room */
  int queue_cost;	/**< Deposit to queue a command */
  int quota_cost;	/**< Number of objects per quota unit */
  int find_cost;	/**< Cost to create an object */
  int page_cost;	/**< Cost to create an object */
  int kill_default_cost;	/**< Default cost to use 'kill' */
  int kill_min_cost;	/**< Minimum cost to use 'kill' */
  int kill_bonus;	/**< Percentage of cost paid to victim of 'kill' */
  int queue_loss;	/**< 1/queue_loss chance of a command costing a penny */
  int max_pennies;	/**< Maximum pennies a player can have */
  int max_guest_pennies;	/**< Maximum pennies a guest can have */
  int max_depth;	/**< Maximum container depth */
  int max_parents;	/**< Maximum parent depth */
  int purge_interval;	/**< Time between automatic purges */
  time_t purge_counter;	/**< Time since last automatic purge */
  int dbck_interval;	/**< Time between automatic dbcks */
  time_t dbck_counter;	/**< Time since last automatic dbck */
  int max_attrcount;	/**< Maximum number of attributes per object */
  int float_precision;	/**< Precision of floating point display */
  int newline_one_char;	/**< Should a newline be counted as 1 character or 2? */
  int player_name_len;	/**< Maximum length of player names */
  int queue_entry_cpu_time;	/**< Maximum cpu time allowed per queue entry */
  int ascii_names;	/**< Are object names restricted to ascii characters? */
  int max_global_fns;	/**< Maximum number of functions */
};

extern OPTTAB options;


#define NUMQ    36

/* Config group viewing permissions */
#define CGP_GOD         0x1
#define CGP_WIZARD      0x3
#define CGP_ADMIN       0x7
#define Can_View_Config_Group(p,g) \
        (!(g->viewperms) || (God(p) && (g->viewperms & CGP_GOD)) || \
         (Wizard(p) && (g->viewperms & CGP_WIZARD)) || \
         (Hasprivs(p) && (g->viewperms & CGP_ADMIN)))


#define DUMP_INTERVAL       (options.dump_interval)
#define DUMP_NOFORK_MESSAGE  (options.dump_message)
#define DUMP_NOFORK_COMPLETE (options.dump_complete)
#define INACTIVITY_LIMIT    (options.idle_timeout)
#define UNCONNECTED_LIMIT    (options.unconnected_idle_timeout)

#define MAX_LOGINS      (options.max_logins)
#define MAX_GUESTS      (options.max_guests)

/* dbrefs are in the conf file */

#define TINYPORT         (options.port)
#define SSLPORT          (options.sslport)
#define PLAYER_START     (options.player_start)
#define MASTER_ROOM      (options.master_room)
#define MONEY            (options.money_singular)
#define MONIES           (options.money_plural)
#define WHISPER_LOUDNESS        (options.whisper_loudness)
#define BLIND_PAGE      (options.blind_page)
#define PAGE_ALIASES    (options.page_aliases)

#define START_BONUS      (options.starting_money)
#define PAY_CHECK        (options.paycheck)
#define GUEST_PAY_CHECK        (options.guest_paycheck)
#define START_QUOTA      (options.starting_quota)
#define LOG_WIPE_PASSWD  (options.log_wipe_passwd)
#define SUPPORT_PUEBLO   (options.support_pueblo)

#define QUEUE_QUOTA      (options.player_queue_limit)

#define MUDNAME          (options.mud_name)
#define DEF_DB_IN        (options.input_db)
#define DEF_DB_OUT       (options.output_db)

#define BASE_ROOM        (options.base_room)

#define PURGE_INTERVAL   (options.purge_interval)
#define DBCK_INTERVAL    (options.dbck_interval)
#define MAX_PARENTS (options.max_parents)
#define MAX_DEPTH (options.max_depth)
#define MAX_PENNIES (options.max_pennies)
#define MAX_GUEST_PENNIES (options.max_guest_pennies)
#define DBTOP_MAX (options.max_dbref)
#define QUEUE_LOSS (options.queue_loss)
#define MAX_ARG 63
#define KILL_BONUS (options.kill_bonus)
#define KILL_MIN_COST (options.kill_min_cost)
#define KILL_BASE_COST (options.kill_default_cost)
#define FIND_COST (options.find_cost)
#define PAGE_COST (options.page_cost)
#define QUOTA_COST (options.quota_cost)
#define QUEUE_COST (options.queue_cost)
#define ROOM_COST (options.room_cost)
#define LINK_COST (options.link_cost)
#define EXIT_COST (options.exit_cost)
#define OBJECT_COST (options.object_cost)
#define GOD ((dbref) 1)
#define ACCESS_FILE (options.access_file)
#define NAMES_FILE (options.names_file)
#define SILENT_PEMIT (options.silent_pemit)
#define PLAYER_LISTEN (options.player_listen)
#define PLAYER_AHEAR (options.player_ahear)
#define STARTUPS (options.startups)
#define FULL_INVIS (options.full_invis)
#define EX_PUBLIC_ATTRIBS (options.ex_public_attribs)
#define FLAGS_ON_EXAMINE (options.flags_on_examine)
#define FREE_OBJECTS (options.free_objects)
#define RESTRICTED_BUILDING (options.restrict_building)
#define NO_FORK (!options.forking_dump)
#define PLAYER_NAME_SPACES (options.player_name_spaces)
#define HASPOWER_RESTRICTED (options.haspower_restricted)
#define SAFER_UFUN (options.safer_ufun)
#define NOISY_WHISPER (options.noisy_whisper)
#define POSSESSIVE_GET (options.possessive_get)
#define POSSGET_ON_DISCONNECTED (options.possessive_get_d)
#define REALLY_SAFE (options.really_safe)
#define DESTROY_POSSESSIONS (options.destroy_possessions)
#define NULL_EQ_ZERO (options.null_eq_zero)
#define TINY_BOOLEANS (options.tiny_booleans)
#define TINY_TRIM_FUN (options.tiny_trim_fun)
#define ADESTROY_ATTR (options.adestroy)
#define AMAIL_ATTR (options.amail)
#define MAIL_LIMIT (options.mail_limit)
#define ROOM_CONNECTS (options.room_connects)
#define ANSI_NAMES (options.ansi_names)
#define COMMA_EXIT_LIST (options.comma_exit_list)
#define COUNT_ALL (options.count_all)
#define EXITS_CONNECT_ROOMS (options.exits_connect_rooms)
#define ZONE_CONTROL_ZMP (options.zone_control)
#define WIZWALL_PREFIX (options.wizwall_prefix)
#define RWALL_PREFIX (options.rwall_prefix)
#define CHAT_STRIP_QUOTE (options.chat_strip_quote)
#define WALL_PREFIX (options.wall_prefix)
#define NO_LINK_TO_OBJECT (!options.link_to_object)
#define QUEUE_PER_OWNER (options.owner_queues)
#define WIZ_NOAENTER (options.wiz_noaenter)
#define USE_IDENT (options.use_ident)
#define IDENT_TIMEOUT (options.ident_timeout)
#define USE_DNS (options.use_dns)
#define MUSH_IP_ADDR (options.ip_addr)
#define MAX_ATTRCOUNT (options.max_attrcount)
#define FLOAT_PRECISION (options.float_precision)
#define RECURSION_LIMIT (options.func_nest_lim)
#define FUNCTION_LIMIT (options.func_invk_lim)
#define CALL_LIMIT (options.call_lim)
#define TINY_MATH (options.tiny_math)
#define NEWLINE_ONE_CHAR (options.newline_one_char)
#define ONLY_ASCII_NAMES (options.ascii_names)
#define MAX_GLOBAL_FNS (options.max_global_fns)

/* Compiler-specific stuff. */

#ifndef __GNUC_PREREQ
#if defined __GNUC__ && defined __GNUC_MINOR__
# define __GNUC_PREREQ(maj, min) \
        ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define __GNUC_PREREQ(maj, min) 0
#endif
#endif

/* For gcc 3 and up, this attribute lets the compiler know that the
 * function returns a newly allocated value, for pointer aliasing
 * optimizations.
 */
#if !defined(__attribute_malloc__) && __GNUC_PREREQ(2, 96)
#define __attribute_malloc__ __attribute__ ((__malloc__))
#elif !defined(__attribute_malloc__)
#define __attribute_malloc__
#endif

/* The C99 restrict keyword lets the compiler do some more pointer
 * aliasing optimizations. Essentially, a RESTRICT pointer function
 * argument can't share that pointer with ones passed via other
 * arguments. 
 * This should be a Configure check sometime.
 */
#if __GNUC_PREREQ(2, 92)
#define RESTRICT __restrict
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define RESTRICT restrict
#else
#define RESTRICT
#endif

/* If a compiler knows a function will never return, it can generate
   slightly better code for calls to it. */
#if defined(WIN32) && _MSC_VER >= 1200
#define NORETURN __declspec(noreturn)
#elif defined(HASATTRIBUTE)
#define NORETURN __attribute__ ((__noreturn__))
#else
#define NORETURN
#endif


#ifdef WIN32
/* --------------- Stuff for Win32 services ------------------ */
/*

   When "exit" is called to handle an error condition, we really want to
   terminate the game thread, not the whole process.
   MS VS.NET (_MSC_VER >= 1200) requires the weird noreturn stuff.

 */

#define exit(arg) Win32_Exit (arg)
void NORETURN WIN32_CDECL Win32_Exit(int exit_code);
#endif				/* WIN32 */


#endif				/* __CONF_H */
