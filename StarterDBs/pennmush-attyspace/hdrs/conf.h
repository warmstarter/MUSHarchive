/* conf.h */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "copyrite.h"
#include "options.h"

#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE
#endif


/*----- Miscellaneous other stuff -----*/

/* limit on player name length */
#define PLAYER_NAME_LIMIT 16
/* limit on object name length */
#define OBJECT_NAME_LIMIT 256

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

/* These seem dumb, but they used to be formulae, and I wanted to 
 * keep the macros to avoid breaking things.
 */
#define OBJECT_ENDOWMENT(cost) (cost)
#define OBJECT_DEPOSIT(pennies) (pennies)

#define QUIT_COMMAND "QUIT"
#define WHO_COMMAND "WHO"
#define LOGOUT_COMMAND "LOGOUT"
#define INFO_COMMAND "INFO"
#define INFO_VERSION "1"
#define DOING_COMMAND "DOING"

#define PREFIX_COMMAND "OUTPUTPREFIX"
#define SUFFIX_COMMAND "OUTPUTSUFFIX"
#define PUEBLO_COMMAND "PUEBLOCLIENT "

/* These CAN be modified, but it's heavily NOT suggested */
#define PUEBLO_SEND "</xch_mudtext><img xch_mode=purehtml><xch_page clear=text><body bgcolor=#000000 fgcolor=#C0C0C0>\n"
#define PUEBLO_HELLO "This world is Pueblo 1.10 Enhanced.  See http://www.chaco.com/pueblo/\r\n"

/*----- intrface.c stuff -----*/

#define MAX_OUTPUT 16384
/* How much output buffer space must be left before we flush the
 * buffer? Reportedly, using '0' fixes problems with Win32 port,
 * and may be more efficient in network use. Using (MAX_OUTPUT / 2)
 * is how it's been done in the past. You get to pick.
 */
#define SPILLOVER_THRESHOLD	0
/* #define SPILLOVER_THRESHOLD  (MAX_OUTPUT / 2) */
#define COMMAND_TIME_MSEC 1000	/* time slice length in milliseconds */
#define COMMAND_BURST_SIZE 100	/* commands allowed per user in a burst */
#define COMMANDS_PER_TIME 1	/* commands per time slice after burst */


