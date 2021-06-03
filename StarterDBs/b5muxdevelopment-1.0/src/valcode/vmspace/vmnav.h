/* $Id: vmnav.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

struct s_xyz {
  double x;
  double y;
  double z;
};

struct s_sph {
   double bearing;
   double elevation;
   double range;
}; 

typedef struct s_xyz t_xyz;
typedef struct s_sph t_sph;

struct s_Nav
{
  t_xyz XYZCoords;
  t_sph SPHCoords;
  int powersys;
  /*double Speed,Des_speed;*/
  double Alpha,Beta,Des_alpha,Des_beta,Roll,Des_roll;
/*  double Max_Speed,Manrate,Acceleration;*/
  int Dock;
  int Flags;
  int HullType;
  int orbit;
  double orange,orad;
  int tmp1,tmp2;
  double tmd1,tmpd2;
  t_System Systems[NSystems];
};


typedef struct s_Nav t_Nav;



