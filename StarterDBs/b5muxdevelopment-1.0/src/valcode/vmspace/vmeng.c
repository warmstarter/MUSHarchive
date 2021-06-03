/* $Id: vmeng.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
#include "header.h"

static char *rcsvmengc="$Id: vmeng.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";

int CanAccessEng(int ship,int playerr)
{

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
  if(!IsPlayerInManList(VMSpace[ship].VMTemp.TmpEng.VMMan_List,playerr))
    {
      VMnotify(playerr,"You must man the eng console");
      return 0;
    }
  return 1;
}


void VMEngStatus(player,ship)
     int player;
     int ship;
     
{
  int per,i;
  VMnotify(player,"Engineering status report:");
  VMnotify(player,"----Reactor---------------------------------------------------");
  VMnotify(player,"Reactor is %s",IsShipInSpaceList(VMShipsPowered,ship) ? "on" : "off");
  VMnotify(player,"Reactor is running at %d percent", VMSpace[ship].VMPerm.Eng.Systems[0].tistat1);
  VMnotify(player,"Reactor max is %d",VMSpace[ship].VMPerm.Eng.Systems[0].istat2);
  VMnotify(player,"Reactor optimal is %d",VMSpace[ship].VMPerm.Eng.Systems[0].istat1);
  VMnotify(player,"Reactor safeties are %s.",checkflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,SAFETYFLAG) ? "on" : "off");
  VMnotify(player,"Reactor fuel is %5.2f/%5.2f",VMSpace[ship].VMPerm.Eng.Systems[0].dstat4,VMSpace[ship].VMPerm.Eng.Systems[0].dstat5);
  VMnotify(player,"----Batteries-------------------------------------------------");
  if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[1].flags,EXISTS)) { 
  VMnotify(player,"Batteries are %s.",checkflag(VMSpace[ship].VMPerm.Eng.Systems[1].flags,BATTSONFLAG) ? "on" : "off");
if (VMSpace[ship].VMPerm.Eng.Systems[1].istat2 > 0)
  VMnotify(player,"Batteries power is at %d/%d (%d%%)",VMSpace[ship].VMPerm.Eng.Systems[1].istat1,VMSpace[ship].VMPerm.Eng.Systems[1].istat4,(int) ((VMSpace[ship].VMPerm.Eng.Systems[1].istat1/VMSpace[ship].VMPerm.Eng.Systems[1].istat4)*100));
  if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[1].flags,BATTSONFLAG))
    {
      VMnotify(player,"Batteries are draining %2.2f percent every turn.",VMSpace[ship].VMPerm.Eng.Systems[1].dstat1);
      VMnotify(player,"Batteries are discharging %d power every turn.",VMSpace[ship].VMPerm.Eng.Systems[1].istat3);
    }
  }
   else {
	VMnotify(player,"This ship has no batteries.");
}
  
  VMnotify(player,"----Power-----------------------------------------------------");
  VMnotify(player,"Power available: %d", VMSpace[ship].VMTemp.TmpEng.PowerAvail);
  VMnotify(player,"Power to Helm: %d Nav: %d Communications: %d Batts: %d EDC: %d", VMSpace[ship].VMTemp.TmpEng.PowerHelm,VMSpace[ship].VMTemp.TmpEng.PowerNav,VMSpace[ship].VMTemp.TmpEng.PowerComm,VMSpace[ship].VMPerm.Eng.Systems[1].tistat0,VMSpace[ship].VMPerm.Eng.Systems[2].tistat0);
  VMnotify(player,"----General-----------------------------------------------------");
  VMnotify(player,"----- System Status ------------------------------");
  VMnotify(player,"System Name\t\t\tStatus");
  for (i = 0; i < ESystems;i++) {
    if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[i].flags,EXISTS)) {
      if (VMSpace[ship].VMPerm.Eng.Systems[i].mintegrity != 0) {
	per =  (VMSpace[ship].VMPerm.Eng.Systems[i].integrity / VMSpace[ship].VMPerm.Eng.Systems[i].mintegrity) * 100;
      }
      else
	per = -1;
      VMnotify(player,"%s\t\t\t %2.2f/%2.2f (%d%%)",VMSpace[ship].VMPerm.Eng.Systems[i].name,VMSpace[ship].VMPerm.Eng.Systems[i].integrity,VMSpace[ship].VMPerm.Eng.Systems[i].mintegrity,per);
    }
  }


  if (VMSpace[ship].VMPerm.mIntegrity > 0) {
    per = (int) VMSpace[ship].VMPerm.Integrity/VMSpace[ship].VMPerm.mIntegrity * 100;
  }
  else
    per = -1;
  VMnotify(player,"Hull integrity %2.2f/%2.2f (%d%%)",VMSpace[ship].VMPerm.Integrity,VMSpace[ship].VMPerm.mIntegrity,per);
}


void VMEngStartReactor(player,ship)
     int player;
     int ship;
{
int i;
  if (IsShipInSpaceList(VMShipsPowered,ship)) {
    VMnotify(player,"Reactor is already running.");
    return;
  }
  AddSpaceEnt(VMShipsPowered,ship);
  VMSpace[ship].VMPerm.Eng.Systems[0].tistat1 = 0;
  VMEngReallocPower(ship);
  VMnotify(player,"Reactor started.");
  if (VMSpace[ship].VMPerm.Eng.Systems[0].dstat4 > 0.0)
    VMSpace[ship].VMPerm.Eng.Systems[0].flags = clearflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,NOGASFLAG);
  else
    VMSpace[ship].VMPerm.Eng.Systems[0].flags = setflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,NOGASFLAG);

/*Evil little communication init*/
for(i=0;i<10;i++) VMSpace[ship].VMTemp.TmpCommu.freqs[i]=-1;
}


