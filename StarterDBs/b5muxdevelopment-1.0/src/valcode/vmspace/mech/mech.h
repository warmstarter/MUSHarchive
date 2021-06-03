#include "map.h"

#define P(x)	x
#define LEFTSIDE 1
#define RIGHTSIDE 2
#define FRONT 3
#define BACK 4

#define STAND 1
#define FALL 0

#define TURN 30                                  /* 30 sec turn */
#define KPH_PER_MP 10.75
#define MP_PER_KPH 0.0930233                    /* 1/KPH_PER_MP  */
#define MP_PER_UPDATE_PER_KPH 0.003100777       /* MP_PER_KPH/30 */
#define SCALEMAP 322.5                          /* 1/update      */
#define HEXLEVEL 5                              /* levels/hex    */
#define ZSCALE 64.5                             /* scalemap/hexlevel */
#define XSCALE 0.1547                           /* hex constant  */
#define YSCALE2 9.61482e-6                      /* update**2     */
#define MP1 10.75                               /* 2*MS_PER_MP   */
#define MP2 21.50                               /* 2*MS_PER_MP   */
#define MP3 32.25                               /* 3*MS_PER_MP   */
#define MP4 43.00                               /* 4*MS_PER_MP   */
#define MP5 53.75                               /* 5*MS_PER_MP   */
#define MP6 64.50                               /* 6*MS_PER_MP   */
#define MP9 96.75                               /* 9*MS_PER_MP   */
#define DELTAFACING 1440.0 

#define NOT_FOUND -1
#define NUM_CRITICALS 12
 
#define ARMOR 1
#define INTERNAL 2
#define REAR 3
 
#define MECHPILOT 0
#define MECHSTARTED 1
#define MECHALL 2

#define NOARC 0
#define FORWARDARC 1
#define LARMARC 2
#define RARMARC 3
#define REARARC 4
#define RSIDEARC 5
#define LSIDEARC 6
#define TURRETARC 10

/*
  Critical Types
  0       Empty
  1-100   Weapons
  101-150 Ammo
  151-219 Reserved
  220-255 Special startings...
*/

/* Display Types */
#define LRS_DISPLAY_WIDTH 70
#define LRS_DISPLAY_WIDTH2 35
#define LRS_DISPLAY_HEIGHT 11
#define LRS_DISPLAY_HEIGHT2 5
 
/* Critical Types... */
#define EMPTY                  0
#define WEAPON_BASE_INDEX      1
#define AMMO_BASE_INDEX        101
#define RESERVED_BASE_INDEX    201
#define SPECIAL_BASE_INDEX     220
 
/* To define one of these-> x=SPECIAL_BASE_INDEX+SHOULDER_OR_HIP */
#define SHOULDER_OR_HIP        0
#define UPPER_ACTUATOR         1
#define LOWER_ACTUATOR         2
#define HAND_OR_FOOT_ACTUATOR  3
#define LIFE_SUPPORT           4
#define SENSORS                5
#define COCKPIT                6
#define ENGINE                 7
#define GYRO                   8
#define HEAT_SINK              9
#define JUMP_JET               10
#define CASE                   11
#define FERRO_FIBROUS          12
#define ENDO_STEEL             13
#define TRIPLE_STRENGTH_MYOMER 14
#define TARGETING_COMPUTER     15
#define MASC                   16
#define C3_MASTER              17
#define C3_SLAVE               18 
#define BEAGLE_PROBE           19
#define ARTEMIS_IV             20
#define ECM                    21

/* To define one of these-> x=WEAPON_BASE_INDEX+SLASER */

#define IS_SRM_2               8
#define IS_SRM_4               9
#define IS_SRM_6               10
#define IS_SSRM_2              25  
#define IS_LRM_5               4
#define IS_LRM_10              5
#define IS_LRM_15              6
#define IS_LRM_20              7

#define IS_LB10AC              22                                        
#define IS_ULT_5AC             23

#define IS_AMS                 26
#define IS_NARC                28

