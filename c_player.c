
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "c_player.h"
#include "p_engine.h"
#include "c_enemy.h"
#include "c_audio.h"
#include "c_item.h"
#include "i_math.h"
#include "p_game.h"
#include "c_map.h"
#include "main.h"
#include "pm.h"
#include "c_sched.h"

player_t * player_ctor (engine_t * engine)
{
  player_t * self = (player_t *) malloc(sizeof(struct player_s));
  __GUARD(self, "player_t obj");
  self->x = (511 / 2) * 256 + 128;
  self->y = (511 / 2) * 256 + 128; 
  self->ang = ANG90;
  self->dyoff = BOBBING_STEP;
  self->yoff = 0;
  self->walking_audio_timer = true;
  self->health = 100;
  self->is_alive = true;
  // printf("creating player ipc: %d %d\n", engine->ipc.co)

  if (engine->ipc.target == 666 && engine->ipc.code > 50  && engine->ipc.code < 60)
    player_load (self, engine->ipc.code % 10);

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

  if (self->is_alive == false)
  {
    game->player->ang = atan2(game->enemy->y - game->player->y, game->enemy->x - game->player->x);
    game->player->vx = 0;
    game->player->vy = 0;
    return;
  }

  ALLEGRO_KEYBOARD_STATE state;
  al_get_keyboard_state(&state);
  
  self->vx = 0; self->vy = 0;

  if (al_key_down(&state, 'w' - 'a' + 1))
  {
    self->vx += cos(self->ang) * 2;
    self->vy += sin(self->ang) * 2;
  }

  if (al_key_down(&state, 's' - 'a' + 1))
  {
    self->vx += cos(self->ang + ANG180);
    self->vy += sin(self->ang + ANG180);
  }
  
  if (al_key_down(&state, 'd' - 'a' + 1))
  {
    self->vx += cos(self->ang + ANG90);
    self->vy += sin(self->ang + ANG90);
  }
  
  if (al_key_down(&state, 'a' - 'a' + 1))
  {
    self->vx += cos(self->ang + ANGN90);
    self->vy += sin(self->ang + ANGN90);
  }
  
  if (al_key_down(&state, 'q' - 'a' + 1))
  {
    engine->pm->state = PM_STOPPED;
  }

  if (al_key_down(&state, 'e' - 'a' + 1))
  {
    const float dist = q_dist(game->player->x, game->player->y, game->item->x, game->item->y);

    if (dist < ITEM_PICKUP_DIST)
    {
      game->engine->ipc.target = 1;
      game->engine->ipc.code = 2;
      pm_stop(game->pm, "game");
      pm_stop(game->pm, "render");
      pm_run(game->pm, "cutscene");
    }
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
    self->dyoff = -1 * bobbing_step;
  else if (self->yoff < -1 * bobbing_limit)
    self->dyoff = bobbing_step;
}


void player_reset_walking_aut (sched_t * sched, void * ptr, int val)
{
  player_t * player = (player_t *) ptr;
  player->walking_audio_timer = true;
}

void player_stuck_preventer (game_t * game)
{
  // check if player is somehow stuck
  // inside a wall

  const long mx = (long) game->player->x >> 8;
  const long my = (long) game->player->y >> 8;

  const long tile = game->map->tiles[mx][my];

  if (tile > MAP_WALL_START && tile < MAP_WALL_END)
  {
    // printf("Stuck!!\n");
    // tp to start position, got nothing better to do
    game->player->x = 256 * 255 + 128;
    game->player->y = 256 * 255 + 128;
  }
}

void player_movement_handler (player_t * self, game_t * game)
{
  if (time(NULL) < game->sched->syncro)
    return;

  player_stuck_preventer(game);
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
      mx = (long) (((int) self->x) + x * 9) >> 8;
      my = (long) (((int) self->y) + y * 9) >> 8;

      tile = game->map->tiles[mx][my];

      if (tile > MAP_DOOR_START && tile < MAP_DOOR_END)
      {
        is_door = true;
        is_inside = true;
        goto _skip_check_fast;
      }
      else if (tile > MAP_COLLIDABLE_START && tile < MAP_COLLIDABLE_END)
      {
        // if prop -> recheck()
        if (tile > MAP_COLLIDABLE_PROP_START)
        {
          // how to recheck
          if (q_dist((float) mx * 256 + 128, (float) my * 256 + 128, game->player->x, game->player->y) > 128)
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
    long ox = mx - ((long) self->x >> 8);
    long oy = my - ((long) self->y >> 8);
    self->x += ox * WALL_JUMP;
    self->y += oy * WALL_JUMP;
    game->map->darkness = 255;
    // char door_sample_name[0xF];
    // sprintf(door_sample_name, "door%d", tile - MAP_DOOR_START - 1);
    audio_play_master(game->audio, "door"); // one sound fits all
    // cahnge floor color
    
    const uint8_t new_tile = game->map->tiles[((long) self->x >> 8)][((long) self->y >> 8)];
    // printf("New tile found %u\n", new_tile);
    if (new_tile > MAP_SKY_START && new_tile < MAP_SKY_END)
    {
      // printf("Color changed\n");
      game->map->color = new_tile - MAP_SKY_START - 1;
    }
  }

  // handle player bobbing effect
  if (self->vx != 0 || self->vy != 0)
  {
    player_bobbing(self, game);
    if (self->walking_audio_timer)
    {
      self->walking_audio_timer = false;
      audio_play_master(game->audio, "step");
      yield(game->sched, player_reset_walking_aut, self, 0, PLAYER_WALKING_AUDIO_DELAY);
    }
  }
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

void player_save (game_t * game, int save_no)
{
  /*
    FILE FORMAT:
    0 float   player->x
    1 float   player->y
    2 float   player->health
  */
  char save_path [0xFF];
  sprintf(save_path, "./DATA/sav/player%d.sav", save_no);

  #ifdef _WIN32
    int fd = open(save_path, O_CREAT | O_WRONLY | O_BINARY, 0600);
  #else
    int fd = open(save_path, O_CREAT | O_WRONLY, 0600);
  #endif

  assert(write(fd, &game->player->x, sizeof(float)) == sizeof(float));
  assert(write(fd, &game->player->y, sizeof(float)) == sizeof(float));
  assert(write(fd, &game->player->health, sizeof(float)) == sizeof(float));

  close(fd);
}

void player_load (player_t * self, int save_no)
{
  char save_path [0xFF];
  sprintf(save_path, "./DATA/sav/player%d.sav", save_no);

  #ifdef _WIN32
    int fd = open(save_path, O_CREAT | O_RDONLY | O_BINARY, 0600);
  #else
    int fd = open(save_path, O_CREAT | O_RDONLY, 0600);
  #endif

  assert(read(fd, &self->x, sizeof(float)) == sizeof(float));
  assert(read(fd, &self->y, sizeof(float)) == sizeof(float));
  assert(read(fd, &self->health, sizeof(float)) == sizeof(float));

  close (fd);
}