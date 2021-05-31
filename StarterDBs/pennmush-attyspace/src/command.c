/* command.c
 * Set up a hash table for the commands, and parse input for commands
 * This implementation is almost totally by Thorvald Natvig, with
 * various mods by Javelin
 */

#include "copyrite.h"
#include "config.h"

#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif

#include "conf.h"
#include "dbdefs.h"
#include "mushdb.h"
#include "game.h"
#include "externs.h"
#include "intrface.h"
#include "match.h"
#include "globals.h"
#include "extmail.h"
#ifdef CHAT_SYSTEM
#include "extchat.h"
#endif
#include "getpgsiz.h"
#include "parse.h"
#include "access.h"
#include "version.h"

#include "htab.h"
#include "function.h"
#include "command.h"
#include "mymalloc.h"
#include "confmagic.h"

HASHTAB htab_command;
HASHTAB htab_reserved_aliases;

void test_command _((dbref player, switch_mask *sw, char *args));
int restrict_command _((const char *name, const char *restriction));
static char *command_isattr _((char *command));
extern void local_commands _((void));
extern void reserve_aliases _((void));
static int command_check _((dbref player, COMMAND_INFO * cmd));
static int switch_find _((COMMAND_INFO * cmd, char *sw));
extern int global_fun_invocations;
extern int global_fun_recursions;

