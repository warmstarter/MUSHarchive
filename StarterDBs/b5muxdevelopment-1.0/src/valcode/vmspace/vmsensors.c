/* $Id: vmsensors.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

#include "header.h"

#define WARPX 100
#define WARPY 100
#define WARPZ 100
static char *rcsvmsensorsc="$Id: vmsensors.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";

t_Sector VMSectors[]={
  {0,"Uncharted",0,0,0,0.0,NULL,NULL,NULL},
  {1,"Hyperspace",0,0,0,0.0,NULL,NULL,NULL},
  {2,"Sim Space 1",0,0,0,0.0,NULL,NULL,NULL},
  {3,"Sim Space 2",0,0,0,0.0,NULL,NULL,NULL},
  {4,"Sim Space 3",0,0,0,0.0,NULL,NULL,NULL},
  {5,"Sim Space 4",0,0,0,0.0,NULL,NULL,NULL},
  {6,"Sim Space 5",0,0,0,0.0,NULL,NULL,NULL},
  {7,"Sol",10000000,-17000000,-18000000,10000000.0,NULL,NULL,NULL},
  {8,"Lacaille 9352",-15000000,-10000000,17000000,10000000.0,NULL,NULL,NULL},
  {9,"Zeta Tucanae",-15000000,8000000,-22000000,10000000.0,NULL,NULL,NULL},
  {10,"Chi Draconis A",14000000,5000000,20000000,10000000.0,NULL,NULL,NULL},
  {11,"DM+45 2505",29635314,-30106165,12533076,10000000.0,NULL,NULL,NULL},
  {12,"Alpha Omega",-30414531,28622256,-25272173,10000000.0,NULL,NULL,NULL},
  {-1,"invalid",0,0,0,-1.0,NULL,NULL,NULL}
};


/*
t_Sector VMSectors[]={
  {0,"Uncharted",0,0,0,0.0,NULL,NULL},
  {1,"Hyperspace",0,0,0,0.0,NULL,NULL},
  {2,"Sim Space 1",0,0,0,0.0,NULL,NULL},
  {3,"Sim Space 2",0,0,0,0.0,NULL,NULL},
  {4,"Sim Space 3",0,0,0,0.0,NULL,NULL},
  {5,"Sim Space 4",0,0,0,0.0,NULL,NULL},
  {6,"Sim Space 5",0,0,0,0.0,NULL,NULL},
  {7,"Sol",0,0,0,20000.0,NULL,NULL},
  {-1,"invalid",0,0,0,-1.0,NULL,NULL}
};
*/
int JPTMAX;
t_JPT JPT[500];

void VMSensorsInit()
{
  int i=0,j;
  JPTMAX=0;
  while(VMSectors[i].num!=-1) {
    VMSectors[i].VMSectorShips=(t_SpaceList *) InitSpaceList(); 
    VMSectors[i].VMSectorSpecials= (t_SpaceList *)  InitSpaceList(); 
    VMSectors[i].VMMines= (t_MineList *)  InitMineList(); 
    i++;
  }
  VMMaxSector=i-1;

  for(j=0;j<MAXSHIP;j++) {
    for(i=0;i<MAXSHIP;i++) {
      ContactMatrix[i][j]=0;
    }
  }
  for(i=0;i<=VMCurrentMax;i++) {
    if(checkflag(VMSpace[i].VMPerm.Flags,HYPERSPACE) &&
	!checkflag(VMSpace[i].VMPerm.Flags,DOCKED) ) {
      VMSpace[i].VMTemp.TmpNav.Sector=1;
      AddSpaceEnt(VMSectors[1].VMSectorShips,i);
      if(checkflag(VMSpace[i].VMPerm.Flags,SPECIAL)) 
	AddSpaceEnt(VMSectors[1].VMSectorSpecials,i);
    }
    if(checkflag(VMSpace[i].VMPerm.Flags,SIMSHIP)) {
      VMSpace[i].VMTemp.TmpNav.Sector=2;
      AddSpaceEnt(VMSectors[2].VMSectorShips,i);
	}
    else if(checkflag(VMSpace[i].VMPerm.Flags,SIMSHIP2)) {
      VMSpace[i].VMTemp.TmpNav.Sector=3;
      AddSpaceEnt(VMSectors[3].VMSectorShips,i);
	}
    else if(checkflag(VMSpace[i].VMPerm.Flags,SIMSHIP3)) {
      VMSpace[i].VMTemp.TmpNav.Sector=4;
      AddSpaceEnt(VMSectors[4].VMSectorShips,i);
	}
    else if(checkflag(VMSpace[i].VMPerm.Flags,SIMSHIP4)) {
      VMSpace[i].VMTemp.TmpNav.Sector=5;
      AddSpaceEnt(VMSectors[5].VMSectorShips,i);
	}
    else if(checkflag(VMSpace[i].VMPerm.Flags,SIMSHIP5)) {
      VMSpace[i].VMTemp.TmpNav.Sector=6;
      AddSpaceEnt(VMSectors[6].VMSectorShips,i);
	}


    else
      VMSetSector(i);
   }
 }


