
/*
 * Binary object handling gear. Shit simple.
 * 
 * Andrew Molitor, amolitor@nmsu.edu
 * 
 * 1992
 * 
 * $Id: udb_obj.c,v 1.1 2001/01/15 03:14:11 wenk Exp $
 */

#include	"autoconf.h"
#include	"config.h"
#include	"externs.h"
#include	"udb.h"
#include	"udb_defs.h"

#ifndef STANDALONE
extern int	FDECL(make_checksum, (char *, int));
extern void	FDECL(dump_database_internal, (int));
#endif

/*
 * Sizes, on disk, of Object and (within the object) Attribute headers 
 */

#define OBJ_HEADER_SIZE		(sizeof(Objname) + sizeof(int))
#define ATTR_HEADER_SIZE	(sizeof(int) * 3)
/*
 * 
 * Routines to get Obj's on and off disk. Obj's are stowed in a
 * fairly complex way: an object header with the object ID (not really
 * needed, but there to facilitate possible recoveries of crashed DBs),
 * and an attribute count. This is followed by the attributes.
 * 
 * We use the standard library here, and do a lot of fread()s.
 * Trust your standard library. If you think this is inefficient, you have
 * your head up your ass. This means you, Jellan.
 */


Obj *
 objfromFILE(f)
FILE *f;
{
	int i, j;
	int size;
	Obj *o;
	Attrib *a;

	/*
	 * Get a new Obj struct 
	 */

	if ((o = (Obj *) malloc(sizeof(Obj))) == (Obj *) 0)
		return ((Obj *) 0);

	/*
	 * Read in the header 
	 */

	if (fread(&(o->name), sizeof(Objname), 1, f) != 1) {
		free(o);
		return ((Obj *) 0);
	}
	if (fread(&i, sizeof(int), 1, f) != 1) {
		free(o);
		return ((Obj *) 0);
	}
	o->at_count = i;

	/*
	 * Now get an array of Attrs 
	 */

	a = o->atrs = (Attrib *) malloc(i * sizeof(Attrib));
	if (!o->atrs) {
		free(o);
		return ((Obj *) 0);
	}
	/*
	 * Now go get the attrs, one at a time. 
	 */

	for (j = 0; j < i;) {

		/* Attribute checksum */
		
		if (fread(&(a[j].checksum), sizeof(int), 1, f) != 1)
			goto bail;
			
		/*
		 * Attribute size 
		 */

		if (fread(&size, sizeof(int), 1, f) != 1)
			 goto bail;

		a[j].size = size;

		/*
		 * Attribute number 
		 */

		if (fread(&(a[j].attrnum), sizeof(int), 1, f) != 1)
			 goto bail;

		/*
		 * get some memory for the data 
		 */

		if ((a[j].data = (char *)malloc(size)) == (char *)0)
			goto bail;

		/*
		 * Preincrement j, so we know how many to free if this next * 
		 * 
		 * *  * * bit fails. 
		 */

		j++;

		/*
		 * Now get the data 
		 */

		if (fread(a[j - 1].data, 1, size, f) != size)
			goto bail;

#ifndef STANDALONE		
		/* Make sure checksum fits */
		
		if (make_checksum((char *)a[j - 1].data, a[j - 1].size) != a[j - 1].checksum) {
			STARTLOG(LOG_ALWAYS, "BUG", "CRUPT")
				log_text((char *)tprintf("Database is corrupt, object %d. Exiting.", (int)(o->name)));
			ENDLOG
			raw_broadcast(0, "Game: Database corruption detected, exiting.");	
			exit(8);
		}
#endif
	}


	/*
	 * Should be all done.. 
	 */

	return (o);

	/*
	 * Oh shit. We gotta free up all these little bits of memory. 
	 */
      bail:
	/*
	 * j points one attribute *beyond* what we need to free up 
	 */

	for (i = 0; i < j; i++)
		free(a[i].data);

	free(a);
	free(o);

	return ((Obj *) 0);
}


int
objtoFILE(o, f)
Obj *o;
FILE *f;
{
	int i;
	Attrib *a;


	/*
	 * Write out the object header 
	 */

	if (fwrite(&(o->name), sizeof(Objname), 1, f) != 1)
		return (-1);

	if (fwrite(&(o->at_count), sizeof(int), 1, f) != 1)
		 return (-1);

	/*
	 * Now do the attributes, one at a time. 
	 */

	a = o->atrs;
	for (i = 0; i < o->at_count; i++) {

#ifndef STANDALONE
		/* Make sure checksum fits */
		
		if (make_checksum((char *)a[i].data, a[i].size) != a[i].checksum) {
			STARTLOG(LOG_ALWAYS, "BUG", "CRUPT")
				log_text((char *)tprintf("Database is corrupt, object %d. Exiting.", (int)(o->name)));
			ENDLOG
			raw_broadcast(0, "Game: Database corruption detected, exiting.");
			exit(8);
		}
#endif

		/* Attribute checksum */
		
		if (fwrite(&(a[i].checksum), sizeof(int), 1, f) != 1)
			return (-1);
			
		/*
		 * Attribute size. 
		 */

		if (fwrite(&(a[i].size), sizeof(int), 1, f) != 1)
			 return (-1);

		/*
		 * Attribute number 
		 */

		if (fwrite(&(a[i].attrnum), sizeof(int), 1, f) != 1)
			 return (-1);


		/*
		 * Attribute data 
		 */

		if (fwrite(a[i].data, 1, a[i].size, f) != a[i].size)
			return (-1);
		
	}

	/*
	 * That's all she wrote. 
	 */

	return (0);
}

/*
 * Return the size, on disk, the thing is going to take up.
 */

int
obj_siz(o)
Obj *o;
{
	int i;
	int siz;

	siz = OBJ_HEADER_SIZE;

	for (i = 0; i < o->at_count; i++)
		siz += (((o->atrs)[i]).size + ATTR_HEADER_SIZE);

	return (siz);
}