/* ---  DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

typedef struct fblock_header FBLKHDR;
typedef struct filecache_block FBLOCK;

#define FBLOCK_SIZE (256 - sizeof(FBLKHDR))

struct fblock_header {
  FBLOCK *nxt;
  int nchars;
};

struct filecache_block {
  FBLKHDR hdr;
  char data[FBLOCK_SIZE];
};

typedef struct options_table OPTTAB;

struct options_table {
  char mud_name[128];
  int port;
  char input_db[256];
  char output_db[256];
  char crash_db[256];
  char mail_db[256];
  int player_start;
  int master_room;
  int idle_timeout;
  int dump_interval;
  char dump_message[256];
  char dump_complete[256];
  long dump_counter;
  int ident_timeout;
  int max_logins;
  int whisper_loudness;
  int blind_page;
  int paycheck;
  int starting_money;
  int starting_quota;
  int player_queue_limit;
  int queue_chunk;
  int active_q_chunk;
  int func_nest_lim;
  int func_invk_lim;
  char log_wipe_passwd[256];
  char money_singular[32];
  char money_plural[32];
  char compressprog[256];
  char uncompressprog[256];
  char compresssuff[256];
  char help_file[256];
  char help_index[256];
  char news_file[256];
  char news_index[256];
  char events_file[256];
  char events_index[256];
#ifdef CHAT_SYSTEM
  char chatdb[256];
  int max_player_chans;
  int max_channels;
  int chan_cost;
#endif
  char connect_file[2][256];
  char motd_file[2][256];
  char wizmotd_file[2][256];
  char newuser_file[2][256];
  char register_file[2][256];
  char quit_file[2][256];
  char down_file[2][256];
  char full_file[2][256];
  char guest_file[2][256];
  FBLOCK *connect_fcache[2];
  FBLOCK *motd_fcache[2];
  FBLOCK *wizmotd_fcache[2];
  FBLOCK *newuser_fcache[2];
  FBLOCK *register_fcache[2];
  FBLOCK *quit_fcache[2];
  FBLOCK *down_fcache[2];
  FBLOCK *full_fcache[2];
  FBLOCK *guest_fcache[2];
  int log_commands;
  int log_huhs;
  int log_forces;
  int log_walls;
  int support_pueblo;
  int login_allow;
  int guest_allow;
  int create_allow;
  int daytime;
  int player_flags;
  int room_flags;
  int reverse_shs;
  int exit_flags;
  int thing_flags;
  int player_toggles;
  int room_toggles;
  int exit_toggles;
  int thing_toggles;
  int use_rwho;
  int rwho_interval;
  long rwho_counter;
  int rwho_port;
  char rwho_host[64];
  char rwho_pass[64];
#ifdef USE_WARNINGS
  int warn_interval;
  long warn_counter;
#endif
  int base_room;
  int use_dns;
  int haspower_restricted;
  int safer_ufun;
#ifdef TREKMUSH
  char dump_warning_10sec[256];
#endif /* TREKMUSH */
  char dump_warning_1min[256];
  char dump_warning_5min[256];
  char index_file[256];
  char index_index[256];
  char rules_file[256];
  char rules_index[256];
  int hate_dest;
  int noisy_whisper;
  int possessive_get;
  int possessive_get_d;
  int really_safe;
  int destroy_possessions;
  int null_eq_zero;
  int tiny_booleans;
  int tiny_trim_fun;
  int tiny_math;
  int adestroy;
  int amail;
  int player_listen;
  int player_ahear;
  int startups;
  int room_connects;
  int ansi_names;
  int ansi_justify;
  int comma_exit_list;
  int count_all;
  int exits_connect_rooms;
  int link_to_object;
  int owner_queues;
  int wiz_noaenter;
  int use_ident;
  char ip_addr[64];
  int player_name_spaces;
  int forking_dump;
  int military_time;
  int restrict_building;
  int free_objects;
  int flags_on_examine;
  int ex_public_attribs;
  int tiny_attrs;
  int full_invis;
  int silent_pemit;
  int player_locate;
  int globals;
  int global_connects;
  int paranoid_nospoof;
  int max_dbref;
  char wizwall_prefix[256];
  char rwall_prefix[256];
  char wall_prefix[256];
  char access_file[256];
  char names_file[256];
  int object_cost;
  int exit_cost;
  int link_cost;
  int room_cost;
  int queue_cost;
  int quota_cost;
  int find_cost;
  int page_cost;
  int kill_default_cost;
  int kill_min_cost;
  int kill_bonus;
  int queue_loss;
  int max_pennies;
  int max_depth;
  int max_parents;
  int purge_interval;
  int dbck_interval;
};

extern OPTTAB options;

typedef struct confparm CONF;

struct confparm {
  const char *name;		/* name of option */
  /* the function handler */
  void (*handler) ();
/* This should be:
 * void (*handler) _((char *opt, char *val, int *loc, int maxval));
 * but some compilers (ultrix 4.2, for example) totally barf, *sigh*
 */
  int *loc;			/* place to put this option */
  int max;			/* max: string length, integer value */
  int overridden;		/* Has the default been overridden? */
  const char *group;		/* The option's group name */
};

/* Config group viewing permissions */
#define CGP_GOD		0x1
#define CGP_WIZARD	0x3
#define CGP_ADMIN	0x7
#define Can_View_Config_Group(p,g) \
	(!(g->viewperms) || (God(p) && (g->viewperms & CGP_GOD)) || \
	 (Wizard(p) && (g->viewperms & CGP_WIZARD)) || \
         (Hasprivs(p) && (g->viewperms & CGP_ADMIN)))


#define DUMP_INTERVAL       (options.dump_interval)
#define DUMP_NOFORK_MESSAGE  (options.dump_message)
#define DUMP_NOFORK_COMPLETE (options.dump_complete)
#define INACTIVITY_LIMIT    (options.idle_timeout)

#define MAX_LOGINS      (options.max_logins)

#define  HELPTEXT	(options.help_file)
#define  HELPINDX	(options.help_index)

#define  NEWS_FILE      (options.news_file)
#define  NEWSINDX       (options.news_index)

