/*-----------------------------------------------------------------
 * The offline database code.
 *
 * "Look upon my works ye mighty and lose all bladder control..."
 * - Things Unsaid by Doogie
 * -- Atuarre
 *
 */
 
 #include "copyrite.h"
 #include "config.h"
 #ifdef I_STRING
 #include <string.h>
 #else
 #include <strings.h>
 #endif
 #include "externs.h"
 #include "intrface.h"
 #include "parse.h"
 #include "confmagic.h"
 #include "function.h"
 
 #include <math.h>
 #include "ansi.h"
 #include "match.h"
 #include "offline.h"

/* ------------------------------------------------------------------------*/

struct DBLIST *offline_finddb(char *filename);
int offline_write(char *attribute, char *value, char *filename);
int offline_delete(char *attribute, char *filename);
char *offline_read(char *attribute, char *filename);
void offline_init();
extern int good_atr_name(char const *s);
struct DBLIST odblist;

int check_perms(dbref dbobject, dbref player, int type);

/* ------------------------------------------------------------------------*/

struct DBLIST *offline_finddb(filename)
	char *filename;
{
	struct DBLIST *dblp = &odblist;

	if(!filename) return(dblp);

	do {
		if(!strcmp(dblp->filename, filename)) return(dblp);
		else dblp = dblp->next;
	} while(dblp->next);

	return(NULL);
}

int check_perms(dbobject, player, type)
	dbref dbobject;
	dbref player;
	int type; /* 0 = Read, 1 = Write */
{
	struct boolexp *key;
	
	if(!GoodObject(dbobject))
	  return -1;
	
	key = getlock(dbobject, (type == 0) ? DBRead_Lock : DBWrite_Lock);
	
        
        /* If key is TRUE, anyone can do it */
        
        if(key && key == TRUE_BOOLEXP)
          return 1;
          
        /* If it's READLOCK and the player is a Wiz/Roy or See_all */
        if(type == GDBM_READLOCK && (Hasprivs(player) || See_All(player)))
          return 1;
          
        /* If the read @lock doesn't exist, return the output of can_examine */
        
        if(!key && type == 0)
          return Can_Examine(player,dbobject);
          
	/* If the write @lock doesn't exist, return the output of controls() */
	if(!key && type == 1)
	  return controls(player, dbobject);
	
	/* Evaluate the lock */
	return eval_lock(player, dbobject, (type == 0) ? DBRead_Lock: DBWrite_Lock);
}
	  
	
	
int offline_write(attribute, value, filename)
	char *attribute;
	char *value;
	char *filename;
{
	struct DBLIST *dblp;
	datum dbkey;
	datum dbdata;

	dblp = offline_finddb(filename);
	if(!dblp) return(0);

	dbkey.dptr = attribute; dbkey.dsize = strlen(attribute)+1;
	dbdata.dptr = value; dbdata.dsize = strlen(value)+1;

	if(gdbm_store(dblp->dbf, dbkey, dbdata, GDBM_REPLACE)) return(0);

	return(1);
}

int offline_delete(attribute, filename)
	char *attribute;
	char *filename;
{
	struct DBLIST *dblp;
	datum dbkey;
	datum dbdata;
	datum dbdatatmp;

	dblp = offline_finddb(filename);
	if(!dblp) return(0);

	dbkey.dptr = attribute; dbkey.dsize = strlen(attribute)+1;

	if(gdbm_delete(dblp->dbf, dbkey)) return(0);

	return(1);
}

char *offline_read(attribute, filename)
	char *attribute;
	char *filename;
{
	struct DBLIST *dblp;
	datum dbkey;
	datum dbdata;

	dblp = offline_finddb(filename);
	if(!dblp) return(NULL);

	dbkey.dptr = attribute, dbkey.dsize = strlen(attribute)+1;
	dbdata = gdbm_fetch(dblp->dbf, dbkey);

	return(dbdata.dptr);
}

