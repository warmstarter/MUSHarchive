#include "header.h"
static char *rcsvmcreatec="$Id: vmcreate.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";


void do_VMAdminInstall(player,data,buffer)
	int player;
	void *data;
	char *buffer;
{

char *fargs[5];
char *name;
int k,i,typ,sys,plc,extr,arc;
i=xcode_parse(buffer,fargs,5);
if(i<3 || i>5) {
	notify(player,"Usage: INSTALL 3 SYS# PLACE# SHIPNUM# ARCS# or INSTALL 0 HULL# SHIPNUM# or INSTALL TYPE# SYSTEM# PLACE# or INSTALL 5 NAME SHIP#");
	return;
}
typ=atoi(fargs[0]);
if(typ==0 && i!=3) {
	notify(player,"Usage: INSTALL 3 SYS# PLACE# SHIPNUM# ARCS# or INSTALL 0 HULL# SHIPNUM# or INSTALL TYPE# SYSTEM# PLACE# or INSTALL 5 NAME SHIP#");
	return;
}
else if(typ==5 && i!=3) {
	notify(player,"Usage: INSTALL 3 SYS# PLACE# SHIPNUM# ARCS# or INSTALL 0 HULL# SHIPNUM# or INSTALL TYPE# SYSTEM# PLACE# or INSTALL 5 NAME SHIP#");
	return;
}
else if(typ==3 && i!=5) {
	notify(player,"Usage: INSTALL 3 SYS# PLACE# SHIPNUM# ARCS# or INSTALL 0 HULL# SHIPNUM# or INSTALL TYPE# SYSTEM# PLACE# or INSTALL 5 NAME SHIP#");
	return;
}

else if((typ==1 || typ==2 ) && i!=4) {
	notify(player,"Usage: INSTALL 3 SYS# PLACE# SHIPNUM# ARCS# or INSTALL 0 HULL# SHIPNUM# or INSTALL TYPE# SYSTEM# PLACE# or INSTALL 5 NAME SHIP#");
	return;
}
if(typ==5) {
	sys=atoi(fargs[2]);
	if(sys < 1 || sys > VMCurrentMax) {
		notify(player,"Invalid ShipNum");
		return;
	}
	name=fargs[1];
	strcpy(VMSpace[sys].VMPerm.Name,name);
	return;
}
sys=atoi(fargs[1]);
plc=atoi(fargs[2]);
if(typ!=0) extr=atoi(fargs[3]);
if(typ==0) {
	if(VMCurrentMax < plc) VMCurrentMax=plc;
	for(k=0;k < HSystems; k++) {
		VMSpace[plc].VMPerm.Helm.Systems[k].flags=0;
	}
	for(k=0;k < NSystems; k++) {
		VMSpace[plc].VMPerm.Nav.Systems[k].flags=0;
	}
	for(k=0;k < ESystems; k++) {
		VMSpace[plc].VMPerm.Eng.Systems[k].flags=0;
	}

	VMInstallHull(plc,sys);
	VMInitTmpShip2(plc);
	notify(player,tprintf("Hull %d Installed on ship %d",sys,plc));
	return;
}
if(typ==1) {
	VMInstallNavSys(extr,sys,plc);
	notify(player,"System Installed");
	return;
}
if(typ==2) {
	VMInstallEngSys(extr,sys,plc);
	notify(player,"System Installed");
	return;
}
if(typ==3) {
	arc=atoi(fargs[4]);
	VMInstallHelmSys(extr,sys,plc,arc);
	notify(player,"System Installed");
	return;
}


}







void VMInitTmpShip2(int i)
{
int k;

	VMSpace[i].VMTemp.TmpNav.VMDockList= (t_SpaceList *) InitSpaceList();
	VMSpace[i].VMTemp.TmpNav.VMRelayList= (t_RelayList *) InitRelayList();
	VMSpace[i].VMTemp.TmpNav.VMShortShips= (t_SpaceList *) InitSpaceList();

    VMSpace[i].VMTemp.TmpNav.VMMan_List = InitManList();
    VMSpace[i].VMTemp.TmpEng.VMMan_List = InitManList();
    VMSpace[i].VMTemp.TmpCommu.VMMan_List = InitManList();
    VMSpace[i].VMTemp.TmpScience.VMMan_List = InitManList();
    VMSpace[i].VMTemp.TmpHelm.VMMan_List = InitManList();
    VMSpace[i].VMTemp.TmpOps.VMMan_List = InitManList();
    VMSpace[i].VMTemp.TmpDam.VMMan_List = InitManList();

for(k=0;k<10;k++) {
	VMSpace[i].VMTemp.TmpCommu.cd[k]=0;
}

	VMSpace[i].VMTemp.TmpCommu.Relay=-1;
	VMSpace[i].VMTemp.TmpNav.Sector=1;
	VMSpace[i].VMTemp.TmpEng.PowerHelm = 0;
	VMSpace[i].VMTemp.TmpEng.PowerNav = 0;
	VMSpace[i].VMTemp.TmpEng.PowerAvail = 0;

	for(k=0;k<NSystems;k++) {
	VMSpace[i].VMPerm.Nav.Systems[k].tistat0=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tistat1=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tistat2=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tistat3=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tistat4=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tdstat0=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tdstat1=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tdstat2=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tdstat3=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tdstat4=0;
	}
	for(k=0;k<ESystems;k++) {
	VMSpace[i].VMPerm.Eng.Systems[k].tistat0=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tistat1=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tistat2=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tistat3=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tistat4=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tdstat0=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tdstat1=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tdstat2=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tdstat3=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tdstat4=0;
/*
	VMSpace[i].VMPerm.Helm.Systems[k].tistat0=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tistat1=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tistat2=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tistat3=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tistat4=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tdstat0=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tdstat1=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tdstat2=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tdstat3=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tdstat4=0;
*/
	}


}


