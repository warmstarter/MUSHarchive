/* $Id: vmhelm.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
#include "header.h"
#include "vmweaptypes.h"

#define DIRECT 1
#define INDIRECT 2
#define GetState(x) (checkflag(x,WEAPONLINE) ? "on" : "off")
#define GetPState(x) (checkflag(x,GUNPORT) ? "open" : "closed")
#define VMRange(x,y) (VMdistance(VMSpace[x].VMPerm.Nav.XYZCoords,VMSpace[y].VMPerm.Nav.XYZCoords))
extern int MasterMineCnt;

static char *rcsvmhelmc="$Id: vmhelm.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
int CanAccessHelm2(int ship,
	      int playerr
	      ) {
  
  if(!ShipActive(ship)) {
    VMnotify(playerr,"This ship is not in space.");
    return 0;
  }

  if (VMGlobalSpaceInit == 0) {
    VMnotify(playerr,"Space has not been initialized.");
    return 0;
  }
  if(ship < 0 || ship > VMCurrentMax) {
    VMnotify(playerr,"Danger Danger. Possible Bug! Ship number is out of possible range");
    return 0;
  }
  if(!IsPlayerInManList(VMSpace[ship].VMTemp.TmpHelm.VMMan_List,playerr)) {
    return 0;
  }
  return 1;
}



int CanAccessHelm(int ship,
	      int playerr
	      ) {
  
  if(!ShipActive(ship)) {
    VMnotify(playerr,"This ship is not in space.");
    return 0;
  }

  if (VMGlobalSpaceInit == 0) {
    VMnotify(playerr,"Space has not been initialized.");
    return 0;
  }
  if(ship < 0 || ship > VMCurrentMax) {
    VMnotify(playerr,"Danger Danger. Possible Bug! Ship number is out of possible range");
    return 0;
  }
  if(!IsPlayerInManList(VMSpace[ship].VMTemp.TmpHelm.VMMan_List,playerr)) {
    VMnotify(playerr,"You must man the helm console");
    return 0;
  }
  return 1;
}

void VMHelmAlloc(int player, 
		int ship, 
		int nargs, 
		char *args[]
		) {
  int i,k;
  double powervals[MAXSYSTEMS];
  double total = 0;
  for (i = 0; i < nargs; i++) {
    powervals[i] = atof(args[i]);
    if (powervals[i] > VMSpace[ship].VMTemp.TmpEng.PowerAvail || powervals[i] < 0) {
	VMnotify(player, "Power out of bounds");
        return;
    }
    total += powervals[i];
  }
  for (i = 0, k = 0; i < MAXSYSTEMS; i++) {
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,EXISTS))
      if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,POWERABLE))
	{
	  if (k < nargs) {
	    if (total > 0) {
	      VMSpace[ship].VMPerm.Helm.Systems[i].tdstat0 = powervals[k] / total;
	    }
	    else
	      VMSpace[ship].VMPerm.Helm.Systems[i].tdstat0 = 0;
	    VMnotifymans(VMSpace[ship].VMTemp.TmpHelm.VMMan_List,"Setting the power of %s to %d%% of the total power",VMSpace[ship].VMPerm.Helm.Systems[i].name,(int) (VMSpace[ship].VMPerm.Helm.Systems[i].tdstat0 * 100.0));
	    
	    k++;
	  }
	  else
	    VMSpace[ship].VMPerm.Helm.Systems[i].tdstat0 = 0;
	}
  }
}

      

void VMHelmReallocPower(int ship
		  ) {
  int i;
  for (i = 0; i < HSystems; i++) {
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,EXISTS))
      if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,POWERABLE)) {
	VMSpace[ship].VMPerm.Helm.Systems[i].tistat0 = VMSpace[ship].VMPerm.Helm.Systems[i].tdstat0 * VMSpace[ship].VMTemp.TmpEng.PowerHelm;
      }
  }
  
  VMSpace[ship].VMTemp.TmpHelm.PowerUsed = VMSpace[ship].VMTemp.TmpEng.PowerHelm;
}   
	  
void VMWeaponChangeState(int ship,
		    int player,
		    char *what,
		    int weapon
		    ) {
  if (weapon < 0 || weapon > HSystems) {
    VMnotify(player,"That system does not exist!");
    return;
  }
  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,EXISTS)) {
    if ((checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,GUNFLAG) || checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,TUBEFLAG)) && !(checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,NONWEAP))) {
      /* if we get to this point then the weapon exists and is a weapon */
      switch (what[0]) {
      case 'O':
      case 'o':
	switch(what[1]) {
	case 'F':
	case 'f':
	  /* here we will put the weapon offline if necessary */
	  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,WEAPONLINE)) {
	    VMSpace[ship].VMPerm.Helm.Systems[weapon].flags = clearflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,WEAPONLINE);
	    VMnotify(player,"[HELM] Weapon %d is now offline.",weapon);
	    return;
	  }
	  else {
	    VMnotify(player,"the weapon is already offline.");
	    return;
	  }
	    
	  break;
	case 'N':
	case 'n':
	  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,WEAPONLINE)) {
	    VMnotify(player,"That weapon is already online.");
	    return;
	  }
	  else {
	    VMSpace[ship].VMPerm.Helm.Systems[weapon].flags = setflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,WEAPONLINE);
	    VMnotify(player,"[HELM] Weapon %d is now online.",weapon);
	    return;
	  }
	  break;
	default:
	  VMnotify(player,"I cant tell what you want me to do.");
	  return;
	}
	break;
      default:
	VMnotify(player,"I cant figure out what you want me to do.");
	return;
      }
      
   }
    else {
      VMnotify(player,"That is not a weapon!");
      return;
    }
  }
  else {
    VMnotify(player,"That system does not exist!");
  }


}


void VMHelmGunPort(int ship,
	      int player,
	      int weapon,
	      char *what
	      ) {
  VMnotify(1,"GP: ship: %d player: %d weapon: %d what: %s",ship,player,weapon,what);
  /* first lets check weapon bounds */
  if (weapon < 0 || weapon > HSystems) {
    VMnotify(player,"That system does not exist!");
    return;
  }
  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,EXISTS)) {
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,GUNFLAG)) {
      if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,HASGUNPORT)) {
    /* now after all that shit is taken care of lets get down to business */
	if (strlen(what) > 6) {
	  VMnotify(player,"usage: gunport <number> [open|close]");
	  return;
	}
	if (!strcmp(what,"open")) {
	  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,GUNPORT)) {
	    VMnotify(player,"That port is already open.");
	    return;
	  }
	  else {
	   VMSpace[ship].VMPerm.Helm.Systems[weapon].flags = setflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,GUNPORT);
	   VMnotify(player,"[HELM] GunPort on gun %d is now open.",weapon);
	   return;
	  }
	}
	else if (!strcmp(what,"close")) {
	  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,GUNPORT)) {
	    VMSpace[ship].VMPerm.Helm.Systems[weapon].flags = clearflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,GUNPORT);
	    VMnotify(player,"[HELM] GunPort on gun %d is now closed.",weapon);

	  }
	  else {
	    VMnotify(player,"Already closed.");
	  }
	}
	else {
	  VMnotify(player,"usage: gunport <num> [open|close]");
	  return;
	}
	    
       
      }
      else {
	VMnotify(player,"That does not have a gunport.");
	return;
      }
      
    }
    else {
      VMnotify(player,"That is not a gun.");
      return;
    }
  }
  else {
    VMnotify(player,"That system does not exist.");
    return;
  }
    
}


