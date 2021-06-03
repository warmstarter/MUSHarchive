#define TECHS 3


typedef struct sTechs TTechs;


struct sTechs {
char *name;
int num;
int needs;
int tech[5];
int diff;
int cneeds;
int cmods[5];
int amt[5];
};



extern TTechs Techs[];

void seetech();
void settech();
void techstat();


void tech_update();

