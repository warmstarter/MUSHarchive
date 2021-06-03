/*
  Glue.h

  Header for special command rooms...

  2.5.93- rdm created
*/
#define POW_SECURITY 1

/* Parameter to the save/load function */
#define VERIFY 0
#define SAVE 1
#define LOAD 2

#define SPECIAL_FREE 0
#define SPECIAL_ALLOC 1

struct CommandsStruct {
  char *name;
  char len;
  char *helpmsg;
  void (*func)();
};
typedef struct CommandsStruct CommandsStruct;


struct SpecialObjectInstance {
  dbref key;
  int type;
  void *data;
  struct SpecialObjectInstance *next;
};

struct SpecialObjectStruct {
  char *type;                      /* Type of the object */
  CommandsStruct *commands;        /* Commands array */
  long datasize;                   /* Size of private buffer */
  void (*allocfreefunc) ();
  int updateTime;                  /* Amount of time between updates */
				   /* (secs) */
  int timeSinceLastUpdate;         /* Used internally to know when to */
				   /* update */
  void (*updatefunc) ();           /* called for every */
				   /* object at every */
				   /* update */
  int power_needed;		   /* WHat power is needed to do */
				   /* restricted commands */
};

/* If you want to add another command special object, you add it as */
/* well as it's supporting functions to this array. Also include the */
/* prototypes for the functions BEFORE you include glue.h */


CommandsStruct vmxsecurity[]={
  {"MAN",3,"Tests a Sec",do_VMSecMan},
  {"STAT",4,"Simple Stats",do_VMSecRStat},
  {"LOGIN",5,"Logs into the system",do_VMSecLogin},
  {"LOGOUT",6,"Logs out",do_VMSecLogout},
  {"STEST",5,"Tests a Sec",do_VMSecTest},
  {"SADD",4,"Add to the file",do_VMSecAdd},
  {"SVIEW",5,"View the File",do_VMSecView},
  {NULL,   0 , NULL , NULL}
};


CommandsStruct vmspacee[]={

  {"VMINIT",3,"Inits Space",VMInitSpace},
  {"SAVE",4,"Saves Space",do_VMSave},
  {"VMSPEC",6,"Recalcs Specials",do_VMSetupSpecials},
  {"VMFLAG",6,"Sets Flags on a Ship",do_VMFlag},
  {"VMSEE",5,"See The Flags on a Ship",do_VMSeeFlag},
  {"VMSETMB",7,"Sets Mainbridge on a ship",do_VMSetMainBridge},
  {NULL,   0 , NULL , NULL}
};
CommandsStruct vmroom[]={
  {"DOCK",4,"Docks",do_VMDock},
  {"UNDOCK",6,"Undocks",do_VMUnDock},
  {"BAYS",4,"Shows Docking Bays",do_VMNavBays},
  {"MAN",3,"mans a console",do_VMMan},
  {"ENGAGE",6,"engage engines",do_VMEngage},
  {"DISENGAGE",4,"disengage engines",do_VMDisEngage},
  {"SPEED",5,"sets speed",do_VMSetSpeed},
  {"SINFO",5,"Mike's check ship function. very spammy",do_VMDoRep},	
  {"DCS",3,"Damage Control status",do_VMDamStat},
  {"SCAN",4,"Scans around",do_VMScanShip},
  {"SEN",4,"Scans around",do_VMScan},
  {"SH",2,"sets heading",do_VMSetHeading},
  {"NS",2,"nav status",do_VMNavStatus},
  {"HS",2,"helm status",do_VMHelmStatus},
  {"AS",2,"armor status",do_VMArmorStatus},
  {"MS",2,"armor status",do_VMMissleStatus},
  {"TAL",3,"Sets <tube> to autoload <missile>",do_VMAutoloadTube},
  {"INT",3,"Sets <tube> to autoload <missile>",do_VMInterceptorState},
{"ARM",3,"arm <tube>",do_VMArmMissle},
  {"TUBE",4,"tube <missletube> <open|close>",do_VMMissleDoor},
  {"LOAD",4,"load <missletube> <misslenumber>",do_VMLoadMissle},
  {"DUMP",4,"load <missletube> <misslenumber>",do_VMDumpMissle},
  {"UNLOAD",6,"unload <missletube> <misslenumber>",do_VMUnloadMissle},
  {"ADD",3,"THIS COMMAND CAN CRASH US!!!!!!!!!!! add <ship> <miss>",do_VMAddMissle},
  {"DCS",3,"Damage Control status",do_VMDamStat},
  {"LOCK",4,"Lock <weapon> <target>",do_VMHelmLockWeapon},
  {"FIRE",4,"Fire <weapon>",do_VMHelmFireWeapon},
  {"ES",2,"eng status",do_VMEngStatus},
  {"START",5,"Starts reactor",do_VMEngStartReactor},
  {"STOP",4,"Stops reactor",do_VMEngStopReactor},
  {"EA",2,"eng allocate",do_VMEngSetAllocs},
  {"NA",2,"nav allocate",do_VMNavAlloc},
  {"HA",2,"helm allocate",do_VMHelmAlloc},
  {"GUN",3,"Changes State of weapon",do_VMHelmWeaponState},
  {"REPAIR",6,"repairs a system",do_VMMiscRepairSystem},
  {"VERBOSE",7,"Makes repair updates get to you",do_VMSetDamVerbose},
  {"GUNPORT",7,"Opens and close gunports if applicable",do_VMHelmGunPort},
  /*{"HA",2,"helm allocate",do_VMHelmAlloc},*/
  {"SR",2,"sets reactor",do_VMEngSetReactor},
  {"BATTERIES",4,"Turns batteries on or off",do_VMEngBatts},
  {"SAFETIES",8,"Turns engine safeties on or off",do_VMSafeties},
  {NULL,   0 , NULL , NULL}
};
CommandsStruct cargoroom[]={
{"ELOAD",5,"Load Commodity",econload},
{"EUNLOAD",7,"UnLoad Commodity",econunload},
{"LCARGO",6,"Lists Commodities",econcommod},
{NULL,   0 , NULL , NULL}
};

