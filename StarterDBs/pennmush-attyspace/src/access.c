
/*
   * The file access.cnf in the game directory will control all 
   access-related directives, replacing lockout.cnf and sites.cnf

   * The format of entries in the file will be:

   wild-host-name    [!]option [!]option [!]option ... # comment

   A wild-host-name is a wildcard pattern to match hostnames with. 
   The wildcard "*" will work like UNIX filename globbing, so
   *.edu will match all sites with names ending in .edu, and
   *.*.*.*.* will match all sites with 4 periods in their name.
   128.32.*.* will match all sites starting with 128.32 (UC Berkeley).
   You can also use user@host to match specific users if you know that
   the host is running ident and you trust its responses (nontrivial).

   The options that can be specified are:
   *CONNECT              Allow connections to non-guest players
   *GUEST                Allow connection to guests
   *CREATE               Allow player creation at login screen
   DEFAULT               All of the above
   NONE                 None of the above
   SUSPECT              Set all players connecting from the site suspect
   REGISTER             Allow players to use the "register" connect command
   DENY_SILENT          Don't log when someone's denied access from here
   REGEXP               Treat the hostname pattern as a regular expression

   Options that are *'d can be prefaced by a !, meaning "Don't allow".

   * The file is parsed line-by-line in order. This makes it possible
   to explicitly allow only certain sites to connect and deny all others,
   or vice versa. Sites can only do the options that are specified
   in the first line they match.

   * If a site is listed in the file with no options at all, it is
   disallowed from any access (treated as !CONNECT, basically)

   * If a site doesn't match any line in the file, it is allowed any
   toggleable access (treated as DEFAULT) but isn't SUSPECT or REGISTER.

   * "make access" produces access.cnf from lockout.cnf/sites.cnf

   * @sitelock'd sites appear after the line "@sitelock" in the file
   Using @sitelock writes out the file.

 */

#include "config.h"
#include "copyrite.h"
#include <stdio.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#ifdef I_SYS_TYPES
#include <sys/types.h>
#endif
#include <fcntl.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifdef I_UNISTD
#include <unistd.h>
#endif
#include "conf.h"
#include "externs.h"
#ifdef MEM_CHECK
#include "memcheck.h"
#endif
#include "myregexp.h"
#include "access.h"
#include "mymalloc.h"
#include "confmagic.h"

/* A linked list data structure to hold the access info */
struct access {
  char host[BUFFER_LEN];
  char comment[BUFFER_LEN];
  int can;
  int cant;
  struct access *next;
};


typedef struct a_acsflag acsflag;
struct a_acsflag {
  const char *name;
  int toggle;			/* Is this a negatable flag? */
  int flag;
};
static acsflag acslist[] =
{
  {"connect", 1, ACS_CONNECT},
  {"create", 1, ACS_CREATE},
  {"guest", 1, ACS_GUEST},
  {"default", 0, ACS_DEFAULT},
  {"register", 0, ACS_REGISTER},
  {"suspect", 0, ACS_SUSPECT},
  {"deny_silent", 0, ACS_DENY_SILENT},
  {"regexp", 0, ACS_REGEXP},
  {NULL, 0, 0}
};

static struct access *access_top;
extern int reserved;		/* reserved file descriptor */
extern time_t mudtime;
static int add_access_node _((const char *host, const int can, const int cant, const char *comment));
static void free_access_list _((void));

/* Allocate a new node and add to the end of the list. */
static int
add_access_node(host, can, cant, comment)
    const char *host;
    const int can;
    const int cant;
    const char *comment;
{
  struct access *end;
  struct access *tmp;

  tmp = (struct access *) mush_malloc(sizeof(struct access), "struct_access");
  if (!tmp)
    return 0;
  tmp->can = can;
  tmp->cant = cant;
  strcpy(tmp->host, host);
  if (comment)
    strcpy(tmp->comment, comment);
  else
    strcpy(tmp->comment, "");
  tmp->next = NULL;

  if (!access_top) {
    /* Add to the beginning */
    access_top = tmp;
  } else {
    end = access_top;
    while (end->next)
      end = end->next;
    end->next = tmp;
  }

  return 1;
}


/* Initialize the linked list and read in the access file.
 * Return 1 if successful, 0 if not
 */
