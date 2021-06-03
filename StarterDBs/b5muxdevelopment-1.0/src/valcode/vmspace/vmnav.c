/* $Id: vmnav.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

#include "header.h"
static char *rcsvmnavc="$Id: vmnav.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
int CanAccessNav2(int ship,int playerr)
{

  if(!ShipActive(ship)) {
        VMnotify(playerr,"This ship is not in space");
        return 0; 
        }

  if(checkflag(VMSpace[ship].VMPerm.Flags,DEAD))
  	{
		VMnotify(playerr,"This is destroyed");
		return 0;
	}


  if (VMGlobalSpaceInit == 0) {
    VMnotify(playerr,"Space has not been initialized.");
    return 0;
  }
  
  if(ship < 0 || ship > VMCurrentMax) 
    {
      VMnotify(playerr,"Danger Danger. Possible Bug!");
      return 0;
    }
	

  if(!IsPlayerInManList(VMSpace[ship].VMTemp.TmpNav.VMMan_List,playerr))
    {
      return 0;
      
    }
  return 1;
}



int CanAccessNavDock(int ship,int playerr)
{

  if(!checkflag(VMSpace[ship].VMPerm.Flags,DOCKED))
  	{
		VMnotify(playerr,"This ship is not docked");
		return 0;
	} 

  if(checkflag(VMSpace[ship].VMPerm.Flags,DEAD))
  	{
		VMnotify(playerr,"This is destroyed");
		return 0;
	}


  if (VMGlobalSpaceInit == 0) {
    VMnotify(playerr,"Space has not been initialized.");
    return 0;
  }
  
  if(ship < 0 || ship > VMCurrentMax) 
    {
      VMnotify(playerr,"Danger Danger. Possible Bug!");
      return 0;
    }
	

  if(!IsPlayerInManList(VMSpace[ship].VMTemp.TmpNav.VMMan_List,playerr))
    {
      VMnotify(playerr,"You must man the nav console");
      return 0;
      
    }
  return 1;
}





int CanAccessNav(int ship,int playerr)
{

  if(!ShipActive(ship)) {
        VMnotify(playerr,"This ship is not in space");
        return 0; 
        }

  if (VMGlobalSpaceInit == 0) {
    VMnotify(playerr,"Space has not been initialized.");
    return 0;
  }
  
  if(ship < 0 || ship > VMCurrentMax) 
    {
      VMnotify(playerr,"Danger Danger. Possible Bug!");
      return 0;
    }
	

  if(!IsPlayerInManList(VMSpace[ship].VMTemp.TmpNav.VMMan_List,playerr))
    {
      VMnotify(playerr,"You must man the nav console");
      return 0;
      
    }
  return 1;
}

double VMGetCoordsx(ShipNum)
int ShipNum;
{
if(ShipNum < 0 || ShipNum > VMCurrentMax)
	return 0;
return(VMSpace[ShipNum].VMPerm.Nav.XYZCoords.x);
}

double VMGetCoordsy(ShipNum)
int ShipNum;
{
if(ShipNum < 0 || ShipNum > VMCurrentMax)
	return 0;
return(VMSpace[ShipNum].VMPerm.Nav.XYZCoords.y);
}

double VMGetCoordsz(ShipNum)
int ShipNum;
{
if(ShipNum < 0 || ShipNum > VMCurrentMax)
	return 0;
return(VMSpace[ShipNum].VMPerm.Nav.XYZCoords.z);
}


void VMAP(player,ShipNum,x,y,z)
int player,ShipNum;
double x,y,z;
{

  VMSpace[ShipNum].VMTemp.TmpNav.XYZCoords.x=x;
  VMSpace[ShipNum].VMTemp.TmpNav.XYZCoords.y=y;
  VMSpace[ShipNum].VMTemp.TmpNav.XYZCoords.z=z;
  VMnotify(player,tprintf("The Navigational computer inputs the course. Engage the Engines."));
  VMSpace[ShipNum].VMTemp.TmpNav.APON=1;


}

void VMADoor(player,ShipNum,DestShip,Bay,code)
int player,ShipNum,DestShip,Bay,code;
{
t_SpaceList *tmpbays;
t_SpaceEnt *tmp;
char *buff,bf[100];
int code2,i;

  if(!CanAccessNav(ShipNum,player)) 
    return;
if(VMdistance(VMSpace[ShipNum].VMPerm.Nav.XYZCoords, VMSpace[DestShip].VMPerm.Nav.XYZCoords) > 20)
	{
		VMnotify(player,"That Ship is not within range");
		return;
	}
if(Bay < 0 || Bay > GetSpaceListSize(VMSpace[DestShip].VMTemp.TmpNav.VMDockList))
	{
		VMnotify(player,"Invalid Bay Number");
		return;
	}
tmpbays=VMSpace[DestShip].VMTemp.TmpNav.VMDockList;
ResetSpaceList(tmpbays);
for(i=0;i<Bay;i++) {
	if(AtEndSpaceList(tmpbays)) break;
	AdvanceSpaceList(tmpbays);
	}
	tmp=GetCurrSpaceEnt(tmpbays);
	if(tmp==NULL) 
		{
		VMnotify(player,"Unknown Error");
		return;
	}
/*
buff=vfetch_attribute(tmp->VMShip,"SHIPNUM");
code2=atoi(buff);*/
code2=GetCode(tmp->VMShip);
if(code!=code2) {
	VMnotify(player,"Invalid Code");
	return;}

/*buff=vfetch_attribute(tmp->VMShip,"VA");
code2=atoi(buff);*/
code2=VMDoorsOpen(tmp->VMShip);
if(code2==1) {
	sprintf(bf,"0");
	atr_add_raw(tmp->VMShip,GetDoorAttrib(tmp->VMShip),bf);
	VMnotify(player,"Door Bays Closed");
	}
if(code2==0) {
	sprintf(bf,"1");
	atr_add_raw(tmp->VMShip,GetDoorAttrib(tmp->VMShip),bf);
	VMnotify(player,"Door Bays Open");
	}
}

void VMDock(player,ShipNum,DestShip,Bay)
int player,ShipNum,DestShip,Bay;

