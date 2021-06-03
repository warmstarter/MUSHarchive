/*
 * valfun.c - 
 */

#include "header.h"
#include "econ.h"
#include "vmspace/vmspace.h"
#include "vmspace/vmmissle.h"
#include "flags.h"
#include "db.h"
#include "udb.h"


extern t_missle AllMissles[];

int Check_UnpackList();

PackList PList[]={
{3858,1705},
{3857,1703},
{3856,1701},
{305,1699},
{306,1697},
{386,1695},
{3093,1683},
{1018,1694},
{1285,1692},
{3036,1691},
{3052,1689},
{3068,1687},
{3069,1685},
{3092,1682},
{3147,1680},
{3155,1678},
{3158,1676},
{3290,1674},
{3101,34},
{3141,1467},
{4835,1466},
{3139,1465},
{3138,1464},
{3137,1463},
{3136,54},
{3134,53},
{3133,52},
{3132,51},
{3131,50},
{3128,49},
{3127,48},
{3126,1461},
{3125,45},
{3124,1672},
{3123,44},
{3115,43},
{3114,42},
{3113,47},
{3112,46},
{3111,35},
{3107,39},
{3109,38},
{3108,37},
{3106,36},
{3103,41},
{3102,40},
{3100,33},
{3099,32},
{5088,1729},
{5112,1731},
{5077,1733},
{5072,1734},
{5079,1735},
{5097,1737},
{5098,1739},
{5087,1741},
{5101,1743},
{5103,1745},
{5104,1747},
{4834,55},
{4837,56},
{4838,57},
{4839,1717},
{4840,1718},
{4841,58},
{4842,1459},
{4843,1719},
{4844,1720},
{4845,1721},
{4846,1722},
{4847,1723},
{4848,1724},
{4849,1725},
{5080,1749},
{5086,1750},
{5081,1751},
{5082,1752},
{5083,1753},
{5084,1754},
{5085,1755},
{4885,61},
{4884,62},
{4883,63},
{4882,64},
{4881,65},
{4880,66},
{4879,67},
{4878,68},
{4877,69},
{4876,70},
{4875,71},
{4874,72},
{4873,73},
{4872,74},
{4871,75},
{4870,76},
{4869,77},
{4868,78},
{4867,1460},
{4866,1462},
{4865,1673},
{4864,1675},
{4863,1677},
{4862,1679},
{4861,1681},
{4860,1684},
{4859,1686},
{4858,1688},
{4857,1690},
{4856,1693},
{4855,1696},
{4854,1698},
{4853,1700},
{4852,1702},
{4851,1704},
{4850,1706},
{5106,1730},
{5113,1732},
{5108,1736},
{5114,1738},
{5115,1740},
{5116,1742},
{5117,1744},
{5118,1746},
{5119,1748},
{4831,30},
/* {4830,31}, */
{4915,1726},
{4914,1468},
{4916,1727},
{10049,1882},
{10833,1876},
{1487,1872},
{-1,-1},
{-1,-1},
{-1,-1},
{-1,-1},
{-1,-1}
};


void do_vmove(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref console,vehicle,playe;
char *fargs[5];
int loc1,loc2,i,commodity,amount;

if(!IsIcReg(player)) return;
i=xcode_parse(buffer,fargs,1);
if(i!=1) {
	notify(player,"Usage: VMOVE <DIRECTION/EXIT>");
	return;
	}
if(!string_compare(fargs[0],"home")) {
	notify(player,"Invalid Direction");	
	return;
	}
loc1=where_is(where_is(player));
do_move(where_is(player),where_is(player),0,fargs[0]);
loc2=where_is(where_is(player));
if(loc2!=loc1) {
	notify(player,"The engines engage as the vehicle moves.");
}
else {
	notify(player,"The vehicle remains still unable to go in that direction.");
}
}

