/* $Id: vmweaponq.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
#include "header.h"

static char *vmweaponqcrcs="$Id: vmweaponq.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";


void VMProcessWeaponQueue(t_WList q
		 ) {
  
  int i;
  int curship;
  ResetWList(q);
  while(!AtEndWList(q)) {
    t_WEnt *tmp;
    int event;
    int timer,ship,v1,v2;
    double d1,d2;
    tmp = (t_WEnt *) GetCurrWEnt(q);
    timer = tmp->timer;
    event = tmp->event;
    ship = tmp->ship;
    v1 = tmp->v1;
    v2 = tmp->v2;
    d1 = tmp->d1;
    d2 = tmp->d2;
    if (timer == 0) {
      RemoveWEnt(q);
      ProcessEvent(ship,event,v1,v2,d1,d2);
      tmp = NULL;
    }
    else
      tmp->timer = tmp->timer - 1;
    AdvanceWList(q);
  }
}

void ProcessEvent(int ship,
	     int event,
	     int v1,
	     int v2,
	     double d1,
	     double d2
	     ) {
  switch (event) {
  case VMLOCKEVENT:
    /* in this case v1 is the weapon, and v2 is the target */
    VMSpace[ship].VMPerm.Helm.Systems[v1].tistat2 = v2;
    /* now notify everyone */
    VMnotifyAll(VMSpace[ship].VMPerm.mainbridge,"*** Weapon %d is now locked onto Ship %s(%d). ***",v1,VMSpace[v2].VMPerm.Name,v2);
    /* for now we will print the name, talk to travis to see if we should always do this or not */
    VMnotifyAll(VMSpace[v2].VMPerm.mainbridge,"***WARNING*** Weapons lock from %s(%d) detected.",VMSpace[ship].VMPerm.Name,ship);
    break;
  case VMLOADMISSLE:
    /* in this case v1 is the tube, v2 is the missle type */
    if (v2 > 0) {
      if (VMSpace[ship].VMPerm.Helm.Missles[v2] > 0) {
	/* there are still some of those missles left */
	VMSpace[ship].VMPerm.Helm.Systems[v1].istat4 = v2;
	VMSpace[ship].VMPerm.Helm.Systems[v1].istat1 = LOADED;
	VMnotifymans(VMSpace[ship].VMTemp.TmpHelm.VMMan_List,"Tube %d is now loaded.",v1);
	VMSpace[ship].VMPerm.Helm.Missles[v2]--;
      }
      else {
	VMnotifymans(VMSpace[ship].VMTemp.TmpHelm.VMMan_List,"Warning! Unable to load tube %d. There are no more %s missles left.",v1,AllMissles[v2].name);
	
      }
    }
    else if (v2 == -1) {
      /* unload missle */
      VMSpace[ship].VMPerm.Helm.Missles[VMSpace[ship].VMPerm.Helm.Systems[v1].istat4]++;
      VMSpace[ship].VMPerm.Helm.Systems[v1].istat4 = 0;
      VMSpace[ship].VMPerm.Helm.Systems[v1].istat1 = EMPTY;
      VMnotifymans(VMSpace[ship].VMTemp.TmpHelm.VMMan_List,"Tube %d is now unloaded.",v1);
    }
    else if (v2 == -2) {
      /* dump missle */
      if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[v1].flags,DOOROPEN)) {
	VMSpace[ship].VMPerm.Helm.Systems[v1].istat4 = 0;
	VMSpace[ship].VMPerm.Helm.Systems[v1].istat1 = EMPTY;
	VMnotifymans(VMSpace[ship].VMTemp.TmpHelm.VMMan_List,"Tube %d has been dumped.",v1);
      }
      else {
	VMnotifymans(VMSpace[ship].VMTemp.TmpHelm.VMMan_List,"Warning! Tube %d cannot be dumped. Outer Door is closed.");
      }

    }
    break;
  case VMDOOREV:
    if (v2) {
    VMSpace[ship].VMPerm.Helm.Systems[v1].istat1 = VMSpace[ship].VMPerm.Helm.Systems[v1].tistat3;
    VMSpace[ship].VMPerm.Helm.Systems[v1].flags = setflag(VMSpace[ship].VMPerm.Helm.Systems[v1].flags,DOOROPEN);
    VMnotifymans(VMSpace[ship].VMTemp.TmpHelm.VMMan_List,"The outer door on tube %d is now open.",v1);
    }
    else {
      VMSpace[ship].VMPerm.Helm.Systems[v1].istat1 = VMSpace[ship].VMPerm.Helm.Systems[v1].tistat3;
      VMSpace[ship].VMPerm.Helm.Systems[v1].flags = clearflag(VMSpace[ship].VMPerm.Helm.Systems[v1].flags,DOOROPEN);
      VMnotifymans(VMSpace[ship].VMTemp.TmpHelm.VMMan_List,"The outer door on tube %d is now closed.",v1);
    }
    break;
  case VMTAL: 
    if (VMSpace[ship].VMPerm.Helm.Systems[v1].tistat4 != 0) {
      VMLoadMissle(v2,ship,v1,VMSpace[ship].VMPerm.Helm.Systems[v1].tistat4);
    }
    break;
  }
}






