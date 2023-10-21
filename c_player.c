
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "c_player.h"
#include "p_engine.h"
#include "c_audio.h"
#include "i_math.h"
#include "p_game.h"
#include "c_map.h"
#include "main.h"
#include "pm.h"
#include "c_sched.h"

#include "p_render.h"

player_t * player_ctor (engine_t * engine)
{
  player_t * self = (player_t *) malloc(sizeof(struct player_s));
  __GUARD(self, "player_t obj");
  self->x = (511 / 2) * 64 + 32;
  self->y = (511 / 2) * 64 + 32; 
  self->ang = ANG90;
  self->dyoff = BOBBING_STEP;
  self->yoff = 0;
  self->is_alive = true;
  // printf("creating player ipc: %d %d\n", engine->ipc.co)


  return self;
}

void player_dtor (player_t * self)
{
  __UNGUARD(self, "player_t obj");
  free(self);
}

void player_io_handler (game_t * game, engine_t * engine)
{
  // keyboard
  player_t * self = game->player;

  // TODO: Przywrócic jak będzie enemy
  // if (self->is_alive == false)
  // {
  //   game->player->ang = atan2(game->enemy->y - game->player->y, game->enemy->x - game->player->x);
  //   game->player->vx = 0;
  //   game->player->vy = 0;
  //   return;
  // }

  ALLEGRO_KEYBOARD_STATE state;
  al_get_keyboard_state(&state);
  
  self->vx = 0; self->vy = 0;

  if (al_key_down(&state, 'w' - 'a' + 1))
  {
    self->vx += cos(self->ang) / 2;
    self->vy += sin(self->ang) / 2;
  }

  if (al_key_down(&state, 's' - 'a' + 1))
  {
    self->vx += cos(self->ang + ANG180) / 4;
    self->vy += sin(self->ang + ANG180) / 4;
  }
  
  if (al_key_down(&state, 'd' - 'a' + 1))
  {
    self->vx += cos(self->ang + ANG90) / 4;
    self->vy += sin(self->ang + ANG90) / 4;
  }
  
  if (al_key_down(&state, 'a' - 'a' + 1))
  {
    self->vx += cos(self->ang + ANGN90) / 4;
    self->vy += sin(self->ang + ANGN90) / 4;
  }
  
  if (al_key_down(&state, 'q' - 'a' + 1))
  {
    engine->pm->state = PM_STOPPED;
  }

  if (al_key_down(&state, ALLEGRO_KEY_ESCAPE))
  {
    // suspend game process
    // rise menu process

    pm_hold(game->engine->pm, "game");
    pm_run(game->engine->pm, "menu");

  }

  // mouse

  ALLEGRO_MOUSE_STATE mstate;
  al_get_mouse_state(&mstate);
  
  self->ang = ((mstate.x + 100) * (3.1415 * 2) / (engine->width - 200));// + (3.1415 * 2);

  if (mstate.x < 100)
    al_set_mouse_xy(engine->display, engine->width - 100, mstate.y);

  if (mstate.x > engine->width - 100)
    al_set_mouse_xy(engine->display, 100, mstate.y);

  if (mstate.y < 100)
    al_set_mouse_xy(engine->display, mstate.x, engine->height - 100);

  if (mstate.y > engine->height - 100)
    al_set_mouse_xy(engine->display, mstate.x, 100);
}


void player_update (game_t * game)
{
  player_io_handler(game, game->engine);
  player_movement_handler(game->player, game);
}


void player_bobbing (player_t * self, game_t * game)
{
  const float bobbing_limit = (float) game->engine->buffer_height / 25;
  const float bobbing_step = (float) game->engine->buffer_height / 1200;
  self->yoff += self->dyoff;

  if (self->yoff > bobbing_limit)
    self->dyoff = -1 * bobbing_step * game->engine->delta;
  else if (self->yoff < -1 * bobbing_limit)
    self->dyoff = bobbing_step * game->engine->delta;

  printf("bob delt: %f\n", bobbing_step * game->engine->delta );
}


void player_movement_handler (player_t * self, game_t * game)
{
  if (time(NULL) < game->sched->syncro)
    return;

  // apply velocity vectors (vx, vy)
  const float speed = 0.25;
  self->x += self->vx * (speed / game->engine->delta);
  self->y += self->vy * (speed / game->engine->delta);

  // check if player is inside the wall
  bool is_inside = false;
  bool is_door = false;

  uint8_t tile;

  long mx;
  long my;

  int x, y;
  
  for (y = -1; y <= 1; y += 2)
    for (x = -1; x <= 1; x += 2)
    {
      mx = (long) (((int) self->x) + x * 9) >> BMWS;
      my = (long) (((int) self->y) + y * 9) >> BMWS;

      tile = game->map->tiles[mx][my];

      if (tile >= MAP_BEG_DOOR && tile < MAP_END_DOOR)
      {
        is_door = true;
        is_inside = true;
        goto _skip_check_fast;
      }
      else if (tile >= MAP_BEG_COLLIDABLE && tile < MAP_END_COLLIDABLE)
      {
        // if prop -> recheck()
        if (tile >= MAP_BEG_PROPS && tile < MAP_END_PROPS)
        {
          // how to recheck
          if (q_dist((float) mx * BMW + (BMW / 2), (float) my * BMW + (BMW / 2), game->player->x, game->player->y) > (BMW / 2))
            return;
        }
        is_inside = true;
        goto _skip_check_fast;
      }

    }

  // revert applied vectors if so
  _skip_check_fast:

  if (is_inside)
  {
    self->x -= self->vx * (speed / game->engine->delta);
    self->y -= self->vy * (speed / game->engine->delta);
  }

  // if door, tp player
  if (is_door)
  {
    long ox = mx - ((long) self->x >> BMWS);
    long oy = my - ((long) self->y >> BMWS);
    self->x += ox * WALL_JUMP;
    self->y += oy * WALL_JUMP;
  }


  // handle player bobbing effect
  // if (self->vx != 0 || self->vy != 0)
  // {
  //   player_bobbing(self, game);
  // }
}

void player_kill (sched_t * sched, void * ptr, int val)
{
  game_t * game = (game_t *) ptr;
  // printf("Player has died()\n");
  // kill game and renderer
  // and run cutscene
  pm_run(game->pm, "cutscene");
  pm_stop(game->pm, "render");
  pm_stop(game->pm, "game");

  game->engine->ipc.target = 1; // msg to p_cutscene
  game->engine->ipc.code = 1; // play death cutscene
  
  // WARN: some pointers, are dangling from now on
  // valgrind will complain
  // but its not an acutuall issue
}