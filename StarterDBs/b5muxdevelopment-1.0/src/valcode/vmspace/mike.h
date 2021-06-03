/* vmdamrep.c */
void VMDamageHull();
void VMCheckShip();
int VMGun2SysNum();
void VMProcessDamageQueue();
void VMRepairSystem();
void VMRepairHull();

/* vmhelm.c */

void VMHelmGunPort();
void VMHelmLockWeapon();
void VMHelmFireWeapon();
void VMHelmStatus();
void VMHelmAlloc();
void VMArmorStatus();
int VMMissleDoor();
void VMLoadMissle();
void VMMissleStatus(); 
void VMHelmReallocPower();
void VMArmMissle();
void VMDumpMissle();
void VMUnloadMissle();
void VMAddMissle();
void VMWeaponChangeState();

/* vminterface.c */
void VMRepOne();

/* vmmisc.c */

void VMMiscRepair();
void VMMiscRepairHull();
void VMSetDamVerbose();
void VMGetShipOutDQ();
void VMSpaceNotify(int, char *,...); 
void VMDamStat(); 

/* vmnav.c */

void VMScanShip();
void VMAssignArcString();
void VMNavAlloc();

/* vmpower.c */
void VMPowerUpdateHelm();

/* vmspace.c */

void VMInstallNavSys();
void VMInstallHelmSys();
void VMInstallHull();
void  VMInstallEngSys();
void VMSetMainBridge();

/* vmweaponq.c */

void VMProcessWeaponQueue();
void ProcessEvent();
void VMProcessMissleQueue();
void ProcessMEvent();

/* vmwlist.c */

void RemoveWEnt();
void ResetWList();

/*vmmlist.c */
void RemoveMEnt();
void ResetMList();

void RemoveRoomEnt();
void ResetRoomList();
void VMInitTmpShip2();
void do_make_ship();
void VMFireMissleTube();
void VMFireBeamWeapon();
void VMSciScanShip();

t_RoomEnt *GetRoomEnt();
void VMCheckTmpSpecial();
void VMInternalJumpShip();
t_ADSList *InitADSList();
void KillADSList();
void AddADSEnt();
void RemoveADSEnt();
void AdvanceADSList();
void ResetADSList();
t_ADSEnt *GetCurrADSEnt();
int AtEndADSList();
int GetADSListSize();
void DumpADSList();
int DumpADSList2Disk();
void see_money();
