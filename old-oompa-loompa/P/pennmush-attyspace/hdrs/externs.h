/* externs.h */

/* Patched 12/1/90 by Michael Stanley (jstanley@uafhp.uark.edu) to 
 * add @search command.  details in file game.c                    
 * Patched 1/3/91 by Stan Lim aka Jin (stanl@zimmer.csufresno.edu) to add
 * do_hide and do_unhide.
 * Patched numerous times by Moonchilde (jt1o@andrew.cmu.edu) between
 * 1/91 and 11/91.
 * Patched numerous times by Amberyl (lwl@eniac.seas.upenn.edu) after
 * 1/92.
 * Patched numerous times by Javelin (dunemush@pennmush.org)
 * after 1/95.
 */

#ifndef __EXTERNS_H
#define __EXTERNS_H

/* Get the time_t definition that we use in prototypes here */
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif

#include "config.h"
#include "copyrite.h"
#include "conf.h"
#include "intrface.h"
#include "myregexp.h"

/* Prototypes for externs not defined elsewhere */
#include "mushdb.h"
#include "attrib.h"
#ifdef CHAT_SYSTEM
#include "extchat.h"
#endif
#include "confmagic.h"

#ifndef HAS_STRCASECMP
extern int strcasecmp _((const char *s1, const char *s2));
extern int strncasecmp _((const char *s1, const char *s2, size_t n));
#endif

#ifdef WIN32
/* From timer.c */
extern void init_timer _((void));
extern void kill_timer _((void));
#endif				/* WIN32 */

/* From attrib.c */
extern struct boolatr *alloc_atr _((const char *name, char *s));
extern ATTR *atr_match _((char const *string));
extern void atr_new_add _((dbref thing, char const *atr, char const *s,
			   dbref player, int flags));
extern int atr_add _((dbref thing, char const *atr, char const *s,
		      dbref player, int flags));
extern int atr_clr _((dbref thing, char const *atr, dbref player));
extern ATTR *atr_get _((dbref thing, char const *atr));
extern ATTR *atr_get_noparent _((dbref thing, char const *atr));
extern int atr_iter_get _((dbref player, dbref thing, char const *name,
			   int (*func) (), void *args));
extern ATTR *atr_complete_match _((dbref player, char const *atr,
				   dbref privs));
extern void atr_free _((dbref thing));
extern void atr_cpy _((dbref dest, dbref source));
extern char const *const convert_atr _((int oldatr));
extern int atr_comm_match _((dbref thing, dbref player, int type, int end,
			     char const *str, int just_match));
extern int do_set_atr _((dbref thing, char const *atr, char const *s,
			 dbref player, int flags));
extern void do_atrlock _((dbref player, char const *arg1, char const *arg2));
extern void do_atrchown _((dbref player, char const *arg1, char const *arg2));

extern int string_to_atrflag _((dbref player, const char *p));

/* From bsd.c */
extern FILE *connlog_fp;
extern FILE *checklog_fp;
extern FILE *wizlog_fp;
extern FILE *tracelog_fp;
extern FILE *cmdlog_fp;
#ifdef TREKMUSH
extern FILE *spacelog_fp;
#endif /* TREKMUSH */
extern int restarting;
extern struct mail *desc_mail _((dbref player));
#ifdef SUN_OS
extern int f_close _((FILE * file));
#define fclose(f) f_close(f);
#endif
extern int hidden _((dbref player));
extern void kill_info_slave _((void));
void dump_reboot_db _((void));
void rwho_update _((void));

/* The #defs for our notify_anything hacks.. Errr. Functions */
#define NA_NORELAY	0x0001
#define NA_NOENTER	0x0002
#define NA_NOLISTEN	0x0004	/* Implies NORELAY. Sorta. */
#define NA_NOPENTER	0x0010
#define NA_PONLY        0x0020
#define NA_PUPPET	0x0040

extern void notify_anything _((dbref speaker, dbref (*func) (), void *fdata, void (*nsfunc) (), int flags, const char *message));
extern dbref na_one _((dbref current, void *data));
extern dbref na_next _((dbref current, void *data));
extern dbref na_loc _((dbref current, void *data));
extern dbref na_nextbut _((dbref current, void *data));
extern dbref na_except _((dbref current, void *data));
extern dbref na_except2 _((dbref current, void *data));
#ifdef CHAT_SYSTEM
extern dbref na_channel _((dbref current, void *data));
#endif

#define notify(p,m)           notify_anything(orator, na_one, (void *)p, NULL, 0, m)
#define notify_by(t,p,m)           notify_anything(t, na_one, (void *)p, NULL, 0, m)
#define notify_noecho(p,m)    notify_anything(orator, na_one, (void *)p, NULL, NA_NORELAY | NA_PUPPET, m)
#define quiet_notify(p,m)     if (!IsQuiet(p)) notify(p,m)