int
read_access_file()
{
  FILE *fp;
  char buf[BUFFER_LEN];
  char *p;
  int can, cant;
  int retval;
  char *comment;

  if (access_top) {
    /* We're reloading the file, so we've got to delete any current 
     * entries
     */
    free_access_list();
  }
  access_top = NULL;
  /* Be sure we have a file descriptor */
#ifndef WIN32
  close(reserved);
#endif
  fp = fopen(ACCESS_FILE, "r");
  if (!fp) {
    do_log(LT_ERR, GOD, GOD, "No %s file found.", ACCESS_FILE);
    retval = 0;
  } else {
    fgets(buf, BUFFER_LEN, fp);
    while (!feof(fp)) {
      /* Strip newline */
      if ((p = strchr(buf, '\n')))
	*p = '\0';
      /* Find beginning of line; ignore blank lines */
      p = buf;
      if (*p && isspace(*p))
	p++;
      if (*p && *p != '#') {
	can = cant = 0;
	comment = NULL;
	/* Is this the @sitelock entry? */
	if (!strncasecmp(p, "@sitelock", 9)) {
	  can = ACS_SITELOCK;
	  buf[9] = '\0';
	} else {
	  if ((comment = strchr(p, '#'))) {
	    *comment++ = '\0';
	    while (*comment && isspace(*comment))
	      comment++;
	  }
	  /* Move past the host name */
	  while (*p && !isspace(*p))
	    p++;
	  if (*p)
	    *p++ = '\0';
	  if (!parse_access_options(p, &can, &cant, NOTHING))
	    /* Nothing listed, so assume we can't do anything! */
	    cant = ACS_DEFAULT;

	}
	if (!add_access_node(buf, can, cant, comment)) {
	  /* Something very bad happened */
	  do_log(LT_ERR, GOD, GOD, "Failed to add access node!");
	  fclose(fp);
	  retval = 0;
	}
      }
      fgets(buf, BUFFER_LEN, fp);
    }
    retval = 1;
    fclose(fp);
  }
#ifndef WIN32
  reserved = open("/dev/null", O_RDWR);
#endif
  return retval;
}

/* Write out the access file from the linked list */
void
write_access_file()
{
  FILE *fp;
  char tmpf[BUFFER_LEN];
  struct access *ap;
  acsflag *c;

  sprintf(tmpf, "%s.tmp", ACCESS_FILE);
  /* Be sure we have a file descriptor */
#ifndef WIN32
  close(reserved);
#endif
  fp = fopen(tmpf, "w");
  if (!fp) {
    do_log(LT_ERR, GOD, GOD, "Unable to open %s.", tmpf);
  } else {
    for (ap = access_top; ap; ap = ap->next) {
      fprintf(fp, "%s ", ap->host);
      switch (ap->can) {
      case ACS_SITELOCK:
	break;
      case ACS_DEFAULT:
	fprintf(fp, "DEFAULT ");
	break;
      default:
	for (c = acslist; c->name; c++)
	  if (ap->can & c->flag)
	    fprintf(fp, "%s ", c->name);
	break;
      }
      switch (ap->cant) {
      case ACS_DEFAULT:
	fprintf(fp, "NONE ");
	break;
      default:
	for (c = acslist; c->name; c++)
	  if (c->toggle && (ap->cant & c->flag))
	    fprintf(fp, "!%s ", c->name);
	break;
      }
      if (ap->comment && *ap->comment)
	fprintf(fp, "# %s\n", ap->comment);
      else
	fprintf(fp, "\n");
    }
    fclose(fp);
#ifdef WIN32
    unlink(ACCESS_FILE);
#endif
    rename(tmpf, ACCESS_FILE);
  }
#ifndef WIN32
  reserved = open("/dev/null", O_RDWR);
#endif
  return;
}

/* Given a hostname and a flag decide if the host can do it.
 * Here's how it works:
 * We run the linked list and take the first match.
 *  (If the hostname is user@host, we try to match both user@host
 *   and just host to each line in the file.)
 * If we make a match, and the line tells us whether the site can/can't
 *   do the action, we're done.
 * Otherwise, we assume that the host can do any toggleable option
 *   (can create, connect, guest), and don't have any special
 *   flags (can't register, isn't suspect)
 */
int
site_can_access(hname, flag)
    const char *hname;
    const int flag;
{
  struct access *ap;
  acsflag *c;
  char *p;
  regexp *re = NULL;

  if (!hname || !*hname)
    return 0;

  if ((p = strchr(hname, '@')))
    p++;

  for (ap = access_top; ap; ap = ap->next) {
    if (ap->can & ACS_REGEXP) {
      if (re != NULL)
	mush_free(re, "regexp"); 
      re = regcomp(ap->host);
      if (re == NULL) /* Compile error. Continue with the next ap.  */
	continue;
    }

    if (!(ap->can & ACS_SITELOCK)
	&& ((ap->can & ACS_REGEXP)
	    ? (regexec(re, (char *)hname)
	       || (p && regexec(re, p)))
	    : (quick_wild(ap->host, hname)
	       || (p && quick_wild(ap->host, p))))) {
      /* Got one */
      if (ap->cant && ((ap->cant & flag) == flag))
	return 0;
      if (ap->can && (ap->can & flag))
	return 1;
      /* Hmm. We don't know if we can or not, so continue */
      break;
    }
  }
  if (re != NULL)
    mush_free(re, "regexp");

  /* Flag was neither set nor unset. If the flag was a toggle,
   * then the host can do it. If not, the host can't */
  for (c = acslist; c->name; c++) {
    if (flag & c->flag)
      return c->toggle ? 1 : 0;
  }
  /* Should never reach here, but just in case */
  return 1;
}

