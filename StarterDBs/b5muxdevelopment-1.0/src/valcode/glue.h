/*  Glue.h

  Header for special command rooms...

  2.5.93- rdm created
*/
#define POW_SECURITY 1
#define POW_MAP 1
#define POW_MECH 1
#define POW_MECHREP 1

#define P(x) 
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

CommandsStruct vmads[]={
  {"ADSINIT",7,"Init's automatic defense system master list",do_VMAdsInit},
  {"ADSLOAD",7,"Loads the automatic defense system database",do_VMAdsLoad},
  {"ADSSAVE",7,"Saves the automatic defense system database",do_VMAdsSave},
  {"ADSSTOP",7,"Stops the automatic defense system",do_VMAdsStop},
  {"ADSSTART",8,"Starts the automatic defense system",do_VMAdsStart},
  {NULL,   0 , NULL , NULL}
};

CommandsStruct vmadsroom[]={
  {NULL,   0 , NULL , NULL}
};

CommandsStruct vmspacee[]={

  {"SECT",4,"Sets the sector of a ship",do_VMSect},
  {"HELM",4,"Helm a ship",do_VMHelm},
  {"SETUP",5,"Initialize relasy",setup_network},
  {"UPDATE",6,"Update relay information",calc_network},
  {"VMOINS",6,"SPACE: SHIPNUM ORBITEENUM RANGE INCREMENT STARTANGLE",do_VMOrbitInstall},
  {"VMORBIT",7,"SPACE: Updates Obits",do_VMOrbits},
  /*{"DUMPPLAYERS",11,"SPACE: Updates Obits",do_VMDumpAllPlayers},*/
  {"VMINIT",6,"SPACE: Inits Space",do_VMInitSpace},
  {"REHELM",6,"SPACE: USE WITH CAUTION (IE DONT USE IT)",do_VMRehelm},
  {"PLACE",5,"SPACE: Place a Space Object",do_VMPlace},
  {"SHOWSPACE",9,"SPACE: Place a Space Object",do_VMShowSpaceAll},
  {"DORB",4,"SPACE: Removes an Obit",do_VMDORB},
  {"SAVE",4,"SPACE: Saves Space",do_VMSave},
  {"VMSPEC",6,"SPACE: Recalcs Specials",do_VMSetupSpecials},
  {"VMROOM",6,"SPACE: Recalcs Specials",do_VMSetupRooms},
  {"VMFLAG",6,"SPACE: Sets Flags on a Ship",do_VMFlag},
  {"LISTHULLS",9,"SPACE: Shows all hulls",do_VMListHulls},
  {"LISTSYS",7,"SPACE: Shows all Systems",do_VMListSystems},
  {"SHOWSHIP",8,"SPACE: Shows useful info about a ship",do_VMShowShip},
  {"VMSEE",5,"SPACE: See The Flags on a Ship",do_VMSeeFlag},
  {"VMSETMB",7,"SPACE: Sets Mainbridge on a ship",do_VMSetMainBridge},
  {"INSTALL",7,"SPACE: INSTALL 3 SYSEM# PLACE# SHIPNUM# ARC# or INSTALL TYPE# SYSTEM# PLACE# SHIPNUM# or INSTALL 0 HULL# SHIP# or INSTALL 5 NAME SHIPNUM#",do_VMAdminInstall},
  {"ADD",3,"THIS COMMAND CAN CRASH US!!!!!!!!!!! add <ship> <miss>",do_VMAddMissle},

  {NULL,   0 , NULL , NULL}
};
CommandsStruct vmroom[]={
  {"ADS",3,"ADS: Auto-Defense System",do_VMADS},
  {"ADOOR",5,"NAV: Open/Close Bay Door",do_VMADoor},
  {"CS",2,"COMMU: Comm Status",do_VMCommStatus},
  {"RLOCK",5,"COMMU: ComLock onto a Relay",do_VMCommRLock},
  {"UNRLOCK",7,"COMMU: Remove any ComLock",do_VMCommUnRLock},
  {"CCODE",5,"COMMU: Encodes an open channel",do_VMCommCCode},
  {"DCODE",5,"COMMU: Removes coding on an open channel",do_VMCommDCode},
  {"OPEN",4,"COMMU: Open A channel",do_VMCommOpen},
  {"CLOSE",5,"COMMU: Close A channel",do_VMCommClose},
  {"TRAN",4,"COMMU: Transmit A Msg",do_VMCommTran},
  {"CON",3,"COMMU: Turn Comm On",do_VMCommOn},
  {"COSCAN",6,"COMMU: Scans a ship for relays",do_VMCommScan},
  {"COFF",4,"COMMU: Turn Comm Off",do_VMCommOff},
  {"DOCK",4,"NAV: Docks",do_VMDock},
  {"UNDOCK",6,"NAV: Undocks",do_VMUnDock},
  {"BAYS",4,"NAV: Shows Docking Bays",do_VMNavBays},
  {"MAN",3,"ALL: DAM: mans a console",do_VMMan},
  {"UNMAN",5,"ALL: DAM: unmans a console",do_VMUnMan},
  {"ENGAGE",6,"NAV: engage engines",do_VMEngage},
  {"DISENGAGE",4,"NAV: disengage engines",do_VMDisEngage},
  {"SPEED",5,"NAV: sets speed",do_VMSetSpeed},
  {"DCS",3,"NAV: Damage Control status",do_VMDamStat},
  {"SCAN",4,"NAV: Scans around",do_VMScanShip},
  {"GATE",4,"NAV: GATE <JumpGate #>",do_VMGate},
  {"SEN",4,"NAV: Scans around",do_VMScan},
  {"SCL",3,"HELM/NAV: Show Contact List",do_VMScan2},
  {"SH",2,"NAV: sets heading",do_VMSetHeading},
  {"CHART",5,"NAV: Make a chart",do_VMChart},
  {"TLOCK",5,"NAV: Locks the tractor beam",do_VMTLock},
  {"TUNLOCK",7,"NAV: UnLocks the tractor beam",do_VMTUnLock},
  {"TENGAGE",7,"NAV: Turns the tractor beam on",do_VMTEngage},
  {"TDISENGAGE",10,"NAV: Turns the tractor beam off",do_VMTDisEngage},
  {"AP",2,"NAV: autopilot, AM <X> <Y> <Z>",do_VMDoAP},
  {"DAP",3,"NAV: Disengage autopilot",do_VMAPOff},
  {"NS",2,"NAV: nav status",do_VMNavStatus},
  {"JUMP",4,"NAV: Engage Jump Drive",do_VMNavJump},
  {"HS",2,"HELM: helm status",do_VMHelmStatus},
  {"AS",2,"HELM/NAV: armor status",do_VMArmorStatus},
  {"MS",2,"missle status",do_VMMissleStatus},
  {"LAY",3,"HELM: Lay a mine from tube <tube>",do_VMHelmLayMine},
  {"SWEEP",5,"HELM: Sweep mines in the area",do_VMHelmSweepMine},
  {"MLIST",5,"HELM: Show mines on sensors.",do_VMHelmShowShipMineList},
  {"ARM",3,"arm <tube>",do_VMArmMissle},
  {"TAL",3,"Sets <tube> to autoload <missile>",do_VMAutoloadTube},
  {"INT",3,"Turns INTerceptor battery <num> <on|off>",do_VMInterceptorState},
  {"TUBE",4,"tube <missletube> <open|close>",do_VMMissleDoor},
  {"LOAD",4,"load <missletube> <misslenumber>",do_VMLoadMissle},
  {"DUMP",4,"load <missletube> <misslenumber>",do_VMDumpMissle},
  {"UNLOAD",6,"unload <missletube> <misslenumber>",do_VMUnloadMissle},
  {"LOCK",4,"HELM: Lock <weapon> <target>",do_VMHelmLockWeapon},
  {"FIRE",4,"HELM: Fire <weapon>",do_VMHelmFireWeapon},
  {"ES",2,"ENG: eng status",do_VMEngStatus},
  {"START",5,"ENG: Starts reactor",do_VMEngStartReactor},
  {"STOP",4,"ENG: Stops reactor",do_VMEngStopReactor},
  {"EA",2,"ENG: eng allocate",do_VMEngSetAllocs},
  {"NA",2,"NAV: nav allocate",do_VMNavAlloc},
  {"HA",2,"HELM: helm allocate",do_VMHelmAlloc},
  {"WEAP",4,"HELM: Changes State of weapon",do_VMHelmWeaponState},
  {"SETG",4,"HELM: Sets gun to do advanced crud",do_VMSetFireGun},
  {"REPAIR",6,"DAM: repairs a system",do_VMMiscRepairSystem},
  {"VERBOSE",7,"DAM: Makes repair updates get to you",do_VMSetDamVerbose},
  {"SCISCAN",7,"DAM: Makes repair updates get to you",do_VMSciScan},
  {"GUNPORT",7,"HELM: Opens and close gunports if applicable",do_VMHelmGunPort},
  {"SR",2,"ENG: sets reactor",do_VMEngSetReactor},
  {"BATT",4,"ENG: Turns batteries on or off",do_VMEngBatts},
  {"SAFETIES",8,"ENG: Turns engine safeties on or off",do_VMSafeties},
  {NULL,   0 , NULL , NULL}
};
CommandsStruct cargoroom[]={
{"ELOAD",5,"Load Commodity",econload},
{"EUNLOAD",7,"UnLoad Commodity",econunload},
{"LCARGO",6,"Lists Commodities",econcommod},
{NULL,   0 , NULL , NULL}
};