COMLIST commands[] =
{
  {"@COMMAND", "ON OFF QUIET ENABLE DISABLE", cmd_command, CMD_T_PLAYER, WIZARD, 0, 0},
  {"@@", NULL, cmd_null, CMD_T_ANY | CMD_T_NOPARSE, 0, 0, 0},
  {"@ALLHALT", NULL, cmd_allhalt, CMD_T_ANY, WIZARD, 0, HALT_ANYTHING},
#ifdef QUOTA
{"@ALLQUOTA", "QUIET", cmd_allquota, CMD_T_ANY, WIZARD, 0, CHANGE_QUOTAS},
#endif
  {"@ATRLOCK", NULL, cmd_atrlock, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
  {"@ATRCHOWN", NULL, cmd_atrchown, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
  {"@ATTRIBUTE", "ACCESS DELETE RENAME RETROACTIVE", cmd_attribute, CMD_T_ANY | CMD_T_EQSPLIT, WIZARD, 0, 0},
  {"@BOOT", "PORT ME", cmd_boot, CMD_T_ANY, 0, 0, 0},
#ifdef CHAT_SYSTEM
#ifdef ROYALTY_FLAG
  {"@CEMIT", "NOEVAL NOISY", cmd_cemit, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, WIZARD | ROYALTY, 0, CEMIT},
#else
  {"@CEMIT", "NOEVAL NOISY", cmd_cemit, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, WIZARD, 0, CEMIT},
#endif
  {"@CHANNEL", "LIST ADD DELETE RENAME NAME PRIVS QUIET DECOMPILE DESC CHOWN WIPE MUTE GAG HIDE WHAT TITLE", cmd_channel, CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@CHAT", NULL, cmd_chat, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
#endif
  {"@CHOWNALL", NULL, cmd_chownall, CMD_T_ANY | CMD_T_EQSPLIT, WIZARD, 0, 0},
  {"@CHOWN", NULL, cmd_chown, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@CHZONEALL", NULL, cmd_chzoneall, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
  {"@CHZONE", NULL, cmd_chzone, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@CONFIG", "LIST GLOBALS DEFAULTS COSTS FUNCTIONS COMMANDS ATTRIBS", cmd_config, CMD_T_ANY, 0, 0, 0},
  {"@CPATTR", NULL, cmd_cpattr, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS, 0, 0, 0},
  {"@CREATE", NULL, cmd_create, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@CLONE", NULL, cmd_clone, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
#ifdef CHAT_SYSTEM
  {"@CLOCK", "JOIN SPEAK MOD SEE HIDE", cmd_clock, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
#endif
  {"@DBCK", NULL, cmd_dbck, CMD_T_ANY, WIZARD, 0, 0},
{"@DECOMPILE", "DB TF FLAGS ATTRIBS SKIPDEFAULTS", cmd_decompile, CMD_T_ANY, 0, 0, 0},
  {"@DESTROY", "OVERRIDE", cmd_destroy, CMD_T_ANY, 0, 0, 0},
  {"@DIG", "TELEPORT", cmd_dig, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0, 0},
  {"@DISABLE", NULL, cmd_disable, CMD_T_ANY, 0, 0, 0},
  {"@DOING", "HEADER", cmd_doing, CMD_T_ANY | CMD_T_NOPARSE | CMD_T_NOGAGGED, 0, 0, 0},
  {"@DOLIST", "NOTIFY", cmd_dolist, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_NOPARSE, 0, 0, 0},
  {"@DRAIN", NULL, cmd_drain, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
  {"@DUMP", "PARANOID DEBUG", cmd_dump, CMD_T_ANY, WIZARD, 0, 0},
  {"@EDIT", NULL, cmd_edit, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_RS_NOPARSE | CMD_T_NOGAGGED, 0, 0, 0},
  {"@ELOCK", NULL, cmd_elock, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@EMIT", "ROOM NOEVAL", cmd_emit, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"@ENABLE", NULL, cmd_enable, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"@ENTRANCES", "EXITS THINGS PLAYERS ROOMS", cmd_entrances, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0, 0},
  {"@EUNLOCK", NULL, cmd_eunlock, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"@FIND", NULL, cmd_find, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0, 0},
  {"@FIRSTEXIT", NULL, cmd_firstexit, CMD_T_ANY, 0, 0, 0},
  {"@FIXDB", "LOCATION CONTENTS EXITS NEXT", cmd_fixdb, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
  {"@FORCE", NULL, cmd_force, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@FUNCTION", "DELETE", cmd_function, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0, 0},
  {"@GEDIT", NULL, cmd_edit, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_RS_NOPARSE | CMD_T_NOGAGGED, 0, 0, 0},
  {"@GREP", "LIST PRINT ILIST IPRINT", cmd_grep, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_NOPARSE | CMD_T_NOGAGGED, 0, 0, 0},
  {"@HALT", "ALL", cmd_halt, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
  {"@HIDE", "NO OFF YES ON", cmd_hide, CMD_T_ANY, 0, 0, 0},
  {"@KICK", NULL, cmd_kick, CMD_T_ANY, WIZARD, 0, 0},
  {"@LEMIT", "NOEVAL", cmd_lemit, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@LINK", NULL, cmd_link, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@LISTMOTD", NULL, cmd_listmotd, CMD_T_ANY, 0, 0, 0},
  {"@LIST", "MOTD FUNCTIONS COMMANDS ATTRIBS", cmd_list, CMD_T_ANY, 0, 0, 0},
  {"@LOCK", NULL, cmd_lock, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_SWITCHES | CMD_T_NOGAGGED, 0, 0, 0},
  {"@LOG", "CHECK CMD CONN ERR TRACE WIZ", cmd_log, CMD_T_ANY | CMD_T_NOGAGGED, WIZARD, 0, 0},
  {"@LOGWIPE", "CHECK CMD CONN ERR TRACE WIZ", cmd_logwipe, CMD_T_ANY | CMD_T_NOGAGGED, WIZARD, 0, 0},
#ifdef USE_MAILER
  {"@MAIL", "NOEVAL STATS DSTATS FSTATS DEBUG NUKE FOLDER UNFOLDER LIST READ CLEAR UNCLEAR PURGE FILE TAG UNTAG FWD FORWARD SEND SILENT URGENT", cmd_mail, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
#endif
  {"@MAP", NULL, cmd_map, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_NOPARSE, 0, 0, 0},
  {"@MOTD", "CONNECT LIST WIZARD DOWN FULL", cmd_motd, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"@MVATTR", NULL, cmd_mvattr, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS, 0, 0, 0},
  {"@NAME", NULL, cmd_name, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@NEWPASSWORD", NULL, cmd_newpassword, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
  {"@NOTIFY", "ALL", cmd_notify, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
  {"@NUKE", NULL, cmd_nuke, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"@OEMIT", "NOEVAL", cmd_oemit, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@OPEN", NULL, cmd_open, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0, 0},
  {"@PARENT", NULL, cmd_parent, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
  {"@PASSWORD", NULL, cmd_password, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
  {"@PCREATE", NULL, cmd_pcreate, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
  {"@PEMIT", "LIST CONTENTS SILENT NOISY NOEVAL", cmd_pemit, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@POLL", NULL, cmd_poll, CMD_T_ANY, 0, 0, 0},
  {"@POOR", NULL, cmd_poor, CMD_T_ANY, 0, 0, 0},
  {"@POWER", NULL, cmd_power, CMD_T_ANY | CMD_T_EQSPLIT, WIZARD, 0, 0},
  {"@PS", "ALL SUMMARY COUNT QUICK", cmd_ps, CMD_T_ANY, 0, 0, 0},
  {"@PURGE", NULL, cmd_purge, CMD_T_ANY, 0, 0, 0},
#ifdef QUOTA
  {"@QUOTA", "ALL SET", cmd_quota, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
#endif
  {"@READCACHE", NULL, cmd_readcache, CMD_T_ANY, WIZARD, 0, 0},
  {"@RECYCLE", "OVERRIDE", cmd_destroy, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"@REMIT", "NOEVAL", cmd_remit, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@REJECTMOTD", NULL, cmd_rejectmotd, CMD_T_ANY, WIZARD, 0, 0},
  {"@RESTART", "ALL", cmd_restart, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
#ifdef ROYALTY_FLAG
  {"@RWALL", NULL, cmd_rwall, CMD_T_ANY, WIZARD | ROYALTY, 0, 0},
  {"@RWALLPOSE", NULL, cmd_rwallpose, CMD_T_ANY, WIZARD | ROYALTY, 0, 0},
  {"@RWALLEMIT", NULL, cmd_rwallemit, CMD_T_ANY, WIZARD | ROYALTY, 0, 0},
#endif
  {"@SCAN", "ROOM SELF ZONE GLOBALS", cmd_scan, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"@SEARCH", NULL, cmd_search, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_RS_NOPARSE, 0, 0, 0},
  {"@SELECT", NULL, cmd_select, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_RS_NOPARSE, 0, 0, 0},
  {"@SET", NULL, cmd_set, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@SHUTDOWN", "PANIC REBOOT PARANOID", cmd_shutdown, CMD_T_ANY, WIZARD, 0, 0},
  {"@SITELOCK", "BAN REGISTER NAME", cmd_sitelock, CMD_T_ANY | CMD_T_EQSPLIT, WIZARD, 0, 0},
  {"@STATS", NULL, cmd_stats, CMD_T_ANY, 0, 0, 0},
  {"@SWEEP", "CONNECTED HERE INVENTORY EXITS", cmd_sweep, CMD_T_ANY, 0, 0, 0},
  {"@SWITCH", "FIRST ALL", cmd_switch, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_RS_NOPARSE | CMD_T_NOGAGGED, 0, 0, 0},
#ifdef QUOTA
  {"@SQUOTA", NULL, cmd_squota, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
#endif
  {"@TELEPORT", NULL, cmd_teleport, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"@TRIGGER", NULL, cmd_trigger, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0, 0},
  {"@ULOCK", NULL, cmd_ulock, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
{"@UNDESTROY", NULL, cmd_undestroy, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"@UNLINK", NULL, cmd_unlink, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"@UNLOCK", NULL, cmd_unlock, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_SWITCHES | CMD_T_NOGAGGED, 0, 0, 0},
{"@UNRECYCLE", NULL, cmd_undestroy, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"@UPTIME", NULL, cmd_uptime, CMD_T_ANY, 0, 0, 0},
  {"@UUNLOCK", NULL, cmd_uunlock, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"@VERB", NULL, cmd_verb, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS, 0, 0, 0},
  {"@VERSION", NULL, cmd_version, CMD_T_ANY, 0, 0, 0},
  {"@WAIT", NULL, cmd_wait, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_NOPARSE, 0, 0, 0},
#ifdef ROYALTY_FLAG
  {"@WALL", "WIZARD ROYALTY EMIT POSE", cmd_wall, CMD_T_ANY, WIZARD | ROYALTY, 0, CAN_WALL},
  {"@WALLPOSE", NULL, cmd_wallpose, CMD_T_ANY, WIZARD | ROYALTY, 0, CAN_WALL},
  {"@WALLEMIT", NULL, cmd_wallemit, CMD_T_ANY, WIZARD | ROYALTY, 0, CAN_WALL},
#else
  {"@WALL", "WIZARD EMIT POSE", cmd_wall, CMD_T_ANY, WIZARD, 0, CAN_WALL},
  {"@WALLPOSE", NULL, cmd_wallpose, CMD_T_ANY, WIZARD, 0, CAN_WALL},
  {"@WALLEMIT", NULL, cmd_wallemit, CMD_T_ANY, WIZARD, 0, CAN_WALL},
#endif
#ifdef USE_WARNINGS
  {"@WARNINGS", NULL, cmd_warnings, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
  {"@WCHECK", "ALL", cmd_wcheck, CMD_T_ANY, 0, 0, 0},
#endif
  {"@WHEREIS", NULL, cmd_whereis, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"@WIPE", NULL, cmd_wipe, CMD_T_ANY, 0, 0, 0},
  {"@WIZWALL", NULL, cmd_wizwall, CMD_T_ANY, WIZARD, 0, 0},
  {"@WIZPOSE", NULL, cmd_wizpose, CMD_T_ANY, WIZARD, 0, 0},
  {"@WIZEMIT", NULL, cmd_wizemit, CMD_T_ANY, WIZARD, 0, 0},
  {"@WIZMOTD", NULL, cmd_wizmotd, CMD_T_ANY, WIZARD, 0, 0},
  {"@ZEMIT", NULL, cmd_zemit, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},

  {"AHELP", NULL, cmd_ahelp, CMD_T_ANY | CMD_T_NOPARSE, 0, 0, 0},
  {"ANEWS", NULL, cmd_anews, CMD_T_ANY | CMD_T_NOPARSE, 0, 0, 0},
  {"BRIEF", NULL, cmd_brief, CMD_T_ANY, 0, 0, 0},
  {"DROP", NULL, cmd_drop, CMD_T_PLAYER | CMD_T_THING, 0, 0, 0},
  {"EXAMINE", "BRIEF DEBUG MORTAL", cmd_examine, CMD_T_ANY, 0, 0, 0},
  {"ENTER", NULL, cmd_enter, CMD_T_ANY, 0, 0, 0},
  {"EVENTS", NULL, cmd_events, CMD_T_ANY | CMD_T_NOPARSE, 0, 0, 0},
#ifdef FOLLOW
  {"FOLLOW", NULL, cmd_follow, CMD_T_PLAYER | CMD_T_THING | CMD_T_NOGAGGED, 0, 0, 0},
#endif
  {"GET", NULL, cmd_get, CMD_T_PLAYER | CMD_T_THING | CMD_T_NOGAGGED, 0, 0, 0},
  {"GIVE", NULL, cmd_give, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"GOTO", NULL, cmd_goto, CMD_T_PLAYER | CMD_T_THING, 0, 0, 0},
  {"HELP", NULL, cmd_help, CMD_T_ANY | CMD_T_NOPARSE, 0, 0, 0},
  {"INVENTORY", NULL, cmd_inventory, CMD_T_ANY, 0, 0, 0},
  {"INDEX", NULL, cmd_index, CMD_T_ANY | CMD_T_NOPARSE, 0, 0, 0},
  {"KILL", NULL, cmd_kill, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"LOOK", "OUTSIDE", cmd_look, CMD_T_ANY, 0, 0, 0},
  {"LEAVE", NULL, cmd_leave, CMD_T_PLAYER | CMD_T_THING, 0, 0, 0},
  {"MOVE", NULL, cmd_goto, CMD_T_PLAYER | CMD_T_THING, 0, 0, 0},
  {"NEWS", NULL, cmd_news, CMD_T_ANY | CMD_T_NOPARSE, 0, 0, 0},
  {"PAGE", "BLIND NOEVAL LIST", cmd_page, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
{"POSE", "NOEVAL NOSPACE", cmd_pose, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"ROB", NULL, cmd_rob, CMD_T_ANY, 0, 0, 0},
#ifdef ALLOW_RPAGE
  {"RPAGE", "LIST ADD DELETE", cmd_rpage, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0, 0},
#endif
  {"RULES", NULL, cmd_rules, CMD_T_ANY | CMD_T_NOPARSE, 0, 0, 0},
  {"READ", NULL, cmd_look, CMD_T_ANY, 0, 0, 0},
  {"SCORE", NULL, cmd_score, CMD_T_ANY, 0, 0, 0},
  {"SAY", "NOEVAL", cmd_say, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
{"SEMIPOSE", "NOEVAL", cmd_semipose, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"SLAY", NULL, cmd_slay, CMD_T_ANY, 0, 0, 0},
  {"TAKE", NULL, cmd_take, CMD_T_PLAYER | CMD_T_THING | CMD_T_NOGAGGED, 0, 0, 0},
  {"THINK", "NOEVAL", cmd_think, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},
  {"WHISPER", "LIST NOISY SILENT NOEVAL", cmd_whisper, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0, 0},
  {"USE", NULL, cmd_use, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0, 0},

/* ATTRIB_SET is an undocumented command - it's sugar to make it possible
 * enable/disable attribute setting with &XX or @XX
 */
  {"ATTRIB_SET", NULL, command_atrset, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED | CMD_T_INTERNAL, 0, 0, 0},
  {NULL, NULL, NULL, 0, 0, 0, 0}
};


SWITCH_VALUE switch_list[] =
{
#include "switchinc.c"
  {NULL, 0}
};

/* These aliases are added before commands are auto-aliased, so they
 * take precedence. That is, "l" will always be "look", no matter what.
 * If you don't do this, then since "l" look ambiguous to the autoaliaser
 * (it could be look or leave), it won't get aliased at all.
 */
COMALIAS command_alias[] =
{
  {"EXAMINE", "E"},
  {"BRIEF", "B"},
  {"LOOK", "L"},
  {"INVENTORY", "I"},
  {"PAGE", "P"},
  {"WHISPER", "W"},
  {"@RWALL", "@RW"},
  {"@WIZWALL", "@WIZ"},
  {"@TRIGGER", "@TR"},
  {NULL, NULL}
};

static char nullstring[] = "";


void
strccat(to, from)
    char *to;
    const char *from;
{
  if (*to)
    strcat(to, ", ");
  strcat(to, from);
}

static int
switch_find(cmd, sw)
    COMMAND_INFO *cmd;
    char *sw;
{
  SWITCH_VALUE *sw_val;
  int i = 0;
  int len;
  if (!sw || !*sw)
    return 0;
  len = strlen(sw);
  /* Special case, for init */
  sw_val = switch_list;
  if (!cmd) {
    while (sw_val->name) {
      if (strcmp(sw_val->name, sw) == 0)
	return sw_val->value;
      sw_val++;
    }
    return 0;
  } else {
    while (sw_val->name) {
      i++;
      if (SW_ISSET(cmd->sw, i) && (strncmp(sw_val->name, sw, len) == 0))
	return i;
      sw_val++;
    }
  }
  return 0;
}

COMMAND_INFO *
command_add(name, type, flags, toggles, powers, sw, func)
    const char *name;
    int type;
    int flags;
    int toggles;
    int powers;
    switch_mask *sw;
    void (*func) ();
{
  COMMAND_INFO *cmd;
  cmd = (COMMAND_INFO *) mush_malloc(sizeof(COMMAND_INFO), "command");
  memset(cmd, 0, sizeof(COMMAND_INFO));
  cmd->name = name;
  cmd->func = func;
  cmd->type = type;
  cmd->flags = flags;
  cmd->toggles = toggles;
  cmd->powers = powers;
  if (sw)
    memcpy(cmd->sw, sw, sizeof(switch_mask));
  else
    SW_ZERO(cmd->sw);
  hashadd(name, (void *) cmd, &htab_command);
  return cmd;
}

COMMAND_INFO *
command_find(name)
    const char *name;
{
  char cmdname[BUFFER_LEN];
  strcpy(cmdname, name);
  upcasestr(cmdname);
  return (COMMAND_INFO *) hashfind(cmdname, &htab_command);
}

COMMAND_INFO *
command_modify(name, type, flags, toggles, powers, sw, func)
    const char *name;
    int type;
    int flags;
    int toggles;
    int powers;
    switch_mask *sw;
    void (*func) ();
{
  COMMAND_INFO *cmd;
  cmd = command_find(name);
  if (!cmd)
    return NULL;
  if (type != -1)
    cmd->type = type;
  if (flags != -1)
    cmd->flags = flags;
  if (toggles != -1)
    cmd->toggles = toggles;
  if (powers != -1)
    cmd->powers = powers;
  if (sw)
    memcpy(cmd->sw, sw, sizeof(switch_mask));
  if (func)
    cmd->func = func;
  return cmd;
}

switch_mask *
switchmask(switches)
    const char *switches;
{
  static switch_mask sw;
  char buff[BUFFER_LEN];
  char *p, *s;
  int switchnum;
  SW_ZERO(sw);
  if (!switches || !switches[0])
    return NULL;
  strcpy(buff, switches);
  p = buff;
  while (p) {
    s = split_token(&p, ' ');
    switchnum = switch_find(NULL, s);
    if (!switchnum)
      return NULL;
    else
      SW_SET(sw, switchnum);
  }
  return &sw;
}

void
command_init_preconfig()
{
  COMLIST *cmd;
  static int done = 0;
  if (done == 1)
    return;
  done = 1;

  hashinit(&htab_command, 128);
  hashinit(&htab_reserved_aliases, 64);
  for (cmd = commands; cmd->name; cmd++) {
    command_add(cmd->name, cmd->type, cmd->flags, cmd->toggles, cmd->powers, switchmask(cmd->switches), cmd->func);
  }

  reserve_aliases();
  local_commands();
  command_mkalias();
}

void
command_init_postconfig()
{
  if (!*EVENT_FILE)
    restrict_command("EVENTS", "nobody");
  if (!*INDEX_FILE)
    restrict_command("INDEX", "nobody");
  if (!*RULES_FILE)
    restrict_command("RULES", "nobody");
  if (HATE_DEST)
    restrict_command("@DESTROY", "noplayer");
  else
    restrict_command("@RECYCLE", "nobody");
  if (!PLAYER_LOCATE)
    restrict_command("@WHEREIS", "nobody");
}


void
command_addalias(cmd, prev, next)
    COMMAND_INFO *cmd;
    char *prev;
    char *next;
{
  int i;
  char buff[BUFFER_LEN];
  if (!prev)
    prev = nullstring;
  if (!next)
    next = nullstring;
  strcpy(buff, cmd->name);
  for (i = strlen(buff) - 1;
       (i > 1) && strncmp(buff, prev, i) && strncmp(buff, next, i);
       i--) {
    buff[i] = '\0';
    if (!hashfind(buff, &htab_reserved_aliases))
      hashadd(buff, (void *) cmd, &htab_command);
  }
}

void
command_mkalias()
{
  COMMAND_INFO *cmd;
  COMSORTSTRUC *first, *this, *newone;
  COMALIAS *alias;
  char *pname;

  /* Hash all the built-in aliases first, so they don't get 
   * overridden by autoaliases
   */
  for (alias = command_alias; alias->name; alias++) {
    cmd = (COMMAND_INFO *) command_find(alias->name);
    if (!cmd) {
      do_rawlog(LT_ERR, "ALIAS : %s for nonexistant command %s", alias->alias, alias->name);
    } else {
      if (!hashfind(alias->alias, &htab_reserved_aliases))
	hashadd(alias->alias, (void *) cmd, &htab_command);
    }
  }

  /* Go through the hash table and construct a sorted linked list of
   * command names.
   */
  first = NULL;
  cmd = hash_firstentry(&htab_command);
  while (cmd) {
    if (!(cmd->type & CMD_T_INTERNAL)) {
      this = first;
      /* Find the name after which this one should be inserted */
      while ((this) && (this->cmd != cmd) && (this->next) && (strcmp(this->next->cmd->name, cmd->name) <= 0))
	this = this->next;
      if (!(this && (this->cmd == cmd))) {
	newone = (COMSORTSTRUC *) mush_malloc(sizeof(COMSORTSTRUC), "comsortstruc");
	newone->cmd = cmd;
	if (this && (this != first)) {
	  newone->next = this->next;
	  this->next = newone;
	} else {
	  newone->next = first;
	  first = newone;
	}
      }
    }
    cmd = hash_nextentry(&htab_command);
  }

  /* Run through our sorted linked list, passing each entry, and those
   * immediately before and after, if any, so it can make unique
   * abbreviations into aliases
   */
  pname = NULL;
  this = first;
  while (this) {
    command_addalias(this->cmd, pname, (char *) ((this->next) ? this->next->cmd->name : NULL));
    newone = this;
    pname = (char *) this->cmd->name;
    this = this->next;
    mush_free(newone, "comsortstruc");
  }

}

/* Real work. Parse the command arguments into arrays */
void
command_argparse(player, cause, from, to, argv, cmd, right_side, forcenoparse)
    dbref player;
    dbref cause;
    char **from;
    char *to;
    char *argv[];
    COMMAND_INFO *cmd;
    int right_side;
    int forcenoparse;
{
  int parse, split, args, i, done;
  char *t, *f;
  char *aold;

  f = *from;

  parse = (right_side) ? (cmd->type & CMD_T_RS_NOPARSE) : (cmd->type & CMD_T_NOPARSE);
  if (parse || forcenoparse)
    parse = PE_NOTHING;
  else
    parse = PE_DEFAULT;

  if (right_side)
    split = PT_NOTHING;
  else
    split = (cmd->type & CMD_T_EQSPLIT) ? PT_EQUALS : PT_NOTHING;

  if (right_side) {
    if (cmd->type & CMD_T_RS_ARGS)
      args = (cmd->type & CMD_T_RS_SPACE) ? PT_SPACE : PT_COMMA;
    else
      args = 0;
  } else {
    if (cmd->type & CMD_T_LS_ARGS)
      args = (cmd->type & CMD_T_LS_SPACE) ? PT_SPACE : PT_COMMA;
    else
      args = 0;
  }

  if ((parse == PE_NOTHING) && args)
    parse = PE_STRIP_BRACES;

  i = 1;
  done = 0;
  *to = '\0';

  if (args) {
    t = to + 1;
  } else {
    t = to;
  }

  while (*f && !done) {
    aold = t;
    while (*f == ' ')
      f++;
    process_expression(to, &t, (const char **) &f, player, cause, cause,
		       parse, (split | args), NULL);
    *t = '\0';
    if (args) {
      argv[i] = aold;
      if (*f)
	f++;
      i++;
      t++;
    }
    if (split && (*f == '=')) {
      f++;
      *from = f;
      done = 1;
    }
  }

  *from = f;

  if (args)
    while (i < MAX_ARG)
      argv[i++] = NULL;
}

/* Is this command an attempt to set an attribute like @VA or &NUM?
 * If so, return the attrib's name. Otherwise, return NULL
 */
static char *
command_isattr(command)
    char *command;
{
  ATTR *a;
  char buff[BUFFER_LEN];
  char *f, *t;

  if (((command[0] == '&') && (command[1])) ||
      ((command[0] == '@') && (command[1] == '_') && (command[2]))) {
    /* User-defined attributes: @_NUM or &NUM */
    if (command[0] == '@')
      return command + 2;
    else
      return command + 1;
  } else if (command[0] == '@') {
    f = command + 1;
    t = buff;
    while ((*f) && (*f != '/'))
      *t++ = *f++;
    *t = 0;
    a = atr_match(buff);
    if (a)
      return (char *) AL_NAME(a);
  }
  return NULL;
}

/* Parse the commands. This is the big one! 
 * Return 1 if the command was recognized and handled, 0 otherwise
 */
char *
command_parse(player, cause, string, fromport)
    dbref player;
    dbref cause;
    char *string;
    int fromport;
{
  char *command, *swtch, *ls, *rs, *switches;
  static char commandraw[BUFFER_LEN];
  char *lsa[MAX_ARG];
  char *rsa[MAX_ARG];
  char *ap;
  char *swp;
  char *attrib;
  const char *replacer;
  COMMAND_INFO *cmd;
  char *p, *t, *c;
  char b;
  int switchnum;
  switch_mask sw;
  int noeval;

  command = (char *) mush_malloc(BUFFER_LEN, "string");
  swtch = (char *) mush_malloc(BUFFER_LEN, "string");
  ls = (char *) mush_malloc(BUFFER_LEN, "string");
  rs = (char *) mush_malloc(BUFFER_LEN, "string");
  switches = (char *) mush_malloc(BUFFER_LEN, "string");
  if (!command || !swtch || !ls || !rs || !switches)
    panic("Couldn't allocate memory in command_parse");
  p = string;
  replacer = NULL;
  attrib = NULL;
  cmd = NULL;
  c = command;
  /* All those one character commands.. Sigh */

  global_fun_invocations = global_fun_recursions = 0;
  switch (*p) {
  case '\0':
    /* Just in case. You never know */
    mush_free((Malloc_t) command, "string");
    mush_free((Malloc_t) swtch, "string");
    mush_free((Malloc_t) ls, "string");
    mush_free((Malloc_t) rs, "string");
    mush_free((Malloc_t) switches, "string");
    return NULL;
  case '#':
    if (!IS(Owner(player), TYPE_PLAYER, PLAYER_GAGGED) && Mobile(player))
      force_by_number(player, string);
    mush_free((Malloc_t) command, "string");
    mush_free((Malloc_t) swtch, "string");
    mush_free((Malloc_t) ls, "string");
    mush_free((Malloc_t) rs, "string");
    mush_free((Malloc_t) switches, "string");
    return NULL;
  case SAY_TOKEN:
    replacer = "SAY";
    break;
  case POSE_TOKEN:
    replacer = "POSE";
    break;
  case SEMI_POSE_TOKEN:
    replacer = "SEMIPOSE";
    break;
  case EMIT_TOKEN:
    replacer = "@EMIT";
    break;
#ifdef CHAT_SYSTEM
  case CHAT_TOKEN:
    if (command_check_byname(player, "@chat") && parse_chat(player, p + 1)) {
      mush_free((Malloc_t) command, "string");
      mush_free((Malloc_t) swtch, "string");
      mush_free((Malloc_t) ls, "string");
      mush_free((Malloc_t) rs, "string");
      mush_free((Malloc_t) switches, "string");
      return NULL;
    }
#endif				/* CHAT_SYSTEM */
    /* And everything else */
  default:
    /* EXCEPT exits, which have priority */
    if (can_move(player, p)) {
      if (Mobile(player) && command_check_byname(player, "GOTO"))
	do_move(player, p, 0);
      mush_free((Malloc_t) command, "string");
      mush_free((Malloc_t) swtch, "string");
      mush_free((Malloc_t) ls, "string");
      mush_free((Malloc_t) rs, "string");
      mush_free((Malloc_t) switches, "string");
      return NULL;
    }
    c = command;
    while (*p == ' ')
      p++;
    process_expression(command, &c, (const char **) &p, player, cause, cause,
		       PE_DEFAULT & ~PE_FUNCTION_CHECK, PT_SPACE, NULL);
    *c = '\0';
    strcpy(commandraw, command);
    upcasestr(command);

    /* Catch &XX and @XX attribute pairs. If that's what we've got,
     * use the magical ATTRIB_SET command
     */
    attrib = command_isattr(command);
    if (attrib) {
      cmd = command_find("ATTRIB_SET");
    } else {
      c = command;
      while ((*c) && (*c != '/') && (*c != ' '))
	c++;
      b = *c;
      *c = '\0';
      cmd = command_find(command);
      *c = b;
      /* Is this for internal use? If so, players can't use it! */
      if (cmd && (cmd->type & CMD_T_INTERNAL))
	cmd = NULL;
    }
  }

  if (replacer) {
    cmd = command_find(replacer);
    p++;
  }
  /* Test if this either isn't a command, or is a disabled one */
  /* If so, return Fully Parsed string for further processing */

  if (!cmd || (cmd->type & CMD_T_DISABLED)) {
    if (!cmd) {
      c = commandraw + strlen(commandraw);
    } else {
      if (*c == '/') {
	/* Oh... DAMN */
	c = commandraw;
	strcpy(switches, commandraw);
	safe_str(cmd->name, commandraw, &c);
	t = (char *) index(switches, '/');
	safe_str(t, commandraw, &c);
      } else {
	c = commandraw;
	safe_str(cmd->name, commandraw, &c);
      }
    }
    if (*p) {
      if (*p == ' ') {
	safe_chr(' ', commandraw, &c);
	p++;
      }
      process_expression(commandraw, &c, (const char **) &p, player, cause, cause, PE_DEFAULT, PT_DEFAULT, NULL);
    }
    *c = '\0';
    mush_free((Malloc_t) command, "string");
    mush_free((Malloc_t) swtch, "string");
    mush_free((Malloc_t) ls, "string");
    mush_free((Malloc_t) rs, "string");
    mush_free((Malloc_t) switches, "string");
    return commandraw;
  }
  /* Check the permissions */
  if (!command_check(player, cmd)) {
    mush_free((Malloc_t) command, "string");
    mush_free((Malloc_t) swtch, "string");
    mush_free((Malloc_t) ls, "string");
    mush_free((Malloc_t) rs, "string");
    mush_free((Malloc_t) switches, "string");
    return NULL;
  }
  /* Parse out any switches */
  SW_ZERO(sw);
  swp = switches;
  *swp = '\0';

  t = NULL;

  /* Don't parse switches for one-char commands */
  if (!replacer) {
    while (*c == '/') {
      t = swtch;
      c++;
      while ((*c) && (*c != ' ') && (*c != '/'))
	*t++ = *c++;
      *t = '\0';
      switchnum = switch_find(cmd, upcasestr(swtch));
      if (!switchnum) {
	if (cmd->type & CMD_T_SWITCHES) {
	  if (*swp)
	    strcat(swp, " ");
	  strcat(swp, swtch);
	} else {
	  notify(player, tprintf("%s doesn't know switch %s.", cmd->name, swtch));
	  mush_free((Malloc_t) command, "string");
	  mush_free((Malloc_t) swtch, "string");
	  mush_free((Malloc_t) ls, "string");
	  mush_free((Malloc_t) rs, "string");
	  mush_free((Malloc_t) switches, "string");
	  return NULL;
	}
      } else {
	SW_SET(sw, switchnum);
      }
    }
  }
  if (!t)
    SW_SET(sw, SWITCH_NONE);

  /* If we're calling ATTRIB_SET, the switch is the attribute name */
  if (attrib)
    swp = attrib;
  else if (!*swp)
    swp = NULL;

  if (*p == ' ')
    p++;
  ap = p;

  /* noeval and direct players.
   * If the noeval switch is set:
   *  (1) if we split on =, and an = is present, eval lhs, not rhs
   *  (2) if we split on =, and no = is present, do not eval arg
   *  (3) if we don't split on =, do not eval arg
   * Special case for ATTRIB_SET by a directly connected player:
   * Treat like noeval, except for #2. Eval arg if no =.
   */

  if ((cmd->func == command_atrset) && fromport) {
    /* Special case: eqsplit, noeval of rhs only */
    command_argparse(player, cause, &p, ls, lsa, cmd, 0, 0);
    command_argparse(player, cause, &p, rs, rsa, cmd, 1, 1);
    SW_SET(sw, SWITCH_NOEVAL);	/* Needed for ATTRIB_SET */
  } else {
    noeval = SW_ISSET(sw, SWITCH_NOEVAL);
    if (cmd->type & CMD_T_EQSPLIT) {
      char *savep = p;
      command_argparse(player, cause, &p, ls, lsa, cmd, 0, noeval);
      if (noeval && *p) {
	/* oops, we have a right hand side, should have evaluated */
	p = savep;
	command_argparse(player, cause, &p, ls, lsa, cmd, 0, 0);
      }
      command_argparse(player, cause, &p, rs, rsa, cmd, 1, noeval);
    } else {
      command_argparse(player, cause, &p, ls, lsa, cmd, 0, noeval);
    }
  }

  if (cmd->func == NULL) {
    do_rawlog(LT_ERR, "No command vector on command %s.", cmd->name);
  } else {
    cmd->func(cmd, player, cause, sw, string, swp, ap,
	      ls, lsa,
	      rs, rsa);
  }

  mush_free((Malloc_t) command, "string");
  mush_free((Malloc_t) swtch, "string");
  mush_free((Malloc_t) ls, "string");
  mush_free((Malloc_t) rs, "string");
  mush_free((Malloc_t) switches, "string");
  return NULL;
}

/* Given a command name and a restriction, apply the restriction to the
 * command in addition to whatever its usual restrictions are.
 * This is used by the configuration file startup in conf.c
 * Valid restrictions are:
 *   nobody     disable the command
 *   nogagged   can't be used by gagged players
 *   nofixed    can't be used by fixed players
 *   noguest    can't be used by guests
 *   admin      can only be used by royalty or wizards
 *   wizard     can only be used by wizards
 *   god        can only be used by god
 *   noplayer   can't be used by players, just objects/rooms/exits
 * Return 1 on success, 0 on failure.
 */
int
restrict_command(name, restriction)
    const char *name;
    const char *restriction;
{
  COMMAND_INFO *command;
  if (!name || !*name)
    return 0;
  command = command_find(name);
  if (!command)
    return 0;
  if (!strcasecmp(restriction, "nobody")) {
    command->type |= CMD_T_DISABLED;
  } else if (string_prefix(restriction, "nogag")) {
    command->type |= CMD_T_NOGAGGED;
  } else if (string_prefix(restriction, "nofix")) {
    command->type |= CMD_T_NOFIXED;
  } else if (!strcasecmp(restriction, "noguest")) {
    command->type |= CMD_T_NOGUEST;
  } else if (!strcasecmp(restriction, "admin")) {
#ifdef ROYALTY_FLAG
    command->flags |= ROYALTY | WIZARD;
#else
    command->flags |= WIZARD;
#endif
  } else if (!strcasecmp(restriction, "wizard")) {
    command->flags |= WIZARD;
  } else if (!strcasecmp(restriction, "god")) {
    command->type |= CMD_T_GOD;
  } else if (!strcasecmp(restriction, "noplayer")) {
    command->type &= ~CMD_T_PLAYER;
  } else {
    return 0;
  }
  return 1;
}

/* This is the only command which should be defined in this
 * file, because it uses variables from this file, etc.
 */
COMMAND (cmd_command) {
  COMMAND_INFO *command;
  SWITCH_VALUE *sw_val;
  char buff[BUFFER_LEN];

  if (!arg_left[0]) {
    notify(player, "You must specify a command.");
    return;
  }
  command = command_find(arg_left);
  if (!command) {
    notify(player, "No such command.");
    return;
  }
  if (SW_ISSET(sw, SWITCH_ON) || SW_ISSET(sw, SWITCH_ENABLE))
    command->type &= ~CMD_T_DISABLED;
  else if (SW_ISSET(sw, SWITCH_OFF) || SW_ISSET(sw, SWITCH_DISABLE))
    command->type |= CMD_T_DISABLED;

  if ((command->func == cmd_command) && (command->type & CMD_T_DISABLED)) {
    notify(player, "@command is ALWAYS enabled.");
    command->type &= ~CMD_T_DISABLED;
  }
  if (!SW_ISSET(sw, SWITCH_QUIET)) {
    notify(player, tprintf("Name       : %s (%s)", command->name, (command->type & CMD_T_DISABLED) ? "Disabled" : "Enabled"));
    if ((command->type & CMD_T_ANY) == CMD_T_ANY)
      strcpy(buff, "Any");
    else {
      buff[0] = '\0';
      if (command->type & CMD_T_ROOM)
	strccat(buff, "Room");
      if (command->type & CMD_T_THING)
	strccat(buff, "Thing");
      if (command->type & CMD_T_EXIT)
	strccat(buff, "Exit");
      if (command->type & CMD_T_PLAYER)
	strccat(buff, "Player");
    }
    notify(player, tprintf("Types      : %s", buff));
    buff[0] = '\0';
    if (command->type & CMD_T_SWITCHES)
      strccat(buff, "Switches");
    if (command->type & CMD_T_NOGAGGED)
      strccat(buff, "Nogag");
    if (command->type & CMD_T_NOFIXED)
      strccat(buff, "Nofix");
    if (command->type & CMD_T_NOGUEST)
      strccat(buff, "Noguest");
    if (command->type & CMD_T_EQSPLIT)
      strccat(buff, "Eqsplit");
    if (command->type & CMD_T_GOD)
      strccat(buff, "God");
    notify(player, tprintf("Flags      : %s", buff));
    buff[0] = '\0';
    for (sw_val = switch_list; sw_val->name; sw_val++)
      if (SW_ISSET(command->sw, sw_val->value))
	strccat(buff, sw_val->name);
    notify(player, tprintf("Switches   : %s", buff));
    buff[0] = '\0';
    if (command->type & CMD_T_LS_ARGS) {
      if (command->type & CMD_T_LS_SPACE)
	strccat(buff, "Space-Args");
      else
	strccat(buff, "Args");
    }
    if (command->type & CMD_T_LS_NOPARSE)
      strccat(buff, "Noparse");
    if (command->type & CMD_T_EQSPLIT) {
      notify(player, tprintf("Leftside   : %s", buff));
      buff[0] = '\0';
      if (command->type & CMD_T_RS_ARGS) {
	if (command->type & CMD_T_RS_SPACE)
	  strccat(buff, "Space-Args");
	else
	  strccat(buff, "Args");
      }
      if (command->type & CMD_T_RS_NOPARSE)
	strccat(buff, "Noparse");
      notify(player, tprintf("Rightside  : %s", buff));
    } else {
      notify(player, tprintf("Arguments  : %s", buff));
    }
  }
}

void
do_list_commands(player)
    dbref player;
{
  COMMAND_INFO *command;
  char *ptrs[BUFFER_LEN / 2];
  char buff[BUFFER_LEN];
  char *bp;
  int nptrs = 0, i;
  command = hash_firstentry(&htab_command);
  while (command) {
    if (!(command->type & CMD_T_LISTED)) {
      ptrs[nptrs++] = (char *) command->name;
      command->type |= CMD_T_LISTED;
    }
    command = hash_nextentry(&htab_command);
  }
  command = hash_firstentry(&htab_command);
  while (command) {
    command->type &= ~CMD_T_LISTED;
    command = hash_nextentry(&htab_command);
  }
  do_gensort(ptrs, nptrs, 0);
  bp = buff;
  safe_str("Commands:", buff, &bp);
  for (i = 0; i < nptrs; i++) {
    safe_chr(' ', buff, &bp);
    safe_str(ptrs[i], buff, &bp);
  }
  *bp = '\0';
  notify(player, buff);
}


/* Check command permissions. Return 1 if player can use command,
 * 0 otherwise, and be noisy about it.
 */
static int
command_check(player, cmd)
    dbref player;
    COMMAND_INFO *cmd;
{
  int ok;

  /* If disabled, return silently */
  if (cmd->type & CMD_T_DISABLED)
    return 0;
  if ((cmd->type & CMD_T_NOGAGGED) && Gagged(player)) {
    notify(player, "Cannot do that while gagged.");
    return 0;
  }
  if ((cmd->type & CMD_T_NOFIXED) && Fixed(player)) {
    notify(player, "Cannot do that while fixed.");
    return 0;
  }
  if ((cmd->type & CMD_T_NOGUEST) && Guest(player)) {
    notify(player, "Guests cannot do that.");
    return 0;
  }
  if ((cmd->type & CMD_T_GOD) && (!God(player))) {
    notify(player, "Only God can do that.");
    return 0;
  }
  switch (Typeof(player)) {
  case TYPE_ROOM:
    ok = (cmd->type & CMD_T_ROOM);
    break;
  case TYPE_THING:
    ok = (cmd->type & CMD_T_THING);
    break;
  case TYPE_EXIT:
    ok = (cmd->type & CMD_T_EXIT);
    break;
  case TYPE_PLAYER:
    ok = (cmd->type & CMD_T_PLAYER);
    break;
  default:
    ok = 0;
  }
  if (!ok) {
    notify(player, "Permission denied, command is type-restricted.");
    return 0;
  }
  /* A command can specify required flags, toggles, or powers, and if
   * any match, the player is ok to do the command. Toggles really
   * should only be used if the command is type-restricted, too!
   */
  if (!(
      ((cmd->flags == 0) && (cmd->toggles == 0) && (cmd->powers == 0)) ||
	 ((cmd->flags) && (cmd->flags & Flags(player))) ||
	 ((cmd->toggles) && (cmd->toggles & Toggles(player))) ||
	 ((cmd->powers) && (cmd->powers & Powers(player))))) {
    notify(player, "Permission denied.");
    return 0;
  }
  return 1;
}

/* command_check, but given a name, not a structure */
int
command_check_byname(player, name)
    dbref player;
    const char *name;
{
  COMMAND_INFO *cmd;
  cmd = command_find(name);
  if (!cmd)
    return 0;
  return command_check(player, cmd);
}
