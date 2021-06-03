/* $Id: vmdamrep.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
/* vmdamrep.c */
/* By Mike Wenk */
#include "header.h"
static char *rcsvmdamrepc="$Id: vmdamrep.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
int
VMTakeAOEDamage(int origin,
		int missle
		) {

int tmpsector,dmg,m,b,arc;
double dist;
t_SpaceEnt *tmp;


tmpsector=VMSpace[origin].VMTemp.TmpNav.Sector;
ResetSpaceList(VMSectors[tmpsector].VMSectorShips);

while(!AtEndSpaceList(VMSectors[tmpsector].VMSectorShips)) {
	tmp=GetCurrSpaceEnt(VMSectors[tmpsector].VMSectorShips);
	if(tmp==NULL) break;
	if(tmp->VMShip==origin) continue;

	dist=VMdistance(VMSpace[origin].VMPerm.Nav.XYZCoords, VMSpace[tmp->VMShip].VMPerm.Nav.XYZCoords);
	m=AllMissles[missle].slope;
	b=AllMissles[missle].basedmg;
	dmg=m*dist+b;
	if(dmg <= 0 ) continue;

	arc=VMGetArc(origin,tmp->VMShip);
	VMnotifymans(VMSpace[tmp->VMShip].VMTemp.TmpHelm.VMMan_List,"Explosion damage has been done to this ship from a missile hitting ship %d",origin);
	VMTakeDamage(tmp->VMShip,dmg,1,arc);
	AdvanceSpaceList(VMSectors[tmpsector].VMSectorShips);
	}

}

