/**
 * \file conf.c
 *
 * \brief PennMUSH runtime configuration.
 *
 * configuration adjustment. Some of the ideas and bits and pieces of the
 * code here are based on TinyMUSH 2.0.
 *
 */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "conf.h"
#include "externs.h"
#include "pueblo.h"
#include "mushdb.h"
#include "parse.h"
#include "command.h"
#include "flags.h"
#include "log.h"
#include "dbdefs.h"
#include "game.h"
#include "attrib.h"
#include "help.h"
#include "function.h"
#include "confmagic.h"

#ifdef macintosh
#include "PMInit.h"
#endif

time_t mudtime;			/* game time, in seconds */

typedef struct confparm CONF;

/** Runtime configuration parameter.
 * This structure represents a runtime configuration option.
 */
struct confparm {
  const char *name;		/**< name of option. */
  /** the function handler. */
  int (*handler)
   (const char *opt, const char *val, void *loc, int maxval, int source);
  void *loc;			/**< place to put this option. */
  int max;			/**< max: string length, integer value. */
  int overridden;		/**< Has the default been overridden? */
  const char *group;		/**< The option's group name */
};

static void show_compile_options(dbref player);
static char *config_list_helper(dbref player, CONF *cp, int lc);
static char *config_list_helper2(dbref player, CONF *cp, int lc);

OPTTAB options;

int cf_bool(const char *opt, const char *val, void *loc, int maxval,
	    int source);
int cf_str(const char *opt, const char *val, void *loc, int maxval, int source);
int cf_int(const char *opt, const char *val, void *loc, int maxval, int source);
int cf_dbref(const char *opt, const char *val, void *loc, int maxval,
	     int source);
int cf_flag(const char *opt, const char *val, void *loc, int maxval,
	    int source);
int config_set(const char *opt, char *val, int source, int restrictions);
void conf_default_set(void);

