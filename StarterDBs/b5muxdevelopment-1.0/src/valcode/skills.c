
#include "header.h"
#include "skills.h"

#define GUNNER 237
#define PILOTING GUNNER+29
#define TECHN PILOTING+29
#define TELEP TECHN+40 
/* Last number is the skill it is below in the hierarchy */


Skill Skills[]={
{(char *)"nada",                0,      0,              -1},
{(char *)"Unarmed Combat",      1,      ATHLETIC,       -1},
{(char *)"Computer",		2,	MENTAL,		-1},
{(char *)"Firearms",		3,	PHYSICAL,	-1},
{(char *)"Pistol",		4,	PHYSICAL,	3},
{(char *)"Rifle",		5,	PHYSICAL,	3},
{(char *)"Light PPG",           6,      PHYSICAL,       8},
{(char *)"Heavy PPG",           7,      PHYSICAL,       8},
{(char *)"PPG Pistol",          8,      PHYSICAL,       4},
{(char *)"Slug Pistol",         9,      PHYSICAL,       4},
{(char *)"PPG Rifle",           10,     PHYSICAL,       5},
{(char *)"Slug Rifle",          11,     PHYSICAL,       5},
{(char *)"Support Weapons",     12,     PHYSICAL,       -1},
{(char *)"PPG Bazooka",         13,     PHYSICAL,       12},
{(char *)"Slug Bazooka",        14,     PHYSICAL,       12},
{(char *)"Rocket Launcher",     15,     PHYSICAL,       12},
{(char *)"Dodge",               16,     PHYSICAL,       -1},
{(char *)"Quickdraw",           17,     PHYSICAL,       -1},
{(char *)"Stealth",             18,     PHYSICAL,       -1},
{(char *)"Escape Artist",       19,     PHYSICAL,       -1},
{(char *)"Survival",            20,     PHYSICAL,       -1},
{(char *)"Blade",               21,     ATHLETIC,       -1},
{(char *)"Small Blade",         22,     ATHLETIC,       21},
{(char *)"Large Blade",         23,     ATHLETIC,       21},
{(char *)"Pole Weapon",         24,     ATHLETIC,       -1},
{(char *)"Minbari Pole Weapon", 25,     ATHLETIC,       24},
{(char *)"Acrobatics",          26,     ATHLETIC,       -1},
{(char *)"Archery",             27,     ATHLETIC,       -1},
{(char *)"Longbow Archery",     28,     ATHLETIC,       27},
{(char *)"Crossbow Archery",    29,     ATHLETIC,       27},
{(char *)"Running",             30,     ATHLETIC,       -1},
{(char *)"Swimming",            31,     ATHLETIC,       -1},
{(char *)"Climbing",            32,     ATHLETIC,       -1},
{(char *)"Language",            33,     MENTAL,         -1},
{(char *)"Interlac Language",   34,     MENTAL,         33},
{(char *)"Abbai Language",      35,     MENTAL,         33},
{(char *)"Antarean Language",   36,     MENTAL,         33},
{(char *)"Arnassian Language",  37,     MENTAL,         33},
{(char *)"Belladon Language",   38,     MENTAL,         33},
{(char *)"Brakiri Language",    39,     MENTAL,         33},
{(char *)"Bremmaer Language",   40,     MENTAL,         33},
{(char *)"Brikiri Language",    41,     MENTAL,         33},
{(char *)"Bulloxian Language",  42,     MENTAL,         33},
{(char *)"Carnellian Language", 43,     MENTAL,         33},
{(char *)"Cascan Language",     44,     MENTAL,         33},
{(char *)"Cauralline Language", 45,     MENTAL,         33},
{(char *)"Centaur Language",    46,     MENTAL,         33},
{(char *)"Cotswan Language",    47,     MENTAL,         33},
{(char *)"Cthulin Language",    48,     MENTAL,         33},
{(char *)"Deneth Language",     49,     MENTAL,         33},
{(char *)"Dilgar Language",     50,     MENTAL,         33},
{(char *)"Drazi Language",      51,     MENTAL,         33},
{(char *)"English Language",    52,     MENTAL,         33},
{(char *)"Froon Language",      53,     MENTAL,         33},
{(char *)"Gaim Language",       54,     MENTAL,         33},
{(char *)"Golian Language",     55,     MENTAL,         33},
{(char *)"Grey Language",       56,     MENTAL,         33},
{(char *)"Grome Language",      57,     MENTAL,         33},
{(char *)"Hyach Language",      58,     MENTAL,         33},
{(char *)"Ikarran Language",    59,     MENTAL,         33},
{(char *)"Ipshan Language",     60,     MENTAL,         33},
{(char *)"Kandarian Language",  61,     MENTAL,         33},
{(char *)"L5 Language",         62,     MENTAL,         33},
{(char *)"Live Eater Language", 63,     MENTAL,         33},
{(char *)"Llort Language",      64,     MENTAL,         33},
{(char *)"Lumati Language",     65,     MENTAL,         33},
{(char *)"Markab Language",     66,     MENTAL,         33},
{(char *)"Minbari Language",    67,     MENTAL,         33},
{(char *)"Lennau Dialect",      68,     MENTAL,         67},
{(char *)"Vik Dialect",         69,     MENTAL,         67},
{(char *)"Adronato Dialect",    70,     MENTAL,         67},
{(char *)"Morellian Language",  71,     MENTAL,         33},
{(char *)"na'ka'leen Language", 72,     MENTAL,         33},
{(char *)"Narnic Language",     73,     MENTAL,         33},
{(char *)"Onteen Language",     74,     MENTAL,         33},
{(char *)"pak'ma'ra Language",  75,     MENTAL,         33},
{(char *)"Pre'lek Language",    76,     MENTAL,         33},
{(char *)"Red Monk Language",   77,     MENTAL,         33},
{(char *)"Shadow Language",     78,     MENTAL,         33},
{(char *)"Shedrak Language",    79,     MENTAL,         33},
{(char *)"Sh'lassen Language",  80,     MENTAL,         33},
{(char *)"Slanaa Language",     81,     MENTAL,         33},
{(char *)"Shag-toth Language",  82,     MENTAL,         33},
{(char *)"Streib Language",     83,     MENTAL,         33},
{(char *)"Thrakallan Language", 84,     MENTAL,         33},
{(char *)"Thrantallil Language",85,     MENTAL,         33},
{(char *)"Throxtil Language",   86,     MENTAL,         33},
{(char *)"Tikarran Language",   87,     MENTAL,         33},
{(char *)"Tokatan Language",    88,     MENTAL,         33},
{(char *)"Trivorian Language",  89,     MENTAL,         33},
{(char *)"Tuchanq Language",    90,     MENTAL,         33},
{(char *)"Varnian Language",    91,     MENTAL,         33},
{(char *)"vin'Zini Language",   92,     MENTAL,         33},
{(char *)"Vorlon Language",     93,     MENTAL,         33},
{(char *)"Vree Language",       94,     MENTAL,         33},
{(char *)"Walker Language",     95,     MENTAL,         33},
{(char *)"Xon Language",        96,     MENTAL,         33},
{(char *)"Xoth Language",       97,     MENTAL,         33},
{(char *)"Ylinnar Language",    98,     MENTAL,         33},
{(char *)"Ynaborian Language",  99,     MENTAL,         33},
{(char *)"Zathran Language",    100,    MENTAL,         33},
{(char *)"Zenerian Language",   101,    MENTAL,         33},
{(char *)"Navigation",          102,    MENTAL,         -1},
{(char *)"Narn Navigation",     103,    MENTAL,         102},
{(char *)"Earth Navigation",    104,    MENTAL,         102},
{(char *)"Centauri Navigation", 105,    MENTAL,         102},
{(char *)"Minbari Navigation",  106,    MENTAL,         102},
{(char *)"Vorlon Navigation",   107,    MENTAL,         102},
{(char *)"Shadow Navigation",   108,    MENTAL,         102},
{(char *)"Misc Navigation",     109,    MENTAL,         102},
{(char *)"Narn Fighter Navigation",     110,    MENTAL,         103},
{(char *)"Earth Fighter Navigation",    111,    MENTAL,         104},
{(char *)"Centauri Fighter Navigation", 112,    MENTAL,         105},
{(char *)"Minbari Fighter Navigation",  113,    MENTAL,         106},
{(char *)"Vorlon Fighter Navigation",   114,    MENTAL,         107},
{(char *)"Shadow Fighter Navigation",   115,    MENTAL,         108},
{(char *)"Misc Fighter Navigation",     116,    MENTAL,         109},
{(char *)"Narn Ship Navigation",        117,    MENTAL,         103},
{(char *)"Earth Ship Navigation",       118,    MENTAL,         104},
{(char *)"Centauri Ship Navigation",    119,    MENTAL,         105},
{(char *)"Minbari Ship Navigation",     120,    MENTAL,         106},
{(char *)"Vorlon Ship Navigation",      121,    MENTAL,         107},
{(char *)"Shadow Ship Navigation",      122,    MENTAL,         108},
{(char *)"Misc Ship Navigation",        123,    MENTAL,         109},
{(char *)"Perception",                  124,    MENTAL,         -1},
{(char *)"Appraisal",                   125,    MENTAL,         -1},
{(char *)"Administration",              126,    MENTAL,         -1},
{(char *)"Military Administration",     127,    MENTAL,         126},
{(char *)"Commercial Administration",   128,    MENTAL,         126},
{(char *)"Criminal Administration",     129,    MENTAL,         126},
{(char *)"Alternate Identity",          130,    MENTAL,         -1},
{(char *)"Forgery",                     131,    MENTAL,         -1},
{(char *)"Gambling",                    132,    MENTAL,         -1},
{(char *)"Tracking",                    133,    MENTAL,         -1},
{(char *)"Strategy",                    134,    MENTAL,         -1},
{(char *)"Narn Strategy",               135,    MENTAL,         134},
{(char *)"Earth Strategy",              136,    MENTAL,         134},
{(char *)"Centauri Strategy",           137,    MENTAL,         134},
{(char *)"Minbari Strategy",            138,    MENTAL,         134},
{(char *)"Vorlon Strategy",             139,    MENTAL,         134},
{(char *)"Shadow Strategy",             140,    MENTAL,         134},
{(char *)"Misc Strategy",               141,    MENTAL,         134},
{(char *)"Tactics",                     142,    MENTAL,         -1},
{(char *)"Narn Tactics",                143,    MENTAL,         142},
{(char *)"Earth Tactics",               144,    MENTAL,         142},
{(char *)"Centauri Tactics",            145,    MENTAL,         142},
{(char *)"Minbari Tactics",             146,    MENTAL,         142},
{(char *)"Vorlon Tactics",              147,    MENTAL,         142},
{(char *)"Shadow Tactics",              148,    MENTAL,         142},
{(char *)"Misc Tactics",                149,    MENTAL,         142},
{(char *)"Disguise",                    150,    MENTAL,         -1},
{(char *)"Narn Disguise",               151,    MENTAL,         150},
{(char *)"Earth Disguise",              152,    MENTAL,         150},
{(char *)"Centauri Disguise",           153,    MENTAL,         150},
{(char *)"Minbari Disguise",            154,    MENTAL,         150},
{(char *)"Vorlon Disguise",             155,    MENTAL,         150},
{(char *)"Shadow Disguise",             156,    MENTAL,         150},
{(char *)"Misc Disguise",               157,    MENTAL,         150},
{(char *)"Demolitions",                 158,    MENTAL,         -1},
{(char *)"Cryptography",                159,    MENTAL,         -1},
{(char *)"Security Systems",            160,    MENTAL,         -1},
{(char *)"Engineering",                 161,    MENTAL,         -1},
{(char *)"Narn Engineering",            162,    MENTAL,         161},
{(char *)"Earth Engineering",           163,    MENTAL,         161},
{(char *)"Centauri Engineering",        164,    MENTAL,         161},
{(char *)"Minbari Engineering",         165,    MENTAL,         161},
{(char *)"Vorlon Engineering",          166,    MENTAL,         161},
{(char *)"Shadow Engineering",          167,    MENTAL,         161},
{(char *)"Misc Engineering",            168,    MENTAL,         161},
{(char *)"Tinker",                      169,    MENTAL,         -1},
{(char *)"Narn Tinker",                 170,    MENTAL,         169},
{(char *)"Earth Tinker",                171,    MENTAL,         169},
{(char *)"Centauri Tinker",             172,    MENTAL,         169},
{(char *)"Minbari Tinker",              173,    MENTAL,         169},
{(char *)"Vorlon Tinker",               174,    MENTAL,         169},
{(char *)"Shadow Tinker",               175,    MENTAL,         169},
{(char *)"Misc Tinker",                 176,    MENTAL,         169},
{(char *)"Training",                    177,    SOCIAL,         -1},
{(char *)"Military Training",           178,    SOCIAL,         177},
{(char *)"Civilian Training",           179,    SOCIAL,         177},
{(char *)"Religious Training",          180,    SOCIAL,         177},
{(char *)"Telepathic Training",         181,    PSI,            177},
{(char *)"Criminal Training",           182,    SOCIAL,         177},
{(char *)"Protocol",                    183,    SOCIAL,         -1},
{(char *)"Military Protocol",           184,    SOCIAL,         183},
{(char *)"Civilian Protocol",           185,    SOCIAL,         183},
{(char *)"Religious Protocol",          186,    SOCIAL,         183},
{(char *)"Criminal Protocol",           187,    SOCIAL,         183},
{(char *)"Bureaucracy",                 188,    SOCIAL,         -1},
{(char *)"Military Bureaucracy",        189,    SOCIAL,         188},
{(char *)"Civilian Bureaucracy",        190,    SOCIAL,         188},
{(char *)"Religious Bureaucracy",       191,    SOCIAL,         188},
{(char *)"Criminal Bureaucracy",        192,    SOCIAL,         188},
{(char *)"Negotiation",                 193,    SOCIAL,         -1},
{(char *)"Military Negotiation",        194,    SOCIAL,         193},
{(char *)"Civilian Negotiation",        195,    SOCIAL,         193},
{(char *)"Religious Negotiation",       196,    SOCIAL,         193},
{(char *)"Criminal Negotiation",        197,    SOCIAL,         193},
{(char *)"Seduction",                   198,    SOCIAL,         -1},
{(char *)"Narn Seduction",              199,    SOCIAL,         198},
{(char *)"Earth Seduction",             200,    SOCIAL,         198},
{(char *)"Centauri Seduction",          201,    SOCIAL,         198},
{(char *)"Minbari Seduction",           202,    SOCIAL,         198},
{(char *)"Misc Seduction",              203,    SOCIAL,         198},
{(char *)"Impersonation",               204,    SOCIAL,         -1},
{(char *)"Narn Impersonation",          205,    SOCIAL,         204},
{(char *)"Earth Impersonation",         206,    SOCIAL,         204},
{(char *)"Centauri Impersonation",      207,    SOCIAL,         204},
{(char *)"Minbari Impersonation",       208,    SOCIAL,         204},
{(char *)"Misc Impersonation",          209,    SOCIAL,         204},
{(char *)"Interrogation",               210,    SOCIAL,         -1},
{(char *)"Chemical Interrogation",      211,    SOCIAL,         210},
{(char *)"Telepathic Interrogation",    212,    PSI,            210},
{(char *)"Physical Interrogation",      213,    SOCIAL,         210},
{(char *)"Leadership",                  214,    SOCIAL,         -1},
{(char *)"Military Leadership",         215,    SOCIAL,         214},
{(char *)"Civilian Leadership",         216,    SOCIAL,         214},
{(char *)"Religious Leadership",        217,    SOCIAL,         214},
{(char *)"Criminal Leadership",         218,    SOCIAL,         214},
{(char *)"Scrounge",                    219,    SOCIAL,         -1},
{(char *)"Military Scrounge",           220,    SOCIAL,         219},
{(char *)"Civilian Scrounge",           221,    SOCIAL,         219},
{(char *)"Religious Scrounge",          222,    SOCIAL,         219},
{(char *)"Criminal Scrounge",           223,    SOCIAL,         219},
{(char *)"Streetwise",                  224,    SOCIAL,         -1},
{(char *)"Narn Streetwise",             225,    SOCIAL,         224},
{(char *)"Earth Streetwise",            226,    SOCIAL,         224},
{(char *)"Centauri Streetwise",         227,    SOCIAL,         224},
{(char *)"Minbari Streetwise",          228,    SOCIAL,         224},
{(char *)"Misc Streetwise",             229,    SOCIAL,         224},
{(char *)"Medtech",                     230,    MENTAL,         -1},
{(char *)"Narn Medtech",                231,    MENTAL,         230},
{(char *)"Earth Medtech",               232,    MENTAL,         230},
{(char *)"Centauri Medtech",            233,    MENTAL,         230},
{(char *)"Minbari Medtech",             234,    MENTAL,         230},
{(char *)"Vorlon Medtech",              235,    MENTAL,         230},
{(char *)"Shadow Medtech",              236,    MENTAL,         230},
{(char *)"Misc Medtech",                237,    MENTAL,         230},
{(char *)"Communications",              238,    MENTAL,         -1},
{(char *)"Gunner",              GUNNER,         PHYSICAL,       -1},
{(char *)"Narn Gunner",		GUNNER+1,	PHYSICAL,	GUNNER},
{(char *)"Earth Gunner",	GUNNER+2,	PHYSICAL,	GUNNER},
{(char *)"Centauri Gunner",     GUNNER+3,       PHYSICAL,       GUNNER},
{(char *)"Minbari Gunner",	GUNNER+4,	PHYSICAL,	GUNNER},
{(char *)"Vorlon Gunner",	GUNNER+5,	PHYSICAL,	GUNNER},
{(char *)"Shadow Gunner",	GUNNER+6,	PHYSICAL,	GUNNER},
{(char *)"Misc Gunner",		GUNNER+7,	PHYSICAL,	GUNNER},
{(char *)"Narn Vehicle Gunner", GUNNER+8,       PHYSICAL,       GUNNER+1},
{(char *)"Earth Vehicle Gunner",GUNNER+9,       PHYSICAL,       GUNNER+2},
{(char *)"Centauri Vehicle Gunner",GUNNER+10,   PHYSICAL,       GUNNER+3},
{(char *)"Minbari Vehicle Gunner",GUNNER+11,    PHYSICAL,       GUNNER+4},
{(char *)"Vorlon Vehicle Gunner",GUNNER+12,     PHYSICAL,       GUNNER+5},
{(char *)"Shadow Vehicle Gunner",GUNNER+13,     PHYSICAL,       GUNNER+6},
{(char *)"Misc Vehicle Gunner", GUNNER+14,      PHYSICAL,       GUNNER+7},
{(char *)"Narn Fighter Gunner", GUNNER+15,      PHYSICAL,       GUNNER+1},
{(char *)"Earth Fighter Gunner",GUNNER+16,      PHYSICAL,       GUNNER+2},
{(char *)"Centauri Fighter Gunner",GUNNER+17,   PHYSICAL,       GUNNER+3},
{(char *)"Minbari Fighter Gunner",GUNNER+18,    PHYSICAL,       GUNNER+4},
{(char *)"Vorlon Fighter Gunner",GUNNER+19,     PHYSICAL,       GUNNER+5},
{(char *)"Shadow Fighter Gunner",GUNNER+20,     PHYSICAL,       GUNNER+6},
{(char *)"Misc Fighter Gunner", GUNNER+21,      PHYSICAL,       GUNNER+7},
{(char *)"Narn Ship/Base Gunner",GUNNER+22,     PHYSICAL,       GUNNER+1},
{(char *)"Earth Ship/Base Gunner",GUNNER+23,    PHYSICAL,       GUNNER+2},
{(char *)"Centauri Ship/Base Gunner",GUNNER+24, PHYSICAL,       GUNNER+3},
{(char *)"Minbari Ship/Base Gunner",GUNNER+25,  PHYSICAL,       GUNNER+4},
{(char *)"Vorlon Ship/Base Gunner",GUNNER+26,   PHYSICAL,       GUNNER+5},
{(char *)"Shadow Ship/Base Gunner",GUNNER+27,   PHYSICAL,       GUNNER+6},
{(char *)"Misc Ship/Base Gunner",GUNNER+28,     PHYSICAL,       GUNNER+7},
{(char *)"Piloting",		PILOTING,	PHYSICAL,	-1},
{(char *)"Narn Piloting",	PILOTING+1,	PHYSICAL,	PILOTING},
{(char *)"Earth Piloting",	PILOTING+2,	PHYSICAL,	PILOTING},
{(char *)"Centauri Piloting",   PILOTING+3,     PHYSICAL,       PILOTING},
{(char *)"Minbari Piloting",	PILOTING+4,	PHYSICAL,	PILOTING},
{(char *)"Vorlon Piloting",	PILOTING+5,	PHYSICAL,	PILOTING},
{(char *)"Shadow Piloting",	PILOTING+6,	PHYSICAL,	PILOTING},
{(char *)"Misc Piloting",	PILOTING+7,	PHYSICAL,	PILOTING},
{(char *)"Narn Vehicle Piloting",PILOTING+8,    PHYSICAL,       PILOTING+1},
{(char *)"Earth Vehicle Piloting",PILOTING+9,   PHYSICAL,       PILOTING+2},
{(char *)"Centauri Vehicle Piloting",PILOTING+10,PHYSICAL,      PILOTING+3},
{(char *)"Minbari Vehicle Piloting",PILOTING+11,PHYSICAL,       PILOTING+4},
{(char *)"Vorlon Vehicle Piloting",PILOTING+12, PHYSICAL,       PILOTING+5},
{(char *)"Shadow Vehicle Piloting",PILOTING+13, PHYSICAL,       PILOTING+6},
{(char *)"Misc Vehicle Piloting",PILOTING+14,   PHYSICAL,       PILOTING+7},
{(char *)"Narn Fighter Piloting",PILOTING+15,   PHYSICAL,       PILOTING+1},
{(char *)"Earth Fighter Piloting",PILOTING+16,  PHYSICAL,       PILOTING+2},
{(char *)"Centauri Fighter Piloting",PILOTING+17,PHYSICAL,      PILOTING+3},
{(char *)"Minbari Fighter Piloting",PILOTING+18,PHYSICAL,       PILOTING+4},
{(char *)"Vorlon Fighter Piloting",PILOTING+19, PHYSICAL,       PILOTING+5},
{(char *)"Shadow Fighter Piloting",PILOTING+20, PHYSICAL,       PILOTING+6},
{(char *)"Misc Fighter Piloting",PILOTING+21,   PHYSICAL,       PILOTING+7},
{(char *)"Narn Ship/Base Piloting",PILOTING+22, PHYSICAL,       PILOTING+1},
{(char *)"Earth Ship/Base Piloting",PILOTING+23,PHYSICAL,       PILOTING+2},
{(char *)"Centauri Ship/Base Piloting",PILOTING+24,PHYSICAL,    PILOTING+3},
{(char *)"Minbari Ship/Base Piloting",PILOTING+25,PHYSICAL,     PILOTING+4},
{(char *)"Vorlon Ship/Base Piloting",PILOTING+26,PHYSICAL,      PILOTING+5},
{(char *)"Shadow Ship/Base Piloting",PILOTING+27,PHYSICAL,      PILOTING+6},
{(char *)"Misc Ship/Base Piloting",PILOTING+28, PHYSICAL,       PILOTING+7},
{(char *)"Technician",          TECHN,          MENTAL,  -1},
{(char *)"Narn Technician",     TECHN+1,        MENTAL,  TECHN},
{(char *)"Earth Technician",    TECHN+2,        MENTAL,  TECHN},
{(char *)"Centauri Technician", TECHN+3,        MENTAL,  TECHN},
{(char *)"Minbari Technician",  TECHN+4,        MENTAL,  TECHN},
{(char *)"Vorlon Technician",   TECHN+5,        MENTAL,  TECHN},
{(char *)"Shadow Technician",   TECHN+6,        MENTAL,  TECHN},
{(char *)"Misc Technician",     TECHN+7,        MENTAL,  TECHN},
{(char *)"Narn Fighter Technician",TECHN+8,     MENTAL,   TECHN+1},
{(char *)"Earth Fighter Technician",TECHN+9,    MENTAL,   TECHN+2},
{(char *)"Centauri Fighter Technician",TECHN+10,MENTAL,   TECHN+3},
{(char *)"Minbari Fighter Technician",TECHN+11, MENTAL,   TECHN+4},
{(char *)"Vorlon Fighter Technician",TECHN+12,  MENTAL,   TECHN+5},
{(char *)"Shadow Fighter Technician",TECHN+13,  MENTAL,   TECHN+6},
{(char *)"Misc Fighter Technician",TECHN+14,    MENTAL,   TECHN+7},
{(char *)"Narn Ship/Base Technician",TECHN+15,  MENTAL,   TECHN+1},
{(char *)"Earth Ship/Base Technician",TECHN+16, MENTAL,   TECHN+2},
{(char *)"Centauri Ship/Base Technician",TECHN+17,MENTAL, TECHN+3},
{(char *)"Minbari Ship/Base Technician",TECHN+18,MENTAL,  TECHN+4},
{(char *)"Vorlon Ship/Base Technician",TECHN+19, MENTAL,  TECHN+5},
{(char *)"Shadow Ship/Base Technician",TECHN+20, MENTAL,  TECHN+6},
{(char *)"Misc Ship/Base Technician",TECHN+21,   MENTAL,  TECHN+7},
{(char *)"Drakh Language",TECHN+22,              MENTAL,  33},
{(char *)"Hurr Language",TECHN+23,               MENTAL,  33},
{(char *)"Yolu Language",TECHN+24,               MENTAL,  33},
{(char *)"Ch'Lonas Language",TECHN+25,           MENTAL,  33},
{(char *)"Koulani Language",TECHN+26,            MENTAL,  33},
{(char *)"Unused",	TECHN+27,            MENTAL,  -1},
{(char *)"Unused",	TECHN+28,            MENTAL,  -1},
{(char *)"Unused",	TECHN+29,            MENTAL,  -1},
{(char *)"Unused",	TECHN+30,            MENTAL,  -1},
{(char *)"Unused",	TECHN+31,            MENTAL,  -1},
{(char *)"Unused",	TECHN+32,            MENTAL,  -1},
{(char *)"Unused",	TECHN+33,            MENTAL,  -1},
{(char *)"Unused",	TECHN+34,            MENTAL,  -1},
{(char *)"Unused",	TECHN+35,            MENTAL,  -1},
{(char *)"Unused",	TECHN+36,            MENTAL,  -1},
{(char *)"Unused",	TECHN+37,            MENTAL,  -1},
{(char *)"Unused",	TECHN+38,            MENTAL,  -1},
{(char *)"Unused",	TECHN+39,            MENTAL,  -1},
{(char *)"Ascension",	TELEP,               MENTAL,  -1},
{(char *)"Meditation",  TELEP+1,             MENTAL,  TELEP},
{(char *)"Enlightenment",TELEP+2,            MENTAL,  TELEP},
{(char *)"Awareness",   TELEP+3,             MENTAL,  TELEP},
{(char *)"Psionics",    TELEP+4,             PSI,     -1},
{(char *)"Telepathy",   TELEP+5,             PSI,     -1},
{(char *)"Telekinesis", TELEP+6,             PSI,     -1},
{(char *)"Blocking",    TELEP+7,             PSI,     -1},
{(char *)"Mind Heal",   TELEP+8,             PSI,     -1},
{(char *)"Body Heal",   TELEP+9,             PSI,     -1},
{(char *)"Read Mind",   TELEP+10,            PSI,     -1},
{(char *)"Broadcast",   TELEP+11,            PSI,     -1},
{(char *)"Mental Damage",TELEP+12,           PSI,     -1},
{(char *)"Physical Damage",TELEP+13,         PSI,     -1},
{(char *)"Move Thing",   TELEP+14,           PSI,     -1},
{(char *)"Psionic Navigation",TELEP+15,      PSI,     -1},
{(char *)"Mind Probe",   TELEP+16,           PSI,     -1},
{(char *)"Space Scan",   TELEP+17,           PSI,     -1},
{(char *)"Space Attack", TELEP+18,           PSI,     -1},
{(char *)"Psionic Sense",TELEP+19,           PSI,     -1},
{(char *)"Psionic Seduce",TELEP+20,          PSI,     -1},
{(char *)"Psionic Charm",TELEP+21,           PSI,     -1},
{(char *)"Ego Whip",     TELEP+22,           PSI,     -1},
{(char *)"Inspire Awe",  TELEP+23,           PSI,     -1},
{(char *)"Suggestion",   TELEP+24,           PSI,     -1},
{(char *)"Terrorize",    TELEP+25,           PSI,     -1},
{(char *)"Mental Siezure",TELEP+26,          PSI,     -1},
{(char *)"Comfort",      TELEP+27,           PSI,     -1},
{(char *)"Sense Attitude",TELEP+28,          PSI,     -1},
{(char *)"Mind Wipe"     ,TELEP+29,          PSI,     -1},
{(char *)NULL,			0,	0}};

