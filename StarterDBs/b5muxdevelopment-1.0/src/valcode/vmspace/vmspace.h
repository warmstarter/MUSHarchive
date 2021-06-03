/* $Id: vmspace.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

/* header file for monster */

/* structures */

#ifndef PI
#define PI 3.14159
#endif
#define MAXSHIP 1000
#define MAXTEMPLATE 441
#define NSystems 10
#define ESystems 10
#define HSystems 50
#define SPACEROOM 36
#define MAXHULLS 100
#define MAXSYSTEMS 600
#define NMISSLES 128
#define MIKE 35
#define FORE 1
#define AFT 2
#define PORT 4
#define STARB 8
#define TOP 16
#define BOTTOM 32
#define  checkflag(spacevar,flag)  (spacevar & flag)
#define VMMINECONST 256.0



#define ShipActive(x) !(checkflag(VMSpace[x].VMPerm.Flags,DEAD) ||  checkflag(VMSpace[x].VMPerm.Flags,DOCKED))

#include "vmdefs.h"
#include "vmwlist.h"
#include "vmweaponstuff.h"
#include "vmmissle.h"
#include "vmsystems.h"
#include "vmnav.h"
#include "vmspacelist.h"
#include "vmminelist.h"
#include "vmsensors.h"
#include "vmeng.h"
#include "vmhelm.h"
#include "vmmlist.h"

/*
#include "vmmisc.h"
*/
#include "vmmanlist.h"
#include "vmspacelist.h"
#include "vmroomlist.h"
#include "vmtemp.h"
#include "vmcomm.h"

struct s_JPT
{
t_xyz XYZ;
int timeleft;
int sector;
};

typedef struct s_JPT t_JPT;
extern t_JPT JPT[];
extern int JPTMAX;

struct s_Temp
{
  t_RoomList *rooms;
  t_TmpNav TmpNav;
  t_TmpEng TmpEng;
  t_TmpHelm TmpHelm;
  t_TmpCommu TmpCommu;
  t_TmpScience TmpScience;
  t_TmpOps TmpOps;
  t_TmpDam TmpDam;
};

struct s_Perm
{

  char Name[50];
  char Class[50];
  char SOwner[50];
  int HullType;
  int Flags;
  double Integrity;
  double mIntegrity;
  int threshold;
  int mainbridge;
  t_Nav Nav;
  t_Eng Eng;
  t_Helm Helm;
  t_Comm Comm;
};


typedef struct s_Temp t_Temp;
typedef struct s_Perm t_Perm;

struct s_VMSpace
{
  t_Temp VMTemp;
  t_Perm VMPerm;
};

struct s_VMTemplate
{
  int num;
  char name[50];
  int hull;
  int ship;
  int nav[20];
  int eng[20];
  int helm[25];
  int hloc[25];
  /*t_Perm VMPerm;*/
};



typedef struct s_VMSpace t_VMSpace;
typedef struct s_VMTemplate t_VMTemplate;


t_VMSpace VMSpace[MAXSHIP];
extern t_VMTemplate VMTemplate[];
/*t_VMTemplate VMTemplate[MAXTEMPLATE];*/
/*
t_VMSpace VMSpace[100];
t_VMTemplate VMTemplate[50];
*/
extern t_SpaceList *VMShipsMoving;
extern t_SpaceList *VMAllPlayers;
extern t_SpaceList *VMShipsTractoring;

extern t_WList *VMWeaponQ;
extern t_WList *VMDamageSBQ;
extern t_MList *VMMissleQ;
extern t_ManList *Commus;
extern t_WList *VMDamageQ;
extern t_SpaceList *VMShipsPowered;

extern int VMCurrentMax;
extern int VMCurrentTMax;
extern int VMGlobalSpaceInit;
extern int VMCounter;
extern int VMRunSpace;



#include "./proto.h"


extern int MaxTemps; 
/* Weapon Types for notify */

#define BEAM 1
#define MISSILE 2

/* area types for SCheck */
#define HELM 0 
#define NAV 1
#define ENG 2
#define MISC 3





