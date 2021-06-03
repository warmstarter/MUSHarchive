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

int AddTerrainMod(mech,target,mech_map,hexRange,sight)
struct mech_data *mech; 
struct mech_data *target; 
struct map_struct *mech_map; 
float hexRange;
int sight;
{
  float pos_x, pos_y, pos_z, end_x, end_y, x_inc, y_inc, z_inc;
  int last_hex_x, last_hex_y, curr_hex_x, curr_hex_y;
  int reached_endpoint, height, woods_count, targetz, mechz;
  int baseToHit=0;
  char terrain;
  int ma, mb, LOS;
  int numHexes;
  char mech_name[100];
  
  /* (mech->x,mech->y) are the starting hex coords, (target->x,target->y) 
     are the ending coordinates
  
     Given the endpoints, find the path, then trace along it from 
     mech to target. Stop, and return false, if the goal is reached.
  */

  ma = mech->mapnumber;
  mb = target->mapnumber;
  if ((LOS=mech_map->LOSinfo[ma][mb]) > 20)
  {
      if (LOS > 80)
      {
	  target->status|=PARTIAL_COVER;
	  mech_notify(mech, MECHALL, "Your target takes partial cover.");
	  return LOS-85;
      }
      else if (LOS > 50)
      {
	  target->status|=PARTIAL_COVER;
	  mech_notify(mech, MECHALL, 
			"Your target takes partial cover in the water.");
	  return LOS-55;
      }
      else
      {
	  target->status&=~PARTIAL_COVER;
	  return LOS-25;
      }
  }

  /* Find path parameters*/
  MapCoordToRealCoord(target->x, target->y, &end_x, &end_y);
  hexRange = FindHexRange(mech->fx,mech->fy,target->fx,target->fy);
  pos_x = mech->fx;
  pos_y = mech->fy;
  if(mech->type == CLASS_MECH && !(mech->status & FALLEN))
    pos_z = mech->fz + ZSCALE/2 + ZSCALE;
  else 
    pos_z = mech->fz + ZSCALE/2;
  
  if(hexRange > 1.0) 
    {
      hexRange=(float)((int)(hexRange + 0.5));
      x_inc=(end_x - pos_x)/hexRange;
      y_inc=(end_y - pos_y)/hexRange;
      if(target->type == CLASS_MECH && !(target->status & FALLEN))
	z_inc=(target->fz + ZSCALE/2 + ZSCALE - pos_z)/hexRange;
      else
	z_inc=(target->fz + ZSCALE/2 - pos_z)/hexRange;
    }
  else 
    {
      return(0);
    }
  
  reached_endpoint = 0;

  woods_count = 0;
  
  if(target->type==CLASS_MECH && !(target->status & FALLEN)) 
    targetz = target->z + 1;
  else 
    targetz = target->z;

  if(mech->type==CLASS_MECH && !(mech->status & FALLEN)) 
    mechz = mech->z + 1;
  else 
    mechz = mech->z;

  if(target->terrain==LIGHT_FOREST && 
     targetz < (target->elev + 2)) 
  {
       baseToHit++;
       woods_count++;
  }
  else if(target->terrain==HEAVY_FOREST && 
	  targetz  < (target->elev + 2)) 
  {
       baseToHit+=2;
       woods_count+=2;
  }

  if(target->terrain==WATER && targetz == 0) 
    baseToHit++;

  if(mech->terrain==WATER && mechz == 0) 
    baseToHit--;
  
  RealCoordToMapCoord(&last_hex_x, &last_hex_y, pos_x, pos_y);
  
  pos_x += x_inc;
  pos_y += y_inc;
  pos_z += z_inc;
  
  RealCoordToMapCoord(&curr_hex_x, &curr_hex_y, pos_x, pos_y);
  if(curr_hex_x > mech_map->map_width-1) curr_hex_x=mech_map->map_width-1;
  if(curr_hex_y > mech_map->map_height-1) curr_hex_y=mech_map->map_height-1;
  if(curr_hex_x < 0) curr_hex_x = 0;
  if(curr_hex_y < 0) curr_hex_y = 0;

  reached_endpoint = ((curr_hex_x==target->x) && (curr_hex_y==target->y));
  
  numHexes=0;
  while (!reached_endpoint && numHexes < hexRange) 
    {
      /* only check if this is a new intervening hex */
      if ((curr_hex_x != last_hex_x) || (curr_hex_y != last_hex_y)) 
        {
          terrain = mech_map->map[curr_hex_y][curr_hex_x].terrain;
          
          /* get the current height */
          height = mech_map->map[curr_hex_y][curr_hex_x].elev;
	  if(terrain==WATER) height= -height;
          
          /* keep track of how many wooded hexes we cross */
          if ((pos_z/ZSCALE) < (float)(height + 2)) 
            {
              if ((terrain == LIGHT_FOREST) || (terrain == HEAVY_FOREST))
                {
                  woods_count += (terrain == LIGHT_FOREST) ? 1 : 2;
                  baseToHit += (terrain == LIGHT_FOREST) ? 1 : 2;
                }
            }
          
          /* make this the new 'current hex' */
          last_hex_x = curr_hex_x;
          last_hex_y = curr_hex_y;
        }
      pos_x += x_inc;
      pos_y += y_inc;
      pos_z += z_inc;
      RealCoordToMapCoord(&curr_hex_x, &curr_hex_y, pos_x, pos_y);
      if(curr_hex_x > mech_map->map_width-1) curr_hex_x=mech_map->map_width-1;
      if(curr_hex_y > mech_map->map_height-1) curr_hex_y=mech_map->map_height-1;
      if(curr_hex_x < 0) curr_hex_x = 0;
      if(curr_hex_y < 0) curr_hex_y = 0;

      reached_endpoint = ((curr_hex_x==target->x) && (curr_hex_y==target->y));
      numHexes++;
    }

  /* Partial Cover */
  if(mech_map->map[last_hex_y][last_hex_x].elev == target->elev + 1 
     && mechz <= targetz && targetz == target->elev + 1) 
  {
    get_mech_atr(mech, mech_name, "MECHNAME");

    if(!sight) mech_notify(target,MECHALL,
                tprintf("You take partial cover from %s:[%c%c].",
                        mech_name,
                        mech->ID[0],
                        mech->ID[1]));
    mech_notify(mech, MECHALL, "Your target takes partial cover.");
    baseToHit+=3;
    target->status|=PARTIAL_COVER;
    LOS = baseToHit+85;
  }
  else if(target->terrain==WATER && target->elev==1 
	  && targetz==0)
  {
    get_mech_atr(mech, mech_name, "MECHNAME");

    if(!sight) mech_notify(target,MECHALL, 
		tprintf("You take partial cover from %s:[%c%c] in water.", 
			mech_name,
			mech->ID[0],
			mech->ID[1]));
    mech_notify(mech, MECHALL, 
		"Your target takes partial cover in the water.");
    baseToHit+=3;  /* works out to +2 */
    target->status|=PARTIAL_COVER;
    LOS = baseToHit+55;
  }
  else 
  {
    target->status&=~PARTIAL_COVER;
    LOS = baseToHit+25;
  }

  mech_map->LOSinfo[ma][mb] = LOS;
  mech_map->LOSinfo[0][0] = 2;
  
  return (baseToHit);  
}

