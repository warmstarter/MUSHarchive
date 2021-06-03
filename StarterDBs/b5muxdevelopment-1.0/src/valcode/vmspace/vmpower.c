/* $Id: vmpower.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

#include "header.h"
static char *rcsvmpowerc="$Id: vmpower.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
void VMUpdateEnergyOne(int ship
		  ) {
  int discharge,balloc,recharge,release,relmax,power,mpower,maxdis;
  double brate;
  int intpower;
  int npower,bleed;

  /* set up all batts variables */
  balloc = VMSpace[ship].VMPerm.Eng.Systems[1].tistat0;
  power = VMSpace[ship].VMPerm.Eng.Systems[1].istat1;
  discharge =  VMSpace[ship].VMPerm.Eng.Systems[1].istat3;
  mpower =  VMSpace[ship].VMPerm.Eng.Systems[1].istat4;
  recharge =  VMSpace[ship].VMPerm.Eng.Systems[1].istat5;
  maxdis =  VMSpace[ship].VMPerm.Eng.Systems[1].istat4;
  brate =  VMSpace[ship].VMPerm.Eng.Systems[1].dstat1;
  /* recharge batts */
  
  if ( balloc != 0) {
    if ( balloc > recharge)
      intpower = recharge;
    else
      intpower = balloc;

    /*    VMnotify(1,"intpower is: %d", intpower);*/
    npower = power + intpower;
    if (npower > mpower)
      VMSpace[ship].VMPerm.Eng.Systems[1].istat1 = mpower;
    else
      VMSpace[ship].VMPerm.Eng.Systems[1].istat1 = npower;
  }
  if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[1].flags,BATTSONFLAG)) {
    /* bleed batts */
    bleed = VMSpace[ship].VMPerm.Eng.Systems[1].istat1 * (brate/100);
    /*VMnotify(1,"bleed is: %d",bleed);*/
    if ((VMSpace[ship].VMPerm.Eng.Systems[1].istat1 - bleed) < 0 )
      VMSpace[ship].VMPerm.Eng.Systems[1].istat1 = 0;
    else
      VMSpace[ship].VMPerm.Eng.Systems[1].istat1 -= bleed;
    /* drain batts */
    if ((VMSpace[ship].VMPerm.Eng.Systems[1].istat1 - discharge) < 0)
      VMSpace[ship].VMPerm.Eng.Systems[1].istat1 = 0;
    else
      VMSpace[ship].VMPerm.Eng.Systems[1].istat1 -= discharge;
  }
  /* if batteries are empty then set the release rate to zero otherwise set it to whatever the max is */
  if ((VMSpace[ship].VMPerm.Eng.Systems[1].istat1 <= 0) || !checkflag(VMSpace[ship].VMPerm.Eng.Systems[1].flags,BATTSONFLAG))
    VMSpace[ship].VMPerm.Eng.Systems[1].istat3 = 0;
  else
    VMSpace[ship].VMPerm.Eng.Systems[1].istat3 = VMSpace[ship].VMPerm.Eng.Systems[1].istat2;
  VMPowerCheckFuel(ship);
  VMEngUpdateEngPower(ship);

  VMPowerUpdateHelm(ship);
  VMPowerUpdateNav(ship);
}

void VMPowerUpdateEngPower(int ship
		      ) {
  /* we need to update reactor stress, and then update the eng power */
  switch (VMPowerUpdateStress(ship)) 
    {
    case 0:
      /* we're okay no stress taken */
      return;
      break;
    case 1:
      /* we are stressing */
      if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,SAFETYFLAG))
	VMnotifymans(VMSpace[ship].VMTemp.TmpEng.VMMan_List,"[ENGINEERING COMPUTER] *WARNING* Engine is taking stress.");
      return;
      break;
    case 2:
      /* we are clearing off stress */
      if (checkflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,SAFETYFLAG))
	VMnotifymans(VMSpace[ship].VMTemp.TmpEng.VMMan_List,"[ENGINEERING COMPUTER]  Engine is reducing stress.");
      return;
      break;
    }
}


