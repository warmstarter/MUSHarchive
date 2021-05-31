/* player.c */

#include "copyrite.h"
#include "config.h"
#include <stdio.h>
#ifdef I_UNISTD
#include <unistd.h>
#endif
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#include <fcntl.h>

#include "conf.h"
#include "mushdb.h"
#include "intrface.h"
#include "externs.h"
#include "access.h"
#include "confmagic.h"


extern time_t mudtime;
extern int reserved;
extern void add_player _((dbref player, char *alias));

int password_check _((dbref player, const char *password));
dbref connect_player _((const char *name, const char *password, const char *host, const char *ip));
dbref create_player _((const char *name, const char *password, const char *host, const char *ip));
dbref email_register_player _((const char *name, const char *email, const char *host, const char *ip));
static dbref make_player _((const char *name, const char *password, const char *host, const char *ip));
void do_password _((dbref player, const char *old, const char *newobj));
void check_last _((dbref player, const char *host, const char *ip));
void check_lastfailed _((dbref player, const char *host));

int
password_check(player, password)
    dbref player;
    const char *password;
{
  ATTR *a;

  /* read the password and compare it */
  if ((a = atr_get_noparent(player, "XYXXY")) &&
      strcmp(uncompress(a->value), password) &&
      strcmp(mush_crypt(password), uncompress(a->value)))
    return 0;

#if (CRYPT_SYSTEM > 0)
  /* prevent direct entry of the raw encrypted password */
  if ((strlen(password) == 13) &&
      (password[0] == 'X') && (password[1] == 'X'))
    return 0;
#endif

  /* we're okay */
  return 1;
}

dbref
connect_player(name, password, host, ip)
    const char *name;
    const char *password;
    const char *host;
    const char *ip;
{
  dbref player;
  dbref i;

  /* validate name */
  if ((player = lookup_player(name)) == NOTHING)
    return NOTHING;

  /* See if player is allowed to connect like this */
  if (Guest(player) && (!Site_Can_Guest(host) || !Site_Can_Guest(ip))) {
    if (!Deny_Silent_Site(host) && !Deny_Silent_Site(ip)) {
      do_log(LT_CONN, 0, 0, "Connection to %s (GUEST) not allowed from %s (%s)",
	     name, host, ip);
    }
    return NOTHING;
  } else if (!Guest(player) && (!Site_Can_Connect(host) || !Site_Can_Connect(ip))) {
    if (!Deny_Silent_Site(host) && !Deny_Silent_Site(ip)) {
      do_log(LT_CONN, 0, 0, "Connection to %s (Non-GUEST) not allowed from %s (%s)",
	     name, host, ip);
    }
    return NOTHING;
  }
  /* validate password */
  if (!Guest(player))
    if (!password_check(player, password)) {
#ifdef CREATION_TIMES
      /* Increment count of login failures */
      ModTime(player)++;
#endif
      check_lastfailed(player, host);
      return NOTHING;
    }
  /* If it's a Guest player, and already connected, search the
   * db for another Guest player to connect them to. */
  if (Guest(player) && Connected(player)) {
    /* Search db for an unconnected Guest */
    for (i = 0; i < db_top; i++) {
      if (IsPlayer(i) && !Hasprivs(i) &&
	  Guest(i) && !Connected(i)) {
	player = i;
	break;
      }
    }
    if (i == db_top) {
      /* We failed to find an unconnected Guest */
      do_log(LT_CONN, 0, 0, "Multiple connection to Guest #%d", player);
    }
  }
  if (Suspect_Site(host) || Suspect_Site(ip)) {
    do_log(LT_CONN, 0, 0, "Connection from Suspect site. Setting %s(#%d) suspect.", Name(player), player);
    Toggles(player) |= PLAYER_SUSPECT;
  }
  return player;
}

/* This function returns NOTHING if the player name was bad, and
 * AMBIGUOUS if the player password was bad
 */
dbref
create_player(name, password, host, ip)
    const char *name;
    const char *password;
    const char *host;
    const char *ip;
{
  if (!ok_player_name(name)) {
    do_log(LT_CONN, 0, 0, "Failed creation (bad name) from %s", host);
    return NOTHING;
  }
  if (!ok_password(password)) {
    do_log(LT_CONN, 0, 0, "Failed creation (bad password) from %s", host);
    return AMBIGUOUS;
  }
  /* else he doesn't already exist, create him */
  return make_player(name, password, host, ip);
}

/* The HAS_SENDMAIL ifdef is kept here as a hint to metaconfig */
#ifdef MAILER
#undef HAS_SENDMAIL
#define HAS_SENDMAIL 1
#undef SENDMAIL
#define SENDMAIL MAILER
#endif