void VMEngStopReactor(player,ship)
     int player;
     int ship;
{
  if (VMGlobalSpaceInit == 0) {
    VMnotify(player,"Space has not been initialized.");
    return;
  }

  if (!IsShipInSpaceList(VMShipsPowered,ship)) {
    VMnotify(player,"Reactor is already off.");
    return;
  }
  if (VMSpace[ship].VMTemp.TmpEng.PowerAvail != 0) {
    VMnotify(player,"Ship must not be generating power for the reactor to be shutdown");
    return;
  }
  
  RemoveSpaceEnt(VMShipsPowered);
  if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[1].flags,BATTSONFLAG))
    VMEngBattsOff(player,ship);
  VMnotify(player,"Reactor is now off.");
 if(IsRoomInManList(Commus,ship)) RemoveManEnt(Commus);
}

void VMEngSetAllocs(player,ship,hpow,npow,bpow,epow,opow)
     int player;
     int ship;
     double hpow;
     double npow;
     double bpow;
     double epow;
     double opow;
{

  double whole;
  if (hpow > VMSpace[ship].VMTemp.TmpEng.PowerAvail) {
     VMnotify(player, "Power out of bounds.");
     return;
  }
  if (npow >VMSpace[ship].VMTemp.TmpEng.PowerAvail) {
     VMnotify(player, "Power out of bounds.");
     return;
  }
  if (bpow > VMSpace[ship].VMTemp.TmpEng.PowerAvail) {
     VMnotify(player, "Power out of bounds.");
     return;
  }
  if (epow > VMSpace[ship].VMTemp.TmpEng.PowerAvail) {
     VMnotify(player, "Power out of bounds.");
     return;
  }
  if (opow > VMSpace[ship].VMTemp.TmpEng.PowerAvail) {
     VMnotify(player, "Power out of bounds.");
     return;
  }

  whole =  hpow + npow + bpow + epow + opow;
  if (whole == 0){
    VMSpace[ship].VMPerm.Eng.PowerPerHelm =0; 
    VMSpace[ship].VMPerm.Eng.PowerPerNav = 0; 
    VMSpace[ship].VMPerm.Eng.PowerPerBatts =0;
    VMSpace[ship].VMPerm.Eng.PowerPerEDC = 0;
    VMSpace[ship].VMPerm.Eng.PowerPerComm = 0;
  }
  else {
    VMSpace[ship].VMPerm.Eng.PowerPerHelm =   hpow / whole;
    VMSpace[ship].VMPerm.Eng.PowerPerNav =  npow / whole;
    VMSpace[ship].VMPerm.Eng.PowerPerBatts = bpow / whole;
    VMSpace[ship].VMPerm.Eng.PowerPerEDC = epow / whole;
    VMSpace[ship].VMPerm.Eng.PowerPerComm = opow / whole;
  }
  
  VMEngUpdateEngPower(ship);
  VMEngReallocPower(ship);

  VMnotify(player,"Allocations set.");

}


     
void VMEngSetReactor(player,ship,setting)
     int player;
     int ship;
     int setting;
{
  if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,NOGASFLAG)) {
    VMnotify(player,"This ship is out of gas.");
    return;
  }
  if (!IsShipInSpaceList(VMShipsPowered,ship)) {
    VMnotify(player,"This ship's reactor is off.");
    return;
  }
  
  /* range check on the setting */
  if (setting < 0 || setting > VMSpace[ship].VMPerm.Eng.Systems[0].istat2)
    {
      VMnotify(player,"[Engineering System] Unable to set reactor to %d, Valid settings are 0 to %d.",setting,VMSpace[ship].VMPerm.Eng.Systems[0].istat2);
      return;
    }
  
  /* we are valid. Set the setting, notify everyone and realloc the power */

  VMSpace[ship].VMPerm.Eng.Systems[0].tistat1 =setting;
  VMnotifymans(VMSpace[ship].VMTemp.TmpEng.VMMan_List,"Reactor setting change by %s. New Setting is %d.",Name(player),VMSpace[ship].VMPerm.Eng.Systems[0].tistat1);
  VMEngUpdateEngPower(ship);
}