CommandsStruct storeroom[]={
{"EUNLOAD",7,"UnLoad Commodity - Give Away Free",econunload},
{"EBUY",4,"Buy Commodity",ebuy},
{"SUPPLY",6,"View Money Supply Available",do_supply},
{"LCARGO",6,"Lists Commodities and prices",econcommod_pr},
{"PRICE",5,"Lists A Specific Commodity's price",econcommod_price},
{NULL,   0 , NULL , NULL}
};

CommandsStruct traderoom[]={
{"EUNLOAD",7,"UnLoad Commodity - Give Away Free",econunload},
{"EBUY",4,"Buy Commodity",ebuy},
{"ESELL",5,"Sell Commodity",esell},
{"SUPPLY",6,"View Money Supply Available",do_supply},
{"LCARGO",6,"Lists Commodities and prices",econcommod_pr},
{"PRICE",5,"Lists A Specific Commodity's price",econcommod_price},
{NULL,   0 , NULL , NULL}
};

/*CommandsStruct bank[]={
{"BALANCE",7,"See your balance",do_balance},
{"DEPOSIT",7,"Deposit Money",do_deposit},
{"WITHDRAW",8,"Withdraw Money",do_withdraw},
{"ACCESS",6,"Accesses a bank account",do_access},
{NULL,   0 , NULL , NULL}
};
*/
CommandsStruct bankadmin[]={
{"BANKINIT",8,"Initializes Bank array",do_InitBanks},
{"BANKLOAD",8,"Load Banks from file",do_LoadBanks},
{"BANKSAVE",8,"Saves Banks to a file",do_SaveBanks},
{"ACCTINIT",8,"Initializes Accounts Database",do_BankSkel},
/*{"ACCTLOAD",8,"Load Banks from file",do_LoadAccts},
{"ACCTSAVE",8,"Saves Banks to a file",do_SaveAccts},
{"BANKMAKE",8,"Creates a Bank",do_MCreateBank},
*/{"BANKSTAT",8,"Bank Status Report",do_BankList},
{"SHOWACCT",8,"Shows An Account",do_BankSkel},
{"SETIR",5,"Set IR on Bank",do_SetIR},
{NULL,   0 , NULL , NULL}
};