int
VMTakeDamage(int target,
	     int damage,
	     int hit,
	     int arc
	     ) {
  int armor,i;
  int inmod,perin,pain;
  double mymod;
  int through,after,armdam;
  int numhits,remaining,reduction;
  double hulldam;
  double sysdam;
  int odam;
  int roll;
  char astr[100];
  sysdam = 0.0;
  through = 0;
  after = 0;
  armdam = 0;
  hulldam = 0;
  roll = 0;
  if ( checkflag(VMSpace[target].VMPerm.Flags,BASE) && !checkflag(VMSpace[target].VMPerm.Flags,VMMORTAL)
) { return(0); }

  if (checkflag(VMSpace[target].VMPerm.Flags,VORLON)) {
    odam = damage;
    damage = (int) damage / 100;
    VMnotify(35,"detected vorlon ship %d. odam: %d newdam: %d",target,odam,damage);
  }



  switch(hit) {
  case 1:
    armor = VMArc2Armor(arc);
    bzero(astr,100);
    if ( armor == 4) {
      strcat(astr,"Fore");
    }
    else if ( armor == 5 ) {
      strcat(astr,"Aft");
    }
    else if ( armor == 6 ) {
      strcat(astr,"Port");
    }
    else if ( armor == 7 ) {
      strcat(astr,"Starboard");
    }
    else if ( armor == 8 ) {
      strcat(astr, "Ventral");
    }
    else if ( armor == 9 ) {
      strcat(astr,"Dorsal");
    }


    if ((checkflag(VMSpace[target].VMPerm.Eng.Systems[armor].flags,EXISTS) && checkflag(VMSpace[target].VMPerm.Eng.Systems[armor].flags,VMARMOR))) {
      /* looks like there is armor */
      /* lets check threshold */
      if (damage > VMSpace[target].VMPerm.Eng.Systems[armor].threshold) {
	damage = damage - VMSpace[target].VMPerm.Eng.Systems[armor].threshold;
      }
      else {
	VMnotifymans(VMSpace[target].VMTemp.TmpHelm.VMMan_List,"Ship has been hit. No damage done to ship.");
	VMSpaceNotify(target,"The ship shakes slighly from something hitting it.");
	return 0;
      }
      if (VMSpace[target].VMPerm.Eng.Systems[armor].integrity > 0) {
	/* looks like we're going to subtract damage from the armor */
	after = VMSpace[target].VMPerm.Eng.Systems[armor].integrity - damage;
	if (after < 0) {
	  through = 0 - after;
	  armdam = VMSpace[target].VMPerm.Eng.Systems[armor].integrity; 
	  after = 0;
	}
	else {
	  armdam = VMSpace[target].VMPerm.Eng.Systems[armor].integrity - after;
	}
	VMSpace[target].VMPerm.Eng.Systems[armor].integrity = after;
	VMnotifymans(VMSpace[target].VMTemp.TmpHelm.VMMan_List,"Ship has been hit. %d damage done to %s armor.",armdam,astr);
	if (after == 0)
	  VMSpaceNotify(target,"The ship shakes slighly from something hitting it.");
      }
      else {
	/* all the integrity in the armor is gone */
	through = damage;
	VMnotifymans(VMSpace[target].VMTemp.TmpHelm.VMMan_List,"You have been hit in a location without any integrity in the armor. %d damage done to %s armor",through,astr);
      }
      
    }
    else {
      /* i guess there is no armor in that location */
      through = damage;
      VMnotifymans(VMSpace[target].VMTemp.TmpHelm.VMMan_List,"You have been hit in a location without armor. %d damage done to %s arc",through,astr);
    }
    if (through > 0) {
      /* there actually is damage through */
      perin = (int) through / VMSpace[target].VMPerm.mIntegrity * 100;
      roll = random() % 100;
      if ( perin >= roll ) {
#ifdef SPACEDEBUG
	VMnotify(35,"Critical hit.");
#endif
	VMSpaceNotify(target,"The ship shakes violently from something hitting it.");
	sysdam = (double) (through / 2); 
	hulldam = (double) (through / 2);
      }
      else {
	VMSpaceNotify(target,"The ship shakes violently from something hitting it.");
	hulldam = through;
      }
    }
    if (hulldam > 0) 
      VMDamageHull(target,hulldam);
    if (sysdam > 0) {
      pain = sysdam / 6;
      remaining = (int) sysdam % 6;
      for ( i = 1; i < 6;i++) {
	VMDamageSystem(target,pain,0);
      }
      VMDamageSystem(target,pain + remaining,0);
    }
    /* oops we may wanna damage hull *sigh* */
    
    VMCheckShip(target);
    

    
    break;

  case 2: /* stasis weapons */
    /* basically what we want to do is figure out how much to reduce the reactor max */
    inmod = random() % 3;
    mymod = (double) inmod / 100;
    reduction = damage / 100;
    reduction = reduction - ( reduction * mymod);
     if (reduction > 0) {
      VMSpace[target].VMPerm.Eng.Systems[0].istat2 -= reduction;
      VMSpace[target].VMPerm.Eng.Systems[0].istat1 -= (reduction/2);

      if (VMSpace[target].VMPerm.Eng.Systems[0].istat2 < 0) 
	VMSpace[target].VMPerm.Eng.Systems[0].istat2 = 0;
      if (VMSpace[target].VMPerm.Eng.Systems[0].istat1 < 0) 
	VMSpace[target].VMPerm.Eng.Systems[0].istat1 = 0;
    
      VMnotifymans(VMSpace[target].VMTemp.TmpEng.VMMan_List,"[Engineering] Stasis beam has hit.  Reactor max lowered by %d to a new max of %d. Reallocating.",reduction,VMSpace[target].VMPerm.Eng.Systems[0].istat2);
      if (VMSpace[target].VMPerm.Eng.Systems[0].tistat1 > VMSpace[target].VMPerm.Eng.Systems[0].istat2 ) {
	VMSpace[target].VMPerm.Eng.Systems[0].tistat1 = VMSpace[target].VMPerm.Eng.Systems[0].istat2;
	VMnotifymans(VMSpace[target].VMTemp.TmpEng.VMMan_List,"[Engineering] Reactor now set to %d.",VMSpace[target].VMPerm.Eng.Systems[0].tistat1);
      }
      
      VMEngUpdateEngPower(target);
    }
    break;
  case 3: /* case 3 is plasma */
    armor = VMArc2Armor(arc);
    bzero(astr,100);
    if ( armor == 4) {
      strcat(astr,"Fore");
    }
    else if ( armor == 5 ) {
      strcat(astr,"Aft");
    }
    else if ( armor == 6 ) {
      strcat(astr,"Port");
    }
    else if ( armor == 7 ) {
      strcat(astr,"Starboard");
    }
    else if ( armor == 8 ) {
      strcat(astr, "Ventral");
    }
    else if ( armor == 9 ) {
      strcat(astr,"Dorsal");
    }


    if ((checkflag(VMSpace[target].VMPerm.Eng.Systems[armor].flags,EXISTS) && checkflag(VMSpace[target].VMPerm.Eng.Systems[armor].flags,VMARMOR))) {
      /* looks like there is armor */
      /* lets check threshold */
      if (damage > VMSpace[target].VMPerm.Eng.Systems[armor].threshold / 2 ) {
	damage = damage - VMSpace[target].VMPerm.Eng.Systems[armor].threshold / 2;
      }
      else {
	VMnotifymans(VMSpace[target].VMTemp.TmpHelm.VMMan_List,"Ship has been hit. No damage done to ship.");
	VMSpaceNotify(target,"The ship shakes slighly from something hitting it.");
	return 0;
      }
      if (VMSpace[target].VMPerm.Eng.Systems[armor].integrity > 0) {
	/* looks like we're going to subtract damage from the armor */
	after = VMSpace[target].VMPerm.Eng.Systems[armor].integrity - damage;
	if (after < 0) {
	  through = 0 - after;
	  armdam = VMSpace[target].VMPerm.Eng.Systems[armor].integrity; 
	  after = 0;
	}
	else {
	  armdam = VMSpace[target].VMPerm.Eng.Systems[armor].integrity - after;
	}
	VMSpace[target].VMPerm.Eng.Systems[armor].integrity = after;
	VMnotifymans(VMSpace[target].VMTemp.TmpHelm.VMMan_List,"Ship has been hit. %d damage done to %s armor.",armdam,astr);
	if (after == 0)
	  VMSpaceNotify(target,"The ship shakes slighly from something hitting it.");
      }
      else {
	/* all the integrity in the armor is gone */
	through = damage;
	VMnotifymans(VMSpace[target].VMTemp.TmpHelm.VMMan_List,"You have been hit in a location without any integrity in the armor. %d damage done to %s armor",through,astr);
      }
      
    }
    else {
      /* i guess there is no armor in that location */
      through = damage;
      VMnotifymans(VMSpace[target].VMTemp.TmpHelm.VMMan_List,"You have been hit in a location without armor. %d damage done to %s arc",through,astr);
    }
    if (through > 0) {
      /* there actually is damage through */
      perin = (int) through / VMSpace[target].VMPerm.mIntegrity * 100;
      roll = random() % 100;
      if ( perin >= roll ) {
#ifdef SPACEDEBUG
	VMnotify(35,"Critical hit.");
#endif
	VMSpaceNotify(target,"The ship shakes violently from something hitting it.");
	sysdam = (double) (through / 2); 
	hulldam = (double) (through / 2);
      }
      else {
	VMSpaceNotify(target,"The ship shakes violently from something hitting it.");
	hulldam = through;
      }
    }
    if (hulldam > 0) 
      VMDamageHull(target,hulldam);
    if (sysdam > 0) {
      pain = sysdam / 6;
      remaining = (int) sysdam % 6;
      for ( i = 1; i < 6;i++) {
	VMDamageSystem(target,pain,1);
      }
      VMDamageSystem(target,pain + remaining,1);
    }
    /* oops we may wanna damage hull *sigh* */
    
    VMCheckShip(target);
    break;

  }
  if (checkflag(VMSpace[target].VMPerm.Flags,DEAD)) {
    VMnotifyShips(target,"is hit with a volley and shatters into pieces!");
    VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"You feel the oxygen leave your lungs as your ship explodes around you...");
    return -1;
  }
}  
void 
VMDamageHull(int victim, 
	     double damage
	     ) {
  int hulldam,inmod;
  double mymod;
  if ( damage > VMSpace[victim].VMPerm.threshold) { 
    hulldam = damage - VMSpace[victim].VMPerm.threshold;
  }
  else 
    hulldam = 0;
  inmod = random() % 3;
  mymod = (double) inmod / 100;
  hulldam = hulldam - (hulldam * mymod);
  VMnotifymans(VMSpace[victim].VMTemp.TmpEng.VMMan_List,"Hull damaged %lf.",hulldam);
  VMSpace[victim].VMPerm.Integrity -= hulldam;
  if (VMSpace[victim].VMPerm.Integrity <= 0) {
    /* BOOM*/
    VMKillShip(victim);
  }

   
}


