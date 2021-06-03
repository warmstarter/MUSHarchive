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

/* Status commands! */
void mech_status(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech, *tempMech;
  char buff[100],*args[3];
  char buff1[100];
  float x1, y1;
  float waterheatsinks=0.;
  int legsinks;
  int numargs;
  int doweap = 0 , doinfo= 0, doarmor= 0, loop;
  char mech_name[100];
  char mech_ref[100];
  char location[50];
  char move_type[50];
  int weaponarc;
    
  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) {
    if(mech->unconcious) 
      {
	notify(player,"Don't you wish you were awake?");
	return;
      }
    numargs=mech_parseattributes(buffer, args, 3);
    if (numargs == 0) doweap = doinfo = doarmor = 1;
    else
      for (loop=0;loop < numargs; loop++)
        {
          switch (args[loop][0])
            {
            case 'A':
            case 'a':
              doarmor = 1;
              break;
            case 'I':
            case 'i':
              doinfo = 1;
              break;
            case 'W':
            case 'w':
              doweap = 1;
              break;
            }
        }

    get_mech_atr(mech, mech_name, "MECHNAME");
    get_mech_atr(mech, mech_ref, "MECHREF");

    if(mech->type==CLASS_MECH)
      {	
	sprintf(buff, "Mech Name: %-18.18s  ID:[%c%c]   Mech Reference: %s", 
		mech_name, 
		mech->ID[0],
		mech->ID[1],
		mech_ref);
	notify(player, buff);
	notify(player, tprintf("Tonnage:   %3d       MaxSpeed: %3d       JumpRange: %d", 
			       mech->tons,
			       (int)mech->maxspeed,
			       (int)(mech->jumpspeed*MP_PER_KPH)));
	if(mech->pilot== -1) 
	  {
	    notify(player, "Pilot: NONE");
	  } 
	else 
	  {
	    sprintf(buff, "Pilot Name: %-28.28s Pilot Injury: %d\n", 
		    Name(mech->pilot), mech->pilotstatus);
	    notify(player, buff);
	  }
	if (doarmor) 
	  {
	    PrintArmorStatus(player, mech);
	    notify(player, " ");
	  }
	if (doinfo)
	  {
	    sprintf(buff, "X, Y, Z:%3d,%3d,%3d  Excess Heat:  %3d deg C.  Heat Production:  %3d deg C.", 
		    mech->x, 
		    mech->y, 
		    mech->z,
		    (int)(10.*mech->heat), 
		    (int)(10.*mech->plus_heat));
	    notify(player, buff);
	    sprintf(buff, "Speed:      %3d KPH  Heading:      %3d deg     Heat Sinks:       %3d", 
		    (int)(mech->speed), 
		    mech->facing, 
		    mech->numsinks);
	    notify(player, buff);
	    sprintf(buff, "Des. Speed: %3d KPH  Des. Heading: %3d deg     Heat Dissipation: %3d deg C.", 
		    (int)(mech->desired_speed), 
		    mech->desiredfacing,
		    (int)(10.*mech->minus_heat));
	    notify(player, buff);
	    if(mech->status & DESTROYED) {
	      notify(player, "DESTROYED");
	    } 
	    if(!(mech->status & STARTED)) {
	      notify(player, "SHUTDOWN");
	    } 
	    if(mech->status & FALLEN) {
	      notify(player, "FALLEN");
	    } 
	    if(!(mech->status & JUMPING) && !(mech->status & FALLEN) 
	       && (mech->status & STARTED) && (mech->chgtarget != -1)) 
	      {
		tempMech=(struct mech_data *) FindObjectsData(mech->chgtarget);
		if(tempMech)
		  {
		    get_mech_atr(tempMech, mech_name, "MECHNAME");
		    sprintf(buff, "CHARGING --> %s:[%c%c]", 
			    mech_name,
			    tempMech->ID[0],
			    tempMech->ID[1]);
		    notify(player, buff);
		  }                 
	      }
	    if(mech->status & JUMPING) {
	      sprintf(buff, "JUMPING --> %3d,%3d",mech->goingx,mech->goingy);
	    }
	    if(mech->status & DFA_ATTACK) {
	      if(mech->dfatarget != -1)
		{
		  tempMech=(struct mech_data *) FindObjectsData(mech->dfatarget);
		  if(tempMech)
		    get_mech_atr(tempMech, mech_name, "MECHNAME");
		  sprintf(buff, "JUMPING --> %3d,%3d  Death From Above Target: %s:[%c%c]", 
			  mech->goingx,
			  mech->goingy, 
			  mech_name, 
			  tempMech->ID[0],
			  tempMech->ID[1]);
		}
	    } 
	    if(mech->status & JUMPING)
	      notify(player, buff);
	    if(mech->status & TORSO_RIGHT) {
	      notify(player, "Torso is 60 degrees right");
	    } 
	    if(mech->status & TORSO_LEFT) {
	      notify(player, "Torso is 60 degrees left");
	    }
	    if(mech->target!= -1) 
	      {
		tempMech=(struct mech_data *) FindObjectsData(mech->target);
		if(tempMech) 
		  {
		    if(InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,
				     FindRange(mech->fx,mech->fy,mech->fz,
					       tempMech->fx,tempMech->fy,
					       tempMech->fz))) 
		      {
			get_mech_atr(tempMech, mech_name, "MECHNAME");
			sprintf(buff, "\nTarget: %s:[%c%c]\t   Range: %.1f hexes   Bearing: %d deg",
				mech_name,
				tempMech->ID[0],tempMech->ID[1],
				FindRange(mech->fx, mech->fy, mech->fz,
					  tempMech->fx, tempMech->fy, 
					  tempMech->fz),
				FindBearing(mech->fx, mech->fy, 
					    tempMech->fx, tempMech->fy));
			notify(player, buff);
			switch(InWeaponArc(mech,tempMech->fx,tempMech->fy)) 
			  {
			  case FORWARDARC:
			    strcpy(buff, "Target in Forward Weapons Arc");
			    break;
			  case LARMARC:
			    strcpy(buff, "Target in Left Arm Weapons Arc");
			    break;
			  case RARMARC:
			    strcpy(buff, "Target in Right Arm Weapons Arc");
			    break;
			  case REARARC:
			    strcpy(buff, "Target in Rear Weapons Arc");
			    break;
			  default:
			    strcpy(buff, "Target NOT in Any Weapons Arc!");
			  }
			ArmorStringFromIndex(mech->aim,location,tempMech->type);
			if(mech->aim==NUM_SECTIONS) strcpy(location, "None");
			sprintf(buff1, "\t   Aimed Shot Location: %s\n",location);
			strcat(buff, buff1);
		      }
		    else 
		      {
			sprintf(buff, "\nTarget: NOT in line of sight!\n");
		      }
		  }
		notify(player, buff);
	      } 
	    else if(mech->targx!=-1 && mech->targy!=-1) 
	      {
		sprintf(buff, "\nTarget: Hex %d %d\n", mech->targx, mech->targy); 
		notify(player, buff);
	      }
	  }
	if (doweap)
	  PrintWeaponStatus(mech, player);
      }
    else if(mech->type==CLASS_VEH_GROUND ||
	    mech->type==CLASS_VEH_VTOL ||
	    mech->type==CLASS_VEH_NAVAL)
      {
	switch (mech->move)
	  {
	  case MOVE_TRACK:
	    strcpy(move_type,"Tracked");
	    break;
	  case MOVE_WHEEL:
	    strcpy(move_type,"Wheeled");
	    break;
	  case MOVE_HOVER:
	    strcpy(move_type,"Hover");
	    break;
	  case MOVE_VTOL:
	    strcpy(move_type,"VTOL");
	    break;
	  default:
	    strcpy(move_type,"Magic");
	    break;
	  }
	sprintf(buff, "Vehicle Name: %-15.15s  ID:[%c%c]   Vehicle Reference: %s", 
		mech_name, 
		mech->ID[0],
		mech->ID[1],
		mech_ref);
	notify(player, buff);
	sprintf(buff, "Tonnage:   %3d     FlankSpeed: %3d       Movement Type: %s", 
			       mech->tons,
			       (int)mech->maxspeed,
			       move_type);
	notify(player, buff);
	
	if(mech->pilot== -1) 
	  {
	    notify(player, "Pilot: NONE");
	  } 
	else 
	  {
	    sprintf(buff, "Pilot Name: %-28.28s Pilot Injury: %d\n", 
		    Name(mech->pilot), mech->pilotstatus);
	    notify(player, buff);
	  }
	if (doarmor) 
	  {
	    PrintArmorStatus(player, mech);
	    notify(player, " ");
	  }
	if (doinfo)
	  {
	    sprintf(buff, "X, Y, Z:%3d,%3d,%3d  Excess Heat:  %3d deg C.  Heat Production:  %3d deg C.", 
		    mech->x, 
		    mech->y, 
		    mech->z,
		    (int)(10.*mech->heat), 
		    (int)(10.*mech->plus_heat));
	    notify(player, buff);
	    sprintf(buff, "Speed:      %3d KPH  Heading:      %3d deg     Heat Sinks:       %3d", 
		    (int)(mech->speed), 
		    mech->facing, 
		    mech->numsinks);
	    notify(player, buff);
	    sprintf(buff, "Des. Speed: %3d KPH  Des. Heading: %3d deg     Heat Dissipation: %3d deg C.", 
		    (int)(mech->desired_speed), 
		    mech->desiredfacing,
		    (int)(10.*mech->minus_heat));
	    notify(player, buff);
	    if(mech->type!=CLASS_VEH_VTOL && mech->sections[TURRET].internal) {
	      sprintf(buff, "Turret Facing: %3d deg", mech->turretfacing);
	      if(mech->critstatus & TURRET_LOCKED)
		strcat(buff, "     TURRET LOCKED");
	      notify(player, buff);
	    } else {
	      sprintf(buff, "Vertical:   %3d KPH", (int)(mech->verticalspeed));
	      notify(player, buff);
	    }
	    if(mech->status & DESTROYED)
	      notify(player, "DESTROYED");
	    if(!(mech->status & STARTED))
	      notify(player, "SHUTDOWN");
	    if(mech->move==MOVE_VTOL && mech->status & LANDED)
	      notify(player, "LANDED");
	    if(mech->status & FALLEN) {
	      switch (mech->move) {
	      case MOVE_TRACK:
		notify(player, "TRACK DESTROYED");
		break;
	      case MOVE_WHEEL:
		notify(player, "AXEL DESTROYED");
		break;
	      case MOVE_HOVER:
		notify(player, "LIFT FAN DESTROYED");
		break;
	      case MOVE_VTOL:
		notify(player, "ROTOR DESTROYED");
		break;
	      case MOVE_HULL:
		notify(player, "ENGINE ROOM DESTROYED");
		break;
	      case MOVE_FOIL:
		notify(player, "FOIL DESTROYED");
		break;
	      }
	    } 
	    if(mech->target!= -1) 
	      {
		tempMech=(struct mech_data *) FindObjectsData(mech->target);
		if(tempMech) 
		  {
		    if(InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,
				     FindRange(mech->fx,mech->fy,mech->fz,
					       tempMech->fx,tempMech->fy,
					       tempMech->fz))) 
		      {
			get_mech_atr(tempMech, mech_name, "MECHNAME");
			sprintf(buff, "\nTarget: %s:[%c%c]\t   Range: %.1f hexes   Bearing: %d deg",
				mech_name,
				tempMech->ID[0],tempMech->ID[1],
				FindRange(mech->fx, mech->fy, mech->fz, 
					  tempMech->fx, tempMech->fy, 
					  tempMech->fz),
				FindBearing(mech->fx, mech->fy, 
					    tempMech->fx, tempMech->fy));
			notify(player, buff);
			weaponarc=InWeaponArc(mech,tempMech->fx,tempMech->fy);
			if(weaponarc >= TURRETARC) 
			  {
			    notify(player, "Target in Turret Arc");
			    weaponarc -= TURRETARC;
			  }
			switch(weaponarc)
			  {
			  case FORWARDARC:
			    strcpy(buff, "Target in Forward Weapons Arc");
			    break;
			  case RSIDEARC:
			    strcpy(buff, "Target in Right Weapons Arc");
			    break;
			  case LSIDEARC:
			    strcpy(buff, "Target in Left Weapons Arc");
			    break;
			  case REARARC:
			    strcpy(buff, "Target in Rear Weapons Arc");
			    break;
			  default:
			    strcpy(buff, "Target NOT in Any Weapons Arc!");
			  }
			ArmorStringFromIndex(mech->aim,location,tempMech->type);
			if(mech->aim==NUM_SECTIONS) strcpy(location, "None");
			sprintf(buff1, "\t   Aimed Shot Location: %s\n",location);
			strcat(buff, buff1);
		      }
		    else 
		      {
			sprintf(buff, "\nTarget: NOT in line of sight!\n");
		      }
		  }
		notify(player, buff);
	      } 
	    else if(mech->targx!=-1 && mech->targy!=-1) 
	      {
		sprintf(buff, "\nTarget: Hex %d %d\n", mech->targx, mech->targy); 
		notify(player, buff);
	      }
	  }
	if (doweap)
	  PrintWeaponStatus(mech, player);
      }
  }
}