void VMHelmLockWeapon(int ship,
		 int player,
		 int target,
		 int nweapon
		 ) {
  int time;
  int weapon;
  weapon = nweapon;
  if (target < 0 || target > VMCurrentMax) {
    VMnotify(player,"Invalid target.");
    return;
  } 
  /* we need to check errors first */
  /* big range check so we dont go in bad memory areas */
  if (weapon == -1) {
    VMnotify(player,"I don't understand what weapon you mean.");
    return;
  }
  if (weapon < 0 || weapon > HSystems) {
    VMnotify(player,"That system does not exist!");
    return;
  }
  /* now check to see if it exists */
  if ( checkflag(VMSpace[target].VMPerm.Flags,PLANET) || checkflag(VMSpace[target].VMPerm.Flags,BEACON) || checkflag(VMSpace[target].VMPerm.Flags,JUMPGATE) || checkflag(VMSpace[target].VMPerm.Flags,SUN)
) {
    if (player != 35) {
    VMnotify(player,"You can not lock that kind of thing for now!");
    return;
    }
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,EXISTS)) {
    VMnotify(player,"That system does not exist!");
    return;
  }
  /* now check to see if it is a weapon */
  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,NONWEAP)) {
    VMnotify(player,"That system is not a weapon.");
    return;
  }
  /* now check to see if it's online */
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,WEAPONLINE)) {
    VMnotify(player,"That weapon is not online.");
    return;
  }

  if (!checkflag(VMSpace[target].VMPerm.Flags,FIGHTER)&&checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,FIGHTERONLY)) {
    VMnotify(player,"That weapon can only track fighter-sized targets.");
    return;
  }

  if (checkflag(VMSpace[target].VMPerm.Flags,FIGHTER)&&!checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,ANTIFIGHTER)) {
    VMnotify(player,"That weapon can not track fighter-sized targets.");
    return;
  }

  if (ContactMatrix[ship][target] > 0) {
    /* if we make it in here our entry in contact matrix should be non zero */
    time = random() % 200;
    time = (time / ContactMatrix[ship][target]) + 1;
    VMnotify(1,"Queuing Weapon lock command for ship %d time: %d",ship,time);
    VMnotify(player,"Locking Weapon %d onto target %d.",weapon,target);
    VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"****WARNING**** Ship %d is attempting to lock weapons.",ship); 
    AddWEnt(VMWeaponQ,time,VMLOCKEVENT,ship,weapon,target,0.0,0.0);
/* Notify Ships ADS if it is active*/
    ADS_NotifyLock(target,ship);
  }
  else
    VMnotify(player,"There's no way in hell I'll let you lock onto that.");
  
}




void VMHelmFireWeapon(int ship,
		      int player,
		      int weapon
		      ) {

  double rng;
  int mrng,i;
  int sck,diff,rnd,damage,hit,target;
  int arc,sarc;
  int tm;
  if (weapon < 0 || weapon > HSystems) {
    VMnotify(player,"That system does not exist!");
    return;
  }
  /* now check to see if it exists */
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,EXISTS)) {
    VMnotify(player,"That system does not exist!");
    return;
  }
  /* now check to see if it is a weapon */
  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,NONWEAP)) {
    VMnotify(player,"That system is not a weapon.");
    return;
  }
  /* now check to see if it's online */
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,WEAPONLINE)) {
    VMnotify(player,"That weapon is not online.");
    return;
  }

  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,NOLOCK)) {
 
    if (VMSpace[ship].VMPerm.Helm.Systems[weapon].tistat2 == 0) {
      VMnotify(player,"That weapon must be locked before firing.");
      return;
    }
  }
  target = VMSpace[ship].VMPerm.Helm.Systems[weapon].tistat2;
  
  if (!(VMGetArc(target,ship) & VMSpace[ship].VMPerm.Helm.Systems[weapon].istat2)) {
    VMnotify(player,"Weapon is not in proper firing arc to hit target."); 
    /*VMnotify(player,"Weapon is not in proper firing arc to hit target. %d!=%d", VMGetArc(target,ship),VMSpace[ship].VMPerm.Helm.Systems[weapon].istat2);*/
    return;
  }
  /* at this point lets see if we hit */
  /* we need distance so lets put it in a variable */
  mrng = VMSpace[ship].VMPerm.Helm.Systems[weapon].istat1;
  rng = VMRange(ship,target);
  if (checkflag(VMSpace[target].VMPerm.Flags,DEAD)) {
    VMnotify(player,"That ship is dead. Weapon unlocked.");
    VMSpace[ship].VMPerm.Helm.Systems[weapon].tistat2 = 0;
    return;
  }
  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,TUBEFLAG)) {
    VMFireMissleTube(player,weapon,ship,target,rng);
    return;
  }
  else {
    VMFireBeamWeapon(player,weapon,ship,target,rng);
  }
}

