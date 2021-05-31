/* flags.c */

/* Functions to cope with flags */
#include "config.h"

#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif

#include "conf.h"
#include "mushdb.h"
#include "externs.h"
#include "intrface.h"
#include "match.h"
#include "htab.h"
#include "confmagic.h"

extern void hide_player _((dbref player, int hide));

#define FLAG_HASH_SIZE 256
#define FLAG_HASH_MASK 255

typedef struct flag_info FLAG;

struct flag_info {
  const char *name;
  char letter;
  int type;
  int flag;
  int perms;
  int negate_perms;
};

typedef struct flag_entry FLAGENT;

struct flag_entry {
  const char *name;
  FLAG *entry;
};

typedef struct flag_alias FLAG_ALIAS;

struct flag_alias {
  const char *alias;
  const char *realname;
};

typedef struct power_info POWER;
struct power_info {
  const char *name;
  int flag;
};

typedef struct power_alias POWER_ALIAS;
struct power_alias {
  const char *alias;
  const char *realname;
};

static FLAG *flag_hash_lookup _((const char *name));
static void flag_hash_insert _((const char *name, FLAG *entry));
void init_flag_hashtab _((void));
int can_set_flag _((dbref player, dbref thing, FLAG *flagp, int negate));
const char *unparse_flags _((dbref thing, dbref player));
const char *flag_description _((dbref player, dbref thing));
const char *togglemask_to_string _((int type, object_flag_type mask));
static FLAG *letter_to_flagptr _((char c, int type, int *toggle));
object_flag_type find_flag _((char *name, int type, int *toggle, int is_conf));
void decompile_flags _((dbref player, dbref thing, const char *name));
void decompile_powers _((dbref player, dbref thing, const char *name));
int convert_flags _((dbref player, char *s, object_flag_type *p_mask, object_flag_type *p_toggle, object_flag_type *p_type));
void set_flag _((dbref player, dbref thing, char *flag, int negate, int hear, int listener));
int handle_flaglists _((dbref player, char *name, char *fstr, int type));
int sees_flag _((dbref privs, dbref thing, char *name));

HASHTAB htab_flag;

/*   Name       Lettter Type            Flag            Perms   Negate_Perm */
FLAG flag_table[] =
{

  {"CHOWN_OK", 'C', NOTYPE, CHOWN_OK, F_ANY, F_ANY},
  {"DARK", 'D', NOTYPE, DARK, F_ANY, F_ANY},
  {"GOING", 'G', NOTYPE, GOING, F_INTERNAL, F_ANY},
  {"HAVEN", 'H', NOTYPE, HAVEN, F_ANY, F_ANY},
  {"INHERIT", 'I', NOTYPE, INHERIT, F_INHERIT, F_INHERIT},
  {"LINK_OK", 'L', NOTYPE, LINK_OK, F_ANY, F_ANY},
  {"OPAQUE", 'O', NOTYPE, OPAQUE, F_ANY, F_ANY},
  {"QUIET", 'Q', NOTYPE, QUIET, F_ANY, F_ANY},
  {"STICKY", 'S', NOTYPE, STICKY, F_ANY, F_ANY},
  {"UNFINDABLE", 'U', NOTYPE, UNFIND, F_ANY, F_ANY},
  {"VISUAL", 'V', NOTYPE, VISUAL, F_ANY, F_ANY},
  {"WIZARD", 'W', NOTYPE, WIZARD, F_INHERIT | F_WIZARD,
   F_INHERIT | F_WIZARD},
  {"SAFE", 'X', NOTYPE, SAFE, F_ANY, F_ANY},
  {"AUDIBLE", 'a', NOTYPE, AUDIBLE, F_ANY, F_ANY},
  {"DEBUG", 'b', NOTYPE, DEBUGGING, F_ANY, F_ANY},
#ifdef USE_WARNINGS
  {"NO_WARN", 'w', NOTYPE, NOWARN, F_ANY, F_ANY},
#endif
  {"ENTER_OK", 'e', NOTYPE, ENTER_OK, F_ANY, F_ANY},
  {"HALT", 'h', NOTYPE, HALT, F_ANY, F_ANY},
  {"NO_COMMAND", 'n', NOTYPE, NO_COMMAND, F_ANY, F_ANY},
  {"LIGHT", 'l', NOTYPE, LIGHT, F_ANY, F_ANY},
#ifdef ROYALTY_FLAG
  {"ROYALTY", 'r', NOTYPE, ROYALTY, F_INHERIT | F_ROYAL,
   F_INHERIT | F_ROYAL},
#endif
  {"TRANSPARENT", 't', NOTYPE, TRANSPARENTED, F_ANY, F_ANY},
  {"VERBOSE", 'v', NOTYPE, VERBOSE, F_ANY, F_ANY},
  {"STARTUP", 'z', NOTYPE, STARTUP, F_INTERNAL,
   F_INTERNAL},

  {"ANSI", 'A', TYPE_PLAYER, PLAYER_ANSI, F_ANY, F_ANY},
#ifdef EXTENDED_ANSI
  {"COLOR", 'C', TYPE_PLAYER, PLAYER_COLOR, F_ANY, F_ANY},
  {"FORCE_WHITE", 'f', TYPE_PLAYER, PLAYER_FORCEWHITE, F_ANY, F_ANY},
#endif
  {"MONITOR", 'M', TYPE_PLAYER, PLAYER_MONITOR, F_ROYAL, F_ANY},
  {"NOSPOOF", 'N', TYPE_PLAYER, PLAYER_NOSPOOF, F_ANY, F_ANY},
  {"ZONE", 'Z', TYPE_PLAYER, PLAYER_ZONE, F_ANY, F_ANY},
  {"CONNECTED", 'c', TYPE_PLAYER, PLAYER_CONNECT, F_INTERNAL | F_MDARK,
   F_INTERNAL | F_MDARK},
  {"GAGGED", 'g', TYPE_PLAYER, PLAYER_GAGGED, F_WIZARD, F_WIZARD},
  {"MYOPIC", 'm', TYPE_PLAYER, PLAYER_MYOPIC, F_ANY, F_ANY},
  {"TERSE", 'x', TYPE_PLAYER, PLAYER_TERSE, F_ANY, F_ANY},
#ifdef JURY_OK
  {"JURY_OK", 'j', TYPE_PLAYER, PLAYER_JURY, F_ROYAL, F_ROYAL},
  {"JUDGE", 'J', TYPE_PLAYER, PLAYER_JUDGE, F_ROYAL, F_ROYAL},
#endif
#ifdef FIXED_FLAG
  {"FIXED", 'F', TYPE_PLAYER, PLAYER_FIXED, F_WIZARD, F_WIZARD},
#endif
#ifdef ONLINE_REG
  {"UNREGISTERED", '?', TYPE_PLAYER, PLAYER_UNREG, F_ROYAL, F_ROYAL},
#endif
#ifdef VACATION_FLAG
  {"ON-VACATION", 'o', TYPE_PLAYER, PLAYER_VACATION, F_ANY, F_ANY},
#endif
  {"SUSPECT", 's', TYPE_PLAYER, PLAYER_SUSPECT, F_WIZARD | F_MDARK,
   F_WIZARD | F_MDARK},
#ifdef TREKMUSH
  {"IN-CHARACTER", '~', TYPE_PLAYER, PLAYER_IN_CHARACTER, F_WIZARD | F_MDARK, F_WIZARD | F_MDARK},
#endif /* TREKMUSH */

  {"MONITOR", 'M', TYPE_THING, THING_LISTEN, F_ANY, F_ANY},
  {"DESTROY_OK", 'd', TYPE_THING, THING_DEST_OK, F_ANY, F_ANY},
  {"PUPPET", 'p', TYPE_THING, THING_PUPPET, F_ANY, F_ANY},
  {"NO_LEAVE", 'N', TYPE_THING, THING_NOLEAVE, F_ANY, F_ANY},
#ifdef TREKMUSH
  {"SPACE-OBJECT", '+', TYPE_THING, THING_SPACE_OBJECT, F_WIZARD, F_WIZARD},
  {"ORG-OBJECT", '=', TYPE_THING, THING_ORG_OBJECT, F_WIZARD, F_WIZARD},
#endif /* TREKMUSH */

  {"ABODE", 'A', TYPE_ROOM, ROOM_ABODE, F_ANY, F_ANY},
  {"FLOATING", 'F', TYPE_ROOM, ROOM_FLOATING, F_ANY, F_ANY},
  {"JUMP_OK", 'J', TYPE_ROOM, ROOM_JUMP_OK, F_ANY, F_ANY},
  {"MONITOR", 'M', TYPE_ROOM, ROOM_LISTEN, F_ANY, F_ANY},
  {"Z_TEL", 'Z', TYPE_ROOM, ROOM_Z_TEL, F_ANY, F_ANY},
  {"NO_TEL", 'N', TYPE_ROOM, ROOM_NO_TEL, F_ANY, F_ANY},
#ifdef UNINSPECTED_FLAG
  {"UNINSPECTED", 'u', TYPE_ROOM, ROOM_UNINSPECT, F_ROYAL, F_ROYAL},
#endif

  {"CLOUDY", 'x', TYPE_EXIT, EXIT_CLOUDY, F_ANY, F_ANY},

  {"ACCESSED", '\0', NOTYPE, ACCESSED, F_INTERNAL | F_DARK,
   F_INTERNAL | F_DARK},
  {"MARKED", '\0', NOTYPE, MARKED, F_INTERNAL | F_DARK,
   F_INTERNAL | F_DARK},
  {"GOING_TWICE", '\0', NOTYPE, GOING_TWICE, F_INTERNAL | F_DARK,
   F_INTERNAL | F_DARK},
  {NULL, '\0', 0, 0, 0, 0}
};