#define CL_SRM_2               60
#define CL_SRM_4               61
#define CL_SRM_6               62
#define CL_SSRM_2              63
#define CL_SSRM_4              64
#define CL_SSRM_6              65
#define CL_LRM_5               66
#define CL_LRM_10              67
#define CL_LRM_15              68
#define CL_LRM_20              69

#define CL_LB_2AC              51
#define CL_LB_5AC              52
#define CL_LB10AC              53
#define CL_LB20AC              54
#define CL_ULT_2AC             55
#define CL_ULT_5AC             56
#define CL_ULT10AC             57
#define CL_ULT20AC             58

#define CL_AMS                 70
#define CL_NARC                72
 
/* Weapons structure and array... */
#define TBEAM     0
#define TMISSILE  1
#define TAMMO     2
#define THAND     3

/* Recycle Time */
#define UNCONCIOUS_TIME 30
#define WEAPON_RECYCLE_TIME 30
#define PHYSICAL_RECYCLE_TIME 30
#define JUMP_TO_HIT_RECYCLE 6
#define DROP_TO_STAND_RECYCLE 6
 
/* Critical Status */

#define CRIT_DESTROYED 255

/* Tic status */

#define TIC_NUM_DESTROYED -2
#define TIC_NUM_RELOADING -3
#define TIC_NUM_RECYCLING -4
#define TIC_NUM_PHYSICAL  -5
 
/* This is the max weapons per area- assuming 12 critical location and */
 /* the smallest weapon requires 1 */
#define MAX_WEAPS_SECTION 12
 
struct weapon_struct {
  char *name;
  char type;
  char heat;
  char damage;
  char min;
  char shortrange;
  char medrange;
  char longrange;
  char criticals;
  unsigned char ammoperton;
  unsigned char weight;    /* divide by 10 and put into float for actual */
                           /* screws CL.Magine.Gun by .05 tons */
  unsigned char special;
};

/* special weapon effects */
#define NONE 0
#define PULSE 1
#define LBX 2
#define ULTRA 3
#define STREAK 4
#define LRM 5
#define GAUSS 6
#define NARC 7

#define MAX_ROLL 11
struct missile_hit_table_struct {
  int key;
  int num_missiles[MAX_ROLL];
};
 
/**** Section #defs... */
/* The unusual order is related to the locations of weapons of high */
/* magnitude versus weapons of low mag */
#define LARM   0
#define RARM   1
#define LTORSO 2
#define RTORSO 3
#define CTORSO 4
#define LLEG   5
#define RLEG   6
#define HEAD   7
#define NUM_SECTIONS 8
/*  These defs are for Vehicles */
#define LSIDE  0
#define RSIDE  1
#define FSIDE  2
#define BSIDE  3
#define TURRET 4
#define ROTOR  5
#define NUM_VEH_SECTIONS 5 

/* Dixie's TIC stuff. (A gift, if you will) :) */
#define NUM_TICS 3
 
/* structure for each critical hit section */
struct critical_slot {
  unsigned char type;  /* Type of item that this is a critical for */
  unsigned char data;  /* Holds information like ammo remaining, etc */
  char mode;           /* Holds info like rear mount, ultra mode...*/
};

#define REAR_MOUNT     0x01    /* set if weapon is rear mounted */
#define ULTRA_MODE     0x02    /* set if weapon is in Ultra firing mode */
#define ON_TC          0x04    /* Set if the wepons mounted with TC */
#define LBX_MODE       0x08    /* set if weapon is firing LBX ammo */
#define ARTEMIS_MODE   0x10    /* artemis compatible missiles/laucher */
#define NARC_MODE      0x20    /* narc compatible missiles/launcher */
 
/* Structure for each of the 8 sections */
struct section_struct {
  char armor;           /* External armor value */
  char internal;        /* Internal armor value */
  char rear;            /* Rear armor value */
  char basetohit;       /* Holds to hit modifiers for weapons in section */
  char config;          /* flags for CASE, etc. */
  char recycle;         /* after physical attack, set counter */
  struct critical_slot criticals[NUM_CRITICALS];  /* Criticals */
};

/* Section configurations */
#define CASE_TECH              0x01   /* section has CASE technology */ 
#define SECTION_DESTROYED      0x02   /* section has been destroyed */ 

