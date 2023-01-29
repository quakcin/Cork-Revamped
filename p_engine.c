/*
 *  Engine Process
 *  Allegro5 and other usefull stuff!
 */

#include <stdio.h>

#include "main.h"
#include "p_engine.h"
#include "pm.h"

#include "p_menu.h"

void p_engine_resize_event (engine_t * self)
{
  self->width = al_get_display_width(self->display);
  self->height = al_get_display_height(self->display);
}

void p_engine_init (proc_t * self)
{
  engine_t * eng = (engine_t *) malloc(sizeof(struct engine_s));
  __GUARD(eng, "engine_t obj");
  
  if (eng == NULL)
  {
    IERROR("Failed to create engine memory heap");
    return;
  }

  eng->width = 1280;
  eng->height = 720;
  eng->buffer_width = eng->width;
  eng->buffer_height = eng->height;
  eng->pm = self->pm;
  eng->is_fullscreen = false;
  
  // HRTimer
  eng->ticks = 0;
  eng->last_tick = 0;
  eng->last_clock = time(NULL);
  eng->delta = 0;

  // IPC
  eng->ipc.target = 1; // cutscene
  eng->ipc.code = 0; // intro

  // CONFIG
  eng->use_captions = true;
  eng->gamma = 90;

  al_init();
  al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_WINDOWED);

	eng->display = al_create_display(eng->width, eng->height);
  eng->save_no = 0;
  self->mem = (void *) eng;

	al_install_mouse();
	al_init_image_addon();
	al_install_keyboard();

	al_install_audio();
	al_init_acodec_addon();
  // al_set_default_mixer(al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_UINT16, ALLEGRO_CHANNEL_CONF_5_1));
	al_reserve_samples(8);
	al_init_primitives_addon();
  al_init_video_addon();

	al_init_font_addon();
	al_init_ttf_addon();
  
  al_set_window_title(eng->display, "CRK Engine");
  // al_start_timer(eng->timer);
  // al_hide_mouse_cursor(eng->display);

  eng->buffer = al_create_bitmap(eng->buffer_width, eng->buffer_height);

  IPRINT("Engine is running");
  pm_run(self->pm, "cutscene");

  // create new game and rise p_game
  // sv_create_game();

}

void p_engine_update (proc_t * self)
{
  engine_t * eng = (engine_t *) self->mem;

  if (eng->refresh-- <= 0)
  {
    al_flip_display();
    eng->refresh = (1000 / 120) * eng->delta;
  }

  eng->ticks++;
  if (time(NULL) != eng->last_clock)
  {
    eng->delta = (float) (eng->ticks - eng->last_tick) / 1000; 
    eng->last_clock = time(NULL);
    eng->last_tick = eng->ticks;
    // printf("Delta %f\n", eng->delta);
  }

  // check if window has been resized
  if (al_acknowledge_resize(eng->display))
    p_engine_resize_event(eng);
}

void p_engine_dtor (proc_t * self)
{
  engine_t * eng = (engine_t *) self->mem;

  al_set_target_bitmap(NULL);
	al_destroy_display(eng->display);
  eng = NULL; // prevent dangling

  __UNGUARD(self->mem, "self->mem in engine obj?");
	free(self->mem);
}


