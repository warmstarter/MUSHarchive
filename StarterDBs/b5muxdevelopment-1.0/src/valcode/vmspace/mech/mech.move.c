#ifdef TEST
typedef int dbref;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

/*#include "config.h"
#include "externs.h"
#include "db.h"*/
#include "../header.h"
#include "mech.h"

void mech_drop(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
struct mech_data *mech;
float s1;

  mech=(struct mech_data *) data;
  if(mech) {
    if(mech->unconcious) 
      {
	notify(player, "You are unconscious....zzzzzzzzz");
	return;
      }
    if(mech->type!=CLASS_MECH) 
      {
	notify(player, "This vehicle cannot drop like a 'Mech.");
	return;
      }
    if(mech->status & STARTED) {
      if(mech->status & FALLEN) 
        {
          notify(player, "You are already prone.");
          return;
        }
      if(mech->status & JUMPING) 
      {
	 notify(player, "You can't drop while jumping!");
	 return;
      }
      if(!(mech->status & JUMPING) && mech->goingy) 
      {
	 notify(player, "You can't drop while trying to stand up!");
	 return;
      }

      s1 = mech->maxspeed / 3.0;
      if (fabs(mech->speed) > s1*2)
      {
	  mech_notify(mech,MECHALL, 
			    "You attempt a controlled drop while running.");
	  if (MadePilotSkillRoll(mech,2))
	  {
	      mech_notify(mech,MECHALL,
				    "You hit the ground with minimal damage");
	  }
	  else
	  {
	      mech_notify(mech,MECHALL,"You fall to the ground hard");
	      MechFalls(mech,2);
	  }
      }
      else if (fabs(mech->speed) > s1)
      {
	  mech_notify(mech,MECHALL, 
			"You attempt a controlled drop from your fast walk.");
	  if (MadePilotSkillRoll(mech,0))
	  {
	      mech_notify(mech,MECHALL,
				"You hit the ground with minimal damage");
	  }
	  else
	  {
	      mech_notify(mech,MECHALL,"You fall to the ground hard");
	      MechFalls(mech,1);
	  }
      }
      else
      {
	  mech_notify(mech,MECHALL,"You drop to the ground prone!");
      }
      mech->status |= FALLEN;
      mech->desired_speed=0;
      mech->speed=0;
    } else {
      notify(player, "Your mech has not been started!");
    }
  }
}

void mech_stand(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;

  mech=(struct mech_data *) data;
  if(mech) {
    if(mech->unconcious) 
      {
	notify(player, "You are unconscious....zzzzzzzzz");
	return;
      }
    if(mech->type!=CLASS_MECH) 
      {
	notify(player, "This vehicle cannot drop like a 'Mech.");
	return;
      }
    if(mech->status & STARTED) {
      if(mech->status & JUMPING) 
	{
	  notify(player, "You're standing while jumping!");
	  return;
	}
      if(mech->critstatus & NO_LEGS)
	{
	  notify(player, "You have no legs to stand on!");
	  return;
	}
      if(mech->critstatus & GYRO_DESTROYED)
	{
	  notify(player, "You cannot stand with a destroyed gyro!");
	  return;
	}
      if(!(mech->status & FALLEN)) 
	{
	 notify(player, "You're already standing!");
	 return;
       }
      mech->status &= ~FALLEN;
      if(!MadePilotSkillRoll(mech,0)) 
	{
	  mech_notify(mech,MECHALL, 
		"You fail your attempt to stand and fall back on the ground");
	  MechFalls(mech,1);
	}
      else 
	{
	  /* Now we set a counter in goingy to keep him from moving or 
	     jumping until he is finished standing */
	  mech->goingy = DROP_TO_STAND_RECYCLE; 
	  mech_notify(mech,MECHALL, "You begin to stand up.");
	}
    } else {
      notify(player, "Your mech has not been started!");
    }
  } else {
    notify(player, "Data not allocated!");
  }
}