int
VMDamageSystem(int victim,
	       int damage,
	       int flag
	       ) {
  int s,per;
  int location,place,done,count;
  double rthresh;
  done = 0;
  count = 0;
  /* first lets figure out what section we hit 
     0 - Helm
     1 - Nav
     2 - Eng
     3 - Misc
     
     */
  
  location = random() % 3;
  /* for now its 3 it should be 4 eventually */ 
  /* this loop makes sure we damage a system that's actually there */ 
  while (!done) {
    count++;
    /* if it takes more than 300 times to find a good system we have problems */
    if (count > 300) {
#ifdef SPACEDEBUG
      VMnotify(35,"ERROR!!!!!!!!!!!!!!!!: Unable to find a good system in 300 ttries, ending this loop");
#endif
      return;
    }
    switch (location) {
    case 0: /* helm */
      s = random() % HSystems;
      if (checkflag(VMSpace[victim].VMPerm.Helm.Systems[s].flags,EXISTS))
	done = 1;
      break;
    case 1: /* nav */
      s = random() % NSystems;
      if (checkflag(VMSpace[victim].VMPerm.Nav.Systems[s].flags,EXISTS))
	done = 1;

      break;
    case 2: /* eng */
      s = random() % ESystems;
      if (checkflag(VMSpace[victim].VMPerm.Nav.Systems[s].flags,EXISTS)) {
	if (checkflag(VMSpace[victim].VMPerm.Nav.Systems[s].flags,VMARMOR)) {
	  break;
	}
	else {
	  done = 1;
	}
      }
      break;
    case 3: /* misc unimplemented */
      s = random() % ESystems;
      break;
    }
  }
  
  switch(location) {
  case 0:
#ifdef SPACEDEBUG
    VMnotify(35,"location is helm, system is %d",s); 
    VMnotify(35,"Damaging System %s by %d points.",VMSpace[victim].VMPerm.Helm.Systems[s].name,damage);
#endif
    rthresh = VMSpace[victim].VMPerm.Helm.Systems[s].threshold;
    if (flag == 1) { /* plasma - halve threshold */
      rthresh = rthresh / 2;
    }

    if (damage > rthresh) {
      damage -= rthresh;
      if (VMSpace[victim].VMPerm.Helm.Systems[s].integrity - damage > 0) {
	VMSpace[victim].VMPerm.Helm.Systems[s].integrity -= damage;
	VMnotifymans(VMSpace[victim].VMTemp.TmpHelm.VMMan_List," System %s(%d) has been hit!. It is damaged by %d points. It is now at %2.2f points.",VMSpace[victim].VMPerm.Helm.Systems[s].name,s,damage,VMSpace[victim].VMPerm.Helm.Systems[s].integrity);
      }
      else {
	VMSpace[victim].VMPerm.Helm.Systems[s].integrity = 0;
	VMnotifymans(VMSpace[victim].VMTemp.TmpHelm.VMMan_List," System %s(%d) has been destroyed.",VMSpace[victim].VMPerm.Helm.Systems[s].name,s,damage,VMSpace[victim].VMPerm.Helm.Systems[s].integrity);
      }
      if (VMSpace[victim].VMPerm.Helm.Systems[s].mintegrity > 0)
	VMSpace[victim].VMPerm.Helm.Systems[s].status =  VMSpace[victim].VMPerm.Helm.Systems[s].integrity / VMSpace[victim].VMPerm.Helm.Systems[s].mintegrity;
      else
	VMSpace[victim].VMPerm.Helm.Systems[s].status = 0;
    }
    else 
      VMnotifymans(VMSpace[victim].VMTemp.TmpHelm.VMMan_List," System %s(%d) has been hit! No damage done to system.",VMSpace[victim].VMPerm.Helm.Systems[s].name,s);
      break;
    case 1:
#ifdef SPACEDEBUG
      VMnotify(35,"location is nav, system is %d",s); 
      VMnotify(35,"Damaging System %s by %d points.",VMSpace[victim].VMPerm.Nav.Systems[s].name,damage);
#endif
      if (damage > VMSpace[victim].VMPerm.Nav.Systems[s].threshold) {
	damage -= VMSpace[victim].VMPerm.Nav.Systems[s].threshold;
	if (VMSpace[victim].VMPerm.Nav.Systems[s].integrity - damage > 0) {
	  VMSpace[victim].VMPerm.Nav.Systems[s].integrity -= damage;
	  VMnotifymans(VMSpace[victim].VMTemp.TmpNav.VMMan_List," System %s(%d) has been hit!. It is damaged by %d points. It is now at %2.2f points.",VMSpace[victim].VMPerm.Nav.Systems[s].name,s,damage,VMSpace[victim].VMPerm.Nav.Systems[s].integrity);
	}
	else { /**/
	  VMSpace[victim].VMPerm.Nav.Systems[s].integrity = 0;
	  VMnotifymans(VMSpace[victim].VMTemp.TmpNav.VMMan_List," System %s(%d) has been destroyed.",VMSpace[victim].VMPerm.Nav.Systems[s].name,s,damage,VMSpace[victim].VMPerm.Nav.Systems[s].integrity);
	}
	if (VMSpace[victim].VMPerm.Nav.Systems[s].mintegrity > 0)
	  VMSpace[victim].VMPerm.Nav.Systems[s].status =  VMSpace[victim].VMPerm.Nav.Systems[s].integrity / VMSpace[victim].VMPerm.Nav.Systems[s].mintegrity;
	else
	  VMSpace[victim].VMPerm.Nav.Systems[s].status = 0;
      }
      else
	VMnotifymans(VMSpace[victim].VMTemp.TmpNav.VMMan_List," System %s(%d) has been hit! No damage done to system.",VMSpace[victim].VMPerm.Helm.Systems[s].name,s);
      break;
    case 2:
#ifdef SPACEDEBUG
      VMnotify(35,"location is eng  system is %d",s); 
      VMnotify(35,"Damaging System %s by %d points.",VMSpace[victim].VMPerm.Eng.Systems[s].name,damage);
#endif
      if (damage > VMSpace[victim].VMPerm.Nav.Systems[s].threshold) {
	damage -= VMSpace[victim].VMPerm.Nav.Systems[s].threshold;
	if (VMSpace[victim].VMPerm.Eng.Systems[s].integrity - damage > 0) {
	  VMSpace[victim].VMPerm.Eng.Systems[s].integrity -= damage;
	  VMnotifymans(VMSpace[victim].VMTemp.TmpEng.VMMan_List," System %s(%d) has been hit!. It is damaged by %d points. It is now at %2.2f points.",VMSpace[victim].VMPerm.Eng.Systems[s].name,s,damage,VMSpace[victim].VMPerm.Eng.Systems[s].integrity);
	}
	else {
	  VMSpace[victim].VMPerm.Eng.Systems[s].integrity = 0;
	  VMnotifymans(VMSpace[victim].VMTemp.TmpEng.VMMan_List," System %s(%d) has been destroyed.",VMSpace[victim].VMPerm.Eng.Systems[s].name,s,damage,VMSpace[victim].VMPerm.Eng.Systems[s].integrity);
	}
	if (VMSpace[victim].VMPerm.Eng.Systems[s].mintegrity > 0)
	  VMSpace[victim].VMPerm.Eng.Systems[s].status =  VMSpace[victim].VMPerm.Eng.Systems[s].integrity / VMSpace[victim].VMPerm.Eng.Systems[s].mintegrity;
	else
	  VMSpace[victim].VMPerm.Eng.Systems[s].status = 0;
      }
      else
	VMnotifymans(VMSpace[victim].VMTemp.TmpEng.VMMan_List," System %s(%d) has been hit! No damage done to system.",VMSpace[victim].VMPerm.Helm.Systems[s].name,s);
	break;
  case 3:
#ifdef SPACEDEBUG
        VMnotify(35,"location is misc, system is %d",s); 
#endif
    break;
  }  

}
int 
VMArc2Armor(int arc
	    ) {
  switch (arc) {
  case FORE:
    return ArmorFore;
    break;
  case AFT:
    return ArmorAft;
    break;
  case PORT:
    return ArmorPort;
    break;
  case STARB:
    return ArmorStar;
    break;
  case TOP:
    return ArmorTop;
    break;
  case BOTTOM:
    return ArmorBottom;
    break;
  default:
    return -1;
    break;
  }
}

