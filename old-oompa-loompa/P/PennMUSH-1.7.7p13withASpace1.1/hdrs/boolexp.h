#ifndef BOOLEXP_H
#define BOOLEXP_H
#include "copyrite.h"

/* Boolean expressions, for locks */
typedef enum boolexp_type {
  BOOLEXP_AND,
  BOOLEXP_OR,
  BOOLEXP_NOT,
  BOOLEXP_CONST,
  BOOLEXP_ATR,
  BOOLEXP_IND,
  BOOLEXP_CARRY,
  BOOLEXP_IS,
  BOOLEXP_OWNER,
  BOOLEXP_EVAL,
  BOOLEXP_FLAG,
  BOOLEXP_BOOL
} boolexp_type;

/* tokens for locks */
#define NOT_TOKEN '!'
#define AND_TOKEN '&'
#define OR_TOKEN '|'
#define AT_TOKEN '@'
#define IN_TOKEN '+'
#define IS_TOKEN '='
#define OWNER_TOKEN '$'

/** An attribute lock specification.
 * This structure is a piece of a boolexp that's used to store 
 * attribute locks (CANDO:1), eval locks (CANDO/1), and flag locks
 * FLAG^WIZARD.
 */
struct boolatr {
  const char *name;		/**< Name of attribute, flag, etc. to test */
  char text[BUFFER_LEN];	/**< Value to test against */
};

/** A boolean expression.
 * Boolean expressions are most widely used in locks. This structure
 * is a general representation of the possible boolean expressions
 * that can be specified in MUSHcode.
 */
struct boolexp {
  /** Type of expression.
   * The type of expressio is one of the boolexp_type's, such as
   * and, or, not, constant, attribute, indirect, carry, is,
   * owner, eval, flag, etc.
   */
  boolexp_type type;
  dbref thing;			/**< An object, or a boolean val */
  /** The expression itself.
   * This union comprises the various possible types of data we
   * might need to represent any of the expression types.
   */
  union {
    /** And and or locks: combinations of boolexps.
     * This union member is used with and and or locks.
     */
    struct {
      struct boolexp *a;	/**< One boolean expression */
      struct boolexp *b;	/**< Another boolean expression */
    } sub;
    struct boolexp *n;		/**< Not locks: boolean expression to negate */
    struct boolatr *atr_lock;	/**< Atr, eval and flag locks */
    const char *ind_lock;	/**< Indirect locks */
  } data;
};

#define TRUE_BOOLEXP ((struct boolexp *) 0)
#define FALSE_ATOM (0)
#define TRUE_ATOM (1)

/* From boolexp.c */
extern struct boolatr *alloc_atr(const char *name, const char *s);
extern struct boolexp *dup_bool(struct boolexp *b);
extern struct boolexp *alloc_bool(void);
extern int sizeof_boolexp(struct boolexp *b);
extern int eval_boolexp(dbref player, struct boolexp *b, dbref target);
extern struct boolexp *parse_boolexp(dbref player, const char *buf,
				     lock_type ltype);
extern void free_boolexp(struct boolexp *b);
struct boolexp *getboolexp(FILE * f, const char *ltype);
void putboolexp(FILE * f, struct boolexp *b);
enum u_b_f { UB_ALL, UB_DBREF };
extern char *unparse_boolexp(dbref player, struct boolexp *b, enum u_b_f flag);
#endif				/* BOOLEXP_H */