void mech_land(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;

  mech=(struct mech_data *) data;
  if(mech) 
    {
      if(mech->unconcious) 
	{
	  notify(player, "You are unconscious....zzzzzzzzz");
	  return;
	}
      if(mech->type!=CLASS_VEH_VTOL && mech->type!=CLASS_MECH)
	{
	  notify(player, "You can't land this type of vehicle.");
	  return;
	}
      if(mech->status & STARTED) 
	{
	  if(mech->type==CLASS_VEH_VTOL)
	    {
	      if(mech->verticalspeed > 1.0 || mech->speed > 1.0)
		{
		  notify(player, "You are moving too fast to land.");
		  return;
		}
	      if(mech->z > mech->elev + 1)
		{
		  notify(player, "You are too high to land here.");
		  return;
		}
	      if(mech->terrain==GRASSLAND || mech->terrain==ROAD ||
		 mech->terrain==BUILDING)
		{
		  notify(player, 
			 "You bring your VTOL to a safe landing.");
		  mech->z = mech->elev;
		  mech->fz = ZSCALE*mech->z;
		  mech->status |= LANDED;
		  mech->speed = 0.0;
		  mech->verticalspeed = 0.0;
		  return;
		}
	      else
		{
		  notify(player, "You can't land on this type of terrain.");
		  return;
		}
	    }
	  if(mech->status & JUMPING) 
	    {
	      mech_notify(mech,MECHALL,
			  "You abort your full jump and attempt to land early");
	      if (MadePilotSkillRoll(mech,0))
		{
		  mech_notify(mech, MECHALL, "You are able to abort the jump.");
		  LandMech(mech);
		}
	      else
		{
		  mech_notify(mech, MECHALL, "You don't quite make it.");
		  MechFalls(mech,1);
		  mech->status &= ~JUMPING;
		  mech->jumpheading = JUMP_TO_HIT_RECYCLE;
		  mech->status &= ~DFA_ATTACK;
		  mech->dfatarget = -1;
		  mech->goingx=mech->goingy= 0;
		  mech->speed=0;
		}
	    }
	  else 
	    {
	      notify(player, "You're not jumping!");
	    }
	} 
      else 
	{
	  notify(player, "Your mech has not been started!");
	}
    } 
  else 
    {
      notify(player, "Data not allocated!");
    }
}

/* Facing related */
void mech_heading(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1];
  int newheading;

  mech=(struct mech_data *) data;
  if(mech) {
    if(mech->unconcious) 
      {
	notify(player, "You are unconscious....zzzzzzzzz");
	return;
      }
    if(mech->status & STARTED) {
      if(mech_parseattributes(buffer, args, 1)==1) {
	newheading=atoi(args[0]);
	if(newheading > 360) newheading=newheading % 360;
	while(newheading < 0) newheading +=360;
	mech->desiredfacing=newheading;
	mech_notify(mech,MECHALL, 
			    tprintf("Heading changed to %d.", newheading));
      } else {
	notify(player, "Invalid number of arguments!");
      }
    } else {
      notify(player, "Your mech has not been started!");
    }
  } else {
    notify(player, "Data not allocated!");
  }
}

void mech_turret(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1];
  int newheading;

  mech=(struct mech_data *) data;
  if(mech) {
    if(mech->unconcious) 
      {
	notify(player, "You are unconscious....zzzzzzzzz");
	return;
      }
    if(mech->type==CLASS_MECH || mech->type==CLASS_VEH_VTOL ||
       !mech->sections[TURRET].internal)
      {
	notify(player, "You don't have a turret.");
	return;
      }
    if(mech->critstatus & TURRET_LOCKED)
      {
	notify(player, "Your turret is locked in position.");
	return;
      }
    if(mech->status & STARTED) {
      if(mech_parseattributes(buffer, args, 1)==1) {
	newheading=atoi(args[0]);
	if(newheading > 360) newheading=newheading % 360;
	while(newheading < 0) newheading +=360;
	mech->turretfacing=newheading; 
	mech_notify(mech,MECHALL, 
		    tprintf("Turret facing changed to %d.", newheading));
      } else {
	notify(player, "Invalid number of arguments!");
      }
    } else {
      notify(player, "Your mech has not been started!");
    }
  } else {
    notify(player, "Data not allocated!");
  }
}


