/*
  map.h
  Structure definitions and what not for the maps for the mechs.

  4.8.93- rdm created
  6.16.93- jb modified, added hex_struct
*/

/* #defines */
#define MAX_MECHS_PER_MAP 150
#define P(x) 	x
/* map links */
#define MAP_UP 0
#define MAP_DOWN 1
#define MAP_RIGHT 2
#define MAP_LEFT 3

/* Map size */
#define MAPX 1000
#define MAPY 1000
#define MAP_NAME_SIZE 12
#define NUM_MAP_LINKS 4
#define MAP_UNUSED_SIZE 10
#define DEFAULT_MAP_WIDTH 21
#define DEFAULT_MAP_HEIGHT 11
#define MAP_DISPLAY_WIDTH 21
#define MAP_DISPLAY_HEIGHT 9
#define MAP_DISPLAY_WIDTH2 10
#define MAP_DISPLAY_HEIGHT2 4
#define MAX_ELEV 9
#define GRASSLAND ' '
#define HEAVY_FOREST '"'
#define LIGHT_FOREST '`'
#define WATER '~'
#define ROUGH '%'
#define MOUNTAINS '^'
#define ROAD '#'
#define BUILDING '@'

struct hex_struct
{
  char A, B;  /* ID characters */
  char terrain;
  char elev;
};

typedef struct hex_struct hex_struct;

struct map_struct {
  dbref mynum;                              /* My dbref */
  hex_struct **map;                         /* The map */
  int map_width;                            /* Width of map <MAPX  */
  int map_height;                           /* Height of map */
  dbref mechsOnMap[MAX_MECHS_PER_MAP];      /* Mechs on the map */
  char LOSinfo[MAX_MECHS_PER_MAP][MAX_MECHS_PER_MAP];
  dbref city;                               /* City link */
  dbref links[NUM_MAP_LINKS];               /* Maps to the sides */
  char mapname[MAP_NAME_SIZE+1];
  char unused[MAP_UNUSED_SIZE];
};

/* The functions */
extern void map_view P((dbref, void *, char *));
extern void map_addhex P((dbref, void *, char *));
extern void map_addrow P((dbref, void *, char *));
extern void map_mapname P((dbref, void *, char *));
extern void map_loadmap P((dbref, void *, char *));
extern void map_savemap P((dbref, void *, char *));
extern void map_setmapsize P((dbref, void *, char *));
extern void map_listmechs P((dbref, void *, char *));
extern void map_clearmechs P((dbref, void *, char *));
extern void map_addlink P((dbref, void *, char *));
extern void map_startxy P((dbref, void *, char *));
extern void newfreemap P((dbref, void **, int));
extern void clear_LOSinfo P((dbref, void *));
