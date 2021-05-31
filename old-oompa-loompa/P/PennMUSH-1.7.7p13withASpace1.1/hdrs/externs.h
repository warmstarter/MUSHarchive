/* externs.h */

#ifndef __EXTERNS_H
#define __EXTERNS_H
/* Get the time_t definition that we use in prototypes here */
#include <time.h>
#ifdef I_LIBINTL
#include <libintl.h>
#endif
#if defined(HAS_GETTEXT) && !defined(DONT_TRANSLATE)
#define T(str) gettext(str)
#define N_(str) gettext_noop(str)
#else
#define T(str) str
#define N_(str) str
#endif
#include "config.h"
#include "copyrite.h"
#include "conf.h"
#include "confmagic.h"
#ifndef HAS_STRCASECMP
#ifdef WIN32
#define strcasecmp(s1,s2) _stricmp((s1), (s2))
#define strncasecmp(s1,s2,n) _strnicmp((s1), (s2), (n))
#else
extern int strcasecmp(const char *s1, const char *s2);
extern int strncasecmp(const char *s1, const char *s2, size_t n);
#endif
#endif

/* these symbols must be defined by the interface */
extern time_t mudtime;

extern int shutdown_flag;	/* if non-zero, interface should shut down */
extern void emergency_shutdown(void);
extern void boot_desc(DESC *d);	/* remove a player */
extern DESC *player_desc(dbref player);	/* find descriptors */
extern DESC *inactive_desc(dbref player);	/* find descriptors */
extern DESC *port_desc(int port);	/* find descriptors */
extern void WIN32_CDECL flag_broadcast(const char *flag1,
				       const char *flag2, const char *fmt, ...)
  __attribute__ ((__format__(__printf__, 3, 4)));

extern void raw_notify(dbref player, const char *msg);
extern dbref short_page(const char *match);
extern dbref visible_short_page(dbref player, const char *match);
extern void do_doing(dbref player, const char *message);

/* the following symbols are provided by game.c */
extern void process_command(dbref player, char *command,
			    dbref cause, int from_port);
extern int init_game_dbs(void);
extern void init_game_postdb(const char *conf);
extern void init_game_config(const char *conf);
extern void dump_database(void);
extern void NORETURN panic(const char *message);
extern char *scan_list(dbref player, char *command);


#ifdef WIN32
/* From timer.c */
extern void init_timer(void);
extern void kill_timer(void);
#endif				/* WIN32 */

/* From bsd.c */
extern FILE *connlog_fp;
extern FILE *checklog_fp;
extern FILE *wizlog_fp;
extern FILE *tracelog_fp;
extern FILE *cmdlog_fp;
extern int restarting;
#ifdef SUN_OS
extern int f_close(FILE * file);
#define fclose(f) f_close(f);
#endif
extern int hidden(dbref player);
extern dbref guest_to_connect(dbref player);
void dump_reboot_db(void);

/* The #defs for our notify_anything hacks.. Errr. Functions */
#define NA_NORELAY      0x0001
#define NA_NOENTER      0x0002
#define NA_NOLISTEN     0x0004	/* Implies NORELAY. Sorta. */
#define NA_NOPENTER     0x0010
#define NA_PONLY        0x0020
#define NA_PUPPET       0x0040	/* Ok to puppet */
#define NA_PUPPET2      0x0080	/* Message to a player from a puppet */
#define NA_MUST_PUPPET  0x0100	/* Ok to puppet even in same room */
#define NA_INTERACTION  0x0200	/* Message follows interaction rules */
#define NA_INTER_DIDIT  0x0400	/* Message is from @verb or ilk */

typedef dbref (*na_lookup) (dbref, void *);
extern void notify_anything(dbref speaker, na_lookup func,
			    void *fdata,
			    char *(*nsfunc) (dbref,
					     na_lookup func,
					     void *, int), int flags,
			    const char *message);
extern dbref na_one(dbref current, void *data);
extern dbref na_next(dbref current, void *data);
extern dbref na_loc(dbref current, void *data);
extern dbref na_nextbut(dbref current, void *data);
extern dbref na_except(dbref current, void *data);
extern dbref na_except2(dbref current, void *data);
extern dbref na_exceptN(dbref current, void *data);
#ifdef CHAT_SYSTEM
extern dbref na_channel(dbref current, void *data);
#endif

