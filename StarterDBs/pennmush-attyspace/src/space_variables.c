/* space_variables.c */

#include <ctype.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif

#include "copyrite.h"
#include "config.h"
#include "ansi.h"
#include "externs.h"
#include "intrface.h"
#include "parse.h"
#include "confmagic.h"
#include "space.h"

/* ------------------------------------------------------------------------ */

struct space_database_t sdb[MAX_SPACE_OBJECTS + 1];
struct comms_database_t cdb[MAX_COMMS_OBJECTS + 1];
double cochrane = COCHRANE;
int n;
int max_space_objects = MIN_SPACE_OBJECTS;
int max_comms_objects = MIN_COMMS_OBJECTS;
int beam_refresh = BEAM_REFRESH;
int missile_refresh = MISSILE_REFRESH;
dbref console_security = CONSOLE_SECURITY;
dbref console_helm = CONSOLE_HELM;
dbref console_engineering = CONSOLE_ENGINEERING;
dbref console_operation = CONSOLE_OPERATION;
dbref console_science = CONSOLE_SCIENCE;
dbref console_damage = CONSOLE_DAMAGE;
dbref console_communication = CONSOLE_COMMUNICATION;
dbref console_tactical = CONSOLE_TACTICAL;
dbref console_transporter = CONSOLE_TRANSPORTER;
dbref console_monitor = CONSOLE_MONITOR;
dbref console_fighter = CONSOLE_FIGHTER;

/* ------------------------------------------------------------------------ */

const char *shield_name[] = {
    "Forward shield",
    "Starboard shield",
    "Aft shield",
    "Port shield"
};

const char *cloak_name[] = {
    "Other",
    "Cloak"
};

const char *type_name[] = {
    "None",
    "Ship",
    "Base",
    "Planet",
    "Anomaly",
    "Star",
};

const char *beam_name[] = {
    "Beam weapon",
    "Phaser emitter",
    "Phaser bank",
    "Phaser cannon",
    "Phaser array",
    "Disruptor",
    "Disruptor bank",
    "Disruptor cannon",
    "Laser",
    "Blaster",
    "Particle beam",
    "Maser",
    "Hyperphaser",
    "Hyperphaser bank",
    "Polaron beam",
    "Meson beam",
    "Antiproton beam",
    "Ion beam",
    "Fusion beam",
    "Pulse phaser",
    "Phase disruptor",
    "Plasma beam",
    "Mauler weapon",
};

const char *missile_name[] = {
    "Missile weapon",
    "Photon torpedo",
    "Particle cannon",
    "Plasma torpedo",
    "Web slinger",
    "Tri-cobalt torpedo",
    "Antiproton cannon",
    "Graviton cannon",
    "Wave-motion gun",
    "Quantum torpedo",
    "Proton torpedo",
    "Disruptor",
    "Disruptor bank",
    "Disruptor cannon",
    "Ballistic missile",
    "Reflex gun",
    "Rail gun",
    "Hellcannon",
    "Hellbore",
};

const char *quadrant_name[] = {
    "Alpha",
    "Beta",
    "Delta",
    "Gamma" };

const char *system_name[] = {
	"Superstructure",
	"Fusion Reactor",
	"Batteries",
	"Beam Weapon",
	"Cloaking Device",
	"EW Systems",
	"Impulse Drive",
	"LR Sensors",
	"M/A Reactor",
	"Missile Weapon",
	"Shield",
	"SR Sensors",
	"Tractor Beams",
	"Transporters",
	"Warp Drive"
};

double repair_mult[] = {
    32.0, 4.0, 2.0, 1.0, 8.0, 8.0, 4.0, 8.0, 8.0, 2.0, 1.0, 8.0, 1.0, 8.0, 4.0
};

const char *damage_name[] = {
	"No Damage",
	"Patched Damage",
	"Minor Damage",
	"Light Damage",
	"Moderate Damage",
	"Heavy Damage",
	"Severe Damage",
	"Inoperative",
	"Destroyed"
};

const char *empire_name[] = {
    "Bajoran",
	"Orion",
    "Tholian",
    "Gorn",
    "Cardassian",
    "Ferengi",
    "Romulan",
    "Klingon",
    "Borg",
    "Dominion",
    "Federation",
    "(Unclaimed)"
};

double empire_center[MAX_EMPIRE_NAME][4] = {
          5.0, -28776689375154.8,    684090738200.5, -139163248952.8,
		 10.0, -29029404898208.9,     23759581431.5, -419649470139.9,
         40.0, -28104015577556.0,   -894841290384.0,  -30856596220.0,
         80.0, -28597721117078.0,  -1018267675264.0,  246852769761.0,
        100.0, -28813717290619.0,    956554482824.0,   61713192440.0,
        100.0, -27702879826694.0,    802271501723.0, -154282981100.0,
        120.0, -29245709637701.0,   -709701713063.0,   30856596220.0,
        160.0, -29554275599903.0,    370279154641.0, -277709365981.0,
        240.0,               0.0, -28381724943537.0,             0.0,
        240.0,  20056787543092.0,  20056787543092.0,             0.0,
        240.0, -28381724943537.0,               0.0,             0.0,
      15000.0,               0.0,               0.0,             0.0
};

/* ------------------------------------------------------------------------ */