void VMSetSector(int ShipNum
		 ) {
  t_xyz tmp;
  int tmpSector,i;
  int flags;

  tmpSector=VMSpace[ShipNum].VMTemp.TmpNav.Sector;


  /*This if handles ships currently in NO sector such as recently
    undocked ship. It basically sticks any ship not in a sector
    into sector 0 to start with*/


  if(tmpSector > 0 && tmpSector < FIRSTSECTOR) 
    return;

  flags=VMSpace[ShipNum].VMPerm.Flags;

  if(checkflag(flags,DEAD) || checkflag(flags,DOCKED)) 
    return;
  /*This returns if a ship is in hyperspace or sim space*/



  if(tmpSector==-1) {
    if(checkflag(VMSpace[ShipNum].VMPerm.Flags,HYPERSPACE)) {
      VMSpace[ShipNum].VMTemp.TmpNav.Sector=1;
    AddSpaceEnt(VMSectors[1].VMSectorShips,ShipNum);
	return;
	}
    VMSpace[ShipNum].VMTemp.TmpNav.Sector=0;
    AddSpaceEnt(VMSectors[0].VMSectorShips,ShipNum);
    tmpSector=0;
    /*
      for(i=0;i<MAXSHIP;i++)
      ContactMatrix[ShipNum][i]=0;
      */

  }




/*	Odds are a ship is in the same sector it was last time so i 	
	check for this first to save time
*/
  if(tmpSector!=0) {
    tmp.x=VMSectors[tmpSector].X;
    tmp.y=VMSectors[tmpSector].Y;
    tmp.z=VMSectors[tmpSector].Z;
 
    if(VMdistance(VMSpace[ShipNum].VMPerm.Nav.XYZCoords,tmp)<=VMSectors[tmpSector].Range)
	return;
 
  }


  /*	If a ship is in sector 0 or a ship is not in the same
	sector is not in the range of the sector it was in last
	time we must now look thru every sector and compare it
	*/

  for(i=FIRSTSECTOR;i<=VMMaxSector;i++) {
    tmp.x=VMSectors[i].X;
    tmp.y=VMSectors[i].Y;
    tmp.z=VMSectors[i].Z;


    if(VMdistance(VMSpace[ShipNum].VMPerm.Nav.XYZCoords,tmp)<=VMSectors[i].Range) {
      if(IsShipInSpaceList(VMSectors[tmpSector].VMSectorShips,ShipNum))
	RemoveSpaceEnt(VMSectors[tmpSector].VMSectorShips);
      VMWipeShipContacts(ShipNum);
      VMSpace[ShipNum].VMTemp.TmpNav.Sector=i;
      AddSpaceEnt(VMSectors[i].VMSectorShips,ShipNum);
			
      return;
    }
  }


  /*	If a sector is not in ANY sector put it in 0. If it was already
	in 0. Just return
	*/


  if(VMSpace[ShipNum].VMTemp.TmpNav.Sector==0) 
    return;

  /*	If it wasnt in 0 last time we have to pull it out of
	whatever sector it was in
	*/

  VMWipeShipContacts(ShipNum);
      if(IsShipInSpaceList(VMSectors[tmpSector].VMSectorShips,ShipNum))
  	RemoveSpaceEnt(VMSectors[tmpSector].VMSectorShips);
  AddSpaceEnt(VMSectors[0].VMSectorShips,ShipNum);
  VMSpace[ShipNum].VMTemp.TmpNav.Sector=0;
}


