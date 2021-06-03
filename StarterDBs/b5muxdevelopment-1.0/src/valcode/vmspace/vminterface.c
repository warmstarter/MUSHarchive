/* $Id: vminterface.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
#include "header.h"
static char *rcsvminterfacec="$Id: vminterface.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
void do_VMADS(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[3];
  int i,ship,dship,bay;
  double x,y,z;
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"Useage: ADS <ON/OFF>");
    return;
  }
  ship=VMShipNum(player);


  if(ship < 0 || ship > VMCurrentMax) {
    VMnotify(player,"You are on an Invalid Ship");
    return;
  }

  if(strcmp(fargs[0],"ON")==0) ADS_Level(player,ship,2);
  else if(strcmp(fargs[0],"OFF")==0) ADS_Level(player,ship,0);
  else {
	VMnotify(player,"Valid settings are ON and OFF.");
	return;
	}
	
 
}

void do_VMSBFuel(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ShipNum,i,ship,amount;
  char *args[3];

  ShipNum=VMShipNum(player);
  if (!IsIcReg(player))
    return;
  i = xcode_parse(buffer,args,2);
  if ( i != 2 ) {
    VMnotify(player,"usage: fuel [ship] [amount]");
    return;
  }
  ship=atoi(args[0]);
  if(ship < 0 || ship > VMCurrentMax) {
    VMnotify(player,"Invalid Ship");
    return;
  }
  amount=atoi(args[1]);
  if(amount < 1) {
	VMnotify(player,"Invalid amount");
	return;
}

  VMSBFuel(player,ShipNum,ship,amount);
}

void do_VMSBDamStat(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ShipNum;
  ShipNum=VMShipNum(player);
  if (!IsIcReg(player))
    return;

  VMSBDamStat(player,ShipNum);
}

void do_VMSBSysStat(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[3];
  int i,ShipNum,ship2;

  if (!IsIcReg(player))
    return;

  ShipNum=VMShipNum(player);

  i = xcode_parse(buffer,args,1);
  if ( i < 1 ) {
    VMnotify(player,"usage: sstat [ship]");
    return;
  }
  ship2=atoi(args[0]);
  if(ship2 < 0 || ship2 > VMCurrentMax) {
    VMnotify(player,"Invalid Ship");
    return;
  }
  VMSBSysStat(player,ship2,ShipNum);
}



void do_VMSBMiscRepairSystem(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[4];
  int i;
  int ShipNum;
  if (!IsIcReg(player))
    return;
  i = xcode_parse(buffer,args,4);
  if ( i < 3 ) {
    VMnotify(player,"usage: repair [ship] [team] [hull] | [ship] [team] [area] [system]");
    return;
  }
  if (i == 3) {
    if (!strcmp(args[2],"hull")) {
      VMSBMiscRepairHull(player,atoi(args[0]),atoi(args[1]));
      return;
    }
    else {
      VMnotify(player,"usage: repair [ship] [team] [hull] | [ship] [area] [system]");
      return;
    }
  }
  if (i == 4) {
    VMSBMiscRepair(player,atoi(args[0]),atoi(args[1]),args[2],atoi(args[3]));
  }
  else {
    VMnotify(player,"usage: repair [ship] [team] [hull] | [ship] [team] [area] [system]");
    return;
  }
}

void do_VMSect(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[7];
  int i,ship,sect;

  i=xcode_parse(buffer,fargs,2);
  if(i!=2 ) {
	notify(player,"Usage: SECT <SHIPNUM> <SECTOR #>");
	return;
	}
 ship=atoi(fargs[0]);
 sect=atoi(fargs[1]);
  
  if(ship < 0 || ship > VMCurrentMax) {
	VMnotify(player,"Invalid Ship #");
	return;
	}
  if(sect < 0 || sect > FIRSTSECTOR ) {
	VMnotify(player,"Invalid Sector");
	return;
	}

VMSpace[ship].VMTemp.TmpNav.Sector=sect;
notify(player,"Ship placed in sector");
}


void do_VMHelm(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[7];
  int i,ship,tem,bdb,aidb,aodb;

  i=xcode_parse(buffer,fargs,6);
  if(i!=6 && i!=3) {
    VMnotify(player,"Usage: HELM <SHIPNUM> <TEMPLATE#> <BRIDGE DB> <AIRIN DB> <AIROUT DB> <NAME> or HELM <SHIPNUM> <TEMPLATE #> <NAME>");
	return;
	}

  ship=atoi(fargs[0]);
  tem=atoi(fargs[1]);
if(i==6) {
  bdb=atoi(fargs[2]);
  aidb=atoi(fargs[3]);
  aodb=atoi(fargs[4]);
  do_make_ship2(tem,ship,bdb,aidb,aodb,fargs[5]);
	}
else {
  bdb=1;
  aidb=0;
  aodb=0;
  do_make_ship2(tem,ship,bdb,aidb,aodb,fargs[2]);
	}
}


void do_VMCommCCode(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[2];
  int i,ship;

  i=xcode_parse(buffer,fargs,2);
  if(i!=2) {
    VMnotify(player,"Usage: CCODE <CHANNEL> <CODE>");
	return;
	}

  ship=VMShipNum(player);
  VMCommCCode(player,ship,atoi(fargs[0]),fargs[1]);

}

void do_VMCommDCode(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[2];
  int i,ship;

  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"Usage: DCODE <CHANNEL>");
	return;
	}

  ship=VMShipNum(player);
  VMCommDCode(player,ship,atoi(fargs[0]));
}

void do_VMCommUnRLock(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ShipNum;


  ShipNum=VMShipNum(player);
  if (!CanAccessComm(ShipNum,player))
    return;

  VMUnRLock(player,ShipNum);
}

void do_VMCommRLock(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[2];
  int i,ShipNum,ship;

  ShipNum=VMShipNum(player);
  if (!CanAccessComm(ShipNum,player))
    return;

  i=xcode_parse(buffer,fargs,2);
  if(i!=2) {
    VMnotify(player,"Usage: RLOCK <SHIP#> <RELAY #>");
    return;
  }
  ship=atoi(fargs[0]);
  if(ship < 0 || ship > VMCurrentMax) {
	VMnotify(player,"Invalid Ship #");
	return;
	}

  VMRLock(player,ShipNum,ship,atoi(fargs[1]));
}

void do_VMCommScan(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[1];
  int i,ShipNum;
  ShipNum=VMShipNum(player);
  i=xcode_parse(buffer,fargs,1);
  if (!CanAccessComm(ShipNum,player))
    return;
  if(i!=1) {
    VMnotify(player,"Usage: COSCAN <SHIP #>");
    return;
  }

  VMCommScan(player,ShipNum,atoi(fargs[0]));
}


void do_VMCommTran(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ship;
  char *fargs[5];
  char *msg[500];
  int i,ShipNum;
  
  ship=VMShipNum(player);
if (!CanAccessComm(ship,player))
    return;
if(strlen(buffer) < 4) {
	VMnotify(player,"Null message.");
	return;
	}
if(strlen(buffer) > 400) {
	VMnotify(player,"Message too long.");
	return;
	}
sprintf(msg,"%s",&buffer[2]);
  i=xcode_parse(buffer,fargs,1);
  if(i < 1) {
	VMnotify(player,"Usage: TRAN <CHANNEL> <MSG>"); 
	return;
	}

/*  VMnotify(player,"Soon. Very Soon");*/
  VMCommTran(player,ship,atoi(fargs[0]),msg);
}

