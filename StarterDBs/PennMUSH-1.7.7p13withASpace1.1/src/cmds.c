/**
 * \file cmds.c
 *
 * \brief Definitions of commands.
 * This file is a set of functions that defines commands. The parsing
 * of commands is elsewhere (command.c), as are the implementations
 * of most of the commands (throughout the source.)
 *
 */

#include "copyrite.h"
#include "config.h"

#include <string.h>

#include "conf.h"
#include "dbdefs.h"
#include "mushdb.h"
#include "externs.h"
#include "match.h"
#include "game.h"
#include "attrib.h"
#include "extmail.h"
#include "malias.h"
#include "getpgsiz.h"
#include "parse.h"
#include "access.h"
#include "version.h"
#include "lock.h"
#include "function.h"
#include "command.h"
#include "flags.h"
#include "log.h"
#include "confmagic.h"

/* External Stuff */
void do_poor(dbref player, char *arg1);
void do_list_memstats(dbref player);

#define DOL_MAP 1		/**< dolist/map bitflag */
#define DOL_NOTIFY 2		/**< dolist/notify bitflag */
#define DOL_DELIM 4		/**< dolist/delim bitflag */

void do_dolist(dbref player, char *list, char *command,
	       dbref cause, unsigned int flags);
void do_list(dbref player, char *arg, int lc);
void do_writelog(dbref player, char *str, int ltype);
void do_readcache(dbref player);
void do_scan(dbref player, char *command, int flag);
void do_uptime(dbref player, int mortal);
extern int config_set(const char *opt, char *val, int source, int restrictions);

/* From command.c */
extern int rhs_present;
void do_hook(dbref player, char *command, char *obj, char *attrname,
	     int hook_type);

COMMAND (cmd_allhalt) {
  do_allhalt(player);
}

#ifdef QUOTA
COMMAND (cmd_allquota) {
  do_allquota(player, arg_left, SW_ISSET(sw, SWITCH_QUIET));
}

#endif

COMMAND (cmd_atrlock) {
  do_atrlock(player, arg_left, arg_right);
}

COMMAND (cmd_attribute) {
  if (SW_ISSET(sw, SWITCH_ACCESS))
    do_attribute_access(player, arg_left, arg_right,
			SW_ISSET(sw, SWITCH_RETROACTIVE));
  else if (SW_ISSET(sw, SWITCH_DELETE))
    do_attribute_delete(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_RENAME))
    do_attribute_rename(player, arg_left, arg_right);
  else
    do_attribute_info(player, arg_left);
}

COMMAND (cmd_atrchown) {
  do_atrchown(player, arg_left, arg_right);
}

COMMAND (cmd_boot) {
  if (SW_ISSET(sw, SWITCH_PORT))
    do_boot(player, arg_left, 1);
  else if (SW_ISSET(sw, SWITCH_ME))
    do_boot(player, (char *) NULL, 2);
  else
    do_boot(player, arg_left, 0);
}

extern int break_called;
COMMAND (cmd_break) {
  if (parse_boolean(arg_left))
    break_called = 1;
}

COMMAND (cmd_chownall) {
  do_chownall(player, arg_left, arg_right, SW_ISSET(sw, SWITCH_PRESERVE));
}

COMMAND (cmd_chown) {
  do_chown(player, arg_left, arg_right, SW_ISSET(sw, SWITCH_PRESERVE));
}

COMMAND (cmd_chzoneall) {
  do_chzoneall(player, arg_left, arg_right);
}

COMMAND (cmd_chzone) {
  (void) do_chzone(player, arg_left, arg_right, 1);
}

COMMAND (cmd_config) {
  int lc;
  lc = SW_ISSET(sw, SWITCH_LOWERCASE);
  if (SW_ISSET(sw, SWITCH_GLOBALS))
    do_config_list(player, NULL, lc);
  else if (SW_ISSET(sw, SWITCH_DEFAULTS))
    do_config_list(player, NULL, lc);
  else if (SW_ISSET(sw, SWITCH_COSTS))
    do_config_list(player, NULL, lc);
  else if (SW_ISSET(sw, SWITCH_FUNCTIONS))
    do_list_functions(player, lc);
  else if (SW_ISSET(sw, SWITCH_COMMANDS))
    do_list_commands(player, lc);
  else if (SW_ISSET(sw, SWITCH_ATTRIBS))
    do_list_attribs(player, lc);
  else if (SW_ISSET(sw, SWITCH_LIST))
    do_config_list(player, arg_left, lc);
  else if (SW_ISSET(sw, SWITCH_FLAGS))
    do_list_flags(player, arg_left, lc);
  else if (SW_ISSET(sw, SWITCH_SET)) {
    if (!Wizard(player)) {
      notify(player, T("You can't remake the world in your image."));
      return;
    }
    if (!arg_left || !*arg_left) {
      notify(player, T("What did you want to set?"));
      return;
    }
    if (!config_set(arg_left, arg_right, 1, 0)
	&& !config_set(arg_left, arg_right, 1, 1))
      notify(player, T("Couldn't set that option"));
    else
      notify(player, T("Option set."));
  } else
    do_config_list(player, arg_left, lc);
}

