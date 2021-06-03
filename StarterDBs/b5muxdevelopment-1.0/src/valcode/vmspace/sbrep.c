#include "header.h"
#include "../econ.h"
static char *rcssbrepc="$Id: sbrep.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
void VMSBFuel(player,ShipNum,ship,amount)
int player,ShipNum,ship,amount;
{
int cargo,avail;
double shipf,shipm,cg;
char buff[50];

  if(VMShipFromRoom(VMSpace[ship].VMPerm.Nav.Dock)!=ShipNum) {
	VMnotify(player,"That ship is not docked here");
	return;
	}
  	cargo=atoi(vget_a(where_is(player),SHIP_A));	
	if(cargo <= 0) {
		VMnotify(player,"There is no supplying cargo room.");
		return;
	}
	if(!(Flags2(cargo)&CONSOLE)) {
		VMnotify(player,"There is no supplying cargo room.");
		return;
	}
	avail=read_ilist(cargo,756,COMMOD_A);
	if(amount > avail) {
		VMnotify(player,"Insufficient fuel available.");
		return;
	}
	shipf=VMSpace[ship].VMPerm.Eng.Systems[0].dstat4;
	shipm=VMSpace[ship].VMPerm.Eng.Systems[0].dstat5;
	if(shipf+amount > shipm+.9) {
		VMnotify(player,"That ship cant hold that much fuel.");
		return;
	}
	if(shipf+amount > shipm) 
	        VMSpace[ship].VMPerm.Eng.Systems[0].dstat4=shipm;
	shipf+=amount;
	VMSpace[ship].VMPerm.Eng.Systems[0].dstat4=shipf;
	avail-=amount;
	change_ilist(cargo,756,avail,COMMOD_A);
	cg=atof(vget_a(cargo,CARGO));
	cg+=EconItems[756].size*amount;
	sprintf(buff,"%f\0",cg);
	atr_add_raw(cargo,CARGO,buff);
	VMnotify(player,"Ship refueled.");
	
}

void
VMSBProcessDamageQueue(t_WList *q
		 ) {
int i=0,dock,num;
    t_WEnt *tmp;
char buff[100];
  ResetWList(q);
  while(!AtEndWList(q)) {
    tmp = (t_WEnt *) GetCurrWEnt(q);
    i=0;
    switch (tmp->event) {
    case 0:
      /* helm */
 i=VMSBRepairSystem(tmp->ship,&(VMSpace[tmp->ship].VMPerm.Helm.Systems[tmp->v2]));
      break;
    case 1:
      /*nav*/
 i=VMSBRepairSystem(tmp->ship,&(VMSpace[tmp->ship].VMPerm.Nav.Systems[tmp->v2]));
      break;
    case 2:
      /* eng */
 i=VMSBRepairSystem(tmp->ship,&(VMSpace[tmp->ship].VMPerm.Eng.Systems[tmp->v2]));
      break;
    case 5: 
      /* hull */
      i=VMSBRepairHull(tmp->ship);
      break;
    }
   if(i==1) {
	dock=VMSpace[tmp->ship].VMPerm.Nav.Dock;
	notify_all_from_inside(dock,1,tprintf("Team %d has finished their work.",tmp->v1));
	RemoveWEnt(q);
  	num=atoi(vget_a(dock,A_VA+14));	
	num--;
 	sprintf(buff,"%d",num);
	atr_add_raw(dock,A_VA+14,buff);

	}
   else AdvanceWList(q);
  }
}



int
VMSBRepairSystem(int ship, 
	       t_System *s
	       ) {
  double rndfact,ptsgained;
  if (s->integrity < s->mintegrity) {
    /* repair */
   ptsgained = 10;
    rndfact = (random() % 3) /200;
    ptsgained = ptsgained * (1-rndfact);
    if (ptsgained < 1) 
      ptsgained = 1; /* always get one point back */
    /*if ( VMSpace[ship].VMPerm.Eng.Systems[2].tistat3 != -1)
      VMnotify(VMSpace[ship].VMPerm.Eng.Systems[2].tistat3,"Repairing system %s by a total of %2.2f points",s->name,ptsgained); 
    */
    s->integrity = s->integrity + ptsgained;
    if (s-> integrity > s->mintegrity)
      s->integrity = s->mintegrity;
    s->status = s->integrity / s->mintegrity;
	return 0;
  }
 return 1;
}