{
t_SpaceList *tmpbays;
t_SpaceEnt *tmp;
int i,flags1,flags2;
int tmpsector;
int airlockout,airlockin;
dbref exitloc;

  if(!CanAccessNav(ShipNum,player)) 
    return;

if(DestShip < 0  || DestShip > VMCurrentMax)
	{
		VMnotify(player,"Invalid Ship");
		return;
	}
tmpsector=VMSpace[ShipNum].VMTemp.TmpNav.Sector;
if(tmpsector>1 && tmpsector < 7) {
	VMnotify(player,"You cannot dock when in sim space");
	return;
	}

if(IsShipInSpaceList(VMShipsPowered,ShipNum))
	{
		VMnotify(player,"Reactor must be off.");
		return;
	}

/*if(!IsShipInSpaceList(VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips,DestShip))*/
if(VMdistance(VMSpace[ShipNum].VMPerm.Nav.XYZCoords, VMSpace[DestShip].VMPerm.Nav.XYZCoords) > 8)
	{
		VMnotify(player,"That Ship is not within range");
	if(Wizard(player) )
	VMnotify(player,"***Invoking Wizard Privilege and Docking anyways.***");
	else
		return;
	}
if(Bay < 0 || Bay > GetSpaceListSize(VMSpace[DestShip].VMTemp.TmpNav.VMDockList))
	{
		VMnotify(player,"Invalid Bay Number");
		return;
	}




flags1=VMSpace[ShipNum].VMPerm.Flags;
flags2=VMSpace[DestShip].VMPerm.Flags;

if(!checkflag(flags1,SHIP) && !checkflag(flags1,PLANET) && !checkflag(flags1,BASE) && !checkflag(flags1,SMALLSHIP))
	{
		VMnotify(player,"This thing doesnt dock");
		return;
	}
if(!checkflag(flags2,SHIP) && !checkflag(flags2,PLANET) && !checkflag(flags2,BASE) && !checkflag(flags2,SMALLSHIP))
	{
		VMnotify(player,"That thing doesnt allow docking");
		return;
	}


if(checkflag(flags2,PLANET) && !checkflag(flags1,CANLAND)) 
	{
		VMnotify(player,"This ship cant land");
		return;
	}
if(!checkflag(flags2,PLANET) && !checkflag(flags1,CANDOCK)) 
	{
		VMnotify(player,"This ship cant dock");
		return;
	}
if(!checkflag(flags2,PLANET) && checkflag(flags1,SHIP) && !checkflag(flags2,HOLDBIGSHIPS)) 
	{
		VMnotify(player,"This ship is too big to dock there");
		return;
	}
if(!checkflag(flags2,PLANET) && checkflag(flags1,SMALLSHIP) && !checkflag(flags2,HOLDSMALLSHIPS)) 
	{
		VMnotify(player,"This ship cant dock there");
		return;
	}



tmpbays=VMSpace[DestShip].VMTemp.TmpNav.VMDockList;
ResetSpaceList(tmpbays);
for(i=0;i<Bay;i++) {
	if(AtEndSpaceList(tmpbays)) break;
	AdvanceSpaceList(tmpbays);
	}
	tmp=GetCurrSpaceEnt(tmpbays);
	if(tmp==NULL) 
		{
		VMnotify(player,"Unknown Error");
		return;
	}

if(!VMDoorsOpen(tmp->VMShip))
	{
		VMnotify(player,"The Bay Doors are Closed");
	if(Wizard(player) )
	VMnotify(player,"***Invoking Wizard Privilege and Docking anyways.***");
	else
		return;
	}



airlockin=VMSpace[ShipNum].VMTemp.TmpNav.airlockin;
airlockout=VMSpace[ShipNum].VMTemp.TmpNav.airlockout;
if(airlockin <= 0 || airlockout > mudstate.db_top  || airlockout <=0 || airlockout > mudstate.db_top  ){
		VMnotify(player,"Airlock Not Configured Error");
		return;
	}
	
if(!VMAddToBay(tmp->VMShip))
	{
		VMnotify(player,"There is no more room in that area");
		return;
	}

VMSpace[ShipNum].VMPerm.Flags=setflag(VMSpace[ShipNum].VMPerm.Flags,DOCKED);

VMWipeShipContacts(ShipNum);
tmpsector=VMSpace[ShipNum].VMTemp.TmpNav.Sector;
if(IsShipInSpaceList(VMSectors[tmpsector].VMSectorShips,ShipNum))
	RemoveSpaceEnt(VMSectors[tmpsector].VMSectorShips);
for(i=0;i < NSystems;i++) {
if(checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[i].flags,TRACT))
VMSpace[ShipNum].VMPerm.Nav.Systems[i].tistat2=0;
}
VMnotify(player,"The Ship is Now Docked ");

exitloc=Exits(airlockin);
s_Exits(exitloc,remove_first(Exits(exitloc),airlockin));
s_Exits(tmp->VMShip,insert_first(Exits(tmp->VMShip),airlockin));
s_Exits(airlockin,tmp->VMShip);
s_Location(airlockout,tmp->VMShip);

/*VMSpace[ShipNum].VMPerm.Nav.Dock=VMShipFromRoom(tmp->VMShip);*/
VMSpace[ShipNum].VMPerm.Nav.Dock=tmp->VMShip;

}


int IsHyper(Ship)
int Ship;
{
if(Ship< 0 || Ship> VMCurrentMax)
	return -1;
if(checkflag(VMSpace[Ship].VMPerm.Flags,HYPERSPACE))
	return 1;

return 0;
}
void VMUnDock(player,ShipNum)
int player,ShipNum;
{
int airlockout,airlockin,dnum;
dbref exitloc;

/* JohnD - commented out until it can be fixed
  if(!CanAccessNavDock(ShipNum,player)) 
    return;
  if (!(VMSpace[ShipNum].VMPerm.Eng.Systems[0].dstat4 > 0) ) {
    VMnotify(player,"This ship is out of gas, so can't undock.");
    return;
  }
*/

if(!checkflag(VMSpace[ShipNum].VMPerm.Flags,DOCKED)) {
	VMnotify(player,"The Ship is not Docked");
	return;
}
if(!VMDoorsOpen(VMSpace[ShipNum].VMPerm.Nav.Dock))
	{
		VMnotify(player,"The Bay Doors are Closed");
	if(Wizard(player) )
	VMnotify(player,"***Invoking Wizard Privilege and Docking anyways.***");
	else
		return;
	}

dnum=VMShipFromRoom(VMSpace[ShipNum].VMPerm.Nav.Dock);
if(checkflag(VMSpace[dnum].VMPerm.Flags,DOCKED)) {
	VMnotify(player,"This Ship is Docked in a Docked Ship");
	return;
}


if(checkflag(VMSpace[dnum].VMPerm.Flags,HYPERSPACE))
  VMSpace[ShipNum].VMPerm.Flags=setflag(VMSpace[ShipNum].VMPerm.Flags,HYPERSPACE);
else
  VMSpace[ShipNum].VMPerm.Flags=clearflag(VMSpace[ShipNum].VMPerm.Flags,HYPERSPACE);

VMSpace[ShipNum].VMPerm.Flags=clearflag(VMSpace[ShipNum].VMPerm.Flags,DOCKED);
VMSpace[ShipNum].VMPerm.Nav.XYZCoords=VMSpace[dnum].VMPerm.Nav.XYZCoords;
VMSpace[ShipNum].VMTemp.TmpNav.Sector=-1;
 
if(checkflag(VMSpace[dnum].VMPerm.Flags,HYPERSPACE)) 
VMSpace[ShipNum].VMPerm.Flags=setflag(VMSpace[ShipNum].VMPerm.Flags,HYPERSPACE);
else
VMSpace[ShipNum].VMPerm.Flags=clearflag(VMSpace[ShipNum].VMPerm.Flags,HYPERSPACE);

VMSetSector(ShipNum);
VMnotify(player,"The Ship is Now UnDocked");
airlockin=VMSpace[ShipNum].VMTemp.TmpNav.airlockin;
airlockout=VMSpace[ShipNum].VMTemp.TmpNav.airlockout;

exitloc=Exits(airlockin);
s_Exits(exitloc,remove_first(Exits(exitloc),airlockin));
s_Exits(SPACEROOM,insert_first(Exits(SPACEROOM),airlockin));
s_Exits(airlockin,SPACEROOM);

s_Location(airlockout,Exits(airlockout));
VMSubToBay(VMSpace[ShipNum].VMPerm.Nav.Dock);
VMSBGetShipOutDQ(ShipNum);

if(checkflag(VMSpace[ShipNum].VMPerm.Eng.Systems[1].flags,EXISTS)) {
  VMSpace[ShipNum].VMPerm.Eng.Systems[1].istat1=VMSpace[ShipNum].VMPerm.Eng.Systems[1].istat4;
}
if(checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[5].flags,EXISTS)) {
  VMSpace[ShipNum].VMPerm.Nav.Systems[5].tistat2=VMSpace[ShipNum].VMPerm.Nav.Systems[5].istat2;
}



}



