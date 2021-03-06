/**
 * \file command.c
 *
 * \brief Parsing of commands.
 *
 * Sets up a hash table for the commands, and parses input for commands.
 * This implementation is almost totally by Thorvald Natvig, with
 * various mods by Javelin and others over time.
 *
 */

#include "copyrite.h"
#include "config.h"

#include <string.h>

#include "conf.h"
#include "dbdefs.h"
#include "mushdb.h"
#include "game.h"
#include "externs.h"
#include "match.h"
#include "attrib.h"
#include "extmail.h"
#include "getpgsiz.h"
#include "parse.h"
#include "access.h"
#include "version.h"
#include "ptab.h"
#include "htab.h"
#include "function.h"
#include "command.h"
#include "mymalloc.h"
#include "flags.h"
#include "log.h"
#include "cmds.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "confmagic.h"

PTAB ptab_command;
PTAB ptab_command_perms;

HASHTAB htab_reserved_aliases;

static const char *command_isattr(char *command);
static int command_check(dbref player, COMMAND_INFO *cmd);
static int switch_find(COMMAND_INFO *cmd, char *sw);
static void strccat(char *buff, char **bp, const char *from);
extern int global_fun_invocations;
extern int global_fun_recursions;

void run_hook(dbref player, dbref cause, struct hook_data *hook,
	      char *saveregs[], int save);
void do_hook(dbref player, char *command, char *obj, char *attrname,
	     int hook_type);


