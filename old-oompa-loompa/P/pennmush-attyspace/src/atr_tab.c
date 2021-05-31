/* atr_tab.c */

#include "config.h"

#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef I_STDLIB
#include <stdlib.h>
#endif
#include <stdio.h>
#include "conf.h"
#include "externs.h"
#include "atr_tab.h"
#include "htab.h"
#include "privtab.h"
#include "mymalloc.h"
#include "confmagic.h"

typedef struct atr_alias ATRALIAS;

struct atr_alias {
  const char *alias;
  const char *realname;
};

HASHTAB htab_attrib;

PRIV attr_privs[] =
{
  {"no_command", '$', AF_NOPROG, AF_NOPROG},
  {"no_inherit", 'i', AF_PRIVATE, AF_PRIVATE},
  {"private", 'i', AF_PRIVATE, AF_PRIVATE},
  {"no_clone", 'c', AF_NOCOPY, AF_NOCOPY},
  {"wizard", 'w', AF_WIZARD, AF_WIZARD},
  {"visual", 'v', AF_VISUAL, AF_VISUAL},
  {"mortal_dark", 'm', AF_MDARK, AF_MDARK},
  {"hidden", 'm', AF_MDARK, AF_MDARK},
  {"regexp", 'R', AF_REGEXP, AF_REGEXP},
  {"case", 'C', AF_CASE, AF_CASE},
  {"locked", '+', AF_LOCKED, AF_LOCKED},
  {NULL, '\0', 0, 0}
};


ATTR *aname_hash_lookup _((const char *name));
void init_aname_hashtab _((void));
extern char *strdup _((const char *));
void do_attribute_access _((dbref player, char *name, char *perms, int retroactive));
void do_attribute_delete _((dbref player, char *name));
void do_attribute_rename _((dbref player, char *old, char *newname));
void do_attribute_info _((dbref player, char *name));
void do_list_attribs _((dbref player));


/* this table can be expanded as necessary */

static ATRALIAS atr_alias_tab[] =
{
  {"ACONN", "ACONNECT"},
  {"ACON", "ACONNECT"},
  {"ADEST", "ADESTROY"},
  {"ADESTR", "ADESTROY"},
  {"ADESTRO", "ADESTROY"},
  {"ADESC", "ADESCRIBE"},
  {"ADESCR", "ADESCRIBE"},
  {"ADESCRI", "ADESCRIBE"},
  {"ADISC", "ADISCONNECT"},
  {"ADISCON", "ADISCONNECT"},
  {"ADISCONN", "ADISCONNECT"},
  {"AFAIL", "AFAILURE"},
  {"AIDESC", "AIDESCRIBE"},
  {"AIDESCR", "AIDESCRIBE"},
  {"AIDESCRI", "AIDESCRIBE"},
  {"APAY", "APAYMENT"},
  {"ASUCC", "ASUCCESS"},
  {"DESC", "DESCRIBE"},
  {"DESCR", "DESCRIBE"},
  {"DESCRI", "DESCRIBE"},
  {"FAIL", "FAILURE"},
  {"FILTE", "FILTER"},
  {"FILT", "FILTER"},
  {"FIL", "FILTER"},
  {"IDESC", "IDESCRIBE"},
  {"IDESCR", "IDESCRIBE"},
  {"IDESCRI", "IDESCRIBE"},
  {"INFILTE", "INFILTER"},
  {"INFILT", "INFILTER"},
  {"INFIL", "INFILTER"},
  {"INFI", "INFILTER"},
  {"INF", "INFILTER"},
  {"INPREFI", "INPREFIX"},
  {"INPREF", "INPREFIX"},
  {"INPRE", "INPREFIX"},
  {"INPR", "INPREFIX"},
  {"INP", "INPREFIX"},
#ifdef USE_MAILER
  {"MAILFILTER", "MAILFILTERS"},
  {"MAILFILT", "MAILFILTERS"},
  {"MAILFIL", "MAILFILTERS"},
  {"MAILFI", "MAILFILTERS"},
  {"MAILFOLDER", "MAILFOLDERS"},
  {"MAILFOLD", "MAILFOLDERS"},
  {"MAILFOL", "MAILFOLDERS"},
  {"MAILFO", "MAILFOLDERS"},
  {"MAILSIGNATUR", "MAILSIGNATURE"},
  {"MAILSIGNATU", "MAILSIGNATURE"},
  {"MAILSIGNAT", "MAILSIGNATURE"},
  {"MAILSIGNA", "MAILSIGNATURE"},
  {"MAILSIGN", "MAILSIGNATURE"},
  {"MAILSIG", "MAILSIGNATURE"},
  {"MAILSI", "MAILSIGNATURE"},
#endif
  {"ODESC", "ODESCRIBE"},
  {"ODESCR", "ODESCRIBE"},
  {"ODESCRI", "ODESCRIBE"},
  {"OFAIL", "OFAILURE"},
  {"OIDESC", "OIDESCRIBE"},
  {"OIDESCR", "OIDESCRIBE"},
  {"OIDESCRI", "OIDESCRIBE"},
  {"OPAY", "OPAYMENT"},
  {"OSUCC", "OSUCCESS"},
  {"PAY", "PAYMENT"},
  {"PREFI", "PREFIX"},
  {"PREF", "PREFIX"},
  {"PRE", "PREFIX"},
  {"SUCC", "SUCCESS"},
  {NULL, NULL}
};

