/* conf.c */

/* configuration adjustment. Some of the ideas and bits and pieces of the
 * code here are based on TinyMUSH 2.0.
 */

#include "copyrite.h"
#include "config.h"

#include <stdio.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#include <ctype.h>

#include "conf.h"
#include "externs.h"
#include "intrface.h"
#include "pueblo.h"
#include "mushdb.h"
#include "confmagic.h"

time_t mudtime;			/* game time, in seconds */

extern object_flag_type find_flag _((char *name, int type, int *toggle, int is_conf));
extern const char *togglemask_to_string _((int type, object_flag_type mask));
extern int restrict_command _((const char *name, const char *restriction));
static void show_compile_options _((dbref player));
static void config_list_helper _((dbref player, CONF *cp));

OPTTAB options;


void cf_bool _((const char *opt, char *val, int *loc, int maxval));
void cf_str _((const char *opt, char *val, int *loc, int maxlen));
void cf_int _((const char *opt, char *val, int *loc, int maxval));
void cf_flag _((const char *opt, char *val, int *loc, int maxval));
void config_set _((const char *opt, char *val));
void conf_default_set _((void));
int config_file_startup _((const char *conf));

CONF conftable[] =
{
  {"mud_name", cf_str, (int *) options.mud_name, 128, 0, "net"},
  {"port", cf_int, &options.port, 32000, 0, "net"},
  {"use_dns", cf_bool, &options.use_dns, 2, 0, "net"},
  {"ip_addr", cf_str, (int *) options.ip_addr, 64, 0, "net"},
  {"input_database", cf_str, (int *) options.input_db, 256, 0, "files"},
  {"output_database", cf_str, (int *) options.output_db, 256, 0, "files"},
  {"crash_database", cf_str, (int *) options.crash_db, 256, 0, "files"},
  {"mail_database", cf_str, (int *) options.mail_db, 256, 0, "files"},
  {"player_start", cf_int, &options.player_start, 100000, 0, "db"},
  {"master_room", cf_int, &options.master_room, 100000, 0, "db"},
  {"base_room", cf_int, &options.base_room, 100000, 0, "db"},
  {"idle_timeout", cf_int, &options.idle_timeout, 100000, 0, "limits"},
  {"dump_interval", cf_int, &options.dump_interval, 100000, 0, "dump"},
  {"dump_message", cf_str, (int *) options.dump_message, 256, 0, "dump"},
{"dump_complete", cf_str, (int *) options.dump_complete, 256, 0, "dump"},
  {"ident_timeout", cf_int, &options.ident_timeout, 60, 0, "net"},
#ifdef TREKMUSH
  {"max_logins", cf_int, &options.max_logins, 250, 0, "limits"},
#else
  {"max_logins", cf_int, &options.max_logins, 128, 0, "limits"},
#endif
{"whisper_loudness", cf_int, &options.whisper_loudness, 100, 0, "limits"},
  {"blind_page", cf_bool, &options.blind_page, 2, 0, "cosmetic"},
  {"paycheck", cf_int, &options.paycheck, 1000, 0, "limits"},
  {"starting_money", cf_int, &options.starting_money, 10000, 0, "limits"},
  {"starting_quota", cf_int, &options.starting_quota, 10000, 0, "limits"},
  {"player_queue_limit", cf_int, &options.player_queue_limit, 100000, 0, "limits"},
  {"queue_chunk", cf_int, &options.queue_chunk, 100000, 0, "limits"},
  {"active_queue_chunk", cf_int, &options.active_q_chunk, 100000, 0, "limits"},
  {"function_recursion_limit", cf_int, &options.func_nest_lim, 100000, 0, "limits"},
  {"function_invocation_limit", cf_int, &options.func_invk_lim, 100000, 0, "limits"},
  {"log_wipe_passwd", cf_str, (int *) options.log_wipe_passwd, 256, 0, NULL},
  {"money_singular", cf_str, (int *) options.money_singular, 32, 0, "cosmetic"},
{"money_plural", cf_str, (int *) options.money_plural, 32, 0, "cosmetic"},
{"compress_suffix", cf_str, (int *) options.compresssuff, 32, 0, "files"},
  {"compress_program", cf_str, (int *) options.compressprog, 256, 0, "files"},
  {"uncompress_program", cf_str, (int *) options.uncompressprog, 256, 0, "files"},
  {"help_file", cf_str, (int *) options.help_file, 256, 0, "files"},
  {"help_index", cf_str, (int *) options.help_index, 256, 0, "files"},
  {"news_file", cf_str, (int *) options.news_file, 256, 0, "files"},
  {"news_index", cf_str, (int *) options.news_index, 256, 0, "files"},
  {"events_file", cf_str, (int *) options.events_file, 256, 0, "files"},
  {"events_index", cf_str, (int *) options.events_index, 256, 0, "files"},
#ifdef CHAT_SYSTEM
  {"chat_database", cf_str, (int *) options.chatdb, 256, 0, "files"},
  {"max_player_chans", cf_int, &options.max_player_chans, 100, 0, "chat"},
  {"max_channels", cf_int, &options.max_channels, 1000, 0, "chat"},
  {"chan_cost", cf_int, &options.chan_cost, 10000, 0, "chat"},
#endif
  {"connect_file", cf_str, (int *) options.connect_file[0], 256, 0, "files"},
  {"motd_file", cf_str, (int *) options.motd_file[0], 256, 0, "files"},
  {"wizmotd_file", cf_str, (int *) options.wizmotd_file[0], 256, 0, "files"},
  {"newuser_file", cf_str, (int *) options.newuser_file[0], 256, 0, "files"},
  {"register_create_file", cf_str, (int *) options.register_file[0], 256, 0, "files"},
  {"quit_file", cf_str, (int *) options.quit_file[0], 256, 0, "files"},
  {"down_file", cf_str, (int *) options.down_file[0], 256, 0, "files"},
  {"full_file", cf_str, (int *) options.full_file[0], 256, 0, "files"},
  {"guest_file", cf_str, (int *) options.guest_file[0], 256, 0, "files"},
  {"connect_html_file", cf_str, (int *) options.connect_file[1], 256, 0, "files"},
{"motd_html_file", cf_str, (int *) options.motd_file[1], 256, 0, "files"},
  {"wizmotd_html_file", cf_str, (int *) options.wizmotd_file[1], 256, 0, "files"},
  {"newuser_html_file", cf_str, (int *) options.newuser_file[1], 256, 0, "files"},
  {"register_create_html_file", cf_str, (int *) options.register_file[1], 256, 0, "files"},
{"quit_html_file", cf_str, (int *) options.quit_file[1], 256, 0, "files"},
{"down_html_file", cf_str, (int *) options.down_file[1], 256, 0, "files"},
{"full_html_file", cf_str, (int *) options.full_file[1], 256, 0, "files"},
  {"guest_html_file", cf_str, (int *) options.guest_file[1], 256, 0, "files"},
  {"log_commands", cf_bool, &options.log_commands, 2, 0, "log"},
  {"log_huhs", cf_bool, &options.log_huhs, 2, 0, "log"},
  {"log_forces", cf_bool, &options.log_forces, 2, 0, "log"},
  {"log_walls", cf_bool, &options.log_walls, 2, 0, "log"},
  {"pueblo", cf_bool, &options.support_pueblo, 2, 0, "net"},
  {"logins", cf_bool, &options.login_allow, 2, 0, "net"},
  {"player_creation", cf_bool, &options.create_allow, 2, 0, "net"},
  {"guests", cf_bool, &options.guest_allow, 2, 0, "net"},
  {"daytime", cf_bool, &options.daytime, 2, 0, "cmds"},
  {"player_flags", cf_flag, &options.player_flags, 64, 0, "flags"},
  {"room_flags", cf_flag, &options.room_flags, 64, 0, "flags"},
  {"exit_flags", cf_flag, &options.exit_flags, 64, 0, "flags"},
  {"thing_flags", cf_flag, &options.thing_flags, 64, 0, "flags"},
  {"rwho", cf_bool, &options.use_rwho, 2, 0, "rwho"},
{"rwho_dump_interval", cf_int, &options.rwho_interval, 32000, 0, "rwho"},
  {"rwho_info_port", cf_int, &options.rwho_port, 32000, 0, "rwho"},
  {"rwho_host", cf_str, (int *) options.rwho_host, 64, 0, "rwho"},
  {"rwho_password", cf_str, (int *) options.rwho_pass, 64, 0, NULL},
#ifdef USE_WARNINGS
  {"warn_interval", cf_int, &options.warn_interval, 32000, 0, "dump"},
#endif
  {"haspower_restricted", cf_bool, &options.haspower_restricted, 2, 0, "funcs"},
  {"safer_ufun", cf_bool, &options.safer_ufun, 2, 0, "funcs"},
#ifdef TREKMUSH
  {"dump_warning_10sec", cf_str, (int *) options.dump_warning_10sec, 256, 0, "dump"},
#endif /* TREKMUSH */
  {"dump_warning_1min", cf_str, (int *) options.dump_warning_1min, 256, 0, "dump"},
  {"dump_warning_5min", cf_str, (int *) options.dump_warning_5min, 256, 0, "dump"},
  {"index_file", cf_str, (int *) options.index_file, 256, 0, "files"},
  {"index_index", cf_str, (int *) options.index_index, 256, 0, "files"},
  {"rules_file", cf_str, (int *) options.rules_file, 256, 0, "files"},
  {"rules_index", cf_str, (int *) options.rules_index, 256, 0, "files"},
  {"hate_dest", cf_bool, &options.hate_dest, 2, 0, "cmds"},
  {"noisy_whisper", cf_bool, &options.noisy_whisper, 2, 0, "cmds"},
  {"possessive_get", cf_bool, &options.possessive_get, 2, 0, "cmds"},
  {"possessive_get_d", cf_bool, &options.possessive_get_d, 2, 0, "cmds"},
  {"really_safe", cf_bool, &options.really_safe, 2, 0, "cmds"},
  {"destroy_possessions", cf_bool, &options.destroy_possessions, 2, 0, "cmds"},
  {"null_eq_zero", cf_bool, &options.null_eq_zero, 2, 0, "tiny"},
  {"tiny_booleans", cf_bool, &options.tiny_booleans, 2, 0, "tiny"},
  {"tiny_trim_fun", cf_bool, &options.tiny_trim_fun, 2, 0, "tiny"},
  {"tiny_math", cf_bool, &options.tiny_math, 2, 0, "tiny"},
  {"adestroy", cf_bool, &options.adestroy, 2, 0, "attribs"},
  {"amail", cf_bool, &options.amail, 2, 0, "attribs"},
  {"player_listen", cf_bool, &options.player_listen, 2, 0, "attribs"},
  {"player_ahear", cf_bool, &options.player_ahear, 2, 0, "attribs"},
  {"startups", cf_bool, &options.startups, 2, 0, "attribs"},
  {"room_connects", cf_bool, &options.room_connects, 2, 0, "attribs"},
  {"reverse_shs", cf_bool, &options.reverse_shs, 2, 0, "attribs"},
  {"ansi_names", cf_bool, &options.ansi_names, 2, 0, "cosmetic"},
  {"ansi_justify", cf_bool, &options.ansi_justify, 2, 0, "cosmetic"},
{"comma_exit_list", cf_bool, &options.comma_exit_list, 2, 0, "cosmetic"},
  {"count_all", cf_bool, &options.count_all, 2, 0, "cosmetic"},
  {"exits_connect_rooms", cf_bool, &options.exits_connect_rooms, 2, 0, "db"},
  {"link_to_object", cf_bool, &options.link_to_object, 2, 0, "cmds"},
  {"owner_queues", cf_bool, &options.owner_queues, 2, 0, "cmds"},
  {"wiz_noaenter", cf_bool, &options.wiz_noaenter, 2, 0, "cmds"},
  {"use_ident", cf_bool, &options.use_ident, 2, 0, "net"},
  {"player_name_spaces", cf_bool, &options.player_name_spaces, 2, 0, "cosmetic"},
  {"forking_dump", cf_bool, &options.forking_dump, 2, 0, "dump"},
  {"military_time", cf_bool, &options.military_time, 2, 0, "cosmetic"},
  {"restricted_building", cf_bool, &options.restrict_building, 2, 0, "cmds"},
  {"free_objects", cf_bool, &options.free_objects, 2, 0, "cmds"},
  {"flags_on_examine", cf_bool, &options.flags_on_examine, 2, 0, "cosmetic"},
  {"ex_public_attribs", cf_bool, &options.ex_public_attribs, 2, 0, "cosmetic"},
  {"tiny_attrs", cf_bool, &options.tiny_attrs, 2, 0, "tiny"},
  {"full_invis", cf_bool, &options.full_invis, 2, 0, "cmds"},
  {"silent_pemit", cf_bool, &options.silent_pemit, 2, 0, "tiny"},
  {"player_locate", cf_bool, &options.player_locate, 2, 0, "cmds"},
  {"globals", cf_bool, &options.globals, 2, 0, "cmds"},
  {"global_connects", cf_bool, &options.global_connects, 2, 0, "attribs"},
  {"paranoid_nospoof", cf_bool, &options.paranoid_nospoof, 2, 0, "cosmetic"},
  {"max_dbref", cf_int, &options.max_dbref, -1, 0, "limits"},
  {"wizwall_prefix", cf_str, (int *) options.wizwall_prefix, 256, 0, "cosmetic"},
  {"rwall_prefix", cf_str, (int *) options.rwall_prefix, 256, 0, "cosmetic"},
{"wall_prefix", cf_str, (int *) options.wall_prefix, 256, 0, "cosmetic"},
  {"access_file", cf_str, (int *) options.access_file, 256, 0, "files"},
  {"names_file", cf_str, (int *) options.names_file, 256, 0, "files"},
  {"object_cost", cf_int, &options.object_cost, 10000, 0, "costs"},
  {"exit_cost", cf_int, &options.exit_cost, 10000, 0, "costs"},
  {"link_cost", cf_int, &options.link_cost, 10000, 0, "costs"},
  {"room_cost", cf_int, &options.room_cost, 10000, 0, "costs"},
  {"queue_cost", cf_int, &options.queue_cost, 10000, 0, "costs"},
  {"quota_cost", cf_int, &options.quota_cost, 10000, 0, "costs"},
  {"find_cost", cf_int, &options.find_cost, 10000, 0, "costs"},
  {"page_cost", cf_int, &options.page_cost, 10000, 0, "costs"},
  {"kill_default_cost", cf_int, &options.kill_default_cost, 10000, 0, "costs"},
  {"kill_min_cost", cf_int, &options.kill_min_cost, 10000, 0, "costs"},
  {"kill_bonus", cf_int, &options.kill_bonus, 10000, 1, "costs"},
  {"queue_loss", cf_int, &options.queue_loss, 10000, 0, "limits"},
  {"max_pennies", cf_int, &options.max_pennies, 10000, 1, "limits"},
  {"max_depth", cf_int, &options.max_depth, 10000, 1, "limits"},
  {"max_parents", cf_int, &options.max_parents, 10000, 0, "limits"},
  {"purge_interval", cf_int, &options.purge_interval, 10000, 0, "dump"},
  {"dbck_interval", cf_int, &options.dbck_interval, 10000, 0, "dump"},
  {NULL, NULL, NULL, 0, 0, NULL}
};