FLAG type_table[] =
{
  {"PLAYER", 'P', TYPE_PLAYER, TYPE_PLAYER, F_INTERNAL,
   F_INTERNAL},
  {"ROOM", 'R', TYPE_ROOM, TYPE_ROOM, F_INTERNAL,
   F_INTERNAL},
  {"EXIT", 'E', TYPE_EXIT, TYPE_EXIT, F_INTERNAL,
   F_INTERNAL},
  {"THING", 'T', TYPE_THING, TYPE_THING, F_INTERNAL,
   F_INTERNAL},
  {NULL, '\0', 0, 0, 0, 0}
};

static FLAG_ALIAS flag_alias_tab[] =
{
  {"CHOWN_O", "CHOWN_OK"},
  {"CHOWN_", "CHOWN_OK"},
  {"CHOWN", "CHOWN_OK"},
  {"CHOW", "CHOWN_OK"},
  {"CHO", "CHOWN_OK"},
  {"CH", "CHOWN_OK"},
  {"DAR", "DARK"},
  {"DA", "DARK"},
  {"GOIN", "GOING"},
  {"GOI", "GOING"},
  {"GO", "GOING"},
  {"HAVE", "HAVEN"},
  {"HAV", "HAVEN"},
  {"INHERI", "INHERIT"},
  {"INHER", "INHERIT"},
  {"INHE", "INHERIT"},
  {"INH", "INHERIT"},
  {"IN", "INHERIT"},
  {"I", "INHERIT"},
  {"LINK_O", "LINK_OK"},
  {"LINK_", "LINK_OK"},
  {"LINK", "LINK_OK"},
  {"LIN", "LINK_OK"},
  {"OPAQU", "OPAQUE"},
  {"OPAQ", "OPAQUE"},
  {"OPA", "OPAQUE"},
  {"OP", "OPAQUE"},
  {"O", "OPAQUE"},
  {"QUIE", "QUIET"},
  {"QUI", "QUIET"},
  {"QU", "QUIET"},
  {"Q", "QUIET"},
  {"STICK", "STICKY"},
  {"STIC", "STICKY"},
  {"STI", "STICKY"},
  {"ST", "STICKY"},
  {"UNFINDABL", "UNFINDABLE"},
  {"UNFINDABL", "UNFINDABLE"},
  {"UNFINDAB", "UNFINDABLE"},
  {"UNFINDA", "UNFINDABLE"},
  {"UNFIND", "UNFINDABLE"},
  {"UNFIN", "UNFINDABLE"},
  {"UNFI", "UNFINDABLE"},
  {"UNF", "UNFINDABLE"},
  {"UN", "UNFINDABLE"},
  {"U", "UNFINDABLE"},
  {"VISUA", "VISUAL"},
  {"VISU", "VISUAL"},
  {"VIS", "VISUAL"},
  {"VI", "VISUAL"},
  {"WIZAR", "WIZARD"},
  {"WIZA", "WIZARD"},
  {"WIZ", "WIZARD"},
  {"WI", "WIZARD"},
  {"W", "WIZARD"},
  {"SAF", "SAFE"},
  {"SA", "SAFE"},
  {"AUDIBL", "AUDIBLE"},
  {"AUDIB", "AUDIBLE"},
  {"AUDI", "AUDIBLE"},
  {"AUD", "AUDIBLE"},
  {"AU", "AUDIBLE"},
  {"DEBU", "DEBUG"},
  {"DEB", "DEBUG"},
  {"TRACE", "DEBUG"},
  {"TRAC", "DEBUG"},
  {"ENTER_O", "ENTER_OK"},
  {"ENTER_", "ENTER_OK"},
  {"ENTER", "ENTER_OK"},
  {"ENTE", "ENTER_OK"},
  {"ENT", "ENTER_OK"},
  {"EN", "ENTER_OK"},
#ifdef USE_WARNINGS
  {"NO_WAR", "NO_WARN"},
  {"NO_WA", "NO_WARN"},
  {"NO_W", "NO_WARN"},
  {"NOWARN", "NO_WARN"},
  {"NOWAR", "NO_WARN"},
  {"NOWA", "NO_WARN"},
#endif
  {"HAL", "HALT"},
  {"NO_COMMAN", "NO_COMMAND"},
  {"NO_COMMA", "NO_COMMAND"},
  {"NO_COMM", "NO_COMMAND"},
  {"NO_COM", "NO_COMMAND"},
  {"NO_CO", "NO_COMMAND"},
  {"NO_C", "NO_COMMAND"},
#ifdef ROYALTY_FLAG
  {"ROYALT", "ROYALTY"},
  {"ROYAL", "ROYALTY"},
  {"ROYA", "ROYALTY"},
  {"ROY", "ROYALTY"},
#endif
  {"TRANSPAREN", "TRANSPARENT"},
  {"TRANSPARE", "TRANSPARENT"},
  {"TRANSPAR", "TRANSPARENT"},
  {"TRANSPA", "TRANSPARENT"},
  {"TRANSP", "TRANSPARENT"},
  {"TRANS", "TRANSPARENT"},
  {"TRAN", "TRANSPARENT"},
  {"TRA", "TRANSPARENT"},
  {"TR", "TRANSPARENT"},
  {"VERBOS", "VERBOSE"},
  {"VERBO", "VERBOSE"},
  {"VERB", "VERBOSE"},
  {"VER", "VERBOSE"},
  {"VE", "VERBOSE"},
  {"ANS", "ANSI"},
  {"AN", "ANSI"},
  {"LISTENER", "MONITOR"},
  {"LISTENE", "MONITOR"},
  {"LISTEN", "MONITOR"},
  {"LISTE", "MONITOR"},
  {"LIST", "MONITOR"},
  {"LIS", "MONITOR"},
  {"MONITO", "MONITOR"},
  {"MONIT", "MONITOR"},
  {"MONI", "MONITOR"},
  {"MON", "MONITOR"},
  {"MO", "MONITOR"},
  {"NOSPOO", "NOSPOOF"},
  {"NOSPO", "NOSPOOF"},
  {"NOSP", "NOSPOOF"},
  {"NOS", "NOSPOOF"},
  {"ZON", "ZONE"},
  {"ZO", "ZONE"},
  {"Z", "ZONE"},
  {"CONNECTE", "CONNECTED"},
  {"CONNECT", "CONNECTED"},
  {"CONNEC", "CONNECTED"},
  {"CONNE", "CONNECTED"},
  {"CONN", "CONNECTED"},
  {"CON", "CONNECTED"},
  {"CO", "CONNECTED"},
  {"GAG", "GAGGED"},
  {"MYOPI", "MYOPIC"},
  {"MYOP", "MYOPIC"},
  {"MYO", "MYOPIC"},
  {"MY", "MYOPIC"},
  {"TERS", "TERSE"},
  {"TER", "TERSE"},
#ifdef EXTENDED_ANSI
  {"COLOUR", "COLOR"},
  {"COLOU", "COLOR"},
  {"COLO", "COLOR"},
  {"COL", "COLOR"},
  {"FORCEWHITE", "FORCE_WHITE"},
  {"WHITE", "FORCE_WHITE"},
  {"WHIT", "FORCE_WHITE"},
  {"WHI", "FORCE_WHITE"},
  {"WEIRDANSI", "FORCE_WHITE"},
#endif
#ifdef JURY_OK
  {"JURYOK", "JURY_OK"},
  {"JURY", "JURY_OK"},
  {"JUR", "JURY_OK"},
  {"JUDG", "JUDGE"},
  {"JUD", "JUDGE"},
#endif
#ifdef FIXED_FLAG
  {"FIXE", "FIXED"},
  {"FIX", "FIXED"},
  {"FI", "FIXED"},
#endif
#ifdef ONLINE_REG
  {"UNREGISTER", "UNREGISTERED"},
  {"UNREGIST", "UNREGISTERED"},
  {"UNREGIS", "UNREGISTERED"},
  {"UNREGI", "UNREGISTERED"},
  {"UNREG", "UNREGISTERED"},
  {"UNRE", "UNREGISTERED"},
  {"UNR", "UNREGISTERED"},
#endif
#ifdef VACATION_FLAG
  {"ON-VACATI", "ON-VACATION"},
  {"ON-VACAT", "ON-VACATION"},
  {"ON-VACA", "ON-VACATION"},
  {"ON-VAC", "ON-VACATION"},
  {"ON-VA", "ON-VACATION"},
  {"ON-V", "ON-VACATION"},
  {"VACATION", "ON-VACATION"},
  {"VACATIO", "ON-VACATION"},
  {"VACATI", "ON-VACATION"},
  {"VACAT", "ON-VACATION"},
  {"VACA", "ON-VACATION"},
  {"VAC", "ON-VACATION"},
#endif
  {"SUSPEC", "SUSPECT"},
  {"SUSPE", "SUSPECT"},
  {"SUSP", "SUSPECT"},
  {"SUS", "SUSPECT"},
  {"SU", "SUSPECT"},
  {"DEST_OK", "DESTROY_OK"},
  {"DESTROY_O", "DESTROY_OK"},
  {"DESTROY_", "DESTROY_OK"},
  {"DESTROY", "DESTROY_OK"},
  {"DESTRO", "DESTROY_OK"},
  {"DESTR", "DESTROY_OK"},
  {"DEST", "DESTROY_OK"},
  {"DES", "DESTROY_OK"},
  {"DE", "DESTROY_OK"},
  {"PUPPE", "PUPPET"},
  {"PUPP", "PUPPET"},
  {"PUP", "PUPPET"},
  {"PU", "PUPPET"},
  {"P", "PUPPET"},
  {"NO_LEAV", "NO_LEAVE"},
  {"NO_LEA", "NO_LEAVE"},
  {"NO_LE", "NO_LEAVE"},
  {"NO_L", "NO_LEAVE"},
  {"NOLEAVE", "NO_LEAVE"},
  {"NOLEAV", "NO_LEAVE"},
  {"NOLEA", "NO_LEAVE"},
  {"NOLE", "NO_LEAVE"},
  {"NOL", "NO_LEAVE"},
  {"ABOD", "ABODE"},
  {"ABO", "ABODE"},
  {"AB", "ABODE"},
  {"FLOATIN", "FLOATING"},
  {"FLOATI", "FLOATING"},
  {"FLOAT", "FLOATING"},
  {"FLOA", "FLOATING"},
  {"FLO", "FLOATING"},
  {"FL", "FLOATING"},
  {"F", "FLOATING"},
  {"JUMP_O", "JUMP_OK"},
  {"JUMP_", "JUMP_OK"},
  {"JUMP", "JUMP_OK"},
  {"JUM", "JUMP_OK"},
  {"JU", "JUMP_OK"},
  {"J", "JUMP_OK"},
  {"NO_TE", "NO_TEL"},
  {"NO_T", "NO_TEL"},
  {"Z_TE", "Z_TEL"},
  {"Z_T", "Z_TEL"},
#ifdef UNINSPECTED_FLAG
  {"UNINSPECT", "UNINSPECTED"},
  {"UNINSPEC", "UNINSPECTED"},
  {"UNINSPE", "UNINSPECTED"},
  {"UNINSP", "UNINSPECTED"},
  {"UNINS", "UNINSPECTED"},
  {"UNIN", "UNINSPECTED"},
  {"UNI", "UNINSPECTED"},
#endif
#ifdef TREKMUSH
  {"IN-CHARACTE", "IN-CHARACTER"},
  {"IN-CHARACT", "IN-CHARACTER"},
  {"IN-CHARAC", "IN-CHARACTER"},
  {"IN-CHARA", "IN-CHARACTER"},
  {"IN-CHAR", "IN-CHARACTER"},
  {"IN-CHA", "IN-CHARACTER"},
  {"IN-CH", "IN-CHARACTER"},
  {"IN-C", "IN-CHARACTER"},
  {"IN", "IN-CHARACTER"},
  {"I-C", "IN-CHARACTER"},
  {"IC", "IN-CHARACTER"},
  {"ORG-OBJEC", "ORG-OBJECT"},
  {"ORG-OBJE", "ORG-OBJECT"},
  {"ORG-OBJ", "ORG-OBJECT"},
  {"ORG-OB", "ORG-OBJECT"},
  {"ORG-O", "ORG-OBJECT"},
  {"ORG", "ORG-OBJECT"},
  {"OR", "ORG-OBJECT"},
  {"O-O", "ORG-OBJECT"},
  {"OO", "ORG-OBJECT"},
  {"SPACE-OBJEC", "SPACE-OBJECT"},
  {"SPACE-OBJE", "SPACE-OBJECT"},
  {"SPACE-OBJ", "SPACE-OBJECT"},
  {"SPACE-OB", "SPACE-OBJECT"},
  {"SPACE-O", "SPACE-OBJECT"},
  {"SPACE", "SPACE-OBJECT"},
  {"SPAC", "SPACE-OBJECT"},
  {"SPA", "SPACE-OBJECT"},
  {"SP", "SPACE-OBJECT"},
  {"S-O", "SPACE-OBJECT"},
  {"SO", "SPACE-OBJECT"},
#endif /* TREKMUSH */

  {NULL, NULL}
};

