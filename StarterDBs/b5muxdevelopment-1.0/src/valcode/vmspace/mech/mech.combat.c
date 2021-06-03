#ifdef TEST
typedef int dbref;
#endif

#define char_getbruise(x)	1
#define char_getattribute(x,y)	4
#define char_getlethal(x)	1
#define char_getbruise(x)	1
#define char_rollskilled()	1
#define char_rollsaving()	1
#define send_mechinfo(x)	
#define send_mechdebuginfo(x)	

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "config.h"
/*#include "externs.h"
#include "db.h"*/
#include "../header.h"
#include "mech.h"
#include "playerstats.h" 
/* for accessing XP -- MW 93 Oct */

/*extern struct player_stats *get_stats P((dbref));*/ /* defined elsewhere */

void mech_target(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[5];
  int argc;
  char section[50];
  char type;

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
        {
          notify(player, "You're dreaming of lots of little sheep....zzzzz");
          return;
        }
      if(!(mech->status & STARTED)) {
        notify(player, "Target computer is not online (start the mech first!)");
        return;
      }
      argc=mech_parseattributes(buffer, args, 5);
      type=CLASS_MECH;
      if(argc==1) 
        {
          switch(args[0][0]) 
            {
            case 'h':
            case 'H':
              mech->aim=HEAD;
              break;
            case 'r':
            case 'R':
              if(args[0][1] == 'l' || args[0][1] == 'L')
                mech->aim=RLEG;
              else if(args[0][1] == 'a' || args[0][1] == 'A')
                mech->aim=RARM;
              else if(args[0][1] == 't' || args[0][1] == 'T')
                mech->aim=RTORSO;
              else if(args[0][1] == 's' || args[0][1] == 'S')
                {
		  mech->aim=RSIDE;
		  type=CLASS_VEH_GROUND;
		}
              else if(args[0][1] == 'o' || args[0][1] == 'O')
                {
		  mech->aim=ROTOR;
		  type=CLASS_VEH_VTOL;
		}
              break;
            case 'l':
            case 'L':
              if(args[0][1] == 'l' || args[0][1] == 'L')
                mech->aim=LLEG;
              else if(args[0][1] == 'a' || args[0][1] == 'A')
                mech->aim=LARM;
              else if(args[0][1] == 't' || args[0][1] == 'T')
                mech->aim=LTORSO;
              else if(args[0][1] == 's' || args[0][1] == 'S')
		{
		  mech->aim=LSIDE;
		  type=CLASS_VEH_GROUND;
		}
              break;
	    case 'f':
	    case 'F':
	      mech->aim=FSIDE;
	      type=CLASS_VEH_GROUND;
	      break;
	    case 'b':
	    case 'B':
	      mech->aim=BSIDE;
	      type=CLASS_VEH_GROUND;
	      break;
	    case 't':
	    case 'T':
	      mech->aim=TURRET;
	      type=CLASS_VEH_GROUND;
	      break;
            case 'c':
            case 'C':
              mech->aim=CTORSO;
              break;
            case '-':
              mech->aim=NUM_SECTIONS;
              notify(player, "Targetting disabled.");
              return;
            default:
              notify(player, "Unknown section.");
              return;
            }
          ArmorStringFromIndex(mech->aim, section, type);
          notify(player, tprintf("%s targetted.", section));
        }
      else
        {
          notify(player, "Invalid number of arguments to function.");
        }
    }
}