void VMUpdateSensors() {

  t_SpaceEnt *tmp;
  
  ResetSpaceList(VMShipsPowered);
  while(!AtEndSpaceList(VMShipsPowered)) {
    tmp=GetCurrSpaceEnt(VMShipsPowered);
    if(tmp==NULL) 
      break;
    
    if(ShipActive(tmp->VMShip))
      VMUpdateShipsSensors(tmp->VMShip);
    AdvanceSpaceList(VMShipsPowered);
  }

}

void VMUpdateShipsSensors(int ShipNum)
{
  int tmpSector;
  t_SpaceEnt *tmp,*tmp3;
  t_MineEnt *tmp2;
  int orange;
  int tship,x=0;
  tmpSector=VMSpace[ShipNum].VMTemp.TmpNav.Sector;
  if(tmpSector==-1)
	{
		return;
	}
  ResetSpaceList(VMSectors[tmpSector].VMSectorShips);

  while(!AtEndSpaceList(VMSectors[tmpSector].VMSectorShips)) {
    tmp=GetCurrSpaceEnt(VMSectors[tmpSector].VMSectorShips);
    if(tmp==NULL) break;
    
     VMCheckScan(ShipNum,tmp->VMShip); 
    
    AdvanceSpaceList(VMSectors[tmpSector].VMSectorShips);
  }
  /* Mine code */
  if (VMSectors[tmpSector].VMMines != NULL) {
    if (GetMineListSize(VMSectors[tmpSector].VMMines) > 0) {
      ResetMineList(VMSectors[tmpSector].VMMines);
      
      while (!AtEndMineList(VMSectors[tmpSector].VMMines)) {
	tmp2 =(t_MineEnt *) GetCurrMineEnt(VMSectors[tmpSector].VMMines);
	if (tmp2 == NULL) 
	  break;
	orange = VMdistance(tmp2->loc,VMSpace[ShipNum].VMPerm.Nav.XYZCoords);
	if (orange < tmp2->mrng) { /* BOOM */
	  if (tmp2->launcher != ShipNum) {
	  VMMineGoBoom(tmp2,ShipNum,tmpSector);
	  ResetSpaceList(VMSectors[tmpSector].VMSectorShips);
	  while(!AtEndSpaceList(VMSectors[tmpSector].VMSectorShips)) {
	    tmp3 = GetCurrSpaceEnt(VMSectors[tmpSector].VMSectorShips);
	    tship = tmp3->VMShip;
	    if (MineInList(tmp2->timer,VMSpace[tship].VMTemp.TmpHelm.VMMines)){
	      RemoveMineEnt(VMSpace[tship].VMTemp.TmpHelm.VMMines);
	    }
	    AdvanceSpaceList(VMSectors[tmpSector].VMSectorShips);
	  }
	  x=1;
	  RemoveMineEnt(VMSectors[tmpSector].VMMines);
	  }
	}
      	if(x!=1) AdvanceMineList(VMSectors[tmpSector].VMMines);
	x=0;
      }
    }
   } 
    else
      VMnotify(35,"Null Minelist for Sector %d",tmpSector);
  }