void do_VMCommOff(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ship;

  ship=VMShipNum(player);
  VMCommOff(player,ship);
}

void do_VMCommOn(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ship;

  ship=VMShipNum(player);
  VMCommOn(player,ship);
}



void do_VMCommStatus(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ship;

  ship=VMShipNum(player);
  VMCommStatus(player,ship);
}

void do_VMCommClose(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[3];
  int i,ship;
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"Usage: CLOSE <CHANNEL>");
	return;
	}
  ship=VMShipNum(player);
   VMCommClose(player,ship,atoi(fargs[0]));
}

void do_VMCommOpen(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[3];
  int i,ship;
  i=xcode_parse(buffer,fargs,2);
  if(i!=2) {
    VMnotify(player,"Usage: OPEN <CHANNEL> <FREQUENCY>");
	return;
	}
  ship=VMShipNum(player);
   VMCommOpen(player,ship,atoi(fargs[0]),atof(fargs[1]));
}



void do_VMOrbits(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{

VMOrbits();
VMnotify(player,"And the planets spin, wheeeee.");
}

void do_VMDORB(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  t_xyz xyz;
  t_sph sph;
  char *fargs[5];
  int i,ship,dship,bay;
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"DORB requires 1 argument");
	return;
	}

  ship=atoi(fargs[0]);
  if(ship < 0 || ship > VMCurrentMax) {
    VMnotify(player,"Invalid Ship");
    return;
  }
 VMSpace[ship].VMPerm.Nav.tmp1=0;

  VMnotify(player,"Orbit Removed");
}

void do_VMOrbitInstall(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  t_xyz xyz;
  t_sph sph;
  char *fargs[5];
  int i,ship,dship,bay;
  i=xcode_parse(buffer,fargs,5);
  if(i!=5) {
    VMnotify(player,"VMOINS requires 5 arguments");
	return;
	}

  ship=atoi(fargs[0]);
  if(ship < 0 || ship > VMCurrentMax) {
    VMnotify(player,"Invalid Ship");
    return;
  }

  VMSpace[ship].VMPerm.Nav.orad=atof(fargs[3]);
  VMSpace[ship].VMPerm.Nav.orange=atof(fargs[2]);
  VMSpace[ship].VMPerm.Nav.tmd1=atof(fargs[4]);
  VMSpace[ship].VMPerm.Nav.tmp1=atoi(fargs[1]);

  VMnotify(player,"Orbit placed");

}
void do_VMPlace(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  t_xyz xyz;
  t_sph sph;
  char *fargs[5];
  int i,ship,dship,bay;
  i=xcode_parse(buffer,fargs,4);
  if(i!=4) {
    VMnotify(player,"PLACE requires 4 arguments");
    return;
  }
  ship=atoi(fargs[0]);
  if(ship < 0 || ship > VMCurrentMax) {
    VMnotify(player,"Invalid Ship");
    return;
  }


  xyz.x=(double)atof(fargs[1]);
  xyz.y=(double)atof(fargs[2]);
  xyz.z=(double)atof(fargs[3]);

  sph=VMxyz_to_sph(xyz);
  VMSpace[ship].VMPerm.Nav.XYZCoords=xyz;
  VMSpace[ship].VMPerm.Nav.SPHCoords=sph;
  VMnotify(player,"Object Placed");

}