/* From compress.c */
#if (COMPRESSION_TYPE != 0)
extern unsigned char *compress _((char const *s));
extern char *uncompress _((unsigned char const *s));
extern char *safe_uncompress _((unsigned char const *s));
#else
extern char ucbuff[];
extern char cbuff[];
#define init_compress(f) 0
#define compress(s) ((unsigned char *)strcpy(cbuff,s))
#define uncompress(s) (strcpy(ucbuff, (char *) s))
#define safe_uncompress(s) (strdup((char *) s))
#endif

/* From conf.c */
extern void do_config_list _((dbref player, const char *type));

/* From cque.c */
extern char *wenv[10], renv[10][BUFFER_LEN];
extern char *wnxt[10], *rnxt[10];
extern void do_second _((void));
extern int do_top _((int ncom));
extern void do_halt _((dbref owner, const char *ncom, dbref victim));
extern void parse_que _((dbref player, const char *command, dbref cause));
extern int nfy_que _((dbref sem, int key, int count));


/* From create.c */
extern dbref do_dig _((dbref player, const char *name, char **argv, int tport));
extern dbref do_create _((dbref player, char *name, int cost));
extern dbref do_real_open _((dbref player, const char *direction, const char *linkto, dbref pseudo));
extern void do_open _((dbref player, const char *direction, char **links));
extern void do_link _((dbref player, const char *name, const char *room_name));
extern void do_unlink _((dbref player, const char *name));
extern dbref do_clone _((dbref player, char *name));

/* From flags.c */
extern const char *unparse_flags _((dbref thing, dbref player));
extern const char *flag_description _((dbref player, dbref thing));
extern object_flag_type find_flag _((char *name, int type, int *toggle, int is_conf));
extern int sees_flag _((dbref privs, dbref thing, char *name));
extern int handle_flaglists _((dbref player, char *name, char *fstr, int type));
extern void set_flag _((dbref player, dbref thing, char *flag, int negate, int hear, int listener));
extern const char *power_description _((dbref thing));
extern object_flag_type find_power _((const char *name));

/* From game.c */
extern void report _((void));
extern int Hearer _((dbref thing));
extern int Commer _((dbref thing));
extern int Listener _((dbref thing));
extern dbref orator;
int parse_chat _((dbref player, char *command));
extern void fork_and_dump _((int forking));

/* From log.c */
extern void start_log _((FILE ** fp, const char *filename));
extern void end_log _((FILE * fp));
extern void do_log _((int logtype, dbref player, dbref object, const char *fmt,...));
extern void do_rawlog _((int logtype, const char *fmt,...));
extern void do_logwipe _((dbref player, int logtype, char *str));

/* From look.c */
extern void look_room _((dbref player, dbref loc, int flag));
extern void do_look_around _((dbref player));
extern void do_look_at _((dbref player, const char *name, int key));

#ifdef USE_MAILER
/* From mail.c */
extern struct mail *maildb;
extern void do_mail_clear _((dbref player, char *msglist));
extern void do_mail_purge _((dbref player));
extern void set_player_folder _((dbref player, int fnum));
extern void add_folder_name _((dbref player, int fld, const char *name));
extern struct mail *find_exact_starting_point _((dbref player));
extern void check_mail _((dbref player, int folder, int silent));
extern int dump_mail _((FILE * fp));
extern int load_mail _((FILE * fp));
extern void mail_init _((void));
extern int mdb_top;
#endif				/* USE_MAILER */

/* From move.c */
extern void enter_room _((dbref player, dbref loc));
extern int can_move _((dbref player, const char *direction));
extern void do_move _((dbref player, const char *direction, int type));
extern void moveto _((dbref what, dbref where));
extern void safe_tel _((dbref player, dbref dest));
extern dbref global_exit _((dbref player, const char *direction));
extern dbref remote_exit _((dbref loc, const char *direction));
extern void move_wrapper _((dbref player, const char *command));

/* From mycrypt.c */
extern char *mush_crypt _((const char *key));

/* From player.c */
extern int password_check _((dbref player, const char *password));
extern dbref lookup_player _((const char *name));

/* From plyrlist.c */
void clear_players _((void));
void add_player _((dbref player, char *alias));
void delete_player _((dbref player, char *alias));

/* From predicat.c */
#ifdef __GNUC__
extern char *tprintf (const char *fmt,...) 
     __attribute__ ((__format__ (__printf__, 1, 2)));
