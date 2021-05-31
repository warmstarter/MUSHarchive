#ifndef __COMMAND_H
#define __COMMAND_H

#include "config.h"
#include "confmagic.h"

#define NUM_BYTES 20
typedef unsigned char switch_mask[NUM_BYTES];
#define SW_SET(m,n)     (m[(n) >> 3] |= (1 << ((n) & 0x7)))
#define SW_CLR(m,n)     (m[(n) >> 3] &= ~(1 << ((n) & 0x7)))
#define SW_ISSET(m,n)   (m[(n) >> 3] & (1 << ((n) & 0x7)))
#define SW_ZERO(m)      bzero(m, NUM_BYTES)

/* These are type restrictors */
#define CMD_T_ROOM      0x80000000
#define CMD_T_THING     0x40000000
#define CMD_T_EXIT      0x20000000
#define CMD_T_PLAYER    0x10000000
#define CMD_T_ANY       0xF0000000
#define CMD_T_GOD       0x08000000

/* Any unknown or undefined switches will be passed in switches, instead of causing error */
#define CMD_T_SWITCHES  0x02000000

/* Command is disabled, set with @command */
#define CMD_T_DISABLED  0x01000000

/* Command will fail if object is gagged */
#define CMD_T_NOGAGGED  0x00800000

/* Command will fail if object is a guest */
#define CMD_T_NOGUEST   0x00400000

/* Command will fail if object is fixed */
#define CMD_T_NOFIXED	0x00200000

/* INTERNAL : Command is listed in @list commands */
#define CMD_T_LISTED    0x00080000

/* INTERNAL : Command is an internal command, and shouldn't be matched
 * or aliased
 */
#define CMD_T_INTERNAL  0x00040000

/* Split arguments at = */
#define CMD_T_EQSPLIT    0x0001

/* Split into argv[] at ,s */
#define CMD_T_ARGS       0x0010

/* Split at spaces instead of commas. CMD_T_ARGS MUST also be defined */
#define CMD_T_ARG_SPACE  0x0020

/* Do NOT parse arguments */
#define CMD_T_NOPARSE    0x0040

#define CMD_T_LS_ARGS    CMD_T_ARGS
#define CMD_T_LS_SPACE   CMD_T_ARG_SPACE
#define CMD_T_LS_NOPARSE CMD_T_NOPARSE
#define CMD_T_RS_ARGS    CMD_T_ARGS<<4
#define CMD_T_RS_SPACE   CMD_T_ARG_SPACE<<4
#define CMD_T_RS_NOPARSE CMD_T_NOPARSE<<4

/* COMMAND prototype 
   Passed arguments:
   executor : Object issuing command.
   sw : switch_mask, check with the SW_ macros.
   raw : *FULL* unparsed, untouched command.
   switches : Any unhandled swithces, or NULL if none.
   args_raw : Full argument, untouched. null-string if none.
   arg_left : Left-side arguments, unparsed if CMD_T_NOPARSE.
   args_left : Parsed arguments, if CMD_T_ARGS is defined.

   Note that if you don't specify EQSPLIT, left is still the data you want. If you define EQSPLIT,
   there are also right_XX values.

   Special case:
   If the NOEVAL switch is given, AND EQSPLIT is defined, the right-side will not be parsed.
   If NOEVAL is givean the EQSPLIT isn't defined, the left-side won't be parsed.
 */

#ifdef CAN_NEWSTYLE
#define COMMAND(command_name) \
void command_name(COMMAND_INFO *cmd, dbref player, dbref cause, switch_mask sw,char *raw, char *switches, char *args_raw, \
                  char *arg_left, char *args_left[MAX_ARG], \
                  char *arg_right, char *args_right[MAX_ARG])
#else
#define COMMAND(command_name) \
void command_name _((COMMAND_INFO *cmd, dbref player, dbref cause, switch_mask sw,char *raw, char *switches, char *args_raw, \
                  char *arg_left, char *args_left[MAX_ARG], \
                  char *arg_right, char *args_right[MAX_ARG])); \
void \
command_name(cmd, player, cause, sw, raw, switches, args_raw, \
	     arg_left, args_left, arg_right, args_right) \
    COMMAND_INFO *cmd; \
    dbref player; \
    dbref cause; \
    switch_mask sw; \
    char *raw; \
    char *switches; \
    char *args_raw; \
    char *arg_left; \
    char *args_left[MAX_ARG]; \
    char *arg_right; \
    char *args_right[MAX_ARG];
#endif

#define COMMAND_PROTO(command_name) \
void command_name _((COMMAND_INFO *cmd, dbref player, dbref cause, switch_mask sw,char *raw, char *switches, char *args_raw, \
                  char *arg_left, char *args_left[MAX_ARG], \
                  char *arg_right, char *args_right[MAX_ARG]))

typedef struct command_list COMLIST;
struct command_list {
  const char *name;
  const char *switches;
  void (*func) ();
  unsigned int type;
  unsigned int flags, toggles, powers;
};

/* For things that DON'T get auto-aliased by mkalias, like 'i' for 'inv' and 'l' for 'look' */

typedef struct command_alias COMALIAS;
struct command_alias {
  const char *name;
  const char *alias;
};

typedef struct command_info COMMAND_INFO;
struct command_info {
  const char *name;
  void (*func) ();
  unsigned int type;
  unsigned int flags, toggles, powers;
  switch_mask sw;
};

typedef struct switch_value SWITCH_VALUE;
struct switch_value {
  const char *name;
  int value;
};

typedef struct com_sort_struc COMSORTSTRUC;
struct com_sort_struc {
  struct com_sort_struc *next;
  COMMAND_INFO *cmd;
};

#include "cmds.h"

#define SWITCH_NONE 0
#include "switches.h"

extern void strccat _((char *to, const char *from));
extern switch_mask *switchmask _((const char *switches));
extern COMMAND_INFO *command_find _((const char *name));
extern COMMAND_INFO *command_add _((const char *name, int type, int flags, int toggles, int powers, switch_mask (*sw), void (*func) ( /* ??? */ )));
extern COMMAND_INFO *command_modify _((const char *name, int type, int flags, int toggles, int powers, switch_mask (*sw), void (*func) ( /* ??? */ )));
extern void command_init_preconfig _((void));
extern void command_init_postconfig _((void));
extern void command_addalias _((COMMAND_INFO * cmd, char *prev, char *next));
extern void command_mkalias _((void));
extern void command_splitup _((dbref player, dbref cause, char *from, char *to, char **args, COMMAND_INFO * cmd, int side));
extern void command_argparse _((dbref player, dbref cause, char **from, char *to, char **argv, COMMAND_INFO * cmd, int side, int forcenoparse));
extern char *command_parse _((dbref player, dbref cause, char *string, int fromport));
extern void do_list_commands _((dbref player));
extern int command_check_byname _((dbref player, const char *name));

#endif				/* __COMMAND_H */
