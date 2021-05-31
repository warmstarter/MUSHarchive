
/* cmds.c
 * Actual definitions of the commands
 * Parsing is handled in command.c
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
#include "externs.h"
#include "intrface.h"
#include "match.h"
#include "game.h"
#include "globals.h"
#include "extmail.h"
#ifdef CHAT_SYSTEM
#include "extchat.h"
#endif
#include "getpgsiz.h"
#include "parse.h"
#include "access.h"
#include "version.h"

#include "function.h"
#include "command.h"

#include "confmagic.h"

/* External Stuff */
void do_config_list _((dbref player, const char *type));
void do_poor _((dbref player, char *arg1));
void do_dolist _((dbref player, char *list, char *command,
		  dbref cause, int flag, int notify_flag));
void do_list _((dbref player, char *arg));
void do_writelog _((dbref player, char *str, int ltype));
void do_reboot _((dbref player, int flag));
void do_readcache _((dbref player));
void do_scan _((dbref player, char *command, int flag));
void do_uptime _((dbref player));


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
    do_attribute_access(player, arg_left, arg_right, SW_ISSET(sw, SWITCH_RETROACTIVE));
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

#ifdef CHAT_SYSTEM
COMMAND (cmd_cemit) {
  do_cemit(player, arg_left, arg_right, SW_ISSET(sw, SWITCH_NOISY));
}

COMMAND (cmd_channel) {
  if (switches)
    do_channel(player, arg_left, arg_right, switches);
  else if (SW_ISSET(sw, SWITCH_LIST))
    do_channel_list(player, arg_left);

  else if (SW_ISSET(sw, SWITCH_ADD))
    do_chan_admin(player, arg_left, arg_right, 0);
  else if (SW_ISSET(sw, SWITCH_DELETE))
    do_chan_admin(player, arg_left, arg_right, 1);
  else if (SW_ISSET(sw, SWITCH_NAME))
    do_chan_admin(player, arg_left, arg_right, 2);
  else if (SW_ISSET(sw, SWITCH_RENAME))
    do_chan_admin(player, arg_left, arg_right, 2);
  else if (SW_ISSET(sw, SWITCH_PRIVS))
    do_chan_admin(player, arg_left, arg_right, 3);
  else if (SW_ISSET(sw, SWITCH_QUIET))
    do_chan_admin(player, arg_left, arg_right, 4);
  else if (SW_ISSET(sw, SWITCH_DECOMPILE))
    do_chan_decompile(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_DESC))
    do_chan_desc(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_TITLE))
    do_chan_title(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_CHOWN))
    do_chan_chown(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_WIPE))
    do_chan_wipe(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_MUTE))
    do_chan_user_flags(player, arg_left, arg_right, 0, 0);
  else if (SW_ISSET(sw, SWITCH_HIDE))
    do_chan_user_flags(player, arg_left, arg_right, 1, 0);
  else if (SW_ISSET(sw, SWITCH_GAG))
    do_chan_user_flags(player, arg_left, arg_right, 2, 0);
  else if (SW_ISSET(sw, SWITCH_WHAT))
    do_chan_what(player, arg_left);
  else
    do_channel(player, arg_left, NULL, arg_right);
}

COMMAND (cmd_chat) {
  do_chat_by_name(player, arg_left, arg_right);
}

#endif

COMMAND (cmd_chownall) {
  do_chownall(player, arg_left, arg_right);
}

COMMAND (cmd_chown) {
  do_chown(player, arg_left, arg_right);
}

COMMAND (cmd_chzoneall) {
  do_chzoneall(player, arg_left, arg_right);
}

COMMAND (cmd_chzone) {
  do_chzone(player, arg_left, arg_right);
}

COMMAND (cmd_config) {
  if (SW_ISSET(sw, SWITCH_GLOBALS))
    do_config_list(player, NULL);
  else if (SW_ISSET(sw, SWITCH_DEFAULTS))
    do_config_list(player, NULL);
  else if (SW_ISSET(sw, SWITCH_COSTS))
    do_config_list(player, NULL);
  else if (SW_ISSET(sw, SWITCH_FUNCTIONS))
    do_list_functions(player);
  else if (SW_ISSET(sw, SWITCH_COMMANDS))
    do_list_commands(player);
  else if (SW_ISSET(sw, SWITCH_ATTRIBS))
    do_list_attribs(player);
  else if (SW_ISSET(sw, SWITCH_LIST))
    do_config_list(player, arg_left);
  else
    do_config_list(player, arg_left);
}