void do_VMAPOff(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[3];
  int i,ship,dship,bay;
  double x,y,z;

  ship=VMShipNum(player);

  if(ship < 0 || ship > VMCurrentMax) {
    VMnotify(player,"You are on an Invalid Ship");
    return;
  }
  if (!CanAccessNav(ship,player))
    return;

VMSpace[ship].VMTemp.TmpNav.APON=0;
    VMnotify(player,"Autopilot Disengaged");

}

void do_VMDoAP(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[3];
  int i,ship,dship,bay;
  double x,y,z;
  i=xcode_parse(buffer,fargs,3);
  if(i!=3) {
    VMnotify(player,"Useage: AP <X> <Y> <Z>");
    return;
  }

  ship=VMShipNum(player);

  if(ship < 0 || ship > VMCurrentMax) {
    VMnotify(player,"You are on an Invalid Ship");
    return;
  }
 
  x=atof(fargs[0]);
  y=atof(fargs[1]);
  z=atof(fargs[2]);

  VMAP(player,ship,x,y,z);
}


void do_VMDock(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[2];
  int i,ship,dship,bay;
  i=xcode_parse(buffer,fargs,2);
  if(i!=2) {
    VMnotify(player,"Dock requires 2 arguments");
    return;
  }

  dship=atoi(fargs[0]);
  bay=atoi(fargs[1]);
  ship=VMShipNum(player);

  if(ship < 0 || ship > VMCurrentMax) {
    VMnotify(player,"Invalid Ship");
    return;
  }
  
  VMDock(player,ship,dship,bay);
} 




void do_VMUnDock(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ship;

  ship=VMShipNum(player);
  
  VMUnDock(player,ship);
}



void do_VMSecMan(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[1];
  int i;
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"Man requires 1 argument");
    return;
	}
	if(!strncmp(fargs[0],"sec",3)) {
		VMSecMan(player);
		return;
	}
	VMnotify(player,"Invalid Console");
}


void do_VMSecLogin(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[2];
  int i;

  i=xcode_parse(buffer,fargs,2);
  if(i!=2) {
    VMnotify(player,"SecTest requires 2 arguments");
    return;
	}

 	VMSecLogin(player,fargs[0],fargs[1]);
}

void do_VMSecRStat(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
 	VMSecRStat(player);
}

void do_VMSecLogout(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
 	VMSecLogout(player);
}



void do_VMSecTest(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[3];
  int i;

  i=xcode_parse(buffer,fargs,3);
  if(i!=3) {
    VMnotify(player,"SecTest requires 3 arguments");
    return;
	}

	VMSecTest(player,fargs[0],fargs[1],atoi(fargs[2]));
}




void do_VMSecView(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{

VMSecView(player);
}


void do_VMSecAdd(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[7];
  int i;

  i=xcode_parse(buffer,fargs,7);
  if(i!=7) {
    VMnotify(player,"SecAdd requires 7 arguments");
    return;
	}

	VMSecAdd(player,atoi(fargs[0]),fargs[1],fargs[2],atoi(fargs[3]),atoi(fargs[4]),atoi(fargs[4]),atoi(fargs[5]),atoi(fargs[6]));
}




int VMShipNum(player)
     int player;
{
  return(atoi(vget_a(Zone(where_is(player)),SHIP_A)));
}



void do_VMSetupSpecials(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  VMSetupSpecials();
  
}

void do_VMSetupRooms(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  VMSetupRooms();
  VMnotify(player,"Rooms setup.");
}

void do_VMRehelm(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  VMRehelm(player);
  notify(player,"All ship armor rehelmed!");
}
void do_VMInitSpace(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  VMInitSpace();
  VMnotify(player,"Space Initialized.");
}


void do_VMShowShip(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[1];
  int i;

  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"ShowShip requires one arguments");
    return;
  }

  VMShowShip(player,atoi(fargs[0]));
}

void do_VMSciScan(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[1];
  int i,ShipNum;
  ShipNum=VMShipNum(player);
  i=xcode_parse(buffer,fargs,1);
  if (!CanAccessSci(ShipNum,player))
    return;
  if(i!=1) {
    VMnotify(player,"sciscan requires one arguments");
    return;
  }

  VMSciScanObject(player,ShipNum,atoi(fargs[0]));
}


void do_VMSeeFlag(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[1];
  int i;

  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"VMSee requires one arguments");
    return;
	}

 VMSeeFlag(player,atoi(fargs[0]));
}

