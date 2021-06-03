/* This file contains damage control routines */
#include "header.h"
#include "vmweaptypes.h"
#include <stdarg.h>


static char *rcsvmdamrepc="$Id: vmmisc.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
int 
CanAccessDamCon(int ship,
	      int playerr
	      ) {
  
  if(!ShipActive(ship)) {
    VMnotify(playerr,"This ship is not in space.");
    return 0;
	}

  if (VMGlobalSpaceInit == 0) {
    VMnotify(playerr,"Space has not been initialized.");
    return;
  }
  if(ship < 0 || ship > VMCurrentMax) 
    {
      VMnotify(playerr,"Danger Danger. Possible Bug! Ship number is out of possible range");
      return 0;
    }
  if(!IsPlayerInManList(VMSpace[ship].VMTemp.TmpDam.VMMan_List,playerr))
    {
      VMnotify(playerr,"You must man the damage control console");
      return 0;
    }
  return 1;
}


int 
CanAccessSci(int ship,
	      int playerr
	      ) {
  
  if(!ShipActive(ship)) {
    VMnotify(playerr,"This ship is not in space.");
    return 0;
	}

  if (VMGlobalSpaceInit == 0) {
    VMnotify(playerr,"Space has not been initialized.");
    return;
  }
  if(ship < 0 || ship > VMCurrentMax) 
    {
      VMnotify(playerr,"Danger Danger. Possible Bug! Ship number is out of possible range");
      return 0;
    }
  if(!IsPlayerInManList(VMSpace[ship].VMTemp.TmpScience.VMMan_List,playerr))
    {
      VMnotify(playerr,"You must man the science console");
      return 0;
    }
  return 1;
}


void VMDamStat(int player, 
	       int ship
	       ) {
  int unused,i,per;
  char nstr[16];
  bzero(nstr,16);
  VMnotify(player,"---------Damage Control Status Report---------");
  VMnotify(player,"Power to Damage: %d",VMSpace[ship].VMPerm.Eng.Systems[2].tistat0);
  VMnotify(player,"Total Teams: %d",VMSpace[ship].VMPerm.Eng.Systems[2].istat1);
  unused = VMSpace[ship].VMPerm.Eng.Systems[2].istat1 - VMSpace[ship].VMPerm.Eng.Systems[2].tistat1;
  VMnotify(player,"Teams in use: %d Teams free: %d",VMSpace[ship].VMPerm.Eng.Systems[2].tistat1,unused);
  VMnotify(player,"---------Team Status--------------------------");
  ResetWList(VMDamageQ);
  while(!AtEndWList(VMDamageQ)) {
    t_WEnt *tmp;
    tmp = (t_WEnt *) GetCurrWEnt(VMDamageQ);
    if (tmp->ship == ship) {
      switch (tmp->event) {
      case 0:

	VMnotify(player,"Team %d is working on %s (Helm #%d).",tmp->v1,VMSpace[ship].VMPerm.Helm.Systems[tmp->v2].name,tmp->v2);
	break;
      case 1:
	VMnotify(player,"Team %d is working on %s (Nav #%d).",tmp->v1,VMSpace[ship].VMPerm.Nav.Systems[tmp->v2].name,tmp->v2);
	break;
      case 2:

	VMnotify(player,"Team %d is working on %s (Eng #%d).",tmp->v1,VMSpace[ship].VMPerm.Eng.Systems[tmp->v2].name,tmp->v2);
	break;
      case 5:
	VMnotify(player,"Team %d is working on hull.",tmp->v1);
	break;
      }
      AdvanceWList(VMDamageQ);
    }
    else
      AdvanceWList(VMDamageQ);
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

void VMMiscRepair(int player,
		  int ship,
		  int team,
		  char *sysstr,
		  int number
		  ) {
  
  int actar = -1;
  
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
  if (team <= 0 || team > (VMSpace[ship].VMPerm.Eng.Systems[2].istat1)) {
    VMnotify(player,"Invalid team given");
    return;
  }
  ResetWList(VMDamageQ);
  while(!AtEndWList(VMDamageQ)) {
    t_WEnt *tmp;
    tmp = (t_WEnt *) GetCurrWEnt(VMDamageQ);
    if (tmp->v1 == team) {
      RemoveWEnt(VMDamageQ);
      VMSpace[ship].VMPerm.Eng.Systems[2].tistat1--;
    }
    AdvanceWList(VMDamageQ);
  }
  switch (actar) {
  case 0:
    if (number < 0 || number > HSystems) {
      VMnotify(player,"that system does not exist.");
      return;
    }
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[number].flags,EXISTS)) {
      VMnotify(player,"Team %d is now working on %s.",team,VMSpace[ship].VMPerm.Helm.Systems[number].name);
      AddWEnt(VMDamageQ,0,actar,ship,team,number,0,0);    
      VMSpace[ship].VMPerm.Eng.Systems[2].tistat1++;
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
      AddWEnt(VMDamageQ,0,actar,ship,team,number,0,0); 
      VMSpace[ship].VMPerm.Eng.Systems[2].tistat1++;   
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
      if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[number].flags,VMARMOR)) {
	VMnotify(player,"You cannot repair armor.");
	return;
      }
      VMnotify(player,"Team %d is now working on %s.",team,VMSpace[ship].VMPerm.Eng.Systems[number].name);
      VMSpace[ship].VMPerm.Eng.Systems[2].tistat1++;
      AddWEnt(VMDamageQ,0,actar,ship,team,number,0,0);    
      return;
    }
    else {
      VMnotify(player,"That system does not exist.");
    }
    break;
  }

  
}

