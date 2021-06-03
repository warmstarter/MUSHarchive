
double read_skill();
double read_skill2();
double VMGetCoordsx();
double VMGetCoordsy();
double VMGetCoordsz();


int CanAccessSci();

FUNCTION(fun_CanSci)
{
int p,ship;
p=match_thing(player,fargs[0]);
if(!Good_obj(p)) {
	notify(player,"Invalid Object");
	return;
	}
ship=atoi(fargs[1]);
fval(buff,bufc,CanAccessSci(p,ship));
}

FUNCTION(fun_readskill2)
{
int p,skill;

p=match_thing(player,fargs[0]);
if(!Good_obj(p)) {
	notify(player,"Invalid Object");
	return;
	}


if(!(Flags3(p)&REGISTERED)) {
	notify(player,"That player is not registered");
	return;
	}
skill=atoi(fargs[1]);
fval(buff,bufc,read_skill2(p,skill));

}
FUNCTION(fun_readskill)
{
int p,skill;

p=match_thing(player,fargs[0]);
if(!Good_obj(p)) {
	notify(player,"Invalid Object");
	return;
	}


if(!(Flags3(p)&REGISTERED)) {
	notify(player,"That player is not registered");
	return;
	}
skill=atoi(fargs[1]);
fval(buff,bufc,read_skill(p,skill));

}


FUNCTION(fun_HurtPhys) {
int p;
float dmg;

p=match_thing(player,fargs[0]);
if(!Good_obj(p)) {
	notify(player,"Invalid Object");
	return;
	}


if(!(Flags3(p)&REGISTERED)) {
	notify(player,"That player is not registered");
	return;
	}

dmg=atof(fargs[1]);
Hurt(p,dmg);
}


FUNCTION(fun_HurtMent) {
int p;
float dmg;

p=match_thing(player,fargs[0]);
if(!Good_obj(p)) {
	notify(player,"Invalid Object");
	return;
	}


if(!(Flags3(p)&REGISTERED)) {
	notify(player,"That player is not registered");
	return;
	}

dmg=atof(fargs[1]);
HurtMent(p,dmg);
}

FUNCTION(fun_Bestow) {
int p,sk;
float lev;

p=match_thing(player,fargs[0]);
sk=atoi(fargs[1]);
lev=atof(fargs[2]);
Do_Bestow(p,sk,lev);
}

FUNCTION(fun_BadHurt) {
int p;
float dmg;

p=match_thing(player,fargs[0]);
if(!Good_obj(p)) {
	notify(player,"Invalid Object");
	return;
	}


if(!(Flags3(p)&REGISTERED)) {
	notify(player,"That player is not registered");
	return;
	}

dmg=atof(fargs[1]);
BadHurt(p,dmg);
}


FUNCTION(fun_ShipCoords) {
double x,y,z;
int ship;
char bf[700];

ship=atof(fargs[0]);
x=VMGetCoordsx(ship);
y=VMGetCoordsy(ship);
z=VMGetCoordsz(ship);
sprintf(bf,"%f %f %f",x,y,z);
safe_str(bf,buff,bufc);
}

FUNCTION(fun_Hyper) {
int ship;
ship=atof(fargs[0]);

fval(buff,bufc,(float)IsHyper(ship));

}

FUNCTION(fun_HyperCoords) {
double x,y,z;
int ship;
char bf[700];

ship=atof(fargs[0]);
x=VMGetCoordsx(ship)/100.0;
y=VMGetCoordsy(ship)/100.0;
z=VMGetCoordsz(ship)/100.0;
sprintf(bf,"%f %f %f",x,y,z);
safe_str(bf,buff,bufc);
}

static char *rcsvalfuncsc="$Id: valfuncs.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