void VMEngUpdateEngPower(ship)
     int ship;
{
  if (VMSpace[ship].VMPerm.Eng.Systems[0].basenumber == 0) {
    if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,NOGASFLAG))
      {
	VMSpace[ship].VMTemp.TmpEng.PowerAvail = VMSpace[ship].VMPerm.Eng.Systems[1].istat3;
	VMSpace[ship].VMTemp.TmpEng.TotalUsed = VMSpace[ship].VMTemp.TmpEng.PowerHelm + VMSpace[ship].VMTemp.TmpEng.PowerNav + VMSpace[ship].VMPerm.Eng.Systems[BATTERIES_SYS].tistat0 + VMSpace[ship].VMPerm.Eng.Systems[EDC_SYS].tistat0; 
      }
    else {
      VMSpace[ship].VMTemp.TmpEng.PowerAvail =  ((VMSpace[ship].VMPerm.Eng.Systems[0].tistat1 * VMSpace[ship].VMPerm.Eng.Systems[0].istat3) / 100) + VMSpace[ship].VMPerm.Eng.Systems[1].istat3;
      VMSpace[ship].VMTemp.TmpEng.TotalUsed = VMSpace[ship].VMTemp.TmpEng.PowerHelm + VMSpace[ship].VMTemp.TmpEng.PowerNav + VMSpace[ship].VMTemp.TmpEng.PowerComm + VMSpace[ship].VMPerm.Eng.Systems[BATTERIES_SYS].tistat0 + VMSpace[ship].VMPerm.Eng.Systems[EDC_SYS].tistat0;
    }
  }
  else {
    if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,NOGASFLAG)) {
      VMSpace[ship].VMTemp.TmpEng.PowerAvail = VMSpace[ship].VMPerm.Eng.Systems[1].istat3;
      VMSpace[ship].VMTemp.TmpEng.TotalUsed = VMSpace[ship].VMTemp.TmpEng.PowerHelm + VMSpace[ship].VMTemp.TmpEng.PowerNav + VMSpace[ship].VMTemp.TmpEng.PowerComm + VMSpace[ship].VMPerm.Eng.Systems[BATTERIES_SYS].tistat0 + VMSpace[ship].VMPerm.Eng.Systems[EDC_SYS].tistat0;
    }
    else {
      VMSpace[ship].VMTemp.TmpEng.PowerAvail =  ((VMSpace[ship].VMPerm.Eng.Systems[0].tistat1 * VMSpace[ship].VMPerm.Eng.Systems[0].istat3) / VMSpace[ship].VMPerm.Eng.Systems[0].basenumber) + VMSpace[ship].VMPerm.Eng.Systems[1].istat3;
      VMSpace[ship].VMTemp.TmpEng.TotalUsed = VMSpace[ship].VMTemp.TmpEng.PowerHelm + VMSpace[ship].VMTemp.TmpEng.PowerNav + VMSpace[ship].VMTemp.TmpEng.PowerComm + VMSpace[ship].VMPerm.Eng.Systems[BATTERIES_SYS].tistat0 + VMSpace[ship].VMPerm.Eng.Systems[EDC_SYS].tistat0;
    }
  }
  
  if (VMSpace[ship].VMTemp.TmpEng.TotalUsed != VMSpace[ship].VMTemp.TmpEng.PowerAvail) 
    VMEngReallocPower(ship);
      
}

  

