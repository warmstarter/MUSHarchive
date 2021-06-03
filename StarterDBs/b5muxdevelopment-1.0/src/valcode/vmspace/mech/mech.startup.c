#ifdef TEST
typedef int dbref;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "config.h"
/*#include "externs.h"
#include "db.h"
*/
#include "../header.h"
#include "mech.h"

void mech_startup(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  dbref newpilot;

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) 
    {
      newpilot=player;
      /*if(newpilot>0 && (newpilot < mudstate.db_top) && Alive(newpilot)) */
      if(newpilot>0 && (newpilot < mudstate.db_top))
	{
	  /*if (!(db[mech->mynum].flags & IN_CHARACTER) ||
	      (db[newpilot].flags & IN_CHARACTER))*/
	  if (1==1)
	    {
	      if(mech->mapindex!=-1) 
		{
		  if(!(mech->status & DESTROYED)) 
		    {
		      if(!(mech->status & STARTED)) 
			{
			  mech->pilot=newpilot;
			  notify(newpilot, "Startup Cycle commencing...");
			  /* use this. (hack) */
			  mech->goingx=(mech->clock+STARTUP_TIME) % MAX_CLOCK;
			  mech->jumpheading=0;
			  mech->sections[RLEG].recycle = 0;
			  mech->sections[LLEG].recycle = 0;
			  mech->sections[RARM].recycle = 0;
			  mech->sections[LARM].recycle = 0;
			}
		      else 
			{
			  notify(player, "The 'Mech is already started!");
			}
		    }
		  else
		    {
		      notify(player, "This 'Mech is destroyed!");
		    }
		}
	      else 
		{
		  notify(player, "You are not on any map!");
		}
	    }
	  else
	    {
	      notify(player,"You cannot start up an in-character mech while out of character!");
	      send_mechinfo(tprintf("startup: OOC player %s attempted to start up IC mech #%d (%)s.",Name(player),mech->mynum,Name(mech->mynum)));
	    }
	}
      else 
	{
	  notify(player, "That is not a valid player!");
	}
    } 
}

void mech_leave_map(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  struct mech_map *mechmap;
  int x,y,z,bayid;
  char *bay,buff[200];
  
  mech=(struct mech_data *) data;
if(CheckData(player, mech)) {

  if(mech->mapindex <=0 || mech->mapindex > mudstate.db_top) {
	notify(player,"You are on an Invalid Map!");
	return;
	}
 x=mech->x;
 y=mech->y;
 z=mech->z;
 sprintf(buff,"BAY-%d-%d-%d\0",x,y,z);
 bay=vfetch_attribute(mech->mapindex,buff);
 bayid=atoi(bay);
 if(bayid<=0) {
	notify(player,"You are not in a valid bay square");
	return;
	} 

      if (player != -1) mech_shutdown(player,data,buffer);
	
 if(bayid>0 && bayid <=mudstate.db_top) {
		move_via_generic(mech->mynum,bayid,NOTHING,0);
		mech->mapindex=-1;
	}
 else notify(player,"Invalid Bay DB#");

}
}


void mech_shutdown(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) 
    {
      if(mech->pilot != -1) 
	{
	  mech_notify(mech,MECHSTARTED, "Mech has been shutdown!");
	  if(mech->status & TORSO_RIGHT) 
	    {
	      mech_notify(mech,MECHSTARTED, 
			  "Torso rotated back to center for shutdown");
	      mech->status &= ~TORSO_RIGHT;
	    } 
	  else if(mech->status & TORSO_LEFT) 
	    {
	      mech_notify(mech,MECHSTARTED, 
			  "Torso rotated back to center for shutdown");
	      mech->status &= ~TORSO_LEFT;
	    }
	  mech->status |= LANDED;
	  mech->status &= ~JUMPING;
	  if(mech->terrain==WATER)
	    mech->z = -mech->elev;
	  else
	    mech->z = mech->elev;
	  mech->fz = ZSCALE*mech->z;
	  mech->status&=~STARTED;
	  mech->pilot= -1;
	  mech->speed = 0.0;
	  mech->desired_speed=0.0;
	  mech->verticalspeed=0.0;
	  mech->target= -1;
	  mech->desiredfacing=mech->facing;
	  mech->clock=0;
	  mech->goingx=-1;
	} 
      else if (player != -1) 
	{
	  notify(player, "Mech not yet started!");
	}
    }
}



