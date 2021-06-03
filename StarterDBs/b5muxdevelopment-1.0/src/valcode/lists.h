extern int change_flist(dbref,int,float,int);
extern float read_flist(dbref,int,int);
extern int change_ilist(dbref,int,int,int);
extern int read_ilist(dbref,int,int);
extern char *vfetch_attribute(dbref,char *);
extern int att_dbref(dbref,char *);
extern char *vget_a(dbref,int);
extern int new_ValDB();
extern void read_ValDB();
extern void write_ValDB();
extern int read_ValDB_interface();
extern int read_ValDBMax_interface();
extern int read_ValDBFind_interface();
extern int read_ValDBNum_interface();
extern int write_ValDB_interface();
extern int PDB_ValDB();
extern int cset_ValDB();
extern int sset_ValDB();
extern int do_VCheck();
extern int new_ValDB_interface();

typedef struct sDBItem DBItem;
typedef struct osDBItem oDBItem;

struct sDBItem {
int commods[3100];
double skills[600];
int used;
int object;
};

struct nsDBItem {
int commods[50];
double skills[50];
int cid[100];
int sid[100];
int cmax,cmin,smax,smin,cidmax,sidmax;
int used;
int object;
};

struct osDBItem {
int commods[3100];
double skills[600];
int used;
int object;
};



extern DBItem ValDB[];
/*extern oDBItem oValDB[];*/
extern int ValDBMax;
static char *rcslistsh="$Id: lists.h,v 1.1 2001/01/15 03:23:16 wenk Exp $";