void VMEngBattsOn(int player, int ship)

{
  if (!IsShipInSpaceList(VMShipsPowered,ship)) {
    VMnotify(player,"Reactor must be on, for batteries to be used");
    return;
  }
    
  if(checkflag(VMSpace[ship].VMPerm.Eng.Systems[1].flags,BATTSONFLAG))
    {
      VMnotify(player,"Batteries are already on");
      return;
    }
  else
    {
      VMSpace[ship].VMPerm.Eng.Systems[1].flags = setflag(VMSpace[ship].VMPerm.Eng.Systems[1].flags,BATTSONFLAG);
      VMnotifymans(VMSpace[ship].VMTemp.TmpEng.VMMan_List,"[ENGINEERING] Batteries are now online");
      VMEngUpdateBattsPower(ship,1);
      VMEngUpdateEngPower(ship);
      /*      VMEngReallocPower(ship);*/
    }

    
}
void VMEngUpdateBattsPower ( int ship
			     ) {

  if (VMSpace[ship].VMPerm.Eng.Systems[1].istat1 > VMSpace[ship].VMPerm.Eng.Systems[1].istat4)
    {
      VMSpace[ship].VMPerm.Eng.Systems[1].istat3 = VMSpace[ship].VMPerm.Eng.Systems[1].istat4;
    }
  else
    {
      VMSpace[ship].VMPerm.Eng.Systems[1].istat3 = 0;
      VMnotifymans(VMSpace[ship].VMTemp.TmpEng.VMMan_List,"Batteries drained.");
    }
}

void VMEngBattsOff(int player, int ship)

{
  if(checkflag(VMSpace[ship].VMPerm.Eng.Systems[1].flags,BATTSONFLAG))
    {
      /* clear the power the batteries are giving. */
      VMSpace[ship].VMPerm.Eng.Systems[1].istat3 = 0;
      VMSpace[ship].VMPerm.Eng.Systems[1].flags = clearflag(VMSpace[ship].VMPerm.Eng.Systems[1].flags,BATTSONFLAG);
      VMnotifymans(VMSpace[ship].VMTemp.TmpEng.VMMan_List,"[ENGINEERING] Batteries are now offline");
      VMEngUpdateEngPower(ship);
    }
  else
    {
      VMnotify(player,"Batteries are already off");
    }
}