#ifdef HAS_SENDMAIL
dbref
email_register_player(name, email, host, ip)
    const char *name;
    const char *email;
    const char *host;
    const char *ip;
{
  char *p;
  char passwd[BUFFER_LEN];
  static char elems[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  int i, len;
  dbref player;
  FILE *fp;

  if (!ok_player_name(name)) {
    do_log(LT_CONN, 0, 0, "Failed registration (bad name) from %s", host);
    return NOTHING;
  }
  /* Make sure that the email address is valid. A valid address must
   * contain either an @ or a !
   * Also, to prevent someone from using the MUSH to mailbomb another site,
   * let's make sure that the site to which the user wants the email
   * sent is also allowed to use the register command.
   * If there's an @, we check whatever's after the last @
   * (since @foo.bar:user@host is a valid email)
   * If not, we check whatever comes before the first !
   */
  if ((p = strrchr(email, '@'))) {
    p++;
    if (!Site_Can_Register(p)) {
      if (!Deny_Silent_Site(p)) {
	do_log(LT_CONN, 0, 0, "Failed registration (bad site in email: %s) from %s", email, host);
      }
      return NOTHING;
    }
  } else if ((p = strchr(email, '!'))) {
    *p = '\0';
    if (!Site_Can_Register(email)) {
      *p = '!';
      if (!Deny_Silent_Site(email)) {
	do_log(LT_CONN, 0, 0, "Failed registration (bad site in email: %s) from %s", email, host);
      }
      return NOTHING;
    } else
      *p = '!';
  } else {
    if (!Deny_Silent_Site(host)) {
      do_log(LT_CONN, 0, 0, "Failed registration (bad email: %s) from %s", email, host);
    }
    return NOTHING;
  }


  /* Come up with a random password of length 7-12 chars */
  len = 7 + getrandom(6);
  for (i = 0; i < len; i++)
    passwd[i] = elems[getrandom(strlen(elems))];
  passwd[len] = '\0';

  /* If we've made it here, we can send the email and create the
   * character. Email first, since that's more likely to go bad.
   * Some security precautions we'll take:
   *  1) We'll use sendmail -t, so we don't pass user-given values to a shell.
   */

#ifndef WIN32
  close(reserved);
#endif
  if ((fp = popen(tprintf("%s -t", SENDMAIL), "w")) == NULL) {
    do_log(LT_CONN, 0, 0, "Failed registration of %s by %s: unable to open sendmail", name, email);
#ifndef WIN32
    reserved = open("/dev/null", O_RDWR);
#endif
    return NOTHING;
  }
  fprintf(fp, "Subject: [%s] Registration of %s\n", MUDNAME, name);
  fprintf(fp, "To: %s\n", email);
  fprintf(fp, "Precedence: junk\n");
  fprintf(fp, "\n");
  fprintf(fp, "This is an automated message.\n");
  fprintf(fp, "\n");
  fprintf(fp, "Your requested player, %s, has been created.\n", name);
  fprintf(fp, "The password is %s\n", passwd);
  fprintf(fp, "\n");
  fprintf(fp, "To access this character, connect to %s and type:\n", MUDNAME);
  fprintf(fp, "\tconnect %s %s\n", name, passwd);
  fprintf(fp, "\n");
  pclose(fp);
#ifndef WIN32
  reserved = open("/dev/null", O_RDWR);
#endif
  /* Ok, all's well, make a player */
  player = make_player(name, passwd, host, ip);
#ifdef TREKMUSH
  (void) atr_add(player, "EMAIL", email, GOD, NOTHING);
#else /* TREKMUSH */
  (void) atr_add(player, "REGISTERED_EMAIL", email, GOD, NOTHING);
#endif /* TREKMUSH */
  return player;
}
#else
dbref
email_register_player(name, email, host, ip)
    const char *name;
    const char *email;
    const char *host;
    const char *ip;
{
  do_log(LT_CONN, 0, 0, "Failed registration (no sendmail) from %s", host);
  return NOTHING;
}
#endif

static dbref
make_player(name, password, host, ip)
    const char *name;
    const char *password;
    const char *host;
    const char *ip;
{

  dbref player;
  char *s;
#ifdef QUOTA
  char temp[SBUF_LEN];
#endif

  /* else he doesn't already exist, create him */
  s = ctime(&mudtime);
  s[strlen(s) - 1] = '\0';
  if (s[8] == ' ')
    s[8] = '0';
  player = new_object();

  /* initialize everything */
  SET(Name(player), name);
  Location(player) = PLAYER_START;
  Home(player) = PLAYER_START;
  Owner(player) = player;
  Parent(player) = NOTHING;
  Flags(player) = TYPE_PLAYER;
  Flags(player) |= options.player_flags;
  Toggles(player) |= options.player_toggles;
  if (Suspect_Site(host) || Suspect_Site(ip))
    Toggles(player) |= PLAYER_SUSPECT;
#ifdef USE_WARNINGS
  set_initial_warnings(player);
#endif
#ifdef CREATION_TIMES
  /* Modtime tracks login failures */
  ModTime(player) = (time_t) 0;
#endif
  atr_add(player, "XYXXY", mush_crypt(password), GOD, NOTHING);
  giveto(player, START_BONUS);	/* starting bonus */
  (void) atr_add(player, "LAST", s, GOD, NOTHING);
  (void) atr_add(player, "LASTSITE", host, GOD, NOTHING);
  (void) atr_add(player, "LASTIP", host, GOD, NOTHING);
  (void) atr_add(player, "LASTFAILED", " ", GOD, NOTHING);
#ifdef QUOTA
  sprintf(temp, "%d", START_QUOTA);
  (void) atr_add(player, "RQUOTA", temp, GOD, NOTHING);
#endif				/* QUOTA */
#ifdef FIXED_FLAG
#ifndef EMPTY_ATTRS
  (void) atr_add(player, "ICLOC", " ", GOD, AF_MDARK | AF_PRIVATE | AF_WIZARD | AF_NOCOPY);
#else
  (void) atr_add(player, "ICLOC", "", GOD, AF_MDARK | AF_PRIVATE | AF_WIZARD | AF_NOCOPY);
#endif
#endif
#ifdef USE_MAILER
  (void) atr_add(player, "MAILCURF", "0", GOD, AF_LOCKED | AF_NOPROG | AF_WIZARD);
  add_folder_name(player, 0, "inbox");
#endif
#ifdef TREKMUSH /* TrekMUSH Patch */
  (void) atr_add(player, "MONEY", "1", GOD, AF_LOCKED | AF_WIZARD | AF_ODARK | AF_NOPROG);
  (void) atr_add(player, "EXPERIENCE", "15", GOD, AF_LOCKED | AF_WIZARD | AF_MDARK | AF_NOPROG);
#endif /* TrekMUSH Patch */
  /* link him to PLAYER_START */
  PUSH(player, Contents(PLAYER_START));

  add_player(player, NULL);
  add_lock(player, Basic_Lock, parse_boolexp(player, "=me"));
  add_lock(player, Enter_Lock, parse_boolexp(player, "=me"));
  add_lock(player, Use_Lock, parse_boolexp(player, "=me"));

#ifdef LOCAL_DATA
  local_data_create(player);
#endif

  return player;
}


void
do_password(player, old, newobj)
    dbref player;
    const char *old;
    const char *newobj;
{

  if (Guest(player)) {
    notify(player, "Guests may not change their passwords.");
    return;
  }
  if (!password_check(player, old)) {
    notify(player, "Sorry");
  } else if (!ok_password(newobj)) {
    notify(player, "Bad new password.");
  } else {
    atr_add(player, "XYXXY", mush_crypt(newobj), GOD, NOTHING);
    notify(player, "Password changed.");
  }
}

void
check_last(player, host, ip)
    dbref player;
    const char *host;
    const char *ip;
{
  char *s;
  ATTR *a;
  ATTR *h;
  char last_time[MAX_COMMAND_LEN / 8];
  char last_place[MAX_COMMAND_LEN];

  s = ctime(&mudtime);
  s[strlen(s) - 1] = '\0';
  if (s[8] == ' ')
    s[8] = '0';
  /* compare to last connect see if player gets salary */
  a = atr_get_noparent(player, "LAST");
  if (!Guest(player) && a && (strncmp(uncompress(a->value), s, 10) != 0))
    giveto(player, PAY_CHECK);
  /* tell the player where he last connected from */
  h = atr_get_noparent(player, "LASTSITE");
  if (h && a) {
    strcpy(last_place, uncompress(h->value));
    strcpy(last_time, uncompress(a->value));
    notify(player, tprintf("Last connect was from %s on %s.",
			   last_place, last_time));
  }
  /* How about last failed connection */
  h = atr_get_noparent(player, "LASTFAILED");
  if (h && a) {
    strcpy(last_place, uncompress(h->value));
    if (strlen(last_place) > 2)
      notify(player, tprintf("Last FAILED connect was from %s.",
			     last_place));
  }
  /* if there is no Lastsite, then the player is newly created.
   * the extra variables are a kludge to work around some weird
   * behavior involving uncompress.
   */

  /* set the new attributes */
  (void) atr_add(player, "LAST", s, GOD, NOTHING);
  (void) atr_add(player, "LASTSITE", host, GOD, NOTHING);
  (void) atr_add(player, "LASTIP", ip, GOD, NOTHING);
  (void) atr_add(player, "LASTFAILED", " ", GOD, NOTHING);
}


/* This is called on a failed connection */
void
check_lastfailed(player, host)
    dbref player;
    const char *host;
{
  char *s;
  char last_place[MAX_COMMAND_LEN];

  s = ctime(&mudtime);
  s[strlen(s) - 1] = 0;
  sprintf(last_place, "%s on %s", host, s);
  (void) atr_add(player, "LASTFAILED", last_place, GOD, NOTHING);
}
