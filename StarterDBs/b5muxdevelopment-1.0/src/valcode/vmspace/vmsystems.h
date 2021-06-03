/* $Id: vmsystems.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */




struct s_System {

  char name[50];
  int kind;	/* 0 Helm,1 Nav,2 EngEtc*/
  int indexnum;	/*Nav, 1-10*/
  int type;	
  int skill;
  int sysnum;	/* its place in the Systems master array*/
  int flags;      /*WEAPON etc EXISTS ACTIVE*/
  int plan;	/*which plan represents this component*/
  int size;	/*Total of sizes !> Capacity*/
  
  double dstat0;
  double 	dstat1;
  double 	dstat2;
  double 	dstat3;
  double 	dstat4;
  double 	dstat5;
  double 	dstat6;
  double 	dstat7;
  double 	dstat8;
  double 	dstat9;
  
  double 	tdstat0;
  double 	tdstat1;
  double 	tdstat2;
  double 	tdstat3;
  double 	tdstat4;

  int	tistat0;
  int	tistat1;
  int	tistat2;
  int	tistat3;
  int	tistat4;

  int	istat0;
  int	istat1;
  int	istat2;
  int	istat3;
  int	istat4;
  int	istat5;
  int	istat6;
  int	istat7;
  int	istat8;
  int	istat9;
  
  double	threshold;
  double	integrity;
  double        mintegrity;
  double 	basenumber;	
  double        status;
};


typedef struct s_System t_System;



struct s_Hull
{

  char name[50];
  /*Modifiers*/
   int HullID;
  int Integrity;
  int Threshold;
  int AddSpeedMod; 
  /* not used int mIntegrity;*/
  double Max_speed;
  double EnergyPerSpeed; /*Speed per neergy modifier*/
  double Man_rate;
  double Accel;
  double Size;
  int Capacity;
  int MaxCargo;
  int Plan;
  
  int Flags; /*Copied to VMPerm.Flags*/
		
	


};

typedef struct s_Hull t_Hull;

t_Hull Hulls[MAXHULLS];
int VMHullMax;
t_System Systems[MAXSYSTEMS];
int VMSysMax;

	

/* Nav
 Systems 1 = Engines.
	dstat0 max speed
	dstat1 max turn
	dstat2 max accel
	dstat3 max used power
*/

#define ArmorFore 4
#define ArmorAft 5
#define ArmorPort 6
#define ArmorStar 7
#define ArmorTop 8
#define ArmorBottom 9