COMMAND (cmd_cpattr) {
  do_cpattr(player, arg_left, args_right, 0, SW_ISSET(sw, SWITCH_NOFLAGCOPY));
}

COMMAND (cmd_create) {
  do_create(player, arg_left, atol(arg_right));
}

COMMAND (cmd_clone) {
  if (SW_ISSET(sw, SWITCH_PRESERVE))
    do_clone(player, arg_left, arg_right, SWITCH_PRESERVE);
  else
    do_clone(player, arg_left, arg_right, SWITCH_NONE);
}

COMMAND (cmd_dbck) {
  do_dbck(player);
}

COMMAND (cmd_decompile) {
  if (SW_ISSET(sw, SWITCH_DB))
    do_decompile(player, arg_left, DEC_DB, SW_ISSET(sw, SWITCH_SKIPDEFAULTS));
  else if (SW_ISSET(sw, SWITCH_TF))
    do_decompile(player, arg_left, DEC_TF, SW_ISSET(sw, SWITCH_SKIPDEFAULTS));
  else if (SW_ISSET(sw, SWITCH_FLAGS))
    do_decompile(player, arg_left, DEC_FLAG, SW_ISSET(sw, SWITCH_SKIPDEFAULTS));
  else if (SW_ISSET(sw, SWITCH_ATTRIBS))
    do_decompile(player, arg_left, DEC_ATTR, SW_ISSET(sw, SWITCH_SKIPDEFAULTS));
  else
    do_decompile(player, arg_left, DEC_NORMAL,
		 SW_ISSET(sw, SWITCH_SKIPDEFAULTS));
}

COMMAND (cmd_teach) {
  do_teach(player, cause, arg_left);
}

COMMAND (cmd_destroy) {
  do_destroy(player, arg_left, (SW_ISSET(sw, SWITCH_OVERRIDE)));
}

COMMAND (cmd_dig) {
  do_dig(player, arg_left, args_right, (SW_ISSET(sw, SWITCH_TELEPORT)));
}

COMMAND (cmd_disable) {
  do_enable(player, arg_left, 0);
}

COMMAND (cmd_doing) {
  if (SW_ISSET(sw, SWITCH_HEADER))
    do_poll(player, arg_left);
  else
    do_doing(player, arg_left);
}

COMMAND (cmd_dolist) {
  unsigned int flags = 0;
  if (SW_ISSET(sw, SWITCH_NOTIFY))
    flags |= DOL_NOTIFY;
  if (SW_ISSET(sw, SWITCH_DELIMIT))
    flags |= DOL_DELIM;
  do_dolist(player, arg_left, arg_right, cause, flags);
}

COMMAND (cmd_dump) {
  if (SW_ISSET(sw, SWITCH_PARANOID))
    do_dump(player, arg_left, DUMP_PARANOID);
  else if (SW_ISSET(sw, SWITCH_DEBUG))
    do_dump(player, arg_left, DUMP_DEBUG);
  else
    do_dump(player, arg_left, DUMP_NORMAL);
}

COMMAND (cmd_edit) {
  do_gedit(player, arg_left, args_right);
}

COMMAND (cmd_elock) {
  do_lock(player, arg_left, arg_right, Enter_Lock);
}

COMMAND (cmd_emit) {
  if (SW_ISSET(sw, SWITCH_ROOM))
    do_lemit(player, arg_left, SW_ISSET(sw, SWITCH_SILENT));
  else
    do_emit(player, arg_left);
}

COMMAND (cmd_enable) {
  do_enable(player, arg_left, 1);
}

