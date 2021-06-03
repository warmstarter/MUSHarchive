/* $Id: vmeng.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

 /* System table: */
#define REACTOR_SYS	0
#define BATTERIES_SYS	1
#define EDC_SYS	2
#define STRUCTINTEG_SYS 3 


/* Reactor 
  istat 0 unused
  istat 1 optimal reactor setting
  istat 2 max reactor setting
  istat 3 basepower
  istat 4 reactor type 
  istat 6 time left before engine overstresses
  istat 7 fuel effiency
  dstat 1 engine factor
  dstat 2 stress capacity
  dstat 3 current stress level
  dstat 4 current fuel level
  dstat 5 max fuel level

  tistat1 current reactor setting. 
  */

/* batteries
   istat 0 power allocated to me
   istat 1 current power in me
   istat 2 max power in me
   istat 3 release rate
   istat 4 max release rate
   istat 5 recharge rate
   dstat 1 batts bleed rate
*/

/* EDC
   istat 0 power allocated to me
   istat 1 ???
*/


struct s_Eng
{
  int Flags;
  double PowerPerComm;
  double PowerPerHelm;
  double PowerPerNav;
  double PowerPerBatts;
  double PowerPerEDC;
  t_System Systems[ESystems];

};


typedef struct s_Eng t_Eng;


/*

void VMSetSpeed();
void VMSetHeading();
void VMNavStatus();
void VMEngage();

int CanAccessNav();
*/