CommandsStruct traderoom[]={
{"EBUY",4,"Buy Commodity",ebuy},
{"SUPPLY",6,"View Money Supply Available",do_supply},
{"LCARGO",6,"Lists Commodities and prices",econcommod_pr},
{NULL,   0 , NULL , NULL}
};

CommandsStruct bank[]={
{"BALANCE",7,"See your balance",do_balance},
{"DEPOSIT",7,"Deposit Money",do_deposit},
{"WITHDRAW",8,"Withdraw Money",do_withdraw},
{"ACCESS",6,"Accesses a bank account",do_access},
{NULL,   0 , NULL , NULL}
};

CommandsStruct factoree[]={
{"STATUS",6,"Factory Status",factory_status},
{"BUILD",5,"Set Plan to Build",factory_build},
{"PSET",4,"Sets the productivity factor",factory_pset},
{NULL,   0 , NULL , NULL}
};

struct SpecialObjectStruct SpecialObjects[]={
{"VMSECURITY", vmxsecurity,0,newfreespace,0,0,NULL,POW_SECURITY},
{"VMSPACE", vmspacee,0,newfreespace,1,0,VMUpdate,POW_SECURITY},
{"VMROOM", vmroom,0,newfreespace,0,0,NULL,POW_SECURITY},
/*{"FACTORY", factoree,0,newfreespace,20,0,factory_update,POW_SECURITY}*/
{"FACTORY", factoree,0,newfreespace,20,0,factory_update,POW_SECURITY},
{"CARGOROOM", cargoroom,0,newfreespace,0,0,NULL,POW_SECURITY},
{"BANK", bank,0,newfreespace,0,0,NULL,POW_SECURITY},
{"TRADEROOM", traderoom,0,newfreespace,0,0,NULL,POW_SECURITY}
};

#define NUM_SPECIAL_OBJECTS \
   ((sizeof(SpecialObjects))/(sizeof(struct SpecialObjectStruct)))

#define NUM_SOI_ENTRIES 1000
struct SpecialObjectInstance *SOI_Table[NUM_SOI_ENTRIES];
