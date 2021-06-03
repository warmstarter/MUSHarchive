/* $Id: vmutils.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
#include "header.h"
#include <stdarg.h>

#define BEAM 1
static char *rcsvmutilsc="$Id: vmutils.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
t_xyz VMsph_to_xyz( t_sph coord )
{
  t_xyz new_coord;
  double r = coord.range;
  double elev = coord.elevation;
  double bearing = coord.bearing;

  new_coord.x = r * cos(bearing / 180.0 * PI) * cos(elev / 180.0 * PI);
  new_coord.y = r * sin(bearing / 180.0 * PI) * cos(elev / 180.0 * PI);
  new_coord.z = r * sin(elev / 180.0 * PI);

  return( new_coord );
}


/**************************************
 *
 * xyz_to_sph()
 *
 *  input:
 *    XYZ coord               a position in rectangular format
 *
 *  purpose:
 *    converts from rectangular to spherical polar
 *
 *  return value:
 *    the spherical polar equivalent of the input value
 *
 * JMS ?? Jul 92
 * JMS 24 May 93 - general cleanup
 * MJW 31 Jul 96 - changed and added to VMSpace
 *************************************/

t_sph VMxyz_to_sph( t_xyz coord )
{
  t_sph new_coord;
  double x = coord.x;
  double y = coord.y;
  double z = coord.z;

  if( y == 0.0 )
  {
    if( x >= 0.0 ) new_coord.bearing = 0.0;
    else new_coord.bearing = 180.0;
  }
  else if( x == 0.0 )
  {
    if( y > 0.0 ) new_coord.bearing = 90.0;
    else new_coord.bearing = 270.0;
  }
  else
  {
    new_coord.bearing = atan( y / x ) * 180.0 / PI;
    if( x < 0.0 ) new_coord.bearing +=180.0;
  }

  if( z == 0.0 )
  {
    new_coord.elevation = 0.0;
  }
  else if(( x == 0.0 ) && ( y == 0.0 ))
  {
    if( z > 0 ) new_coord.elevation = 90.0;
    else new_coord.elevation = -90.0;
  }
  else
  {
    new_coord.elevation = atan( z / sqrt( x*x + y*y )) * 180.0 / PI;
  }

  new_coord.range = sqrt( x*x + y*y + z*z );

  /* bring bearing into range */
  if( new_coord.bearing < 0.0 ) new_coord.bearing += 360.0;
  if( new_coord.bearing >= 360.0 ) new_coord.bearing -=360.0;

  return( new_coord );
}


/**************************************
 *
 * distance()
 *
 *  input:
 *    XYZ a                                     point #1
 *    XYZ b                                     point #2
 *
 *  purpose:
 *    computes the distance between two points
 *
 *  return value:
 *    the distance
 *
 * JMS ?? Jul 92
 * JMS 24 May 93 - general cleanup
 * MJW 31 Jul 96 - changed and added to VMSpace
 *
 *************************************/

double VMdistance( t_xyz a, t_xyz b )
{
  double dx, dy, dz;

  dx = b.x - a.x;
  dy = b.y - a.y;
  dz = b.z - a.z;

  return( sqrt( dx*dx + dy*dy + dz*dz ));
}
#ifdef STDC_HEADERS
void VMnotifyAll( dbref target, char *message, ... )
#else
void VMnotifyAll(va_alist)
va_dcl

#endif
{
  va_list ap;
  char write_buf[10240];

#ifdef STDC_HEADERS
  va_start( ap,message );
#else
	char *message;
	dbref target;
	va_start(ap);
	target=va_arg(ap,dbref);
	message=va_arg(ap,char *);
#endif

 vsprintf( write_buf, message, ap);
  write_buf[10239] = '\0';
  va_end(ap);
  notify_all_from_inside( target,1, write_buf );
 
/*  notify_all_from_inside( target,1, message);*/
}

#ifdef STDC_HEADERS
void VMnotify( dbref target, char *message, ... )
#else
void VMnotify(va_alist)
va_dcl

#endif
{
  va_list list;
  char write_buf[10240];

#ifdef STDC_HEADERS
  va_start( list,message );
#else
	char *message;
	dbref target;
	va_start(list);
	target=va_arg(list,dbref);
	message=va_arg(list,char *);
#endif
	vsprintf( write_buf, message, list );
  write_buf[10239] = '\0';
  va_end( list );
  notify( target, write_buf );
 
/*  notify( target, message);*/
}


#ifdef STDC_HEADERS
void VMcnotify(char *message, ... )
#else
void VMcnotify(va_alist)
va_dcl

#endif