void VMNavShowBays(player,ShipNum,destship)
int player;
int ShipNum,destship;
{

  if(!CanAccessNav(ShipNum,player)) 
    return;

if(destship < 0 || destship > VMCurrentMax)
	{
	VMnotify(player,"Invalid Ship");
	return;
	}

VMNavBays(player,destship,1);

}


void VMNavBays(player,ShipNum,which)
int player;
int ShipNum;
int which;
{
t_SpaceEnt *tmp;
t_SpaceList *tmpbays;
char stat[20];

int i=0;

if(which==0){
  if(!CanAccessNav(ShipNum,player)) 
    return;
 }

tmpbays=VMSpace[ShipNum].VMTemp.TmpNav.VMDockList;

ResetSpaceList(tmpbays);

VMnotify(player,"Docking Bays on %s [%d]",VMSpace[ShipNum].VMPerm.Name,ShipNum);
VMnotify(player,"--------------------------------------------------------------------");



while(!AtEndSpaceList(tmpbays)) {
	tmp=GetCurrSpaceEnt(tmpbays);
	if(tmp==NULL) break;

if(VMDoorsOpen(tmp->VMShip))
	sprintf(stat,"OPEN");
else
	sprintf(stat,"CLOSED");

	VMnotify(player,"%2d) %30s  DOORS: %8s  VACANCIES: %d",i,Name(tmp->VMShip),stat,VMHowManyLeft(tmp->VMShip));
	i++;
	AdvanceSpaceList(tmpbays);
  }

}

int GetCode(int roomnum)
{
char *buff;
buff=vfetch_attribute(roomnum,"XTYPE");
if(strcmp(buff,"SHIPDOCK")==0) 
	return(atoi(vfetch_attribute(roomnum,"SHIPNUM")));
else if(strcmp(buff,"SBDOCK")==0) 
	return(atoi(vget_a(roomnum,A_VA+1)));
	/*return(atoi(vfetch_attribute(tmp->VMShip,"VB"))*/
else 
	return(atoi(vfetch_attribute(roomnum,"SHIPNUM")));

}

int GetDoorAttrib(int ShipNum)
{
char *buff;
buff=vfetch_attribute(ShipNum,"XTYPE");
if(strcmp(buff,"SBDOCK")==0) return(A_VA);
if(strcmp(buff,"SHIPDOCK")==0) return(A_VA+12);
return(A_VA);
}

int VMDoorsOpen(int ShipNum)
{
if(atoi(vget_a(ShipNum,GetDoorAttrib(ShipNum)))==1)
	return 1;
	return 0;
}

void VMSubToBay(int BayNum)
{
int num;
char tmp[30];
char *buff;
int rank;

rank=0;
buff=vfetch_attribute(BayNum,"XTYPE");
if(strcmp(buff,"SHIPDOCK")==0) {
	num=atoi(vget_a(BayNum,RANK_A));
	rank=1;
	}
else
	num=atoi(vget_a(BayNum,SKILL_A));

num++;
sprintf(tmp,"%d",num);
if(rank==1)
atr_add_raw(BayNum,RANK_A,tmp);
else
atr_add_raw(BayNum,SKILL_A,tmp);
}


int VMHowManyLeft(int BayNum)
{
char *buff;

buff=vfetch_attribute(BayNum,"XTYPE");
if(strcmp(buff,"SHIPDOCK")==0) {
	return(atoi(vget_a(BayNum,RANK_A)));
}

return(atoi(vget_a(BayNum,SKILL_A)));

}

int VMAddToBay(int BayNum)
{
int num;
char tmp[30];
char *buff;
int rank;

rank=0;
buff=vfetch_attribute(BayNum,"XTYPE");
if(strcmp(buff,"SHIPDOCK")==0) {
	num=atoi(vget_a(BayNum,RANK_A));
	rank=1;
}
else
num=atoi(vget_a(BayNum,SKILL_A));

if(num <= 0) {
	return 0;
	}


num--;
sprintf(tmp,"%d",num);
if(rank==1)
atr_add_raw(BayNum,RANK_A,tmp);
else
atr_add_raw(BayNum,SKILL_A,tmp);
return 1;
}



int VMCanDoors(int ShipNum)
{

if(atoi(vget_a(ShipNum,A_VA+1))==1)
	return 1;
	return 0;
}


void VMPoint(player,ShipNum,Ship)
int player;
int ShipNum;
{
t_SpaceList *tmpshort;
double x,y,z;
t_xyz xyz1,xyz2;
t_sph sph;
int per;
char bf[200];

if(Ship < 0 || Ship > JPTMAX) {
	notify(player,"Invalid Jump Point");
	return;
}
xyz1=VMSpace[ShipNum].VMPerm.Nav.XYZCoords;
xyz2=JPT[Ship].XYZ;
	if(JPT[Ship].sector==VMSpace[ShipNum].VMTemp.TmpNav.Sector && VMdistance(JPT[Ship].XYZ,VMSpace[ShipNum].VMPerm.Nav.XYZCoords) > 5) {

/*xyz1=VMSpace[ShipNum].VMPerm.Nav.XYZCoords;
xyz2=VMSpace[Ship].VMPerm.Nav.XYZCoords;
	if(VMdistance(xyz1,xyz2) > 5) { */
                notify(player,"You are not close enough to that Jump Point");
		return;
	}
notify(player,"\n\t\tYou have entered the Jump Gate");
VMInternalJumpShip(ShipNum,xyz2);
}

void VMGate(player,ShipNum,Ship)
int player;
int ShipNum;
{
t_SpaceList *tmpshort;
double x,y,z;
t_xyz xyz1,xyz2;
t_sph sph;
int per;
char bf[200];

if(!checkflag(VMSpace[Ship].VMPerm.Flags,JUMPGATE)) {
	notify(player,"That is not a jumpgate");
	return;
	}

xyz1=VMSpace[ShipNum].VMPerm.Nav.XYZCoords;
xyz2=VMSpace[Ship].VMPerm.Nav.XYZCoords;
	if(VMdistance(xyz1,xyz2) > 5) {
                notify(player,"You are not close enough to that JumpGate");
		return;
	}
notify(player,"\n\t\tYou have entered the Jump Gate");
VMInternalJumpShip(ShipNum,xyz2);
}