void VMMiscRepairHull(int player,
		 int ship,
		 int team 
		 ) {
  if (team < 0 || team > VMSpace[ship].VMPerm.Eng.Systems[2].istat1) {
    VMnotify(player,"Invalid team given");
    return;
  }
  ResetWList(VMDamageQ);
  while(!AtEndWList(VMDamageQ)) {
    t_WEnt *tmp;
    tmp = (t_WEnt *) GetCurrWEnt(VMDamageQ);
    if (tmp->v1 == team) {
      RemoveWEnt(VMDamageQ);
      VMSpace[ship].VMPerm.Eng.Systems[2].tistat1--;
    }
    AdvanceWList(VMDamageQ);
  }
  AddWEnt(VMDamageQ,0,5,ship,team,0,0,0);       
  VMSpace[ship].VMPerm.Eng.Systems[2].tistat1++;
  VMnotify(player,"Team %d is now working on the hull.",team);
}

void VMSetDamVerbose(int player,
		int ship,
		int flag
		) {
  if(flag) { /* turn on */
    VMSpace[ship].VMPerm.Eng.Systems[2].tistat3 = player;
    VMnotify(player,"repair verbose mode engaged.");
  }
  else {
    VMSpace[ship].VMPerm.Eng.Systems[2].tistat3 = -1;
    VMnotify(player,"repair verbose mode disengaged");
  }
}


void 
VMGetShipOutDQ(int ship) {
  
  ResetWList(VMDamageQ);
  while(!AtEndWList(VMDamageQ)) {
    t_WEnt *tmp;
    tmp = (t_WEnt *) GetCurrWEnt(VMDamageQ);
    if (tmp->ship == ship) {
      RemoveWEnt(VMDamageQ);
      VMSpace[ship].VMPerm.Eng.Systems[2].tistat1--;
    }
    AdvanceWList(VMDamageQ);
  }
}


void
VMSpaceNotify(int ship, char *message, ... )
{
  DESC *d;
  int ShipNum;
  va_list list;
  char write_buf[10240];
#ifdef STDC_HEADERS
  va_start( list,message );
#else
	char *format;
	va_start(list);
	format=va_arg(list,char *);
#endif
  vsprintf( write_buf, message, list );
  /* sprintf doesn't guarentee string will be null terminated */
  write_buf[10239] = '\0';
  va_end( list );
  DESC_ITER_CONN(d) {
    if (Hidden(d->player))
      continue;
    ShipNum = VMShipNum(d->player);
    if (ship == ShipNum)
      notify(d->player,write_buf);
  }
}