void do_VMShowSpaceAll(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  VMShowSpaceAll(player);
}

void do_VMFlag(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[2];
  int i;

  i=xcode_parse(buffer,fargs,2);
  if(i!=2) {
    VMnotify(player,"VMFlag requires two arguments");
    return;
   }

  VMFlag(player,atoi(fargs[0]),fargs[1]);
}



void do_VMMan(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[2];
  int i,ShipNum;
  double speed;
  
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"MAN requires one argument");
    return;
  }


  ShipNum=VMShipNum(player);
  VMMan(player,ShipNum,fargs[0]);
}

void do_VMUnMan(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[2];
  int i,ShipNum;
  double speed;
  
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"MAN requires one argument");
    return;
  }


  ShipNum=VMShipNum(player);
  VMUnMan(player,ShipNum,fargs[0],1);
}


void do_VMSave(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  VMSave();
  VMnotify(player,"Space saved.");


}


void do_VMDisEngage(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int i,ShipNum;
  char *fargs[2];
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"Usage: TDISENGAGE <BEAM SYSTEM #>");
    return;
	}
  ShipNum=VMShipNum(player);
  VMDisEngage(player,ShipNum,atoi(fargs[0]));

}

void do_VMTEngage(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int i,ShipNum;
  char *fargs[2];
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"Usage: TENGAGE <BEAM SYSTEM #>");
    return;
	}
  ShipNum=VMShipNum(player);
  VMTEngage(player,ShipNum,atoi(fargs[0]));

}
void do_VMTUnLock(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int i,ShipNum;
  char *fargs[2];
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"Usage: TUNLOCK <BEAM SYSTEM #>");
    return;
	}
  ShipNum=VMShipNum(player);
  VMTUnLock(player,ShipNum,atoi(fargs[0]));
}

void do_VMTDisEngage(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int i,ShipNum;
  char *fargs[2];
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"Usage: TDISENGAGE <BEAM SYSTEM #>");
    return;
	}
  ShipNum=VMShipNum(player);
  VMTDisEngage(player,ShipNum,atoi(fargs[0]));
}

void do_VMTLock(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int i,ShipNum;
  char *fargs[2];
  double speed;
  
  i=xcode_parse(buffer,fargs,2);
  if(i!=2) {
    VMnotify(player,"Usage: TLOCK <BEAM SYSTEM #> <SHIP #>");
    return;
  }
  ShipNum=VMShipNum(player);
  VMTLock(player,ShipNum,atoi(fargs[0]),atoi(fargs[1]));
}

void do_VMChart(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int i,ShipNum;
  char *fargs[2];
  double speed;
  
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"Usage: CHART <RANGE>");
    return;
  }
  ShipNum=VMShipNum(player);
  VMChart(player,ShipNum,atof(fargs[0]));
}

void do_VMEngage(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int i,ShipNum;
  ShipNum=VMShipNum(player);
  VMEngage(player,ShipNum);
}

void do_VMSetSpeed(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[2];
  int i,ShipNum;
  double speed;
  
  i=xcode_parse(buffer,fargs,1);

  if(i!=1) {
    VMnotify(player,"Speed requires one Argument");
    return;
  }

  ShipNum=VMShipNum(player);
  
  speed=atof(fargs[0]);
  VMSetSpeed(player,ShipNum,speed);

}

void do_VMNavJump(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
int i,ShipNum;
  
 ShipNum=VMShipNum(player);
  if (!CanAccessNav(ShipNum,player))
    return;
 
  VMNavJump(player,ShipNum);
}

void do_VMPoint(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
int i,ShipNum,Ship;
  char *fargs[2];
  
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"Useage: JPOINT <Jumppoint #>");
    return;
  }
 Ship=atoi(fargs[0]);
 ShipNum=VMShipNum(player);
  if (!CanAccessNav(ShipNum,player))
    return;
 
  VMPoint(player,ShipNum,Ship);

}

void do_VMGate(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
int i,ShipNum,Ship;
  char *fargs[2];
  
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"Useage: GATE <Jumpgate #>");
    return;
  }

 Ship=atoi(fargs[0]);
  if(Ship < 0 || Ship > VMCurrentMax) {
    VMnotify(player,"Invalid Jumpgate");
    return;
  }

 ShipNum=VMShipNum(player);
  if (!CanAccessNav(ShipNum,player))
    return;
 
  VMGate(player,ShipNum,Ship);
}

void do_VMADoor(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
int i,ShipNum,dship,bay,ship,code;
char *fargs[2];
  
  i=xcode_parse(buffer,fargs,3);
  if(i!=3) {
		notify(player,"Useage: ADOOR <SHIPNUM> <BAY> <CODE>");
		return;
	}
  dship=atoi(fargs[0]);
  bay=atoi(fargs[1]);
  code=atoi(fargs[2]);
  if(code<=0) {
	notify(player,"Invalid Code");
	return;
	}
  ship=VMShipNum(player);

  if(ship < 0 || ship > VMCurrentMax) {
    VMnotify(player,"Invalid Ship");
    return;
  }
  if(dship < 0 || dship > VMCurrentMax) {
    VMnotify(player,"Invalid Ship");
    return;
  }
  
VMADoor(player,ship,dship,bay,code);
}