int
VMSBRepairHull(int ship
	     ) {

  double rndfact,ptsgained;
  rndfact = 1.0;
  if (VMSpace[ship].VMPerm.Integrity < VMSpace[ship].VMPerm.mIntegrity) {
    /* repair */
    rndfact = (random() % 3) /100;
    ptsgained = 10;
    rndfact = (random() % 3) /200;
    ptsgained = ptsgained * (1-rndfact);
    if (ptsgained < 1) 
      ptsgained = 1; /* always get one point back */
/*VMnotify(2,tprintf("Repairing hull=%d, max=%d",VMSpace[ship].VMPerm.Integrity , VMSpace[ship].VMPerm.mIntegrity));*/
    /*if ( VMSpace[ship].VMPerm.Eng.Systems[2].tistat3 != -1)
      VMnotify(VMSpace[ship].VMPerm.Eng.Systems[2].tistat3,"Repairing system %s by a total of %2.2f points","hull",ptsgained); 
    */
    VMSpace[ship].VMPerm.Integrity += ptsgained;
    if (VMSpace[ship].VMPerm.Integrity > VMSpace[ship].VMPerm.mIntegrity)
      VMSpace[ship].VMPerm.Integrity = VMSpace[ship].VMPerm.mIntegrity;
   return 0;
  }
 return 1;
}


void VMSBDamStat(int player, 
	       int ship
	       ) {
  int teams,uteams,unused,i,per,rdb;
  char nstr[16];
  bzero(nstr,16);
  rdb=where_is(player);
  teams=atoi(vget_a(rdb,A_VA+13));
  uteams=atoi(vget_a(rdb,A_VA+14));
  VMnotify(player,"---------Damage Control Status Report---------");
  /*VMnotify(player,"Power to Damage: %d",VMSpace[ship].VMPerm.Eng.Systems[2].tistat0);*/
  VMnotify(player,"Total Teams: %d",teams);
  VMnotify(player,"Teams in use: %d Teams free: %d",uteams,teams-uteams);
  VMnotify(player,"---------Team Status--------------------------");
  ResetWList(VMDamageSBQ);
  while(!AtEndWList(VMDamageSBQ)) {
    t_WEnt *tmp;
    tmp = (t_WEnt *) GetCurrWEnt(VMDamageSBQ);
    /*if (VMSpace[tmp->ship].VMPerm.Nav.Dock == ship) {*/
    if (VMShipFromRoom(VMSpace[tmp->ship].VMPerm.Nav.Dock) == ship) {
      switch (tmp->event) {
      case 0:

	VMnotify(player,"Team %d is working on %s (Helm #%d).",tmp->v1,VMSpace[tmp->ship].VMPerm.Helm.Systems[tmp->v2].name,tmp->v2);
	break;
      case 1:
	VMnotify(player,"Team %d is working on %s (Nav #%d).",tmp->v1,VMSpace[tmp->ship].VMPerm.Nav.Systems[tmp->v2].name,tmp->v2);
	break;
      case 2:

	VMnotify(player,"Team %d is working on %s (Eng #%d).",tmp->v1,VMSpace[tmp->ship].VMPerm.Eng.Systems[tmp->v2].name,tmp->v2);
	break;
      case 5:
	VMnotify(player,"Team %d is working on hull.",tmp->v1);
	break;
      }
      AdvanceWList(VMDamageSBQ);
    }
    else
      AdvanceWList(VMDamageSBQ);
  }
}

