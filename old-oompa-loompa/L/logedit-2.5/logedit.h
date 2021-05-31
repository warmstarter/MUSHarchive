/* Logedit.h - configuration file for logedit */

/* Uncomment this line if compiling under SunOS */
/* #define SUNOS */

/* If your system has strings.h instead of string.h, uncomment this */
/* #define STRINGS_H */

/* This define should point to a full, valid directory where logedit
   can find its config file (.logeditrc by default) */
#define CONFDIR		"/cogsci/grad/alansz"

/* This is the name of the logedit config file */
#define CONFFILE	".logeditrc"

/* Uncomment this line if you use TinyMUSH 2.0 and log from there */
/* #define TINYMUSH */

/*  These unix functions compile and compare regexps. If you don't have
  these, you may have others, such as regcomp/regexec/regfree or
  or something else, in which case you'll have to hack at these
  defines some to make things work */
#define regexp_comp(x)	re_comp(x)
#define regexp_exec(x)	re_exec(x)

/* Mess at your own risk. */
#include <stdio.h>
#include <stdlib.h>
#ifndef SUNOS
#include <stddef.h>
#endif
#include <ctype.h>
#include <time.h>
#ifdef STRINGS_H
#include <strings.h>
#else
#include <string.h>
#endif

#ifdef CURSESX
#include <cursesX.h>
#endif
#ifdef CURSES
#include <curses.h>
#endif

#ifndef TINYMUSH
#define PENN150
#endif

#ifdef SUNOS
#define EXIT_FAILURE 1
#endif

#define VER             "v2.5"
#define SWITCHCHAR      '-'
/* curses.h defines TRUE and FALSE */
#ifndef CURSESX
#ifndef CURSES
#define TRUE    1
#define FALSE   0
#endif
#endif
#define BUFFER_LEN        4096
#define ADMIN           0x0001
#define CHANNEL         0x0002
#define PAGE            0x0004
#define WHISPER         0x0008
#define EXCEPT          0x0010
#define YOUCONVERT      0x0020
#define LEFT            0x0040
#define STATS            0x0080
#define TERSE            0x0100
#define DOING            0x0200
#define NOSPAM            0x0400
#define NOCONF            0x0800
#define REGEXPS            0x1000
#define BLANKLINE       0x2000
#define MAILNIX            0x4000
#define TFWORLDUSE        0x8000/* Use a single world */
#define TFWORLDSKIP        0x10000  /* Skip world(s) */
#define ANSI            0x20000
#define WRAP		0x40000
#define TIMEMSG		0x80000
#define ALTCONF		0x100000
#define USERPOSE	0x200000
#define QUICK		0x400000
#define TFMSG		0x800000
#define NONE		0               /* Special flag */

#define STATUS(x)    (switchlist & x) ? "  (on)" : " (off)"
#define MAX_EXCEPTS     30
#define MAX_WORLDS	30

#define DELIMITER        "#log"
#define PAGETOPREF      "You paged"
#define PPTOPREF        "Long distance to"
#define PPFROMPREF      "From afar,"
#define WALL            "Announcement:"
#define WIZWALL         "Broadcast"
#define GAMEPREF        "GAME:"
#define TFWORLDCHANGE	"---- World*----"
#define TIMEMESSAGE	"[*]*message sent to*"

#ifdef PENN150
#define WHISTOPREF        "You whisper,"
#define WPFROMPREF        "You sense:"
#define RWALL            "Admin"
#define MAILPREF        "MAIL:"
#define MAILDELIM        "------------------------------------"
#define DOING_START        "Player Name*On For*"
#define DOING_END        "There*player*connected."
#else
#ifdef TINYMUSH
#define WHISTOPREF        "You whisper"
#define WPFROMPREF        "You sense"
#define DOING_START        "Player Name*On For*"
#define DOING_END        "*Players logged in."
#endif
#endif
typedef int     boolean;
typedef unsigned long byte;
struct name_entry {
    char            name[30];
    int             says;
    int             mentions;
    struct name_entry *left;
    struct name_entry *right;
};
struct name_entry *name_tree = NULL;
struct regexp_entry {
    char            str[60];
    struct regexp_entry *next;
};
struct regexp_entry *regexp_list = NULL;
int             debug = FALSE;

