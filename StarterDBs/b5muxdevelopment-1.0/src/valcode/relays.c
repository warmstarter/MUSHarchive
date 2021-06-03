/*
 * relays.c - 
 */
#include "header.h"
#include "relays.h"
#include "./vmspace/vmspacelist.h"
#include "comm.h"
#include "vmrelaylist.h"

#define FOOFMAXRELRNG 20000000

t_RelayList *RelayList;
int CheckShipRelCode(int ship, float frq);
int CheckShipCodes(int ship1, int ship2, float frq);
int CanShipsTalk( int ship1, int ship2, float frq);
int CanRelaysTalk( int rel1, int rel2, float frq);
int CheckRange(int ship1, int ship2);

void do_code();
void relay_status();

void do_code(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref console,vehicle,playe;
char *fargs[5],ptr[50];
int i,commodity,amount,eq,cd;


if(!IsIcReg(player)) return;

i=xcode_parse(buffer,fargs,2);
if(i!=2) {
	notify(player,"Usage: CODE <FREQUENCY> <CODE>");
	return;
	}
console=where_is(player);

if(atoi(fargs[0]) <= 0 || atoi(fargs[0]) > 30) {
	notify(player,"Invalid Frequency Range");
	return;
	}
cd=atoi(fargs[1]);
if(cd==-1) {
	notify(player,"Invalid code.");
	return;
	}
sprintf(ptr,"%s",read_slist(console,atoi(fargs[0]),ATRIBS_A));
eq=atoi(ptr);
if(eq==-1) {
	notify(player,"That frequency range is not currently being relayed.");
	return;
	}
change_slist(console,atoi(fargs[0]),fargs[1],ATRIBS_A);
notify(player,"New Relay Code Set. \nThe relay network will be updated in a few moments...");

}


static int RelayInit=0;

void setup_network(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
do_setup_network();
}


void do_setup_network()
{
dbref i;
int ship;
char *buf;
int vmid=0;

bzero(VMShipsCanTalkMatrix,sizeof(VMShipsCanTalkMatrix));
bzero(VMBackID,sizeof(VMBackID));
bzero(VMShipID,sizeof(VMShipID));
VMBackMax=0;

RelayInit=1;
RelayList=InitRelayList();
for(i=0;i<mudstate.db_top;i++) {
	if(Flags2(i) & CONSOLE ) {
		buf=vfetch_attribute(i,"XTYPE");
		if(!buf) { }
		else{
		if(strcmp(buf,"RELAY")==0) {
			ship=VMShipFromRoom(i);
/*VMnotify(2,tprintf("Adding room %d to ship %d as a relay",i,ship));*/
			if(ship >0 && ship < MAXSHIP) {
				/*if(VMShipID[ship]==0) { 
					VMBackMax++;
					VMShipID[ship]=VMBackMax;
				}
				*/
				AddRelayEnt(RelayList,ship,i,vmid);
					vmid++;
				
			}
		}
		}
	}
  }
do_calc_network();
}

void calc_network(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
do_calc_network();
}


void do_calc_network()
{
int i,ship1,ship2,dist,room1,room2;
int rix1,rix2;
int pwr,eq1,eq2;
long int frq;
t_RelayEnt *tmp,*item,*item2;
char code1[500],code2[500],*ptr;

if(RelayInit==0) {
	return;
	}
bzero(VMShipsCanTalkMatrix,sizeof(VMShipsCanTalkMatrix));
ResetRelayList(RelayList);
while(!AtEndRelayList(RelayList)) {
	tmp=SaveRelay(RelayList);
	item=GetCurrRelayEnt(RelayList);
	ship1=item->VMShip;
	room1=item->VMRoom;
	rix1=item->VMID;
	ResetRelayList(RelayList);
	while(!AtEndRelayList(RelayList)) {
		item2=GetCurrRelayEnt(RelayList);
		if(item!=item2) {
		ship2=item2->VMShip;
		room2=item2->VMRoom;
		/*rix1=VMShipID[ship1];
		rix2=VMShipID[ship2];*/
		rix2=item2->VMID;
		ptr=vget_a(room1,A_VA);
		pwr=atoi(ptr);
		ptr=vget_a(room2,A_VA);
		pwr+=atoi(ptr);
		dist=VMdistance(VMSpace[ship1].VMPerm.Nav.XYZCoords,VMSpace[ship2].VMPerm.Nav.XYZCoords);

                if(dist < FOOFMAXRELRNG) {
			for(i=1;i<30;i++) {
				sprintf(code1,"%s",read_slist(room1,i,ATRIBS_A));
				sprintf(code2,"%s",read_slist(room2,i,ATRIBS_A));
				eq1=atoi(code1);
				eq2=atoi(code2);
				if(strcmp(code1,code2)==0 && eq1!=-1 && eq2!=-1) {
					frq=pow(2,i);
					VMShipsCanTalkMatrix[rix1][rix2]= VMShipsCanTalkMatrix[rix1][rix2] | frq;
					}	
				}		
			}
		}	
		AdvanceRelayList(RelayList);
	}
	RestoreRelay(RelayList,tmp);
	AdvanceRelayList(RelayList);
	}
}