void econunload(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref console,vehicle,playe;
char *fargs[5];
int i,commodity,amount;
/*return;*/

if(!IsIcReg(player)) return;
i=xcode_parse(buffer,fargs,3);
if(i < 2) {
	notify(player,"Useage: EUNLOAD <COMMODITY #> <AMOUNT>");
	return;
	}
if(i!=3 && IsVehicle(player)) {
	notify(player,"Useage: EUNLOAD <COMMODITY #> <AMOUNT>");
	return;
	}

 if(i!=2 && !IsVehicle(player)) {
	notify(player,"Useage: EUNLOAD <COMMODITY #> <AMOUNT>");
	return;
	}
console=where_is(player);
vehicle=player;
if(i==3) playe=match_thing(player,fargs[0]);
else playe=player;
if(console==NOTHING || playe==NOTHING || vehicle==NOTHING) {return;}
if(i==3){
commodity=atoi(fargs[1]);
amount=atoi(fargs[2]);
 }
else {
commodity=atoi(fargs[0]);
amount=atoi(fargs[1]);
 }
if(commodity < 1 || commodity > COMODS) {
	notify(player,"Invalid Commodity");
	return;
	}
i=econloadparent(playe,player,console,commodity,amount);
}


void do_vaddmis(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref console,vehicle,playe;
char *fargs[5];
int typ,lc,i,commodity,amount,loader,receiver;
char cstr[30],cstr2[30];
int tmp,tmp2,mis,ship;
float carg;

if(!IsIcReg(player)) return;

typ=atoi(vfetch_attribute(where_is(where_is(player)),"MIS"));
lc=where_is(where_is(player));
ship=VMShipFromRoom(lc);
if(typ!=1 || ship <= 1 || ship > VMCurrentMax) {
	notify(player,"The vehicle must be in a valid ship Missile Room to do that.");
	return;
}

i=xcode_parse(buffer,fargs,3);
if(i!=2) {
	notify(player,"Usage: LOADMIS <MISSILE CMD #> <AMOUNT>");
	return;
	} 
console=where_is(where_is(player));
commodity=atoi(fargs[0]);
amount=atoi(fargs[1]);

loader=where_is(player);
receiver=console;
tmp=read_ilist(loader,commodity,COMMOD_A);
if(tmp < amount) {
 notify(playe,tprintf("There is not that much %s",EconItems[commodity].name));
 return;
  }

else if(amount < 1) {
 notify(playe,tprintf("Invalid Amount"));
 return;
  }
mis=-1;
for(i=1;i<NUMMISS;i++) {
	if(AllMissles[i].plan==commodity)  {
		mis=i;
		break;
		}
	}
  if(mis==-1) {
	notify(player,"That Commodity Is Not A Missile");
	return;
	}

  tmp2=tmp-amount;
  change_ilist(loader,commodity,tmp2,COMMOD_A);
  for(i=0;i<tmp2;i++) VMAddMissle(1,ship,mis);
  notify(playe,"Missile(s) Added");
}



void do_veconunload(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref console,vehicle,playe;
char *fargs[5];
char *typ;
int i,commodity,amount;

if(!IsIcReg(player)) return;

typ=vfetch_attribute(where_is(where_is(player)),"XTYPE");
if(strcmp(typ,"CARGOROOM")!=0 && strcmp(typ,"FACTORY")!=0 && strcmp(typ,"SHIPDOCK")!=0) {
	notify(player,"The vehicle must be in a cargo room or factory to do that.");
	return;
}

i=xcode_parse(buffer,fargs,2);
if(i != 2) {
	notify(player,"Useage: ELOAD <COMMODITY #> <AMOUNT>");
	return;
	}
console=where_is(where_is(player));
commodity=atoi(fargs[0]);
amount=atoi(fargs[1]);

i=econloadparent(player,where_is(player),console,commodity,amount);
}

