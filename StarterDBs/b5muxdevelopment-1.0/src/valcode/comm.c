#include "header.h"
#include "comm.h"
#include "./vmspace/vmspaceflags.h"
#include "./vmspace/proto.h"

extern int CanShipsTalk(int,int,float);
int VMCanTalkFrq(int shipfrom,int shipto, double freq,double poww);

#define VMCanTalk(x,y)	1
/*
#define VMCanTalkFrq(x,y,z)	1
*/

freqs Freqs[]={
{0x0000001},   /*1*/
{0x0000002},
{0x0000004},
{0x0000008},
{0x0000010},   /*5*/
{0x0000020},
{0x0000040},
{0x0000080},
{0x0000100},
{0x0000200},   /*10*/
{0x0000400},
{0x0000800},
{0x0001000},
{0x0002000},
{0x0004000},   /*15*/
{0x0008000},
{0x0010000},
{0x0020000},
{0x0040000},   
{0x0080000},   /*20*/
{0x0100000},
{0x0200000},
{0x0400000},
{0x0800000},   
{0x1000000},   /*25*/
{0x2000000},
{0x4000000},
{0x8000000},   
{0x10000000},  
{0x20000000},   /*30*/
{0x40000000},   /*31*/
{0x80000000}   /*32*/
};


void do_cset(player,cause,key,cfreq)
dbref player,cause;
int key;
char *cfreq;
{
dbref it;
char *itm,*tmm;
double dfreq;
char frq[30];
int atrb=0,typ;

if(!IsIcReg(player)) return;
itm=vget_a(player,ITEM_A);
it=match_thing(0,itm);
tmm=vget_a(it,A_VA);
typ=atoi(tmm);
if(it==NOTHING || !(Flags3(it) & ITEM) || typ!=2)
	{
		notify(player,"You arent using a proper Communication ITEM");
		return;
	}
dfreq=atof(cfreq);
if(dfreq >=32 || dfreq < 0) 
	{
		notify(player,"Frequencies must be in the range 0 to 31.99");
		return;
	}

	sprintf(frq,"%lf",dfreq);
	atr_add_raw(it,A_VA+1,frq);

	notify(player,tprintf("Station set to %f",dfreq));


}


void do_turn(player,cause,key,message)
dbref player,cause;
int key;
char *message;
{
dbref it;
char *itm,*tmm;
int typ;
if(!IsIcReg(player)) return;

itm=vget_a(player,ITEM_A);
it=match_thing(0,itm);
tmm=vget_a(it,A_VA);
typ=atoi(tmm);
if(it==NOTHING || !(Flags3(it) & ITEM) || typ!=2)
	{
		notify(player,"You arent using a proper communication ITEM");
		return;
	}


if(strcmp(message,"on")==0) 
	{
		if(!IsPlayerInManList(Commus,it)) {
			AddManEnt(Commus,it,-1);
			notify(player,tprintf("You turn %s on",Name(it)));
			}
		else
			notify(player,"It is already on");

	}
if(strcmp(message,"off")==0) 
	{
		if(IsPlayerInManList(Commus,it)) {
			RemoveManEnt(Commus);
			notify(player,tprintf("You turn %s off",Name(it)));	
		}

		else
			notify(player,"It is already off");

	}

}





void do_ctran(player,cause,key,msg)
dbref player,cause;
int key;
char *msg;
{
dbref it;
char *itm,*tmm;
char *fq;
double dfreq;
int shipnum,typ;


if(!IsIcReg(player)) return;

itm=vget_a(player,ITEM_A);
it=match_thing(0,itm);
tmm=vget_a(it,A_VA);
typ=atoi(tmm);
if(it==NOTHING || !(Flags3(it) & ITEM) || typ!=2)
	{
		notify(player,"You arent using a proper Communication ITEM");
		return;
	}




fq=vget_a(it,A_VA+1);
dfreq=atof(fq);
shipnum=VMShipNum(player);
if(VMGlobalSpaceInit == 0) {
	notify(player,"Space is not initialized.");
	return;
	}
if(checkflag(VMSpace[shipnum].VMPerm.Flags,DEAD)) {
	notify(player,"This ship is dead, jim.");
	return;
	}
if(checkflag(VMSpace[shipnum].VMPerm.Flags,DOCKED)) {
	notify(player,"No transmitting in a docked ship for now");
	return;
	}
if(shipnum < 0 || shipnum > VMCurrentMax)  {
	notify(player,"Danger Danger. Bad shipnumber");
	return;
	}
VMTransmit(dfreq,msg,shipnum,100.0);
}


void VMTransmit(dfreq,msg,shipnum,poww)
double dfreq;
char *msg;
int shipnum;
double poww;