void
VMSectorNotify(int  ship, 
	       int sector,
	       int res,
	       char *message, ... )
{
  va_list list;
  int i;
  char write_buf[10240];
  t_SpaceEnt *tmp;
#ifdef STDC_HEADERS
  va_start( list,message );
#else
	char *format;
	va_start(list);
	format=va_arg(list,char *);
#endif	

  vsprintf( write_buf, message, list );
  /* sprintf doesn't guarentee string will be null terminated */
  write_buf[10239] = '\0';
  va_end( list );
  ResetSpaceList(VMSectors[sector].VMSectorShips);
  while (!AtEndSpaceList(VMSectors[sector].VMSectorShips)) {
    tmp = (t_SpaceEnt *) GetCurrSpaceEnt(VMSectors[sector].VMSectorShips);
    if (tmp->VMShip != ship) {
      if (ContactMatrix[tmp->VMShip][ship] >= res)
	notify_all_from_inside(VMSpace[i].VMPerm.mainbridge,1, write_buf );
    }
    AdvanceSpaceList(VMSectors[sector].VMSectorShips);
    
  }
}


void VMSetupRooms() {
  /* requires that the zone of the mainbridge be the zone for the ENTIRE ship */
  /* this function adds rooms to the proper list and checks them for damage. */ 
  /* temp is a list of ship's to zone mappings */
  int i,mbr,zone;
  int ship;
  char damchar[1024];
  t_ManList *temp;
  t_ManEnt *tmp;
  temp = InitManList();
  if (temp == NULL) {
    return;
  }
  for (i = 0;i < VMCurrentMax;i++) {
    mbr = VMSpace[i].VMPerm.mainbridge; 
    VMSpace[i].VMTemp.rooms =(t_RoomList *) InitRoomList();
    zone = Zone(mbr);
#ifdef DEBUG
    VMnotify(35,"About to add ship %d with zone %d to temp manlist",i,zone);
#endif
    if (!IsPlayerInManList(temp,i)){
      AddManEnt(temp,i,zone);
#ifdef DEBUG
      VMnotify(35," addded ship %d with zone %d to temp manlist",i,zone);
#endif
    }
#ifdef DEBUG
    else
      VMnotify(35," didn't add ship %d with zone %d to temp manlist cuz it was already in there.",i,zone);
#endif
    
  }
  /* Now go thru the whole DB and set up Roomlists throughout the list */ 

  for(i=0;i<mudstate.db_top;i++) {
    if (isRoom(i)) {
      /* we are a room */
      if (IsRoomInManList(temp,Zone(i))) {
	/* we are a room in the zone that is attached to the current entry of temp */
	tmp = GetCurrManEnt(temp);
	if (tmp == NULL) {
	  break;
	}
	ship = tmp->VMPlayer;
	if (!IsRoomInRoomList(VMSpace[ship].VMTemp.rooms,i)) { 
	  AddRoomEnt(VMSpace[ship].VMTemp.rooms,i,0);
#ifdef DEBUG
	  VMnotify(35,"added room %d to ship %d's roomlist",i,ship);
#endif
	}
      }
    }
  }
}

void VMSciScanObject(int player, int ship, int tship) {
  int skillfact;
  /* first check equipment */ 
  if (!checkflag(VMSpace[ship].VMPerm.Nav.Systems[3].flags,EXISTS)) {
    VMnotify(player,"You have nothing to scan with.");
    return;
  }
  if (ContactMatrix[ship][tship] < 50) {
    VMnotify(player,"The contact is too hazy to scan.  Either move closer, or invest more power.");
    return;
  }
  skillfact = 1; /*a skill hook should go here */
  
  if (checkflag(VMSpace[ship].VMPerm.Flags,SIMSHIP)) {
    VMnotify(player,"Simships cant sciscan.  I dont know why not, but they cant! Go away.");
    return;
  }
  if (checkflag(VMSpace[tship].VMPerm.Flags,SHIP)) {
    VMnotify(player,"Scanning Ship(#%d)",tship);
    if (ContactMatrix[tship][ship] > 70) {
      VMnotifyAll(VMSpace[tship].VMPerm.mainbridge,"You have been scanned by Contact #%d",ship);
    }
    VMSciScanShip(player,ship,tship);
    return;
  }
  if (checkflag(VMSpace[tship].VMPerm.Flags,JUMPGATE)) {
    VMnotify(player,"Scanning Jumpgate(#%d)",tship);
    VMnotify(player,"Well George, I would have scanned a jumpgate. cept that is not implemented completely. yet.");
    return;
  }
  if (checkflag(VMSpace[tship].VMPerm.Flags,BASE)) {
    VMnotify(player,"Scanning Base(#%d)",tship);
    if (ContactMatrix[tship][ship] > 70) {
      VMnotifyAll(VMSpace[tship].VMPerm.mainbridge,"You have been scanned by Contact #%d",ship);
    }
    VMSciScanShip(player,ship,tship);
    /*VMnotify(player,"Well George, I would have scanned a base, cept that is not implemented completely. yet.");*/
    return;
  }
  if (checkflag(VMSpace[tship].VMPerm.Flags,NEBULA)) {
    VMnotify(player,"Scanning Nebula(#%d)",tship);
    VMnotify(player,"Well George, I would have scanned a nebula. cept that is not implemented completely. yet.");
    return;
  }
  if (checkflag(VMSpace[tship].VMPerm.Flags,PLANET)) {
    VMnotify(player,"Scanning Planet(#%d)",tship);
    VMnotify(player,"Well George, I would have scanned a planet. cept that is not implemented completely. yet.");
    return;
  }
  if (checkflag(VMSpace[tship].VMPerm.Flags,SUN)) {
    VMnotify(player,"Scanning Star(#%d)",tship);
    VMSciScanShip(player,ship,tship);
    /*VMnotify(player,"Well George, I would have scanned a star. cept that is not implemented completely. yet.");*/
    return;
  }
  if (checkflag(VMSpace[tship].VMPerm.Flags,BEACON)) {
    VMnotify(player,"Scanning beacon(#%d)",tship);
    VMnotify(player,"Well George, I would have scanned a beacon. cept that is not implemented completely. yet.");
    return;
  }
  
				
  VMnotify(player, "I dont know what the hell you just tried to scan, but i cant figure it out.  Maybe try tea leaves, or chicken entrails. ");
  return;

}