void VMProcessMissleQueue(t_MList *q
			  ) {
  int i;
  i=0;
  ResetMList(q);
  while(!AtEndMList(q) && i < GetMListSize(q)) {
    t_MEnt *tmp;
    tmp = (t_MEnt *) GetCurrMEnt(q);
    if (tmp->timer == 0) {
/*	VMnotify(2,"Timer=%d",tmp->timer);*/
#ifdef SPACEDEBUG
      VMnotify(35,"missle->acrrng is %d",tmp->acrrng);
#endif
      ProcessMEvent(tmp->launcher, tmp->target, tmp->gunner,tmp->orgin,tmp->destination,tmp->missle,tmp->acrrng,tmp->intel,tmp->ecm,tmp->eccm); 
      RemoveMEnt(q);
    }
    else {
/*	VMnotify(2,"Dec Timer=%d i=%d size=%d",tmp->timer,i,GetMListSize(q));*/
      tmp->timer = tmp->timer - 1;
	}
    AdvanceMList(q);
    i++;
  }
}
 

void ProcessMEvent(int launcher,
		   int target,
		   int gunner,
		   t_xyz orgin,
		   t_xyz destination,
		   int missle, 
		   int acrrng,
		   int intel,
		   int ecm,
		   int eccm
	     ) {

  double range,orange;
  int tecm,teccm;
  int hit,roll,dskill,lskill;

  lskill = 0;
  dskill = 0;
  tecm = 0;
  teccm = 0;
  /* tim for a reunion, where's our target now???? */
  orange = VMdistance(orgin,VMSpace[target].VMPerm.Nav.XYZCoords);
  range = VMdistance(destination,VMSpace[target].VMPerm.Nav.XYZCoords);
#ifdef SPACEDEBUG
  VMnotify(35,"range: %2.2f acrrng: %d",range,acrrng);
#endif
  if (orange > AllMissles[missle].maxrange) {
    /* missile has run out of gas */
    VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"Missle from %d has missed.",launcher);
    VMnotifymans(VMSpace[launcher].VMTemp.TmpHelm.VMMan_List,"The missle you launced on %d has missed. It has fallen short.",target);
      VMShipNotifyOfFire(launcher,target,10,50,BEAM,0);
    return;
  }
  /*if (checkflag(AllMissles[missle].flags,AOE)) {*/
    /* area of effect... this changes all the rules */
    
  if (range > acrrng) { /* looks like our friend has gotten outta the way??!? */

#ifdef SPACEDEBUG
    VMnotify(35,"Im here");
#endif
    VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"Missle from %d has missed.",launcher);
      VMShipNotifyOfFire(launcher,target,10,50,BEAM,0);
    VMnotifymans(VMSpace[launcher].VMTemp.TmpHelm.VMMan_List,"The missle you launced on %d has missed.",target);
    return;
  }

  /* well we could have hit... */
  
  if (checkflag(VMSpace[target].VMPerm.Flags,EWON)) { /* ew systems are online */
    /* for now we'll only use the launcher vs the target. Maybe we'll add in other later. */
    tecm = VMSpace[target].VMPerm.Helm.ecm;
  }
  else { 
    tecm = 0;
  }
  if (checkflag(VMSpace[launcher].VMPerm.Flags,EWON)) { /* ew systems are online */
    /* for now we'll only use the launcher vs the target. Maybe we'll add in other later. */
    teccm = VMSpace[launcher].VMPerm.Helm.eccm + eccm; 
  }
  else { 
    teccm = eccm;
  }
#ifdef SPACEDEBUG
  VMnotify(35,"tecm: %d teccm: %d",tecm,teccm);
#endif
  if (checkflag(VMSpace[target].VMPerm.Flags,INTERON)) { /* interceptors are online */
    if (VMHandledInterceptor(target,launcher,missle,teccm,tecm,destination)) {
      return;
    }
  }
  /* skill check should be added to hit, and yes i want it this way.  Hit is primarily based on the above  NOT skill difficulty. */
  /* for skill check. lskill is the skill modifier from the launcher. dskill is the defender skill */
  hit = 50 + teccm - tecm + (intel - 50) + lskill - dskill;
  
  roll = random() % 100;
  roll++;