typedef struct confgroupparm CONFGROUP;

struct confgroupparm {
  const char *name;		/* name of group */
  const char *desc;		/* description of group */
  int viewperms;		/* who can view this group */
};

CONFGROUP confgroups[] =
{
  {"attribs", "Options affecting attributes", 0},
  {"chat", "Chat system options", 0},
  {"cmds", "Options affecting command behavior", 0},
  {"compile", "Compile-time options", 0},
  {"cosmetic", "Cosmetic options", 0},
  {"costs", "Costs", 0},
  {"db", "Database options", 0},
  {"dump", "Options affecting dumps and other periodic processes", 0},
  {"files", "Files used by the MUSH", CGP_GOD},
  {"funcs", "Options affecting function behavior", 0},
  {"limits", "Limits and other constants", 0},
  {"log", "Logging options", 0},
  {"net", "Networking and connection-related options", 0},
  {"rwho", "RWHO options", 0},
  {"tiny", "TinyMUSH compatibility options", 0},
  {NULL, NULL, 0}
};

void
cf_bool(opt, val, loc, maxval)
    const char *opt;
    char *val;
    int *loc;
    int maxval;
{
  /* enter boolean parameter */

  if (!strcasecmp(val, "yes") || !strcasecmp(val, "true") ||
      !strcasecmp(val, "1"))
    *loc = 1;
  else if (!strcasecmp(val, "no") || !strcasecmp(val, "false") ||
	   !strcasecmp(val, "0"))
    *loc = 0;
  else {
    fprintf(stderr, "CONFIG: option %s value %s invalid.\n", opt, val);
    fflush(stderr);
  }
}