void VMCheckScan(int ShipNum,int OtherShip)
{
  int oldres;
int newres;
char msg[500];
if(ShipNum==OtherShip) return;

oldres=ContactMatrix[ShipNum][OtherShip]; /* Store Current Resolution*/


/* TODO: Get New Resolution*/

newres=VMGetResolution(ShipNum,OtherShip);
ContactMatrix[ShipNum][OtherShip]=newres; 

if(newres > 25) {
	if(oldres > 25)	return;


/*func call for now in short rng*/
sprintf(msg,"Contact [%d] picked up on Short Sensors",OtherShip);
VMnotifyOfContact(ShipNum,OtherShip,msg);
	if(oldres < 1)
	AddSpaceEnt(VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips,OtherShip);

	return;	
	}

if(newres >= 1)
	{
	if(oldres >= 1 && oldres <= 25)	return;

	if(oldres > 25) {
		sprintf(msg,"Contact [%d] Lost on Short Sensors now on Long Sensors",OtherShip);
	}

/*func call for now in long range*/
	else
	{
		sprintf(msg,"Contact [???] picked up on Long Sensors");
		AddSpaceEnt(VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips,OtherShip);
	}
	VMnotifyOfContact(ShipNum,OtherShip,msg);

	return;	
	}

	if(oldres < 1) return;

	if(oldres >= 1 && oldres <= 25) {
if(IsShipInSpaceList(VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips,OtherShip))
	RemoveSpaceEnt(VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips);

/*func call for in long and now lost*/
if(VMSpace[ShipNum].VMTemp.TmpHelm.ADSOn > 1 && VMSpace[ShipNum].VMTemp.TmpHelm.ADSTarget==OtherShip) {
	ADS_NotifyLost(ShipNum,OtherShip);
}
sprintf(msg,"Contact [???] Lost");
VMnotifyOfContact(ShipNum,OtherShip,msg);

	}
		
	if(oldres > 25) {
if(IsShipInSpaceList(VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips,OtherShip))
	RemoveSpaceEnt(VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips);

/*func call for in short and now lost*/
sprintf(msg,"Contact [%d] Lost",OtherShip);
VMnotifyOfContact(ShipNum,OtherShip,msg);

	}


}




int VMCheckShortRange(int ShipNum,int OtherShip)
{
t_xyz xyz1,xyz2;

xyz1=VMSpace[ShipNum].VMPerm.Nav.XYZCoords;
xyz2=VMSpace[OtherShip].VMPerm.Nav.XYZCoords;

if(VMdistance(xyz1,xyz2) < VMSpace[ShipNum].VMPerm.Nav.Systems[0].tistat2)
	return 1;

return 0;
}




int 
VMCheckLongRange(int ShipNum,
		 int OtherShip
		 ) {
  t_xyz xyz1,xyz2;

  xyz1=VMSpace[ShipNum].VMPerm.Nav.XYZCoords;
  xyz2=VMSpace[OtherShip].VMPerm.Nav.XYZCoords;

  if(VMdistance(xyz1,xyz2) < VMSpace[ShipNum].VMPerm.Nav.Systems[0].tistat1)
    return 1;
  
  return 0;
}