void VMSBSysStat(int player, 
	       int ship,
		int ship2
	       ) {
  int unused,i,per;
  char nstr[16];
  bzero(nstr,16);
    if (VMShipFromRoom(VMSpace[ship].VMPerm.Nav.Dock) != ship2) {
		VMnotify(player,"That ship is not docked here");
		return;
	}

  VMnotify(player,"---------System Status------------------------");
  /* helm */
  for ( i = 0; i < HSystems; i++) {
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,EXISTS)) {
      if (VMSpace[ship].VMPerm.Helm.Systems[i].integrity != VMSpace[ship].VMPerm.Helm.Systems[i].mintegrity) {
	if (VMSpace[ship].VMPerm.Helm.Systems[i].mintegrity > 0)
	  per = (VMSpace[ship].VMPerm.Helm.Systems[i].integrity / VMSpace[ship].VMPerm.Helm.Systems[i].mintegrity) * 100; 
	else
	  per = -1;
	VMnotify(player,"%s(helm %d)\t\t %2.2f/%2.2f (%d%%)",VMSpace[ship].VMPerm.Helm.Systems[i].name,i,VMSpace[ship].VMPerm.Helm.Systems[i].integrity,VMSpace[ship].VMPerm.Helm.Systems[i].mintegrity,per);
      }
    }
  }
  /* nav */
  for ( i = 0; i < NSystems; i++) {
    if (checkflag(VMSpace[ship].VMPerm.Nav.Systems[i].flags,EXISTS)) {
      if (VMSpace[ship].VMPerm.Nav.Systems[i].integrity != VMSpace[ship].VMPerm.Nav.Systems[i].mintegrity) {
	if (VMSpace[ship].VMPerm.Nav.Systems[i].mintegrity > 0)
	  per = (VMSpace[ship].VMPerm.Nav.Systems[i].integrity / VMSpace[ship].VMPerm.Nav.Systems[i].mintegrity) * 100; 
	else
	  per = -1;
	VMnotify(player,"%s(nav %d)\t\t %2.2f/%2.2f (%d%%)",VMSpace[ship].VMPerm.Nav.Systems[i].name,i,VMSpace[ship].VMPerm.Nav.Systems[i].integrity,VMSpace[ship].VMPerm.Nav.Systems[i].mintegrity,per);
      }
    }
  }
  
  for ( i = 0; i < ESystems; i++) {
    if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[i].flags,EXISTS)) {
      if (VMSpace[ship].VMPerm.Eng.Systems[i].integrity != VMSpace[ship].VMPerm.Eng.Systems[i].mintegrity) {
	if (VMSpace[ship].VMPerm.Eng.Systems[i].mintegrity > 0)
	  per = (VMSpace[ship].VMPerm.Eng.Systems[i].integrity / VMSpace[ship].VMPerm.Eng.Systems[i].mintegrity) * 100; 
	else
	  per = -1;
	VMnotify(player,"%s(eng %d)\t\t %2.2f/%2.2f (%d%%)",VMSpace[ship].VMPerm.Eng.Systems[i].name,i,VMSpace[ship].VMPerm.Eng.Systems[i].integrity,VMSpace[ship].VMPerm.Eng.Systems[i].mintegrity,per);
      }
    }
  }
  if (VMSpace[ship].VMPerm.Integrity != VMSpace[ship].VMPerm.mIntegrity) {
    if (VMSpace[ship].VMPerm.mIntegrity > 0)
      per = (VMSpace[ship].VMPerm.Integrity / VMSpace[ship].VMPerm.mIntegrity) * 100; 
    else
      per = -1;
    VMnotify(player,"Hull integrity %2.2f/%2.2f (%d%%)",VMSpace[ship].VMPerm.Integrity,VMSpace[ship].VMPerm.mIntegrity,per);
  }

}