void mech_settarget(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech, *target;
  struct map_struct *mech_map;
  char *args[5];
  char targetID[2];
  int argc;
  int LOS=1;
  int newx, newy;
  dbref targetref;
  char target_name[100];

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) {
    if(mech->unconcious) 
      {
	notify(player, "You're dreaming of lots of little sheep....zzzzz");
	return;
      }
    if(!(mech->status & STARTED)) {
      notify(player, "Target computer is not online (start the mech first!)");
      return;
    }
      
    argc=mech_parseattributes(buffer, args, 5);
    if(argc==1) {
      targetID[0]=args[0][0];
      targetID[1]=args[0][1];
      targetref=FindTargetDBREFFromMapNumber(mech, targetID);
      target=(struct mech_data *) FindObjectsData(targetref);
      if(target)
	LOS=InLineOfSight(mech,target,target->x,target->y,
			  FindRange(mech->fx,mech->fy,mech->fz,
				    target->fx,target->fy,target->fz));
      if(targetref!= -1 && LOS) {
	mech_notify(mech,MECHALL, tprintf("Target set to %c%c",
			       targetID[0],targetID[1]));
        
        get_mech_atr(target, target_name, "MECHNAME");
        
	mech_notify(mech,MECHALL, 
		    tprintf("Target identified as %s", target_name));
	mech->target=targetref;
      } else {
	notify(player, "That is not a valid targetID. Try again.");
      }
    } else if(argc==2) {
      /* Targetted a square */
      mech_map=(struct map_struct *) FindObjectsData(mech->mapindex);
      if(!mech_map) {
	notify(player, "You are on an invalid map! Map index reset!");
	mech_shutdown(player,data,"");
	send_mechinfo(tprintf("settarget:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));

	mech->mapindex= -1;
	return;
      }
      newx=atoi(args[0]);
      newy=atoi(args[1]);
      if(newx<0 || newx>=mech_map->map_width || 
	 newy<0 || newy>=mech_map->map_height) {
	notify(player, "Illegal coordinates!");
      } else {
	mech->target= -1;
	mech->targx=newx;
	mech->targy=newy;
	if(mech_map->map[newy][newx].terrain==WATER)
	  mech->targz= -mech_map->map[newy][newx].elev;
	else
	  mech->targz=mech_map->map[newy][newx].elev;
	notify(player, tprintf("Target coordinates set at (X,Y) %d, %d",
			       newx, newy));
      }
    }
  }
}

void mech_fireweapon(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  struct map_struct *mech_map;
  char *args[5];
  int argc;
  int weapnum;

  mech=(struct mech_data *) data;
  mech_map=(struct map_struct *)FindObjectsData(mech->mapindex);
  if(!mech_map) {
    notify(player, "You are on an invalid map! Map index reset!");
    mech_shutdown(player,data,"");
    send_mechinfo(tprintf("fireweapon:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
    mech->mapindex= -1;
    return;
  }
  if(CheckData(player, mech)) 
    {
      if(!(mech->status & STARTED)) 
	{
	  notify(player, "Fusion reactor is not online!");
	  return;
	}
      if(mech->unconcious) 
	{
	  notify(player, "You fire your weapons in your dream...zzzzz");
	  return;
	}
      argc=mech_parseattributes(buffer, args, 5);
      if(argc>=1) 
	{
	  weapnum=atoi(args[0]);
	  FireWeaponNumber(player,mech,mech_map,weapnum,argc,args,0);
	} 
      else 
	{
	  notify(player, "Not enough arguments to the function");
	}
    }
}

int FireWeaponNumber(player,mech,mech_map,weapnum,argc,args,sight)
dbref player;
struct mech_data *mech;
struct map_struct *mech_map;
int argc;
int weapnum;
char **args;
int sight;
{
  int weaptype;
  dbref target;
  char targetID[2];
  int found_target=0;
  int mapx, mapy, loop, LOS;
  float enemyX, enemyY, enemyZ;
  struct mech_data *tempMech=NULL;
  struct mech_data *spotMech=NULL;
  int spotTerrain;
  float spotRange;
  int section, critical;
  float range;
  int indirectFire=1000;

  if(weapnum < 0) 
    {
      notify(player, "The weapons system chirps: 'Illegal Weapon Number!'");
      return(0);
    }
  
  weaptype=FindWeaponNumberOnMech(mech, weapnum, &section, &critical);

  if(weaptype== -1) {
    notify(player, "The weapons system chirps: 'Illegal Weapon Number!'");
    return(0);
  } else if (weaptype== -2) {
    notify(player, 
           "The weapons system chirps: 'That Weapon has been destroyed!'");
    return(0);
  } else if (weaptype== -3) {
    /* Reloading/Recycling */
    notify(player, 
           "The weapon system chirps: 'That weapon is still reloading!'");
    return(0);
  } else if (weaptype== -4) {
    /* Reloading/Recycling */
    notify(player, 
           "The weapon system chirps: 'That weapon is still recharging!'");
    return(0);
  } else if (weaptype == -5) {
    notify(player, "The limb holding that weapon is still retracting from an attack!");
    return(0);
  } else if (weaptype == IS_AMS || weaptype == CL_AMS) {
    notify(player, "That weapon is defensive only!");
    return(0);
  } else { /* valid weapon */
    if(argc==1) 
      {
	/* Fire at the default target */
	if(!FindTargetXY(mech, &enemyX, &enemyY, &enemyZ)) 
	  {
	    notify(player, "You do not have a default target set!");
	    return(0);
	  }
        if(!IsInWeaponArc(mech, enemyX, enemyY, section, critical))
	  {
	    notify(player, "Default target is not in your weapons arc!");
	    return(0);
	  }
	if(mech->target != -1) 
	  {
	    tempMech=(struct mech_data *) FindObjectsData(mech->target);
	    if(tempMech) 
	      {
                found_target=1;
		mapx=tempMech->x;
		mapy=tempMech->y;
		range=FindRange(mech->fx,mech->fy,mech->fz,
				tempMech->fx,tempMech->fy,tempMech->fz);
		LOS=InLineOfSight(mech,tempMech,mapx, mapy, range);
		if(!LOS)
		  {
		    notify(player, "That target is not in your line of sight!");
		    return(0);
		  }
	      }
	    else 
	      {
		send_mechinfo("Error in FireWeaponNumber routine");
		return(0);
	      }
	  } 
	else   /* default target is a hex */
	  {
	    /* look for enemies in the default hex cause they may have moved */
	    for(loop=0; loop<MAX_MECHS_PER_MAP && !found_target; loop++) 
	      {
		if(mech_map->mechsOnMap[loop] != mech->mynum 
		   && mech_map->mechsOnMap[loop] != -1) 
		  {
		    tempMech=(struct mech_data *) 
		      FindObjectsData(mech_map->mechsOnMap[loop]);
		    if(tempMech) 
		      {
			if(mech->targx==tempMech->x 
			   && mech->targy==tempMech->y)
			  {
			    enemyX=tempMech->fx;
			    enemyY=tempMech->fy;
			    enemyZ=tempMech->fz;
			    mapx=tempMech->x;
			    mapy=tempMech->y;
			    found_target=1;
			  }
		      }
		    else 
		      { 
			send_mechinfo("Error in mech_fireweapon routine!");
		      }
		  }
	      }

	    if(!found_target) 
	      {
		tempMech=(struct mech_data *) NULL;
		mapx=mech->targx;
		mapy=mech->targy;
		enemyX = mapx;
		enemyY = mapy;
		enemyZ = mech->targz;
	      } 

	    /* don't check LOS for missile weapons firing at hex number */
	    range=FindRange(mech->fx,mech->fy,mech->fz,enemyX,enemyY,enemyZ);
	    LOS=InLineOfSight(mech, tempMech, mapx, mapy, range);
	    if(!LOS && (MechWeapons[weaptype].special!=LRM))
	      {
		notify(player, "That hex target is not in your line of sight!");
		return(0);
	      }
	    else if(MechWeapons[weaptype].special==LRM) 
              {
                for(loop = 0; loop < MAX_MECHS_PER_MAP; loop++) 
                  {
                    if(mech_map->mechsOnMap[loop] != mech->mynum 
                       && mech_map->mechsOnMap[loop] != -1) 
                      {
                        spotMech=(struct mech_data *) 
                          FindObjectsData(mech_map->mechsOnMap[loop]);
                        if(spotMech) 
                          {
                            if((spotMech->status & STARTED) 
                               && !spotMech->unconcious 
                               && mech->team==spotMech->team)
                              {
                                spotRange=FindRange(spotMech->fx,
                                                    spotMech->fy,
						    spotMech->fz,
                                                    enemyX,
                                                    enemyY,
						    enemyZ);
                                
                                if(InLineOfSight(spotMech, tempMech, mapx, mapy,
                                                 spotRange))
                                   {
                                     if(tempMech)
                                       spotTerrain=1 + AddTerrainMod(spotMech, tempMech, mech_map, spotRange, sight) + AttackMovementMods(spotMech);
                                     else
                                       spotTerrain=1 + AttackMovementMods(spotMech);
                                     indirectFire=(spotTerrain < indirectFire)
                                       ? spotTerrain : indirectFire;
                                   }
                              }
                          }
                        else 
                          { 
                            send_mechinfo("Error in mech_fireweapon routine!");
                          }
                      }
                  }
                if(indirectFire==1000) 
                  {
                    notify(player, "You don't have a spotter for that hex!");
                    return(0);
                  }
              }
          }
        if(!sight)
          mech_notify(mech,MECHALL, 
                      tprintf("You fire your %s at the default target!", 
                              &MechWeapons[weaptype].name[3]));
      } 
    else if(argc==2) 
      {
	/* Fire at the numbered target */
	targetID[0]=args[1][0];
	targetID[1]=args[1][1];
	target=FindTargetDBREFFromMapNumber(mech, targetID);
	if(target != -1) 
	  {
	    tempMech=(struct mech_data *)FindObjectsData(target);
	    if(tempMech) 
	      {
		enemyX=tempMech->fx;
		enemyY=tempMech->fy;
		enemyZ=tempMech->fz;
		mapx=tempMech->x;
		mapy=tempMech->y;
	      } 
	    else 
	      { 
		send_mechinfo("Error in FireWeaponNumber routine!");
		return(0);
	      }
	  } 
	else 
	  {
	    notify(player, "That is not a valid target ID!");
	    return(0);
	  }

        if(!IsInWeaponArc(mech, enemyX, enemyY, section, critical))
	  {
	    notify(player, "That target is not in your weapons arc!");
	    return(0);
	  }

	range=FindRange(mech->fx,mech->fy,mech->fz,enemyX,enemyY,enemyZ);
	LOS=InLineOfSight(mech, tempMech,tempMech->x, tempMech->y, range); 
	if(!LOS)
	  {
	    notify(player, "That target is not in your line of sight!");
	    return(0);
	  }
	if(!sight)
          mech_notify(mech,MECHALL,
                      tprintf("You fire your %s at target %c%c", 
                              &MechWeapons[weaptype].name[3], 
                              targetID[0],targetID[1]));
      } 
    else if(argc==3) 
      {
	/* Fire at the Map X Y */ 
	mapx=atoi(args[1]);
	mapy=atoi(args[2]);
	if(mapx<0 || mapx>mech_map->map_width || 
	   mapy < 0 || mapy >mech_map->map_height) {
          notify(player, "Map coordinates out of range!");
	  return(0);
	}
	/* look for enemies in that hex... */
	for(loop=0; loop<MAX_MECHS_PER_MAP && !found_target; loop++) 
	  {
	    if(mech_map->mechsOnMap[loop] != mech->mynum 
	       && mech_map->mechsOnMap[loop] != -1) 
	      {
		tempMech=(struct mech_data *) 
		  FindObjectsData(mech_map->mechsOnMap[loop]);
		if(tempMech) 
		  {
		    if(mapx==tempMech->x && mapy==tempMech->y)
		      {
			enemyX=tempMech->fx;
			enemyY=tempMech->fy;
			enemyZ=tempMech->fz;
			found_target=1;
		      }
		  }
		else 
		  { 
		    send_mechinfo("Error in mech_fireweapon routine!");
		  }
	      }
	  }
	if(!found_target) 
	  {
	    tempMech=(struct mech_data *) NULL;
	    MapCoordToRealCoord(mapx,mapy,&enemyX,&enemyY);
	    if(mech_map->map[mapy][mapx].terrain==WATER)
	         mech->targz= -mech_map->map[mapy][mapx].elev;
	    else
	         mech->targz=mech_map->map[mapy][mapx].elev;
	  }
	
        if(!IsInWeaponArc(mech, enemyX, enemyY, section, critical))
          {
            notify(player, "That hex target is not in your weapons arc!");
            return(0);
          }
	
	/* Don't check LOS for missile weapons */
	range=FindRange(mech->fx,mech->fy,mech->fz,enemyX,enemyY,enemyZ);
	LOS=InLineOfSight(mech, tempMech, mapx, mapy, range);
	if(!LOS && (MechWeapons[weaptype].special!=LRM))
	  {
	    notify(player, "That hex target is not in your line of sight!");
	    return(0);
	  }
        else if(MechWeapons[weaptype].special==LRM)
          {
            for(loop = 0; loop < MAX_MECHS_PER_MAP; loop++) 
              {
                if(mech_map->mechsOnMap[loop] != mech->mynum 
                   && mech_map->mechsOnMap[loop] != -1) 
                  {
                    spotMech=(struct mech_data *) 
                      FindObjectsData(mech_map->mechsOnMap[loop]);
                    if(spotMech) 
                      {
                        if((spotMech->status & STARTED) 
                           && !spotMech->unconcious 
                           && mech->team==spotMech->team)
                          {
                            spotRange=FindRange(spotMech->fx,
                                                spotMech->fy,
						spotMech->fz,
                                                enemyX,
                                                enemyY,
						enemyZ);
                            
                            if(InLineOfSight(spotMech, tempMech, mapx, mapy,
                                             spotRange))
                            {
                              if(tempMech)
                                spotTerrain=1 + AddTerrainMod(spotMech, tempMech, mech_map, spotRange, sight) + AttackMovementMods(spotMech);
                              else
                                spotTerrain=1 + AttackMovementMods(spotMech);
                              indirectFire=(spotTerrain < indirectFire)
                                ? spotTerrain : indirectFire;
                            }
                          }
                      }
                    else 
                      { 
                        send_mechinfo("Error in mech_fireweapon routine!");
                      }
                  }
              }
            if(indirectFire==1000) 
              {
                notify(player, "You don't have a spotter for that hex!");
                return(0);
              }
          }
	if(!sight)
          mech_notify(mech,MECHALL,
                      tprintf("You fire your %s at hex %d %d", 
                              &MechWeapons[weaptype].name[3], mapx, mapy));
      } 
    else 
      {
        notify(player, "Too many arguments to fire function!");
	return(0);
      }
    FireWeapon(mech,mech_map,tempMech,LOS,weaptype,weapnum,section,critical,
               enemyX,enemyY,mapx,mapy,range,indirectFire,sight);
    return(1);
  }
}

void mech_dump(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  int argc;
  char *args[5];
  int weapnum;
  int weapindx;
  int section;
  int critical;
  int ammoLoc;
  int ammoCrit;
  
  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
	{
	  notify(player, "Those roses are really sweet smelling...zzzzz");
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
	  weapnum=atoi(args[0]);
	  weapindx=FindWeaponIndex(mech,weapnum);
	  if(weapindx >= 0) 
	    {
	      FindWeaponNumberOnMech(mech,weapnum,&section, &critical);
	      if(MechWeapons[weapindx].type!=TBEAM &&
		     !(mech->sections[section].criticals[critical].mode & LBX_MODE) &&
		     !(mech->sections[section].criticals[critical].mode & ARTEMIS_MODE) &&
		     !(mech->sections[section].criticals[critical].mode & NARC_MODE))
		{
		  if(FindAmmoForWeapon(mech, weapindx, 0, &ammoLoc, &ammoCrit)) 
		    {
		      
		      if(mech->sections[ammoLoc].criticals[ammoCrit].data > 0)
			{
			  mech->sections[ammoLoc].criticals[ammoCrit].data=0;
			  mech_notify(mech,MECHALL,"Ammunition dumped!");
			}
		      else 
			{
			  notify(player, 
			  "You are out of ammunition for that weapon already!");
			}
		    } 
		  else 
		    { 
		      notify(player,"You don't have any ammunition for that weapon stored on this mech!");
		    }
		}
              else if (mech->sections[section].criticals[critical].mode & ARTEMIS_MODE)
                {
                  if(FindArtemisAmmoForWeapon(mech, weapindx, 0, &ammoLoc, &ammoCrit))
                    {
                      if(mech->sections[ammoLoc].criticals[ammoCrit].data > 0)
                        {
                          mech->sections[ammoLoc].criticals[ammoCrit].data=0;
                          mech_notify(mech,MECHALL,"Ammunition dumped!");
                        }
                      else
                        {
                          notify(player,
                          "You are out of ammunition for that weapon already!");
                        }
                    }
                  else
                    {
                      notify(player,"You don't have any ammunition for that weapon stored on this mech!");
                    }
	        }
              else if (mech->sections[section].criticals[critical].mode & LBX_MODE)
                {
                  if(FindLBXAmmoForWeapon(mech, weapindx, 0, &ammoLoc, &ammoCrit))
                    {
                      if(mech->sections[ammoLoc].criticals[ammoCrit].data > 0)
                        {
                          mech->sections[ammoLoc].criticals[ammoCrit].data=0;
                          mech_notify(mech,MECHALL,"Ammunition dumped!");
                        }
                      else
                        {
                          notify(player,
                          "You are out of ammunition for that weapon already!");
                        }
                    }
                  else
                    {
                      notify(player,"You don't have any ammunition for that weapon stored on this mech!");
                    }
	        }
              else if (mech->sections[section].criticals[critical].mode & NARC_MODE)
                {
                  if(FindNarcAmmoForWeapon(mech, weapindx, 0, &ammoLoc, &ammoCrit))
                    {
                      if(mech->sections[ammoLoc].criticals[ammoCrit].data > 0)
                        {
                          mech->sections[ammoLoc].criticals[ammoCrit].data=0;
                          mech_notify(mech,MECHALL,"Ammunition dumped!");
                        }
                      else
                        {
                          notify(player,
                          "You are out of ammunition for that weapon already!");
                        }
                    }
                  else
                    {
                      notify(player,"You don't have any ammunition for that weapon stored on this mech!");
                    }
	        }
	      else 
		{
		  notify(player,"That weapon doesn't use ammunition!");
		}
	    }
	  else
	    {
	      notify(player, "Invalid weapon number!");
	    } 
	}
      else 
	{
	  notify(player, "Not enough arguments to the function");
	} 
    }
}

int IsRunning(speed,maxspeed)
float speed; 
float maxspeed; 
{
  if(speed > ((maxspeed*2)/3)) return 1;
  else return (0);
}

int AttackMovementMods(mech)
struct mech_data *mech; 
{
  int returnValue;
  float maxspeed;

  maxspeed=mech->maxspeed;
  if((mech->heat >= 9.) && (mech->specials & TRIPLE_MYOMER_TECH))
    maxspeed+=1.5*MP1;

  if(!(fabs(mech->speed)>0.0)) {
    returnValue=0;
  } else if(IsRunning(mech->speed, maxspeed)) {
    returnValue=2;
  } else {
    /* Walking */
    returnValue=1;
  } 

  if(mech->status & FALLEN || (!(mech->status & JUMPING) && mech->goingy)) 
    returnValue=2;

  if(mech->status & JUMPING)
    returnValue=3;
  else if(mech->jumpheading)
    returnValue += (mech->jumpheading+1)/2;

  return (returnValue);
}

int TargetMovementMods(target,range)
struct mech_data *target; 
float range;
{
  int returnValue=0;
  float target_speed;
 
  if(target->status & JUMPING) target_speed = target->jumpspeed;
  else target_speed = target->speed;
  
  if(target_speed <= MP2) {
    /* Mech moved 0-2 hexes */
    returnValue=0;
  } else if(target_speed <= MP4) {
    /* Mech moved 3-4 hexes */
    returnValue=1;
  } else if(target_speed <= MP6) {
    /* Mech moved 5-6 hexes */
    returnValue=2;
  } else if(target_speed <= MP9) {
    /* Mech moved 7-9 hexes */
    returnValue=3;
  } else {
    /* Moving more than 9 hexes */
    returnValue=4;
  } 

  if(!(target->status & STARTED) || target->unconcious) returnValue+= -4;
  if(target->status & FALLEN) returnValue+=(range <= 1.0) ? -2 : 1;

  if(target->status & JUMPING) {
    returnValue+=2;
  }
  return (returnValue);
}

void FireWeapon(mech,mech_map,target,LOS,weapindx,weapnum,section,
                critical,enemyX,enemyY,mapx,mapy,range,indirectFire,sight)
struct mech_data *mech; 
struct map_struct *mech_map; 
struct mech_data *target; 
int LOS;
int weapindx; 
int weapnum;
int section;
int critical;
float enemyX; 
float enemyY; 
int mapx;
int mapy;
float range;
int indirectFire;
int sight;
{
  int rangeBase;
  int baseToHit;
  int ammoLoc;
  int ammoCrit;
  int ammoLoc1;
  int ammoCrit1;
  int AMSammoLoc;
  int AMSammoCrit;
  int AMSsect;
  int AMScrit;
  int count;
  int i, loop, done=0, sum=0, index;
  int roll;
  char mech_name[100];
  int trytohit;

  /* Find and check Ammunition */
  if(MechWeapons[weapindx].type != TBEAM && 
     !sight &&
     !(mech->sections[section].criticals[critical].mode & LBX_MODE) &&
     !(mech->sections[section].criticals[critical].mode & ARTEMIS_MODE) &&
     !(mech->sections[section].criticals[critical].mode & NARC_MODE))
    { 
      if(FindAmmoForWeapon(mech, weapindx, section, &ammoLoc, &ammoCrit)) 
        {
          if(!mech->sections[ammoLoc].criticals[ammoCrit].data) 
            {
              mech_notify(mech,MECHPILOT, "You are out of ammo for that weapon!");
              return;
            }
          else if (mech->sections[section].criticals[critical].mode & ULTRA)
            {
              mech->sections[ammoLoc].criticals[ammoCrit].data--;
              FindAmmoForWeapon(mech, weapindx, section, &ammoLoc1, &ammoCrit1);
              if(!mech->sections[ammoLoc1].criticals[ammoCrit1].data) 
                {
                  mech->sections[section].criticals[critical].mode &= ~ULTRA;
                }
              mech->sections[ammoLoc].criticals[ammoCrit].data++;
            }
        } 
      else 
        {
          mech_notify(mech,MECHPILOT, 
                      "You don't have any ammo for that weapon stored on this mech!");
          return;
        }
    }
  else if(!sight &&
          (mech->sections[section].criticals[critical].mode & LBX_MODE))
    {
      if(FindLBXAmmoForWeapon(mech, weapindx, section, &ammoLoc, &ammoCrit)) 
        {
          if(!mech->sections[ammoLoc].criticals[ammoCrit].data) 
            {
              mech_notify(mech,MECHPILOT, "You are out of LBX ammo for that weapon!");
              return;
            }
        } 
      else 
        {
          mech_notify(mech,MECHPILOT, 
                      "You don't have any ammo for that weapon stored on this mech!");
          return;
        }
    }
  else if(!sight &&
          (mech->sections[section].criticals[critical].mode & ARTEMIS_MODE))
    {
      if(FindArtemisAmmoForWeapon(mech, weapindx, section, &ammoLoc, &ammoCrit)) 
        {
          if(!mech->sections[ammoLoc].criticals[ammoCrit].data) 
            {
              mech_notify(mech,MECHPILOT, "You are out of Artemis IV compatible missiles!");
              return;
            }
        } 
      else 
        {
          mech_notify(mech,MECHPILOT, 
                      "You don't have any ammo for that weapon stored on this mech!");
          return;
        }
    }
  else if(!sight &&
          (mech->sections[section].criticals[critical].mode & NARC_MODE))
    {
      if(FindNarcAmmoForWeapon(mech, weapindx, section, &ammoLoc, &ammoCrit)) 
        {
          if(!mech->sections[ammoLoc].criticals[ammoCrit].data) 
            {
              mech_notify(mech,MECHPILOT, "You are out of LBX ammo for that weapon!");
              return;
            }
        } 
      else 
        {
          mech_notify(mech,MECHPILOT, 
                      "You don't have any ammo for that weapon stored on this mech!");
          return;
        }
    }
  
  baseToHit=FindPilotGunnery(mech);
  /* add in to-hit mods from criticals */
  baseToHit+=mech->basetohit; 

  /* add in to-hit mods for section damage */
  baseToHit+=mech->sections[section].basetohit;

  /* Find the range for the weapon... */
  rangeBase=FindBaseToHitByRange(weapindx, range);
    
  /* Add in the rangebase.. */
  baseToHit+=rangeBase;
    
  /* Add in the movement modifiers */
  baseToHit+=AttackMovementMods(mech);
  baseToHit+=OverheatMods(mech);

  if(MechWeapons[weapindx].special == PULSE)
      baseToHit -= 2;
  
  if((mech->sections[section].criticals[critical].mode & ON_TC &&
      mech->aim == NUM_SECTIONS && !(mech->critstatus & TC_DESTROYED)) ||
     mech->sections[section].criticals[critical].mode & LBX_MODE)
    baseToHit -= 1;
  else if(mech->sections[section].criticals[critical].mode & ON_TC &&
          mech->aim != NUM_SECTIONS && !(mech->critstatus & TC_DESTROYED))
    baseToHit += 3;

  if(target) 
    {
      if(mech->aim==HEAD && MechWeapons[weapindx].type != TMISSILE
         && (!(target->status & STARTED) || target->unconcious))
        baseToHit += 7;  /* get rid of the -4, and add +3 */

      baseToHit+=TargetMovementMods(target,range);
      
      /* Add in the terrain modifier */
      if (indirectFire < 1000)
	baseToHit += indirectFire;
      else 
	baseToHit+=AddTerrainMod(mech, target, mech_map, range, sight);

      if(sight) 
        {
          mech_notify(mech,MECHPILOT,tprintf("Sight weapon: baseToHit %d",baseToHit));
          return;
        }
      
      if (baseToHit > 12)
	{
	  mech_notify(mech,MECHALL,
		      tprintf("Fire weapon: baseToHit %d\tYou choose not to fire",baseToHit));
	  return;
	}

      get_mech_atr(mech, mech_name, "MECHNAME");
      
      roll=Roll();
      mech_notify(mech,MECHALL, tprintf("Fire weapon: baseToHit %d\tRoll: %d",
					baseToHit,roll));
      if (roll >= baseToHit || MechWeapons[weapindx].special != STREAK) 
      {
	if(InLineOfSight(target,mech,mech->x,mech->y,range))
	  mech_notify(target,MECHALL,
		      tprintf("%s:[%c%c] has fired a %s at you!",
			      mech_name, mech->ID[0], mech->ID[1],
			      &MechWeapons[weapindx].name[3]));
	else 
	  mech_notify(target,MECHALL,
		      tprintf("Something has fired a %s at you from bearing %d!",
			      &MechWeapons[weapindx].name[3],
			      FindBearing(target->fx,target->fy,mech->fx,mech->fy)));
	      
	  if(MechWeapons[weapindx].type==TMISSILE 
	     && (target->status & AMS_ENABLED)) 
	    {
	      trytohit = 1;
	      if(target->specials & IS_ANTI_MISSILE_TECH) 
		{ 
                  if(FindWeaponFromIndex(target, IS_AMS, &AMSsect, &AMScrit))
		    {
		      if(FindAmmoForWeapon(target, IS_AMS, AMSsect, &AMSammoLoc, &AMSammoCrit)) 
		        { 
		          if(target->sections[AMSammoLoc].criticals[AMSammoCrit].data
			     && !target->sections[AMSsect].criticals[AMScrit].data)
                            {
                              AMSTarget(mech,weapindx,target,LOS,baseToHit,'I',
                                        AMSammoLoc,AMSammoCrit,roll,
          ((mech->sections[section].criticals[critical].mode & ARTEMIS_MODE) || 
          ((mech->sections[section].criticals[critical].mode & NARC_MODE) && 
           (target->status & NARC_ATTACHED))));
			      target->sections[AMSsect].criticals[AMScrit].data=WEAPON_RECYCLE_TIME;
                              target->weapheat += (float)MechWeapons[IS_AMS].heat;
			      trytohit = 0;
                            }
		        }
		    }
		  else
	 	    {
	              send_mechinfo(tprintf("AMS flags set with no weapons on mech %d",target->mynum));
		    }
		}
	      else if(target->specials & CL_ANTI_MISSILE_TECH) 
		{ 
                  if(FindWeaponFromIndex(target, CL_AMS, &AMSsect, &AMScrit))
		    {
		      if(FindAmmoForWeapon(target, CL_AMS, AMSsect, &AMSammoLoc, &AMSammoCrit)) 
		        { 
		          if(target->sections[AMSammoLoc].criticals[AMSammoCrit].data
			     && !target->sections[AMSsect].criticals[AMScrit].data)
			    {
                              AMSTarget(mech,weapindx,target,LOS,baseToHit,'C',
                                        AMSammoLoc,AMSammoCrit,roll,
          ((mech->sections[section].criticals[critical].mode & ARTEMIS_MODE) || 
          ((mech->sections[section].criticals[critical].mode & NARC_MODE) && 
           (target->status & NARC_ATTACHED))));
			      target->sections[AMSsect].criticals[AMScrit].data=WEAPON_RECYCLE_TIME;
                              target->weapheat += (float)MechWeapons[CL_AMS].heat;
			      trytohit = 0;
                            }
		        }
		    }
		  else
	 	    {
	              send_mechinfo(tprintf("AMS flags set with no weapons on mech %d",target->mynum));
		    }
		}
	      else 
		send_mechinfo(tprintf(
		    "AMS enabled with no technology on mech %d",target->mynum));

	      if(trytohit && (roll >= baseToHit))
		    HitTarget(mech, weapindx, target, LOS, 0, 0, 0,
      ((mech->sections[section].criticals[critical].mode & ARTEMIS_MODE) || 
      ((mech->sections[section].criticals[critical].mode & NARC_MODE) && 
       (target->status & NARC_ATTACHED))));
	    }
	  else if (roll == 2 &&
	        mech->sections[section].criticals[critical].mode & ULTRA_MODE)
            {
	      mech_notify(mech,MECHALL,tprintf(
			    "Your loader jams on your %s, destroying it!",
			    &(MechWeapons[weapindx].name[3])));
              DestroyWeapon(mech,section,weapindx+WEAPON_BASE_INDEX,
              					GetWeaponCrits(mech,weapindx));
            }
	  else if(roll >= baseToHit) 
            {
	      HitTarget(mech, weapindx, target, LOS,
            mech->sections[section].criticals[critical].mode & ULTRA_MODE,
            mech->sections[section].criticals[critical].mode & ON_TC, 
            mech->sections[section].criticals[critical].mode & LBX_MODE,
          ((mech->sections[section].criticals[critical].mode & ARTEMIS_MODE) || 
          ((mech->sections[section].criticals[critical].mode & NARC_MODE) && 
           (target->status & NARC_ATTACHED))));
            }
      } 
    }
  else 
    {
      if (indirectFire < 1000)
	baseToHit += indirectFire;
      
      if(sight) 
        {
          mech_notify(mech,MECHPILOT,tprintf("Sight weapon: baseToHit %d",baseToHit));
          return;
        }

      if (baseToHit > 12)
	{
	  mech_notify(mech,MECHALL, tprintf("Fire weapon: baseToHit %d\tYou choose not to fire",baseToHit));
	  return;
	}
      
      roll=Roll();
      mech_notify(mech,MECHALL, tprintf("Fire weapon: baseToHit %d\tRoll: %d",
					baseToHit,roll));
    }
  
  /* Set recycle times */
  mech->sections[section].criticals[critical].data=WEAPON_RECYCLE_TIME;

  if (MechWeapons[weapindx].special == STREAK && roll < baseToHit)
  {
       mech_notify(mech,MECHALL,"Your streak fails to lock on");
       return;
  }
  
  /* Add in weapon heat */
  mech->weapheat += (float)MechWeapons[weapindx].heat;
  if (mech->sections[section].criticals[critical].mode & ULTRA_MODE)
      mech->weapheat += (float)MechWeapons[weapindx].heat;

  /* Decrement Ammunition */
  if(MechWeapons[weapindx].type!=TBEAM) 
  {
    if(mech->sections[ammoLoc].criticals[ammoCrit].data) 
      mech->sections[ammoLoc].criticals[ammoCrit].data--;
    if (mech->sections[section].criticals[critical].mode & ULTRA_MODE)
      if(mech->sections[ammoLoc1].criticals[ammoCrit1].data) 
	  mech->sections[ammoLoc1].criticals[ammoCrit1].data--;
  }
  
  /* Let everyone know we fired */
  MechFireBroadcast(mech,target,mapx,mapy,mech_map,
		    &MechWeapons[weapindx].name[3]);
}

void HitTarget(mech,weapindx,hitMech, LOS, ultra_mode, on_tc, lbx_mode, plus2)
struct mech_data *mech; 
int weapindx; 
struct mech_data *hitMech; 
int LOS;
int ultra_mode;
int on_tc;
int lbx_mode;
int plus2;
{
  int isrear, iscritical;
  int hitloc;
  int num_missiles_hit;
  int loop;
  int roll;
  int aim_hit=0;
  int hit_roll;

  if(MechWeapons[weapindx].special == NARC) 
    {
      hitMech->status |= NARC_ATTACHED;
      mech_notify(hitMech, MECHALL, "A NARC Beacon has been attached to your 'Mech!");
      mech_notify(mech, MECHALL, "Your NARC Beacon attaches to the target!");
      return;
   } 

  if(mech->aim!=NUM_SECTIONS && 
     (!(hitMech->status & STARTED) || hitMech->unconcious))
    {
      roll=Roll();
      if(mech->aim==HEAD)
        {
          if(roll >= 8) 
            aim_hit=1;
        }
      else
        if(roll==6 || roll==7 || roll==8) 
          aim_hit=1;
    }
  
  if(MechWeapons[weapindx].type != TMISSILE && !ultra_mode && !lbx_mode) {
    if(aim_hit)
      hitloc=FindAimHitLoc(mech, hitMech, &isrear, &iscritical);
    else if(on_tc)
      hitloc=FindTCHitLoc(mech, hitMech, &isrear, &iscritical);
    else
      hitloc=FindTargetHitLoc(mech, hitMech, &isrear, &iscritical); 
    DamageMech(hitMech,mech,LOS,mech->pilot, hitloc, isrear, iscritical, 
	       MechWeapons[weapindx].damage, 0);
  } else {
    /* Missile weapon.  Multiple Hit locations... */
    for(loop=0; MissileHitTable[loop].key != -1; loop++) {
      if(MissileHitTable[loop].key == weapindx) {
        if (MechWeapons[weapindx].type == STREAK)
	    num_missiles_hit= MissileHitTable[loop].num_missiles[10];
        else
          {
            hit_roll = Roll() - 2;
            if(plus2) hit_roll=(hit_roll > 7) ? 10 : hit_roll + 2;
	    num_missiles_hit= MissileHitTable[loop].num_missiles[hit_roll];
          }
        
        if (ultra_mode || lbx_mode)
	    mech_notify(mech,MECHALL,
		 tprintf("You hit with %d projectiles!", num_missiles_hit));
        else
	    mech_notify(mech,MECHALL,
		 tprintf("You hit with %d missiles!", num_missiles_hit));

	while(num_missiles_hit) {
	  hitloc=FindTargetHitLoc(mech, hitMech, &isrear, &iscritical); 
	  if(MechWeapons[weapindx].damage==1) {
	    /* LRM (Groupings of 5)*/
	    if(num_missiles_hit>=5) {
	      DamageMech(hitMech, mech, LOS, mech->pilot, hitloc, isrear, 
                         iscritical, 5, 0);
	      num_missiles_hit-=5;
	    } else {
	      DamageMech(hitMech, mech, LOS, mech->pilot, hitloc, isrear, 
                         iscritical, num_missiles_hit, 0);
	      num_missiles_hit=0;
	    }	      
	  } else if (ultra_mode) {
            if(aim_hit)
              hitloc=FindAimHitLoc(mech, hitMech, &isrear, &iscritical);
            else if(on_tc)
              hitloc=FindTCHitLoc(mech, hitMech, &isrear, &iscritical);
            else
              hitloc=FindTargetHitLoc(mech, hitMech, &isrear, &iscritical); 
            DamageMech(hitMech,mech,LOS,mech->pilot,hitloc,isrear,iscritical, 
                       MechWeapons[weapindx].damage, 0);
	    num_missiles_hit--;
          } else if (lbx_mode) {
            DamageMech(hitMech, mech, LOS, mech->pilot, hitloc, isrear, 
                       iscritical, 1, 0);
	    num_missiles_hit--;
	  } else {
	    /* SRM (Groupings of one) */
	    DamageMech(hitMech, mech, LOS, mech->pilot, hitloc, isrear, 
                       iscritical, 2, 0);
	    num_missiles_hit--;
	  }
	}
	return; /* Quit looping */
      }
    }
  }
}

void AMSTarget(mech,weapindx,hitMech,LOS,baseToHit,type,ammoLoc,ammoCrit,roll,plus2)
struct mech_data *mech; 
int weapindx; 
struct mech_data *hitMech; 
int LOS;
int baseToHit;
char type;
int ammoLoc;
int ammoCrit;
int roll;
int plus2;
{
  int isrear, iscritical;
  int hitloc;
  int num_missiles_incoming=0;
  int num_missiles_hit=0;
  int num_missiles_shotdown=0;
  int loop;
  int index;
  int spent;
  int hit_roll;

  if(MechWeapons[weapindx].special == NARC) 
    {
      hitMech->status |= NARC_ATTACHED;
      mech_notify(hitMech, MECHALL, "A NARC Beacon has been attached to your 'Mech!");
      mech_notify(mech, MECHALL, "Your NARC Beacon attaches to the target!");
      return;
    } 

  /* Attacker is using a missile weapon and the defender is using an
     AMS system.  The AMS system intercepts before the to-hit roll */

  for(loop=0; MissileHitTable[loop].key != -1; loop++) 
    {
      if(MissileHitTable[loop].key == weapindx)
	{
	  num_missiles_incoming = MissileHitTable[loop].num_missiles[10];
	  num_missiles_shotdown = (type=='C') ? Roll() : random() % 6 + 1;
	  num_missiles_incoming -= num_missiles_shotdown;

	  /* decrement AMS ammo */
	  spent = 2*(random() % 6 + 1);
	  if(spent >= hitMech->sections[ammoLoc].criticals[ammoCrit].data)
	    hitMech->sections[ammoLoc].criticals[ammoCrit].data = 0;
	  else
	    hitMech->sections[ammoLoc].criticals[ammoCrit].data -= spent;

	  if(num_missiles_incoming > 0) 
	    {
	      mech_notify(mech,MECHALL,tprintf("The target shoots down %d of your missiles!", num_missiles_shotdown));
	      mech_notify(hitMech,MECHALL,tprintf("Your Anti-Missile System activates and shoots down %d incoming missiles!",num_missiles_shotdown));
	    }
	  else
	    {
	      mech_notify(mech,MECHALL,"All of your missiles are shot down by the target!");	
	      mech_notify(hitMech,MECHALL,tprintf("Your Anti-Missile System activates and shoots down %d incoming missiles!",num_missiles_shotdown));
	      return;
	    }
	  
	  if(roll >= baseToHit) 
	    {

	      /* Now figure the new convoluted missile table */
	      
	      if(num_missiles_incoming >= 18) 
		{
		  index = 6;
		}
	      else if(num_missiles_incoming >= 13) 
		{
		  index = 5;
		}
	      else if(num_missiles_incoming >= 9) 
		{
		  index = 4;
		}
	      else if(num_missiles_incoming >= 6)
		{
		  index = 3;
		}
	      else if(num_missiles_incoming >= 5) 
		{
		  index = 2;
		}
	      else if(num_missiles_incoming >= 4)
		{
		  index = 1;
		}
	      else 
		{
		  index = 0;
		}
	      
              hit_roll = Roll() - 2;
              if(plus2) hit_roll=(hit_roll > 7) ? 10 : hit_roll + 2;
	      num_missiles_hit = MissileHitTable[index].num_missiles[hit_roll];
	      if(num_missiles_hit > num_missiles_incoming) 
		num_missiles_hit = num_missiles_incoming;
	      
	      mech_notify(mech,MECHALL,
			  tprintf("You hit with %d missiles!", num_missiles_hit));
	      
	      while(num_missiles_hit) {
		hitloc=FindTargetHitLoc(mech, hitMech, &isrear, &iscritical); 
		if(MechWeapons[weapindx].damage==1) {
		  /* LRM (Groupings of 5)*/
		  if(num_missiles_hit>=5) {
		    DamageMech(hitMech, mech, LOS, mech->pilot, hitloc, isrear, 
			       iscritical, 5, 0);
		    num_missiles_hit-=5;
		  } else {
		    DamageMech(hitMech, mech, LOS, mech->pilot, hitloc, isrear, 
			       iscritical, num_missiles_hit, 0);
		    num_missiles_hit=0;
		  }	      
		} else {
		  /* SRM (Groupings of one) */
		  DamageMech(hitMech, mech, LOS, mech->pilot, hitloc, isrear, 
			     iscritical, 2, 0);
		  num_missiles_hit--;
		}
	      }
	      return; /* Quit looping */
	    }
	}
    }
}


int FindAreaHitGroup(mech,target)
struct mech_data *mech; 
struct mech_data *target; 
{
  int bearing;
  int returnValue;
  int quadr;
  
  bearing=FindBearing(mech->fx, mech->fy, target->fx, target->fy);
  quadr=bearing-target->facing;
  if(quadr < 0) quadr+=360;

  if((quadr<45) || (quadr>315)) {
    returnValue=BACK;
  } else if ((quadr>=45) && (quadr<135)) {
    returnValue=LEFTSIDE;
  } else if ((quadr>=135) && (quadr<=225)) {
    returnValue=FRONT;
  } else {
    returnValue=RIGHTSIDE;
  }

  return (returnValue);
}

int FindTargetHitLoc(mech,target,isrear,iscritical)
struct mech_data *mech; 
struct mech_data *target; 
int *isrear; 
int *iscritical; 
{
  int hitGroup;
  int roll;
  int rc;

  *isrear=0;
  *iscritical=0;

  hitGroup=FindAreaHitGroup(mech, target);
  
  if(hitGroup==BACK) *isrear=1;
  
  if(target->type==CLASS_MECH && (target->status & PARTIAL_COVER)) 
    {
      rc=FindPunchLocation(hitGroup);
    }
  else 
    {
      rc=FindHitLocation(target,hitGroup,iscritical);
    }
  return (rc);      
}

int FindTCHitLoc(mech,target,isrear,iscritical)
struct mech_data *mech; 
struct mech_data *target; 
int *isrear; 
int *iscritical; 
{
  int hitGroup;
  int roll;
  int rc;

  *isrear=0;
  *iscritical=0;

  hitGroup=FindAreaHitGroup(mech, target);
  if(hitGroup==BACK) *isrear=1;
  
  if(target->type==CLASS_MECH)
    switch(mech->aim) 
      {
      case RARM:
	if(hitGroup!=LEFTSIDE)
	  return(RARM);
	break;
      case LARM:
	if(hitGroup!=RIGHTSIDE)
	  return(LARM);
	break;
      case RLEG:
	if(hitGroup!=LEFTSIDE && !(target->status & PARTIAL_COVER))
	  return(RLEG);
	break;
      case LLEG:
	if(hitGroup!=RIGHTSIDE && !(target->status & PARTIAL_COVER))
	  return(LLEG);
	break;
      case RTORSO:
	if(hitGroup!=LEFTSIDE)
        return(RTORSO);
	break;
      case LTORSO:
	if(hitGroup!=RIGHTSIDE)
	  return(LTORSO);
	break;
      case CTORSO:
	return(CTORSO);
      }      
  else
    switch(mech->aim) 
      {
      case RSIDE:
	if(hitGroup!=LEFTSIDE)
	  return(RSIDE);
	break;
      case LSIDE:
	if(hitGroup!=RIGHTSIDE)
	  return(LSIDE);
	break;
      case FSIDE:
	if(hitGroup!=BACK)
	  return(FSIDE);
	break;
      case BSIDE:
	if(hitGroup!=FRONT)
	  return(BSIDE);
	break;
      case TURRET:
        return(TURRET);
	break;
      }
  
  if(target->type==CLASS_MECH && (target->status & PARTIAL_COVER)) 
    {
      rc=FindPunchLocation(hitGroup);
    }
  else 
    {
      rc=FindHitLocation(target,hitGroup,iscritical);
    }
  return (rc);      
}

int FindAimHitLoc(mech,target,isrear,iscritical)
struct mech_data *mech; 
struct mech_data *target; 
int *isrear; 
int *iscritical; 
{
  int hitGroup;
  int roll;
  int rc;

  *isrear=0;
  *iscritical=0;

  hitGroup=FindAreaHitGroup(mech, target);
  if(hitGroup==BACK) *isrear=1;

  if(target->type==CLASS_MECH)
    switch(mech->aim) 
      {
      case RARM:
	if(hitGroup!=LEFTSIDE)
	  return(RARM);
	break;
      case LARM:
	if(hitGroup!=RIGHTSIDE)
	  return(LARM);
	break;
      case RLEG:
	if(hitGroup!=LEFTSIDE && !(target->status & PARTIAL_COVER))
	  return(RLEG);
	break;
      case LLEG:
	if(hitGroup!=RIGHTSIDE && !(target->status & PARTIAL_COVER))
	  return(RLEG);
	break;
      case RTORSO:
	if(hitGroup!=LEFTSIDE)
	  return(RTORSO);
	break;
      case LTORSO:
	if(hitGroup!=RIGHTSIDE)
	  return(LTORSO);
	break;
      case CTORSO:
	return(CTORSO);
      case HEAD:
	return(HEAD);
      }      
  else
    switch(mech->aim) 
      {
      case RSIDE:
	if(hitGroup!=LEFTSIDE)
	  return(RSIDE);
	break;
      case LSIDE:
	if(hitGroup!=RIGHTSIDE)
	  return(LSIDE);
	break;
      case FSIDE:
	if(hitGroup!=BACK)
	  return(FSIDE);
	break;
      case BSIDE:
	if(hitGroup!=FRONT)
	  return(BSIDE);
	break;
      case TURRET:
        return(TURRET);
	break;
      }
  
  if(target->type==CLASS_MECH && (target->status & PARTIAL_COVER)) 
    {
      rc=FindPunchLocation(hitGroup);
    }
  else 
    {
      rc=FindHitLocation(target,hitGroup,iscritical);
    }
  return (rc);      
}

char BOOM[7][62]= {
  "#    #     #            ######  ####### ####### #     #   ###",
  "#   #     # #           #     # #     # #     # ##   ##   ###",
  "#  #     #   #          #     # #     # #     # # # # #   ###",
  "###     #     #  #####  ######  #     # #     # #  #  #    #",
  "#  #    #######         #     # #     # #     # #     #",
  "#   #   #     #         #     # #     # #     # #     #   ###",
  "#    #  #     #         ######  ####### ####### #     #   ###"
  };

void KillMechContentsIfIC(dbref aRef)
{
int i;
i=0;
  DOLIST(aRef,aRef)
    {
      send_mechdebuginfo(
	 tprintf("Determining whether to kill player #%d (%s).",
		 aRef, Name(aRef))
			 );
      
      /*if (db[aRef].flags & IN_CHARACTER)*/
	if(1==1)
	{
	  /* Yup, player is in character. @tel to the Afterlife. */
	  send_mechdebuginfo("Sending downed pilot to the Afterlife.");
	  /*do_teleport(GOD, tprintf("#%d",aRef),"#666");*/
	notify(aRef,"You feel your bones breaking!");
	/*move_via_generic(aRef,666,NOTHING,0);*/
	}
      else
send_mechinfo(tprintf("Player #%d (%s) out of character but in IC mech!",
		      aRef,Name(aRef)));
	i++;
	if(i>5) return;
    }
}

void DestroyMech(target,mech)
struct mech_data *target; 
struct mech_data *mech; 
{
  int loop;
  struct map_struct *mech_map;
  char target_name[100];
  char mech_name[100];

  if(!(target->status & DESTROYED)) 
    {
      get_mech_atr(mech, mech_name, "MECHNAME");
      get_mech_atr(target, target_name, "MECHNAME");
      
      if(mech!=target)
         mech_notify(mech,MECHALL, "You destroyed the target!");
      for(loop=0;loop<7;loop++) 
        mech_notify(target, MECHALL, BOOM[loop]);
      mech_notify(target, MECHALL, "You have been destroyed!");
      mech_map=(struct map_struct *) FindObjectsData(target->mapindex);
      if(mech_map) MechBroadcast(mech,target,mech_map,
		      tprintf("%s:[%c%c] has been destroyed by %s:[%c%c]!!",
			      target_name,
			      target->ID[0],target->ID[1],
			      mech_name,
			      mech->ID[0],mech->ID[1]));

      /*if (db[target->mynum].flags & IN_CHARACTER)*/
	if(1==1)
	{
	  /* Kill all IN_CHARACTER players in this mech
	     by sending them to the Afterlife (@tel <person>=#666).
	     -- MW 93 Oct */
	if(!Flags2(target->mynum)&PREREG)
	  KillMechContentsIfIC(db[target->mynum].contents);
       }
      
      /* shut it down */
      target->status = DESTROYED;
      target->speed = 0.0;
      target->desired_speed = 0.0;
      target->verticalspeed = 0.0;
      target->pilot = -1;
      if(target->terrain==WATER) target->z= -target->elev;
      else target->z=target->elev;
      target->fz=ZSCALE*target->z;
    }
}

#define DXP_NORMAL_DAMAGE       0
#define DXP_CRITICAL_HIT        1
void AccumulateXP(dbref pilot,
                  struct mech_data *attacker,
                  int numOccurrences,
                  int reason)
{
    float               xpMultiplier, mechMultiplier, numXP;
    int                 intXP;
    char                reasonText[32];
    struct player_stats *s;

    /* Only do this if we have a pilot to accumulate XP on */
    if (pilot == -1) return;

    /* Only accum XP if both mech and pilot are in a roleplay state */
/*    send_mechdebuginfo(
	 tprintf("Testing accumXP for %d %s from %s in %s.",
		 numOccurrences, 
		 (reason == DXP_CRITICAL_HIT ? "critical(s)":"pts damage"),
		 db[pilot].name, 
		 db[attacker->mynum].name)); */

    /*if ((db[pilot].flags & db[attacker->mynum].flags & IN_CHARACTER) != 0)*/
    if (1==1)
    {
        /* Pilot and mech are in character. */
        /*s = get_stats(pilot);*/
	s=0;
        if (s) /* Can we accum XP? */
        {
    
            switch(reason)
            {
                case DXP_CRITICAL_HIT:
                    xpMultiplier = 5.0;
                    strcpy(reasonText,"critical hit");
                    break;
                case DXP_NORMAL_DAMAGE:
                    xpMultiplier = 0.5;
                    strcpy(reasonText,"damage inflicted");
                    break;
                default:
                    xpMultiplier = 1.0;
                    strcpy(reasonText,"some action done");
                    break;
            }
            if (attacker->tons > 0)
                mechMultiplier = (50.0 / (float) attacker->tons);
            else
            {
                /* Punt if we have a weird tonnage,
                   use mech multiplier of 0.5 (100 ton mech) */
                mechMultiplier = 0.5;
                
                /* Bring this to the attention of the admins */
                send_mechinfo(tprintf(
                    "AccumulateXP: Weird tonnage for IC mech %d (%s): %d",
                            attacker->mynum, Name(attacker->mynum),
                            (short) attacker->tons)
                              );
            }
            numXP = mechMultiplier * xpMultiplier * (float) (numOccurrences);
            intXP = (int) numXP;
            /*s->xp += intXP;
	    s->xpAvail += intXP;*/
            send_mechdebuginfo(
                 tprintf("Added %d XP for %s by %s.",
                         intXP, reasonText, Name(pilot))
                               );
        }
        else
            send_mechinfo(
               tprintf("accumXP: Unregistered player in IC mech: `%s' in #%d (%s).",
                       Name(pilot), attacker->mynum,
                       Name(attacker->mynum))
                          );
    }
}

void DamageMech(wounded,attacker,LOS,attackPilot,hitloc,isrear,
                       iscritical,damage,intDamage)
struct mech_data *wounded;
struct mech_data *attacker;
int LOS;
int attackPilot; 
int hitloc; 
int isrear; 
int iscritical;
int damage;
int intDamage; 
{
    char  locationBuff[20];
    char  notificationBuff[80];
    char  rearMessage[10];
    float numXP;
    struct player_stats *s;
    
    /*
    if(wounded->status & DESTROYED) 
    {
        if(attackPilot!= -1)
            mech_notify(attacker,MECHALL,
                        "You blow the target into still smaller pieces of metal!");
        return;
    }
    */
    
    if(isrear && wounded->type==CLASS_MECH)
    {
        strcpy(rearMessage, "(Rear)");
    }
    else
    {
        *rearMessage='\0';
    }

    if(isrear && wounded->type!=CLASS_MECH)
      {
	if(hitloc==FSIDE) hitloc==BSIDE;
	isrear=0;
      }
    
    /* XP for hits = (50/(attacker tonnage)) * 0.5 * (pts of damage).
       -- MW 93 Oct */
    if (attackPilot != -1 && !(wounded->status & DESTROYED))
    {
        AccumulateXP(attackPilot,attacker,
                     (damage+intDamage),DXP_NORMAL_DAMAGE);
    }
    
    ArmorStringFromIndex(hitloc, locationBuff, wounded->type);
    if((intDamage>0) && (damage==0))
    {
        sprintf(notificationBuff, "for %d points of damage in the %s %s",
                intDamage, locationBuff, rearMessage);
    }
    else
    {
        sprintf(notificationBuff, "for %d points of damage in the %s %s",
                damage, locationBuff, rearMessage);
    }
    
    if(LOS && attackPilot!= -1)
    {
        mech_notify(attacker,MECHALL, tprintf("You hit %s", notificationBuff));
    }
    
    mech_notify(wounded, MECHALL, tprintf("You have been hit %s",
                                          notificationBuff));
    if(hitloc==HEAD && wounded->type==CLASS_MECH)
    {
        mech_notify(wounded,MECHALL, 
				"You take 10 points of Lethal dammage!!");
        headhitmwdamage(wounded);
    }
    
    wounded->turndamage+=damage+intDamage;
    
    do
    {
        if(damage > 0)   /* Otherwise it's all internal damage from ammo */
        {
            /* Now decrement armor, and if neccessary, handle criticals... */
            if(isrear && (hitloc==CTORSO || hitloc==RTORSO || hitloc==LTORSO)) {
                intDamage = damage - wounded->sections[hitloc].rear;
                wounded->sections[hitloc].rear-=damage;
                if(wounded->sections[hitloc].rear <0)
                {
                    wounded->sections[hitloc].rear=0;
                }
            }
            else
            {
                intDamage = damage - wounded->sections[hitloc].armor;
                wounded->sections[hitloc].armor-=damage;
                if(wounded->sections[hitloc].armor < 0)
                {
                    wounded->sections[hitloc].armor=0;
                }
            }
            if(iscritical)
            {
                switch (Roll())
                {
                    case 8:
                    case 9:
                    HandleCritical(wounded,attacker,LOS,hitloc,1);
                    break;
                    case 10:
                    case 11:
                    HandleCritical(wounded,attacker,LOS,hitloc,2);
                    break;
                    case 12:
                    HandleCritical(wounded,attacker,LOS,hitloc,3);
                    break;
                    default:
                    break;
                }
                iscritical = 0;
            }
        }
        
        if(intDamage > 0)
        {
            /* Some of the damage got through to the internal structure */
            if(wounded->sections[hitloc].internal)
            {
                /* Critical hits? */
                switch (Roll())
                {
                    case 8:
                    case 9:
                    HandleCritical(wounded, attacker, LOS, hitloc, 1);
                    break;
                    case 10:
                    case 11:
                    HandleCritical(wounded, attacker, LOS, hitloc, 2);
                    break;
                    case 12:
                    switch(hitloc)
                    {
                        case RARM:
                        case LARM:
                        case RLEG:
                        case LLEG:
                        /* Limb blown off */
                        DestroySection(wounded, attacker, LOS, hitloc);
                        if(damage > 0) intDamage = 0;
                        break;
                        default:
                        /* Ouch */
                        HandleCritical(wounded, attacker, LOS, hitloc, 3);
                        break;
                    }
                    default:
                    break;
                    /* No critical hit */
                }
                if(wounded->sections[hitloc].internal <= intDamage)
                {
                    intDamage -= wounded->sections[hitloc].internal;
                    DestroySection(wounded, attacker, LOS, hitloc);
                    if(hitloc==CTORSO || hitloc==HEAD 
		       || wounded->type!=CLASS_MECH)
                    {
                        DestroyMech(wounded, attacker); 
			intDamage = 0;
                    }
                }
                else
                {
                    wounded->sections[hitloc].internal -= intDamage;
                    intDamage=0;
                }
            }
            else
            {
                /* All internal armor gone, so... */
                if(damage == 0 && 
		   (wounded->sections[hitloc].config 
		    & CASE_TECH || wounded->specials & CLAN_TECH))
                {
                    /* Handle ammo explosions and case */
                    /* Now decrement armor, and if neccessary blow out rear armor */
                    if(hitloc==CTORSO || hitloc==RTORSO || hitloc==LTORSO)
                    {
                        wounded->sections[hitloc].rear -= intDamage;
                        if(wounded->sections[hitloc].rear < 0)
                        {
                            wounded->sections[hitloc].rear=0;
                            /* blown out back means all damage is done */
                            intDamage=0;
                        }
                    }
                    else
                    {
                        /* Apply to the outer armor, no transfers */
                        wounded->sections[hitloc].armor -= intDamage;
                        if(wounded->sections[hitloc].armor < 0)
                        {
                            wounded->sections[hitloc].armor=0;
                            intDamage=0;
                        }
                    }
                }
                else
                {
		  /* This area has been completely destroyed.
		     go to next area.. */
		  switch(hitloc)
		    {
		    case RARM:
		    case RLEG:
		      hitloc=RTORSO;
		      break;
		    case LARM:
		    case LLEG:
		      hitloc=LTORSO;
		      break;
		      
		    case RTORSO:
		    case LTORSO:
		      hitloc=CTORSO;
		      break;
		      
		    case HEAD:
		    case CTORSO:
		      /* The mech has been destroyed */
		      intDamage=0;
		      DestroyMech(wounded, attacker);
		      break;
		    }
                }
            }
            if(damage > 0) damage=intDamage;  /* 0 damage == all internal */
        }
    }
    while (intDamage > 0); /* As long as we haven't done all the */
}                            /* damage... */

/* this takes care of setting all the criticals to CRIT_DESTROYED */
void DestroyWeapon(wounded,hitloc,type,numcrits)
struct mech_data *wounded;
int hitloc;
unsigned char type;
char numcrits;
{
    int i;
    char sum=0;
    
    for(i=0; i<NUM_CRITICALS; i++) 
    {
      if(wounded->sections[hitloc].criticals[i].type==type
	 && wounded->sections[hitloc].criticals[i].data!=CRIT_DESTROYED) 
	{
	  wounded->sections[hitloc].criticals[i].data= CRIT_DESTROYED;
	  sum++;
	  if(sum==numcrits) return;
	}
    }
}

void HandleCritical(wounded,attacker,LOS,hitloc,num)
struct mech_data *wounded, *attacker; 
int LOS;
int hitloc; 
int num; 
{
  int i;
  int critHit;
  unsigned char critType;
  unsigned char critData;
  int damage, count, index;
  int loop, weapindx;
  int critList[NUM_CRITICALS];
  int destroycrit;
  int weapon_slot;
  
  if(wounded->type!=CLASS_MECH && wounded->type!=CLASS_VEH_VTOL)
    {
      for(i=0;i<num;i++)
	{
	  mech_notify(wounded,MECHALL,"CRITICAL HIT!!");
	  switch(random() % 6)
	    {
	    case 0:
	      /* Crew stunned for one turn...treat like a head hit */
	      mech_notify(wounded,MECHALL, 
			  "You take 10 points of Lethal dammage!!");
	      headhitmwdamage(wounded);
	      break;
	    case 1:
	      /* Weapon jams, set them recylcling maybe */
	      break;
	    case 2:
	      /* Engine Hit */
	      mech_notify(wounded,MECHALL,"Your engine takes a direct hit!!  You can't move anymore.");
	      wounded->maxspeed=0.0;
	      wounded->speed=0.0;
	      break;
	    case 3:
	      /* Crew Killed */
	      mech_notify(wounded,MECHALL,"Your armor is pierced and you are killed instantly!!");
	      DestroyMech(wounded,attacker);
	      break;
	    case 4:
	      /* Fuel Tank Explodes */
	      mech_notify(wounded,MECHALL,"Your fuel tank explodes in a ball of fire!!");
	      if(wounded!=attacker)
	        mech_notify(attacker,MECHALL,"The target explodes in a ball of fire!");
	      DestroyMech(wounded,attacker);
	      break;
	    case 5:
	      /* Ammo/Power Plant Explodes */
	      mech_notify(wounded,MECHALL,"Your power plant explodes!!");
	      if(wounded!=attacker)
	        mech_notify(attacker,MECHALL,"The target suddenly explodes!");
	      DestroyMech(wounded,attacker);
	      break;
	    }
	}
      return;
    }
  else if(wounded->type==CLASS_VEH_VTOL)
    {
      for(i=0;i<num;i++)
	{
	  mech_notify(wounded,MECHALL,"CRITICAL HIT!!");
	  switch(random() % 6)
	    {
	    case 0:
	      /* Crew killed */
	      mech_notify(wounded,MECHALL, 
			  "Your cockpit is destroyed!!");
	      if(!(wounded->status & LANDED))
		{
		  mech_notify(attacker,MECHALL,"You knock the VTOL out of the sky!");
		  if(wounded->terrain==WATER)
		    MechFalls(wounded, wounded->z+wounded->elev);
		  else
		    MechFalls(wounded, wounded->z-wounded->elev);
		}

	      if(wounded->terrain==WATER)
		wounded->z = -wounded->elev;
	      else
		wounded->z = wounded->elev;
	      wounded->fz = ZSCALE*wounded->z;

	      wounded->speed=0.0;
	      wounded->verticalspeed=0.0;
	      wounded->status |= DESTROYED;
	      wounded->status &= ~STARTED;
	      wounded->pilot = -1;
	      break;
	    case 1:
	      /* Weapon jams, set them recylcling maybe */
	      break;
	    case 2:
	      /* Engine Hit */
	      mech_notify(wounded,MECHALL,"Your engine takes a direct hit!!");
	      if(!(wounded->status & LANDED))
		{
		  if(wounded->terrain==GRASSLAND || wounded->terrain==ROAD || 
		     wounded->terrain==BUILDING)
		    {
		      if(MadePilotSkillRoll(wounded,wounded->z-wounded->elev)) 
			{
			  mech_notify(wounded,MECHPILOT, "You land safely!");
			  wounded->status |= LANDED;
			  wounded->z = wounded->elev;
			  wounded->fz = ZSCALE*wounded->z;
			  wounded->maxspeed=0.0;
			  wounded->speed=0.0;
			  wounded->verticalspeed=0.0;
			}
		    }
		  else
		    {
		      mech_notify(wounded,MECHALL,"You crash to the ground!");
		      mech_notify(attacker,MECHALL,"You knock the VTOL out of the sky!");
		      if(wounded->terrain==WATER)
			MechFalls(wounded, wounded->z+wounded->elev);
		      else
			MechFalls(wounded, wounded->z-wounded->elev);
		    }
		}
	      wounded->maxspeed=0.0;
	      break;
	    case 3:
	      /* Crew Killed */
	      mech_notify(wounded,MECHALL, 
			  "Your cockpit is destroyed!!");
	      if(!(wounded->status & LANDED))
		{
		  mech_notify(attacker,MECHALL,"You knock the VTOL out of the sky!");
		  if(wounded->terrain==WATER)
		    MechFalls(wounded, wounded->z+wounded->elev);
		  else
		    MechFalls(wounded, wounded->z-wounded->elev);
		}

	      if(wounded->terrain==WATER)
		wounded->z = -wounded->elev;
	      else
		wounded->z = wounded->elev;
	      wounded->fz = ZSCALE*wounded->z;

	      wounded->speed=0.0;
	      wounded->verticalspeed=0.0;
	      wounded->status |= DESTROYED;
	      wounded->status &= ~STARTED;
	      wounded->pilot = -1;
	      break;
	    case 4:
	      /* Fuel Tank Explodes */
	      mech_notify(wounded,MECHALL,"Your fuel tank explodes in a ball of fire!!");
	      if(wounded!=attacker)
	        mech_notify(attacker,MECHALL,"The target explodes in a ball of fire!");
	      if(wounded->terrain==WATER)
		wounded->z = -wounded->elev;
	      else
		wounded->z = wounded->elev;
	      wounded->fz = ZSCALE*wounded->z;

	      wounded->speed=0.0;
	      wounded->verticalspeed=0.0;
	      DestroyMech(wounded,attacker);
	      break;
	    case 5:
	      /* Ammo/Power Plant Explodes */
	      mech_notify(wounded,MECHALL,"Your power plant explodes!!");
	      if(wounded!=attacker)
	        mech_notify(attacker,MECHALL,"The target suddenly explodes!");
	      if(wounded->terrain==WATER)
		wounded->z = -wounded->elev;
	      else
		wounded->z = wounded->elev;
	      wounded->fz = ZSCALE*wounded->z;

	      wounded->speed=0.0;
	      wounded->verticalspeed=0.0;
	      DestroyMech(wounded,attacker);
	    }
	}
      return;
    }      
      
  while(num > 0)
    {
      count=0;
      while(count==0)
	{
	  for(i=0;i<NUM_CRITICALS;i++) 
	    {
	      critType=wounded->sections[hitloc].criticals[i].type;
	      if(wounded->sections[hitloc].criticals[i].data!=CRIT_DESTROYED
		 && critType!=EMPTY
		 && critType!=CASE+SPECIAL_BASE_INDEX  
		 && critType!=FERRO_FIBROUS+SPECIAL_BASE_INDEX   
		 && critType!=ENDO_STEEL+SPECIAL_BASE_INDEX
		 && critType!=TRIPLE_STRENGTH_MYOMER+SPECIAL_BASE_INDEX
		 && critType!=MASC+SPECIAL_BASE_INDEX) 
		{
		  critList[count]=i;
		  count++;
		}
	    }
	  
	  if(count==0)  /* transfer Crit to next location */
	    switch(hitloc) {
	    case RARM:
	    case RLEG:
	      hitloc=RTORSO;
	      break;
	      
	    case LARM:
	    case LLEG:
	      hitloc=LTORSO;
	      break;
	      
	    case RTORSO:
	    case LTORSO:
	      hitloc=CTORSO;
	      break;
	      
	    case HEAD:
	    case CTORSO:
	      return;  /* all crits gone, mech must be dead already */
	      break;
	    }
	} 
      
      index=random() % count;
      critHit=critList[index]; /* This one should be linear */
      
      critType=wounded->sections[hitloc].criticals[critHit].type;
      critData=wounded->sections[hitloc].criticals[critHit].data;
      
      mech_notify(wounded,MECHALL,"CRITICAL HIT!!!");
    
      /* XP for hits = (50/(attacker tonnage)) * 0.5 * (pts of damage).
         -- MW 93 Oct */
      if(wounded!=attacker)
      {
          AccumulateXP(attacker->pilot,attacker,1,DXP_CRITICAL_HIT);
      }

      if(critType >= WEAPON_BASE_INDEX && critType < AMMO_BASE_INDEX) 
        {
	  mech_notify(wounded,MECHALL, 
		   tprintf("Your %s has been destroyed!!",
			   &MechWeapons[critType-WEAPON_BASE_INDEX].name[3]));
          if(LOS && wounded!=attacker)
            mech_notify(attacker,MECHALL, tprintf("You destroy a %s!!",
		      &MechWeapons[critType-WEAPON_BASE_INDEX].name[3]));
          /* Have to destroy all the weapons of this type in this section */
          DestroyWeapon(wounded,hitloc,critType,
                        GetWeaponCrits(wounded,critType-WEAPON_BASE_INDEX));
	  if (MechWeapons[critType-WEAPON_BASE_INDEX].special == GAUSS)
	  {
	      mech_notify(wounded,MECHALL,"It explodes for 20 points damage");
	      if(LOS && wounded!=attacker)
		mech_notify(attacker,MECHALL,
		  "Your target is covered with a large electrical discharge");
	      DamageMech(wounded, attacker, 0, -1, hitloc, 0, 0, 0, 20);
	  }
          num--;
	}
      else if(critType >= AMMO_BASE_INDEX && critType < RESERVED_BASE_INDEX) 
        {
          /* BOOM! */
          /* That's going to hurt... */
	  weapindx=critType-AMMO_BASE_INDEX;
          damage=critData*MechWeapons[weapindx].damage;
	  if(MechWeapons[weapindx].type==TMISSILE)
	    {
	      for(loop=0; MissileHitTable[loop].key != -1; loop++) 
		if(MissileHitTable[loop].key == weapindx) 
		  damage *= MissileHitTable[loop].num_missiles[10];
	    }
	  if (MechWeapons[weapindx].special == GAUSS)
	  {
	      mech_notify(wounded,MECHALL,
			    "One of your Gauss Rifle Ammo feeds is destroyed");
	      wounded->sections[hitloc].criticals[critHit].data= CRIT_DESTROYED;
	  }
	  else if(damage) 
	    {
              mech_notify(wounded,MECHALL, "Ammunition explosion!");
	      if(LOS && wounded!=attacker)
                mech_notify(attacker,MECHALL,
		  "Your target has an ammunition explosion!  BOOM!!");
	      wounded->sections[hitloc].criticals[critHit].data= CRIT_DESTROYED;
	      DamageMech(wounded, attacker, 0, -1, hitloc, 0, 0, 0, damage);
	      wounded->pilotstatus+=2;
              mech_notify(wounded,MECHPILOT, 
    "You take 2 points of personal injury from the ammunition explosion!");
	      UpdatePilotStatus(wounded);
	    }
	  else  
	    {
	      mech_notify(wounded,MECHPILOT, 
		    "You have no ammunition left in that location, lucky you!");
	    }
          num--;
        } 
      else if(critType >= SPECIAL_BASE_INDEX) 
        {
          destroycrit = 1;
          switch (critType-SPECIAL_BASE_INDEX) 
            {
            case LIFE_SUPPORT:
              wounded->critstatus |= LIFE_SUPPORT_DESTROYED;
	      mech_notify(wounded,MECHALL, 
				"Your life support has been destroyed!!");
              break;
            case COCKPIT:
              /* Destroy Mech for now, but later kill pilot as well */
              wounded->status = DESTROYED;
              wounded->speed=0;
	      wounded->desired_speed=0;
	      mech_notify(wounded,MECHALL, 
			"Your cockpit is destroyed!!  Your body is fried!!!");
              if(LOS) 
                 mech_notify(attacker,MECHALL, 
"You destroy the cockpit!!  The pilot's blood splatters down the sides!!!");
              wounded->pilot = -1;
              break;
            case SENSORS:
              if(!(wounded->critstatus & SENSORS_DAMAGED)) 
                {
                  wounded->lrs_range /= 2;
                  wounded->tac_range /= 2;
                  wounded->scan_range /= 2;
                  wounded->basetohit+=2;
                  wounded->critstatus |= SENSORS_DAMAGED;
		  mech_notify(wounded,MECHALL,
				"Your sensors have been damaged!!");
                }
              else 
                {
                  wounded->lrs_range=0;
                  wounded->tac_range=0;
                  wounded->scan_range=0;
                  wounded->basetohit=75;
 	          mech_notify(wounded,MECHALL, 
					"Your sensors have been destroyed!!");
                }
              break;
            case HEAT_SINK:
              if (wounded->specials & DOUBLE_HEAT_TECH)
              {
                 wounded->numsinks -= 2;
                 DestroyWeapon(wounded,hitloc,critType,(char)3);
		 destroycrit = 0;
              }
              else if (wounded->specials & CLAN_TECH)
              {
                 wounded->numsinks -= 2;
                 DestroyWeapon(wounded,hitloc,critType,(char)2);
		 destroycrit = 0;
	      }
              else
                wounded->numsinks--;
	      mech_notify(wounded,MECHALL, "You lost a heat sink!");
              break;
            case JUMP_JET:
              wounded->jumpspeed-=MP1;
	      if(wounded->jumpspeed < 0) 
 		  wounded->jumpspeed=0;
	      mech_notify(wounded,MECHALL, 
				"One of your jump jet engines has shut down!");
              if (wounded->jumpspeed == 0 && wounded->status & JUMPING)
                {
		  mech_notify(wounded,MECHALL,
		       "Losing your last Jump Jet you fall from the sky!!!!!");
		  if (LOS && wounded != attacker)
		    mech_notify(attacker,MECHALL,
			   "Your hit knocks the 'Mech from the sky");
		  MechFalls(wounded, 1); 
		  wounded->status &= ~JUMPING;
		  wounded->jumpheading = JUMP_TO_HIT_RECYCLE;
		  wounded->goingx=wounded->goingy= 0;
		} 
              break;
            case ENGINE:
	      if(wounded->engineheat < 10) 
		{
		  wounded->engineheat+=5;
		  mech_notify(wounded,MECHALL, 
"Your engine shielding takes a hit!  It's getting hotter in here!!");
		}
	      else 
		{
		  mech_notify(wounded,MECHALL,"Your engine is destroyed!!");
		  if(wounded!=attacker)
		    mech_notify(attacker,MECHALL, "You destroy the engine!!");
		  wounded->status=DESTROYED;
		  wounded->speed=0;
		  wounded->desired_speed=0;
		  wounded->pilot= -1;
		}
              break;
            case TARGETING_COMPUTER:
		if (!wounded->critstatus & TC_DESTROYED)
		{
		  mech_notify(wounded,MECHALL,
		  			"Your Targeting Computer is Destroyed");  
		  wounded->critstatus |= TC_DESTROYED;
		}
            case GYRO:
              if(!(wounded->critstatus & GYRO_DAMAGED)) 
                {
                  wounded->critstatus |= GYRO_DAMAGED;
                  wounded->pilotskillbase+=3;
		  mech_notify(wounded,MECHALL, "Your Gyro has been damaged!");
		  if(!MadePilotSkillRoll(wounded,0)) 
		    {
		      if (!(wounded->status & FALLEN) && 
			  !(wounded->status & JUMPING))
			{
			  mech_notify(wounded,MECHALL,
				   "You lose your balance and fall down!");
			  MechFalls(wounded,1);
			}
		      else if (!(wounded->status & FALLEN) && 
			       (wounded->status & JUMPING))
			{
			  mech_notify(wounded,MECHALL,
				   "You fall from the sky!!!!!");
			  if (LOS && wounded != attacker)
			    mech_notify(attacker,MECHALL,
				   "Your hit knocks the 'Mech from the sky");
			  MechFalls(wounded,
				    (int)(wounded->jumpspeed*MP_PER_KPH)); 
			  wounded->status &= ~JUMPING;
			  wounded->jumpheading = JUMP_TO_HIT_RECYCLE;
			  wounded->goingx=wounded->goingy= 0;
			}
		    }
		}
              else if(!(wounded->critstatus & GYRO_DESTROYED))
                {
                  wounded->critstatus |= GYRO_DESTROYED;
		  mech_notify(wounded,MECHALL, 
					"Your Gyro has been destroyed!!");
                  if(!(wounded->status & FALLEN) 
		     && !(wounded->status & JUMPING)) 
                    {
		      mech_notify(wounded,MECHALL, 
					"You fall and you can't get up!!");
                      if(LOS && wounded!=attacker) 
			mech_notify(attacker,MECHALL, 
					"Your hit knocks the 'Mech over!");
                      wounded->maxspeed=0;
                      MechFalls(wounded,1);
                    }
		  else if(!(wounded->status & FALLEN) 
			  && (wounded->status & JUMPING)) 
                    {
		      mech_notify(wounded,MECHALL, 
					"You fall from the sky!!!!!");
		      if (LOS && wounded != attacker)
			 mech_notify(attacker,MECHALL,
				    "Your hit knocks the 'Mech from the sky");
		      MechFalls(wounded,(int)(wounded->jumpspeed*MP_PER_KPH)); 
		      wounded->status &= ~JUMPING;
		      wounded->jumpheading = JUMP_TO_HIT_RECYCLE;
		      wounded->goingx=wounded->goingy= 0;
		    }
                }
              else 
                {
		    mech_notify(wounded,MECHALL, 
				"Your destroyed gryo takes another hit!");
                }
              break;
            case SHOULDER_OR_HIP:
	      if(hitloc==LARM) wounded->critstatus |= LSHDR_DESTROYED;
	      if(hitloc==RARM) wounded->critstatus |= RSHDR_DESTROYED;
              if(hitloc==LARM || hitloc==RARM) 
                {
                  wounded->sections[hitloc].basetohit=4;
		  mech_notify(wounded,MECHALL, 
			"Your shoulder joint takes a hit and is frozen!");
                } 
              else if(hitloc==RLEG || hitloc==LLEG) 
                {
                  if(!(wounded->critstatus & HIP_DAMAGED)) 
                    {
                      wounded->critstatus |= HIP_DAMAGED;
                      wounded->maxspeed /= 2;
                      wounded->desired_speed /= 2;
                      wounded->speed /= 2;
                      wounded->pilotskillbase+=2;
		      mech_notify(wounded,MECHALL,
				"Your hip takes a direct hit and freezes up!!");
                    }
                  else 
                    { 
		      wounded->critstatus |= BOTH_HIPS_DAMAGED;
                      wounded->maxspeed=0;
                      wounded->desired_speed=0;
                      wounded->speed=0;
                      wounded->pilotskillbase+=2;
		      mech_notify(wounded,MECHALL, 
	"Your other hip takes a direct hit!! You are immobile!!");
                    }
                }
              break;
            case LOWER_ACTUATOR:
	      if (hitloc == LARM && 
		  !(wounded->critstatus & LLOWACT_DESTROYED))
		{
		  wounded->critstatus |= LLOWACT_DESTROYED;
		  wounded->sections[hitloc].basetohit+=1;
		  mech_notify(wounded,MECHALL, 
				"Your lower left arm actuator is destroyed!!");
		}
	      else if(hitloc == RARM && 
		      !(wounded->critstatus & LLOWACT_DESTROYED))
		{
		  wounded->critstatus |= RLOWACT_DESTROYED;
		  wounded->sections[hitloc].basetohit+=1;
		  mech_notify(wounded,MECHALL, 
				"Your lower right arm actuator is destroyed!!");
		}
              else if(hitloc == LLEG || hitloc == RLEG)
                {
                  if(wounded->sections[hitloc].basetohit<3) 
                    {
                      wounded->sections[hitloc].basetohit+=1;
                      wounded->pilotskillbase+=1;
                      wounded->maxspeed -= MP1;
		      if(wounded->maxspeed < 0.0) wounded->maxspeed=0.0;
		      wounded->speed = 0.0;
		      mech_notify(wounded,MECHALL,
				 "One of your leg actuators is destroyed!!");
		      if(!(wounded->status & JUMPING) &&
			 !MadePilotSkillRoll(wounded,0)) 
			{
			  mech_notify(wounded,MECHALL,
				    "You lose your balance and fall down!");
			  MechFalls(wounded,1);
			}
                    }
                }
              break;
            case UPPER_ACTUATOR:
              if(hitloc == LARM && !(wounded->critstatus & LUPACT_DESTROYED))
		 {
		   wounded->critstatus |= LUPACT_DESTROYED;
		   wounded->sections[hitloc].basetohit+=1;
		   mech_notify(wounded,MECHALL, 
				"Your left upper arm actuator is destroyed!!");
		 }
	      else if (hitloc == RARM && 
		       !(wounded->critstatus & RUPACT_DESTROYED))
                {
		  wounded->critstatus |= RUPACT_DESTROYED;
		  wounded->sections[hitloc].basetohit+=1;
		  mech_notify(wounded,MECHALL, 
				"Your right upper arm actuator is destroyed!!");
                }
              else if(hitloc == LLEG || hitloc == RLEG)
                {
                  if(wounded->sections[hitloc].basetohit<3) 
                    {
                      wounded->sections[hitloc].basetohit+=1;
                      wounded->pilotskillbase+=1;
                      wounded->maxspeed -= MP1;
		      if(wounded->maxspeed < 0.0) wounded->maxspeed=0.0;
                      wounded->speed = 0.0;
		      mech_notify(wounded,MECHALL, 
				"One of your leg actuators is destroyed!!");
		      if(!(wounded->status & JUMPING) &&
			 !MadePilotSkillRoll(wounded,0)) 
			{
			  mech_notify(wounded,MECHALL,
					"You lose your balance and fall down!");
			  MechFalls(wounded,1);
			}
		    }
                }
              break;
            case HAND_OR_FOOT_ACTUATOR:
	      if(hitloc==LARM) wounded->critstatus |= LHAND_DESTROYED;
	      if(hitloc==RARM) wounded->critstatus |= RHAND_DESTROYED;
              if(hitloc==LARM || hitloc==RARM) 
                {
		    mech_notify(wounded,MECHALL, 
				"Your hand actuator is destroyed!!");
                }
              else if(hitloc == LLEG || hitloc == RLEG)
                {
                  if(wounded->sections[hitloc].basetohit<3) 
                    {
                      wounded->sections[hitloc].basetohit+=1;
                      wounded->pilotskillbase+=1;
                      wounded->maxspeed -= MP1;
		      if(wounded->maxspeed < 0.0) wounded->maxspeed=0.0;
                      wounded->speed = 0.0;
		      mech_notify(wounded,MECHALL, 
					"Your foot actuator is destroyed!!");
		      if(!(wounded->status & JUMPING) &&
			 !MadePilotSkillRoll(wounded,0)) 
			{
			  mech_notify(wounded,MECHALL,
					"You lose your balance and fall down!");
			  MechFalls(wounded,1);
			}
                    }
                }
              break;
            case C3_MASTER:
            case C3_SLAVE:
              wounded->critstatus |= C3_DESTROYED;
              mech_notify(wounded, MECHALL, "Your C3 system has been destroyed!");
              break;
            case ECM:
              wounded->critstatus |= ECM_DESTROYED;
              mech_notify(wounded, MECHALL, "Your ECM system has been destroyed!");
              break;
            case BEAGLE_PROBE:
              wounded->critstatus |= BEAGLE_DESTROYED;
              mech_notify(wounded, MECHALL, "Your Beagle Probe has been destroyed!");
              break;
            case ARTEMIS_IV:
              weapon_slot = wounded->sections[hitloc].criticals[critHit].data;
              if(weapon_slot > NUM_CRITICALS) 
                {
                  send_mechinfo(tprintf("Artemis IV error on mech %d",wounded->mynum));
                  break;
                }
              wounded->sections[hitloc].criticals[weapon_slot].mode &= ~ARTEMIS_MODE;
              mech_notify(wounded, MECHALL, "Your Artemis IV system has been destroyed!");
              break;
            }
         if (destroycrit)
          wounded->sections[hitloc].criticals[critHit].data= CRIT_DESTROYED;
         num--;
        }
      else num--;
    }
} 

int FindPunchLocation(hitGroup)
int hitGroup;
{ 
  int rc, roll;

  roll=random() % 6 + 1;
    switch(hitGroup) {
    
    case LEFTSIDE:
      switch(roll) {
      case 1:
      case 2:
	rc=LTORSO;
	break;
      case 3:
	rc=CTORSO;
	break;
      case 4:
      case 5:
	rc=LARM;
	break;
      case 6:
	rc=HEAD;
	break;
      }
      break;

    case BACK:
    case FRONT:
      switch(roll) {
      case 1:
	rc=LARM;
	break;
      case 2:
	rc=LTORSO;
	break;
      case 3:
	rc=CTORSO;
	break;
      case 4:
	rc=RTORSO;
	break;
      case 5:
	rc=RARM;
	break;
      case 6:
	rc=HEAD;
	break;
      }
      break;
 
    case RIGHTSIDE:
      switch(roll) {
      case 1:
      case 2:
	rc=RTORSO;
	break;
      case 3:
	rc=CTORSO;
	break;
      case 4:
      case 5:
	rc=RARM;
	break;
      case 6:
	rc=HEAD;
	break;
      }
    }
  return(rc);
}

int FindKickLocation(hitGroup)
int hitGroup;
{ 
  int rc, roll;

  roll=random() % 6 + 1;
    switch(hitGroup) {
    
    case LEFTSIDE:
	rc=LLEG;
	break;

    case BACK:
    case FRONT:
      switch(roll) {
      case 1:
      case 2:
      case 3:
	rc=RLEG;
	break;
      case 4:
      case 5:
      case 6:
	rc=LLEG;
	break;
      }
      break;
 
    case RIGHTSIDE:
        rc=RLEG;
        break;
    }
  return(rc);
}

int FindHitLocation(mech,hitGroup,iscritical)
     struct mech_data *mech;
     int hitGroup, *iscritical;
{
  int roll, hitloc;
  
  *iscritical=0;
  roll=Roll();
  
  switch(mech->type)
    {
    case CLASS_MECH:
      switch(hitGroup) 
	{
	case LEFTSIDE:
	  switch(roll) 
	    {
	    case 2:
	      hitloc=LTORSO;
	      *iscritical=1;
	      break;
	    case 3:
	      hitloc=LLEG;
	      break;
	    case 4:
	    case 5:
	      hitloc=LARM;
	      break;
	    case 6:
	      hitloc=LLEG;
	      break;
	    case 7:
	      hitloc=LTORSO;
	      break;
	    case 8:
	      hitloc=CTORSO;
	      break;
	    case 9:
	      hitloc=RTORSO;
	      break;
	    case 10:
	      hitloc=RARM;
	      break;
	    case 11:
	      hitloc=RLEG;
	      break;
	    case 12:
	      hitloc=HEAD;
	      break;
	    }
	  break;
	  
	case RIGHTSIDE:
	  switch(roll) 
	    {
	    case 2:
	      hitloc=RTORSO;
	      *iscritical=1;
	      break;
	    case 3:
	      hitloc=RLEG;
	      break;
	    case 4:
	    case 5:
	      hitloc=RARM;
	      break;
	    case 6:
	      hitloc=RLEG;
	      break;
	    case 7:
	      hitloc=RTORSO;
	      break;
	    case 8:
	      hitloc=CTORSO;
	      break;
	    case 9:
	      hitloc=LTORSO;
	      break;
	    case 10:
	      hitloc=LARM;
	      break;
	    case 11:
	      hitloc=LLEG;
	      break;
	    case 12:
	      hitloc=HEAD;
	      break;
	    }
	  break;
	  
	case FRONT:
	case BACK:
	  switch (roll) 
	    {
	    case 2:
	      hitloc=CTORSO;
	      *iscritical=1;
	      break;
	    case 3:
	    case 4:
	      hitloc=RARM;
	      break;
	    case 5:
	      hitloc=RLEG;
	      break;
	    case 6:
	      hitloc=RTORSO;
	      break;
	    case 7:
	      hitloc=CTORSO;
	      break;
	    case 8:
	      hitloc=LTORSO;
	      break;
	    case 9:
	      hitloc=LLEG;
	      break;
	    case 10:
	    case 11:
	      hitloc=LARM;
	      break;
	    case 12:
	      hitloc=HEAD;
	      break;
	    }
	}
      break;
    case CLASS_VEH_GROUND:
      switch(hitGroup) 
	{
	case LEFTSIDE:
	  switch(roll) 
	    {
	    case 2:
	      hitloc=LSIDE;
	      *iscritical=1;
	      break;
	    case 3:
	      hitloc=LSIDE;
	      if(!(mech->status & FALLEN))
		{
		  switch(mech->move)
		    {
		    case MOVE_TRACK:
		      mech_notify(mech,MECHALL,"One of your tracks is destroyed, imobilizing your vehicle!!");
		      break;
		    case MOVE_WHEEL:
		      mech_notify(mech,MECHALL,"One of your wheels is destroyed, imobilizing your vehicle!!");
		      break;
		    case MOVE_HOVER:
		      mech_notify(mech,MECHALL,"Your lift fan is destroyed, imobilizing your vehicle!!");
		      break;
		    }
		  mech->status |= FALLEN;
		  mech->maxspeed = 0.0;
		  mech->speed = 0.0;
		}
	      break;
	    case 4:
	    case 5:
	      hitloc=LSIDE;
	      if(!(mech->status & FALLEN))
		{
		  switch(mech->move)
		    {
		    case MOVE_TRACK:
		      mech_notify(mech,MECHALL,"One of your tracks is damaged!!");
		      break;
		    case MOVE_WHEEL:
		      mech_notify(mech,MECHALL,"One of your wheels is damaged!!");
		      break;
		    case MOVE_HOVER:
		      mech_notify(mech,MECHALL,"Your air skirt is damaged!!");
		      break;
		    }
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    case 6:
	    case 7:
	    case 8:
	      hitloc=LSIDE;
	      break;
	    case 9:
	      hitloc=LSIDE;
	      if(mech->move==MOVE_HOVER && !(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your air skirt is damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		} 
	      break;
	    case 10:
	      if(mech->sections[TURRET].internal) hitloc=TURRET;
	      else hitloc=LSIDE;
	      break;
	    case 11:
	      if(mech->sections[TURRET].internal 
		 && !(mech->critstatus & TURRET_LOCKED))
		{
		  hitloc=TURRET;
		  mech_notify(mech,MECHALL,"Your turret is damaged and is locked in position!!");
		  mech->critstatus |= TURRET_LOCKED;
		}
	      else hitloc=LSIDE;
	      break;
	    case 12:
	      hitloc=LSIDE;
	      *iscritical=1;
	      break;
	    }
	  break;
	  
	case RIGHTSIDE:
	  switch(roll) 
	    {
	    case 2:
	      hitloc=RSIDE;
	      *iscritical=1;
	      break;
	    case 3:
	      hitloc=RSIDE;
	      if(!(mech->status & FALLEN))
		{
		  switch(mech->move)
		    {
		    case MOVE_TRACK:
		      mech_notify(mech,MECHALL,"One of your tracks is destroyed, imobilizing your vehicle!!");
		      break;
		    case MOVE_WHEEL:
		      mech_notify(mech,MECHALL,"One of your wheels is destroyed, imobilizing your vehicle!!");
		      break;
		    case MOVE_HOVER:
		      mech_notify(mech,MECHALL,"Your lift fan is destroyed, imobilizing your vehicle!!");
		      break;
		    }
		  mech->status |= FALLEN;
		  mech->maxspeed = 0.0;
		  mech->speed = 0.0;
		}
	      break;
	    case 4:
	    case 5:
	      hitloc=RSIDE;
	      if(!(mech->status & FALLEN))
		{
		  switch(mech->move)
		    {
		    case MOVE_TRACK:
		      mech_notify(mech,MECHALL,"One of your tracks is damaged!!");
		      break;
		    case MOVE_WHEEL:
		      mech_notify(mech,MECHALL,"One of your wheels is damaged!!");
		      break;
		    case MOVE_HOVER:
		      mech_notify(mech,MECHALL,"Your air skirt is damaged!!");
		      break;
		    }
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    case 6:
	    case 7:
	    case 8:
	      hitloc=RSIDE;
	      break;
	    case 9:
	      hitloc=RSIDE;
	      if(mech->move==MOVE_HOVER && !(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your air skirt is damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		} 
	      break;
	    case 10:
	      if(mech->sections[TURRET].internal)
		hitloc=TURRET;
	      else hitloc=RSIDE;
	      break;
	    case 11:
	      if(mech->sections[TURRET].internal && 
		 !(mech->critstatus & TURRET_LOCKED))
		{
		  hitloc=TURRET;
		  mech_notify(mech,MECHALL,"Your turret is damaged and is locked in position!!");
		  mech->critstatus |= TURRET_LOCKED;
		}
	      else hitloc=RSIDE;
	      break;
	    case 12:
	      hitloc=RSIDE;
	      *iscritical=1;
	      break;
	    }
	  break;
	  
	case FRONT:
	case BACK:
	  switch(roll) 
	    {
	    case 2:
	      hitloc=FSIDE;
	      *iscritical=1;
	      break;
	    case 3:
	      hitloc=FSIDE;
	      if(!(mech->status & FALLEN)) 
		{
		  switch(mech->move)
		    {
		    case MOVE_TRACK:
		      mech_notify(mech,MECHALL,"One of your tracks is destroyed, imobilizing your vehicle!!");
		      break;
		    case MOVE_WHEEL:
		      mech_notify(mech,MECHALL,"One of your wheels is destroyed, imobilizing your vehicle!!");
		      break;
		    case MOVE_HOVER:
		      mech_notify(mech,MECHALL,"Your lift fan is destroyed, imobilizing your vehicle!!");
		      break;
		    }
		  mech->status |= FALLEN;
		  mech->maxspeed = 0.0;
		  mech->speed = 0.0;
		}
	      break;
	    case 4:
	    case 5:
	      hitloc=FSIDE;
	      if(!(mech->status & FALLEN))
		{
		  switch(mech->move)
		    {
		    case MOVE_TRACK:
		      mech_notify(mech,MECHALL,"One of your tracks is damaged!!");
		      break;
		    case MOVE_WHEEL:
		      mech_notify(mech,MECHALL,"One of your wheels is damaged!!");
		      break;
		    case MOVE_HOVER:
		      mech_notify(mech,MECHALL,"Your air skirt is damaged!!");
		      break;
		    }
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    case 6:
	    case 7:
	    case 8:
	      hitloc=FSIDE;
	      break;
	    case 9:
	      hitloc=FSIDE;
	      if(mech->move==MOVE_HOVER && !(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your air skirt is damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		} 
	      break;
	    case 10:
	      if(mech->sections[TURRET].internal)
		hitloc=TURRET;
	      else hitloc=FSIDE;
	      break;
	    case 11:
	      if(mech->sections[TURRET].internal &&
		 !(mech->critstatus & TURRET_LOCKED))
		{
		  hitloc=TURRET;
		  mech_notify(mech,MECHALL,"Your turret is damaged and is locked in position!!");
		  mech->critstatus |= TURRET_LOCKED;
		}
	      else hitloc=FSIDE;
	      break;
	    case 12:
	      hitloc=FSIDE;
	      *iscritical=1;
	      break;
	    }
	  break;
	}
      break;
    case CLASS_VEH_VTOL:
      switch(hitGroup) 
	{
	case LEFTSIDE:
	  switch(roll) 
	    {
	    case 2:
	      hitloc=ROTOR;
	      *iscritical=1;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is destroyed!!");
		  if(!(mech->status & LANDED))
		    {
		      mech_notify(mech,MECHALL,"You crash to the ground!");
		      if(mech->terrain==WATER)
			MechFalls(mech, mech->z+mech->elev);
		      else 
			MechFalls(mech, mech->z-mech->elev);
		    }
		  mech->status |= FALLEN;
		  mech->maxspeed = 0.0;
		  mech->speed = 0.0;
		}      
	      break;
	    case 3:
	      hitloc=ROTOR;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is destroyed!!");
		  if(!(mech->status & LANDED))
		    {
		      mech_notify(mech,MECHALL,"You crash to the ground!");
		      if(mech->terrain==WATER)
			MechFalls(mech, mech->z+mech->elev);
		      else 
			MechFalls(mech, mech->z-mech->elev);
		    }
		  mech->status |= FALLEN;
		  mech->maxspeed = 0.0;
		  mech->speed = 0.0;
		}      
	      break;
	    case 4:
	    case 5:
	      hitloc=ROTOR;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    case 6:
	    case 7:
	    case 8:
	    case 9:
	      hitloc=LSIDE;
	      break;
	    case 10:
	    case 11:
	      hitloc=ROTOR;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    case 12:
	      hitloc=ROTOR;
	      *iscritical=1;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    }
	  break;
	  
	case RIGHTSIDE:
	  switch(roll) 
	    {
	    case 2:
	      hitloc=ROTOR;
	      *iscritical=1;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is destroyed!!");
		  if(!(mech->status & LANDED))
		    {
		      mech_notify(mech,MECHALL,"You crash to the ground!");
		      if(mech->terrain==WATER)
			MechFalls(mech, mech->z+mech->elev);
		      else 
			MechFalls(mech, mech->z-mech->elev);
		    }
		  mech->status |= FALLEN;
		  mech->maxspeed = 0.0;
		  mech->speed = 0.0;
		}      
	      break;
	    case 3:
	      hitloc=ROTOR;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is destroyed!!");
		  if(!(mech->status & LANDED))
		    {
		      mech_notify(mech,MECHALL,"You crash to the ground!");
		      if(mech->terrain==WATER)
			MechFalls(mech, mech->z+mech->elev);
		      else 
			MechFalls(mech, mech->z-mech->elev);
		    }
		  mech->status |= FALLEN;
		  mech->maxspeed = 0.0;
		  mech->speed = 0.0;
		}      
	      break;
	    case 4:
	    case 5:
	      hitloc=ROTOR;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is damaged!!");
		  mech->speed = 0.0;
		  mech->maxspeed -= MP1;
		}
	      break;
	    case 6:
	    case 7:
	    case 8:
	    case 9:
	      hitloc=RSIDE;
	      break;
	    case 10:
	    case 11:
	      hitloc=ROTOR;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    case 12:
	      hitloc=ROTOR;
	      *iscritical=1;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    }
	  break;	  
	  
	case FRONT:
	case BACK:
	  switch(roll) 
	    {
	    case 2:
	      hitloc=ROTOR;
	      *iscritical=1;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is destroyed!!");
		  if(!(mech->status & LANDED))
		    {
		      mech_notify(mech,MECHALL,"You crash to the ground!");
		      if(mech->terrain==WATER)
			MechFalls(mech, mech->z+mech->elev);
		      else 
			MechFalls(mech, mech->z-mech->elev);
		    }
		  mech->status |= FALLEN;
		  mech->maxspeed = 0.0;
		  mech->speed = 0.0;
		}      
	      break;
	    case 3:
	      hitloc=ROTOR;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is destroyed!!");
		  if(!(mech->status & LANDED))
		    {
		      mech_notify(mech,MECHALL,"You crash to the ground!");
		      if(mech->terrain==WATER)
			MechFalls(mech, mech->z+mech->elev);
		      else 
			MechFalls(mech, mech->z-mech->elev);
		    }
		  mech->status |= FALLEN;
		  mech->maxspeed = 0.0;
		  mech->speed = 0.0;
		}      
	      break;
	    case 4:
	    case 5:
	      hitloc=ROTOR;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    case 6:
	    case 7:
	    case 8:
	    case 9:
	      hitloc=FSIDE;
	      break;
	    case 10:
	    case 11:
	      hitloc=ROTOR;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    case 12:
	      hitloc=ROTOR;
	      *iscritical=1;
	      if(!(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your rotor is damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    }
	  break;
	}
      break;
    case CLASS_VEH_NAVAL:
      switch(hitGroup) 
	{
	case LEFTSIDE:
	  switch(roll) 
	    {
	    case 2:
	      hitloc=LSIDE;
	      *iscritical=1;
	      break;
	    case 3:
	      hitloc=LSIDE;
	      if(!(mech->status & FALLEN))
		{
		  switch(mech->move)
		    {
		    case MOVE_HULL:
		      mech_notify(mech,MECHALL,"Your engine room is destroyed, imobilizing your vehicle!!");
		      break;
		    case MOVE_FOIL:
		      mech_notify(mech,MECHALL,"Your foils are destroyed, imobilizing your vehicle!!");
		      break;
		    }
		  mech->status |= FALLEN;
		  mech->maxspeed = 0.0;
		  mech->speed = 0.0;
		}
	      break;
	    case 4:
	    case 5:
	      hitloc=LSIDE;
	      if(!(mech->status & FALLEN))
		{
		  switch(mech->move)
		    {
		    case MOVE_HULL:
		      mech_notify(mech,MECHALL,"Your engine room is damaged!!");
		      break;
		    case MOVE_FOIL:
		      mech_notify(mech,MECHALL,"Your foils are damaged!!");
		      break;
		    }
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    case 6:
	    case 7:
	    case 8:
	      hitloc=LSIDE;
	      break;
	    case 9:
	      hitloc=LSIDE;
	      if(mech->move==MOVE_FOIL && !(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your foils are damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		} 
	      break;
	    case 10:
	      if(mech->sections[TURRET].internal)
		hitloc=TURRET;
	      else hitloc=LSIDE;
	      break;
	    case 11:
	      if(mech->sections[TURRET].internal &&
		 !(mech->critstatus & TURRET_LOCKED))
		{
		  hitloc=TURRET;
		  mech_notify(mech,MECHALL,"Your turret is damaged and is locked in position!!");
		  mech->critstatus |= TURRET_LOCKED;
		}
	      else hitloc=LSIDE;
	      break;
	    case 12:
	      hitloc=LSIDE;
	      *iscritical=1;
	      break;
	    }
	  break;
	  
	case RIGHTSIDE:
	  switch(roll) 
	    {
	    case 2:
	      hitloc=RSIDE;
	      *iscritical=1;
	      break;
	    case 3:
	      hitloc=RSIDE;
	      if(!(mech->status & FALLEN))
		{
		  switch(mech->move)
		    {
		    case MOVE_HULL:
		      mech_notify(mech,MECHALL,"Your engine room is destroyed, imobilizing your vehicle!!");
		      break;
		    case MOVE_FOIL:
		      mech_notify(mech,MECHALL,"Your foils are destroyed, imobilizing your vehicle!!");
		      break;
		    }
		  mech->status |= FALLEN;
		  mech->maxspeed = 0.0;
		  mech->speed = 0.0;
		}
	      break;
	    case 4:
	    case 5:
	      hitloc=RSIDE;
	      if(!(mech->status & FALLEN))
		{
		  switch(mech->move)
		    {
		    case MOVE_HULL:
		      mech_notify(mech,MECHALL,"Your engine room is damaged!!");
		      break;
		    case MOVE_FOIL:
		      mech_notify(mech,MECHALL,"Your foils are damaged!!");
		      break;
		    }
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    case 6:
	    case 7:
	    case 8:
	      hitloc=RSIDE;
	      break;
	    case 9:
	      hitloc=RSIDE;
	      if(mech->move==MOVE_FOIL && !(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your foils are damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		} 
	      break;
	    case 10:
	      if(mech->sections[TURRET].internal)
		hitloc=TURRET;
	      else hitloc=RSIDE;
	      break;
	    case 11:
	      if(mech->sections[TURRET].internal &&
		 !(mech->critstatus & TURRET_LOCKED))
		{
		  hitloc=TURRET;
		  mech_notify(mech,MECHALL,"Your turret is damaged and is locked in position!!");
		  mech->critstatus |= TURRET_LOCKED;
		}
	      else hitloc=RSIDE;
	      break;
	    case 12:
	      hitloc=RSIDE;
	      *iscritical=1;
	      break;
	    }
	  break;
	  
	case FRONT:
	case BACK:
	  switch(roll) 
	    {
	    case 2:
	      hitloc=FSIDE;
	      *iscritical=1;
	      break;
	    case 3:
	      hitloc=FSIDE;
	      if(!(mech->status & FALLEN))
		{
		  switch(mech->move)
		    {
		    case MOVE_HULL:
		      mech_notify(mech,MECHALL,"Your engine room is destroyed, imobilizing your vehicle!!");
		      break;
		    case MOVE_FOIL:
		      mech_notify(mech,MECHALL,"Your foils are destroyed, imobilizing your vehicle!!");
		      break;
		    }
		  mech->status |= FALLEN;
		  mech->maxspeed = 0.0;
		  mech->speed = 0.0;
		}
	      break;
	    case 4:
	      hitloc=FSIDE;
	      if(!(mech->status & FALLEN))
		{
		  switch(mech->move)
		    {
		    case MOVE_HULL:
		      mech_notify(mech,MECHALL,"Your engine room is damaged!!");
		      break;
		    case MOVE_FOIL:
		      mech_notify(mech,MECHALL,"Your foils are damaged!!");
		      break;
		    }
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		}
	      break;
	    case 5:
	      hitloc=FSIDE;
	      if(mech->move==MOVE_FOIL && !(mech->status & FALLEN))
		{
		  mech_notify(mech,MECHALL,"Your foils are damaged!!");
		  mech->maxspeed -= MP1;
		  mech->speed = 0.0;
		} 
	      break;
	    case 6:
	    case 7:
	    case 8:
	    case 9:
	      hitloc=FSIDE;
	      break;
	    case 10:
	      if(mech->sections[TURRET].internal)
		hitloc=TURRET;
	      else hitloc=FSIDE;
	      break;
	    case 11:
	      if(mech->sections[TURRET].internal &&
		 !(mech->critstatus & TURRET_LOCKED))
		{
		  hitloc=TURRET;
		  mech_notify(mech,MECHALL,"Your turret is damaged and is locked in position!!");
		  mech->critstatus |= TURRET_LOCKED;
		}
	      else hitloc=FSIDE;
	      break;
	    case 12:
	      hitloc=FSIDE;
	      *iscritical=1;
	      break;
	    }
	  break;
	}
      break;
    }
  mech->maxspeed = (mech->maxspeed > 0.0) ? mech->maxspeed : 0.0;
  return(hitloc);
}  

void DestroySection(wounded,attacker,LOS,hitloc)
struct mech_data *wounded, *attacker; 
int LOS;
int hitloc; 
{
int i;
char areaBuff[30];
int oldjs;
unsigned char critType;
int told = 0;
int nhs = 0;

  /* Prevent the rare occurance of a section getting destroyed twice */
  if(wounded->sections[hitloc].config & SECTION_DESTROYED) return;

  /* Ouch. They got toasted */
  wounded->sections[hitloc].armor=0;
  wounded->sections[hitloc].internal=0;
  wounded->sections[hitloc].rear=0;
  wounded->sections[hitloc].config |= SECTION_DESTROYED;

  ArmorStringFromIndex(hitloc, areaBuff, wounded->type);
  mech_notify(wounded,MECHALL, 
			tprintf("Your %s has been destroyed!", areaBuff));
  if(LOS && (attacker!=wounded)) 
    mech_notify(attacker,MECHALL, tprintf("You destroy the %s!", areaBuff));

  if(wounded->type!=CLASS_MECH) return;

  oldjs = wounded->jumpspeed;
  for(i=0; i<NUM_CRITICALS; i++) 
  {
    if (wounded->sections[hitloc].criticals[i].data != CRIT_DESTROYED)
    {
     wounded->sections[hitloc].criticals[i].data = CRIT_DESTROYED;
     critType=wounded->sections[hitloc].criticals[i].type;
     if(critType >= SPECIAL_BASE_INDEX) 
     {
       switch (critType-SPECIAL_BASE_INDEX) 
       {
           case HEAT_SINK:
             if (wounded->specials & DOUBLE_HEAT_TECH)
             {
                 if ((nhs++) % 3 == 2)
                    wounded->numsinks++;
             }
             wounded->numsinks--;
             break;
           case JUMP_JET:
             wounded->jumpspeed-=MP1;
	     if(wounded->jumpspeed < 0) 
		wounded->jumpspeed=0;
             if (wounded->jumpspeed == 0 && wounded->status & JUMPING)
                {
		  mech_notify(wounded,MECHALL,
		       "Losing your last Jump Jet you fall from the sky!!!!!");
		  if (LOS && wounded != attacker)
		    mech_notify(attacker,MECHALL,
			   "Your hit knocks the 'Mech from the sky");
		  MechFalls(wounded, (int)(oldjs*MP_PER_KPH)); 
		  wounded->status &= ~JUMPING;
		  wounded->jumpheading = JUMP_TO_HIT_RECYCLE;
		  wounded->goingx=wounded->goingy= 0;
		} 
             break;
           case ENGINE:
	      if(wounded->engineheat < 10) 
		{
		  wounded->engineheat+=5;
		}
	      else 
		{
		  if (!told)
		  {
  		    mech_notify(wounded,MECHALL,"Your engine is destroyed!!");
		    if(wounded!=attacker)
		      mech_notify(attacker,MECHALL, "You destroy the engine!!");
		    told = 1;
		  }
		  wounded->status=DESTROYED;
		  wounded->speed=0;
		  wounded->desired_speed=0;
		  wounded->pilot= -1;
		}
              break;
            case TARGETING_COMPUTER:
		if (!wounded->critstatus & TC_DESTROYED)
		{
		  mech_notify(wounded,MECHALL,
		  			"Your Targeting Computer is Destroyed");  
		  wounded->critstatus |= TC_DESTROYED;
		}
	      break;
        }
     }
    }
  }
  
  if(hitloc==LARM) wounded->critstatus |= LARM_DESTROYED;
  else if(hitloc==RARM) wounded->critstatus |= RARM_DESTROYED;
  else if((hitloc==RLEG || hitloc==LLEG) 
	  && !(wounded->critstatus & LEG_DESTROYED)) 
    {
      wounded->critstatus |= LEG_DESTROYED;
      wounded->maxspeed=MP1;
      wounded->pilotskillbase+=5;
      if(!(wounded->status & FALLEN) && !(wounded->status & JUMPING)) 
	{
	  mech_notify(wounded,MECHALL, "You fall down with only one leg!!");
	  MechFalls(wounded,1);
	}
    }
  else if(hitloc==RLEG || hitloc==LLEG) 
    {
      wounded->critstatus |= NO_LEGS;
      wounded->maxspeed=0;
      if(!(wounded->status & FALLEN) && !(wounded->status & JUMPING)) 
	{
	  mech_notify(wounded,MECHALL, "You fall down with no legs!!");
	  MechFalls(wounded,1);
	}
    }
  else if(hitloc==LTORSO && !(wounded->critstatus & LARM_DESTROYED))
    DestroySection(wounded,attacker,LOS,LARM);
  else if(hitloc==RTORSO && !(wounded->critstatus & RARM_DESTROYED))
    DestroySection(wounded,attacker,LOS,RARM);
  
}

int headhitmwdamage(struct mech_data *mech)
{
      
   struct player_stats *s;
   dbref player;
   int damage, playerhits, bruise, lethaldam, roll, ret, playerBLD; 
   
   player=mech->pilot;
           
   /*if(!(db[mech->mynum].flags && IN_CHARACTER))  */
	if(1==1)
      /* check to see if mech is IC */
   {
      ret=10;
 /* This is a return value that tells the calling function that it is IC */
         
      damage=10;                         
         /* dammage due to head hit */
      
      bruise=char_getbruise(player); 
         /* gets the players bruise dammage */
      
      /*s=get_stats(player);             */
	s=0;
         /* get the player_stats structure */
              
      playerBLD=char_getattribute(player, "Build");
         /* get the player's BLD value */
         
      if(char_getattribute(player, "Toughness") < 0)
         roll=char_rollsaving();
         /*  Gets the saving roll for someone with toughness  */
      else
         roll=char_rollskilled();
         /*  Gets the saving roll for someone without toughness  */
         
      bruise-=damage;
         /* this part subtracts 10 from players lethal damage */
      
     if(bruise < 0)      
     { 
          lethaldam = char_getlethal(player);
          lethaldam += bruise;
	  
          if(lethaldam < 0)
	  {    
              lethaldam = 0;
	      /*s->lethal = lethaldam; */
              KillMechContentsIfIC(player); 
	      return ret;    
          }
	  /*s->lethal = lethaldam;*/
	  bruise = 0;
     }
     /*s->bruise = bruise; */
      

     playerhits=bruise; 
         /* playerhits is redundant, but had a purpose before revision */ 

         if(playerhits > 8*playerBLD)     /*  first wound factor */
         {
            if(roll < 3)
            {
               mech->unconcious+=30;
            }
         }
         else if(playerhits > 6*playerBLD)  /*  second WF  */ 
         {
            if(roll < 5)
            {
               mech->unconcious+=30;
            }
         }
         else if(playerhits > 4*playerBLD)
         {
             if(roll < 7)                   /*  third WF  */
             {
               mech->unconcious+=30; 
             }
         }
         else if(playerhits > 2*playerBLD)
         {
             if(roll < 9)                   /*  4th WF  */
             {
               mech->unconcious+=30; 
             }
         }
         else
         {
             if(roll < 11)                   /*  5th WF  */
             {
               mech->unconcious+=30; 
             }
         }
    }
    else
    {
	mech->pilotstatus++;
	UpdatePilotStatus(mech);
    }
   
   return ret;
   
}