void VMScanShip(player,ShipNum,Ship)
int player;
int ShipNum;
{
t_SpaceList *tmpshort;
double x,y,z;
t_xyz xyz1,xyz2;
t_sph sph;
int per;
char bf[200];
int nam,reso,shel1,shel2,mb,zmo;
char typ[20],sh1[10],sh2[10];
tmpshort=VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips;

if(!IsShipInSpaceList(tmpshort,Ship)) {
	VMnotify(player,"That ship is not on sensors.");
	return;
	}
mb=VMSpace[Ship].VMPerm.mainbridge;
zmo=-1;
nam=-1;
if(mb > 1) {
	zmo=Zone(mb);
	nam=atoi(vfetch_attribute(zmo,"HASNAME"));
	if(nam > 0) sprintf(bf,"%s",vfetch_attribute(zmo,"SHIPNAME"));
	/*if(nam > 0) sprintf(bf,"%s",vget_a(zmo,A_VA));*/
	}
	/*notify(2,tprintf("ZMO=%d NAM=%d MB=%d bf=%s",zmo,nam,mb,bf));*/
/*notify(2,"Here");*/
xyz1=VMSpace[ShipNum].VMPerm.Nav.XYZCoords;
xyz2=VMSpace[Ship].VMPerm.Nav.XYZCoords;
sph=VMRelative(ShipNum,Ship);
reso=ContactMatrix[ShipNum][Ship];
shel2=VMGetArc(ShipNum,Ship);
shel1=VMGetArc(Ship,ShipNum);
VMAssignArcString(shel1,sh1);
VMAssignArcString(shel2,sh2);

	sprintf(typ,"SHIP");
if(checkflag(VMSpace[Ship].VMPerm.Flags,JUMPGATE))
	sprintf(typ,"JUMPGATE");
else if(checkflag(VMSpace[Ship].VMPerm.Flags,BASE))
	sprintf(typ,"BASE");
else if(checkflag(VMSpace[Ship].VMPerm.Flags,PLANET))
	sprintf(typ,"PLANET");
else if(checkflag(VMSpace[Ship].VMPerm.Flags,SUN))
	sprintf(typ,"STAR");
else if(checkflag(VMSpace[Ship].VMPerm.Flags,BEACON))
	sprintf(typ,"BEACON");
  x=VMSpace[Ship].VMPerm.Nav.XYZCoords.x;
  y=VMSpace[Ship].VMPerm.Nav.XYZCoords.y;
  z=VMSpace[Ship].VMPerm.Nav.XYZCoords.z;
if(nam > 0)
VMnotify(player,"   Name: %s      \n   Type: %9s\n",bf,typ);
else
VMnotify(player,"   Name: %s      Type: %9s\n",VMSpace[Ship].VMPerm.Name,typ);
  if(!checkflag(VMSpace[Ship].VMPerm.Flags,HYPERSPACE)) {
    VMnotify(player," X: %9.2f   Y: %9.2f   Z: %9.2f",x,y,z); }
VMnotify(player,"                 Bearing: %6.2f Elev: %6.2f  Range: %8.0f",sph.bearing,sph.elevation,sph.range);
VMnotify(player,"                 Heading: %6.2f       %6.2f  Speed: %4.0f\n",VMSpace[Ship].VMPerm.Nav.Alpha,VMSpace[Ship].VMPerm.Nav.Beta,VMSpace[Ship].VMPerm.Nav.Systems[1].tdstat2);
VMnotify(player,"                 Resolution = %d",reso);
VMnotify(player,"                 Our %s arc is facing their %s arc",sh1,sh2);

if(checkflag(VMSpace[Ship].VMPerm.Flags,BASE))
VMScanDamStat(player,Ship);
if(checkflag(VMSpace[Ship].VMPerm.Flags,SHIP))
VMScanDamStat(player,Ship);



}


void VMScanBrief(player,ShipNum)
int player;
int ShipNum;
{
t_SpaceList *tmpshort;
t_SpaceList *tmplong;
t_SpaceEnt *tmp;
t_xyz xyz1,xyz2;
t_sph sph;
int i,k;
char typ[25];

tmpshort=VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips;
/*
tmplong=VMSpace[ShipNum].VMTemp.TmpNav.VMLongShips;
*/

xyz1=VMSpace[ShipNum].VMPerm.Nav.XYZCoords;

VMnotify(player,"\n\n ---Type--- NUM      Bearing         Range    Heading    Speed    Arcs");
VMnotify(player,"-----------------------------------------------------------------------");
VMShowContacts2(player,tmpshort,xyz1,ShipNum);

VMnotify(player,"-----------------------------------------------------------------------\n");
}

void VMScan(player,ShipNum)
int player;
int ShipNum;
{
t_SpaceList *tmpshort;
t_SpaceList *tmplong;
t_SpaceEnt *tmp;
t_xyz xyz1,xyz2;
t_sph sph;
int i,k;
char typ[25];

tmpshort=VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips;
/*
tmplong=VMSpace[ShipNum].VMTemp.TmpNav.VMLongShips;
*/

xyz1=VMSpace[ShipNum].VMPerm.Nav.XYZCoords;

VMnotify(player,"\n\n ---Type--- NUM      Bearing         Range    Heading    Speed    Arcs");
VMnotify(player,"-----------------------------------------------------------------------");
VMShowContacts(player,tmpshort,xyz1,ShipNum);

/*
VMShowContacts(player,tmplong,xyz1,1,ShipNum);
*/

VMnotify(player,"-----------------------------------------------------------------------\n");
for(i=0;i<JPTMAX;i++) {
	if(JPT[i].sector==VMSpace[ShipNum].VMTemp.TmpNav.Sector && VMdistance(JPT[i].XYZ,VMSpace[ShipNum].VMPerm.Nav.XYZCoords) < 100) {
		sph=VMRelative2(ShipNum,JPT[i].XYZ);
		VMnotify(player,"[ JumpPoint %3d]  %6.2f %6.2f  %8.0f",i,sph.bearing,sph.elevation,sph.range);
		}
	}
VMnotify(player,"-----------------------------------------------------------------------\n");

}

void VMShowContacts2(player,tmplist,xyz1,ShipNum)
int player;
t_SpaceList *tmplist;
t_xyz xyz1;
int ShipNum;
{
t_SpaceEnt *tmp;
t_sph sph;
char typ[25];
char numm[9];
char arc1,arc2;
t_xyz xyz2;
int shel1,shel2;
int reso,spp;

ResetSpaceList(tmplist);

while(!AtEndSpaceList(tmplist)) {
	tmp=GetCurrSpaceEnt(tmplist);
	if(tmp==NULL) break;


reso=ContactMatrix[ShipNum][tmp->VMShip];

sprintf(numm,"%4d",tmp->VMShip);
if(reso<=25)
sprintf(numm,"????");	

shel2=VMGetArc(ShipNum,tmp->VMShip);
shel1=VMGetArc(tmp->VMShip,ShipNum);

arc1='*';
arc2='*';

VMAssignArc(shel1,&arc1);
VMAssignArc(shel2,&arc2);

spp=0;
sph=VMRelative(ShipNum,tmp->VMShip);
	sprintf(typ,"SHIP");
if(checkflag(VMSpace[tmp->VMShip].VMPerm.Flags,SHIP)) {
	spp=1;
	sprintf(typ,"SHIP");
	}
else if(checkflag(VMSpace[tmp->VMShip].VMPerm.Flags,BASE)) {
	spp=1;
	sprintf(typ,"BASE");
	}
else if(checkflag(VMSpace[tmp->VMShip].VMPerm.Flags,FIGHTER)) {
	spp=1;
	sprintf(typ,"SHIP");
	}

xyz2=VMSpace[tmp->VMShip].VMPerm.Nav.XYZCoords;
if(reso>25 && spp==1)
VMnotify(player,"[%9s %s]  %6.2f %6.2f  %8.0f  %6.2f %6.2f  %4.0f    [%c:%c]",typ,numm,sph.bearing,sph.elevation,sph.range,VMSpace[tmp->VMShip].VMPerm.Nav.Alpha,VMSpace[tmp->VMShip].VMPerm.Nav.Beta,VMSpace[tmp->VMShip].VMPerm.Nav.Systems[1].tdstat2,arc1,arc2);

else if(spp==1)
VMnotify(player,"[%9s ????]  %6.2f %6.2f  %8.0f  ???.?? ???.??  ????    [%c:?]",typ,sph.bearing,sph.elevation,sph.range,arc1);


AdvanceSpaceList(tmplist);


 }
}