COMMAND (cmd_cpattr) {
  do_cpattr(player, arg_left, args_right, 0);
}

COMMAND (cmd_create) {
  do_create(player, arg_left, atol(arg_right));
}

COMMAND (cmd_clone) {
  do_clone(player, arg_left);
}

#ifdef CHAT_SYSTEM
COMMAND (cmd_clock) {
  if (SW_ISSET(sw, SWITCH_JOIN))
    do_chan_lock(player, arg_left, arg_right, CL_JOIN);
  else if (SW_ISSET(sw, SWITCH_SPEAK))
    do_chan_lock(player, arg_left, arg_right, CL_SPEAK);
  else if (SW_ISSET(sw, SWITCH_MOD))
    do_chan_lock(player, arg_left, arg_right, CL_MOD);
  else if (SW_ISSET(sw, SWITCH_SEE))
    do_chan_lock(player, arg_left, arg_right, CL_SEE);
  else if (SW_ISSET(sw, SWITCH_HIDE))
    do_chan_lock(player, arg_left, arg_right, CL_HIDE);
  else
    notify(player, "You must specify a type of lock");
}
#endif

COMMAND (cmd_dbck) {
  do_dbck(player);
}

COMMAND (cmd_decompile) {
  if (SW_ISSET(sw, SWITCH_DB))
    do_decompile(player, arg_left, 1, SW_ISSET(sw, SWITCH_SKIPDEFAULTS));
  else if (SW_ISSET(sw, SWITCH_TF))
    do_decompile(player, arg_left, 2, SW_ISSET(sw, SWITCH_SKIPDEFAULTS));
  else if (SW_ISSET(sw, SWITCH_FLAGS))
    do_decompile(player, arg_left, 3, SW_ISSET(sw, SWITCH_SKIPDEFAULTS));
  else if (SW_ISSET(sw, SWITCH_ATTRIBS))
    do_decompile(player, arg_left, 4, SW_ISSET(sw, SWITCH_SKIPDEFAULTS));
  else
    do_decompile(player, arg_left, 0, SW_ISSET(sw, SWITCH_SKIPDEFAULTS));
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
  do_dolist(player, arg_left, arg_right, cause, 0, (SW_ISSET(sw, SWITCH_NOTIFY)));
}

COMMAND (cmd_drain) {
  do_notify(player, cause, 2, arg_left, arg_right);
}

COMMAND (cmd_dump) {
  if (SW_ISSET(sw, SWITCH_PARANOID))
    do_dump(player, arg_left, 1);
  else if (SW_ISSET(sw, SWITCH_DEBUG))
    do_dump(player, arg_left, 2);
  else
    do_dump(player, arg_left, 0);
}

COMMAND (cmd_edit) {
  do_gedit(player, arg_left, args_right);
}

COMMAND (cmd_elock) {
  do_lock(player, arg_left, arg_right, (char *) Enter_Lock);
}

COMMAND (cmd_emit) {
  if (SW_ISSET(sw, SWITCH_ROOM))
    do_lemit(player, arg_left);
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
  do_unlock(player, arg_left, (char *) Enter_Lock);
}

COMMAND (cmd_find) {
  do_find(player, arg_left, args_right);
}

COMMAND (cmd_firstexit) {
  do_firstexit(player, arg_left);
}

COMMAND (cmd_fixdb) {
  if (SW_ISSET(sw, SWITCH_LOCATION))
    do_fixdb(player, arg_left, arg_right, 0);
  else if (SW_ISSET(sw, SWITCH_CONTENTS))
    do_fixdb(player, arg_left, arg_right, 1);
  else if (SW_ISSET(sw, SWITCH_EXITS))
    do_fixdb(player, arg_left, arg_right, 2);
  else if (SW_ISSET(sw, SWITCH_NEXT))
    do_fixdb(player, arg_left, arg_right, 3);
  else
    notify(player, "Command MUST have a switch.");
}

