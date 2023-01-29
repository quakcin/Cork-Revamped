
#include <stdlib.h>
#include <stdbool.h>

typedef struct game_s game_t;
typedef struct sched_s sched_t;

#define ITEM_VERY_NEAR 256 * 16
#define ITEM_PICKUP_DIST 350

typedef struct item_s
{
  float x, y;
  bool in_inventory;
  float lpx; 
  float lpy;
  float lpdist;
} item_t;

item_t * item_ctor (game_t * game);
void item_dtor (item_t * self);
void item_dir_suggest (sched_t * sched, void * ptr, int val);
void item_pickup_tip (sched_t * sched, void * ptr, int val);

void item_save (game_t * game, int save_no);
void item_load (item_t * self, int save_no);