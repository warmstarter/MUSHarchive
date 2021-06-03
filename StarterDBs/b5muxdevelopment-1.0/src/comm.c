#include "comm.h"



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


void do_cset(player,cause,key,cstation,cfreq)
dbref player,cause;
int key;
char *cstation;
char *cfreq;
{
dbref it;
char *itm;
double dfreq;
int stat;
char frq[30];
int atrb=0;

if(!IsIcReg(player)) return;
itm=vget_a(player,ITEM);
it=match_thing(0,itm);
if(it==NOTHING || !(Flags3(it) & ITEM))
	{
		notify(player,"You arent using a proper ITEM");
		return;
	}
dfreq=atof(cfreq);
stat=atoi(cstation);
if(dfreq >=32 || dfreq < 0) 
	{
		notify(player,"Frequencies must be in the range 0 to 31.99");
		return;
	}
if(stat > 4 || stat < 1) 
	{
		notify(player,"You can only set stations 1 to 4.");
		return;
	}

	sprintf(frq,"%lf",dfreq);
	atrb=A_VA+stat;
	atr_add_raw(it,atrb,frq);

	notify(player,tprintf("Station %d set to %f",stat,dfreq));


}


void do_turn(player,cause,key,message)
dbref player,cause;
int key;
char *message;
{
dbref it;
char *itm;

if(!IsIcReg(player)) return;

itm=vget_a(player,ITEM);
it=match_thing(0,itm);
if(it==NOTHING || !(Flags3(it) & ITEM))
	{
		notify(player,"You arent using a proper ITEM");
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





void do_ctran(player,cause,key,cstation,msg)
dbref player,cause;
int key;
char *cstation;
char *msg;
{
dbref it;
char *itm;
char *fq;
double dfreq;
int stat;
int shipnum;


if(!IsIcReg(player)) return;

itm=vget_a(player,ITEM);
it=match_thing(0,itm);
if(it==NOTHING || !(Flags3(it) & ITEM))
	{
		notify(player,"You arent using a proper ITEM");
		return;
	}


stat=atoi(cstation);

if(stat > 4 || stat < 1) 
	{
		notify(player,"You can only transmit to stations 1 to 4.");
		return;
	}


fq=vget_a(it,A_VA+stat);
dfreq=atof(fq);
shipnum=VMShipNum(player);
if(VMGlobalSpaceInit == 0) {
	notify(player,"Space is not initialized.");
	return;
	}
if(checkflag(VMSpace[shipnum].VMPerm.Flags,DEAD)) {
	notify(player,"This ship is dead");
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
t_ManEnt *tmp;
dbref tmpp;
int shipp,k,l;
double ftmp;
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
				for(k=1;k<5 && l==0;k++) {
					ft=vget_a(tmpp,A_VA+k);
					ftmp=atof(ft);
					if(ftmp==dfreq) {
					l=1;
					}
				}
				if(l==1) {
					notify(where_is(tmpp),tprintf("[%6.4f] %s",dfreq,msg));

				}

			}
		}

	}
	AdvanceManList(Commus);		
	}
}




