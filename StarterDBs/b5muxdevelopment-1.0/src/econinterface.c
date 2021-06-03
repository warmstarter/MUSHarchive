/*
 * valfun.c - 
 */

void econunload(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref console,vehicle,playe;
char *fargs[5];
int i,commodity,amount;


if(!IsIcReg(player)) return;

i=xcode_parse(buffer,fargs,3);
if(!((i==3 && IsVehicle(player)) ||  !(i==2 && !IsVehicle(player)))) return;
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

i=econloadparent(console,playe,player,commodity,amount);
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
if(!(i==3 && IsVehicle(player)) &&  !(i==2 && !IsVehicle(player))) return;
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

void econcommod_pr(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref object,viewer;
char *fargs[3];
char *curr;
int i;

if(!IsIcReg(player)) return;

i=xcode_parse(buffer,fargs,2);
if(i<1 || i==2 && !IsVehicle(player)) {notify(player,"Error in arguments"); return;}
object=where_is(player);
if(i==2) {viewer=match_thing(player,fargs[0]);
curr=fargs[1];
 }
else {viewer=player;
curr=fargs[0];
}

if(object!=NOTHING && viewer!=NOTHING) {
 notify(viewer,"------------------------------------------------------------------------------");
 econ_commod_price(object,viewer,curr);
 notify(viewer,"------------------------------------------------------------------------------");
 notify(viewer,tprintf("Cargo Space Free: %.2f / %.2f",atof(vget_a(object,CARGO)),atof(vget_a(object,MCARGO))));
 notify(viewer,"------------------------------------------------------------------------------");
  }
}


void ebuy(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref console,vehicle,playe;
char *curr;
char *fargs[5];
int i,commodity,amount,mon;


if(!IsIcReg(player)) return;

i=xcode_parse(buffer,fargs,4);
/*VAL*/
if(!(i==4 && IsVehicle(player)) &&  !(i==3 && !IsVehicle(player)) ) {
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
if(i==4) curr=fargs[3];
else	curr=fargs[2];
mon=-1;
for(i=1;i<=MONEYS;i++) {
	if(string_compare2(curr,Moneys[i].name)==0) {mon=i;}
}
if(mon==-1) {notify(playe,tprintf("I dont know the Currency %s",curr)); return;}


i=etradeparent(playe,console,player,commodity,amount,mon,console);
}



int etradeparent(playe,loader,receiver,commodity,amount,mon,rom)
dbref loader,receiver,playe;
int commodity,amount,mon,rom;
{
char cstr[30],cstr2[30];
int tmp,tmp2;
float carg,cash;
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

else if((float)amount * EconItems[commodity].baseprice * Moneys[mon].worth * read_flist(rom,commodity,A_VA) > read_flist(receiver,mon,MONEY_A) )
	{
		notify(playe,tprintf("Not enough money available for transaction"));
		return -1;
	}

else {
  cash=(float)read_flist(loader,mon,MONEY_A)+(float)amount*EconItems[commodity].baseprice*Moneys[mon].worth*read_flist(rom,commodity,A_VA);
  change_flist(loader,mon,cash,MONEY_A);
  cash=(float)read_flist(receiver,mon,MONEY_A)-(float)amount*EconItems[commodity].baseprice*Moneys[mon].worth*read_flist(rom,commodity,A_VA);
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

i=xcode_parse(buffer,fargs,3);
if(i!=3) {
		notify(player,"Useage EPAY <PLAYER> <AMOUNT> <CURRENCY>");
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

i=xcode_parse(buffer,fargs,3);
if(i!=3) {
		notify(player,"Useage EGIVE <PLAYER> <AMOUNT> <COMMODITY>");
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



