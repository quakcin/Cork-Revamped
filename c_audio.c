
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "c_audio.h"
#include "p_game.h"
#include "c_player.h"
#include "c_map.h"
#include "i_math.h"
#include "main.h"

audio_t * audio_ctor (void)
{
  audio_t * self = (audio_t *) malloc(sizeof(struct audio_s));
  __GUARD(self, "audio_t obj");

  if (self == NULL)
  {
    IERROR("Failed to create audio object");
    return NULL;
  }

  printf("[AUDIO] Create object at %p\n", self);

  strcpy(self->name, "__AUDIO");
  self->next = NULL;

  return self;
}


/*
  FIXME: Actuall destroy samples
*/
void audio_dtor (audio_t * self)
{
  audio_t * ptr;
  audio_t * nxt;
  int i;

  al_stop_samples();

  for (ptr = self->next; ptr != NULL; ptr = nxt)
  {
    nxt = ptr->next;
    for (i = 0; i < ptr->normal_count; i++)
      al_destroy_sample(ptr->normal[i]);

      al_destroy_sample(ptr->muffled[i]);

    __UNGUARD(ptr, "audio_t node");
    free(ptr);
  }
  __UNGUARD(self, "audio_t obj");
  free(self); 
}

audio_t * audio_find (audio_t * self, char * name)
{
  audio_t * ptr;

  for (ptr = self; ptr != NULL; ptr = ptr->next)
  {
    // printf("cmping: %s/%s\n", ptr->name, name);
    if (strcmp(ptr->name, name) == 0)
      return ptr;
  }

  return NULL;
}

void audio_play_master (audio_t * self, char * name)
{
  audio_t * aud = audio_find(self, name);

  // printf("Wsiuming %p\n", aud);

  if (aud == NULL)
  {
    IERROR("Audio does not exist!");
    return;
  }

  int idx = get_random_int(0, aud->normal_count - 1, 1);
  al_play_sample(aud->normal[idx], 1.0, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}

void audio_play (audio_t * self, game_t * game, char * name, float x, float y)
{

  audio_t * ptr = audio_find(self, name);
  if (ptr == NULL)
  {
    printf("[AUDIO] Could not play sample: %s\n", name);
    return;
  }
  // is sound muffled or not?
  ALLEGRO_SAMPLE * sample;

  // if (map_line_of_sight(game->map, game->player->x, game->player->y, x, y))
  //   sample = ptr->normal[get_random_long(0, ptr->normal_count - 1, 1)];
  // else
  //   sample = ptr->muffled[get_random_long(0, ptr->muffled_count - 1, 1)];

  if (sample == NULL)
    IERROR("[AUDIO] Encounterd NULL sample");

  // calculate pan and volume

  float ang = fmod(2 * ANG360 + atan2(y - game->player->y, x - game->player->x) - game->player->ang, ANG180);
  float pan = (fabs(ang - 3.1415) < 0.3) ? 0 : 2 * ang / ANG180 - 1;
  float vol = 1000 / (sqrt(pow(game->player->x - x, 2) + pow(game->player->y - y, 2)) + 1);

  if (vol > 3)
    vol = 3;

  al_play_sample(sample, vol, -1 * pan, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}


bool audio_load (audio_t * self, char * path, char * name)
{
  // first, create new list node
  audio_t * aud = (audio_t *) malloc(sizeof(struct audio_s));
  __GUARD(aud, "audio_t node");

  if (aud == NULL)
  {
    IERROR("Failed to create audio node");
    return false;
  }

  strcpy(aud->name, name);
  aud->next = self->next;
  self->next = aud;

  // load both normal and muffled
  int i = 0;
  char buff[0xFF];

  /* load normal sounds */

  do
  {
    sprintf(buff, "./DATA/%s/norm_%s_%d.ogg", path, name, i);
    aud->normal[i] = al_load_sample(buff);

    if (aud->normal[i++] == NULL)
      break;
    else
      printf("\tLOADED NORMAL AUDIO %s AS SAMPLE %d\n", buff, i);
  }
  while (i < 4);

  aud->normal_count = i - 1;
  // printf("Loaded %d normal sounds\n", i);
  /* load muffled sounds */
  i = 0;

  do
  {
    sprintf(buff, "./DATA/%s/muff_%s_%d.ogg", path, name, i);
    aud->muffled[i] = al_load_sample(buff);

    if (aud->muffled[i++] == NULL)
      break;
    else
      printf("\tLOADED MUFFLED AUDIO %s AS SAMPLE %d\n", buff, i);
  }
  while (i < 4);

  aud->muffled_count = i - 1;
}