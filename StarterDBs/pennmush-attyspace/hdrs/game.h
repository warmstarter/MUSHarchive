/* game.h */
/* Command handlers */

#ifndef __GAME_H
#define __GAME_H

#include "conf.h"
#include "dbdefs.h"
#include "extmail.h"
#ifdef CHAT_SYSTEM
#include "extchat.h"
#endif

/* Miscellaneous flags */
#define CHECK_INVENTORY            0x10
#define CHECK_NEIGHBORS            0x20
#define CHECK_SELF                 0x40
#define CHECK_HERE                 0x80
#define CHECK_ZONE                 0x100
#define CHECK_GLOBAL               0x200

/* hash table stuff */
extern void init_func_hashtab _((void));	/* eval.c */
extern void init_aname_hashtab _((void));	/* attrib.c */
extern void init_flag_hashtab _((void));	/* flags.c */

/* From atr_tab.c */
extern void do_attribute_access _((dbref player, char *name, char *perms, int retroactive));
extern void do_attribute_delete _((dbref player, char *name));
extern void do_attribute_rename _((dbref player, char *old, char *newname));
extern void do_attribute_info _((dbref player, char *name));
extern void do_list_attribs _((dbref player));

/* From bsd.c */
extern void do_new_spitfile _((dbref player, char *arg1, const char *index_file, const char *text_file, int restricted));
extern void fcache_init _((void));
extern void fcache_load _((dbref player));
extern void hide_player _((dbref player, int hide));
extern void do_motd _((dbref player, int key, const char *message));
extern void do_poll _((dbref player, const char *message));
/* From cque.c */
extern void do_notify _((dbref player, dbref cause, int key, char *what, char *count));
extern void do_wait _((dbref player, dbref cause, char *arg1, char *cmd));
extern void do_queue _((dbref player, const char *what, int flag));
extern void do_halt1 _((dbref player, const char *arg1, const char *arg2));
extern void do_allhalt _((dbref player));
extern void do_allrestart _((dbref player));
extern void do_restart _((void));
extern void do_restart_com _((dbref player, const char *arg1));

/* From compress.c */
#if (COMPRESSION_TYPE > 0)
extern int init_compress _((FILE * f));
#endif

/* From conf.c */
extern int config_file_startup _((const char *conf));

/* From db.c */
extern int db_paranoid_write _((FILE * f, int flag));

/* From eval.c */
extern void do_list_functions _((dbref player));
extern void do_function _((dbref player, char *name, char **argv));
extern void do_function_delete _((dbref player, char *name));

/* From game.c */
extern void do_dump _((dbref player, char *num, int flag));
extern void do_shutdown _((dbref player, int panic_flag));

/* From log.c */
extern void do_logwipe _((dbref player, int logtype, char *str));

/* From look.c */
extern void do_examine _((dbref player, const char *name, int brief));
extern void do_inventory _((dbref player));
extern void do_find _((dbref player, const char *name, char **argv));
extern void do_whereis _((dbref player, const char *name));
extern void do_score _((dbref player));
extern void do_sweep _((dbref player, const char *arg1));
extern void do_entrances _((dbref player, const char *where, char **argv, int val));
extern void do_decompile _((dbref player, const char *name, int dbflag, int skipdef));

#ifdef USE_MAILER
/* From mail.c */
extern void do_mail _((dbref player, char *arg1, char *arg2));
extern void do_mail_stats _((dbref player, char *name, int full));
extern void do_mail_debug _((dbref player, char *action, char *victim));
extern void do_mail_nuke _((dbref player));
extern void do_mail_change_folder _((dbref player, char *fld, char *newname));
extern void do_mail_unfolder _((dbref player, char *fld));
extern void do_mail_list _((dbref player, const char *msglist));
extern void do_mail_read _((dbref player, char *msglist));
extern void do_mail_clear _((dbref player, char *msglist));
extern void do_mail_unclear _((dbref player, char *msglist));
extern void do_mail_purge _((dbref player));
extern void do_mail_file _((dbref player, char *msglist, char *folder));
extern void do_mail_tag _((dbref player, char *msglist));
extern void do_mail_untag _((dbref player, char *msglist));
extern void do_mail_fwd _((dbref player, char *msglist, char *tolist));
extern void do_mail_send _((dbref player, char *tolist, char *message, mail_flag flags, int silent));
#endif				/* USE_MAILER */

