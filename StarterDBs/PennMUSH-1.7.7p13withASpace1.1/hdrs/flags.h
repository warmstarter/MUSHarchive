/* flags.h */

/* flag and powers stuff */

#ifndef __FLAGS_H
#define __FLAGS_H

#include "conf.h"
typedef struct flag_info FLAG;

/** A flag.
 * This structure represents a flag in the table of flags that are
 * available for setting on objects in the game.
 */
struct flag_info {
  const char *name;	/**< Name of the flag */
  char letter;		/**< Flag character, which may be nul */
  int type;		/**< Bitflags of object types this flag applies to */
  int bitpos;		/**< Bit position assigned to this flag for now */
  int perms;		/**< Bitflags of who can set this flag */
  int negate_perms;	/**< Bitflags of who can clear this flag */
};

typedef struct flag_alias FLAG_ALIAS;

/** A flag alias.
 * A simple structure that associates an alias with a canonical flag name.
 */
struct flag_alias {
  const char *alias;		/**< The alias name */
  const char *realname;		/**< The real name of the flag */
};

typedef struct power_info POWER;

/** A power.
 * This structure represent a power in the table of powers available
 * for setting on objects. It associates each with a bitmask
 */
struct power_info {
  const char *name;	/**< Name of the power */
  int flag;		/**< Associated bitmask */
};

typedef struct power_alias POWER_ALIAS;

/** A power alias.
 * A simple structure that associates an alias with a canonical power name.
 */
struct power_alias {
  const char *alias;		/**< The alias name */
  const char *realname;		/**< The real name of the power */
};

/* From flags.c */
extern FLAG *flag_hash_lookup(const char *name, int type);
extern int has_flag_by_name(dbref thing, const char *flag, int type);
extern const char *unparse_flags(dbref thing, dbref player);
extern const char *flag_description(dbref player, dbref thing);
extern int sees_flag(dbref privs, dbref thing, const char *name);
extern void set_flag(dbref player, dbref thing, const char *flag, int negate,
		     int hear, int listener);
extern const char *power_description(dbref thing);
extern int find_power(const char *name);
extern int flaglist_check(dbref player, dbref it, const char *fstr, int type);
extern int flaglist_check_long(dbref player, dbref it, const char *fstr,
			       int type);
extern FLAG *match_flag(const char *name);
extern int match_power(const char *name);
extern const char *show_command_flags(object_flag_type flags, int powers);
extern void twiddle_flag_internal(dbref thing, const char *flag, int negate);
extern object_flag_type new_flag_bitmask(void);
extern object_flag_type clone_flag_bitmask(object_flag_type given);
extern void copy_flag_bitmask(object_flag_type dest, object_flag_type given);
extern void destroy_flag_bitmask(object_flag_type bitmask);
extern void set_flag_bitmask(object_flag_type bitmask, int bit);
extern void clear_flag_bitmask(object_flag_type bitmask, int bit);
extern int has_bit(object_flag_type bitmask, int bitpos);
extern int has_all_bits(object_flag_type source, object_flag_type bitmask);
extern int null_flagmask(object_flag_type source);
extern int has_any_bits(object_flag_type source, object_flag_type bitmask);
extern object_flag_type string_to_bits(const char *str);
extern const char *bits_to_string(object_flag_type bitmask, dbref privs,
				  dbref thing);
extern void flag_write_all(FILE *);
extern void flag_read_all(FILE *);
extern int type_from_old_flags(long old_flags);
extern object_flag_type flags_from_old_flags(long old_flags, long old_toggles,
					     int type);
extern void add_flag(const char *name, const char letter, int type,
		     int perms, int negate_perms);
extern void do_list_flags(dbref player, const char *arg, int lc);
extern char *list_all_flags(const char *name, dbref privs, int which);
extern void do_flag_info(dbref player, const char *name);
extern void do_flag_delete(dbref player, const char *name);
extern void do_flag_disable(dbref player, const char *name);
extern void do_flag_alias(dbref player, const char *name, const char *alias);
extern void do_flag_enable(dbref player, const char *name);
extern void do_flag_restrict(dbref player, const char *name,
			     char *args_right[]);
extern void do_flag_add(dbref player, const char *name, char *args_right[]);