int VMPowerUpdateStress(int ship
		    ) {
  double stress,ostress,enfac;
  int or;
  int cr;
  int bl;
  double newstress;
  if (VMSpace[ship].VMPerm.Eng.Systems[0].istat4 == ENGTYPE_VORLON)
    return 0;
  if (VMSpace[ship].VMPerm.Eng.Systems[0].istat4 == ENGTYPE_FUSION) {
    or = VMSpace[ship].VMPerm.Eng.Systems[0].istat1;
    cr = VMSpace[ship].VMPerm.Eng.Systems[0].tistat1;
    bl = VMSpace[ship].VMPerm.Eng.Systems[0].basenumber;
    enfac = VMSpace[ship].VMPerm.Eng.Systems[0].dstat1;
    ostress = VMSpace[ship].VMPerm.Eng.Systems[0].dstat3;
    stress = ostress + calc_stress(or,cr,bl,enfac);
    if (stress == ostress) {
      return 0;
    }
    if (stress < ostress) {
      VMSpace[ship].VMPerm.Eng.Systems[0].dstat3 = stress;
      VMSpace[ship].VMPerm.Eng.Systems[0].flags = clearflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,STRESSINFLAG);
      return 2;
    }
    if (stress > ostress) {
      VMSpace[ship].VMPerm.Eng.Systems[0].flags = setflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,STRESSINFLAG);
      VMSpace[ship].VMPerm.Eng.Systems[0].dstat3 = stress;
      newstress = stress - ostress;
      VMSpace[ship].VMPerm.Eng.Systems[0].istat6 = (VMSpace[ship].VMPerm.Eng.Systems[0].dstat2 - stress) / newstress; 
      return 1;
    }

    
  }
  
}


double  calc_stress(int optreact, 
	    int curreact,
	    int basereact,
	    double enginefactor
	    ) {
  
  double x;
  double val;
  /* x is an intermediate number used to calc stuff */
  x = (curreact - optreact) * (100 / (basereact == 0 ? 1 : basereact));
  if ( x == 0 )
    return 0.0;
  val = (x + 1/x) + enginefactor - 1;
  return val;
}



void VMPowerCheckFuel(int ship
		 ) {
  double fuel, nfuel;
  int fueleff;
  int currea,maxrea,optrea;
  fuel = 0.0;
  nfuel = 0.0;
  fueleff = 0;
  currea = 0;
  maxrea = 0;
  optrea = 0;
	
  fuel = VMSpace[ship].VMPerm.Eng.Systems[0].dstat4;
  fueleff = VMSpace[ship].VMPerm.Eng.Systems[0].istat7;
  currea = VMSpace[ship].VMPerm.Eng.Systems[0].tistat1;
  maxrea = VMSpace[ship].VMPerm.Eng.Systems[0].istat2;
  optrea = VMSpace[ship].VMPerm.Eng.Systems[0].istat1;
 /* VMnotify(35,"Initial Presentation: fuel: %6.3f fueleff: %d currea: %d maxrea: %d optrea: %d",fuel,fueleff,currea,maxrea,optrea);*/
  if (currea > optrea) {
    /* ships burn fuel greater at higher reactors */ 
    fueleff = fueleff + (currea - optrea); 
  }
 
  /*VMnotify(35,"Second Presentation: fuel: %6.3f fueleff: %d currea: %d maxrea: %d optrea: %d",fuel,fueleff,currea,maxrea,optrea);*/
  nfuel = fuel - ((currea * fueleff)/10000000.0);
  /* VMnotify(35,"After Nfuel: nfuel: %6.3f fuel: %6.3f fueleff: %d currea: %d maxrea: %d optrea: %d",nfuel,fuel,fueleff,currea,maxrea,optrea);*/
  if (nfuel < 0) {
    VMSpace[ship].VMPerm.Eng.Systems[0].dstat4 = 0;
    /* now we need to shut down the reactor */
    if (!checkflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,NOGASFLAG)) {
      VMnotifymans(VMSpace[ship].VMTemp.TmpEng.VMMan_List,"[Engineering Computer] *WARNING* ship is out of fuel");

      VMSpace[ship].VMPerm.Eng.Systems[0].flags = setflag(VMSpace[ship].VMPerm.Eng.Systems[0].flags,NOGASFLAG);
    }
  }
  else
    VMSpace[ship].VMPerm.Eng.Systems[0].dstat4 = nfuel;
}