/*   Name      Flag   */
POWER power_table[] =
{
  {"Announce", CAN_WALL},
  {"Boot", CAN_BOOT},
  {"Builder", CAN_BUILD},
  {"Cemit", CEMIT},
  {"Chat_Privs", CHAT_PRIVS},
  {"Functions", GLOBAL_FUNCS},
  {"Guest", IS_GUEST},
  {"Halt", HALT_ANYTHING},
  {"Hide", CAN_HIDE},
  {"Idle", UNLIMITED_IDLE},
  {"Immortal", NO_PAY | NO_QUOTA | UNKILLABLE},
  {"Login", LOGIN_ANYTIME},
  {"Long_Fingers", LONG_FINGERS},
  {"No_Pay", NO_PAY},
  {"No_Quota", NO_QUOTA},
  {"Open_Anywhere", OPEN_ANYWHERE},
  {"Pemit_All", PEMIT_ALL},
  {"Player_Create", CREATE_PLAYER},
  {"Poll", SET_POLL},
  {"Queue", HUGE_QUEUE},
  {"Quotas", CHANGE_QUOTAS},
  {"Search", SEARCH_ALL},
  {"See_All", SEE_ALL},
  {"See_Queue", PS_ALL},
  {"Tport_Anything", TEL_OTHER},
  {"Tport_Anywhere", TEL_ANYWHERE},
  {"Unkillable", UNKILLABLE},
  {NULL, 0}
};