void offline_init()
{
	struct DBLIST *dblp = &odblist;
	struct DBLIST dbinput;
	int dbobject;
	char *dbquota;
	int retval;
	datum dbkey;
	datum dbkeynext;
	datum dbdata;
	char atrtemp[130];
	char *p;

	dblp->dbf = gdbm_open((char *)"data/main.gdbm", 1024, GDBM_WRCREAT | GDBM_FAST , 0600, NULL);
	if(!dblp->dbf) {
		fprintf(stderr, "Unable to open main GDBM database!\n");
		dblp->dbf = NULL; dblp->next = NULL;
		return;
	}
	strcpy(dblp->filename,"main.gdbm");
	dblp->next = NULL;

	fprintf(stderr, "REPACKING: data/main.gdbm\n");

	gdbm_reorganize(dblp->dbf);

	fprintf(stderr, "ANALYZING: data/main.gdbm\n");

	for (dbobject = 0; dbobject < db_top; dbobject++) {
		dbquota = offline_read(tprintf("#%d+QUOTA", dbobject), NULL);
		if(dbquota) {
			GDBMQuota(dbobject) = atoi(dbquota);
			free(dbquota); /* Externally malloc'd, do not use MUSH wrapper */
		} else {
			GDBMQuota(dbobject) = 0;
		}
	}

	/* Process all attributes in the database and calculate usages */
	dbkey = gdbm_firstkey(dblp->dbf);
	while(dbkey.dptr) {
		dbdata = gdbm_fetch(dblp->dbf, dbkey);
		if(!dbdata.dptr) {
			do_log(LT_ERR, 0, 0, "GDBM: Removing bogus empty attribute %s",
			   dbkey.dptr);
			gdbm_delete(dblp->dbf, dbkey);
		} else {
			strcpy(atrtemp, dbkey.dptr);
			p = strchr(atrtemp, ':'); 
			if(p) {
				*p='\0'; p++;
				GDBMUsage(parse_dbref(atrtemp)) +=
				   strlen(p) + strlen(dbdata.dptr);
			}
		}
		dbkeynext = gdbm_nextkey(dblp->dbf, dbkey);
		free(dbkey.dptr); free(dbdata.dptr);
		dbkey = dbkeynext;
	}

	fprintf(stderr, "ANALYZING: data/main.gdbm (done)\n");
	return;
}

void offline_shutdown()
{
	struct DBLIST *dblp = &odblist;

	gdbm_close(dblp->dbf);
	return;
}

char *offline_raw_lattr()
{
	struct DBLIST *dblp;
	datum dbkey;
	datum dbnextkey;
	char buffer[BUFFER_LEN];
	static char buff[BUFFER_LEN];
	char *bp = buff;

	dblp = offline_finddb(NULL);
	if(!dblp) return(NULL);

	dbkey = gdbm_firstkey(dblp->dbf);
	if(!dbkey.dptr) return(NULL);
	strncpy(buffer, dbkey.dptr, dbkey.dsize);
	safe_str(buffer, buff, &bp);
	for(;;) {
		dbnextkey = gdbm_nextkey(dblp->dbf, dbkey);
		free(dbkey.dptr); 
		dbkey = dbnextkey;
		if(!dbkey.dptr) return((char *)&buff);
		strncpy(buffer, dbkey.dptr, dbkey.dsize);
		safe_str(tprintf(" %s", buffer), buff, &bp);
	}

	/* NOTREACHED */
}

void offline_add_atr(dbobject, attribute)
	dbref dbobject;
	char *attribute;
{
	char *newlist;
	char *p;

	p = offline_read(tprintf("%s+ATRLIST", unparse_dbref(dbobject)), NULL);
	if(!p) {
		offline_write(tprintf("%s+ATRLIST", unparse_dbref(dbobject)),
		   attribute, NULL);
		return;
	} else {
                newlist = mush_malloc(strlen(p) + strlen(attribute) + 1,
		   "gdbm.add_atr.newlist");
                strcpy(newlist, p);
                free(p); /* Externally malloc'd, do not use MUSH wrapper */
                strcat(newlist, " ");
		strcat(newlist, attribute);
                offline_write(tprintf("%s+ATRLIST", unparse_dbref(dbobject)),
                   newlist, NULL);
                mush_free(newlist, "gdbm.add_atr.newlist");
                return;
	}

	/* NOTREACHED */
}

