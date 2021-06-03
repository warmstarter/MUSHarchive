#include "vmspace/vmspace.h"

typedef struct Sfreqs freqs;


struct Sfreqs {
long int freq;
};

extern freqs Freqs[];


long int VMShipsCanTalkMatrix[200][200];

int VMShipID[MAXSHIP];
int VMBackID[200];
int VMBackMax;

static char *rcscommh="$Id: comm.h,v 1.1 2001/01/15 03:23:15 wenk Exp $";