void mech_rotatetorso(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[2];

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) {
    if(mech->unconcious) 
      {
	notify(player, "You are unconscious....zzzzzzzzz");
	return;
      }
    if(mech->type!=CLASS_MECH)
      {
	notify(player, "You don't have a torso.");
	return;
      }
    if(!(mech->status & STARTED)) {
      notify(player, "Fusion reactor is not online (start the mech first!)");
      return;
    }

    if(mech_parseattributes(buffer, args, 2)==1) {
      switch(args[0][0]) {
      case 'L':
      case 'l':
	if(mech->status & TORSO_RIGHT) {
	  mech->status = mech->status & ~TORSO_RIGHT;
	  mech->status = mech->status | TORSO_NORMAL;
	} else if(mech->status & TORSO_NORMAL) {
	  mech->status = mech->status & ~TORSO_NORMAL;
	  mech->status = mech->status | TORSO_LEFT;
	}
	mech_notify(mech,MECHALL,"You rotate your torso left");
	break;
      case 'R':
      case 'r':
	if(mech->status & TORSO_LEFT) {
	  mech->status = mech->status & ~TORSO_LEFT;
	  mech->status = mech->status | TORSO_NORMAL;
	} else if(mech->status & TORSO_NORMAL) {
	  mech->status = mech->status & ~TORSO_NORMAL;
	  mech->status = mech->status | TORSO_RIGHT;
	}
	mech_notify(mech,MECHALL,"You rotate your torso right");
	break;
      case 'C':
      case 'c':
	if(mech->status & TORSO_RIGHT) {
	  mech->status = mech->status & ~TORSO_RIGHT;
	  mech->status = mech->status | TORSO_NORMAL;
	} else if(mech->status & TORSO_LEFT) {
	  mech->status = mech->status & ~TORSO_LEFT;
	  mech->status = mech->status | TORSO_NORMAL;
	}
	mech_notify(mech,MECHALL,"You center your torso");
	break;
      default:
	notify(player, "Rotate must have LEFT RIGHT or CENTER");
	break;
      }
    } else {
      notify(player, "Invalid number of arguments!");
    }
  }
}

void mech_speed(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1], buff[50];
  float newspeed, walkspeed, maxspeed;
  
  mech=(struct mech_data *) data;
  if(mech) {
    if(mech->unconcious) 
      {
	notify(player, "You are unconscious....zzzzzzzzz");
	return;
      }
    if(mech->status & STARTED) {
      if(mech_parseattributes(buffer, args, 1)==1) {
	newspeed=atof(args[0]);
	if(mech->move==MOVE_VTOL)
	  maxspeed=sqrt(mech->maxspeed*mech->maxspeed - mech->verticalspeed*mech->verticalspeed);
	else
	  maxspeed=(mech->maxspeed > 0.0) ? mech->maxspeed : 0.0;
	if((mech->heat >= 9.) && (mech->specials & TRIPLE_MYOMER_TECH))
	  maxspeed+=1.5*MP1;
	walkspeed= 2*maxspeed/3;
	if((newspeed > maxspeed) || (newspeed < -walkspeed)) {
	  sprintf(buff, "Max speed is + %d KPH and - %d KPH", 
		  (int)maxspeed, (int)walkspeed);
	  notify(player, buff);
	} else {
	  mech->desired_speed=newspeed;
	  mech_notify(mech,MECHALL,
	  	tprintf("Desired speed changed to %d KPH", (int)newspeed));
	  if(mech->status & FALLEN)
	    {
	      if(mech->type!=CLASS_MECH)
		notify(player, "Your vehicle's movement system is destroyed.");
	      else
		notify(player,"You are currently prone and cannot move.");
	    }
	  if(!(mech->status & JUMPING) && mech->goingy 
	     && mech->type==CLASS_MECH)
	    notify(player,"You are currently standing up and cannot move.");
	}
      } else {
	notify(player, "Invalid number of parameters!");
      }
    } else {
      notify(player, "Your mech has not been started!");
    }
  } else {
    notify(player, "Data not allocated!");
  }
}