void 
VMMiscWeapScan(int player,
	       int sship, 
	       int tship
	       ) {
  int diff;
  int sc;
  int i;
  diff = 0;
  sc = 0;

  diff = 100 - ContactMatrix[sship][tship];
  sc = skill_check(player,SCISKILL,diff);

  if (sc < 0) { /* failure */
    VMnotify(player,"You fail to notice anything about the ship you tried to scan.");
    return;
  }
  /*  if (sc > 40) { * at this point we'll see the max *
    what = 4;
  }
  else if ( sc > 30) {
    what = 3;
  }
  else 
    what = 2;
    */
  VMnotify(player,"Tactical Scan on target %d",tship);
  for (i = 0; i < HSystems;i++) {
    if (VMWeaponErrorCheck(tship,player,i,LGUN,1)) {
      /* hopefully we're a gun */
      VMnotify(player,"Target system %d on helm is a gun.");
      if (checkflag(VMSpace[tship].VMPerm.Helm.Systems[i].flags,HASGUNPORT)) {
	VMnotify(player,"Gunport on Contact %d system %d is %s",tship,i,checkflag(VMSpace[tship].VMPerm.Helm.Systems[i].flags,GUNPORT) ? "open" : "closed");
      }
    }
  }
}





void VMSciScanShip(int player, int sship, int tship) {
  int pl,mpl, rooms;
  int er,erf;
  char *blah;
  er = 0;
  erf = 0;
  pl = 0;
  mpl = 0;
  rooms = 0;
  
  VMnotify(player,"Scientific Scan on Ship %s (%d)",VMSpace[tship].VMPerm.Name,tship);
  pl= CountPeople(VMSpace[tship].VMTemp.rooms,0);
  mpl= CountPeople(VMSpace[tship].VMTemp.rooms,1);
  rooms = GetRoomListSize(VMSpace[tship].VMTemp.rooms);
  er = ContactMatrix[sship][tship] / 10;
  erf = 5 - er;
  if (erf > 2) {
    erf -=5;
  }
  else
    erf = 0;
  
  if (erf > 0) {
    pl -= erf;
    mpl -= erf;
  }
  else if (erf < 0) {
    pl += erf;
    mpl += erf;
  }
  if (isRoom(VMSpace[tship].VMPerm.mainbridge)) {
    blah = (char *) vget_a(db[VMSpace[tship].VMPerm.mainbridge].zone,A_DESC);
  }
  else {
    blah = "You dont see anything special.";
  }
  VMnotify(player,"--------------------------------");
  VMnotify(player,"Contact: %d \n %s",tship,blah);
  VMnotify(player,"--------------------------------");
  VMnotify(player, "Found %d Rooms on target.",rooms);
  VMnotify(player,"Found %d lifeforms on target.",pl);
  VMnotify(player, "Found %d lifeforms on target awake.",mpl); 
}		