/** Table of all runtime configuration options. */
CONF conftable[] = {
  {"input_database", cf_str, options.input_db, sizeof options.input_db, 0,
   "files"}
  ,
  {"output_database", cf_str, options.output_db, sizeof options.output_db, 0,
   "files"}
  ,
  {"crash_database", cf_str, options.crash_db, sizeof options.crash_db, 0,
   "files"}
  ,
  {"mail_database", cf_str, options.mail_db, sizeof options.mail_db, 0, "files"}
  ,
#ifdef CHAT_SYSTEM
  {"chat_database", cf_str, options.chatdb, sizeof options.chatdb, 0, "files"}
  ,
#endif
  {"compress_suffix", cf_str, options.compresssuff, sizeof options.compresssuff,
   0,
   "files"}
  ,
  {"compress_program", cf_str, options.compressprog,
   sizeof options.compressprog, 0,
   "files"}
  ,
  {"uncompress_program", cf_str, options.uncompressprog,
   sizeof options.uncompressprog, 0,
   "files"}
  ,
  {"access_file", cf_str, options.access_file, sizeof options.access_file, 0,
   "files"}
  ,
  {"names_file", cf_str, options.names_file, sizeof options.access_file, 0,
   "files"}
  ,
  {"connect_file", cf_str, options.connect_file[0],
   sizeof options.connect_file[0], 0, "messages"}
  ,
  {"motd_file", cf_str, options.motd_file[0], sizeof options.motd_file[0], 0,
   "messages"}
  ,
  {"wizmotd_file", cf_str, options.wizmotd_file[0],
   sizeof options.wizmotd_file[0], 0, "messages"}
  ,
  {"newuser_file", cf_str, options.newuser_file[0],
   sizeof options.newuser_file[0], 0, "messages"}
  ,

  {"register_create_file", cf_str, options.register_file[0],
   sizeof options.register_file[0],
   0, "messages"}
  ,
  {"quit_file", cf_str, options.quit_file[0], sizeof options.quit_file[0], 0,
   "messages"}
  ,
  {"down_file", cf_str, options.down_file[0], sizeof options.down_file[0], 0,
   "messages"}
  ,
  {"full_file", cf_str, options.full_file[0], sizeof options.full_file[0], 0,
   "messages"}
  ,
  {"guest_file", cf_str, options.guest_file[0], sizeof options.guest_file[0], 0,
   "messages"}
  ,

  {"connect_html_file", cf_str, options.connect_file[1],
   sizeof options.connect_file[1], 0,
   "messages"}
  ,
  {"motd_html_file", cf_str, options.motd_file[1],
   sizeof options.connect_file[1], 0,
   "messages"}
  ,
  {"wizmotd_html_file", cf_str, options.wizmotd_file[1],
   sizeof options.wizmotd_file[1], 0,
   "messages"}
  ,
  {"newuser_html_file", cf_str, options.newuser_file[1],
   sizeof options.newuser_file[1], 0,
   "messages"}
  ,
  {"register_create_html_file", cf_str, options.register_file[1],
   sizeof options.register_file[1], 0, "messages"}
  ,
  {"quit_html_file", cf_str, options.quit_file[1], sizeof options.quit_file[1],
   0,
   "messages"}
  ,
  {"down_html_file", cf_str, options.down_file[1], sizeof options.down_file[1],
   0,
   "messages"}
  ,
  {"full_html_file", cf_str, options.full_file[1], sizeof options.full_file[1],
   0,
   "messages"}
  ,
  {"guest_html_file", cf_str, options.guest_file[1],
   sizeof options.guest_file[1], 0,
   "messages"}
  ,

  {"player_start", cf_dbref, &options.player_start, 100000, 0, "db"}
  ,
  {"master_room", cf_dbref, &options.master_room, 100000, 0, "db"}
  ,
  {"base_room", cf_dbref, &options.base_room, 100000, 0, "db"}
  ,
  {"exits_connect_rooms", cf_bool, &options.exits_connect_rooms, 2, 0, "db"}
  ,
  {"zone_control_zmp_only", cf_bool, &options.zone_control, 2, 0, "db"}
  ,

  {"mud_name", cf_str, options.mud_name, 128, 0, "net"}
  ,
  {"ip_addr", cf_str, options.ip_addr, 64, 0, "net"}
  ,
  {"port", cf_int, &options.port, 32000, 0, "net"}
  ,
  {"sslport", cf_int, &options.sslport, 32000, 0, "net"}
  ,
  {"use_dns", cf_bool, &options.use_dns, 2, 0, "net"}
  ,
  {"use_ident", cf_bool, &options.use_ident, 2, 0, "net"}
  ,
  {"ident_timeout", cf_int, &options.ident_timeout, 60, 0, "net"}
  ,
  {"logins", cf_bool, &options.login_allow, 2, 0, "net"}
  ,
  {"player_creation", cf_bool, &options.create_allow, 2, 0, "net"}
  ,
  {"guests", cf_bool, &options.guest_allow, 2, 0, "net"}
  ,
  {"pueblo", cf_bool, &options.support_pueblo, 2, 0, "net"}
  ,

  {"forking_dump", cf_bool, &options.forking_dump, 2, 0, "dump"}
  ,
  {"dump_message", cf_str, options.dump_message, sizeof options.dump_message, 0,
   "dump"}
  ,
  {"dump_complete", cf_str, options.dump_complete, sizeof options.dump_complete,
   0, "dump"}
  ,
  {"dump_warning_1min", cf_str, options.dump_warning_1min,
   sizeof options.dump_warning_1min,
   0, "dump"}
  ,
  {"dump_warning_5min", cf_str, options.dump_warning_5min,
   sizeof options.dump_warning_5min, 0,
   "dump"}
  ,
  {"dump_interval", cf_int, &options.dump_interval, 100000, 0, "dump"}
  ,
  {"warn_interval", cf_int, &options.warn_interval, 32000, 0, "dump"}
  ,
  {"purge_interval", cf_int, &options.purge_interval, 10000, 0, "dump"}
  ,
  {"dbck_interval", cf_int, &options.dbck_interval, 10000, 0, "dump"}
  ,

  {"money_singular", cf_str, options.money_singular,
   sizeof options.money_singular, 0,
   "cosmetic"}
  ,
  {"money_plural", cf_str, options.money_plural, sizeof options.money_plural, 0,
   "cosmetic"}
  ,
  {"player_name_spaces", cf_bool, &options.player_name_spaces, 2, 0,
   "cosmetic"}
  ,
  {"ansi_names", cf_bool, &options.ansi_names, 2, 0, "cosmetic"}
  ,
  {"only_ascii_in_names", cf_bool, &options.ascii_names, 2, 0, "cosmetic"}
  ,
  {"float_precision", cf_int, &options.float_precision, 10000, 0, "cosmetic"}
  ,
  {"comma_exit_list", cf_bool, &options.comma_exit_list, 2, 0, "cosmetic"}
  ,
  {"count_all", cf_bool, &options.count_all, 2, 0, "cosmetic"}
  ,
  {"blind_page", cf_bool, &options.blind_page, 2, 0, "cosmetic"}
  ,
  {"page_aliases", cf_bool, &options.page_aliases, 2, 0, "cosmetic"}
  ,
  {"flags_on_examine", cf_bool, &options.flags_on_examine, 2, 0, "cosmetic"}
  ,
  {"ex_public_attribs", cf_bool, &options.ex_public_attribs, 2, 0,
   "cosmetic"}
  ,
  {"wizwall_prefix", cf_str, options.wizwall_prefix,
   sizeof options.wizwall_prefix, 0,
   "cosmetic"}
  ,
  {"rwall_prefix", cf_str, options.rwall_prefix, sizeof options.rwall_prefix, 0,
   "cosmetic"}
  ,
  {"wall_prefix", cf_str, options.wall_prefix, sizeof options.wall_prefix, 0,
   "cosmetic"}
  ,
  {"chat_strip_quote", cf_bool, &options.chat_strip_quote, 2, 0, "cosmetic"}
  ,
  {"newline_one_char", cf_bool, &options.newline_one_char, 2, 0, "cosmetic"}
  ,

  {"max_dbref", cf_dbref, &options.max_dbref, -1, 0, "limits"}
  ,
  {"max_attrs_per_obj", cf_int, &options.max_attrcount, 8192, 0, "limits"}
  ,
  {"max_logins", cf_int, &options.max_logins, 128, 0, "limits"}
  ,
  {"max_guests", cf_int, &options.max_guests, 128, 0, "limits"}
  ,
  {"idle_timeout", cf_int, &options.idle_timeout, 100000, 0, "limits"}
  ,
  {"unconnected_idle_timeout", cf_int, &options.unconnected_idle_timeout,
   100000, 0, "limits"}
  ,
  {"whisper_loudness", cf_int, &options.whisper_loudness, 100, 0, "limits"}
  ,
  {"starting_quota", cf_int, &options.starting_quota, 10000, 0, "limits"}
  ,
  {"starting_money", cf_int, &options.starting_money, 10000, 0, "limits"}
  ,
  {"paycheck", cf_int, &options.paycheck, 1000, 0, "limits"}
  ,
  {"guest_paycheck", cf_int, &options.guest_paycheck, 1000, 0, "limits"}
  ,
  {"max_pennies", cf_int, &options.max_pennies, 100000, 0, "limits"}
  ,
  {"max_guest_pennies", cf_int, &options.max_guest_pennies, 100000, 0,
   "limits"}
  ,
  {"max_parents", cf_int, &options.max_parents, 10000, 0, "limits"}
  ,
  {"mail_limit", cf_int, &options.mail_limit, 5000, 0, "limits"}
  ,
  {"max_depth", cf_int, &options.max_depth, 10000, 0, "limits"}
  ,
  {"player_queue_limit", cf_int, &options.player_queue_limit, 100000, 0,
   "limits"}
  ,
  {"queue_loss", cf_int, &options.queue_loss, 10000, 0, "limits"}
  ,
  {"queue_chunk", cf_int, &options.queue_chunk, 100000, 0, "limits"}
  ,
  {"active_queue_chunk", cf_int, &options.active_q_chunk, 100000, 0,
   "limits"}
  ,
  {"function_recursion_limit", cf_int, &options.func_nest_lim, 100000, 0,
   "limits"}
  ,
  {"function_invocation_limit", cf_int, &options.func_invk_lim, 100000, 0,
   "limits"}
  ,
  {"call_limit", cf_int, &options.call_lim, 1000000, 0,
   "limits"}
  ,
  {"player_name_len", cf_int, &options.player_name_len, BUFFER_LEN, 0,
   "limits"}
  ,
  {"queue_entry_cpu_time", cf_int, &options.queue_entry_cpu_time, 100000, 0,
   "limits"}
  ,
  {"max_global_fns", cf_int, &options.max_global_fns, 2000, 0, 0}
  ,

#ifdef CHAT_SYSTEM
  {"max_channels", cf_int, &options.max_channels, 1000, 0, "chat"}
  ,
  {"max_player_chans", cf_int, &options.max_player_chans, 100, 0, "chat"}
  ,
  {"chan_cost", cf_int, &options.chan_cost, 10000, 0, "chat"}
  ,
#endif

  {"log_commands", cf_bool, &options.log_commands, 2, 0, "log"}
  ,
  {"log_huhs", cf_bool, &options.log_huhs, 2, 0, "log"}
  ,
  {"log_forces", cf_bool, &options.log_forces, 2, 0, "log"}
  ,
  {"log_walls", cf_bool, &options.log_walls, 2, 0, "log"}
  ,

  {"player_flags", cf_flag, options.player_flags, sizeof options.player_flags,
   0, "flags"}
  ,
  {"room_flags", cf_flag, options.room_flags, sizeof options.room_flags, 0,
   "flags"}
  ,
  {"exit_flags", cf_flag, options.exit_flags, sizeof options.exit_flags, 0,
   "flags"}
  ,
  {"thing_flags", cf_flag, options.thing_flags, sizeof options.thing_flags, 0,
   "flags"}
  ,

  {"haspower_restricted", cf_bool, &options.haspower_restricted, 2, 0,
   "funcs"}
  ,
  {"safer_ufun", cf_bool, &options.safer_ufun, 2, 0, "funcs"}
  ,

  {"noisy_whisper", cf_bool, &options.noisy_whisper, 2, 0, "cmds"}
  ,
  {"possessive_get", cf_bool, &options.possessive_get, 2, 0, "cmds"}
  ,
  {"possessive_get_d", cf_bool, &options.possessive_get_d, 2, 0, "cmds"}
  ,
  {"link_to_object", cf_bool, &options.link_to_object, 2, 0, "cmds"}
  ,
  {"owner_queues", cf_bool, &options.owner_queues, 2, 0, "cmds"}
  ,
  {"full_invis", cf_bool, &options.full_invis, 2, 0, "cmds"}
  ,
  {"wiz_noaenter", cf_bool, &options.wiz_noaenter, 2, 0, "cmds"}
  ,
  {"really_safe", cf_bool, &options.really_safe, 2, 0, "cmds"}
  ,
  {"destroy_possessions", cf_bool, &options.destroy_possessions, 2, 0,
   "cmds"}
  ,

  {"null_eq_zero", cf_bool, &options.null_eq_zero, 2, 0, "tiny"}
  ,
  {"tiny_booleans", cf_bool, &options.tiny_booleans, 2, 0, "tiny"}
  ,
  {"tiny_trim_fun", cf_bool, &options.tiny_trim_fun, 2, 0, "tiny"}
  ,
  {"tiny_math", cf_bool, &options.tiny_math, 2, 0, "tiny"}
  ,
  {"silent_pemit", cf_bool, &options.silent_pemit, 2, 0, "tiny"}
  ,

  {"adestroy", cf_bool, &options.adestroy, 2, 0, "attribs"}
  ,
  {"amail", cf_bool, &options.amail, 2, 0, "attribs"}
  ,
  {"player_listen", cf_bool, &options.player_listen, 2, 0, "attribs"}
  ,
  {"player_ahear", cf_bool, &options.player_ahear, 2, 0, "attribs"}
  ,
  {"startups", cf_bool, &options.startups, 2, 0, "attribs"}
  ,
  {"room_connects", cf_bool, &options.room_connects, 2, 0, "attribs"}
  ,
  {"reverse_shs", cf_bool, &options.reverse_shs, 2, 0, "attribs"}
  ,
  {"object_cost", cf_int, &options.object_cost, 10000, 0, "costs"}
  ,
  {"exit_cost", cf_int, &options.exit_cost, 10000, 0, "costs"}
  ,
  {"link_cost", cf_int, &options.link_cost, 10000, 0, "costs"}
  ,
  {"room_cost", cf_int, &options.room_cost, 10000, 0, "costs"}
  ,
  {"queue_cost", cf_int, &options.queue_cost, 10000, 0, "costs"}
  ,
  {"quota_cost", cf_int, &options.quota_cost, 10000, 0, "costs"}
  ,
  {"find_cost", cf_int, &options.find_cost, 10000, 0, "costs"}
  ,
  {"page_cost", cf_int, &options.page_cost, 10000, 0, "costs"}
  ,
  {"kill_default_cost", cf_int, &options.kill_default_cost, 10000, 0,
   "costs"}
  ,
  {"kill_min_cost", cf_int, &options.kill_min_cost, 10000, 0, "costs"}
  ,
  {"kill_bonus", cf_int, &options.kill_bonus, 100, 0, "costs"}
  ,

  {"log_wipe_passwd", cf_str, options.log_wipe_passwd,
   sizeof options.log_wipe_passwd, 0,
   NULL}
  ,

  {NULL, NULL, NULL, 0, 0, NULL}
};

