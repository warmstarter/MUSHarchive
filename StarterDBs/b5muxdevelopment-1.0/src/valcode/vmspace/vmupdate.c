/* $Id: vmupdate.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
#include "header.h"
#define DBC 1
double VMInternalMax(double max, double maxpow,int VMShip, int sys);

static char *rcsvmupdatec="$Id: vmupdate.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";


void VMUpdate(void)
{
  int i=0;
  static int Orb=0;
  if(!VMRunSpace) return;
  VMCounter++;
   if(VMCounter==4) Elevator_Update();
   if(VMCounter==8) Elevator_Update();
  if(VMCounter%3==0)  {
   if(VMCounter > 9)  
    VMCounter=0;
	Orb++;
	if(Orb==720) {
		Orb=0;
		/*VMOrbits();*/
	}
   VMUpdate_Energy();
   VMProcessDamageQueue(VMDamageQ); 
   VMSBProcessDamageQueue(VMDamageSBQ); 
   VMUpdateSpecials();
   VMCounter++;
  }

  VMUpdate_Movement();
  VMUpdate_Tractors();
  VMProcessWeaponQueue(VMWeaponQ); 
  VMProcessMissleQueue(VMMissleQ); 
  VMUpdateSensors();
  /*  VMUpdateEW(); */
}




void VMUpdate_Tractors()
{
  int i=0,k;
  t_SpaceEnt *tmp;
  ResetSpaceList(VMShipsTractoring);
  while(!AtEndSpaceList(VMShipsTractoring)) {
    tmp=GetCurrSpaceEnt(VMShipsTractoring);
    if(tmp==NULL) break;
	for(k=0;k<NSystems;k++) {
	if(checkflag(VMSpace[tmp->VMShip].VMPerm.Nav.Systems[k].flags,TRACT) && VMSpace[tmp->VMShip].VMPerm.Nav.Systems[k].tistat2==1)
	VMDo_Tractor(tmp->VMShip,k);
	}
    AdvanceSpaceList(VMShipsTractoring);
  }

}

void VMUpdate_Movement(void)
{

  int i=0;
  t_SpaceEnt *tmp;


  ResetSpaceList(VMShipsMoving);
  while(!AtEndSpaceList(VMShipsMoving)) {
    tmp=GetCurrSpaceEnt(VMShipsMoving);
    if(tmp==NULL) break;
    if(VMSpace[tmp->VMShip].VMTemp.TmpNav.APON==1) 
		VMUpdate_AP(tmp->VMShip);
    VMUpdateManeuver(tmp->VMShip);
    VMMove_Ship(tmp->VMShip);
    if(VMCounter==9)
    VMSetSector(tmp->VMShip);
    AdvanceSpaceList(VMShipsMoving);
  }
  
}


