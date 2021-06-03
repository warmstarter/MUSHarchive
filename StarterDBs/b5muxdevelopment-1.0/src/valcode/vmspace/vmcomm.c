/* $Id: vmcomm.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

#include "header.h"

extern void VMTransmit(double,char *, int ,double);

static char *vmcommcrcs="$Id: vmcomm.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
void VMCommTran(player, ship, chan,buffer)
int player,chan,ship;
char *buffer;
{
t_SpaceEnt *tmp;
float frq;
double pw2,i1,i2;
double pw;
if(!IsRoomInManList(Commus,ship)) {
	VMnotify(player,"Communications are not on");
	return;
	}
if(chan < 0 || chan > 9) {
	VMnotify(player,"Invalid Channel.");
	return;
	}
frq=VMSpace[ship].VMTemp.TmpCommu.freqs[chan];
if(frq<0) {
	VMnotify(player,"That Channel is closed.");
	return;
	}
/*
ResetSpaceList(VMShipsPowered);
while(!AtEndSpaceList(VMShipsPowered)) {
	tmp=GetCurrSpaceEnt(VMShipsPowered);
		if(CanShipsTalk(ship,tmp->VMShip,frq)) {
			VMnotifymans(VMSpace[tmp->VMShip].VMTemp.TmpCommu.VMMan_List,"[%f] %s",frq,buffer);
		}
AdvanceSpaceList(VMShipsPowered);
}
*/
pw=(double)VMSpace[ship].VMTemp.TmpEng.PowerComm;
i2=(double)VMSpace[ship].VMPerm.Eng.Systems[3].istat1;
i1=(double)VMSpace[ship].VMPerm.Eng.Systems[3].istat2;
if(i1!=0) pw=pw/i1;
else pw=0;
pw=pw*i2;
pw2=(int)pw;
VMTransmit((double)frq,buffer,ship,pw);
VMnotify(player,tprintf("Message sent over frequency %f.",frq));
}

int CanAccessComm(int ship,int playerr)
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
	

  if(!IsPlayerInManList(VMSpace[ship].VMTemp.TmpCommu.VMMan_List,playerr))
    {
      VMnotify(playerr,"You must man the Commu console");
      return 0;
      
    }
  return 1;
}








void VMCommStatus(player,ShipNum)
int player;
int ShipNum;
{
int k;
int i1,i2;
double pw;
  if(!CanAccessComm(ShipNum,player)) 
    return;

VMnotify(player,"\n-----------------------------------------------");
VMnotify(player," =	     Communications Status            =");
VMnotify(player,"-----------------------------------------------");
VMnotify(player,"   Channel      Frequency/Status\n");
for(k=0;k<10;k++) {
	if( VMSpace[ShipNum].VMTemp.TmpCommu.freqs[k]!=-1)
		if(VMSpace[ShipNum].VMTemp.TmpCommu.cd[k]==1)
			VMnotify(player,tprintf("    %2d)          %3.6f ENCODED",k,VMSpace[ShipNum].VMTemp.TmpCommu.freqs[k]));
		else 
			VMnotify(player,tprintf("    %2d)          %3.6f",k,VMSpace[ShipNum].VMTemp.TmpCommu.freqs[k]));
	else
		VMnotify(player,tprintf("    %2d)             CLOSE",k));
 }

if(VMSpace[ShipNum].VMTemp.TmpCommu.Relay!=-1) {
	VMnotify(player,"  Locked onto Relay %d of Ship %d",VMSpace[ShipNum].VMTemp.TmpCommu.Relay+1,VMSpace[ShipNum].VMTemp.TmpCommu.RShip);
  }
pw=(double)VMSpace[ShipNum].VMTemp.TmpEng.PowerComm;
i2=VMSpace[ShipNum].VMPerm.Eng.Systems[3].istat1;
i1=VMSpace[ShipNum].VMPerm.Eng.Systems[3].istat2;
if(i1 !=0) pw=pw/i1;
else pw=0;
pw=pw*i2;
pw=pw/10.0;

VMnotify(player,"\n  Effective Range = %lf",pw);
VMnotify(player,"-----------------------------------------------\n");

}