void VMHelmStatus(int player,
	     int ship
	     )
{ 
  int i,j,k;
  int per;
  int power,arcs;
  char GunType[32];
  char arcbuf[9]; 
  k = 1;
  VMnotify(player,"-----Helm Status-------------------------------------------------------------");
  /* first thing we wanna do is go thru and find all non gun systems */
  for (i = 0; i < HSystems; i++) {
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,EXISTS) && checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,POWERABLE) && checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,NONWEAP)) {
      VMnotify(player,"System: %s %%Power: %1f%% Act Power: %d",VMSpace[ship].VMPerm.Helm.Systems[i].name,VMSpace[ship].VMPerm.Helm.Systems[i].tdstat0 * 100,VMSpace[ship].VMPerm.Helm.Systems[i].tistat0);
    }
  }
  /* now guns */
  VMnotify(player,"-----Guns--------------------------------------------------------------------");
  for (i = 0,k = 1; i < HSystems; i++) { /* i is system number, k is gun number */ 
    bzero(GunType,32);
    if (VMWeaponErrorCheck(ship,player,i,LGUN,1)) { /* whee, if we make it here its really a gun */

      /*arcs = VMSpace[ship].VMPerm.Helm.Systems[i].istat2;*/
      for (j = 0;j <= 8;j++)
	arcbuf[j] = 0;
      if ((VMSpace[ship].VMPerm.Helm.Systems[i].istat2 & FORE)) {
	strcat(arcbuf,"F");
      }
      if ((VMSpace[ship].VMPerm.Helm.Systems[i].istat2 & AFT)) { 
	strcat(arcbuf,"A");
      }
      if ((VMSpace[ship].VMPerm.Helm.Systems[i].istat2 & PORT)) { 
	strcat(arcbuf,"P");
      }
      if ((VMSpace[ship].VMPerm.Helm.Systems[i].istat2 & STARB)) { 
	strcat(arcbuf,"S");
      }
      if ((VMSpace[ship].VMPerm.Helm.Systems[i].istat2 & TOP)) { 
	strcat(arcbuf,"D");
      }
      if (VMSpace[ship].VMPerm.Helm.Systems[i].istat2 & BOTTOM) { 
	strcat(arcbuf,"V");
      }
      if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,HASGUNPORT)) { 
	if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,STASIS)) {
	  strcpy(GunType,"Stasis ");
	}
	else {
	  strcpy(GunType,"Laser ");
	}

	VMnotify(player,"Gun: %2d(Sys %d) Chrg: %3d  State: %3s Gunport: %s Arcs: %6s  Type: %s   Locked: %d",k,i,VMSpace[ship].VMPerm.Helm.Systems[i].tistat1,GetState(VMSpace[ship].VMPerm.Helm.Systems[i].flags),GetPState(VMSpace[ship].VMPerm.Helm.Systems[i].flags),arcbuf,GunType,VMSpace[ship].VMPerm.Helm.Systems[i].tistat2);
      }
      else {
	if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,STASIS)) {
	  strcpy(GunType,"Stasis");
	}
	else {
	  strcpy(GunType,"Laser ");
      }
      
	VMnotify(player,"Gun: %2d(Sys %d) Chrg: %3d State: %3s Arcs: %6s  Type: %s   Locked: %d",k,i,VMSpace[ship].VMPerm.Helm.Systems[i].tistat1,GetState(VMSpace[ship].VMPerm.Helm.Systems[i].flags),arcbuf,GunType,VMSpace[ship].VMPerm.Helm.Systems[i].tistat2);
      }
      k++;
    }
  }
  VMnotify(player,"-----System Status-----------------------------------------------------------");
  VMnotify(player,"Sys #  System Name\t\t\tStatus");
  for (i = 0; i < HSystems;i++) {
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,EXISTS)) {
      if (VMSpace[ship].VMPerm.Helm.Systems[i].mintegrity != 0) {
	per = (VMSpace[ship].VMPerm.Helm.Systems[i].integrity / VMSpace[ship].VMPerm.Helm.Systems[i].mintegrity) * 100;
      }
else
	per = -1;
      VMnotify(player,"%2d) %28s\t %2.2f/%2.2f (%d%%)",i,VMSpace[ship].VMPerm.Helm.Systems[i].name,VMSpace[ship].VMPerm.Helm.Systems[i].integrity,VMSpace[ship].VMPerm.Helm.Systems[i].mintegrity,per);
    }
  }

}
      
  


int VMWeaponErrorCheck(int ship,
		   int player,
		   int system, 
		   int level,
		   int quiet
		   ) {
  /* all levels need the following check */
  if (system < 0 || system > HSystems) {
    if (!quiet)
      VMnotify(player,"That system does not exist.");
    return 0;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,EXISTS)) {
    if (!quiet)
      VMnotify(player,"That system does not exist!");
return 0;
  }
  if (level == LNOWEAPON)
    return 1;
  /* now check to see if it is a weapon */
  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,NONWEAP)) {
    if (!quiet)
      VMnotify(player,"That system is not a weapon.");
    return 0;
  }
  if (level == LGUN || level == LGUNPRT)
    if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,GUNFLAG)) {
      if (!quiet)   
	VMnotify(player,"That system is not a gun.");
      return 0;
    }
  if (level == LMISS)
    if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,TUBEFLAG)) {
      if (!quiet)   
	VMnotify(player,"That system is not a missile weapon.");
      return 0;
    }
  if (level == LGUNPRT)
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,HASGUNPORT))
      if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,GUNPORT)) {
      if (!quiet)   
	VMnotify(player,"That gun's port is closed.");
      return 0;
      }

  return 1;
}



void VMMissWarningVerb(int player,
int ship,
		  int missle,
		  char what[]
		  ) {
  
  if (missle < 0 || missle > HSystems) {
    VMnotify(player,"That system does not exist.");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[missle].flags,EXISTS)) {
    VMnotify(player    ,"That system does not exist.");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[missle].flags,TUBEFLAG)) {
    VMnotify(player,"That system is not a missile.");
    return;
  }


}


int 
VMMissleDoor(int player,
	     int ship,
	     int tube,
	     int flag
	     ) {
  int blah;
  if (tube < 0 || tube > HSystems) {
    VMnotify(player,"That system does not exist");
    return 0;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,EXISTS)) {
    VMnotify(player    ,"That system does not exist");
    return 0;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,TUBEFLAG)) {
    VMnotify(player,"That system is not a launch tube.");
    return 0;
  }
  if ( VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 == LOADING) {
    VMnotify(player,"You cannot open or close the missile door while loading,unloading or dumping.");
    return 0;
  }
  if (VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 == OPENING) {
    VMnotify(player,"Its in the process of opening or closing. Please wait til its done.");
    return 0;
  }
  if (flag) {
    /* open door */
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,DOOROPEN)) {
      VMnotify(player,"The outer door on that tube is already open.");
      return 0;
    }
    if (VMSpace[ship].VMPerm.Helm.Systems[tube].status < .50) {
      VMnotify(player,"That missile is too damaged to be opened now.");
      return 0;
    }
    if (VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 < LOADED) {
      VMnotify(player,"You cannot open the outer doors of the launch tube now.");
      return 0;
      }
    AddWEnt(VMWeaponQ,VMSpace[ship].VMPerm.Helm.Systems[tube].istat6,VMDOOREV,ship,tube,flag,0.0,0.0);
    VMSpace[ship].VMPerm.Helm.Systems[tube].tistat3 =VMSpace[ship].VMPerm.Helm.Systems[tube].istat1;
    VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 = OPENING;
    VMnotify(player,"Opening the outer door on tube %d. This will be done in %d seconds.",tube,VMSpace[ship].VMPerm.Helm.Systems[tube].istat6);
    
    return VMSpace[ship].VMPerm.Helm.Systems[tube].istat6 ;
  }
  else {
    /* close door */
    if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,DOOROPEN)) {
      VMnotify(player,"The outer door on that tube is already closed.");
      return;
    }
    if (VMSpace[ship].VMPerm.Helm.Systems[tube].status < .50) {
      VMnotify(player,"That missile is too damaged to be opened now.");
      return;
    }
    AddWEnt(VMWeaponQ,VMSpace[ship].VMPerm.Helm.Systems[tube].istat6,VMDOOREV,ship,tube,flag,0.0,0.0);
    VMSpace[ship].VMPerm.Helm.Systems[tube].tistat3 =VMSpace[ship].VMPerm.Helm.Systems[tube].istat1;
    VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 = OPENING;
    VMnotify(player,"Closing the outer door on tube %d.",tube);
    return VMSpace[ship].VMPerm.Helm.Systems[tube].istat6;
  }
}