/* ground combat types */
#define CLASS_MECH            0
#define CLASS_VEH_GROUND      1
#define CLASS_VEH_VTOL        2
#define CLASS_VEH_NAVAL       3

/* ground movement types */
#define MOVE_BIPED            0
#define MOVE_TRACK            1
#define MOVE_WHEEL            2
#define MOVE_HOVER            3 
#define MOVE_VTOL             4 
#define MOVE_HULL             5
#define MOVE_FOIL             6

struct mech_data {
  char type;                 /* The type of this unit */
  char move;                 /* The movement type of this unit */
  dbref mynum;               /* My dbref */
  int mapnumber;             /* My number on the map */
  char pilotskill;           /* default piloting skill */
  char gunneryskill;         /* default gunneryskill */
  char tons;                 /* How much I weigh */
  dbref pilot;               /* My pilot */
  char pilotstatus;          /* damage pilot has taken */
  char unconcious;           /* Unconcious counter, 0 means awake */
  dbref mapindex;            /* 0..MAX_MAPS (dbref of map object) */
  char aim;                  /* section of target aimed at */
  char ID[2];                /* Only for internal use */
  char brief;                /* toggle brievity */
  char pilotskillbase;       /* holds constant skills mods */
  int turndamage;            /* holds damge taken in 5 sec interval */
  char team;                 /* Only for internal use */
  char terrain;              /* Terrain I am in */
  char elev;                 /* Elevation I am at */
  int facing;                /* 0-359.. */
  int desiredfacing;         /* You are turning if this != facing */
  float heat;                /* Heat index */
  char tac_range;            /* Tactical range for sensors */
  char lrs_range;            /* Long range for sensors */
  char scan_range;           /* Range for scanners */
  char numsinks;             /* number of heatsinks (also engine */
                             /* crits ( - from heatsinks) ) */
  float weapheat;            /* Weapon heat factor-> see manifesto */
  float plus_heat;           /* how much heat I am producing */
  float minus_heat;          /* how much heat I can dissipate */
  int status;                /* see key below */
  int specials;              /* see key below */
  char engineheat;           /* +5 per critical hit there */
  char basetohit;            /* total to hit modifiers from critical hits */ 
  int critstatus;            /* see key below */
  float startfx, startfy;    /* in real coords (for jump and goto) */
  float startfz, endfz;
  float jumplength;          /* in real coords (for jump and goto) */
  int goingx, goingy;        /* in map coords (for jump and goto) */
  int  x, y, z;              /* hex quantized x,y,z on the map in MP (hexes)*/
  int last_x, last_y;        /* last hex entered */
  float fx, fy, fz;          /* exact x, y and z on the map */
  float verticalspeed;       /* VTOL vertical speed in KPH */
  float speed;               /* Speed in KPH */
  float desired_speed;       /* Desired speed in KPH */
  float maxspeed;            /* Maxspeed (running) in KPH */
  float jumpspeed;           /* Jumping distance or current height in km */
  int jumpheading;           /* Jumping head */
  int turretfacing;          /* Jumping head */
  dbref chgtarget;           /* My CHARGE target */
  dbref dfatarget;           /* My DFA target */
  dbref target;              /* My default target */
  int targx, targy, targz;   /* in map coords */ /* to target squares */
  struct section_struct sections[NUM_SECTIONS]; /* armor */
  unsigned long tic[NUM_TICS];  /* A gift, for Dixie */
  char clock;                /* counter for something, etc.. (0-MAX_CLOCK-1) */
  char mech_type[31];        /* Holds the 30 char ID for the mech */
};

/* Counter for reloads etc... */
#define MAX_CLOCK 60
#define STARTUP_TIME 30
 