Skilltypes SkillTypes[]={
{(char *)NULL},
{(char *)"ATHLETIC"},
{(char *)"MENTAL"},
{(char *)"PHYSICAL"},
{(char *)"SOCIAL"},
{(char *)"PSI"},
{(char *)NULL}
};

Skill Advans[]={
{(char *)NULL,			0,	0},
{(char *)"Extra Edge",		1,	0},
{(char *)"Land Grant",		2,	0},
{(char *)"Title",		3,	0},
{(char *)"Wealth",		4,	0},
{(char *)"Well Equipped",	5,	0},
{(char *)"Contacts",		6,	0},
{(char *)"Reputation",		7,	0},
{(char *)"Well Connected",	8,	0},
{(char *)"Toughness",		9,	0},
{(char *)NULL,			0,	0}};

void do_testskill(player,cause,extra)
dbref player,cause;
int extra;
{
int i;

notify(player,"Available Skills\n----------------");

for(i=1;i<SKILLSs;i++) {
	notify(player,tprintf("%d) %32s - %s",i,Skills[i].name,SkillTypes[Skills[i].charact].name));
	}
}

void skills_view(object,viewer)
dbref object,viewer;
{
int x;
float tmp;
notify(viewer,"| Skill#\t\tSkill Name\tValue\t\t\t|");
notify(viewer,"-----------------------------------------------------------------");
for(x=1;x<=SKILLS;x++) {
tmp=read_flist(object,x,SKILL_A);
if(tmp>0) {
 notify(viewer,tprintf("| %3d) %27s\t%f\t\t|",x,Skills[x].name,tmp));
 }
}
notify(viewer,"-----------------------------------------------------------------");
}

