
/*
 * econ.c -- 
 */


#define DIAMOND 5000
#define COMPST  29
#define SHIPST 36

Plans plans[]={


/*Plan # , NAME, TYPE
diff, # of inputs, 	Amount Out, Out Cmdty, 
5 Input Amounts, 
5 Input Commods,*/		

{0,(char*)NULL,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

{1,	(char*)"Mine Diamond 1",	1,
10,	0,	1,	1,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{2,	(char*)"Mine Ruby 1",	2,
7,	0,	1,	2,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{3,	(char*)"Mine Emerald 1",	3,
8,	0,	1,	3,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{4,	(char*)"Collect Pearl 1",	4,
6,	0,	1,	4,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{5,	(char*)"Mine Gold 1",	5,
6,	0,	1,	6,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{6,	(char*)"Mine Silver 1",	6,
5,	0,	2,	7,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{7,	(char*)"Mine Platinum 1",	7,
7,	0,	1,	8,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{8,	(char*)"Mine Copper 1",	8,
4,	0,	2,	11,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{9,	(char*)"Mine Coal 1",	9,
3,	0,	2,	26,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{10,	(char*)"Drill Oil 1",	10,
5,	0,	1,	27,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{11,	(char*)"Cut Hardwool 1",	11,
3,	0,	2,	19,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{12,	(char*)"Cut Softwool 1",	12,
2,	0,	2,	20,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{13,	(char*)"Gather Water - Scarce 1",	13,
10,	0,	1,	17,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},
{14,	(char*)"Gather Water 1",	14,
1,	0,	3,	17,
0,	0,	0,	0,	0,
0,	0,	0,	0,	0},

{0,(char*)NULL,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};		


/*1 unit~=500 Cr*/

Commod EconItems[]={
{(char *)NULL,		0,	0,	0,0},
{(char *)"Diamond",	1,	.1,	DIAMOND		,0},
{(char *)"Ruby",	2,	.1,	  .75*DIAMOND		,0},
{(char *)"Emerald",	3,	.1,	  .5*DIAMOND		,0},
{(char *)"Pearl",	4,	.1,	  .01*DIAMOND		,0},
{(char *)"Steel",	5,	.1,	  .00001*DIAMOND		,0},
{(char *)"Gold",	6,	.1,	  .000578*DIAMOND	,0},
{(char *)"Silver",	7,	.1,	  .000071*DIAMOND	,0},
{(char *)"Platinum",	8,	.1,	  .0006*DIAMOND		,0},
{(char *)"Quantium-40",	9,	.1,	 1.2*DIAMOND		,0},
{(char *)"Silicates",	10,	.1,	  .000001*DIAMOND	,0},
{(char *)"Copper",	11,	.1,	  .0000002*DIAMOND	,0},
{(char *)"Radioactives",12,	.1,	  .1*DIAMOND		,0},
{(char *)"Alloy"	,13,	.1,	  .00003*DIAMOND		,0},
{(char *)"Meats"	,14,	.1,	  .0000009*DIAMOND	,0},
{(char *)"Vegetables"	,15,	.1,	  .0000002*DIAMOND	,0},
{(char *)"Grains"	,16,	.1,	  .0000004*DIAMOND	,0},
{(char *)"Water"	,17,	.1,	  .00000005*DIAMOND	,0},
{(char *)"Diary"	,18,	.1,	  .00000005*DIAMOND	,0},

{(char *)"Hardwood"	,19,	.1,	  .000002*DIAMOND	,0},
{(char *)"Softwood"	,20,	.1,	  .000001*DIAMOND	,0},
{(char *)"Refined Coal"	,21,	.1,	  .000002*DIAMOND	,0},
{(char *)"Refined Oil"	,22,	.1,	  .00002*DIAMOND		,0},
{(char *)"Natural Gas"	,23,	.1,	  .00001*DIAMOND		,0},

{(char *)"Nitrates"	,24,	.1,	  .000002*DIAMOND	,0},
{(char *)"Plastic",	25,	.1,	  .000001*DIAMOND	,0},
{(char *)"Raw Coal",	26,	.1,	  .00000012*DIAMOND	,0},
{(char *)"Oil",		27,	.1,	  .000013*DIAMOND	,0},
{(char *)"Mineral Ore",	28,	.1,	  .0001*DIAMOND	 	,0},


/*Components*/
{(char *)"Links"	,COMPST,	1,	  .00005*DIAMOND	,1},
{(char *)"Data Crystal" ,COMPST+1,	1,	  .00005*DIAMOND	,1},
{(char *)"Light PPG"	,COMPST+2,	5,	  .00015*DIAMOND	,1},
{(char *)"Heavy PPG"	,COMPST+3,	10,	  .0003*DIAMOND		,1},
{(char *)"PPG Rifle"	,COMPST+4,	15,	  .0006*DIAMOND		,1},
{(char *)"Stun Gun"	,COMPST+5,	5,	  .00005*DIAMOND	,1},
{(char *)"Sword"	,COMPST+6,	8,	  .000005*DIAMOND	,1},


/*Some Ship Stuff*/
{(char *)"Jump Engine 1",SHIPST,	1000,	  1000*DIAMOND	,1},
{(char *)"Ion Enginer 1",SHIPST+1,	100,	  .1*DIAMOND	,1},
{(char *)"Ion Enginer 2",SHIPST+2,	750,	  10*DIAMOND	,1},
{(char *)"Jump Gate"	,SHIPST+3,	10000,	  2000*DIAMOND	,1},
{(char *)"Magneto-Grav Engine 1",SHIPST+4,	150,	  .15*DIAMOND	,1},
{(char *)"Magneto-Grav Engine 2",SHIPST+5,	850,	  15*DIAMOND	,1},
{(char *)"Earth DD Hull",SHIPST+6,	2500,	  1200*DIAMOND	,1},
{(char *)"Sensor Bank 1 ",SHIPST+7,	120,	  1.5*DIAMOND	,1},
{(char *)"Scanner Bank 1 ",SHIPST+8,	120,	  1.5*DIAMOND	,1},




{(char *)NULL,		0,	0,	0,0}};

Currency Moneys[]={
{(char *)NULL,		0,	0.0,	0,	0},
{(char *)"Credits",	1,	4.0,	1.0,	1000000},
{(char *)"Crystals",	2,	2.0,	.8,	1000000},
{(char *)"Ducats",	3,	3.5,	.9,	1000000},
{(char *)"Marks",	4,	5.5,	.5,	1000000},
{(char *)"C'thulin",	5,	6.5,	.4,	1000000},
{(char *)"Gordor",	6,	4.5,	.7,	1000000},
{(char *)"F'hrodo",	7,	7.5,	.3,	1000000},
{(char *)NULL,		0,	0.0,	0,	0}};


void do_econlist(player,cause,extra)
dbref player,cause;
int extra;
{
int x=1;
while(EconItems[x].num!=0) {
 notify_quiet(player,tprintf("Item:%20s Number:%3d Size:%4.3f Base Price:%3f",EconItems[x].name,EconItems[x].num,EconItems[x].size,EconItems[x].baseprice));
 x++;}
}

void econ_commod(object,viewer)
dbref object,viewer;
{
int x,tmp;

notify(viewer,"Commodity Id#\tCommodity Name\t\tAmount\t\tSize");
for(x=1;x<=COMODS;x++) {
 tmp=read_ilist(object,x,COMMOD_A);
 if(tmp>0) {
  notify(viewer,tprintf("%3d\t%21s\t\t%3d\t\t%5.2f",x,EconItems[x].name,tmp,tmp*EconItems[x].size));
  }
 }
}

void see_commods(player,cause,extra)
dbref player,cause;
int extra; {
notify(1,"Items you are carrying\n----------------------");
econ_commod(player,player);

 notify(player,"----------------------------------------------------------------");
 notify(player,tprintf("Cargo Space Free: %.2f / %.2f",atof(vget_a(player,CARGO)),atof(vget_a(player,MCARGO))));
 notify(player,"----------------------------------------------------------------");



}


void econ_commod_price(object,viewer,curr)
dbref object,viewer;
char *curr;
{
int x,tmp,mon=-1;
for(x=1;x<=MONEYS;x++) {
 if(string_compare2(curr,Moneys[x].name)==0) {mon=x;}
 }
if(mon==-1) {notify(viewer,tprintf("I dont know that Currency %s",curr)); return;}

notify(viewer,"Commodity Id#\tCommodity Name\tAmount\t\tSize\t\tPrice");
for(x=1;x<=COMODS;x++) {
 tmp=read_ilist(object,x,COMMOD_A);
 if(tmp>0) {
  notify(viewer,tprintf("%3d\t%21s\t%3d\t\t%5.2f\t\t%5.2f",x,EconItems[x].name,tmp,tmp*EconItems[x].size,EconItems[x].baseprice*Moneys[mon].worth*read_flist(object,x,A_VA)));
  }
 }
}


void do_supply(player,cause,extra)
dbref player,cause;
int extra;
{

notify(player,"Money Available for Purchases\n----------------------");
see_money(where_is(player),player);
}

void do_see_money(player,cause,extra)
dbref player,cause;
int extra;
{

notify(player,"Money you are carrying\n----------------------");
see_money(player,player);
}



void see_money(object,viewer)
dbref object, viewer;
{
int x;
float tmp;
for(x=1;x<=MONEYS;x++) {
 tmp=read_flist(object,x,MONEY_A);
 if(tmp>0) {
  notify(viewer,tprintf("%8f - %s",tmp,Moneys[x].name));
	}
    }
}
