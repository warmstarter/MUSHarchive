/* $Id: vmflags.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
#include "header.h"


#include "vmflags.h"

static char *rcsvmflagsc="$Id: vmflags.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
t_Flag Flags[]={
{"simspace",SIMSHIP},
{"simspace2",SIMSHIP2},
{"simspace3",SIMSHIP3},
{"simspace4",SIMSHIP4},
{"simspace5",SIMSHIP5},
{"vorlon",VORLON},
{"hyperspace",HYPERSPACE},
{"jumpgate",JUMPGATE},
{"nebula",NEBULA},
{"special",SPECIAL},
{"base",BASE},
{"canland",CANLAND},
{"docked",DOCKED},
{"dead",DEAD},
{"planet",PLANET},
{"ship",SHIP},
{"holdsmallships",HOLDSMALLSHIPS},
{"holdsbigships",HOLDBIGSHIPS},
{"candock",CANDOCK},
{"smalship",SMALLSHIP},
{"sun",SUN},
{"beacon",BEACON},
{"fighter",FIGHTER},
{"mortal",VMMORTAL},
{"none",-1}
};


void VMFlag(player,ShipNum,flagname)
int player,ShipNum;
char *flagname;
{
int i=0,not=0;
if(ShipNum < 0 || ShipNum > VMCurrentMax)
 {
	VMnotify(player,"Invalid Ship Number");
	return;
 }

if(*flagname=='!') {
	not=1;
	flagname++;
	}

while(Flags[i].FlagNum!=-1) {
 if(!strcmp(Flags[i].name,flagname)) {
if(!not){
VMSpace[ShipNum].VMPerm.Flags=setflag(VMSpace[ShipNum].VMPerm.Flags,Flags[i].FlagNum);
 	VMnotify(player,"Flag Set");
	return;
		}
else{
VMSpace[ShipNum].VMPerm.Flags=clearflag(VMSpace[ShipNum].VMPerm.Flags,Flags[i].FlagNum);
 	VMnotify(player,"Flag Cleared");
	return;
		}
	}
 i++;
 }
 VMnotify(player,"No such Flag");
}


void VMSeeFlag(player,ShipNum)
int player,ShipNum;
{
int i=0;
if(ShipNum < 0 || ShipNum > VMCurrentMax)
 {
	VMnotify(player,"Invalid Ship Number");
	return;
 }
VMnotify(player,"Flags on Ship %3d",ShipNum);
VMnotify(player,"-----------------");
while(Flags[i].FlagNum!=-1) {
 if(checkflag(VMSpace[ShipNum].VMPerm.Flags,Flags[i].FlagNum)) {
  VMnotify(player,"%s",Flags[i].name);
	 }
 i++;
  }
VMnotify(player,"-----------------");
}