#define notify(p,m)           notify_anything(orator, na_one, &(p), NULL, 0, m)
#define notify_must_puppet(p,m)           notify_anything(orator, na_one, &(p), NULL, NA_MUST_PUPPET, m)
#define notify_by(t,p,m)           notify_anything(t, na_one, &(p), NULL, 0, m)
#define notify_noecho(p,m)    notify_anything(orator, na_one, &(p), NULL, NA_NORELAY | NA_PUPPET, m)
#define quiet_notify(p,m)     if (!IsQuiet(p)) notify(p,m)
extern void notify_format(dbref player, const char *fmt, ...)
  __attribute__ ((__format__(__printf__, 2, 3)));

/* From compress.c */
/* Define this to get some statistics on the attribute compression
 * in @stats/tables. Only for word-based compression (COMPRESSION_TYPE 3 or 4
 */
/* #define COMP_STATS /* */
#if (COMPRESSION_TYPE != 0)
extern unsigned char *
compress(char const *s)
  __attribute_malloc__;
    extern char *uncompress(unsigned char const *s);
    extern char *safe_uncompress(unsigned char const *s) __attribute_malloc__;
#else
extern char ucbuff[];
#define init_compress(f) 0
#define compress(s) ((unsigned char *)strdup(s))
#define uncompress(s) (strcpy(ucbuff, (char *) s))
#define safe_uncompress(s) (strdup((char *) s))
#endif

/* From cque.c */
extern char *wenv[10], renv[NUMQ][BUFFER_LEN];
extern char *wnxt[10], *rnxt[NUMQ];
extern void do_second(void);
extern int do_top(int ncom);
extern void do_halt(dbref owner, const char *ncom, dbref victim);
extern void parse_que(dbref player, const char *command, dbref cause);
extern int queue_attribute_base
  (dbref executor, const char *atrname, dbref enactor, int noparent);
#define queue_attribute(a,b,c) queue_attribute_base(a,b,c,0)
#define queue_attribute_noparent(a,b,c) queue_attribute_base(a,b,c,1)
extern void dequeue_semaphores(dbref thing, char const *aname, int count,
			       int all, int drain);


/* From create.c */
extern dbref do_dig(dbref player, const char *name, char **argv, int tport);
extern dbref do_create(dbref player, char *name, int cost);
extern dbref do_real_open(dbref player, const char *direction,
			  const char *linkto, dbref pseudo);
extern void do_open(dbref player, const char *direction, char **links);
extern void do_link(dbref player, const char *name, const char *room_name,
		    int preserve);
extern void do_unlink(dbref player, const char *name);
extern dbref do_clone(dbref player, char *name, char *newname, int preserve);

/* From game.c */
extern void report(void);
extern int Hearer(dbref thing);
extern int Commer(dbref thing);
extern int Listener(dbref thing);
extern dbref orator;
int parse_chat(dbref player, char *command);
extern void fork_and_dump(int forking);

/* From look.c */
enum look_type { LOOK_NORMAL, LOOK_TRANS, LOOK_AUTO, LOOK_CLOUDYTRANS,
  LOOK_CLOUDY
};
extern void look_room(dbref player, dbref loc, enum look_type style);
extern void do_look_around(dbref player);
extern void do_look_at(dbref player, const char *name, int key);

/* From move.c */
extern void enter_room(dbref player, dbref loc, int nomovemsgs);
extern int can_move(dbref player, const char *direction);
enum move_type { MOVE_NORMAL, MOVE_GLOBAL, MOVE_ZONE };
extern void do_move(dbref player, const char *direction, enum move_type type);
extern void moveto(dbref what, dbref where);
extern void safe_tel(dbref player, dbref dest, int nomovemsgs);
extern int global_exit(dbref player, const char *direction);
extern int remote_exit(dbref loc, const char *direction);
extern void move_wrapper(dbref player, const char *command);
extern void do_follow(dbref player, const char *arg);
extern void do_unfollow(dbref player, const char *arg);
extern void do_desert(dbref player, const char *arg);
extern void do_dismiss(dbref player, const char *arg);
extern void clear_followers(dbref leader, int noisy);
extern void clear_following(dbref follower, int noisy);

/* From mycrypt.c */
extern char *mush_crypt(const char *key);

/* From player.c */
extern int password_check(dbref player, const char *password);
extern dbref lookup_player(const char *name);
/* from player.c */
extern dbref create_player(const char *name, const char *password,
			   const char *host, const char *ip);