struct switch_struct {
    char            sw_char;
    char            sw_name[20];
    int             sw_args;
    byte            sw_mask;
    char            sw_desc[80];
};
typedef struct switch_struct SWITCH;

SWITCH          sw_array[50] = {
    {'a', "@admin", 0, ADMIN,
    "Remove admin/broadcast messages"},
    {'c', "@channel", 0, CHANNEL,
    "Remove @channel messages"},
    {'d', "WHO", 0, DOING,
    "Remove DOING/WHO lists"},
    {'f', "tf worlds", 1, NONE,
    "Remove/select tf worlds"},
    {'l', "leaves/arrives", 0, LEFT,
    "Remove *has left, *has arrived, *goes home"},
    {'m', "@mail", 0, MAILNIX,
    "Remove @mail"},
    {'n', "no .logeditrc", 0, NOCONF,
    "Don't use .logeditrc config file"},
    {'o', "output file", 1, NONE,
    "Output the log to a specified file"},
    {'p', "page", 0, PAGE,
    "Remove pages"},
    {'q', "quick-strip", 0, QUICK,
    "Remove everything but player actions"},
    {'r', "regexp", 1, REGEXPS,
    "Remove all lines matching regexp"},
    {'s', "stats", 0, STATS,
    "Include attendance and other statistics"},
    {'t', "terse", 0, TERSE,
    "Remove room descriptions (terse)"},
    {'w', "whisper", 0, WHISPER,
    "Remove whispers"},
    {'x', "exclude", 1, EXCEPT,
    "Remove actions by specified player"},
    {'y', "you", 1, YOUCONVERT,
    "Convert 'You say,' to '<name> says,'"},
    {'A', "ANSI", 0, ANSI,
    "Remove ANSI codes"},
    {'F', "config file", 1, ALTCONF,
    "Use next argument as config file"},
    {'I', "insert line", 0, BLANKLINE,
    "Insert blank line between sentences"},
    {'O', "o-spam", 0, NOSPAM,
    "Agressive anti-spam (for osuccs/odrops)"},
    {'T', "@idle/@away/@haven", 0, TIMEMSG,
    "Remove @idle/@away/@haven timestamp messages"},
    {'W', "wrap", 1, WRAP,
    "Wordwrap at column # (default is 72)\nIndent first line at #, rest at #"},
    {':', "user-poses/says", 0, USERPOSE,
    "Remove lines beginning with : and \"" },
    {'%', "tf messages", 0, TFMSG,
    "Remove tf messages beginning with %'s"},
    {0, "", 0, NONE, ""}
};


/* Prototypes */

struct name_entry *new_entry(char *);
void            syntax(int paged);
void            edit(FILE * input, FILE * output, byte switchlist, char xname[MAX_EXCEPTS][30], int xcount, char yname[], char wname[MAX_WORLDS][80], int wcount, int wrap, int first_indent, int rest_indent);
boolean         match(char *str, char *pattern);
boolean         wild_match(char *str, char *pattern);
void            build_name_tree(FILE * input);
void            add_name_tree(char *name, struct name_entry * top);
void            print_stats(FILE * output, char *, char *);
void            print_tree_stats(FILE *, struct name_entry *, int);
char           *firstword(char *line);
void            msg(char *);
void            add_regexp(char *);
boolean         match_regexp(char *);
boolean         spam_match(char *, char *);
void            safe_fputs(char *, FILE *, int, int, int);
void            wrap_fputs(char *, FILE *, int, int, int);
void            parse_wrap(char *, int *, int *, int *);
boolean         add_mention(char *name, int issay);
boolean         isplayer(char *name);
struct regexp_entry *new_regexp(char *name);
char *nixansi(char *);
char *restword(char *);