COMMAND (cmd_entrances) {
  if (SW_ISSET(sw, SWITCH_EXITS))
    do_entrances(player, arg_left, args_right, 1);
  else if (SW_ISSET(sw, SWITCH_THINGS))
    do_entrances(player, arg_left, args_right, 2);
  else if (SW_ISSET(sw, SWITCH_PLAYERS))
    do_entrances(player, arg_left, args_right, 3);
  else if (SW_ISSET(sw, SWITCH_ROOMS))
    do_entrances(player, arg_left, args_right, 4);
  else
    do_entrances(player, arg_left, args_right, 0);
}

COMMAND (cmd_eunlock) {
  do_unlock(player, arg_left, Enter_Lock);
}

COMMAND (cmd_find) {
  do_find(player, arg_left, args_right);
}

COMMAND (cmd_firstexit) {
  do_firstexit(player, arg_left);
}

COMMAND (cmd_flag) {
  if (SW_ISSET(sw, SWITCH_LIST))
    do_list_flags(player, arg_left, 0);
  else if (SW_ISSET(sw, SWITCH_ADD))
    do_flag_add(player, arg_left, args_right);
  else if (SW_ISSET(sw, SWITCH_DELETE))
    do_flag_delete(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_ALIAS))
    do_flag_alias(player, arg_left, args_right[1]);
  else if (SW_ISSET(sw, SWITCH_RESTRICT))
    do_flag_restrict(player, arg_left, args_right);
  else if (SW_ISSET(sw, SWITCH_DISABLE))
    do_flag_disable(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_ENABLE))
    do_flag_enable(player, arg_left);
  else
    do_flag_info(player, arg_left);
}

COMMAND (cmd_force) {
  do_force(player, arg_left, arg_right);
}

COMMAND (cmd_function) {
  if (SW_ISSET(sw, SWITCH_DELETE))
    do_function_delete(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_ENABLE))
    do_function_toggle(player, arg_left, 1);
  else if (SW_ISSET(sw, SWITCH_DISABLE))
    do_function_toggle(player, arg_left, 0);
  else if (SW_ISSET(sw, SWITCH_RESTRICT))
    do_function_restrict(player, arg_left, args_right[1]);
  else if (SW_ISSET(sw, SWITCH_RESTORE))
    do_function_restore(player, arg_left);
  else {
    int split;
    char *saved;
    split = 0;
    saved = NULL;
    if (args_right[1] && *args_right[1] && !(args_right[2] && *args_right[2])) {
      split = 1;
      saved = args_right[2];
      if ((args_right[2] = strchr(args_right[1], '/')) == NULL) {
	notify(player, T("#-1 INVALID SECOND ARGUMENT"));
	return;
      }
      *args_right[2]++ = '\0';
    }
    if (args_right[1] && *args_right[1])
      do_function(player, arg_left, args_right);
    else if (arg_left && *arg_left)
      do_function_report(player, arg_left);
    else
      do_function(player, NULL, NULL);
    if (split) {
      if (args_right[2])
	*--args_right[2] = '/';
      args_right[2] = saved;
    }
  }
}

COMMAND (cmd_gedit) {
  do_gedit(player, arg_left, args_right);
}

COMMAND (cmd_grep) {
  do_grep(player, arg_left, arg_right, ((SW_ISSET(sw, SWITCH_IPRINT))
					|| (SW_ISSET(sw, SWITCH_PRINT))),
	  ((SW_ISSET(sw, SWITCH_IPRINT))
	   || (SW_ISSET(sw, SWITCH_ILIST))));
}

COMMAND (cmd_halt) {
  if (SW_ISSET(sw, SWITCH_ALL))
    do_allhalt(player);
  else
    do_halt1(player, arg_left, arg_right);
}

COMMAND (cmd_hide) {
  hide_player(player, !(SW_ISSET(sw, SWITCH_NO) || SW_ISSET(sw, SWITCH_OFF)));
}

COMMAND (cmd_hook) {
  int hook_type = 0;

  if (!Wizard(player)) {
    notify(player, T("You need a fishing license to use that hook."));
    return;
  }

  if (SW_ISSET(sw, SWITCH_AFTER))
    hook_type = 2;
  else if (SW_ISSET(sw, SWITCH_BEFORE))
    hook_type = 1;
  else {
    notify(player, T("You must give a switch for @hook"));
    return;
  }
  do_hook(player, arg_left, args_right[1], args_right[2], hook_type);
}