CommandsStruct bankmanager[]={
/*  {"ACCTCREATE",10,"Creates an Account",do_CreateAcct},*/
  {"ACCTSHOW",8,"Displays an Acct",do_BankSkel},
  {"ACCTLOCK",8,"Locks out an Acct",do_BankSkel},
  {"ACCTUNLOCK",10,"Unlock's an Acct",do_BankSkel},
  {"ACCTDEL",7,"Delete's an acct",do_BankSkel},
  {"SETPIN",6,"Sets a PIN on an acct",do_BankSkel},
{NULL,   0 , NULL , NULL}
};

CommandsStruct bank[]={
  {"ACCESS",6,"Sets The Bank up to Access an Acct",do_BankSkel},
  {"WITHDRAWL",9,"Does a withdrawl",do_BankSkel},
  {"DISPLAY",7,"Display's Acct Status",do_BankSkel},
  {"DEPOSIT",7,"Does a deposit",do_BankSkel},
  {"SHOWCREDIT",10,"Display's credit info",do_BankSkel},
  {"CHANGEPIN", 9,"changes PIN for the acct",do_BankSkel},
  {NULL,   0 , NULL , NULL}
};


CommandsStruct relay[]={
{"STATUS",6,"Relay Status",relay_status},
{"CODE",4,"Set the code on a bandwidth range",do_code},
{"OPEN",4,"Open a frequency range to relay",OpenRelay},
{"CLOSE",5,"Stop relaying a frequency range",CloseRelay},
/*{"SETUP",5,"Initialize relasy",setup_network},
{"UPDATE",6,"Update relay information",calc_network},*/
{"CHECK",5,"Check a specific relay connection",check_network},
{NULL,   0 , NULL , NULL}
};

CommandsStruct valworld[]={
{"ADD",3,"Add a new Val DB for player", new_ValDB_interface},
{"CSET",4,"PLAYER CMD# VALUE",cset_ValDB},
{"SSET",4,"PLAYER SKILL VALUE",sset_ValDB},
{"SAVE",4,"Save ValDB",write_ValDB_interface},
{"LOAD",4,"Load ValDB",read_ValDB_interface},
{"EOB",3,"Change the Object value of a ValDB Entry. EOB VAL# DB#",PDB_ValDB},
{"MAX",3,"Check the max #",read_ValDBMax_interface},
{"VCHECK",6,"Run a check through whole valdb for unused slots",do_VCheck},
{"MARK",4,"Toggles the Used/Unused flag on ValDB. USE WITH CAUTION!!!!",ValDB_Mark},
{"VFIND",5,"Find the db # of a player dbref",read_ValDBFind_interface},
{"CHECK",5,"Check the db # of a valdb",read_ValDBNum_interface},
{NULL,   0 , NULL , NULL}
};