#ifdef SPACEDEBUG
  VMnotify(35,"hit: %d roll: %d",hit,roll);
#endif
  if (roll < hit) { /* we hit */  
    if (!checkflag(VMSpace[target].VMPerm.Flags,DEAD)) {
      VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"Missle from %d has hit.",launcher);
      VMnotifymans(VMSpace[launcher].VMTemp.TmpHelm.VMMan_List,"The missle you launced on %d has hit.",target);
/* I know this isnt beam */
      VMShipNotifyOfFire(launcher,target,10,50,BEAM,1);
      if (checkflag(AllMissles[missle].flags,STASIS)) {
	VMTakeDamage(target,AllMissles[missle].damage,2,VMGetArc(launcher,target));
      }

      else {
	VMTakeDamage(target,AllMissles[missle].damage,1,VMGetArc(launcher,target));
  if (checkflag(AllMissles[missle].flags,AOE)) VMTakeAOEDamage(target,missle);
      }
    }
    else {
      VMnotifymans(VMSpace[launcher].VMTemp.TmpHelm.VMMan_List,"The ship your missle was targetting has been destroyed.");
    }

  }
  else {
    if (!checkflag(VMSpace[target].VMPerm.Flags,DEAD)) {
      VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"Missle from %d has missed.",launcher);
      VMShipNotifyOfFire(launcher,target,10,50,BEAM,0);
      VMnotifymans(VMSpace[launcher].VMTemp.TmpHelm.VMMan_List,"The missle you launced on %d has missed.",target);
    }
    else {
      VMnotifymans(VMSpace[launcher].VMTemp.TmpHelm.VMMan_List,"The ship your missle was targetting has been destroyed.");
    }
  }


  /*}*/
}

  

int VMHandledInterceptor(int target, int launcher, int missle, int eccm, int ecm, t_xyz dest ) {
  int i;
  int base = 0;
  int itecm = 0;
  int iteccm = 0;
  int mtecm = 0;
  int mteccm = 0;
  int roll = 0;
  
  /* first thing we need to do is check to see if the ship we have has any interceptors in the proper arc */
  for (i = 0; i < HSystems; i++) {
    if(checkflag(VMSpace[target].VMPerm.Helm.Systems[i].flags,EXISTS)) {
      if(checkflag(VMSpace[target].VMPerm.Helm.Systems[i].flags,INTERCEPTOR)) {
	if(checkflag(VMSpace[target].VMPerm.Helm.Systems[i].flags,INTERON)) {
	  if(!checkflag(VMSpace[target].VMPerm.Helm.Systems[i].flags,FIRED)) {
	    if (VMGetArc2(dest,target) & VMSpace[target].VMPerm.Helm.Systems[i].istat2) {
	      
	      base = 50 + eccm - ecm +  VMSpace[target].VMPerm.Helm.Systems[i].istat1 - AllMissles[missle].intel;
	      roll = random() % 100;
	      roll++;
#ifdef SPACEDEBUG
	      VMnotify(35,"INT: base: %d roll %d",base,roll);
#endif
	      if (roll < base) {
		VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"Contact %d's missile explodes in a flash of light.",launcher);
		VMnotifyAll(VMSpace[launcher].VMPerm.mainbridge,"Your missile explodes in a flash of light from Contact %d's interceptor battery.",target);
		VMSpace[target].VMPerm.Helm.Systems[i].flags =   setflag(VMSpace[target].VMPerm.Helm.Systems[i].flags,FIRED);
		return 1;
	      }
	      else{
		VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"Your interceptor misses contact %d's missile.",launcher);
		VMnotifyAll(VMSpace[launcher].VMPerm.mainbridge,"Contact %d attempts to destroy your missile with an interceptor, but misses.",target);
		VMSpace[target].VMPerm.Helm.Systems[i].flags =   setflag(VMSpace[target].VMPerm.Helm.Systems[i].flags,FIRED); 
	      }

	    }
	    else {
	      VMnotifymans(VMSpace[target].VMTemp.TmpHelm.VMMan_List,"Interceptor %d is not in proper firing arc to attack missile.",i);
	    }

	  }
	  else {
#ifdef SPACEDEBUG
	    VMnotify(35,"Interceptor already fired.");
#endif
	  }
	}
      }
    }
  }
  return 0;
}