void advan_view(object,viewer)
dbref object,viewer;
{
int x,tmp;
notify(viewer,"| Advantage#\tAdvantage Name\t\tValue\t\t\t|");
notify(viewer,"-----------------------------------------------------------------");
for(x=1;x<=ADVANS;x++) {
tmp=read_ilist(object,x,ADVAN_A);
if(tmp>0) {
 notify(viewer,tprintf("| %3d) %22s\t\t%3d\t\t\t|",x,Advans[x].name,tmp));
 }
}
notify(viewer,"-----------------------------------------------------------------");
}

void qattrib_view(object,viewer)
dbref object,viewer;
{
int x;
float bld,intu,he,mf;
float food,water;
bld=read_flist(object,1,ATRIBS_A);
intu=read_flist(object,2,ATRIBS_A);
he=read_flist(object,6,ATRIBS_A);
mf=read_flist(object,7,ATRIBS_A);

food=read_flist(object,11,ATRIBS_A);
water=read_flist(object,12,ATRIBS_A);

notify(viewer,"QuickStats");
notify(viewer,tprintf("Health: %.2f / %.2f\tMental Fatigue: %.2f / %.2f\nFood: %.2f  Water: %.2f",he,5*bld,mf,5*intu,food,water));
}

void Do_Bestow(player,skill,level)
int player,skill;
float level;
{

/*if(!QuietIsIcReg(player)) return;*/
if(!(Flags3(player)&REGISTERED)) return;
if(skill <=0 || skill > SKILLS) return;

change_flist(player,skill,level,SKILL_A);

}