{
  va_list list;
  char write_buf[10240];

#ifdef STDC_HEADERS
  va_start( list,message );
#else
	char *message;
	va_start(list);
	message=va_arg(list,char *);
#endif 
  vsprintf( write_buf, message, list );
  write_buf[10239] = '\0';
  va_end( list );
  do_processcom(SPACEGUY,"Space", write_buf );
 
/*  do_processcom(SPACEGUY,"Space", message);*/
}

#ifdef STDC_HEADERS
void VMnotifymans( t_ManList *man, char *message, ... )
#else
void VMnotifymans(va_alist)
va_dcl

#endif
{
  t_ManEnt *tmp;
  int person;
  va_list list;
  char write_buf[10240];

#ifdef STDC_HEADERS
  va_start( list,message );
#else
	char *message;
	t_ManList *man;
	va_start(list);
	man=va_arg(list,t_ManList *);
	message=va_arg(list,char *);
#endif
	vsprintf( write_buf, message, list );
  write_buf[10239] = '\0';
  va_end( list );

  ResetManList(man);
  while(!AtEndManList(man)) {
    tmp = GetCurrManEnt(man);
    person = tmp->VMPlayer;
    notify_with_cause( person , 1, write_buf );

    /*notify( person , write_buf );*/


    /* bad notify( person , message);*/
    AdvanceManList(man);
  }

}

#ifdef STDC_HEADERS
void VMlog_space( char *message, ... )
#else
void VMlog_space(va_alist)
va_dcl

#endif

{
  va_list list;

#ifdef STDC_HEADERS
  va_start( list,message );
#else
	char *message;
	va_start(list);
	message=va_arg(list,char *);
#endif
  fprintf( stderr, "SPACE: " );
	vfprintf( stderr, message, list );
  va_end( list );
  fprintf( stderr, "\n" );

}





t_sph VMRelative(int Ship1,int Ship2)
{
t_xyz xyz1,xyz2,xyz3;
t_sph sph;

xyz2=VMSpace[Ship1].VMPerm.Nav.XYZCoords;
xyz3=VMSpace[Ship2].VMPerm.Nav.XYZCoords;

xyz1.x=xyz3.x-xyz2.x;
xyz1.y=xyz3.y-xyz2.y;
xyz1.z=xyz3.z-xyz2.z;


sph=VMxyz_to_sph(xyz1);

VMNormalize(&sph.bearing,0.0,360.0);
VMNormalize(&sph.elevation,0.0,360.0);
return (sph);
}

t_sph VMRelative2(int Ship1,t_xyz Ship2)
{
t_xyz xyz1,xyz2,xyz3;
t_sph sph;

xyz2=VMSpace[Ship1].VMPerm.Nav.XYZCoords;
/*xyz3=VMSpace[Ship2].VMPerm.Nav.XYZCoords;*/

xyz3= Ship2;

xyz1.x=xyz3.x-xyz2.x;
xyz1.y=xyz3.y-xyz2.y;
xyz1.z=xyz3.z-xyz2.z;


sph=VMxyz_to_sph(xyz1);

VMNormalize(&sph.bearing,0.0,360.0);
VMNormalize(&sph.elevation,0.0,360.0);
return (sph);
}


void VMnotifyShips(int ShipNum, char *msg)
{

  int i;
  
  for(i=0;i<=VMCurrentMax;i++) {
    if(ContactMatrix[i][ShipNum]>25) {
      VMnotifyOfContact(i,ShipNum,tprintf("Ship [%3d] %s",ShipNum,msg));
    }
    else if(ContactMatrix[i][ShipNum]>0) {
      VMnotifyOfContact(i,ShipNum,tprintf("Ship [???] %s",msg));
    }
  }

  
}


void VMShipNotifyOfFire(int src, /* originating ship */
			int target, /* excluding ship */
			int lres,
			int hres,
			int type,
			int hit
			) {
  
  int i;

  for(i=0;i<=VMCurrentMax;i++) {
    if (i == target) /* dont notify exclude guy */ 
      continue;
    switch (type) {
    case BEAM:
      if(ContactMatrix[i][target] > hres) {
	if (ContactMatrix[i][src] > hres) {
		if(hit==1) VMnotifyOfContact(i,target,tprintf("Ship [%3d] has fired upon Ship [%3d] and hit!",src,target));
		else VMnotifyOfContact(i,target,tprintf("Ship [%3d] has fired upon Ship [%3d]",src,target));
	}
	else if (ContactMatrix[i][src] > lres){
		if(hit==1) VMnotifyOfContact(i,target,tprintf("Ship [???] has fired upon Ship [%3d] and hit!",target));
		else VMnotifyOfContact(i,target,tprintf("Ship [???] has fired upon Ship [%3d].",target));
	}
      }
      else if (ContactMatrix[i][target] > lres) {
	if (ContactMatrix[i][src] > hres) {
		if(hit==1) VMnotifyOfContact(i,target,tprintf("Ship [%3d] has fired upon Ship [???] and hit!",src));
		else VMnotifyOfContact(i,target,tprintf("Ship [%3d] has fired upon Ship [???].",src));
	}
	else if (ContactMatrix[i][src] > lres){
		if(hit==1) VMnotifyOfContact(i,target,"Ship [???] has fired upon Ship [???] and hit!");
		else VMnotifyOfContact(i,target,"Ship [???] has fired upon Ship [???].");
	  }  
      }
    }
    /*break;*/
  }
}