/* status element... */
#define TORSO_NORMAL  0x01
#define LANDED        0x01 /* For VTOL use only */
#define TORSO_RIGHT   0x02 /* Torso heading -= 60 degrees */
#define TORSO_LEFT    0x04 /* Torso heading += 60 degrees */
#define STARTED       0x08 /* Mech is warmed up */
#define PARTIAL_COVER 0x10
#define DESTROYED     0x20
#define JUMPING       0x40 /* Handled in UPDATE */
#define FALLEN        0x80
#define DFA_ATTACK    0x100
#define GETTING_UP    0x200
#define FLIPPED_ARMS  0x400
#define AMS_ENABLED   0x800 /* only settable if mech has ANTI-MISSILE_TECH */
#define NARC_ATTACHED 0x1000 /* set if mech has a NARC beacon attached. */

/* critstatus element */
#define LEG_DESTROYED          0x80
#define TURRET_LOCKED          0x80  /* Vehicle only */
#define LIFE_SUPPORT_DESTROYED 0x40
#define HIP_DAMAGED            0x20
#define GYRO_DAMAGED           0x10
#define GYRO_DESTROYED         0x01
#define SENSORS_DAMAGED        0x02
#define NO_LEGS                0x04
#define LARM_DESTROYED         0x08
#define RARM_DESTROYED         0x100
#define RHAND_DESTROYED        0x200
#define LHAND_DESTROYED        0x400
#define BOTH_HIPS_DAMAGED      0x800
#define LSHDR_DESTROYED        0x1000
#define RSHDR_DESTROYED        0x2000
#define LLOWACT_DESTROYED      0x4000
#define RLOWACT_DESTROYED      0x8000
#define LUPACT_DESTROYED       0x10000
#define RUPACT_DESTROYED       0x20000
#define TC_DESTROYED           0x40000
#define C3_DESTROYED           0x80000
#define ECM_DESTROYED          0x100000
#define BEAGLE_DESTROYED       0x200000

/* specials element: used to tell quickly what type of tech the mech has */
#define TRIPLE_MYOMER_TECH      0x01
#define CL_ANTI_MISSILE_TECH    0x02
#define IS_ANTI_MISSILE_TECH    0x04
#define DOUBLE_HEAT_TECH        0x08
#define MASC_TECH               0x10
#define CLAN_TECH		0x20
#define FLIPABLE_ARMS           0x40
#define C3_MASTER_TECH          0x80
#define C3_SLAVE_TECH           0x100
#define ARTEMIS_IV_TECH         0x200
#define ECM_TECH                0x400
#define BEAGLE_PROBE_TECH       0x800

extern struct weapon_struct MechWeapons[];
extern struct missile_hit_table_struct MissileHitTable[];

extern void *FindObjectsData();

/* math functions */
extern char *strtok();
#define fcos cos
#define fsin sin
#define fatan atan
#define TWOPIOVER360 0.0174533

/* mech.build.c */
extern int mech_parseattributes P((char *,char **,int));
extern int CheckData P((dbref, void *));
extern void FillDefaultCriticals P((struct mech_data *,int));
extern int ArmorSectionFromString P((int, char *));
extern int WeaponIndexFromString P((char *));
extern void DumpWeapons P((dbref));
extern char ArmorStatus P((struct mech_data *, int, int));
extern void DumpMechSpecialObjects P((dbref));
extern int FindSpecialItemCodeFromString P((char *));

/* mech.combat.c */
extern void mech_settarget P((dbref, void *, char *));
extern void mech_target P((dbref, void *, char *));
extern void mech_fireweapon P((dbref, void *, char *));
extern int FireWeaponNumber P((dbref, struct mech_data *, struct map_struct *, 
			  int , int, char **, int));
extern void mech_dump P((dbref, void *, char *));
extern int IsRunning P((float,float));
extern int AttackMovementMods P((struct mech_data *));
extern int TargetMovementMods P((struct mech_data *, float));
extern void FireWeapon P((struct mech_data *, struct map_struct *, 
			  struct mech_data *, int, int, int, int, int,
			  float,float,int,int,float,int,int));
extern void HitTarget P((struct mech_data *, int, struct mech_data *,int,int,
			int,int,int));
extern int FindAreaHitGroup P((struct mech_data *, struct mech_data *));
extern int FindTargetHitLoc P((struct mech_data *, struct mech_data *, 
			  int *, int*));
extern int FindTCHitLoc P((struct mech_data *, struct mech_data *, 
			  int *, int*));