void VMShowContacts(player,tmplist,xyz1,ShipNum)
int player;
t_SpaceList *tmplist;
t_xyz xyz1;
int ShipNum;
{
t_SpaceEnt *tmp;
t_sph sph;
char typ[25];
char numm[9];
char arc1,arc2;
t_xyz xyz2;
int shel1,shel2;
int reso;

ResetSpaceList(tmplist);

while(!AtEndSpaceList(tmplist)) {
	tmp=GetCurrSpaceEnt(tmplist);
	if(tmp==NULL) break;


reso=ContactMatrix[ShipNum][tmp->VMShip];

sprintf(numm,"%4d",tmp->VMShip);
if(reso<=25)
sprintf(numm,"????");	

shel2=VMGetArc(ShipNum,tmp->VMShip);
shel1=VMGetArc(tmp->VMShip,ShipNum);

arc1='*';
arc2='*';

VMAssignArc(shel1,&arc1);
VMAssignArc(shel2,&arc2);


sph=VMRelative(ShipNum,tmp->VMShip);
	sprintf(typ,"SHIP");
if(checkflag(VMSpace[tmp->VMShip].VMPerm.Flags,JUMPGATE))
	sprintf(typ,"JUMPGATE");
else if(checkflag(VMSpace[tmp->VMShip].VMPerm.Flags,BASE))
	sprintf(typ,"BASE");
else if(checkflag(VMSpace[tmp->VMShip].VMPerm.Flags,PLANET))
	sprintf(typ,"PLANET");
else if(checkflag(VMSpace[tmp->VMShip].VMPerm.Flags,SUN))
	sprintf(typ,"STAR");
else if(checkflag(VMSpace[tmp->VMShip].VMPerm.Flags,BEACON))
	sprintf(typ,"BEACON");

xyz2=VMSpace[tmp->VMShip].VMPerm.Nav.XYZCoords;
if(reso>25)
VMnotify(player,"[%9s %s]  %6.2f %6.2f  %8.0f  %6.2f %6.2f  %4.0f    [%c:%c]",typ,numm,sph.bearing,sph.elevation,sph.range,VMSpace[tmp->VMShip].VMPerm.Nav.Alpha,VMSpace[tmp->VMShip].VMPerm.Nav.Beta,VMSpace[tmp->VMShip].VMPerm.Nav.Systems[1].tdstat2,arc1,arc2);

else
VMnotify(player,"[%9s ????]  %6.2f %6.2f  %8.0f  ???.?? ???.??  ????    [%c:?]",typ,sph.bearing,sph.elevation,sph.range,arc1);


AdvanceSpaceList(tmplist);


 }
}

void VMAssignArcString(int arc,char *carc)
{

if(arc==FORE) {
	sprintf(carc,"Fore");
	return;
	}
if(arc==AFT) {
	sprintf(carc,"Aft");
	return;
	}
if(arc==PORT) {
	sprintf(carc,"Port");
	return;
	}
if(arc==STARB) {
	sprintf(carc,"Starb");
	return;
	}
if(arc==TOP) {
	sprintf(carc,"Dorsal");
	return;
	}
if(arc==BOTTOM) {
	sprintf(carc,"Ventral");
	return;
	}
}

void VMAssignArc(int arc,char *carc)
{

if(arc==FORE) {
	*carc='^';
	return;
	}
if(arc==AFT) {
	*carc='v';
	return;
	}
if(arc==PORT) {
	*carc='<';
	return;
	}
if(arc==STARB) {
	*carc='>';
	return;
	}
if(arc==TOP) {
	*carc='~';
	return;
	}
if(arc==BOTTOM) {
	*carc='_';
	return;
	}
}

void VMNavJump(player,ShipNum)
	int player,ShipNum;
{
int tmpsector;

if(!checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[5].flags,EXISTS)) {
	VMnotify(player,"[COMPUTER]This ship has no jump engine.");
	return;
	}
tmpsector=VMSpace[ShipNum].VMTemp.TmpNav.Sector;
if(tmpsector>1 && tmpsector < 7) {
	VMnotify(player,"You cannot use jump engines when in sim space");
	return;
	}

if(VMSpace[ShipNum].VMPerm.Nav.Systems[5].tistat2<VMSpace[ShipNum].VMPerm.Nav.Systems[5].istat2 ) {
	VMnotify(player,"[COMPUTER]The jump engine is not fully charged.");
	if(Wizard(player) && VMSpace[ShipNum].VMPerm.Nav.Systems[5].tistat2 > 0)
	VMnotify(player,"***Invoking Wizard Privilege and jumping anyways.***");
	else
	return;
	}

JPT[JPTMAX].XYZ=VMSpace[ShipNum].VMPerm.Nav.XYZCoords;
JPT[JPTMAX].timeleft=VMSpace[ShipNum].VMPerm.Nav.Systems[5].istat3;
JPT[JPTMAX].sector=VMSpace[ShipNum].VMTemp.TmpNav.Sector;
JPTMAX++;
VMnotify(player,"You engage the jump engine and enter the point");
VMInternalJumpShip(ShipNum,VMSpace[ShipNum].VMPerm.Nav.XYZCoords);
VMSpace[ShipNum].VMPerm.Nav.Systems[5].tistat2=0;
}

void VMDisEngage(player,ShipNum)
	int player,ShipNum;
{

if(IsShipInSpaceList(VMShipsTractoring,ShipNum)) {
	VMnotify(player,"[COMPUTER]Cannot Disengage while tractoring");
	return;
	}

if(VMSpace[ShipNum].VMPerm.Nav.Systems[1].tdstat2 > 0) {
	VMnotify(player,"[COMPUTER]Cannot Disengage while moving");
	return;
	}
if(!VMDisEngageInternal(ShipNum)) {
	VMnotify(player,"[COMPUTER]This ship is not Engaged");
	return;
	}

VMnotify(player,"[COMPUTER]Ship Disengaged");
}

int VMDisEngageInternal(ShipNum)
	int ShipNum;
{

if(!IsShipInSpaceList(VMShipsMoving,ShipNum)) 
	return 0;

	RemoveSpaceEnt(VMShipsMoving);
	return 1;
}


void VMChart(player,ShipNum,range)
     int player,ShipNum;
	double range;
{
t_SpaceList *tmplist;
t_xyz xyz1,xyz2;
double x,y,z;
t_sph sph;
int reso,shel1,shel2,ship;
int i,k,sx,sy,sz;
int ix,iy,iz,ll;
int dataxy[9][9];
int dataxz[9][9];
char buff[100];
int typ;
t_SpaceEnt *tmp;

tmplist=VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips;

  if(!CanAccessNav(ShipNum,player)) 
    return;
xyz1=VMSpace[ShipNum].VMPerm.Nav.XYZCoords;
bzero(dataxy,sizeof(dataxy));
bzero(dataxz,sizeof(dataxz));
ResetSpaceList(tmplist);
dataxy[4][4]=1;
dataxz[4][4]=1;
while(!AtEndSpaceList(tmplist)) {
	tmp=GetCurrSpaceEnt(tmplist);
	if(tmp==NULL) break;
	ship=tmp->VMShip;
	xyz2=VMSpace[ship].VMPerm.Nav.XYZCoords;
	x=xyz1.x-xyz2.x;
	y=xyz1.y-xyz2.y;
	z=xyz1.z-xyz2.z;
	sx=x*4/range;
	sy=y*4/range;
	sz=z*4/range;
	ix=4+sx;
	iy=4+sy;
	iz=4+sz;

	if(checkflag(VMSpace[ship].VMPerm.Flags,JUMPGATE)) typ=3;
	else if(checkflag(VMSpace[ship].VMPerm.Flags,BASE)) typ=4;
	else if(checkflag(VMSpace[ship].VMPerm.Flags,PLANET)) typ=5;
	else typ=2;
	if(ix<9 && ix>=0 && iy<9 && iy>=0) {
	if(dataxy[ix][iy]!=0) dataxy[ix][iy]=6;
	else dataxy[ix][iy]=typ;
	}
	if(ix<9 && ix>=0 && iz<9 && iz>=0) {
	if(dataxz[ix][iz]!=0) dataxz[ix][iz]=6;
	else dataxz[ix][iz]=typ;
	}
	AdvanceSpaceList(tmplist);
	}

VMnotify(player, "\nYou prepare a chart of your surrounding system\n----------------------------------------------\n\n");
for(i=0;i<9;i++) {
	buff[0]=' ';
	for(k=1;k<=36;k+=4) {
	ll=(k-1)/4;
	ll=8-ll;
		buff[k]='.';
		buff[k+2]='.';
		buff[k+3]=' ';
		if(dataxy[ll][i]==1) buff[k+1]='#';
		else if(dataxy[ll][i]==2) buff[k+1]='S';
		else if(dataxy[ll][i]==3) buff[k+1]='J';
		else if(dataxy[ll][i]==4) buff[k+1]='B';
		else if(dataxy[ll][i]==5) buff[k+1]='O';
		else if(dataxy[ll][i]==6) buff[k+1]='*';
		else buff[k+1]='.';
		}
	buff[37]='|';
	buff[38]='|';
	buff[39]=' ';
	for(k=40;k<=75;k+=4) {
	ll=(k-40)/4;
	ll=8-ll;
		buff[k]='.';
		buff[k+2]='.';
		buff[k+3]=' ';
		if(dataxy[ll][i]==1) buff[k+1]='#';
		else if(dataxy[ll][i]==2) buff[k+1]='S';
		else if(dataxy[ll][i]==3) buff[k+1]='J';
		else if(dataxy[ll][i]==4) buff[k+1]='B';
		else if(dataxy[ll][i]==5) buff[k+1]='O';
		else if(dataxy[ll][i]==6) buff[k+1]='*';
		else buff[k+1]='.';
		}
		buff[76]='\0';
	VMnotify(player,buff);
	}
}