#else
extern char *tprintf _((const char *fmt,...));
#endif
extern int could_doit _((dbref player, dbref thing));
extern void did_it _((dbref player, dbref thing, const char *what, const char *def, const char *owhat, const char *odef, const char *awhat, dbref loc));
extern int can_see _((dbref player, dbref thing, int can_see_loc));
extern int controls _((dbref who, dbref what));
extern int can_pay_fees _((dbref who, int pennies));
extern void giveto _((dbref who, dbref pennies));
extern int payfor _((dbref who, int cost));
extern int nearby _((dbref obj1, dbref obj2));
#ifdef QUOTA
extern int get_current_quota _((dbref who));
extern void change_quota _((dbref who, int payment));
extern int pay_quota _((dbref who, int cost));
#endif
extern int ok_name _((const char *name));
extern int ok_player_name _((const char *name));
extern int ok_password _((const char *password));
extern dbref parse_match_possessive _((dbref player, const char *str));
extern void page_return _((dbref player, dbref target, const char *type, const char *message, const char *def));
extern char *grep_util _((dbref player, dbref thing, char *pattern, char *lookfor, int len, int insensitive));
extern dbref where_is _((dbref thing));
void charge_action _((dbref player, dbref thing, const char *awhat));
dbref first_visible _((dbref player, dbref thing));

/* From regexp.c (extract from Henry Spencer's package) */
extern regexp *regcomp _((char *));
extern int regexec _((register regexp *, register char *));
extern char regexp_errbuf[];

#ifdef ALLOW_RPAGE
/* From rpage.c */
extern void dump_server_database _((void));
extern void rpage_init _((void));
extern void rpage_shutdown _((void));
extern void recv_rpage _((void));
#endif				/* ALLOW_RPAGE */

/* From set.c */
extern void do_edit _((dbref player, dbref thing, char *q, char **argv));
extern void do_chzone _((dbref player, const char *name, const char *newobj));
extern void do_parent _((dbref player, char *name, char *parent_name));
extern int do_set _((dbref player, const char *name, char *flag));
extern void do_name _((dbref player, const char *name, char *newname));
extern void chown_object _((dbref player, dbref thing, dbref newowner));
extern void do_wipe _((dbref player, char *name));

/* From speech.c */
extern void ns_esnotify _((char *dest, dbref speaker, dbref (*func) (), void *fdata));
extern void notify_except _((dbref first, dbref exception, const char *msg));
extern void notify_except2 _((dbref first, dbref exc1, dbref exc2, const char *msg));
extern void do_pemit_list _((dbref player, char *list, const char *message));
extern int filter_found _((dbref thing, const char *msg, int flag));


/* From strutil.c */
extern char *split_token _((char **sp, char sep));
extern char *chopstr _((const char *str, int lim));
extern int string_prefix _((const char *string, const char *prefix));
extern const char *string_match _((const char *src, const char *sub));
extern char *strupper _((const char *s));
extern char *upcasestr _((char *s));
extern char *skip_space _((const char *s));
extern char *seek_char _((const char *s, char c));
extern int u_strlen _((const unsigned char *s));
extern unsigned char *u_strcpy _((unsigned char *target, const unsigned char *source));
#define u_strdup(x) (unsigned char *)strdup((char *) x)
#ifndef HAS_STRDUP
extern char *strdup _((const char *s));
#endif
extern int safe_chr _((char c, char *buf, char **bufp));
extern int safe_copy_str _((const char *c, char *buff, char **bp, int maxlen));
extern char *replace_string _((const char *old, const char *newbit, const char *string));
extern char *trim_space_sep _((char *str, char sep));
extern int do_wordcount _((char *str, char sep));
extern int minmatch _((const char *str, const char *target, int min));

#define safe_str(s,b,p)         safe_copy_str(s,b,p,BUFFER_LEN - 1)
#define safe_short_str(s,b,p)   safe_copy_str(s,b,p,SBUF_LEN - 1)

extern int ansi_strlen _((char *string));

/* From unparse.c */
extern char *unparse_boolexp _((dbref player, struct boolexp * b, int flag));
extern const char *real_unparse _((dbref player, dbref loc, int obey_myopic));
extern const char *object_header _((dbref player, dbref loc));
extern const char *unparse_object _((dbref player, dbref loc));

/* From utils.c */
extern void parse_attrib _((dbref player, char *str, dbref *thing, ATTR **attrib));
extern int member _((dbref thing, dbref list));
extern int recursive_member _((dbref disallow, dbref from, int count));
extern dbref remove_first _((dbref first, dbref what));
extern dbref reverse _((dbref list));
extern struct dblist *listcreate _((dbref ref));
extern void listadd _((struct dblist * head, dbref ref));
extern void listfree _((struct dblist * head));
extern int is_number _((const char *str));
extern int is_strict_number _((const char *str));
extern Malloc_t mush_malloc _((int size, const char *check));
extern void mush_free _((Malloc_t ptr, const char *check));
extern int getrandom _((int x));
extern char *shortname _((dbref it));