extern int FindAimHitLoc P((struct mech_data *, struct mech_data *, 
			  int *, int*));
extern void DestroyMech P((struct mech_data *, struct mech_data *));
extern void DamageMech P((struct mech_data *, struct mech_data *, int, int,
			  int, int, int, int, int));
extern void DestroyWeapon P((struct mech_data *, int, unsigned char, char));
extern void HandleCritical P((struct mech_data *, struct mech_data *, 
			  int, int, int));
extern int FindPunchLocation P((int));
extern int FindKickLocation P((int));
extern int FindHitLocation P((struct mech_data *, int, int *));
extern void DestroySection P((struct mech_data *, struct mech_data *, int, int));
extern void AMSTarget P((struct mech_data *, int, struct mech_data *, int, int, char, int, int, int, int));

/* mech.contacts.c */
extern void mech_brief P((dbref, void *, char *));
extern void mech_contacts P((dbref, void *, char *));

/* mech.los.c */
extern int AddTerrainMod P((struct mech_data *, struct mech_data *,
			  struct map_struct *, float, int));
extern void clear_LOSinfo P((dbref, void *));
extern int InLineOfSight P((struct mech_data *, struct mech_data *,
			  int, int, float));

/* mech.maps.c */
extern void mech_lrsmap P((dbref, void *, char *));
extern void mech_tacmap P((dbref, void *, char *));

/* mech.move.c */
extern void mech_drop P((dbref, void *, char *));
extern void mech_stand P((dbref, void *, char *));
extern void mech_land P((dbref, void *, char *));
extern void mech_takeoff P((dbref, void *, char *));
extern void mech_heading P((dbref, void *, char *));
extern void mech_turret P((dbref, void *, char *));
extern void mech_rotatetorso P((dbref, void *, char *));
extern void mech_speed P((dbref, void *, char *));
extern void mech_vertical P((dbref, void *, char *));
extern void mech_jump P((dbref, void *, char *));
extern void LandMech P((struct mech_data *));
extern void MechFalls P((struct mech_data *, int));

/* mech.notify.c */
extern void MechBroadcast P((struct mech_data *, struct mech_data *,
			  struct map_struct *, char *));
extern void MechFireBroadcast P((struct mech_data *, struct mech_data *,
			  int, int, struct map_struct *, char *));
extern void mech_notify P((struct mech_data *, int, char *));
extern int get_mech_atr P((struct mech_data *, char *, char *));
     
/* mech.physical.c */
extern void mech_punch P((dbref, void *, char *));
extern void mech_club P((dbref, void *, char *));
extern void mech_kick P((dbref, void *, char *));
extern void mech_charge P((dbref, void *, char *));
extern void PhysicalAttack P((struct mech_data *, int, int, int, int, char **,
			  struct map_struct *, int));
extern void PhysicalDamage P((struct mech_data *, struct mech_data *, 
			  int, int, int));
extern void DeathFromAbove P((struct mech_data *, struct mech_data *));
extern void ChargeMech P((struct mech_data *, struct mech_data *));

/* mech.restrict.c */
extern void mech_Rsetxy P((dbref, void *, char *));
extern void mech_Rsetmapindex P((dbref, void *, char *));
extern void mech_Rsetteam P((dbref, void *, char *));
extern void mech_Rupdate P((dbref, void *, char *));
extern void newfreemech P((dbref, void **, int));

/* mech.scan.c */
extern void mech_sight P((dbref, void *, char *));
extern void mech_scan P((dbref, void *, char *));
extern void mech_report P((dbref, void *, char *));
extern void mech_bearing P((dbref, void *, char *));
extern void mech_range P((dbref, void *, char *));
extern void mech_view P((dbref, void *, char *));
extern void PrintReport P((dbref,struct mech_data *,struct mech_data *,float));
extern void PrintEnemyStatus P((dbref, struct mech_data *, 
			  struct mech_data *, float));
extern void PrintEnemyWeaponStatus P((struct mech_data *, dbref));

/* mech.startup.c */
extern void mech_startup P((dbref, void *, char *));
extern void mech_shutdown P((dbref, void *, char *));
extern void mech_leave_map P((dbref, void *, char *));

