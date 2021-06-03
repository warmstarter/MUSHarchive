#define WP 1   /*Define weaponmaster char db*/

#define WEAPONTHINGY 30

/*typedef int dbref;*/
typedef struct Scombat Combat;
typedef struct Scombatheader CombatHeader;

struct Scombatheader {
int size;
Combat *start;
Combat *last;
};

struct Scombat {
dbref player;
dbref victim;
Combat *next;
};


CombatHeader CombatQList;
CombatHeader HealList;

extern void initCombat();
extern int AddCombat(CombatHeader*,dbref,dbref);
extern int IsInCombat(CombatHeader*,dbref);
extern int RemoveCombat(CombatHeader*,dbref);
extern void DoAttack(Combat *);
extern void DoRest(Combat *);
extern void AttackAll();
extern void DoPlayerAttack(dbref);
extern void Hurt();