void mech_vertical(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1], buff[50];
  float newspeed, walkspeed, maxspeed;
  
  mech=(struct mech_data *) data;
  if(mech) {
    if(mech->unconcious) 
      {
	notify(player, "You are unconscious....zzzzzzzzz");
	return;
      }
    if(mech->type!=CLASS_VEH_VTOL) 
      {
	notify(player, "This command is for VTOLs only.");
	return;
      }
    
    if(mech->status & STARTED) {
      if(mech_parseattributes(buffer, args, 1)==1) {
	newspeed=atof(args[0]);
	maxspeed=sqrt(mech->maxspeed*mech->maxspeed - mech->desired_speed*mech->desired_speed);
	if((newspeed > maxspeed) || (newspeed < -maxspeed)) {
	  sprintf(buff, "Max vertical speed is + %d KPH and - %d KPH", 
		  (int)maxspeed, (int)maxspeed);
	  notify(player, buff);
	} else {
	  if(mech->status & FALLEN)
	    {
	      notify(player, "Your vehicle's movement system is destroyed.");
	      return;
	    }
	  if(mech->status & LANDED)
	    {
	      notify(player, "You need to take off first.");
	      return;
	    }
	  mech->verticalspeed=newspeed;
	  mech_notify(mech,MECHALL,
	  	tprintf("Vertical speed changed to %d KPH", (int)newspeed));
	}
      } else {
	notify(player, "Invalid number of parameters!");
      }
    } else {
      notify(player, "Your vehicle has not been started!");
    }
  } else {
    notify(player, "Data not allocated!");
  }
}

void mech_takeoff(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1], buff[50];
  float newspeed, walkspeed, maxspeed;
  
  mech=(struct mech_data *) data;
  if(mech) {
    if(mech->unconcious) 
      {
	notify(player, "You are unconscious....zzzzzzzzz");
	return;
      }
    if(mech->type==CLASS_MECH)
      {
	notify(player, "Only VTOLs can take off.");
	return;
      }
    if(mech->status & STARTED) {
      if(!(mech->status & LANDED))
	{
	  notify(player, "You haven't landed!");
	  return;
	}
      notify(player, "Your rotor whines overhead as you lift off into the sky.");
      mech->status &= ~LANDED;
      mech->z=mech->elev + 1;
      mech->fz=ZSCALE * mech->z;
    } else {
      notify(player, "Your mech has not been started!");
    }
  } else {
    notify(player, "Data not allocated!");
  }
}