{
double poww2;
t_ManEnt *tmp;
dbref tmpp;
int shipp,k,l;
double ftmp;
int match;
char *ft;


ResetManList(Commus);
while(!AtEndManList(Commus))
	{
	tmp=GetCurrManEnt(Commus);			

	if((tmpp=tmp->VMPlayer)!=-1) {
		if(isPlayer(where_is(tmpp))) {
			shipp=VMShipNum(where_is(tmpp));
			if(shipp > 0 && shipp <= VMCurrentMax)
			{
				l=0;
				ft=vget_a(tmpp,A_VA+1);
				ftmp=atof(ft);
				if(VMCanTalkFrq(shipnum,shipp,ftmp,poww)) {
					if(ftmp==dfreq) { 
						notify_all_from_inside(where_is(tmpp),1,tprintf("%s's %s emits, '%s'",Name(where_is(tmpp)),Name(tmpp),msg));
					}	
			   	}

			}
		}

	}
	else {
		shipp=tmp->VMRoom;
		match=-1;
		for(k=0;k<10;k++) if(VMSpace[shipp].VMTemp.TmpCommu.freqs[k]==dfreq) match=k;

		poww=VMSpace[shipnum].VMTemp.TmpEng.PowerComm;
		poww=poww/(double)VMSpace[shipnum].VMPerm.Eng.Systems[3].istat2;
		poww=poww*VMSpace[shipnum].VMPerm.Eng.Systems[3].istat1;
		poww=poww/10.0;

		poww2=VMSpace[shipp].VMTemp.TmpEng.PowerComm;
		poww2=poww2/(double)VMSpace[shipp].VMPerm.Eng.Systems[3].istat2;
		poww2=poww2*VMSpace[shipp].VMPerm.Eng.Systems[3].istat1;
		poww2=poww2/10.0;

		poww=poww+poww2;

		if(VMCanTalkFrq(shipnum,shipp,dfreq,poww)) {
			if(match!=-1) { 
			/*	notify_all_from_inside(where_is(tmpp),1,tprintf("%s's %s emits, '%s'",Name(where_is(tmpp)),Name(tmpp),msg));*/
				VMnotifymans(VMSpace[shipp].VMTemp.TmpCommu.VMMan_List,tprintf("[%d|%5.3f] %s",match,dfreq,msg));
			}	
	   	}

	}
	AdvanceManList(Commus);		
	}
}


int VMCanTalkFrq(int shipfrom,int shipto, double freq,double poww)
{
double dist;

dist=VMdistance( VMSpace[shipfrom].VMPerm.Nav.XYZCoords, VMSpace[shipto].VMPerm.Nav.XYZCoords);
/*notify(2,tprintf("s1=%d s2=%d d=%ld",shipfrom,shipto,dist));*/
if(CanShipsTalk(shipfrom,shipto,(float)freq)==1) return(1);

/* used to have a 10 multipler here. dunno why */
if(dist <= poww) {
	return(1);
   }
	return(0);
}

void do_cpage(player,cause,key,cmd)
dbref player,cause;
int key;
char *cmd;
{
dbref it,pl;
int ship1,ship2;
int modee;
char *plyr;
char *fq;
char *ft;
char *lk;
char *hid;
char *itm;
int hide;
dbref lin;
double dfreq;
int shipnum;
char fr[20];
int typ;
char *tmm;
	
if(!IsIcReg(player)) return;

itm=vget_a(player,ITEM_A);
it=match_thing(0,itm);
tmm=vget_a(it,A_VA);
typ=atoi(tmm);
if(it==NOTHING || !(Flags3(it) & ITEM) || typ!=2)
	{
		notify(player,"You arent using a proper Communication ITEM");
		return;
	}


if(strncmp(cmd,"ans",3)==0) {
	plyr=vget_a(it,A_VA+2);
	pl=match_thing(0,plyr);

	if(pl==NOTHING || pl==0) 
	{
		notify(player,"Noone is paging you.");
		return;
	}

	ft=vget_a(it,A_VA+1);
	atr_add_raw(it,A_VA+3,ft);
	ft=vget_a(it,A_VA+2);
	hide=match_thing(0,ft);	
	hide+=100;
	sprintf(fr,"%d",hide);
	atr_add_raw(it,A_VA+1,fr);

	sprintf(fr,"1");
	atr_add_raw(it,A_VA+4,fr);
		
	notify_all_from_inside(where_is(it),1,tprintf("%s taps the beeping %s.",Name(where_is(it)),Name(it)));
	}
else if(strncmp(cmd,"off",3)==0) {	
	plyr=vget_a(it,A_VA+4);
	modee=atoi(plyr);

	if(modee!=1) 
	{
		notify(player,"You arent in page mode.");
		return;
	}

	ft=vget_a(it,A_VA+3);
	atr_add_raw(it,A_VA+1,ft);

	sprintf(fr,"-1");
	atr_add_raw(it,A_VA+2,fr);
	sprintf(fr,"0");
	atr_add_raw(it,A_VA+4,fr);
	notify_all_from_inside(where_is(it),1,tprintf("%s taps the %s.",Name(where_is(it)),Name(it)));
	}

else {
	plyr=vget_a(it,A_VA+4);
	modee=atoi(plyr);

	if(modee==1) 
	{
		notify(player,"You are already in page mode.");
		return;
	}

	pl=match_thing(0,cmd);
	if(pl==NOTHING ) 
	{
		notify(player,"Player not found.");
		return;
	}

	lk=vget_a(pl,COMPUTER_A);
	lin=match_thing(0,lk);


	if(lin==NOTHING || !(Flags3(lin) & ITEM))
	{
		notify(player,"That person has no registered link.");
		return;
	}	

	hid=vget_a(lin,A_VA+5);
	hide=atoi(hid);
	if(hide==1)
	{
		notify(player,"That person cannot be found.");
		return;
	}

	ship1=VMShipNum(player);
	ship2=VMShipNum(pl);

	if(VMCanTalk(ship1,ship2)) {
		notify_all_from_inside(where_is(where_is(lin)),1,tprintf("%s's %s beeps with a page.",Name(where_is(lin)),Name(lin)));
		sprintf(fr,"#%d",player);
		atr_add_raw(lin,A_VA+2,fr);
		hid=vget_a(lin,A_VA+1);
		atr_add_raw(lin,A_VA+3,hid);	

		dfreq=player+100;
		sprintf(fr,"%lf",dfreq);
		atr_add_raw(it,A_VA+1,fr);	
		sprintf(fr,"1");
		atr_add_raw(it,A_VA+4,fr);	
		notify_all_from_inside(where_is(it),1,tprintf("%s taps the %s to send a page.",Name(where_is(it)),Name(it)));
	
	}
  }

}

static char *rcscommc="$Id: comm.c,v 1.1 2001/01/15 03:23:15 wenk Exp $";
