/* $Id: vmspace.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
#include "header.h"

static char *vmspacecid="$Id: vmspace.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";

int VMRunSpace=0;
int VMCurrentMax;
int VMCurrentTMax;
int VMGlobalSpaceInit=0;
int MasterMineCnt=0;
int VMCounter=0;
int MasterMineCnt;
t_SpaceList *VMShipsMoving;
t_SpaceList *VMAllPlayers;
t_SpaceList *VMShipsTractoring;
t_MList *VMMissleQ;
t_ManList *Commus;
t_WList *VMWeaponQ;
t_WList *VMDamageQ;
t_WList *VMDamageSBQ;
t_SpaceList *VMShipsPowered;

int MaxTemps=441;

t_VMTemplate VMTemplate[]={
{0,"Star",75,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Nav Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Eng Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},	/*Helm location*/
{1,"Small_Planet",74,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Nav Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Eng Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},	/*Helm location*/
{2,"Med_Planet",73,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Nav Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Eng Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},	/*Helm location*/
{3,"Big_Planet",72,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Nav Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Eng Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},	/*Helm location*/
{4,"Jumpgate",76,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Nav Sys*/
0,0,0,0,8,8,8,8,8,8,0,0,0,0,0,0,0,0,0,0,	/*Eng Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},	/*Helm location*/
{5,"Warship_Sim",58,1,
337,174,401,293,199,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
265,525,550,0,89,87,87,87,87,87,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys*/
458,486,439,439,429,429,430,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE,FORE|STARB|PORT,FORE|AFT,FORE|AFT,FORE|STARB|PORT,FORE|STARB|PORT,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},       /*Helm location*/
{6,"Orion-D_SB",70,1,
338,0,417,294,201,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,             /*Nav Sys Sen Eng Thr Scan Jam Jump*/
256,506,550,127,102,102,102,102,104,104,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
447,447,447,447,447,447,50,50,50,50,429,429,429,429,429,183,183,183,183,183,183,183,183,183,183,      /*Helm Sys*/
FORE|TOP,FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|BOTTOM,AFT|PORT|TOP,AFT|STARB|BOTTOM,
FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,
FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,
FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,
FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,AFT|PORT|TOP,AFT|STARB|BOTTOM},     /*Helm location*/
{7,"Babylon_BS",71,1,
339,0,418,295,203,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,             /*Nav Sys Sen Eng Thr Scan Jam Jump*/
257,507,550,129,105,105,49,49,49,49,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
447,447,447,447,513,513,513,513,440,440,440,440,431,431,184,184,184,184,184,184,184,184,184,184,184,      /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|BOTTOM,
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,FORE|TOP,FORE|BOTTOM,AFT|TOP,AFT|BOTTOM,
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|TOP,FORE|BOTTOM,FORE|PORT|TOP,FORE|PORT|BOTTOM,
FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|TOP,AFT|PORT|TOP,AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM},     /*Helm location*/
{8,"Wasp_PD",0,1,
302,135,369,266,186,0,423,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Nav Sys Sen Eng Thr Scan Jam Jump*/
223,490,539,108,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{9,"Centaro_YA",1,1,
603,136,348,614,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,          /*Nav Sys Sen Eng Thr Scan Jam Jump*/
589,490,539,108,5,1,2,2,2,2,0,0,0,0,0,0,0,0,0,0,            /*Eng Sys Reac Batt EDC Comm F A P S D V*/
477,516,516,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{10,"Hades_SH",2,1,
612,141,355,619,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
229,490,539,110,15,14,15,15,14,14,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
435,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{11,"Babylon_DS",71,1,
339,0,418,295,203,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,             /*Nav Sys Sen Eng Thr Scan Jam Jump*/
257,507,550,129,105,105,49,49,49,49,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
440,440,513,513,513,513,183,183,183,183,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Helm Sys*/
FORE|TOP,AFT|BOTTOM,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,
FORE|TOP,FORE|PORT|TOP,FORE|STARB|TOP,AFT|BOTTOM,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,
0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{12,"Atlantis_SH",4,1,
308,139,351,270,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
229,490,539,110,8,2,5,5,5,5,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{13,"Praetorian_SH",5,1,
606,138,350,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
225,493,539,112,8,2,5,5,5,5,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{14,"Aryx_SH",6,1,
604,136,349,616,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
581,490,539,108,9,5,8,8,5,5,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{15,"Decius_SH",7,1,
606,138,352,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
225,493,539,112,9,8,8,8,8,8,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{16,"Kestrel_SH",2,1,
612,141,355,619,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
229,490,539,110,15,14,15,15,14,14,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{17,"SA-10_Aries",9,1,
303,143,355,267,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
227,491,539,110,11,8,9,9,9,9,0,0,0,0,0,0,0,0,0,0,       /*Eng Sys Reac Batt EDC Comm F A P S D V*/
484,484,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|AFT|PORT|STARB,FORE|AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{18,"Porcupine_CVL",10,1,
319,149,377,279,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
583,496,542,122,43,42,68,68,42,42,0,0,0,0,0,0,0,0,0,0,   /*Eng Sys Reac Batt EDC Comm F A P S D V*/
435,435,435,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT|STARB|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,FORE|PORT|STARB|TOP,
AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{19,"SA-12_Flying_Fox",11,1,                        /*ID Name HullNum 1*/
304,143,355,268,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
231,491,539,110,12,9,10,10,10,10,0,0,0,0,0,0,0,0,0,0,   /*Eng Sys Reac Batt EDC Comm F A P S D V*/
484,484,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|AFT|PORT|STARB,FORE|AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{20,"SA-15_Tiger",12,1,                            /*ID Name HullNum 1*/
305,143,355,269,188,0,424,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
653,492,539,110,13,10,12,12,11,11,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
433,433,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{21,"Shepherd-A_TS",13,1,
319,149,377,279,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
583,496,542,122,44,68,45,45,68,68,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
435,435,435,435,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,AFT|PORT|STARB|TOP,
FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{22,"Delta-III_FI",14,1,                               /*ID Name HullNum 1*/
303,142,354,267,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
227,491,539,110,13,2,10,10,10,10,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
484,484,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{23,"Enhat_FY",15,1,                                   /*ID Name HullNum 1*/
317,162,361,277,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
243,495,539,113,12,10,11,11,10,10,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
451,451,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{24,"Sentri_FI",16,1,                                     /*ID Name HullNum 1*/
606,142,358,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
584,494,539,112,14,11,13,13,11,11,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
480,480,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{25,"SA-23A_Nova",17,1,                            /*ID Name HullNum 1*/
309,146,358,270,188,0,424,0,0,0,0,0,0,0,0,0,0,0,0,0,    /*Nav Sys Sen Eng Thr Scan Jam Jump*/
653,492,539,110,15,12,14,14,13,13,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
434,434,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{26,"SA-23E_Aurora",19,1,                              /*ID Name HullNum 1*/
310,146,358,271,188,0,424,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
232,492,539,110,18,14,16,16,16,16,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
435,435,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{27,"SA-23G_Aurora",19,1,                                /*ID Name HullNum 1*/
310,146,358,271,188,0,424,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
232,492,539,110,18,14,16,16,16,16,0,0,0,0,0,0,0,0,0,0,   /*Eng Sys Reac Batt EDC Comm F A P S D V*/
435,435,425,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{28,"SA-25_Badger",20,1,                     /*ID Name HullNum 1*/
310,146,358,271,188,0,424,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
232,492,539,110,19,16,17,17,17,17,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
435,435,435,426,426,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,AFT,FORE|AFT,FORE|AFT,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{29,"Frazi_FI",21,1,                     /*ID Name HullNum 1*/
607,146,360,616,187,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
585,492,539,109,19,15,19,19,17,17,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
437,437,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{30,"SA-42B_Thunderbolt",22,1,                     /*ID Name HullNum 1*/
312,146,358,272,188,0,424,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
587,492,539,110,19,16,18,18,17,17,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
436,426,426,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{31,"Nial_FI",23,1,                     /*ID Name HullNum 1*/
314,162,361,274,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
226,495,539,113,21,18,20,20,20,20,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
452,452,452,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{32,"Hel'zha_FI",24,1,                                      /*ID Name HullNum 1*/
316,162,361,276,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       /*Nav Sys Sen Eng Thr Scan Jam Jump*/
226,495,539,113,23,21,22,22,22,22,0,0,0,0,0,0,0,0,0,0,   /*Eng Sys Reac Batt EDC Comm F A P S D V*/
716,716,428,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{33,"Vorlon_FI",25,1,                     /*ID Name HullNum 1*/
340,179,367,296,205,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
259,525,850,114,26,24,25,25,25,25,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
576,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{34,"Shadow_FI",26,1,                     /*ID Name HullNum 1*/
340,179,367,296,205,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
259,525,850,114,26,25,25,25,25,25,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
468,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{35,"Hyperion-T_CA",58,1,                     /*ID Name HullNum 1*/
330,155,396,286,193,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
252,535,545,122,85,82,87,87,84,84,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
458,458,458,458,486,486,443,443,443,481,481,481,183,183,183,183,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,AFT,AFT,FORE,FORE,FORE|PORT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT|STARB|TOP,
FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|TOP,
AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{36,"Omega-A_DD",60,1,                     /*ID Name HullNum 1*/
332,158,399,288,197,216,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
250,500,547,122,87,87,89,89,87,87,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
459,459,459,459,445,445,482,482,482,482,482,482,482,482,482,482,482,482,184,184,184,184,184,184,0,     /*Helm Sys weapons*/
FORE,FORE,AFT,AFT,FORE,FORE,FORE|PORT|TOP,FORE|PORT|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,
FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|STARB|TOP,
AFT|STARB|BOTTOM,FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,FORE|PORT|BOTTOM,FORE|STARB|BOTTOM,
AFT|PORT|TOP,AFT|STARB|TOP,PORT|TOP|BOTTOM,STARB|TOP|BOTTOM,0},     /*Helm location*/
{37,"Omega-B_DD",60,1,                                  /*ID Name HullNum 1*/
332,158,399,288,197,216,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
250,502,547,122,87,87,89,89,87,87,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
445,445,445,445,714,714,714,714,482,482,482,482,482,482,482,482,482,482,482,184,184,184,184,184,184,     /*Helm Sys weapons*/
FORE,FORE,FORE,FORE,AFT,AFT,AFT,AFT,FORE|PORT|TOP,FORE|PORT|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,
FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|STARB|TOP,
AFT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,FORE|PORT|BOTTOM,FORE|STARB|BOTTOM,AFT|PORT|TOP,
AFT|STARB|TOP,PORT|TOP|BOTTOM,STARB|TOP|BOTTOM},     /*Helm location*/
{38,"Vorchan_WS",53,1,                                      /*ID Name HullNum 1*/
608,154,394,621,198,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
246,535,543,124,70,70,73,73,70,70,0,0,0,0,0,0,0,0,0,0,   /*Eng Sys Reac Batt EDC Comm F A P S D V*/
489,471,471,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE|PORT|STARB,FORE|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{39,"Primus_BC",55,1,                                       /*ID Name HullNum 1*/
335,156,397,291,198,218,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
588,502,546,124,79,79,565,565,79,79,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
461,461,461,461,471,471,471,471,471,471,471,471,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT|STARB,
FORE|PORT|STARB,FORE|PORT,FORE|AFT|PORT,FORE|STARB,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{40,"Sharlin_WC",62,1,                     /*ID Name HullNum 1*/
337,174,401,293,199,220,422,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
255,505,549,125,90,90,91,91,90,90,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
450,450,450,450,450,450,457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,347, /*Helm Sys weapons*/
FORE,FORE,FORE,FORE,AFT,AFT,FORE,FORE,FORE,FORE,FORE|PORT,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|STARB,AFT|PORT,
AFT|PORT,AFT|STARB,AFT|STARB,AFT,AFT,AFT,AFT,FORE},     /*Helm location*/
{41,"Vorlon_TR",52,1,                     /*ID Name HullNum 1*/
341,180,391,297,206,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
261,526,851,120,93,92,93,93,93,93,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
569,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{42,"T'Loth_AC",57,1,                                     /*ID Name HullNum 1*/
318,159,400,278,196,212,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
251,535,546,121,82,82,88,88,82,82,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
488,488,488,488,442,442,442,442,442,442,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,FORE,FORE,FORE,FORE,AFT,AFT,PORT,STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{43,"G'Quan_CA",61,1,                     /*ID Name HullNum 1*/
611,159,400,622,196,213,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
254,501,547,121,89,84,89,89,87,87,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
460,460,474,474,474,474,439,439,439,439,430,430,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,FORE,FORE,AFT,AFT,FORE,FORE,AFT,AFT,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{44,"Purity_PT",27,1,                     /*ID Name HullNum 1*/
609,164,370,620,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
147,497,540,117,29,28,28,28,28,28,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
760,760,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{45,"Cerius_PS",28,1,                     /*ID Name HullNum 1*/
306,150,372,614,194,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,497,539,118,31,29,28,28,28,28,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{46,"G'Nath_SL",29,1,                     /*ID Name HullNum 1*/
607,149,373,616,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
238,492,539,115,34,31,32,32,31,31,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Helm Sys weapons*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{47,"G'Toth_N'Bah_FR",30,1,                             /*ID Name HullNum 1*/
607,149,373,616,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
238,496,539,115,41,37,38,38,37,37,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
432,432,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{48,"Galenn_PL",31,1,                     /*ID Name HullNum 1*/
325,165,374,283,195,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
249,500,543,119,36,31,33,33,32,32,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys weapons*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{49,"Skylark_FR",32,1,                     /*ID Name HullNum 1*/
323,166,375,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
235,496,541,116,38,37,33,33,32,32,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys weapons*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{50,"Berezebel_FR",33,1,                     /*ID Name HullNum 1*/
306,150,372,614,194,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
237,498,539,118,38,37,36,36,36,36,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
477,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{51,"Noir_FR",34,1,                     /*ID Name HullNum 1*/
323,149,371,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
235,496,540,116,41,38,34,34,34,34,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{52,"Gronquist_FR",35,1,                     /*ID Name HullNum 1*/
323,149,371,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
235,496,541,116,42,39,35,35,35,35,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
479,479,479,479,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{53,"Lazaren_FR",36,1,                     /*ID Name HullNum 1*/
609,166,376,620,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
657,496,541,117,43,40,37,37,37,37,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{54,"Teskan_FR",37,1,                     /*ID Name HullNum 1*/
323,166,375,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
235,496,541,116,44,42,37,37,37,37,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{55,"Charleston_FR",38,1,                     /*ID Name HullNum 1*/
324,166,371,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
239,497,541,116,70,44,38,38,38,38,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
479,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{56,"Askari_FR",39,1,                     /*ID Name HullNum 1*/
306,150,372,614,194,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
237,498,541,118,44,41,44,44,38,38,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{57,"Condor-D_TR",40,1,                     /*ID Name HullNum 1*/
323,149,375,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
67,497,542,116,43,42,43,43,42,42,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
627,627,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT|STARB|BOTTOM,AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{58,"Omaha_FR",41,1,                     /*ID Name HullNum 1*/
324,166,375,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
239,497,544,116,76,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
479,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{59,"Vakar_FR",42,1,                     /*ID Name HullNum 1*/
609,166,376,620,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
657,496,541,117,577,577,46,46,46,46,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{60,"Hunley_TA",43,1,                     /*ID Name HullNum 1*/
324,166,377,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
239,497,544,116,384,73,384,384,73,73,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
479,479,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{61,"Asimov_LL",44,1,                     /*ID Name HullNum 1*/
319,153,396,279,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,497,544,116,74,74,74,74,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
479,479,479,479,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT|STARB|TOP,AFT|PORT|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{62,"Ryan_FR",45,1,                     /*ID Name HullNum 1*/
324,166,377,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
731,497,544,116,79,77,78,78,78,78,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
627,627,627,627,183,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,
FORE|PORT|STARB|TOP,FORE|AFT|TOP|BOTTOM,FORE|AFT|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{63,"Vostok_FR",46,1,                     /*ID Name HullNum 1*/
328,160,398,285,193,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
253,535,546,116,87,79,54,54,54,54,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
627,627,627,627,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,FORE|PORT|STARB|TOP,
AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{64,"Presidential-D_TR",47,1,                     /*ID Name HullNum 1*/
610,153,396,285,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
244,498,544,122,83,600,84,84,84,84,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
481,481,184,184,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{65,"Corwin_FR",48,1,                     /*ID Name HullNum 1*/
328,160,378,285,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
253,535,546,122,87,84,596,596,596,596,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
627,627,627,627,184,184,184,184,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,
FORE|PORT|STARB|TOP,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{66,"Cotten-B_TE",49,1,                     /*ID Name HullNum 1*/
328,157,399,285,197,209,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
253,535,546,122,596,79,87,87,596,596,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
443,443,443,443,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,FORE|PORT|STARB|TOP,
AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{67,"Explorer-A_SS",50,1,                     /*ID Name HullNum 1*/
613,160,378,623,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
250,500,547,122,732,91,48,48,91,91,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
445,445,718,718,718,718,718,718,718,718,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,FORE|PORT|TOP,AFT|PORT|TOP,FORE|PORT|BOTTOM,AFT|PORT|BOTTOM,FORE|STARB|TOP,
AFT|STARB|BOTTOM,FORE|STARB|TOP,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{68,"Ashan_FR",51,1,                     /*ID Name HullNum 1*/
325,165,374,283,195,0,420,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
249,500,544,119,577,70,77,77,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{69,"Olympus-G_FL",52,1,                               /*ID Name HullNum 1*/
610,155,377,286,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
715,498,545,122,77,75,78,78,77,77,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
676,673,673,673,673,62,62,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys weapons*/
FORE|AFT|PORT|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|PORT,FORE|STARB,
FORE|AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{70,"Olympus-D_FL",52,1,                               /*ID Name HullNum 1*/
610,155,377,286,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
715,498,545,122,77,75,78,78,77,77,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
676,676,564,564,564,564,625,625,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys weapons*/
FORE|AFT|PORT|STARB,FORE|AFT|PORT|STARB,FORE|PORT|TOP,FORE|PORT|BOTTOM,FORE|STARB|TOP,FORE|STARB|BOTTOM,
FORE|PORT,FORE|STARB,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{71,"White_Star_WS",63,1,                     /*ID Name HullNum 1*/
334,168,393,290,199,219,420,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
591,503,545,125,74,71,72,72,71,71,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
449,455,455,455,455,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{72,"Vorlon_DE",64,1,                     /*ID Name HullNum 1*/
341,180,410,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
263,528,851,126,95,94,95,95,95,95,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
569,569,569,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT|STARB|TOP|BOTTOM,FORE|AFT|PORT|TOP|BOTTOM,FORE|AFT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{73,"Vorlon_DR",79,1,                     /*ID Name HullNum 1*/
341,181,411,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
264,529,851,126,97,96,97,97,97,97,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
483,483,566,566,566,566,566,566,566,566,570,570,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT,FORE|STARB,FORE|PORT,FORE|PORT,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|STARB,FORE|STARB,FORE|PORT|STARB|TOP|BOTTOM,
AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{74,"Vorlon_PK",66,1,                     /*ID Name HullNum 1*/
341,182,412,297,207,222,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
265,530,851,126,98,98,99,99,98,98,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
466,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{75,"Shadow_SC",84,1,                     /*ID Name HullNum 1*/
342,853,413,298,207,854,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
261,526,851,126,573,572,573,573,573,573,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
855,855,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{76,"Shadow_CR",68,1,                     /*ID Name HullNum 1*/
341,181,411,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
856,528,851,126,574,95,574,574,95,95,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
467,470,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{77,"Phalanx_OS",196,1,                     /*ID Name HullNum 1*/
923,0,1421,1313,1771,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
588,500,540,118,41,41,41,41,41,41,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
702,702,472,472,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT|STARB,FORE|PORT|STARB,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{78,"Hyperion-E_CC",58,1,                     /*ID Name HullNum 1*/
330,155,396,286,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
247,534,545,122,85,82,87,87,84,84,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
458,458,445,445,443,443,443,481,481,481,183,183,183,183,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE|PORT|STARB,AFT|PORT|STARB,AFT|PORT|STARB,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,
FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{79,"Wiz_Ship",78,1,                     /*ID Name HullNum 1*/
552,553,554,555,556,557,558,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
559,560,561,562,551,551,551,551,551,551,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
466,483,483,483,483,483,483,563,563,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT|STARB,AFT,AFT,FORE|PORT|STARB,FORE|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{80,"Escape_Pod",0,1,
302,0,0,266,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys*/
223,490,539,108,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{81,"Vorlon_CA",65,1,                     /*ID Name HullNum 1*/
341,181,411,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
856,529,851,126,27,575,27,27,27,27,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
483,566,566,566,566,570,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{82,"Vorlon_CVA",67,1,                     /*ID Name HullNum 1*/
341,181,411,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
856,529,851,126,27,575,27,27,27,27,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
483,566,566,566,566,570,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{83,"Vorlon_CL",80,1,                     /*ID Name HullNum 1*/
341,181,414,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
856,529,851,126,574,574,575,575,575,575,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
465,566,566,566,566,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{84,"Vorlon_CVL",64,1,                     /*ID Name HullNum 1*/
341,180,410,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
263,528,851,126,95,94,95,95,95,95,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
300,566,566,569,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{85,"Vorlon_SC",64,1,                     /*ID Name HullNum 1*/
342,180,410,298,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
261,526,851,126,95,94,95,95,95,95,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
570,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{86,"Shadow_DR",82,1,                     /*ID Name HullNum 1*/
341,181,411,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
264,529,851,126,27,575,27,27,575,575,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
467,467,470,470,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{87,"Shadow_CV",83,1,                     /*ID Name HullNum 1*/
341,181,411,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
856,528,851,126,301,574,301,301,574,574,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
467,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{88,"Shadow_DD",84,1,                     /*ID Name HullNum 1*/
341,853,413,297,207,854,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
261,526,851,126,573,572,573,573,573,573,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
345,345,469,469,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{89,"Vorlon_DB",64,1,                     /*ID Name HullNum 1*/
341,180,410,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
263,528,851,126,95,94,95,95,95,95,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
300,566,566,569,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{90,"Wafflerian_DD",86,1,                     /*ID Name HullNum 1*/
552,553,554,555,556,557,558,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
559,560,561,562,551,551,551,551,551,551,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
466,483,483,483,483,483,483,563,563,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT|STARB,
AFT,AFT,FORE|PORT|STARB,FORE|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{91,"Leviathan_LL",87,1,                     /*ID Name HullNum 1*/
552,553,554,555,556,557,558,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
559,560,561,562,551,551,551,551,551,551,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
465,465,445,445,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{92,"Delta-IV_FI",14,1,                                /*ID Name HullNum 1*/
305,142,354,269,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
653,491,539,110,13,2,10,10,10,10,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
824,824,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{93,"Nova-A_DR",59,1,                                    /*ID Name HullNum 1*/
326,157,398,284,197,210,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
252,535,546,122,87,84,87,87,87,87,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
673,673,673,673,673,673,673,673,673,673,673,673,673,673,673,673,673,673,183,183,183,183,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE|PORT,FORE|PORT,FORE|PORT,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|STARB,
FORE|STARB,FORE|STARB,AFT|PORT,AFT|PORT,AFT|STARB,AFT|STARB,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,
AFT|STARB|BOTTOM,0,0,0},  /*Helm location*/
{94,"Nova-B_DR",59,1,                                  /*ID Name HullNum 1*/
326,157,398,284,197,210,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
252,535,546,122,87,84,87,87,87,87,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
673,673,673,673,673,673,673,673,673,714,714,714,714,714,714,714,714,714,183,183,183,183,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT|PORT,AFT|STARB,FORE,FORE|PORT,
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT|TOP,FORE|STARB|TOP,
AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0},  /*Helm location*/
{95,"Nova_SS",59,1,                                     /*ID Name HullNum 1*/
330,157,398,286,197,210,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
247,534,546,122,87,84,87,87,87,87,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
183,183,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Helm Sys weapons*/
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{96,"Gorith_FI",89,1,                                      /*ID Name HullNum 1*/
321,146,360,615,187,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
659,491,539,109,14,12,13,13,13,13,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
691,691,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{97,"Sho'Kos_PC",90,1,                                     /*ID Name HullNum 1*/
630,592,403,281,196,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
657,496,542,121,74,71,72,72,71,71,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
444,439,439,690,690,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{98,"Sho'Kar_CS",91,1,                                       /*ID Name HullNum 1*/
636,155,402,646,196,214,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
720,499,545,121,80,597,80,80,597,597,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
681,681,681,682,682,682,682,685,685,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,AFT|PORT,AFT|STARB,FORE,FORE,AFT,AFT,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{99,"Dag'Kar_FG",92,1,                                    /*ID Name HullNum 1*/
635,592,403,645,196,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
719,498,543,121,76,74,75,75,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
626,626,626,626,626,626,430,430,430,430,430,430,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE,FORE,FORE,FORE,FORE,FORE,FORE,FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0},       /*Helm location*/
{100,"Thentus_FF",93,1,                                    /*ID Name HullNum 1*/
635,592,403,645,196,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
722,498,544,121,77,593,76,76,593,593,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
689,689,473,473,473,473,690,690,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{101,"Rongoth_DD",94,1,                                         /*ID Name HullNum 1*/
635,155,402,645,196,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,           /*Nav Sys Sen Eng Thr Scan Jam Jump*/
144,499,544,121,71,71,73,73,71,71,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
382,382,473,473,473,473,442,442,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},       /*Helm location*/
{102,"Var'Nic_DP",95,1,                                         /*ID Name HullNum 1*/
635,155,402,645,196,214,0,0,0,0,0,0,0,0,0,0,0,0,0,0,         /*Nav Sys Sen Eng Thr Scan Jam Jump*/
144,499,544,121,79,597,87,807,597,597,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
687,687,382,473,473,439,439,626,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT,FORE|PORT,FORE,FORE|PORT,FORE|STARB,AFT|PORT|STARB,AFT|PORT|STARB,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{103,"Ka'Toc_DB",96,1,                                        /*ID Name HullNum 1*/
636,155,402,646,196,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       /*Nav Sys Sen Eng Thr Scan Jam Jump*/
655,534,545,121,79,597,79,79,597,597,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
686,686,677,442,442,442,442,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT,FORE|STARB,FORE,FORE,FORE,AFT,AFT,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{104,"G'Karith_CP",97,1,                                    /*ID Name HullNum 1*/
632,159,400,643,196,213,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
178,499,545,121,596,84,565,565,84,84,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
683,683,683,683,439,439,439,439,684,684,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE,FORE,AFT,AFT,FORE|PORT|STARB,AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{105,"Bin'Tak_DR",98,1,                                      /*ID Name HullNum 1*/
611,159,400,622,196,213,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
260,502,547,121,595,90,594,594,90,90,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
680,680,680,680,680,680,678,681,681,681,681,681,682,682,682,682,682,626,626,430,430,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,
AFT,FORE,FORE|PORT,FORE|STARB,AFT,AFT,FORE,FORE,FORE,FORE,0,0,0,0}, /*Helm location*/
{106,"Arcismus_SS",99,1,                                   /*ID Name HullNum 1*/
630,149,373,281,192,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
658,496,544,115,78,78,79,79,78,78,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
439,439,439,439,690,690,690,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT,FORE|STARB,AFT,AFT,FORE,FORE,AFT,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{107,"Ja'Dul_SB",100,1,                                   /*ID Name HullNum 1*/
638,0,418,647,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       /*Nav Sys Sen Eng Thr Scan Jam Jump*/
670,506,550,128,390,390,390,390,408,408,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
692,692,692,693,693,693,693,693,693,693,693,693,693,693,693,695,695,695,695,695,695,0,0,0,0, /*Helm Sys weapons*/
FORE,AFT|PORT,AFT|STARB,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,
FORE|TOP,FORE|BOTTOM,FORE|BOTTOM,AFT|PORT|TOP,AFT|PORT|TOP,AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM,
AFT|STARB|BOTTOM,FORE|TOP,FORE|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM,0,0,0,0}, /*Helm location*/
{108,"Ja'Stat_WB",101,1,                                        /*ID Name HullNum 1*/
639,0,418,648,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,             /*Nav Sys Sen Eng Thr Scan Jam Jump*/
669,507,550,128,602,602,602,602,103,103,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
692,692,692,692,692,679,679,694,694,693,693,693,693,693,693,695,695,695,684,684,628,628,628,624,624, /*Helm Sys weapons*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,
FORE|AFT|PORT|TOP,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,
FORE|AFT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,FORE|AFT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,
FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,
FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM}, /*Helm location*/
{109,"Razik_FI",102,1,                                    /*ID Name HullNum 1*/
606,142,358,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
668,493,539,112,13,10,12,12,10,10,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
706,706,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{110,"Rutarian_FI",103,1,                                 /*ID Name HullNum 1*/
605,142,358,618,651,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
224,494,539,112,15,10,13,13,11,11,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
705,704,704,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{111,"Covran_SC",104,1,                                      /*ID Name HullNum 1*/
637,154,394,291,198,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
725,501,545,124,600,78,81,81,600,600,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
701,701,701,701,700,700,700,700,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{112,"Haven_PB",105,1,                                    /*ID Name HullNum 1*/
633,629,404,644,198,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
666,499,542,124,45,43,68,68,43,43,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
699,472,472,472,472,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{113,"Mograth_FF",106,1,                                  /*ID Name HullNum 1*/
633,629,404,644,198,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
665,499,543,124,75,73,75,75,73,73,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
699,699,703,472,472,698,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE,FORE|PORT,FORE|STARB,FORE|AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{114,"Maximus_FF",107,1,                                  /*ID Name HullNum 1*/
608,629,404,621,198,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
664,534,544,124,73,70,71,71,70,70,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
472,472,472,472,698,698,698,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT|STARB,AFT|PORT|STARB,AFT|PORT|STARB,FORE|AFT|PORT|STARB,FORE|AFT|PORT|STARB,
FORE|AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{115,"Darkner_FF",108,1,                                  /*ID Name HullNum 1*/
637,629,404,291,198,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
663,500,544,124,69,69,71,71,69,69,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
702,702,699,699,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{116,"Kutai_GS",109,1,                                     /*ID Name HullNum 1*/
633,154,379,644,198,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
137,534,542,124,72,72,75,75,72,72,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
699,699,699,699,699,699,472,472,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT|STARB,AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{117,"Sulust_DE",110,1,                                   /*ID Name HullNum 1*/
637,154,394,291,198,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
662,501,545,124,75,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
702,472,472,472,472,698,698,698,698,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{118,"Altarian_DD",111,1,                                  /*ID Name HullNum 1*/
634,154,394,644,198,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /*Nav Sys Sen Eng Thr Scan Jam Jump*/
663,500,545,124,75,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
697,697,697,471,471,471,471,471,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,AFT,FORE|PORT|STARB,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{119,"Centurion_AC",112,1,                                /*ID Name HullNum 1*/
335,156,397,291,198,218,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
228,502,545,124,79,79,54,54,79,79,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
461,461,461,699,699,471,471,471,471,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{120,"Dargan_CS",113,1,                                    /*ID Name HullNum 1*/
335,156,397,291,454,218,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /*Nav Sys Sen Eng Thr Scan Jam Jump*/
661,502,546,124,81,453,85,85,81,81,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
461,461,697,697,697,471,471,471,471,471,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{121,"Emperor_TR",55,1,                                     /*ID Name HullNum 1*/
335,156,397,291,198,218,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
588,502,546,124,79,79,565,565,79,79,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
471,471,471,471,471,471,471,471,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT,FORE|AFT|PORT,
FORE|STARB,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{122,"Balvarin_CV",114,1,                                 /*ID Name HullNum 1*/
631,156,397,642,198,218,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
148,535,546,124,79,78,87,87,79,79,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
472,472,472,472,472,698,698,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,AFT|PORT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{123,"Octurion_BB",115,1,                                      /*ID Name HullNum 1*/
335,156,397,291,198,218,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        /*Nav Sys Sen Eng Thr Scan Jam Jump*/
660,503,547,124,89,598,599,599,598,598,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
461,461,461,461,461,461,697,697,697,697,471,471,471,471,471,471,471,471,471,471,471,471,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,
FORE|PORT,FORE|PORT,FORE|PORT,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|STARB,FORE|STARB,
FORE|STARB,AFT|PORT,AFT|STARB,0,0,0}, /*Helm location*/
{124,"Lias_SS",116,1,                                          /*ID Name HullNum 1*/
631,150,372,642,194,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        /*Nav Sys Sen Eng Thr Scan Jam Jump*/
724,499,543,118,54,54,596,596,54,54,0,0,0,0,0,0,0,0,0,0,      /*Eng Sys Reac Batt EDC Comm F A P S D V*/
472,472,472,472,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{125,"Marcanos_SB",117,1,                                       /*ID Name HullNum 1*/
640,0,417,649,202,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,             /*Nav Sys Sen Eng Thr Scan Jam Jump*/
256,506,550,131,100,100,100,100,601,601,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
710,710,710,710,711,711,711,711,711,711,711,711,711,711,711,711,711,711,711,711,711,711,711,711,0,  /*Helm Sys*/
FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|PORT|BOTTOM,AFT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,
FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|BOTTOM,FORE|TOP,FORE|TOP,FORE|BOTTOM,FORE|BOTTOM,AFT|TOP,AFT|TOP,
AFT|BOTTOM,AFT|BOTTOM,PORT|TOP,PORT|TOP,PORT|BOTTOM,PORT|BOTTOM,STARB|TOP,STARB|TOP,STARB|BOTTOM,STARB|BOTTOM,0}, /*Helm location*/
{126,"Kraken_SB",118,1,                                         /*ID Name HullNum 1*/
641,0,417,650,202,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,             /*Nav Sys Sen Eng Thr Scan Jam Jump*/
257,507,550,131,106,106,106,106,107,107,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
707,707,707,707,707,707,708,708,708,708,708,708,709,709,709,709,709,709,709,709,709,709,709,709,709,  /*Helm Sys*/
FORE|AFT|PORT|TOP,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,
FORE|AFT|STARB|BOTTOM,FORE,AFT,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|AFT|PORT,FORE|AFT|PORT,FORE|AFT|PORT,
FORE|AFT|STARB,FORE|AFT|STARB,FORE|AFT|STARB,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|BOTTOM}, /*Helm location*/
{127,"Morgan_BW",119,1,                                      /*ID Name HullNum 1*/
330,155,399,286,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
655,534,544,122,83,83,83,83,83,83,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
673,714,714,832,832,481,481,481,481,481,481,481,481,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE,AFT|PORT,AFT|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,PORT,PORT,STARB,STARB,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{128,"Oracle_CS",120,1,                                     /*ID Name HullNum 1*/
332,155,396,288,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
248,500,545,122,83,80,596,596,80,80,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
717,717,718,718,718,718,718,718,625,183,183,183,183,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE|BOTTOM,FORE|BOTTOM,AFT|TOP,AFT|TOP,PORT|TOP,STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{129,"Sunhawk_BC",121,1,                                  /*ID Name HullNum 1*/
727,169,395,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,497,541,123,75,75,78,78,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
729,730,730,730,730,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{130,"Secundus_AC",55,1,                                     /*ID Name HullNum 1*/
335,156,397,291,198,218,421,421,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
588,502,546,124,79,79,565,565,79,79,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
61,61,61,61,471,471,471,471,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{131,"Delta-V_FI",14,1,                                /*ID Name HullNum 1*/
311,142,354,271,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
232,491,539,110,13,2,10,10,10,10,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
823,823,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{132,"Grey_Sharlin_WC",62,1,                          /*ID Name HullNum 1*/
522,174,401,446,199,220,422,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
356,505,549,125,90,90,91,91,90,90,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
450,450,450,450,450,450,457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,457,347, /*Helm Sys weapons*/
FORE,FORE,FORE,FORE,AFT,AFT,FORE,FORE,FORE,FORE,FORE|PORT,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|STARB,AFT|PORT,
AFT|PORT,AFT|STARB,AFT|STARB,AFT,AFT,AFT,AFT,FORE},     /*Helm location*/
{133,"G'Quonth_AC",61,1,                                 /*ID Name HullNum 1*/
611,159,400,622,196,213,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
254,501,547,121,89,84,89,89,87,87,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
460,460,686,686,474,474,474,474,439,439,439,439,626,626,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,FORE|PORT,FORE|STARB,FORE,FORE,AFT,AFT,FORE,FORE,AFT,AFT,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{134,"Poseidon_CVA",144,1,                                 /*ID Name HullNum 1*/
332,160,378,288,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
250,500,547,122,89,89,599,599,89,89,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
714,714,714,714,482,482,482,482,184,184,184,184,184,184,185,185,185,185,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,
FORE|PORT|TOP,FORE|AFT|PORT|TOP,AFT|PORT|BOTTOM,FORE|STARB|TOP,FORE|AFT|STARB|TOP,AFT|STARB|BOTTOM,
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0}, /*Helm location*/
{135,"Condor-E_TR",40,1,                     /*ID Name HullNum 1*/
323,149,375,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
67,497,542,116,43,42,43,43,42,42,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
435,435,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT|STARB|BOTTOM,AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{136,"Cotten-A_TE",49,1,                     /*ID Name HullNum 1*/
328,157,399,285,197,209,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
253,535,546,122,596,79,87,87,596,596,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{137,"Medusa-A_BB",143,1,                     /*ID Name HullNum 1*/
332,160,398,288,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
250,500,547,122,89,89,63,63,89,89,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
459,459,459,459,673,673,673,673,714,714,714,714,771,771,771,768,768,768,768,184,184,184,184,184,184,   /*Helm Sys*/
FORE,FORE,FORE,AFT,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,
FORE,FORE|PORT,FORE|STARB,FORE,FORE,FORE,AFT,FORE|PORT|BOTTOM,FORE|STARB|BOTTOM,FORE|AFT|PORT|TOP,
FORE|AFT|STARB|BOTTOM,AFT|PORT|TOP,AFT|STARB|TOP},  /*Helm location*/
{138,"Omega-D_DD",60,1,                     /*ID Name HullNum 1*/
613,158,399,623,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
359,504,547,122,87,87,89,89,87,87,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
674,674,365,365,365,365,675,675,675,675,675,675,675,675,675,675,675,675,184,184,184,184,184,184,0,   /*Helm Sys*/
FORE,FORE,FORE,FORE,AFT,AFT,FORE|PORT|TOP,FORE|PORT|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,FORE|AFT|PORT|TOP,
FORE|AFT|PORT|BOTTOM,FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM,FORE|AFT|STARB|TOP,
FORE|AFT|STARB|BOTTOM,FORE|PORT|BOTTOM,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|STARB|TOP,PORT|TOP|BOTTOM,STARB|TOP|BOTTOM,0},  /*Helm location*/
{139,"Orestes-E_MO",137,1,                     /*ID Name HullNum 1*/
330,153,398,286,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
234,534,543,122,80,79,87,87,80,80,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
458,458,531,531,782,782,443,443,443,443,481,481,183,183,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,AFT,AFT,FORE|PORT,FORE|STARB,FORE|PORT|TOP,AFT|PORT|BOTTOM,FORE|STARB|TOP,AFT|STARB|BOTTOM,
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{140,"Sagittarius-A_CG",134,1,                     /*ID Name HullNum 1*/
610,155,396,286,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
244,498,544,122,80,600,84,84,80,80,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
481,481,625,625,625,625,625,625,625,625,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT,FORE|PORT,
FORE|STARB,FORE|STARB,AFT|PORT|STARB,AFT|PORT|STARB,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{141,"Sagittarius-B_CG",134,1,                     /*ID Name HullNum 1*/
610,155,396,286,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
244,498,544,122,80,600,84,84,80,80,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
481,481,427,427,427,427,427,427,427,427,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT,FORE|PORT,
FORE|STARB,FORE|STARB,AFT|PORT|STARB,AFT|PORT|STARB,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{142,"Avenger-A_CV",136,1,                     /*ID Name HullNum 1*/
323,155,396,282,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
240,498,544,122,79,79,87,87,79,79,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
773,773,773,512,512,512,512,183,183,183,183,183,183,183,183,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT|TOP,FORE|PORT|BOTTOM,
FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM,
0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{143,"Avenger-G_CV",136,1,                     /*ID Name HullNum 1*/
323,155,396,282,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
240,498,544,122,79,79,87,87,79,79,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
773,773,773,438,438,438,438,438,438,183,183,183,183,183,183,183,183,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,PORT|TOP,STARB|TOP,AFT|PORT,AFT|STARB,FORE|PORT|TOP,
FORE|PORT|BOTTOM,FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,AFT|STARB|TOP,
AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0},  /*Helm location*/
{144,"Grove-A_MB",206,1,                     /*ID Name HullNum 1*/
324,153,378,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,496,545,116,600,600,600,600,600,600,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
627,627,627,627,62,62,62,62,62,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,FORE|PORT,FORE|STARB,FORE|AFT|PORT,
FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{145,"Artemis-B_FF",131,1,                     /*ID Name HullNum 1*/
326,166,377,284,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
239,497,543,122,71,71,77,77,71,71,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
782,782,782,782,782,782,481,481,481,481,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT|PORT,AFT|STARB,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,
FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{146,"Turner-G_QS",41,1,                     /*ID Name HullNum 1*/
324,166,375,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,496,544,116,76,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
771,771,771,771,627,625,625,625,625,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|STARB,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT,
FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{147,"Perseus-G_MS",129,1,                     /*ID Name HullNum 1*/
323,149,377,282,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
242,496,543,122,74,73,75,75,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
764,764,438,438,625,625,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|TOP,FORE|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,FORE,FORE,FORE|PORT|STARB|TOP,
AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{148,"Freeh_PH",207,1,                     /*ID Name HullNum 1*/
324,149,372,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,497,543,116,70,69,70,70,69,69,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
531,443,443,438,438,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{149,"Tethys-K_PC",128,1,                     /*ID Name HullNum 1*/
323,149,371,282,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
67,497,542,122,47,45,47,47,45,45,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
443,443,438,438,438,438,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT|STARB,FORE|PORT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,
FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{150,"Tethys-Z_PC",128,1,                     /*ID Name HullNum 1*/
323,149,371,282,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
67,497,542,122,47,45,47,47,45,45,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
481,481,481,625,625,625,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,FORE|PORT|STARB|BOTTOM,FORE|AFT|PORT,FORE|AFT|STARB,AFT|PORT|STARB,
FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{151,"xxxFleiss_CE",208,1,                     /*ID Name HullNum 1*/
319,149,376,279,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
238,496,542,116,69,44,69,69,44,44,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
443,438,438,806,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{152,"Lamphrey_BP",122,1,                     /*ID Name HullNum 1*/
308,139,351,270,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
229,490,539,108,21,18,20,20,20,20,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{153,"Eisner_PL",209,1,                     /*ID Name HullNum 1*/
319,153,396,279,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,497,544,116,70,70,73,73,70,70,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
627,627,627,627,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{154,"Buffett_OB",206,1,                     /*ID Name HullNum 1*/
324,153,378,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,496,545,116,600,600,600,600,600,600,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
627,627,627,627,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{155,"Hermes-A_TR",127,1,                     /*ID Name HullNum 1*/
319,166,375,279,193,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
242,496,541,116,76,593,76,76,593,593,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
481,481,481,481,625,625,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,FORE|PORT|BOTTOM,FORE|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,
FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{156,"Stempel_BF",210,1,                     /*ID Name HullNum 1*/
324,166,375,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
731,497,544,116,45,45,47,47,45,45,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
773,773,627,627,627,627,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{157,"Majestic_FR",202,1,                     /*ID Name HullNum 1*/
324,149,375,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
239,497,544,116,76,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
479,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE|AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{158,"Trump_TG",211,1,                     /*ID Name HullNum 1*/
319,157,378,279,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
731,497,543,116,70,69,70,70,69,69,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
479,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{159,"Rukeyser_SB",212,1,                     /*ID Name HullNum 1*/
324,0,417,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
725,501,546,122,598,598,598,598,90,90,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
773,773,773,773,627,627,627,627,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|STARB|TOP,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,AFT|PORT|STARB|BOTTOM,FORE|PORT|STARB|TOP,
FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{160,"Aegis-A_OS",150,1,                     /*ID Name HullNum 1*/
330,0,416,286,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
250,500,544,116,81,81,82,82,81,81,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
458,458,438,438,438,438,429,429,429,429,184,184,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|TOP|BOTTOM,FORE|TOP|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,
FORE|AFT|STARB|BOTTOM,FORE|PORT|STARB|TOP|BOTTOM,FORE|PORT|STARB|TOP|BOTTOM,FORE|PORT|STARB|TOP|BOTTOM,
FORE|PORT|STARB|TOP|BOTTOM,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{161,"Aegis-B_OS",150,1,                     /*ID Name HullNum 1*/
330,0,416,286,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
250,500,544,116,81,81,82,82,81,81,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
448,438,438,438,438,429,429,429,429,184,184,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,FORE|PORT|STARB|TOP|BOTTOM,
FORE|PORT|STARB|TOP|BOTTOM,FORE|PORT|STARB|TOP|BOTTOM,FORE|PORT|STARB|TOP|BOTTOM,FORE|AFT|PORT|STARB|TOP,
FORE|AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{162,"Reagan_OS",149,1,                     /*ID Name HullNum 1*/
326,0,416,284,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
234,534,540,116,37,37,37,37,37,37,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
438,438,429,429,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,FORE|PORT|STARB|TOP|BOTTOM,FORE|PORT|STARB|TOP|BOTTOM,
FORE|AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{163,"Class-D1_DM",214,1,                     /*ID Name HullNum 1*/
324,0,416,282,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
666,499,539,110,10,10,10,10,10,10,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{164,"Class-D2_DM",215,1,                     /*ID Name HullNum 1*/
324,0,416,282,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
666,499,539,110,15,15,15,15,15,15,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
440,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{165,"Class-D3_DM",216,1,                     /*ID Name HullNum 1*/
324,0,416,282,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
666,499,539,110,14,14,14,14,14,14,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
773,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{166,"Drakkar_DS",190,1,                     /*ID Name HullNum 1*/
330,155,399,286,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
234,534,545,122,85,81,85,85,81,81,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
839,763,763,771,443,443,481,481,764,764,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|AFT|PORT,FORE|AFT|STARB,FORE,FORE,FORE,AFT|PORT|TOP,AFT|STARB|TOP,FORE|AFT|PORT|BOTTOM,
FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{167,"Menger_CVS",213,1,                     /*ID Name HullNum 1*/
324,166,396,282,193,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,497,545,116,596,79,600,600,600,600,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
443,443,481,481,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT,FORE|AFT|STARB,FORE|TOP,AFT|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{168,"Greenspan_BQ",217,1,                     /*ID Name HullNum 1*/
324,166,375,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,496,544,116,71,71,74,74,71,71,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
771,443,443,482,482,764,764,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{169,"Malthus_GL",210,1,                     /*ID Name HullNum 1*/
727,166,375,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,496,544,116,45,45,47,47,45,45,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
763,763,773,773,831,831,627,627,627,627,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,
FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{170,"Keynes_BG",218,1,                     /*ID Name HullNum 1*/
323,166,375,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
731,497,544,116,68,68,45,45,68,68,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
773,773,443,443,481,481,481,481,764,764,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB,FORE|AFT|PORT|STARB,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT,FORE|STARB,
FORE|PORT|STARB,AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{171,"Rossotti_WR",41,1,                     /*ID Name HullNum 1*/
727,166,375,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,496,544,116,76,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
763,763,763,763,481,481,481,481,481,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|AFT|PORT|STARB|TOP,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{172,"Summers_XE",202,1,                     /*ID Name HullNum 1*/
326,149,375,284,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
731,497,544,116,76,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
817,817,481,481,481,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{173,"Veblen_FE",202,1,                     /*ID Name HullNum 1*/
324,149,375,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
731,497,544,116,76,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
481,481,481,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{174,"Sowell_SL",219,1,                     /*ID Name HullNum 1*/
319,149,372,279,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
583,496,543,116,593,75,593,593,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
443,443,481,481,481,481,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{175,"Bastiat_SN",43,1,                     /*ID Name HullNum 1*/
324,166,377,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
239,497,544,116,384,73,384,384,73,73,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
818,818,627,627,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{176,"Hayek_GB",220,1,                     /*ID Name HullNum 1*/
326,149,372,284,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
67,497,541,116,68,42,68,68,42,42,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
763,627,627,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{177,"Double-V_FI",201,1,                     /*ID Name HullNum 1*/
520,386,380,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,110,15,11,14,14,13,13,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
826,826,426,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{178,"Mises_SB",221,1,                     /*ID Name HullNum 1*/
727,0,366,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
661,501,546,122,601,601,601,601,601,601,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
771,771,481,481,481,481,481,481,481,481,481,481,481,481,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|TOP,FORE|BOTTOM,FORE|PORT|TOP,FORE|PORT|BOTTOM,
FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|TOP,AFT|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM,
0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{179,"Farragut_GB",222,1,                     /*ID Name HullNum 1*/
610,166,377,286,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
715,498,546,116,600,577,600,600,577,577,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
476,478,481,481,481,481,803,803,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT,FORE|STARB,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{180,"xxxHindenburg_DE",223,1,                     /*ID Name HullNum 1*/
610,166,375,286,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
724,499,544,116,577,77,577,577,77,77,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
482,482,482,482,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT|STARB,AFT|PORT|STARB,AFT|PORT|STARB,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{181,"Mahan_CVE",224,1,                     /*ID Name HullNum 1*/
326,166,377,284,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
239,496,544,116,42,42,43,43,42,42,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
478,478,481,481,481,481,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,FORE|PORT|STARB|TOP,
AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{182,"Nelson_GB",225,1,                     /*ID Name HullNum 1*/
326,149,373,284,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
658,496,544,116,72,70,72,72,70,70,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
478,481,481,803,803,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT,FORE|STARB,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{183,"Nagumo_CE",208,1,                     /*ID Name HullNum 1*/
319,149,376,282,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
583,496,542,116,69,44,69,69,44,44,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
481,481,481,481,481,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{184,"Spruance_GB",208,1,                     /*ID Name HullNum 1*/
326,149,376,284,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
658,496,542,116,69,44,69,69,44,44,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
478,481,481,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{185,"Starfox_FI",11,1,                     /*ID Name HullNum 1*/
308,143,355,270,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
227,491,539,110,12,9,10,10,10,10,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
804,804,425,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{186,"Shargoti_BC",147,1,
522,174,401,446,199,220,422,789,789,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
356,505,549,125,595,595,732,732,595,595,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
462,462,462,462,462,462,738,738,738,738,738,738,738,738,738,738,738,738,738,738,738,347,347,0,0,   /*Helm Sys*/
FORE,FORE,FORE,FORE,AFT,AFT,FORE,FORE,FORE,FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT|PORT,AFT|PORT,
AFT|STARB,AFT|STARB,AFT,AFT,AFT,FORE,FORE,0,0},  /*Helm location*/
{187,"Neshatan_GS",146,1,
337,174,401,293,199,220,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
353,505,547,125,599,599,63,63,599,599,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
450,450,450,450,450,450,450,450,450,450,457,457,457,457,457,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|PORT,AFT|STARB,AFT|STARB,FORE,FORE|PORT,FORE|STARB,
AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{188,"Tigara_AC",142,1,
55,173,401,59,199,220,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
258,503,546,125,88,88,726,726,88,88,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
52,52,51,51,51,51,51,51,475,475,475,475,475,475,475,475,475,475,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE|PORT,FORE|STARB,AFT,AFT,FORE,FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,
AFT|PORT,AFT|STARB,AFT,AFT,0,0,0,0,0,0,0},  /*Helm location*/
{189,"Leshath_SC",142,1,
337,173,401,293,199,220,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
262,505,546,125,88,88,726,726,88,88,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
738,738,738,738,738,738,738,738,738,738,738,738,738,738,347,347,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT|PORT,AFT|STARB,AFT,AFT,AFT,AFT,
FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{190,"Tinashi_WF",140,1,
337,173,401,293,199,219,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
262,505,544,125,87,87,599,599,87,87,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
449,449,449,475,475,475,475,475,475,347,347,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT,AFT,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{191,"Troligan_CR",141,1,
55,172,377,59,199,0,789,789,789,789,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
170,503,546,125,565,565,389,389,565,565,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
457,457,457,457,457,457,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,AFT,AFT,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{192,"Morshin_CV",135,1,
55,173,401,59,199,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
591,503,544,125,76,384,76,76,384,384,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
457,457,457,457,457,457,346,346,346,346,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT,AFT,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{193,"Torotha_FF",138,1,
55,173,406,59,199,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
258,503,545,125,77,593,76,76,593,593,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
51,51,457,457,457,457,346,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{194,"Shaveen_PC",63,1,
325,168,393,283,199,0,420,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
175,496,543,125,74,71,74,74,71,71,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
457,457,457,457,801,346,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,AFT|PORT|STARB,AFT|PORT|STARB,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{195,"Rogata_TG",133,1,
55,172,377,59,199,0,420,420,420,420,789,789,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
591,503,545,125,74,74,77,77,74,74,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
475,475,475,475,475,475,475,475,346,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE|AFT|PORT,FORE|AFT|STARB,AFT,AFT,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{196,"Isil'zha_FR",205,1,
325,165,395,283,195,0,420,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
249,500,544,119,597,808,597,597,808,808,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{197,"Chudomo_TF",15,1,
317,162,361,277,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
243,495,539,113,12,10,11,11,10,10,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
451,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{198,"Tishat_FI",126,1,
314,162,361,274,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
243,495,539,113,20,17,19,19,18,18,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
452,452,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{199,"Traga_FI",125,1,
314,162,361,274,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
243,495,539,113,19,16,18,18,17,17,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
451,451,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{200,"Ronati_BP",148,1,
314,162,361,274,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
672,495,539,113,22,20,21,21,21,21,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{201,"Norgath_SB",152,1,
57,0,418,60,204,0,422,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
671,507,550,132,517,517,517,517,456,456,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
510,510,510,510,510,510,510,510,510,511,511,511,511,511,511,511,511,511,511,511,511,347,347,347,347,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,
FORE|AFT|STARB|BOTTOM,AFT|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,FORE|TOP,FORE|BOTTOM,FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,
FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,AFT|TOP,AFT|BOTTOM,AFT|PORT|STARB|TOP,
AFT|PORT|STARB|BOTTOM,FORE|PORT|STARB|TOP,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,AFT|PORT|STARB|BOTTOM},  /*Helm location*/
{202,"Sheganna_OS",151,1,
334,0,364,290,195,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
175,502,541,119,69,69,69,69,69,69,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
737,457,457,457,457,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|TOP|BOTTOM,FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{203,"Evial_DM",237,1,
325,0,416,283,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
175,491,539,113,17,17,17,17,17,17,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
738,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{204,"T'Gan_OS",226,1,
635,0,416,645,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
663,496,540,115,68,68,68,68,68,68,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
442,442,626,626,430,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,FORE|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{205,"T'Khar_SH",124,1,
607,146,360,616,187,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
585,492,539,109,13,10,9,9,9,9,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
437,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{206,"T'Rakh_BP",3,1,
607,136,349,616,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
585,492,539,108,21,19,20,20,20,20,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{207,"Ta'Rakh_BP",3,1,
607,136,349,616,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
585,492,539,108,21,19,20,20,20,20,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
437,437,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{208,"D'Mal_DM",227,1,
630,0,416,281,187,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
719,497,539,109,9,9,9,9,9,9,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
681,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{209,"D'Tath_DM",228,1,
630,0,416,281,187,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
244,497,539,109,10,10,10,10,10,10,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
682,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{210,"D'Kak_DM",229,1,
635,0,416,645,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
248,497,539,109,11,11,11,11,11,11,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
430,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{211,"D'Shal_DM",230,1,
635,0,416,645,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
230,291,539,109,802,802,802,802,802,802,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
680,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{212,"Demos_CL",130,1,
608,154,394,621,198,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
246,535,543,124,71,71,74,74,71,71,0,0,0,0,0,0,0,0,0,0,   /*Eng Sys Reac Batt EDC Comm F A P S D V*/
489,61,61,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT|STARB,FORE|PORT|STARB,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{213,"Scion_BP",88,1,
606,136,348,617,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
586,493,539,108,20,17,18,18,18,18,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{214,"Morell_YA",18,1,
606,142,358,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
667,493,539,112,12,9,5,5,5,5,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
480,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{215,"Larisi_SH",123,1,
606,142,358,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
667,493,539,112,12,9,5,5,5,5,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
480,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{216,"Ekos-A_DM",195,1,
631,0,416,642,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
666,499,539,112,9,9,9,9,9,9,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
711,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{217,"Ekos-B_DM",231,1,
631,0,416,642,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
666,499,539,112,15,15,15,15,15,15,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
711,711,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{218,"Ocara-A_DM",232,1,
634,0,416,644,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
725,535,539,112,11,11,11,11,11,11,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
697,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{219,"Ocara-B_DM",233,1,
637,0,416,535,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
662,535,539,112,17,17,17,17,17,17,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
702,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{220,"Ocara-C_DM",234,1,
634,0,416,644,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
725,535,539,112,20,20,20,20,20,20,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
697,711,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{221,"Ocara-D_DM",235,1,
634,0,416,644,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
725,535,539,112,15,15,15,15,15,15,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
697,697,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{222,"Ocara-E_DM",236,1,
637,0,416,535,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
662,535,539,112,21,21,21,21,21,21,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
702,711,711,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,FORE|AFT|PORT|STARB|TOP|BOTTOM,FORE|AFT|PORT|STARB|TOP|BOTTOM,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{223,"Moas_GS",181,1,
56,169,395,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
248,500,545,123,600,77,85,85,600,600,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
767,767,761,787,787,787,787,791,791,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{224,"Pshul'shi_DR",139,1,
56,140,405,58,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
248,500,546,123,80,78,565,565,78,78,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
777,769,769,770,772,772,772,772,772,772,805,805,805,805,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE,AFT,FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT,FORE|STARB,AFT|PORT,
AFT|STARB,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{225,"Eyehawk_SC",121,1,
727,169,395,728,197,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
147,497,541,123,75,75,78,78,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
760,760,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{226,"Throkan_FL",238,1,
727,387,372,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,496,543,123,74,71,74,74,71,71,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
763,763,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{227,"Xill_BS",56,1,
727,171,397,728,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
233,534,545,123,54,54,54,54,79,79,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
485,485,744,744,745,745,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|TOP,AFT|BOTTOM,
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{228,"Xvell_ES",69,1,
727,164,381,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
654,495,543,123,70,70,70,70,69,69,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
745,745,745,745,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{229,"Xeecra_TP",154,1,
727,177,366,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
170,503,546,123,598,598,598,598,87,87,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
745,745,745,745,745,745,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|TOP,FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{230,"Battleglobe_WS",182,1,
521,171,395,809,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
590,504,545,123,75,75,76,76,76,76,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
783,783,785,785,785,785,785,785,785,785,785,785,784,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,AFT,FORE,FORE,FORE,FORE,FORE,AFT,AFT,AFT,AFT,AFT,FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{231,"Mishakur_DR",192,1,
56,140,405,58,197,212,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
230,500,546,123,596,596,87,87,596,596,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
748,748,748,748,748,748,533,533,145,145,776,776,752,752,752,752,752,752,696,696,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,FORE,AFT,AFT,FORE,FORE,FORE,FORE,FORE|PORT,FORE|STARB,FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT|TOP,
FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,FORE|PORT|STARB,FORE|PORT|STARB,0,0,0,0,0},  /*Helm location*/
{232,"Leskrati_CR",187,1,
727,169,385,728,197,210,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
237,498,545,123,80,80,83,83,80,80,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
754,754,754,533,533,752,752,752,752,752,752,752,752,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,PORT,STARB,FORE,FORE,FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT|TOP,FORE|PORT|BOTTOM,
FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{233,"Garasoch_CVA",188,1,
609,169,405,620,197,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
731,497,545,123,79,79,82,82,79,79,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
754,754,752,752,752,752,752,752,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE|TOP,FORE|BOTTOM,PORT|TOP,STARB|TOP,AFT|BOTTOM,AFT|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{234,"Targath_CS",183,1,
56,169,385,58,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
248,500,545,123,577,577,79,79,577,577,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
748,748,754,754,145,145,752,752,752,752,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,AFT,AFT,FORE|PORT,FORE|STARB,FORE|TOP,FORE|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{235,"Athraskala_HB",184,1,
56,169,385,58,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
245,535,545,197,600,600,80,80,600,600,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
781,753,753,696,696,696,696,790,790,790,790,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|AFT|PORT,FORE|AFT|STARB,FORE,FORE,AFT,AFT,FORE,FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{236,"Abrithi_AC",180,1,
727,153,405,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,497,546,123,76,76,79,79,76,76,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
533,533,775,775,538,538,753,753,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE|AFT|PORT,FORE|AFT|STARB,FORE,FORE,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{237,"Delegor_FF",178,1,
609,387,375,620,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
239,497,542,123,78,577,600,600,78,78,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
753,753,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|TOP,FORE|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{238,"Protra_SC",176,1,
65,387,375,66,197,216,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
663,500,541,123,77,77,600,600,77,77,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
770,776,776,776,776,567,567,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,AFT,AFT,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{239,"Ochlavita_DD",172,1,
727,387,375,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,497,545,123,71,71,77,77,71,71,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
533,533,755,755,749,749,749,749,752,752,790,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,AFT,AFT,AFT|PORT|TOP,AFT|STARB|TOP,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{240,"Tratharti_GS",173,1,
56,166,396,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
230,500,545,123,71,71,77,77,71,71,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
781,747,747,747,747,747,747,533,533,753,753,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT,AFT,FORE,FORE,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{241,"Jashakar_FF",167,1,
727,387,381,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,497,543,123,69,69,69,69,45,45,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
533,533,749,749,775,775,538,538,750,750,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{242,"Orgolest_OS",198,1,
727,0,416,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
234,534,540,123,37,37,37,37,37,37,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
751,751,538,538,753,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{243,"Thorun-I_FI",163,1,
520,386,354,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,14,13,16,16,14,14,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
820,820,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{244,"Thorun-II_FI",163,1,
520,386,354,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,14,13,16,16,14,14,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
756,756,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{245,"Atrimis_CL",171,1,
56,387,385,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
245,535,544,123,384,384,79,79,384,384,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
578,652,652,759,759,759,759,688,688,688,688,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT|TOP,FORE|STARB|TOP,PORT|BOTTOM,STARB|TOP,FORE|PORT,FORE|STARB,AFT|PORT,
AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{246,"Tacomi_PC",168,1,
727,387,381,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
731,497,542,123,69,44,69,69,44,44,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
652,759,759,759,759,759,759,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,PORT,STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{247,"Azafac_FR",165,1,
727,166,395,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
147,497,544,117,69,69,75,75,69,69,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
759,759,759,759,759,759,688,688,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT|PORT,AFT|STARB,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{248,"Cacaras-A_OS",197,1,
56,0,416,58,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
723,503,540,117,34,34,34,34,34,34,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
759,759,759,688,688,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|STARB,FORE|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{249,"Cacaras-B_OS",197,1,
56,0,416,58,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
723,503,540,117,34,34,34,34,34,34,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
580,580,759,759,759,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT|STARB,FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{250,"Rotia_FI",156,1,
520,386,358,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,10,9,10,10,9,9,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
757,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{251,"Seffensa_AC",112,1,
56,156,397,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
248,500,545,123,79,79,54,54,79,79,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
810,810,810,746,746,760,760,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,
FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{252,"Thosalsi_CVA",114,1,
56,156,397,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
251,535,546,123,79,78,87,87,79,79,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
746,746,760,760,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,PORT|BOTTOM,STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{253,"Esthasa_DD",111,1,
56,154,394,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
245,535,545,117,75,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
810,746,746,746,760,760,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,AFT,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{254,"Sussha_FF",106,1,
56,629,404,58,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
245,535,543,123,75,73,75,75,73,73,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
810,810,746,746,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT|STARB,FORE,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{255,"Shasi_FI",157,1,
520,386,358,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,11,10,12,12,10,10,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
757,757,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{256,"?Norsca_BC",245,1,
606,138,350,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
225,493,539,112,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{257,"?Tacacci_FF",246,1,
606,138,350,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
225,493,539,112,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{258,"?Qoricc_DD",247,1,
606,138,350,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
225,493,539,112,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{259,"?Crocti_CVP",248,1,
606,138,350,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
225,493,539,112,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{260,"?Cascor_OS",199,1,
606,138,350,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
225,493,539,112,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{261,"?Calaq_FI",162,1,
606,138,350,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
225,493,539,112,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{262,"?Tiqincc_FI",158,1,
606,138,350,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
225,493,539,112,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{263,"?Caccar_FI",155,1,
606,138,350,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
225,493,539,112,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{264,"Urutha_Kal_DR",194,1,
521,167,405,809,197,220,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
359,504,547,123,598,85,599,599,598,598,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
734,733,733,536,536,536,536,536,536,735,735,735,735,735,735,793,793,793,793,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,
AFT,AFT,FORE|PORT|STARB|TOP,FORE|PORT|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0},  /*Helm location*/
{265,"Irokai_Kam_BC",193,1,
521,167,471,809,197,216,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
357,504,547,123,596,80,811,811,596,596,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
734,769,536,536,735,735,735,735,735,735,793,793,793,793,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT|STARB,AFT|PORT|STARB,FORE|PORT|STARB|TOP,
FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{266,"Tachila_Kor_CVS",191,1,
521,171,385,809,197,216,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
362,504,546,123,83,597,87,87,83,83,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
733,735,735,735,735,735,735,793,793,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT,FORE|STARB,AFT,AFT,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{267,"Alichi_Kav_HS",179,1,
521,176,375,809,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
590,504,545,123,75,75,78,78,75,75,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
53,533,533,533,533,793,793,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{268,"Senchlat_Kes_SC",177,1,
521,176,375,809,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
590,504,545,123,808,593,82,82,593,593,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
733,733,536,536,735,735,735,793,793,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,FORE|PORT|STARB,AFT|PORT,AFT|STARB,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{269,"Okath_Kat_FF",170,1,
727,164,381,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
161,501,544,123,384,74,593,593,74,74,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
536,536,536,536,536,403,403,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT|TOP,FORE|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{270,"Dartha_FI",161,1,
520,163,360,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
721,494,539,111,12,10,11,11,10,10,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
736,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{271,"Dovoch_FI",161,1,
520,163,360,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
721,494,539,111,12,10,11,11,10,10,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
792,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{272,"Mafka_CT",185,1,
727,169,385,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,497,546,123,83,577,565,565,83,83,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
770,770,770,787,787,787,787,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{273,"Shafab_CA",181,1,
727,169,395,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
237,498,545,123,600,77,85,85,600,600,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
763,763,770,770,779,786,786,787,787,787,787,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,FORE,FORE,FORE,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{274,"Martoba_PC",174,1,
727,387,375,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
731,497,541,123,75,74,77,77,74,74,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
763,779,779,772,772,787,787,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{275,"Romak_FF",169,1,
609,387,381,620,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
583,496,542,123,46,69,47,47,69,69,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
787,787,787,787,787,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|TOP,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{276,"Drofta-I_FI",159,1,
520,386,380,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,15,12,14,14,13,13,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
758,758,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{277,"Drofta-II_FI",159,1,
520,386,380,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,15,12,14,14,13,13,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
788,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{278,"Erlorra_CR",240,1,
727,159,400,728,197,210,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
655,534,545,123,81,79,565,811,79,79,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
842,770,770,145,787,830,759,752,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,AFT,FORE|AFT|PORT,FORE|PORT,FORE|AFT|STARB,AFT|PORT|STARB,AFT|PORT,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{279,"Turlisk_CV",241,1,
727,169,405,728,197,210,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
237,496,545,123,80,577,565,565,577,577,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
763,763,787,787,787,787,787,787,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT|STARB,FORE|PORT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,AFT|PORT|STARB,AFT|PORT|STARB,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{280,"Govall_DD",242,1,
56,169,405,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
251,535,545,123,70,70,73,73,70,70,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
787,787,830,815,815,800,712,712,712,712,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|AFT|PORT,FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT,FORE|PORT,FORE,FORE|STARB,FORE|STARB,FORE|STARB,
FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{281,"Allovan_FF",203,1,
56,387,375,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
663,500,543,123,70,47,73,73,47,47,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
747,749,749,750,750,750,750,750,750,750,750,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|STARB,AFT|PORT,AFT|PORT,AFT|STARB,
AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{282,"Terillon_FF",204,1,
727,387,375,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
240,498,543,123,77,593,77,77,593,593,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
763,749,749,787,787,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE|PORT,FORE|PORT,FORE|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{283,"Lellat-A_FI",201,1,
520,386,380,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,15,12,14,14,13,13,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
822,822,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{284,"Lellat-B_FI",201,1,
520,386,380,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,15,12,14,14,13,13,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
825,825,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{285,"Latallia_SH",243,1,
520,146,355,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,12,10,11,11,10,10,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
519,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{286,"Lakara_CR",132,1,
56,169,395,58,197,216,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
241,501,545,123,600,600,81,81,600,600,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
568,568,765,765,765,765,794,794,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE|PORT|TOP,FORE|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,FORE|AFT|PORT|TOP,
FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{287,"Lyata_PC",244,1,
727,149,381,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
715,496,542,123,69,44,69,69,44,44,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
578,765,794,794,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT|STARB,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{288,"Pirocia_SB",200,1,
521,0,366,809,201,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
353,506,550,130,102,102,102,102,602,602,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
571,571,571,571,571,571,571,571,571,571,766,766,766,766,766,766,766,766,766,766,766,794,794,794,794,     /*Helm Sys*/
FORE|PORT|TOP,FORE|PORT|BOTTOM,FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM,
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|PORT|BOTTOM,FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,
AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,
FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|STARB|BOTTOM},    /*Helm location*/
{289,"Avioki_CR",189,1,
65,171,405,66,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
152,500,544,123,600,600,83,83,600,600,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
739,739,739,739,742,742,742,742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{290,"Pikitos_FI",164,1,
520,163,354,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
231,491,539,111,19,16,19,19,19,19,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
812,740,740,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{291,"Alykent_GP",153,1,
65,0,366,66,201,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
590,504,546,130,565,565,565,565,85,85,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
743,743,743,743,743,743,743,743,741,741,741,741,741,741,741,741,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|TOP,FORE|BOTTOM,AFT|TOP,AFT|BOTTOM,PORT|TOP,PORT|BOTTOM,STARB|TOP,STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|BOTTOM,
FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,AFT|PORT|BOTTOM,AFT|STARB|TOP,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{292,"Scorava_CA",186,1,
609,169,385,620,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
147,497,545,123,83,83,83,83,83,83,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
778,778,762,762,762,762,762,762,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,PORT,STARB,AFT,AFT,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{293,"Croscotu_DD",175,1,
609,387,385,620,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
242,496,544,123,75,75,78,78,75,75,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
770,770,770,772,772,774,774,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|STARB,AFT,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{294,"Noscor_FI",160,1,
520,386,380,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,14,12,13,13,13,13,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
780,780,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{295,"Jomic_FF",166,1,
609,387,395,620,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       /*Nav Sys Sen Eng Thr Scan Jam Jump*/
238,496,543,123,44,68,45,45,68,68,0,0,0,0,0,0,0,0,0,0,   /*Eng Sys Reac Batt EDC Comm F A P S D V*/
772,772,772,762,762,759,759,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE,AFT,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{296,"She'dar_FR",141,1,
55,172,377,59,195,219,420,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
175,496,545,119,565,565,389,389,565,565,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
457,457,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT|STARB,AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{297,"Delasi_FR",55,1,
634,156,397,644,194,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
725,501,545,118,79,79,565,565,79,79,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
471,471,471,471,471,471,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{298,"G'Nar_FR",97,1,
635,159,400,645,192,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /*Nav Sys Sen Eng Thr Scan Jam Jump*/
144,499,544,115,596,84,565,565,84,84,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
439,439,439,439,439,690,690,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE,AFT,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{299,"Vorlon_OP",64,1,                     /*ID Name HullNum 1*/
343,0,415,299,208,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
858,532,852,524,859,859,859,859,859,859,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
344,344,344,344,368,368,368,368,368,368,368,368,368,368,368,368,537,537,537,537,537,537,537,537,537,  /*Helm Sys weapons*/
FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,AFT|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|PORT|BOTTOM,
FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP|BOTTOM,
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|STARB|TOP,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|STARB|BOTTOM,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM},     /*Helm location*/
{300,"Artemis-Z_FF",131,1,                     /*ID Name HullNum 1*/
326,166,377,284,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
239,497,543,122,71,71,77,77,71,71,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
443,443,443,443,443,443,481,481,481,481,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT|PORT,AFT|STARB,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,
FORE|AFT|STARB|BOTTOM,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{301,"Hyperion-L_CG",58,1,                     /*ID Name HullNum 1*/
330,155,396,286,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
247,534,545,122,85,82,87,87,84,84,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
481,481,481,481,481,481,481,481,481,813,813,184,184,184,184,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|STARB|TOP,FORE|PORT|STARB|TOP,
FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,
FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{302,"Hyperion-G_AC",58,1,                     /*ID Name HullNum 1*/
330,155,396,286,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
247,534,545,122,85,82,87,87,84,84,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
486,486,771,771,771,481,481,481,183,183,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,FORE|PORT,FORE|STARB,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|BOTTOM,
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{303,"Hyperion-B_CG",58,1,                     /*ID Name HullNum 1*/
330,155,396,286,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
247,534,545,122,85,82,87,87,84,84,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
481,625,625,625,625,625,625,625,183,183,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|PORT|STARB,FORE,FORE,FORE|AFT|PORT,FORE|AFT|STARB,AFT,AFT,FORE|PORT|TOP,
FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{304,"Hyperion-D_CA",58,1,                     /*ID Name HullNum 1*/
330,155,396,286,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
252,535,545,122,85,82,87,87,84,84,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
486,486,445,445,445,445,443,443,443,481,481,481,183,183,183,183,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,FORE,AFT,AFT,FORE|PORT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT|STARB|TOP,
FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{305,"Hyperion-Z_CA",58,1,                     /*ID Name HullNum 1*/
330,155,396,286,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
247,534,545,122,85,82,87,87,84,84,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
782,782,782,782,782,443,443,481,481,481,183,183,183,183,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,
FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{306,"Olympus-B_GS",52,1,                               /*ID Name HullNum 1*/
610,155,377,286,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
715,498,545,122,77,75,78,78,77,77,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
673,673,673,673,763,763,763,763,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|AFT|PORT|STARB,FORE|AFT|PORT|STARB,FORE|PORT,FORE|STARB,FORE|PORT|STARB|TOP,
AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{307,"Omega-G_DD",60,1,                     /*ID Name HullNum 1*/
332,158,399,288,197,216,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
250,500,547,122,87,87,89,89,87,87,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
459,459,459,459,445,445,445,445,482,482,482,482,482,482,482,482,482,482,482,482,184,184,184,184,184,     /*Helm Sys*/
FORE,FORE,AFT,AFT,FORE,FORE,AFT,AFT,FORE|PORT|TOP,FORE|PORT|STARB,AFT|PORT|TOP,AFT|PORT|STARB,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,
FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM,FORE|AFT|STARB|TOP,FORE|AFT|STARB|BOTTOM,FORE|PORT|BOTTOM,FORE|STARB|BOTTOM,
AFT|PORT|STARB|TOP,PORT|TOP|BOTTOM,STARB|TOP|BOTTOM},    /*Helm location*/
{308,"Tethys-T_PC",128,1,                     /*ID Name HullNum 1*/
323,149,371,282,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
67,497,542,122,47,45,47,47,45,45,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
481,813,813,183,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB|TOP,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,FORE|PORT|STARB|BOTTOM,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{309,"Tethys-E_PC",128,1,                     /*ID Name HullNum 1*/
323,149,371,282,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
67,497,542,122,47,45,47,47,45,45,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
531,531,818,818,818,818,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT|STARB,FORE|PORT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{310,"Tethys-I_PC",128,1,                     /*ID Name HullNum 1*/
323,149,371,282,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
67,497,542,122,47,45,47,47,45,45,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
531,531,438,438,438,438,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT|STARB,FORE|PORT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,
FORE|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{311,"Aishinta_CE",140,1,
337,173,401,293,199,219,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
262,505,544,125,87,87,599,599,87,87,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
457,457,457,457,457,457,457,457,457,457,457,347,347,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|STARB,AFT,AFT,AFT|PORT,AFT|STARB,FORE|PORT,FORE|STARB,
0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{312,"Esharan_FF",140,1,
337,173,401,293,199,219,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
262,505,544,125,87,87,599,599,87,87,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
449,801,801,475,475,475,475,475,475,347,347,347,347,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,AFT,AFT,FORE|PORT,FORE|STARB,AFT|PORT,
AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{313,"Shantavi_FF",140,1,
337,173,401,293,199,219,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
262,505,544,125,87,87,599,599,87,87,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
449,449,449,455,455,475,475,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,AFT,AFT,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{314,"Sharaal_WC",62,1,                     /*ID Name HullNum 1*/
55,174,401,59,199,219,422,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
590,505,546,125,90,90,91,91,90,90,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
52,52,52,52,51,51,51,51,51,51,457,457,457,457,457,457,457,457,457,457,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,AFT,AFT,FORE,FORE,FORE,FORE,AFT,AFT,FORE|PORT,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|STARB,AFT|PORT,
AFT|PORT,AFT|STARB,AFT|STARB,0,0,0,0,0},    /*Helm location*/
{315,"Altarian_Magnus_DL",111,1,                                  /*ID Name HullNum 1*/
634,154,394,644,198,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /*Nav Sys Sen Eng Thr Scan Jam Jump*/
663,500,545,124,75,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
702,697,697,697,472,472,472,472,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,AFT,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{316,"Amar_CVF",108,1,                                  /*ID Name HullNum 1*/
637,629,404,291,198,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
663,500,544,124,69,69,71,71,69,69,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
472,472,472,472,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{317,"Balvarix_CVS",114,1,                                 /*ID Name HullNum 1*/
631,156,397,642,198,218,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
148,535,546,124,79,78,87,87,79,79,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
699,699,472,472,472,472,472,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{318,"Darmoti_WE",130,1,
608,154,394,621,198,217,421,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
246,535,543,124,71,71,74,74,71,71,0,0,0,0,0,0,0,0,0,0,   /*Eng Sys Reac Batt EDC Comm F A P S D V*/
263,61,61,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT|STARB,FORE|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{319,"Decurion_AC",112,1,                                /*ID Name HullNum 1*/
335,156,397,291,198,218,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
228,502,545,124,79,79,54,54,79,79,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
461,471,471,471,471,698,698,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{320,"Elutarian_DD",111,1,                                  /*ID Name HullNum 1*/
634,154,394,644,198,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /*Nav Sys Sen Eng Thr Scan Jam Jump*/
663,500,545,124,75,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
471,471,471,471,471,471,64,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT,FORE|STARB,AFT|PORT|STARB,AFT|PORT,AFT|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{321,"Mogratti_FF",106,1,                                  /*ID Name HullNum 1*/
633,629,404,644,198,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
665,499,543,124,75,73,75,75,73,73,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
489,699,699,472,472,472,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|AFT|PORT|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{322,"Sitara_FI",16,1,                                     /*ID Name HullNum 1*/
606,142,358,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
584,494,539,112,14,11,13,13,11,11,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
705,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{323,"Vasachi_DE",110,1,                                   /*ID Name HullNum 1*/
637,154,394,291,198,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
662,501,545,124,75,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
61,472,472,472,472,472,698,698,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{324,"Vorchar_WS",53,1,                                      /*ID Name HullNum 1*/
637,154,394,291,198,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
725,535,543,124,70,70,73,73,70,70,0,0,0,0,0,0,0,0,0,0,   /*Eng Sys Reac Batt EDC Comm F A P S D V*/
471,471,471,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{325,"G'Lan_CA",61,1,                     /*ID Name HullNum 1*/
611,159,400,622,196,213,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
254,501,547,121,89,84,89,89,87,87,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
687,687,677,677,444,444,474,474,474,474,439,439,439,439,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE,FORE,FORE,FORE,AFT,AFT,FORE,FORE,AFT,AFT,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{326,"G'Tal_CC",61,1,                     /*ID Name HullNum 1*/
611,159,400,622,196,213,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
254,501,547,121,89,84,89,89,87,87,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
680,680,680,680,681,681,681,681,682,682,682,682,430,430,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,AFT,FORE,FORE,AFT,AFT,FORE,FORE,AFT,AFT,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{327,"G'Sten_WC",97,1,                                    /*ID Name HullNum 1*/
635,159,400,645,196,213,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
178,499,545,121,596,84,565,565,84,84,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
683,683,683,683,382,439,439,439,439,439,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE,FORE,FORE,AFT,AFT,AFT,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{328,"Rothan_DD",94,1,                                         /*ID Name HullNum 1*/
635,155,402,645,196,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,           /*Nav Sys Sen Eng Thr Scan Jam Jump*/
144,499,544,121,71,71,73,73,71,71,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
488,488,473,473,473,473,838,838,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{329,"Sho'Kov_PC",90,1,                                     /*ID Name HullNum 1*/
630,592,403,281,196,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
657,496,542,121,74,71,72,72,71,71,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
439,439,626,626,626,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{330,"T'Rann_CVA",57,1,                                     /*ID Name HullNum 1*/
318,159,400,278,196,212,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
251,535,546,121,82,82,88,88,82,82,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
683,683,683,683,439,439,439,439,439,439,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE,FORE,AFT,AFT,PORT,STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{331,"Var'Loth_DD",95,1,                                         /*ID Name HullNum 1*/
635,155,402,645,196,214,0,0,0,0,0,0,0,0,0,0,0,0,0,0,         /*Nav Sys Sen Eng Thr Scan Jam Jump*/
144,499,544,121,79,597,87,807,597,597,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
488,837,837,442,442,442,442,626,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT,FORE|PORT,FORE|PORT,FORE|STARB,AFT|PORT|STARB,AFT|PORT|STARB,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{332,"Unarmed_Wiz_Ship",78,1,                     /*ID Name HullNum 1*/
552,553,554,555,556,557,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
559,560,561,562,551,551,551,551,551,551,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{333,"Razarik_FI",102,1,                                    /*ID Name HullNum 1*/
606,142,358,617,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
668,493,539,112,13,10,12,12,10,10,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
706,706,275,275,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Helm Sys weapons*/
FORE,FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{334,"Tarza_FI",89,1,                                      /*ID Name HullNum 1*/
321,146,360,615,187,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
659,491,539,109,14,12,13,13,13,13,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
691,691,518,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{335,"T'Rakk_FF",8,1,                                      /*ID Name HullNum 1*/
635,592,403,645,196,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
722,496,543,121,101,76,86,86,76,76,0,0,0,0,0,0,0,0,0,0,  /*Eng Sys Reac Batt EDC Comm F A P S D V*/
488,488,848,848,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,FORE|PORT|STARB,AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{336,"Bimith_DF",249,1,
56,169,385,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
241,501,545,123,577,577,79,79,577,577,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
765,765,765,765,765,765,794,794,794,794,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|TOP,FORE|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,AFT|TOP,AFT|BOTTOM,FORE|PORT|STARB|TOP,
AFT|PORT|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{337,"Miliani_CV",257,1,
56,169,385,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
241,501,544,123,77,77,79,79,77,77,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
568,765,765,765,765,794,794,794,794,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT|TOP,FORE|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,
AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{338,"Shyarie_FF",250,1,
727,387,375,728,454,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
715,496,543,123,593,74,593,593,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
322,322,322,322,322,794,794,794,794,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{339,"Tiraca_FF",251,1,
56,387,372,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
662,500,543,123,593,74,593,593,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
568,765,765,794,794,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT|TOP,FORE|STARB|TOP,FORE|PORT|TOP,FORE|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{340,"Kotha_FI",252,1,
520,386,380,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
668,492,539,111,15,13,14,14,13,13,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
758,758,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{341,"Skiatha_SC",253,1,
56,169,385,58,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
588,501,546,123,80,597,83,83,80,80,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
765,765,765,765,794,794,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{342,"Alanti_OS",254,1,
56,0,416,58,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
245,535,540,117,44,44,44,44,44,44,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
578,765,794,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB|TOP|BOTTOM,FORE|AFT|PORT|STARB|TOP|BOTTOM,FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{343,"Elira-1_DM",255,1,
727,0,416,728,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
247,498,539,111,10,10,10,10,10,10,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
833,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{344,"Elira-2_DM",256,1,
727,0,416,728,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
247,498,539,111,15,15,15,15,15,15,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
322,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{345,"Brokados_CVB",258,1,
56,171,385,58,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
152,500,546,123,82,600,83,83,600,600,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
816,816,816,816,742,742,742,742,742,742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{346,"Corumai_DR",259,1,
65,167,399,66,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
161,502,547,123,596,82,392,392,82,82,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
739,739,739,739,739,739,739,739,739,742,742,742,742,742,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE,FORE,AFT,AFT,AFT,FORE|PORT|STARB,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{347,"Tashkat_CR",260,1,
65,171,385,66,197,218,3,3,3,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
152,501,546,123,82,600,83,83,600,600,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
739,739,739,739,739,742,742,742,742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,FORE,AFT,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{348,"Halik_FK",261,1,
609,164,372,620,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
4,496,543,123,593,74,593,593,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
742,742,742,742,742,742,742,742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT|STARB,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,AFT|PORT|STARB,AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{349,"Falkosi_FI",262,1,
520,163,358,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
6,492,539,111,13,10,11,11,10,10,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
740,740,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{350,"Ishtaka_TB",263,1,
521,0,366,809,203,0,3,3,3,3,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
353,506,550,130,105,105,105,105,464,464,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
280,280,280,280,280,280,287,287,287,287,287,287,289,289,289,289,289,289,289,289,289,289,289,289,0,     /*Helm Sys*/
FORE|TOP,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,AFT|BOTTOM,FORE|TOP,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,
AFT|STARB|BOTTOM,AFT|BOTTOM,FORE|TOP,FORE|BOTTOM,FORE|PORT|TOP,FORE|PORT|BOTTOM,FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,
AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM,AFT|TOP,AFT|BOTTOM,0},    /*Helm location*/
{351,"Cidikar_CVA",264,1,
56,171,405,58,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
152,534,546,123,84,79,85,85,84,84,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
816,816,742,742,742,742,742,742,742,742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE,FORE,FORE|AFT|PORT,FORE|AFT|PORT,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|STARB,FORE|AFT|STARB,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{352,"Ikorta_AC",265,1,
56,176,375,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
152,496,545,123,70,70,73,73,70,70,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
743,743,742,742,742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{353,"Tokrana_OS",254,1,
56,0,416,58,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
152,495,540,117,44,44,44,44,44,44,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
743,743,741,741,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{354,"Rehsa-G_DM",266,1,
609,0,416,620,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
292,496,539,111,9,9,9,9,9,9,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
741,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{355,"Stormfalcon_CA",267,1,
727,169,377,728,197,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
240,534,544,123,83,83,81,81,81,81,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
729,729,311,311,763,763,730,307,307,829,829,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE,FORE,FORE|PORT,FORE|STARB,FORE,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{356,"Strikehawk_CVB",268,1,
727,169,395,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
315,497,544,123,73,73,577,577,73,73,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
730,730,763,763,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{357,"Warbird_CR",268,1,
727,169,395,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
315,497,544,123,73,73,577,577,73,73,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
763,763,763,763,307,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|PORT|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{358,"Sky_Serpent_FI",269,1,
313,386,355,273,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
668,491,539,188,29,28,28,28,28,28,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
320,821,821,519,519,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{359,"Star_Snake_FI",270,1,
520,386,358,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
231,492,539,111,10,9,10,10,9,9,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
327,327,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{360,"Brostilli_SB",263,1,
521,0,366,809,201,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
256,506,550,130,408,408,408,408,602,602,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
333,333,333,333,329,329,329,329,329,329,329,329,331,331,331,331,307,307,834,834,834,827,827,827,827,     /*Helm Sys*/
FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,FORE|TOP,FORE|BOTTOM,AFT|TOP,AFT|BOTTOM,PORT|TOP,PORT|BOTTOM,STARB|TOP,STARB|BOTTOM,
FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,AFT|PORT|STARB|TOP,AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|STARB|BOTTOM},    /*Helm location*/
{361,"Strikebird_CV",268,1,
727,169,395,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
315,497,544,123,73,73,577,577,73,73,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
763,763,307,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{362,"Stareagle_FF",271,1,
727,387,371,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,495,542,123,69,44,69,69,44,44,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
730,760,760,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{363,"Shodroma_OS",272,1,
56,0,416,58,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
245,535,543,117,70,70,70,70,70,70,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
828,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|AFT|PORT|TOP|BOTTOM,FORE|AFT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{364,"Type-BT_DM",255,1,
609,0,416,620,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,492,539,111,10,10,10,10,10,10,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
833,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{365,"Type-BP_DM",274,1,
609,0,416,620,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,492,539,111,17,17,17,17,17,17,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{366,"Type-BR_DM",273,1,
609,0,416,620,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,492,539,111,15,15,15,15,15,15,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
307,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{367,"Type-FF_SM",274,1,
609,0,416,620,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,492,539,111,17,17,17,17,17,17,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{368,"Tiac_EV",116,1,
56,150,372,58,193,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
245,535,544,117,54,54,596,596,54,54,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
819,819,829,829,829,829,791,791,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{369,"Suom_CV",111,1,
56,154,394,58,197,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
248,500,545,123,75,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
787,787,787,787,829,829,791,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,FORE|PORT,FORE|STARB,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{370,"Kuach_FL",268,1,
727,169,395,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,497,544,123,73,73,577,577,73,73,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
797,760,760,760,760,791,791,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{371,"Koist_FI",275,1,
520,386,380,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,13,10,15,15,13,13,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
825,825,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{372,"Reska_FI",276,1,
520,386,380,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,13,10,11,11,10,10,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
822,822,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{373,"Tracha_TR",277,1,
727,387,385,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,493,544,117,577,577,79,79,577,577,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
787,787,791,791,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB,AFT|PORT|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{374,"Tora_OS",254,1,
727,0,416,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,495,540,117,44,44,44,44,44,44,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
761,787,787,791,791,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB|TOP|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{375,"Resh'kas'u_CVL",279,1,
727,169,405,728,197,210,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
237,498,544,123,600,101,81,81,101,101,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
778,770,770,770,772,772,805,805,805,805,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,AFT,FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{376,"Sim'sall'e_CT",278,1,
727,169,405,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,495,545,117,600,101,79,79,101,101,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
770,770,772,772,805,805,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{377,"Urik'hal_DD",280,1,
727,387,377,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,495,543,123,77,593,77,77,593,593,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
770,770,772,805,805,407,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{378,"Tra'shu'li_LI",281,1,
727,387,396,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
147,497,543,117,593,74,593,593,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
770,772,772,772,805,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,AFT|PORT|STARB,FORE|AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{379,"Por'fa'tis_FI",282,1,
520,146,354,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
582,491,539,111,15,12,14,14,13,13,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
780,780,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{380,"Thar'not'ak_CR",283,1,
727,140,405,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
244,495,544,123,79,808,83,83,79,79,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
769,769,769,769,836,836,805,805,805,805,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,AFT,FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{381,"Ursh'tal'u_SB",284,1,
521,0,366,809,201,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
256,506,540,130,601,601,601,601,408,408,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
840,840,840,840,463,463,463,463,409,409,409,409,409,409,419,419,419,419,805,805,805,805,805,805,805,     /*Helm Sys*/
FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|STARB|BOTTOM,FORE|TOP,FORE|PORT|BOTTOM,
FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|BOTTOM,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|BOTTOM,
FORE|AFT|STARB|TOP,FORE|PORT|TOP,FORE|PORT|BOTTOM,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM},    /*Helm location*/
{382,"Thor'ka_OS",254,1,
56,0,416,58,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
663,496,540,117,44,44,44,44,44,44,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
777,772,772,805,805,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|TOP|BOTTOM,FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{383,"Colt'u_DM",256,1,
609,0,416,620,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,491,539,111,15,15,15,15,15,15,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
805,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{384,"Cort'i_DM",285,1,
609,0,416,620,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
236,491,539,111,14,14,14,14,14,14,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
419,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{385,"Xeel_CVW",286,1,
727,171,394,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
654,534,545,123,577,577,577,577,76,76,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
745,745,745,745,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{386,"Vaarl_SS",287,1,
727,171,394,728,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
654,534,545,123,78,78,78,78,77,77,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
485,487,487,487,487,487,487,487,487,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|TOP,FORE|BOTTOM,FORE|PORT|TOP,FORE|STARB|TOP,AFT|TOP,AFT|BOTTOM,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{387,"Vymish_TR",289,1,
727,171,394,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
654,534,543,117,77,77,77,77,384,384,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
485,487,487,487,487,487,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{388,"Tzymm_FI",290,1,
520,163,354,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
224,494,539,111,508,508,508,508,19,19,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
515,514,514,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{389,"Xonn_DR",291,1,
727,167,401,728,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
152,534,546,123,596,596,596,596,85,85,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
485,485,485,485,485,744,744,745,745,745,745,745,745,745,745,745,745,745,745,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|BOTTOM,
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|TOP,FORE|BOTTOM,FORE|PORT|TOP,FORE|PORT|BOTTOM,FORE|STARB|TOP,FORE|STARB|BOTTOM,
AFT|TOP,AFT|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM,0,0,0,0,0,0},    /*Helm location*/
{390,"Zorth_FI",292,1,
520,163,358,523,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
668,493,539,111,10,9,10,10,9,9,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
514,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{391,"Xoti-A_OS",293,1,
727,0,416,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
292,496,540,117,45,45,45,45,45,45,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
485,745,745,487,487,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB|TOP|BOTTOM,FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{392,"Xoti-B_OS",293,1,
727,0,416,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
292,496,540,117,45,45,45,45,45,45,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
744,745,745,487,487,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|PORT|STARB|TOP|BOTTOM,FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{393,"Class-X_DM",274,1,
609,0,416,620,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
292,493,539,111,17,17,17,17,17,17,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
487,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{394,"Xorr_WS",288,1,
727,171,394,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
654,534,545,123,600,600,600,600,577,577,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
485,485,485,745,745,745,745,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{395,"Hyperion-Raider_CA",58,1,                     /*ID Name HullNum 1*/
330,155,396,286,193,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,      /*Nav Sys Sen Eng Thr Scan Jam Jump*/
252,535,545,122,85,82,87,87,84,84,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
486,486,443,443,443,481,481,481,183,183,183,183,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys weapons*/
FORE,FORE,FORE|PORT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,
FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0},     /*Helm location*/
{396,"Bisaria_FE",251,1,
727,387,372,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
724,499,543,123,593,74,593,593,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
322,765,765,794,794,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE|PORT|TOP,FORE|STARB|TOP,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{397,"Fetula_WT",250,1,
727,387,375,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
715,496,543,123,593,74,593,593,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
322,322,765,765,765,794,794,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT|STARB|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,FORE|AFT|PORT|TOP,FORE|AFT|STARB|TOP,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{398,"Lokita_CV",132,1,
56,169,395,58,197,216,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
241,501,545,123,600,600,81,81,600,600,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
568,568,765,765,765,765,794,794,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE,FORE|PORT|TOP,FORE|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,FORE|AFT|PORT|TOP,
FORE|AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{399,"Marata_TR",257,1,
727,169,385,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
144,499,544,123,77,77,79,79,77,77,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
765,765,765,765,794,794,794,794,794,794,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT|TOP,FORE|STARB|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|BOTTOM,FORE|PORT|STARB|TOP,FORE|PORT|TOP,
FORE|STARB|TOP,AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{400,"Batrado_TR",189,1,
609,171,405,620,193,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
292,496,544,117,600,600,83,83,600,600,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
742,742,742,742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{401,"Brikorta_CVL",265,1,
727,176,375,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
292,496,545,123,70,70,73,73,70,70,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
533,533,742,742,742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{402,"Haltona_FF",261,1,
65,164,372,66,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
152,498,543,123,593,74,593,593,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
739,739,742,742,742,742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE,FORE|PORT|STARB,FORE|PORT|STARB,AFT|PORT|STARB,AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{403,"Kaliva_CR",189,1,
521,171,405,809,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
175,500,544,123,600,600,83,83,600,600,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
844,844,742,742,742,742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{404,"Lykorai_CVA",259,1,
65,167,399,66,197,215,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
152,498,547,123,596,82,392,392,82,82,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
741,741,741,741,741,741,741,741,741,741,741,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT|TOP,FORE|PORT|BOTTOM,FORE|STARB|TOP,FORE|STARB|BOTTOM,FORE|AFT|PORT|TOP,FORE|AFT|PORT|TOP,FORE|AFT|PORT|BOTTOM,FORE|AFT|STARB|TOP,
FORE|AFT|STARB|BOTTOM,FORE|AFT|STARB|BOTTOM,AFT|PORT|STARB|TOP,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{405,"Shakara_CS",260,1,
65,171,385,66,193,215,3,3,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
152,534,546,123,600,76,81,81,76,76,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
742,742,742,742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{406,"Takata_CM",260,1,
65,171,385,66,197,218,845,845,845,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
152,501,546,123,82,600,83,83,600,600,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
846,846,846,846,742,742,742,742,742,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,AFT|PORT|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{407,"Claweagle_FF",271,1,
727,387,371,728,197,847,847,0,0,0,0,0,0,0,0,0,0,0,0,0, /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,491,542,123,69,44,69,69,44,44,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{408,"Darkhawk_CG",121,1,                                  /*ID Name HullNum 1*/
727,169,395,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,495,541,123,75,75,78,78,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
729,760,760,696,696,696,696,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{409,"Fanged_Serpent_FI",294,1,
313,386,355,273,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
668,491,539,188,29,28,28,28,28,28,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
320,821,821,519,519,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    /*Helm location*/
{410,"Guardhawk_BE",121,1,                                  /*ID Name HullNum 1*/
727,169,395,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
237,495,541,123,75,75,78,78,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
307,307,307,307,760,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{411,"Jumphawk_CC",121,1,                                  /*ID Name HullNum 1*/
727,169,395,728,197,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
315,497,541,123,75,75,78,78,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
730,730,763,763,760,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,FORE,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{412,"Nightfalcon_CVA",267,1,
727,169,377,728,197,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
315,497,544,123,565,83,81,81,81,81,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
763,763,763,763,763,729,729,829,829,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{413,"Sleekbird_CA",268,1,
727,169,395,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
147,497,544,123,73,73,577,577,73,73,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
760,760,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{414,"Solarhawk_BC",121,1,                                  /*ID Name HullNum 1*/
727,169,395,728,197,363,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,496,541,123,75,75,78,78,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
311,311,311,311,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{415,"War_Talon_CVE",268,1,
727,169,395,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
315,497,544,123,73,73,577,577,73,73,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
307,307,760,760,760,760,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|PORT,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{416,"Wareagle_FF",271,1,
727,387,371,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
151,495,542,123,69,44,69,69,44,44,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
729,760,760,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT|STARB,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{417,"Geun_DE",137,1,                     /*ID Name HullNum 1*/
56,153,398,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
230,500,543,123,80,79,87,87,80,80,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
458,458,531,531,782,782,443,443,443,443,481,481,183,183,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
791,791,791,791,761,761,787,787,787,787,760,760,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|STARB,AFT,AFT,FORE,FORE,FORE|AFT|PORT,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|STARB,FORE|AFT|PORT|STARB|TOP,
FORE|AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{418,"Mearc_GS",181,1,
56,169,395,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
248,500,545,123,600,77,85,85,600,600,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
791,791,791,761,761,787,787,787,787,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE,FORE,FORE,FORE,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{419,"Moor_DG",111,1,
56,154,394,58,197,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
248,500,545,123,75,75,76,76,75,75,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
791,791,791,791,791,787,787,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE|PORT,FORE|PORT,FORE|STARB,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{420,"Slyach_FF",251,1,
56,387,372,58,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
662,500,543,123,593,74,593,593,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
702,787,787,829,829,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT,FORE|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{421,"Ar'tees_TR",281,1,
727,387,396,728,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
237,496,543,117,593,74,593,593,74,74,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
778,778,805,805,805,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|AFT|PORT|STARB,FORE|PORT|STARB,AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{422,"Sashul'kur_BR",279,1,
727,169,405,728,197,210,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
315,497,544,123,600,101,81,81,101,101,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
778,841,841,841,805,805,805,805,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE,FORE,AFT,FORE|PORT,FORE|STARB,AFT|PORT,AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{423,"Sim'tor'ka_TR",278,1,
56,169,405,58,193,210,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
245,534,545,117,600,101,79,79,101,101,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
772,772,772,772,805,805,805,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|PORT|STARB,FORE|AFT|PORT,FORE|AFT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{424,"Urik'tal_DE",280,1,
727,387,377,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
315,495,543,123,77,593,77,77,593,593,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
778,772,772,805,805,805,805,805,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,FORE|AFT|PORT,FORE|AFT|STARB,FORE|AFT|PORT|STARB,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{425,"Vaarka_SE",287,1,
727,171,394,728,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
654,534,545,123,78,78,78,78,77,77,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
744,487,487,487,487,487,487,487,487,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|AFT|PORT|STARB|BOTTOM,FORE|TOP,FORE|BOTTOM,FORE|PORT|TOP,FORE|STARB|TOP,AFT|TOP,AFT|BOTTOM,
AFT|PORT|BOTTOM,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{426,"Xeon_AS",286,1,
727,171,394,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
654,534,545,123,577,577,577,577,76,76,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
487,487,487,487,487,487,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|TOP,FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|STARB|BOTTOM,AFT|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{427,"Ximm_CE",288,1,
727,171,394,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
654,534,545,123,600,600,600,600,577,577,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
485,487,487,487,487,487,487,487,487,487,487,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|PORT|BOTTOM,FORE|STARB|TOP,
FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,AFT|STARB|TOP,AFT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{428,"Xixx_SG",287,1,
727,171,394,728,197,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
654,534,545,123,78,78,78,78,77,77,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
849,849,849,849,849,849,849,849,745,487,487,487,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE,FORE|PORT,FORE|STARB,AFT,AFT,AFT|PORT,AFT|STARB,FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|TOP,
FORE|AFT|PORT|STARB|BOTTOM,FORE|AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{429,"Xurr_SV",288,1,
727,171,394,728,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
654,534,545,123,600,600,600,600,577,577,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
835,835,745,745,745,745,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|AFT|PORT|STARB|BOTTOM,FORE|PORT|TOP,FORE|STARB|BOTTOM,AFT|PORT|BOTTOM,AFT|STARB|TOP,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{430,"Shadow_RO",64,1,                     /*ID Name HullNum 1*/
343,0,415,299,208,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
858,532,852,524,861,861,861,861,861,861,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
857,857,857,857,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT|STARB|TOP,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,AFT|PORT|STARB|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{431,"Traveler_VE",297,1,                     /*ID Name HullNum 1*/
341,181,411,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
264,529,851,126,860,96,96,96,96,96,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
862,863,863,863,864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT|STARB,FORE|TOP|BOTTOM,FORE|PORT|TOP|BOTTOM,FORE|STARB|TOP|BOTTOM,FORE|AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{432,"Lordship_VE",81,1,                     /*ID Name HullNum 1*/
341,181,414,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
264,529,851,126,27,575,860,860,860,860,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
865,866,866,866,866,866,866,866,866,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE,FORE|PORT|STARB|TOP|BOTTOM,FORE|PORT|STARB|TOP|BOTTOM,FORE|PORT|TOP|BOTTOM,FORE|STARB|TOP|BOTTOM,AFT|PORT|TOP|BOTTOM,AFT|STARB|TOP|BOTTOM,
AFT|PORT|STARB|TOP|BOTTOM,AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{433,"Triumviron_VE",85,1,                     /*ID Name HullNum 1*/
341,181,411,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
264,529,851,126,575,301,27,27,27,27,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
867,867,867,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT|STARB|TOP|BOTTOM,FORE|PORT|STARB|TOP|BOTTOM,FORE|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{434,"Thoughtforce_VE",296,1,                     /*ID Name HullNum 1*/
341,181,411,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
264,529,851,126,96,860,27,27,27,27,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
868,869,869,869,869,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|AFT|PORT|STARB|TOP,FORE|PORT|STARB|TOP|BOTTOM,FORE|AFT|PORT|TOP|BOTTOM,FORE|AFT|STARB|TOP|BOTTOM,AFT|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{435,"Dark_Knife_VE",295,1,                     /*ID Name HullNum 1*/
341,181,410,297,207,221,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump Tractor*/
856,529,851,126,574,574,861,861,861,861,0,0,0,0,0,0,0,0,0,0,    /*Eng Sys Reac Batt EDC Comm F A P S D V*/
870,870,870,871,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
FORE|PORT|STARB|TOP,FORE|PORT|STARB|TOP,FORE|PORT|STARB|BOTTOM,FORE|PORT|STARB|TOP|BOTTOM,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
{436,"Demeter_SR",300,1,                             /*ID Name HullNum 1*/
339,0,418,295,203,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
669,507,550,134,105,105,105,105,105,105,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{437,"Ja'Kal_SR",301,1,                              /*ID Name HullNum 1*/
639,0,418,648,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
669,507,550,134,103,103,103,103,103,103,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{438,"Longor_SR",302,1,                              /*ID Name HullNum 1*/
641,0,417,650,202,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
669,507,550,134,106,106,106,106,106,106,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{439,"Shagenn_SR",303,1,                             /*ID Name HullNum 1*/
57,0,418,60,204,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       /*Nav Sys Sen Eng Thr Scan Jam Jump*/
671,507,550,134,517,517,517,517,517,517,0,0,0,0,0,0,0,0,0,0,     /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{440,"League_SR",304,1,
521,0,366,809,203,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /*Nav Sys Sen Eng Thr Scan Jam Jump*/
256,506,550,134,105,105,105,105,464,464,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  /*Helm location*/
{441,"Warlock-A_DD",60,1,                     /*ID Name HullNum 1*/
332,158,399,288,197,216,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /*Nav Sys Sen Eng Thr Scan Jam Jump*/
250,500,547,122,389,85,599,599,389,389,0,0,0,0,0,0,0,0,0,0, /*Eng Sys Reac Batt EDC Comm F A P S D V*/
448,448,676,676,718,718,718,718,718,718,718,718,564,564,564,438,438,579,579,625,625,184,184,184,184,   /*Helm Sys*/
FORE,FORE,FORE,AFT,FORE|PORT|TOP,FORE|PORT|BOTTOM,FORE|STARB|TOP,FORE|STARB|BOTTOM,AFT|PORT|TOP,AFT|PORT|BOTTOM,
AFT|STARB|TOP,AFT|STARB|BOTTOM,FORE|TOP,FORE|AFT|PORT|STARB|BOTTOM,AFT|TOP,FORE|AFT|PORT|TOP,FORE|AFT|STARB|BOTTOM,
FORE|PORT,FORE|STARB,FORE|PORT,FORE|STARB,FORE|PORT|STARB|BOTTOM,AFT|PORT|STARB|TOP,PORT|TOP|BOTTOM,STARB|TOP|BOTTOM},  /*Helm location*/
{-1,"NADA",0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,            /*Nav Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,            /*Eng Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /*Helm Sys*/
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /*Helm location*/
};

void VMMan(player,ShipNum,console)
     int player,ShipNum;
     char *console;
{
  t_ManEnt *tmp;
  if (VMGlobalSpaceInit == 0) {
    VMnotify(player,"Space has not been initialized.");
    return;
  }
  
  if(ShipNum < 0 || ShipNum > VMCurrentMax) {
    VMnotify(player,"Possible Bug!");
    return;
  }

  if(strncmp(console,"nav",3)==0) {
    VMSetManInternal(VMSpace[ShipNum].VMTemp.TmpNav.VMMan_List,player);
    VMnotifyAll(where_is(player),"%s mans the Navigation console",Name(player));
    return;
  }
  if(strncmp(console,"dam",3)==0) {
    VMSetManInternal(VMSpace[ShipNum].VMTemp.TmpDam.VMMan_List,player);
    VMnotifyAll(where_is(player),"%s mans the Damage Control console",Name(player));
    return;
  }
  if(strncmp(console,"helm",4)==0) {
    VMSetManInternal(VMSpace[ShipNum].VMTemp.TmpHelm.VMMan_List,player);
    VMnotifyAll(where_is(player),"%s mans the Helm console",Name(player));
    return;
  }
  if(strncmp(console,"sci",3)==0) {
    VMSetManInternal(VMSpace[ShipNum].VMTemp.TmpScience.VMMan_List,player);
    VMnotifyAll(where_is(player),"%s mans the Science and Environment console",Name(player));
    return;
  }
  if(strncmp(console,"commu",5)==0) {
    VMSetManInternal(VMSpace[ShipNum].VMTemp.TmpCommu.VMMan_List,player);
    VMnotifyAll(where_is(player),"%s mans the Communications console",Name(player));
    return;
  }
  if(strncmp(console,"eng",3)==0) {

    VMSetManInternal(VMSpace[ShipNum].VMTemp.TmpEng.VMMan_List,player);
    VMnotifyAll(where_is(player),"%s mans the Engineering and Power Control console",Name(player));
    return;
  }
  if(strncmp(console,"oper",4)==0) {
    VMSetManInternal(VMSpace[ShipNum].VMTemp.TmpOps.VMMan_List,player);
    VMnotifyAll(where_is(player),"%s mans the Operations console",Name(player));
    return;
  }
  else 
    VMnotify(player,"I dont see that console here");
}



void VMSetManInternal(list,player)
     t_ManList *list;
     int player;
{
  t_ManEnt *tmp;

  if(IsRoomInManList(list,where_is(player))) {
    tmp=GetCurrManEnt(list);
    tmp->VMRoom=where_is(player);
    tmp->VMPlayer=player;
  }
  else 
    AddManEnt(list,player,where_is(player));
	
  
}
void VMUnMan(int player, int ship, char *ident, int verbose) 
{
  if (VMGlobalSpaceInit == 0) {
    VMnotify(player,"Space has not been initialized.");
    return;
  }
  
  if(ship < 0 || ship > VMCurrentMax) {
    VMnotify(player,"Possible Bug!");
    return;
  }
  if (!strcmp(ident,"ALL")) {
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpHelm.VMMan_List,verbose,"Helm");
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpNav.VMMan_List,verbose,"Navigation");
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpEng.VMMan_List,verbose,"Engineering");
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpDam.VMMan_List,verbose,"Damage Control");
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpCommu.VMMan_List,verbose,"Communications");
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpScience.VMMan_List,verbose,"Science");
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpOps.VMMan_List,verbose,"Operations");
    return;
  }

  if(strncmp(ident,"nav",3)==0) {
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpNav.VMMan_List,verbose,"Navigation");
    return;
  }
  if(strncmp(ident,"dam",3)==0) {
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpDam.VMMan_List,verbose,"Damage Control");
    return;
  }
  if(strncmp(ident,"helm",4)==0) {
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpHelm.VMMan_List,verbose,"Helm");
    return;
  }
  if(strncmp(ident,"sci",3)==0) {
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpScience.VMMan_List,verbose,"Science");
    return;
  }
  if(strncmp(ident,"commu",5)==0) {
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpCommu.VMMan_List,verbose,"Communications");
    return;
  }
  if(strncmp(ident,"eng",3)==0) {
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpEng.VMMan_List,verbose,"Engineering");
    return;
  }
  if(strncmp(ident,"oper",4)==0) {
    VMUnMan2(player,VMSpace[ship].VMTemp.TmpOps.VMMan_List,verbose,"Operations");
    return;
  }
  else
  if (verbose) 
    VMnotify(player,"I dont see that console here");
}


void VMUnMan2(int player, t_ManList *list, int verbose, char *ident) 
{
  char tmpspace[1024];
  if(IsPlayerInManList(list,player)) {
    RemoveManEnt(list);
    if (verbose) {
      /* we have a valid pointer. build the string */
      sprintf(tmpspace,"%s unmans the %s console.",Name(player),ident);
      notify_except(where_is(player),player,player,tmpspace);
      VMnotify(player,"You unman the %s console.",ident);
    }
  }
  else
    VMnotify(player,"You aren't manning the %s console.",ident);
}


void VMInitSpace() {
  int tmp=0,i,mike;
  MasterMineCnt = 0;
  VMGlobalSpaceInit = 1;
  Load_ShipDB();
  MasterMineCnt = 0;

	VMScanPlayers();
  
  /*TEMP!*/
  if(VMCurrentMax < 3) {
    VMCurrentMax=3;
    tmp= NOSHIPDB;
  }
  /*END*/
  
  VMRunSpace=1;
  VMInitTmpShip();
  VMWeaponQ = (t_WList *) InitWList();
  VMDamageQ =(t_WList *) InitWList();
  VMDamageSBQ =(t_WList *) InitWList();
  VMMissleQ = (t_MList *) InitMList();
  VMShipsMoving = (t_SpaceList *) InitSpaceList();
  VMShipsTractoring =(t_SpaceList *) InitSpaceList();
  Commus=InitManList();
  VMShipsPowered = (t_SpaceList *) InitSpaceList();
  
  /*DANGER DANGER TEMPORY!*/
  if(tmp==1) {

  
    /* The NEW way to do it*/
    VMInstallHull(0,0);
    VMInstallHull(1,0);
    VMInstallHull(2,0);
    VMInstallHull(3,0);
    for(i=0;i<=3;i++) {
      /* added by mjw cuz some systems were grabbed that had junk in the flags */
      for (mike = 0; mike < HSystems;mike++) {
	VMSpace[i].VMPerm.Helm.Systems[mike].flags = 0;
      }
      for (mike = 0; mike < NSystems;mike++) {
	VMSpace[i].VMPerm.Nav.Systems[mike].flags = 0;
      }
      for (mike = 0; mike < ESystems;mike++) {
	VMSpace[i].VMPerm.Eng.Systems[mike].flags = 0;
      }
      
      VMInstallNavSys(i,0, 0);
      VMInstallNavSys(i,1, 1);
      VMInstallNavSys(i,2, 3);
      VMInstallNavSys(i,3, 2);
      VMInstallEngSys(i,8,0);
      VMInstallEngSys(i,9,1);
      VMInstallEngSys(i,10,2);
      VMInstallHelmSys(i,4,0,0);
      VMInstallHelmSys(i,5,1,0);
      VMInstallHelmSys(i,6,2,0);
      VMInstallHelmSys(i,7,3,0);
      VMInstallHelmSys(i,12,4,0);
      VMInstallHelmSys(i,12,5,0);
      VMInstallHelmSys(i,12,5,0);
      VMInstallHelmSys(i,13,6,FORE);
      VMInstallHelmSys(i,13,7,FORE|PORT);

      VMInstallEngSys(i,11,4);
      VMInstallEngSys(i,11,5);
      VMInstallEngSys(i,11,6);
      VMInstallEngSys(i,11,7);
      VMInstallEngSys(i,11,8);
      VMInstallEngSys(i,11,9);



    }

    strcpy(VMSpace[1].VMPerm.Name,"White Star");
    strcpy(VMSpace[0].VMPerm.Name,"Babylon 4");
    strcpy(VMSpace[2].VMPerm.Name,"Coventry");
    strcpy(VMSpace[3].VMPerm.Name,"Babylon5");



  }
  /*End tmp*/
  
  VMSensorsInit();
  VMnotify(35,"Init of sensors is finished.");
  VMSetupSpecials(); 
  VMSetupDocks(); 
  
  /*VMLoadSystems();*/
  
  /*do_comsend(select_channel("Space"),"Space Initialized");*/

do_setup_network();
}


void VMInstallNavSys(int VMShipNum,int Sys, int place)
{
int hullt;

  VMSpace[VMShipNum].VMPerm.Nav.Systems[place] = Systems[Sys];
  hullt=VMSpace[VMShipNum].VMPerm.HullType;
  if (checkflag(VMSpace[VMShipNum].VMPerm.Nav.Systems[place].flags,POWERABLE))
    VMSpace[VMShipNum].VMPerm.Nav.powersys++;

/* Check Engine	*/
if(Systems[Sys].indexnum == 1) {
  VMSpace[VMShipNum].VMPerm.Nav.Systems[place].dstat0*=Hulls[hullt].Max_speed;
  VMSpace[VMShipNum].VMPerm.Nav.Systems[place].dstat2*=Hulls[hullt].Accel;
  /*VMSpace[VMShipNum].VMPerm.Nav.Systems[place].dstat3*=Hulls[hullt].EnergyPerSpeed;
  VMSpace[VMShipNum].VMPerm.Nav.Systems[place].dstat8+=Hulls[hullt].Max_Speed_Mod;
*/
	}
/* Check Engine	*/
if(Systems[Sys].indexnum == 2) {
  VMSpace[VMShipNum].VMPerm.Nav.Systems[place].dstat1*=Hulls[hullt].Man_rate;
	}

}

void VMInstallHelmSys(int VMShipNum,
		      int Sys, 
		      int place,
		      int other
		      ) {
  int hullt,i,newplace;
  newplace = -1;
  hullt=VMSpace[VMShipNum].VMPerm.HullType;
  if (Systems[Sys].indexnum == -1) { /* its a gun */
    if(checkflag(VMSpace[VMShipNum].VMPerm.Helm.Systems[place].flags,EXISTS)) {
      for (i = 0; i < HSystems;i++) {
	if(!checkflag(VMSpace[VMShipNum].VMPerm.Helm.Systems[i].flags,EXISTS)){
	  newplace = i;
	  break;
	}
      }
    }
    else {
      newplace = place;
    }
    if (newplace == -1) {
      /* for some reason we couldn't figure out where to put it */
      VMnotify(35,"We have a problem boss...");
      /*wrap_log("Error in placing system in array"); */
      return;
    }
    VMSpace[VMShipNum].VMPerm.Helm.Systems[place] = Systems[Sys];
    VMSpace[VMShipNum].VMPerm.Helm.powersys++;
    VMSpace[VMShipNum].VMPerm.Helm.Systems[place].istat2 = other;

  }
  else {
    if (checkflag(Systems[Sys].flags,INTERCEPTOR)) {
      VMSpace[VMShipNum].VMPerm.Helm.Systems[place] = Systems[Sys];
      VMSpace[VMShipNum].VMPerm.Helm.powersys++;
      VMSpace[VMShipNum].VMPerm.Helm.Systems[place].istat2 = other;
    }
    else { 
      VMSpace[VMShipNum].VMPerm.Helm.Systems[place] = Systems[Sys];
      VMSpace[VMShipNum].VMPerm.Helm.powersys++;
    }
  }

}

void VMInstallHull(int VMShipNum,int VMHull)
{

  VMSpace[VMShipNum].VMPerm.Flags = Hulls[VMHull].Flags;
  VMSpace[VMShipNum].VMPerm.HullType = VMHull;
  VMSpace[VMShipNum].VMPerm.Integrity = Hulls[VMHull].Integrity;
  VMSpace[VMShipNum].VMPerm.mIntegrity = Hulls[VMHull].Integrity;
  VMSpace[VMShipNum].VMPerm.threshold = Hulls[VMHull].Threshold;
} 


void 
VMInstallEngSys(int VMShipNum,
		 int Sys, 
		 int place) {

    VMSpace[VMShipNum].VMPerm.Eng.Systems[place] = Systems[Sys];


}



void Setup_ManLists()
{
  int i;
  
  for(i=0;i<=VMCurrentMax;i++) {
    VMSpace[i].VMTemp.TmpNav.VMMan_List = InitManList();
    VMSpace[i].VMTemp.TmpEng.VMMan_List = InitManList();
    VMSpace[i].VMTemp.TmpCommu.VMMan_List = InitManList();
    VMSpace[i].VMTemp.TmpScience.VMMan_List = InitManList();
    VMSpace[i].VMTemp.TmpHelm.VMMan_List = InitManList();
    VMSpace[i].VMTemp.TmpOps.VMMan_List = InitManList();
    VMSpace[i].VMTemp.TmpDam.VMMan_List = InitManList();
  }
}


void VMSave()
{
  FILE *fptr;
  int i;
  
  if((fptr=fopen("./VMSpace.db","w"))==NULL) return;
 
  for(i=0;i<=VMCurrentMax;i++) 
    fwrite(&VMSpace[i].VMPerm,sizeof(t_Perm),1,fptr);

fclose(fptr);

}




void Load_ShipDB()
{
  FILE *fptr;
  VMCurrentMax=0;
  
  if((fptr=fopen("./VMSpace.db","r"))==NULL) return;
  
  while(!feof(fptr)) {
    fread(&VMSpace[VMCurrentMax].VMPerm,sizeof(t_Perm),1,fptr);
    VMCurrentMax++;

  }
  VMCurrentMax-=2;
fclose(fptr);
}



void VMInitTmpShip()
{
int i,k;

Setup_ManLists();

for(i=0;i<=VMCurrentMax;i++) {

	VMSpace[i].VMTemp.TmpNav.Sector=-1;
	VMSpace[i].VMTemp.TmpEng.PowerHelm = 0;
	VMSpace[i].VMTemp.TmpEng.PowerNav = 0;
	VMSpace[i].VMTemp.TmpEng.PowerAvail = 0;

	VMSpace[i].VMTemp.TmpEng.PowerAvail = 0;
	VMSpace[i].VMTemp.TmpCommu.Relay= -1;
        for(k=0;k<10;k++) {
	VMSpace[i].VMTemp.TmpCommu.cd[k]= 0;

         }

	for(k=0;k<NSystems;k++) {
	VMSpace[i].VMPerm.Nav.Systems[k].tistat0=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tistat1=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tistat2=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tistat3=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tistat4=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tdstat0=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tdstat1=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tdstat2=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tdstat3=0;
	VMSpace[i].VMPerm.Nav.Systems[k].tdstat4=0;
	}
	for(k=0;k<ESystems;k++) {
	VMSpace[i].VMPerm.Eng.Systems[k].tistat0=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tistat1=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tistat2=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tistat3=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tistat4=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tdstat0=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tdstat1=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tdstat2=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tdstat3=0;
	VMSpace[i].VMPerm.Eng.Systems[k].tdstat4=0;
/*
	VMSpace[i].VMPerm.Helm.Systems[k].tistat0=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tistat1=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tistat2=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tistat3=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tistat4=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tdstat0=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tdstat1=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tdstat2=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tdstat3=0;
	VMSpace[i].VMPerm.Helm.Systems[k].tdstat4=0;
*/
	}

	VMSpace[i].VMTemp.TmpNav.VMDockList= (t_SpaceList *) InitSpaceList();
	VMSpace[i].VMTemp.TmpNav.VMRelayList=(t_RelayList *) InitRelayList();
	VMSpace[i].VMTemp.TmpNav.relays=0;
	VMSpace[i].VMTemp.TmpNav.VMShortShips=(t_SpaceList *) InitSpaceList();
/*	VMSpace[i].VMTemp.TmpNav.VMLongShips=(t_SpaceList *) InitSpaceList();
*/

  }


}


/*
void Load_Templates()
{
  FILE *fptr;
  VMCurrentTMax=0;
  
  if((fptr=fopen("./VMTemplate.db","r"))==NULL) return;
  
  while(!feof(fptr)) {
    fread(&VMTemplate[VMCurrentTMax].VMPerm,sizeof(t_Perm),1,fptr);
    VMCurrentTMax++;
  }
  VMCurrentTMax-=2;
fclose(fptr);
}

*/



void VMSetupDocks()
{

int i,ship;


for(i=0 ; i<mudstate.db_top;i++) {

  if((Flags3(i) & RELAY) && isRoom(i)) {
	/*VMnotify(2,tprintf("Adding relay %d",i));*/
	VMAddRelay(i);
	}

  if((Flags2(i) & SHIPIN) && isRoom(i)) 
	VMAddDock(i);
  else if((Flags2(i) & SHIPEXIT) && isExit(i)) {
 	ship=VMShipFromRoom(i);
	VMSpace[ship].VMTemp.TmpNav.airlockout=i;
	}			
  else if((Flags2(i) & SHIPIN) && isExit(i)) {
 	ship=VMShipFromRoom(i);
	VMSpace[ship].VMTemp.TmpNav.airlockin=i;
	}			


  } 
}

int VMShipFromRoom(room)
	int room;
{
 return(atoi(vget_a(Zone(room),SHIP_A)));
}


void VMAddRelay(int room)
{

int ShipNum;


ShipNum=VMShipFromRoom(room);

VMnotify(2,tprintf("Adding relay to room %d, ship %d",room,ShipNum));

if(ShipNum < 0 || ShipNum > VMCurrentMax) return;

if(!IsShipInRelayList(VMSpace[ShipNum].VMTemp.TmpNav.VMRelayList,room))
	{
	VMSpace[ShipNum].VMTemp.TmpNav.relays++;
	AddRelayEnt(VMSpace[ShipNum].VMTemp.TmpNav.VMRelayList,ShipNum,room,VMSpace[ShipNum].VMTemp.TmpNav.relays);

	}


}

void VMAddDock(int room)
{
int ShipNum;

ShipNum=VMShipFromRoom(room);

if(ShipNum < 0 || ShipNum > VMCurrentMax) return;


if(!IsShipInSpaceList(VMSpace[ShipNum].VMTemp.TmpNav.VMDockList,room))
	AddSpaceEnt(VMSpace[ShipNum].VMTemp.TmpNav.VMDockList,room);

}

void
VMSetMainBridge(int player,
		int ship, 
		int mb
		) {
  VMSpace[ship].VMPerm.mainbridge = mb;
  VMnotify(player,"Mainbridge of ship %d set to room %d.",ship,mb);
}


void VMRehelm(int player)
{

int i,k;
for(i=1;i<VMCurrentMax;i++) {
notify(player,tprintf("Rehelming ship %d",i));

	for(k=0;k<ESystems;k++) {
		if(VMSpace[i].VMPerm.Eng.Systems[k].sysnum > 0 || checkflag(VMSpace[i].VMPerm.Eng.Systems[k].flags,EXISTS)) {
    			VMSpace[i].VMPerm.Eng.Systems[k] = Systems[VMSpace[i].VMPerm.Eng.Systems[k].sysnum];
			}
		}
	for(k=0;k<NSystems;k++) {
		if(VMSpace[i].VMPerm.Nav.Systems[k].sysnum > 0 || checkflag(VMSpace[i].VMPerm.Nav.Systems[k].flags,EXISTS)) {
    			VMSpace[i].VMPerm.Nav.Systems[k] = Systems[VMSpace[i].VMPerm.Nav.Systems[k].sysnum];
			}
		}
	for(k=0;k<HSystems;k++) {
		if(VMSpace[i].VMPerm.Helm.Systems[k].sysnum > 0 || checkflag(VMSpace[i].VMPerm.Helm.Systems[k].flags,EXISTS)) {
    			VMSpace[i].VMPerm.Helm.Systems[k] = Systems[VMSpace[i].VMPerm.Helm.Systems[k].sysnum];
			}
		}



	}

}

