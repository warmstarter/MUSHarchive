/* $Id: vmtemp.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

#include "../vmrelaylist.h"
#include "vmadslist.h"
struct s_TmpNav
{
  int relays;
  t_SpaceList *VMDockList;
  t_RelayList *VMRelayList;
  t_SpaceList *VMShortShips;
  t_SpaceList *VMLongShips;
  t_ADSList *ADS;
  t_ManList *VMMan_List;
  t_xyz XYZCoords;
  double visibility;
  int APON;
  int Sector;
  int airlockin;
  int airlockout;
  double CurShortRange;
  double CurLongRange;
  int PowerUsed;
};

typedef struct s_TmpNav t_TmpNav;

struct s_TmpEng 
{
  t_ManList *VMMan_List;
  int PowerAvail;
  int PowerNav;
  int PowerHelm;
  int PowerComm;
  int TotalUsed;
};

typedef struct s_TmpEng t_TmpEng;

struct s_TmpHelm
{
  t_ManList *VMMan_List;
  t_MineList *VMMines;
  int PowerGuns;
  int PowerMiss;
  int PowerUsed;

  int ADSOn;
  int ADSTarget; 
};

typedef struct s_TmpHelm t_TmpHelm;

struct s_TmpScience
{
  t_ManList *VMMan_List;
};
typedef struct s_TmpScience t_TmpScience;

struct s_TmpCommu
{
  t_ManList *VMMan_List;
  char code[50][10];
  int cd[10];
  float freqs[10];
  int Relay;
  int RShip;
  t_RelayEnt *tRelay;
   
};

typedef struct s_TmpCommu t_TmpCommu;

struct s_TmpOps
{
  t_ManList *VMMan_List;
};

typedef struct s_TmpOps t_TmpOps;

struct s_TmpDam
{
  t_ManList *VMMan_List;
};

typedef struct s_TmpDam t_TmpDam;

