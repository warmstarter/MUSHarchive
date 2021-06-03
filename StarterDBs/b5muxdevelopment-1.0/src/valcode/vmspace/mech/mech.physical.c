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
#define M_PI 3.14
#include "../header.h"
#include "mech.h"

void mech_punch(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech, *target;
  struct map_struct *mech_map;
  char *args[5];
  int argc, i, count, hitbase, ltohit, rtohit;
  
  mech=(struct mech_data *) data;
  mech_map=(struct map_struct *)FindObjectsData(mech->mapindex);
  if(!mech_map) {
    notify(player, "You are on an invalid map! Map index reset!");
    mech_shutdown(player,data,"");
    send_mechinfo(tprintf("Punch:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
    mech->mapindex= -1;
    return;
  }
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
	{
	  notify(player,"You are unconcious....zzzzzzz");
	  return;
	}
      if(mech->type!=CLASS_MECH) 
	{
	  notify(player,"You can't punch with this vehicle!");
	  return;
	}
      if(!(mech->status & STARTED)) 
	{
	  notify(player, "Fusion reactor is not online!");
	  return;
	}
      argc=mech_parseattributes(buffer, args, 5);

      if (argc == 0)
        {

	  ltohit = 4 + (2*mech->sections[LARM].basetohit + 
			(mech->critstatus & LHAND_DESTROYED)?1:0);
	  rtohit = 4 + (2*mech->sections[RARM].basetohit + 
		       (mech->critstatus & RHAND_DESTROYED)?1:0);
	  if (mech->critstatus & LARM_DESTROYED)
	    mech_notify(mech,MECHALL,"Your left arm is destroyed, you can't punch with it.");
	  
	  else if (mech->critstatus & LSHDR_DESTROYED)
	    mech_notify(mech,MECHALL,"Your left shoulder is destroyed, you can't punch with that arm.");
	  
	  else
	    PhysicalAttack(mech,10,ltohit, 1,argc,args,mech_map,LARM);
	  if (mech->critstatus & RARM_DESTROYED)
	    mech_notify(mech,MECHALL,"Your right arm is destroyed, you can't punch with it.");
	  else if (mech->critstatus & RSHDR_DESTROYED)
	    mech_notify(mech,MECHALL,"Your right shoulder is destroyed, you can't punch with that arm.");
	  else
	    PhysicalAttack(mech,10,rtohit, 1,argc,args,mech_map,RARM);
        }          
      else 
        {
          if (args[0][0] == 'L' || args[0][0] == 'B' ||
              args[0][0] == 'l' || args[0][0] == 'b')
            {
	      if (mech->critstatus & LARM_DESTROYED)
		mech_notify(mech,MECHALL,"Your left arm is destroyed, you can't punch with it.");
	      
	      else if (mech->critstatus & LSHDR_DESTROYED)
		mech_notify(mech,MECHALL,"Your left shoulder is destroyed, you can't punch with that arm.");
	      else
		PhysicalAttack(mech,10,ltohit,1,argc-1,&args[1],mech_map,LARM);
            }
          if (args[0][0] == 'R' || args[0][0] == 'B' || 
              args[0][0] == 'r' || args[0][0] == 'b')
            {
	      if (mech->critstatus & RARM_DESTROYED)
		mech_notify(mech,MECHALL,"Your right arm is destroyed, you can't punch with it.");
	      
	      else if (mech->critstatus & RSHDR_DESTROYED)
		mech_notify(mech,MECHALL,"Your right shoulder is destroyed, you can't punch with that arm.");
	      else
		PhysicalAttack(mech,10,rtohit,1,argc-1,&args[1],mech_map,RARM);
            }          
        }
    }
}

void mech_club(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech, *target;
  struct map_struct *mech_map;
  char *args[5];
  int argc;

  mech=(struct mech_data *) data;
  mech_map=(struct map_struct *)FindObjectsData(mech->mapindex);
  if(!mech_map) {
    notify(player, "You are on an invalid map! Map index reset!");
    mech_shutdown(player,data,"");
    send_mechinfo(tprintf("Club:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
    mech->mapindex= -1;
    return;
  }
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
	{
	  notify(player,"You are unconcious....zzzzzzz");
	  return;
	}
      if(mech->type!=CLASS_MECH) 
	{
	  notify(player,"You can't club with this vehicle!");
	  return;
	}
      if(!(mech->status & STARTED)) 
	{
	  notify(player, "Fusion reactor is not online!");
	  return;
	}
      if (mech->terrain != HEAVY_FOREST && mech->terrain != LIGHT_FOREST)
        {
          mech_notify(mech,MECHALL, "You can not seem to find any trees around to club with.");
          return;
        }
      argc=mech_parseattributes(buffer, args, 5);
      if (mech->critstatus & LARM_DESTROYED)
	mech_notify(mech,MECHALL,"Your left arm is destroyed, you can't club.");
      else if (mech->critstatus & RARM_DESTROYED)
	mech_notify(mech,MECHALL,"Your right arm is destroyed, you can't club.");
      else if (mech->critstatus & RHAND_DESTROYED)
	mech_notify(mech,MECHALL,"You can't pick anything up without your right hand.");
      else if (mech->critstatus & LHAND_DESTROYED)
	mech_notify(mech,MECHALL,"You can't pick anything up without your left hand.");
      else
	PhysicalAttack(mech,5,4+2*mech->sections[RARM].basetohit+
		       2*mech->sections[LARM].basetohit,
		       2,argc,args,mech_map, RARM);
    }
}

void mech_kick(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech, *target;
  struct map_struct *mech_map;
  char *args[5];
  int argc;

  mech=(struct mech_data *) data;
  mech_map=(struct map_struct *)FindObjectsData(mech->mapindex);
  if(!mech_map) {
    notify(player, "You are on an invalid map! Map index reset!");
    mech_shutdown(player,data,"");
    send_mechinfo(tprintf("Kick:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
    mech->mapindex= -1;
    return;
  }
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
	{
	  notify(player,"You are unconcious....zzzzzzz");
	  return;
	}
      if(mech->type!=CLASS_MECH) 
	{
	  notify(player,"You can't kick with this vehicle!");
	  return;
	}
      if(!(mech->status & STARTED)) 
	{
	  notify(player, "Fusion reactor is not online!");
	  return;
	}

      if (mech->critstatus & BOTH_HIPS_DAMAGED)
	{
	  mech_notify(mech, MECHALL, "Your hips are immobilized, you can't kick");
	  return;
	}
      argc=mech_parseattributes(buffer, args, 5);
      if (argc > 1) /* changed from argc >=1 by MW 93 Oct */
        PhysicalAttack(mech,5,3,3,argc-1,&args[1],mech_map,
                       (args[0][0]=='R'||args[0][0]=='r')?RLEG:LLEG);
      else 
        PhysicalAttack(mech,5,3,3,argc,args,mech_map,RLEG);
    }
}

void mech_charge(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech, *target;
  struct map_struct *mech_map;
  float range;
  int targetnum;
  char targetID[5];
  char *args[5];
  int argc;

  mech=(struct mech_data *) data;
  mech_map=(struct map_struct *)FindObjectsData(mech->mapindex);
  if(!mech_map) {
    notify(player, "You are on an invalid map! Map index reset!");
    mech_shutdown(player,data,"");
    send_mechinfo(tprintf("Charge:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
    mech->mapindex= -1;
    return;
  }
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
	{
	  notify(player,"You are unconcious....zzzzzzz");
	  return;
	}
      if(mech->type!=CLASS_MECH) 
	{
	  notify(player,"You can't charge with this vehicle!");
	  return;
	}
      if(!(mech->status & STARTED)) 
	{
	  notify(player, "Fusion reactor is not online!");
	  return;
	}
      argc=mech_parseattributes(buffer, args, 2);
      if(argc == 0)  /* default target */
        {
          if (mech->target == -1)
            {
              mech_notify(mech,MECHALL, "You do not have a default target set!");
              return;
            }
          target=(struct mech_data *) FindObjectsData(mech->target);
          if(!target)
            {
              mech_notify(mech,MECHALL, "Invalid default target!");
              mech->target = -1;
              return;
            }
          mech->chgtarget = mech->target;
          mech_notify(mech,MECHALL, "Charge target set to default target.");
        }
      else if (argc == 1) /* ID number */
        {
          if(args[0][0]=='-') 
            {
              mech->chgtarget = -1;
              mech_notify(mech,MECHPILOT,"You are no longer charging.");
              return;
            }
          targetID[0]=args[0][0];
          targetID[1]=args[0][1];
          targetnum=FindTargetDBREFFromMapNumber(mech, targetID);
          if(targetnum == -1) 
            {
              notify(mech->pilot,"That is not a valid target ID!");
              return;
            }
          target = (struct mech_data *)FindObjectsData(targetnum);
          if(!target)
            {
              mech_notify(mech,MECHALL, "Invalid target data!");
              return;
            }
          mech->chgtarget = targetnum;
          mech_notify(mech,MECHALL, 
                      tprintf("Charge target set to %c%c.",
                              target->ID[0],
                              target->ID[1]));
        }
    }
}

void PhysicalAttack(mech, damageweight, baseToHit, AttackType, 
                           argc, args, mech_map, sect)
struct mech_data *mech;
struct map_struct *mech_map;
int damageweight, baseToHit, AttackType, argc, sect;
char **args;
{
  struct mech_data *target;
  float range;
  char targetID[2];
  int targetnum, i, done=0, count, roll;
  unsigned char weaptype[MAX_WEAPS_SECTION];
  unsigned char weapdata[MAX_WEAPS_SECTION];
  int critical[MAX_WEAPS_SECTION];
  char location[20];
  char mech_name[100];
  
  if (mech->status & FALLEN)
    {
      mech_notify(mech,MECHALL,"You can't attack from a prone position.");
      return;
    }

  /* Weapons recycling check on major section */
  count=FindWeapons(mech, sect, weaptype, weapdata, critical);
  for(i = 0; !done && i < count; i++)
    {
      if (mech->sections[sect].criticals[critical[i]].data > 0
	  && mech->sections[sect].criticals[critical[i]].data!=CRIT_DESTROYED) 
        {
          ArmorStringFromIndex(sect,location,mech->type);
          mech_notify(mech,MECHALL,
                 tprintf("You have weapons recycling on your %s.",location));
          return;
        }
    }

  /* PUNCH crosschecks */
  if(AttackType == 1) 
    {
      if (mech->sections[LLEG].recycle != 0 
          || mech->sections[RLEG].recycle != 0)
        {
          mech_notify(mech,MECHALL, "You're still recovering from your kick.");
          return;
        }
    }
  
  /* CLUB crosschecks */
  if (AttackType == 2)
    {
      /* Check Weapons recycling on LARM because we only checked RARM above. */
      count=FindWeapons(mech, LARM, weaptype, weapdata, critical);
      for(i = 0; !done && i < count; i++)
        {
          if (mech->sections[LARM].criticals[critical[i]].data > 0 
	      && mech->sections[sect].criticals[critical[i]].data!=CRIT_DESTROYED) 
            {
              mech_notify(mech, MECHALL,
                     "You have weapons recycling on your Left Arm.");
              return;
            }
        }
      if (mech->sections[LLEG].recycle != 0 
          || mech->sections[RLEG].recycle != 0)
        {
          mech_notify(mech,MECHALL, "You're still recovering from your kick.");
          return;
        }
      if(mech->sections[LARM].recycle != 0)
	{
	  mech_notify(mech,MECHALL, 
	  		"Your Left Arm is not ready to attack again.");
	  return;
	}
    }

  /* KICK crosschecks */
  if (AttackType == 3)
    {
      if (mech->sections[LLEG].recycle != 0 
          || mech->sections[RLEG].recycle != 0)
        {
          mech_notify(mech,MECHALL,"Your legs are not ready to attack again.");
          return;
        }
      if (mech->sections[RARM].recycle != 0 
          || mech->sections[LARM].recycle != 0)
        {
	  mech_notify(mech,MECHALL, 
          	    "Your arms are still recovering from your last attack.");
          return;
        }
    }
  
  /* major section recycle */
  if (mech->sections[sect].recycle != 0)
    {
      ArmorStringFromIndex(sect,location,mech->type);
      mech_notify(mech,MECHALL, 
      		tprintf("Your %s is not ready to attack again.", location));
      return;
    }
  
  if(argc == 0)  /* default target */
    {
      if (mech->target == -1)
        {
          mech_notify(mech,MECHALL, "You do not have a default target set!");
          return;
        }
      target=(struct mech_data *) FindObjectsData(mech->target);
      if(!target)
        {
          mech_notify(mech,MECHALL, "Invalid default target!");
          return;
        }
      if ((range = FindRange(mech->fx,mech->fy,mech->fz,
			     target->fx,target->fy,target->fz)) >= 1)
        {
          mech_notify(mech,MECHALL,"Target out of range!");
          return;
        }
    }
  else /* Any number of targets, take only the first -- mw 93 Oct */
    {
      targetID[0]=args[0][0];
      targetID[1]=args[0][1];
      targetnum=FindTargetDBREFFromMapNumber(mech, targetID);
      if(targetnum == -1) 
        {
          notify(mech->pilot,"That is not a valid target ID!");
          return;
        }
      target = (struct mech_data *)FindObjectsData(targetnum);
      if(!target)
        {
          mech_notify(mech,MECHALL, "Invalid default target!");
          return;
        }
      if ((range = FindRange(mech->fx,mech->fy,mech->fz,
			     target->fx,target->fy,target->fz)) >= 1)
        {
          mech_notify(mech,MECHALL,"Target out of range!");
          return;
        }
    }      
  
  if(target->move!=MOVE_VTOL)
    {
      if (AttackType == 1 && mech->z > target->z)
	{
	  mech_notify(mech, MECHALL,
		      "The target is too low in elevation for you to punch at.");
	  return;
	}
      if (AttackType == 3 && mech->z < target->z)
	{
	  mech_notify(mech, MECHALL,
		      "The target is too high in elevation for you to kick at.");
	  return;
	}
      if (mech->z - target->z > 1 || target->z - mech->z > 1)
	{
	  mech_notify(mech, MECHALL,
		      "You can't attack, the elevation difference is too large.");
	  return;
	}
    }
  else
    {
      if (AttackType == 1 && target->z-mech->z < 3 
	  && target->z-mech->z > 0)
	{
	  mech_notify(mech, MECHALL,
		      "The target is too far away for you to punch at.");
	  return;
	}
      
      if (AttackType == 3 && mech->z == target->z )
	{
	  mech_notify(mech, MECHALL,
		      "The target is too far away for you to kick at.");
	  return;
	}
      if (target->z-mech->z > -1 && target->z-mech->z < 4)
	{
	  mech_notify(mech, MECHALL,
		      "You can't attack, the elevation difference is too large.");
	  return;
	}
    }
  
  
  /* Add in the movement modifiers */
  baseToHit+=AttackMovementMods(mech);
  baseToHit+=TargetMovementMods(target,0.0);
  
  get_mech_atr(mech, mech_name, "MECHNAME");
  
  roll = Roll();
  if (AttackType == 1)
    {
      if((target->status & FALLEN && target->type==CLASS_MECH) ||
	 target->type!=CLASS_MECH) 
	{
	  mech_notify(mech, MECHALL, "The target is too low to punch!");
	  return;
	}
      if((target->status & JUMPING) || (mech->status & JUMPING &&
					target->type==CLASS_MECH)) 
	{
	  mech_notify(mech, MECHALL,
	              "You cannot physically attack a jumping mech!");
	  return;
	}
      if(!(mech->status & JUMPING) && mech->goingy && mech->type==CLASS_MECH) 
	{
	  mech_notify(mech, MECHALL,
	              "You are still trying to stand up!");
	  return;
	}
      mech_notify(mech,MECHALL,
tprintf("You try and punch the target %c%c.  BaseToHit:  %d,\tRoll:  %d",
                                  target->ID[0],target->ID[1],baseToHit,roll));
      mech_notify(target,MECHSTARTED, tprintf("%s:[%c%c] tries to punch you!",
				mech_name,mech->ID[0],mech->ID[1]));
    }
  else if (AttackType == 2)
    {
      if((target->status & JUMPING) || (mech->status & JUMPING &&
					target->type==CLASS_MECH)) 
	{
	  mech_notify(mech,MECHALL,
	  		"Jumping mechs cannot engage in physical combat!");
	  return;
	}
      if(!(mech->status & JUMPING) && mech->goingy && mech->type==CLASS_MECH) 
	{
	  mech_notify(mech, MECHALL,
	              "You are still trying to stand up!");
	  return;
	}
      mech_notify(mech,MECHALL,
tprintf("You try and club the target %c%c.  BaseToHit:  %d,\tRoll:  %d",
                                  target->ID[0],target->ID[1],baseToHit,roll));
      mech_notify(target,MECHSTARTED,tprintf("%s:[%c%c] tries to club you!",
				mech_name,mech->ID[0],mech->ID[1]));
    }
  else 
    {
      if((target->status & JUMPING) || (mech->status & JUMPING &&
					target->type==CLASS_MECH)) 
	{
	  mech_notify(mech,MECHALL,
	  		"Jumping mechs cannot engage in physical combat!");
	  return;
	}
      if(!(mech->status & JUMPING) && mech->goingy && mech->type==CLASS_MECH) 
	{
	  mech_notify(mech, MECHALL,
	              "You are still trying to stand up!");
	  return;
	}
      mech_notify(mech,MECHALL,
tprintf("You try and kick the target %c%c.  BaseToHit:  %d,\tRoll:  %d",
                                  target->ID[0],target->ID[1],baseToHit,roll));
      mech_notify(target,MECHSTARTED,tprintf("%s:[%c%c] tries to kick you!",
				mech_name,mech->ID[0],mech->ID[1]));
    }
  
  /* set the sections to recycling */
  mech->sections[sect].recycle = PHYSICAL_RECYCLE_TIME;
  if(AttackType == 2) mech->sections[LARM].recycle = PHYSICAL_RECYCLE_TIME;
    
  if(roll >= baseToHit) 
    {
      /*  hit the target */
      PhysicalDamage(mech,target,damageweight,AttackType, sect);
    }
  else if(AttackType==3 || AttackType==2)
    {
      mech_notify(mech,MECHALL,"You miss and try to remain standing!");
      if(!MadePilotSkillRoll(mech,0)) 
	{
	  mech_notify(mech,MECHALL,"You lose your balance and fall down!");
	  MechFalls(mech,1);
	}
    }
}

void PhysicalDamage(mech,target,weightdmg, AttackType, sect)
struct mech_data *mech, *target;
int weightdmg, AttackType, sect;
{
  int hitloc, damage, hitgroup, isrear, iscritical;
  
  damage = mech->tons/weightdmg;
  if((mech->heat >= 9.) && (mech->specials & TRIPLE_MYOMER_TECH))
    {
      damage = damage*2;
    }
  switch(AttackType)
    {
    case 1:
      if (sect == LARM && mech->critstatus & LLOWACT_DESTROYED)
	{
	  damage = damage / 2;
	  if (mech->critstatus & LUPACT_DESTROYED)
	    damage = damage / 2;
	}
      else if (sect == RARM && mech->critstatus & RLOWACT_DESTROYED)
	{
	  damage = damage / 2;
	  if (mech->critstatus & RUPACT_DESTROYED)
	    damage = damage / 2;
	}
    case 2:
      hitgroup = FindAreaHitGroup(mech,target);
      if(mech->type==CLASS_MECH)
	{
	  if (mech->elev < target->elev)
	    hitloc = FindKickLocation(hitgroup);
	  else
	    hitloc = FindPunchLocation(hitgroup);
	}
      else
	hitloc=FindTargetHitLoc(mech, target, &isrear, &iscritical);
      break;
    case 3:      
      if (target->status & FALLEN || target->type!=CLASS_MECH)
	hitloc=FindTargetHitLoc(mech, target, &isrear, &iscritical); 
      else
	{
          hitgroup = FindAreaHitGroup(mech,target);
	  if (mech->elev > target->elev)
            hitloc = FindPunchLocation(hitgroup);
          else
            hitloc = FindKickLocation(hitgroup);
	}
      if(target->type==CLASS_MECH)
	if(!MadePilotSkillRoll(target,0)) 
	  {
	    mech_notify(target,MECHSTARTED,"The kick knocks you to the ground!");
	    MechFalls(target,1);
	  }
      break;
    }
  DamageMech(target,mech,1,mech->pilot, hitloc, (hitgroup == BACK)?1:0, 0,
	     damage, 0);
}

const int resect[4]={LARM, RARM, LLEG, RLEG}; 

void DeathFromAbove(mech,target)
     struct mech_data *mech;
     struct mech_data *target;
{
  int baseToHit;
  int roll;
  int hitGroup;
  int hitloc;
  int isrear=0;
  int iscritical=0;
  int target_damage;
  int mech_damage;
  int spread;
  int i;
  char mech_name[100];
  int count;
  unsigned char weaptype[MAX_WEAPS_SECTION];
  unsigned char weapdata[MAX_WEAPS_SECTION];
  int critical[MAX_WEAPS_SECTION];
  int j;
  int done=0;
  char location[50];

  /* Weapons recycling check on each major section */
  for(j=0; j<4; j++)
    {
      count=FindWeapons(mech, resect[j], weaptype, weapdata, critical);
      for(i = 0; i < count; i++)
        {
          if (mech->sections[resect[j]].criticals[critical[i]].data > 0
              && mech->sections[resect[j]].criticals[critical[i]].data!=CRIT_DESTROYED) 
            {
              ArmorStringFromIndex(resect[j],location,mech->type);
              mech_notify(mech,MECHALL,
                          tprintf("You have weapons recycling on your %s.",location));
              return;
            }
        }
    }
  if (mech->sections[LLEG].recycle != 0 
          || mech->sections[RLEG].recycle != 0)
    {
      mech_notify(mech,MECHALL,
                  "Your legs are still recovering from your last attack.");
      return;
    }
  if (mech->sections[RARM].recycle != 0 
      || mech->sections[LARM].recycle != 0)
    {
      mech_notify(mech,MECHALL, 
                  "Your arms are still recovering from your last attack.");
      return;
    }
  if(target->status & JUMPING) 
    {
      mech_notify(mech,MECHALL,
      			"Your target is jumping, you cannot land on it.");
      return;
    }
  
  baseToHit=5;
  baseToHit+=AttackMovementMods(mech);
  baseToHit+=TargetMovementMods(target,0.0);
  if (baseToHit > 12)
    {
	mech_notify(mech,MECHALL,tprintf(
"DFA: baseToHit %d\tYou choose not to attack and land from your jump.",
							baseToHit)); 
      return;
    }
  
  get_mech_atr(mech, mech_name, "MECHNAME");

  roll=Roll();
  mech_notify(mech,MECHALL,tprintf(
    			"DFA: baseToHit %d\tRoll: %d",baseToHit,roll));
  if(roll >= baseToHit) 
    {
      /* OUCH */
      mech_notify(target,MECHSTARTED,
                  tprintf("DEATH FROM ABOVE!!!\n%s:[%c%c] lands on you from above!",
                          mech_name,
                          mech->ID[0],
                          mech->ID[1]));
      mech_notify(mech,MECHALL,"You land on your target legs first!");
      hitGroup=FindAreaHitGroup(mech,target);
      if(hitGroup==BACK) isrear=1;
      target_damage=(3*mech->tons+15)/10;
      spread=target_damage/5;
      for(i=0;i < spread;i++) 
	{
	  if(target->status & FALLEN || target->type!=CLASS_MECH)
	    hitloc=FindHitLocation(target,hitGroup,&iscritical);
	  else
	    hitloc=FindPunchLocation(hitGroup);
	  DamageMech(target, mech, 1, mech->pilot, hitloc, isrear, iscritical, 5, 0);
	}
      if(target_damage % 5) 
	{
	  if(target->status & FALLEN || target->type!=CLASS_MECH)
	    hitloc=FindHitLocation(target,hitGroup,&iscritical);
	  else
	    hitloc=FindPunchLocation(hitGroup);
	  DamageMech(target, mech, 1, mech->pilot, hitloc, isrear, iscritical, (target_damage % 5), 0);
	}
      mech_damage=(2*mech->tons+10)/10;
      spread=mech_damage/5;
      for(i=0;i < spread;i++) 
	{
	  hitloc=FindKickLocation(FRONT);
	  DamageMech(mech, mech, 0, -1, hitloc, 0, 0, 5, 0);
	}
      if(mech_damage % 5) 
	{
	  hitloc=FindKickLocation(FRONT);
	  DamageMech(mech, mech, 0, -1, hitloc, 0, 0, (mech_damage % 5), 0);
	}
      if(!MadePilotSkillRoll(mech,4)) 
	{
	  mech_notify(mech,MECHALL,
	    		"Your piloting skill fails and you fall over!!");
	  MechFalls(mech,1);
	}
      if(!MadePilotSkillRoll(target,2)) 
	{
	    mech_notify(target,MECHSTARTED,
	         "Your piloting skill fails and you fall over!!");
	  MechFalls(target,1);
	}
    }
  else 
    {
      /* Missed DFA attack */
      mech_notify(mech,MECHALL,"You miss your DFA attack and fall on your back!!");
      mech_notify(target,MECHSTARTED,
	       tprintf("%s:[%c%c] lands on its back right next to you!",
		       mech_name,
		       mech->ID[0],
		       mech->ID[1]));
      mech_damage=(3*mech->tons+15)/10;
      spread=mech_damage/5;
      for(i=0;i < spread;i++) 
	{
	  hitloc=FindHitLocation(mech,BACK,&iscritical);
	  DamageMech(mech, mech, 0, -1, hitloc, 1, iscritical, 5, 0);
	}
      if(mech_damage % 5) 
	{
	  hitloc=FindHitLocation(mech,BACK,&iscritical);
	  DamageMech(mech, mech, 0, -1, hitloc, 1, iscritical, (mech_damage % 5), 0);
	}
      /* now damage pilot */
      if(!MadePilotSkillRoll(mech,2)) 
	{
	  mech->pilotstatus++;
	  mech_notify(mech,MECHALL,
	  		"You take 1 point of personal injury from the fall!");
	  UpdatePilotStatus(mech);
	}
      mech->speed=0.0;
      mech->desired_speed=0.0;
      mech->status |= FALLEN;
      if(mech->terrain==WATER) mech->z = -mech->elev;
      else mech->z = mech->elev;
      mech->fz = mech->z * ZSCALE;
    }
  mech->sections[RARM].recycle = PHYSICAL_RECYCLE_TIME;
  mech->sections[LARM].recycle = PHYSICAL_RECYCLE_TIME;
  mech->sections[RLEG].recycle = PHYSICAL_RECYCLE_TIME;
  mech->sections[LLEG].recycle = PHYSICAL_RECYCLE_TIME;
}

void ChargeMech(mech,target)
     struct mech_data *mech;
     struct mech_data *target;
{
  int baseToHit;
  int roll;
  int hitGroup;
  int hitloc;
  int isrear=0;
  int iscritical=0;
  int target_damage;
  int mech_damage;
  int spread;
  int i;
  int mech_charge;
  int target_charge;
  int mech_baseToHit;
  int targ_baseToHit;
  int mech_roll;
  int targ_roll;
  char mech_name[100];
  char target_name[100];
  int count;
  unsigned char weaptype[MAX_WEAPS_SECTION];
  unsigned char weapdata[MAX_WEAPS_SECTION];
  int critical[MAX_WEAPS_SECTION];
  int j;
  int done=0;
  char location[50];
  

  if (target->chgtarget != mech->mynum) 
    {
      /* Weapons recycling check on each major section */
      for(j=0; j<4; j++)
        {
          count=FindWeapons(mech, resect[j], weaptype, weapdata, critical);
          for(i = 0; i < count; i++)
            {
              if (mech->sections[resect[j]].criticals[critical[i]].data > 0
                  && mech->sections[resect[j]].criticals[critical[i]].data!=CRIT_DESTROYED) 
                {
                  ArmorStringFromIndex(resect[j],location,mech->type);
                  mech_notify(mech,MECHALL,
                              tprintf("You have weapons recycling on your %s.",location));
                  return;
                }
            }
        }
      if (mech->speed < MP1)
        {
          mech_notify(mech, MECHALL, "You aren't moving fast enough to charge.");
          return;
        }
      if (mech->sections[LLEG].recycle != 0 
          || mech->sections[RLEG].recycle != 0)
        {
          mech_notify(mech,MECHALL, "Your legs are still recovering from your last attack.");
          return;
        }
      if (mech->sections[RARM].recycle != 0 
          || mech->sections[LARM].recycle != 0)
        {
          mech_notify(mech,MECHALL, 
                      "Your arms are still recovering from your last attack.");
          return;
        }
      if(target->status & JUMPING) 
        {
          mech_notify(mech,MECHALL,
                      "Your target is jumping, you charge underneath it.");
          return;
        }
      if(mech->status & JUMPING) 
        {
          mech_notify(mech,MECHALL, "You can't charge while jumping, try death from above.");
          return;
        }
      if(InWeaponArc(mech,target->fx,target->fy)!=FORWARDARC) 
        {
          mech_notify(mech,MECHALL, "Your charge target is not in your forward arc and you are unable to charge it.");
          return;
        }
      
      target_damage=((mech->speed - target->speed*cos((mech->facing - target->facing)*(M_PI/180.)))*MP_PER_KPH)*(mech->tons+5)/10;
      
      if(target_damage <= 0) 
        {
          mech_notify(mech,MECHPILOT,"Your target pulls away from you and you are unable to charge it.");
          return;
        }
      
      
      baseToHit=5;
      baseToHit+=FindPilotPiloting(mech) - FindPilotPiloting(target);
      baseToHit+=AttackMovementMods(mech);
      baseToHit+=TargetMovementMods(target,0.0);
      if (baseToHit > 12)
        {
          mech_notify(mech,MECHALL,
                      tprintf("Charge: baseToHit %d\tYou choose not to charge.",baseToHit)); 
          return;
        }

      get_mech_atr(mech, mech_name, "MECHNAME");

      roll=Roll();
      mech_notify(mech,MECHALL,tprintf(
                                       "Charge: baseToHit %d\tRoll: %d",baseToHit,roll));
      if(roll >= baseToHit) 
        {
          /* OUCH */
          mech_notify(target,MECHSTARTED,
                      tprintf("CRASH!!!\n%s:[%c%c] charges into you!",
                              mech_name,
                              mech->ID[0],
                              mech->ID[1]));
          mech_notify(mech,MECHALL,"SMASH!!! You crash into your target!");
          hitGroup=FindAreaHitGroup(mech,target);
          if(hitGroup==BACK) isrear=1;
          else isrear=0;
          
          spread=target_damage/5;
          for(i=0;i < spread;i++) 
            {
              hitloc=FindHitLocation(target,hitGroup,&iscritical);
              DamageMech(target, mech, 1, mech->pilot, hitloc, isrear, iscritical, 5, 0);
            }
          if(target_damage % 5) 
            {
              hitloc=FindHitLocation(target,hitGroup,&iscritical);
              DamageMech(target, mech, 1, mech->pilot, hitloc, isrear, iscritical, (target_damage % 5), 0);
            }
          hitGroup=FindAreaHitGroup(target,mech);
          if(hitGroup==BACK) isrear=1;
          else isrear=0;
          mech_damage=(target->tons+5)/10;
          spread=mech_damage/5;
          for(i=0;i < spread;i++) 
            {
              hitloc=FindHitLocation(mech,hitGroup,&iscritical);
              DamageMech(mech, mech, 0, -1, hitloc, isrear, iscritical, 5, 0);
            }
          if(mech_damage % 5) 
            {
              hitloc=FindHitLocation(mech,hitGroup,&iscritical);
              DamageMech(mech, mech, 0, -1, hitloc, isrear, iscritical, (mech_damage % 5), 0);
            }
          if(!MadePilotSkillRoll(mech,2)) 
            {
              mech_notify(mech,MECHALL,
                          "Your piloting skill fails and you fall over!!");
              MechFalls(mech,1);
            }
          if(!MadePilotSkillRoll(target,2)) 
            {
              mech_notify(target,MECHSTARTED,
                          "Your piloting skill fails and you fall over!!");
              MechFalls(target,1);
            }
          mech->speed = 0;   /* stop him and let him accelerate again */
        }
      mech->sections[RLEG].recycle = PHYSICAL_RECYCLE_TIME;
      mech->sections[LLEG].recycle = PHYSICAL_RECYCLE_TIME;
      mech->sections[RARM].recycle = PHYSICAL_RECYCLE_TIME;
      mech->sections[LARM].recycle = PHYSICAL_RECYCLE_TIME;
    }
  else    /* They are both charging each other */
    {
      mech_charge = 1;
      target_charge = 1;
      done=0;
      /* Weapons recycling check on each major section */
      for(j=0; j<4 && !done; j++)
        {
          count=FindWeapons(mech, resect[j], weaptype, weapdata, critical);
          for(i = 0; i < count && !done; i++)
            {
              if (mech->sections[resect[j]].criticals[critical[i]].data > 0
                  && mech->sections[resect[j]].criticals[critical[i]].data!=CRIT_DESTROYED) 
                {
                  ArmorStringFromIndex(resect[j],location,mech->type);
                  mech_notify(mech,MECHALL,
                              tprintf("You have weapons recycling on your %s.",location));
                  mech_charge=0;
                  done=1;
                }
            }
        }
      done=0;
      for(j=0; j<4 && !done; j++)
        {
          count=FindWeapons(target, resect[j], weaptype, weapdata, critical);
          for(i = 0; i < count && !done; i++)
            {
              if (target->sections[resect[j]].criticals[critical[i]].data > 0
                  && target->sections[resect[j]].criticals[critical[i]].data!=CRIT_DESTROYED) 
                {
                  ArmorStringFromIndex(resect[j],location,mech->type);
                  mech_notify(target,MECHALL,
                              tprintf("You have weapons recycling on your %s.",location));
                  target_charge=0;
                  done=1;
                }
            }
        }
      if (!(target->status & STARTED) || target->unconcious) 
        target_charge = 0;
      if (!(mech->status & STARTED) || mech->unconcious) 
        mech_charge = 0;
      if (mech->speed < MP1)
        {
          mech_notify(mech, MECHALL, "You aren't moving fast enough to charge.");
          mech_charge = 0;
        }
      if (target->speed < MP1)
        {
          mech_notify(target, MECHALL, "You aren't moving fast enough to charge.");
          target_charge = 0;
        }
      if (mech->sections[LLEG].recycle != 0 
          || mech->sections[RLEG].recycle != 0)
        {
          mech_notify(mech,MECHALL, "Your legs are still recovering from your last attack.");
          mech_charge = 0;
        }
      if (target->sections[LLEG].recycle != 0 
          || target->sections[RLEG].recycle != 0)
        {
          mech_notify(target,MECHALL, "Your legs are still recovering from your last attack.");
          target_charge = 0;
        }
      if (mech->sections[RARM].recycle != 0 
          || mech->sections[LARM].recycle != 0)
        {
          mech_notify(mech,MECHALL, 
                      "Your arms are still recovering from your last attack.");
          mech_charge = 0;
        }
      if (target->sections[RARM].recycle != 0 
          || target->sections[LARM].recycle != 0)
        {
          mech_notify(target,MECHALL, 
                      "Your arms are still recovering from your last attack.");
          target_charge = 0;
        }
      if(target->status & JUMPING) 
        {
          mech_notify(mech,MECHALL,
                      "Your target is jumping, you charge underneath it.");
          mech_notify(target,MECHALL, "You can't charge while jumping, try death from above.");
          mech_charge = 0;
          target_charge = 0;
        }
      if(mech->status & JUMPING) 
        {
          mech_notify(target,MECHALL,
                      "Your target is jumping, you charge underneath it.");
          mech_notify(mech,MECHALL, "You can't charge while jumping, try death from above.");
          mech_charge = 0;
          target_charge = 0;
        }
      if(InWeaponArc(mech,target->fx,target->fy)!=FORWARDARC) 
        {
          mech_notify(mech,MECHALL, "Your charge target is not in your forward arc and you are unable to charge it.");
          mech_charge = 0;
        }
      if(InWeaponArc(target,mech->fx,mech->fy)!=FORWARDARC) 
        {
          mech_notify(target,MECHALL, "Your charge target is not in your forward arc and you are unable to charge it.");
          target_charge = 0;
        }

      target_damage=((mech->speed - target->speed*cos((mech->facing - target->facing)*(M_PI/180.)))*MP_PER_KPH)*(mech->tons+5)/10;
      if(target_damage <= 0) 
        {
          mech_notify(mech,MECHPILOT,"Your target pulls away from you and you are unable to charge it.");
          mech_charge = 0;
        }

      mech_damage=((target->speed - mech->speed*cos((target->facing - mech->facing)*(M_PI/180.)))*MP_PER_KPH)*(target->tons+5)/10;
      if(mech_damage <= 0) 
        {
          mech_notify(target,MECHPILOT,"Your target pulls away from you and you are unable to charge it.");
          target_charge = 0;
        }

      mech_baseToHit=5;
      mech_baseToHit+=FindPilotPiloting(mech) - FindPilotPiloting(target);
      mech_baseToHit+=AttackMovementMods(mech);
      mech_baseToHit+=TargetMovementMods(target,0.0);

      targ_baseToHit=5;
      targ_baseToHit+=FindPilotPiloting(target) - FindPilotPiloting(mech);
      targ_baseToHit+=AttackMovementMods(target);
      targ_baseToHit+=TargetMovementMods(mech,0.0);

      if (mech_charge)
        if (mech_baseToHit > 12)
          {
            mech_notify(mech,MECHALL,
                        tprintf("Charge: baseToHit %d\tYou choose not to charge.",mech_baseToHit)); 
            mech_charge = 0;
          }
      if(target_charge)
        if (targ_baseToHit > 12)
          {
            mech_notify(target,MECHALL,
                        tprintf("Charge: baseToHit %d\tYou choose not to charge.",targ_baseToHit)); 
            target_charge = 0;
          }
      
      if(!mech_charge && !target_charge) 
        {
          target->chgtarget = -1;  /* mech->chgtarget is set after the return */
          return;
        }
      
      get_mech_atr(mech, mech_name, "MECHNAME");
      get_mech_atr(target, target_name, "MECHNAME");

      mech_roll=Roll();
      targ_roll=Roll();
      
      if(mech_charge)
        mech_notify(mech,MECHALL,tprintf("Charge: baseToHit %d\tRoll: %d",mech_baseToHit,mech_roll));
      if(target_charge)
        mech_notify(target,MECHALL,tprintf("Charge: baseToHit %d\tRoll: %d",targ_baseToHit,targ_roll));
      if(mech_roll >= mech_baseToHit && mech_charge) 
        {
          /* OUCH */
          mech_notify(target,MECHALL,
                      tprintf("CRASH!!!\n%s:[%c%c] charges into you!",
                              mech_name,
                              mech->ID[0],
                              mech->ID[1]));
          mech_notify(mech,MECHALL,"SMASH!!! You crash into your target!");
          hitGroup=FindAreaHitGroup(mech,target);
          if(hitGroup==BACK) isrear=1;
          else isrear=0;
          
          spread=target_damage/5;
          for(i=0;i < spread;i++) 
            {
              hitloc=FindHitLocation(target,hitGroup,&iscritical);
              DamageMech(target, mech, 1, mech->pilot, hitloc, isrear, iscritical, 5, 0);
            }
          if(target_damage % 5) 
            {
              hitloc=FindHitLocation(target,hitGroup,&iscritical);
              DamageMech(target, mech, 1, mech->pilot, hitloc, isrear, iscritical, (target_damage % 5), 0);
            }
          hitGroup=FindAreaHitGroup(target,mech);
          if(hitGroup==BACK) isrear=1;
          else isrear=0;
          mech_damage=(target->tons+5)/10;
          spread=mech_damage/5;
          for(i=0;i < spread;i++) 
            {
              hitloc=FindHitLocation(mech,hitGroup,&iscritical);
              DamageMech(mech, mech, 0, -1, hitloc, isrear, iscritical, 5, 0);
            }
          if(mech_damage % 5) 
            {
              hitloc=FindHitLocation(mech,hitGroup,&iscritical);
              DamageMech(mech, mech, 0, -1, hitloc, isrear, iscritical, (mech_damage % 5), 0);
            }
          mech->speed = 0;   /* stop him and let him accelerate again */
          if(!MadePilotSkillRoll(mech,2)) 
            {
              mech_notify(mech,MECHALL,
                          "Your piloting skill fails and you fall over!!");
              MechFalls(mech,1);
            }
          if(!MadePilotSkillRoll(target,2)) 
            {
              mech_notify(target,MECHALL,
                          "Your piloting skill fails and you fall over!!");
              MechFalls(target,1);
            }
        }
      if(targ_roll >= targ_baseToHit && target_charge) 
        {
          /* OUCH */
          mech_notify(mech,MECHALL,
                      tprintf("CRASH!!!\n%s:[%c%c] charges into you!",
                              target_name,
                              target->ID[0],
                              target->ID[1]));
          mech_notify(target,MECHALL,"SMASH!!! You crash into your target!");
          hitGroup=FindAreaHitGroup(target,mech);
          if(hitGroup==BACK) isrear=1;
          else isrear=0;
          
          /* Recompute because it may have been overwritten */
          mech_damage=((target->speed - mech->speed*cos((target->facing - mech->facing)*(M_PI/180.)))*MP_PER_KPH)*(target->tons+5)/10;
          spread=mech_damage/5;
          for(i=0;i < spread;i++) 
            {
              hitloc=FindHitLocation(mech,hitGroup,&iscritical);
              DamageMech(mech, target, 1, target->pilot, hitloc, isrear, iscritical, 5, 0);
            }
          if(mech_damage % 5) 
            {
              hitloc=FindHitLocation(mech,hitGroup,&iscritical);
              DamageMech(mech, target, 1, target->pilot, hitloc, isrear, iscritical, (target_damage % 5), 0);
            }
          hitGroup=FindAreaHitGroup(mech,target);
          if(hitGroup==BACK) isrear=1;
          else isrear=0;
          target_damage=(mech->tons+5)/10;
          spread=target_damage/5;
          for(i=0;i < spread;i++) 
            {
              hitloc=FindHitLocation(target,hitGroup,&iscritical);
              DamageMech(target, target, 0, -1, hitloc, isrear, iscritical, 5, 0);
            }
          if(mech_damage % 5) 
            {
              hitloc=FindHitLocation(target,hitGroup,&iscritical);
              DamageMech(target, target, 0, -1, hitloc, isrear, iscritical, (mech_damage % 5), 0);
            }
          target->speed = 0;   /* stop him and let him accelerate again */
          if(!MadePilotSkillRoll(mech,2)) 
            {
              mech_notify(mech,MECHALL,
                          "Your piloting skill fails and you fall over!!");
              MechFalls(mech,1);
            }
          if(!MadePilotSkillRoll(target,2)) 
            {
              mech_notify(target,MECHALL,
                          "Your piloting skill fails and you fall over!!");
              MechFalls(target,1);
            }
        }
      mech->sections[RLEG].recycle = PHYSICAL_RECYCLE_TIME;
      mech->sections[LLEG].recycle = PHYSICAL_RECYCLE_TIME;
      mech->sections[RARM].recycle = PHYSICAL_RECYCLE_TIME;
      mech->sections[LARM].recycle = PHYSICAL_RECYCLE_TIME;
      target->sections[RLEG].recycle = PHYSICAL_RECYCLE_TIME;
      target->sections[LLEG].recycle = PHYSICAL_RECYCLE_TIME;
      target->sections[RARM].recycle = PHYSICAL_RECYCLE_TIME;
      target->sections[LARM].recycle = PHYSICAL_RECYCLE_TIME;
      target->chgtarget = -1;    /* mech->chgtarget reset after return */
    }
}