CommandsStruct create1[]={
{"NEW",3,"Wipes your character and sets you up to be created.",new_playergen},
{"PRIORITIES",10,"List what you want in order from greatest to least priority. With 1-Attributes, 2-Skills, 3-Money",choose_pri},
{NULL,   0 , NULL , NULL}
};

CommandsStruct create2[]={
{"SSELL",5,"Sell a skill. SSELL SKILL#.",ssell},
{"ASELL",5,"Sell an attribute. ASELL ATTRIB#.",asell},
{"SBUY",4,"Buy a skill. SBUY SKILL#.",sbuy},
{"ABUY",4,"Buy an attribute. ABUY ATTRIB#.",abuy},
{"DONE",4,"Type when you are done buying things.",do_done},
{NULL,   0 , NULL , NULL}
};

CommandsStruct shipdock[]={
{"STATUS",6,"Factory Status",factory_status},
{"BUILD",5,"Set Plan to Build",factory_build},
{"PLAN",4,"See the specifics of a plan",factory_plandef},
{"ELOAD",5,"Load Commodity",econload},
{"EUNLOAD",7,"UnLoad Commodity",econunload},
{"PSET",4,"Sets the productivity factor",factory_pset},
{"REPAIR",6,"Repair a system on a docked ship",do_VMSBMiscRepairSystem},
{"DSTAT",5,"Status of repair teams",do_VMSBDamStat},
{"SSTAT",5,"System status on a docked ship",do_VMSBSysStat},
{"FUEL",4,"Adds fuel to a docked ship",do_VMSBFuel},
{NULL,   0 , NULL , NULL}
};

CommandsStruct factoree[]={
{"STATUS",6,"Factory Status",factory_status},
{"BUILD",5,"Set Plan to Build",factory_build},
{"PLAN",4,"See the specifics of a plan",factory_plandef},
{"ELOAD",5,"Load Commodity",econload},
{"EUNLOAD",7,"UnLoad Commodity",econunload},
{"PSET",4,"Sets the productivity factor",factory_pset},
{NULL,   0 , NULL , NULL}
};

CommandsStruct ashipy[]={
{"MAKESHIP",8,"Makes a ship",do_MakeShip},
{"PLACE",5,"Places a ship in a Dock",do_DPlace},
{NULL,   0 , NULL , NULL}
};

CommandsStruct elevroom[]={
{"PUSH",4,"Push a Button",push_elevator},
{NULL,   0 , NULL , NULL}
};

CommandsStruct vehtype[]={
{"MSUP",4,"View the vehicles money supply",vmons},
{"VCOM",4,"View the vehicles commodities",vcoms},
{"PAYME",5,"Xfer money from vehicle to you",do_vepay},
{"VMOVE",5,"Move the lift",do_vmove},
{"ESELL",5,"Sell Commods if in trade room",do_vesell},
{"EBUY",4,"Buy Commods if in trade room",do_vebuy},
{"EUNLOAD",7,"Unload Commods if in storage room",do_veconunload},
{"LOADMIS",7,"Loads a missile from the vehicle to a ship",do_vaddmis},
{"ELOAD",5,"Load Commods if in storage room",do_veconload},
{NULL,   0 , NULL , NULL}
};

CommandsStruct sbdock[]={
{"REPAIR",6,"Repair a system on a docked ship",do_VMSBMiscRepairSystem},
{"FUEL",4,"Adds fuel to a docked ship",do_VMSBFuel},
{"STAT",4,"Status of repair teams",do_VMSBDamStat},
{"SSTAT",5,"System status on a docked ship",do_VMSBSysStat},
{NULL,   0 , NULL , NULL}
};

CommandsStruct elevout[]={
{"CALL",4,"Call the Elevator",call_elevator},
{NULL,   0 , NULL , NULL}
};


CommandsStruct compcommands[]={
  {"SETFACTION <FACTION>",   10, "@Sets the faction refernce <privledged>", comp_setfaction},
  {"LOGIN", 5, "Login to the Network",comp_login},
/*  Added so I could get it to compile - et
  {"LOGOUT", 5, "Logout of the the Network",comp_logout},
*/
  {"LS", 2, "Get a listing of the files in the current directory.", comp_ls},
  {"TOP", 3, "Change to the top directory", comp_top},
  {NULL,     0, NULL, NULL}                       
};

