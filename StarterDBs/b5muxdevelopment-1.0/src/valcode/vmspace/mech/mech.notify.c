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

void MechBroadcast(mech,target,mech_map,buffer)
struct mech_data *mech, *target;
struct map_struct *mech_map;
char *buffer;
{
  int loop;
  struct mech_data *tempMech;

  if(target) 
    {
      for(loop=0; loop<MAX_MECHS_PER_MAP; loop++) 
	{
	  if(mech_map->mechsOnMap[loop] != mech->mynum 
	     && mech_map->mechsOnMap[loop] != -1
	     && mech_map->mechsOnMap[loop] != target->mynum) 
	    {
	      tempMech=(struct mech_data *) 
		FindObjectsData(mech_map->mechsOnMap[loop]);
	      if(tempMech) 
		{
		  mech_notify(tempMech,MECHSTARTED,buffer);
		}
	    }
	}
    }
  else 
    {
      for(loop=0; loop<MAX_MECHS_PER_MAP; loop++) 
	{
	  if(mech_map->mechsOnMap[loop] != mech->mynum 
	     && mech_map->mechsOnMap[loop] != -1) 
	    {
	      tempMech=(struct mech_data *) 
		FindObjectsData(mech_map->mechsOnMap[loop]);
	      if(tempMech) 
		{
		  mech_notify(tempMech,MECHSTARTED,buffer);
		}
	    }
	}
    }
}

