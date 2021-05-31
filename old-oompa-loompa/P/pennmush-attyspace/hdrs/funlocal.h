#include <sys/param.h>
#include <gdbm.h>

struct DBLIST {
	GDBM_FILE dbf;
	char filename[MAXPATHLEN];
	struct DBLIST *next;
};

#define GDBMQuota(x)	(int)db[x].gdbmquota
#define GDBMUsage(x)	(int)db[x].gdbmusage

#define GDBM_READLOCK	0
#define GDBM_WRITELOCK	1