COMMAND (cmd_force) {
  do_force(player, arg_left, arg_right);
}

COMMAND (cmd_function) {
  if (SW_ISSET(sw, SWITCH_DELETE))
    do_function_delete(player, arg_left);
  else
    do_function(player, arg_left, args_right);
}

COMMAND (cmd_gedit) {
  do_gedit(player, arg_left, args_right);
}

COMMAND (cmd_grep) {
  do_grep(player, arg_left, arg_right, ((SW_ISSET(sw, SWITCH_IPRINT)) || (SW_ISSET(sw, SWITCH_PRINT))), ((SW_ISSET(sw, SWITCH_IPRINT)) || (SW_ISSET(sw, SWITCH_ILIST))));
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

COMMAND (cmd_kick) {
  do_kick(player, arg_left);
}

COMMAND (cmd_lemit) {
  do_lemit(player, arg_left);
}

COMMAND (cmd_link) {
  do_link(player, arg_left, arg_right);
}

COMMAND (cmd_listmotd) {
  do_motd(player, 3, "");
}

COMMAND (cmd_list) {
  if (SW_ISSET(sw, SWITCH_MOTD))
    do_motd(player, 3, "");
  else if (SW_ISSET(sw, SWITCH_FUNCTIONS))
    do_list_functions(player);
  else if (SW_ISSET(sw, SWITCH_COMMANDS))
    do_list_commands(player);
  else if (SW_ISSET(sw, SWITCH_ATTRIBS))
    do_list_attribs(player);
  else
    do_list(player, arg_left);
}

COMMAND (cmd_lock) {
  if ((switches) && (switches[0]))
    do_lock(player, arg_left, arg_right, switches);
  else
    do_lock(player, arg_left, arg_right, (char *) Basic_Lock);
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

#ifdef USE_MAILER
COMMAND (cmd_mail) {
  int urgent = SW_ISSET(sw, SWITCH_URGENT);
  int silent = SW_ISSET(sw, SWITCH_SILENT);
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
  else if (SW_ISSET(sw, SWITCH_FOLDER))
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
  else if (SW_ISSET(sw, SWITCH_FWD))
    do_mail_fwd(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_FORWARD))
    do_mail_fwd(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_SEND) || silent || urgent)
    do_mail_send(player, arg_left, arg_right,
		 urgent ? M_URGENT : 0,
		 silent ? 1 : 0);
  else
    do_mail(player, arg_left, arg_right);
}
#endif

COMMAND (cmd_map) {
  do_dolist(player, arg_left, arg_right, cause, 1, 0);
}

COMMAND (cmd_motd) {
  if (SW_ISSET(sw, SWITCH_CONNECT))
    do_motd(player, 1, arg_left);
  else if (SW_ISSET(sw, SWITCH_LIST))
    do_motd(player, 3, "");
  else if (SW_ISSET(sw, SWITCH_WIZARD))
    do_motd(player, 2, arg_left);
  else if (SW_ISSET(sw, SWITCH_DOWN))
    do_motd(player, 4, arg_left);
  else if (SW_ISSET(sw, SWITCH_FULL))
    do_motd(player, 5, arg_left);
  else
    do_motd(player, 1, arg_left);
}

COMMAND (cmd_mvattr) {
  do_cpattr(player, arg_left, args_right, 1);
}

COMMAND (cmd_name) {
  do_name(player, arg_left, arg_right);
}

COMMAND (cmd_newpassword) {
  do_newpassword(player, arg_left, arg_right);
}

COMMAND (cmd_notify) {
  do_notify(player, cause, (SW_ISSET(sw, SWITCH_ALL)), arg_left, arg_right);
}

COMMAND (cmd_nuke) {
  do_destroy(player, arg_left, 1);
}