void MechFireBroadcast(mech,target,x,y,mech_map,weapname)
struct mech_data *mech, *target;
int x, y;
struct map_struct *mech_map;
char *weapname;
{
  int loop,attacker,defender;
  float fx, fy, fz;
  int mapx, mapy;
  struct mech_data *tempMech;
  char mechID[2], buff[50];
  char target_name[100];
  char mech_name[100];
  
  if(target) 
    {
      get_mech_atr(mech, mech_name, "MECHNAME");
      get_mech_atr(target, target_name, "MECHNAME");
      
      mapx=target->x;
      mapy=target->y;
      fx=target->fx;
      fy=target->fy;
      fz=target->fz;
      for(loop=0; loop<MAX_MECHS_PER_MAP; loop++) 
	{
	  attacker=0;
	  defender=0;
	  if(mech_map->mechsOnMap[loop] != mech->mynum 
	     && mech_map->mechsOnMap[loop] != -1
	     && mech_map->mechsOnMap[loop] != target->mynum) 
	    {
	      tempMech=(struct mech_data *) 
		FindObjectsData(mech_map->mechsOnMap[loop]);
	      if(tempMech)
		{
		  if(InLineOfSight(tempMech,mech,mech->x,mech->y, 
				   FindRange(tempMech->fx,tempMech->fy,
					     tempMech->fz,
					     mech->fx,mech->fy,
					     mech->fz))) 
		    attacker=1;
		  if(InLineOfSight(tempMech,target,mapx,mapy, 
				   FindRange(tempMech->fx,tempMech->fy,
					     tempMech->fz,fx,fy,fz))) 
		    defender=1;
		  
		  
		  if(tempMech->team == mech->team) 
			{
		      mechID[0]=mech->ID[0]+32;
		      mechID[1]=mech->ID[1]+32;
		    }
		  else 
		    {
		      mechID[0]=mech->ID[0];
		      mechID[1]=mech->ID[1];
		    }
		  
		  if(tempMech->team == target->team) 
		    sprintf(buff,"%s:[%c%c]!", target_name,
			    target->ID[0]+32, target->ID[1]+32);
		  else sprintf(buff,"%s:[%c%c]!", target_name,
			       target->ID[0], target->ID[1]);
		  
		  if(attacker && defender)
		    mech_notify(tempMech,MECHSTARTED,
			   tprintf("%s:[%c%c] fires a %s at %s",
				   mech_name,
				   mechID[0],mechID[1],
				   weapname,buff));
		  else if(attacker && !defender)
		    mech_notify(tempMech,MECHSTARTED,
			   tprintf("%s:[%c%c] fires a %s at something!",
				   mech_name,
				   mechID[0],mechID[1],
				   weapname));
		  else if(!attacker && defender)
		    mech_notify(tempMech,MECHSTARTED,
			   tprintf("Something fires a %s at %s",
				   weapname,buff));
		}  
	    }
	}
    }
  else 
    {
      get_mech_atr(mech, mech_name, "MECHNAME");
      
      mapx=x;
      mapy=y;
      MapCoordToRealCoord(x,y,&fx,&fy);
      if(mech_map->map[y][x].terrain==WATER)
	fz = -ZSCALE*(float)(mech_map->map[x][y].elev);
      else
	fz = ZSCALE*(float)(mech_map->map[x][y].elev);
      sprintf(buff,"hex %d %d!", mapx, mapy);
      for(loop=0; loop<MAX_MECHS_PER_MAP; loop++) 
	{
	  attacker=0;
	  defender=0;
	  if(mech_map->mechsOnMap[loop] != mech->mynum 
	     && mech_map->mechsOnMap[loop] != -1)
	    {
	      tempMech=(struct mech_data *) 
		FindObjectsData(mech_map->mechsOnMap[loop]);
	      if(tempMech)
		{
		  if(InLineOfSight(tempMech,mech,mech->x,mech->y, 
				   FindRange(tempMech->fx,tempMech->fy,
					     tempMech->fz,
					     mech->fx,mech->fy,
					     mech->fz))) 
		    attacker=1;
		  if(InLineOfSight(tempMech,target,mapx,mapy, 
				   FindRange(tempMech->fx,tempMech->fy,
					     tempMech->fz,fx,fy,fz))) 
		    defender=1;
		  
		  if(tempMech->team == mech->team) 
		    {
		      mechID[0]=mech->ID[0]+32;
		      mechID[1]=mech->ID[1]+32;
		    }
		  else 
		    {
		      mechID[0]=mech->ID[0];
		      mechID[1]=mech->ID[1];
		    }
	      
		  if(attacker && defender)
		    mech_notify(tempMech,MECHSTARTED,
			   tprintf("%s:[%c%c] fires a %s at %s",
				   mech_name,
				   mechID[0],mechID[1],
				   weapname,buff));
		  else if(attacker && !defender)
		    mech_notify(tempMech,MECHSTARTED,
			   tprintf("%s:[%c%c] fires a %s at something!",
				   mech_name,
				   mechID[0],mechID[1],
				   weapname));
		  else if(!attacker && defender)
		    mech_notify(tempMech,MECHSTARTED,
			   tprintf("Something fires a %s at %s",
				   weapname,buff));
		}  
	    }
	}
    }
}

void mech_notify(mech,type,buffer)
struct mech_data *mech;
int type;
char *buffer;
{
  if (type == MECHPILOT)
    {
      if (mech->pilot != -1 && !mech->unconcious)
	notify(mech->pilot,buffer);
    }
  else if ((type == MECHALL && !(mech->status & DESTROYED)) || 
	   (type == MECHSTARTED && (mech->status & STARTED)))
    {
      if(mech->unconcious)
	notify_except(db[mech->mynum].contents,mech->pilot,mech->pilot,buffer);
      else
	/*notify_except(db[mech->mynum].contents,NOTHING,NOTHING,buffer);*/
	notify_all_from_inside(mech->mynum,1,buffer);
    }
}

int get_mech_atr(mech, mech_atr, attr)
     struct mech_data *mech;
     char *mech_atr;
     char *attr;
{
  ATTR *typeAttr;
  char *p;
  
  /*typeAttr=atr_str(GOD, mech->mynum, attr);*/
  if(!typeAttr) 
    {
      strcpy(mech_atr, Name(mech->mynum));
      return(0);
    }
  /*else
    {*/
      /*if(p=atr_get(mech->mynum,typeAttr)) 
        {
          strcpy(mech_atr, p);
          return(1);
        }*/
      else 
        {
          strcpy(mech_atr, Name(mech->mynum));
          return(0);
        }
    /*}*/
}



