
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "c_enemy.h"
#include "c_player.h"
#include "p_game.h"
#include "main.h"
#include "c_audio.h"
#include "p_engine.h"
#include "c_point.h"
#include "c_map.h"
#include "c_sched.h"
#include "p_render.h"
#include "c_item.h"
#include "i_math.h"


enemy_t * enemy_ctor (game_t * game)
{
  enemy_t * self = (enemy_t *) malloc(sizeof(struct enemy_s));
  __GUARD(self, "enemy_t obj");

  self->viz = viz_ctor(game);
  self->viz->x = &self->x;
  self->viz->y = &self->y;
  self->viz->txt = 0; 

  self->x = 0;
  self->y = 0;
  self->ang = 0;
  self->facing = 1;
  self->is_spawned = false;
  self->chase = 6;
  self->agroed = true;
  self->speed = 10;
  self->atacking = 0;
  self->door_check = true;
  self->audio = 0;

  if (game->engine->ipc.target == 666 && game->engine->ipc.code > 50 && game->engine->ipc.code < 60)
  {
    enemy_load(self, game->engine->ipc.code % 10);
    spawn_enemy_rutines(game);
  }

  return self;
}

/*
  Update enemy sprite animation
*/
void enemy_sprite_update (sched_t * sched, void * ptr, int val)
{
  game_t * game = (game_t *) ptr;

  int ang = (int) (fmod((8 * 3.1415 + game->player->ang) - game->enemy->ang - 3.1415, 2 * 3.1415) / (3.1415 / 180));
  int frame = val & 3;
  int sprite = MAP_CORK_BEG + 4 * (ang / 45) + frame;

  if (game->enemy->atacking > 0)
    sprite = MAP_CORK_ATTACK + frame;

  game->map->texture_buffer[0] = game->map->texture_buffer[sprite];
  yield(sched, enemy_sprite_update, ptr, val + 1, 200);
}

/*
  Try Attacking Player
*/
void try_attacking (sched_t * sched, void * ptr, int val)
{
  game_t * game = (game_t *) ptr;
  float dist = q_dist(game->player->x, game->player->y, game->enemy->x, game->enemy->y);

  // printf("attacing in: %f\n", dist);
  if (dist < CORK_ATTACK_DIST)
  {
    game->enemy->atacking = 30;
    game->enemy->facing = -1;
    game->enemy->chase = get_random_int(6, 14, 1);
    game->player->health -= (float) get_random_int(30, 90, 1) / 5;
    audio_play_master(game->audio, "hit");

    if (game->player->health <= 0)
    {
      // kill 
      game->player->is_alive = false;
      yield(game->sched, player_kill, ptr, 0, 8000);
    }
    else
    {
      audio_play_master(game->audio, "pain");
    }

    printf("Player health %f\n", game->player->health);
    reset_points(game);
  }
  else if (game->enemy->atacking > 0)
  {
    if (game->enemy->door_check == false)
    {
      long mx = ((long) game->enemy->x) >> 8;
      long my = ((long) game->enemy->y) >> 8;
      uint8_t tile = game->map->tiles[mx][my];
      // printf("tile is %u\n", (unsigned int) tile);

      // printf("val mod 2 == %d\n", val % 2);
      if (val % 2 == 0)
      {
        audio_play_master(game->audio, (tile < MAP_DOOR_END) 
          ? "cork_door" 
          : "cork_expl");
      }
    }
    else
    {
      audio_play_master(game->audio, "miss");
    }
  }

  if (game->player->is_alive)
    yield(sched, try_attacking, ptr, val + 1, 250);
}

void spawn_enemy_rutines (game_t * game)
{
  yield(game->sched, enemy_sprite_update, game, 0, 10);
  yield(game->sched, try_attacking, game, 0, 10);
  // printf("Spawned!\n");
}

/*
  Cork hasn't been summoned yet
*/
void try_spawning (game_t * game)
{
  if (game->map->points_count < 16)
    return;

  // spawn!
  game->enemy->x = game->map->spawn->x;
  game->enemy->y = game->map->spawn->y;
  game->enemy->is_spawned = true;
  game->enemy->facing = 1;
  spawn_enemy_rutines(game);
}