void offline_del_atr(dbobject, attribute)
	dbref dbobject;
	char *attribute;
{
	char *newlist;
	char *p;
	char *q;

	p = offline_read(tprintf("%s+ATRLIST", unparse_dbref(dbobject)), NULL);
	if(!p) {
		/* DB is corrupt or caller was bad about error checking */
		do_log(LT_ERR, 0, 0,
		   tprintf("GDBM: Warning, no ATRLIST on delete for object %d!",
		   dbobject));
		return;
	} else {
		newlist = mush_malloc(strlen(p) + 1, "gdbm.atr_del.newlist"); *newlist = '\0';
		q = strtok(p, " ");
		while(q) {
			if(strcmp(q,attribute)) {
				if(!*newlist)
					strcpy(newlist, q);
				else {
					strcat(newlist, " ");
					strcat(newlist, q);
				}
			}
			q = strtok(NULL," ");
		}
	
		if(*newlist)
			offline_write(tprintf("%s+ATRLIST", unparse_dbref(dbobject)),
			   newlist, NULL);
		else
			offline_delete(tprintf("%s+ATRLIST", unparse_dbref(dbobject)),
			   NULL);

		free(p); /* Externally malloc'd, do not use MUSH wrapper */
		mush_free(newlist, "gdbm.atr_del.newlist");
		return;
	}

	/* NOTREACHED */
}	

void offline_object_wipe(dbobject)
	dbref dbobject;
{
	char *atrlist;
	char *p;

	atrlist = offline_read(tprintf("%s+ATRLIST", unparse_dbref(dbobject)), NULL);
	if(!atrlist) return;

	p = strsep(&atrlist," ");
	while(p) {
		offline_delete(tprintf("%s:%s", unparse_dbref(dbobject), p), NULL);
		do_log(LT_ERR, 0, 0, "GDBM: Object nuke, wiping attribute %s:%s", unparse_dbref(dbobject), p);
		p = strsep(&atrlist," ");
	}
	free(atrlist); /* Externally malloc'd, do not use MUSH wrapper */

	offline_delete(tprintf("%s+ATRLIST", unparse_dbref(dbobject)), NULL);
	do_log(LT_ERR, 0, 0, "GDBM: Object nuke, wiping attribute %s+ATRLIST", unparse_dbref(dbobject));

	offline_delete(tprintf("%s+QUOTA", unparse_dbref(dbobject)), NULL);
	do_log(LT_ERR, 0, 0, "GDBM: Object nuke, wiping attribute %s+QUOTA", unparse_dbref(dbobject));

	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_dbset)
{
	dbref dbobject;
	char *p;
	long int tmpusage;
        
        dbobject = noisy_match_result(executor, args[0], NOTYPE, MAT_EVERYTHING);
             
        if(!GoodObject(dbobject)) {
          safe_str("#-1 NO SUCH OBJECT VISIBLE", buff, bp);
          return;
        }
        
        if(check_perms(dbobject, executor, GDBM_WRITELOCK) == 0) {
          safe_str("#-1 PERMISSION DENIED.", buff, bp);
          return;
        }
        
	p = offline_read(tprintf("%s:%s", unparse_dbref(dbobject), strupper(args[1])), NULL);

	if(nargs == 2) {
		if(!p) {
			safe_str("#-1 NO SUCH DB OR ATTRIBUTE", buff, bp);
			return;
		} else {
			offline_delete(tprintf("%s:%s", unparse_dbref(dbobject),
			   strupper(args[1])), NULL);
			GDBMUsage(dbobject) = GDBMUsage(dbobject) - strlen(p) - strlen(args[1]);
			offline_del_atr(dbobject, strupper(args[1]));
			free(p); /* Externally malloc'd, do not use MUSH wrapper */
			return;
		}
	}

	if(!good_atr_name(args[1]) || strlen(args[1]) > 128) {
		safe_str("#-1 INVALID ATTRIBUTE NAME", buff, bp);
		return;
	}

	if(p) {
		tmpusage = GDBMUsage(dbobject) - (int) strlen(p) - (int) strlen(args[1]);
		if(tmpusage + (int) strlen(args[1]) + (int) strlen(args[2]) > GDBMQuota(dbobject)) {
			safe_str("#-1 QUOTA EXCEEDED", buff, bp);
			free(p); /* Externally malloc'd, do not use MUSH wrapper */
			return;
		}
		GDBMUsage(dbobject) = tmpusage;
		free(p); /* Externally malloc'd, do not use MUSH wrapper */
	} else {
		if(GDBMUsage(dbobject) + (int) strlen(args[1]) + (int) strlen(args[2]) > GDBMQuota(dbobject)) {
			safe_str("#-1 QUOTA EXCEEDED", buff, bp);
			return;
		}
		offline_add_atr(dbobject, strupper(args[1]));
	}

	if(!offline_write(tprintf("%s:%s", unparse_dbref(dbobject),
	   strupper(args[1])), args[2], NULL)) {
		safe_str("#-1 CANNOT WRITE", buff, bp);
	} 

	GDBMUsage(dbobject) = GDBMUsage(dbobject) + strlen(args[1]) + strlen(args[2]);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_dbget)
{
	dbref dbobject;
	char *p;
	

	dbobject = noisy_match_result(executor, args[0], NOTYPE, MAT_EVERYTHING);

	if(!GoodObject(dbobject)) {
	  safe_str("#-1 NO SUCH OBJECT VISIBLE", buff, bp);
	  return;
	}
	
	if(check_perms(dbobject, executor, GDBM_READLOCK) == 0) {
	  safe_str("#-1 PERMISSION DENIED.", buff, bp);
	  return;
	}
	
	p = offline_read(tprintf("%s:%s", unparse_dbref(dbobject),
	   strupper(args[1])), NULL);

	if(p) {
		safe_str(p, buff, bp);
		free(p); /* Externally malloc'd, do not use MUSH wrapper */
	} else {
		safe_str("#-1 NO SUCH ATTRIBUTE", buff, bp);
	}

	return;
}