void attrib_view(object,viewer)
dbref object,viewer;
{

int x;
float bld,intu,ref,lrn,cha,health,fatig,psi,food,water;
float exp;
bld=read_flist(object,1,ATRIBS_A);
intu=read_flist(object,2,ATRIBS_A);
ref=read_flist(object,3,ATRIBS_A);
lrn=read_flist(object,4,ATRIBS_A);
cha=read_flist(object,5,ATRIBS_A);
psi=read_flist(object,8,ATRIBS_A);

health=read_flist(object,6,ATRIBS_A);
fatig=read_flist(object,7,ATRIBS_A);
food=read_flist(object,11,ATRIBS_A);
water=read_flist(object,12,ATRIBS_A);
exp=read_flist(object,10,ATRIBS_A);
notify(viewer," __________________________________________________");
VMnotify(viewer,"| Character Stats: %30s  |",Name(object));
notify(viewer,"-----------------------------------------------------------------");
notify(viewer,tprintf("| Build:    %f \t\tIntuition:      %f\t|",bld,intu));
notify(viewer,tprintf("| Reflexes: %f \t\tLearning:       %f\t|",ref,lrn));
notify(viewer,tprintf("| Charisma: %f \t\tPsionics:       %f\t|",cha,psi));
notify(viewer,"-----------------------------------------------------------------");
notify(viewer,"| Characteristics\t\t\t\t\t\t|");
notify(viewer,"-----------------------------------------------------------------");
notify(viewer,tprintf("| Health:   %.1f/%.1f\t\tMental Fatigue: %.1f/%.1f\t|",health,5*bld,fatig,5*intu));
notify(viewer,tprintf("| Food:	   %5.2f\t\tWater:         %5.2f\t\t|",food,water));
notify(viewer,tprintf("| Exp:	    %f\t\tPsionics:       %f\t|",exp,18-(psi+lrn)  ));
notify(viewer,tprintf("| Athletic: %f \t\tMental:         %f\t|",18-(bld+ref),18-(intu+lrn)));
notify(viewer,tprintf("| Physical: %f \t\tSocial:         %f\t|",18-(ref+intu),18-(intu+cha)));
notify(viewer,"-----------------------------------------------------------------");
}