int VMGetResolution(int ShipNum, 
		    int OtherShip
		    ) {

  t_xyz xyz1,xyz2;
  double dst,assigned,needed,factor,distt,res;
  int rmod,jmod,jpow,jmax;
  xyz1=VMSpace[ShipNum].VMPerm.Nav.XYZCoords;
  xyz2=VMSpace[OtherShip].VMPerm.Nav.XYZCoords;
 rmod=0; 
  /*
    if(VMdistance(xyz1,xyz2) < VMSpace[ShipNum].VMPerm.Nav.Systems[0].tistat2)
    return 50;

    if(VMdistance(xyz1,xyz2) < VMSpace[ShipNum].VMPerm.Nav.Systems[0].tistat1)
    return 5;
    */
  
  assigned=VMSpace[ShipNum].VMPerm.Nav.Systems[0].tdstat0 * VMSpace[ShipNum].VMTemp.TmpEng.PowerNav;
  needed=VMSpace[ShipNum].VMPerm.Nav.Systems[0].dstat3;
  distt=VMSpace[ShipNum].VMPerm.Nav.Systems[0].dstat1;
  if(needed==0) return 0;
  if(distt==0) return 0;
  factor=assigned / needed;
  if( factor > 1) 
    factor=1 + (assigned-needed) / (2 * needed);
  if( factor > 2) 
    factor=1 + (assigned-needed) / (3 * needed);
  if( factor > 3) 
    factor=1 + (assigned-needed) / (4 * needed);
  if(factor <= 0.0) 
    return 0.0;

  /*if(Hulls[VMSpace[OtherShip].VMPerm.HullType].Size<=200)
  factor=factor*(Hulls[VMSpace[OtherShip].VMPerm.HullType].Size/100.0);
  else 
  factor=factor*(Hulls[VMSpace[OtherShip].VMPerm.HullType].Size/200.0+.5);
*/
/*
  if(Hulls[VMSpace[OtherShip].VMPerm.HullType].Size>=100)
  factor=factor*(Hulls[VMSpace[OtherShip].VMPerm.HullType].Size/10000.0+.99);
*/

  if(Hulls[VMSpace[OtherShip].VMPerm.HullType].Size<=400)
  distt=distt*(Hulls[VMSpace[OtherShip].VMPerm.HullType].Size/100.0);
  else 
  distt=distt*(Hulls[VMSpace[OtherShip].VMPerm.HullType].Size/4000.0+.99975);

  dst=VMdistance(xyz1,xyz2);
  if(checkflag(VMSpace[OtherShip].VMPerm.Flags,JUMPGATE)) {
		dst=dst/3.0;
	}
if(distt!=0 && factor!=0) 
  res=100.0 - (50.0 / (factor*distt) ) * dst;
  /*res=factor*200.0 - (150.0 / distt) * dst;*/
else res=0;
  if(res > 100) res=100.0;
  /*
    if(VMdistance(xyz1,xyz2) < 100)
    return 50;

    if(VMdistance(xyz1,xyz2) < 500)
    return 5;
    */
if(checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[4].flags,EXISTS)) {
  assigned=VMSpace[ShipNum].VMPerm.Nav.Systems[4].tdstat0 * VMSpace[ShipNum].VMTemp.TmpEng.PowerNav;
  jmod=VMSpace[ShipNum].VMPerm.Nav.Systems[4].istat1;
  jpow=VMSpace[ShipNum].VMPerm.Nav.Systems[4].istat2;
  jmax=VMSpace[ShipNum].VMPerm.Nav.Systems[4].istat3;
  if(jpow==0) rmod=0;
  else {
  	rmod=(assigned/jpow)*jmod;
	if(rmod > jmax) rmod=jmax;

	}
  res-=rmod;
  if(res < 0) res=0;
}

  if(checkflag(VMSpace[OtherShip].VMPerm.Flags,JUMPGATE)) {
	res*=2.1;
	if(res > 100) res=100;
	}

return res;

}



void VMUpdateSpecials()
{
t_SpaceEnt *tmp1,*tmp2;
t_SpaceList *list1,*list2;
int k,i=0;

for(i=0;i<JPTMAX;i++) {
	JPT[i].timeleft--;
	if(JPT[i].timeleft <=0) {
		JPT[i]=JPT[JPTMAX];
		JPTMAX--;
	}
}
while(VMSectors[i].num!=-1) {
list2=VMSectors[i].VMSectorSpecials;
list1=VMSectors[i].VMSectorShips;

ResetSpaceList(list1);

while(!AtEndSpaceList(list1)) {
 tmp1=GetCurrSpaceEnt(list1);
 if(tmp1==NULL) break;

for(k=0;k<JPTMAX;k++)  {
	if(JPT[k].sector==VMSpace[tmp1->VMShip].VMTemp.TmpNav.Sector)
		VMCheckTmpSpecial(k,tmp1->VMShip);
	}

 ResetSpaceList(list2);
  while(!AtEndSpaceList(list2)) {
	tmp2=GetCurrSpaceEnt(list2);
 	if(tmp2==NULL) break;

	VMCheckSpecial(tmp2->VMShip,tmp1->VMShip);

	AdvanceSpaceList(list2);
	}
  AdvanceSpaceList(list1);
  }

  i++;
 }

}