void VMDo_Tractor(int VMShip,int beam)
{
t_SpaceList *tmpl;
int ship,hullt;
t_sph sph;
double factor;
int basp,basr,basf,f;
double p,siz,r;
  double a,b,c,speed,d;
double xx,yy,zz;
/* initialize all variables */

ship = 0;
hullt = 0;
factor = 0.0;
basp = 0;
basr = 0;
basf = 0;
f = 0;
p = 0.0;
r = 0.0;
siz = 0.0;
a = 0.0;
b = 0.0;
c = 0.0;
d = 0.0;
speed = 0.0;
xx = 0;
yy = 0;
zz = 0;

tmpl=VMSpace[VMShip].VMTemp.TmpNav.VMShortShips;

ship=VMSpace[VMShip].VMPerm.Nav.Systems[beam].tistat3;
VMnotify(DBC,"Start of Do_Tractor VMShip: %d ship: %d",VMShip,ship); 

if(ship==0) return;
if(!IsShipInSpaceList(tmpl,ship)) {
	VMnotifymans(VMSpace[VMShip].VMTemp.TmpNav.VMMan_List,"[COMPUTER] Ship %d no longer on sensors. Tractor beam unlocked.",ship);
	VMSpace[VMShip].VMPerm.Nav.Systems[beam].tistat3=0;
	/*VMSpace[VMShip].VMPerm.Nav.Systems[beam].tistat2=0;*/
	/*VMnotify(2,"No ship found!");*/
	return;
	}
hullt=VMSpace[VMShip].VMPerm.HullType;
siz=Hulls[hullt].Size;
basr=VMSpace[VMShip].VMPerm.Nav.Systems[beam].istat1;
basp=VMSpace[VMShip].VMPerm.Nav.Systems[beam].istat2;
f=VMSpace[VMShip].VMPerm.Nav.Systems[beam].istat3;
p=VMSpace[VMShip].VMPerm.Nav.Systems[beam].tdstat0*VMSpace[VMShip].VMTemp.TmpEng.PowerNav;
r=VMdistance(VMSpace[VMShip].VMPerm.Nav.XYZCoords, VMSpace[ship].VMPerm.Nav.XYZCoords);
VMnotify(DBC,"Do_Tractor: hullt: %d siz: %10.5f basr: %d basp: %d f: %d p: %8.3f r: %8.3f",hullt,siz,basr,basp,f,p,r);

if(basp==0 || basr==0 || siz==0) {
/*	VMnotify(2,"Zero problem basr=%d basp=%d  f=%d siz=%lf",basr,basp,f,siz);*/
	return;
	}
factor=(100.0/siz)*(p/basp)*(r/basr)*(f);
VMnotify(DBC,"factor: %20.5f",factor); 
if(r > VMSpace[VMShip].VMPerm.Nav.Systems[beam].istat4) {
	factor=0;
	VMnotify(DBC,"factor is now 0");
}
	/*VMnotify(2,"No Zero problem basr=%d basp=%d  f=%d p=%lf siz=%lf",basr,basp,f,p,siz);*/

sph=VMRelative(ship,VMShip);

  a=cos(sph.bearing*PI/180.0);
  b=cos(sph.elevation*PI/180.0);
  c=sin(sph.elevation*PI/180.0);
  d=sin(sph.bearing*PI/180.0);
  b=sqrt(b*b);
  speed=-factor;
  xx = abs(speed*b*a);
  yy = abs(speed*b*d);
  zz = abs(speed*c);
  VMnotify(DBC,"a: %10.3f b: %10.3f c: %10.3f d: %10.3f speed: %10.3f",a,b,c,d,speed);
  VMnotify(DBC,"OldX: %15.5f OldY: %15.5f OldZ: %15.5f",VMSpace[ship].VMPerm.Nav.XYZCoords.x,VMSpace[ship].VMPerm.Nav.XYZCoords.y,VMSpace[ship].VMPerm.Nav.XYZCoords.z);
  VMnotify(DBC,"xx: %15.5f yy: %15.5f zz: %15.5f",xx,yy,zz);
  if (VMSpace[ship].VMPerm.Nav.XYZCoords.x >= VMSpace[VMShip].VMPerm.Nav.XYZCoords.x) {
  VMSpace[ship].VMPerm.Nav.XYZCoords.x-= xx;
  }
  else {
  VMSpace[ship].VMPerm.Nav.XYZCoords.x+= xx;
  }

  if (VMSpace[ship].VMPerm.Nav.XYZCoords.y >= VMSpace[VMShip].VMPerm.Nav.XYZCoords.y) {
  VMSpace[ship].VMPerm.Nav.XYZCoords.y-= yy;
  }
  else {
  VMSpace[ship].VMPerm.Nav.XYZCoords.y+= yy;
  }
  if (VMSpace[ship].VMPerm.Nav.XYZCoords.z >= VMSpace[VMShip].VMPerm.Nav.XYZCoords.z) {
  VMSpace[ship].VMPerm.Nav.XYZCoords.z-= zz;
  }
  else {
  VMSpace[ship].VMPerm.Nav.XYZCoords.z+= zz;
  }
/*
  VMSpace[ship].VMPerm.Nav.XYZCoords.x+=speed*b*a;
  VMSpace[ship].VMPerm.Nav.XYZCoords.y+=speed*b*d;
  VMSpace[ship].VMPerm.Nav.XYZCoords.z+=speed*c;
*/
VMSpace[ship].VMPerm.Nav.SPHCoords=VMxyz_to_sph(VMSpace[ship].VMPerm.Nav.XYZCoords);
/*VMnotify(2,"I made it this far factor = %lf ship=%d",factor,ship);*/
  VMnotify(DBC,"=======================================");
}