/*----------------------------------------------------------------------
 * Hash functions of various sorts
 */


ATTR *
aname_hash_lookup(name)
    const char *name;
{
  /* given an attribute name, look it up in the complete attribute table
   * (real names plus aliases), and return the appropriate real attribute.
   */
  return (ATTR *) hashfind((char *) strupper(name), &htab_attrib);
}

void
init_aname_hashtab()
{
  ATTR *ap;
  ATRALIAS *aliasp;

  hashinit(&htab_attrib, 512);

  /* build the basic hash table */
  for (ap = attr; ap->name; ap++)
    hashadd(ap->name, (void *) ap, &htab_attrib);

  /* now add in aliases */
  for (aliasp = atr_alias_tab; aliasp->alias; aliasp++) {
    if ((ap = aname_hash_lookup(aliasp->realname)) != NULL)
      hashadd(aliasp->alias, (void *) ap, &htab_attrib);
    else
      fprintf(stderr,
	    "ATR INIT: attribute alias %s matches no known attribute.\n",
	      aliasp->alias);
  }
}

/* Add an attribute to the hash table, given its name and permissions */
void
do_attribute_access(player, name, perms, retroactive)
    dbref player;
    char *name;
    char *perms;
    int retroactive;
{
  ATTR *ap, *ap2;
  int flags = 0;
  int i;

  /* Parse name and perms */
  if (!name || !*name) {
    notify(player, "Which attribute do you mean?");
    return;
  }
  flags = string_to_privs(attr_privs, perms, 0);
  if (!flags) {
    notify(player, "I don't understand those permissions.");
    return;
  }
  upcasestr(name);
  /* Is this attribute already in the table? */
  ap = aname_hash_lookup(name);
  if (ap) {
    /* Already in the table, so we must delete before we add */
    hashdelete(name, &htab_attrib);
  } else {
    /* Create fresh */
    ap = (ATTR *) mush_malloc(sizeof(ATTR), "ATTR");
    if (!ap) {
      notify(player, "Critical memory failure - Alert God!");
      do_log(LT_ERR, 0, 0, "do_attribute_access: unable to malloc ATTR");
      return;
    }
    AL_NAME(ap) = strdup(name);
    AL_STR(ap) = NULL;
  }
  AL_FLAGS(ap) = flags;
  AL_CREATOR(ap) = player;
  hashadd(name, (void *) ap, &htab_attrib);

  /* Ok, now we need to see if there are any attributes of this name
   * set on objects in the db. If so, and if any of them have names
   * allocated, we will deallocate them and turn them into static pointers
   * to the attribute name table 
   */
  for (i = 0; i < db_top; i++) {
    if ((ap2 = atr_get_noparent(i, name))) {
      if (!(AL_FLAGS(ap2) & AF_STATIC)) {
	/* This one's not static. Make it so */
	mush_free((Malloc_t)AL_NAME(ap2), "ATTR name");
	AL_NAME(ap2) = AL_NAME(ap);
	AL_FLAGS(ap2) |= AF_STATIC;
      }
      /* And, while we're here, if we've been told to be RETROACTIVE,
       * set the permissions/creator on this object
       */
      if (retroactive) {
	AL_FLAGS(ap2) = flags | AF_STATIC;
	AL_CREATOR(ap2) = player;
      }
    }
  }

  notify(player, tprintf("%s -- Attribute permissions now: %s", name,
			 privs_to_string(attr_privs, flags)));
}