COMMAND (cmd_kick) {
  do_kick(player, arg_left);
}

COMMAND (cmd_lemit) {
  do_lemit(player, arg_left, SW_ISSET(sw, SWITCH_SILENT));
}

COMMAND (cmd_link) {
  do_link(player, arg_left, arg_right, SW_ISSET(sw, SWITCH_PRESERVE));
}

COMMAND (cmd_listmotd) {
  do_motd(player, MOTD_LIST, "");
}

COMMAND (cmd_list) {
  int lc;
  lc = SW_ISSET(sw, SWITCH_LOWERCASE);
  if (SW_ISSET(sw, SWITCH_MOTD))
    do_motd(player, MOTD_LIST, "");
  else if (SW_ISSET(sw, SWITCH_FUNCTIONS))
    do_list_functions(player, lc);
  else if (SW_ISSET(sw, SWITCH_COMMANDS))
    do_list_commands(player, lc);
  else if (SW_ISSET(sw, SWITCH_ATTRIBS))
    do_list_attribs(player, lc);
  else if (SW_ISSET(sw, SWITCH_FLAGS))
    do_list_flags(player, arg_left, lc);
  else
    do_list(player, arg_left, lc);
}

COMMAND (cmd_lock) {
  if ((switches) && (switches[0]))
    do_lock(player, arg_left, arg_right, switches);
  else
    do_lock(player, arg_left, arg_right, Basic_Lock);
}

COMMAND (cmd_log) {
  if (SW_ISSET(sw, SWITCH_CHECK))
    do_writelog(player, arg_left, LT_CHECK);
  else if (SW_ISSET(sw, SWITCH_CMD))
    do_writelog(player, arg_left, LT_CMD);
  else if (SW_ISSET(sw, SWITCH_CONN))
    do_writelog(player, arg_left, LT_CONN);
  else if (SW_ISSET(sw, SWITCH_ERR))
    do_writelog(player, arg_left, LT_ERR);
  else if (SW_ISSET(sw, SWITCH_TRACE))
    do_writelog(player, arg_left, LT_TRACE);
  else if (SW_ISSET(sw, SWITCH_WIZ))
    do_writelog(player, arg_left, LT_WIZ);
  else
    do_writelog(player, arg_left, LT_CMD);
}

COMMAND (cmd_logwipe) {
  if (SW_ISSET(sw, SWITCH_CHECK))
    do_logwipe(player, LT_CHECK, arg_left);
  else if (SW_ISSET(sw, SWITCH_CMD))
    do_logwipe(player, LT_CMD, arg_left);
  else if (SW_ISSET(sw, SWITCH_CONN))
    do_logwipe(player, LT_CONN, arg_left);
  else if (SW_ISSET(sw, SWITCH_TRACE))
    do_logwipe(player, LT_TRACE, arg_left);
  else if (SW_ISSET(sw, SWITCH_WIZ))
    do_logwipe(player, LT_WIZ, arg_left);
  else
    do_logwipe(player, LT_ERR, arg_left);
}

COMMAND (cmd_lset) {
  do_lset(player, arg_left, arg_right);
}

#ifdef USE_MAILER
COMMAND (cmd_mail) {
  int urgent = SW_ISSET(sw, SWITCH_URGENT);
  int silent = SW_ISSET(sw, SWITCH_SILENT);
  int nosig = SW_ISSET(sw, SWITCH_NOSIG);
  /* First, mail commands that can be used even if you're gagged */
  if (SW_ISSET(sw, SWITCH_STATS))
    do_mail_stats(player, arg_left, 0);
  else if (SW_ISSET(sw, SWITCH_DSTATS))
    do_mail_stats(player, arg_left, 1);
  else if (SW_ISSET(sw, SWITCH_FSTATS))
    do_mail_stats(player, arg_left, 2);
  else if (SW_ISSET(sw, SWITCH_DEBUG))
    do_mail_debug(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_NUKE))
    do_mail_nuke(player);
  else if (SW_ISSET(sw, SWITCH_FOLDERS))
    do_mail_change_folder(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_UNFOLDER))
    do_mail_unfolder(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_LIST))
    do_mail_list(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_READ))
    do_mail_read(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_CLEAR))
    do_mail_clear(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_UNCLEAR))
    do_mail_unclear(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_PURGE))
    do_mail_purge(player);
  else if (SW_ISSET(sw, SWITCH_FILE))
    do_mail_file(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_TAG))
    do_mail_tag(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_UNTAG))
    do_mail_untag(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_FWD) || SW_ISSET(sw, SWITCH_FORWARD)
	   || SW_ISSET(sw, SWITCH_SEND) || silent || urgent || nosig) {
    /* These commands are not allowed to gagged players */
    if (Gagged(player)) {
      notify(player, T("You cannot do that while gagged."));
      return;
    }
    if (SW_ISSET(sw, SWITCH_FWD))
      do_mail_fwd(player, arg_left, arg_right);
    else if (SW_ISSET(sw, SWITCH_FORWARD))
      do_mail_fwd(player, arg_left, arg_right);
    else if (SW_ISSET(sw, SWITCH_SEND) || silent || urgent || nosig)
      do_mail_send(player, arg_left, arg_right,
		   urgent ? M_URGENT : 0, silent, nosig);
  } else
    do_mail(player, arg_left, arg_right);	/* Does its own gagged check */
}