void
cf_str(opt, val, loc, maxlen)
    const char *opt;
    char *val;
    int *loc;
    int maxlen;
{
  /* enter string parameter */

  /* truncate if necessary */
  if (strlen(val) >= (Size_t) maxlen) {
    val[maxlen - 1] = '\0';
    fprintf(stderr, "CONFIG: option %s value truncated\n", opt);
    fflush(stderr);
  }
  strcpy((char *) loc, val);
}


void
cf_int(opt, val, loc, maxval)
    const char *opt;
    char *val;
    int *loc;
    int maxval;
{
  /* enter integer parameter */

  int n;

  n = atoi(val);

  /* enforce limits */
  if ((maxval >= 0) && (n > maxval)) {
    n = maxval;
    fprintf(stderr, "CONFIG: option %s value limited to %d\n",
	    opt, maxval);
    fflush(stderr);
  }
  *loc = n;
}

void
cf_flag(opt, val, loc, maxval)
    const char *opt;
    char *val;
    int *loc;
    int maxval;
{
  /* set default flags */

  int f = -1;
  int toggle;

  /* figure out what flag type we're setting */

  switch (opt[0]) {
  case 'p':
    f = find_flag(val, TYPE_PLAYER, &toggle, 1);
    break;
  case 'r':
    f = find_flag(val, TYPE_ROOM, &toggle, 1);
    break;
  case 'e':
    f = find_flag(val, TYPE_EXIT, &toggle, 1);
    break;
  case 't':
    f = find_flag(val, TYPE_THING, &toggle, 1);
    break;
  default:
    fprintf(stderr, "CONFIG: weird flag set directive '%s'\n", opt);
  }

  if (f == -1) {
    fprintf(stderr, "CONFIG: flag '%s' cannot be set.\n", val);
    return;
  }
  if (f == -2) {
    fprintf(stderr, "CONFIG: flag '%s' for type not found.\n", val);
    return;
  }
  if (!toggle)
    *loc |= f;
  else {
    switch (opt[0]) {
    case 'p':
      options.player_toggles |= f;
      break;
    case 'r':
      options.room_toggles |= f;
      break;
    case 'e':
      options.exit_toggles |= f;
      break;
    case 't':
      options.thing_toggles |= f;
      break;
    }
  }
}