void do_veconload(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref console,vehicle,playe;
char *fargs[5];
char *typ;
int i,commodity,amount;

if(!IsIcReg(player)) return;
typ=vfetch_attribute(where_is(where_is(player)),"XTYPE");
if(strcmp(typ,"CARGOROOM")!=0 && strcmp(typ,"FACTORY")!=0 && strcmp(typ,"SHIPDOCK")!=0) {
	notify(player,"The vehicle must be in a cargo room or factory to do that.");
	return;
}

i=xcode_parse(buffer,fargs,2);
if(i != 2) {
	notify(player,"Useage: ELOAD <COMMODITY #> <AMOUNT>");
	return;
	}

console=where_is(where_is(player));
commodity=atoi(fargs[0]);
amount=atoi(fargs[1]);

i=econloadparent(player,console,where_is(player),commodity,amount);

}


void econload(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref console,vehicle,playe;
char *fargs[5];
int i,commodity,amount;


if(!IsIcReg(player)) return;

i=xcode_parse(buffer,fargs,3);
/*VAL*/


if(i < 2) {
	notify(player,"Useage: ELOAD <COMMODITY #> <AMOUNT>");
	return;
	}
if(i!=3 && IsVehicle(player)) {
	notify(player,"Useage: ELOAD <COMMODITY #> <AMOUNT>");
	return;
	}

 if(i!=2 && !IsVehicle(player)) {
	notify(player,"Useage: ELOAD <COMMODITY #> <AMOUNT>");
	return;
	}

console=where_is(player);
vehicle=player;
if(i==3) playe=match_thing(player,fargs[0]);
else playe=player;
if(console==NOTHING || playe==NOTHING || vehicle==NOTHING) {return;}
if(i==3){
commodity=atoi(fargs[1]);
amount=atoi(fargs[2]);
 }
else {
commodity=atoi(fargs[0]);
amount=atoi(fargs[1]);
 }

i=econloadparent(playe,console,player,commodity,amount);
}




int econloadparent(playe,loader,receiver,commodity,amount)
dbref loader,receiver,playe;
int commodity,amount;
{
char cstr[30],cstr2[30];
int tmp,tmp2;
float carg;
if(commodity <=0 ||  commodity > COMODS) {
	notify(playe,"Invalid Commodity");
	return;
	}
tmp=read_ilist(loader,commodity,COMMOD_A);
if(tmp < amount) {
 notify(playe,tprintf("There is not that much %s",EconItems[commodity].name));
 return -1;
  }
else if(amount < 1) {
 notify(playe,tprintf("Invalid Amount"));
 return -1;
  }

else if(((float)amount*EconItems[commodity].size) > atof(vget_a(receiver,CARGO))) {
   notify(playe,"Not enough room to store the cargo");
   return -1;
  }
else {
  carg=atof(vget_a(receiver,CARGO))-amount*EconItems[commodity].size;
  sprintf(cstr,"%f",carg);
  atr_add_raw(receiver,CARGO,cstr); 
  carg=atof(vget_a(loader,CARGO))+amount*EconItems[commodity].size;
  sprintf(cstr2,"%f",carg);
  atr_add_raw(loader,CARGO,cstr2); 
  notify(playe,"Cargo transferred");
  tmp2=tmp-amount;
  change_ilist(loader,commodity,tmp2,COMMOD_A);
  change_ilist(receiver,commodity,read_ilist(receiver,commodity,COMMOD_A)+amount,COMMOD_A);
  return 1;
 }
}


void econcommod(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref object,viewer;
char *fargs[2];
int i;

if(!IsIcReg(player)) return;

i=xcode_parse(buffer,fargs,1);
if(i==1 && !IsVehicle(player)) return;
object=where_is(player);
if(i==1) viewer=match_thing(player,fargs[0]);
else viewer=player;

if(object!=NOTHING && viewer!=NOTHING) {
 notify(viewer,"----------------------------------------------------------------");
 econ_commod(object,viewer);
 notify(viewer,"----------------------------------------------------------------");
 notify(viewer,tprintf("Cargo Space Free: %.2f / %.2f",atof(vget_a(object,CARGO)),atof(vget_a(object,MCARGO))));
 notify(viewer,"----------------------------------------------------------------");
  }
}