CommandsStruct mechcommands[]={
  {"HEADING <NUM>",   7, "Sets the heading to given value.", mech_heading},
  {"SPEED <NUM>",     5, "Sets the speed to given value.", mech_speed},
  {"STATUS [A[RMOR]|I[NFO]|W[EAPONS]]",    6, "Prints the mech's status", mech_status},
  {"CRITS <SECTION>", 5, "Shows the Critical hits status", mech_critstatus},
  {"WEAPONSPECS", 11, "Shows the specifications for your weapons", 
     mech_weaponspecs},
  {"TACTICAL [<BEARING> <RANGE> | <TARGET-ID>]",   8, "Shows the tactical map at the mech's location at bearing and range", mech_tacmap},
  {"BRIEF",5,"Toggles Brief display of information",mech_brief},
  {"LRS <MECH | TERRAIN | ELEV> [<BEARING> <RANGE> | <TARGET-ID>]",   3, 
     "Shows the long range map", mech_lrsmap},
  {"LOCK [<TARGET-ID> | <X Y>]", 4, "Sets the target to the value.",
     mech_settarget},
  {"SCAN [<TARGET-ID> | <X Y>]", 4, "Scans the default target, num, or x,y",
     mech_scan},
  {"VIEW [<TARGET-ID>]", 4, "View the war painting on the target",
     mech_view},
  {"CONTACTS [<Prefix> | <TARGET-ID>]", 8, 
     "List all current contacts", mech_contacts},
  {"REPORT [<TARGET-ID> | <X Y>]", 6, 
     "Information on default target, num, or x,y", mech_report},
  {"SIGHT <WEAPNUM> [<TARGET-ID> or <X Y>]", 5, "Computes base-to-hit for given weapon and target.", mech_sight},
  {"FIRE <WEAPNUM> [<TARGET-ID> or <X Y>]", 5, /* must be 5 to avoid */
                                            /* conflict */
     "Fires Weap at loc at def. target or specified target.", mech_fireweapon},
  {"TARGET <SECTION> or '-'", 6, "Sets your aimed shot target or disables targetting.", mech_target},
  {"AMS", 3, "Toggles Anti-Missile System on and off", mech_ams},
  {"FLIPARMS",8,"Flips the arms to and from the rear arcs, if possible.",
						mech_fliparms},
  {"ULTRA <weapnum>",5,"Sets Weapon to and from Ultra Mode.",mech_ultra},
  {"LBX <weapnum>",3,"Sets Weapon to and from LBX Mode.",mech_lbx},
  {"ARTEMIS <weapnum>",7,"Sets Weapon to and from ARTEMIS Mode.",mech_artemis},
  {"NARC <weapnum>",4,"Sets Weapon to and from NARC Mode.",mech_narc},
  {"RANGE [<X Y>] [<X Y>]", 5, 
      "Range to target, range to x y, range to x,y, from x, y", mech_range},
  {"BEARING [<X Y>] [<X Y>]", 7, "Same format as RANGE.", mech_bearing},
  {"STARTUP", 7, "Commences startup cycle.", mech_startup},
  {"SHUTDOWN", 8, "Shuts down the mech.", mech_shutdown},
  {"LVMAP", 5, "Leaves the map via a bay.", mech_leave_map},
  {"STAND", 5, "Stand up after a fall or dropping prone", mech_stand},
  {"DROP", 4, "Force your 'Mech to drop prone where it is", mech_drop},
  {"JUMP [<TARGET-ID> | <BEARING> <RANGE>]", 4, 
	"Jump on default target or given target or jump given bearing and range.", mech_jump},
  {"ROTTORSO <LEFT | RIGHT | CENTER>", 8, 
     "Rotates the torso 60 degrees right or left.", mech_rotatetorso},
  {"LAND", 4, "Terminate your jump or land a VTOL", mech_land},
  {"DUMP <WEAPNUM>",   4, "Dumps the ammunition for the weapon.", mech_dump},

  /* Physical combat */

  {"PUNCH [R | L | B] [<TARGET-ID>]",5,"Punches a target",mech_punch},
  {"CLUB [<TARGET-ID>]",4,"Clubs a target with a tree",mech_club},
  {"KICK [R | L] [<TARGET-ID>]",4,"Kicks a target",mech_kick},
  {"CHARGE [<TARGET-ID> | - ]",6,"Charges a target. '-' removes charge command.",mech_charge},
    
  /* TIC Support */
  {"CLEARTIC <NUM>", 8, "Clears the TIC number given ", mech_cleartic},
  {"ADDTIC  <NUM> <WEAPNUM>", 6, "Adds weapnum to given TIC", mech_addtic},
  {"DELTIC <NUM> <WEAPNUM>", 6, "Deletes weapnum from given TIC", 
     mech_deltic},
  {"FIRETIC <NUM> [<TARGET> or <X Y>]", 7, "Fires the given TIC", mech_firetic},
  {"LISTTIC <NUM>", 7, "Lists weapons in the given TIC", mech_listtic},
  

  /* Restricted commands */
  {"UPDATE", 6, "@Updates movement, weapons, heat, etc.", mech_Rupdate},
  {"SETTEAM <NUM>", 7, "@Sets the teams.", mech_Rsetteam},
  {"SETXY <X> <Y>", 5, "@Sets the x & y value of the mech.", mech_Rsetxy},
  {"SETMAPINDX <NUM>", 10, "@Sets the mech's map index to num.",
     mech_Rsetmapindex},

  {NULL,     0,  NULL,                    NULL}
};