typedef struct confgroupparm CONFGROUP;

/** A runtime configuration group.
 * This struction represents the name and information about a group
 * of runtime configuration directives. Groups are used to organize
 * the display of configuration options.
 */
struct confgroupparm {
  const char *name;		/**< name of group */
  const char *desc;		/**< description of group */
  int viewperms;		/**< who can view this group */
};

/** The table of all configuration groups. */
CONFGROUP confgroups[] = {
  {"attribs", "Options affecting attributes", 0},
  {"chat", "Chat system options", 0},
  {"cmds", "Options affecting command behavior", 0},
  {"compile", "Compile-time options", 0},
  {"cosmetic", "Cosmetic options", 0},
  {"costs", "Costs", 0},
  {"db", "Database options", 0},
  {"dump", "Options affecting dumps and other periodic processes", 0},
  {"files", "Files used by the MUSH", CGP_GOD},
  {"flags", "Default flags for new objects", 0},
  {"funcs", "Options affecting function behavior", 0},
  {"limits", "Limits and other constants", 0},
  {"log", "Logging options", 0},
  {"messages", "Message files sent by the MUSH", CGP_GOD},
  {"net", "Networking and connection-related options", 0},
  {"tiny", "TinyMUSH compatibility options", 0},
  {NULL, NULL, 0}
};

/** Parse a boolean configuration option.
 * \param opt name of the configuration option.
 * \param val value of the option.
 * \param loc address to store the value.
 * \param maxval (unused).
 * \param source 0 if read from config file; 1 if from command.
 * \retval 0 failure (unable to parse val).
 * \retval 1 success.
 */
int
cf_bool(const char *opt, const char *val, void *loc,
	int maxval __attribute__ ((__unused__)), int source)
{
  /* enter boolean parameter */

  if (!strcasecmp(val, "yes") || !strcasecmp(val, "true") ||
      !strcasecmp(val, "1"))
    *((int *) loc) = 1;
  else if (!strcasecmp(val, "no") || !strcasecmp(val, "false") ||
	   !strcasecmp(val, "0"))
    *((int *) loc) = 0;
  else {
    if (source == 0) {
      do_rawlog(LT_ERR, T("CONFIG: option %s value %s invalid.\n"), opt, val);
    }
    return 0;
  }
  return 1;
}


