#ifdef TEST
typedef int dbref;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "config.h"
/*#include "externs.h"
#include "db.h"*/

#include "../header.h"
#include "mech.h"

/*****************************************************************************/
/* TIC Routines                                                              */
/*****************************************************************************/
void mech_cleartic(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  int ticnum, argc;
  char *args[3];

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) {
    if(mech->unconcious) 
      {
	notify(player, "Nice place to take a nap...zzzzz");
	return;
      }
    argc=mech_parseattributes(buffer, args, 3);
    if(argc==1) {
      ticnum=atoi(args[0]);
      if(ticnum >= 0 && ticnum < NUM_TICS) {
	mech->tic[ticnum]=0;
	notify(player, "TIC cleared!");
      } else {
	notify(player, "TIC out of range!");
      } 
    } else {
      notify(player, "Invalid number of arguments to function");
    }
  }
}

void mech_addtic(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  int ticnum, argc;
  char *args[3];
  int weapnum;
  unsigned long newweap;
  int section;
  int critical;

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) {
    if(mech->unconcious) 
      {
	notify(player, "Nice place to take a nap...zzzzz");
	return;
      }
    argc=mech_parseattributes(buffer, args, 3);
    if(argc==2) {
      ticnum=atoi(args[0]);
      if(ticnum>=0 && ticnum <NUM_TICS) {
	weapnum=atoi(args[1]);
	if(weapnum>=0) {
	  if(FindWeaponNumberOnMech(mech, weapnum, &section, &critical)!= -1) {
	    /* Okay, now we must put it in.. */
	    newweap=1;
	    newweap <<= weapnum;
	    mech->tic[ticnum] |= newweap;
	    notify(player, "Weapon added to TIC!");
	  } else {
	    notify(player, "No weapon with that number found on the Mech!");
	  }
	} else {
	  notify(player, "Weaponnum must be >= 0");
	}
      } else {
	notify(player, "TIC out of range!");
      } 
    } else {
      notify(player, "Invalid number of arguments to function");
    }
  }
}

void mech_deltic(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  int ticnum, argc;
  char *args[3];
  int weapnum;
  unsigned long deadweap;
  int section;
  int critical;

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) {
    if(mech->unconcious) 
      {
	notify(player, "Nice place to take a nap...zzzzz");
	return;
      }
    argc=mech_parseattributes(buffer, args, 3);
    if(argc==2) {
      ticnum=atoi(args[0]);
      if(ticnum>=0 && ticnum <NUM_TICS) {
	weapnum=atoi(args[1]);
	if(weapnum>=0) {
	  if(FindWeaponNumberOnMech(mech, weapnum, &section, &critical)!= -1) {
	    deadweap=1;
	    deadweap <<= weapnum;
	    deadweap = ~deadweap;
	    mech->tic[ticnum] &= deadweap;
	    notify(player, "Weapon cleared from TIC!");
	  } else {
	    notify(player, "No weapon with that number found on the Mech!");
	  }
	} else {
	  notify(player, "Weaponnum must be > 0");
	}
      } else {
	notify(player, "TIC out of range!");
      } 
    } else {
      notify(player, "Invalid number of arguments to function");
    }
  }
}

void mech_firetic(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  struct map_struct *mech_map;
  int ticnum, argc;
  char *args[5];
  int loop;
  int weapnum;
  unsigned long weaps;

  mech=(struct mech_data *) data;
  mech_map=(struct map_struct *)FindObjectsData(mech->mapindex);
  if(!mech_map) {
    notify(player, "You are on an invalid map! Map index reset!");
    mech_shutdown(player,data,"");
    send_mechinfo(tprintf("firetic:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
    mech->mapindex= -1;
    return;
  }
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
	{
	  notify(player, "Nice place to take a nap...zzzzz");
	  return;
	}
      if(!(mech->status & STARTED)) 
	{
	  notify(player, "Fusion reactor is not online!");
	  return;
	}

      argc=mech_parseattributes(buffer, args, 5);
      if(argc>=1) 
	{
	  ticnum=atoi(args[0]);
	  if(ticnum>=0 && ticnum <NUM_TICS) 
	    {
	      notify(player, "Firing all weapons in TIC at default target!");
	      weaps = 1;
	      for(loop=0; loop<sizeof(long)*8; loop++) 
		{ 
		  if(mech->tic[ticnum] & weaps) 
		    {
		      weapnum=loop;
		      FireWeaponNumber(player,mech,
						mech_map,weapnum,argc,args,0); 
		    } 
		  weaps <<= 1;
		}
	    }
	  else 
	    {
	      notify(player, "TIC out of range!");
	    } 
	}    
      else 
	{
	  notify(player, "Not enough arguments to function");
	}
    }
}

void mech_listtic(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  unsigned char weaparray[MAX_WEAPS_SECTION], dummy[MAX_WEAPS_SECTION];
  int dummyint[MAX_WEAPS_SECTION];
  int ticnum, argc, loop, ii, weapnum=0, count;
  char *args[3], weapname[20][20];
  unsigned long weaps;

  mech=(struct mech_data *) data;

  /* make a list of the available weapons */
  for(loop=0; loop<NUM_SECTIONS; loop++) {
    count = FindWeapons(mech, loop, weaparray, dummy, dummyint);
    if (count) 
      for (ii=0; ii<count; ii++) 
        sprintf(weapname[weapnum++], &MechWeapons[weaparray[ii]].name[3]);
    }

  if(CheckData(player, mech)) {
    if(mech->unconcious) 
      {
	notify(player, "Nice place to take a nap...zzzzz");
	return;
      }
    argc=mech_parseattributes(buffer, args, 3);
    if(argc==1) {
      ticnum=atoi(args[0]);
      if(ticnum>=0 && ticnum <NUM_TICS) {
	weaps=1;
	notify(player, tprintf("--- TIC #%d Listing ---", ticnum));
	for(loop=0; loop<sizeof(unsigned long)*8; loop++) {
	  if(mech->tic[ticnum] & weaps) {
	    notify(player, tprintf(" Weapon #%d: %s", loop,
	      weapname[loop]));
	  } 
	  weaps <<= 1;
	}
      } else {
	notify(player, "TIC out of range!");
      } 
    } else {
      notify(player, "Invalid number of arguments to function");
    }
  }
}
