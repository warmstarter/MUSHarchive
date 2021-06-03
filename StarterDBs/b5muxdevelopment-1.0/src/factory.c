/*Factory code!*/

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
if(plan!=0) 
notify(player,tprintf("Currently Building Plan: %d , %s", plan,plans[plan].name));
notify(player,"----------------------------------------------------------------");
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
notify(player,tprintf("Plan Set to %d",plan));
sprintf(tmp,"%d",atoi(args[0]));
vput_attribute(key,"vb", tmp);
}

void factory_update(key,data)
dbref key;
void *data;
{
int plan,factor;
int amount,k;
char tmp[20];
float acargo=0,dcargo=0,carr;
/*notify(2,tprintf("Updating Factory #%d",key));*/

plan=atoi(vget_a(key,A_VA+1));


if(plan<=0 || plan>PLANS) return;
factor=get_fact_limit(key,plan);


if(factor==-1) return;
acargo=(float)factor*plans[plan].amtout*EconItems[plans[plan].commout].size;

for(k=0;k<plans[plan].nin;k++) {
  dcargo+=(float)factor*plans[plan].amin[k]*EconItems[plans[plan].commin[k]].size;
 }
if((carr=atof(vget_a(key,CARGO))) - acargo + dcargo < 0) {return;}

carr=carr+dcargo-acargo;
sprintf(tmp,"%f",carr);
atr_add_raw(key,CARGO,tmp);


for(k=0;k<plans[plan].nin;k++) {
 amount=read_ilist(key,plans[plan].commin[k],COMMOD_A);
 amount-=factor*plans[plan].amin[k]; 
 change_ilist(key,plans[plan].commin[k],amount,COMMOD_A);

/*notify(2,tprintf("Consuming %d of %s",factor*plans[plan].amin[k],EconItems[plans[plan].commin[k]].name));
*/
 }

 amount=read_ilist(key,plans[plan].commout,COMMOD_A);
 amount+=factor*plans[plan].amtout;
 change_ilist(key,plans[plan].commout,amount,COMMOD_A);

  

}



int get_fact_limit(int key,int plan)
{
int k,prodf,limit,olimit,tmplimit;
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