void do_VMScanShip(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
int i,ShipNum,Ship;
  char *fargs[2];
  
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"Scan requires 1 argument");
    return;
  }

 Ship=atoi(fargs[0]);
  if(Ship < 0 || Ship > VMCurrentMax) {
    VMnotify(player,"Invalid Ship to Scan");
    return;
  }

 ShipNum=VMShipNum(player);
  if (!CanAccessNav(ShipNum,player))
    return;
 
  VMScanShip(player,ShipNum,Ship);
}

void do_VMScan2(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
int ShipNum;

 ShipNum=VMShipNum(player);
  if (!CanAccessNav2(ShipNum,player) && !CanAccessHelm2(ShipNum,player))
	{
	notify(player,"You must man the nav or helm console.");
    return;
	}
 
  VMScanBrief(player,ShipNum);
}

void do_VMScan(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
int ShipNum;

 ShipNum=VMShipNum(player);
  if (!CanAccessNav(ShipNum,player))
    return;
 
  VMScan(player,ShipNum);
}



void do_VMSetHeading(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[3];
  int i,ShipNum;
  double alp,bet,rol;
  i=xcode_parse(buffer,fargs,3);
  if(i!=3) {
    VMnotify(player,"Set Heading requires THREE arguments");
    return;
  }


  ShipNum=VMShipNum(player);

  alp=atof(fargs[0]);
  bet=atof(fargs[1]);
  rol=atof(fargs[2]);

  VMSetHeading(player,ShipNum,alp,bet,rol);


}


void do_VMNavBays(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  double x,y,z,speed;
  char *fargs[1];
  int ShipNum;
  int i,ship,dship,bay;
  i=xcode_parse(buffer,fargs,1);
  if(i!=1 && i!=0) {
    VMnotify(player,"BAYS requires 0 or 1 arguments");
    return;
	}

  ShipNum=VMShipNum(player);
if(i==1)
  VMNavShowBays(player,ShipNum,atoi(fargs[0]));

else  
VMNavBays(player,ShipNum,0);


}


void do_VMNavStatus(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  double x,y,z,speed;
  int ShipNum;
  ShipNum=VMShipNum(player);
  VMNavStatus(player,ShipNum);
}

void do_VMListHulls(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  double x,y,z,speed;
  int ShipNum;
  VMListHulls(player);
}

void do_VMListSystems(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  double x,y,z,speed;
  int ShipNum;
  VMListSystems(player);
}


/* eng commands*/
void do_VMEngStatus(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ShipNum;
  
  ShipNum=VMShipNum(player);
  if (!CanAccessEng(ShipNum,player))
    return;
  VMEngStatus(player,ShipNum);
}

void do_VMEngStartReactor(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (!CanAccessEng(ShipNum,player))
    return;
  VMEngStartReactor(player,ShipNum);
}

void do_VMEngStopReactor(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (!CanAccessEng(ShipNum,player))
    return;
  VMEngStopReactor(player,ShipNum);
}


  
void do_VMEngSetAllocs(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[5];
  int nargs;
  int i;
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (xcode_parse(buffer,args,5) != 5 ) {
    VMnotify(player,"usage: ea <helm> <nav> <comm> <batts> EDC");
    return;
  }
  if (!CanAccessEng(ShipNum,player))
    return;
  for (i = 0; i < 5;i++) { 
	if (args[i] < 0) { 
		VMnotify(player,"[Computer] Allocations can not be less than 0.");
		return;
	}
  }
  VMEngSetAllocs(player,ShipNum,atof(args[0]),atof(args[1]),atof(args[3]),atof(args[4]),atof(args[2]));
}


void do_VMEngSetReactor(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[1];
  int nargs;
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (xcode_parse(buffer,args,1) != 1 ) {
    VMnotify(player,"usage: sr <per>");
    return;
  }
  if (!CanAccessEng(ShipNum,player))
    return;
    
  VMEngSetReactor(player,ShipNum,atoi(args[0]));
}




void do_VMEngBatts(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[1];
  int nargs;
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (!CanAccessEng(ShipNum,player))
    return;
  if (xcode_parse(buffer,args,1) != 1 ) {
    VMnotify(player,"usage: batteries off|on");
    return;
  }
  if (!strncmp(args[0],"off",3))
    {
      VMEngBattsOff(player,ShipNum);
    }
  else if (!strncmp(args[0],"on",2))
    {
      VMEngBattsOn(player,ShipNum);
    }
  else
    {
      VMnotify(player,"usage: batteries off|on");
    }
}


void do_VMSafeties(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[1];
  int nargs;
  int ship;
  ship = VMShipNum(player);
  if (!CanAccessEng(ship,player))
    return;
  if (xcode_parse(buffer,args,1) != 1) {
    VMnotify(player,"usage: safeties <online|offline>");
    return;
  }
  if (strncmp(args[0],"on",2)) {
    VMEngSafety(player,ship,2);
    return;
  }
  else if (strncmp(args[0],"off",3)) {
    VMEngSafety(player,ship,1);
     return;
  }
  else
    VMnotify(player,"usage: safeties <online|offline>");
}

