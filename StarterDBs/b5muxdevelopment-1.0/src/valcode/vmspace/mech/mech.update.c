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

void move_mech(mech)
struct mech_data *mech; 
{
  float newx=0.0, newy=0.0, newz=0.0;
  float range, jump_pos;
  struct mech_data *target;
  struct map_struct *mech_map;

  mech_map=(struct map_struct *) FindObjectsData(mech->mapindex);

  if (!mech_map && mech->pilot >= 0)
      mech_map = ValidMap(mech->pilot, mech->mapindex);

  if(!mech_map) {
    mech_notify(mech,MECHALL,"You are on an invalid map! Map index reset!");
    if(mech->status & JUMPING)
	mech_land(mech->pilot,(void *)mech,"");
    mech_shutdown(mech->pilot,(void *)mech,"");
    send_mechinfo(tprintf("move_mech:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
    mech->mapindex= -1;
    return;
  }

  switch(mech->move)
    {
    case MOVE_BIPED:
      if(mech->status & JUMPING)
	{ 
	  FindComponents(mech->jumpspeed, mech->jumpheading, &newx, &newy);
	  mech->fx+=newx;   
	  mech->fy+=newy;
	  jump_pos=sqrt((mech->fx-mech->startfx)*(mech->fx-mech->startfx) 
			+ (mech->fy-mech->startfy)*(mech->fy-mech->startfy));
	  mech->fz=((4*mech->jumpspeed*MP_PER_KPH*ZSCALE)
		    / (mech->jumplength*mech->jumplength))
	    * jump_pos * (mech->jumplength - jump_pos) + mech->startfz 
	      + jump_pos * (mech->endfz 
			    - mech->startfz)/(mech->jumplength*HEXLEVEL);
	  mech->z=(int)(mech->fz/ZSCALE);
	  if((mech->x == mech->goingx) && (mech->y == mech->goingy))
	    LandMech(mech);
	}
      else if(fabs(mech->speed)>0.0) 
	{
	  FindComponents(mech->speed, mech->facing, &newx, &newy);
	  mech->fx+=newx;   
	  mech->fy+=newy;
	  if(mech->terrain==WATER) 
	    mech->z = -mech->elev;
	  else 
	    mech->z = mech->elev;
	  mech->fz = mech->z * ZSCALE;
	}
      break;
    case MOVE_TRACK:
    case MOVE_WHEEL:
      if(fabs(mech->speed)>0.0)
	{
	  FindComponents(mech->speed, mech->facing, &newx, &newy);
	  mech->fx+=newx;   
	  mech->fy+=newy;
	  if(mech->terrain==WATER) mech->z = -mech->elev;
	  else mech->z = mech->elev;
	  mech->fz = mech->z * ZSCALE;
	}
      break;
    case MOVE_HOVER:
      if(fabs(mech->speed)>0.0)
	{
	  FindComponents(mech->speed, mech->facing, &newx, &newy);
	  mech->fx+=newx;   
	  mech->fy+=newy;
	  if(mech->terrain==WATER) mech->z = 0;
	  else mech->z = mech->elev;
	  mech->fz = mech->z * ZSCALE;
	}
      break;
    case MOVE_VTOL:
      if(!(mech->status & LANDED))
	{
	  FindComponents(mech->speed, mech->facing, &newx, &newy);
	  mech->fx+=newx;   
	  mech->fy+=newy;
	  mech->fz+=mech->verticalspeed;
	  mech->z=mech->fz/ZSCALE;
	}
      break;
    case MOVE_HULL:
    case MOVE_FOIL:
      if(fabs(mech->speed)>0.0)
	{
	  FindComponents(mech->speed, mech->facing, &newx, &newy);
	  mech->fx+=newx;   
	  mech->fy+=newy;
	  mech->z = 0;
	  mech->fz = 0.0;
	}
      break;
    }

  mech->last_x=mech->x;
  mech->last_y=mech->y;

  RealCoordToMapCoord(&mech->x, &mech->y, mech->fx, mech->fy);

  CheckEdgeOfMap(mech);

  if(mech->last_x!=mech->x || mech->last_y!=mech->y) 
  {
    mech->terrain=mech_map->map[mech->y][mech->x].terrain;
    mech->elev=mech_map->map[mech->y][mech->x].elev;
    NewHexEntered(mech,mech_map,newx,newy);
  }

  if(mech->move==MOVE_VTOL && !(mech->status & LANDED))
    CheckVTOLHeight(mech);

  if(mech->chgtarget!=-1)  /* CHARGE!!! */
    {
      target=(struct mech_data *) FindObjectsData(mech->chgtarget);
      if(target) 
        {
          if(FindRange(mech->fx,mech->fy,mech->fz,
		       target->fx,target->fy,target->fz) < 1.) 
            {
              ChargeMech(mech,target);
              mech->chgtarget = -1;
            }
        }
      else 
        {
          mech_notify(mech,MECHPILOT,"Invalid CHARGE target!");
          mech->chgtarget = -1;
        }
    }
}

void CheckVTOLHeight(mech)
     struct mech_data *mech;
{
  
  switch(mech->terrain)
    {
    case WATER:
      if(mech->z <= 0) 
	{
	  mech_notify(mech,MECHALL, "You crash your vehicle into the water!!");
	  mech_notify(mech,MECHALL, "Water pours into the cockpit....gulp!");
	  mech->verticalspeed = 0.0;
	  mech->speed = 0.0;
	  mech->status |= DESTROYED;
	  mech->z = -mech->elev;
	  mech->fz = ZSCALE*mech->z;
	}
      break;
    case GRASSLAND:
    case ROAD:
    case BUILDING:
      if(mech->z <= mech->elev) 
	{
	  /* Land the VTOL */
	  mech_notify(mech,MECHALL, "You bring your VTOL to a safe landing.");
	  mech->z = mech->elev;
	  mech->fz = ZSCALE*mech->z;
	  mech->status |= LANDED;
	  mech->speed = 0.0;
	  mech->verticalspeed = 0.0;
	}
      break;
    default:
      if(mech->z <= mech->elev)
	{
	  mech_notify(mech,MECHALL, "CRASH!! You smash your VTOL into the ground!!");
	  mech_notify(mech,MECHALL, "Your VTOL is inoperable.");
	  mech->status |= DESTROYED;
	  mech->status &= ~STARTED;
	  mech->pilot = -1;
	  mech->z = mech->elev;
	  mech->fz = ZSCALE*mech->z;
	  mech->speed = 0.0;
	  mech->verticalspeed = 0.0;
	}
      break;
    }
}


void UpdateHeading(mech)
struct mech_data *mech; 
{
  int offset;
  int normangle;
  int turretoffset;

  if(!(mech->critstatus & GYRO_DESTROYED)) 
    {
      if(mech->type!=CLASS_MECH && mech->critstatus & TURRET_LOCKED)
	{
	  if(mech->turretfacing > mech->facing)
	    turretoffset = mech->turretfacing - mech->facing;
	  else
	    turretoffset = 360 + mech->turretfacing - mech->facing;
	}
      if(mech->facing != mech->desiredfacing) 
	{
	  normangle = mech->facing - mech->desiredfacing;
	  
	  if(mech->status & JUMPING) 
	    offset = 6 * mech->jumpspeed * MP_PER_KPH;
	  else if (fabs(mech->speed) < 1.0)
	    {
	      offset = 3 * mech->maxspeed * MP_PER_KPH;
	    }
	  else
	    {
	      offset = 2 * mech->maxspeed * MP_PER_KPH;
	      if ((abs(normangle) > offset) &&
		  (mech->speed > (mech->maxspeed * 2/3)))
		{
		  offset -= offset/2 * (3.0 * mech->speed/mech->maxspeed - 2.0);
		}
	    }
	  
	  if(normangle < 0) normangle += 360;
	  
	  if(normangle > 180) {
	    mech->facing += offset;
	    if(mech->facing >= 360) mech->facing -= 360;
	    normangle += offset;
	    if(normangle >= 360) mech->facing = mech->desiredfacing;
	  } 
	  else {
	    mech->facing -= offset;
	    if(mech->facing < 0) mech->facing += 360;
	    normangle -= offset;
	    if(normangle < 0) mech->facing = mech->desiredfacing;
	  }
	}
      if(mech->type!=CLASS_MECH && mech->critstatus & TURRET_LOCKED)
	{
	  mech->turretfacing = turretoffset + mech->facing;
	  if(mech->turretfacing >= 360) mech->turretfacing -= 360;
	  else if(mech->turretfacing < 0) mech->turretfacing += 360;
	}
    }
  else if (mech->facing != mech->desiredfacing)
    {
      mech_notify(mech,MECHPILOT,
		  "Your destroyed gyro prevents you from changing headings");
      mech->desiredfacing = mech->facing;
    }
}

void UpdateSpeed(mech)
struct mech_data *mech; 
{
  float acc,tempspeed,maxspeed;
  int heat;

  if(!(mech->status & FALLEN) && !(mech->status & JUMPING) && !mech->goingy
     && (mech->maxspeed > 0.0)) 
    {
      tempspeed=fabs(mech->desired_speed);
      maxspeed=mech->maxspeed;

      if(mech->heat >= 5.) {
	if((mech->heat >= 9.) && (mech->specials & TRIPLE_MYOMER_TECH))
	  {
	    maxspeed+=1.5*MP1;
	    tempspeed*=(maxspeed/mech->maxspeed);
	  }
	
	if(mech->heat >= 25.) {
	  /* -5 MP */
	  tempspeed*=(maxspeed-MP5)/maxspeed;
	} else if(mech->heat >= 20.) {
	  /* -4 MP */
	  tempspeed*=(maxspeed-MP4)/maxspeed;
	} else if(mech->heat >= 15.) {
	  /* -3 MP */
	  tempspeed*=(maxspeed-MP3)/maxspeed;
	} else if(mech->heat >= 10.) {
	  /* -2 MP */
	  tempspeed*=(maxspeed-MP2)/maxspeed;
	} else if(mech->heat >= 5.) {
	  /* -1 MP */
	  if(!(mech->specials & TRIPLE_MYOMER_TECH))
	    tempspeed*=(maxspeed-MP1)/maxspeed;
	}
      }

      if(tempspeed <= MP1) tempspeed=0.0;
      
      switch(mech->terrain) 
	{
	case ROUGH:
	  if(mech->move!=MOVE_VTOL && mech->move!=MOVE_HOVER)
	    tempspeed*=(maxspeed-MP2)/maxspeed;
	  break;
	case MOUNTAINS:
	  if(mech->move!=MOVE_VTOL)
	    tempspeed*=(maxspeed-MP3)/maxspeed;
	  break;
	case LIGHT_FOREST:
	  if(mech->move!=MOVE_VTOL)
	    tempspeed*=(maxspeed-MP2)/maxspeed;
	  break;
	case HEAVY_FOREST:
	  if(mech->move!=MOVE_VTOL)
	    tempspeed*=(maxspeed-MP3)/maxspeed;
	  break;
	case WATER:
	  if(mech->move==MOVE_BIPED) 
	    {
	      if(mech->elev >= 2) tempspeed*=(maxspeed-MP4)/maxspeed;
	      else if(mech->elev == 1) tempspeed*=(maxspeed-MP2)/maxspeed;
	      else if(mech->elev == 0) tempspeed*=(maxspeed-MP1)/maxspeed;
	    }
	  break;
	default:
	  break;
	}

      /* set MP1 as minimun speed */
      if(tempspeed <= MP1) 
	{
	  tempspeed=0.0;
	  if(fabs(mech->desired_speed) > 0.0) 
	    tempspeed=MP1;
	}


      if(mech->desired_speed < 0.) tempspeed= -tempspeed;
      
      if(tempspeed != mech->speed) {
	acc=maxspeed/10.;
	if(tempspeed < mech->speed) {
	  /* Decelerating */
	  mech->speed-=acc;
	  if(tempspeed > mech->speed)
	    mech->speed=tempspeed;
	} else {
	  /* Accelerating */
	  mech->speed+=acc;
	  if(tempspeed < mech->speed) 
	    mech->speed=tempspeed;
	}
      }
    }
}

int OverheatMods(mech)
struct mech_data *mech; 
{
  int returnValue;

  if(mech->heat >= 24.) {
    /* +4 to fire... */
    returnValue=4;
  } else if(mech->heat >= 17.) {
    /* +3 to fire... */
    returnValue=3;
  } else if(mech->heat >= 13.) {
    /* +2 to fire... */
    returnValue=2;
  } else if(mech->heat >= 8.) {
    /* +1 to fire... */
    returnValue=1;
  } else {
    returnValue=0;
  } 
  return (returnValue);
}

void HandleOverheat(mech)
struct mech_data *mech; 
{
  int avoided=0;

  if(mech->heat >= 14.) {
    if(mech->heat >= 30.) {
      /* Shutdown */
    } else if(mech->heat >= 26.) {
      /* Shutdown avoid on 10+ */
      if(Roll() >= 3) avoided=1;
    } else if(mech->heat >= 22.) {
      /* Shutdown avoid on 8+ */
      if(Roll() >= 3) avoided=1;
    } else if(mech->heat >= 18.) {
      /* Shutdown avoid on 6+ */
      if(Roll() >= 3) avoided=1;
    } else if(mech->heat >= 14.) {
      /* Shutdown avoid on 4+ */
      if(Roll() >= 3) avoided=1;
    }
    if(!(avoided)) {
	mech_notify(mech, MECHALL, "Reactor shutting down...");
      if(mech->status & JUMPING) 
	{
	  mech_notify(mech, MECHALL, "You fall from the sky!!!!!");
	  MechFalls(mech,(int)(mech->jumpspeed*MP_PER_KPH));   /* OUCH */
	  mech->status &= ~JUMPING;
	  mech->jumpheading = JUMP_TO_HIT_RECYCLE;
	  mech->goingx=mech->goingy= 0;
	}
      
      mech->status &= ~STARTED;
      mech->pilot= -1;
      mech->clock=0;
      mech->speed=0.;
      mech->desired_speed=0.;
      mech->goingx = -1;
    }

    avoided=0;
    /* Ammo */
    if(mech->heat >= 19.) {
      if(mech->heat >= 28.) {
	/* Ammo explosion (Avoid 8+) */
	if(Roll() >= 3) avoided=1;
      } else if(mech->heat >= 23.) {
	/* Ammo explosion (Avoid 6+) */
	if(Roll() >= 3) avoided=1;
      } else if(mech->heat >= 19.) {
	/* Ammo explosion (Avoid 4+) */
	if(Roll() >= 3) avoided=1;
      } 
      if(!(avoided)) { 
	  int ammoloc, ammocritnum, damage;
	  
	  damage=FindDestructiveAmmo(mech, &ammoloc, &ammocritnum);
	  
	  if(damage) 
	    {
	      /* BOOM! */
	      /* That's going to hurt... */
	      mech_notify(mech, MECHALL, "Ammunition explosion!!");
	      mech->sections[ammoloc].criticals[ammocritnum].data = 
								CRIT_DESTROYED;
	      DamageMech(mech, mech, 0, -1, ammoloc, 0, 0, 0, damage);
	      mech->pilotstatus+=2;
	      mech_notify(mech,MECHPILOT, 
    "You take 2 points of personal injury from the ammunition explosion!");
	      UpdatePilotStatus(mech);
	    }
	  else 
	    mech_notify(mech,MECHPILOT, "You have no ammunition, lucky you!");
	}
    }
  }
}

void UpdateHeat(mech)
struct mech_data *mech; 
{
  int heatsinks;
  int legsinks;
  float maxspeed;
  float intheat;
  float inheat;
  
  inheat = mech->heat;

  maxspeed=mech->maxspeed;
  if((mech->heat >= 9.) && (mech->specials & TRIPLE_MYOMER_TECH))
    maxspeed+=1.5*MP1;
  
  mech->plus_heat=0.;

  if(fabs(mech->speed)>0.0) 
    {
      if(IsRunning(mech->speed,maxspeed)) mech->plus_heat+=2.;
      else mech->plus_heat+=1.;
    }
  
  if(mech->status & JUMPING) mech->plus_heat+=(mech->jumpspeed*MP_PER_KPH > 3.) ? mech->jumpspeed*MP_PER_KPH : 3.;

  if ((mech->status & STARTED) || (mech->goingx >= 0))
      mech->plus_heat+=(float)mech->engineheat;

  intheat=mech->plus_heat;

  mech->plus_heat+=mech->weapheat;

  /* ADD Water effects here */
  if(mech->terrain==WATER && mech->elev!=0) 
    {
      legsinks=FindLegHeatSinks(mech);
      legsinks=(legsinks > 4) ? 4 : legsinks;
      if(mech->elev==1 && !(mech->status & FALLEN)) 
	{
	  mech->minus_heat = (2*mech->numsinks > legsinks+mech->numsinks) 
	    ? (float)(legsinks+mech->numsinks) : (float)(2*mech->numsinks);
	}
      else
	{
	  mech->minus_heat = (2*mech->numsinks > 6+mech->numsinks) 
	    ? (float)(6+mech->numsinks) : (float)(2*mech->numsinks);
	}
    }
  else 
    {
      mech->minus_heat=(float)(mech->numsinks);
    }
  
  mech->weapheat-=(mech->minus_heat-intheat)/WEAPON_RECYCLE_TIME;
  if(mech->weapheat<0.0) mech->weapheat=0.0;

  mech->heat=mech->plus_heat - mech->minus_heat;
  if(mech->heat < 0.0) mech->heat=0.0;
  
  if((mech->clock % TURN)==0)
    if(mech->critstatus & LIFE_SUPPORT_DESTROYED) 
      {
	if(mech->heat>25.) 
	  {
	    mech->pilotstatus+=2;
	    mech_notify(mech,MECHPILOT,
			    "You take 2 points of personal injury from heat!!");
	    UpdatePilotStatus(mech);
	  }
	else if(mech->heat>=15.)
	  {
	    mech->pilotstatus+=1;
	    mech_notify(mech,MECHPILOT,
			    "You take 1 point of personal injury from heat!!");
	    UpdatePilotStatus(mech);
	  }
      }
  if(mech->heat >= 19)
    {
      if(inheat < 19) 
	{
	  mech_notify(mech,MECHALL,"=====================================");
	  mech_notify(mech,MECHALL,"Your Excess Heat indicator turns RED!");
	  mech_notify(mech,MECHALL,"=====================================");
	}
    }
  else if(mech->heat >= 14) 
    { 
      if(inheat >= 19 || inheat < 14) 
	{
	  mech_notify(mech,MECHALL,"=======================================");
	  mech_notify(mech,MECHALL,"Your Excess Heat indicator turns YELLOW");
	  mech_notify(mech,MECHALL,"=======================================");
	}
    }
  else
    {
      if(inheat >= 14) 
	{
	  mech_notify(mech,MECHALL,"======================================");
	  mech_notify(mech,MECHALL,"Your Excess Heat indicator turns GREEN");
	  mech_notify(mech,MECHALL,"======================================");
	}
    }
  HandleOverheat(mech);
}

void UpdateWeapons(mech)
struct mech_data *mech; 
{

  int loop;
  int count, i;
  int crit[MAX_WEAPS_SECTION];
  unsigned char weaptype[MAX_WEAPS_SECTION];
  unsigned char weapdata[MAX_WEAPS_SECTION];
  char location[20];
  
  for(loop=0; loop<NUM_SECTIONS; loop++) 
    {
      count=FindWeapons(mech, loop, weaptype, weapdata, crit);
      for(i=0; i<count; i++) 
        {
          if(mech->sections[loop].criticals[crit[i]].data > 0
	     && mech->sections[loop].criticals[crit[i]].data!=CRIT_DESTROYED) 
            {
              mech->sections[loop].criticals[crit[i]].data--;
	      if(mech->sections[loop].criticals[crit[i]].data==0) 
		mech_notify(mech,MECHSTARTED,tprintf("%s finished recycling.",
					&MechWeapons[weaptype[i]].name[3]));
            }
        }
      if (mech->sections[loop].recycle)
	{
	  if (--mech->sections[loop].recycle == 0)
	    {
	      ArmorStringFromIndex(loop,location,mech->type);
	      mech_notify(mech,MECHSTARTED,
			tprintf("Your %s has finished its attack.", location));
	    }
	}
    }
}

int NewHexEntered(mech,mech_map,deltax,deltay)
     struct mech_data *mech;
     struct map_struct *mech_map;
     float deltax, deltay;
{
  int elevation, lastelevation;
  int oldterrain;
  int ot,le;
  struct mech_data *target;
  
  le = lastelevation = mech_map->map[mech->last_y][mech->last_x].elev;
  ot = oldterrain = mech_map->map[mech->last_y][mech->last_x].terrain;
  
  switch (mech->move)
    {
    case MOVE_BIPED:
      if (mech->status & JUMPING)
	{
	  if (mech->terrain == WATER)
	    return;
	  
	  if (oldterrain == WATER)
	    lastelevation = -le;
	  
	  elevation = mech->z;
	  
	  if (elevation < mech->elev)
	    {
	      mech->fx -= deltax;
	      mech->fy -= deltay;
	      mech->x = mech->last_x;
	      mech->y = mech->last_y;
	      mech->z = lastelevation;
	      mech->fz = mech->z * ZSCALE;
	      mech->elev = le;
	      mech->terrain = ot;
	      mech_notify(mech,MECHALL,
			  "You attempt to jump over elevation that is too high!!");
	      if (mech->pilot == -1 || !(Flags2(mech->pilot)&CONNECTED) || 
		  MadePilotSkillRoll(mech,(int)(mech->fz/ZSCALE/3)))
		{
		  mech_notify(mech,MECHALL,"You land safely.");
		  LandMech(mech);
		}
	      else
		{
		  mech_notify(mech, MECHALL, 
			      "You crash into the obstacle and fall from the sky!!!!!");
		  MechFalls(mech,mech->z);   /* OUCH */
		  mech->status &= ~JUMPING;
		  mech->jumpheading = JUMP_TO_HIT_RECYCLE;
		  mech->goingx=mech->goingy= 0;
		}
	    }
	  return;
	}
      else 
	{
	  if (oldterrain == WATER)
	    lastelevation = -lastelevation;
	  if (mech->terrain == WATER)
	    elevation = -mech->elev;
	  else 
	    elevation = mech->elev;
	  
	  if (elevation > lastelevation + 2)
	    {
	      mech->elev = le;
	      mech->terrain = ot;
	      mech_notify(mech,MECHALL,
			  "You attempt to climb a hill too steep for you.");
	      if (mech->pilot == -1 || 
		  !(Flags2(mech->pilot)&CONNECTED) || 
		  MadePilotSkillRoll(mech,(int)((mech->speed+MP1)*MP_PER_KPH/3)))
		{
		  mech_notify(mech,MECHALL, "You manage to stop before crashing.");
		}
	      else
		{
		  mech_notify(mech,MECHALL,
			      "You run headlong into the cliff and fall down!!");
		  MechFalls(mech,(int)(1+(mech->speed)*MP_PER_KPH)/4);
		}
	      mech->fx -= deltax;
	      mech->fy -= deltay;
	      mech->x = mech->last_x;
	      mech->y = mech->last_y;
	      mech->z = lastelevation;
	      mech->fz = mech->z * ZSCALE;
	      mech->desired_speed = 0;
	      mech->speed = 0;
	      return;
	    }
	  else if (elevation < lastelevation - 2)
	    {
	      mech_notify(mech,MECHALL,
			  "You notice a large drop in front of you");
	      if (mech->pilot == -1 || 
		  !(Flags2(mech->pilot)&CONNECTED) || 
		  MadePilotSkillRoll(mech,(int)((mech->speed+MP1)*MP_PER_KPH/3)))
		{
		  mech_notify(mech,MECHALL, "You manage to stop before falling off.");
		}
	      else
		{
		  mech_notify(mech,MECHALL,
			      "You run off the cliff and fall to the ground below.");
		  MechFalls(mech,lastelevation-elevation);
		  return;
		}
	      mech->fx -= deltax;
	      mech->fy -= deltay;
	      mech->x = mech->last_x;
	      mech->y = mech->last_y;
	      mech->z = lastelevation;
	      mech->fz = mech->z * ZSCALE;
	      mech->desired_speed = 0;
	      mech->speed = 0;
	      mech->elev = le;
	      mech->terrain = ot;
	      return;
	    }
	}
      
      le = elevation - lastelevation;
      le = (le < 0) ? -le : le;
      if ( le > 0)
	{
	  deltax = (le == 1) ? MP2 : MP3;
	  if (mech->speed > 0)
	    {
	      mech->speed -= deltax;
	      if (mech->speed < 0) mech->speed = 0;
	    }
	  else if (mech->speed < 0)
	    {
	      mech->speed += deltax;
	      if (mech->speed > 0) mech->speed = 0;
	    }
	}
      
      if(mech->terrain==WATER) 
	{
	  mech_notify(mech,MECHPILOT, 
		      "You use your piloting skill to manuever through the water.");
	  if(!MadePilotSkillRoll(mech,(mech->elev-2))) 
	    {
	      mech_notify(mech, MECHALL, "You slip in the water and fall down");
	      MechFalls(mech,1);
	    }
	}
      break;
    case MOVE_TRACK:
    case MOVE_WHEEL:
      if (oldterrain == WATER)
	lastelevation = -lastelevation;
      if (mech->terrain == WATER)
	elevation = -mech->elev;
      else 
	elevation = mech->elev;
      
      if (elevation > lastelevation + 1)
	{
	  mech->elev = le;
	  mech->terrain = ot;
	  mech_notify(mech,MECHALL,
		      "You attempt to climb a hill too steep for you.");
	  if (mech->pilot == -1 || 
	      !(Flags2(mech->pilot)&CONNECTED) || 
	      MadePilotSkillRoll(mech,(int)((mech->speed+MP1)*MP_PER_KPH/3)))
	    {
	      mech_notify(mech,MECHALL, "You manage to stop before crashing.");
	    }
	  else
	    {
	      mech_notify(mech,MECHALL,
			  "You smash into a cliff!!");
	      MechFalls(mech,(int)(mech->speed*MP_PER_KPH/4));
	    }
	  mech->fx -= deltax;
	  mech->fy -= deltay;
	  mech->x = mech->last_x;
	  mech->y = mech->last_y;
	  mech->z = lastelevation;
	  mech->fz = mech->z * ZSCALE;
	  mech->desired_speed = 0;
	  mech->speed = 0;
	  return;
	}
      else if (elevation < lastelevation - 1)
	{
	  mech_notify(mech,MECHALL,
		      "You notice a large drop in front of you");
	  if (mech->pilot == -1 || 
	      !(Flags2(mech->pilot)&CONNECTED) || 
	      MadePilotSkillRoll(mech,(int)((mech->speed+MP1)*MP_PER_KPH/3)))
	    {
	      mech_notify(mech,MECHALL, "You manage to stop before falling off.");
	    }
	  else
	    {
	      mech_notify(mech,MECHALL,
			  "You drive off the cliff and fall to the ground below.");
	      MechFalls(mech,lastelevation-elevation);
	      return;
	    }
	  mech->fx -= deltax;
	  mech->fy -= deltay;
	  mech->x = mech->last_x;
	  mech->y = mech->last_y;
	  mech->z = lastelevation;
	  mech->fz = mech->z * ZSCALE;
	  mech->desired_speed = 0;
	  mech->speed = 0;
	  mech->elev = le;
	  mech->terrain = ot;
	  return;
	}
      
      
      if(mech->terrain==WATER) 
	{
	  mech_notify(mech,MECHALL,
		      "You notice a body of water in front of you");
	  if (mech->pilot == -1 || 
	      !(Flags2(mech->pilot)&CONNECTED) || 
	      MadePilotSkillRoll(mech,(int)((mech->speed+MP1)*MP_PER_KPH/3)))
	    {
	      mech_notify(mech,MECHALL, "You manage to stop before falling in.");
	    }
	  else
	    {
	      mech_notify(mech,MECHALL,
			  "You drive into the water and your vehicle becomes inoperable.");
	      mech->speed=0.0;
	      mech->status |= DESTROYED;
	      mech->status &= ~STARTED;
	      mech->pilot= -1;
	      return;
	    }
	  mech->fx -= deltax;
	  mech->fy -= deltay;
	  mech->x = mech->last_x;
	  mech->y = mech->last_y;
	  mech->z = lastelevation;
	  mech->fz = mech->z * ZSCALE;
	  mech->desired_speed = 0;
	  mech->speed = 0;
	  mech->elev = le;
	  mech->terrain = ot;
	  return;
	}

      le = elevation - lastelevation;
      le = (le < 0) ? -le : le;
      if ( le > 0)
	{
	  deltax = (le == 1) ? MP2 : MP3;
	  if (mech->speed > 0)
	    {
	      mech->speed -= deltax;
	      if (mech->speed < 0) mech->speed = 0;
	    }
	  else if (mech->speed < 0)
	    {
	      mech->speed += deltax;
	      if (mech->speed > 0) mech->speed = 0;
	    }
	}
      break;
    case MOVE_HOVER:
      if (oldterrain == WATER)
	lastelevation = 0;
      if (mech->terrain == WATER)
	elevation = 0;
      else 
	elevation = mech->elev;
      
      if (elevation > lastelevation + 1)
	{
	  mech->elev = le;
	  mech->terrain = ot;
	  mech_notify(mech,MECHALL,
		      "You attempt to climb a hill too steep for you.");
	  if (mech->pilot == -1 || 
	      !(Flags2(mech->pilot)&CONNECTED) || 
	      MadePilotSkillRoll(mech,(int)((mech->speed+MP1)*MP_PER_KPH/3)))
	    {
	      mech_notify(mech,MECHALL, "You manage to stop before crashing.");
	    }
	  else
	    {
	      mech_notify(mech,MECHALL,
			  "You smash into a cliff!!");
	      MechFalls(mech,(int)(mech->speed*MP_PER_KPH/4));
	    }
	  mech->fx -= deltax;
	  mech->fy -= deltay;
	  mech->x = mech->last_x;
	  mech->y = mech->last_y;
	  mech->z = lastelevation;
	  mech->fz = mech->z * ZSCALE;
	  mech->desired_speed = 0;
	  mech->speed = 0;
	  return;
	}
      else if (elevation < lastelevation - 1)
	{
	  mech_notify(mech,MECHALL,
		      "You notice a large drop in front of you");
	  if (mech->pilot == -1 || 
	      !(Flags2(mech->pilot)&CONNECTED) || 
	      MadePilotSkillRoll(mech,(int)((mech->speed+MP1)*MP_PER_KPH/3)))
	    {
	      mech_notify(mech,MECHALL, "You manage to stop before falling off.");
	    }
	  else
	    {
	      mech_notify(mech,MECHALL,
			  "You drive off the cliff and fall to the ground below.");
	      MechFalls(mech,lastelevation-elevation);
	      return;
	    }
	  mech->fx -= deltax;
	  mech->fy -= deltay;
	  mech->x = mech->last_x;
	  mech->y = mech->last_y;
	  mech->z = lastelevation;
	  mech->fz = mech->z * ZSCALE;
	  mech->desired_speed = 0;
	  mech->speed = 0;
	  mech->elev = le;
	  mech->terrain = ot;
	  return;
	}
      
      le = elevation - lastelevation;
      le = (le < 0) ? -le : le;
      if ( le > 0)
	{
	  deltax = (le == 1) ? MP2 : MP3;
	  if (mech->speed > 0)
	    {
	      mech->speed -= deltax;
	      if (mech->speed < 0) mech->speed = 0;
	    }
	  else if (mech->speed < 0)
	    {
	      mech->speed += deltax;
	      if (mech->speed > 0) mech->speed = 0;
	    }
	}
      break;
    case MOVE_VTOL:
      if (mech->terrain == WATER)
	return;

      if(ot == WATER)
	lastelevation = -le;
	  
      if(mech->terrain==LIGHT_FOREST || mech->terrain==HEAVY_FOREST)
	elevation = mech->elev + 2;
      else 
	elevation = mech->elev;
	  
      if (mech->z < elevation)
	{
	  mech->fx -= deltax;
	  mech->fy -= deltay;
	  mech->x = mech->last_x;
	  mech->y = mech->last_y;
	  mech->z = lastelevation;
	  mech->fz = mech->z * ZSCALE;
	  mech->elev = le;
	  mech->terrain = ot;
	  mech_notify(mech,MECHALL,
		      "You attempt to fly over elevation that is too high!!");
	  if (mech->pilot == -1 || !(Flags2(mech->pilot)&CONNECTED) || 
	      (MadePilotSkillRoll(mech,(int)(mech->fz/ZSCALE/3)) &&
	       (ot == GRASSLAND || ot == ROAD || ot == BUILDING)))
	    {
	      mech_notify(mech,MECHALL,"You land safely.");
	      mech->status |= LANDED;
	      mech->speed = 0.0;
	      mech->verticalspeed = 0.0;
	    }
	  else
	    {
	      mech_notify(mech, MECHALL, 
			  "You crash into the obstacle and fall from the sky!!!!!");
	      MechFalls(mech,mech->z);   /* OUCH */
	    }
	}
      break;
    case MOVE_HULL:
    case MOVE_FOIL:
      if(mech->terrain != WATER)
	{
	  mech->fx -= deltax;
	  mech->fy -= deltay;
	  mech->x = mech->last_x;
	  mech->y = mech->last_y;
	  mech->z = 0;
	  mech->fz = mech->z * ZSCALE;
	  mech->elev = le;
	  mech->terrain = ot;
	  mech_notify(mech,MECHALL,
		      "You can't move off the water!");
	}
      break;
    }
}

void CheckDamage(wounded)
     struct mech_data *wounded;
{
  /* should be called from UpdatePilotSkillRolls */
  /* this is so that a roll will be made only when the mech takes damage */
  /* turndamage is also cleared every 30 secs in UpdatePilotSkillRolls */

  if(wounded->turndamage >= 20) 
    {
      if(!(wounded->status & JUMPING) && !(wounded->status & FALLEN))
	{
          mech_notify(wounded,MECHPILOT, "You stagger from the damage!");
	  if(!MadePilotSkillRoll(wounded,1)) 
	    {
	      mech_notify(wounded,MECHALL,"You fall over from all the damage!!");
	      MechFalls(wounded,1);
	    }
        }
      wounded->turndamage = 0;
    }
}  

int PilotStatusRollNeeded[] = {0,3,5,7,10,11};

int UpdatePilotSkillRolls(mech)
     struct mech_data *mech;
{
  int makeroll=0;
  float maxspeed;
  int roll;
  int roll_needed;
  
  if(((mech->clock % TURN)==0) && !(mech->status & FALLEN) 
					&& !(mech->status & JUMPING))   
    /* do this once a turn (30 secs), only if mech is standing */
    {
      maxspeed=mech->maxspeed;

      if(!(mech->status & STARTED)) 
	makeroll=4;

      if((mech->heat >= 9.) && (mech->specials & TRIPLE_MYOMER_TECH))
	maxspeed+=1.5*MP1;

      if(IsRunning(mech->speed,maxspeed) && 
	((mech->critstatus & GYRO_DAMAGED) || (mech->critstatus & HIP_DAMAGED)))
	makeroll=1;
      
      if(makeroll) 
	if(!MadePilotSkillRoll(mech,(makeroll-1))) 
	  {
	    mech_notify(mech,MECHALL,
		    "Your damaged mech falls as you try to run");
	    MechFalls(mech,1);
	  }
    }
  /* Decrement conciousness counter and see if he wakes up */
  if(mech->unconcious) 
    {
      if(mech->status & DESTROYED) 
	{
	  /* Let the pilot know he died. */
	  mech->unconcious = 0;
	  mech->status &= ~DESTROYED;
	  mech_notify(mech, MECHALL, "This 'Mech was destroyed while the pilot was unconcious.");
	  mech->status |= DESTROYED;
	  return;
        }
      mech->unconcious--;
      if(!mech->unconcious) 
	{
	  roll = Roll();
	  roll_needed = PilotStatusRollNeeded[mech->pilotstatus];
          if(mech->pilot!=-1) notify(mech->pilot, "You attempt to regain conciousness!");
          if(mech->pilot!=-1) notify(mech->pilot, tprintf("Regain Conciousness on: %d  \tRoll: %d",roll_needed,roll));
	  if(roll < roll_needed) 
	    mech->unconcious=UNCONCIOUS_TIME;
	  else 
	    mech_notify(mech, MECHALL, "The pilot regains conciousness!");
	}
    }	      
  if(mech->type==CLASS_MECH) CheckDamage(mech);
  if((mech->clock % TURN)==0) mech->turndamage=0;
}  

int UpdatePilotStatus(mech)
struct mech_data *mech;
{
int roll;
int roll_needed;

  if (mech->pilotstatus == 0)
      return;
  else if (mech->pilotstatus > 5)
  {
      mech_notify(mech,MECHPILOT, "You are killed from personal injuries!!");
      mech->pilot= -1;
      mech->status=DESTROYED;     /* Change to kill pilot eventually */
      mech->speed=0.;
      mech->desired_speed=0.;
      return;
  }
  roll = Roll();
  roll_needed = PilotStatusRollNeeded[mech->pilotstatus];
  mech_notify(mech, MECHPILOT, "You attempt to keep conciousness!");
  mech_notify(mech, MECHPILOT, tprintf("Retain Conciousness on: %d  \tRoll: %d",
				 roll_needed,roll));
  if (roll >= roll_needed)
      return;

  mech_notify(mech, MECHPILOT, "Conciousness slips away from you as you enter a sea of darkness...");

  mech->unconcious=UNCONCIOUS_TIME;
  mech->desired_speed=0;
  mech_notify(mech, MECHALL, 
		    "The pilot loses conciousness!!");
}


/* This function is called once every second for every Mech in the */
/* list */ 
void mech_update(key,data)
dbref key; 
void *data;
{
  struct mech_data *mech;
  int count;

  mech=(struct mech_data *) data;
  if(data) {
    mech->clock = (mech->clock+1) % MAX_CLOCK;
    if(mech->status & STARTED) {
      UpdateHeading(mech);
      UpdateSpeed(mech);
      UpdateWeapons(mech); /* Recycle/Reload times */
      move_mech(mech);
    } 

    if(mech->unconcious || (mech->status & STARTED)) {
      UpdatePilotSkillRolls(mech);
    }
    
    if ((mech->status & STARTED) || mech->plus_heat > 0.) {
      UpdateHeat(mech);
    }

    if (!(mech->status & JUMPING) && mech->jumpheading) {
        mech->jumpheading--;
        if (!mech->jumpheading) {
	    mech_notify(mech,MECHPILOT,
                               "You have finally stabilized after your jump.");
        }
    }

    if (!(mech->status & JUMPING) && mech->goingy) {
        mech->goingy--;
        if (!mech->goingy) {
	    mech_notify(mech,MECHPILOT,
                               "You have finally finished standing up.");
        }
    }
    
    if((mech->pilot != -1) && !(mech->status & STARTED)) {
      /* They are starting up the mech... */

      count=abs(mech->goingx - mech->clock);
      switch(count) {
      case 25:
	mech_notify(mech, MECHALL, "-> Main reactor is now online <-");
	break;
      case 20:
	mech_notify(mech, MECHALL, "-> Gyros are now stable <-");
	break;
      case 15:
	mech_notify(mech, MECHALL, "-> Main computer system is now online <-");
	break;
      case 10:
	mech_notify(mech, MECHALL, "-> Scanners are now operational <-");
	break;
      case 5:
	mech_notify(mech, MECHALL, 
				"-> Targetting system is now operational <-");
	break;
      case 0:
	mech_notify(mech, MECHALL, "-=> All systems operational! <= -");
	mech->status |= STARTED;
	mech->goingx = -1;
	break;
      }
    }
  }
}