void mech_jump(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  struct mech_data *tempMech;
  struct map_struct *mech_map;
  char *args[3];
  int argc;
  int target;
  char targetID[2];
  int mapx, mapy, bearing;
  float range;
  float realx, realy;

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) {
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
  
    if(mech->unconcious) 
      {
	notify(player, "You are unconscious....zzzzzzzzz");
	return;
      }
    if(mech->type!=CLASS_MECH)
      {
	notify(player, "This vehicle cannot jump like a 'Mech.");
	return;
      }
    if(!(mech->status & STARTED)) {
      notify(player, "Fusion reactor is not online (start the mech first!)");
      return;
    }

    if (mech->status & FALLEN) {
      notify(player, "You can't Jump from a FALLEN position");
      return;
    }

    if (mech->status & JUMPING) {
      notify(player, "You're already jumping!");
      return;
    }

    if(mech->jumpheading) 
      {
        notify(player, "You haven't stabilized from your last jump yet.");
        return;
      }

    if(mech->goingy) 
      {
        notify(player, "You haven't finished standing up yet.");
        return;
      }
    
    if(fabs(mech->jumpspeed)>0.0) 
      {
	argc=mech_parseattributes(buffer, args, 3);
	if(argc==0) 
	  {
	    /* DFA current target... */
	    target=mech->target;
	    tempMech=(struct mech_data *) FindObjectsData(target);
	    if(tempMech)
	      {
		range=FindRange(mech->fx,mech->fy,mech->fz,
				tempMech->fx,tempMech->fy,tempMech->fz);
		if(!InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,range)) 
		  {
		    notify(player, "Target is not in line of sight!");
		    return;
		  }
		mapx = tempMech->x;
		mapy = tempMech->y;
		mech->status |= DFA_ATTACK;
		mech->dfatarget=mech->target;
	      }
	    else
	      {
		notify(player,"Invalid Target!");
		return;
	      }
	  } 
	else if(argc==1) 
	  {
	    /* Jump Target */
	    targetID[0]=args[0][0];
	    targetID[1]=args[0][1];
	    target=FindTargetDBREFFromMapNumber(mech, targetID);
	    tempMech=(struct mech_data *) FindObjectsData(target);
	    if(tempMech)
	      {
		range=FindRange(mech->fx,mech->fy,mech->fz,
				tempMech->fx,tempMech->fy,tempMech->fz);
		if(!InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,range)) 
		  {
		    notify(player, "Target is not in line of sight!");
		    return;
		  }
		mapx = tempMech->x;
		mapy = tempMech->y;
		mech->status |= DFA_ATTACK;
		mech->dfatarget = tempMech->mynum;
	      } 
	    else 
	      {
		notify(player, "Invalid Target!");
		return;
	      }
	  } 
	else if(argc==2) 
	  {
	    bearing=atoi(args[0]);
	    range=atof(args[1]);
	    FindXY(mech->fx,mech->fy,bearing,range,&realx,&realy);
	    
	    /* This is so we are jumping to the center of a hex */
	    /* and the bearing jives with the target hex */
	    RealCoordToMapCoord(&mapx,&mapy,realx,realy);
	  } 
	else if(argc>2)
	  {
	    notify(player, "Too many arguments to JUMP function!");
	    return;
	  }
	
	if(range > (mech->jumpspeed*MP_PER_KPH)) 
	  {
	    notify(player, "That target is out of range!");
	    return;
	  } 
	else 
	  {
	    MapCoordToRealCoord(mapx,mapy,&realx,&realy);
	    bearing=FindBearing(mech->fx,mech->fy,realx,realy);

	    if(mapx >= mech_map->map_width || mapy >= mech_map->map_height 
	       || mapx < 0 || mapy < 0)
	       {
		 notify(player, "That would take you off the map!");
		 return;
	        }

	    if(mech->x == mapx && mech->y == mapy) 
	      {
		/* already there */
		notify(player, "You're already in the target hex.");
		return;
	      }
	    
	    /* TAKE OFF! */
	    mech->jumpheading = bearing;
	    mech->status = mech->status | JUMPING;
	    mech->startfx=mech->fx;
	    mech->startfy=mech->fy;
	    mech->startfz=mech->fz;
	    mech->jumplength=sqrt((realx-mech->startfx)*(realx-mech->startfx) 
			       + (realy-mech->startfy)*(realy-mech->startfy));
	    mech->goingx=mapx;
	    mech->goingy=mapy;
	    mech->endfz=ZSCALE*mech_map->map[mapy][mapx].elev;
	    mech->speed=0.0;
	    if(mech->status & DFA_ATTACK)
	      mech_notify(mech,MECHALL,
		 "You engage your jump jets for a Death From Above attack!");
	    else
	      mech_notify(mech,MECHALL,"You engage your jump jets");
	  }
      } 
    else 
      {
	notify(player, "This mech doesn't have jump jets!");
      }
  }
}

void LandMech(mech)
struct mech_data *mech; 
{
  struct mech_data *target;
  