void VMSBMiscRepair(int player,
		  int ship,
		  int team,
		  char *sysstr,
		  int number
		  ) {
  
  int sb,num,teams,actar = -1;
  char buff[50];
  sb=VMShipNum(player); 
  if(VMShipFromRoom(VMSpace[ship].VMPerm.Nav.Dock)!=sb) {
	VMnotify(player,"That ship is not docked here");
	return;
	}
 
  if(!strncmp(sysstr,"helm",4)) 
    actar = 0;
  else if(!strncmp(sysstr,"nav",3)) 
    actar = 1;
  else if(!strncmp(sysstr,"eng",3)) 
    actar = 2;
  if (actar == -1) {
    VMnotify(player,"System group may either be helm, nav, or eng");
    return;
  }
  
  teams=atoi(vget_a(where_is(player),A_VA+13));	/* VB stores teams*/

  if (team <= 0 || team > teams) {
    VMnotify(player,"Invalid team given");
    return;
  }
  ResetWList(VMDamageSBQ);
  while(!AtEndWList(VMDamageSBQ)) {
    t_WEnt *tmp;
    tmp = (t_WEnt *) GetCurrWEnt(VMDamageSBQ);
    if (tmp->v1 == team) {
      RemoveWEnt(VMDamageSBQ);
  	num=atoi(vget_a(where_is(player),A_VA+14));	
	num--;
 	sprintf(buff,"%d",num);
	atr_add_raw(where_is(player),A_VA+14,buff);

      /*VMSpace[ship].VMPerm.Eng.Systems[2].tistat1--;*/
    }
    AdvanceWList(VMDamageSBQ);
  }
  switch (actar) {
  case 0:
    if (number < 0 || number > HSystems) {
      VMnotify(player,"that system does not exist.");
      return;
    }
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[number].flags,EXISTS)) {
      VMnotify(player,"Team %d is now working on %s.",team,VMSpace[ship].VMPerm.Helm.Systems[number].name);
      AddWEnt(VMDamageSBQ,0,actar,ship,team,number,0,0);    
  	num=atoi(vget_a(where_is(player),A_VA+14));	
	num++;
 	sprintf(buff,"%d",num);
	atr_add_raw(where_is(player),A_VA+14,buff);
      /*VMSpace[ship].VMPerm.Eng.Systems[2].tistat1++;*/
      return;
    }
    else {
      VMnotify(player,"That system does not exist.");
    }
    break;
  case 1:
    if (number < 0 || number > NSystems) {
      VMnotify(player,"that system does not exist.");
      return;
    }
    if (checkflag(VMSpace[ship].VMPerm.Nav.Systems[number].flags,EXISTS)) {
      VMnotify(player,"Team %d is now working on %s.",team,VMSpace[ship].VMPerm.Nav.Systems[number].name);
      AddWEnt(VMDamageSBQ,0,actar,ship,team,number,0,0); 
  	num=atoi(vget_a(where_is(player),A_VA+14));	
	num++;
 	sprintf(buff,"%d",num);
	atr_add_raw(where_is(player),A_VA+14,buff);
      /*VMSpace[ship].VMPerm.Eng.Systems[2].tistat1++;   */
      return;
    }
    else {
      VMnotify(player,"That system does not exist.");
    }
    break;
  case 2:
    if (number < 0 || number > ESystems) {
      VMnotify(player,"that system does not exist.");
      return;
    }
    if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[number].flags,EXISTS)) {
      VMnotify(player,"Team %d is now working on %s.",team,VMSpace[ship].VMPerm.Eng.Systems[number].name);
  	num=atoi(vget_a(where_is(player),A_VA+14));	
	num++;
 	sprintf(buff,"%d",num);
	atr_add_raw(where_is(player),A_VA+14,buff);
      /*VMSpace[ship].VMPerm.Eng.Systems[2].tistat1++;*/
      AddWEnt(VMDamageSBQ,0,actar,ship,team,number,0,0);    
      return;
    }
    else {
      VMnotify(player,"That system does not exist.");
    }
    break;
  }

  
}

void VMSBMiscRepairHull(int player,
		 int ship,
		 int team 
		 ) {
  int num;
  char buff[50];
  if (team < 0 || team > VMSpace[ship].VMPerm.Eng.Systems[2].istat1) {
    VMnotify(player,"Invalid team given");
    return;
  }
  ResetWList(VMDamageSBQ);
  while(!AtEndWList(VMDamageSBQ)) {
    t_WEnt *tmp;
    tmp = (t_WEnt *) GetCurrWEnt(VMDamageSBQ);
    if (tmp->v1 == team) {
      RemoveWEnt(VMDamageSBQ);
  	num=atoi(vget_a(where_is(player),A_VA+14));	
	num--;
 	sprintf(buff,"%d",num);
	atr_add_raw(where_is(player),A_VA+14,buff);
      /*VMSpace[ship].VMPerm.Eng.Systems[2].tistat1--;*/
    }
    AdvanceWList(VMDamageSBQ);
  }
  AddWEnt(VMDamageSBQ,0,5,ship,team,0,0,0);       
  	num=atoi(vget_a(where_is(player),A_VA+14));	
	num++;
 	sprintf(buff,"%d",num);
	atr_add_raw(where_is(player),A_VA+14,buff);
  /*VMSpace[ship].VMPerm.Eng.Systems[2].tistat1++;*/
  VMnotify(player,"Team %d is now working on the hull.",team);
}


void 
VMSBGetShipOutDQ(int ship) {
 int sb; 
  int num;
  char buff[50];
  ResetWList(VMDamageSBQ);
  while(!AtEndWList(VMDamageSBQ)) {
    t_WEnt *tmp;
    tmp = (t_WEnt *) GetCurrWEnt(VMDamageSBQ);
    if (tmp->ship == ship) {
      RemoveWEnt(VMDamageSBQ);
	sb=VMSpace[tmp->ship].VMPerm.Nav.Dock;
  	num=atoi(vget_a(sb,A_VA+14));	
	num--;
 	sprintf(buff,"%d",num);
	atr_add_raw(sb,A_VA+14,buff);
      /*VMSpace[ship].VMPerm.Eng.Systems[2].tistat1--;*/
    }
    AdvanceWList(VMDamageSBQ);
  }
}