#ifdef MAIL_ALIASES

COMMAND (cmd_malias) {
  if (SW_ISSET(sw, SWITCH_LIST))
    do_malias_list(player);
  else if (SW_ISSET(sw, SWITCH_ALL))
    do_malias_all(player);
  else if (SW_ISSET(sw, SWITCH_MEMBERS) || SW_ISSET(sw, SWITCH_WHO))
    do_malias_members(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_CREATE))
    do_malias_create(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_SET))
    do_malias_set(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_DESTROY))
    do_malias_destroy(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_ADD))
    do_malias_add(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_REMOVE))
    do_malias_remove(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_DESCRIBE))
    do_malias_desc(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_RENAME))
    do_malias_rename(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_STATS))
    do_malias_stats(player);
  else if (SW_ISSET(sw, SWITCH_CHOWN))
    do_malias_chown(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_USEFLAG))
    do_malias_privs(player, arg_left, arg_right, 0);
  else if (SW_ISSET(sw, SWITCH_SEEFLAG))
    do_malias_privs(player, arg_left, arg_right, 1);
  else if (SW_ISSET(sw, SWITCH_NUKE))
    do_malias_nuke(player);
  else
    do_malias(player, arg_left, arg_right);
}
#endif
#endif

COMMAND (cmd_map) {
  unsigned int flags = DOL_MAP;
  if (SW_ISSET(sw, SWITCH_DELIMIT))
    flags |= DOL_DELIM;
  do_dolist(player, arg_left, arg_right, cause, flags);
}

COMMAND (cmd_motd) {
  if (SW_ISSET(sw, SWITCH_CONNECT))
    do_motd(player, MOTD_MOTD, arg_left);
  else if (SW_ISSET(sw, SWITCH_LIST))
    do_motd(player, MOTD_LIST, "");
  else if (SW_ISSET(sw, SWITCH_WIZARD))
    do_motd(player, MOTD_WIZ, arg_left);
  else if (SW_ISSET(sw, SWITCH_DOWN))
    do_motd(player, MOTD_DOWN, arg_left);
  else if (SW_ISSET(sw, SWITCH_FULL))
    do_motd(player, MOTD_FULL, arg_left);
  else
    do_motd(player, MOTD_MOTD, arg_left);
}

COMMAND (cmd_mvattr) {
  do_cpattr(player, arg_left, args_right, 1, SW_ISSET(sw, SWITCH_NOFLAGCOPY));
}

COMMAND (cmd_name) {
  do_name(player, arg_left, arg_right);
}

COMMAND (cmd_newpassword) {
  do_newpassword(player, arg_left, arg_right);
}

COMMAND (cmd_nspemit) {
  int silent;

  if (SW_ISSET(sw, SWITCH_SILENT))
    silent = 1;
  else if (SW_ISSET(sw, SWITCH_NOISY))
    silent = 0;
  else
    silent = SILENT_PEMIT ? 1 : 0;

  if (SW_ISSET(sw, SWITCH_LIST))
    do_pemit_list(player, arg_left, arg_right, 0);
  else
    do_pemit(player, arg_left, arg_right, silent, 0);
}

COMMAND (cmd_nuke) {
  do_destroy(player, arg_left, 1);
}

COMMAND (cmd_oemit) {
  do_oemit_list(player, arg_left, arg_right);
}

COMMAND (cmd_open) {
  do_open(player, arg_left, args_right);
}