void mech_critstatus(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1];
  int index;

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) {
    if(mech->unconcious) 
      {
	notify(player,"Don't you wish you were awake?");
	return;
      }
    if(1==mech_parseattributes(buffer, args, 1)) {
      index=ArmorSectionFromString(mech->type, args[0]);
      if(index!= -1) {
	CriticalStatus(player, mech, index);
      } else {
	notify(player, "Invalid section!");
      }
    } else {
      notify(player, "You must specify a section to list the criticals for!");
    }
  }
}

void mech_weaponspecs(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[5];
  int argc;
  int weapnum;
  int loop;
  unsigned char weaparray[MAX_WEAPS_SECTION];
  unsigned char weapdata[MAX_WEAPS_SECTION];
  int critical[MAX_WEAPS_SECTION];
  unsigned char weaps[8*MAX_WEAPS_SECTION];
  char weapname[30];
  int num_weaps;
  int weapcount=0;
  int index;
  int duplicate, ii;

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
	{
	  notify(player,"Don't you wish you were awake?");
	  return;
	}
      notify(player,"Weapon Name             Heat  Damage  Range: Min Short Medium Long");
      for(loop=0; loop<NUM_SECTIONS; loop++) 
	{
	  num_weaps=FindWeapons(mech, loop, weaparray, weapdata, critical);
	  for(index=0; index<num_weaps; index++) 
	    {
	      duplicate=0;
	      for(ii=0; ii<weapcount; ii++)
		if(weaparray[index]==weaps[ii]) duplicate=1;
	      if(!duplicate) 
		{
		  weaps[weapcount]=weaparray[index];
		  weapcount++;
		  sprintf(weapname, MechWeapons[weaparray[index]].name);
		  strncat(weapname, "                        ", (22-strlen(weapname)));
		  notify(player,         
			 tprintf("%s   %2d     %2d           %2d   %2d     %2d    %2d",
				 weapname,
				 MechWeapons[weaparray[index]].heat,
				 MechWeapons[weaparray[index]].damage,
				 MechWeapons[weaparray[index]].min,
				 MechWeapons[weaparray[index]].shortrange,
				 MechWeapons[weaparray[index]].medrange,
				 MechWeapons[weaparray[index]].longrange));
		}
	    }
	}
    }
}