void VMLoadMissle(int player,
	     int ship,
	     int tube,
	     int missle
	     ) {
  int loadtime;
  if (tube < 0 || tube > HSystems) {
    VMnotify(player,"That system does not exist");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,EXISTS)) {
    VMnotify(player    ,"That system does not exist");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,TUBEFLAG)) {
    VMnotify(player,"That system is not a launch tube.");
    return;
  }  
  if (missle < 0 || missle > NUMMISS) {
    VMnotify(player,"That is not a valid missile.");
    return;
  }
  if (VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 != EMPTY) {
    VMnotify(player,"That tube must be completely empty before you can load it with anything.");
    return;
  }
  if (VMSpace[ship].VMPerm.Helm.Missles[missle] <= 0) { 
    VMnotify(player,"You are out of that kind of missile.");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,DOOROPEN)) {
    
    loadtime = AllMissles[missle].loadtime;
    /* add a skill check to either reduce or increase loadtime */
    AddWEnt(VMWeaponQ,loadtime,VMLOADMISSLE,ship,tube,missle,0.0,0.0);
    VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 = LOADING;
    VMnotify(player,"Loading tube %d with %s(%d remaining of that missile after load.).",tube,AllMissles[missle].name,VMSpace[ship].VMPerm.Helm.Missles[missle] -1);
  }
  else {
    VMnotify(player,"The outer door on this tube must be closed to load or unload missiles.");
  }

  
}

void VMMissleStatus(int player, 
	       int ship
	       ) {
  int i;
  int loaded = 0;
  int locked = 0;
  char state[10];
  char door[10];
  char arcbuf[9];
  t_sph pnt;
  t_MEnt *tmp;
  bzero(state,10);
  bzero(door,10);
  VMnotify(player,"-----Missile Status----------------------------------------------------------");
    for (i = 1; i <= NUMMISS; i++) { 
 	if(VMSpace[ship].VMPerm.Helm.Missles[i]>0)
    VMnotify(player,"Missile: %25s(%3d)        Inventory: %d",AllMissles[i].name,i,VMSpace[ship].VMPerm.Helm.Missles[i]);
  }
  VMnotify(player,"-----Event Status----------------------------------------------------------");
  ResetMList(VMMissleQ);
  while (! AtEndMList(VMMissleQ)) { 
    tmp =(t_MEnt *) GetCurrMEnt(VMMissleQ);
    if (tmp->target == ship) {
#ifdef SPACEDEBUG
      VMnotify(35,"made it here.");
#endif
      pnt = VMxyz_to_sph(tmp->orgin);
      VMnotify(player,"Ship %d has a missile targetting you. Estimated missile Speed is %d, Estimated time of impact is %d second%s.",tmp->launcher,AllMissles[tmp->missle].speed,tmp->timer,tmp->timer > 1 ? "s" : "");
      VMnotify(player,"Orgin point is bearing: %2.2f elevation: %2.2f range: %2.2f",pnt.bearing,pnt.elevation,pnt.range);
    }
    if (tmp->launcher == ship) {
      VMnotify(player,"You have a missile targetting %d Estimated impact in %d second%s.",tmp->target,tmp->timer,tmp->timer > 1 ? "s" : "");
    }
    AdvanceMList(VMMissleQ);
  }

  VMnotify(player,"-----Interceptor Status----------------------------------------------------");
  for (i = 0; i < HSystems; i++) { 
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,EXISTS)) {
      if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,INTERCEPTOR)) {
      	if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,INTERON)) {
	  strcpy(state,"on");
	}
	else {
	  strcpy(state,"off");
	}
	bzero(arcbuf,9);
	if ((VMSpace[ship].VMPerm.Helm.Systems[i].istat2 & FORE)) {
	  strcat(arcbuf,"F");
	}
	if ((VMSpace[ship].VMPerm.Helm.Systems[i].istat2 & AFT)) { 
	  strcat(arcbuf,"A");
	}
	if ((VMSpace[ship].VMPerm.Helm.Systems[i].istat2 & PORT)) { 
	  strcat(arcbuf,"P");
	}
	if ((VMSpace[ship].VMPerm.Helm.Systems[i].istat2 & STARB)) { 
	  strcat(arcbuf,"S");
	}
	if ((VMSpace[ship].VMPerm.Helm.Systems[i].istat2 & TOP)) { 
	  strcat(arcbuf,"D");
	}
	if (VMSpace[ship].VMPerm.Helm.Systems[i].istat2 & BOTTOM) { 
	  strcat(arcbuf,"V");
	}

	VMnotify(player,"System: %d State: %s Power: %d Arcs: %s",i,state,VMSpace[ship].VMPerm.Helm.Systems[i].tistat0,arcbuf);
      }
    }
  }
      

  VMnotify(player,"-----Tube Status-----------------------------------------------------------");



  /* go thru the systems array and give a damage report */
  for (i = 0; i < HSystems; i++) { /* i is system number, k is gun number */ 
    /* error checks to see if this is really a tube */
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,EXISTS)) {
      if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,TUBEFLAG)) {
	if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,WEAPONLINE)) {
	  strcpy(state,"on");
	}
	else {
	  strcpy(state,"off");
	}
	if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,DOOROPEN)) {
	  strcpy(door,"open");
	}
	else {
	  strcpy(door,"closed");
	}
	if (VMSpace[ship].VMPerm.Helm.Systems[i].istat4 != 0) {
	  /* tube is loaded */ 
	  loaded = 1;
	}
	if (VMSpace[ship].VMPerm.Helm.Systems[i].tistat2 != 0) {
	  /* tube is loaded */ 
	  locked = 1;
	}
	VMnotify(player,"Tube: %3d         Door: %6s       State: %4s        Loaded with: %s \n                  Locked on:  %10s                Power: %d",i,door,state,(loaded ? AllMissles[VMSpace[ship].VMPerm.Helm.Systems[i].istat4].name : "nothing"), (!locked ? "nothing" : VMSpace[VMSpace[ship].VMPerm.Helm.Systems[i].tistat2].VMPerm.Name),  VMSpace[ship].VMPerm.Helm.Systems[i].tistat0);
      }
    }
  }
}	      
      


void VMArmMissle(int player,
	    int ship,
	    int tube
	    ) {
  if (tube < 0 || tube > HSystems) {
    VMnotify(player,"That system does not exist.");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,EXISTS)) {
    VMnotify(player,"That system does not exist");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,TUBEFLAG)) {
    VMnotify(player,"That system is not a tube.");
    return;
  }
  if (VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 == LOADED && VMSpace[ship].VMPerm.Helm.Systems[tube].tistat2 > 0 && VMSpace[ship].VMPerm.Helm.Systems[tube].istat4 != 0) {
    VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 = ARMED;
    VMnotify(player,"Tube %d is now ARMED.",tube);
  }
  else {
    VMnotify(player,"Tube %d must be completely loaded and targetted onto something in order to arm.",tube);
  }
}