COMMAND (cmd_oemit) {
  do_oemit(player, arg_left, arg_right);
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
    silent = 1;
  else if (SW_ISSET(sw, SWITCH_NOISY))
    silent = 0;
  else
    silent = SILENT_PEMIT ? 1 : 0;

  if (SW_ISSET(sw, SWITCH_LIST))
    do_pemit_list(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_CONTENTS))
    do_remit(player, arg_left, arg_right);
  else
    do_pemit(player, arg_left, arg_right, silent);
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
  else if (SW_ISSET(sw, SWITCH_SUMMARY))
    do_queue(player, arg_left, 2);
  else if (SW_ISSET(sw, SWITCH_COUNT))
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
  do_remit(player, arg_left, arg_right);
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
  do_wall(player, arg_left, 1, 1);
}

COMMAND (cmd_rwallpose) {
  do_wall(player, arg_left, 1, 2);
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
  do_switch(player, arg_left, args_right, cause, 1);
}

COMMAND (cmd_set) {
  do_set(player, arg_left, arg_right);
}

COMMAND (cmd_shutdown) {
  int paranoid;
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
    do_sitelock(player, arg_left, NULL, 1);
  else if (SW_ISSET(sw, SWITCH_REGISTER))
    do_sitelock(player, arg_left, NULL, 0);
  else if (SW_ISSET(sw, SWITCH_NAME))
    do_sitelock(player, arg_left, NULL, 2);
  else
    do_sitelock(player, arg_left, arg_right, 0);
}

COMMAND (cmd_stats) {
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
  do_switch(player, arg_left, args_right, cause, (SW_ISSET(sw, SWITCH_FIRST)));
}

#ifdef QUOTA
COMMAND (cmd_squota) {
  do_quota(player, arg_left, arg_right, 1);
}

#endif

COMMAND (cmd_teleport) {
  do_teleport(player, arg_left, arg_right);
}

COMMAND (cmd_trigger) {
  do_trigger(player, arg_left, args_right);
}

COMMAND (cmd_ulock) {
  do_lock(player, arg_left, arg_right, (char *) Use_Lock);
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
    do_unlock(player, arg_left, (char *) Basic_Lock);
}

COMMAND (cmd_uptime) {
  do_uptime(player);
}

COMMAND (cmd_uunlock) {
  do_unlock(player, arg_left, (char *) Use_Lock);
}

COMMAND (cmd_verb) {
  do_verb(player, cause, arg_left, args_right);
}

COMMAND (cmd_version) {
  do_version(player);
}

COMMAND (cmd_wait) {
  do_wait(player, cause, arg_left, arg_right);
}

COMMAND (cmd_wall) {
  int target = 2;
  int type = 1;
  if (SW_ISSET(sw, SWITCH_WIZARD))
    target = 0;
  if (SW_ISSET(sw, SWITCH_ROYALTY))
    target = 1;
  if (SW_ISSET(sw, SWITCH_EMIT))
    type = 3;
  if (SW_ISSET(sw, SWITCH_POSE))
    type = 2;
  do_wall(player, arg_left, target, type);
}

COMMAND (cmd_wallpose) {
  do_wall(player, arg_left, 2, 2);
}

COMMAND (cmd_wallemit) {
  do_wall(player, arg_left, 2, 3);
}

#ifdef USE_WARNINGS
COMMAND (cmd_warnings) {
  do_warnings(player, arg_left, arg_right);
}

COMMAND (cmd_wcheck) {
  if (SW_ISSET(sw, SWITCH_ALL))
    do_wcheck_all(player);
  else
    do_wcheck(player, arg_left);
}
#endif

COMMAND (cmd_whereis) {
  do_whereis(player, arg_left);
}

COMMAND (cmd_wipe) {
  do_wipe(player, arg_left);
}

COMMAND (cmd_wizwall) {
  do_wall(player, arg_left, 0, 1);
}

COMMAND (cmd_wizmotd) {
  do_motd(player, 2, arg_left);
}

COMMAND (cmd_wizpose) {
  do_wall(player, arg_left, 0, 2);
}

COMMAND (cmd_wizemit) {
  do_wall(player, arg_left, 0, 3);
}

COMMAND (cmd_zemit) {
  do_zemit(player, arg_left, arg_right);
}

COMMAND (cmd_ahelp) {
  do_new_spitfile(player, arg_left, HELPINDX, HELPTEXT, 1);
}

