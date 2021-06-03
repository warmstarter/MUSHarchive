/*Factory code!*/

#include "header.h"
#include "econ.h"


void factory_status(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
dbref key;
int plan;
notify(player,"Factory Status");
notify(player,"----------------------------------------------------------------");
key=where_is(player);
econ_commod(key,player);
notify(player,"----------------------------------------------------------------");
notify(player,tprintf("Cargo Space Free: %.2f / %.2f",atof(vget_a(key,CARGO)),atof(vget_a(key,MCARGO))));
notify(player,"----------------------------------------------------------------");
notify(player,tprintf("Production Factor:  %d / %d      Efficiency: %.3f",atoi(vget_a(key,A_VA+3)),atoi(vget_a(key,A_VA+4)),atof(vget_a(key,A_VA+2))));

plan=atoi(vget_a(key,A_VA+1));
if(plan>0) 
notify(player,tprintf("Currently Building Plan: %d , %s", plan,plans[plan].name));
notify(player,"----------------------------------------------------------------");
}

void factory_plandef(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int i=0,k=1,plan;
char *args[2];
char tmp[20];
dbref key;
/*VMnotify(player,"This command is temporarily disabled.");
return;*/
key=where_is(player);
if(xcode_parse(buffer,args,1)!=1) {notify(player,"Invalid arguments"); return;}
plan=atoi(args[0]);
i=VMsmatch(vget_a(key,A_VA),args[0]);
if (i==0) {notify(player,"Invalid Plan"); return;}
if (i==0) {notify(player,"Invalid Plan"); return;}
if(plan < 0 || plan > PLANS) {
	notify(2,"Invalid Plans");
	return;
	}
notify(player,tprintf("Plan (%d) %s takes %d inputs consumes %d units of energy and %d units of outputs (%d) %s:",plan,plans[plan].name,plans[plan].nin,plans[plan].eng,plans[plan].amtout,plans[plan].commout,EconItems[plans[plan].commout].name));
notify(player,"--------------------------------------------------");
for(i=0;i<plans[plan].nin;i++) {
	if(plans[plan].commin[i] < COMODS)
	notify(player,tprintf("  This plan requires %d units of (%d) %s",plans[plan].amin[i],plans[plan].commin[i],EconItems[plans[plan].commin[i]].name));
	}
notify(player,"--------------------------------------------------");

}
void factory_build(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int i=0,k=1,plan;
char *args[2];
char tmp[20];
dbref key;
key=where_is(player);
if(xcode_parse(buffer,args,1)!=1) {notify(player,"Invalid arguments"); return;}
/*
while(i==0 && (plan=read_ilist(key,k,A_VA))!=-1) {
 k++;
 if(plan==atoi(args[0])) {i=1;}
 }
*/
plan=atoi(args[0]);
i=VMsmatch(vget_a(key,A_VA),args[0]);
if (i==0) {notify(player,"Invalid Plan"); return;}
if(plan < 0) 
{notify(player,"Invalid Plan"); return;}
notify(player,tprintf("Plan Set to %d",plan));
sprintf(tmp,"%d",atoi(args[0]));
vput_attribute(key,"vb", tmp);
}



void factory_update(key,data)
dbref key;
void *data;
{
int plan,factor;
int succ,amount,k;
char tmp[20];
float acargo=0,dcargo=0,carr,carr2;
/* notify(2,tprintf("Updating Factory #%d",key)); */

plan=atoi(vget_a(key,A_VA+1));


if(plan<=0 || plan>PLANS) return;
factor=get_fact_limit(key,plan);

/*VMnotifyAll(key,"Attempting to build. Maximum factory production factor=%d",factor);*/

if(factor==-1) return;
acargo=(float)factor*plans[plan].amtout*EconItems[plans[plan].commout].size;

dcargo=0;
for(k=0;k<plans[plan].nin;k++) {
  dcargo+=(float)factor*plans[plan].amin[k]*EconItems[plans[plan].commin[k]].size;
 }
if((carr=atof(vget_a(key,CARGO))) - acargo + dcargo < 0) {return;}

carr2=carr;
carr=carr+dcargo-acargo;
sprintf(tmp,"%f",carr);
succ=1;
if(random()%17+2 <= plans[plan].diff)   {
succ=0;
}
else atr_add_raw(key,CARGO,tmp);


for(k=0;k<plans[plan].nin;k++) {
 amount=read_ilist(key,plans[plan].commin[k],COMMOD_A);
 amount-=factor*plans[plan].amin[k]; 
 change_ilist(key,plans[plan].commin[k],amount,COMMOD_A);

 /* notify(2,tprintf("Consuming %d of %s",factor*plans[plan].amin[k],EconItems[plans[plan].commin[k]].name));
  */
 }

if(succ==0) {
	carr2=carr2+dcargo;
	sprintf(tmp,"%f",carr2);
 	atr_add_raw(key,CARGO,tmp);
	return;
}
 atr_add_raw(key,CARGO,tmp);
 amount=read_ilist(key,plans[plan].commout,COMMOD_A);
 amount+=factor*plans[plan].amtout;
 change_ilist(key,plans[plan].commout,amount,COMMOD_A);

/* should it be 59  cell a*/
 amount=read_ilist(key,59,COMMOD_A);
 amount-=factor*plans[plan].eng;
 change_ilist(key,59,amount,COMMOD_A);
  
acargo=(float)factor*EconItems[59].size*plans[plan].eng;
carr=atof(vget_a(key,CARGO));
carr=carr+acargo;
sprintf(tmp,"%f",carr);
atr_add_raw(key,CARGO,tmp);

}



int get_fact_limit(int key,int plan)
{
int k,prodf,limit,olimit,tmplimit,eng,eavail;
float eff;
prodf=atoi(vget_a(key,A_VA+3));
eff=atof(vget_a(key,A_VA+2));
olimit=prodf*eff;
limit=olimit;
for(k=0;k<plans[plan].nin;k++) {
 tmplimit=read_ilist(key,plans[plan].commin[k],COMMOD_A)/plans[plan].amin[k]; 
 if(tmplimit<1) return -1;
 if(tmplimit < limit) limit=tmplimit;
}
eng=limit*plans[plan].eng;
if(eng!=0) {
	if((eavail=read_ilist(key,59,COMMOD_A)) < eng) 
		{
			limit=eavail/plans[plan].eng;
		}
	}

return(limit);
}

void factory_pset(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int i=0,k=1,plan;
int mprodf,prodf;
char tmp[20];
dbref key;
char *args[2];
key=where_is(player);
if(xcode_parse(buffer,args,1)!=1) {notify(player,"Invalid arguments"); return;}
prodf=atoi(args[0]);
mprodf=atoi(vget_a(key,A_VA+4));
if(mprodf < prodf || prodf < 0) {notify(player,"Invalid Production Factor Setting!"); return;}
sprintf(tmp,"%d",atoi(args[0]));
vput_attribute(key,"vd", tmp);
}



static char *rcsfactoryc="$Id: factory.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