#ifdef USE_WARNINGS
/* From warnings.c */
extern void run_topology _((void));
extern void do_warnings _((dbref player, char *name, char *warns));
extern void do_wcheck _((dbref player, char *name));
extern void do_wcheck_all _((dbref player));
extern void set_initial_warnings _((dbref player));
extern const char *unparse_warnings _((dbref thing));
#endif

/* From wild.c */
extern int local_wild _((char *s, char *d, int p, int os));
extern int local_wild_match _((const char *s, const char *d));
extern int wildcard _((const char *s));
extern int quick_wild_new _((const char *tstr, const char *dstr, int cs));
extern int regexp_match_case _((const char *s, const char *d, int cs));
extern int wild_match_case _((const char *s, const char *d, int cs));
#define quick_wild(t,d) quick_wild_new(t,d,0)
#define regexp_match(s,d) regexp_match_case(s,d,1)
#define wild_match(s,d) wild_match_case(s,d,0)

/* From wiz.c */
extern void do_chownall _((dbref player, const char *name, const char *target));
extern void do_teleport _((dbref player, const char *arg1, const char *arg2));

/* From function.c and other fun*.c */
extern char *rptr[10];
extern char *strip_braces _((char const *line));
extern void save_global_regs _((const char *funcname, char *preserve[]));
extern void restore_global_regs _((const char *funcname, char *preserve[]));
extern int delim_check _((char *buff, char **bp, int nfargs, char **fargs, int sep_arg, char *sep));
extern int get_gender _((dbref player));


/* From boolexp.c */
extern int sizeof_boolexp _((struct boolexp * b));
extern int eval_boolexp _((dbref player, struct boolexp * b, dbref target, int nrecurs, lock_type ltype));
extern struct boolexp *parse_boolexp _((dbref player, const char *buf));


/* From destroy.c */
void do_destroy _((dbref player, char *name, int confirm));
void do_undestroy _((dbref player, char *name));
dbref free_get _((void));
void fix_free_list _((void));
void purge _((void));
void do_purge _((dbref player));

void dbck _((void));
void do_dbck _((dbref player));
void undestroy _((dbref player, dbref thing));

/* From chat.c */
#ifdef CHAT_SYSTEM
extern void init_chat _((void));
extern void do_channel _((dbref player, const char *name, const char *target, const char *com));
extern void do_chat _((dbref player, CHAN *chan, const char *arg1));
extern void do_chan_admin _((dbref player, char *name, const char *perms, int flag));
extern int find_channel _((const char *p, CHAN **chan));
extern int find_channel_partial _((const char *p, CHAN **chan, dbref player));
extern void do_channel_list _((dbref player, const char *partname));
extern int do_chat_by_name _((dbref player, const char *name, const char *msg));
extern void do_chan_decompile _((dbref player, const char *name));
extern void do_chan_chown _((dbref player, const char *name, const char *newowner));
#endif				/* CHAT_SYSTEM */

/* From db.c */
extern const char *set_string _((const char **ptr, const char *newstr));

/* lock.c */
struct boolexp *getlock _((dbref thing, lock_type type));
lock_type match_lock _((lock_type type));
void add_lock _((dbref thing, lock_type type, struct boolexp * key));
void delete_lock _((dbref thing, lock_type type));
void free_locks _((lock_list *ll));
int eval_lock _((dbref player, dbref thing, lock_type ltype));
void do_unlock _((dbref player, const char *name, lock_type type));
void do_lock _((dbref player, const char *name, const char *keyname, lock_type type));

/* local.c */
void local_startup _((void));
void local_dump_database _((void));
void local_shutdown _((void));
void local_timer _((void));
#ifdef LOCAL_DATA
void local_data_create _((dbref object));
void local_data_clone _((dbref clone, dbref source));
void local_data_free _((dbref object));
#endif

/* rwho.c */
int rwhocli_setup _((const char *server, const char *serverpw, const char *myname, const char *comment));
int rwhocli_shutdown _((void));
int rwhocli_userlogin _((const char *uid, const char *name, time_t tim));
int rwhocli_userlogout _((const char *uid));
int rwhocli_pingalive _((void));

/* funlist.c */
void do_gensort _((char **s, int n, int sort_type));


/* This is from sig.c, but put at the end because it confuses indent */
typedef
Signal_t(*Sigfunc) _((int));
#ifdef HAS_SIGACTION
#ifdef signal
#undef signal
#endif
#ifdef CAN_PROTOTYPE_SIGNAL
    extern Sigfunc signal _((int signo, Sigfunc func));
#endif
#endif
#endif				/* __EXTERNS_H */
