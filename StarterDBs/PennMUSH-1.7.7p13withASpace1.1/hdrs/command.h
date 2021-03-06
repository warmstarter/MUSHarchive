#ifndef __COMMAND_H
#define __COMMAND_H

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
#define CMD_T_NOFIXED   0x00200000

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
   switches : Any unhandled switches, or NULL if none.
   args_raw : Full argument, untouched. null-string if none.
   arg_left : Left-side arguments, unparsed if CMD_T_NOPARSE.
   args_left : Parsed arguments, if CMD_T_ARGS is defined.
   args_right : Parsed right-side arguments, if CMD_T_RSARGS is defined.

   Note that if you don't specify EQSPLIT, left is still the data you want. If you define EQSPLIT,
   there are also right_XX values.

   Special case:
   If the NOEVAL switch is given, AND EQSPLIT is defined, the right-side will not be parsed.
   If NOEVAL is givean the EQSPLIT isn't defined, the left-side won't be parsed.
 */

#define COMMAND(command_name) \
void command_name (COMMAND_INFO *cmd, dbref player, dbref cause, \
 switch_mask sw,char *raw, char *switches, char *args_raw, \
                  char *arg_left, char *args_left[MAX_ARG], \
                  char *arg_right, char *args_right[MAX_ARG]); \
void command_name(COMMAND_INFO *cmd __attribute__ ((__unused__)), \
                  dbref player __attribute__ ((__unused__)), \
                  dbref cause __attribute__ ((__unused__)), \
                  switch_mask sw __attribute__ ((__unused__)), \
                  char *raw __attribute__ ((__unused__)), \
                  char *switches __attribute__ ((__unused__)), \
                  char *args_raw __attribute__ ((__unused__)), \
                  char *arg_left __attribute__ ((__unused__)), \
                  char *args_left[MAX_ARG] __attribute__ ((__unused__)), \
                  char *arg_right __attribute__ ((__unused__)), \
                  char *args_right[MAX_ARG] __attribute__ ((__unused__)))

#define COMMAND_PROTO(command_name) \
void command_name (COMMAND_INFO *cmd, dbref player, dbref cause, switch_mask sw,char *raw, char *switches, char *args_raw, \
                  char *arg_left, char *args_left[MAX_ARG], \
                  char *arg_right, char *args_right[MAX_ARG])

/* For things that DON'T get auto-aliased by mkalias, like 'i' for 'inv' and 'l' for 'look' */

typedef struct command_alias COMALIAS;
/** An alias to a command.
 * This structure represents a command alias, which consists of the command's
 * real (canonical) name and an alias for that name.
 */
struct command_alias {
  const char *name;	/**< Canonical command name */
  const char *alias;	/**< Command alias */
};

typedef struct command_info COMMAND_INFO;
typedef void (*command_func) (COMMAND_INFO *, dbref, dbref, switch_mask, char *,
			      char *, char *, char *, char *[MAX_ARG], char *,
			      char *[MAX_ARG]);

/** A hook specification.
 */
struct hook_data {
  dbref obj;		/**< Object where the hook attribute is stored. */
  char *attrname;	/**< Attribute name of the hook attribute */
};

/** A command.
 * This structure represents a command in the table of available commands.
 */
struct command_info {
  const char *name;	/**< Canonical name of the command */
  command_func func;	/**< Function to call when command is run */
  unsigned int type;	/**< Types of objects that can use the command */
  object_flag_type flagmask;	/**< Flags to which the command is restricted */
  unsigned int powers;	/**< Powers to which the command is restricted */
  switch_mask sw;	/**< Bitflags of switches this command can take */
  /** Hooks on this command.
   */
  struct {
    struct hook_data before;	/**< Hook to evaluate before command */
    struct hook_data after;	/**< Hook to evaluate after command */
  } hooks;
};

typedef struct command_list COMLIST;
/** A command list entry.
 * This structure stores the static array of commands that are 
 * initially loaded into the command table. Commands can also be
 * added dynamically, outside of this array.
 */
struct command_list {
  const char *name;	/**< Command name */
  const char *switches;	/**< Space-separated list of switch names */
  command_func func;	/**< Function to call when command is run */
  unsigned int type;	/**< Types of objects that can use the command */
  const char *flagstr;	/**< Space-separated list of flags that can use */
  unsigned int powers;	/**< Powers to which the command is restricted */
};

typedef struct switch_value SWITCH_VALUE;
/** The value associated with a switch.
 * Command switches are given integral values at compile time when
 * the switchinc.c and switches.h files are rebuilt. This structure
 * associates switch names with switch numbers
 */
struct switch_value {
  const char *name;	/**< Name of the switch */
  int value;		/**< Number of the switch */
};

typedef struct com_sort_struc COMSORTSTRUC;

/** Sorted linked list of commands.
 * This structure is used to build a sorted linked list of pointers
 * to command data.
 */
struct com_sort_struc {
  struct com_sort_struc *next;	/**< Pointer to next in list */
  COMMAND_INFO *cmd;		/**< Command data */
};

/** Permissions for commands.
 * This structure is used to associate names for command permissions
 * (e.g. "player") with the appropriate bitmask
 */
struct command_perms_t {
  const char *name;	/**< Permission name */
  unsigned int type;	/**< Bitmask for this permission */
};

#define SWITCH_NONE 0
#include "switches.h"

extern switch_mask *switchmask(const char *switches);
extern COMMAND_INFO *command_find(const char *name);
extern COMMAND_INFO *command_find_exact(const char *name);
extern COMMAND_INFO *command_add
  (const char *name, int type, const char *flagstr, int powers,
   const char *switchstr, command_func func);
extern COMMAND_INFO *make_command
  (const char *name, int type, object_flag_type flagmask, int powers,
   switch_mask *sw, command_func func);
extern COMMAND_INFO *command_modify
  (const char *name, int type, object_flag_type flagmask, int powers,
   switch_mask *sw, command_func func);
extern void reserve_alias(const char *a);
extern int alias_command(const char *command, const char *alias);
extern void command_init_preconfig(void);
extern void command_init_postconfig(void);
extern void command_splitup
  (dbref player, dbref cause, char *from, char *to, char **args,
   COMMAND_INFO *cmd, int side);
extern void command_argparse
  (dbref player, dbref cause, char **from, char *to, char **argv,
   COMMAND_INFO *cmd, int side, int forcenoparse);
extern char *command_parse
  (dbref player, dbref cause, char *string, int fromport);
extern void do_list_commands(dbref player, int lc);
extern char *list_commands(void);
extern int command_check_byname(dbref player, const char *name);
extern int restrict_command(const char *name, const char *restriction);
extern void reserve_aliases(void);
extern void local_commands(void);

#endif				/* __COMMAND_H */