static POWER_ALIAS power_alias_tab[] =
{
  {"@cemit", "Cemit"},
  {"@wall", "Announce"},
  {"wall", "Announce"},
  {NULL, NULL}
};




/*---------------------------------------------------------------------------
 * Flag hash table handlers 
 */


static FLAG *
flag_hash_lookup(name)
    const char *name;
{
  FLAG *t;

  t = (FLAG *) hashfind(strupper(name), &htab_flag);
  if (t)
    return t;

  /* provided for backwards compatibility: type flag checking */
  for (t = type_table; t->name != NULL; t++)
    if (string_prefix(name, t->name))
      return t;

  return NULL;
}

static void
flag_hash_insert(name, entry)
    const char *name;
    FLAG *entry;
{
  hashadd(name, (void *) entry, &htab_flag);
}

void
init_flag_hashtab()
{
  FLAG *f;
  FLAG_ALIAS *a;

  hashinit(&htab_flag, 128);

  /* do regular flags first */
  for (f = flag_table; f->name; f++)
    flag_hash_insert(f->name, f);

  /* now add in the aliases */
  for (a = flag_alias_tab; a->alias; a++) {
    if ((f = flag_hash_lookup(a->realname)) != NULL)
      flag_hash_insert(a->alias, (FLAG *) f);
    else
      fprintf(stderr,
	      "FLAG INIT: flag alias %s matches no known flag.\n",
	      a->alias);
  }
}