/*
  Cork Walking AI / Mechanism
*/
void try_moving (game_t * game)
{
  // Pivot
  float ang = atan2(game->player->y - game->enemy->y, game->player->x - game->enemy->x);
  float px = game->enemy->facing * cos(ang) * CORK_PIVOT_DIST + game->enemy->x;
  float py = game->enemy->facing * sin(ang) * CORK_PIVOT_DIST + game->enemy->y;

  // printf("Attempting to move()\n");
  point_t * near = find_nearest_point(game, px, py);
  // printf("Attemptred resulted in %p\n", near);

  if (near != NULL)
  {
    if (get_dist(near->x, near->y, game->enemy->x, game->enemy->y) < 32) 
      near->enabled = false;
    ang = atan2(near->y - game->enemy->y, near->x - game->enemy->x);
  }
  else
  {
    // printf("No more points to trail\n");
    return;
  }

  // face mappoint
  game->enemy->ang = atan2(near->y - game->enemy->y, near->x - game->enemy->x);

  float dist = q_dist(game->player->x,game->player->y, game->enemy->x, game->enemy->y);
  // printf("enemy dist %f\n", dist);

  // always move foward no matter what? or stall until has any points???

  float speed = game->enemy->speed;

  if (game->enemy->facing == 1 && dist > CROK_SAFE_DIST) // change this into teleporintg
  {
    /*
      If far away it teleports back and forth between two points
      which is stupid
    */
    // printf("Teleporting\n");
    game->enemy->x = near->x;
    game->enemy->y = near->y;
    near->enabled = false; // this should fix the problem!
    // TODO: Check if solution works!!
    return;
  }

  // printf("facing: %f\n", game->enemy->facing);

  // slow down when really near player
  if (dist < 256 * 2.5) 
    speed = speed * 0.75 + (float) get_random_int(1, 10, 1);

  if (dist < CORK_ATTACK_DIST * 0.75 && game->enemy->facing == 1) /* todo: find better way */
  {
    // printf("Break: %f\n", dist);
    return;
  }

  if (dist < CORK_AGRO_DIST && game->enemy->facing == 1)
    game->enemy->ang = atan2(game->player->y - game->enemy->y, game->player->x - game->enemy->x);

  game->enemy->x += cos(game->enemy->ang) * speed;
  game->enemy->y += sin(game->enemy->ang) * speed;

  const long mx = ((long) game->enemy->x) >> 8;
  const long my = ((long) game->enemy->y) >> 8;
  const uint8_t tile = game->map->tiles[mx][my];

  if (game->enemy->door_check)
  {
    // check for doors in current BM
    if (tile > MAP_COLLIDABLE_START && tile < MAP_COLLIDABLE_END) // TODO: Check if narrow
    {
      // slow donw enemy!
      if (tile < MAP_SPRITES_START)
      {
        game->enemy->atacking = 20 * 2;
        game->enemy->viz->txt = MAP_EXPL;
      }
      else
      {
        game->enemy->atacking = 18;
      }

      game->enemy->door_check = false;
    }
  }
  else
  {
    if (tile < MAP_COLLIDABLE_START || tile > MAP_COLLIDABLE_END) // play qued sound
    {
      game->enemy->door_check = true;
      game->enemy->viz->txt = 0;
    }
  }

  // ----------------------------------------
  // -- audio timer

}

void enemy_chase_update (game_t * game)
{
  float dist = q_dist(game->player->x, game->player->y, game->enemy->x, game->enemy->y);
  // printf("Chase timer: %d, facing: %f, health %f\n", game->enemy->chase, game->enemy->facing, game->player->health);

  if ((game->enemy->facing == 1 && dist < 10 * 256) || game->enemy->facing == -1)
    game->enemy->agroed = true;

  if (game->enemy->agroed && game->enemy->atacking <= 0)
    game->enemy->chase -= 1;

  if (game->enemy->chase <= 0)
  {
    // flip flag, and get new chase timer
    game->enemy->facing *= -1;

    game->enemy->chase = get_random_int(8, 16, 1);
    game->enemy->agroed = false;
    reset_points(game);
  }

  if (game->player->is_alive == false)
    game->enemy->facing = -1;
  

  // that's a fast cork
  int speed_chance = get_random_int(0, 100, 1); // vfast
  float vmin = 35;
  float vmax = 40;

  if (speed_chance < 70) // average
  {
    vmin = 25;
    vmax = 35;
  }

  game->enemy->speed = (float) get_random_int(vmin, vmax, 1);
}