void
config_set(opt, val)
    const char *opt;
    char *val;
{
  CONF *cp;
  char *p;

  /* Was this "restrict_command <command> <restriction>"? If so, do it */
  if (!strcasecmp(opt, "restrict_command")) {
    for (p = val; *p && !isspace(*p); p++) ;
    if (*p) {
      *p++ = '\0';
      if (!restrict_command(val, p)) {
	fprintf(stderr, "CONFIG: Invalid command or restriction for %s.\n", val);
	fflush(stderr);
      }
    } else {
      fprintf(stderr, "CONFIG: restrict_command %s requires a restriction value.\n", val);
      fflush(stderr);
    }
    return;
  }
  /* search conf table for the option; if found, add it, if not found,
   * complain about it.
   */
  for (cp = conftable; cp->name; cp++) {
    if (!strcmp(cp->name, opt)) {
      cp->handler(opt, val, cp->loc, cp->max);
      cp->overridden = 1;
      return;
    }
  }

  fprintf(stderr, "CONFIG: directive '%s' in cnf file ignored.\n", opt);
  fflush(stderr);
}

void
conf_default_set()
{
  strcpy(options.mud_name, "TinyMUSH");
  options.port = 4201;
  strcpy(options.input_db, "data/indb.Z");
  strcpy(options.output_db, "data/outdb.Z");
  strcpy(options.crash_db, "data/PANIC.db");
#ifdef CHAT_SYSTEM
  strcpy(options.chatdb, "data/chatdb.Z");
  options.chan_cost = 1000;
  options.max_player_chans = 3;
  options.max_channels = 200;
#endif
  strcpy(options.mail_db, "data/maildb.Z");
  options.player_start = 0;
  options.master_room = 2;
  options.base_room = 0;
  options.idle_timeout = 10801;
  options.dump_interval = 3601;
  strcpy(options.dump_message, "GAME: Dumping database. Game may freeze for a minute");
  strcpy(options.dump_complete, "GAME: Dump complete. Time in.");
  options.ident_timeout = 5;
  options.max_logins = 128;
  options.whisper_loudness = 100;
  options.blind_page = 1;
  options.paycheck = 50;
  options.starting_money = 100;
  options.starting_quota = 20;
  options.player_queue_limit = 100;
  options.queue_chunk = 3;
  options.active_q_chunk = 0;
  options.func_nest_lim = 50;
  options.func_invk_lim = 2500;
  strcpy(options.money_singular, "Penny");
  strcpy(options.money_plural, "Pennies");
  strcpy(options.log_wipe_passwd, "zap!");
  strcpy(options.compressprog, "compress");
  strcpy(options.uncompressprog, "uncompress");
  strcpy(options.compresssuff, ".Z");
  strcpy(options.help_file, "txt/help.txt");
  strcpy(options.help_index, "txt/help.idx");
  strcpy(options.news_file, "txt/news.txt");
  strcpy(options.news_index, "txt/news.idx");
  strcpy(options.events_file, "");
  strcpy(options.events_index, "");
  strcpy(options.connect_file[0], "txt/connect.txt");
  strcpy(options.motd_file[0], "txt/motd.txt");
  strcpy(options.wizmotd_file[0], "txt/wizmotd.txt");
  strcpy(options.newuser_file[0], "txt/newuser.txt");
  strcpy(options.register_file[0], "txt/register.txt");
  strcpy(options.quit_file[0], "txt/quit.txt");
  strcpy(options.down_file[0], "txt/down.txt");
  strcpy(options.full_file[0], "txt/full.txt");
  strcpy(options.guest_file[0], "txt/guest.txt");
  strcpy(options.connect_file[1], "txt/connect.html");
  strcpy(options.motd_file[1], "txt/motd.html");
  strcpy(options.wizmotd_file[1], "txt/wizmotd.html");
  strcpy(options.newuser_file[1], "txt/newuser.html");
  strcpy(options.register_file[1], "txt/register.html");
  strcpy(options.quit_file[1], "txt/quit.html");
  strcpy(options.down_file[1], "txt/down.html");
  strcpy(options.full_file[1], "txt/full.html");
  strcpy(options.guest_file[1], "txt/guest.html");
  options.log_commands = 0;
  options.log_huhs = 0;
  options.log_forces = 1;
  options.log_walls = 0;
  options.support_pueblo = 0;
  options.login_allow = 1;
  options.guest_allow = 1;
  options.create_allow = 1;
  options.daytime = 0;
  options.player_flags = 0;
  options.room_flags = 0;
  options.exit_flags = 0;
  options.thing_flags = 0;
  options.player_toggles = 0;
  options.room_toggles = 0;
  options.exit_toggles = 0;
  options.thing_toggles = 0;
  options.use_rwho = 0;
  options.rwho_interval = 241;
  options.rwho_port = 6889;
  strcpy(options.rwho_host, "littlewood.math.okstate.edu");
  strcpy(options.rwho_pass, "getyours");
#ifdef USE_WARNINGS
  options.warn_interval = 3600;
#endif
  options.use_dns = 1;
  options.haspower_restricted = 0;
  options.safer_ufun = 1;
#ifdef TREKMUSH
  strcpy(options.dump_warning_10sec, "GAME: Database will be dumped in 10 seconds.");
#endif /* TREKMUSH */
  strcpy(options.dump_warning_1min, "GAME: Database will be dumped in 1 minute.");
  strcpy(options.dump_warning_5min, "GAME: Database will be dumped in 5 minutes.");
  strcpy(options.index_file, "");
  strcpy(options.index_index, "");
  strcpy(options.rules_file, "");
  strcpy(options.rules_index, "");
  options.hate_dest = 0;
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
  options.player_listen = 1;
  options.player_ahear = 1;
  options.startups = 1;
  options.room_connects = 0;
  options.reverse_shs = 1;
  options.ansi_names = 1;
  options.ansi_justify = 1;
  options.comma_exit_list = 1;
  options.count_all = 0;
  options.exits_connect_rooms = 0;
  options.link_to_object = 1;
  options.owner_queues = 0;
  options.wiz_noaenter = 0;
  options.use_ident = 1;
  strcpy(options.ip_addr, "");
  options.player_name_spaces = 0;
  options.forking_dump = 1;
  options.military_time = 1;
  options.restrict_building = 0;
  options.free_objects = 1;
  options.flags_on_examine = 1;
  options.ex_public_attribs = 1;
  options.tiny_attrs = 0;
  options.full_invis = 0;
  options.silent_pemit = 0;
  options.player_locate = 1;
  options.globals = 1;
  options.global_connects = 1;
  options.paranoid_nospoof = 0;
  options.max_dbref = 0;
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
  options.max_depth = 10;
  options.max_parents = 10;
  options.purge_interval = 601;
  options.dbck_interval = 599;

}