void do_VMNavAlloc(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[MAXSYSTEMS+1];
  int nargs,ship;
  ship = VMShipNum(player);
  if (!CanAccessNav(ship,player))
    return;
  nargs = xcode_parse(buffer,args,MAXSYSTEMS);
  if (nargs > VMSpace[ship].VMPerm.Nav.powersys || nargs < 0) {
    VMnotify(player,"Kosh says no.");
    return;
  }
  
  VMNavAlloc(player,ship,nargs,args);
}

void do_VMHelmStatus(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ShipNum;
  ShipNum=VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  VMHelmStatus(player,ShipNum);
}

void do_VMHelmLockWeapon(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[3];
  int i,target,weapon;
  int ShipNum;
  i = xcode_parse(buffer,args,2);
  if ( i < 2 ) {
    VMnotify(player,"usage: lock <weapon> <target> ");
    return;
  }
  ShipNum=VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  weapon = atoi(args[0]);
  target = atoi(args[1]);
  VMHelmLockWeapon(ShipNum,player,target,weapon);
}

void do_VMHelmAlloc(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[MAXSYSTEMS+1];
  int nargs,ship;
  ship = VMShipNum(player);
  if (!CanAccessHelm(ship,player))
    return;
  nargs = xcode_parse(buffer,args,MAXSYSTEMS);
  if (nargs > VMSpace[ship].VMPerm.Helm.powersys || nargs < 0) {
    VMnotify(player,"Kosh says no.");
    return;
  }
  
  VMHelmAlloc(player,ship,nargs,args);
}

void do_VMHelmWeaponState(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
	char *args[2];
	int i,weap,ship;
	i = xcode_parse(buffer,args,2);
	if ( i != 2 )
	{
		VMnotify(player,"usage: weapon <num> [on|off[line]]");
		return;
	}
	weap = atoi(args[0]);
	ship = VMShipNum(player);
  	if (!CanAccessHelm(ship,player))
    		return;
	VMWeaponChangeState(ship,player,args[1],weap);
}		

void do_VMHelmGunPort(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
	char *args[2];
	int i,weap,ship;
	i = xcode_parse(buffer,args,2);
	if ( i != 2 )
	{
		VMnotify(player,"usage: gunport <num> [on|off[line]]");
		return;
	}
	weap = atoi(args[0]);
	ship = VMShipNum(player);
  	if (!CanAccessHelm(ship,player))
    		return;
	VMHelmGunPort(ship,player,weap,args[1]);
}		

void do_VMSetFireGun(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
	char *args[2];
	int i,weap,ship;
	i = xcode_parse(buffer,args,2);
	if ( i != 2 )
	{
		VMnotify(player,"Usage: setg <systemnum> [NORMAL|SWEEP|POINT|DISPER]");
		return;
	}
	weap = atoi(args[0]);
	ship = VMShipNum(player);
  	if (!CanAccessHelm(ship,player))
    		return;
	VMSetFireGun(player,ship,weap,args[1]);
}		


void do_VMHelmFireWeapon(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[3];
  int i,target,weapon;
  int ShipNum;
  i = xcode_parse(buffer,args,1);
  if ( i < 1 ) {
    VMnotify(player,"usage: fire <weapon> ");
    return;
  }
  ShipNum=VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  weapon = atoi(args[0]);
  VMHelmFireWeapon(ShipNum,player,weapon);
}

void do_VMSetMainBridge(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int i,ShipNum,Ship,MB;
  char *fargs[3];
  
  i=xcode_parse(buffer,fargs,2);
  if(i!=2) {
    VMnotify(player,"usage: vmsetmb <ship> <bridge>");
    return;
  }

  Ship=atoi(fargs[0]);
  if(Ship < 0 || Ship > VMCurrentMax) {
    VMnotify(player,"Invalid Ship to Scan");
    return;
  }
  MB = atoi(fargs[1]);
  if ( MB < 0 || MB > mudstate.db_top ) {
    VMnotify(player,"Invalid bridge number given");
    return;
  }
  VMSetMainBridge(player,Ship,MB);
}

void do_VMArmorStatus(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ShipNum;
  ShipNum=VMShipNum(player);
  if (!CanAccessNav2(ShipNum,player) && !CanAccessHelm2(ShipNum,player))
	{
	notify(player,"You must man the helm or nav console.");
    return;
	}
  VMArmorStatus(player,ShipNum);
}



