#define SKILLSs 324
#define SKILLS 366
#define ADVANS 9

typedef struct Sskill Skill;
typedef struct Sskilltypes Skilltypes;

struct Sskill {
char *name;
int num;
int charact;
int parent;
};

struct Sskilltypes
{
char *name;
};

#define ATHLETIC 	1
#define MENTAL 		2
#define PHYSICAL	3
#define SOCIAL		4
#define PSI		5

extern Skilltypes SkillTypes[];
extern Skill Skills[];
extern void skills_view(dbref,dbref);
extern void attrib_view(dbref,dbref);
extern void advan_view(dbref,dbref);
extern double get_max_skill();

static char *rcsskillsh="$Id: skills.h,v 1.1 2001/01/15 03:23:16 wenk Exp $";
