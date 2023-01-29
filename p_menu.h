

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_video.h>
#include <stdbool.h>

typedef struct menu_s menu_t;
typedef struct proc_s proc_t;
typedef struct engine_s engine_t;
typedef struct sched_s sched_t;
typedef struct pm_s pm_t;

typedef struct button_s
{
  struct button_s * next;
  void (*event)( struct button_s * self, struct menu_s * menu );
  char text[32];
  int value;
  bool enabled;
} button_t;

typedef struct menu_s
{
  button_t * buttons;
  button_t * swap;
  engine_t * engine;
  sched_t * sched;
  ALLEGRO_FONT * font;
  ALLEGRO_VIDEO * video;
  pm_t * pm;
  bool during_game;
  bool ghosting;

  int lmx, lmy; // remember mouse
                // so player doesn't get roteted
} menu_t;


button_t * page_save_or_load (bool is_saving);
button_t * page_options (menu_t * menu);

void p_menu_init (proc_t * self);
void p_menu_update (proc_t * self);
void p_menu_dtor (proc_t * self);
void page_opt_back (button_t * self, menu_t * menu);