void VMnotifyOfContact(int ShipNum,int Target,char *msg)
{
  if(checkflag(VMSpace[Target].VMPerm.Flags,SHIP)) 
    VMnotifymans(VMSpace[ShipNum].VMTemp.TmpHelm.VMMan_List,msg);
  
  if(checkflag(VMSpace[Target].VMPerm.Flags,BASE)) 
    VMnotifymans(VMSpace[ShipNum].VMTemp.TmpHelm.VMMan_List,msg);
  
  if(checkflag(VMSpace[Target].VMPerm.Flags,SPECIAL)) 
    VMnotifymans(VMSpace[ShipNum].VMTemp.TmpNav.VMMan_List,msg);
  
  if(checkflag(VMSpace[Target].VMPerm.Flags,PLANET)) 
    VMnotifymans(VMSpace[ShipNum].VMTemp.TmpNav.VMMan_List,msg);

}


int VMGetArc(int SeeShip,int SeenShip)
{
t_sph sph1,sph2;
double max,yzrange,thetayz,y,z;
t_xyz xyz;
int which,which1,which2;

sph1=VMRelative(SeenShip,SeeShip);
sph2.bearing=sph1.bearing - VMSpace[SeenShip].VMPerm.Nav.Alpha;
sph2.elevation=sph1.elevation - VMSpace[SeenShip].VMPerm.Nav.Beta;
sph2.range=sph1.range;

/*
x=r*cos(a / 180.0 * PI) * cos(b / 180.0 * PI);
y=r*sin(a / 180.0 * PI) * cos(b / 180.0 * PI);
z=r*sin(b / 180.0 * PI);
*/

xyz=VMsph_to_xyz(sph2);

yzrange=sqrt(xyz.y*xyz.y+xyz.z*xyz.z);
thetayz=atan2(xyz.y,xyz.z)*180.0 / PI;
thetayz-=VMSpace[SeenShip].VMPerm.Nav.Roll;
y=yzrange * sin(thetayz * PI / 180.0);
z=yzrange * cos(thetayz * PI / 180.0);

z*=cos(sph2.bearing)/fabs(cos(sph2.bearing));

max=xyz.x;
which1=FORE;
which2=AFT;
if(fabs(y) > fabs(max)) { max = y; which1 = PORT; which2 = STARB;}
if(fabs(z) > fabs(max)) { max = z; which1 = TOP; which2 = BOTTOM;}

if(max==0) return(which1);
if( max / fabs(max) == 1) which=which1;
else	which=which2;

return(which);

}



int VMGetArc2(t_xyz SeeShip,int SeenShip)
{
t_sph sph1,sph2;
double max,yzrange,thetayz,y,z;
t_xyz xyz;
int which,which1,which2;

sph1=VMRelative2(SeenShip,SeeShip);
sph2.bearing=sph1.bearing - VMSpace[SeenShip].VMPerm.Nav.Alpha;
sph2.elevation=sph1.elevation - VMSpace[SeenShip].VMPerm.Nav.Beta;
sph2.range=sph1.range;

/*
x=r*cos(a / 180.0 * PI) * cos(b / 180.0 * PI);
y=r*sin(a / 180.0 * PI) * cos(b / 180.0 * PI);
z=r*sin(b / 180.0 * PI);
*/

xyz=VMsph_to_xyz(sph2);

yzrange=sqrt(xyz.y*xyz.y+xyz.z*xyz.z);
thetayz=atan2(xyz.y,xyz.z)*180.0 / PI;
thetayz-=VMSpace[SeenShip].VMPerm.Nav.Roll;
y=yzrange * sin(thetayz * PI / 180.0);
z=yzrange * cos(thetayz * PI / 180.0);

z*=cos(sph2.bearing)/fabs(cos(sph2.bearing));

max=xyz.x;
which1=FORE;
which2=AFT;
if(fabs(y) > fabs(max)) { max = y; which1 = PORT; which2 = STARB;}
if(fabs(z) > fabs(max)) { max = z; which1 = TOP; which2 = BOTTOM;}

if(max==0) return(which1);
if( max / fabs(max) == 1) which=which1;
else	which=which2;

return(which);

}