void see_skills(player,cause,key,message)
dbref player,cause;
int key;
char *message;
{
dbref it;
int tmp,d,x;

if(!(Flags3(player) & PREREG)) {
	notify(player,"You must go through the player creation process");
	return;
	}

if(strlen(message)>0) it=match_thing(player,message);
else it=player;
if(it==NOTHING) {return;}
if(it!=player && !Wizard(player)) { notify(player,"Permission Denied"); return;}
attrib_view(it,player);
d=0;
for(x=1;x<=ADVANS;x++) {
tmp=read_ilist(it,x,ADVAN_A);
if(tmp>0) d=1;
}
if(d==1) advan_view(it,player);
skills_view(it,player);
}

void do_speak(player,cause,key,message)
dbref player,cause;
int key;
char *message;
{
int skill,lang;
char tmm[100];
if(!IsIcReg(player)) return;
lang=atoi(message);
if(lang < 33 || (lang > 101 && lang < 319) || lang > 323) {
	notify(player,"That is not a language.");
	return;
	}

skill=get_max_skill(player,lang);
if(skill <=0) {
	notify(player,"You dont speak that language.");
	return;
	}
sprintf(tmm,"%d",lang);
atr_add_raw(player,LANG_A,tmm);
notify(player,tprintf("You begin to speak %s.",Skills[lang].name));
}