void econcommod_price(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref object,viewer;
char *fargs[3];
char bff[30];
char *curr;
int i,comd,mon;
char *vall;
float cost,vald;

i=xcode_parse(buffer,fargs,2);
if(i!=2) {
	notify(player,"Usage: PRICE <COMMODITY #> <CURRENCY>");
	return;
	}

if(!IsIcReg(player)) return;
curr=fargs[1];
mon=-1;
for(i=1;i<=MONEYS;i++) {
	if(string_compare2(curr,Moneys[i].name)==0) {mon=i;}
}

if(mon==-1) {notify(player,tprintf("I dont know the Currency %s",curr)); return;}
comd=atoi(fargs[0]);
if(comd > COMODS || comd < 1) {
	notify(player,"Invalid commodity");
	return;
	}
sprintf(bff,"pmod%d",comd);
vall=vfetch_attribute(where_is(player),bff);
vald=atof(vall);
if(vald==0) vald=1;

 cost=(float)EconItems[comd].baseprice*Moneys[mon].worth*vald*PMOD;
 notify(player,"------------------------------------------------------------------------------");
 notify(player,tprintf(" %d) %s    %f   %s",comd,EconItems[comd].name,cost,Moneys[mon].name));
/*notify(player,tprintf("Baseprice %f worth %f vald %f",EconItems[comd].baseprice*PMOD,Moneys[mon].worth,vald));*/
 notify(player,"------------------------------------------------------------------------------");
}


void econcommod_pr(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref object,viewer;
char *fargs[3];
char *curr;
int i;
char *vall;
float tax;

if(!IsIcReg(player)) return;

i=xcode_parse(buffer,fargs,2);
if(i<1 || i==2 && !IsVehicle(player)) {notify(player,"Useage: LCARGO <CURRENCY>"); return;}
object=where_is(player);
if(i==2) {viewer=match_thing(player,fargs[0]);
curr=fargs[1];
 }
else {viewer=player;
curr=fargs[0];
}

if(object!=NOTHING && viewer!=NOTHING) {
vall=vfetch_attribute(where_is(player),"tax");
tax=atof(vall);
 notify(viewer,"------------------------------------------------------------------------------");
 econ_commod_price(object,viewer,curr);
 notify(viewer,"------------------------------------------------------------------------------");
 notify(viewer,tprintf("Tax: %.4f   Cargo Space Free: %.2f / %.2f",tax,atof(vget_a(object,CARGO)),atof(vget_a(object,MCARGO))));
 notify(viewer,"------------------------------------------------------------------------------");
  }
}


void ebuy(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *vall;
float vald;
dbref console,vehicle,playe;
char *curr;
char *fargs[5];
int i,commodity,amount,mon;


if(!IsIcReg(player)) return;

i=xcode_parse(buffer,fargs,3);
/*VAL*/
/*
if(!(i==4 && IsVehicle(player)) &&  !(i==3 && !IsVehicle(player)) ) {
	return;
	}
*/
if(i!=3) {
	notify(player,"Usage: EBUY <COMMODITY #> <AMOUNT> <CURRENCY NAME>");
	return;
	}
console=where_is(player);
vehicle=player;
if(i==4) playe=match_thing(player,fargs[0]);
else playe=player;
if(console==NOTHING || playe==NOTHING || vehicle==NOTHING) {return;}
if(i==4){
	commodity=atoi(fargs[1]);
	amount=atoi(fargs[2]);
 }
else {
	commodity=atoi(fargs[0]);
	amount=atoi(fargs[1]);
 }
if(commodity > COMODS || commodity < 1) {
	notify(player,"Invalid commodity");
	return;
	}
if(i==4) curr=fargs[3];
else	curr=fargs[2];
mon=-1;
for(i=1;i<=MONEYS;i++) {
	if(string_compare2(curr,Moneys[i].name)==0) {mon=i;}
}
if(mon==-1) {notify(playe,tprintf("I dont know the Currency %s",curr)); return;}

vall=vfetch_attribute(where_is(player),"tax");
vald=atof(vall);

i=etradeparent(playe,console,player,commodity,amount,mon,console,vald);
}

