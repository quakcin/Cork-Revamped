
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

typedef struct game_s game_t;

typedef struct audio_s
{
  char name[32];
  unsigned int normal_count;
  unsigned int muffled_count;
  ALLEGRO_SAMPLE * normal[4];
  ALLEGRO_SAMPLE * muffled[4];
  struct audio_s * next;
} audio_t;


bool audio_load (audio_t * self, char * path, char * name);
audio_t * audio_ctor (void);
void audio_dtor (audio_t * self);
void audio_play (audio_t * self, game_t * game, char * name, float x, float y);
void audio_play_master (audio_t * self, char * name);