COMMAND (cmd_parent) {
  do_parent(player, arg_left, arg_right);
}

COMMAND (cmd_password) {
  do_password(player, arg_left, arg_right);
}

COMMAND (cmd_pcreate) {
  do_pcreate(player, arg_left, arg_right);
}

COMMAND (cmd_pemit) {
  int silent;

  if (SW_ISSET(sw, SWITCH_SILENT))
    silent = PEMIT_SILENT;
  else if (SW_ISSET(sw, SWITCH_NOISY))
    silent = 0;
  else
    silent = SILENT_PEMIT ? PEMIT_SILENT : 0;

  if (SW_ISSET(sw, SWITCH_LIST))
    do_pemit_list(player, arg_left, arg_right, 1);
  else if (SW_ISSET(sw, SWITCH_CONTENTS))
    do_remit(player, arg_left, arg_right, silent);
  else
    do_pemit(player, arg_left, arg_right, silent, 1);
}

COMMAND (cmd_poll) {
  do_poll(player, arg_left);
}

COMMAND (cmd_poor) {
  do_poor(player, arg_left);
}

COMMAND (cmd_power) {
  do_power(player, arg_left, arg_right);
}

COMMAND (cmd_ps) {
  if (SW_ISSET(sw, SWITCH_ALL))
    do_queue(player, arg_left, 1);
  else if (SW_ISSET(sw, SWITCH_SUMMARY) || SW_ISSET(sw, SWITCH_COUNT))
    do_queue(player, arg_left, 2);
  else if (SW_ISSET(sw, SWITCH_QUICK))
    do_queue(player, arg_left, 3);
  else
    do_queue(player, arg_left, 0);
}

COMMAND (cmd_purge) {
  do_purge(player);
}

#ifdef QUOTA
COMMAND (cmd_quota) {
  if (SW_ISSET(sw, SWITCH_ALL))
    do_allquota(player, arg_left, (SW_ISSET(sw, SWITCH_QUIET)));
  else if (SW_ISSET(sw, SWITCH_SET))
    do_quota(player, arg_left, arg_right, 1);
  else
    do_quota(player, arg_left, "", 0);
}
#endif				/* QUOTA */

COMMAND (cmd_readcache) {
  do_readcache(player);
}

COMMAND (cmd_remit) {
  int flags;
  if (SW_ISSET(sw, SWITCH_SILENT))
    flags = PEMIT_SILENT;
  else if (SW_ISSET(sw, SWITCH_NOISY))
    flags = 0;
  else
    flags = SILENT_PEMIT ? PEMIT_SILENT : 0;
  if (SW_ISSET(sw, SWITCH_LIST))
    flags |= PEMIT_LIST;
  do_remit(player, arg_left, arg_right, flags);
}

COMMAND (cmd_rejectmotd) {
  do_motd(player, 4, arg_left);
}

COMMAND (cmd_restart) {
  if (SW_ISSET(sw, SWITCH_ALL))
    do_allrestart(player);
  else
    do_restart_com(player, arg_left);
}

COMMAND (cmd_rwall) {
  do_wall(player, arg_left, WALL_RW, SW_ISSET(sw, SWITCH_EMIT));
}

COMMAND (cmd_rwallemit) {
  do_wall(player, arg_left, 1, 3);
}

COMMAND (cmd_scan) {
  if (SW_ISSET(sw, SWITCH_ROOM))
    do_scan(player, arg_left, CHECK_NEIGHBORS | CHECK_HERE);
  else if (SW_ISSET(sw, SWITCH_SELF))
    do_scan(player, arg_left, CHECK_INVENTORY | CHECK_SELF);
  else if (SW_ISSET(sw, SWITCH_ZONE))
    do_scan(player, arg_left, CHECK_ZONE);
  else if (SW_ISSET(sw, SWITCH_GLOBALS))
    do_scan(player, arg_left, CHECK_GLOBAL);
  else
    do_scan(player, arg_left, CHECK_INVENTORY | CHECK_NEIGHBORS |
	    CHECK_SELF | CHECK_HERE | CHECK_ZONE | CHECK_GLOBAL);
}

COMMAND (cmd_search) {
  do_search(player, arg_left, args_right);
}

COMMAND (cmd_select) {
  do_switch(player, arg_left, args_right, cause, 1,
	    (SW_ISSET(sw, SWITCH_NOTIFY)));
}