void VMCheckSpecial(int Special,int Ship)
{
int tmpSector;
t_xyz xyz;
t_sph sph;

return;	/* Special Code disabled now*/
if(Special==Ship) return;
if(!checkflag(VMSpace[Special].VMPerm.Flags,SPECIAL)) return;

/*
if(VMdistance(VMSpace[Special].VMPerm.Nav.XYZCoords,VMSpace[Ship].VMPerm.Nav.XYZCoords) <= VMSpace[Special].VMPerm.Nav.Systems[0].istat2) {
*/
if(VMdistance(VMSpace[Special].VMPerm.Nav.XYZCoords,VMSpace[Ship].VMPerm.Nav.XYZCoords) <= 5) {

  if(checkflag(VMSpace[Special].VMPerm.Flags,JUMPGATE)) {

if(VMSpace[Ship].VMPerm.Nav.Systems[1].tdstat2 < 1 || !HeadingMyWay(Special,Ship)) return;


VMnotifymans(VMSpace[Ship].VMTemp.TmpNav.VMMan_List,"\n\t\tYou have entered the Jumpgate\n");


xyz=VMSpace[Special].VMPerm.Nav.XYZCoords;
VMInternalJumpShip(Ship,xyz);
	return;
	}
/*if nebula etc*/

   }
}	
	

void VMSetupSpecials()
{
int i;


for(i=0;i<=VMCurrentMax;i++) {

if(checkflag(VMSpace[i].VMPerm.Flags,JUMPGATE)) {
	VMSetSector(i);
if(!IsShipInSpaceList(VMSectors[VMSpace[i].VMTemp.TmpNav.Sector].VMSectorSpecials,i))
AddSpaceEnt(VMSectors[VMSpace[i].VMTemp.TmpNav.Sector].VMSectorSpecials,i);
  }
 }
}


void VMWipeOneShipsContacts(int ShipNum)
{
SemiKillSpaceList(VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips,ShipNum,SHORT);
/*SemiKillSpaceList(VMSpace[ShipNum].VMTemp.TmpNav.VMLongShips,ShipNum,LONG);
*/
}

void VMWipeShipContacts(int ShipNum)
{
  t_SpaceEnt *tmp;
  
  SemiKillSpaceList(VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips,ShipNum,SHORT);
  /*
    SemiKillSpaceList(VMSpace[ShipNum].VMTemp.TmpNav.VMLongShips,ShipNum,LONG);
    */

  ResetSpaceList(VMShipsPowered);
  while(!AtEndSpaceList(VMShipsPowered)) {
    tmp=GetCurrSpaceEnt(VMShipsPowered);
    if(tmp==NULL) break;

    if(ContactMatrix[tmp->VMShip][ShipNum]>=1) {
      if(IsShipInSpaceList(VMSpace[tmp->VMShip].VMTemp.TmpNav.VMShortShips,ShipNum))
	RemoveSpaceEnt(VMSpace[tmp->VMShip].VMTemp.TmpNav.VMShortShips);
	}

/*
if(checkflag(ContactMatrix[tmp->VMShip][ShipNum],LONG)) {
if(IsShipInSpaceList(VMSpace[tmp->VMShip].VMTemp.TmpNav.VMLongShips,ShipNum))
    RemoveSpaceEnt(VMSpace[tmp->VMShip].VMTemp.TmpNav.VMLongShips);
	}
*/

ContactMatrix[tmp->VMShip][ShipNum]=0;

    AdvanceSpaceList(VMShipsPowered);
	}

}



int 
HeadingMyWay(int Ship1, 
	     int Ship2
	     ) {
  t_sph sph;
  double delta1,delta2;
  sph=VMRelative(Ship2,Ship1);
  delta1=fabs(VMSpace[Ship2].VMPerm.Nav.Alpha-sph.bearing);
  delta2=fabs(VMSpace[Ship2].VMPerm.Nav.Beta-sph.elevation);
  if (delta1 + delta2  < 22) 
    return 1;
  return 0;
}

/* cheesy copy of this function for use in sweep fire */