/* mech.status.c */
extern void mech_status P((dbref, void *, char *));
extern void mech_critstatus P((dbref, void *, char *));
extern void mech_weaponspecs P((dbref, void *, char *));
extern void CriticalStatus P((dbref, struct mech_data *, int));
extern void PrintWeaponStatus P((struct mech_data *, dbref));
extern void PrintArmorStatus P((dbref, struct mech_data *));

/* mech.tic.c */
extern void mech_cleartic P((dbref, void *, char *));
extern void mech_addtic P((dbref, void *, char *));
extern void mech_deltic P((dbref, void *, char *));
extern void mech_firetic P((dbref, void *, char *));
extern void mech_listtic P((dbref, void *, char *));

/* mech.update.c */
extern void move_mech P((struct mech_data *));
extern void UpdateHeading P((struct mech_data *));
extern void UpdateSpeed P((struct mech_data *));
extern int OverheatMods P((struct mech_data *));
extern void HandleOverheat P((struct mech_data *));
extern void UpdateHeat P((struct mech_data *));
extern void UpdateWeapons P((struct mech_data *));
extern int NewHexEntered P((struct mech_data *, struct map_struct *,
			float, float));
extern void CheckDamage P((struct mech_data *));
extern int UpdatePilotSkillRolls P((struct mech_data *));
extern int UpdatePilotStatus P((struct mech_data *));
extern void mech_update P((dbref, void *));
extern void CheckVTOLHeight P((struct mech_data *));

/* mech.utils.c */
extern struct map_struct *ValidMap P((dbref,dbref));
extern dbref FindTargetDBREFFromMapNumber P((struct mech_data *, char *));
extern void FindComponents P((float, int, float *, float *));
extern void CheckEdgeOfMap P((struct mech_data *));
extern int FindBearing P((float, float, float, float));
extern int InWeaponArc P((struct mech_data *, float, float));
extern int IsInWeaponArc P((struct mech_data *, float, float,int,int));
extern int FindBaseToHitByRange P((int, float));
extern int FindPilotPiloting P((struct mech_data *));
extern int FindPilotGunnery P((struct mech_data *));
extern int MadePilotSkillRoll P((struct mech_data *, int));
extern int Roll P((void));
extern void FindXY P((float, float, int, float, float *, float *));
extern float FindRange P((float, float, float, float, float, float));
extern float FindHexRange P((float, float, float, float));
extern void RealCoordToMapCoord P((int *, int *, float, float));
extern void MapCoordToRealCoord P((int, int, float *, float *));
extern int FindTargetXY P((struct mech_data *, float *, float *, float *));
extern int FindWeapons P((struct mech_data *, int, unsigned char *,
			  unsigned char *, int *));
extern int FindAmmunition P((struct mech_data *, unsigned char *, 
			  unsigned char *, unsigned char *));
extern int FindLegHeatSinks P((struct mech_data *));
extern int FindWeaponNumberOnMech P((struct mech_data *, int, int *, int *));
extern int FindWeaponIndex P((struct mech_data *, int));
extern int FindAmmoForWeapon P((struct mech_data *, int, int, int *, int *));
extern int FindLBXAmmoForWeapon P((struct mech_data *, int, int, int *, int *));
extern int FindArtemisAmmoForWeapon P((struct mech_data *, int, int, int *, int *));
extern int FindNarcAmmoForWeapon P((struct mech_data *, int, int, int *, int *));
extern int FindArtemisForWeapon P((struct mech_data *, int, int));
extern int FindDestructiveAmmo P((struct mech_data *, int *, int *));
extern int FindRoundsForWeapon P((struct mech_data *, int));
extern void ArmorStringFromIndex P((int, char *, char));

/* mech.advanced.c */
extern void mech_ams P((dbref, void *, char *));
extern void mech_fliparms P((dbref, void *, char *));
extern void mech_ultra P((dbref, void *, char *));
extern void mech_lbx P((dbref, void *, char *));
extern void mech_artemis P((dbref, void *, char *));
extern void mech_narc P((dbref, void *, char *));

