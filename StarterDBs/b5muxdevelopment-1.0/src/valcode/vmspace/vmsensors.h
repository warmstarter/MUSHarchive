/* $Id: vmsensors.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

#define FIRSTSECTOR 7

int ContactMatrix[MAXSHIP][MAXSHIP];
int VMMaxSector;


struct s_Sector {
  int num;
  char *Name;
  double X,Y,Z;
  double Range;
  t_SpaceList *VMSectorShips; 
  t_SpaceList *VMSectorSpecials; 
  t_MineList *VMMines;

};


struct s_Sensors
{
	int MaxLongRange;
	int MaxShortRange;
	double BaseNumber;
	double Threshold;
	double Damage;
};


typedef struct s_Sensors t_Sensors;
typedef struct s_Sector t_Sector;


extern t_Sector VMSectors[];
