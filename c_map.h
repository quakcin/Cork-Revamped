
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

// #include <inttypes.h>

#define MAP_ANIMATION_FRAMES_OFFSET 45
#define MAP_COLLIDABLE_START 4
#define MAP_COLLIDABLE_END 37
#define MAP_SPRITES_START 20
#define MAP_LAMPS_START 32
#define MAP_LAMPS_END 41
#define MAP_WALL_START 4
#define MAP_WALL_END 21
#define MAP_SKY_START 0
#define MAP_SKY_END 5
#define MAP_DOOR_START 4
#define MAP_DOOR_END 9

#define MAP_ANIM_WALL 17
#define MAP_ANIM_PROP 29
#define MAP_ANIM_LAMP 33

#define MAP_CORK_BEG 109
#define MAP_CORK_ATTACK 141
#define MAP_CORK_SIZE 36

#define MAP_ITEM_BEG 145

#define MAP_EXPL 146
#define MAP_EXPL_BEG 147

#define MAP_COLLIDABLE_PROP_START 21
#define MAP_COLLIDABLE_PROP_END 37


typedef struct game_s game_t;
typedef struct point_s point_t;
typedef struct sched_s sched_t;
typedef struct item_s item_t;

typedef struct map_s
{
  uint8_t tiles [511][511];
  ALLEGRO_COLOR colors[4][2];
  ALLEGRO_BITMAP * texture_buffer [0xFF]; // not all of 'em are poulated!
  ALLEGRO_BITMAP * overlays[5];
  point_t * points[32][32];
  point_t * spawn;
  long points_count; // TODO: Check if not leaking
  uint8_t resspack; // ressourcess pack
  uint8_t color;
  uint8_t darkness;
  int lightness;
  int apprx_lightness;
} map_t;

void map_dtor (map_t * self);
map_t * map_ctor (game_t * game);
// void map_load (game_t * game);
void map_load_from_save (map_t * self, int no);
void map_update_animated_textures (sched_t * sched, void * ptr, int ticks);
bool map_line_of_sight (map_t * self, float x, float y, float dx, float dy);
void map_unlink_animated_texures (map_t * self);
void map_save (game_t * game, int no);
void map_load_texures (map_t * self);
void map_load_audio (game_t * game, int respack);