void see_qskills(player,cause,key,message)
dbref player,cause;
int key;
char *message;
{
dbref it;

if(!IsIcReg(player)) return;

if(strlen(message)>0) it=match_thing(player,message);
else it=player;
if(it==NOTHING) {return;}
if(it!=player && !Wizard(player)) { notify(player,"Permission Denied"); return;}
qattrib_view(it,player);
}

double read_skill2(player,skill)
int player,skill;
{

if(skill<0 || skill> SKILLS) return(0);
return(read_flist(player,skill,SKILL_A));

}

double read_skill(player,skill)
int player,skill;
{

return(get_max_skill(player,skill));
}


int skill_check(player,	skill, diffact)
dbref player;
int skill,diffact;
{
double level=0,chance,base,roll,nroll,sgn,fx;
double bld,intu,ref,lrn,cha,psi,tot;
int fina;

level=get_max_skill(player,skill);
if(level < .1 && !Wizard(player)) return(-5);
bld=read_flist(player,1,ATRIBS_A);
intu=read_flist(player,2,ATRIBS_A);
ref=read_flist(player,3,ATRIBS_A);
lrn=read_flist(player,4,ATRIBS_A);
cha=read_flist(player,5,ATRIBS_A);
psi=read_flist(player,8,ATRIBS_A);
switch(Skills[skill].charact)
	{

		case ATHLETIC:
			base=18-(bld+ref)*.7;
			break;
		case MENTAL:
			base=18-(intu+lrn)*.7;
			break;
		case PHYSICAL:
			base=18-(ref+intu)*.7;
			break;
		case SOCIAL:
			base=18-(intu+cha)*.7;
			break;
		case PSI:
			base=18-(lrn+psi)*.7;
			break;
		default:	return;
	}	

diffact-=4;
chance=(.89*base + diffact) - 2.42*level;

tot=0;
roll=12;

while(roll>=12) {
roll=random()%12+1;
/* A cheap ploy to make it less random */
nroll=(roll-6.0)/6.0;
if(nroll < 0) {
	sgn=-1;
	nroll=-nroll;
	}
else sgn=1;
fx=pow(nroll,2.0);
/*if(nroll!=0) fx=pow(nroll,2.0)/pow(nroll,.5);
else fx=0;
*/
tot+=(fx*sgn*6.0+6.0);
}

roll=tot;

/*
notify(player,tprintf("Roll=%f Level=%f Chance=%f base=%f diffact=%d",roll,level,chance,base,diffact));
*/
fina=(int) (roll-chance);

 Increase_Skill(player,skill);

return fina;
}