int 
HeadingMyWay2(int Ship1, 
	     int Ship2,
	     int diff
	     ) {
  t_sph sph;
  double delta1,delta2;
  sph=VMRelative(Ship2,Ship1);
  delta1=fabs(VMSpace[Ship2].VMPerm.Nav.Alpha-sph.bearing);
  delta2=fabs(VMSpace[Ship2].VMPerm.Nav.Beta-sph.elevation);
  if (delta1 + delta2  < diff) 
    return 1;
  return 0;
}

void VMCheckTmpSpecial(int Special,int Ship)
{
int tmpSector;
t_xyz xyz;
t_sph sph;


if(VMdistance(JPT[Special].XYZ,VMSpace[Ship].VMPerm.Nav.XYZCoords) <= 5) {

/* They are ALL Jumpgates*/

if(VMSpace[Ship].VMPerm.Nav.Systems[1].tdstat2 < 1 || !HeadingMyWayTmp(Special,Ship)) return;


VMnotifymans(VMSpace[Ship].VMTemp.TmpNav.VMMan_List,"\n\t\tYou have entered the Jumpgate\n");

xyz=JPT[Special].XYZ;
VMInternalJumpShip(Ship,xyz);
	}
}	
	
int 
HeadingMyWayTmp(int Special, 
	     int Ship
	     ) {
  t_sph sph;
  double delta1,delta2;
  sph=VMRelative2(Ship,JPT[Special].XYZ);
  delta1=fabs(VMSpace[Ship].VMPerm.Nav.Alpha-sph.bearing);
  delta2=fabs(VMSpace[Ship].VMPerm.Nav.Beta-sph.elevation);
  if (delta1 + delta2  < 22) 
    return 1;
  return 0;
}

void VMInternalJumpShip(int Ship,t_xyz XYZ)
{
t_xyz xyz;
t_sph sph;
int tmpSector;
char bf[40];

sprintf(bf,"has jumped out of this space.");
VMnotifyShips(Ship,bf);
VMWipeShipContacts(Ship);

	if((tmpSector=VMSpace[Ship].VMTemp.TmpNav.Sector)==1) {

/*xyz=VMSpace[Special].VMPerm.Nav.XYZCoords;*/
xyz=XYZ;
xyz.x=xyz.x*WARPX;
xyz.y=xyz.y*WARPY;
xyz.z=xyz.z*WARPZ;
sph=VMxyz_to_sph(xyz);
VMSpace[Ship].VMPerm.Nav.XYZCoords=xyz;
VMSpace[Ship].VMPerm.Nav.SPHCoords=sph;

		VMSpace[Ship].VMTemp.TmpNav.Sector=-1;
VMSpace[Ship].VMPerm.Flags=clearflag(VMSpace[Ship].VMPerm.Flags,HYPERSPACE);
if(IsShipInSpaceList(VMSectors[1].VMSectorShips,Ship))
	RemoveSpaceEnt(VMSectors[1].VMSectorShips);
	VMSetSector(Ship);
	return;
	}

	tmpSector=VMSpace[Ship].VMTemp.TmpNav.Sector;
	VMSpace[Ship].VMTemp.TmpNav.Sector=1;
/*xyz=VMSpace[Special].VMPerm.Nav.XYZCoords;*/
xyz=XYZ;
xyz.x=xyz.x/WARPX;
xyz.y=xyz.y/WARPY;
xyz.z=xyz.z/WARPZ;
sph=VMxyz_to_sph(xyz);
VMSpace[Ship].VMPerm.Nav.XYZCoords=xyz;
VMSpace[Ship].VMPerm.Nav.SPHCoords=sph;
VMSpace[Ship].VMPerm.Flags=setflag(VMSpace[Ship].VMPerm.Flags,HYPERSPACE);
if(IsShipInSpaceList(VMSectors[tmpSector].VMSectorShips,Ship))
	RemoveSpaceEnt(VMSectors[tmpSector].VMSectorShips);
	AddSpaceEnt(VMSectors[1].VMSectorShips,Ship);

}