void VMPowerUpdateHelm(int ship
		  ) {
  int i,newpow,pow,div;
  double actbleed,bleedloss;
  VMHelmReallocPower(ship);
  /*  VMnotify(1,"Updating helm stuff");*/
  for (i = 0;i < HSystems;i++) {
    /* clear all fired flags */ 
    if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,GUNFLAG)) {
      /* put a copy of the curent power into a temp variable */
      VMSpace[ship].VMPerm.Helm.Systems[i].tistat4 = VMSpace[ship].VMPerm.Helm.Systems[i].tistat1;
      bleedloss = VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 - (VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 * (1 - VMSpace[ship].VMPerm.Helm.Systems[i].dstat0/100));
      if (bleedloss < 1.0)
	bleedloss = 1.0;
      VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 -= bleedloss;
      if (VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 < 0)
	VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 = 0;
      if (checkflag(VMSpace[ship].VMPerm.Helm.Systems[i].flags,WEAPONLINE)) {

	/* charge the gun */
	/* code added to make sure the gun is penalized for special stuff. */
	if (VMSpace[ship].VMPerm.Helm.Systems[i].tistat3 != 0) {
	  div = 0;
	  div = VMSpace[ship].VMPerm.Helm.Systems[i].tistat3 + 1;
	  if (div <= 0) {
	    div = 1;
	  }
	}
	else
	  div = 1;
#ifdef DEBUG
	VMcnotify("div: %d",div);
#endif
	pow = VMSpace[ship].VMPerm.Helm.Systems[i].tistat0 / div;
	/* if power in the system is greater than acceptance we dont give a shit all they get is acceptance */
	if (pow > VMSpace[ship].VMPerm.Helm.Systems[i].istat5)
	  pow = VMSpace[ship].VMPerm.Helm.Systems[i].istat5;
	
	if ((VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 + pow) > VMSpace[ship].VMPerm.Helm.Systems[i].istat4) {
	  VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 = VMSpace[ship].VMPerm.Helm.Systems[i].istat4;
	}
	else {
	  /* i guess we're ok. */
	  VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 += pow;
	}
	if ((VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 == VMSpace[ship].VMPerm.Helm.Systems[i].istat4) && (VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 != VMSpace[ship].VMPerm.Helm.Systems[i].tistat4)) {
	  VMnotifymans(VMSpace[ship].VMTemp.TmpHelm.VMMan_List,"[HELM] Gun %d is fully charged.",i);

	}

      }
      else
	VMSpace[ship].VMPerm.Helm.Systems[i].tistat1 = 0;




    }
    
  }
  
}




void VMPowerUpdateNav(int ship)
{
int pw;
if(checkflag(VMSpace[ship].VMPerm.Nav.Systems[5].flags,EXISTS)) {
	VMSpace[ship].VMPerm.Nav.Systems[5].tistat2*=.9999;
	pw=VMSpace[ship].VMPerm.Nav.Systems[5].tdstat0*VMSpace[ship].VMTemp.TmpEng.PowerNav;
	if(pw > VMSpace[ship].VMPerm.Nav.Systems[5].istat1) pw=VMSpace[ship].VMPerm.Nav.Systems[5].istat1;
/*  We now update power only half as often as nav for cpu eff, 
	so double charging */

	VMSpace[ship].VMPerm.Nav.Systems[5].tistat2+= pw*2.0;
	if(VMSpace[ship].VMPerm.Nav.Systems[5].tistat2 > VMSpace[ship].VMPerm.Nav.Systems[5].istat2) VMSpace[ship].VMPerm.Nav.Systems[5].tistat2=VMSpace[ship].VMPerm.Nav.Systems[5].istat2;
	}

}