void do_vebuy(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *vall;
float vald;
dbref console,vehicle,playe;
char *curr;
char *fargs[5];
int i,commodity,amount,mon;
char *typ;

if(!IsIcReg(player)) return;
typ=vfetch_attribute(where_is(where_is(player)),"XTYPE");
if(strcmp(typ,"TRADEROOM")!=0 && strcmp(typ,"STOREROOM")!=0) {
	notify(player,"The vehicle must be in a trade room or store to do that.");
	return;
}

i=xcode_parse(buffer,fargs,3);
/*VAL*/
if(i!=3) {
	notify(player,"Usage: EBUY <COMMODITY #> <AMOUNT> <CURRENCY NAME>");
	return;
	}
vehicle=where_is(player);
console=where_is(vehicle);
if(console==NOTHING || playe==NOTHING || vehicle==NOTHING) {return;}
	commodity=atoi(fargs[0]);
if(commodity > COMODS|| commodity < 1) {
	notify(player,"Invalid commodity");
	return;
	}
	amount=atoi(fargs[1]);
	curr=fargs[2];
mon=-1;
for(i=1;i<=MONEYS;i++) {
	if(string_compare2(curr,Moneys[i].name)==0) {mon=i;}
}
if(mon==-1) {notify(player,tprintf("I dont know the Currency %s",curr)); return;}

vall=vfetch_attribute(where_is(vehicle),"tax");
vald=atof(vall);
i=etradeparent(player,console,vehicle,commodity,amount,mon,console,vald);
}

void vcoms(player,data,buffer)
dbref player;
void *data;
char *buffer;
{

notify(player,"Commodities for this Vehicle\n--------------------------------");

econ_commod(where_is(player),player);

 notify(player,"----------------------------------------------------------------");
 notify(player,tprintf("Cargo Space Free: %.2f / %.2f",atof(vget_a(where_is(player),CARGO)),atof(vget_a(where_is(player),MCARGO))));
 notify(player,"----------------------------------------------------------------");

}

void vmons(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
notify(player,"Money Supply for this Vehicle\n--------------------------------");
see_money(where_is(player),player);
notify(player,"--------------------------------");

}


void vcommods(player,data,buffer)
dbref player;
void *data;
char *buffer;
{

notify(player,"Items this vehicle is carrying\n----------------------");
econ_commod(where_is(player),player);

 notify(player,"----------------------------------------------------------------");
 notify(player,tprintf("Cargo Space Free: %.2f / %.2f",atof(vget_a(where_is(player),CARGO)),atof(vget_a(where_is(player),MCARGO))));
 notify(player,"----------------------------------------------------------------");

}

void do_vesell(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *vall;
float vald;
dbref console,vehicle,playe;
char *curr;
char *fargs[5];
char *typ;
int i,commodity,amount,mon;


if(!IsIcReg(player)) return;
typ=vfetch_attribute(where_is(where_is(player)),"XTYPE");
if(strcmp(typ,"TRADEROOM")!=0) {
	notify(player,"The vehicle must be in a trade room to do that.");
	return;
}

i=xcode_parse(buffer,fargs,3);
/*VAL*/
if(i!=3) {
	notify(player,"Usage: ESELL <COMMODITY #> <AMOUNT> <CURRENCY NAME>");
	return;
	}
vehicle=where_is(player);
console=where_is(vehicle);
if(console==NOTHING || playe==NOTHING || vehicle==NOTHING) {return;}
	commodity=atoi(fargs[0]);
if(commodity > COMODS || commodity < 1) {
	notify(player,"Invalid commodity");
	return;
	}
	amount=atoi(fargs[1]);
	curr=fargs[2];
mon=-1;
for(i=1;i<=MONEYS;i++) {
	if(string_compare2(curr,Moneys[i].name)==0) {mon=i;}
}
if(mon==-1) {notify(player,tprintf("I dont know the Currency %s",curr)); return;}

vall=vfetch_attribute(where_is(vehicle),"tax");
vald=-1.0*atof(vall);
i=etradeparent(player,vehicle,console,commodity,amount,mon,console,vald);
}


