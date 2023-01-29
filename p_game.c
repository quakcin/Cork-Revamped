
#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "pm.h"

#include "p_engine.h"
#include "p_render.h"
#include "p_game.h"

#include "c_player.h"
#include "c_enemy.h"
#include "c_map.h"
#include "c_audio.h"
#include "c_point.h"
#include "c_sched.h"
#include "c_item.h"


void p_game_init (proc_t * self)
{
  game_t * game = (game_t *) malloc(sizeof(struct game_s));
  __GUARD(game, "game_t obj");
  engine_t * engine = (engine_t *) pm_get(self->pm, "engine")->mem;

  if (game == NULL)
  {
    IERROR("Failed to create game object");
    return;
  }

  // ----------------------------------------
  // -- Render Splash before loading stuff

  ALLEGRO_BITMAP * splash = al_load_bitmap("./DATA/res0/splash.png");
  al_draw_scaled_bitmap(splash, 0, 0, 1280, 720, 0, 0, engine->buffer_width, engine->buffer_height, 0);
  al_flip_display();
  
  // INVOKE: create random map UNLESS loading saved game
  // ALSO: Get random door spawn and set it below along with the first preloaded map obj
  game->engine = engine; // expose engine

  // Create and load vital stuff
  game->sched = sched_ctor();
  game->viz = NULL;
  game->player = player_ctor(engine);
  game->enemy = enemy_ctor(game);
  game->item = item_ctor(game);
  game_init_audio(game);

  game->map = map_ctor(game);
  game->pm = self->pm;

  al_destroy_bitmap(splash);

  // wait for correct time frame
  game->sched->syncro = time(NULL) + 2;

  // Actaull load game
  // sv_load_game(game, engine->save_no); 
  // map_load(game);

  // enable renderer process
	pm_run(self->pm, "render");

  // remember game obj in heap
  self->mem = (void *) game;

  // run shceduler list
  p_game_sched_list (game);

  // hide mouse cursor
  al_hide_mouse_cursor(game->engine->display);
}

/* good horror has to have good musci eh? */
void p_game_ambience_update (sched_t * sched, void * ptr, int val)
{
  audio_play_master((audio_t *) ptr, "ambience");
  yield(sched, p_game_ambience_update, ptr, val, (60 + 56) * 1000);
}

void p_game_beg_cap_end (sched_t * sched, void * ptr, int val)
{
  game_t * game = (game_t *) ptr;
  audio_play_master(game->audio, "begcap1");
  render_use_captions(game->pm, "Zanim to korkowate bydle mnie dopadnie", 2);
}

void p_game_beg_cap_beg (sched_t * sched, void * ptr, int val)
{
  game_t * game = (game_t *) ptr;
  audio_play_master(game->audio, "begcap0");
  render_use_captions(game->pm, "Muszę odnaleźć starożytny artefakt", 2);
  yield(sched, p_game_beg_cap_end, ptr, val, 3500);
}

void p_game_sched_list (game_t * self)
{
  yield(self->sched, enemy_update, self, 0, 1);
  yield(self->sched, task_create_point, self, 0, 1);
  yield(self->sched, item_dir_suggest, self, 0, 3000);
  yield(self->sched, item_pickup_tip, self, 0, 250);
  yield(self->sched, p_game_ambience_update, self->audio, 0, 1500);
  yield(self->sched, p_game_beg_cap_beg, self, 0, 3 * 1000);
  map_update_animated_textures(self->sched, self->map, 0);
  // map_update_animated_textures (game->map, (long) engine->ticks);
}

void p_game_update (proc_t * self)
{
  game_t * game = (game_t *) self->mem;
  engine_t * engine = (engine_t *) self->pm->proc_list->mem;
  // player_update(game->player, game);
  // enemy_update(game->enemy, game);
  // player_update(game);
  
  player_update(game);
  sched_update(game->sched, engine);

  // ipc update
  if (game->engine->ipc.target == 666)
  {
    if (game->engine->ipc.code == 21) // quit now
    {
      game->player->is_alive = false;
      pm_run(game->pm, "menu");
      pm_stop(game->pm, "render");
      pm_stop(game->pm, "game");
    }
    else if (game->engine->ipc.code > 50 && game->engine->ipc.code < 60)
    {
      int save_id = game->engine->ipc.code % 10;
      printf("\tLoad save game state %d\n", save_id);
    }
    else if (game->engine->ipc.code > 60 && game->engine->ipc.code < 70)
    {
      int no = game->engine->ipc.code % 10;
      // call all save()'s
      printf("\tsaving game as %d\n", no);
      map_save(game, no);
      player_save(game, no);
      item_save(game, no);
      enemy_save(game, no);
      points_save(game, no);
    }
    /* 9x is init() ACK */
    game->engine->ipc.code = 0;
  }
  al_hide_mouse_cursor(game->engine->display);

  // printf("Player pos %f %f\n", game->player->x, game->player->y);
}

void p_game_dtor (proc_t * self)
{
  game_t * game = (game_t *) self->mem;
  // 0. Kill Renderer
  // 1. Destroy Map Graph
  // 2. Destroy Trailing Nodes
  // 3. Destroy Player
  viz_dtor(game);
  player_dtor(game->player);
  audio_dtor(game->audio);
  enemy_dtor(game->enemy);
  sched_dtor(game->sched);
  item_dtor(game->item);
  map_dtor(game->map);
  // 4. Destroy Cork
  // 5. Destroy Game Object
  __UNGUARD(game, "game_t obj");
  free(game);
}

/*
  Global sounds, used on every single map
*/
void game_init_audio (game_t * self)
{
  // load all of the samples
  self->audio = audio_ctor();
  // audio_load(self->audio, "aud", "step");
  audio_load(self->audio, "aud", "miss");
  audio_load(self->audio, "aud", "hit");

  audio_load(self->audio, "aud", "quote_dir_30");
  audio_load(self->audio, "aud", "quote_dir_60");
  audio_load(self->audio, "aud", "quote_dir_120");
  audio_load(self->audio, "aud", "quote_dir_wrong");

  audio_load(self->audio, "aud", "quote_pos_further");
  audio_load(self->audio, "aud", "quote_pos_closer");
  audio_load(self->audio, "aud", "quote_pos_near");

  audio_load(self->audio, "aud", "cork_door");
  audio_load(self->audio, "aud", "cork_expl");

  audio_load(self->audio, "aud", "chase");
  audio_load(self->audio, "aud", "pain");
  audio_load(self->audio, "aud", "ambience");

  audio_load(self->audio, "aud", "begcap0");
  audio_load(self->audio, "aud", "begcap1");
}