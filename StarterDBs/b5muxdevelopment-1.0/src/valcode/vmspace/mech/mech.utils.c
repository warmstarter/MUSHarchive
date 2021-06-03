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

struct map_struct *ValidMap(player,map)
dbref player; 
dbref map; 
{
  struct map_struct *returnValue=NULL;
  ATTR *typeAttr;
  char *xc;
  
  if(map >=0 && map < mudstate.db_top) {
    /*typeAttr=atr_str(GOD, map, "XTYPE");*/
    xc=vfetch_attribute(map,"XTYPE");
    /*if(typeAttr) {*/
    if(xc) {
      /*if(strcmp("MAP", atr_get(map, typeAttr)) ==0 ) {*/
      if(strcmp("MAP", xc) ==0 ) {
	returnValue=(struct map_struct *) FindObjectsData(map); 
	if(!returnValue) {
	  notify(player, "The map has not been allocated!!");
	}
      } else {
	notify(player, "That is not a valid map!");
      }
    } else {
      notify(player, "That is not a valid map! (no XTYPE!)");
    }
  } else {
    notify(player, "Index out of range!");
  }
  
  return (returnValue);
}

dbref FindTargetDBREFFromMapNumber(mech,mapnum)
struct mech_data *mech; 
char *mapnum; 
{
  int loop;
  struct mech_data *tempMech;
  dbref returnValue= -1;
  struct map_struct *map;

  if(mech->mapindex!= -1) {
    map=(struct map_struct *) FindObjectsData(mech->mapindex);
    if(!map) {
      send_mechinfo(tprintf("FTDBREFFMN:invalid map:Mech: %d  Index: %d",
					    mech->mynum,mech->mapindex));
      mech->mapindex= -1;
      return (returnValue);
    }

    for(loop=0; loop<MAX_MECHS_PER_MAP; loop++) {
      if(map->mechsOnMap[loop] != mech->mynum 
	 && map->mechsOnMap[loop] != -1) {
	tempMech=(struct mech_data *) 
	  FindObjectsData(map->mechsOnMap[loop]);
	if(tempMech) {
          if(!strncasecmp(tempMech->ID, mapnum,2)) {
            returnValue=tempMech->mynum;
            break; /* out of for loop */
	  }
	}
      }
    }
  }
  return (returnValue);
}

void FindComponents(magnitude,degrees,x,y)
float magnitude, *x, *y;
int degrees; 
{
  *x=magnitude*fcos((float)(TWOPIOVER360*(degrees+90)));
  *y=magnitude*fsin((float)(TWOPIOVER360*(degrees+90)));
  *x = -(*x);  /* because 90 is to the right */
  *y = -(*y);  /* because y increases downwards */
}

void CheckEdgeOfMap(mech)
struct mech_data *mech; 
{
  int pinned=0;
  struct map_struct *mech_map;