/*---------------------------------------------------------------------------
 * Other functions dealing with flags
 */

int
can_set_flag(player, thing, flagp, negate)
    dbref player;
    dbref thing;
    FLAG *flagp;
    int negate;
{
  /* returns 1 if player can set a flag on thing. */
  int myperms;

  if (!flagp || !GoodObject(player) || !GoodObject(thing))
    return 0;

  myperms = negate ? flagp->negate_perms : flagp->perms;

  if (myperms & F_INTERNAL)
    return 0;

  if ((flagp->type != NOTYPE) && (flagp->type != Typeof(thing)))
    return 0;

  if ((myperms & F_INHERIT) && !Wizard(player) &&
      (!Inheritable(player) || !Owns(player, thing)))
    return 0;

  /* You've got to *own* something (or be Wizard) to set it
   * chown_ok or dest_ok. This prevents subversion of the
   * zone-restriction on @chown and @dest
   */
  if (((flagp->flag == CHOWN_OK) && (flagp->type == NOTYPE)) ||
      ((flagp->flag == THING_DEST_OK) && (flagp->type == TYPE_THING))) {
    if (!Owns(player, thing) && !Wizard(player))
      return 0;
    else
      return 1;
  }
  /* Checking for the ZONE flag. If you set this, the player had
   * better be zone-locked! 
   */
  if ((flagp->type == TYPE_PLAYER) && (flagp->flag == PLAYER_ZONE) &&
      !negate && (getlock(thing, Zone_Lock) == TRUE_BOOLEXP)) {
    notify(player, "You must @lock/zone before you can set a player ZONE");
    return 0;
  }
  if (myperms & F_ANY)
    return 1;
  if ((myperms & F_WIZARD) && !Wizard(player))
    return 0;
  else if ((myperms & F_ROYAL) && !Hasprivs(player))
    return 0;
  else if ((myperms & F_GOD) && !God(player))
    return 0;

  /* special case for the privileged flags. We do need to check
   * for royalty setting flags on objects they don't own, because
   * the controls check will not stop the flag set if the royalty
   * player is in a zone. Also, only God can set the wizbit on
   * players.
   */
  if (Wizard(thing) && (flagp->flag == PLAYER_GAGGED) &&
      (flagp->type == TYPE_PLAYER))
    return 0;			/* can't gag wizards/God */

  if (God(player))		/* God can do (almost) anything) */
    return 1;

  /* Make sure we don't accidentally permission-check toggles when
   * checking priv bits.
   */
  if (flagp->type == NOTYPE) {

    /* A wiz can set things he owns WIZ, but can't reset his own bit. */
    if (flagp->flag == WIZARD)
      return (Wizard(player) && Owns(player, thing) && !IsPlayer(thing));
    /* Wizards can set or unset anything royalty. Royalty can set anything
     * they own royalty, but cannot reset their own bits. */
#ifdef ROYALTY_FLAG
    if (flagp->flag == ROYALTY) {
      return (!Guest(thing) && (Wizard(player) || (Royalty(player) &&
			      Owns(player, thing) && !IsPlayer(thing))));
    }
#endif
  }
  return 1;
}

const char *
unparse_flags(thing, player)
    dbref thing;
    dbref player;
{
  /* print out the flag symbols (letters) */

  static char buf[BUFFER_LEN];
  char *p;
  static const char type_codes[] = "R-EP--*";
  FLAG *f;
  int t;

  p = buf;
  t = Typeof(thing);
  if (t != TYPE_THING)
    *p++ = type_codes[t];

  /* get generic flags */
  for (f = flag_table; f->type == NOTYPE; f++) {
    if ((Flags(thing) & f->flag) && Can_See_Flag(player, thing, f))
      *p++ = f->letter;
  }

  /* exits have no special flags. This may change in the future */
  if (t == TYPE_EXIT || t == TYPE_GARBAGE) {
    *p = '\0';
    return buf;
  }
  /* go to beginning of list of flags for the type and print flags for
   * that type.
   */
  while (f->type != t)
    f++;
  for (; f->type == t; f++) {
    if ((Toggles(thing) & f->flag) && Can_See_Flag(player, thing, f))
      *p++ = f->letter;
  }

  *p = '\0';

  return buf;
}

