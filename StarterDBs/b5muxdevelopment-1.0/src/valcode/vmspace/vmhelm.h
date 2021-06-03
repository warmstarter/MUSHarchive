/* $Id: vmhelm.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

#define GRAP 0

/* system mappings */
/* typical gun 

   istat 0 
   istat 1 max range to hit
   istat 2 arcs i can get
   istat 3 arc im mounted in
   istat 4 Max charge
   istat 5 acceptance
   istat 6 plasma effective range 
   istat 7 min size i can hit w/o penalty
   istat 8
   istat 9

   dstat 0 bleed
   dstat 1 
   dstat 2 
   dstat 3 
   dstat 4 
   dstat 5 
   dstat 6 
   dstat 7 
   dstat 8 
   dstat 9 

   tistat0 power allocated to me
   tistat1 charge in me
   tistat3 is my state. 
   tdstat0 percent power allocated to me

   basenumber max charge

   
  */
struct s_Helm
{
  /* How many of each missle we have */

  int Missles[NMISSLES];
  int powersys;
  int numweaps; /* this is the last weapon number in the system array */
  int ecm;
  int eccm;
  t_System Systems[HSystems];

};


typedef struct s_Helm t_Helm;

struct s_Missle {
  char *name;
  int id; /* where in the array we are */
  int make;
  int flags;
  int basedam;
  int speed;
  int realrng;
  int acrrng;
  int afxrng;
};

typedef struct s_Missle t_Missle;

/* Location 1-10 reserved for weapons, location 11 is EW, location 12 is TAC, location 13 is reserved */