void esell(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *vall;
float vald;
dbref console,vehicle,playe;
char *curr;
char *fargs[5];
int i,commodity,amount,mon;


if(!IsIcReg(player)) return;

i=xcode_parse(buffer,fargs,3);
/*VAL*/
/*
if(!(i==4 && IsVehicle(player)) &&  !(i==3 && !IsVehicle(player)) ) {
	return;
	}
*/
if(i!=3) {
	notify(player,"Usage: ESELL <COMMODITY #> <AMOUNT> <CURRENCY NAME>");
	return;
	}

console=where_is(player);
vehicle=player;
if(i==4) playe=match_thing(player,fargs[0]);
else playe=player;
if(console==NOTHING || playe==NOTHING || vehicle==NOTHING) {return;}
if(i==4){
	commodity=atoi(fargs[1]);
	amount=atoi(fargs[2]);
 }
else {
	commodity=atoi(fargs[0]);
	amount=atoi(fargs[1]);
 }
if(commodity > COMODS|| commodity < 1) {
	notify(player,"Invalid commodity");
	return;
	}
if(i==4) curr=fargs[3];
else	curr=fargs[2];
mon=-1;
for(i=1;i<=MONEYS;i++) {
	if(string_compare2(curr,Moneys[i].name)==0) {mon=i;}
}
if(mon==-1) {notify(playe,tprintf("I dont know the Currency %s",curr)); return;}

vall=vfetch_attribute(where_is(player),"tax");
vald=-1.0*atof(vall);

i=etradeparent(playe,playe,console,commodity,amount,mon,console,vald);
}




int etradeparent(playe,loader,receiver,commodity,amount,mon,rom,tax)
dbref loader,receiver,playe;
int commodity,amount,mon,rom;
float tax;
{
char bff[50],*vall;
char cstr[30],cstr2[30];
int tmp,tmp2;
float vald,cost;
float carg,cash;
if(commodity > COMODS || commodity < 1) {
	notify(playe,"Invalid commodity");
	return;
	}
tmp=read_ilist(loader,commodity,COMMOD_A);
if(tmp < amount) {
 notify(playe,tprintf("There is not that much %s",EconItems[commodity].name));
 return -1;
  }
else if(amount < 1) {
 notify(playe,tprintf("Invalid Amount"));
 return -1;
  }

else if(((float)amount*EconItems[commodity].size) > atof(vget_a(receiver,CARGO))) {
   notify(playe,"Not enough room to store the cargo");
   return -1;
  }

/*else if((float)amount * EconItems[commodity].baseprice *PMOD * Moneys[mon].worth * read_flist(rom,commodity,A_VA) > read_flist(receiver,mon,MONEY_A) )*/
else if((float)amount * EconItems[commodity].baseprice *PMOD * Moneys[mon].worth  > read_flist(receiver,mon,MONEY_A) )
	{
		notify(playe,tprintf("Not enough money available for transaction"));
		return -1;
	}

else {
sprintf(bff,"pmod%d",commodity);
vall=vfetch_attribute(rom,bff);
vald=atof(vall);
if(vald==0) {
  vald=1;
}
  cost=(float)amount*EconItems[commodity].baseprice*PMOD*Moneys[mon].worth*vald;
  cost=cost+cost*tax;

  cash=(float)read_flist(loader,mon,MONEY_A)+cost;
  change_flist(loader,mon,cash,MONEY_A);

  cash=(float)read_flist(receiver,mon,MONEY_A)-cost;
  change_flist(receiver,mon,cash,MONEY_A);

  carg=atof(vget_a(receiver,CARGO))-amount*EconItems[commodity].size;
  sprintf(cstr,"%f",carg);
  atr_add_raw(receiver,CARGO,cstr); 
  carg=atof(vget_a(loader,CARGO))+amount*EconItems[commodity].size;
  sprintf(cstr2,"%f",carg);
  atr_add_raw(loader,CARGO,cstr2); 
  notify(playe,"Cargo transferred");

  tmp2=tmp-amount;
  change_ilist(loader,commodity,tmp2,COMMOD_A);
  change_ilist(receiver,commodity,read_ilist(receiver,commodity,COMMOD_A)+amount,COMMOD_A);
  return 1;
 }
}