extern dbref connect_player(const char *name, const char *password,
			    const char *host, const char *ip, char *errbuf);
extern void check_last(dbref player, const char *host, const char *ip);
extern void check_lastfailed(dbref player, const char *host);

/* From parse.c */
extern int is_number(const char *str);
extern int is_strict_number(const char *str);
extern int is_strict_integer(const char *str);

/* From plyrlist.c */
void clear_players(void);
void add_player(dbref player, const char *alias);
void delete_player(dbref player, const char *alias);

/* From predicat.c */

extern char *WIN32_CDECL tprintf(const char *fmt, ...)
  __attribute__ ((__format__(__printf__, 1, 2)));

extern int could_doit(dbref player, dbref thing);
extern void did_it(dbref player, dbref thing, const char *what,
		   const char *def, const char *owhat, const char *odef,
		   const char *awhat, dbref loc);
extern void real_did_it(dbref player, dbref thing, const char *what,
			const char *def, const char *owhat,
			const char *odef, const char *awhat, dbref loc,
			char *myenv[10]);
extern int can_see(dbref player, dbref thing, int can_see_loc);
extern int controls(dbref who, dbref what);
extern int can_pay_fees(dbref who, int pennies);
extern void giveto(dbref who, dbref pennies);
extern int payfor(dbref who, int cost);
extern int nearby(dbref obj1, dbref obj2);
#ifdef QUOTA
extern int get_current_quota(dbref who);
extern void change_quota(dbref who, int payment);
#endif
extern int ok_name(const char *name);
extern int ok_command_name(const char *name);
extern int ok_player_name(const char *name);
extern int ok_password(const char *password);
extern dbref parse_match_possessive(dbref player, const char *str);
extern void page_return(dbref player, dbref target, const char *type,
			const char *message, const char *def);
extern char *grep_util(dbref player, dbref thing, char *pattern,
		       char *lookfor, int len, int insensitive);
extern dbref where_is(dbref thing);
extern int charge_action(dbref player, dbref thing, const char *awhat);
dbref first_visible(dbref player, dbref thing);

/* From set.c */
extern void chown_object(dbref player, dbref thing, dbref newowner,
			 int preserve);

/* From speech.c */
extern char *ns_esnotify(dbref speaker, na_lookup func, void *fdata, int para);
extern void notify_except(dbref first, dbref exception, const char *msg,
			  int flags);
extern void notify_except2(dbref first, dbref exc1, dbref exc2,
			   const char *msg);
/* Return thing/PREFIX + msg */
extern void make_prefixstr(dbref thing, const char *msg, char *tbuf1);
extern int filter_found(dbref thing, const char *msg, int flag);

/* From strutil.c */
extern char *split_token(char **sp, char sep);
extern char *chopstr(const char *str, size_t lim);
extern int string_prefix(const char *RESTRICT string,
			 const char *RESTRICT prefix);
extern const char *string_match(const char *src, const char *sub);
extern char *strupper(const char *s);
extern char *strlower(const char *s);
extern char *upcasestr(char *s);
extern char *skip_space(const char *s);
extern char *seek_char(const char *s, char c);
extern int u_strlen(const unsigned char *s);
extern unsigned char *u_strcpy
  (unsigned char *target, const unsigned char *source);
#define u_strdup(x) (unsigned char *)strdup((char *) x)
#ifndef HAS_STRDUP
char *
strdup(const char *s)
  __attribute_malloc__;
#endif
    char *mush_strdup(const char *s, const char *check)
 __attribute_malloc__;
#ifdef WIN32
#ifndef HAS_VSNPRINTF
#define HAS_VSNPRINTF
#define vsnprintf _vsnprintf
#endif
#endif
    extern char *remove_markup(const char *orig, size_t * stripped_len);
    extern char *skip_leading_ansi(const char *s);
    typedef struct {
      char text[BUFFER_LEN];
      char *codes[BUFFER_LEN];
      size_t len;
    } ansi_string;


    extern ansi_string *parse_ansi_string(const char *src) __attribute_malloc__;
    extern void free_ansi_string(ansi_string *as);
    extern void populate_codes(ansi_string *as);
    extern void depopulate_codes(ansi_string *as);
