
#include "p_game.h"
#include "c_item.h"
#include "p_render.h"
#include "c_sched.h"
#include "i_math.h"
#include "c_player.h"
#include "c_audio.h"
#include "p_engine.h"
#include "main.h"

#include <stdio.h>
#include <math.h>

/*
  Both x and y has to be set by map_t 
  during new game genertion process
*/

item_t * item_ctor (game_t * game)
{
  item_t * self = (item_t *) malloc(sizeof(struct item_s));
  self->x = 0;
  self->y = 0;
  self->in_inventory = false;
  self->lpdist = 0;
  self->lpx = 0;
  self->lpy = 0;

  viz_t * vizp = viz_ctor(game);
  vizp->x = &self->x;
  vizp->y = &self->y;
  vizp->txt = 145;

  if (game->engine->ipc.target == 666 && game->engine->ipc.code > 50 && game->engine->ipc.code < 60)
    item_load (self, game->engine->ipc.code % 10);

  return self;
}

void item_dtor (item_t * self)
{
  free(self);
}

void item_pickup_tip (sched_t * sched, void * ptr, int val)
{
  game_t * game = (game_t *) ptr;

  const float dist = q_dist(game->player->x, game->player->y, game->item->x, game->item->y);

  if (dist < ITEM_PICKUP_DIST)
    render_use_captions(game->pm, "Wciśnij 'e' aby podnieść szpunt", 1);

  yield(sched, item_pickup_tip, ptr, val, 500);
}

/*
  Tips / Captions for player
*/
void item_dir_suggest (sched_t * sched, void * ptr, int val)
{
  game_t * game = (game_t *) ptr;

  if (game->player->is_alive == false)
    return; // kill coroutine

  // if very near, give very near clue
  const float p_dist = q_dist(game->player->x, game->player->y, game->item->x, game->item->y);
  const float delt_ang = fmod(3 * PI2 + atan2(game->item->y - game->player->y, game->item->x - game->player->x), PI2);
  // printf("Ang to %f %f \n", delt_ang / (3.1415/180), game->player->ang / (3.1415/180));
  // printf("Dist to item %fm\n", p_dist / 256);

  if (game->item->lpdist == 0 || p_dist < 256)
  {
    ; // do nothing
  }
  else if (p_dist < ITEM_VERY_NEAR)
  {
    audio_play_master(game->audio, "quote_pos_near");
    render_use_captions(game->pm, "To musi być gdzieś tutaj", 2);
  }
  else
  {
    if (p_dist < game->item->lpdist + 512)
    {
      if (within_angle(delt_ang, game->player->ang, ANG15))
      {
        audio_play_master(game->audio, "quote_dir_30");
        render_use_captions(game->pm, "To napewno w tę stronę", 2);
      }
      else if (within_angle(delt_ang, game->player->ang, ANG30))
      {
        audio_play_master(game->audio, "quote_dir_60");
        render_use_captions(game->pm, "Idę w dobrym kierunku", 2);
      }
      else if (within_angle(delt_ang, game->player->ang, ANG60))
      {
        audio_play_master(game->audio, "quote_dir_120");
        render_use_captions(game->pm, "Chyba w tą stronę", 2);
      }
      else
      {
        audio_play_master(game->audio, "quote_pos_closer");
        render_use_captions(game->pm, "Jestem coraz bliżej", 2);
      }
    }
    else
    {
      if (delt_ang < ANG120)
      {
        audio_play_master(game->audio, "quote_dir_wrong");
        render_use_captions(game->pm, "Chyba muszę zawrócić", 2);
      }
      else
      {
        audio_play_master(game->audio, "quote_pos_further");
        render_use_captions(game->pm, "Chyba się oddalam", 2);
      }
    }
  }

  game->item->lpx = game->player->x;
  game->item->lpy = game->player->y;
  game->item->lpdist = p_dist;

  // yield(sched, item_dir_suggest, ptr, val, (float) get_random_int(3, 3, 1000));
  yield(sched, item_dir_suggest, ptr, val, (float) get_random_int(8, 16, 1000));
}


void item_save (game_t * game, int save_no)
{
  char save_path [0xFF];
  sprintf(save_path, "./DATA/sav/item%d.sav", save_no);

  #ifdef _WIN32
    int fd = open(save_path, O_CREAT | O_WRONLY | O_BINARY, 0600);
  #else
    int fd = open(save_path, O_CREAT | O_WRONLY, 0600);
  #endif

  assert(write(fd, &game->item->x, sizeof(float)) == sizeof(float));
  assert(write(fd, &game->item->y, sizeof(float)) == sizeof(float));

  // printf("Item Location %f %f\n", game->item->x, game->item->y);

  close(fd);
}

void item_load (item_t * self, int save_no)
{
  char save_path [0xFF];
  sprintf(save_path, "./DATA/sav/item%d.sav", save_no);

  #ifdef _WIN32
    int fd = open(save_path, O_CREAT | O_RDONLY | O_BINARY, 0600);
  #else
    int fd = open(save_path, O_CREAT | O_RDONLY, 0600);
  #endif

  assert(read(fd, &self->x, sizeof(float)) == sizeof(float));
  assert(read(fd, &self->y, sizeof(float)) == sizeof(float));

  close (fd);
}