void do_unpack(player,cause,key,buffer)
dbref player,cause;
int key;
char *buffer;
{
char *fargs[3];
char nam[20],arg2[20],cstr[50];
int i,mo;
int ddb=0,clone;
float carg;
int cm,com;
if(!IsIcReg(player)) return;
i=xcode_parse(buffer,fargs,1);
if(i!=1) {
		notify(player,"Usage: UNPACK <COMMODITY#>");
		return;
	}
com=atoi(fargs[0]);
if( com < 1 || com > COMODS) {
	notify(player,"Invalid Commodity.");
	return;
	}
cm=read_ilist(player,com,COMMOD_A);
if(cm < 1) {
	notify(player,tprintf("You do not have enough of %s.",EconItems[com].name));
	return;
	}

mo=Check_UnpackList(com);
if(mo==0) {
	notify(player,tprintf("%s can not be unpacked.",EconItems[com].name));
	return;
	}
cm--;
change_ilist(player,com,cm,COMMOD_A);
carg=atof(vget_a(player,CARGO));
carg+=(float)EconItems[com].size;
sprintf(cstr,"%f",carg);
atr_add_raw(player,CARGO,cstr);
notify(player,tprintf("You unpack a %s.",EconItems[com].name));
/* do_clone(1,1,0,nam,arg2);*/
ddb=mo;
clone=create_obj(1,Typeof(ddb),Name(ddb),10);
/*clone=create_obj(1,0x1,Name(ddb),10);*/
/*move_via_generic(clone,where_is(player),NULL,0);*/
atr_free(clone);
/*atr_cpy(1,clone,ddb);*/
s_Parent(clone,ddb);
s_Name(clone,Name(ddb));
s_Pennies(clone,10);
s_Home(clone,clone_home(1,ddb));
s_Flags(clone,Flags(ddb));
s_Flags2(clone,Flags2(ddb));
s_Flags3(clone,Flags3(ddb));
move_via_generic(clone,player,NULL,0);
}


int Check_UnpackList(obj)
int obj;
{
int i=0,db=0;
while(PList[i].db!=-1) {
	if(PList[i].commod==obj) {
		db=PList[i].db;
		break;
		}
	i++;
	}
return(db);
}


void do_vepay(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *fargs[3];
int i,veh;
int mon;
dbref recc;
double amt;
double tmp;
if(!IsIcReg(player)) return;
/*
if((Flags3(where_is(player)) & REGISTERED)) {
	notify(player,"You cant do that here.");
	return;
	}
*/
i=xcode_parse(buffer,fargs,2);
if(i!=2) {
		notify(player,"Usage: PAYME <AMOUNT> <CURRENCY>");
		return;
	}

mon=-1;
for(i=1;i<=MONEYS;i++) {
	if(string_compare2(fargs[1],Moneys[i].name)==0) {mon=i;}
}
if(mon==-1) {notify(player,tprintf("I dont know the Currency %s",fargs[1])); return;}

recc=player;
veh=where_is(player);

amt=atof(fargs[0]);
tmp=read_flist(veh,mon,MONEY_A);
if( amt > tmp) {
	notify(player,tprintf("This vehicle does not have that many %s",fargs[1]));
	return;
	}
if( amt <= 0) {
	notify(player,"Invalid Amount");
	return;
	}
tmp-=amt;
if( tmp < 0) {
	notify(player,tprintf("This vehicle does not have that many %s",fargs[1]));
	return;
}

change_flist(veh,mon,tmp,MONEY_A);

tmp=read_flist(recc,mon,MONEY_A);
tmp+=amt;
change_flist(recc,mon,tmp,MONEY_A);

notify_all_from_inside(where_is(player),1,tprintf("%s gives %s %lf %s",Name(veh),Name(recc),amt,Moneys[mon].name));

}