void do_VMDoRep(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ShipNum,nargs,sys;
  char *args[4];
  nargs=xcode_parse(buffer,args,3);
  if (nargs != 3) {
    VMnotify(player,"usage: sinfo <shipnum> <systemarray> <sys>");
    return;
  }
  ShipNum = atoi(args[0]);
  sys = atoi(args[2]);
  
  if (!strcmp(args[1],"helm")) {
    VMRepOne(player,&(VMSpace[ShipNum].VMPerm.Helm.Systems[sys]));
  }
  else if (!strcmp(args[1],"nav")) {
    VMRepOne(player,&(VMSpace[ShipNum].VMPerm.Nav.Systems[sys]));
  }
  else if (!strcmp(args[1],"eng")) {
    VMRepOne(player,&(VMSpace[ShipNum].VMPerm.Eng.Systems[sys]));
  }
}



void VMRepOne(int player, t_System *sys) {
  VMnotify(player,"Name %s",sys->name);
  VMnotify(player,"Kind: %d",sys->kind);
  VMnotify(player,"Indexnum: %d",sys->indexnum);
  VMnotify(player,"Type: %d",sys->type);
  VMnotify(player,"Skill: %d",sys->skill);
  VMnotify(player,"Sysnum: %d",sys->sysnum);
  VMnotify(player,"Flags: %d",sys->flags);
  VMnotify(player,"plan: %d",sys->plan);
  VMnotify(player,"size: %d",sys->size);

  VMnotify(player,"dstat0: %2.2f",sys->dstat0);
  VMnotify(player,"dstat1: %2.2f",sys->dstat1);
  VMnotify(player,"dstat2: %2.2f",sys->dstat2);
  VMnotify(player,"dstat3: %2.2f",sys->dstat3);
  VMnotify(player,"dstat4: %2.2f",sys->dstat4);
  VMnotify(player,"dstat5: %2.2f",sys->dstat5);
  VMnotify(player,"dstat6: %2.2f",sys->dstat6);
  VMnotify(player,"dstat7: %2.2f",sys->dstat7);
  VMnotify(player,"dstat8: %2.2f",sys->dstat8);
  VMnotify(player,"dstat0: %2.2f",sys->dstat9);


  VMnotify(player,"tdstat0: %2.2f",sys->tdstat0);
  VMnotify(player,"tdstat1: %2.2f",sys->tdstat1);
  VMnotify(player,"tdstat2: %2.2f",sys->tdstat2);
  VMnotify(player,"tdstat3: %2.2f",sys->tdstat3);
  VMnotify(player,"tdstat4: %2.2f",sys->tdstat4);

  VMnotify(player,"istat0: %d",sys->dstat0);
  VMnotify(player,"istat1: %d",sys->dstat1);
  VMnotify(player,"istat2: %d",sys->istat2);
  VMnotify(player,"istat3: %d",sys->istat3);
  VMnotify(player,"istat4: %d",sys->istat4);
  VMnotify(player,"istat5: %d",sys->istat5);
  VMnotify(player,"istat6: %d",sys->istat6);
  VMnotify(player,"istat7: %d",sys->istat7);
  VMnotify(player,"istat8: %d",sys->istat8);
  VMnotify(player,"istat0: %d",sys->istat9);


  VMnotify(player,"tistat0: %d",sys->tistat0);
  VMnotify(player,"tistat1: %d",sys->tistat1);
  VMnotify(player,"tistat2: %d",sys->tistat2);
  VMnotify(player,"tistat3: %d",sys->tistat3);
  VMnotify(player,"tistat4: %d",sys->tistat4);

}

	 
void do_VMDamStat(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ShipNum;
  ShipNum=VMShipNum(player);
  if (!CanAccessDamCon(ShipNum,player))
    return;
  VMDamStat(player,ShipNum);
}

void do_VMMiscRepairSystem(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[3];
  int i;
  int ShipNum;
  ShipNum=VMShipNum(player);
  if (!CanAccessDamCon(ShipNum,player))
    return;
  i = xcode_parse(buffer,args,3);
  if ( i < 2 ) {
    VMnotify(player,"usage: repair [team] [hull] | [team] [area] [system]");
    return;
  }
  if (i == 2) {
    if (!strcmp(args[1],"hull")) {
      VMMiscRepairHull(player,ShipNum,atoi(args[0]));
      return;
    }
    else {
      VMnotify(player,"usage: repair [team] [hull] | [area] [system]");
      return;
    }
  }
  if (i == 3) {
    VMMiscRepair(player,ShipNum,atoi(args[0]),args[1],atoi(args[2]));
  }
  else {
    VMnotify(player,"usage: repair [team] [hull] | [team] [area] [system]");
    return;
  }
}


void do_VMSetDamVerbose(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[1];
  int nargs;
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (!CanAccessDamCon(ShipNum,player))
    return;
  if (xcode_parse(buffer,args,1) != 1 ) {
    VMnotify(player,"usage: verbose [on|off]");
    return;
  }
  if (!strcmp(args[0],"on")) {
    VMSetDamVerbose(player,ShipNum,1);
  }
  else if (!strcmp(args[0],"off")) {
    VMSetDamVerbose(player,ShipNum,0);
  }
   
}


void do_VMMissleStatus(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  int ShipNum;
  ShipNum=VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  VMMissleStatus(player,ShipNum);
}