  /* Handle DFA attack */
  if(mech->status & DFA_ATTACK) 
    {
      /* is the target here? */
      target=(struct mech_data *) FindObjectsData(mech->dfatarget);
      if(target) 
	{
	  if(target->x==mech->x && target->y==mech->y)
	    DeathFromAbove(mech,target);
	  else 
	    {
	      mech_notify(mech,MECHPILOT,"Your DFA target has moved!");
	    }
	}
      else 
	{
	   mech_notify(mech,MECHPILOT, "Your target has become invalid.");
	}
    }
    mech_notify(mech,MECHALL,"You finish your jump.");

  /* Check piloting rolls, etc. */
  if(mech->critstatus & LEG_DESTROYED) 
    {
      mech_notify(mech,MECHPILOT,"Your missing leg makes it harder to land");
      if(!MadePilotSkillRoll(mech,0)) 
	 {
	   mech_notify(mech,MECHALL,
		   "Your missing leg has caused you to fall upon landing!");
	   MechFalls(mech,1);
	 }
    }
  else if(mech->sections[RLEG].basetohit || mech->sections[LLEG].basetohit) 
    {
      mech_notify(mech,MECHPILOT,
			"Your damaged leg actuators make it harder to land");
      if(!MadePilotSkillRoll(mech,0)) 
	 {
	   mech_notify(mech,MECHALL,
    "Your damaged leg actuators have caused you to fall upon landing!");
	   MechFalls(mech,1);
	 }
    }
  mech->status &= ~JUMPING;
  mech->jumpheading = JUMP_TO_HIT_RECYCLE;
  mech->status &= ~DFA_ATTACK;
  mech->dfatarget = -1;
  mech->goingx=mech->goingy= 0;
  mech->speed=0;
  if(mech->terrain==WATER) 
    mech->z = -mech->elev;
  else 
    mech->z = mech->elev;
  mech->fz = mech->z * ZSCALE;
}

void MechFalls(mech,levels)
     struct mech_data *mech;
     int levels;
{
  int roll, spread, i, hitloc, hitGroup, isrear=0, damage, iscritical=0;
  
  /* damage pilot */
  mech_notify(mech,MECHPILOT,"You try to avoid taking damage in the fall."); 
  if(!MadePilotSkillRoll(mech,levels)) 
    {
      mech->pilotstatus++;
      mech_notify(mech,MECHPILOT, 
			"You take 1 point of personal injury from the fall!");
      UpdatePilotStatus(mech);
    }
  
  mech->speed=0;
  mech->desired_speed=0;
  if(mech->move==MOVE_VTOL) 
    {
      mech->verticalspeed = 0;
      mech->status |= LANDED;
    }
  if(mech->type==CLASS_MECH) mech->status |= FALLEN;
  if(mech->terrain==WATER) mech->z = -mech->elev;
  else mech->z = mech->elev;
  mech->fz=mech->z * ZSCALE;

  roll=random() % 6 +1;
  switch(roll)
    {
    case 1:
      hitGroup=FRONT;
      break;
    case 2:
      mech->facing+=60;
      hitGroup=RIGHTSIDE;
      break;
    case 3:
      mech->facing+=120;
      hitGroup=RIGHTSIDE;
      break;
    case 4:
      mech->facing+=180;
      hitGroup=BACK;
      break;
    case 5:
      mech->facing+=240;
      hitGroup=LEFTSIDE;
      break;
    case 6:
      mech->facing+=300;
      hitGroup=LEFTSIDE;
      break;
    }
  if(hitGroup==BACK) isrear=1;
  if(mech->facing >= 360) mech->facing-=360;
  mech->desiredfacing=mech->facing;
  if(mech->terrain!=WATER) damage=levels*(mech->tons+5)/10;
  else damage=levels*(mech->tons+5)/20;
  spread=damage/5;
  for(i=0;i < spread;i++) 
    {
      hitloc=FindHitLocation(mech,hitGroup,&iscritical);
      DamageMech(mech, mech, 0, -1, hitloc, isrear, iscritical, 5, 0);
    }
  if(damage % 5) 
    {
      hitloc=FindHitLocation(mech,hitGroup,&iscritical);
      DamageMech(mech, mech, 0, -1, hitloc, isrear, iscritical, (damage % 5), 0);
    }
}