void CriticalStatus(player,mech,index)
dbref player; 
struct mech_data *mech; 
int index; 
{
  int loop;
  char buffer[100];
  unsigned char type, data;
  int max_crits=NUM_CRITICALS;

  if(mech->type==CLASS_MECH)
    {
      switch(index) {
      case HEAD:
	sprintf(buffer, "Head Criticals:");
	max_crits=6;
	break;
      case CTORSO:
	sprintf(buffer, "Center Torso Criticals:");
	break;
      case RTORSO:
	sprintf(buffer, "Right Torso Criticals:");
	break;
      case LTORSO:
	sprintf(buffer, "Left Torso Criticals:");
	break;
      case RARM:
	sprintf(buffer, "Right Arm Criticals:");
	break;
      case LARM:
	sprintf(buffer, "Left Arm Criticals:");
	break;
      case RLEG:
	sprintf(buffer, "Right Leg Criticals:");
	max_crits=6;
	break;
      case LLEG:
	sprintf(buffer, "Left Leg Criticals:");
	max_crits=6;
	break;
      }
    }
  else
    { 
      switch(index) {
      case FSIDE:
	sprintf(buffer, "Front Side Criticals:");
	break;
      case BSIDE:
	sprintf(buffer, "Back Side Criticals:");
	break;
      case LSIDE:
	sprintf(buffer, "Left Side Criticals:");
	break;
      case RSIDE:
	sprintf(buffer, "Right Side Criticals:");
	break;
      case ROTOR:
	sprintf(buffer, "Rotor Criticals:");
	break;
      case TURRET:
	sprintf(buffer, "Turret Criticals:");
	break;
      }
    }

  notify(player, buffer);
  for(loop=0; loop<max_crits; loop++) {
    if(loop+1<10) {
      sprintf(buffer, " %d ", loop+1);
    } else {
      sprintf(buffer, "%d ", loop+1);
    }
    type=mech->sections[index].criticals[loop].type;
    data=mech->sections[index].criticals[loop].data;

    if(type==EMPTY) {
      strcat(buffer, "Empty");
      data=0; /* avoid printing destroyed next to empty slots */
    } else if(type >= WEAPON_BASE_INDEX && type < AMMO_BASE_INDEX) {
      strcat(buffer, MechWeapons[type-WEAPON_BASE_INDEX].name);
    } else if (type >= AMMO_BASE_INDEX && type < RESERVED_BASE_INDEX) {
      char trash[50];
      strcat(buffer, MechWeapons[type-AMMO_BASE_INDEX].name);
      if(mech->sections[index].criticals[loop].mode & LBX_MODE)
        strcat(buffer, " Shotgun");
      else if(mech->sections[index].criticals[loop].mode & ARTEMIS_MODE)
        strcat(buffer, " Artemis IV compatible");
      else if(mech->sections[index].criticals[loop].mode & NARC_MODE)
        strcat(buffer, " Narc Beacon compatible");
      strcat(buffer, " Ammo");
      if(data != CRIT_DESTROYED) {
	sprintf(trash, " (%d)", data);
	strcat(buffer, trash);
      }
    } else if(type >=RESERVED_BASE_INDEX && type < SPECIAL_BASE_INDEX) {
      strcat(buffer, "Reserved!");
    } else {
      /* It is a Special Type... */
      type-=SPECIAL_BASE_INDEX;
      switch(type) {
      case SHOULDER_OR_HIP:
	switch(index) {
	case RLEG:
	case LLEG:
	  strcat(buffer, "Hip");
	  break;
	case RARM:
	case LARM:
	  strcat(buffer, "Shoulder");
	  break;
	default:
	  strcat(buffer, "*** ERROR ***");
	  break;
	}
	break;
      case UPPER_ACTUATOR:
	switch(index) {
	case RLEG:
	case LLEG:
	  strcat(buffer, "Upper Leg Actuator");
	  break;
	case RARM:
	case LARM:
	  strcat(buffer, "Upper Arm Actuator");
	  break;
	default:
	  strcat(buffer, "*** ERROR ***");
	  break;
	}
	break;
      case LOWER_ACTUATOR:
	switch(index) {
	case RLEG:
	case LLEG:
	  strcat(buffer, "Lower Leg Actuator");
	  break;
	case RARM:
	case LARM:
	  strcat(buffer, "Lower Arm Actuator");
	  break;
	default:
	  strcat(buffer, "*** ERROR ***");
	  break;
	}
	break;
      case HAND_OR_FOOT_ACTUATOR:
	switch(index) {
	case RLEG:
	case LLEG:
	  strcat(buffer, "Foot Actuator");
	  break;
	case RARM:
	case LARM:
	  strcat(buffer, "Hand Actuator");
	  break;
	default:
	  strcat(buffer, "*** ERROR ***");
	  break;
	}
	break;

      case LIFE_SUPPORT:
	if(index==HEAD) {
	  strcat(buffer, "Life Support");
	} else {
	  strcat(buffer, "*** ERROR ***");
	} 
	break;

      case SENSORS:
	if(index==HEAD) {
	  strcat(buffer, "Sensors");
	} else {
	  strcat(buffer, "*** ERROR ***");
	} 
	break;

      case COCKPIT:
	if(index==HEAD) {
	  strcat(buffer, "Cockpit");
	} else {
	  strcat(buffer, "*** ERROR ***");
	} 
	break;

      case ENGINE:
	if(index==CTORSO || index==LTORSO || index==RTORSO) {
	  strcat(buffer, "Engine");
	} else {
	  strcat(buffer, "*** ERROR ***");
	}
	break;

      case GYRO:
	if(index==CTORSO) {
	  strcat(buffer, "Gyro");
	} else {
	  strcat(buffer, "*** ERROR ***");
	}
	break;

      case HEAT_SINK:
	strcat(buffer, "Heat Sink");
	break;

      case JUMP_JET:
	strcat(buffer, "Jump Jet");
	break;

      case CASE:
	strcat(buffer, "Cellular Ammunition Storage Equipment");
	break;

      case FERRO_FIBROUS:
	strcat(buffer, "Ferro Fibrous Armor");
	break;

      case ENDO_STEEL:
	strcat(buffer, "Endo Steel Internal Structure");
	break;

      case TRIPLE_STRENGTH_MYOMER:
	strcat(buffer, "Triple Strength Myomer");
	break;

      case MASC:
	strcat(buffer, "Myomer Accelerator Signal Circuitry");
	break;

      case TARGETING_COMPUTER:
	strcat(buffer, "Targeting Computer");
	break;

      case C3_MASTER:
        strcat(buffer, "C3 Command Computer");
        break;
        
      case C3_SLAVE:
        strcat(buffer, "C3 Slave Computer");
        break;
        
      case BEAGLE_PROBE:
        strcat(buffer, "Beagle Active Probe");
        break;
        
      case ARTEMIS_IV:
        strcat(buffer, "Artemis IV Fire-Control System");
        break;
        
      case ECM:
        strcat(buffer, "Guardian ECM Suite");
        break;
        
      }
    }

    if(data==CRIT_DESTROYED) {
      strcat(buffer, " (Destroyed)");
    }
    notify(player, buffer);
  }
}