void VMEngage(player,ShipNum)
     int player,ShipNum;
{

  if(!CanAccessNav(ShipNum,player)) 
    return;

if(IsShipInSpaceList(VMShipsMoving,ShipNum)) {
	VMnotify(player,"[COMPUTE] Engines already engaged");
	return;
	}

  AddSpaceEnt(VMShipsMoving,ShipNum);
  VMnotify(player,"[COMPUTER] Engines Engaged");
}



void VMSetSpeed(player,ShipNum,speed)
     int player, ShipNum;
     double speed;
{

  if(!CanAccessNav(ShipNum,player)) 
    return;
  
  if(speed < 0 || speed > VMSpace[ShipNum].VMPerm.Nav.Systems[1].dstat8+Hulls[VMSpace[ShipNum].VMPerm.HullType].AddSpeedMod ) 
  /*if(speed < 0 ) */
    {
	VMnotify(player,"[COMPUTER] Invalid Speed");
	return;
	}
  
  VMSpace[ShipNum].VMPerm.Nav.Systems[1].tdstat1=speed;
  VMnotify(player,"[COMPUTER] Speed Set");
}



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


void VMNavStatus(player,ShipNum)
     int player,ShipNum;
{
  double x2,y2,z2,x,y,z,speed,alp,bet,dspeed,dalp,dbet,rol,drol,perc,assigned;
  t_sph sph;
  int tmpSector,i,per;
  
  /*ShipNum=atoi(vget_a(where_is(player),A_VA));*/

  ShipNum=VMShipNum(player);
  if(!CanAccessNav(ShipNum,player)) 
    return;
  x=VMSpace[ShipNum].VMPerm.Nav.XYZCoords.x;
  y=VMSpace[ShipNum].VMPerm.Nav.XYZCoords.y;
  z=VMSpace[ShipNum].VMPerm.Nav.XYZCoords.z;
  x2=VMSpace[ShipNum].VMTemp.TmpNav.XYZCoords.x;
  y2=VMSpace[ShipNum].VMTemp.TmpNav.XYZCoords.y;
  z2=VMSpace[ShipNum].VMTemp.TmpNav.XYZCoords.z;
  speed=VMSpace[ShipNum].VMPerm.Nav.Systems[1].tdstat2;
  dspeed=VMSpace[ShipNum].VMPerm.Nav.Systems[1].tdstat1;
  alp=VMSpace[ShipNum].VMPerm.Nav.Alpha;
  bet=VMSpace[ShipNum].VMPerm.Nav.Beta;
  dalp=VMSpace[ShipNum].VMPerm.Nav.Des_alpha;
  dbet=VMSpace[ShipNum].VMPerm.Nav.Des_beta;
  rol=VMSpace[ShipNum].VMPerm.Nav.Roll;
  drol=VMSpace[ShipNum].VMPerm.Nav.Des_roll;
  tmpSector=VMSpace[ShipNum].VMTemp.TmpNav.Sector;
  
  sph=VMSpace[ShipNum].VMPerm.Nav.SPHCoords;
  if(!checkflag(VMSpace[ShipNum].VMPerm.Flags,HYPERSPACE)) {
    VMnotify(player,"\n-----------------------------------------------------------");
    VMnotify(player," Ship Posistion");
    VMnotify(player," X: %9.2f   Y: %9.2f   Z: %9.2f",x,y,z);
    VMnotify(player," B: %9.2f   E: %9.2f   R: %9.2f",sph.bearing,sph.elevation,sph.range);
  }
 if(VMSpace[ShipNum].VMTemp.TmpNav.APON==1) {
  VMnotify(player,"\n Autopilot Enaged on coordinates");
    VMnotify(player," X: %9.2f   Y: %9.2f   Z: %9.2f",x2,y2,z2);
 }
  VMnotify(player,"\n Ship Orientation");
  VMnotify(player,"Desired Heading,Elev,Roll: %6.2f , %6.2f , %6.2f",dalp,dbet,drol);
  VMnotify(player,"Actual  Heading,Elev,Roll: %6.2f , %6.2f , %6.2f",alp,bet,rol);
  VMnotify(player,"Desired / Actual    Speed: %6.2f / %6.2f",dspeed,speed);
  if(tmpSector!=-1)
    VMnotify(player,"   Current Sector: [%s]",VMSectors[tmpSector].Name);
  VMnotify(player,"-----------------------------------------------------------");
  

  VMnotify(player," Navigation Systems - Power Assigned %d",VMSpace[ShipNum].VMTemp.TmpEng.PowerNav);
  VMnotify(player,"-----------------------------------------------------------");
  for(i=0;i<NSystems;i++) {
    if(checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[i].flags,EXISTS) &&
       checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[i].flags,POWERABLE)) 
      {
	perc=0.0;
	assigned=VMSpace[ShipNum].VMPerm.Nav.Systems[i].tdstat0 * VMSpace[ShipNum].VMTemp.TmpEng.PowerNav;
	if(VMSpace[ShipNum].VMPerm.Nav.Systems[i].dstat3!=0.0) 
	  perc=assigned/VMSpace[ShipNum].VMPerm.Nav.Systems[i].dstat3 * 100.0;
	
	VMnotify(player," [%3d] %24s  | Assigned = %5.1lf | %5.1lf%% of Max",i,VMSpace[ShipNum].VMPerm.Nav.Systems[i].name,assigned,perc);
      }
  }
  VMnotify(player,"----- System Status ------------------------------");
  VMnotify(player,"System Name\t\t\tStatus");
  for (i = 0; i < NSystems;i++) {
    if (checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[i].flags,EXISTS)) {
      if (VMSpace[ShipNum].VMPerm.Nav.Systems[i].mintegrity != 0) {
	per = (VMSpace[ShipNum].VMPerm.Nav.Systems[i].integrity / VMSpace[ShipNum].VMPerm.Nav.Systems[i].mintegrity) * 100;
      }
      else
	per = -1;
      VMnotify(player,"%24s\t\t\t %2.2f/%2.2f (%d%%)",VMSpace[ShipNum].VMPerm.Nav.Systems[i].name,VMSpace[ShipNum].VMPerm.Nav.Systems[i].integrity,VMSpace[ShipNum].VMPerm.Nav.Systems[i].mintegrity,per);
    }
  }



  VMnotify(player,"-----------------------------------------------------------");

 if (checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[5].flags,EXISTS) ) {
  VMnotify(player,tprintf("  Jump Engine Current Charge = %d",VMSpace[ShipNum].VMPerm.Nav.Systems[5].tistat2));


  VMnotify(player,"-----------------------------------------------------------\n");

}
 if (checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[4].flags,EXISTS) && VMSpace[ShipNum].VMPerm.Nav.Systems[4].tistat3>0) {
for(i=0;i<NSystems;i++) {
	if (checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[i].flags,TRACT) && VMSpace[ShipNum].VMPerm.Nav.Systems[i].tistat3>0) {
  		VMnotify(player,"  Tractor Beam [%d] Locked on Ship %d",i,VMSpace[ShipNum].VMPerm.Nav.Systems[i].tistat3);
 	if(VMSpace[ShipNum].VMPerm.Nav.Systems[i].tistat2==1)
  		VMnotify(player,"  Tractor Beam [%d] is ON",i);
  	else VMnotify(player,"  Tractor Beam [%d] is OFF",i);
		}
	}
  VMnotify(player,"-----------------------------------------------------------\n");

	}

	 
 
