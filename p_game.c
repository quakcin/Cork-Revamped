
#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "pm.h"

#include "p_engine.h"
#include "p_render.h"
#include "p_game.h"

#include "c_player.h"
#include "c_map.h"
#include "c_audio.h"
#include "c_sched.h"


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

  // SPLASH
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
  // game->enemy = enemy_ctor(game);
  // game->item = item_ctor(game);
  // game_init_audio(game);

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

  // hide mouse cursor
  al_hide_mouse_cursor(game->engine->display);
}

void p_game_update (proc_t * self)
{
  game_t * game = (game_t *) self->mem;
  engine_t * engine = (engine_t *) self->pm->proc_list->mem;
  
  player_update(game);
  sched_update(game->sched, engine);

  al_hide_mouse_cursor(game->engine->display);
}

void p_game_dtor (proc_t * self)
{
  game_t * game = (game_t *) self->mem;
  // viz_dtor(game);
  // player_dtor(game->player);
  // audio_dtor(game->audio);
  // enemy_dtor(game->enemy);
  // sched_dtor(game->sched);
  // item_dtor(game->item);
  // map_dtor(game->map);
  // 4. Destroy Cork
  // 5. Destroy Game Object
  __UNGUARD(game, "game_t obj");
  free(game);
}