void do_VMMissleDoor(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[2];
  int nargs;
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  if (xcode_parse(buffer,args,2) != 2 ) {
    VMnotify(player,"usage: tube <num> [open|close]");
    return;
  }
  if (!strcmp(args[1],"open")) {
    VMMissleDoor(player,ShipNum,atoi(args[0]),1);
  }
  else if (!strcmp(args[1],"close")) {
    VMMissleDoor(player,ShipNum,atoi(args[0]),0);
  }
   
}


void do_VMLoadMissle(player,data,buffer)
  int player;
void *data;
char *buffer;
{
  char *args[2];
  int nargs;
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  if (xcode_parse(buffer,args,2) != 2 ) {
    VMnotify(player,"usage: load <tube> <missle>");
    return;
  }
  VMLoadMissle(player,ShipNum,atoi(args[0]),atoi(args[1]));
}


void do_VMUnloadMissle(player,data,buffer)
  int player;
void *data;
char *buffer;
{
  char *args[2];
  int nargs;
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  if (xcode_parse(buffer,args,1) != 1 ) {
    VMnotify(player,"usage: unload <tube>");
    return;
  }
  VMUnloadMissle(player,ShipNum,atoi(args[0]));
}


void do_VMDumpMissle(player,data,buffer)
  int player;
void *data;
char *buffer;
{
  char *args[2];
  int nargs;
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  if (xcode_parse(buffer,args,1) != 1 ) {
    VMnotify(player,"usage: dump <tube>");
    return;
  }
  VMDumpMissle(player,ShipNum,atoi(args[0]));
}


void do_VMAddMissle(player,data,buffer)
  int player;
void *data;
char *buffer;
{
  char *args[2];
  int nargs;
  int ShipNum;
  if (xcode_parse(buffer,args,2) != 2 ) {
    VMnotify(player,"addmiss <ship> <missle>");
    return;
  }
  VMAddMissle(player,atoi(args[0]),atoi(args[1]));
}

void do_VMArmMissle(player,data,buffer)
  int player;
void *data;
char *buffer;
{
  char *args[2];
  int nargs;
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  if (xcode_parse(buffer,args,1) != 1 ) {
    VMnotify(player,"usage: arm <tube>");
    return;
  }
  VMArmMissle(player,ShipNum,atoi(args[0]));
}


void do_VMAutoloadTube(player,data,buffer)
  int player;
void *data;
char *buffer;
{
  char *args[2];
  int nargs;
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  if (xcode_parse(buffer,args,2) != 2 ) {
    VMnotify(player,"usage: tal <tube> <missle>");
    return;
  }
  VMAutoloadTube(player,ShipNum,atoi(args[0]),atoi(args[1]));
}

void do_VMInterceptorState(player,data,buffer)
  int player;
void *data;
char *buffer;
{
  char *args[2];
  int nargs;
  int ShipNum;
  ShipNum = VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  if (xcode_parse(buffer,args,2) != 2 ) {
    VMnotify(player,"usage: inter <tube> off|on");
    return;
  }
  if (!strcmp(args[1],"on")) {
    VMInterceptorState(player,ShipNum,atoi(args[0]),1);
  }
  else if (!strcmp(args[1],"off")) {
    VMInterceptorState(player,ShipNum,atoi(args[0]),0);
  }  

}


void do_VMHelmLayMine(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[3];
  int i,target,weapon;
  int ShipNum;
  i = xcode_parse(buffer,args,1);
  if ( i < 1 ) {
    VMnotify(player,"usage: lay <mislauncher> ");
    return;
  }
  ShipNum=VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  weapon = atoi(args[0]);
  VMHelmLayMine(ShipNum,player,weapon);
}

void do_VMHelmShowShipMineList(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[3];
  int i,target,weapon;
  int ShipNum;
  ShipNum=VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  VMHelmShowShipMineList(player,ShipNum);
}

void do_VMHelmSweepMine(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *args[3];
  int i,target,weapon;
  int ShipNum;
  ShipNum=VMShipNum(player);
  if (!CanAccessHelm(ShipNum,player))
    return;
  VMHelmSweepMine(player,ShipNum);
}

void do_VMAdsInit(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  VMnotify(player,"That command has not been implemented yet.");
}

void do_VMAdsLoad(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  VMnotify(player,"That command has not been implemented yet.");
}

void do_VMAdsSave(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  VMnotify(player,"That command has not been implemented yet.");
}

void do_VMAdsStop(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  VMnotify(player,"That command has not been implemented yet.");
}

void do_VMAdsStart(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  VMnotify(player,"That command has not been implemented yet.");
}
void do_DumpAllPlayers(player,data,buffer) 
	int player;     
	void *data;     
	char *buffer;   
{
	t_SpaceEnt *tmp;
	VMnotify(player, "total List Entries: %d", GetSpaceListSize(VMAllPlayers));
	ResetSpaceList(VMAllPlayers);
	while (!AtEndSpaceList(VMAllPlayers)) {
		tmp = GetCurrSpaceEnt(VMAllPlayers);
		VMnotify(player," AllPlayers: %s(#%d) ", Name(tmp->VMShip),tmp->VMShip);

		AdvanceSpaceList(VMAllPlayers);
	}
}