CommandsStruct vehiclecommands[]={
  {"HEADING <NUM>",   7, "Sets the heading to given value.", mech_heading},
  {"SPEED <NUM>",     5, "Sets the speed to given value.", mech_speed},
  {"VERTICAL <NUM>",  8, "Sets the vertical speed for a VTOL to given value.", 
     mech_vertical},
  {"STATUS [A[RMOR]|I[NFO]|W[EAPONS]]",    6, "Prints the mech's status", mech_status},
  {"WEAPONSPECS", 11, "Shows the specifications for your weapons", 
     mech_weaponspecs},
  {"TACTICAL [<BEARING> <RANGE> | <TARGET-ID>]",   8, "Shows the tactical map at the mech's location at bearing and range", mech_tacmap},
  {"BRIEF",5,"Toggles Brief display of information",mech_brief},
  {"LRS <MECH | TERRAIN | ELEV> [<BEARING> <RANGE> | <TARGET-ID>]",   3, 
     "Shows the long range map", mech_lrsmap},
  {"LOCK [<TARGET-ID> | <X Y>]", 4, "Sets the target to the value.",
     mech_settarget},
  {"SCAN [<TARGET-ID> | <X Y>]", 4, "Scans the default target, num, or x,y",
     mech_scan},
  {"VIEW [<TARGET-ID>]", 4, "View the war painting on the target",
     mech_view},
  {"CONTACTS [<Prefix> | <TARGET-ID>]", 8, 
     "List all current contacts", mech_contacts},
  {"REPORT [<TARGET-ID> | <X Y>]", 6, 
     "Information on default target, num, or x,y", mech_report},
  {"SIGHT <WEAPNUM> [<TARGET-ID> or <X Y>]", 5, "Computes base-to-hit for given weapon and target.", mech_sight},
  {"FIRE <WEAPNUM> [<TARGET-ID> or <X Y>]", 5, /* must be 5 to avoid */
                                            /* conflict */
     "Fires Weap at loc at def. target or specified target.", mech_fireweapon},
  {"TARGET <SECTION> or '-'", 6, "Sets your aimed shot target or disables targetting.", mech_target},
  {"AMS", 3, "Toggles Anti-Missile System on and off", mech_ams},
  {"ULTRA <weapnum>",5,"Sets Weapon to and from Ultra Mode.",mech_ultra},
  {"LBX <weapnum>",3,"Sets Weapon to and from LBX Mode.",mech_lbx},
  {"ARTEMIS <weapnum>",7,"Sets Weapon to and from ARTEMIS Mode.",mech_artemis},
  {"NARC <weapnum>",4,"Sets Weapon to and from NARC Mode.",mech_narc},
  {"RANGE [<X Y>] [<X Y>]", 5, 
      "Range to target, range to x y, range to x,y, from x, y", mech_range},
  {"BEARING [<X Y>] [<X Y>]", 7, "Same format as RANGE.", mech_bearing},
  {"STARTUP", 7, "Commences startup cycle.", mech_startup},
  {"SHUTDOWN", 8, "Shuts down the mech.", mech_shutdown},
  {"TAKEOFF", 7, "VTOL take off command", mech_takeoff},
  {"LAND", 4, "Terminate your jump or land a VTOL", mech_land},
  {"TURRET", 6, "Set the turret facing.", mech_turret},
  {"DUMP <WEAPNUM>",   4, "Dumps the ammunition for the weapon.", mech_dump},

  /* TIC Support */
  {"CLEARTIC <NUM>", 8, "Clears the TIC number given ", mech_cleartic},
  {"ADDTIC  <NUM> <WEAPNUM>", 6, "Adds weapnum to given TIC", mech_addtic},
  {"DELTIC <NUM> <WEAPNUM>", 6, "Deletes weapnum from given TIC", 
     mech_deltic},
  {"FIRETIC <NUM> [<TARGET> or <X Y>]", 7, "Fires the given TIC", mech_firetic},
  {"LISTTIC <NUM>", 7, "Lists weapons in the given TIC", mech_listtic},
  

  /* Restricted commands */
  {"UPDATE", 6, "@Updates movement, weapons, heat, etc.", mech_Rupdate},
  {"SETTEAM <NUM>", 7, "@Sets the teams.", mech_Rsetteam},
  {"SETXY <X> <Y>", 5, "@Sets the x & y value of the mech.", mech_Rsetxy},
  {"SETMAPINDX <NUM>", 10, "@Sets the mech's map index to num.",
     mech_Rsetmapindex},

  {NULL,     0,  NULL,                    NULL}
};