void VMListSystems(int player) {
  int s = 0;
  VMnotify(player,"----------Systems--------------");
  while (strcmp(Systems[s].name,"NADA")) {
    VMnotify(player,"-----------------------------");
    VMnotify(player,"%d) Name: %s Hull Integrity: %5.2f Threshold: %5.2f",s,Systems[s].name,Systems[s].integrity,Systems[s].threshold);
    s++;
  }
}

void VMShowShip(int player,int ship) {
   
  if (VMGlobalSpaceInit == 0) {
    VMnotify(player,"Space has not been initialized.");
    return;
  }

  if (ship < 1 || ship > VMCurrentMax) {
    VMnotify(player,"That doesn't exist. Sorry.");
    return;
  }
  

  VMnotify(player,"Ship #%d Name: %s",ship,VMSpace[ship].VMPerm.Name);
  VMnotify(player,"Hull: %s(%d)",Hulls[VMSpace[ship].VMPerm.HullType].name,VMSpace[ship].VMPerm.HullType);
  VMnotify(player,"X: %2.2f Y: %2.2f Z: %2.2f ",VMSpace[ship].VMPerm.Nav.XYZCoords.x,VMSpace[ship].VMPerm.Nav.XYZCoords.y,VMSpace[ship].VMPerm.Nav.XYZCoords.z);
  if (VMSpace[ship].VMTemp.rooms == NULL) {
    VMnotify(player,"Null Roomlist");
  }
  else
    VMnotify(player,"Rooms: %d",GetRoomListSize(VMSpace[ship].VMTemp.rooms));
  VMnotify(player,"Mainbridge: %d  CurrentMax: %d  Orbit: %d",VMSpace[ship].VMPerm.mainbridge,VMCurrentMax,VMSpace[ship].VMPerm.Nav.tmp1); 
  VMnotify(player,"Size: %f",Hulls[VMSpace[ship].VMPerm.HullType].Size);
  VMnotify(player,"SOwner: %s",VMSpace[ship].VMPerm.SOwner == NULL ? "Null" : VMSpace[ship].VMPerm.SOwner);
  VMSeeFlag(player,ship);

}

void do_MakeShip(player,data,buffer)
	int player;
	void *data;
	char *buffer;
{

char *fargs[5];
char *name;
int k,i,typ,sys,plc,extr,arc;
i=xcode_parse(buffer,fargs,2);
if(i!=2) {
	VMnotify(player,"Usage: MAKESHIP <TEMPLATE#> <OWNER DB>");
	return;
	}
if(!isPlayer(atoi(fargs[1]))) {
	notify(player,"The owner must be a player");
	return;
	}
do_make_ship(atoi(fargs[0]),where_is(player),atoi(fargs[1]));
VMnotify(player,"Ship Built");
}


void do_make_ship2(temp,snum,brdg,ao,ai,nam)
int temp,snum,brdg,ao,ai;
char *nam;
{
int room,zon;
int cost,i,k;
char buff[50];
char ptr[20];


if(snum > MAXSHIP) return;
if(VMCurrentMax < snum) VMCurrentMax=snum;

if(ao!=0 || brdg!=0 || ai!=0) {
VMSpace[snum].VMTemp.TmpNav.airlockout=ao;
VMSpace[snum].VMTemp.TmpNav.airlockin=ai;
VMSpace[snum].VMPerm.mainbridge=brdg;
	}
	for(k=0;k < HSystems; k++) {
		VMSpace[snum].VMPerm.Helm.Systems[k].flags=0;
	}
	for(k=0;k < NSystems; k++) {
		VMSpace[snum].VMPerm.Nav.Systems[k].flags=0;
	}
	for(k=0;k < ESystems; k++) {
		VMSpace[snum].VMPerm.Eng.Systems[k].flags=0;
	}

VMInstallHull(snum,VMTemplate[temp].hull);
for(i=0;i<20;i++) {
	if(VMTemplate[temp].nav[i]!=0 || i==0)
	VMInstallNavSys(snum,VMTemplate[temp].nav[i],i);
	}
for(i=0;i<20;i++) {
	if(VMTemplate[temp].eng[i]!=0)
	VMInstallEngSys(snum,VMTemplate[temp].eng[i],i);
	}
for(i=0;i<25;i++) {
	if(VMTemplate[temp].helm[i]!=0)
	/*VMInstallHelmSys(snum,VMTemplate[temp].helm[i],i,FORE|STARB|PORT);*/

	VMInstallHelmSys(snum,VMTemplate[temp].helm[i],i,VMTemplate[temp].hloc[i]);
	}
strcpy(VMSpace[snum].VMPerm.Name,nam);
}