#define EVENT_FILE            (options.events_file)
#define EVENTINDX             (options.events_index)

/* dbrefs are in the conf file */

#define TINYPORT         (options.port)
#define PLAYER_START     (options.player_start)
#define MASTER_ROOM      (options.master_room)
#define MONEY            (options.money_singular)
#define MONIES           (options.money_plural)
#define WHISPER_LOUDNESS	(options.whisper_loudness)
#define BLIND_PAGE	(options.blind_page)

#define START_BONUS      (options.starting_money)
#define PAY_CHECK        (options.paycheck)
#define START_QUOTA      (options.starting_quota)
#define LOG_WIPE_PASSWD	 (options.log_wipe_passwd)
#define SUPPORT_PUEBLO   (options.support_pueblo)

#define QUEUE_QUOTA      (options.player_queue_limit)

#define MUDNAME          (options.mud_name)
#define DEF_DB_IN	 (options.input_db)
#define DEF_DB_OUT	 (options.output_db)

#define BASE_ROOM        (options.base_room)

#define USE_RWHO	 (options.use_rwho)
#define RWHO_INTERVAL    (options.rwho_interval)
#define RWHOSERV         (options.rwho_host)
#define RWHOPORT         (options.rwho_port)
#define RWHOPASS         (options.rwho_pass)

#define PURGE_INTERVAL   (options.purge_interval)
#define DBCK_INTERVAL    (options.dbck_interval)
#define MAX_PARENTS (options.max_parents)
#define MAX_DEPTH (options.max_depth)
#define MAX_PENNIES (options.max_pennies)
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
#define PARANOID_NOSPOOF (options.paranoid_nospoof)
#define GLOBAL_CONNECTS (options.global_connects)
#define DO_GLOBALS (options.globals)
#define PLAYER_LOCATE (options.player_locate)
#define SILENT_PEMIT (options.silent_pemit)
#define PLAYER_LISTEN (options.player_listen)
#define PLAYER_AHEAR (options.player_ahear)
#define STARTUPS (options.startups)
#define FULL_INVIS (options.full_invis)
#define TINY_ATTRS (options.tiny_attrs)
#define EX_PUBLIC_ATTRIBS (options.ex_public_attribs)
#define FLAGS_ON_EXAMINE (options.flags_on_examine)
#define FREE_OBJECTS (options.free_objects)
#define RESTRICTED_BUILDING (options.restrict_building)
#define MILITARY_TIME (options.military_time)
#define NO_FORK (!options.forking_dump)
#define PLAYER_NAME_SPACES (options.player_name_spaces)
#define ANSI_JUSTIFY (options.ansi_justify)
#define HASPOWER_RESTRICTED (options.haspower_restricted)
#define SAFER_UFUN (options.safer_ufun)
#define INDEX_FILE (options.index_file)
#define INDEXINDX (options.index_index)
#define RULES_FILE (options.rules_file)
#define RULESINDX (options.rules_index)
#define HATE_DEST (options.hate_dest)
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
#define ROOM_CONNECTS (options.room_connects)
#define ANSI_NAMES (options.ansi_names)
#define COMMA_EXIT_LIST (options.comma_exit_list)
#define COUNT_ALL (options.count_all)
#define EXITS_CONNECT_ROOMS (options.exits_connect_rooms)
#define WIZWALL_PREFIX (options.wizwall_prefix)
#define RWALL_PREFIX (options.rwall_prefix)
#define WALL_PREFIX (options.wall_prefix)
#define NO_LINK_TO_OBJECT (!options.link_to_object)
#define QUEUE_PER_OWNER (options.owner_queues)
#define WIZ_NOAENTER (options.wiz_noaenter)
#define USE_IDENT (options.use_ident)
#define MUSH_IP_ADDR (options.ip_addr)

#ifdef WIN32
/* --------------- Stuff for Win32 services ------------------ */
/*

   NJG:

   When "exit" is called to handle an error condition, we really want to
   terminate the game thread, not the whole process.

 */

#define exit(arg) Win32_Exit (arg)
void Win32_Exit(int exit_code);
#endif				/* WIN32 */

#endif				/* __OPTIONS_H */