CommandsStruct debugcommands[]={
  {"LIST [MECHS | MAPS]",   4, "@Shows all the special objects allocated", debug_list},
  {"SHUTDOWN <MAP#>", 8, "@Shutdown all mechs on the map and clear it.", debug_shutdown},
  {"SAVEDB", 6, "@Saves the SpecialObject DB", debug_savedb},
  {"LOADDB", 6, "@Loads the SpecialObject DB", debug_loaddb},
  {NULL,     0, NULL, NULL}                       
};

CommandsStruct mapcommands[]={
  {"VIEW <X> <Y>",  4, "Shows the map centered at X,Y", map_view},
  {"ADDHEX <X> <Y> <TERRAIN> <ELEV>", 6, 
     "@Changes the terrain and elevation of the given hex", map_addhex},
  {"ADDROW <ROW> <TERRAIN> <ELEV>", 6, "@Sets the given row in the map to the given terrain and elevation strings", map_addrow},
  {"MAPNAME <NAME>", 7, "@Names the map", map_mapname},
  {"LOADMAP <NAME>", 7, "@Loads the named map", map_loadmap},
  {"SAVEMAP <NAME>", 7, "@Saves the map as name", map_savemap},
  {"SETMAPSIZE <X> <Y>", 10, "@Sets x and y size of map", map_setmapsize},
  {"LISTMECHS", 9, "Lists mechs on the map", map_listmechs},
  {"CLEARMECHS [DBNUM]", 10, "@Clears mechs from the map", map_clearmechs},
  {"ADDLINK <UPLINK> [<DOWNLINK> <LEFTLINK> <RGTLINK>]", 
     7, "@Adds map links to edges", map_addlink},
  {"STARTXY <X> <Y>", 7, "@Sets x and y bases of the map", map_startxy},  
  {NULL,     0, NULL, NULL}                       
};


CommandsStruct mechrepcommands[]={
  {"SETTECH <PLAYERNUM>", 7, "@Sets the tech to playernum.", mechrep_Rsettech},
  {"SETTARGET <NUM>", 9, "@Sets the mech to be repaired/built to num",
				     mechrep_Rsettarget},
  {"LOADNEW <TYPENAME>",7, "@Loads a new mech template.", mechrep_Rloadnew},
  {"RESTORE",7, "@Completely repairs and reloads mech. ", mechrep_Rrestore},
  {"SAVENEW <TYPENAME>",7, "@Saves the mech as a template.", mechrep_Rsavetemp},
  {"SETARMOR <LOC> <AVAL> <IVAL> <RVAL>", 8,
     "@Sets the armor, int. armor, and rear armor.", mechrep_Rsetarmor},
  {"ADDWEAP <NAME> <LOC> <CRIT SECS> [RT]", 7, 
      "@Adds weapon to the mech, using given loc and crit slots", 
			mechrep_Raddweap},
  {"REPAIR <LOC> <TYPE> <[VAL | SUBSECT]>", 6, "@Repairs the mech.",
     mechrep_Rrepair},
  {"RELOAD <NAME> <LOC> <SUBSECT> [L | A | N]", 6,
     "@Reloads weapon in location and critical subsection.", 
     mechrep_Rreload},
  {"ADDSP <ITEM> <LOC> <SUBSECT> [<DATA>]", 5,
     "@Adds a special item in location & critical subsection.", 
     mechrep_Raddspecial},
  {"DISPLAY <LOC>", 7, "@Displays all the items in the location.", 
     mechrep_Rdisplaysection},
  {"SHOWTECH", 8, "@Shows the advanced technology of the mech.", 
     mechrep_Rshowtech},
  {"ADDTECH <TYPE>", 7, 
	"@Adds the advanced technology to the mech.", mechrep_Raddtech},
  {"DELTECH", 7, 
	"@Deletes the advanced technology of the mech, slots set to empty.", 
	     mechrep_Rdeltech},
  {"SETTONS <NUM>", 7, "@Sets the mech tonnage", mechrep_Rsettons},
  {"SETTYPE <MECH | GROUND | VTOL | NAVAL>", 7, 
     "@Sets the mech type", mechrep_Rsettype},
  {"SETMOVE <TRACK | WHEEL | HOVER | VTOL | HULL | FOIL>", 7, 
     "@Sets the mech movement type", mechrep_Rsetmove},
  {"SETMAXSPEED <NUM>", 11, "@Sets the max speed of the mech.", 
     mechrep_Rsetspeed},
  {"SETHEATSINKS <NUM>", 12, "@Sets the number of heat sinks.", 
     mechrep_Rsetheatsinks},
  {"SETJUMPSPEED <NUM>", 12, "@Sets the jump speed of the mech.", 
     mechrep_Rsetjumpspeed},
  {"SETLRSRANGE <NUM>", 11, "@Sets the lrs range of the mech.", 
     mechrep_Rsetlrsrange},
  {"SETTACRANGE <NUM>", 11, "@Sets the tactical range of the mech.", 
     mechrep_Rsettacrange},
  {"SETSCANRANGE <NUM>", 12, "@Sets the scan range of the mech.", 
     mechrep_Rsetscanrange},
  {NULL,     0,  NULL,                    NULL}
}; 