COMMAND (cmd_set) {
  do_set(player, arg_left, arg_right);
}

COMMAND (cmd_shutdown) {
  int paranoid;
  if (!Wizard(player)) {
    notify(player, T("You don't have the authority to do that!"));
    return;
  }
  paranoid = (SW_ISSET(sw, SWITCH_PARANOID)) ? 1 : 0;
  if (SW_ISSET(sw, SWITCH_REBOOT))
    do_reboot(player, paranoid);
  else if (SW_ISSET(sw, SWITCH_PANIC))
    do_shutdown(player, -1);
  else
    do_shutdown(player, paranoid);
}

COMMAND (cmd_sitelock) {
  if (SW_ISSET(sw, SWITCH_BAN))
    do_sitelock(player, arg_left, NULL, NULL, 1);
  else if (SW_ISSET(sw, SWITCH_REGISTER))
    do_sitelock(player, arg_left, NULL, NULL, 0);
  else if (SW_ISSET(sw, SWITCH_NAME))
    do_sitelock(player, arg_left, NULL, NULL, 2);
  else if (SW_ISSET(sw, SWITCH_REMOVE))
    do_sitelock(player, arg_left, NULL, NULL, 3);
  else if (SW_ISSET(sw, SWITCH_CHECK))
    do_sitelock(player, arg_left, NULL, NULL, 4);
  else
    do_sitelock(player, arg_left, args_right[1], args_right[2], 0);
}

COMMAND (cmd_stats) {
  if (SW_ISSET(sw, SWITCH_TABLES))
    do_list_memstats(player);
  else
    do_stats(player, arg_left);
}

COMMAND (cmd_sweep) {
  if (SW_ISSET(sw, SWITCH_CONNECTED))
    do_sweep(player, "connected");
  else if (SW_ISSET(sw, SWITCH_HERE))
    do_sweep(player, "here");
  else if (SW_ISSET(sw, SWITCH_INVENTORY))
    do_sweep(player, "inventory");
  else if (SW_ISSET(sw, SWITCH_EXITS))
    do_sweep(player, "exits");
  else
    do_sweep(player, arg_left);
}

COMMAND (cmd_switch) {
  do_switch(player, arg_left, args_right, cause, (SW_ISSET(sw, SWITCH_FIRST)),
	    (SW_ISSET(sw, SWITCH_NOTIFY)));
}

#ifdef QUOTA
COMMAND (cmd_squota) {
  do_quota(player, arg_left, arg_right, 1);
}

#endif

COMMAND (cmd_teleport) {
  if (rhs_present && !*arg_right)
    notify(player, T("You can't teleport to nothing!"));
  else
    do_teleport(player, arg_left, arg_right, (SW_ISSET(sw, SWITCH_SILENT)));
}

COMMAND (cmd_trigger) {
  do_trigger(player, arg_left, args_right);
}

COMMAND (cmd_ulock) {
  do_lock(player, arg_left, arg_right, Use_Lock);
}

COMMAND (cmd_undestroy) {
  do_undestroy(player, arg_left);
}

COMMAND (cmd_unlink) {
  do_unlink(player, arg_left);
}

COMMAND (cmd_unlock) {
  if ((switches) && (switches[0]))
    do_unlock(player, arg_left, switches);
  else
    do_unlock(player, arg_left, Basic_Lock);
}

COMMAND (cmd_uptime) {
  do_uptime(player, SW_ISSET(sw, SWITCH_MORTAL));
}

COMMAND (cmd_uunlock) {
  do_unlock(player, arg_left, Use_Lock);
}

COMMAND (cmd_verb) {
  do_verb(player, cause, arg_left, args_right);
}

COMMAND (cmd_version) {
  do_version(player);
}

COMMAND (cmd_wait) {
  do_wait(player, cause, arg_left, arg_right, SW_ISSET(sw, SWITCH_UNTIL));
}

COMMAND (cmd_wall) {
  do_wall(player, arg_left, WALL_ALL, SW_ISSET(sw, SWITCH_EMIT));
}

COMMAND (cmd_warnings) {
  do_warnings(player, arg_left, arg_right);
}

COMMAND (cmd_wcheck) {
  if (SW_ISSET(sw, SWITCH_ALL))
    do_wcheck_all(player);
  else if (SW_ISSET(sw, SWITCH_ME))
    do_wcheck_me(player);
  else
    do_wcheck(player, arg_left);
}