COMLIST commands[] = {

  {"@COMMAND", "ON OFF QUIET ENABLE DISABLE RESTRICT", cmd_command,
   CMD_T_PLAYER | CMD_T_EQSPLIT, 0, 0},
  {"@@", NULL, cmd_null, CMD_T_ANY | CMD_T_NOPARSE, 0, 0},
  {"@ALLHALT", NULL, cmd_allhalt, CMD_T_ANY, "WIZARD", HALT_ANYTHING},
#ifdef QUOTA
  {"@ALLQUOTA", "QUIET", cmd_allquota, CMD_T_ANY, "WIZARD", CHANGE_QUOTAS},
#endif
  {"@ATRLOCK", NULL, cmd_atrlock, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},
  {"@ATRCHOWN", NULL, cmd_atrchown, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},

  {"@ATTRIBUTE", "ACCESS DELETE RENAME RETROACTIVE", cmd_attribute,
   CMD_T_ANY | CMD_T_EQSPLIT, "WIZARD", 0},
  {"@BOOT", "PORT ME", cmd_boot, CMD_T_ANY, 0, 0},
  {"@BREAK", NULL, cmd_break, CMD_T_ANY, 0, 0},
#ifdef CHAT_SYSTEM
  {"@CEMIT", "NOEVAL NOISY", cmd_cemit,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
  {"@CHANNEL",
   "LIST ADD DELETE RENAME NAME PRIVS QUIET NOISY DECOMPILE DESCRIBE CHOWN WIPE MUTE UNMUTE GAG UNGAG HIDE UNHIDE WHAT TITLE BRIEF RECALL BUFFER",
   cmd_channel,
   CMD_T_ANY | CMD_T_SWITCHES | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
  {"@CHAT", NULL, cmd_chat, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
#endif
  {"@CHOWNALL", "PRESERVE", cmd_chownall, CMD_T_ANY | CMD_T_EQSPLIT, "WIZARD",
   0},

  {"@CHOWN", "PRESERVE", cmd_chown,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
  {"@CHZONEALL", NULL, cmd_chzoneall, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},

  {"@CHZONE", NULL, cmd_chzone,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
  {"@CONFIG",
   "SET LOWERCASE LIST GLOBALS DEFAULTS COSTS FLAGS FUNCTIONS COMMANDS ATTRIBS",
   cmd_config, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},
  {"@CPATTR", "CONVERT", cmd_cpattr, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS,
   0, 0},
  {"@CREATE", NULL, cmd_create, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED,
   0, 0},
  {"@CLONE", "PRESERVE", cmd_clone, CMD_T_ANY | CMD_T_NOGAGGED | CMD_T_EQSPLIT,
   0, 0},
#ifdef CHAT_SYSTEM

  {"@CLOCK", "JOIN SPEAK MOD SEE HIDE", cmd_clock,
   CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},
#endif
  {"@DBCK", NULL, cmd_dbck, CMD_T_ANY, "WIZARD", 0},

  {"@DECOMPILE", "DB TF FLAGS ATTRIBS SKIPDEFAULTS", cmd_decompile,
   CMD_T_ANY, 0, 0},
  {"@DESTROY", "OVERRIDE", cmd_destroy, CMD_T_ANY, 0, 0},

  {"@DIG", "TELEPORT", cmd_dig,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0},
  {"@DISABLE", NULL, cmd_disable, CMD_T_ANY, "WIZARD", 0},

  {"@DOING", "HEADER", cmd_doing,
   CMD_T_ANY | CMD_T_NOPARSE | CMD_T_NOGAGGED, 0, 0},
  {"@DOLIST", "NOTIFY DELIMIT", cmd_dolist,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_NOPARSE, 0, 0},
  {"@DRAIN", "ALL ANY", cmd_notify_drain, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},
  {"@DUMP", "PARANOID DEBUG", cmd_dump, CMD_T_ANY, "WIZARD", 0},

  {"@EDIT", NULL, cmd_edit,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_RS_NOPARSE |
   CMD_T_NOGAGGED, 0, 0},
  {"@ELOCK", NULL, cmd_elock, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED,
   0, 0},
  {"@EMIT", "ROOM NOEVAL SILENT", cmd_emit, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},
  {"@ENABLE", NULL, cmd_enable, CMD_T_ANY | CMD_T_NOGAGGED, "WIZARD", 0},

  {"@ENTRANCES", "EXITS THINGS PLAYERS ROOMS", cmd_entrances,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0},
  {"@EUNLOCK", NULL, cmd_eunlock, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},

  {"@FIND", NULL, cmd_find,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0},
  {"@FIRSTEXIT", NULL, cmd_firstexit, CMD_T_ANY, 0, 0},
  {"@FLAG", "ADD LIST RESTRICT DELETE ALIAS DISABLE ENABLE", cmd_flag,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0},

  {"@FORCE", "NOEVAL", cmd_force, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED,
   0, 0},
  {"@FUNCTION", "DELETE ENABLE DISABLE RESTORE RESTRICT", cmd_function,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0},
  {"@GREP", "LIST PRINT ILIST IPRINT", cmd_grep,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_NOPARSE | CMD_T_NOGAGGED, 0, 0},
  {"@HALT", "ALL", cmd_halt, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},
  {"@HIDE", "NO OFF YES ON", cmd_hide, CMD_T_ANY, 0, 0},
  {"@HOOK", "AFTER BEFORE", cmd_hook, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS,
   "WIZARD", 0},
  {"@KICK", NULL, cmd_kick, CMD_T_ANY, "WIZARD", 0},

  {"@LEMIT", "NOEVAL SILENT", cmd_lemit,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
  {"@LINK", "PRESERVE", cmd_link, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0,
   0},
  {"@LISTMOTD", NULL, cmd_listmotd, CMD_T_ANY, 0, 0},

  {"@LIST", "LOWERCASE MOTD FLAGS FUNCTIONS COMMANDS ATTRIBS", cmd_list,
   CMD_T_ANY, 0, 0},
  {"@LOCK", NULL, cmd_lock,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_SWITCHES | CMD_T_NOGAGGED, 0, 0},
  {"@LOG", "CHECK CMD CONN ERR TRACE WIZ", cmd_log,
   CMD_T_ANY | CMD_T_NOGAGGED, "WIZARD", 0},
  {"@LOGWIPE", "CHECK CMD CONN ERR TRACE WIZ", cmd_logwipe,
   CMD_T_ANY | CMD_T_NOGAGGED | CMD_T_GOD, 0, 0},
  {"@LSET", NULL, cmd_lset,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
#ifdef USE_MAILER
  {"@MAIL",
   "NOEVAL NOSIG STATS DSTATS FSTATS DEBUG NUKE FOLDERS UNFOLDER LIST READ CLEAR UNCLEAR PURGE FILE TAG UNTAG FWD FORWARD SEND SILENT URGENT",
   cmd_mail, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},
#ifdef MAIL_ALIASES

  {"@MALIAS",
   "SET CREATE DESTROY DESCRIBE RENAME STATS CHOWN NUKE ADD REMOVE LIST ALL WHO MEMBERS USEFLAG SEEFLAG",
   cmd_malias, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
#endif
#endif

  {"@MAP", "DELIMIT", cmd_map, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_NOPARSE,
   0, 0},
  {"@MOTD", "CONNECT LIST WIZARD DOWN FULL", cmd_motd,
   CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},
  {"@MVATTR", "CONVERT", cmd_mvattr, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS,
   0, 0},
  {"@NAME", NULL, cmd_name, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED
   | CMD_T_NOGUEST, 0, 0},
  {"@NEWPASSWORD", NULL, cmd_newpassword, CMD_T_ANY | CMD_T_EQSPLIT, "WIZARD",
   0},
  {"@NOTIFY", "ALL ANY", cmd_notify_drain, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},
  {"@NSPEMIT", "LIST SILENT NOISY NOEVAL", cmd_nspemit,
   CMD_T_ANY | CMD_T_EQSPLIT, "WIZARD", CAN_NSPEMIT},
  {"@NUKE", NULL, cmd_nuke, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},

  {"@OEMIT", "NOEVAL", cmd_oemit, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED,
   0, 0},
  {"@OPEN", NULL, cmd_open,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0},
  {"@PARENT", NULL, cmd_parent, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},
  {"@PASSWORD", NULL, cmd_password, CMD_T_PLAYER | CMD_T_EQSPLIT
   | CMD_T_NOGUEST, 0, 0},
  {"@PCREATE", NULL, cmd_pcreate, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},

  {"@PEMIT", "LIST CONTENTS SILENT NOISY NOEVAL", cmd_pemit,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
  {"@POLL", NULL, cmd_poll, CMD_T_ANY, 0, 0},
  {"@POOR", NULL, cmd_poor, CMD_T_ANY, 0, 0},
  {"@POWER", NULL, cmd_power, CMD_T_ANY | CMD_T_EQSPLIT, "WIZARD", 0},
  {"@PS", "ALL SUMMARY COUNT QUICK", cmd_ps, CMD_T_ANY, 0, 0},
  {"@PURGE", NULL, cmd_purge, CMD_T_ANY, 0, 0},
#ifdef QUOTA
  {"@QUOTA", "ALL SET", cmd_quota, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},
#endif
  {"@READCACHE", NULL, cmd_readcache, CMD_T_ANY, "WIZARD", 0},
  {"@RECYCLE", "OVERRIDE", cmd_destroy, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},

  {"@REMIT", "LIST NOEVAL NOISY SILENT", cmd_remit,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
  {"@REJECTMOTD", NULL, cmd_rejectmotd, CMD_T_ANY, "WIZARD", 0},
  {"@RESTART", "ALL", cmd_restart, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},
  {"@RWALL", "NOEVAL EMIT", cmd_rwall, CMD_T_ANY, "WIZARD ROYALTY", 0},
  {"@SCAN", "ROOM SELF ZONE GLOBALS", cmd_scan,
   CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},
  {"@SEARCH", NULL, cmd_search,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_RS_NOPARSE, 0, 0},
  {"@SELECT", "NOTIFY", cmd_select,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_RS_NOPARSE, 0, 0},
  {"@SET", NULL, cmd_set, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
  {"@SHUTDOWN", "PANIC REBOOT PARANOID", cmd_shutdown, CMD_T_ANY, "WIZARD", 0},
  {"@SITELOCK", "BAN CHECK REGISTER REMOVE NAME", cmd_sitelock,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS, "WIZARD", 0},
  {"@STATS", "TABLES", cmd_stats, CMD_T_ANY, 0, 0},

  {"@SWEEP", "CONNECTED HERE INVENTORY EXITS", cmd_sweep, CMD_T_ANY, 0, 0},
  {"@SWITCH", "NOTIFY FIRST ALL", cmd_switch,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_RS_NOPARSE |
   CMD_T_NOGAGGED, 0, 0},
#ifdef QUOTA
  {"@SQUOTA", NULL, cmd_squota, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},
#endif

  {"@TELEPORT", "SILENT", cmd_teleport,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
  {"@TRIGGER", NULL, cmd_trigger,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS | CMD_T_NOGAGGED, 0, 0},
  {"@ULOCK", NULL, cmd_ulock, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED,
   0, 0},
  {"@UNDESTROY", NULL, cmd_undestroy, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},
  {"@UNLINK", NULL, cmd_unlink, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},

  {"@UNLOCK", NULL, cmd_unlock,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_SWITCHES | CMD_T_NOGAGGED, 0, 0},
  {"@UNRECYCLE", NULL, cmd_undestroy, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},
  {"@UPTIME", "MORTAL", cmd_uptime, CMD_T_ANY, 0, 0},
  {"@UUNLOCK", NULL, cmd_uunlock, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},
  {"@VERB", NULL, cmd_verb, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_ARGS, 0, 0},
  {"@VERSION", NULL, cmd_version, CMD_T_ANY, 0, 0},
  {"@WAIT", "UNTIL", cmd_wait, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_RS_NOPARSE,
   0, 0},
  {"@WALL", "NOEVAL EMIT", cmd_wall, CMD_T_ANY, "WIZARD ROYALTY", CAN_WALL},

  {"@WARNINGS", NULL, cmd_warnings, CMD_T_ANY | CMD_T_EQSPLIT, 0, 0},
  {"@WCHECK", "ALL ME", cmd_wcheck, CMD_T_ANY, 0, 0},
  {"@WHEREIS", NULL, cmd_whereis, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},
  {"@WIPE", NULL, cmd_wipe, CMD_T_ANY, 0, 0},
  {"@WIZWALL", "NOEVAL EMIT", cmd_wizwall, CMD_T_ANY, "WIZARD", 0},
  {"@WIZMOTD", NULL, cmd_wizmotd, CMD_T_ANY, "WIZARD", 0},
  {"@ZEMIT", NULL, cmd_zemit, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED,
   0, 0},

  {"BRIEF", NULL, cmd_brief, CMD_T_ANY, 0, 0},
  {"DESERT", NULL, cmd_desert, CMD_T_PLAYER | CMD_T_THING, 0, 0},
  {"DISMISS", NULL, cmd_dismiss, CMD_T_PLAYER | CMD_T_THING, 0, 0},
  {"DROP", NULL, cmd_drop, CMD_T_PLAYER | CMD_T_THING, 0, 0},
  {"EXAMINE", "ALL BRIEF DEBUG MORTAL", cmd_examine, CMD_T_ANY, 0, 0},
  {"ENTER", NULL, cmd_enter, CMD_T_ANY, 0, 0},

  {"FOLLOW", NULL, cmd_follow,
   CMD_T_PLAYER | CMD_T_THING | CMD_T_NOGAGGED, 0, 0},
  {"GET", NULL, cmd_get, CMD_T_PLAYER | CMD_T_THING | CMD_T_NOGAGGED, 0, 0},
  {"GIVE", "SILENT", cmd_give, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0,
   0},
  {"GOTO", NULL, cmd_goto, CMD_T_PLAYER | CMD_T_THING, 0, 0},
  {"INVENTORY", NULL, cmd_inventory, CMD_T_ANY, 0, 0},

  {"KILL", NULL, cmd_kill, CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
  {"LOOK", "OUTSIDE", cmd_look, CMD_T_ANY, 0, 0},
  {"LEAVE", NULL, cmd_leave, CMD_T_PLAYER | CMD_T_THING, 0, 0},
  {"MOVE", NULL, cmd_goto, CMD_T_PLAYER | CMD_T_THING, 0, 0},

  {"PAGE", "BLIND NOEVAL LIST PORT OVERRIDE", cmd_page,
   CMD_T_ANY | CMD_T_RS_NOPARSE | CMD_T_NOPARSE | CMD_T_EQSPLIT |
   CMD_T_NOGAGGED, 0, 0},
  {"POSE", "NOEVAL NOSPACE", cmd_pose, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},
  {"SCORE", NULL, cmd_score, CMD_T_ANY, 0, 0},
  {"SAY", "NOEVAL", cmd_say, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},
  {"SEMIPOSE", "NOEVAL", cmd_semipose, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},
  {"SLAY", NULL, cmd_slay, CMD_T_ANY, 0, 0},

  {"TAKE", NULL, cmd_take, CMD_T_PLAYER | CMD_T_THING | CMD_T_NOGAGGED,
   0, 0},
  {"TEACH", NULL, cmd_teach, CMD_T_ANY | CMD_T_NOPARSE, 0, 0},
  {"THINK", "NOEVAL", cmd_think, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},

  {"UNFOLLOW", NULL, cmd_unfollow,
   CMD_T_PLAYER | CMD_T_THING | CMD_T_NOGAGGED, 0, 0},
  {"USE", NULL, cmd_use, CMD_T_ANY | CMD_T_NOGAGGED, 0, 0},

  {"WHISPER", "LIST NOISY SILENT NOEVAL", cmd_whisper,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED, 0, 0},
  {"WITH", "NOEVAL", cmd_with, CMD_T_PLAYER | CMD_T_THING | CMD_T_EQSPLIT,
   0, 0},

/* ATTRIB_SET is an undocumented command - it's sugar to make it possible
 * enable/disable attribute setting with &XX or @XX
 */

  {"ATTRIB_SET", NULL, command_atrset,
   CMD_T_ANY | CMD_T_EQSPLIT | CMD_T_NOGAGGED | CMD_T_INTERNAL, 0, 0},
  {NULL, NULL, NULL, 0, 0, 0}
};


/* switch_list is defined in switchinc.c */
#include "switchinc.c"

struct command_perms_t command_perms[] = {
  {"player", CMD_T_PLAYER},
  {"thing", CMD_T_THING},
  {"exit", CMD_T_EXIT},
  {"room", CMD_T_ROOM},
  {"any", CMD_T_ANY},
  {"god", CMD_T_GOD},
  {"nobody", CMD_T_DISABLED},
  {"nogagged", CMD_T_NOGAGGED},
  {"noguest", CMD_T_NOGUEST},
  {"nofix", CMD_T_NOFIXED},
#ifdef DANGEROUS
  {"listed", CMD_T_LISTED},
  {"switches", CMD_T_SWITCHES},
  {"internal", CMD_T_INTERNAL},
  {"ls_space", CMD_T_LS_SPACE},
  {"ls_noparse", CMD_T_LS_NOPARSE},
  {"rs_space", CMD_T_RS_SPACE},
  {"rs_noparse", CMD_T_RS_NOPARSE},
  {"eqsplit", CMD_T_EQSPLIT},
  {"ls_args", CMD_T_LS_ARGS},
  {"rs_args", CMD_T_RS_ARGS},
#endif
  {NULL, 0}
};


static void
strccat(char *buff, char **bp, const char *from)
{
  if (*buff)
    safe_str(", ", buff, bp);
  safe_str(from, buff, bp);
}

static int
switch_find(COMMAND_INFO *cmd, char *sw)
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

/** Allocate and populate a COMMAND_INFO structure.
 * This function generates a new COMMAND_INFO structure, populates it
 * with given values, and returns a pointer. It should not be used
 * for local hacks - use command_add() instead.
 * \param name command name.
 * \param type types of objects that can use the command.
 * \param flagmask mask of flags (one is sufficient to use the command).
 * \param powers mask of powers (one is sufficient to use the command).
 * \param sw mask of switches the command accepts.
 * \param func function to call when the command is executed.
 * \return pointer to a newly allocated COMMAND_INFO structure.
 */
COMMAND_INFO *
make_command(const char *name, int type, object_flag_type flagmask, int powers,
	     switch_mask *sw, command_func func)
{
  COMMAND_INFO *cmd;
  cmd = (COMMAND_INFO *) mush_malloc(sizeof(COMMAND_INFO), "command");
  memset(cmd, 0, sizeof(COMMAND_INFO));
  cmd->name = name;
  cmd->func = func;
  cmd->type = type;
  cmd->flagmask = flagmask;
  cmd->powers = powers;
  if (sw)
    memcpy(cmd->sw, sw, sizeof(switch_mask));
  else
    SW_ZERO(cmd->sw);
  cmd->hooks.before.obj = NOTHING;
  cmd->hooks.before.attrname = NULL;
  cmd->hooks.after.obj = NOTHING;
  cmd->hooks.after.attrname = NULL;
  return cmd;
}

/** Add a new command to the command table.
 * This function is the top-level function for adding a new command.
 * \param name name of the command.
 * \param type types of objects that can use the command.
 * \param flagstr space-separated list of flags sufficient to use command.
 * \param powers mask of powers sufficient to use command.
 * \param switchstr space-separated list of switches for the command.
 * \param func function to call when command is executed.
 * \return pointer to a newly allocated COMMAND_INFO entry or NULL.
 */
COMMAND_INFO *
command_add(const char *name, int type, const char *flagstr,
	    int powers, const char *switchstr, command_func func)
{
  object_flag_type flagmask = NULL;
  switch_mask *sw = switchmask(switchstr);

  if (flagstr)
    flagmask = string_to_bits(flagstr);
  ptab_start_inserts(&ptab_command);
  ptab_insert(&ptab_command, name,
	      make_command(name, type, flagmask, powers, sw, func));
  ptab_end_inserts(&ptab_command);
  return command_find(name);
}


/** Search for a command by (partial) name.
 * This function searches the command table for a (partial) name match
 * and returns a pointer to the COMMAND_INFO entry. It returns NULL
 * if the name given is a reserved alias or if no command table entry
 * matches.
 * \param name name of command to match.
 * \return pointer to a COMMAND_INFO entry, or NULL.
 */
COMMAND_INFO *
command_find(const char *name)
{

  char cmdname[BUFFER_LEN];
  strcpy(cmdname, name);
  upcasestr(cmdname);
  if (hash_find(&htab_reserved_aliases, cmdname))
    return NULL;
  return (COMMAND_INFO *) ptab_find(&ptab_command, cmdname);
}

/** Search for a command by exact name.
 * This function searches the command table for an exact name match
 * and returns a pointer to the COMMAND_INFO entry. It returns NULL
 * if the name given is a reserved alias or if no command table entry
 * matches.
 * \param name name of command to match.
 * \return pointer to a COMMAND_INFO entry, or NULL.
 */
COMMAND_INFO *
command_find_exact(const char *name)
{

  char cmdname[BUFFER_LEN];
  strcpy(cmdname, name);
  upcasestr(cmdname);
  if (hash_find(&htab_reserved_aliases, cmdname))
    return NULL;
  return (COMMAND_INFO *) ptab_find_exact(&ptab_command, cmdname);
}


/** Modify a command's entry in the table.
 * Given a command name and other parameters, look up the command
 * in the table, and if it's there, modify the parameters.
 * \param name name of command to modify.
 * \param type new types for command, or -1 to leave unchanged.
 * \param flagmask new mask of flags for command, or NULL to leave unchanged.
 * \param powers new mask of powers for command, or -1 to leave unchanged.
 * \param sw new mask of switches for command, or NULL to leave unchanged.
 * \param func new function to call, or NULL to leave unchanged.
 * \return pointer to modified command entry, or NULL.
 */
COMMAND_INFO *
command_modify(const char *name, int type, object_flag_type flagmask,
	       int powers, switch_mask *sw, command_func func)
{
  COMMAND_INFO *cmd;
  cmd = command_find(name);
  if (!cmd)
    return NULL;
  if (type != -1)
    cmd->type = type;
  if (flagmask)
    cmd->flagmask = flagmask;
  if (powers != -1)
    cmd->powers = powers;
  if (sw)
    memcpy(cmd->sw, sw, sizeof(switch_mask));
  if (func)
    cmd->func = func;
  return cmd;
}

/** Convert a switch string to a switch mask.
 * Given a space-separated list of switches in string form, return
 * a pointer to a static switch mask.
 * \param switches list of switches as a string.
 * \return pointer to a static switch mask.
 */
switch_mask *
switchmask(const char *switches)
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

/** Add an alias to the table of reserved aliases.
 * This function adds an alias to the table of reserved aliases, preventing
 * it from being matched for standard commands. It's typically used to
 * insure that 'e' will match a global 'east;e' exit rather than the 
 * 'examine' command.
 * \param a alias to reserve.
 */
void
reserve_alias(const char *a)
{
  static char placeholder[2] = "x";
  hashadd(strupper(a), (void *) placeholder, &htab_reserved_aliases);
}

/** Initialize command tables (before reading config file).
 * This function performs command table initialization that should take place
 * before the configuration file has been read. It initializes the 
 * command prefix table and the reserved alias table, inserts all of the
 * commands from the commands array into the prefix table, initializes
 * the command permission prefix table, and inserts all the permissions
 * from the command_perms array into the prefix table. Finally, it
 * calls local_commands() to do any cmdlocal.c work.
 */
void
command_init_preconfig(void)
{
  struct command_perms_t *c;
  COMLIST *cmd;
  static int done = 0;
  if (done == 1)
    return;
  done = 1;

  ptab_init(&ptab_command);
  hashinit(&htab_reserved_aliases, 16, sizeof(COMMAND_INFO));
  reserve_aliases();
  ptab_start_inserts(&ptab_command);
  for (cmd = commands; cmd->name; cmd++) {
    ptab_insert(&ptab_command, cmd->name,
		make_command(cmd->name, cmd->type, string_to_bits(cmd->flagstr),
			     cmd->powers, switchmask(cmd->switches),
			     cmd->func));
  }
  ptab_end_inserts(&ptab_command);

  ptab_init(&ptab_command_perms);
  ptab_start_inserts(&ptab_command_perms);
  for (c = command_perms; c->name; c++)
    ptab_insert(&ptab_command_perms, c->name, c);
  ptab_end_inserts(&ptab_command_perms);

  local_commands();
}

/** Initialize commands (after reading config file).
 * This function performs command initialization that should take place
 * after the configuration file has been read.
 * Currently, there isn't any.
 */
void
command_init_postconfig(void)
{
  return;
}


/** Alias a command.
 * Given a command name and an alias for it, install the alias.
 * \param command canonical command name.
 * \param alias alias to associate with command.
 * \retval 0 failure (couldn't locate command).
 * \retval 1 success.
 */
int
alias_command(const char *command, const char *alias)
{
  COMMAND_INFO *cmd;

  /* Make sure the alias doesn't exit already */
  if (command_find_exact(alias))
    return 0;

  /* Look up the original */
  cmd = command_find_exact(command);
  if (!cmd)
    return 0;

  ptab_start_inserts(&ptab_command);
  ptab_insert(&ptab_command, strupper(alias), cmd);
  ptab_end_inserts(&ptab_command);
  return 1;
}

/* This is set to true for EQ_SPLIT commands that actually have a rhs.
 * Used in @teleport, ATTRSET and possibly other checks. It's ugly.
 * Blame Talek for it. ;)
 */
int rhs_present;

/** Parse the command arguments into arrays.
 * This function does the real work of parsing command arguments into
 * argument arrays. It is called separately to parse the left and
 * right sides of commands that are split at equal signs.
 * \param player the enactor.
 * \param cause the dbref causing the execution.
 * \param from pointer to address of where to parse arguments from.
 * \param to string to store parsed arguments into.
 * \param argv array of parsed arguments.
 * \param cmd pointer to command data.
 * \param right_side if true, parse on the right of the =. Otherwise, left.
 * \param forcenoparse if true, do no evaluation during parsing.
 */
void
command_argparse(dbref player, dbref cause, char **from, char *to,
		 char *argv[], COMMAND_INFO *cmd, int right_side,
		 int forcenoparse)
{
  int parse, split, args, i, done;
  char *t, *f;
  char *aold;

  f = *from;

  parse =
    (right_side) ? (cmd->type & CMD_T_RS_NOPARSE) : (cmd->type & CMD_T_NOPARSE);
  if (parse || forcenoparse)
    parse = PE_NOTHING;
  else
    parse = PE_DEFAULT | PE_COMMAND_BRACES;

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
    parse = PE_COMMAND_BRACES;

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
      if (i == MAX_ARG)
	done = 1;
    }
    if (split && (*f == '=')) {
      rhs_present = 1;
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

/** Determine whether a command is an attribute to set an attribute.
 * Is this command an attempt to set an attribute like @VA or &NUM?
 * If so, return the attrib's name. Otherwise, return NULL
 * \param command command string (first word of input).
 * \return name of the attribute to be set, or NULL.
 */
static const char *
command_isattr(char *command)
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
    buff[0] = '@';
    t = buff + 1;
    while ((*f) && (*f != '/'))
      *t++ = *f++;
    *t = '\0';
    /* @-commands have priority over @-attributes with the same name */
    if (command_find(buff))
      return NULL;
    a = atr_match(buff + 1);
    if (a)
      return AL_NAME(a);
  }
  return NULL;
}

/** Parse commands.
 * Parse the commands. This is the big one! 
 * We distinguish parsing of input sent from a player at a socket
 * (in which case attribute values to set are not evaluated) and
 * input sent in any other way (in which case attribute values to set
 * are evaluated, and attributes are set NO_COMMAND).
 * Return NULL if the command was recognized and handled, the evaluated
 * text to match against $-commands otherwise.
 * \param player the enactor.
 * \param cause dbref that caused the command to be executed.
 * \param string the input to be parsed.
 * \param fromport if true, command was typed by a player at a socket.
 * \return NULL if a command was handled, otherwise the evaluated input.
 */
char *
command_parse(dbref player, dbref cause, char *string, int fromport)
{
  char *command, *swtch, *ls, *rs, *switches;
  static char commandraw[BUFFER_LEN];
  char *lsa[MAX_ARG];
  char *rsa[MAX_ARG];
  char *ap, *swp;
  const char *attrib, *replacer;
  COMMAND_INFO *cmd;
  char *p, *t, *c;
  char b;
  int switchnum;
  switch_mask sw;
  int noeval;

  rhs_present = 0;

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
  case '#':
    if (!Gagged(player) && Mobile(player))
      force_by_number(player, string);
    /* Fall though is cool */
  case '\0':
    /* Just in case. You never know */
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
    if (*(p + 1) && *(p + 1) == ' ')
      replacer = "POSE";
    else
      replacer = "SEMIPOSE";
    break;
  case EMIT_TOKEN:
    replacer = "@EMIT";
    break;
#ifdef CHAT_SYSTEM
  case CHAT_TOKEN:
#ifdef CHAT_TOKEN_ALIAS
  case CHAT_TOKEN_ALIAS:
#endif
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
		       (PE_DEFAULT & ~PE_FUNCTION_CHECK) | PE_COMMAND_BRACES,
		       PT_SPACE, NULL);
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
  /* Test if this either isn't a command, or is a disabled one
   * If so, return Fully Parsed string for further processing.
   */

  if (!cmd || (cmd->type & CMD_T_DISABLED)) {
    if (!cmd) {
      c = commandraw + strlen(commandraw);
    } else {
      if (replacer) {
	/* These commands don't allow switches, and need a space
	 * added after their canonical name
	 */
	c = commandraw;
	safe_str(cmd->name, commandraw, &c);
	safe_chr(' ', commandraw, &c);
      } else if (*c == '/') {
	/* Oh... DAMN */
	c = commandraw;
	strcpy(switches, commandraw);
	safe_str(cmd->name, commandraw, &c);
	t = strchr(switches, '/');
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
      process_expression(commandraw, &c, (const char **) &p, player, cause,
			 cause,
			 (PE_DEFAULT & ~PE_FUNCTION_CHECK) | PE_COMMAND_BRACES,
			 PT_DEFAULT, NULL);
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
	  notify_format(player,
			T("%s doesn't know switch %s."), cmd->name, swtch);
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
    swp = (char *) attrib;
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
    do_rawlog(LT_ERR, T("No command vector on command %s."), cmd->name);
  } else {
    char *saveregs[NUMQ] = { NULL };
    run_hook(player, cause, &cmd->hooks.before, saveregs, 1);
    cmd->func(cmd, player, cause, sw, string, swp, ap, ls, lsa, rs, rsa);
    run_hook(player, cause, &cmd->hooks.after, saveregs, 0);
  }

  mush_free((Malloc_t) command, "string");
  mush_free((Malloc_t) swtch, "string");
  mush_free((Malloc_t) ls, "string");
  mush_free((Malloc_t) rs, "string");
  mush_free((Malloc_t) switches, "string");
  return NULL;
}

/** Add a restriction to a command.
 * Given a command name and a restriction, apply the restriction to the
 * command in addition to whatever its usual restrictions are.
 * This is used by the configuration file startup in conf.c
 * Valid restrictions are:
 * \verbatim
 *   nobody     disable the command
 *   nogagged   can't be used by gagged players
 *   nofixed    can't be used by fixed players
 *   noguest    can't be used by guests
 *   admin      can only be used by royalty or wizards
 *   wizard     can only be used by wizards
 *   god        can only be used by god
 *   noplayer   can't be used by players, just objects/rooms/exits
 * \endverbatim
 * Return 1 on success, 0 on failure.
 * \param name name of command to restrict.
 * \param restriction space-separated string of restrictions
 * \retval 1 successfully restricted command.
 * \retval 0 failure (unable to find command name).
 */
int
restrict_command(const char *name, const char *restriction)
{
  COMMAND_INFO *command;
  struct command_perms_t *c;
  int clear;
  FLAG *mask;
  int powers;
  char *tp;

  if (!name || !*name || !restriction || !*restriction ||
      !(command = command_find(name)))
    return 0;

  while (restriction && *restriction) {
    if ((tp = strrchr(restriction, ' ')))
      *tp++ = '\0';

    clear = 0;
    if (*restriction == '!') {
      restriction++;
      clear = 1;
    }

    if (!strcasecmp(restriction, "noplayer")) {
      /* Pfft. And even !noplayer works. */
      clear = !clear;
      restriction += 2;
    }

    /* Gah. I love backwards compatiblity. */
    if (!strcasecmp(restriction, "admin")) {
      FLAG *roy = match_flag("ROYALTY");
      FLAG *wiz = match_flag("WIZARD");
      if (clear && command->flagmask) {
	clear_flag_bitmask(command->flagmask, roy->bitpos);
	clear_flag_bitmask(command->flagmask, wiz->bitpos);
      } else {
	if (!command->flagmask)
	  command->flagmask = new_flag_bitmask();
	set_flag_bitmask(command->flagmask, roy->bitpos);
	set_flag_bitmask(command->flagmask, wiz->bitpos);
      }
    } else if ((c = ptab_find(&ptab_command_perms, restriction))) {
      if (clear)
	command->type &= ~c->type;
      else
	command->type |= c->type;
    } else if ((mask = match_flag(restriction))) {
      if (clear && command->flagmask)
	clear_flag_bitmask(command->flagmask, mask->bitpos);
      else {
	if (!command->flagmask)
	  command->flagmask = new_flag_bitmask();
	set_flag_bitmask(command->flagmask, mask->bitpos);
      }
    } else if ((powers = match_power(restriction))) {
      if (clear)
	command->powers &= ~powers;
      else
	command->powers |= powers;
    }
    restriction = tp;
  }
  return 1;
}

/** Definition of the \@command command.
 * This is the only command which should be defined in this
 * file, because it uses variables from this file, etc.
 */
COMMAND (cmd_command) {
  COMMAND_INFO *command;
  SWITCH_VALUE *sw_val;
  char buff[BUFFER_LEN];
  char *bp = buff;

  if (!arg_left[0]) {
    notify(player, T("You must specify a command."));
    return;
  }
  command = command_find(arg_left);
  if (!command) {
    notify(player, T("No such command."));
    return;
  }
  if (Wizard(player)) {
    if (SW_ISSET(sw, SWITCH_ON) || SW_ISSET(sw, SWITCH_ENABLE))
      command->type &= ~CMD_T_DISABLED;
    else if (SW_ISSET(sw, SWITCH_OFF) || SW_ISSET(sw, SWITCH_DISABLE))
      command->type |= CMD_T_DISABLED;

    if (SW_ISSET(sw, SWITCH_RESTRICT)) {
      if (!arg_right || !arg_right[0]) {
	notify(player, T("How do you want to restrict the command?"));
	return;
      }

      if (!restrict_command(arg_left, arg_right))
	notify(player, T("Restrict attempt failed."));
    }

    if ((command->func == cmd_command) && (command->type & CMD_T_DISABLED)) {
      notify(player, T("@command is ALWAYS enabled."));
      command->type &= ~CMD_T_DISABLED;
    }
  }
  if (!SW_ISSET(sw, SWITCH_QUIET)) {
    notify_format(player,
		  "Name       : %s (%s)", command->name,
		  (command->type & CMD_T_DISABLED) ? "Disabled" : "Enabled");
    if ((command->type & CMD_T_ANY) == CMD_T_ANY)
      safe_strl("Any", 3, buff, &bp);
    else {
      buff[0] = '\0';
      if (command->type & CMD_T_ROOM)
	strccat(buff, &bp, "Room");
      if (command->type & CMD_T_THING)
	strccat(buff, &bp, "Thing");
      if (command->type & CMD_T_EXIT)
	strccat(buff, &bp, "Exit");
      if (command->type & CMD_T_PLAYER)
	strccat(buff, &bp, "Player");
    }
    *bp = '\0';
    notify_format(player, "Types      : %s", buff);
    buff[0] = '\0';
    bp = buff;
    if (command->type & CMD_T_SWITCHES)
      strccat(buff, &bp, "Switches");
    if (command->type & CMD_T_NOGAGGED)
      strccat(buff, &bp, "Nogag");
    if (command->type & CMD_T_NOFIXED)
      strccat(buff, &bp, "Nofix");
    if (command->type & CMD_T_NOGUEST)
      strccat(buff, &bp, "Noguest");
    if (command->type & CMD_T_EQSPLIT)
      strccat(buff, &bp, "Eqsplit");
    if (command->type & CMD_T_GOD)
      strccat(buff, &bp, "God");
    *bp = '\0';
    notify_format(player, "Restrict   : %s", buff);
    buff[0] = '\0';
    notify(player, show_command_flags(command->flagmask, command->powers));
    bp = buff;
    for (sw_val = switch_list; sw_val->name; sw_val++)
      if (SW_ISSET(command->sw, sw_val->value))
	strccat(buff, &bp, sw_val->name);
    *bp = '\0';
    notify_format(player, "Switches   : %s", buff);
    buff[0] = '\0';
    bp = buff;
    if (command->type & CMD_T_LS_ARGS) {
      if (command->type & CMD_T_LS_SPACE)
	strccat(buff, &bp, "Space-Args");
      else
	strccat(buff, &bp, "Args");
    }
    if (command->type & CMD_T_LS_NOPARSE)
      strccat(buff, &bp, "Noparse");
    if (command->type & CMD_T_EQSPLIT) {
      *bp = '\0';
      notify_format(player, "Leftside   : %s", buff);
      buff[0] = '\0';
      bp = buff;
      if (command->type & CMD_T_RS_ARGS) {
	if (command->type & CMD_T_RS_SPACE)
	  strccat(buff, &bp, "Space-Args");
	else
	  strccat(buff, &bp, "Args");
      }
      if (command->type & CMD_T_RS_NOPARSE)
	strccat(buff, &bp, "Noparse");
      *bp = '\0';
      notify_format(player, "Rightside  : %s", buff);
    } else {
      *bp = '\0';
      notify_format(player, "Arguments  : %s", buff);
    }
    if (Wizard(player)) {
      if (GoodObject(command->hooks.before.obj))
	notify_format(player, "@hook/before: #%d/%s", command->hooks.before.obj,
		      command->hooks.before.attrname);
      if (GoodObject(command->hooks.after.obj))
	notify_format(player, "@hook/after: #%d/%s", command->hooks.after.obj,
		      command->hooks.after.attrname);
    }
  }
}

/** Display a list of defined commands.
 * This function sends a player the list of commands.
 * \param player the enactor.
 * \param lc if true, list is in lowercase rather than uppercase.
 */
void
do_list_commands(dbref player, int lc)
{
  char *b = list_commands();
  notify_format(player, "Commands: %s", lc ? strlower(b) : b);
}

/** Return a list of defined commands.
 * This function returns a space-separated list of commands as a string.
 */
char *
list_commands(void)
{
  COMMAND_INFO *command;
  const char *ptrs[BUFFER_LEN / 2];
  static char buff[BUFFER_LEN];
  char *bp;
  int nptrs = 0, i;
  command = (COMMAND_INFO *) ptab_firstentry(&ptab_command);
  while (command) {
    ptrs[nptrs] = command->name;
    nptrs++;
    command = (COMMAND_INFO *) ptab_nextentry(&ptab_command);
  }
  bp = buff;
  safe_str(ptrs[0], buff, &bp);
  for (i = 1; i < nptrs; i++) {
    safe_chr(' ', buff, &bp);
    safe_str(ptrs[i], buff, &bp);
  }
  *bp = '\0';
  return buff;
}


/* Check command permissions. Return 1 if player can use command,
 * 0 otherwise, and maybe be noisy about it.
 */
static int
command_check(dbref player, COMMAND_INFO *cmd)
{
  int ok;
  int check_flags;

  /* If disabled, return silently */
  if (cmd->type & CMD_T_DISABLED)
    return 0;
  if ((cmd->type & CMD_T_NOGAGGED) && Gagged(player)) {
    notify(player, T("You cannot do that while gagged."));
    return 0;
  }
  if ((cmd->type & CMD_T_NOFIXED) && Fixed(player)) {
    notify(player, T("You cannot do that while fixed."));
    return 0;
  }
  if ((cmd->type & CMD_T_NOGUEST) && Guest(player)) {
    notify(player, T("Guests cannot do that."));
    return 0;
  }
  if ((cmd->type & CMD_T_GOD) && (!God(player))) {
    notify(player, T("Only God can do that."));
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
    notify(player, T("Permission denied, command is type-restricted."));
    return 0;
  }
  /* A command can specify required flags or powers, and if
   * any match, the player is ok to do the command.
   */
  ok = 1;
  check_flags = cmd->flagmask && !null_flagmask(cmd->flagmask);

  if (!((!cmd->powers && !check_flags) ||
	(cmd->powers && (cmd->powers & Powers(player))) ||
	(check_flags && has_any_flags_by_mask(player, cmd->flagmask)))) {
    notify(player, T("Permission denied."));
    ok = 0;
  }
  return ok;
}

/** Determine whether a player can use a command.
 * This function checks whether a player can use a command.
 * If the command is disallowed, the player is informed.
 * \param player player whose privileges are checked.
 * \param name name of command.
 * \retval 0 player may not use command.
 * \retval 1 player may use command.
 */
int
command_check_byname(dbref player, const char *name)
{
  COMMAND_INFO *cmd;
  cmd = command_find(name);
  if (!cmd)
    return 0;
  return command_check(player, cmd);
}

/** Run a before or after command hook.
 * This function runs a hook before or after a command execution.
 * \param player the enactor.
 * \param cause dbref that caused command to execute.
 * \param hook pointer to the hook.
 * \param saveregs array to store a copy of the final q-registers.
 * \param save if true, use saveregs to store a ending q-registers.
 */
void
run_hook(dbref player, dbref cause, struct hook_data *hook, char *saveregs[],
	 int save)
{
  ATTR *atr;
  char *code;
  const char *cp;
  char buff[BUFFER_LEN], *bp;
  char *origregs[NUMQ];

  if (!hook || !GoodObject(hook->obj) || IsGarbage(hook->obj)
      || !hook->attrname)
    return;

  atr = atr_get(hook->obj, hook->attrname);

  if (!atr)
    return;

  code = safe_uncompress(AL_STR(atr));
  if (!code)
    return;
#ifdef MEM_CHECK
  add_check("hook.code");
#endif

  save_global_regs("run_hook", origregs);
  restore_global_regs("hook.regs", saveregs);

  cp = code;
  bp = buff;

  process_expression(buff, &bp, &cp, hook->obj, cause, player, PE_DEFAULT,
		     PT_DEFAULT, NULL);

  if (save)
    save_global_regs("hook.regs", saveregs);
  restore_global_regs("run_hook", origregs);

  mush_free(code, "hook.code");
}

/** Set up or remove a command hook.
 * \verbatim
 * This is the top-level function for @hook. If an object and attribute
 * are given, establishes a hook; if neither are given, removes a hook.
 * \endverbatim
 * \param player the enactor.
 * \param command command to hook.
 * \param obj name of object containing the hook attribute.
 * \param attrname of hook attribute on obj.
 * \param hook_type 1 (before) or 2 (after).
 */
void
do_hook(dbref player, char *command, char *obj, char *attrname, int hook_type)
{
  COMMAND_INFO *cmd;

  cmd = command_find(command);
  if (!cmd) {
    notify(player, T("No such command."));
    return;
  }
  if ((cmd->func == cmd_password) || (cmd->func == cmd_newpassword)) {
    notify(player, T("Hooks not allowed with that command."));
    return;
  }

  if (!obj && !attrname) {
    struct hook_data *h;
    if (hook_type == 1)
      h = &cmd->hooks.before;
    else if (hook_type == 2)
      h = &cmd->hooks.after;
    else {
      notify(player, T("Unknown hook type"));
      return;
    }
    notify_format(player, T("Hook removed from %s."), cmd->name);
    h->obj = NOTHING;
    mush_free(h->attrname, "hook.attr");
    h->attrname = NULL;
  } else if (!obj || !*obj || !attrname || !*attrname) {
    notify(player, T("You must give both an object and attribute."));
  } else {
    dbref objdb = match_thing(player, obj);
    struct hook_data *h;
    if (!GoodObject(objdb))
      return;
    if (hook_type == 1)
      h = &cmd->hooks.before;
    else if (hook_type == 2)
      h = &cmd->hooks.after;
    else {
      notify(player, T("Unknown hook type"));
      return;
    }
    h->obj = objdb;
    if (h->attrname)
      mush_free(h->attrname, "hook.attr");
    h->attrname = mush_strdup(strupper(attrname), "hook.attr");
    notify_format(player, T("Hook set for %s"), cmd->name);
  }
}