void check_network(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int i,ship1,ship2,dist,room1,room2;
int frqq,rix1,rix2;
long int frq;
char *fargs[3];

i=xcode_parse(buffer,fargs,3);
if(i!=3) { 
	notify(player,"Bad Valdar!");
	return;
	}

ship1=atoi(fargs[0]);
ship2=atoi(fargs[1]);
frqq=atoi(fargs[2]);
frq=pow(2,frqq);
/*rix1=VMShipID[ship1];
rix2=VMShipID[ship2];*/
/*rix1=VMSpace[ship1].VMTemp.TmpCommu.Relay;
rix2=VMSpace[ship2].VMTemp.TmpCommu.Relay;*/
rix1=ship1;
rix2=ship2;

if(VMShipsCanTalkMatrix[rix1][rix2] > 0)
	notify(player,"They SEE each other...");

if(!(VMShipsCanTalkMatrix[rix1][rix2] & frq))
	notify(player,"Those ships cant talk on that frequency");
else
	notify(player,"Alls clear for comm!");
notify(player,tprintf("The matrix element is %ld",VMShipsCanTalkMatrix[rix1][rix2]));
notify(player,tprintf("rix1 = %d, rix2=%d",rix1,rix2));
}


int CheckShipRelCode(ship, frq)
int ship;
float frq;
{
int chan=-1,i;
int eq,cd1,cd2,console;
char ptr[70];
t_RelayEnt *tmp; 
char cod[70];
long int ifrq;

for(i=0;i< 10; i++) {
 if(VMSpace[ship].VMTemp.TmpCommu.freqs[i]==frq) {
	chan=i;
	break;
	}
 }
if(chan==-1) return 0;

ifrq=(long int)frq;

console=-1;
if(VMSpace[ship].VMTemp.TmpCommu.Relay < 0 ) return 0;
tmp=VMSpace[ship].VMTemp.TmpCommu.tRelay;
console=tmp->VMRoom;

if(console<=0) return 0;

sprintf(ptr,"%s",read_slist(console,ifrq,ATRIBS_A));
eq=atoi(ptr);
if(eq==-1) {
	return 0;
	}
if(strcmp(ptr,"0")==0) return 1;
cd1=VMSpace[ship].VMTemp.TmpCommu.cd[chan];
if(cd1==0) {
	sprintf(cod,"0");
	}
else sprintf(cod,"%s",VMSpace[ship].VMTemp.TmpCommu.code[chan]);
if(strcmp(ptr,cod)==0) return 1;
return 0;

}


int CheckShipCodes(ship1, ship2, frq)
int ship1,ship2;
float frq;
{
int chan1=-1, chan2=-1,i;
int cd1,cd2;

/*VMnotify(2,"Here A, ship1=%d, ship2=%d, frq=%f",ship1,ship2,frq);*/
for(i=0;i< 10; i++) {
 if(VMSpace[ship1].VMTemp.TmpCommu.freqs[i]==frq) {
	chan1=i;
	break;
	}
 }
if(chan1==-1) return 0;
for(i=0;i< 10; i++) {
 if(VMSpace[ship2].VMTemp.TmpCommu.freqs[i]==frq) {
	chan2=i;
	break;
	}
 }
if(chan2==-1) return 0;
cd1=VMSpace[ship2].VMTemp.TmpCommu.cd[chan1];
cd2=VMSpace[ship2].VMTemp.TmpCommu.cd[chan2];
/*VMnotify(2,"Here. cd1 = %d, cd2=%d",cd1,cd2);*/
if(cd1==0) return 1;
if(cd1!=cd2) return 0;
if(strcmp(VMSpace[ship1].VMTemp.TmpCommu.code[chan1],VMSpace[ship2].VMTemp.TmpCommu.code[chan2])==0) return 1;
return 0;

} 