/* From move.c */
extern void do_get _((dbref player, const char *what));
extern void do_drop _((dbref player, const char *name));
extern void do_enter _((dbref player, const char *what, int is_alias));
extern void do_leave _((dbref player));
extern void do_firstexit _((dbref player, const char *what));

/* From player.c */
extern void do_password _((dbref player, const char *old, const char *newobj));

/* From predicat.c */
extern void do_switch _((dbref player, char *expression, char **argv, dbref cause, int first));
extern void do_verb _((dbref player, dbref cause, char *arg1, char **argv));
extern void do_grep _((dbref player, char *obj, char *lookfor, int flag, int insensitive));

/* From rob.c */
extern void do_kill _((dbref player, const char *what, int cost, int slay));
extern void do_give _((dbref player, char *recipient, char *amnt));
extern void do_rob _((dbref player, const char *what));

#ifdef ALLOW_RPAGE
/* From rpage.c */
extern void do_rpage();
extern void do_rpage_add();
extern void do_rpage_delete();
extern void do_rpage_list();
#endif				/* ALLOW_RPAGE */

/* From set.c */
extern void do_name _((dbref player, const char *name, char *newname));
extern void do_chown _((dbref player, const char *name, const char *newobj));
extern void do_chzone _((dbref player, const char *name, const char *newobj));
extern int do_set _((dbref player, const char *name, char *flag));
extern void do_cpattr _((dbref player, char *oldpair, char **newpair, int move));
extern void do_gedit _((dbref player, char *it, char **argv));
extern void do_trigger _((dbref player, char *object, char **argv));
extern void do_use _((dbref player, const char *what));
extern void do_parent _((dbref player, char *name, char *parent_name));
extern void do_wipe _((dbref player, char *name));

/* From speech.c */
extern void do_say _((dbref player, const char *tbuf1));
extern void do_oemit _((dbref player, const char *arg1, const char *arg2));
extern void do_whisper _((dbref player, const char *arg1, const char *arg2, int noisy));
extern void do_whisper_list _((dbref player, const char *arg1, const char *arg2, int noisy));
extern void do_pemit _((dbref player, const char *arg1, const char *arg2, int silent));
extern void do_pemit_list _((dbref player, char *list, const char *message));
extern void do_pose _((dbref player, const char *tbuf1, int space));
extern void do_wall _((dbref player, const char *message, int privs, int key));
extern void do_page _((dbref player, const char *arg1, const char *arg2));
extern void do_multipage _((dbref player, const char *arg1, const char *arg2));
extern void do_think _((dbref player, const char *message));
extern void do_emit _((dbref player, const char *tbuf1));
extern void do_remit _((dbref player, const char *arg1, const char *arg2));
extern void do_lemit _((dbref player, const char *tbuf1));
extern void do_zemit _((dbref player, const char *arg1, const char *arg2));

/* From wiz.c */
extern void do_debug_examine _((dbref player, const char *name));
extern void do_enable _((dbref player, const char *param, int state));
extern void do_fixdb _((dbref player, const char *name, const char *val, int sw_opt));
extern void do_kick _((dbref player, const char *num));
extern void do_search _((dbref player, const char *arg1, char **arg3));
extern void do_pcreate _((dbref creator, const char *player_name, const char *player_password));
#ifdef QUOTA
extern void do_quota _((dbref player, const char *arg1, const char *arg2, int set_q));
extern void do_allquota _((dbref player, const char *arg1, int quiet));
#endif
extern void do_teleport _((dbref player, const char *arg1, const char *arg2));
extern void do_force _((dbref player, const char *what, char *command));
extern void do_stats _((dbref player, const char *name));
extern void do_newpassword _((dbref player, const char *name, const char *password));
extern void do_boot _((dbref player, const char *name, int flag));
extern void do_chzoneall _((dbref player, const char *name, const char *target));
extern int force_by_number _((dbref player, char *command));
extern void do_power _((dbref player, const char *name, const char *power));
extern void do_sitelock _((dbref player, const char *site, const char *opts, int type));

/* From destroy.c */
extern void do_dbck _((dbref player));
extern void do_destroy _((dbref player, char *name, int confirm));

/* From lock.c */
extern void do_lock _((dbref player, const char *name, const char *keyname, lock_type type));
extern void do_unlock _((dbref player, const char *name, lock_type type));

/* From timer.c */
extern void init_timer _((void));

/* From version.c */
extern void do_version _((dbref player));

#endif				/* __GAME_H */