void VMDumpMissle(int player,
	     int ship,
	     int tube
	     ) {
  int loadtime;
  int missle;
  if (tube < 0 || tube > HSystems) {
    VMnotify(player,"That system does not exist.");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,EXISTS)) {
    VMnotify(player    ,"That system does not exist");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,TUBEFLAG)) {
    VMnotify(player,"That system is not a launch tube.");
    return;
  }  
  if (VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 != LOADED) {
    VMnotify(player,"That tube must be completely loaded before you can unload it.");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,DOOROPEN)) {
    VMnotify(player,"The outer door of that missile tube must be opened in order for you to do that.");
    return;
  }  
  missle = VMSpace[ship].VMPerm.Helm.Systems[tube].istat4;
  loadtime = AllMissles[missle].loadtime;
  loadtime = loadtime / 4;
  if (loadtime <= 0) {
    loadtime = 1;
  }
  /* add a skill check to either reduce or increase loadtime */
  AddWEnt(VMWeaponQ,loadtime,VMLOADMISSLE,ship,tube,-2,0.0,0.0);
  VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 = LOADING;
}


void VMUnloadMissle(int player,
	       int ship,
	       int tube
	       ) {
  int loadtime;
  int missle;
  if (tube < 0 || tube > HSystems) {
    VMnotify(player,"That system does not exist");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,EXISTS)) {
    VMnotify(player    ,"That system does not exist");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,TUBEFLAG)) {
    VMnotify(player,"That system is not a launch tube.");
    return;
  }  
  if (VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 != LOADED) {
    VMnotify(player,"That tube must be completely loaded before you can unload it.");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,DOOROPEN)) {
    missle = VMSpace[ship].VMPerm.Helm.Systems[tube].istat4;
    loadtime = AllMissles[missle].loadtime;
    /* add a skill check to either reduce or increase loadtime */
    AddWEnt(VMWeaponQ,loadtime,VMLOADMISSLE,ship,tube,-1,0.0,0.0);
    VMSpace[ship].VMPerm.Helm.Systems[tube].istat1 = LOADING;
  }
  else {
    VMnotify(player,"The outer door on this tube must be closed to load or unload missiles.");
  }
}


void VMAddMissle(int player,
	    int ship,
	    int missle
	    ) {
  VMSpace[ship].VMPerm.Helm.Missles[missle]++;
  VMnotify(player,"Missile %d is now at %d missiles.",missle,VMSpace[ship].VMPerm.Helm.Missles[missle]);
}



void VMAutoloadTube(int player,
		    int ship,
		    int tube,
		    int missile
		    ) {
  if (tube < 0 || tube > HSystems) {
    VMnotify(player,"Invalid range given on tube number.");
    return;
  }

  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,EXISTS)) {
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[tube].flags,TUBEFLAG)) {
      if (missile > 0 && missile <= NUMMISS) {
	VMSpace[ship].VMPerm.Helm.Systems[tube].tistat4 = missile;
        VMnotify(player,"Missile tube %d is now autoloading missile %d",tube,missile);
      }
      else {
	VMnotify(player,"Invalid Missile");
      }
    }
    else {
      VMnotify(player,"Not a launch tube");
    }
  }
  else {
    VMnotify(player,"That System doesn't exist kid.");
  }
}

void VMArmorStatus(int player,
            int ship
            ) {
  int i;
  char astr[100];
  int per;

  for (i = 4 ; i <= 9; i++) {
    bzero(astr,100);
    if ( i == 4) {
      strcat(astr,"Fore");
    }
    else if ( i == 5 ) {
      strcat(astr,"Aft");
    }
    else if ( i == 6 ) {
      strcat(astr,"Port");
    }
    else if ( i == 7 ) {
      strcat(astr,"Starboard");
    }
    else if ( i == 8 ) {
      strcat(astr, "Dorsal");
    }
    else if ( i == 9 ) {
      strcat(astr,"Ventral");
    }

    if (VMSpace[ship].VMPerm.Eng.Systems[i].mintegrity > 0) {
      per = (int) VMSpace[ship].VMPerm.Eng.Systems[i].integrity / VMSpace[ship].VMPerm.Eng.Systems[i].mintegrity * 100;
    }
    else
      per = -1;
    astr[10] = 0;
    VMnotify(player,"Armor %s Status is %4.2f/%4.2f(%d%%)",astr,VMSpace[ship].VMPerm.Eng.Systems[i].integrity,VMSpace[ship].VMPerm.Eng.Systems[i].mintegrity,per
);
  }
}







void VMFireMissleTube(int player, int weapon,int ship,int target, double rng) {
  int tm;
  if (VMSpace[ship].VMPerm.Helm.Systems[weapon].istat1 == ARMED) {
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,DOOROPEN)) {
      t_xyz src,tgt;
      int time,missle;
      if (rng > VMSpace[ship].VMPerm.Helm.Systems[weapon].istat3) {
	VMnotify(player,"[HELM] The target you are firing on is out of range for this weapon.");
	return;
      }
      missle = VMSpace[ship].VMPerm.Helm.Systems[weapon].istat4;
      src = VMSpace[ship].VMPerm.Nav.XYZCoords;
      tgt = VMSpace[VMSpace[ship].VMPerm.Helm.Systems[weapon].tistat2].VMPerm.Nav.XYZCoords;
      time = rng / AllMissles[ VMSpace[ship].VMPerm.Helm.Systems[weapon].istat4].speed;
#ifdef SPACEDEBUG 
      VMnotify(35,"speed: %d Range: %2.2f",AllMissles[ VMSpace[ship].VMPerm.Helm.Systems[weapon].istat4].speed,rng);
      VMnotify(35,"time: %d",time); 
#endif
      if (time <= 0) 
	time = 1;
#ifdef SPACEDEBUG 
      VMnotify(35,"Amod is%d. istat0 is %d",AllMissles[missle].amod,VMSpace[ship].VMPerm.Helm.Systems[weapon].istat0);
#endif
      AddMEnt(VMMissleQ , time , ship , player , target , src , tgt , VMSpace[ship].VMPerm.Helm.Systems[weapon].istat4 , AllMissles[missle].amod + VMSpace[ship].VMPerm.Helm.Systems[weapon].istat0,AllMissles[missle].intel,AllMissles[missle].baseecm,AllMissles[missle].baseeccm);
      VMnotify(player,"Missile tube %d has launched. Target is %d. Estimated time of impact is %d seconds.",weapon,target,time);
      VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"***WARNING*** Contact %d has launched a missile at you. Estimated time of impact is %d.",ship,time);
      /* might as well mark the tube as empty. */
      VMSpace[ship].VMPerm.Helm.Systems[weapon].istat4 = 0;
      VMSpace[ship].VMPerm.Helm.Systems[weapon]