int CheckRange(ship1, ship2)
{
double r;
int b1,b2,range=0;
int assigned,basep;

assigned=VMSpace[ship1].VMTemp.TmpEng.PowerComm;
basep=VMSpace[ship1].VMPerm.Eng.Systems[3].istat2;
b1=basep;
if(basep>0) { 
basep=assigned/basep;
range=VMSpace[ship1].VMPerm.Eng.Systems[3].istat1*basep;
}
assigned=VMSpace[ship2].VMTemp.TmpEng.PowerComm;
basep=VMSpace[ship2].VMPerm.Eng.Systems[3].istat2;
b2=basep;
if(basep>0) { 
basep=assigned/basep;
range+=VMSpace[ship2].VMPerm.Eng.Systems[3].istat1*basep;
}

range=range/10.0;
r=VMdistance(VMSpace[ship1].VMPerm.Nav.XYZCoords, VMSpace[ship2].VMPerm.Nav.XYZCoords);
/*VMnotify(2,"r = %lf, range=%d ship1=%d ship2=%d basep1=%d basep2=%d",r,range,ship1,ship2,b1,b2);
*/
if(r <= range*70) return 1;
return 0;
}

int CanShipsTalk( ship1, ship2, frq)
int ship1, ship2;
float frq;
{
int dorel1,dorel2,rs1,rs2;
t_RelayEnt *tmp;
double r;
int range=0;
int assigned,basep;
dorel1=VMSpace[ship1].VMTemp.TmpCommu.Relay;
dorel2=VMSpace[ship2].VMTemp.TmpCommu.Relay;

 if(( CheckRange(ship1,ship2) && CheckShipCodes(ship1,ship2,frq))) return 1;

/*
if(dorel1==-1 || dorel2==-1) {
 if(( CheckRange(ship1,ship2) && CheckShipCodes(ship1,ship2,frq))) return 1;
 return 0;
	}
*/
if(dorel1 > -1 && dorel2 > -1) {
	tmp=VMSpace[ship1].VMTemp.TmpCommu.tRelay;
	rs1=tmp->VMShip;
	tmp=VMSpace[ship2].VMTemp.TmpCommu.tRelay;
	rs2=tmp->VMShip;
	if(CheckRange(ship1,rs1) ==0) {
		VMSpace[ship1].VMTemp.TmpCommu.Relay=-1;
		VMnotifymans(VMSpace[ship1].VMTemp.TmpCommu.VMMan_List,"Relay signal lost.");
		return 0;
	}

	if(CheckRange(ship2,rs2) ==0) {
		VMSpace[ship2].VMTemp.TmpCommu.Relay=-1;
		VMnotifymans(VMSpace[ship2].VMTemp.TmpCommu.VMMan_List,"Relay signal lost.");
		return 0;
	}

	if(CheckShipRelCode(ship1,frq)==0) return 0;
	if(CheckShipRelCode(ship2,frq)==0) return 0;
	if(CanRelaysTalk(dorel1,dorel2,frq)==0) return 0;
	return 1;
	}
return 0;
}

int CanRelaysTalk( rel1, rel2, frq)
int rel1,rel2;
float frq;
{
long int ifrq;
int ifr;

if(rel1==rel2) return 1;

ifr=(int)frq;
ifrq=pow(2,ifr);
if(!(VMShipsCanTalkMatrix[rel1][rel2] & ifrq))
	return 0;

return 1;

}

void relay_status(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int i,ship,dist,room;
int frqq,nom=0,eq;
long int frq;
char code1[50];

if(!IsIcReg(player)) return;
room=where_is(player);
notify(player," ----------------------------------------------------------");
notify(player," Relay Status");
notify(player," ----------------------------------------------------------\n");

for(i=1;i<30;i++) {
	sprintf(code1,"%s",read_slist(room,i,ATRIBS_A));
	eq=atoi(code1);
	if(code1[0]=='0') {
		notify(player,tprintf("[%2d - %2d.999999]    UNCODED",i,i));
		nom=1;
	}
	else if(eq!=-1) {
		notify(player,tprintf("[%2d - %2d.999999]    ENCODED ",i,i));
		nom=1;
	}
    }
if(nom==0) notify(player,"\nNo frequencies relayed.\n");

notify(player," ----------------------------------------------------------\n");

}