void VMEngSafety(int player, int ship, int what)

{
  if (what ==1) { 
    /* turn safeties on */
    if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,SAFETYFLAG))
      {
	VMnotify(player,"Safeties are already on.");
	return;
      }
    else
      {
	VMnotify(player,"Safeties are now on.");
	VMSpace[ship].VMPerm.Eng.Systems[0].flags = setflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,SAFETYFLAG);
	return;
      }
  }
  else if (what == 2){
    /* turn safeties off */
    if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,SAFETYFLAG))
      {
	VMnotify(player,"Safeties are now off.");
	VMSpace[ship].VMPerm.Eng.Systems[0].flags = clearflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,SAFETYFLAG);
	return;
      }
    else
      {
	VMnotify(player,"Safeties are already off.");
	return;
      }
  }
}


void VMEngReallocPower(ship)
     int ship;
{
  /*  VMnotify(1,"PowerPerHelm is %.5f | PowerPerNav is: %.5f | PowerPerBatts is: %.5f | PowerPerEDC is %.5f",VMSpace[ship].VMPerm.Eng.PowerPerHelm,VMSpace[ship].VMPerm.Eng.PowerPerNav,VMSpace[ship].VMPerm.Eng.PowerPerBatts,VMSpace[ship].VMPerm.Eng.PowerPerEDC);*/
  VMnotifymans(VMSpace[ship].VMTemp.TmpEng.VMMan_List,"[Engineering] System Reallocation");
  VMSpace[ship].VMTemp.TmpEng.PowerHelm =  VMSpace[ship].VMPerm.Eng.PowerPerHelm * VMSpace[ship].VMTemp.TmpEng.PowerAvail;
  VMSpace[ship].VMTemp.TmpEng.PowerNav =  VMSpace[ship].VMPerm.Eng.PowerPerNav * VMSpace[ship].VMTemp.TmpEng.PowerAvail;
  VMSpace[ship].VMTemp.TmpEng.PowerComm =  VMSpace[ship].VMPerm.Eng.PowerPerComm * VMSpace[ship].VMTemp.TmpEng.PowerAvail;
  VMSpace[ship].VMPerm.Eng.Systems[1].tistat0 =  VMSpace[ship].VMPerm.Eng.PowerPerBatts * VMSpace[ship].VMTemp.TmpEng.PowerAvail;
  VMSpace[ship].VMPerm.Eng.Systems[2].tistat0 =  VMSpace[ship].VMPerm.Eng.PowerPerEDC * VMSpace[ship].VMTemp.TmpEng.PowerAvail;

  /* did we do a good job??? */

  while ((VMSpace[ship].VMTemp.TmpEng.PowerHelm + VMSpace[ship].VMTemp.TmpEng.PowerNav + VMSpace[ship].VMTemp.TmpEng.PowerComm + VMSpace[ship].VMPerm.Eng.Systems[1].tistat0 + VMSpace[ship].VMPerm.Eng.Systems[2].tistat0) != VMSpace[ship].VMTemp\
.TmpEng.PowerAvail)
    VMSpace[ship].VMPerm.Eng.Systems[2].tistat0++;
  
  VMnotifymans(VMSpace[ship].VMTemp.TmpEng.VMMan_List,"[Engineering] Power to Helm: %d Nav: %d Communications: %d Batteries: %d EDC: %d ",VMSpace[ship].VMTemp.TmpEng.PowerHelm,VMSpace[ship].VMTemp.TmpEng.PowerNav,VMSpace[ship].VMTemp.TmpEng.PowerComm,VMSpace[ship].VMPerm.Eng.Systems[1].tistat0,VMSpace[ship].VMPerm.Eng.Systems[2].tistat0);
  if (VMSpace[ship].VMTemp.TmpEng.PowerNav != VMSpace[ship].VMTemp.TmpNav.PowerUsed )
    VMNavReallocPower(ship);
  
}