void
VMKillShip(int ship) {
  int i;
  int sect;
  VMnotifyAll(VMSpace[ship].VMPerm.mainbridge,"This ship has gone poof!",ship); 

  sect=VMSpace[ship].VMTemp.TmpNav.Sector;
  if(sect > 1 && sect < 7) return;

  
  VMWipeShipContacts(ship);
  if (IsShipInSpaceList(VMShipsPowered,ship)) {
    RemoveSpaceEnt(VMShipsPowered);
  }
  if (IsShipInSpaceList(VMShipsMoving,ship)) {
    RemoveSpaceEnt(VMShipsMoving);
  }
  if(IsShipInSpaceList(VMSectors[VMSpace[ship].VMTemp.TmpNav.Sector].VMSectorShips,ship)) {
    RemoveSpaceEnt(VMSectors[VMSpace[ship].VMTemp.TmpNav.Sector].VMSectorShips);
  }
  VMSpace[ship].VMTemp.TmpNav.Sector=-1;
  
  for(i = 0;i < HSystems;i++) {
    VMSpace[ship].VMPerm.Helm.Systems[i].tistat0 = 0;
    VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 = 0;
    VMSpace[ship].VMPerm.Helm.Systems[i].tistat2 = 0;
    VMSpace[ship].VMPerm.Helm.Systems[i].tistat3 = 0;
    VMSpace[ship].VMPerm.Helm.Systems[i].tistat4 = 0;
    VMSpace[ship].VMPerm.Helm.Systems[i].tdstat0 = 0;
    VMSpace[ship].VMPerm.Helm.Systems[i].tdstat1 = 0;
    VMSpace[ship].VMPerm.Helm.Systems[i].tdstat2 = 0;
    VMSpace[ship].VMPerm.Helm.Systems[i].tdstat3 = 0;
    VMSpace[ship].VMPerm.Helm.Systems[i].tdstat4 = 0;
  }

  for(i = 0;i < NSystems;i++) {
    VMSpace[ship].VMPerm.Nav.Systems[i].tistat0 = 0;
    VMSpace[ship].VMPerm.Nav.Systems[i].tistat1 = 0;
    VMSpace[ship].VMPerm.Nav.Systems[i].tistat2 = 0;
    VMSpace[ship].VMPerm.Nav.Systems[i].tistat3 = 0;
    VMSpace[ship].VMPerm.Nav.Systems[i].tistat4 = 0;
    VMSpace[ship].VMPerm.Nav.Systems[i].tdstat0 = 0;
    VMSpace[ship].VMPerm.Nav.Systems[i].tdstat1 = 0;
    VMSpace[ship].VMPerm.Nav.Systems[i].tdstat2 = 0;
    VMSpace[ship].VMPerm.Nav.Systems[i].tdstat3 = 0;
    VMSpace[ship].VMPerm.Nav.Systems[i].tdstat4 = 0;
  }

  for(i = 0;i < ESystems;i++) {
    VMSpace[ship].VMPerm.Eng.Systems[i].tistat0 = 0;
    VMSpace[ship].VMPerm.Eng.Systems[i].tistat1 = 0;
    VMSpace[ship].VMPerm.Eng.Systems[i].tistat2 = 0;
    VMSpace[ship].VMPerm.Eng.Systems[i].tistat3 = 0;
    VMSpace[ship].VMPerm.Eng.Systems[i].tistat4 = 0;
    VMSpace[ship].VMPerm.Eng.Systems[i].tdstat0 = 0;
    VMSpace[ship].VMPerm.Eng.Systems[i].tdstat1 = 0;
    VMSpace[ship].VMPerm.Eng.Systems[i].tdstat2 = 0;
    VMSpace[ship].VMPerm.Eng.Systems[i].tdstat3 = 0;
    VMSpace[ship].VMPerm.Eng.Systems[i].tdstat4 = 0;
  }

  VMSpace[ship].VMTemp.TmpEng.PowerAvail = 0;
  VMSpace[ship].VMTemp.TmpEng.PowerNav = 0;
  VMSpace[ship].VMTemp.TmpEng.PowerHelm = 0;
  VMGetShipOutDQ(ship);
  VMSpace[ship].VMPerm.Flags = setflag(VMSpace[ship].VMPerm.Flags,DEAD);
}



int 
VMSCheck(int player,
	 int skill,
	 int diff,
	 int area
	 ) {
  /* for now, just wrap skill_check */
  if (skill != 0)
    return (skill_check(player,skill,diff));
  else
    return 0;
}

  