  mech_map=(struct map_struct *) FindObjectsData(mech->mapindex);
  if(!mech_map) {
    mech_notify(mech,MECHPILOT, "You are on an invalid map! Map index reset!");
    mech_shutdown(mech->pilot,(void *)mech,"");
    send_mechinfo(tprintf("CheckEdgeofMap:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
    mech->mapindex= -1;
    return;
  }
  /* Prevents you from going off the map */
  /* Eventually this could wrap and all that.. */
  if(mech->x <0 ) {
    mech->x=0;
    pinned=1;
  } else if(mech->x > mech_map->map_width - 1) {
    mech->x=mech_map->map_width - 1;
    pinned=1;
  }
  
  if(mech->y < 0) {
    mech->y=0;
    pinned=1;
  } else if(mech->y > mech_map->map_height - 1) {
    mech->y=mech_map->map_height - 1;
    pinned=1;
  }

  if(pinned) {
    mech_notify(mech,MECHALL, "You cannot move off the map! (Yet)");
    MapCoordToRealCoord(mech->x,mech->y,&mech->fx,&mech->fy);
    if(mech->status & JUMPING) LandMech(mech);
  }
}

int FindBearing(x0,y0,x1,y1)
float x0, y0, x1, y1; 
{
  float deltax, deltay;
  float temp, rads;
  int degrees;

  deltax=(x0-x1);
  deltay=(y0-y1);
  if(deltax==0.0) deltax=0.000001; /* Avoid division by zero */
  temp=deltay/deltax;
  if(temp<0.0) temp= -temp;
  rads=fatan(temp);
  degrees=(int)(rads*360.0/6.283185307);
  if(deltax<0.0 && deltay<0.0) {
    degrees+=180;
  } else if(deltax<0.0) {
    degrees=180-degrees;
  } else if(deltay<0.0) {
    degrees=360-degrees;
  }
  degrees=degrees - 90;
  if(degrees>=360) degrees-=360;
  if(degrees<0) degrees+=360;
  return (degrees);
}

int InWeaponArc(mech,x,y)
     struct mech_data *mech;
     float x,y; 
{
  int temp;
  int bearingToTarget;
  int returnValue=NOARC;
  int mechfacing;
  
  bearingToTarget=FindBearing(mech->fx, mech->fy, x, y);
  mechfacing=mech->facing;
  
  if(mech->type==CLASS_MECH) 
    {
      if(mech->status & TORSO_RIGHT) 
	{
	  mechfacing+=60;
	  if(mechfacing<0) mechfacing+=360;
	} 
      else if(mech->status & TORSO_LEFT) 
	{
	  mechfacing-=60;
	  if(mechfacing>=360) mechfacing-=360;
	}
      temp=mechfacing - bearingToTarget;
      
      if (abs(temp) >= 300 || abs(temp)<= 60) 
	returnValue = FORWARDARC;
      else if (temp <= -240 || (temp >= 60 && temp <= 120))
	returnValue = LARMARC;
      else if ((temp <= -60 && temp >= -120) || temp >= 240)
	returnValue = RARMARC;
      else if (abs(temp) >= 150 && abs(temp) <= 210)
	returnValue = REARARC;
    } 
  else 
    {
      temp=mechfacing - bearingToTarget;
      
      if (abs(temp) >= 300 || abs(temp)<= 60)
	returnValue = FORWARDARC;
      else if (temp <= -240 || (temp >= 60 && temp <= 120))
	returnValue = LSIDEARC;
      else if ((temp <= -60 && temp >= -120) || temp >= 240)
	returnValue = RSIDEARC;
      else if (abs(temp) >= 120 && abs(temp) <= 240)
	returnValue = REARARC;
      if(mech->type!=CLASS_VEH_VTOL 
	 && mech->sections[TURRET].internal) /* We have a turret */
	{
	  temp=mech->turretfacing - bearingToTarget;
	  
	  if (abs(temp) >= 300 || abs(temp)<= 60)
	    returnValue += TURRETARC;
	}
    }
  return (returnValue);
}

int FindBaseToHitByRange(weapindx,frange)
int weapindx;
float frange; 
{
  int returnValue=50;
  int range;
  
  range=(int)(frange + 0.95);
  if(range> MechWeapons[weapindx].longrange) {
    returnValue=1000; /* can't hit! */
  } else if(range > MechWeapons[weapindx].medrange) {
    /* Long range... */
    returnValue=4;
  } else if(range > MechWeapons[weapindx].shortrange) {
    /* Medium range */
    returnValue=2;
  } else if(range > MechWeapons[weapindx].min) {
    /* Short range... */
    returnValue=0;
  } else {
    /* Less than or equal to minimum range */
    int temp;

    temp=MechWeapons[weapindx].min-range+1;
    
    returnValue=temp;
  }
  return (returnValue);
}

int FindPilotPiloting(mech)
struct mech_data *mech; 
{
ATTR *typeAttr;
int g,p;
/*
  if (db[mech->mynum].flags & QUIET)
   if (typeAttr = (atr_str(GOD, mech->mynum, "MECHSKILLS")))
	if (sscanf(atr_get(mech->mynum,typeAttr),"%d %d",&p,&g) == 2)
	    return p;
*/
  /*if(mech->pilot != -1)
    if(db[mech->pilot].flags & REGISTERED)
      return(char_getskilltarget(mech->pilot, "Piloting-Battlemech", 0));
*/
  return(mech->pilotskill);
}

int FindPilotGunnery(mech)
struct mech_data *mech; 
{
ATTR *typeAttr;
int g,p;
/*
  if (db[mech->mynum].flags & QUIET)
    if (typeAttr = (atr_str(GOD, mech->mynum, "MECHSKILLS")))
	if (sscanf(atr_get(mech->mynum,typeAttr),"%d %d",&p,&g) == 2)
	    return g;
*/
 /* if(mech->pilot != -1)
    if(db[mech->pilot].flags & REGISTERED)
      return(char_getskilltarget(mech->pilot, "Gunnery-Battlemech", 0));
*/
  return(mech->gunneryskill);
}

int MadePilotSkillRoll(mech,mods)
     struct mech_data *mech;
     int mods;
{
  int roll, roll_needed;
  
  if(mech->status & FALLEN) return(1);
  if(mech->unconcious || !(mech->status & STARTED)) return(0);

  roll=Roll();
  roll_needed=FindPilotPiloting(mech) + mods + mech->pilotskillbase;
  
  mech_notify(mech,MECHPILOT, "You make a piloting skill roll!");
  mech_notify(mech,MECHPILOT, tprintf("Modified Pilot Skill: %d  \tRoll: %d",
				 roll_needed,roll));
  if(roll >= roll_needed) 
    return(1);
  else return(0);
}

int Roll() {
  int returnValue;

  returnValue=((random() % 6) + (random() % 6) + 2);

  return(returnValue);
}

void FindXY(x0,y0,bearing,range,x1,y1) 
float x0, y0, range, *x1, *y1;
int bearing; 
{
  float returnValue;
  float xscale, correction;

  correction=(float)(bearing % 60)/60.0;
  if(correction>0.5) correction=1.0-correction;
  correction=correction*2.0;     /* 0 - 1 correction */
  xscale=(1.0 + XSCALE*correction)*SCALEMAP;
  *y1 = y0 - cos((float)bearing*6.283185307/360.0)*range*SCALEMAP;
  *x1 = x0 + sin((float)bearing*6.283185307/360.0)*range*xscale;
}

float FindRange(x0,y0,z0,x1,y1,z1)  /* range in hexes */
float x0, y0, z0, x1, y1, z1; 
{
  float returnValue;
  float bearing, xscale;
  float XYrange;
  float Zrange;

  bearing=(float)(FindBearing(x0,y0,x1,y1) % 60)/60.0;
  if(bearing>0.5) bearing=1.0-bearing;
  bearing=bearing*2.0;     /* 0 - 1 correction */
  xscale=(1.0 + XSCALE*bearing)/SCALEMAP;
  xscale=xscale*xscale;
  XYrange=sqrt(xscale*(x0-x1)*(x0-x1) + YSCALE2*(y0-y1)*(y0-y1));
  Zrange=(z0-z1)/SCALEMAP;
  returnValue=sqrt(XYrange*XYrange + Zrange*Zrange);

  return(returnValue);
}

float FindHexRange(x0,y0,x1,y1)  /* range in hexes */
float x0, y0, x1, y1; 
{
  float returnValue;
  float bearing, xscale;

  bearing=(float)(FindBearing(x0,y0,x1,y1) % 60)/60.0;
  if(bearing>0.5) bearing=1.0-bearing;
  bearing=bearing*2.0;     /* 0 - 1 correction */
  xscale=(1.0 + XSCALE*bearing)/SCALEMAP;
  xscale=xscale*xscale;
  returnValue=sqrt(xscale*(x0-x1)*(x0-x1) + YSCALE2*(y0-y1)*(y0-y1));

  return(returnValue);
}

/* CONVERSION ROUTINES courtesy Mike :) */

void RealCoordToMapCoord(hex_x,hex_y,cart_x, cart_y)
int *hex_x, *hex_y;
float cart_x, cart_y;
{
  float x, y, alpha=0.288675134, root_3 = 1.732050808;
  int x_count, y_count, x_offset, y_offset;
  
  /* first scale cart coords to 1 */
  cart_x=cart_x/SCALEMAP;
  cart_y=cart_y/SCALEMAP;
  
  if (cart_x < alpha) {
    x_count = -2;
    x = cart_x + 5 * alpha;
    }
  else {
    x_count = (int) ((cart_x - alpha) / root_3);
    x = cart_x - alpha - x_count * root_3;
    }

  y_count = (int) cart_y;
  y = cart_y - y_count;

  if ((x >= 0.0) && (x < (2.0 * alpha))) {
    x_offset = 0;
    y_offset = 0;
    }

  else if ((x >= (3.0 * alpha)) && (x < (5.0 * alpha))) {
    if ((y >= 0.0) && (y < 0.5)) {
      x_offset = 1;
      y_offset = 0;
      }
    else {
      x_offset = 1;
      y_offset = 1;
      }
    }

  else if ((x >= 2.0 * alpha) && (x < (3.0 * alpha))) {
    if ((y >= 0.0) && (y < 0.5)) { 
      if ((2 * alpha * y) <= (x - 2.0 * alpha)) {
        x_offset = 1;
        y_offset = 0;
        }
      else {
        x_offset = 0;
        y_offset = 0;
        }
      }
    else if ((2 * alpha * (1.0 - y)) > (x - 2.0 * alpha)) {
      x_offset = 0;
      y_offset = 0;
      }
    else {
      x_offset = 1;
      y_offset = 1;
      }
    }

  else if ((y >= 0.0) && (y < 0.5)) {
      if ((2 * alpha * y) <= (11.0 * alpha - x - 5.0 * alpha)) {
        x_offset = 1;
        y_offset = 0;
        }
      else {
        x_offset = 2;
        y_offset = 0;
        }
      }
  else if ((2 * alpha * (y - 0.5) ) <= (x - 5.0 * alpha)) {
    x_offset = 2;
    y_offset = 0;
    }
  else {
    x_offset = 1;
    y_offset = 1;
    }


  *hex_x = x_count * 2 + x_offset;
  *hex_y = y_count + y_offset;

}

/* if hex_x is even, new_y = (hex_y + 1) * 0.5,
                     new_x = hex_x * 3 alpha + 2 alpha.
   if hex_x is odd, then new_x = (hex_x - 1) * 3 alpha + 5 alpha,
                         new_y = hex_y. 
*/

void MapCoordToRealCoord(hex_x, hex_y, cart_x, cart_y)
int hex_x, hex_y;
float *cart_x, *cart_y;
{
  float alpha=0.288675134;

  if (hex_x + 2 - (hex_x + 2) / 2 * 2) { /* hex_x is odd */
    *cart_x = (float) (hex_x - 1.0) * 3.0 * alpha + 5.0 * alpha;
    *cart_y = (float) (hex_y);
    }
  else {
    *cart_x = (float) (hex_x) * 3.0 * alpha + 2.0 * alpha;
    *cart_y = (float) hex_y  + 0.5;
  }
  *cart_x = *cart_x*SCALEMAP;
  *cart_y = *cart_y*SCALEMAP;
}

int FindTargetXY(mech,x,y,z)
struct mech_data *mech; 
float *x; 
float *y; 
float *z;
{
  int returnValue=0;
  struct mech_data *tempMech;
  struct map_struct *mech_map;
  
  if(mech->target != -1) 
    {
      tempMech=(struct mech_data *) FindObjectsData(mech->target);
      if(tempMech) 
	{
	  *x=tempMech->fx;
	  *y=tempMech->fy;
	  *z=tempMech->fz;
	  returnValue=1;
	}
    } 
  else if(mech->targx!= -1 && mech->targy != -1) 
    {
      MapCoordToRealCoord(mech->targx, mech->targy, x, y);
      *z= -ZSCALE*(float)(mech->targz);
      returnValue=1;

    }
  return (returnValue);
}

/* ASSERTION: Weapons must be located next to each other in criticals */
/* This is a hacked function. Sorry. */

int FindWeapons(mech,index,weaparray,weapdataarray,critical)
struct mech_data *mech; 
int index; 
unsigned char *weaparray; 
unsigned char *weapdataarray;
int *critical;
{
  int loop;
  int weapcount=0;
  unsigned char temp;
  unsigned char lastweap;
  unsigned char data;
  int num_crits=0;

  for(loop=0;loop<MAX_WEAPS_SECTION; loop++) 
    {
      temp=mech->sections[index].criticals[loop].type;
      data=mech->sections[index].criticals[loop].data;
      if(temp >=WEAPON_BASE_INDEX && temp < AMMO_BASE_INDEX) 
        {
          temp-= WEAPON_BASE_INDEX;
          
          if(weapcount==0) 
            {
              lastweap=temp;
              weapdataarray[weapcount]=data;
              weaparray[weapcount]=temp;
	      critical[weapcount]=loop;
	      weapcount++;
              num_crits=1;
              continue;
            }

          if(temp != lastweap || 
             (num_crits == GetWeaponCrits(mech,temp))) 
            {
              if(temp != lastweap 
                 && num_crits != GetWeaponCrits(mech,lastweap)) 
                {
                  send_mechinfo(tprintf("Error in the numcriticals for weapon on #%d!",mech->mynum));
                }
              weaparray[weapcount]=temp;
              weapdataarray[weapcount]=data;
	      critical[weapcount]=loop;
              lastweap=temp;
              num_crits=1;
              weapcount++;
            } 
          else num_crits++;
        }
    }
  return (weapcount);
}

int FindAmmunition(mech,weaparray,ammoarray,modearray)
struct mech_data *mech; 
unsigned char *weaparray; 
unsigned char *ammoarray;
unsigned char *modearray; 
{
  int loop;
  int weapcount=0;
  unsigned char temp;
  unsigned char data;
  unsigned char mode;
  int num_crits=0;
  int index,i,duplicate;

for(index=0; index<NUM_SECTIONS; index++)
  for(loop=0;loop<MAX_WEAPS_SECTION; loop++) 
    {
      duplicate=0;
      data=mech->sections[index].criticals[loop].data;
      temp=mech->sections[index].criticals[loop].type;
      mode=mech->sections[index].criticals[loop].mode;
      if(temp >=AMMO_BASE_INDEX && temp < RESERVED_BASE_INDEX)
        {
          temp-= AMMO_BASE_INDEX;
          
          for(i=0; i<weapcount; i++)
	    {
	      if(temp==weaparray[i] && mode==modearray[i] 
                 && data!=CRIT_DESTROYED)
		{
		  ammoarray[i]+=data;
		  duplicate=1;
		}
	     }
	   if(!duplicate && data!=CRIT_DESTROYED)
	     {
	        weaparray[weapcount]=temp;
	        ammoarray[weapcount]=data;
                modearray[weapcount]=mode;
                weapcount++;
             }
        }
    }
  return (weapcount);
}

int FindLegHeatSinks(mech)
struct mech_data *mech; 
{
  int loop;
  unsigned char ltemp;
  unsigned char ldata;
  unsigned char rtemp;
  unsigned char rdata;
  int heatsinks=0;

  for(loop=0;loop<NUM_CRITICALS; loop++) 
    {
      ltemp=mech->sections[LLEG].criticals[loop].type;
      ldata=mech->sections[LLEG].criticals[loop].data;
      rtemp=mech->sections[RLEG].criticals[loop].type;
      rdata=mech->sections[RLEG].criticals[loop].data;
      if(ltemp == HEAT_SINK+SPECIAL_BASE_INDEX && ldata != CRIT_DESTROYED)
	heatsinks++;
      if(rtemp == HEAT_SINK+SPECIAL_BASE_INDEX && rdata != CRIT_DESTROYED)
	heatsinks++;
    }
  return (heatsinks);
}

/* Added for tic support. */
/* returns the weapon index- -1 for not found, -2 for destroyed, -3, -4 */
/* for reloading/recycling */
int FindWeaponNumberOnMech(mech,number,section,crit)
struct mech_data *mech; 
int number; 
int *section;
int *crit;
{
  int loop;
  unsigned char weaparray[MAX_WEAPS_SECTION];
  unsigned char weapdata[MAX_WEAPS_SECTION];
  int critical[MAX_WEAPS_SECTION];
  int running_sum=0;
  int num_weaps;
  int index;
  int returnValue= -1;

  for(loop=0; loop<NUM_SECTIONS; loop++) {
    num_weaps = FindWeapons(mech, loop, weaparray, weapdata, critical);
    if(number < running_sum + num_weaps) {
      /* we found it... */
      index=number-running_sum;
      if(weapdata[index]==CRIT_DESTROYED) {
	returnValue= TIC_NUM_DESTROYED;
      } else {
	if(weapdata[index]>0) {
	  if(MechWeapons[weaparray[index]].type==TBEAM) {
	    returnValue= TIC_NUM_RECYCLING;
	  } else {
	    /* Ammo weapon */
	    returnValue= TIC_NUM_RELOADING;
	  }
	} else {
	  /* The recylce data for the weapon is clear- it is ready to fire! */
	  returnValue=weaparray[index];
	}
      }
      if (mech->sections[loop].recycle)
	/* just did a physical attack */
	{
	  returnValue = TIC_NUM_PHYSICAL;
	}
      *section=loop;
      *crit=critical[index];
      break; /* out of for loop */
    } else {
      running_sum+=num_weaps;
    }
  }
  return (returnValue);
}

int FindWeaponFromIndex(mech,weapindx,section,crit)
struct mech_data *mech; 
int weapindx; 
int *section;
int *crit;
{
  int loop;
  unsigned char weaparray[MAX_WEAPS_SECTION];
  unsigned char weapdata[MAX_WEAPS_SECTION];
  int critical[MAX_WEAPS_SECTION];
  int running_sum=0;
  int num_weaps;
  int index;
  int returnValue=0;

  for(loop=0; loop<NUM_SECTIONS; loop++) 
    {
      num_weaps = FindWeapons(mech, loop, weaparray, weapdata, critical);
      for(index=0;index<num_weaps;index++)
        if(weaparray[index]==weapindx) 
          {
            *section=loop;
            *crit=critical[index];
            returnValue=1;              /* Found it */
            if(!weapdata[index]) return(returnValue);   
            /* Return if not Recycling/Destroyed */
            /* Otherwise keep looking */
          }
    }
  return(returnValue);   /* Only returns 0 if it can't find weapon */
}

int FindWeaponIndex(mech,number)
struct mech_data *mech; 
int number; 
{
  int loop;
  unsigned char weaparray[MAX_WEAPS_SECTION];
  unsigned char weapdata[MAX_WEAPS_SECTION];
  int critical[MAX_WEAPS_SECTION];
  int running_sum=0;
  int num_weaps;
  int index;
  int returnValue= -1;

  for(loop=0; loop<NUM_SECTIONS; loop++) 
    {
      num_weaps = FindWeapons(mech, loop, weaparray, weapdata, critical);
      if(number < running_sum + num_weaps) 
	{
	  /* we found it... */
	  index=number-running_sum;
	  returnValue=weaparray[index];
	  break; /* out of for loop */
	} else {
	  running_sum+=num_weaps;
	}
    }
  return (returnValue);
}

int FindAmmoForWeapon(mech,weapindx,start,section,critical)
struct mech_data *mech; 
int weapindx; 
int start;
int *section; 
int *critical; 
{
  int loop;
  int critloop;
  int desired;
  int returnValue=0;
  
  desired=AMMO_BASE_INDEX+weapindx;

  /* Can't use LBX ammo as normal, but can use Narc and Artemis as normal */
  for(critloop=0; critloop<NUM_CRITICALS; critloop++) {
    if(mech->sections[start].criticals[critloop].type==desired &&
       mech->sections[start].criticals[critloop].data!=CRIT_DESTROYED &&
       mech->sections[start].criticals[critloop].mode!=LBX_MODE &&
       mech->sections[start].criticals[critloop].mode!=NARC_MODE) {
      *section=start;
      *critical=critloop;
      returnValue=1;

      /* This lets us keep looking for the ammo, in case we have */
      /* multiple places where it is stored.. */
      if(mech->sections[start].criticals[critloop].data!=CRIT_DESTROYED &&
	 mech->sections[start].criticals[critloop].data > 0) {
	return (returnValue);
      }
    }
  }

  for(loop=0; loop<NUM_SECTIONS; loop++) {
    if (loop != start)
    {
      for(critloop=0; critloop<NUM_CRITICALS; critloop++) {
        if(mech->sections[loop].criticals[critloop].type==desired &&
  	   mech->sections[loop].criticals[critloop].data!=CRIT_DESTROYED &&
           mech->sections[loop].criticals[critloop].mode!=LBX_MODE &&
           mech->sections[loop].criticals[critloop].mode!=NARC_MODE) {
               
          *section=loop;
	  *critical=critloop;
	  returnValue=1;

	  /* This lets us keep looking for the ammo, in case we have */
	  /* multiple places where it is stored.. */
	  if(mech->sections[loop].criticals[critloop].data!=CRIT_DESTROYED &&
	     mech->sections[loop].criticals[critloop].data > 0) {
	    return (returnValue);
	  }
        }
      }
    }
  }
  return (returnValue);
}

/* Call only if the weapon is in LBX mode already */
int FindLBXAmmoForWeapon(mech,weapindx,start,section,critical)
struct mech_data *mech; 
int weapindx; 
int start;
int *section; 
int *critical; 
{
  int loop;
  int critloop;
  int desired;
  int returnValue=0;
  
  desired=AMMO_BASE_INDEX+weapindx;

  for(critloop=0; critloop<NUM_CRITICALS; critloop++) {
    if(mech->sections[start].criticals[critloop].type==desired &&
       mech->sections[start].criticals[critloop].data!=CRIT_DESTROYED &&
       mech->sections[start].criticals[critloop].mode==LBX_MODE) {
      *section=start;
      *critical=critloop;
      returnValue=1;

      /* This lets us keep looking for the ammo, in case we have */
      /* multiple places where it is stored.. */
      if(mech->sections[start].criticals[critloop].data!=CRIT_DESTROYED &&
	 mech->sections[start].criticals[critloop].data > 0) {
	return (returnValue);
      }
    }
  }

  for(loop=0; loop<NUM_SECTIONS; loop++) {
    if (loop != start)
    {
      for(critloop=0; critloop<NUM_CRITICALS; critloop++) {
        if(mech->sections[loop].criticals[critloop].type==desired &&
  	   mech->sections[loop].criticals[critloop].data!=CRIT_DESTROYED &&
           mech->sections[loop].criticals[critloop].mode==LBX_MODE) {
	  *section=loop;
	  *critical=critloop;
	  returnValue=1;

	  /* This lets us keep looking for the ammo, in case we have */
	  /* multiple places where it is stored.. */
	  if(mech->sections[loop].criticals[critloop].data!=CRIT_DESTROYED &&
	     mech->sections[loop].criticals[critloop].data > 0) {
	    return (returnValue);
	  }
        }
      }
    }
  }
  return (returnValue);
}

/* Call only if the weapon is in Aretmis mode already */
int FindArtemisAmmoForWeapon(mech,weapindx,start,section,critical)
struct mech_data *mech; 
int weapindx; 
int start;
int *section; 
int *critical; 
{
  int loop;
  int critloop;
  int desired;
  int returnValue=0;
  
  desired=AMMO_BASE_INDEX+weapindx;

  for(critloop=0; critloop<NUM_CRITICALS; critloop++) {
    if(mech->sections[start].criticals[critloop].type==desired &&
       mech->sections[start].criticals[critloop].data!=CRIT_DESTROYED &&
       mech->sections[start].criticals[critloop].mode==ARTEMIS_MODE) {
      *section=start;
      *critical=critloop;
      returnValue=1;

      /* This lets us keep looking for the ammo, in case we have */
      /* multiple places where it is stored.. */
      if(mech->sections[start].criticals[critloop].data!=CRIT_DESTROYED &&
	 mech->sections[start].criticals[critloop].data > 0) {
	return (returnValue);
      }
    }
  }

  for(loop=0; loop<NUM_SECTIONS; loop++) {
    if (loop != start)
    {
      for(critloop=0; critloop<NUM_CRITICALS; critloop++) {
        if(mech->sections[loop].criticals[critloop].type==desired &&
  	   mech->sections[loop].criticals[critloop].data!=CRIT_DESTROYED &&
           mech->sections[loop].criticals[critloop].mode==ARTEMIS_MODE) {
	  *section=loop;
	  *critical=critloop;
	  returnValue=1;

	  /* This lets us keep looking for the ammo, in case we have */
	  /* multiple places where it is stored.. */
	  if(mech->sections[loop].criticals[critloop].data!=CRIT_DESTROYED &&
	     mech->sections[loop].criticals[critloop].data > 0) {
	    return (returnValue);
	  }
        }
      }
    }
  }
  return (returnValue);
}

/* Call only if the weapon is in Narc mode already */
int FindNarcAmmoForWeapon(mech,weapindx,start,section,critical)
struct mech_data *mech; 
int weapindx; 
int start;
int *section; 
int *critical; 
{
  int loop;
  int critloop;
  int desired;
  int returnValue=0;
  
  desired=AMMO_BASE_INDEX+weapindx;

  for(critloop=0; critloop<NUM_CRITICALS; critloop++) {
    if(mech->sections[start].criticals[critloop].type==desired &&
       mech->sections[start].criticals[critloop].data!=CRIT_DESTROYED &&
       mech->sections[start].criticals[critloop].mode==NARC_MODE) {
      *section=start;
      *critical=critloop;
      returnValue=1;

      /* This lets us keep looking for the ammo, in case we have */
      /* multiple places where it is stored.. */
      if(mech->sections[start].criticals[critloop].data!=CRIT_DESTROYED &&
	 mech->sections[start].criticals[critloop].data > 0) {
	return (returnValue);
      }
    }
  }

  for(loop=0; loop<NUM_SECTIONS; loop++) {
    if (loop != start)
    {
      for(critloop=0; critloop<NUM_CRITICALS; critloop++) {
        if(mech->sections[loop].criticals[critloop].type==desired &&
  	   mech->sections[loop].criticals[critloop].data!=CRIT_DESTROYED &&
           mech->sections[loop].criticals[critloop].mode==NARC_MODE) {
	  *section=loop;
	  *critical=critloop;
	  returnValue=1;

	  /* This lets us keep looking for the ammo, in case we have */
	  /* multiple places where it is stored.. */
	  if(mech->sections[loop].criticals[critloop].data!=CRIT_DESTROYED &&
	     mech->sections[loop].criticals[critloop].data > 0) {
	    return (returnValue);
	  }
        }
      }
    }
  }
  return (returnValue);
}

int FindArtemisForWeapon(mech,section,critical)
struct mech_data *mech; 
int section; 
int critical; 
{
  int loop;
  int critloop;
  int desired;
  
  desired=SPECIAL_BASE_INDEX+ARTEMIS_IV;

  for(critloop=0; critloop<NUM_CRITICALS; critloop++) {
    if(mech->sections[section].criticals[critloop].type==desired &&
       mech->sections[section].criticals[critloop].data!=CRIT_DESTROYED) {
      if(mech->sections[section].criticals[critloop].data==critical)
        return(1);
      }
    }
  return(0);
}

int FindDestructiveAmmo(mech,section,critical)
struct mech_data *mech; 
int *section; 
int *critical; 
{
  int loop;
  int critloop;
  int maxdamage=0;
  int damage;
  int weapindx;
  int i;
  unsigned char type, data;
  
  for(loop=0; loop<NUM_SECTIONS; loop++) {
    for(critloop=0; critloop<NUM_CRITICALS; critloop++) {
      if(mech->sections[loop].criticals[critloop].type>=AMMO_BASE_INDEX &&
	 mech->sections[loop].criticals[critloop].type<RESERVED_BASE_INDEX &&
	 mech->sections[loop].criticals[critloop].data!=CRIT_DESTROYED) {

	data=mech->sections[loop].criticals[critloop].data;
	type=mech->sections[loop].criticals[critloop].type;

	weapindx=type-AMMO_BASE_INDEX;
	damage=data*MechWeapons[weapindx].damage;
	if(MechWeapons[weapindx].type==TMISSILE)
	  {
	    for(i=0; MissileHitTable[i].key != -1; i++) 
	      if(MissileHitTable[i].key == weapindx) 
		damage *= MissileHitTable[i].num_missiles[10];
	  }

	if(damage > maxdamage) 
	  {
	    *section=loop;
	    *critical=critloop;
	    maxdamage=damage;
	  }
      }
    }
  }
  return(maxdamage);
}

int FindRoundsForWeapon(mech,weapindx)
struct mech_data *mech; 
int weapindx; 
{
  int loop;
  int critloop;
  int desired;
  int returnValue=0;
  
  desired=AMMO_BASE_INDEX+weapindx;

  for(loop=0; loop<NUM_SECTIONS; loop++) {
    for(critloop=0; critloop<NUM_CRITICALS; critloop++) {
      if(mech->sections[loop].criticals[critloop].type==desired &&
	 mech->sections[loop].criticals[critloop].data!=CRIT_DESTROYED) {
	returnValue+=mech->sections[loop].criticals[critloop].data;
	}
      }
    }
  return (returnValue);
}


void ArmorStringFromIndex(index,buffer,type)
     int index; 
     char *buffer;
     char type;
{
  if(type==CLASS_MECH) {
    switch(index) {
    case HEAD:
      sprintf(buffer, "Head");
      break;
    case CTORSO:
      sprintf(buffer, "Center Torso");
      break;
    case RTORSO:
      sprintf(buffer, "Right Torso");
      break;
    case LTORSO:
      sprintf(buffer, "Left Torso");
      break;
    case RARM:
      sprintf(buffer, "Right Arm");
      break;
    case LARM:
      sprintf(buffer, "Left Arm");
      break;
    case RLEG:
      sprintf(buffer, "Right Leg");
      break;
    case LLEG:
      sprintf(buffer, "Left Leg");
      break;
    default:
      sprintf(buffer, "Invalid!!");
      break;
    }
  } else if(type!=CLASS_VEH_VTOL) {
    switch(index) {
    case LSIDE:
      sprintf(buffer, "Left Side");
      break;
    case RSIDE:
      sprintf(buffer, "Right Side");
      break;
    case FSIDE:
      sprintf(buffer, "Front Side");
      break;
    case BSIDE:
      sprintf(buffer, "Rear Side");
      break;
    case TURRET:
      sprintf(buffer, "Turret");
      break;
    default:
      sprintf(buffer, "Invalid!!");
      break;
    } 
  } else {
    switch(index) {
    case LSIDE:
      sprintf(buffer, "Left Side");
      break;
    case RSIDE:
      sprintf(buffer, "Right Side");
      break;
    case FSIDE:
      sprintf(buffer, "Front Side");
      break;
    case BSIDE:
      sprintf(buffer, "Rear Side");
      break;
    case ROTOR:
      sprintf(buffer, "Rotor");
      break;
    default:
      sprintf(buffer, "Invalid!!");
      break;
    } 
  }
}

int IsInWeaponArc(mech,x,y,section,critical)
struct mech_data *mech;
float x,y;
int section, critical;
{
  int weaponarc;
  int isrear;

  weaponarc = InWeaponArc(mech,x,y);

  if(mech->type==CLASS_MECH)
    {
      isrear=(mech->sections[section].criticals[critical].mode & REAR_MOUNT) ||
	((section == LARM || section == RARM) && (mech->status & FLIPPED_ARMS));
      
      if (weaponarc == REARARC)
	{
	  if (isrear)
	    return 1;
	  else
	    return 0;
	}
      
      if (isrear)
	return 0;
      
      if (weaponarc == FORWARDARC)
	return 1;
      
      if (weaponarc == LARMARC && section == LARM)
	return 1;
      
      if (weaponarc == RARMARC && section == RARM)
	return 1;
      
      return 0;
    }
  else
    {
      if(weaponarc >= TURRETARC && section == TURRET) 
	return 1;

      if (weaponarc >= TURRETARC)
	weaponarc -= TURRETARC;
      
      if(weaponarc == FORWARDARC && section == FSIDE)
	return 1;

      if(weaponarc == REARARC && section == BSIDE)
	return 1;

      if(weaponarc == LSIDEARC && section == LSIDE)
	return 1;
      
      if(weaponarc == RSIDEARC && section == RSIDE)
	return 1;

      return 0;
    }
}
    
int GetWeaponCrits(mech,weapindx)
     struct mech_data *mech;
     int weapindx;
{
  if(mech->type==CLASS_MECH)
    return(MechWeapons[weapindx].criticals);
  else return(1);
}