COMMAND (cmd_whereis) {
  do_whereis(player, arg_left);
}

COMMAND (cmd_wipe) {
  do_wipe(player, arg_left);
}

COMMAND (cmd_wizwall) {
  do_wall(player, arg_left, WALL_WIZ, SW_ISSET(sw, SWITCH_EMIT));
}

COMMAND (cmd_wizmotd) {
  do_motd(player, 2, arg_left);
}

COMMAND (cmd_zemit) {
  do_zemit(player, arg_left, arg_right);
}

COMMAND (cmd_brief) {
  do_examine(player, arg_left, 1, 0);
}

COMMAND (cmd_drop) {
  do_drop(player, arg_left);
}

COMMAND (cmd_examine) {
  int all = SW_ISSET(sw, SWITCH_ALL);
  if (SW_ISSET(sw, SWITCH_BRIEF))
    do_examine(player, arg_left, 1, all);
  else if (SW_ISSET(sw, SWITCH_DEBUG))
    do_debug_examine(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_MORTAL))
    do_examine(player, arg_left, 2, all);
  else
    do_examine(player, arg_left, 0, all);
}

COMMAND (cmd_enter) {
  do_enter(player, arg_left);
}

COMMAND (cmd_dismiss) {
  do_dismiss(player, arg_left);
}

COMMAND (cmd_desert) {
  do_desert(player, arg_left);
}

COMMAND (cmd_follow) {
  do_follow(player, arg_left);
}

COMMAND (cmd_unfollow) {
  do_unfollow(player, arg_left);
}

COMMAND (cmd_get) {
  do_get(player, arg_left);
}

COMMAND (cmd_give) {
  do_give(player, arg_left, arg_right, (SW_ISSET(sw, SWITCH_SILENT)));
}

COMMAND (cmd_goto) {
  move_wrapper(player, arg_left);
}

COMMAND (cmd_inventory) {
  do_inventory(player);
}

COMMAND (cmd_kill) {
  do_kill(player, arg_left, atol(arg_right), 0);
}

COMMAND (cmd_look) {
  do_look_at(player, arg_left, (SW_ISSET(sw, SWITCH_OUTSIDE)));
}

COMMAND (cmd_leave) {
  do_leave(player);
}

COMMAND (cmd_page) {
  char *t;
  if ((arg_right) && (*arg_right)) {
    t = arg_right;
  } else {
    t = NULL;
  }
  if (SW_ISSET(sw, SWITCH_PORT))
    do_page_port(player, arg_left, t);
  else
    do_page(player, arg_left, t, cause, SW_ISSET(sw, SWITCH_NOEVAL),
	    !(SW_ISSET(sw, SWITCH_BLIND) ||
	      (!(SW_ISSET(sw, SWITCH_LIST)) && (BLIND_PAGE))),
	    SW_ISSET(sw, SWITCH_OVERRIDE));
}

COMMAND (cmd_pose) {
  do_pose(player, arg_left, (SW_ISSET(sw, SWITCH_NOSPACE)));
}

COMMAND (cmd_say) {
  do_say(player, arg_left);
}

COMMAND (cmd_score) {
  do_score(player);
}

COMMAND (cmd_semipose) {
  do_pose(player, arg_left, 1);
}

COMMAND (cmd_slay) {
  do_kill(player, arg_left, 0, 1);
}

COMMAND (cmd_take) {
  do_get(player, arg_left);
}

COMMAND (cmd_think) {
  do_think(player, arg_left);
}

COMMAND (cmd_whisper) {
  do_whisper(player, arg_left, arg_right,
	     (SW_ISSET(sw, SWITCH_NOISY) ||
	      (!SW_ISSET(sw, SWITCH_SILENT) && NOISY_WHISPER)));
}

COMMAND (cmd_use) {
  do_use(player, arg_left);
}

COMMAND (command_atrset) {
  dbref thing;

  if ((thing = match_controlled(player, arg_left)) == NOTHING)
    return;

  /* If it's &attr obj, we must pass a NULL. If &attr obj=, pass "" */
  if (rhs_present) {
    do_set_atr(thing, switches, arg_right, player,
	       0x1 | (SW_ISSET(sw, SWITCH_NOEVAL) ? 0 : 0x02));
  } else {
    do_set_atr(thing, switches, NULL, player, 1);
  }
}

COMMAND (cmd_null) {
  return;
}