void enemy_reset_audio (sched_t * sched, void * ptr, int val)
{
  game_t * game = (game_t *) ptr;
  game->enemy->audio = false;
}

void enemy_update (sched_t * sched, void * ptr, int val)
{
  game_t * game = (game_t *) ptr;
  enemy_t * self = game->enemy;

  // handle AI and other stuff
  if (self->is_spawned == false)
  {
    try_spawning(game);
    yield(sched, enemy_update, ptr, val, 500);
    return;
  }
  
  if (self->atacking <= 0)
    try_moving(game);
  else
    self->atacking -= 1;

  if ((val & 15) == 0)
    audio_play(game->audio, game, "step", self->x, self->y);

  if ((val % 40) == 0)
    enemy_chase_update(game);

  if ((val & 15) == 0 && self->audio == false)
  {
    // perform line of sight check
    if (map_line_of_sight(game->map, self->x, self->y, game->player->x, game->player->y))
    {
      // based on different dist
      const float dist = q_dist(self->x, self->y, game->player->x, game->player->y);
      self->audio = true;
      audio_play_master(game->audio, "chase");
      yield(game->sched, enemy_reset_audio, game, 0, 23900);
    }
  }

  yield(sched, enemy_update, ptr, val + 1, 40);
}

void enemy_dtor (enemy_t * self)
{
  __UNGUARD(self, "enemy_t obj");
  free(self);
}


void enemy_save (game_t * game, int save_no)
{
  char save_path [0xFF];
  sprintf(save_path, "./DATA/sav/enemy%d.sav", save_no);

  #ifdef _WIN32
    int fd = open(save_path, O_CREAT | O_WRONLY | O_BINARY, 0600);
  #else
    int fd = open(save_path, O_CREAT | O_WRONLY, 0600);
  #endif

  // floats
  assert(write(fd, &game->enemy->x, sizeof(float)) == sizeof(float));
  assert(write(fd, &game->enemy->y, sizeof(float)) == sizeof(float));
  assert(write(fd, &game->enemy->ang, sizeof(float)) == sizeof(float));
  assert(write(fd, &game->enemy->facing, sizeof(float)) == sizeof(float));
  assert(write(fd, &game->enemy->speed, sizeof(float)) == sizeof(float));

  // ints
  assert(write(fd, &game->enemy->chase, sizeof(int)) == sizeof(int));
  assert(write(fd, &game->enemy->atacking, sizeof(int)) == sizeof(int));

  // bools
  assert(write(fd, &game->enemy->is_spawned, sizeof(bool)) == sizeof(bool));
  assert(write(fd, &game->enemy->agroed, sizeof(bool)) == sizeof(bool));
  assert(write(fd, &game->enemy->door_check, sizeof(bool)) == sizeof(bool));

  close(fd);
}

void enemy_load (enemy_t * self, int save_no)
{
  char save_path [0xFF];
  sprintf(save_path, "./DATA/sav/enemy%d.sav", save_no);

  #ifdef _WIN32
    int fd = open(save_path, O_CREAT | O_RDONLY | O_BINARY, 0600);
  #else
    int fd = open(save_path, O_CREAT | O_RDONLY, 0600);
  #endif

  // floats
  assert(read(fd, &self->x, sizeof(float)) == sizeof(float));
  assert(read(fd, &self->y, sizeof(float)) == sizeof(float));
  assert(read(fd, &self->ang, sizeof(float)) == sizeof(float));
  assert(read(fd, &self->facing, sizeof(float)) == sizeof(float));
  assert(read(fd, &self->speed, sizeof(float)) == sizeof(float));

  // ints
  assert(read(fd, &self->chase, sizeof(int)) == sizeof(int));
  assert(read(fd, &self->atacking, sizeof(int)) == sizeof(int));

  // bools
  assert(read(fd, &self->is_spawned, sizeof(bool)) == sizeof(bool));
  assert(read(fd, &self->agroed, sizeof(bool)) == sizeof(bool));
  assert(read(fd, &self->door_check, sizeof(bool)) == sizeof(bool));

  close (fd);
}