/** Parse a string configuration option.
 * \param opt name of the configuration option.
 * \param val value of the option.
 * \param loc address to store the value.
 * \param maxval maximum length of value string.
 * \param source 0 if read from config file; 1 if from command.
 * \retval 0 failure (unable to parse val).
 * \retval 1 success.
 */
int
cf_str(const char *opt, const char *val, void *loc, int maxval, int source)
{
  /* enter string parameter */
  size_t len = strlen(val);

  /* truncate if necessary */
  if (len >= (size_t) maxval) {
    if (source == 0) {
      do_rawlog(LT_ERR, T("CONFIG: option %s value truncated\n"), opt);
    }
    len = maxval - 1;
  }
  memcpy(loc, val, len);
  *((char *) loc + len) = '\0';
  return 1;
}

/** Parse a dbref configuration option.
 * \param opt name of the configuration option.
 * \param val value of the option.
 * \param loc address to store the value.
 * \param maxval maximum dbref.
 * \param source 0 if read from config file; 1 if from command.
 * \retval 0 failure (unable to parse val).
 * \retval 1 success.
 */
int
cf_dbref(const char *opt, const char *val, void *loc, int maxval, int source)
{
  /* enter dbref or integer parameter */

  int n;
  size_t offset = 0;

  if (val && val[0] == '#')
    offset = 1;

  n = parse_integer(val + offset);

  /* enforce limits */
  if ((maxval >= 0) && (n > maxval)) {
    n = maxval;
    if (source == 0) {
      do_rawlog(LT_ERR, T("CONFIG: option %s value limited to #%d\n"), opt,
		maxval);
    }
  }
  if (source && (!GoodObject(n) || IsGarbage(n))) {
    do_rawlog(LT_ERR,
	      T("CONFIG: attempt to set option %s to a bad dbref (#%d)"),
	      opt, n);
    return 0;
  }
  *((dbref *) loc) = n;
  return 1;
}

/** Parse an integer configuration option.
 * \param opt name of the configuration option.
 * \param val value of the option.
 * \param loc address to store the value.
 * \param maxval maximum value.
 * \param source 0 if read from config file; 1 if from command.
 * \retval 0 failure (unable to parse val).
 * \retval 1 success.
 */
int
cf_int(const char *opt, const char *val, void *loc, int maxval, int source)
{
  /* enter integer parameter */

  int n;

  n = atoi(val);

  /* enforce limits */
  if ((maxval >= 0) && (n > maxval)) {
    n = maxval;
    if (source == 0) {
      do_rawlog(LT_ERR, T("CONFIG: option %s value limited to %d\n"), opt,
		maxval);
    }
  }
  *((int *) loc) = n;
  return 1;
}


/** Parse a flag configuration option.
 * This is just like parsing a string option, but collects multiple
 * string options with the same name into a single value.
 * \param opt name of the configuration option.
 * \param val value of the option.
 * \param loc address to store the value.
 * \param maxval maximum length of value.
 * \param source 0 if read from config file; 1 if from command.
 * \retval 0 failure (unable to parse val).
 * \retval 1 success.
 */
int
cf_flag(const char *opt, const char *val, void *loc, int maxval, int source)
{
  size_t len = strlen(val);
  size_t total = strlen((char *) loc);

  /* truncate if necessary */
  if (len + total + 1 >= (size_t) maxval) {
    len = maxval - total - 1;
    if (len <= 0) {
      if (source == 0)
	do_rawlog(LT_ERR, T("CONFIG: option %s value overflow\n"), opt);
      return 0;
    }
    if (source == 0)
      do_rawlog(LT_ERR, T("CONFIG: option %s value truncated\n"), opt);
  }
  sprintf((char *) loc, "%s %s", (char *) loc, val);
  return 1;
}

/** Set a configuration option.
 * This function sets a runtime configuration option. During the load
 * of the configuration file, it gets run twice - once to set the
 * main set of options and once again to set restrictions and aliases
 * that require having the flag table available.
 * \param opt name of the option.
 * \param val value to set the option to.
 * \param source 0 if being set from mush.cnf, 1 from softcode.
 * \param restrictions 1 if we're setting restriction options, 0 for others.
 * \retval 1 success.
 * \retval 0 failure.
 */