void
do_attribute_delete(player, name)
    dbref player;
    char *name;
{
  ATTR *ap;
  char *namecpy;
  int i;
  if (!name || !*name) {
    notify(player, "Which attribute do you mean?");
    return;
  }
  upcasestr(name);
  /* Is this attribute in the table? */
  ap = aname_hash_lookup(name);
  if (!ap) {
    notify(player, "That attribute isn't in the attribute table");
    return;
  }
  /* We found it. Before we delete it, we've got to run the database
   * and see if there are any of that attribute set that are using
   * AF_STATIC names. If so, we need to make them their own copies of
   * the name and clear AF_STATIC.
   */
  for (i = 0; i < db_top; i++) {
    if ((ap = atr_get_noparent(i, name))) {
      if (AL_FLAGS(ap) & AF_STATIC) {
	namecpy = (char *) mush_malloc(strlen(name) + 1, "ATTR name");
	if (!namecpy)
	  panic("Out of memory in do_attribute_delete!");
	strcpy(namecpy, name);
	AL_NAME(ap) = namecpy;
      }
    }
  }
  /* Ok, take it out of the hash table */
  hashdelete(name, &htab_attrib);
  notify(player, tprintf("Removed %s from attribute table.", name));
  return;
}

void
do_attribute_rename(player, old, newname)
    dbref player;
    char *old;
    char *newname;
{
  ATTR *ap;
  if (!old || !*old || !newname || !*newname) {
    notify(player, "Which attributes do you mean?");
    return;
  }
  upcasestr(old);
  upcasestr(newname);
  /* Is the new name already in use? */
  ap = aname_hash_lookup(newname);
  if (ap) {
    notify(player, tprintf("The name %s is already used in the attribute table.", newname));
    return;
  }
  /* Is the old name a real attribute? */
  ap = aname_hash_lookup(old);
  if (!ap) {
    notify(player, "That attribute isn't in the attribute table");
    return;
  }
  /* Ok, take it out and put it back under the new name */
  hashdelete(old, &htab_attrib);
  free((Malloc_t)AL_NAME(ap));
  AL_NAME(ap) = strdup(newname);
  hashadd(newname, (void *) ap, &htab_attrib);
  notify(player, tprintf("Renamed %s to %s in attribute table.", old, newname));
  return;
}

/* Info on an attribute in the table */
void
do_attribute_info(player, name)
    dbref player;
    char *name;
{
  ATTR *ap;
  if (!name || !*name) {
    notify(player, "Which attribute do you mean?");
    return;
  }
  upcasestr(name);
  /* Is this attribute in the table? */
  ap = aname_hash_lookup(name);
  if (!ap) {
    notify(player, "That attribute isn't in the attribute table");
    return;
  }
  notify(player, tprintf("Attribute: %s", AL_NAME(ap)));
  notify(player, tprintf("    Flags: %s", privs_to_string(attr_privs, AL_FLAGS(ap))));
  notify(player, tprintf("  Creator: #%d", AL_CREATOR(ap)));
  return;
}

void
do_list_attribs(player)
    dbref player;
{
  ATTR *ap;
  char *ptrs[BUFFER_LEN / 2];
  char buff[BUFFER_LEN];
  char *bp;
  int nptrs = 0, i;
  HASHTAB temp;

  hashinit(&temp, 256);
  ap = hash_firstentry(&htab_attrib);
  while (ap) {
    hashadd(ap->name, (void *) ap->name, &temp);
    ap = hash_nextentry(&htab_attrib);
  }
  bp = hash_firstentry(&temp);
  while (bp) {
    ptrs[nptrs++] = (char *) bp;
    bp = hash_nextentry(&temp);
  }
  do_gensort(ptrs, nptrs, 0);
  bp = buff;
  safe_str("Attribs:", buff, &bp);
  for (i = 0; i < nptrs; i++) {
    safe_chr(' ', buff, &bp);
    safe_str(ptrs[i], buff, &bp);
  }
  *bp = '\0';
  notify(player, buff);
  hashfree(&temp);
}
