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

/* Selectors for new/free function */
#define SPECIAL_FREE 0
#define SPECIAL_ALLOC 1

void mech_Rsetxy(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
struct mech_data *mech;
char *args[2];
int x,y;
struct map_struct *mech_map;

if(!Wizard(player) && !(Flags2(player)&CONSOLE)) {
	notify(player,"Restricited Command");
	return; }

    mech=(struct mech_data *) data;
    if(CheckData(player, mech)) {
      mech_map=(struct map_struct *) FindObjectsData(mech->mapindex);
      if(!mech_map) {
	notify(player, "You are on an invalid map! Map index reset!");
	mech_shutdown(player,data,"");
	send_mechinfo(tprintf("Rsetxy:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
	mech->mapindex= -1;
	return;
      }
      if(mech_parseattributes(buffer, args, 2)==2) {
	x=atoi(args[0]);
	y=atoi(args[1]);
	if(mech->mapindex!=-1) 
	  {
	    if(x>(mech_map->map_width-1) || y>(mech_map->map_height-1) 
	       || x<0 || y<0) 
	      {
		notify(player, "Invalid coordinates!");
	      } 
	    else 
	      {
		mech->x= x;
		mech->last_x= x;
		mech->y= y;
		mech->last_y= y;
		MapCoordToRealCoord(mech->x, mech->y, &mech->fx, &mech->fy);
		mech->terrain=mech_map->map[mech->y][mech->x].terrain;
		mech->elev=mech_map->map[mech->y][mech->x].elev;
		if(mech->terrain==WATER) mech->z = -mech->elev;
		else mech->z = mech->elev;
		mech->fz = mech->z * ZSCALE;
		notify(player, tprintf("Pos changed to %d,%d", x,y));
	      }
	  }
	else 
	  {
	    notify(player, "You are not on any map!");
	  }
      } 
      else 
	{
	  notify(player, "Invalid number of arguments to SETXY!");
	}
    }
}

/* Team/Map commands */
void mech_Rsetmapindex(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
struct mech_data *mech;
char *args[1];
int newindex, notdone;
int loop;
struct map_struct *newmap;
struct map_struct *oldmap;
struct mech_data *tempMech;
char targ[2];
  
if(!Wizard(player) && !(Flags2(player)&CONSOLE)) {
	notify(player,"Restricited Command");
	return; }
  
    mech=(struct mech_data *) data;
    if(CheckData(player, mech)) {
      if(mech_parseattributes(buffer, args, 1)==1) {
	newindex=atoi(args[0]);

	newmap=ValidMap(player, newindex);
	if(newmap) {

	  /* Remove the mech from it's old map */
	  if(mech->mapindex != -1) {
	    oldmap=ValidMap(player, mech->mapindex);
	    if(oldmap) {
	      for(loop=0; (loop<MAX_MECHS_PER_MAP) 
		  && (oldmap->mechsOnMap[loop] 
		      != mech->mynum); loop++);
	      
	      if(loop != MAX_MECHS_PER_MAP) {
		oldmap->mechsOnMap[loop]= -1; /* clear it */
	      }
	    }
	  }
          
          for(loop=0; loop<MAX_MECHS_PER_MAP; loop++)
            if(newmap->mechsOnMap[loop] == mech->mynum) 
              {
                notify(player, "You're already on that map!");
		notify(player, "Setting your map index number to it.");
		mech->mapindex = newindex;
                return;
              }
          
	  /* Find a clear spot for this mech */
	  if (mech->team != 0) targ[0] = 64+mech->team;
	  else targ[0] = 'Z';
	  targ[1]='A';
          notdone=1;
          while (notdone)
            {
              notdone = 0;
              for(loop=0; (loop<MAX_MECHS_PER_MAP) 
                  && (newmap->mechsOnMap[loop] != -1); loop++)
                {
                  tempMech=(struct mech_data *) 
                    FindObjectsData(newmap->mechsOnMap[loop]);
                  if (tempMech->ID[0] == targ[0] && 
                      tempMech->ID[1] == targ[1]) 
                    {
                      targ[1]++;
                      notdone = 1;
                    }
                  if(mech->ID[1] > 90) 
                    {
                      targ[1]='A';
                      targ[0]++;
                      notdone = 1;
                    }
                }
            }
	  if(loop != MAX_MECHS_PER_MAP) {
	    mech->ID[0]=targ[0];
	    mech->ID[1]=targ[1];
	    mech->mapindex=newindex;
	    newmap->mechsOnMap[loop]=mech->mynum;
	    newmap->LOSinfo[0][0] = 1;
	    mech->mapnumber = loop;
	    if(mech->x > (newmap->map_width-1) || 
			mech->y > (newmap->map_height-1) )
	    {
		mech->x= 0;
		mech->last_x= 0;
		mech->y= 0;
		mech->last_y= 0;
		MapCoordToRealCoord(mech->x, mech->y, &mech->fx, &mech->fy);
		mech->terrain=newmap->map[mech->y][mech->x].terrain;
		mech->elev=newmap->map[mech->y][mech->x].elev;
		notify(player, 
	"You're current position is out of bounds, Pos changed to 0,0");
	    }
	    notify(player, tprintf("MapIndex changed to %d", newindex));
	    notify(player, tprintf("Your ID: %c%c", mech->ID[0], mech->ID[1]));
	  } else {
	    notify(player, "There are too many mechs on that map!");
	    mech->mapindex= -1;
	  }
	} 
      } else {
	notify(player, "Invalid number of arguments to SETMAPINDX!");
      }
    }
}

void mech_Rsetteam(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1];
  int team, loop, notdone;
  struct mech_data *tempMech;
  struct map_struct *newmap;

if(!Wizard(player) && !(Flags2(player)&CONSOLE)) {
	notify(player,"Restricited Command");
	return; }
    mech=(struct mech_data *) data;
    if(CheckData(player, mech)) {
      if (mech->mapindex == -1)
        {
          notify(player, "Mech is not on a map:  Can't set team");
          return;
        }
      newmap=ValidMap(player, mech->mapindex);
      if(!newmap)
        {
          notify(player, "Map index reset!");
          mech->mapindex = -1;
          return;
        }
      
      if(mech_parseattributes(buffer, args, 1)==1) {
	team=atoi(args[0]);
	if (team < 1) team = 1;
	if (team > 26) team = 26;
	mech->team=team;
	mech->ID[0]=team+64;
        mech->ID[1]='A';
        notdone = 1;
        while (notdone)
          {
            notdone = 0;
            for(loop=0; (loop<MAX_MECHS_PER_MAP) 
                && (newmap->mechsOnMap[loop] != -1); loop++)
              {
                tempMech=(struct mech_data *) 
                  FindObjectsData(newmap->mechsOnMap[loop]);
                if (mech->mynum != tempMech->mynum &&
                    tempMech->ID[0] == mech->ID[0] && 
                    tempMech->ID[1] == mech->ID[1]) 
                  {
                    mech->ID[1]++;
                    notdone = 1;
                  }
                if(mech->ID[1] > 90) 
                  {
                    mech->ID[1]='A';
                    mech->ID[0]++;
                    notdone = 1;
                  }
              }
          }
	notify(player, tprintf("Team set to %d", team));
	notify(player, tprintf("ID set to %c%c", mech->ID[0],mech->ID[1]));
      } else {
	notify(player, "Invalid number of arguments!");
      }
    }
}


/* This is a manual update, it's out of date now... */
void mech_Rupdate(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  /* Update position and all that... */
  float newx, newy;
  struct mech_data *mech;
  char buff[10];
  struct map_struct *mech_map;

if(!Wizard(player) && !(Flags2(player)&CONSOLE)) {
	notify(player,"Restricited Command");
	return; }
    mech=(struct mech_data *) data;
    if(CheckData(player, mech)) {
      mech_map=(struct map_struct *) FindObjectsData(mech->mapindex);
      if(!mech_map) {
	mech_notify(mech,MECHPILOT,
			"You are on an invalid map! Map index reset!");
	mech_shutdown(mech->pilot,data,"");
	send_mechinfo(tprintf("Rupdate:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
	mech->mapindex= -1;
	return;
      }
      newx = (mech->speed * fcos((float) TWOPIOVER360*mech->facing));
      newy = (mech->speed * fsin((float) TWOPIOVER360*mech->facing));
      newy = -newy;
      sprintf(buff, "%.2f,%.2f", newx, newy);
      notify(player, tprintf("Position changed by deltas x,y: %s meters",
			     buff));
      mech->fx+=newx;
      mech->fy+=newy;

      mech->last_x=mech->x;
      mech->last_y=mech->y;
      RealCoordToMapCoord(&mech->x, &mech->y, mech->fx, mech->fy);
      
      mech->terrain=mech_map->map[mech->y][mech->x].terrain;
      mech->elev=mech_map->map[mech->y][mech->x].elev;

      if(mech->last_x!=mech->x || mech->last_y!=mech->y) 
	NewHexEntered(mech,mech_map,newx,newy);
    }
}

#define SPECIAL_FREE 0
#define SPECIAL_ALLOC 1

/* Alloc/free routine */
void newfreemech(key,data,selector)
dbref key; 
void **data; 
int selector; 
{
  struct mech_data *new;
  struct map_struct *map;
  int i;

  switch(selector) {
  case SPECIAL_ALLOC:
    new=(struct mech_data *) malloc(sizeof(struct mech_data));
    if(new) {
      send_mechinfo("Allocated successfully");
      new->type=CLASS_MECH;
      new->move=MOVE_BIPED;
      new->mynum=key;
      new->mapnumber= 1;
      new->tons= 0;
      new->pilot= -1;
      new->pilotstatus=0;
      new->pilotskill = 5;
      new->gunneryskill = 4;
      new->unconcious=0;
      new->mapindex= -1;
      new->ID[0]=' ';
      new->ID[1]=' ';
      new->aim = NUM_SECTIONS;
      new->brief = 1;
      new->pilotskillbase=0;
      new->turndamage=0;
      new->team= 0;  
      new->terrain=' ';
      new->elev=0;
      new->facing=0;
      new->desiredfacing=0;
      new->heat=0.0;
      new->plus_heat=0.0;
      new->minus_heat=0.0;
      new->numsinks=0; 
      new->weapheat=0.0;
      new->status=TORSO_NORMAL;
      new->engineheat=0;
      new->basetohit=0;
      new->critstatus=0;
      new->specials=0;
      new->startfx= 0.0;
      new->startfy= 0.0;
      new->jumplength= 0.0;
      new->goingx= 0;
      new->goingy= 0;
      new->x=0;
      new->y=0;
      new->z=0;
      new->last_x=0;
      new->last_y=0;
      new->fx=0.0;
      new->fy=0.0;
      new->fz=0.0;
      
      new->verticalspeed=0.0;
      new->speed=0.0;
      new->desired_speed=0.0;
      new->maxspeed=0.0;

      new->jumpspeed=0.0;
      new->jumpheading=0;

      new->turretfacing=0;

      new->chgtarget= -1;
      new->dfatarget= -1;
      new->target= -1;
      new->targx= -1;
      new->targy= -1;
      for(i=0; i<NUM_SECTIONS; i++) {
	new->sections[i].armor=0;
	new->sections[i].internal=0;
	new->sections[i].rear=0;
	new->sections[i].basetohit=0;
	new->sections[i].config=0;
        new->sections[i].recycle = 0;
	FillDefaultCriticals(new, i);
      }
      for(i=0; i<NUM_TICS; i++) new->tic[i]=0;
      new->clock=0;
      new->mech_type[0]=0;

      *data=new;
    } else {
      send_mechinfo("Special_ALLOC failed");
    }
    break;

  case SPECIAL_FREE:
    if(*data) 
      {
	new = (struct mech_data *) *data;
	if (new->mapindex != -1 && 
	    (map = (struct map_struct *) FindObjectsData(new->mapindex)))
	  for(i=0; i<MAX_MECHS_PER_MAP; i++) 
	    {
	      if (map->mechsOnMap[i] == new->mynum)
		map->mechsOnMap[i] = -1;
	    }	    
	free(*data);
	send_mechinfo("Free successful");
      } else {
	send_mechinfo("Special Free failed");
      }
    break;
  }
}

