
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#define WALL_JUMP 160;

#define BOBBING_STEP 0.5

#define PLAYER_WALKING_AUDIO_DELAY 500

typedef struct engine_s engine_t;
typedef struct game_s game_t;
typedef struct sched_s sched_t;

typedef struct player_s
{
  float yoff, dyoff;
  float x, y, ang;
  float vx, vy;
  bool is_alive;
} player_t;

player_t * player_ctor (engine_t * engine);
void player_dtor (player_t * self);
void player_io_handler (game_t * self, engine_t * engine);
void player_update (game_t * game);
void player_movement_handler (player_t * self, game_t * game);
void player_kill (sched_t * sched, void * ptr, int val);