#ifdef WIN32
#define strncoll(s1,s2,n) _strncoll((s1), (s2), (n))
#define strcasecoll(s1,s2) _stricoll((s1), (s2))
#define strncasecoll(s1,s2,n) _strnicoll((s1), (s2), (n))
#else
    extern int strncoll(const char *s1, const char *s2, size_t t);
    extern int strcasecoll(const char *s1, const char *s2);
    extern int strncasecoll(const char *s1, const char *s2, size_t t);
#endif

/* Append a character to the end of a BUFFER_LEN long string.
 * You shouldn't use arguments with side effects with this macro.
 */
#define safe_chr(x, buf, bp) \
                    ((*(bp) - (buf) >= BUFFER_LEN - 1) ? \
                        1 : (*(*(bp))++ = (x), 0))
/* Like sprintf */
    extern int safe_format(char *buff, char **bp, const char *RESTRICT fmt, ...)
  __attribute__ ((__format__(__printf__, 3, 4)));
/* Append an int to the end of a buffer */
    extern int safe_integer(int i, char *buff, char **bp);
    extern int safe_uinteger(unsigned int, char *buff, char **bp);
/* Same, but for a SBUF_LEN buffer, not BUFFER_LEN */
#define SBUF_LEN 64
    extern int safe_integer_sbuf(int i, char *buff, char **bp);
/* Append a NVAL to a string */
    extern int safe_number(NVAL n, char *buff, char **bp);
/* Append a dbref to a buffer */
    extern int safe_dbref(dbref d, char *buff, char **bp);
/* Append a string to a buffer */
    extern int safe_str(const char *s, char *buff, char **bp);
/* Append a string to a buffer, sticking it in quotes if there's a space */
    extern int safe_str_space(const char *s, char *buff, char **bp);
/* Append len characters of a string to a buffer */
    extern int safe_strl(const char *s, int len, char *buff, char **bp);
/* Append a boolean to the end of a string */
#define safe_boolean(x, buf, bufp) \
                safe_chr((x) ? '1' : '0', (buf), (bufp))
/* Append X characters to the end of a string, taking ansi and html codes into
   account. */
    extern int safe_ansi_string(ansi_string *as, size_t start, size_t len,
				char *buff, char **bp);
/* Append N copies of the character X to the end of a string */
    extern int safe_fill(char x, size_t n, char *buff, char **bp);
/* Append an accented string */
    extern int safe_accent(const char *RESTRICT base,
			   const char *RESTRICT tmplate, size_t len, char *buff,
			   char **bp);

    extern char *replace_string
      (const char *RESTRICT old, const char *RESTRICT newbit,
       const char *RESTRICT string) __attribute_malloc__;
    extern char *replace_string2(const char *old[2], const char *newbits[2],
				 const char *RESTRICT string)
 __attribute_malloc__;
    extern const char *standard_tokens[2];	/* ## and #@ */
    extern char *trim_space_sep(char *str, char sep);
    extern int do_wordcount(char *str, char sep);
    extern char *remove_word(char *list, char *word, char sep);
    extern char *next_in_list(const char **head);
    extern void safe_itemizer(int cur_num, int done, const char *delim,
			      const char *conjoin, const char *space,
			      char *buff, char **bp);

    typedef struct {
      const char *base;
      const char *entity;
    } accent_info;
    extern accent_info accent_table[];

    extern int ansi_strlen(const char *string);
    extern int ansi_strnlen(const char *string, size_t numchars);

/* From unparse.c */
    const char *real_unparse
      (dbref player, dbref loc, int obey_myopic, int use_nameformat,
       int use_nameaccent);
    extern const char *unparse_object(dbref player, dbref loc);
#define object_header(p,l) unparse_object(p,l)
    extern const char *unparse_object_myopic(dbref player, dbref loc);
    extern const char *unparse_room(dbref player, dbref loc);
    extern int nameformat(dbref player, dbref loc, char *tbuf1);
    extern const char *accented_name(dbref thing);

/* From utils.c */
    extern void parse_attrib(dbref player, char *str, dbref *thing,
			     ATTR **attrib);
    extern int member(dbref thing, dbref list);
    extern int recursive_member(dbref disallow, dbref from, int count);
    extern dbref remove_first(dbref first, dbref what);
    extern dbref reverse(dbref list);
    extern Malloc_t mush_malloc(size_t size,
				const char *check) __attribute_malloc__;
    extern void mush_free(Malloc_t RESTRICT ptr, const char *RESTRICT check);
    extern long get_random_long(long low, long high);
    extern char *shortname(dbref it);
    extern dbref absolute_room(dbref it);
    int can_interact(dbref from, dbref to, int type);