.istat1 = EMPTY;
      /* now we autoload the missle if it set to */
      if (VMSpace[ship].VMPerm.Helm.Systems[weapon].tistat4 != 0) {
	VMnotify(player,"Autloading tube %d",weapon);
	tm = VMMissleDoor(player,ship,weapon,0);
	AddWEnt(VMWeaponQ,tm+2,VMTAL,ship,weapon,player,0.0,0.0);
	
      }
       
	  
    }
	
    else {
      VMnotify(player,"The outer door for that tube must be open.");
    }
  }
  else {
    
    VMnotify(player,"That weapon must be armed.");
    return;
  }
}



/* HERE!!!!!! */
void 
VMFireBeamWeapon(
		      int player, 
		      int weapon,
		      int ship, 
		      int target, 
		      double rng
		      ) {
  /* variables*/
  int base; /* base to hit */
  int sck; /* skill bonus */
  int diff; /* range difficulty modifier */ 
  int ew; /* EW factor */
  int movf; /* movement factor */ 
  
  int roll; /* roll */
  int damage; /* damage caused */
  /* formula to calculate whether we "hit" - base = 50 + sck - diff - ew - movf
     IF base < roll then hit */
  diff = (int)  rng * VMSpace[ship].VMPerm.Helm.Systems[weapon].dstat2 * .026667;
  sck = VMSCheck(player,VMSpace[ship].VMPerm.Helm.Systems[weapon].skill,diff,HELM);
  
  if (sck == -5) {
    VMnotify(player, "You have NO idea how to do that.");
    return;
  }
  if (rng > VMSpace[ship].VMPerm.Helm.Systems[weapon].istat1) {
    VMnotify(player,"[HELM] The target you are firing on is out of range for \this weapon.");
    return;
  }
  
  if (VMSpace[ship].VMPerm.Helm.Systems[weapon].tistat1 < 5) {

    VMnotify(player,"[HELM] That gun does not have enough of a charge to fire.");
    return;
  }

ADS_NotifyFire(target,ship);
  /* if we make it this far we are in range, and we have enuff of a charge to fire, as well as we passed the skill check */
  /*since we almost always need damamge, and charge cleared, lets get it now */
  damage = VMSpace[ship].VMPerm.Helm.Systems[weapon].tistat1; 
  VMSpace[ship].VMPerm.Helm.Systems[weapon].tistat1 = 0;
  /* normal case */
  /* skills can only affect 50% */
  if (sck > 50) 
    sck = 50;
  
  base = 50 + sck - diff; /* generic base */ 
  roll = random() % 100; /* it doesn't hurt to roll at this point */ 
  
  /* at this point lets handle interceptors NOTE this is just a skeleton for now */ 
  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,ICPTABLE)) {
    if (VMHandleInterceptor(ship,target,base,roll) != 0) 
      return;
  }
    

  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,PLASMA)) {
        /* stasis weapons */
       /* clear charge regardless */
    if (roll < base) { 
      int mrng;
      int erng;
      int rdam;
      mrng = VMSpace[ship].VMPerm.Helm.Systems[weapon].istat1;
      if (VMSpace[ship].VMPerm.Helm.Systems[weapon].istat6 > 0)
	erng = VMSpace[ship].VMPerm.Helm.Systems[weapon].istat6;
      else
	erng = VMSpace[ship].VMPerm.Helm.Systems[weapon].istat1 / 2;
      if (rng > erng) {
	if ((mrng - erng) != 0)
	  rdam = damage - (rng - erng) * damage / (mrng - erng);
	else
	  rdam = damage;
      }
      else
	rdam = damage;
      if (rdam > damage)
	rdam = damage;

      if (rdam < 1)
	rdam = 1;
      /* hit */
      VMnotify(player,"[HELM] Firing %s %d. Shot hit.",VMSpace[ship].VMPerm.Helm.Systems[weapon].name,weapon);
      VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"Contact %d has fired upon you and hit.",ship);
/* mike=schmoo for(mixing up his own function */
      /*VMShipNotifyOfFire(ship,target,BEAM,10,50);*/
      VMShipNotifyOfFire(ship,target,10,50,BEAM,1);
      VMTakeDamage(target,rdam,3,VMGetArc(ship,target));
	}
    else {
      /* missed */
      VMnotify(player,"[HELM] Firing %s %d. Shot missed.",VMSpace[ship].VMPerm.Helm.Systems[weapon].name,weapon);
      VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"Contact %d has fired upon you and missed.",ship);
      /*VMShipNotifyOfFire(ship,target,BEAM,10,50);*/
      VMShipNotifyOfFire(ship,target,10,50,BEAM,0);
    }
  }
  else if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,STASIS)) {
    /* stasis weapons */
       /* clear charge regardless */
    if (roll < base) { 
      /* hit */
      VMnotify(player,"[HELM] Firing %s %d. Shot hit.",VMSpace[ship].VMPerm.Helm.Systems[weapon].name,weapon);
      VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"Contact %d has fired upon you and hit.",ship);
      VMShipNotifyOfFire(ship,target,10,50,BEAM,1);
      /*VMShipNotifyOfFire(ship,target,BEAM,10,50);*/
      VMTakeDamage(target,damage,2,VMGetArc(ship,target));
    }
    else {
      /* missed */
      VMnotify(player,"[HELM] Firing %s %d. Shot missed.",VMSpace[ship].VMPerm.Helm.Systems[weapon].name,weapon);
      VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"Contact %d has fired upon you and missed.",ship);
      VMShipNotifyOfFire(ship,target,10,50,BEAM,0);
      /*VMShipNotifyOfFire(ship,target,BEAM,10,50);*/
    }
  }
  else { /* all other weapons */
       /* clear charge regardless */
    if (roll < base) { 
      /* hit */
      VMnotify(player,"[HELM] Firing %s %d. Shot hit.",VMSpace[ship].VMPerm.Helm.Systems[weapon].name,weapon);
      VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"Contact %d has fired upon you and hit.",ship);
      VMShipNotifyOfFire(ship,target,10,50,BEAM,1);
      /*VMShipNotifyOfFire(ship,target,BEAM,10,50);*/
      VMTakeDamage(target,damage,1,VMGetArc(ship,target));
    }
    else {
      /* missed */
      VMnotify(player,"[HELM] Firing %s %d. Shot missed.",VMSpace[ship].VMPerm.Helm.Systems[weapon].name,weapon);
      VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"Contact %d has fired upon you and missed.",ship);
      VMShipNotifyOfFire(ship,target,10,50,BEAM,0);
      /*VMShipNotifyOfFire(ship,target,BEAM,10,50);*/
    }
  }

}
  


void VMHandleInterceptor(int ship,
			 int target,
			 int base,
			 int roll
			 ) {
  return 0;
}