int
config_set(const char *opt, char *val, int source, int restrictions)
{
  CONF *cp;
  char *p;

  if (!val)
    return 0;			/* NULL val is no good, but "" is ok */

  /* Was this "restrict_command <command> <restriction>"? If so, do it */
  if (!strcasecmp(opt, "restrict_command")) {
    if (!restrictions)
      return 1;
    for (p = val; *p && !isspace((unsigned char) *p); p++) ;
    if (*p) {
      *p++ = '\0';
      if (!restrict_command(val, p)) {
	if (source == 0) {
	  do_rawlog(LT_ERR,
		    T("CONFIG: Invalid command or restriction for %s.\n"), val);
	}
	return 0;
      }
    } else {
      if (source == 0) {
	do_rawlog(LT_ERR,
		  T
		  ("CONFIG: restrict_command %s requires a restriction value.\n"),
		  val);
      }
      return 0;
    }
    return 1;
  } else if (!strcasecmp(opt, "restrict_function")) {
    if (!restrictions)
      return 1;
    for (p = val; *p && !isspace((unsigned char) *p); p++) ;
    if (*p) {
      *p++ = '\0';
      if (!restrict_function(val, p)) {
	if (source == 0) {
	  do_rawlog(LT_ERR,
		    T("CONFIG: Invalid function or restriction for %s.\n"),
		    val);
	}
	return 0;
      }
    } else {
      if (source == 0) {
	do_rawlog(LT_ERR,
		  T
		  ("CONFIG: restrict_function %s requires a restriction value.\n"),
		  val);
      }
      return 0;
    }
    return 1;
  } else if (!strcasecmp(opt, "reserve_alias")) {
    if (!restrictions)
      return 1;
    reserve_alias(val);
    return 1;
  } else if (!strcasecmp(opt, "command_alias")) {
    if (!restrictions)
      return 1;
    for (p = val; *p && !isspace((unsigned char) *p); p++) ;
    if (*p) {
      *p++ = '\0';
      if (!alias_command(val, p)) {
	if (source == 0) {
	  do_rawlog(LT_ERR, T("CONFIG: Couldn't alias %s to %s.\n"), p, val);
	}
	return 0;
      }
    } else {
      if (source == 0) {
	do_rawlog(LT_ERR,
		  T("CONFIG: command_alias %s requires an alias.\n"), val);
      }
      return 0;
    }
    return 1;
  } else if (!strcasecmp(opt, "attribute_alias")) {
    if (!restrictions)
      return 1;
    for (p = val; *p && !isspace((unsigned char) *p); p++) ;
    if (*p) {
      *p++ = '\0';
      if (!alias_attribute(val, p)) {
	if (source == 0) {
	  do_rawlog(LT_ERR, T("CONFIG: Couldn't alias %s to %s.\n"), p, val);
	}
	return 0;
      }
    } else {
      if (source == 0) {
	do_rawlog(LT_ERR,
		  T("CONFIG: attribute_alias %s requires an alias.\n"), val);
      }
      return 0;
    }
    return 1;
  } else if (!strcasecmp(opt, "function_alias")) {
    if (!restrictions)
      return 1;
    for (p = val; *p && !isspace((unsigned char) *p); p++) ;
    if (*p) {
      *p++ = '\0';
      if (!alias_function(val, p)) {
	if (source == 0) {
	  do_rawlog(LT_ERR, T("CONFIG: Couldn't alias %s to %s.\n"), p, val);
	}
	return 0;
      }
    } else {
      if (source == 0) {
	do_rawlog(LT_ERR,
		  T("CONFIG: function_alias %s requires an alias.\n"), val);
      }
      return 0;
    }
    return 1;
  } else if (!strcasecmp(opt, "help_command")
	     || !strcasecmp(opt, "ahelp_command")) {
    char *comm, *file;
    int admin = !strcasecmp(opt, "ahelp_command");
    if (!restrictions)
      return 1;
    /* Add a new help-like command */
    if (source == 1)
      return 0;
    if (!val || !*val) {
      do_rawlog(LT_ERR,
		T
		("CONFIG: help_command requires a command name and file name.\n"));
      return 0;
    }
    comm = val;
    for (file = val; *file && !isspace((unsigned char) *file); file++) ;
    if (*file) {
      *file++ = '\0';
      add_help_file(comm, file, admin);
      return 1;
    } else {
      do_rawlog(LT_ERR,
		T
		("CONFIG: help_command requires a command name and file name.\n"));
      return 0;
    }
  } else if (restrictions) {
    return 1;
  }
  /* search conf table for the option; if found, add it, if not found,
   * complain about it. Forbid use of @config to set options without
   * groups (log_wipe_passwd), or the file and message groups (@config/set
   * output_data=../../.bashrc? Ouch.)  */
  for (cp = conftable; cp->name; cp++) {
    int i = 0;
    if ((!source || (cp->group && strcmp(cp->group, "files") != 0
		     && strcmp(cp->group, "messages") != 0))
	&& !strcasecmp(cp->name, opt)) {
      i = cp->handler(opt, val, cp->loc, cp->max, source);
      if (i)
	cp->overridden = 1;
      return i;
    }
  }

  if (source == 0) {
    do_rawlog(LT_ERR, T("CONFIG: directive '%s' in cnf file ignored.\n"), opt);
  }
  return 0;
}

/** Set the default configuration options.
 */
void
conf_default_set(void)
{
  strcpy(options.mud_name, "TinyMUSH");
  options.port = 4201;
  options.sslport = 0;
  strcpy(options.input_db, "data/indb");
  strcpy(options.output_db, "data/outdb");
  strcpy(options.crash_db, "data/PANIC.db");
#ifdef CHAT_SYSTEM
  strcpy(options.chatdb, "data/chatdb");
  options.chan_cost = 1000;
  options.max_player_chans = 3;
  options.max_channels = 200;
#endif
  strcpy(options.mail_db, "data/maildb");
  options.player_start = 0;
  options.master_room = 2;
  options.base_room = 0;
  options.idle_timeout = 0;
  options.unconnected_idle_timeout = 5;
  options.dump_interval = 3601;
  strcpy(options.dump_message,
	 T("GAME: Dumping database. Game may freeze for a minute"));
  strcpy(options.dump_complete, T("GAME: Dump complete. Time in."));
  options.ident_timeout = 5;
  options.max_logins = 128;
  options.max_guests = 0;
  options.whisper_loudness = 100;
  options.blind_page = 1;
  options.page_aliases = 0;
  options.paycheck = 50;
  options.guest_paycheck = 0;
  options.starting_money = 100;
  options.starting_quota = 20;
  options.player_queue_limit = 100;
  options.queue_chunk = 3;
  options.active_q_chunk = 0;
  options.func_nest_lim = 50;
  options.func_invk_lim = 2500;
  options.call_lim = 0;
  strcpy(options.money_singular, "Penny");
  strcpy(options.money_plural, "Pennies");
  strcpy(options.log_wipe_passwd, "zap!");
#ifdef WIN32
  strcpy(options.compressprog, "");
  strcpy(options.uncompressprog, "");
  strcpy(options.compresssuff, "");
#else
  strcpy(options.compressprog, "compress");
  strcpy(options.uncompressprog, "uncompress");
  strcpy(options.compresssuff, ".Z");
#endif				/* WIN32 */
  strcpy(options.connect_file[0], "txt/connect.txt");
  strcpy(options.motd_file[0], "txt/motd.txt");
  strcpy(options.wizmotd_file[0], "txt/wizmotd.txt");
  strcpy(options.newuser_file[0], "txt/newuser.txt");
  strcpy(options.register_file[0], "txt/register.txt");
  strcpy(options.quit_file[0], "txt/quit.txt");
  strcpy(options.down_file[0], "txt/down.txt");
  strcpy(options.full_file[0], "txt/full.txt");
  strcpy(options.guest_file[0], "txt/guest.txt");
  options.log_commands = 0;
  options.log_huhs = 0;
  options.log_forces = 1;
  options.log_walls = 0;
  options.support_pueblo = 0;
  options.login_allow = 1;
  options.guest_allow = 1;
  options.create_allow = 1;
  strcpy(options.player_flags, "");
  strcpy(options.room_flags, "");
  strcpy(options.exit_flags, "");
  strcpy(options.thing_flags, "");
  options.warn_interval = 3600;
  options.use_dns = 1;
  options.haspower_restricted = 0;
  options.safer_ufun = 1;
  strcpy(options.dump_warning_1min,
	 T("GAME: Database will be dumped in 1 minute."));
  strcpy(options.dump_warning_5min,
	 T("GAME: Database will be dumped in 5 minutes."));
  options.noisy_whisper = 0;
  options.possessive_get = 1;
  options.possessive_get_d = 1;
  options.really_safe = 1;
  options.destroy_possessions = 1;
  options.null_eq_zero = 0;
  options.tiny_booleans = 0;
  options.tiny_math = 0;
  options.tiny_trim_fun = 0;
  options.adestroy = 0;
  options.amail = 0;
  options.mail_limit = 5000;
  options.player_listen = 1;
  options.player_ahear = 1;
  options.startups = 1;
  options.room_connects = 0;
  options.reverse_shs = 1;
  options.ansi_names = 1;
  options.comma_exit_list = 1;
  options.count_all = 0;
  options.exits_connect_rooms = 0;
  options.zone_control = 1;
  options.link_to_object = 1;
  options.owner_queues = 0;
  options.wiz_noaenter = 0;
  options.use_ident = 1;
  strcpy(options.ip_addr, "");
  options.player_name_spaces = 0;
  options.forking_dump = 1;
  options.restrict_building = 0;
  options.free_objects = 1;
  options.flags_on_examine = 1;
  options.ex_public_attribs = 1;
  options.full_invis = 0;
  options.silent_pemit = 0;
  options.max_dbref = 0;
  options.chat_strip_quote = 1;
  strcpy(options.wizwall_prefix, "Broadcast:");
  strcpy(options.rwall_prefix, "Admin:");
  strcpy(options.wall_prefix, "Announcement:");
  strcpy(options.access_file, "access.cnf");
  strcpy(options.names_file, "names.cnf");
  options.object_cost = 10;
  options.exit_cost = 1;
  options.link_cost = 1;
  options.room_cost = 10;
  options.queue_cost = 10;
  options.quota_cost = 1;
  options.find_cost = 100;
  options.page_cost = 0;
  options.kill_default_cost = 100;
  options.kill_min_cost = 10;
  options.kill_bonus = 50;
  options.queue_loss = 63;
  options.max_pennies = 100000;
  options.max_guest_pennies = 100000;
  options.max_depth = 10;
  options.max_parents = 10;
  options.max_global_fns = 50;
  options.purge_interval = 601;
  options.dbck_interval = 599;
  options.max_attrcount = 2048;
  options.float_precision = 6;
  options.newline_one_char = 1;
  options.player_name_len = 16;
  options.queue_entry_cpu_time = 1500;
  options.ascii_names = 1;
  options.call_lim = 10000;
}