const char *
flag_description(player, thing)
    dbref player;
    dbref thing;
{
  static char buf[BUFFER_LEN];
  char fbuf[BUFFER_LEN];
  char *bp;
  FLAG *f;
  int t;

  t = Typeof(thing);

  strcpy(buf, "Type: ");
  switch (t) {
  case TYPE_ROOM:
    strcat(buf, "Room");
    break;
  case TYPE_EXIT:
    strcat(buf, "Exit");
    break;
  case TYPE_THING:
    strcat(buf, "Thing");
    break;
  case TYPE_PLAYER:
    strcat(buf, "Player");
    break;
  default:
    strcat(buf, "***UNKNOWN TYPE***");
    break;
  }
  strcat(buf, " Flags:");

  bp = fbuf;

  /* get generic flags */
  for (f = flag_table; f->type == NOTYPE; f++) {
    if ((Flags(thing) & f->flag) && Can_See_Flag(player, thing, f)) {
      safe_chr(' ', fbuf, &bp);
      safe_str(f->name, fbuf, &bp);
    }
  }

  /* go to beginning of list of flags for the type and print flags for
   * that type.
   */
  while ((f->type != t) && (f->type != NOTYPE))
    f++;
  for (; f->type == t; f++) {
    if ((Toggles(thing) & f->flag) && Can_See_Flag(player, thing, f)) {
      safe_chr(' ', fbuf, &bp);
      safe_str(f->name, fbuf, &bp);
    }
  }

  *bp = '\0';

  /* no length checking needed. We're never going to have an overflow. */
  sprintf(buf, "%s%s", buf, fbuf);
  return buf;
}


/* Used to show the default toggle configuration from do_config_list */
const char *
togglemask_to_string(type, mask)
    int type;
    object_flag_type mask;
{
  static char fbuf[BUFFER_LEN];
  char *bp;
  FLAG *f;

  bp = fbuf;
  f = flag_table;
  while (f->type != type)
    f++;
  for (; f->type == type; f++) {
    if (mask & f->flag) {
      if (bp != fbuf)
	safe_chr(' ', fbuf, &bp);
      safe_str(f->name, fbuf, &bp);
    }
  }
  *bp = '\0';
  return fbuf;
}

#ifdef CAN_NEWSTYLE
static FLAG *
letter_to_flagptr(char c, int type, int *toggle)
#else
static FLAG *
letter_to_flagptr(c, type, toggle)
    char c;
    int type;
    int *toggle;
#endif
{
  /* convert letter to flag */

  FLAG *f;

  *toggle = 0;

  /* try generic flags */
  /* Doh! Kludge-city. We'll ignore the CHOWN_OK flag on players, because
   * it's more useful to check 'C' as COLOR. Argle.
   */
  for (f = flag_table; f->type == NOTYPE; f++) {
    if ((c == f->letter) &&
	((c != 'C') || (type != TYPE_PLAYER)))
      return (f);
  }

  /* try type-specific flags */

  *toggle = 1;
  while ((f->type != type) && (f->type != NOTYPE))
    f++;

  for (; f->type == type; f++) {
    if (c == f->letter)
      return (f);
  }
  return NULL;
}


object_flag_type
find_flag(name, type, toggle, is_conf)
    char *name;
    int type;
    int *toggle;
    int is_conf;
{
  /* given a flag name and an object type, return a flag, and set the
   * value of toggle to 0 if flag, 1 if toggle. We also check if it's
   * a legal flag to set, if it's a conf flag set.
   */

  FLAG *f;

  *toggle = 0;

  if ((f = flag_hash_lookup(upcasestr(name))) == NULL)
    return -1;

  if (is_conf && (f->perms == F_INTERNAL))
    return -1;

  if (f->type == NOTYPE) {
    return f->flag;
  } else {
    *toggle = 1;
    if (type != f->type) {
      if (is_conf)
	return -2;
      else
	return -1;
    } else {
      return f->flag;
    }
  }
  return f->flag;		/* NOTREACHED */
}

void
decompile_flags(player, thing, name)
    dbref player;
    dbref thing;
    const char *name;
{
  /* print out the flags for a decompile */

  FLAG *f;
  int t, ok;

  t = Typeof(thing);
  ok = !Suspect(thing);

  /* do generic flags */
  for (f = flag_table; f->type == NOTYPE; f++)
    if ((Flags(thing) & f->flag) && !(f->perms & F_INTERNAL) &&
	Can_See_Flag(player, thing, f))
      notify(player, tprintf("@set %s = %s", name, f->name));

  /* do normal flags */
  while ((f->type != t) && (f->type != NOTYPE))
    f++;
  for (; f->type == t; f++)
    if ((Toggles(thing) & f->flag) && Can_See_Flag(player, thing, f))
      notify(player, tprintf("@set %s = %s", name, f->name));
}


void
decompile_powers(player, thing, name)
    dbref player;
    dbref thing;
    const char *name;
{
  /* print out the powers for a decompile */

  POWER *p;

  for (p = power_table; p->name; p++) {
    /* Special case for immortal, which we don't show any more */
    if (!strcasecmp(p->name, "immortal"))
      continue;
    if (Powers(thing) & p->flag)
      notify(player, tprintf("@power %s = %s", name, p->name));
  }
}

int
convert_flags(player, s, p_mask, p_toggle, p_type)
    dbref player;
    char *s;
    object_flag_type *p_mask;
    object_flag_type *p_toggle;
    object_flag_type *p_type;
{
  /* convert flags for search */

  FLAG *f;
  object_flag_type mask, toggle, type;
  int done;
  mask = toggle = 0;
  type = NOTYPE;

  while (s && *s) {

    done = 0;

    switch (*s) {
    case 'P':
      type = TYPE_PLAYER;
      break;
    case 'R':
      type = TYPE_ROOM;
      break;
    case 'E':
      type = TYPE_EXIT;
      break;
    default:

      /* check generic flags first */
      for (f = flag_table; (f->type == NOTYPE) && !done; f++) {
	if (*s == f->letter) {
	  mask |= f->flag;
	  done = 1;
	}
      }

      /* try type-specific flags */

      /* if we have a type, start searching from that type */
      if (!done && (type != NOTYPE))
	while ((f->type != type) && (f->type != NOTYPE))
	  f++;

      for (; !done && (f->type != NOTYPE); f++) {
	if (*s == f->letter) {
	  /* if we don't have a type yet, we do now. If we do, we need
	   * to check for a conflict.
	   */
	  if (type == NOTYPE) {
	    type = f->type;
	    toggle |= f->flag;
	    done = 1;
	  } else if (type != f->type) {
	    notify(player, tprintf("Type conflict with flag '%c'.", *s));
	    return 0;
	  } else {
	    /* We've got a match */
	    toggle |= f->flag;
	    done = 1;
	  }
	}
      }

      /* if we get this far and still haven't found anything, error. */
      if (!done) {
	notify(player, tprintf("%c: unknown flag.", *s));
	return 0;
      }
    }
    s++;
  }

  *p_mask = mask;
  *p_toggle = toggle;
  *p_type = type;
  return 1;
}