struct SpecialObjectStruct SpecialObjects[]={
  {"MECH",    mechcommands,  sizeof(struct mech_data), 
     newfreemech, 1, 0, mech_update, POW_MECH},
  {"VEHICLE", vehiclecommands,  sizeof(struct mech_data), 
     newfreemech, 1, 0, mech_update, POW_MECH},
  {"DEBUG",   debugcommands, 0, debug_allocfree, 0, 0, NULL, POW_SECURITY},
  {"MECHREP", mechrepcommands, sizeof(struct mechrep_data), 
     newfreemechrep, 0, 0, NULL, POW_MECHREP},
  {"MAP",     mapcommands,   sizeof(struct map_struct), 
     newfreemap, 1, 0, clear_LOSinfo, POW_MAP},
  {"COMP",     compcommands,   sizeof(struct comp_data), 
     newfreecomp, 0, 0, NULL, POW_SECURITY},
{"VMSECURITY", vmxsecurity,0,newfreespace,0,0,NULL,POW_SECURITY},
{"VMSPACE", vmspacee,0,newfreespace,1,0,VMUpdate,POW_SECURITY},
{"VMROOM", vmroom,0,newfreespace,0,0,NULL,POW_SECURITY},
{"ADS", vmads,0,newfreespace,0,0,NULL,POW_SECURITY},
{"ADSROOM", vmadsroom,0,newfreespace,0,0,NULL,POW_SECURITY},
/*{"FACTORY", factoree,0,newfreespace,20,0,factory_update,POW_SECURITY}*/
{"FACTORY", factoree,0,newfreespace,1200,0,factory_update,POW_SECURITY},
{"CARGOROOM", cargoroom,0,newfreespace,0,0,NULL,POW_SECURITY},
{"BANK", bank,0,newfreespace,0,0,NULL,POW_SECURITY},
{"BANKADM", bankadmin,0,newfreespace,0,0,NULL,POW_SECURITY},
{"BANKMGR", bankmanager,0,newfreespace,0,0,NULL,POW_SECURITY},
{"RELAY", relay,0,newfreespace,0,0,NULL,POW_SECURITY},
{"TRADEROOM", traderoom,0,newfreespace,0,0,NULL,POW_SECURITY},
{"STOREROOM", storeroom,0,newfreespace,0,0,NULL,POW_SECURITY},
{"CREATE1", create1,0,newfreespace,0,0,NULL,POW_SECURITY},
{"CREATE2", create2,0,newfreespace,0,0,NULL,POW_SECURITY},
{"ELEVROOM", elevroom,0,newfreespace,0,0,NULL,POW_SECURITY},
{"ELEVOUT", elevout,0,newfreespace,0,0,NULL,POW_SECURITY},
{"ASHIPY", ashipy,0,newfreespace,0,0,NULL,POW_SECURITY},
{"VEHTYP", vehtype,0,newfreespace,0,0,NULL,POW_SECURITY},
{"VALWORLD", valworld,0,newfreespace,0,0,NULL,POW_SECURITY},
{"SBDOCK", sbdock,0,newfreespace,0,0,NULL,POW_SECURITY},
{"SHIPDOCK", shipdock,0,newfreespace,1200,0,factory_update,POW_SECURITY}
};

#define NUM_SPECIAL_OBJECTS \
   ((sizeof(SpecialObjects))/(sizeof(struct SpecialObjectStruct)))

#define NUM_SOI_ENTRIES 1000
struct SpecialObjectInstance *SOI_Table[NUM_SOI_ENTRIES];
static char *rcsglueh="$Id: glue.h,v 1.1 2001/01/15 03:23:16 wenk Exp $";
