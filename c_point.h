
#include <stdbool.h>

typedef struct game_s game_t;
typedef struct sched_s sched_t;
typedef struct map_s map_t;

#define MAP_POINTS_DIST 1024
#define MAP_POINTS_RESTORE 2049
#define MAX_POINT_ITER 2048

typedef struct point_s
{
  struct point_s * next;
  float x;
  float y;
  bool enabled;
} point_t;

void try_creating_point (game_t * game);
point_t * find_nearest_point (game_t * game, float x, float y);
void task_create_point (sched_t * sched, void * ptr, int val);
void reset_points (game_t * game);
void points_save (game_t * game, int save_no);
void points_load (map_t * map, int save_no);