void VMInterceptorState(int player,  int ship, int system,int flag) {
  int i;
  int found = 0;
  if (system < 0 || system > HSystems) {
    VMnotify(player,"That system does not exist!");
    return;
  }
  /* now check to see if it exists */
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,EXISTS)) {
    VMnotify(player,"That system does not exist!");
    return;
  }
  /* now check to see if it is a system */
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,INTERCEPTOR)) {
    VMnotify(player,"That system is not an interceptor.");
    return;
  }

  
  if (VMSpace[ship].VMPerm.Helm.Systems[system].tistat0 < VMSpace[ship].VMPerm.Helm.Systems[system].basenumber) {
    VMnotify(player,"that system needs more power.");
    return;
  }
  if (flag) {
    if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,INTERON)) {
    VMSpace[ship].VMPerm.Helm.Systems[system].flags = setflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,INTERON);
    VMSpace[ship].VMPerm.Helm.Systems[system].flags = setflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,INTERON); 
    VMnotify(player,"Interceptor set on line.");

    }
    else {
      VMnotify(player,"Interceptor system %d is already online.",system);
    }
  }
  else {
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,INTERON)) {
    VMSpace[ship].VMPerm.Helm.Systems[system].flags = clearflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,INTERON);
    for (i =0; i < HSystems; i++) {
      if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,EXISTS)) {
	if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,INTERCEPTOR)) {
	  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,INTERON)) {
	    found = 1;
	  }
	}
      }
    }
    if (!found) {
      VMSpace[ship].VMPerm.Helm.Systems[system].flags = clearflag(VMSpace[ship].VMPerm.Helm.Systems[system].flags,INTERON);
    }
    VMnotify(player,"Interceptor set off line.");
    }
    else {
      VMnotify(player,"Interceptor system %d is already offline.",system);
    }
  }
}

void VMSetFireGun(int player, int ship, int gun, char *state) {
  int type = 0;
  
  if (gun < 0 || gun > HSystems) {
    VMnotify(player,"Invalid gun.");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[gun].flags,EXISTS)) {
    VMnotify(player,"That system doesn't exist!");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[gun].flags,GUNFLAG)) {
    VMnotify(player,"That system is not a gun.");
    return;
  }

  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[gun].flags,ADV)) {
    VMnotify(player,"That gun is not capable of using such advanced effects.");
    return;
  }
  if (strlen(state) < 0 || strlen(state) > 10) {
    VMnotify(player,"Usage: setg <systemnum> NORMAL SWEEP POINT DISPER");
    return;
  }

  if (strcasecmp(state,"sweep") == 0) {
    if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[gun].flags,CSWEEP)) {
      VMnotify(player,"This gun cannot be set to fire sweep.");
      return;
    }
    type  = SSWEEP;
  }
  else if (strcasecmp(state,"normal") == 0) {
    type = SNORMAL;
  }
  else if (strcasecmp(state,"point") == 0) {
    if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[gun].flags,CPOINT)) {
      VMnotify(player,"This gun cannot be set to fire at a point.");
      return;
    }
    type = SPOINT;
  }
  else if (strcasecmp(state,"disper") == 0) {
    if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[gun].flags,CDISPER)) {
      VMnotify(player,"This gun cannot be set to fire at a point.");
      return;
    }
    type = SDISPER;
  }
  else {
    VMnotify(player,"Usage: setg <systemnum> NORMAL SWEEP POINT DISPER");
    return;
  }

  if (VMSpace[ship].VMPerm.Helm.Systems[gun].tistat3 == type) {
    /* Its already the way the user is asking us... hmm call him names and return i guess */
    VMnotify(player,"That weapon is already set %s!",state);
    return;
  }
  VMSpace[ship].VMPerm.Helm.Systems[gun].tistat3 = type;
  VMnotify(player,"[HELM] Weapon %s(%d) is now set to fire %s.",VMSpace[ship].VMPerm.Helm.Systems[gun].name,gun,state);
  /* be evil... clear all energy stored in the gun */
  VMSpace[ship].VMPerm.Helm.Systems[gun].tistat1 = 0;
}


/* 
void VMSetHeading(player,ShipNum,alp,bet,rol)
     int player,ShipNum;
     double alp,bet,rol;
{
  int i,speed;
  
  if(!CanAccessNav(ShipNum,player)) return;
  if(rol > 360 || rol < 0 || alp < 0 || alp > 360 || bet < 0 || bet > 360) {
    VMnotify(player,"Invalid Heading");
    return;
  }

  VMSpace[ShipNum].VMPerm.Nav.Des_alpha=alp;
  VMSpace[ShipNum].VMPerm.Nav.Des_beta=bet;
  VMSpace[ShipNum].VMPerm.Nav.Des_roll=rol;
  VMnotify(player,"[COMPUTER] Heading Set");

}
*/





void VMHelmLayMine(int ship,
		   int player,
		   int weapon
		   ) {

  t_xyz pos;
  int sector;
  int miss;
  int prng;
  int mrng;
  if (weapon < 0 || weapon > HSystems) {
    VMnotify(player,"That system does not exist!");
    return;
  }
  /* now check to see if it exists */
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,EXISTS)) {
    VMnotify(player,"That system does not exist!");
    return;
  }
  /* now check to see if it is a weapon */
  if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,NONWEAP)) {
    VMnotify(player,"That system is not a weapon.");
    return;
  }
  /* now check to see if it's online */
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,WEAPONLINE)) {
    VMnotify(player,"That weapon is not online.");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,TUBEFLAG)) {
    VMnotify(player,"You can only lay mines from launch tubes.");
    return;
  }

  /* figure out our current position */
  pos = VMSpace[ship].VMPerm.Nav.XYZCoords;
  sector = VMSpace[ship].VMTemp.TmpNav.Sector;

  /* Go ahead and add the mine to the minelist for the sector */

  /* paranoid check for list, if its bad, we have a problem, and something isn't getting done right */

  if (VMSectors[sector].VMMines == NULL ) {
    /* shit, null minelist */
    VMnotify(35,"NullMinelist!!");
    VMnotify(player,"ERROR: Null Minelist detected from lay subsystem, please @mail the following to Mike and Valdar: Null Minelist from VMHelmLayMine in sector %d",sector);
    return;
  }
  miss = VMSpace[ship].VMPerm.Helm.Systems[weapon].istat4;
  if (miss < 0 || miss > NUMMISS) {
    VMnotify(player,"There is nothing in that tube to lay.");
    return;
  }
  if (!checkflag(VMSpace[ship].VMPerm.Helm.Systems[weapon].flags,DOOROPEN)) {
    VMnotify(player,"You must first open the outer door on that tube.");
    return;
  }
  /* determine proximity explosion from missles range */
  if (checkflag(AllMissles[miss].flags,MINE)) {
    prng = AllMissles[miss].maxrange;
  }
  else
    prng = 5;
  /* max output range is power * (if MINE then 1, else .5) *5 */
  if (checkflag(AllMissles[miss].flags,MINE)) {

    mrng = VMGetMineMaxRange(miss);
  }
  else 
    mrng = VMGetMineMaxRange(miss) / 2;

  if (VMSpace[ship].VMTemp.TmpHelm.VMMines == NULL) {
    VMSpace[ship].VMTemp.TmpHelm.VMMines = (t_MineList *)  InitMineList();
  }

  AddMineEnt(VMSpace[ship].VMTemp.TmpHelm.VMMines,MasterMineCnt++,ship,player,pos,miss,prng,mrng,ALL);
  AddMineEnt(VMSectors[sector].VMMines,MasterMineCnt,ship,player,pos,miss,prng,mrng,ALL);
  VMnotify(player,"Mine launched.");
  
}