int CountPeople(t_RoomList *list,int flag) {
  t_RoomEnt *tmp;
  int depth = 0;
  int cnt = 0;
  int thing;
  if (list == NULL) {
    VMnotify(35,"List is NULL!");
    return -1;
  }
  ResetRoomList(list);
  while (!AtEndRoomList(list)) {
    if (depth > 20000) {
      break;
      VMcnotify("Infinite loop bailout in function VMCountPlayers");
    }
    tmp =(t_RoomEnt *) GetCurrRoomEnt(list);
    DOLIST(thing, Contents(tmp->room)) {
      if (isPlayer(thing)) {
	if (flag) {
	  if (Connected(thing))
	    cnt++;
	}
	else
	  cnt++;
      }
    }
    depth++;
    AdvanceRoomList(list);
  }
  return cnt;
}

void VMMineGoBoom(t_MineEnt *mine,
		  int ship,
		  int sector
		  ) {
  int mrng;
  int mis;
  int damage;
  double dist;
  int dm;
  t_SpaceEnt *tmp;
  mis = mine->missle;
  damage = AllMissles[mis].damage;
  mrng = (int)mine->mrng;
  ResetSpaceList(VMSectors[sector].VMSectorShips);
  while(!AtEndSpaceList(VMSectors[sector].VMSectorShips)) {
    tmp = GetCurrSpaceEnt(VMSectors[sector].VMSectorShips);
    dist = VMdistance(VMSpace[tmp->VMShip].VMPerm.Nav.XYZCoords,mine->loc);
    dm = VMMineDamageRange(dist,mrng,mis);
    if (dm > 0) { /* damage occured */
      if ( checkflag(AllMissles[mis].flags,STASIS)) {
	VMTakeDamage(tmp->VMShip,dm,2,VMGetArc2(mine->loc,tmp->VMShip));
      }
      else 
	VMTakeDamage(tmp->VMShip,dm,1,VMGetArc2(mine->loc,tmp->VMShip));
    }
    AdvanceSpaceList(VMSectors[sector].VMSectorShips);
  }
}

int VMMineDamageRange(double distance, 
		      int maxrange, 
		      int missile
		      ) {


  int damage;
  int div1,div2;
  double consta = VMMINECONST;

  damage = 0;
  if (consta==0) consta=.0001;
  div1 = (int) distance / consta;
  div2 = (int) pow(div1,3);
  if(div2==0) div2=1;
  damage =  AllMissles[missile].damage / div2;
  if (damage < 0) { /* huh? */
    return 0;
  }
  return damage;
}

int VMGetMineMaxRange(int mis
		      ) {

  double consta = VMMINECONST;
  int range = 0;
  int c3,r3;
 
  c3 = pow(consta,3);
  r3= AllMissles[mis].damage * c3;

  range = pow(r3,0.33333);
  if (range < 0)
    return 0;
  return range;
}



  
int VMShowSpaceAll(int player) { 

  /* this function will show (a wizard) all of space */ 

  double x,y,z;
  int i,depth;
  int sect;
  depth = 0; 
  for (i = 0; i < VMCurrentMax; i++) {
    if (depth > 10000) { 
      VMnotify(35,"inf loop protection, returning...");
      return;
    }
    
    x = VMSpace[i].VMPerm.Nav.XYZCoords.x;
    y = VMSpace[i].VMPerm.Nav.XYZCoords.y;
    z = VMSpace[i].VMPerm.Nav.XYZCoords.z;
    sect = VMSpace[i].VMTemp.TmpNav.Sector;
    VMnotify(player,"----------------------------------------------");
    VMnotify(player,"Data on Ship %s(%d)",VMSpace[i].VMPerm.Name,i);
    VMnotify(player,"Sector: %s(%d)",VMSpace[i].VMPerm.Name,i);
    VMnotify(player,"X: %12.2f Y: %12.2f Z: %12.2f",x,y,z);
    
    depth++;
  }
}

void VMScanPlayers() {

	int i,j;
	VMAllPlayers = InitSpaceList();

	for (i = 0;i < mudstate.db_top; i++) {
	
	/*	if (IsPlayer(i)) {
			if (QuietIcReg(i)) {
				AddSpaceEnt(VMAllPlayers,i);
			}
		}*/
	}
}


