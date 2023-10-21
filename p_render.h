

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

/* refaktor rozmiaru burfora 256 -> 64 */
#define BMW 64
#define BMWL (BMW - 1)
#define BMWS 6

#define SPRITE_BUFFER_SIZE 1024
#define ZBUFFER_SIZE 1920

typedef struct game_s game_t;

// typedef struct render_s
// {
//   ALLEGRO_FONT * font;
//   char captions[128];
//   long cap_timer;
//   long cap_height;
// } render_t;

typedef struct sprite_s
{
  long mx, my;
  float x, y, z;
  uint8_t txt;
} sprite_t;

/*
  Objects that are renderable, must be
  instanciated in viz(buffer) list in
  game object
*/
typedef struct viz_s
{
  struct viz_s * next;
  float * x;
  float * y;
  uint8_t txt;
} viz_t;

void p_render_init (proc_t * self);
void p_render_update (proc_t * self);
void p_render_dtor (proc_t * self);
viz_t * viz_ctor (game_t * game);
void viz_dtor (game_t * game);
void render_use_captions (pm_t * pm, char * msg, long time);