/* Limit how many files we can nest */
static int conf_recursion = 0;

/** Read a configuration file.
 * This function is called to read a configuration file. We may recurse,
 * as there's an 'include' directive. It's called twice, once before
 * the flag table load (when we want all options except restriction/alias
 * options) and once after (when we only want restriction/alias options.)
 * \param conf name of configuration file to read.
 * \param restrictions 1 if reading restriction options, 0 otherwise.
 * \retval 1 success.
 * \retval 0 failure.
 */
int
config_file_startup(const char *conf, int restrictions)
{
  /* read a configuration file. Return 0 on failure, 1 on success */
  /* If 'restrictions' is 0, ignore restrict*. If it's 1, only
   * look at restrict*
   */

  FILE *fp = NULL;
  CONF *cp;
  char tbuf1[BUFFER_LEN];
  char *p, *q, *s;

  static char cfile[BUFFER_LEN];	/* Remember the last one */
  if (conf_recursion == 0) {
    if (conf && *conf)
      strcpy(cfile, conf);
#ifdef macintosh
    fp = fopen(cfile, "rb");
#else
    fp = fopen(cfile, "r");
#endif
    if (fp == NULL) {
      do_rawlog(LT_ERR, T("ERROR: Cannot open configuration file %s."), cfile);
      return 0;
    }
    do_rawlog(LT_ERR, "Reading %s", cfile);
  } else {
    if (conf && *conf)
#ifdef macintosh
      fp = fopen(conf, "rb");
#else
      fp = fopen(conf, "r");
#endif
    if (fp == NULL) {
      do_rawlog(LT_ERR, T("ERROR: Cannot open configuration file %s."),
		(conf && *conf) ? conf : "Unknown");
      return 0;
    }
    do_rawlog(LT_ERR, "Reading %s", conf);
  }

  fgets(tbuf1, BUFFER_LEN, fp);
  while (!feof(fp)) {

    p = tbuf1;

    if (*p == '#') {
      /* comment line */
      fgets(tbuf1, BUFFER_LEN, fp);
      continue;
    }
    /* this is a real line. Strip the end-of-line and characters following it.
     * Split the line into command and argument portions. If it exists,
     * also strip off the trailing comment. We try to make this work
     * whether the eol is \n (unix, yay), \r\n (dos/win, ew), or \r (mac, hmm)
     * This basically rules out embedded newlines as currently written
     */

    for (p = tbuf1; *p && (*p != '\n') && (*p != '\r'); p++) ;
    *p = '\0';			/* strip the end of line char(s) */
    for (p = tbuf1; *p && isspace((unsigned char) *p); p++)	/* strip spaces */
      ;
    for (q = p; *q && !isspace((unsigned char) *q); q++)	/* move over command */
      ;
    if (*q)
      *q++ = '\0';		/* split off command */
    for (; *q && isspace((unsigned char) *q); q++)	/* skip spaces */
      ;
    /* If the first character of the value is a #, and that is
       followed by a number, treat it as a dbref instead of a
       comment. */
    if (*q == '#' && isdigit((unsigned char) *(q + 1))) {
      for (s = q + 1; *s && (*s != '#'); s++)	/* look for a real comment */
	;
    } else {
      for (s = q; *s && (*s != '#'); s++)	/* look for comment */
	;
    }
    if (*s)			/* if found nuke it */
      *s = '\0';
    for (s = s - 1; (s >= q) && isspace((unsigned char) *s); s--)	/* smash trailing stuff */
      *s = '\0';

    if (strlen(p) != 0) {	/* skip blank lines */
      /* Handle include filename directives separetly */
      if (strcasecmp(p, "include") == 0) {
	conf_recursion++;
	if (conf_recursion > 10) {
	  do_rawlog(LT_ERR, T("CONFIG: include depth too deep in file %s"),
		    conf);
	} else {
	  config_file_startup(q, restrictions);
	}
	conf_recursion--;
      } else
	config_set(p, q, 0, restrictions);
    }
    fgets(tbuf1, BUFFER_LEN, fp);
  }

  /* Warn about any config options that aren't overridden by the
   * config file.
   */
  if (conf_recursion == 0) {
    for (cp = conftable; cp->name; cp++) {
      if (!cp->overridden) {
	do_rawlog(LT_ERR,
		  T
		  ("CONFIG: directive '%s' missing from cnf file, using default value."),
		  cp->name);
      }
    }

    /* these directives aren't player-settable but need to be initialized */
    mudtime = time(NULL);
    options.dump_counter = mudtime + options.dump_interval;
    options.purge_counter = mudtime + options.purge_interval;
    options.dbck_counter = mudtime + options.dbck_interval;
    options.warn_counter = mudtime + options.warn_interval;

#ifdef WIN32
    /* if we're on Win32, complain about compression */
    if ((options.compressprog && *options.compressprog)) {
      do_rawlog(LT_ERR,
		T
		("CONFIG: compression program is specified but not used in Win32, ignoring"),
		options.compressprog);
    }

    if (((options.compresssuff && *options.compresssuff))) {
      do_rawlog(LT_ERR,
		T
		("CONFIG: compression suffix is specified but not used in Win32, ignoring"),
		options.compresssuff);
    }

    /* Also remove the compression options */
    *options.uncompressprog = 0;
    *options.compressprog = 0;
    *options.compresssuff = 0;

#endif
#ifdef macintosh
    /* Copy Nick's Win32 code because we do the same thing */
    if ((options.compressprog && *options.compressprog)) {
      do_rawlog(LT_ERR,
		T
		("CONFIG: compression program is specified but not used in MacOS, ignoring"),
		options.compressprog);
    }

    if (((options.compresssuff && *options.compresssuff))) {
      do_rawlog(LT_ERR,
		T
		("CONFIG: compression suffix is specified but not used in MacOS, ignoring"),
		options.compresssuff);
    }

    /* Also remove the compression options */
    *options.uncompressprog = 0;
    *options.compressprog = 0;
    *options.compresssuff = 0;

    /* Now convert the UNIX paths specified to MacOS ones */
    PMConvertPath(options.input_db, options.input_db, 255);
    PMConvertPath(options.output_db, options.output_db, 255);
    PMConvertPath(options.crash_db, options.crash_db, 255);
#ifdef CHAT_SYSTEM
    PMConvertPath(options.chatdb, options.chatdb, 255);
#endif
    PMConvertPath(options.mail_db, options.mail_db, 255);
    PMConvertPath(options.connect_file[0], options.connect_file[0], 255);
    PMConvertPath(options.motd_file[0], options.motd_file[0], 255);
    PMConvertPath(options.wizmotd_file[0], options.wizmotd_file[0], 255);
    PMConvertPath(options.newuser_file[0], options.newuser_file[0], 255);
    PMConvertPath(options.register_file[0], options.register_file[0], 255);
    PMConvertPath(options.quit_file[0], options.quit_file[0], 255);
    PMConvertPath(options.down_file[0], options.down_file[0], 255);
    PMConvertPath(options.full_file[0], options.full_file[0], 255);
    PMConvertPath(options.guest_file[0], options.guest_file[0], 255);
    PMConvertPath(options.access_file, options.access_file, 255);
    PMConvertPath(options.names_file, options.names_file, 255);
#endif

  }
  fclose(fp);
  return 1;
}