void OpenRelay(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int i,ship,dist,room,eq,eqq;
long int frq;
char *fargs[2],ptr[50];

i=xcode_parse(buffer,fargs,1);
room=where_is(player);
if(i!=1) { 
	notify(player,"Usage: OPEN <FREQUENCY>");
	return;
	}
if(atoi(fargs[0]) <= 0 || atoi(fargs[0]) > 30) {
	notify(player,"Invalid Frequency Range");
	return;
	}
	sprintf(ptr,"%s",vget_a(room,A_VA+1));
	eq=atoi(ptr);
	if(eq<=0) {
		notify(player,"You already are relaying your maximum number of frequency ranges.");
		return;
	}
	frq=atoi(fargs[0]);
	sprintf(ptr,"%s",read_slist(room,frq,ATRIBS_A));
	eqq=atoi(ptr);
	if(eqq!=-1) {
		notify(player,"That frequency range is already being relayed");
		return;
	}

	sprintf(ptr,"0");
	change_slist(room,frq,ptr,ATRIBS_A);
	eq--;
	sprintf(ptr,"%d",eq);
	atr_add_raw(room,A_VA+1,ptr);

	notify(player,"Freqeuncy range opened and relayed.\nThe relay network will update in a few moments.");

}

void CloseRelay(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int i,ship,dist,room,eq,eqq;
long int frq;
char *fargs[2],ptr[50];

i=xcode_parse(buffer,fargs,1);
room=where_is(player);
if(i!=1) { 
	notify(player,"Usage: CLOSE <FREQUENCY>");
	return;
	}
if(atoi(fargs[0]) <= 0 || atoi(fargs[0]) > 30) {
	notify(player,"Invalid Frequency Range");
	return;
	}
	sprintf(ptr,"%s",vget_a(room,A_VA+1));
	eq=atoi(ptr);
	frq=atoi(fargs[0]);
	sprintf(ptr,"%s",read_slist(room,frq,ATRIBS_A));
	eqq=atoi(ptr);
	if(eqq==-1) {
		notify(player,"That frequency range is already closed.");
		return;
	}

	sprintf(ptr,"-1");
	change_slist(room,frq,ptr,ATRIBS_A);
	eq++;
	sprintf(ptr,"%d",eq);
	atr_add_raw(room,A_VA+1,ptr);

	notify(player,"Freqeuncy range closed and no longer relayed.\nThe relay network will update in a few moments.");

}


void VMUnRLock(player,ship)
dbref player;
int ship;
{

VMSpace[ship].VMTemp.TmpCommu.Relay=-1;
VMnotify(player,"Dropping any relay lock.");

}

void VMRLock(player,ship,ShipNum,rel)
dbref player;
int ship,ShipNum,rel;
{
t_SpaceList *tmpshort;
t_RelayList *relays;
t_SpaceEnt *tmp;
t_RelayEnt *rtmp;
t_RelayEnt *retmp;
t_RelayEnt *gtmp;
int i,room,id=-1;

if(rel<1) {
	VMnotify(player,"Invalid Relay");
	return;
	}


tmpshort=VMSpace[ship].VMTemp.TmpNav.VMShortShips;
relays=VMSpace[ShipNum].VMTemp.TmpNav.VMRelayList;

ResetRelayList(relays);
for(i=0;i<rel-1;i++) { 
	if(AtEndRelayList(relays)) break;
	AdvanceRelayList(relays);
	}

rtmp=GetCurrRelayEnt(relays);

if(rtmp==NULL) {
	VMnotify(player,"Invalid Relay");
	return;
	}
/*room=rtmp->VMShip;*/
room=rtmp->VMRoom;


ResetRelayList(RelayList);
while(!AtEndRelayList(RelayList)) {

retmp=GetCurrRelayEnt(RelayList);
if(retmp==NULL) {
	VMnotify(player,"Invalid Relay");
	return;
	}
if(retmp->VMRoom == room) {
	gtmp=retmp;
	id=retmp->VMID;
	break;
	}
AdvanceRelayList(RelayList);
}
if(id==-1 || gtmp==NULL) {
	VMnotify(player,"Invalid Relay");
	return;
	}

  if(!IsShipInSpaceList(tmpshort,ShipNum) && ShipNum != ship) {
	VMnotify(player,"That ship is not on short range sensors.");
	return;
	}

VMSpace[ship].VMTemp.TmpCommu.Relay=id;
VMSpace[ship].VMTemp.TmpCommu.RShip=ShipNum;
VMSpace[ship].VMTemp.TmpCommu.tRelay=gtmp;
VMnotify(player,"Locked onto Relay");
}	


static char *rcsrelaysc="$Id: relays.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