static FLAG mon_table[] =
{
  {"MONITOR", 'M', TYPE_PLAYER, PLAYER_MONITOR, F_ROYAL, F_ROYAL},
  {"MONITOR", 'M', TYPE_THING, THING_LISTEN, F_ANY, F_ANY},
  {"MONITOR", 'M', TYPE_ROOM, ROOM_LISTEN, F_ANY, F_ANY},
  {NULL, '\0', 0, 0, 0, 0}
};

void
set_flag(player, thing, flag, negate, hear, listener)
    dbref player;
    dbref thing;
    char *flag;
    int negate;
    int hear;
    int listener;
{
  /* attempt to set a flag on an object */

  FLAG *f;
  char tbuf1[BUFFER_LEN];

  if ((f = flag_hash_lookup(strupper(flag))) == NULL) {
    notify(player, "I don't recognize that flag.");
    return;
  }
  /* HORRIBLE HACK: added to make MONITOR work. This needs to
   * be fixed in the future, _somehow_...
   */
  if (!strcmp(f->name, "MONITOR")) {
    for (f = mon_table; f->name != NULL; f++)
      if (Typeof(thing) == f->type)
	break;
    if (f->name == NULL) {
      notify(player, "Permission denied.");
      return;
    }
  }

  if (!can_set_flag(player, thing, f, negate)) {
    notify(player, "Permission denied.");
    return;
  }
  /* The only players who can be Dark are wizards. */
  if ((f->flag == DARK) && !negate && (f->type == NOTYPE) &&
      Alive(thing) && !Wizard(thing)) {
    notify(player, "Permission denied.");
    return;
  }
  if (negate) {

    if ((f->flag == GOING) && (f->type == NOTYPE)) {
      /* This is, frankly, a bit of a hack. */
      notify(player, "@set !GOING has been disabled. Use @undestroy instead.");
      return;
    }
    /* remove the flag */
    if (f->type == NOTYPE)
      Flags(thing) &= ~(f->flag);
    else
      Toggles(thing) &= ~(f->flag);

    /* log if necessary */
    if ((f->flag == WIZARD) && (f->type == NOTYPE))
      do_log(LT_WIZ, player, thing, "WIZFLAG RESET");
#ifdef ROYALTY_FLAG
    else if ((f->flag == ROYALTY) && (f->type == NOTYPE))
      do_log(LT_WIZ, player, thing, "ROYAL FLAG RESET");
#endif
    else if ((f->flag == PLAYER_SUSPECT) && (f->type == TYPE_PLAYER))
      do_log(LT_WIZ, player, thing, "SUSPECT FLAG RESET");

    /* those who unDARK return to the WHO */
    if ((f->flag == DARK) && (f->type == NOTYPE) && (IsPlayer(thing)))
      hide_player(thing, 0);

    /* notify the area if something stops listening */
    if (IsThing(thing) && (f->type == TYPE_THING) &&
	(((f->flag == THING_PUPPET) && !listener && !Hearer(thing)) ||
	 ((f->flag == THING_LISTEN) && !hear && !Listener(thing)))) {
      sprintf(tbuf1, "%s is no longer listening.", Name(thing));
      notify_except(Contents(Location(thing)), NOTHING, tbuf1);
      notify_except(Contents(thing), NOTHING, tbuf1);
    }
    if (IsRoom(thing) && (f->type == TYPE_ROOM) &&
	(f->flag == ROOM_LISTEN) && !hear && !Listener(thing)) {
      sprintf(tbuf1, "%s is no longer listening.", Name(thing));
      notify_except(Contents(thing), NOTHING, tbuf1);
    }
    if ((f->flag == AUDIBLE) && (f->type == NOTYPE)) {
      switch (Typeof(thing)) {
      case TYPE_EXIT:
	if (Audible(Source(thing))) {
	  sprintf(tbuf1, "Exit %s is no longer broadcasting.", Name(thing));
	  notify_except(Contents(Source(thing)), NOTHING, tbuf1);
	}
	break;
      case TYPE_ROOM:
	notify_except(Contents(thing), NOTHING,
		    "Audible exits in this room have been deactivated.");
	break;
      case TYPE_THING:
      case TYPE_PLAYER:
	notify_except(Contents(thing), thing,
		      "This room is no longer broadcasting.");
	notify(thing, "Your contents can no longer be heard from outside.");
	break;
      }
    }
    if (((f->flag == QUIET) && (f->type == NOTYPE)) ||
	(!AreQuiet(player, thing)))
      notify(player, "Flag reset.");
  } else {

    /* set the flag */
    if (f->type == NOTYPE)
      Flags(thing) |= f->flag;
    else
      Toggles(thing) |= f->flag;

    /* log if necessary */
    if ((f->flag == WIZARD) && (f->type == NOTYPE))
      do_log(LT_WIZ, player, thing, "WIZFLAG SET");
#ifdef ROYALTY_FLAG
    if ((f->flag == ROYALTY) && (f->type == NOTYPE))
      do_log(LT_WIZ, player, thing, "ROYAL FLAG SET");
#endif
    else if ((f->flag == PLAYER_SUSPECT) && (f->type == TYPE_PLAYER))
      do_log(LT_WIZ, player, thing, "SUSPECT FLAG SET");

    if ((f->type == NOTYPE) && (f->flag == INHERIT) && GoodObject(Zone(thing)))
      notify(player, "Warning: Setting inherit flag on zoned object");
    /* DARK players should be treated as logged out */
    if ((f->flag == DARK) && (f->type == NOTYPE) && (IsPlayer(thing)))
      hide_player(thing, 1);

    /* notify area if something starts listening */
    if (IsThing(thing) && (f->type == TYPE_THING) &&
	((f->flag == THING_PUPPET) || (f->flag == THING_LISTEN)) &&
	!hear && !listener) {
      sprintf(tbuf1, "%s is now listening.", Name(thing));
      notify_except(Contents(Location(thing)), NOTHING, tbuf1);
      notify_except(Contents(thing), NOTHING, tbuf1);
    }
    if (IsRoom(thing) && (f->type == TYPE_ROOM) &&
	(f->flag == ROOM_LISTEN) && !hear && !listener) {
      sprintf(tbuf1, "%s is now listening.", Name(thing));
      notify_except(Contents(thing), NOTHING, tbuf1);
    }
    /* notify for audible exits */
    if ((f->flag == AUDIBLE) && (f->type == NOTYPE)) {
      switch (Typeof(thing)) {
      case TYPE_EXIT:
	if (Audible(Source(thing))) {
	  sprintf(tbuf1, "Exit %s is now broadcasting.", Name(thing));
	  notify_except(Contents(Source(thing)), NOTHING, tbuf1);
	}
	break;
      case TYPE_ROOM:
	notify_except(Contents(thing), NOTHING,
		      "Audible exits in this room have been activated.");
	break;
      case TYPE_PLAYER:
      case TYPE_THING:
	notify_except(Contents(thing), thing,
		      "This room is now broadcasting.");
	notify(thing, "Your contents can now be heard from outside.");
	break;
      }
    }
    if (((f->flag == QUIET) && (f->flag == NOTYPE)) ||
	(!AreQuiet(player, thing)))
      notify(player, "Flag set.");
  }
}