/** List configuration directives or groups.
 * \param player the enactor.
 * \param type the directive or group name.
 * \param lc if 1, list in lowercase.
 */
void
do_config_list(dbref player, const char *type, int lc)
{
  CONFGROUP *cgp;
  CONF *cp;

  if (SUPPORT_PUEBLO)
    notify_noenter(player, tprintf("%cSAMP%c", TAG_START, TAG_END));
  if (type && *type) {
    /* Look up the type in the group table */
    int found = 0;
    for (cgp = confgroups; cgp->name; cgp++) {
      if (string_prefix(T(cgp->name), type)
	  && Can_View_Config_Group(player, cgp)) {
	found = 1;
	break;
      }
    }
    if (!found) {
      /* It wasn't a group. Is is one or more specific options? */
      for (cp = conftable; cp->name; cp++) {
	if (cp->group && string_prefix(cp->name, type)) {
	  notify(player, config_list_helper(player, cp, lc));
	  found = 1;
	}
      }
      if (!found) {
	/* Wasn't found at all. Ok. */
	notify(player, T("I only know the following types of options:"));
	for (cgp = confgroups; cgp->name; cgp++) {
	  if (Can_View_Config_Group(player, cgp))
	    notify_format(player, " %-15s %s", T(cgp->name), cgp->desc);
	}
      }
    } else {
      /* Show all entries of that type */
      notify(player, cgp->desc);
      if (string_prefix("compile", type))
	show_compile_options(player);
      else
	for (cp = conftable; cp->name; cp++) {
	  if (cp->group && !strcmp(cp->group, cgp->name)) {
	    notify(player, config_list_helper(player, cp, lc));
	  }
	}
    }
  } else {
    /* If we're here, we ran @config without a type. */
    notify(player,
	   T("Use: @config/list <type of options> where type is one of:"));
    for (cgp = confgroups; cgp->name; cgp++) {
      if (Can_View_Config_Group(player, cgp))
	notify_format(player, " %-15s %s", T(cgp->name), cgp->desc);
    }
  }
  if (SUPPORT_PUEBLO)
    notify_noenter(player, tprintf("%c/SAMP%c", TAG_START, TAG_END));
}

#define MAYBE_LC(x) (lc ? strlower(x) : x)
static char *
config_list_helper(dbref player __attribute__ ((__unused__)), CONF *cp, int lc)
{
  static char result[BUFFER_LEN];

  if ((cp->handler == cf_str) || (cp->handler == cf_flag))
    sprintf(result, " %-40s %s", MAYBE_LC(cp->name), (char *) cp->loc);
  else if (cp->handler == cf_int)
    sprintf(result, " %-40s %d", MAYBE_LC(cp->name), *((int *) cp->loc));
  else if (cp->handler == cf_bool)
    sprintf(result, " %-40s %s", MAYBE_LC(cp->name),
	    (*((int *) cp->loc) ? "Yes" : "No"));
  else if (cp->handler == cf_dbref)
    sprintf(result, " %-40s #%d", MAYBE_LC(cp->name), *((dbref *) cp->loc));
  return result;
}

/* This one doesn't return the names */
static char *
config_list_helper2(dbref player __attribute__ ((__unused__)), CONF *cp, int lc
		    __attribute__ ((__unused__)))
{
  static char result[BUFFER_LEN];
  if ((cp->handler == cf_str) || (cp->handler == cf_flag))
    sprintf(result, "%s", (char *) cp->loc);
  else if (cp->handler == cf_int)
    sprintf(result, "%d", *((int *) cp->loc));
  else if (cp->handler == cf_bool)
    sprintf(result, "%s", (*((int *) cp->loc) ? "Yes" : "No"));
  else if (cp->handler == cf_dbref)
    sprintf(result, "#%d", *((dbref *) cp->loc));
  return result;
}