/*
  VMnotify(player,"Max Speed = %f , Max Accel = %f \n E Per Speed = %f , Max Man Rate = %f  ",VMSpace[ShipNum].VMPerm.Nav.Systems[1].dstat0, VMSpace[ShipNum].VMPerm.Nav.Systems[1].dstat2, VMSpace[ShipNum].VMPerm.Nav.Systems[1].dstat3, VMSpace[ShipNum].VMPerm.Nav.Systems[2].dstat1);
*/

}

void VMTUnLock(player,ShipNum,ship)
int player,ShipNum,ship;
{
 if(!checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[4].flags,EXISTS)) {
	VMnotify(player,"Youd dont HAVE a tractor beam!");
	return;
	}

if(VMSpace[ShipNum].VMPerm.Nav.Systems[4].tistat2==1) {
	VMnotify(player,"Cannot unlock while engaged.");
	return;
 }
VMnotify(player,"Tractor Beam UnLocked");
  VMSpace[ShipNum].VMPerm.Nav.Systems[4].tistat3=0;

}
void VMTLock(player,ShipNum,beam,ship)
int player,ShipNum,beam,ship;
{
t_SpaceList *tmpshort;
tmpshort=VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips;

if(beam < 0 || beam > NSystems) {
	VMnotify(player,"Invalid nav system.");
	return;
	}
 if(!checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[beam].flags,TRACT)) {
	VMnotify(player,"That system is not a tractor beam!");
	return;
	}
if(!IsShipInSpaceList(tmpshort,ship)) {
	VMnotify(player,"That ship is not on sensors.");
	return;
	}
VMnotify(player,"Tractor Beam Locked");
  VMSpace[ShipNum].VMPerm.Nav.Systems[beam].tistat3=ship;

}

void VMTDisEngage(player,ShipNum,beam)
int player,ShipNum,beam;
{
int i,rem=1;
if(beam < 0 || beam > NSystems) {
	VMnotify(player,"Invalid nav system.");
	return;
	}
 if(!checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[beam].flags,TRACT)) {
	VMnotify(player,"That system is not a tractor beam!");
	return;
	}
if(VMSpace[ShipNum].VMPerm.Nav.Systems[beam].tistat2==0) {
	VMnotify(player,"The tractor beam is already disenaged.");
	return;
	}

VMnotify(player,"Tractor Beam DisEngaged.");
VMSpace[ShipNum].VMPerm.Nav.Systems[beam].tistat2=0;
if(!IsShipInSpaceList(VMShipsTractoring,ShipNum)) 
	return;
for(i=0;i < NSystems;i++) {
	if(VMSpace[ShipNum].VMPerm.Nav.Systems[i].tistat2==1 && checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[i].flags,TRACT))
		rem=0;

}

if(rem==1) RemoveSpaceEnt(VMShipsTractoring);


}

void VMTEngage(player,ShipNum,beam)
int player,ShipNum,beam;
{
int ship;
t_SpaceList *tmpshort;
tmpshort=VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips;
if(beam < 0 || beam > NSystems) {
	VMnotify(player,"Invalid nav system.");
	return;
	}
 if(!checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[beam].flags,TRACT)) {
	VMnotify(player,"That system is not a tractor beam!");
	return;
	}

  ship=VMSpace[ShipNum].VMPerm.Nav.Systems[beam].tistat3;

if(VMSpace[ShipNum].VMPerm.Nav.Systems[beam].tistat2==1) {
	VMnotify(player,"The tractor beam is already enaged.");
	return;
	}

if(ship==0) {

	VMnotify(player,"Bad Ship");
	return;
	}

if(!IsShipInSpaceList(tmpshort,ship)) {
	VMnotify(player,"That ship is no longer on sensors.");
	VMSpace[ShipNum].VMPerm.Nav.Systems[beam].tistat3=0;
	return;
	}

VMnotify(player,"Tractor Beam Engaged.");
VMSpace[ShipNum].VMPerm.Nav.Systems[beam].tistat2=1;

if(!IsShipInSpaceList(VMShipsTractoring,ShipNum)) 
	AddSpaceEnt(VMShipsTractoring,ShipNum);

}

void VMNavAlloc(int player, 
		int ship, 
		int nargs, 
		char *args[]
		)
{
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
    if (checkflag(VMSpace[ship].VMPerm.Nav.Systems[i].flags,EXISTS))
      if (checkflag(VMSpace[ship].VMPerm.Nav.Systems[i].flags,POWERABLE))
	{
/*	VMnotify(1,"I made it past the exists and power checks.");*/
	  if (k < nargs) {
	    if (total > 0) {
	      VMSpace[ship].VMPerm.Nav.Systems[i].tdstat0 = powervals[k] / total;
	    }
	    else
	      VMSpace[ship].VMPerm.Nav.Systems[i].tdstat0 = 0;
	    VMnotifymans(VMSpace[ship].VMTemp.TmpNav.VMMan_List,"Setting the power of %15s to %d%% of the total power",VMSpace[ship].VMPerm.Nav.Systems[i].name,(int) (VMSpace[ship].VMPerm.Nav.Systems[i].tdstat0 * 100.0));
	    
	    k++;
	  }
	  else
	    VMSpace[ship].VMPerm.Nav.Systems[i].tdstat0 = 0;
	}
  }
}

      

void
VMNavReallocPower(int ship
		  )
{
  int i;
  for (i = 0; i < MAXSYSTEMS; i++) {
    if (checkflag(VMSpace[ship].VMPerm.Nav.Systems[i].flags,EXISTS))
      if (checkflag(VMSpace[ship].VMPerm.Nav.Systems[i].flags,POWERABLE)) {
	VMSpace[ship].VMPerm.Nav.Systems[i].tistat0 = VMSpace[ship].VMPerm.Nav.Systems[i].tdstat0 * VMSpace[ship].VMTemp.TmpEng.PowerNav;
      }
  }
  
  VMSpace[ship].VMTemp.TmpNav.PowerUsed = VMSpace[ship].VMTemp.TmpEng.PowerNav;
}   
	  

