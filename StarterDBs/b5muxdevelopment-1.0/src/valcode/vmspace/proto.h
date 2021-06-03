/* $Id: proto.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

#ifndef _PROTO_H_
#define _PROTO_H_
/* vmspacelist.c */

t_SpaceList *initspacelist(void);
void killSpacelist(t_SpaceList);
void AddSpaceEnt(t_SpaceList *,int);
void RemoveSpaceEnt(t_SpaceList *);
void AdvanceSpaceList(t_SpaceList *);
void ResetSpaceList(t_SpaceList *);
t_SpaceEnt *GetCurrSpaceEnt(t_SpaceList *);
int AtEndSpaceList(t_SpaceList *);
int GetSpaceListSize(t_SpaceList *);
int IsShipInSpaceList(t_SpaceList *, int);

/* vmmanlist.c */

t_ManList *InitManList(void);
void KillManList(t_ManList *);
void AddManEnt(t_ManList *,int,int);
void RemoveManEnt(t_ManList *);
void AdvanceManList(t_ManList *);
void ResetManList(t_ManList *);
t_ManEnt *GetCurrManEnt(t_ManList *);
int AtEndManList(t_ManList *);
int GetManListSize(t_ManList *);
int IsRoomInManList(t_ManList *,int);
int IsPlayerInManList(t_ManList *,int);

/* vminterface.c */
void do_VMSave();
int VMShipNum(int);
void do_VMMan(int,void *,char *);
void do_VMEngage(int,void *,char *);
void do_VMSetSpeed(int,void *,char *);
void do_VMSetHeading(int,void *,char *);
void do_VMNavStatus(int,void *,char *);
void do_VMEngStatus(int,void *,char *);
void do_VMEngStartReactor(int,void *,char *);
void do_VMEngStopReactor(int,void *,char *);
void do_VMEngSetAllocs(int,void *,char *);
void do_VMEngSetReactor(int,void *,char *);
void do_VMDock();
void do_VMUnDock();
void do_VMSecMan();
void do_VMSecLogin();
void do_VMSecRStat();
void do_VMSecLogout();
void do_VMSecTest();
void do_VMSecView();
void do_VMSecAdd();
void do_VMSetupSpecial();
void do_VMSeeFlag();
void do_VMFlag();
void do_VMDisEngage();

/*Security*/
void VMSecMan();
void VMSecLogin();
void VMSecRStat();
void VMSecLogout();
void VMSecTest();
void VMSecView();
void VMSecAdd();
int VMVerCurrent();
int VMCanAccessSecurity();
int VMSecGetVerify();
int VMSecVerify();
void VMClearCurrent();



/*Sensors*/
void VMSetupSpecials();
void VMSensorsInit();
void VMSetSector();
void VMUpdateSensors();
void VMUpdateShipsSensors();
void VMCheckScan();
int VMCheckShortRange();
int VMCheckLongRange();
void VMUpdateSpecials();
void VMCheckSpecial();
void VMWipeOneShipsContacts();
void VMWipeShipContacts();
int HeadingMyWay();


/*Flags*/
void VMSeeFlag();
void VMFlag();




/* vmeng.c */
int CanAccessEng(int,int);
void VMEngStatus(int ,int );
void VMEngStartReactor(int ,int );
void VMEngStopReactor(int ,int );
void VMEngSetAllocs(int ,int ,double ,double ,double, double ,double);
void VMEngReallocPower(int );
void VMEngSetReactor(int ,int ,int );
void VMEngUpdateEngPower(int );

int VMEngCheckFuel();
void VMEngSafety();
void VMEngBattsOff();
void VMEngUpdateBattsPower();
void VMEngBattsOn();

/*vmpower.c*/
void VMUpdateEnergyOne();
void VMPowerUpdateEngPower();
int VMPowerUpdateStress();
double calc_stress();
void VMPowerCheckFuel();



/* vmnav.c */
void VMDisEngage();
int VMDisEngageInternal();
void VMDock();
void VMUnDock();
void VMNavShowBays();
void VMNavBays();
int VMDoorsOpen();
int VMHowManyLeft();
int VMAddToBay();
void VMSubToBay();
int VMCanDoors();
void VMScan();
void VMShowContacts();
void VMAssignArc();
int CanAccessNav(int ,int );
void VMEngage(int ,int );
void VMSetSpeed(int ,int ,double );
void VMSetHeading(int ,int ,double ,double ,double );
void VMNavStatus(int ,int );
/* vmutils.c */
t_xyz VMsph_to_xyz( t_sph  );
t_sph VMxyz_to_sph( t_xyz  );
double VMdistance( t_xyz , t_xyz  );
#ifdef STDC_HEADERS
void VMnotifyAll( dbref , char *, ... );
void VMnotify( dbref , char *, ... );
void VMnotifymans( t_ManList *, char *, ... );
void VMlog_space( char *, ... );
#else
void VDECL(VMnotifyAll, ( dbref , char *, ... ));
void VDECL(VMnotify, ( dbref , char *, ... ));
void VDECL(VMnotifymans,( t_ManList *, char *, ... ));
void VDECL(VMlog_space,( char *, ... ));
#endif
int VMGetArc();
void VMnotifyOfContact();
t_sph VMRelative();
t_sph VMRelative2();
t_sph VMRelative3();


/* vmupdate.c */ 
void VMUpdate_Movement(void);
void VMMove_Ship(int );
void VMUpdateManeuver(int );
void VMManNeedUpdate(double *,double ,double );
void VMUnNormalize(double * ,double ,double );
void VMNormalize(double * ,double ,double );

void VMUpdate_Energy();


/* vmspace.c */

void VMSave();
void VMMan(int ,int ,char *);

void VMSetManInternal(t_ManList *,int);

void VMInitSpace(void);
void VMInitTmpShip(void);
void Setup_ManLists(void);
void Load_ShipDB(void);
void Load_Templates(void);
void VMLoadSystems();
void VMInitTmpShip();
void VMSetupDocks();
int VMShipFromRoom();
void VMAddDock();

int VMGetResolution();


#endif