void VMHelmShowShipMineList(int player, 
			    int ship
			    ) {

  int size; 
  t_MineEnt *tmp;
  if (VMSpace[ship].VMTemp.TmpHelm.VMMines == NULL) {
    VMnotify(player,"No mines on sensors.");
    return;
  }
  size = GetMineListSize(VMSpace[ship].VMTemp.TmpHelm.VMMines);

  if (size > 0) {
    VMnotify(player,"%d mines on sensors:",size);
    ResetMineList(VMSpace[ship].VMTemp.TmpHelm.VMMines);
    while (!AtEndMineList(VMSpace[ship].VMTemp.TmpHelm.VMMines)) {
      tmp = (t_MineEnt *) GetCurrMineEnt(VMSpace[ship].VMTemp.TmpHelm.VMMines);
      VMnotify(player,"-----------------------------");
      VMnotify(player,"Mine Location:  X: %9.2f   Y: %9.2f   Z: %9.2f", tmp->loc.x,tmp->loc.y,tmp->loc.z);
      VMnotify(player,"Type: %s",(checkflag(tmp->flags,ALL) || checkflag(tmp->flags,TYPE)) ? AllMissles[tmp->missle].name : "???");
      VMnotify(player,"Launcher: %s",(checkflag(tmp->flags,ALL) || checkflag(tmp->flags,LAUNCHER)) ? VMSpace[tmp->launcher].VMPerm.Name : "???");
      VMnotify(player,"Range: %lf", VMdistance(VMSpace[ship].VMPerm.Nav.XYZCoords,tmp->loc));
      AdvanceMineList(VMSpace[ship].VMTemp.TmpHelm.VMMines);
    }
  }
  else {
    VMnotify(player,"No Mines on sensors.");
    KillMineList(VMSpace[ship].VMTemp.TmpHelm.VMMines);
    VMSpace[ship].VMTemp.TmpHelm.VMMines = NULL;
    return;
  }

}

void VMHelmSweepMine(int player,
		     int ship
		     ) {
  double assigned,needed,factor,distt,res;
  int sector;
  int rng,sz,diff,base,roll,size,skl;
  t_xyz pos,mpos;
  t_MineEnt *tmp;
  int det = 0;
  pos = VMSpace[ship].VMPerm.Nav.XYZCoords;
  sector = VMSpace[ship].VMTemp.TmpNav.Sector;
  
  if (VMSectors[sector].VMMines == NULL) {
    VMnotify(player,"Mine sweep complete. No mines detected.(1)");
    VMnotify(35,"Null minelist for sector(%d)",sector);
    return;
  }
  sz = GetMineListSize(VMSectors[sector].VMMines);  
  if (sz == 0) {
    VMnotify(player,"Mine sweep complete. No mines detected.(2)");
    return;
  }
  
  /* ok, go through the list */
  ResetMineList(VMSectors[sector].VMMines);
  while (!AtEndMineList(VMSectors[sector].VMMines)) {
    tmp =(t_MineEnt *) GetCurrMineEnt(VMSectors[sector].VMMines);
    mpos = tmp->loc;
    rng = (int) VMdistance(pos,mpos);
    size = AllMissles[tmp->missle].size;
    /* get the resolution */

    assigned=VMSpace[ship].VMPerm.Nav.Systems[0].tdstat0 * VMSpace[ship].VMTemp.TmpEng.PowerNav;
    needed=VMSpace[ship].VMPerm.Nav.Systems[0].dstat3;
    distt=VMSpace[ship].VMPerm.Nav.Systems[0].dstat1;
    if(needed==0) 
      res = 0;
    if(distt==0) 
      res = 0;
    factor=assigned / needed;
    if( factor > 1) 
      factor=1 + (assigned-needed) / (2 * needed);
    if( factor > 2) 
      factor=1 + (assigned-needed) / (3 * needed);
    if( factor > 3) 
      factor=1 + (assigned-needed) / (4 * needed);
    if(factor <= 0.0) 
      res = 0.0;
    factor=factor*(size/200.0+.5);
    res=factor*100.0 - (50.0 / distt) * VMdistance(pos,mpos);
    if(res > 100) 
      res=100.0;

    /* Now that we have the resolution */

    if (checkflag(VMSpace[ship].VMPerm.Nav.Systems[0].flags,MINESWEEPER)) {
      res = res - 20;
      if (res < 0) 
	res = 0;
    }
    else {
      res = res - 50;
      if (res < 0) 
	res = 0;
    }
    
    base = 5 + res;
    diff = abs(res - 100) + 1;
    skl = skill_check(player,VMSpace[ship].VMPerm.Nav.Systems[0].skill,diff);

    base += skl;

    roll = random() % 100;
    if (roll < base) {
      /* sucess */
      if ( ! MineInList(tmp->timer,VMSpace[ship].VMTemp.TmpHelm.VMMines)) {
	if (VMSpace[ship].VMTemp.TmpHelm.VMMines == NULL) 
	  VMSpace[ship].VMTemp.TmpHelm.VMMines =(t_MineList *) InitMineList();
	if (VMSpace[ship].VMTemp.TmpHelm.VMMines == NULL) {
	  VMnotify(35,"big problem in init from sweep, its still null");
	  return;
	}
	AddMineEnt(VMSpace[ship].VMTemp.TmpHelm.VMMines,tmp->timer,tmp->launcher,tmp->gunner,tmp->loc,tmp->missle,tmp->crng,tmp->mrng,tmp->flags);
	VMnotify(player,"Mine detected.");
	det = 10;
      }
    }

    AdvanceMineList(VMSectors[sector].VMMines);
    
  }
  if (det != 10) {
    VMnotify(player,"No mines detected.");
  }
}

int MineInList(int time, 
	       t_MineList *list) {
  t_MineEnt *tmp;
  if (list == NULL) {
    return 0;
  }
  
  if (GetMineListSize(list) > 0) {
    ResetMineList(list);
    while (!AtEndMineList(list)) {
      tmp = (t_MineEnt *) GetCurrMineEnt(list);
      if (tmp->timer == time) {
	/* we have a match */
	return 1;
      }
      AdvanceMineList(list);
    }
    return 0;
  }
  return 0;
}