/* ------------------------------------------------------------------------ */

FUNCTION(fun_dblattr)
{
        char *atrlist;
        dbref dbobject;
        char *s, *b, *r;

	dbobject = noisy_match_result(executor, args[0], NOTYPE, MAT_EVERYTHING);

        if(!GoodObject(dbobject)) {
		safe_str("#-1 NO SUCH OBJECT VISIBLE", buff, bp);
		return;
	}

	if(!check_perms(dbobject, executor, GDBM_READLOCK)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
		return;
	}

	atrlist = offline_read(tprintf("%s+ATRLIST",
	   unparse_dbref(dbobject)), NULL);

        if(!atrlist) {
                safe_str("#-1 EMPTY OR NONEXISTENT DB", buff, bp);
                return;
        }

        if(nargs > 1) {

          s = trim_space_sep(atrlist, ' ');
          b = *bp;
          do {
            r = split_token(&s, ' ');
            if(local_wild_match(args[1], r)) {
              if(*bp != b)
                safe_chr(' ', buff, bp);
              safe_str(r, buff, bp);
            }
          } while(s);
        }
        else
          safe_str(atrlist, buff, bp);

        free(atrlist); /* Externally malloc'd, do not use MUSH wrapper */
}        


/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_dbhasattr)
{
	dbref dbobject;
	char *p;

	dbobject = noisy_match_result(executor, args[0], NOTYPE, MAT_EVERYTHING);
        if(!GoodObject(dbobject)) {
          safe_str("#-1	NO SUCH OBJECT VISIBLE", buff, bp);
          return;
        }
        
	if(!check_perms(dbobject, executor, GDBM_READLOCK)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
		return;
	}

	p = offline_read(tprintf("%s:%s", unparse_dbref(dbobject),
	   strupper(args[1])), NULL);
	if(!p)
		safe_chr('0', buff, bp);
	else {
		safe_chr('1', buff, bp);
		free(p); /* Externally malloc'd, do not use MUSH wrapper */
	}
	return;
}
/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_dbeval)
{
	dbref dbobject;
	char *p;
	char const *q;

	dbobject = noisy_match_result(executor, args[0], NOTYPE, MAT_EVERYTHING);
	if(!GoodObject(dbobject)) {
	  safe_str("#-1	NO SUCH OBJECT VISIBLE", buff, bp);
	  return;
	}
	
	if(!check_perms(dbobject, executor, GDBM_READLOCK)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
		return;
	}

	p = offline_read(tprintf("%s:%s", unparse_dbref(dbobject),
	   strupper(args[1])), NULL);
	q = p;

	if(!p) {
		safe_str("#-1 NO SUCH ATTRIBUTE", buff, bp);
		return;
	} else {
		process_expression(buff, bp, &q, executor, caller, enactor,
		   PE_DEFAULT, PT_DEFAULT, pe_info);
		free(p); /* Externally malloc'd, do not use MUSH wrapper */
		return;
	}
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_dbquota)
{
	dbref dbobject;

	dbobject = noisy_match_result(executor, args[0], NOTYPE, MAT_EVERYTHING);

	if(!GoodObject(dbobject)) {
	  safe_str("#-1	NO SUCH VISIBLE OBJECT", buff, bp);
	  return;
	}

	if(nargs == 1 && !check_perms(dbobject, executor, GDBM_READLOCK)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
		return;
	}
	if (nargs == 2 && !Wizard(executor) && !Royalty(executor)) {
	    safe_str("#-1 PERMISSION DENIED", buff, bp);
	    return;
	}
	  
 	if(nargs == 1) {
		safe_str(tprintf("%d", GDBMQuota(dbobject)), buff, bp);
		return;
	} else {
		GDBMQuota(dbobject) = abs(atoi(args[1]));
		offline_write(tprintf("%s+QUOTA", unparse_dbref(dbobject)),
		   args[1], NULL);
		return;
	}
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_dbusage)
{
	dbref dbobject;

	dbobject = noisy_match_result(executor, args[0], NOTYPE, MAT_EVERYTHING);

	if(!GoodObject(dbobject)) {
	  safe_str("#-1	NO SUCH OBJECT VISIBLE", buff, bp);
	  return;
	}

	if(!check_perms(dbobject, executor, GDBM_READLOCK)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
		return;
	}

	safe_str(tprintf("%d", GDBMUsage(dbobject)), buff, bp);
	return;
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_dbv)
{
	char *p;

	p = offline_read(tprintf("%s:%s",unparse_dbref(executor),
	   strupper(args[0])), NULL);
	if(p) {
		safe_str(p, buff, bp);
		free(p);
		return;
	} else {
		safe_str("#-1 NO SUCH ATTRIBUTE", buff, bp);
		return;
	}

	/* NOTREACHED */
}

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_dbufun) {
	char *tptr[10], *name, *p, *s;
	char const *tp;
	dbref dbobject;
	int j;
 
	s = args[0];
  
	if((name = (char *)index(s, '/')) != NULL) {
		*name++ = '\0';
		dbobject = noisy_match_result(executor, s, NOTYPE, MAT_EVERYTHING);
	} else { 
		name = s;
		dbobject = executor;
	}
  
	if(!check_perms(dbobject, executor, GDBM_READLOCK)) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
		return;
	}

	p = offline_read(tprintf("%s:%s", unparse_dbref(dbobject), upcasestr(name)), NULL);
  
	if(!p) {
		safe_str("#-1 NO SUCH VISIBLE ATTRIBUTE", buff, bp);
		return;
	}

/* Save %0-%9 registers */
  for (j = 0; j < 10; j++)
    tptr[j] = wenv[j];
        
  if (nargs > 10)
    nargs = 10;

/* Pop in our own registers */
  for (j = 0; j < nargs; j++)
    wenv[j] = args[j+1];
  for (; j < 10; j++)
    wenv[j] = NULL;

  tp = p;
  process_expression(buff, bp, &tp, executor, executor, enactor,
        PE_DEFAULT, PT_DEFAULT, pe_info);
  free((Malloc_t)p);

/* Restore registers */
 
  for (j = 0; j < 10; j++)
    wenv[j] = tptr[j];

  return;
        
} 

/* ------------------------------------------------------------------------ */