#define twiddle_flag_bitmask(bm,b,neg) (neg ? clear_flag_bitmask(bm,b) : \
                                                set_flag_bitmask(bm,b))
#define has_all_flags_by_mask(x,bm) has_all_bits(Flags(x),bm)
#define has_any_flags_by_mask(x,bm) has_any_bits(Flags(x),bm)
#define twiddle_flag(thing,f,negate) \
  twiddle_flag_bitmask(Flags(thing),f->bitpos,negate)
#define set_flag_internal(t,f) twiddle_flag_internal(t,f,0)
#define clear_flag_internal(t,f) twiddle_flag_internal(t,f,1)

/*---------------------------------------------------------------------
 * Object types (no longer part of the flags)
 */

#define TYPE_ROOM       0x1
#define TYPE_THING      0x2
#define TYPE_EXIT       0x4
#define TYPE_PLAYER     0x8
#define TYPE_GARBAGE    0x10
#define TYPE_MARKED     0x20
#define NOTYPE          0xFFFF



/*--------------------------------------------------------------------------
 * Flag permissions
 */

#define F_ANY           0x10	/* can be set by anyone - obsolete now */
#define F_INHERIT       0x20	/* must pass inherit check */
#define F_OWNED         0x40	/* can be set on owned objects */
#define F_ROYAL         0x80	/* can only be set by royalty */
#define F_WIZARD        0x100	/* can only be set by wizards */
#define F_GOD           0x200	/* can only be set by God */
#define F_INTERNAL      0x400	/* only the game can set this */
#define F_DARK          0x800	/* only God can see this flag */
#define F_MDARK         0x1000	/* admin/God can see this flag */
#define F_ODARK         0x2000	/* owner/admin/God can see this flag */
#define F_DISABLED      0x4000	/* flag can't be used */


/*--------------------------------------------------------------------------
 * Powers table
 */

#define CAN_BUILD       0x10	/* can use builder commands */
#define TEL_ANYWHERE    0x20	/* teleport self anywhere */
#define TEL_OTHER       0x40	/* teleport someone else */
#define SEE_ALL         0x80	/* can examine all and use priv WHO */
#define NO_PAY          0x100	/* Needs no money */
#define CHAT_PRIVS      0x200	/* can use restricted channels */
#define CAN_HIDE        0x400	/* can go DARK on the WHO list */
#define LOGIN_ANYTIME   0x800	/* not affected by restricted logins */
#define UNLIMITED_IDLE  0x1000	/* no inactivity timeout */
#define LONG_FINGERS    0x2000	/* can grab stuff remotely */
#define CAN_BOOT        0x4000	/* can boot off players */
#define CHANGE_QUOTAS   0x8000	/* can change other players' quotas */
#define SET_POLL        0x10000	/* can change the poll */
#define HUGE_QUEUE      0x20000	/* queue limit of db_top + 1 */
#define PS_ALL          0x40000	/* look at anyone's queue */
#define HALT_ANYTHING   0x80000	/* do @halt on others, and @allhalt */
#define SEARCH_EVERYTHING  0x100000	/* @stats, @search, @entrances all */
#define GLOBAL_FUNCS    0x200000	/* add global functions */
#define CREATE_PLAYER   0x400000	/* @pcreate */
#define IS_GUEST        0x800000	/* Guest, restrict access */
#define CAN_WALL        0x1000000	/* @wall */
#define CEMIT           0x2000000	/* Was: Can @cemit */
#define UNKILLABLE      0x4000000	/* Cannot be killed */
#define PEMIT_ALL       0x8000000	/* Can @pemit to HAVEN players */
#define NO_QUOTA        0x10000000	/* Has no quota restrictions */
#define LINK_ANYWHERE   0x20000000	/* Can @link an exit to any room */
#define OPEN_ANYWHERE   0x40000000	/* Can @open an exit from any room */
#define CAN_NSPEMIT     0x80000000	/* Can use @nspemit and nspemit() */

/* These powers are obsolete, but are kept around to implement
 * DBF_SPLIT_IMMORTAL
 */
#define CAN_DEBUG       0x4000000	/* Can set/unset the debug flag */
#define IMMORTAL        0x100	/* cannot be killed, uses no money */
#endif				/* __FLAGS_H */