void clear_LOSinfo(key,data)
dbref key;
void *data;
{
struct map_struct *mech_map;
int i,nummechs;

    mech_map = (struct map_struct *)data;
    if (data && mech_map->LOSinfo[0][0])
    {
	nummechs = 0;
	for (i=0; i<MAX_MECHS_PER_MAP; i++)
	    if (mech_map->mechsOnMap[i] != -1)
		nummechs = i+1;

	for (i=0; i<nummechs; i++)
	    bzero(mech_map->LOSinfo[i],nummechs);
    }
}

int InLineOfSight(mech,target,x,y,hexRange)
     struct mech_data *mech, *target;
     int x, y;
     float hexRange;
{
  struct map_struct *mech_map;
  float pos_x, pos_y, pos_z, end_x, end_y, x_inc, y_inc, z_inc, limit;
  int last_hex_x, last_hex_y, curr_hex_x, curr_hex_y, view_not_blocked,
    reached_endpoint, height, woods_count;
  char terrain;
  int ma,mb,mechz,targetz;
  int numHexes;
  
  mech_map=(struct map_struct *)FindObjectsData(mech->mapindex);
  if(!mech_map) {
    mech_notify(mech,MECHPILOT,"You are on an invalid map! Map index reset!");
    send_mechinfo(tprintf("InLineOfSight:invalid map:Mech %d  Index %d",
					    mech->mynum,mech->mapindex));
    mech->mapindex= -1;
    return;
  }


  if(target)
    {
      if(target->z >= 15 || mech->z >= 15) limit = 70.0;
      else limit = 35.0;
      
      ma = mech->mapnumber;
      mb = target->mapnumber;
      if (mech_map->LOSinfo[ma][mb])
	{
	  return (mech_map->LOSinfo[ma][mb]-1);
	}
    }
  else
    {
      if(mech->z >= 15) limit = 70.0;
      else limit = 35.0;
      if (x < 0 || y < 0 || x >=mech_map->map_width || y >=mech_map->map_height)
	return 0;
    }

  if(hexRange > limit) return 0;

  /* (mech->x,mech->y) are the starting hex coords, (x,y) are the ending
     coordinates
  
     Given the endpoints, find the path, then trace along it from 
     mech to target. Stop, and return false, if the goal is reached.
  */

  /* Find path parameters*/
  if(target) 
    {
      end_x = target->fx;
      end_y = target->fy;
      hexRange = FindHexRange(mech->fx,mech->fy,target->fx,target->fy);
    }
  else 
    {
      MapCoordToRealCoord(x, y, &end_x, &end_y);
      hexRange = FindHexRange(mech->fx,mech->fy,end_x,end_y);
    }
  
  pos_x = mech->fx;
  pos_y = mech->fy;
  if(mech->type == CLASS_MECH && !(mech->status & FALLEN))
    pos_z = mech->fz + ZSCALE/2 + ZSCALE;
  else 
    pos_z = mech->fz + ZSCALE/2;
  
  if(hexRange > 1.0) 
    {
      hexRange=(float)((int)(hexRange + 0.5));
      x_inc=(end_x - pos_x)/hexRange;
      y_inc=(end_y - pos_y)/hexRange;
      if(target)
	{
	  if(target->type == CLASS_MECH && !(target->status & FALLEN))
	    z_inc=(target->fz + ZSCALE/2 + ZSCALE - pos_z)/hexRange;
	  else
	    z_inc=(target->fz + ZSCALE/2 - pos_z)/hexRange;
	}
      else
	{
	  if(mech_map->map[y][x].terrain==WATER) 
	    z_inc = -((float)mech_map->map[y][x].elev*ZSCALE + pos_z)/hexRange;
	  else 
	    z_inc = ((float)mech_map->map[y][x].elev*ZSCALE - pos_z)/hexRange;
	}
    }
  else 
    {
      return(1);
    }
  
  view_not_blocked = 1;
  reached_endpoint = 0;

  if(target)
    {
      if(target->type==CLASS_MECH && !(target->status & FALLEN)) 
        targetz = target->z + 1;
      else 
        targetz = target->z;
      /* special check for submerged mechs */
      if(targetz < 0) return(0);
     }

  if(mech->type==CLASS_MECH && !(mech->status & FALLEN)) 
    mechz = mech->z + 1;
  else 
    mechz = mech->z;

  /* special check for submerged mechs */
  if(mechz < 0) return(0);
  

  /* Search the line of sight for blocks */
  /* Line-Of-Sight Determination:

     1) Determination is made from current hex to target hex.
     2) A 'Mech is one elevation higher than it's terrain.
    *3) If intervening terrain is higher than attacking and defending
        mechs, los is blocked.
    *4) Three wooded intervening hexes, or two heavily wooded constitute
        a blocked los.
    *5) A 'Mech in a  water hex of depth two or more has a blocked los.
  
     if either the mech or its target are in water of depth >= 2,
     we do not have los.
  */
 
  RealCoordToMapCoord(&last_hex_x, &last_hex_y, pos_x, pos_y);
  
  pos_x += x_inc;
  pos_y += y_inc;
  pos_z += z_inc;
  
  RealCoordToMapCoord(&curr_hex_x, &curr_hex_y, pos_x, pos_y);
  if(curr_hex_x > mech_map->map_width-1) curr_hex_x = mech_map->map_width-1;
  if(curr_hex_y > mech_map->map_height-1) curr_hex_y = mech_map->map_height-1;
  if(curr_hex_x < 0) curr_hex_x = 0;
  if(curr_hex_y < 0) curr_hex_y = 0;

  reached_endpoint = ((curr_hex_x == x) && (curr_hex_y == y));

  woods_count = 0;
  numHexes=0;
  while (view_not_blocked && !reached_endpoint && numHexes < hexRange) 
    {
    /*
      sprintf(buff, "fx, fy, fz: %.1f, %.1f, %.1f  x, y, z: %d, %d, %d",
	      pos_x, pos_y, pos_z, curr_hex_x, curr_hex_y, (int)(pos_z/ZSCALE));
      send_mechinfo(buff);
    */
      /* only check if this is a new intervening hex */
      if ((curr_hex_x != last_hex_x) || (curr_hex_y != last_hex_y)) 
        {
	  terrain = mech_map->map[curr_hex_y][curr_hex_x].terrain;
	  
	  /* get the current height */
	  height = mech_map->map[curr_hex_y][curr_hex_x].elev;
	  if(terrain==WATER) height= -height;
          
	  /* keep track of how many wooded hexes we cross */
	  if ((pos_z/ZSCALE) < (float)(height + 2))
            {
              if ((terrain == LIGHT_FOREST) || (terrain == HEAVY_FOREST))
                {
                  woods_count += (terrain == LIGHT_FOREST) ? 1 : 2;
                }
            }
          
          view_not_blocked = (woods_count < 3) && 
	    ((float)(height) < (pos_z/ZSCALE));
          
          /* make this the new 'current hex' */
          last_hex_x = curr_hex_x;
          last_hex_y = curr_hex_y;
        }
      pos_x += x_inc;
      pos_y += y_inc;
      pos_z += z_inc;
      RealCoordToMapCoord(&curr_hex_x, &curr_hex_y, pos_x, pos_y);
      if(curr_hex_x > mech_map->map_width-1) curr_hex_x = mech_map->map_width-1;
      if(curr_hex_y > mech_map->map_height-1) curr_hex_y = mech_map->map_height-1;
      if(curr_hex_x < 0) curr_hex_x = 0;
      if(curr_hex_y < 0) curr_hex_y = 0;
      
      reached_endpoint = ((curr_hex_x == x) && (curr_hex_y == y));
      numHexes++;
    }
  if (target)
    {
      mech_map->LOSinfo[ma][mb] = view_not_blocked+1;
      mech_map->LOSinfo[0][0] = 2;
    }
  return (view_not_blocked);  
}