int
config_file_startup(conf)
    const char *conf;
{
  /* read a configuration file. Return 0 on failure, 1 on success */

  FILE *fp;
  CONF *cp;
  char tbuf1[BUFFER_LEN];
  char *p, *q, *s;

  static char cfile[BUFFER_LEN];	/* Remember the last one */
  if (conf && *conf)
    strcpy(cfile, conf);
  fp = fopen(cfile, "r");

  if (fp == NULL) {
    do_rawlog(LT_ERR, "ERROR: Cannot open configuration file %s.\n",
	      cfile);
    return 0;
  }
  conf_default_set();		/* initialize defaults */

  fgets(tbuf1, BUFFER_LEN, fp);
  while (!feof(fp)) {

    p = tbuf1;

    if (*p == '#') {
      /* comment line */
      fgets(tbuf1, BUFFER_LEN, fp);
      continue;
    }
    /* this is a real line. Strip the newline and characters following it.
     * Split the line into command and argument portions. If it exists,
     * also strip off the trailing comment.
     */

    for (p = tbuf1; *p && (*p != '\n'); p++) ;
    *p = '\0';			/* strip '\n' */
    for (p = tbuf1; *p && isspace(*p); p++)	/* strip spaces */
      ;
    for (q = p; *q && !isspace(*q); q++)	/* move over command */
      ;
    if (*q)
      *q++ = '\0';		/* split off command */
    for (; *q && isspace(*q); q++)	/* skip spaces */
      ;
    for (s = q; *s && (*s != '#'); s++)		/* look for comment */
      ;
    if (*s)			/* if found nuke it */
      *s = '\0';
    for (s = s - 1; (s >= q) && isspace(*s); s--)	/* smash trailing stuff */
      *s = '\0';

    if (strlen(p) != 0)		/* skip blank lines */
      config_set(p, q);

    fgets(tbuf1, BUFFER_LEN, fp);
  }

  /* Warn about any config options that aren't overridden by the
   * config file.
   */
  for (cp = conftable; cp->name; cp++) {
    if (!cp->overridden) {
      fprintf(stderr, "CONFIG: directive '%s' missing from cnf file, using default value.\n", cp->name);
      fflush(stderr);
    }
  }


  /* these directives aren't player-settable but need to be initialized */
  mudtime = time(NULL);
  options.dump_counter = mudtime + options.dump_interval;
  if (options.use_rwho)
    options.rwho_counter = mudtime + options.rwho_interval;
#ifdef USE_WARNINGS
  options.warn_counter = mudtime + options.warn_interval;
#endif

  fclose(fp);
  return 1;
}


