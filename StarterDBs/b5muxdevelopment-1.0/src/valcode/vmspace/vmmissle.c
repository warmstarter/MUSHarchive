#include "header.h"
static char *idmisslec="$Id: vmmissle.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
t_missle AllMissles[]= {
  {(char *) NULL,-1,0,0,
   0,0,0,0,0,0,0,0,0},
  {"Class-Y Missile",1,1113,5,
   2400,250,1.5,50,0,0,1,60,0,-4,60},
  {"Fighter Missile",2,1115,7,
   3000,150,1.5,50,0,0,1,80,0,-4,80},
  {"Leech Missile",3,1017,10,
   3000,150,1.5,50,0,0,4,200,STASIS,0,0},
  {"Mine 1",4,0,10,100,0,0,0,0,0,2,300,MINE,0,0},
  {"Light Missile",5,1117,10,
   6000,150,1.5,50,0,0,4,150,0,-5,150},
  {"Anti Fighter Missile",6,1157,9,
   4500,150,1.5,50,0,0,4,150,0,-5,150},
  {"Long Range Missile",7,1123,10,
   9000,150,1.5,50,0,0,2,200,0,-5,200},
  {"Basic Missile",8,1403,10,
   6000,150,1.5,50,0,0,4,250,0,-5,250},
  {"Piercing Missile",9,1225,10,
   6000,150,1.5,50,0,0,4,250,0,-10,250},
  {"Flash Missile",10,1119,10,
   6000,150,1.5,50,0,0,4,250,0,-2,250},
  {"Heavy Missile",11,1121,10,
   3000,150,1.5,50,0,0,4,350,0,-5,350},
  {"250 GW Ion Torpedo",12,1407,14,
   5000,150,1.5,50,0,0,4,250,0,-5,250},
  {"300 GW Ion Torpedo",13,1409,16,
   6000,150,1.5,50,0,0,4,300,0,-5,300},
  {"130 GW Ballistic Torpedo",14,351,9,
   2500,150,1.5,50,0,0,4,130,0,-5,130},
  {"240 GW Bomb",15,395,8,
   2000,150,1.5,50,0,0,4,240,0,-6,240},
  {"200 GW Packet Torpedo",16,501,12,
   5000,150,1.5,50,0,0,4,200,0,-5,200},
  {"200 MW Energy Mine",17,1395,9,
   5000,150,1.5,50,0,0,5,200,0,-2,200},
  {"300 MW Energy Mine",18,1397,14,
   6500,150,1.5,50,0,0,5,300,0,-2,300},
  {"350 MW Energy Mine",19,1399,15,
   6000,150,1.5,50,0,0,5,350,0,-2,350},
  {"Super-Heavy Missile",20,468,12,
   3000,150,1.5,50,0,0,4,350,0,-6,540},
};