/* From warnings.c */
    extern void run_topology(void);
    extern void do_warnings(dbref player, const char *name, const char *warns);
    extern void do_wcheck(dbref player, const char *name);
    extern void do_wcheck_me(dbref player);
    extern void do_wcheck_all(dbref player);
    extern void set_initial_warnings(dbref player);
    extern const char *unparse_warnings(dbref thing);

/* From wild.c */
    extern int local_wild_match_case(const char *RESTRICT s,
				     const char *RESTRICT d, int cs);
    extern int wildcard(const char *s);
    extern int quick_wild_new(const char *RESTRICT tstr,
			      const char *RESTRICT dstr, int cs);
    extern int regexp_match_case(const char *RESTRICT s, const char *RESTRICT d,
				 int cs);
    extern int quick_regexp_match(const char *RESTRICT s,
				  const char *RESTRICT d, int cs);
    extern int wild_match_case(const char *RESTRICT s, const char *RESTRICT d,
			       int cs);
    extern int quick_wild(const char *RESTRICT tsr, const char *RESTRICT dstr);
#define regexp_match(s,d) regexp_match_case(s,d,1)
#define wild_match(s,d) wild_match_case(s,d,0)
#define local_wild_match(s,d) local_wild_match_case(s, d, 0)

/* From function.c and other fun*.c */
    typedef enum list_type {
      ALPHANUM_LIST, NUMERIC_LIST, DBREF_LIST, FLOAT_LIST, INSENS_ALPHANUM_LIST,
      UNKNOWN_LIST
    } list_type;
    extern char *rptr[10];
    extern char *strip_braces(char const *line);
    extern void save_global_regs(const char *funcname, char *preserve[]);
    extern void restore_global_regs(const char *funcname, char *preserve[]);
    extern void load_global_regs(char *preserve[]);
    extern void save_global_env(const char *funcname, char *preserve[]);
    extern void restore_global_env(const char *funcname, char *preserve[]);
    extern int delim_check
      (char *buff, char **bp, int nfargs, char **fargs, int sep_arg, char *sep);
    extern int get_gender(dbref player);
    extern int gencomp(char *a, char *b, list_type sort_type);

/* From destroy.c */
    void do_undestroy(dbref player, char *name);
    dbref free_get(void);
    void fix_free_list(void);
    void purge(void);
    void do_purge(dbref player);

    void dbck(void);
    void undestroy(dbref player, dbref thing);

/* From db.c */
    extern const char *getstring_noalloc(FILE * f);
    extern const char *set_name(dbref obj, const char *newname);
    extern long getref(FILE * f);
    extern void putref(FILE * f, long int ref);
    extern void putstring(FILE * f, const char *s);
    extern dbref new_object(void);
    extern int db_write_object(FILE * f, dbref i);
    extern dbref db_write(FILE * f, int flag);
    extern dbref db_read(FILE * f);

/* local.c */
    void local_startup(void);
    void local_dump_database(void);
    void local_dbck(void);
    void local_shutdown(void);
    void local_timer(void);
    void local_connect(dbref player, int isnew, int num);
    void local_disconnect(dbref player, int num);
    void local_data_create(dbref object);
    void local_data_clone(dbref clone, dbref source);
    void local_data_free(dbref object);
    int local_can_interact_first(dbref from, dbref to, int type);
    int local_can_interact_last(dbref from, dbref to, int type);

    /* flaglocal.c */
    void local_flags(void);

/* funlist.c */
    void do_gensort(char **s, int n, list_type sort_type);

/* sig.c */
    typedef void (*Sigfunc) (int);
/* Set up a signal handler. Use instead of signal() */
    Sigfunc install_sig_handler(int signo, Sigfunc f);
/* Call from in a signal handler to re-install the handler. Does nothing
   with persistent signals */
    void reload_sig_handler(int signo, Sigfunc f);
/* Ignore a signal. Like i_s_h with SIG_IGN) */
    void ignore_signal(int signo);
/* Block one signal temporarily. */
    void block_a_signal(int signo);
/* Unblock a signal */
    void unblock_a_signal(int signo);
/* Block all signals en masse. */
    void block_signals(void);
#endif				/* __EXTERNS_H */