void
do_config_list(player, type)
    dbref player;
    const char *type;
{
  CONFGROUP *cgp;
  CONF *cp;

  if (SUPPORT_PUEBLO)
    notify_noenter(player, tprintf("%cPRE%c", TAG_START, TAG_END));
  if (type && *type) {
    /* Look up the type in the group table */
    int found = 0;
    for (cgp = confgroups; cgp->name; cgp++) {
      if (string_prefix(cgp->name, type) && Can_View_Config_Group(player, cgp)) {
	found = 1;
	break;
      }
    }
    if (!found) {
      /* It wasn't a group. Is is one or more specific options? */
      for (cp = conftable; cp->name; cp++) {
	if (cp->group && string_prefix(cp->name, type)) {
	  config_list_helper(player, cp);
	  found = 1;
	}
      }
      if (!found) {
	/* Wasn't found at all. Ok. */
	notify(player, "I only know the following types of options:");
	for (cgp = confgroups; cgp->name; cgp++) {
	  if (Can_View_Config_Group(player, cgp))
	    notify(player, tprintf(" %-15s %s", cgp->name, cgp->desc));
	}
      }
    } else {
      /* Show all entries of that type */
      notify(player, cgp->desc);
      if (!strcasecmp(type, "compile"))
	show_compile_options(player);
      else
	for (cp = conftable; cp->name; cp++) {
	  if (cp->group && !strcmp(cp->group, cgp->name)) {
	    config_list_helper(player, cp);
	  }
	}
    }
  } else {
    /* If we're here, we ran @config without a type. */
    notify(player, "Use: @config/list <type of options> where type is one of:");
    for (cgp = confgroups; cgp->name; cgp++) {
      if (Can_View_Config_Group(player, cgp))
	notify(player, tprintf(" %-15s %s", cgp->name, cgp->desc));
    }
  }
  if (SUPPORT_PUEBLO)
    notify_noenter(player, tprintf("%c/PRE%c", TAG_START, TAG_END));
}