void PrintWeaponStatus(mech,player)
struct mech_data *mech; 
dbref player; 
{
  unsigned char weaparray[MAX_WEAPS_SECTION];
  unsigned char weapdata[MAX_WEAPS_SECTION];
  int critical[MAX_WEAPS_SECTION];
  unsigned char ammoweap[8*MAX_WEAPS_SECTION];
  unsigned char ammo[8*MAX_WEAPS_SECTION];
  unsigned char modearray[8*MAX_WEAPS_SECTION];
  int count,ammoweapcount;
  int loop;
  int ii,jj;
  int len;
  char weapname[20];
  char weapbuff[70];
  char tempbuff[80];
  char location[20];
  int running_sum=0;
  int ammoLoc;
  int ammoCrit;
  int rounds;
  char mode;
  char ammo_mode;

  if(mech->type==CLASS_MECH) {
    sprintf(tempbuff,
	    "LARM: %-5.5s        RARM: %-5.5s        LLEG: %-5.5s        RLEG: %-5.5s",
	    (mech->sections[LARM].recycle > 0) ? 
	    tprintf("%d",mech->sections[LARM].recycle) : "Ready",
	    (mech->sections[RARM].recycle > 0) ? 
	    tprintf("%d",mech->sections[RARM].recycle) : "Ready",
	    (mech->sections[LLEG].recycle > 0) ? 
	    tprintf("%d",mech->sections[LLEG].recycle) : "Ready",
	    (mech->sections[RLEG].recycle > 0) ? 
	    tprintf("%d",mech->sections[RLEG].recycle) : "Ready");
    notify(player,tempbuff);
    if (mech->status & FLIPPED_ARMS)
      notify(player,"*** Mech arms are flipped into the rear arc ***");
  }
  
  ammoweapcount=FindAmmunition(mech,ammoweap,ammo,modearray);
  notify(player, "==================WEAPON SYSTEMS===========================AMMUNITION========");
  notify(player, "------ Weapon ------- [##]  Location ---- Status ||---- Ammo Type ---- Rounds");
  for(loop=0; loop<NUM_SECTIONS; loop++) {
    count=FindWeapons(mech, loop, weaparray, weapdata, critical);
    if(count) {
      ArmorStringFromIndex(loop, tempbuff, mech->type);
      sprintf(location,"%-14.14s",tempbuff);

      for(ii=0; ii<count; ii++) {

        if(weaparray[ii]==IS_AMS || weaparray[ii]==CL_AMS)
	  sprintf(weapbuff," %-16.16s %c%c%c [%2d]  ",
			&MechWeapons[weaparray[ii]].name[3],
                        (mech->status & AMS_ENABLED)?' ':'O',
                        (mech->status & AMS_ENABLED)?'O':'F',
                        (mech->status & AMS_ENABLED)?'N':'F',
			running_sum+ii);
        else
          {
            if(mech->sections[loop].criticals[critical[ii]].mode & ULTRA_MODE)
              mode = 'U';
            else if(mech->sections[loop].criticals[critical[ii]].mode & LBX_MODE)
              mode = 'L';
            else if(mech->sections[loop].criticals[critical[ii]].mode & ARTEMIS_MODE) 
              mode = 'A';
            else if(mech->sections[loop].criticals[critical[ii]].mode & NARC_MODE)
              mode = 'N';
           else
              mode = ' ';
            sprintf(weapbuff," %-16.16s %c%c%c [%2d]  ",
                    &MechWeapons[weaparray[ii]].name[3],
                    (mech->sections[loop].criticals[critical[ii]].mode & REAR_MOUNT)?'R':' ',
                    mode,
                    (mech->sections[loop].criticals[critical[ii]].mode & ON_TC)?'T':' ',
                    running_sum+ii);
          }
        
	strcat(weapbuff, location);
	
	if(weapdata[ii]==CRIT_DESTROYED) 
	  {
	    strcat(weapbuff, "*****  || ");
	  } 
	else 
	  { 
	    if(weapdata[ii])  
	      {
		sprintf(tempbuff," %2d    || ", weapdata[ii]);
		strcat(weapbuff, tempbuff);
	      } 
	    else 
	      { 
		strcat(weapbuff, "Ready  || ");
	      }
	  }
	if((ii+running_sum)<ammoweapcount)
	  {
            if(modearray[ii+running_sum] & LBX_MODE) ammo_mode = 'L';
            else if(modearray[ii+running_sum] & ARTEMIS_MODE) ammo_mode = 'A';
            else if(modearray[ii+running_sum] & NARC_MODE) ammo_mode = 'N';
            else ammo_mode = ' ';
	    sprintf(weapname,"%-16.16s %c",
                    &MechWeapons[ammoweap[ii+running_sum]].name[3],
                    ammo_mode);
	    sprintf(tempbuff,"  %3d",ammo[ii+running_sum]);
	    strcat(weapname,tempbuff);
	  }
	else
	  {
	    sprintf(weapname,"   ");
	  }
	strcat(weapbuff, weapname);
	notify(player, weapbuff);
      }
      running_sum += count;
    }
  }
}