double get_max_skill(player,skill)
dbref player;
int skill;
{
int tmpskill,depth=0;
double tmpmax,tmpval;

if(skill<0 || skill> SKILLS) return(0);
tmpskill=skill;
tmpmax=read_flist(player,tmpskill,SKILL_A);

while(depth < 22 && Skills[tmpskill].parent!=-1 && tmpskill!=-1) {
depth++;
tmpskill=Skills[tmpskill].parent;
tmpval=read_flist(player,tmpskill,SKILL_A) - depth;

if(tmpval > tmpmax) {
	tmpmax=tmpval;
	}
}
if(depth==22) notify(1,"Depth max reached");
return tmpmax;
}

void Increase_Skill(player,skill) 
dbref player;
int skill;
{
int tim2,tim1,tim;
double pts,cpts,ccc,exp,learn;
char *tmp;
char tmm[100];

if(Flags3(where_is(player))&PREREG) return;

if(skill < 0 || skill> SKILLS) return;
tim2=(int)time(0);
tmp=vget_a(player,TSKILL_A);
tim1=atoi(tmp);
tim=tim2-tim1;
sprintf(tmm,"%d",tim2);
if(Skills[skill].parent!=33)
atr_add_raw(player,TSKILL_A,tmm);

if(tim > 3600) tim=3600;
if(Skills[skill].parent==33) {
	if(tim > 100) tim=100;
}
learn=read_flist(player,4,ATRIBS_A);
pts=tim/3600.0;
pts=pts/20;
cpts=read_flist(player,skill,SKILL_A);
if(cpts >=learn+.5)  return;

if(cpts < 1) cpts=1;
pts=pts/(2*(cpts-.5));
exp=read_flist(player,10,ATRIBS_A);
exp+=(pts/10.0);
ccc=cpts;
if(ccc<1) ccc=1;
pts=pts/ccc;
cpts+=pts;
change_flist(player,skill,cpts,SKILL_A);
if(Skills[skill].parent!=33)
change_flist(player,10,exp,ATRIBS_A);

}



static char *rcsskillsc="$Id: skills.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