#undef MAYBE_LC

/* config(option): returns value of option
 * config(): returns list of all option names
 */
FUNCTION(fun_config)
{
  CONF *cp;

  /* Never reached, since maxargs == 1 in the function table.
     On second thought after writing it, I think having config()
     set options as well as @config/set is a mistake. */
  if (nargs == 2) {		/* Set the option */
#ifdef FUNCTION_SIDE_EFFECTS
    if (!Wizard(executor)) {
      safe_str(T(e_perm), buff, bp);
      return;
    }
    if (!args[0] || !*args[0]) {
      safe_str(T("#-1 NO SUCH OPTION"), buff, bp);
      return;
    }
    if (!config_set(args[0], args[1], 1, 0)
	&& !config_set(args[0], args[1], 1, 1)) {
      safe_str(T("#-1 UNABLE TO SET OPTION"), buff, bp);
      return;
    }
    /* Return the new value */
#else
    safe_str(T("#-1 TWO-ARGUMENT FORM OF FUNCTION DISABLED"), buff, bp);
    return;
#endif
  }

  if (args[0] && *args[0]) {
    for (cp = conftable; cp->name; cp++) {
      if (cp->group && !strcasecmp(cp->name, args[0])) {
	safe_str(config_list_helper2(executor, cp, 0), buff, bp);
	return;
      }
    }
    safe_str(T("#-1 NO SUCH CONFIG OPTION"), buff, bp);
    return;
  } else {
    for (cp = conftable; cp->name; cp++) {
      safe_str(cp->name, buff, bp);
      safe_chr(' ', buff, bp);
    }
  }
}

/** Enable or disable a configuration option.
 * \param player the enactor.
 * \param param the option to enable/disable.
 * \param state 1 to enable, 0 to disable.
 */
void
do_enable(dbref player, const char *param, int state)
{
  CONF *cp;

  for (cp = conftable; cp->name; cp++) {
    if (cp->group && !strcasecmp(cp->name, param)) {
      if (cp->handler == cf_bool) {
	cf_bool(param, (state ? "yes" : "no"), cp->loc, cp->max, 1);
	if (state == 0)
	  notify(player, T("Disabled."));
	else
	  notify(player, T("Enabled."));
	do_log(LT_WIZ, player, NOTHING, "%s %s",
	       cp->name, (state) ? "ENABLED" : "DISABLED");
      } else
	notify(player, T("That isn't a on/off option."));
      return;
    }
  }
  notify(player, T("No such option."));
}

static void
show_compile_options(dbref player)
{
#if (CRYPT_SYSTEM == 0)
  notify(player, T(" Passwords are not stored encrypted."));
#endif
#if (CRYPT_SYSTEM == 2)
  notify(player, T(" Passwords are encrypted with SHS."));
#endif
#if (CRYPT_SYSTEM == 3) || (CRYPT_SYSTEM == 1)
  notify(player, T(" Passwords are converted from system crypt to SHS"));
#endif
#if (CRYPT_SYSTEM == 4)
  notify(player, T(" Passwords are converted from plaintext to SHS"));
#endif
#if (COMPRESSION_TYPE == 0)
  notify(player, T(" Attributes are not compressed in memory."));
#endif
#if (COMPRESSION_TYPE == 1) || (COMPRESSION_TYPE == 2)
  notify(player, T(" Attributes are Huffman compressed in memory."));
#endif
#if (COMPRESSION_TYPE == 3)
  notify(player, T(" Attributes are word compressed in memory."));
#endif
#if (COMPRESSION_TYPE == 4)
  notify(player, T(" Attributes are 8-bit word compressed in memory."));
#endif

#ifdef SINGLE_LOGFILE
  notify(player, T(" Logging is to a single log file."));
#else
  notify(player, T(" Logging is to multiple log files."));
#endif

#ifdef MEM_CHECK
  notify(player, " The memory allocation tracker is operating.");
#endif

#ifdef INFO_SLAVE
  notify(player, T(" DNS and ident lookups are handled by a slave process."));
#else
  notify(player, T(" DNS and ident lookups are handled by the MUSH process."));
#endif

#ifdef NT_TCP
  notify(player, T(" Windows NT native TCP/IP networking functions in use."));
#else
  notify(player, T(" BSD sockets networking in use."));
#endif

  notify(player, T(" Floating point functions are enabled."));

#ifdef FUNCTION_SIDE_EFFECTS
  notify(player, T(" Side effect functions are enabled."));
#else
  notify(player, T(" Side effect functions are disabled."));
#endif

#ifdef USE_MAILER
  notify(player,
	 T(" The extended built-in MUSH mailing system is being used."));
#ifdef ALLOW_NOSUBJECT
  notify(player,
	 T(" Messages without subject lines have subject (no subject)"));
#else
  notify(player,
	 T
	 (" Messages without subject lines use the beginning of the message as subject"));
#endif
#else
  notify(player, T(" The built-in MUSH mailing system is not being used."));
#endif

#ifdef CHAT_SYSTEM
  notify(player, T(" The extended chat system is enabled."));
#else
  notify(player, T(" The chat system is disabled."));
#endif

#ifdef QUOTA
  notify(player, T(" Quota restrictions are being enforced."));
#endif


#ifdef EMPTY_ATTRS
  notify(player, T(" Empty attributes are kept."));
#ifdef VISIBLE_EMPTY_ATTRS
  notify(player, T(" Empty attributes are visible."));
#else
  notify(player, T(" Empty attributes are not visible."));
#endif
#ifdef DELETE_ATTRS
  notify(player, T(" Attributes can be deleted."));
#else
  notify(player, T(" Attributes cannot be deleted."));
#endif
#else
  notify(player, T(" Empty attributes are deleted."));
#endif

#ifdef ONLINE_REG
  notify(player, T(" The UNREGISTERED flag is enabled."));
#endif
#ifdef VACATION_FLAG
  notify(player, T(" The VACATION flag is enabled."));
#endif
#ifdef UNINSPECTED_FLAG
  notify(player, T(" The UNINSPECTED flag is enabled."));
#endif

#ifdef HAS_GETDATE
  notify(player, T(" Extended convtime() is supported."));
#else
  notify(player, T(" convtime() is stricter."));
#endif

#if defined(HAS_ITIMER) || defined(WIN32)
  notify(player, T(" CPU usage limiting is supported."));
#else
  notify(player, T(" CPU usage limiting is NOT supported."));
#endif

}