void
VMCheckShip(int ship
	    ) {

}

int VMGun2SysNum(int ship,
		 int gun
		 ) {
}

void
VMProcessDamageQueue(t_WList *q
		 ) {
  ResetWList(q);
  while(!AtEndWList(q)) {
    t_WEnt *tmp;
    tmp = (t_WEnt *) GetCurrWEnt(q);
    switch (tmp->event) {
    case 0:
      /* helm */
      VMRepairSystem(tmp->ship,&(VMSpace[tmp->ship].VMPerm.Helm.Systems[tmp->v2]));
      break;
    case 1:
      /*nav*/
      VMRepairSystem(tmp->ship,&(VMSpace[tmp->ship].VMPerm.Nav.Systems[tmp->v2]));
      break;
    case 2:
      /* eng */
      VMRepairSystem(tmp->ship,&(VMSpace[tmp->ship].VMPerm.Eng.Systems[tmp->v2]));
      break;
    case 5: 
      /* hull */
      VMRepairHull(tmp->ship);
      break;
    }
    AdvanceWList(q);
  }
}

void 
VMRepairSystem(int ship, 
	       t_System *s
	       ) {
  double rndfact,ptsgained;
  if (s->integrity < s->mintegrity) {
    /* repair */
    ptsgained = 1; /* always get a point back */
    ptsgained += (VMSpace[ship].VMPerm.Eng.Systems[2].tistat0 / 50);
    rndfact = (random() % 3) /100;
    ptsgained = ptsgained * (1-rndfact);
    ptsgained = ptsgained * VMSpace[ship].VMPerm.Eng.Systems[2].status;
    if (ptsgained < 1) 
      ptsgained = 1; /* always get one point back */
    if ( VMSpace[ship].VMPerm.Eng.Systems[2].tistat3 != -1)
      VMnotify(VMSpace[ship].VMPerm.Eng.Systems[2].tistat3,"Repairing system %s by a total of %2.2f points",s->name,ptsgained); 
    s->integrity = s->integrity + ptsgained;
    if (s-> integrity > s->mintegrity)
      s->integrity = s->mintegrity;
    s->status = s->integrity / s->mintegrity;
  }
}
void
VMRepairHull(int ship
	     ) {

  double rndfact,ptsgained;
  rndfact = 1.0;
  if (VMSpace[ship].VMPerm.Integrity < VMSpace[ship].VMPerm.mIntegrity) {
    /* repair */
    ptsgained = 1; /* always get a point back */
    ptsgained += (VMSpace[ship].VMPerm.Eng.Systems[2].tistat0 / 50);
    rndfact = (random() % 3) /100;
    ptsgained = ptsgained * (1-rndfact);
    ptsgained = ptsgained * VMSpace[ship].VMPerm.Eng.Systems[2].status;
    if (ptsgained < 1) 
      ptsgained = 1; /* always get one point back */
    if ( VMSpace[ship].VMPerm.Eng.Systems[2].tistat3 != -1)
      VMnotify(VMSpace[ship].VMPerm.Eng.Systems[2].tistat3,"Repairing system %s by a total of %2.2f points","hull",ptsgained); 
    VMSpace[ship].VMPerm.Integrity += ptsgained;
    if (VMSpace[ship].VMPerm.Integrity > VMSpace[ship].VMPerm.mIntegrity)
      VMSpace[ship].VMPerm.Integrity = VMSpace[ship].VMPerm.mIntegrity;
  }
}
