#include "header.h"
#include "vmweaptypes.h"

#define ADSCHAR 602

void ADS_Level(player,ship,level)
int player,ship,level;
{

if(level==0) {
	VMSpace[ship].VMTemp.TmpHelm.ADSTarget=0;
	VMnotify(player,"ADS Disengaged.");
	return;
}
VMSpace[ship].VMTemp.TmpHelm.ADSOn=2;	
VMnotify(player,"ADS Engaged.");

}

void ADS_NotifyFire(ship,target)
int ship,target;
{

if(VMSpace[ship].VMTemp.TmpHelm.ADSTarget==target) return;
ADS_NotifyLock(ship,target);
}


void ADS_NotifyLock(ship,target)
int ship,target;
{
int i,flags;
t_SpaceList *tmpshort;

tmpshort=VMSpace[ship].VMTemp.TmpNav.VMShortShips;

if(VMSpace[ship].VMTemp.TmpHelm.ADSOn>1 ) {
/*	if (IsShipInSpaceList(tmpshort,target) ) { */
for(i=0;i<HSystems;i++) {
	flags=VMSpace[ship].VMPerm.Helm.Systems[i].flags;
	if (checkflag(flags,EXISTS) && checkflag(flags,WEAPONLINE) && !checkflag(flags,NONWEAP))
		VMSpace[ship].VMPerm.Helm.Systems[i].tistat2=target;
}
VMnotifyAll(VMSpace[ship].VMPerm.mainbridge,"*** All weapons locked onto Ship %d ***",target);
VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"*** Weapons lock from ship %d detected. ***",ship);
VMSpace[ship].VMTemp.TmpHelm.ADSTarget=target;
ADS_CheckFire(ship);
/*		}*/
	}
}

void ADS_NotifyLost(ship,target)
int ship,target;
{
int i,flags;
if(VMSpace[ship].VMTemp.TmpHelm.ADSOn>1) {
for(i=0;i<HSystems;i++) {
	flags=VMSpace[ship].VMPerm.Helm.Systems[i].flags;
	if (checkflag(flags,EXISTS) && checkflag(flags,WEAPONLINE) && !checkflag(flags,NONWEAP))
		VMSpace[ship].VMPerm.Helm.Systems[i].tistat2=0;
}
VMnotifyAll(VMSpace[ship].VMPerm.mainbridge,"*** All weapons unlocked from Ship %d ***",target);
VMnotifyAll(VMSpace[target].VMPerm.mainbridge,"*** Weapons lock dropped from ship %d. ***",ship);
VMSpace[ship].VMTemp.TmpHelm.ADSTarget=0;
}


}


void ADS_CheckFire(ship)
int ship;
{
int i;
if(VMSpace[ship].VMTemp.TmpHelm.ADSOn>1 && VMSpace[ship].VMTemp.TmpHelm.ADSTarget>0) {

	for(i=0;i<HSystems;i++)  {
    		if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,GUNFLAG)) {
			if ((VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 == VMSpace[ship].VMPerm.Helm.Systems[i].istat4)) { 
				ADS_Fire(ship,i);
			}
		}
		else ADS_Fire(ship,i);

	}


}

}



void ADS_Fire(ship,system)
int ship,system;
{
VMHelmFireWeapon(ship,ADSCHAR,system);
}

