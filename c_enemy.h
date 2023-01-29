#include <stdbool.h>

typedef struct game_s game_t;
typedef struct sched_s sched_t;
typedef struct viz_s viz_t;

#define CORK_ATTACK_DIST 256 * 1.5
#define CORK_AGRO_DIST 1024
#define CROK_SAFE_DIST 12 * 256 

// #define CORK_HIT_DIST 256
#define CORK_PIVOT_DIST 256 * 3
#define CORK_DELTA 40


typedef struct enemy_s
{
  viz_t * viz;
  // viz_t * expl; 
  // explosion when breaking trought stuff

  float x, y, ang;
  float facing;

  float speed;
  int chase; // chase time in 
  int atacking;
  bool audio; // if < 0, reset -> enemy update() coroutine

  bool is_spawned;
  bool door_check;
  bool agroed;

} enemy_t;

enemy_t * enemy_ctor (game_t * game);
void enemy_dtor (enemy_t * self);
// void enemy_update (enemy_t * self, game_t * game);
void enemy_update (sched_t * sched, void * ptr, int val);

void enemy_save (game_t * game, int save_no);
void enemy_load (enemy_t * self, int save_no);
void spawn_enemy_rutines (game_t * game);