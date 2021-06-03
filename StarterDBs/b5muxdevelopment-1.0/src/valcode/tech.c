
#include "header.h"
#include "tech.h"
#include "econ.h"





TTechs Techs[]={
{(char *)NULL,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{(char *)"Math",1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
{(char *)"Logic",2,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
{(char *)"Chemistry",3,2,1,2,0,0,0,1,1,1,0,0,0,0,1,0,0,0,0},
{(char *)NULL,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};







void seetech(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
int i,tmp;


if(!IsIcReg(player)) return;

notify(player,"This Research Lab Currently has the Following Information:");
notify(player,"----------------------------------------------------------");
for(i=1;i<=TECHS;i++) {
 tmp=read_ilist(where_is(player),i,SKILL_A);
 if(tmp>0) notify(player,tprintf("%3d %20s %2d",i,Techs[i].name,tmp));
 }
notify(player,"----------------------------------------------------------");
} 

void techstat(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
  int lev,tmp,i,rate;
  float eff;


if(!IsIcReg(player)) return;
  notify(player,"\n\nCurrent Lab Status:");
  notify(player,"----------------------------------------------------------");
  tmp=atoi(vget_a(where_is(player),A_VA));
  eff=atof(vget_a(where_is(player),A_VA+2));
  rate=atoi(vget_a(where_is(player),A_VA+1));
  if(tmp > 0 && tmp <=TECHS && eff > 0 && eff <= 3 && rate > 0 && rate < 10) {
	lev=read_ilist(where_is(player),tmp,SKILL_A);
	notify(player,tprintf(" Lab Rating = %d\t\tEfficiency %f%%",rate,eff*100));
	notify(player,tprintf(" Currently Researching: %s to a level of %d",Techs[tmp].name,lev+1));
  	notify(player,"\n\t\tThis Research requires:");
  	notify(player,"----------------------------------------------------------");
	for(i=0;i<Techs[tmp].cneeds;i++) 
  		notify(player,tprintf("\t%3d unit(s) of %s",(lev+1)*Techs[tmp].amt[i],EconItems[Techs[tmp].cmods[i]].name));

  	notify(player,"\n----------------------------------------------------------");
	notify(player,"\t\tThis Technology Depends on:");
  	notify(player,"----------------------------------------------------------");
	for(i=0;i<Techs[tmp].needs;i++) 
		notify(player,tprintf("\t%s",Techs[Techs[tmp].tech[i]].name));
	}
  else notify(player,"Lab Idle");

  	notify(player,"\n----------------------------------------------------------");
	notify(player,"\t\tCurrent Technologies:");
  	notify(player,"----------------------------------------------------------");
	for(i=1;i<=TECHS;i++)
		if((lev=read_ilist(where_is(player),i,SKILL_A)) > 0)
 			notify(player,tprintf("\t%s (%d)",Techs[i].name,lev));


  notify(player,"----------------------------------------------------------");
}




int meet_reqs(int roomdb,TTechs ttech)
{
  int i,lev;
  lev=read_ilist(roomdb,ttech.num,SKILL_A);
  for(i=0;i<ttech.needs;i++) 
{

	if(read_ilist(roomdb,Techs[ttech.num].tech[i],SKILL_A) <= lev) return 0;
}

  for(i=0;i<ttech.cneeds;i++)
	if(read_ilist(roomdb,Techs[ttech.num].cmods[i],COMMOD_A) < (lev+1)*ttech.amt[i]) return 0;

  return 1;
}


void tech_update(key,data)
dbref key;
void *data;
{

  int whichtech,cmdy,lev,rate,i;

  whichtech=atoi(vget_a(key,A_VA));
  if(whichtech <=0 || whichtech > TECHS) return;
  if(meet_reqs(key,Techs[whichtech])) {
	lev=read_ilist(key,whichtech,SKILL_A);
  	for(i=0;i<Techs[whichtech].cneeds;i++)
	{
		cmdy=read_ilist(key,Techs[whichtech].cmods[i],COMMOD_A);
		cmdy=cmdy-Techs[whichtech].amt[i]*(lev+1);
		change_ilist(key,Techs[whichtech].cmods[i],cmdy,COMMOD_A);
	}
		possibly_increase_tech(key,Techs[whichtech]);
   }

}



void possibly_increase_tech(int roomdb, TTechs ttech)
{
  int ran,chance,rate,lev;

  chance=50;
  lev=read_ilist(roomdb,ttech.num,SKILL_A);
  rate=atoi(vget_a(roomdb,A_VA+1));
  chance+=10*rate;
  chance-=10*lev;
  chance-=10*ttech.diff;

  if(chance>=random()%100) 
	change_ilist(roomdb,ttech.num,lev+1,SKILL_A);

}


void settech(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *args[2];
char tmp[20];

if(!IsIcReg(player)) return;
if(xcode_parse(buffer,args,1)!=1) {notify(player,"Invalid Arguments!"); return;}
if(strlen(args[0])>10) return;
sprintf(tmp,"%d",atoi(args[0]));
if(atoi(tmp) < 0 || atoi(tmp) > TECHS) {notify(player,"Invalid Tech #"); return;}
vput_attribute(where_is(player),"va",tmp);
notify(player,tprintf("Now Researching %s",Techs[atoi(tmp)].name));

}


static char *rcstechc="$Id: tech.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
