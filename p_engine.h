
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_video.h>

typedef struct proc_s proc_t;
typedef struct pm_s pm_t;

typedef struct engine_s
{
	ALLEGRO_DISPLAY * display;
  ALLEGRO_BITMAP * buffer;
  pm_t * pm;

  unsigned int width;
  unsigned int height;
  unsigned int save_no;

  unsigned int buffer_width;
  unsigned int buffer_height;

  // HRTimer
  unsigned long long ticks;
  unsigned long long last_tick;
  unsigned long long last_clock;
  float delta;
  float refresh; 

  bool is_fullscreen;

  struct ipc_s
  {
    int target;
    int code;
  } ipc;

  bool use_captions;
  long gamma;

} engine_t;

void p_engine_init (proc_t * self);
void p_engine_update (proc_t * self);
void p_engine_dtor (proc_t * self);