void do_epay(player,cause,key,buffer)
dbref player,cause;
int key;
char *buffer;
{
char *fargs[3];
int i;
int mon;
dbref recc;
double amt;
double tmp;
if(!IsIcReg(player)) return;

if(Flags3(where_is(player)) & REGISTERED) {
	notify(player,"You cant do that here.");
	return;
	}
i=xcode_parse(buffer,fargs,3);
if(i!=3) {
		notify(player,"Usage EPAY <PLAYER> <AMOUNT> <CURRENCY>");
		return;
	}

mon=-1;
for(i=1;i<=MONEYS;i++) {
	if(string_compare2(fargs[2],Moneys[i].name)==0) {mon=i;}
}
if(mon==-1) {notify(player,tprintf("I dont know the Currency %s",fargs[2])); return;}

recc=match_thing(player,fargs[0]);
if(where_is(recc) != where_is(player)) {
	notify(player,tprintf("You don't see %s around",Name(recc)));
	return;
	}



amt=atof(fargs[1]);
tmp=read_flist(player,mon,MONEY_A);
if( amt > tmp) {
	notify(player,tprintf("You dont have that many %s",fargs[2]));
	return;
	}
if( amt <= 0) {
	notify(player,"Invalid Amount");
	return;
	}
tmp-=amt;
change_flist(player,mon,tmp,MONEY_A);

tmp=read_flist(recc,mon,MONEY_A);
tmp+=amt;
change_flist(recc,mon,tmp,MONEY_A);

notify_all_from_inside(where_is(player),1,tprintf("%s gives %s %lf %s",Name(player),Name(recc),amt,Moneys[mon].name));

}



void do_egive(player,cause,key,buffer)
dbref player,cause;
int key;
char *buffer;
{
char *fargs[3];
char cstr[30],cstr2[30];
int tmp,tmp2;
int i,mon;
int recc,commodity;
int amount;
float carg;
if(!IsIcReg(player)) return;
if((Flags3(where_is(player)) & REGISTERED)) {
	notify(player,"You cant do that here.");
	return;
	}
i=xcode_parse(buffer,fargs,3);
if(i!=3) {
		notify(player,"Usage EGIVE <PLAYER> <AMOUNT> <COMMODITY>");
		return;
	}
commodity=atoi(fargs[2]);
amount=atoi(fargs[1]);
recc=match_thing(player,fargs[0]);
if(where_is(recc) != where_is(player)) {
	notify(player,tprintf("You don't see %s around",Name(recc)));
	return;
	}
if(commodity < 1 || commodity > COMODS) {
	notify(player,"Invalid Commodity");
	return;
	}
tmp=read_ilist(player,commodity,COMMOD_A);
if(tmp < amount) {
 notify(player,tprintf("You dont have that much %s",EconItems[commodity].name));
 return;
  }
else if(amount < 1) {
 notify(player,tprintf("Invalid Amount"));
 return;
  }

else if(((float)amount*EconItems[commodity].size) > atof(vget_a(recc,CARGO))) {
   notify(player,tprintf("%s doesnt have room",Name(recc)));
   return;
  }
else {

  carg=atof(vget_a(recc,CARGO))-(float)amount*EconItems[commodity].size;
  sprintf(cstr,"%f",carg);
  atr_add_raw(recc,CARGO,cstr); 

  carg=atof(vget_a(player,CARGO))+(float)amount*EconItems[commodity].size;
  sprintf(cstr2,"%f",carg);
  atr_add_raw(player,CARGO,cstr2); 

notify_all_from_inside(where_is(player),1,tprintf("%s gives %s %d %s",Name(player),Name(recc),amount,EconItems[commodity].name));

  tmp2=tmp-amount;
  change_ilist(player,commodity,tmp2,COMMOD_A);
  change_ilist(recc,commodity,read_ilist(recc,commodity,COMMOD_A)+amount,COMMOD_A);
 }
}



static char *rcseconinterfacec="$Id: econinterface.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