static void
config_list_helper(player, cp)
    dbref player;
    CONF *cp;
{
  if (cp->handler == cf_str)
    notify(player, tprintf(" %-40s %s", cp->name, (char *) cp->loc));
  else if (cp->handler == cf_int)
    notify(player, tprintf(" %-40s %d", cp->name, *cp->loc));
  else if (cp->handler == cf_bool)
    notify(player, tprintf(" %-40s %s", cp->name,
			   (*cp->loc ? "Yes" : "No")));
  else if (cp->handler == cf_flag) {
    int type = NOTYPE;
    switch (cp->name[0]) {
    case 'p':
      type = TYPE_PLAYER;
      break;
    case 'r':
      type = TYPE_ROOM;
      break;
    case 'e':
      type = TYPE_EXIT;
      break;
    case 't':
      type = TYPE_THING;
      break;
    }
    notify(player, tprintf(" %-40s %s", cp->name,
			   togglemask_to_string(type, *cp->loc)));
  }
}

static void
show_compile_options(player)
    dbref player;
{
#if (CRYPT_SYSTEM == 0)
  notify(player, " Passwords are not stored encrypted.");
#endif
#if (CRYPT_SYSTEM == 1)
  notify(player, " Passwords are encrypted with the operating system's crypt function.");
#endif
#if (CRYPT_SYSTEM == 2)
  notify(player, " Passwords are encrypted with SHS.");
#endif

#if (COMPRESSION_TYPE == 0)
  notify(player, " Attributes are not compressed in memory.");
#endif
#if (COMPRESSION_TYPE == 1)
  notify(player, " Attributes are Huffman compressed in memory.");
#endif
#if (COMPRESSION_TYPE == 2)
  notify(player, " Attributes are bigram compressed in memory.");
#endif
#if (COMPRESSION_TYPE == 3)
  notify(player, " Attributes are word compressed in memory.");
#endif

#ifdef SINGLE_LOGFILE
  notify(player, " Logging is to a single log file.");
#else
  notify(player, " Logging is to multiple log files.");
#endif

#ifdef MEM_CHECK
  notify(player, " The memory allocation tracker is operating.");
#endif

#ifdef INFO_SLAVE
  notify(player, " DNS and ident lookups are handled by a slave process.");
#else
  notify(player, " DNS and ident lookups are handled by the MUSH process.");
#endif

#ifdef FLOATING_POINTS
  notify(player, " Floating point functions are enabled.");
#else
  notify(player, " Floating point functions are disabled.");
#endif

#ifdef FUNCTION_SIDE_EFFECTS
  notify(player, " Side effect functions are enabled.");
#else
  notify(player, " Side effect functions are disabled.");
#endif

#ifdef USE_MAILER
  notify(player, " The extended built-in MUSH mailing system is being used.");
#ifdef MAIL_SUBJECTS
  notify(player, " @mail messages have subject lines.");
#ifdef ALLOW_NOSUBJECT
  notify(player, " Messages without subject lines have subject (no subject)");
#else
  notify(player, " Messages without subject lines use the beginning of the message as subject");
#endif
#else
  notify(player, " @mail messages do not have subject lines.");
#endif
#else
  notify(player, " The built-in MUSH mailing system is not being used.");
#endif

#ifdef CHAT_SYSTEM
  notify(player, " The extended chat system is enabled.");
#else
  notify(player, " The chat system is disabled.");
#endif

#ifdef QUOTA
  notify(player, " Quota restrictions are being enforced.");
#endif

#ifdef ROYALTY_FLAG
  notify(player, " The royalty flag is enabled.");
#else
  notify(player, " The royalty flag is disabled.");
#endif

#ifdef USE_WARNINGS
  notify(player, " The MUSH building warning system is enabled.");
  if (options.warn_interval)
    notify(player, tprintf(" Warnings will be automatically given every %d minutes",
			   options.warn_interval / 60));
  else
    notify(player, " Warnings will not be automatically given.");
#else
  notify(player, " The MUSH building warning system is disabled.");
#endif

#ifdef CREATION_TIMES
  notify(player, " Creation/modification times for objects are enabled.");
#else
  notify(player, " Creation/modification times for objects are not enabled.");
#endif


#ifdef EMPTY_ATTRS
  notify(player, " Empty attributes are kept.");
#ifdef VISIBLE_EMPTY_ATTRS
  notify(player, " Empty attributes are visible.");
#else
  notify(player, " Empty attributes are not visible.");
#endif
#ifdef DUMP_EMPTY_ATTRS
  notify(player, " Empty attributes are dumped.");
#else
  notify(player, " Empty attributes are not dumped.");
#endif
#ifdef DELETE_ATTRS
  notify(player, " Attributes can be deleted.");
#else
  notify(player, " Attributes cannot be deleted.");
#endif
#else
  notify(player, " Empty attributes are deleted.");
#endif

#ifdef DUMP_LESS_GARBAGE
  notify(player, " Garbage objects are not dumped to disk.");
#endif

#ifdef FIXED_FLAG
  notify(player, " The FIXED flag is enabled.");
#else
  notify(player, " The FIXED flag is disabled.");
#endif

#ifdef ONLINE_REG
  notify(player, " The UNREGISTERED flag is enabled.");
#endif
#ifdef VACATION_FLAG
  notify(player, " The VACATION flag is enabled.");
#endif
#ifdef UNINSPECTED_FLAG
  notify(player, " The UNINSPECTED flag is enabled.");
#endif

#ifdef SPEECH_LOCK
  notify(player, " @lock/speech is available.");
#endif
#ifdef LEAVE_LOCK
  notify(player, " @lock/leave is available.");
#endif
#ifdef DROP_LOCK
  notify(player, " @lock/drop is available.");
#endif
#ifdef GIVE_LOCK
  notify(player, " @lock/give is available.");
#endif

#ifdef EXTENDED_ANSI
  notify(player, " Extended ANSI functions are enabled.");
#else
  notify(player, " Extended ANSI functions are not enabled.");
#endif
}