int
handle_flaglists(player, name, fstr, type)
    dbref player;
    char *name;
    char *fstr;
    int type;



				/* 0 for orflags, 1 for andflags */
{
  char *s;
  FLAG *fp;
  int toggle, negate, temp;
  int ret = type;
  dbref it = match_thing(player, name);

  toggle = negate = temp = 0;

  if (it == NOTHING)
    return 0;

  for (s = fstr; *s; s++) {

    /* Check for a negation sign. If we find it, we note it and 
     * increment the pointer to the next character.
     */

    if (*s == '!') {
      negate = 1;
      s++;
    } else {
      negate = 0;		/* It's important to clear this at appropriate times;
				 * else !Dc means (!D && !c), instead of (!D && c). */
    }

    if (!*s)
      /* We got a '!' that wasn't followed by a letter.
       * Fail the check. */
      return (type == 1) ? 0 : ret;

    /* Find the flag. */
    if ((fp = letter_to_flagptr(*s, Typeof(it), &toggle)) == NULL) {

      /* Either we got a '!' that wasn't followed by a letter, or
       * we couldn't find that flag. For AND, since we've failed
       * a check, we can return false. Otherwise we just go on.
       */

      if (type == 1)
	return 0;
      else
	continue;

    } else {

      /* does the object have this flag? */

      if ((!toggle && (Flags(it) & fp->flag)) ||
	  (toggle && (Toggles(it) & fp->flag) &&
	   Can_See_Flag(player, it, fp)))
	temp = 1;
      else
	temp = 0;

      if ((type == 1) && ((negate && temp) || (!negate && !temp))) {

	/* Too bad there's no NXOR function...
	 * At this point we've either got a flag and we don't want
	 * it, or we don't have a flag and we want it. Since it's
	 * AND, we return false.
	 */
	return 0;

      } else if ((type == 0) &&
		 ((!negate && temp) || (negate && !temp))) {

	/* We've found something we want, in an OR. We OR a
	 * true with the current value.
	 */

	ret |= 1;
      }
      /* Otherwise, we don't need to do anything. */
    }
  }
  return (ret);
}


int
sees_flag(privs, thing, name)
    dbref privs;
    dbref thing;
    char *name;
{
  /* Does thing have the flag named name && can privs see it? */
  FLAG *f;
  int retval;
  if ((f = flag_hash_lookup(upcasestr(name))) == NULL)
    return 0;

  /* HORRIBLE HACK: added to make MONITOR work. This needs to
   * be fixed in the future, _somehow_...
   */
  if (!strcmp(f->name, "MONITOR")) {
    for (f = mon_table; f->name != NULL; f++)
      if (Typeof(thing) == f->type)
	break;
    if (f->name == NULL) {
      return 0;
    }
    retval = Toggles(thing) & f->flag;
  } else if (f->type == NOTYPE) {
    retval = Flags(thing) & f->flag;
  } else {
    if (Typeof(thing) != f->type)
      return 0;
    else
      retval = Toggles(thing) & f->flag;
  }
  return retval && Can_See_Flag(privs, thing, f);
}


const char *
power_description(thing)
    dbref thing;
{
  static char fbuf[BUFFER_LEN];
  char *bp;
  POWER *p;

  bp = fbuf;

  for (p = power_table; p->name; p++) {
    /* Special case for immortal, which we don't show any more */
    if (!strcasecmp(p->name, "immortal"))
      continue;
    if (Powers(thing) & p->flag) {
      if (bp != fbuf)
	safe_chr(' ', fbuf, &bp);
      safe_str(p->name, fbuf, &bp);
    }
  }
  *bp = '\0';

  return fbuf;
}

object_flag_type
find_power(name)
    const char *name;
{
  POWER *p;
  POWER_ALIAS *a;

  for (p = power_table; p->name; p++) {
    if (string_prefix(p->name, name))
      return p->flag;
  }

  /* Check the alias table */
  for (a = power_alias_tab; a->alias; a++) {
    if (string_prefix(a->alias, name))
      return find_power(a->realname);
  }

  /* Got nothing. Return -1 */
  return -1;
}
