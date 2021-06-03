#define PMOD 10000000


typedef struct Scommod Commod;
typedef struct Scurrency Currency;
typedef struct Splans Plans;
typedef struct SPackList PackList;

extern int space_parse(char *, char **,int);

struct SPackList {
int db;
int commod;
};



struct Scommod {
char *name;
int num;
float size;
double baseprice;
int component;
};

struct Scurrency {
char *name;
int num;
float worth;	/*Somewhat a misnomer. How many of this currency it takes for 1 generic money*/
float velocity;
int supply;
};

struct Splans {
int nplan;
char *name;
int type;
int eng;
int diff;
int nin;
int amtout;
int commout;
int amin[25];
int commin[25];
};

#define COMODS 1898
#define MONEYS 8
#define MAXCOMODS  1900
#define PLANS 1900
extern Commod EconItems[];
extern Currency Moneys[];

void do_vaddmis();
void factory_update();
void factory_plandef();
void factory_status();
void factory_pset();
void factory_build();
int get_fact_limit();
void econload();
void econunload();
void vmons();
void vcoms();
void do_vepay();
void do_vmove();
void do_veconload();
void do_veconunload();
void do_vebuy();
void do_vesell();
void ebuy();
void esell();
int econloadparent();
void econcommod();
void econcommod_pr();
void econ_commod();
void econ_commod_price();
void do_supply();
void do_access();
void do_withdraw();
void do_deposit();
void do_balance();
void econcommod_price();
void see_money();
#define IsVehicle(x) 	((Flags2(x) & VEHICLE) !=0)


extern Plans plans[];
extern PackList PList[];

static char *rcseconh="$Id: econ.h,v 1.1 2001/01/15 03:23:16 wenk Exp $";