void VMCommOn(player,ShipNum)
dbref player;
int ShipNum;
{

  if(!CanAccessComm(ShipNum,player)) 
    return;

if(VMSpace[ShipNum].VMTemp.TmpEng.PowerComm<=0) {
	VMnotify(player,"No power to communications");
	return;
	}
if(!checkflag(VMSpace[ShipNum].VMPerm.Eng.Systems[3].flags,EXISTS)) {
	VMnotify(player,"This ship has no communication system.");
	return;
	}
if(IsRoomInManList(Commus,ShipNum)) {
	VMnotify(player,"Communications are already on");
	return;
	}
	AddManEnt(Commus,-1,ShipNum);
	VMnotify(player,"Communications engaged");
}

void VMCommOff(player,ShipNum)
dbref player;
int ShipNum;
{

  if(!CanAccessComm(ShipNum,player)) 
    return;

if(!IsRoomInManList(Commus,ShipNum)) {
	VMnotify(player,"Communications are not on");
	return;
	}
	VMnotify(player,"Communications shutdown");
	RemoveManEnt(Commus);
}


void VMCommOpen(player,ShipNum,channel,freq)
dbref player;
int ShipNum,channel;
float freq;
{

  if(!CanAccessComm(ShipNum,player)) 
    return;

if(channel <0 || channel >9) {
	VMnotify(player,"Invalid channel");
	return;
	}
if(freq < 0 || freq >=32) {
	VMnotify(player,"Invalid frequency");
	return;
	}
	VMnotify(player,"Channel set");
	VMSpace[ShipNum].VMTemp.TmpCommu.freqs[channel]=freq;

}

void VMCommClose(player,ShipNum,channel)
dbref player;
int ShipNum,channel;
{

  if(!CanAccessComm(ShipNum,player)) 
    return;

if(channel <0 || channel >9) {
	VMnotify(player,"Invalid channel");
	return;
	}
	VMnotify(player,"Channel Closed");
	VMSpace[ShipNum].VMTemp.TmpCommu.freqs[channel]=-1;

}
void VMCommScan(player,ShipNum,ship)
dbref player;
int ShipNum,ship;
{
t_SpaceList *tmpshort;
t_RelayList *relays;
t_RelayEnt *tmp;
int i;

  if(!CanAccessComm(ShipNum,player)) 
    return;

tmpshort=VMSpace[ShipNum].VMTemp.TmpNav.VMShortShips;

if(!IsShipInSpaceList(tmpshort,ship) && ship!=ShipNum) {
	VMnotify(player,"That ship is not on short range sensors.");
	return;
	}

	
relays=VMSpace[ship].VMTemp.TmpNav.VMRelayList;
ResetRelayList(relays);
VMnotify(player,tprintf("\nRelays on Ship %d\n---------------------------------------------------------------",ship));
i=1;
while(!AtEndRelayList(relays)) {
	tmp=GetCurrRelayEnt(relays);
	if(tmp==NULL)
	  VMnotify(player,"Problem!");
	else
	VMnotify(player,tprintf("%2d) %30s",i,Name(tmp->VMRoom)));
	i++;
	if(AtEndRelayList(relays)) break;
	AdvanceRelayList(relays);
	}
VMnotify(player,"---------------------------------------------------------------\n");
}


void VMCommCCode(player,ShipNum,channel,code)
dbref player;
int ShipNum,channel;
char *code;
{

  if(!CanAccessComm(ShipNum,player)) 
    return;

if(channel <0 || channel >9) {
	VMnotify(player,"Invalid channel");
	return;
	}
if( strlen(code) > 45) {
	VMnotify(player,"Code too long");
	return;
	}
	sprintf(VMSpace[ShipNum].VMTemp.TmpCommu.code[channel],"%s",code);
	VMSpace[ShipNum].VMTemp.TmpCommu.cd[channel]=1;
	VMnotify(player,"Channel encoded");
}


void VMCommDCode(player,ShipNum,channel,code)
dbref player;
int ShipNum,channel;
char *code;
{

  if(!CanAccessComm(ShipNum,player)) 
    return;

if(channel <0 || channel >9) {
	VMnotify(player,"Invalid channel");
	return;
	}
	VMSpace[ShipNum].VMTemp.TmpCommu.cd[channel]=0;
	VMnotify(player,"Channel uncoded");
}



