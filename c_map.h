
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

// #include <inttypes.h>


#define MAP_BEG_FLOOR 0
#define MAP_END_FLOOR 16
#define MAP_BEG_COLLIDABLE 16
#define MAP_BEG_WALL 16
  #define MAP_BEG_DOOR 100
  #define MAP_END_DOOR 128
  #define MAP_BEG_PROPS 128
  #define MAP_END_WALL 128
#define MAP_END_COLLIDABLE 158
#define MAP_END_PROPS 250


typedef struct game_s game_t;
typedef struct sched_s sched_t;

typedef struct map_s
{
  uint8_t tiles[512][512];
  ALLEGRO_COLOR colors[4][2];
  ALLEGRO_BITMAP * texture_buffer [0xFF + 1]; // not all of 'em are poulated!
  ALLEGRO_BITMAP * overlays[5];
  long points_count; // TODO: Check if not leaking
  uint8_t resspack; // ressourcess pack
  uint8_t color;
  uint8_t darkness;
  int lightness;
  int apprx_lightness;
} map_t;

void map_dtor (map_t * self);
map_t * map_ctor (game_t * game);