void VMUpdate_AP(int VMShip)
{

double x,y,z,x2,y2,z2,h,rng,dspd,spd;
double b,e,r;
t_sph sph;

rng=VMdistance(VMSpace[VMShip].VMTemp.TmpNav.XYZCoords, VMSpace[VMShip].VMPerm.Nav.XYZCoords);

  spd=VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat2;
  dspd=VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat1;

if(rng < 5)  {
	VMSpace[VMShip].VMTemp.TmpNav.APON=0;
	VMnotifymans(VMSpace[VMShip].VMTemp.TmpNav.VMMan_List,"The ship has arrived at its destination");
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat2=0;
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat1=0;

  VMSpace[VMShip].VMPerm.Nav.XYZCoords.x=VMSpace[VMShip].VMTemp.TmpNav.XYZCoords.x;
  VMSpace[VMShip].VMPerm.Nav.XYZCoords.y=VMSpace[VMShip].VMTemp.TmpNav.XYZCoords.y;
  VMSpace[VMShip].VMPerm.Nav.XYZCoords.z=VMSpace[VMShip].VMTemp.TmpNav.XYZCoords.z;

	return;
	}
else if(rng < 20 && spd > 2) {
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat2=5;
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat1=5;
	VMnotifymans(VMSpace[VMShip].VMTemp.TmpNav.VMMan_List,"The ship slows down as it approaches its target.");
	}
else if(rng < 50 && spd > 5) {
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat2=5;
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat1=5;
	VMnotifymans(VMSpace[VMShip].VMTemp.TmpNav.VMMan_List,"The ship slows down as it approaches its target.");
	}
else if(rng < 250 && spd > 40) {
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat2=40;
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat1=40;
	VMnotifymans(VMSpace[VMShip].VMTemp.TmpNav.VMMan_List,"The ship slows down as it approaches its target.");
	}
else if(rng < 1200 && spd > 150) {
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat2=150;
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat1=150;
	VMnotifymans(VMSpace[VMShip].VMTemp.TmpNav.VMMan_List,"The ship slows down as it approaches its target.");
	}
else if(rng < 2500 && spd > 250) {
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat2=250;
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat1=250;
	VMnotifymans(VMSpace[VMShip].VMTemp.TmpNav.VMMan_List,"The ship slows down as it approaches its target.");
	}
else if(rng < 5000 && spd > 500) {
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat2=500;
  VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat1=500;
	VMnotifymans(VMSpace[VMShip].VMTemp.TmpNav.VMMan_List,"The ship slows down as it approaches its target.");
	}

	b=VMSpace[VMShip].VMPerm.Nav.Des_alpha;
	e=VMSpace[VMShip].VMPerm.Nav.Des_beta;


sph=VMRelative2(VMShip,VMSpace[VMShip].VMTemp.TmpNav.XYZCoords);

	if(fabs(sph.bearing-b) > 0.5 || fabs(sph.elevation-e) > 0.5) {
		VMSpace[VMShip].VMPerm.Nav.Des_alpha=sph.bearing;
		VMSpace[VMShip].VMPerm.Nav.Des_beta=sph.elevation;
		VMnotifymans(VMSpace[VMShip].VMTemp.TmpNav.VMMan_List,"The navigation computer corrects its course.");

	}	

}


void VMMove_Ship(int VMShip)
{
  double a,b,c,speed,d;


  a=cos(VMSpace[VMShip].VMPerm.Nav.Alpha*PI/180.0);
  b=cos(VMSpace[VMShip].VMPerm.Nav.Beta*PI/180.0);
  c=sin(VMSpace[VMShip].VMPerm.Nav.Beta*PI/180.0);
  d=sin(VMSpace[VMShip].VMPerm.Nav.Alpha*PI/180.0);
  b=sqrt(b*b);
  speed=VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat2;


  VMSpace[VMShip].VMPerm.Nav.XYZCoords.x+=speed*b*a;
  VMSpace[VMShip].VMPerm.Nav.XYZCoords.y+=speed*b*d;
  VMSpace[VMShip].VMPerm.Nav.XYZCoords.z+=speed*c;


VMSpace[VMShip].VMPerm.Nav.SPHCoords=VMxyz_to_sph(VMSpace[VMShip].VMPerm.Nav.XYZCoords);

}