void VMScanDamStat(int player, 
	       int ship
	       ) {
  int unused,i,per;
  char nstr[16];
  bzero(nstr,16);
  VMnotify(player,"----------------------Power Allocation-----------------------");

  VMnotify(player,"Power available: %d", VMSpace[ship].VMTemp.TmpEng.PowerAvail);
  VMnotify(player,"Power to Helm: %d Nav: %d Communications: %d Batts: %d EDC: %d", VMSpace[ship].VMTemp.TmpEng.PowerHelm,VMSpace[ship].VMTemp.TmpEng.PowerNav,VMSpace[ship].VMTemp.TmpEng.PowerComm,VMSpace[ship].VMPerm.Eng.Systems[1].tistat0,VMSpace[ship].VMPerm.Eng.Systems[2].tistat0);

  VMnotify(player,"-------------------Damaged System Status---------------------");
  /* helm */
  for ( i = 0; i < HSystems; i++) {
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,EXISTS)) {
      if (VMSpace[ship].VMPerm.Helm.Systems[i].integrity != VMSpace[ship].VMPerm.Helm.Systems[i].mintegrity) {
	if (VMSpace[ship].VMPerm.Helm.Systems[i].mintegrity > 0)
	  per = (VMSpace[ship].VMPerm.Helm.Systems[i].integrity / VMSpace[ship].VMPerm.Helm.Systems[i].mintegrity) * 100; 
	else
	  per = -1;
	VMnotify(player,"%s(helm %d)\t\t(%d%%)",VMSpace[ship].VMPerm.Helm.Systems[i].name,i,per);
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
	VMnotify(player,"%s(nav %d)\t\t (%d%%)",VMSpace[ship].VMPerm.Nav.Systems[i].name,i,per);
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
	VMnotify(player,"%s(eng %d)\t\t (%d%%)",VMSpace[ship].VMPerm.Eng.Systems[i].name,i,per);
      }
    }
  }
    if (VMSpace[ship].VMPerm.mIntegrity > 0)
      per = (VMSpace[ship].VMPerm.Integrity / VMSpace[ship].VMPerm.mIntegrity) * 100; 
    else
      per = -1;
  VMnotify(player,"-------------------------------------------------------------");
    VMnotify(player,"Hull integrity (%d%%)",per);

  VMnotify(player,"-------------------------------------------------------------");

}


void GodVMDock(player,ShipNum,DestShip,Bay)
int player,ShipNum,DestShip,Bay;

{
t_SpaceList *tmpbays;
t_SpaceEnt *tmp;
int i,flags1,flags2;
int tmpsector;
int airlockout,airlockin;
dbref exitloc;

if(DestShip < 0  || DestShip > VMCurrentMax)
	{
		VMnotify(player,"Invalid Ship");
		return;
	}

if(IsShipInSpaceList(VMShipsPowered,ShipNum))
	{
		VMnotify(player,"Reactor must be off.");
		return;
	}

if(Bay < 0 || Bay > GetSpaceListSize(VMSpace[DestShip].VMTemp.TmpNav.VMDockList))
	{
		VMnotify(player,"Invalid Bay Number");
		return;
	}




flags1=VMSpace[ShipNum].VMPerm.Flags;
flags2=VMSpace[DestShip].VMPerm.Flags;

if(!checkflag(flags2,SHIP) && !checkflag(flags2,PLANET) && !checkflag(flags2,BASE) && !checkflag(flags2,SMALLSHIP))
	{
		VMnotify(player,"That thing doesnt allow docking");
		return;
	}




tmpbays=VMSpace[DestShip].VMTemp.TmpNav.VMDockList;
ResetSpaceList(tmpbays);
for(i=0;i<Bay;i++) {
	if(AtEndSpaceList(tmpbays)) break;
	AdvanceSpaceList(tmpbays);
	}
	tmp=GetCurrSpaceEnt(tmpbays);
	if(tmp==NULL) 
		{
		VMnotify(player,"Unknown Error");
		return;
	}



airlockin=VMSpace[ShipNum].VMTemp.TmpNav.airlockin;
airlockout=VMSpace[ShipNum].VMTemp.TmpNav.airlockout;
if(airlockin <= 0 || airlockout > mudstate.db_top  || airlockout <=0 || airlockout > mudstate.db_top  ){
		VMnotify(player,"Airlock Not Configured Error");
		return;
	}
	

VMSpace[ShipNum].VMPerm.Flags=setflag(VMSpace[ShipNum].VMPerm.Flags,DOCKED);

VMWipeShipContacts(ShipNum);
tmpsector=VMSpace[ShipNum].VMTemp.TmpNav.Sector;
if(IsShipInSpaceList(VMSectors[tmpsector].VMSectorShips,ShipNum))
	RemoveSpaceEnt(VMSectors[tmpsector].VMSectorShips);
for(i=0;i < NSystems;i++) {
if(checkflag(VMSpace[ShipNum].VMPerm.Nav.Systems[i].flags,TRACT))
VMSpace[ShipNum].VMPerm.Nav.Systems[i].tistat2=0;
}
VMnotify(player,"The Ship is Now Docked ");

exitloc=Exits(airlockin);
s_Exits(exitloc,remove_first(Exits(exitloc),airlockin));
s_Exits(tmp->VMShip,insert_first(Exits(tmp->VMShip),airlockin));
s_Exits(airlockin,tmp->VMShip);
s_Location(airlockout,tmp->VMShip);

/*VMSpace[ShipNum].VMPerm.Nav.Dock=VMShipFromRoom(tmp->VMShip);*/
VMSpace[ShipNum].VMPerm.Nav.Dock=tmp->VMShip;

}


void GodVMUnDock(player,ShipNum)
int player,ShipNum;
{
int airlockout,airlockin,dnum;
dbref exitloc;


if(!checkflag(VMSpace[ShipNum].VMPerm.Flags,DOCKED)) {
	VMnotify(player,"The Ship is not Docked");
	return;
}

dnum=VMShipFromRoom(VMSpace[ShipNum].VMPerm.Nav.Dock);
if(checkflag(VMSpace[dnum].VMPerm.Flags,HYPERSPACE))
  VMSpace[ShipNum].VMPerm.Flags=setflag(VMSpace[ShipNum].VMPerm.Flags,HYPERSPACE);
else
  VMSpace[ShipNum].VMPerm.Flags=clearflag(VMSpace[ShipNum].VMPerm.Flags,HYPERSPACE);

VMSpace[ShipNum].VMPerm.Flags=clearflag(VMSpace[ShipNum].VMPerm.Flags,DOCKED);
VMSpace[ShipNum].VMPerm.Nav.XYZCoords=VMSpace[dnum].VMPerm.Nav.XYZCoords;
VMSpace[ShipNum].VMTemp.TmpNav.Sector=-1;
 
if(checkflag(VMSpace[dnum].VMPerm.Flags,HYPERSPACE)) 
VMSpace[ShipNum].VMPerm.Flags=setflag(VMSpace[ShipNum].VMPerm.Flags,HYPERSPACE);
else
VMSpace[ShipNum].VMPerm.Flags=clearflag(VMSpace[ShipNum].VMPerm.Flags,HYPERSPACE);

VMSetSector(ShipNum);
VMnotify(player,"The Ship is Now UnDocked");
airlockin=VMSpace[ShipNum].VMTemp.TmpNav.airlockin;
airlockout=VMSpace[ShipNum].VMTemp.TmpNav.airlockout;

exitloc=Exits(airlockin);
s_Exits(exitloc,remove_first(Exits(exitloc),airlockin));
s_Exits(SPACEROOM,insert_first(Exits(SPACEROOM),airlockin));
s_Exits(airlockin,SPACEROOM);

s_Location(airlockout,Exits(airlockout));
VMSBGetShipOutDQ(ShipNum);
}