void do_make_ship(temp,yard,own)
int temp,yard,own;
{
int room,zon,ai,ao;
int cost,i;
char buff[50];
char ptr[20];


if(temp < 0 || temp > MaxTemps) return;

cost=mudconf.createmin;
room=create_obj(own,TYPE_ROOM,"Bridge",0);
zon=create_obj(own,TYPE_THING,"Zone Object",cost);
sprintf(buff,"Airlock<%d>;%d",VMCurrentMax+1,VMCurrentMax+1);
ai=create_obj(own,TYPE_EXIT,buff,0);
s_Exits(ai,yard);
s_Next(ai,Exits(yard));
s_Exits(yard,ai);
s_Flags2(ai,Flags2(ai) | SHIPIN);
s_Location(ai,room);
ao=create_obj(own,TYPE_EXIT,buff,0);
s_Exits(ao,room);
s_Next(ao,Exits(room));
s_Exits(room,ao);
s_Location(ao,yard);
s_Flags2(ao,Flags2(ao) | SHIPEXIT);
VMCurrentMax++;

VMSpace[VMCurrentMax].VMTemp.TmpNav.airlockout=ao;
VMSpace[VMCurrentMax].VMTemp.TmpNav.airlockin=ai;
VMSpace[VMCurrentMax].VMPerm.mainbridge=room;


s_Home(zon,new_home(1));
sprintf(ptr,"%d",VMCurrentMax);
db[room].zone=zon;
db[ai].zone=zon;
db[ao].zone=zon;
atr_add_raw(zon,SHIP_A,ptr);
vput_attribute(room,"XTYPE","VMROOM");
s_Flags2(room,Flags2(room) | CONSOLE);
CreateNewSpecialObject(1,room);

VMInstallHull(VMCurrentMax,VMTemplate[temp].hull);
for(i=0;i<20;i++) {
if(VMTemplate[temp].nav[i]!=0 || i==0)
VMInstallNavSys(VMCurrentMax,VMTemplate[temp].nav[i],i);
}
for(i=0;i<20;i++) {
if(VMTemplate[temp].eng[i]!=0)
VMInstallEngSys(VMCurrentMax,VMTemplate[temp].eng[i],i);
}
for(i=0;i<25;i++) {
if(VMTemplate[temp].helm[i]!=0)
VMInstallHelmSys(VMCurrentMax,VMTemplate[temp].helm[i],i,VMTemplate[temp].hloc[i]);
}



VMInitTmpShip2(VMCurrentMax);
/*VMAddDock(ai);*/
VMSpace[VMCurrentMax].VMPerm.Flags=setflag(VMSpace[VMCurrentMax].VMPerm.Flags,DOCKED);
VMSpace[VMCurrentMax].VMPerm.Nav.Dock=yard;
VMSetSector(VMCurrentMax);
VMSave();
}



void do_DPlace(player,data,buffer)
	int player;
	void *data;
	char *buffer;
{

char *fargs[5];
char *name;
int i,ship,dock;
i=xcode_parse(buffer,fargs,2);
if(i!=2) {
	VMnotify(player,"Usage: PLACE <SHIP#> <DOCK DB#>");
	return;
	}

ship=atoi(fargs[0]);
dock=atoi(fargs[1]);
if( ship < 0 || ship > VMCurrentMax) {
	VMnotify(player,"Invalid Ship Number");
	return;
	}
if(dock < 0 ) {
	VMnotify(player,"Invalid Dock DB#");
	return;
	}

if(!(Flags2(dock) & SHIPIN) && isRoom(dock)) {
	VMnotify(player,"Invalid Dock DB#");
	return;
	}

if(!checkflag(VMSpace[ship].VMPerm.Flags,DOCKED)) {
	VMnotify(player,"That ship is not docked");
	return;
	}
	

VMSpace[ship].VMPerm.Nav.Dock=dock;
VMnotify(player,"Ship Placed");
}


