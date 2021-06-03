#include <stdio.h>
#include "combat.h"


void main() {

printf("Initializing Combat Q\n");
InitCombat();
contents();
printf("Adding 5 to it\n");
AddCombat(5);
printf("Isin 5 %d\n",IsInCombat(5));
contents();
printf("Adding 5 to it\n");
AddCombat(5);
contents();
printf("Adding 12 to it\n");
AddCombat(12);
contents();
printf("Removing 5\n");
RemoveCombat(5);
contents();
printf("Adding 14 to it\n");
AddCombat(14);
printf("Isin 5 %d\n",IsInCombat(5));
printf("Isin 12 %d\n",IsInCombat(12));
contents();
}