COMMAND (cmd_anews) {
  do_new_spitfile(player, arg_left, NEWSINDX, NEWS_FILE, 1);
}

COMMAND (cmd_brief) {
  do_examine(player, arg_left, 1);
}

COMMAND (cmd_drop) {
  do_drop(player, arg_left);
}

COMMAND (cmd_examine) {
  if (SW_ISSET(sw, SWITCH_BRIEF))
    do_examine(player, arg_left, 1);
  else if (SW_ISSET(sw, SWITCH_DEBUG))
    do_debug_examine(player, arg_left);
  else if (SW_ISSET(sw, SWITCH_MORTAL))
    do_examine(player, arg_left, 2);
  else
    do_examine(player, arg_left, 0);
}

COMMAND (cmd_enter) {
  do_enter(player, arg_left, 0);
}

COMMAND (cmd_events) {
  do_new_spitfile(player, arg_left, EVENTINDX, EVENT_FILE, 0);
}

COMMAND (cmd_follow) {
  notify(player, "COMMAND NOT IMPLEMENTED YET.");
}

COMMAND (cmd_get) {
  do_get(player, arg_left);
}

COMMAND (cmd_give) {
  do_give(player, arg_left, arg_right);
}

COMMAND (cmd_goto) {
  move_wrapper(player, arg_left);
}

COMMAND (cmd_help) {
  do_new_spitfile(player, arg_left, HELPINDX, HELPTEXT, 0);
}

COMMAND (cmd_inventory) {
  do_inventory(player);
}

COMMAND (cmd_index) {
  do_new_spitfile(player, arg_left, INDEXINDX, INDEX_FILE, 0);
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

COMMAND (cmd_news) {
  do_new_spitfile(player, arg_left, NEWSINDX, NEWS_FILE, 0);
}

COMMAND (cmd_page) {
  char *t;
  if ((arg_right) && (*arg_right)) {
    t = arg_right;
  } else {
    t = NULL;
  }
  if (SW_ISSET(sw, SWITCH_LIST))
    do_multipage(player, arg_left, t);
  else if ((BLIND_PAGE) || (SW_ISSET(sw, SWITCH_BLIND)))
    do_page(player, arg_left, t);
  else
    do_multipage(player, arg_left, t);
}

COMMAND (cmd_pose) {
  do_pose(player, arg_left, (SW_ISSET(sw, SWITCH_NOSPACE)));
}

COMMAND (cmd_rob) {
  do_rob(player, arg_left);
}

#ifdef ALLOW_RPAGE
COMMAND (cmd_rpage) {
  if (SW_ISSET(sw, SWITCH_LIST))
    do_rpage_list(player);
  else if (SW_ISSET(sw, SWITCH_ADD))
    do_rpage_add(player, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_DELETE))
    do_rpage_delete(player, arg_left, arg_right);
  else
    do_rpage(player, arg_left, arg_right);
}
#endif

COMMAND (cmd_rules) {
  do_new_spitfile(player, arg_left, (char *) RULESINDX, RULES_FILE, 0);
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
  int noisy;
  if (SW_ISSET(sw, SWITCH_NOISY))
    noisy = 1;
  else if (SW_ISSET(sw, SWITCH_SILENT))
    noisy = 0;
  else
    noisy = (NOISY_WHISPER ? 1 : 0);
  if (SW_ISSET(sw, SWITCH_LIST))
    do_whisper_list(player, arg_left, arg_right, noisy);
  else
    do_whisper(player, arg_left, arg_right, noisy);
}

COMMAND (cmd_use) {
  do_use(player, arg_left);
}

COMMAND (command_atrset) {
  dbref thing;

  if ((thing = match_controlled(player, arg_left)) == NOTHING)
    return;

  /* If it's &attr obj, we must pass a NULL. If &attr obj=, pass "" */
  if (strchr(raw, '=')) {
    do_set_atr(thing, switches, arg_right, player, 0x1 | (SW_ISSET(sw, SWITCH_NOEVAL) ? 0 : 0x02));
  } else {
    do_set_atr(thing, switches, NULL, player, 1);
  }
}

COMMAND (cmd_null) {
  return;
}