/* Add an entry to the linked list after the @sitelock entry.
 * If there is no @sitelock entry, add one to the end of the list
 * and then add the entry.
 * Build an appropriate comment based on the player and date
 */
int
add_access_sitelock(player, host, can, cant)
    dbref player;
    const char *host;
    const int can;
    const int cant;
{
  struct access *end;
  struct access *tmp;
  char *date;

  tmp = (struct access *) mush_malloc(sizeof(struct access), "struct_access");
  if (!tmp)
    return 0;
  tmp->can = can;
  tmp->cant = cant;
  strcpy(tmp->host, host);
  date = (char *) ctime(&mudtime);
  date[strlen(date) - 1] = '\0';
  sprintf(tmp->comment, "By %s(#%d) on %s", Name(player), player, date);
  tmp->next = NULL;

  if (!access_top) {
    /* Add to the beginning, but first add a sitelock marker */
    if (!add_access_node("@sitelock", ACS_SITELOCK, 0, ""))
      return 0;
    access_top->next = tmp;
  } else {
    end = access_top;
    while (end->next && end->can != ACS_SITELOCK)
      end = end->next;
    /* Now, either we're at the sitelock or the end */
    if (end->can != ACS_SITELOCK) {
      /* We're at the end and there's no sitelock marker. Add one */
      if (!add_access_node("@sitelock", ACS_SITELOCK, 0, ""))
	return 0;
      end = end->next;
    } else {
      /* We're in the middle, so be sure we keep the list linked */
      tmp->next = end->next;
    }
    end->next = tmp;
  }
  return 1;
}

/* Free the entire access list */
static void
free_access_list()
{
  struct access *ap, *next;
  ap = access_top;
  while (ap) {
    next = ap->next;
    mush_free((Malloc_t) ap, "struct_access");
    ap = next;
  }
  access_top = NULL;
}


/* Dump the access list for the player */
void
do_list_access(player)
    dbref player;
{
  struct access *ap;
  acsflag *c;
  char flaglist[BUFFER_LEN];
  char *bp;

  for (ap = access_top; ap; ap = ap->next) {
    if (ap->can != ACS_SITELOCK) {
      bp = flaglist;
      for (c = acslist; c->name; c++) {
	if (c->flag == ACS_DEFAULT)
	  continue;
	if (ap->can & c->flag) {
	  safe_chr(' ', flaglist, &bp);
	  safe_str(c->name, flaglist, &bp);
	}
	if (c->toggle && (ap->cant & c->flag)) {
	  safe_chr(' ', flaglist, &bp);
	  safe_chr('!', flaglist, &bp);
	  safe_str(c->name, flaglist, &bp);
	}
      }
      *bp = '\0';
      notify(player, tprintf("SITE: %-20s   FLAGS:%s", ap->host, flaglist));
      notify(player, tprintf(" COMMENT: %s", ap->comment));
    } else {
      notify(player, "---- @sitelock will add sites immediately below this line ----");
    }

  }
}

/* Parse options and return the appropriate can and cant bits.
 * Return the number of options successfully parsed.
 * This makes a copy of the options string, so it's not modified
 */
int
parse_access_options(opts, can, cant, player)
    const char *opts;
    int *can;
    int *cant;
    dbref player;		/* Who to notify of errors, or NOTHING to write to log */
{
  char myopts[BUFFER_LEN];
  char *p;
  char *w;
  acsflag *c;
  int found, totalfound;

  if (!opts || !*opts)
    return 0;
  strcpy(myopts, opts);
  totalfound = 0;
  p = trim_space_sep(myopts, ' ');
  while ((w = split_token(&p, ' '))) {
    found = 0;
    if (*w == '!') {
      /* Found a negated warning */
      w++;
      for (c = acslist; c->name; c++) {
	if (c->toggle && !strncasecmp(w, c->name, strlen(c->name))) {
	  *cant |= c->flag;
	  found++;
	}
      }
    } else {
      /* None is special */
      if (!strncasecmp(w, "NONE", 4)) {
	*cant = ACS_DEFAULT;
	found++;
      } else {
	for (c = acslist; c->name; c++) {
	  if (!strncasecmp(w, c->name, strlen(c->name))) {
	    *can |= c->flag;
	    found++;
	  }
	}
      }
    }
    /* At this point, we haven't matched any warnings. */
    if (!found) {
      if (GoodObject(player))
	notify(player, tprintf("Unknown access option: %s", w));
      else
	do_log(LT_ERR, GOD, GOD, "Unknown access flag: %s", w);
    } else {
      totalfound += found;
    }
  }
  return totalfound;
}