void VMUpdateManeuver(int VMShip)
{
  double *tmpSetting,tmpDes_Setting,tmpMax,tmpAccel,maxpow,assigned,factor;
  double TMAX;

  /*ALPHA Updating*/
  tmpSetting=&VMSpace[VMShip].VMPerm.Nav.Alpha;
  tmpDes_Setting=VMSpace[VMShip].VMPerm.Nav.Des_alpha;
  maxpow=VMSpace[VMShip].VMPerm.Nav.Systems[2].dstat3; 
  tmpMax=VMSpace[VMShip].VMPerm.Nav.Systems[2].dstat1;
  tmpMax=VMInternalMax(tmpMax,maxpow,VMShip, 2);
  TMAX=VMSpace[VMShip].VMPerm.Nav.Systems[2].dstat1;
  if(tmpMax > TMAX) tmpMax=TMAX;


  VMUnNormalize(tmpSetting,tmpDes_Setting,360.0); 
  VMManNeedUpdate(tmpSetting,tmpDes_Setting,tmpMax);
  VMNormalize(tmpSetting,0.0,360.0);

  /*BETAUpdating*/
  tmpSetting=&VMSpace[VMShip].VMPerm.Nav.Beta;
  tmpDes_Setting=VMSpace[VMShip].VMPerm.Nav.Des_beta;
  
  VMUnNormalize(tmpSetting,tmpDes_Setting,360.0); 
  VMManNeedUpdate(tmpSetting,tmpDes_Setting,tmpMax);
  VMNormalize(tmpSetting,0.0,360.0);

  /*ROLL Updating*/
  tmpSetting=&VMSpace[VMShip].VMPerm.Nav.Roll;
  tmpDes_Setting=VMSpace[VMShip].VMPerm.Nav.Des_roll;
  VMUnNormalize(tmpSetting,tmpDes_Setting,360.0); 
  VMManNeedUpdate(tmpSetting,tmpDes_Setting,tmpMax);
  VMNormalize(tmpSetting,0.0,360.0);

  /*Speed Updating*/
  tmpSetting=&VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat2;
  tmpDes_Setting=VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat1;
  tmpAccel=VMSpace[VMShip].VMPerm.Nav.Systems[1].dstat2;
  VMManNeedUpdate(tmpSetting,tmpDes_Setting,tmpAccel);
  maxpow=VMSpace[VMShip].VMPerm.Nav.Systems[1].dstat3; 
  tmpMax=VMSpace[VMShip].VMPerm.Nav.Systems[1].dstat0;
/*VMnotify(1,"1 Max Speed = %f",tmpMax);*/

  tmpMax=VMInternalMax(tmpMax,maxpow,VMShip,1);

/*VMnotify(1,"2 Max Speed = %f",tmpMax);*/
  if(*tmpSetting > tmpMax) { 
	*tmpSetting=tmpMax;
  	VMSpace[VMShip].VMPerm.Nav.Systems[1].tdstat1=tmpMax;
	VMnotifymans(VMSpace[VMShip].VMTemp.TmpNav.VMMan_List,"[COMPUTER] Speed setting reset to current Maximum.");
	}
}

double VMInternalMax(double max, double maxpow,int VMShip,int sys)
{
double tmpMax,assigned,factor;
  if(maxpow > 0) {
	assigned=VMSpace[VMShip].VMPerm.Nav.Systems[sys].tdstat0 * VMSpace[VMShip].VMTemp.TmpEng.PowerNav;
  	factor=assigned / maxpow;
  	if(factor> 1.0) factor=1.0 + (assigned - maxpow) / (2 * maxpow);
  	if(factor> 2.0) factor=1.0 + (assigned - maxpow) / (3 * maxpow);
  	if(factor> 3.0) factor=1.0 + (assigned - maxpow) / (4 * maxpow);
  	tmpMax=max * factor;
  	}
   else
	tmpMax=0.0;
return tmpMax;
}

void VMManNeedUpdate(tmpSetting,tmpDes_Setting,tmpMax)
     double *tmpSetting,tmpDes_Setting,tmpMax;
{

  if(*tmpSetting!=tmpDes_Setting) {
    if(fabs(*tmpSetting-tmpDes_Setting) < tmpMax)
      {
	*tmpSetting=tmpDes_Setting;
	return;
      }
    if(tmpDes_Setting > *tmpSetting) 
      *tmpSetting +=tmpMax;
    else 
      *tmpSetting -=tmpMax;
  }

}
	
 


void VMUnNormalize(tmpSetting,tmpDes_Setting,angle)
     double *tmpSetting,tmpDes_Setting,angle;
{
  
  double min1,min2,min3;
  min1=fabs(*tmpSetting-tmpDes_Setting);
  min2=fabs(*tmpSetting-tmpDes_Setting+angle);
  min3=fabs(*tmpSetting-tmpDes_Setting-angle);
  
  if(min2 < min1 && min2 < min3) (*tmpSetting)+=angle;
  if(min3 < min2 && min3 < min1) (*tmpSetting)-=angle;

}
 