void PrintArmorStatus(player, mech)
     dbref player;
     struct mech_data *mech;
{
  char buff[100];
  
  if(mech->type==CLASS_MECH) {
    sprintf(buff,  "         FRONT                REAR                INTERNAL");
    notify(player, buff);
    sprintf(buff,  "          (%1d)                  (*)                  (%1d)",
	    mech->sections[HEAD].armor,
	    mech->sections[HEAD].internal);
    
    notify(player, buff);
    
    sprintf(buff, "      /%2d|%2d|%2d\\           /%2d|%2d|%2d\\           /%2d|%2d|%2d\\",
	    mech->sections[LTORSO].armor,
	    mech->sections[CTORSO].armor,
	    mech->sections[RTORSO].armor,
	    mech->sections[LTORSO].rear,
	    mech->sections[CTORSO].rear,
	    mech->sections[RTORSO].rear,
	    mech->sections[LTORSO].internal,
	    mech->sections[CTORSO].internal,
	    mech->sections[RTORSO].internal);
    
    notify(player, buff);
    
    sprintf(buff, "     (%2d/ || \\%2d)         (   |  |   )         (%2d/ || \\%2d)",
	    mech->sections[LARM].armor,
	    mech->sections[RARM].armor,
	    mech->sections[LARM].internal,
	    mech->sections[RARM].internal);
    
    notify(player, buff);
    
    sprintf(buff, "       /  /\\  \\               /  \\               /  /\\  \\");
    notify(player, buff);
    
    sprintf(buff, "      (%2d/  \\%2d)             /    \\             (%2d/  \\%2d)",
	    mech->sections[LLEG].armor,
	    mech->sections[RLEG].armor,
	    mech->sections[LLEG].internal,
	    mech->sections[RLEG].internal);
    notify(player, buff);
  } else {  
    sprintf(buff,  "          FRONT                                INTERNAL");
    notify(player, buff);
    sprintf(buff,  "         ,`.%2d,'.                              ,`.%2d,'.",
	    mech->sections[FSIDE].armor,
	    mech->sections[FSIDE].internal);
    
    notify(player, buff);
     
    sprintf(buff, "        |  |__|  |                            |  |__|  |");
    
    notify(player, buff);
    
    if(mech->type==CLASS_VEH_VTOL)
      sprintf(buff, "        |  |%2d|  |                            |  |%2d|  |",
	      mech->sections[ROTOR].armor,
	      mech->sections[ROTOR].internal);
    else
      sprintf(buff, "        |  |%2d|  |                            |  |%2d|  |",
	      mech->sections[TURRET].armor,
	      mech->sections[TURRET].internal);
    
    notify(player, buff);

    sprintf(buff, "        |%2d|~~|%2d|                            |%2d|~~|%2d|",
	    mech->sections[LSIDE].armor,
	    mech->sections[RSIDE].armor,
	    mech->sections[LSIDE].internal,
	    mech->sections[RSIDE].internal);
    
    notify(player, buff);
    
    sprintf(buff, "         \\,'%2d`./                              \\,'%2d`./",
	    mech->sections[BSIDE].armor,
	    mech->sections[BSIDE].internal);
    
    notify(player, buff);
    
    sprintf(buff, "          ~~~~~~                                ~~~~~~");
    
    notify(player, buff);
    
 }    
}  











