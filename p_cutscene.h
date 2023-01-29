
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_video.h>

typedef struct proc_s proc_t;
typedef struct engine_s engine_t;

void p_cutscene_init (proc_t * self);
void p_cutscene_update (proc_t * self);
void p_cutscene_dtor (proc_t * self);

typedef struct p_cutscene_s
{
  ALLEGRO_VIDEO * video;
  engine_t * engine;
} p_cutscene_t;