void VMNormalize(tmpSetting,startangle,endangle)
     double *tmpSetting,startangle,endangle;
{
  double delta1,delta2;
  int ticks;
  
  delta1=endangle-startangle;
  
  if((*tmpSetting) < startangle) {
    delta2=startangle- *tmpSetting;
    ticks=(int) delta2/delta1+1;
    (*tmpSetting)+=ticks*delta1;
  }
  else if((*tmpSetting) > endangle) {
    delta2=*tmpSetting - endangle;
    ticks=(int) delta2/delta1+1;
    (*tmpSetting)-=ticks*delta1;
  }

}	
	




void VMUpdate_Energy(void)

{
  t_SpaceEnt *tmp;
  t_SpaceEnt *tmpa;
  int curship,i;
  ResetSpaceList(VMShipsPowered);
  while(!AtEndSpaceList(VMShipsPowered)) {
    tmpa = GetCurrSpaceEnt(VMShipsPowered);
    curship = tmpa->VMShip;
ADS_CheckFire(curship);

    for(i=0;i < HSystems;i++) {
      if (checkflag(VMSpace[curship].VMPerm.Helm.Systems[i].flags,FIRED)) {
	VMSpace[curship].VMPerm.Helm.Systems[i].flags = clearflag(VMSpace[curship].VMPerm.Helm.Systems[i].flags,FIRED);
#ifdef SPACEDEBUG
	VMnotify(35,"resetting the FIRED flag on system %d on ship %d",i,curship);
#endif 
      }
    }
    AdvanceSpaceList(VMShipsPowered);
  }
  ResetSpaceList(VMShipsPowered);
  while(!AtEndSpaceList(VMShipsPowered)) {
    tmp = GetCurrSpaceEnt(VMShipsPowered);
    curship = tmp->VMShip;
    VMUpdateEnergyOne(curship);
    if(VMSpace[curship].VMTemp.TmpEng.PowerComm==0 && IsRoomInManList(Commus,curship)) {
		VMnotifymans(VMSpace[curship].VMTemp.TmpCommu.VMMan_List,"Due to lack of power, communications shut down");
		RemoveManEnt(Commus);
	}
    AdvanceSpaceList(VMShipsPowered);
  }
}





/* Planet Orbits
rc, orad = increment. very small like .0001
drad, tmd1 = 0 to 2 pi. around the orbitee
odist, organe =  range from orbitee
xt,tmp1 = orbitee shipnum
*/
void VMOrbits()
{
int i,xt;
double x,y,z,xh,yh,zh,xc,yc,zc;
float drad,rc;
float odist,ang,angg;
  for(i=0;i<=VMCurrentMax;i++) {
     xt=VMSpace[i].VMPerm.Nav.tmp1;	
   if(xt <= 0) continue;
     rc=VMSpace[i].VMPerm.Nav.orad*.5;	
     odist=VMSpace[i].VMPerm.Nav.orange;	
     drad=VMSpace[i].VMPerm.Nav.tmd1;	
   if(drad < 0 || drad > 6.28319) 
    {
     VMSpace[i].VMPerm.Nav.tmd1=0;
	
     }
     drad=VMSpace[i].VMPerm.Nav.tmd1;	
    if(drad < 1.57) {
     ang=drad/2.00000;
     }
    if(drad < 3.1459 && drad >= 1.57) {
     angg=3.1459-drad;
     ang=angg/2.00000;
      }
    if(drad < 4.71239 && drad >= 3.1459) {
     angg=drad-3.14159;
     ang=-1*angg/2.00000;
      }
    if(drad < 6.28319 && drad >= 4.71239) {
     angg=6.28319-drad;
     ang=-1*angg/2.00000;
       }

     x=VMSpace[i].VMPerm.Nav.XYZCoords.x;	
     y=VMSpace[i].VMPerm.Nav.XYZCoords.y;	
     z=VMSpace[i].VMPerm.Nav.XYZCoords.z;	
     xh=VMSpace[xt].VMPerm.Nav.XYZCoords.x;	
     yh=VMSpace[xt].VMPerm.Nav.XYZCoords.y;	
     zh=VMSpace[xt].VMPerm.Nav.XYZCoords.z;	
     xc=cos(drad)*cos(ang);
     xc=xh+(xc*odist);
     yc=sin(drad)*cos(ang);
     yc=yh+(yc*odist);
     zc=sin(drad)*sin(ang);
     zc=zh+(zc*odist);
     VMSpace[i].VMPerm.Nav.XYZCoords.x=xc;	
     VMSpace[i].VMPerm.Nav.XYZCoords.y=yc;	
     VMSpace[i].VMPerm.Nav.XYZCoords.z=zc;	
     rc=drad+rc;
     VMSpace[i].VMPerm.Nav.tmd1=rc;	
  }
 }








