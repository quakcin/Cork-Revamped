
#include "p_cutscene.h"
#include "p_engine.h"
#include "pm.h"

#include <stdio.h>

void p_cutscene_init (proc_t * self)
{
  printf("p_cuts_init() risen\n");
  p_cutscene_t * cuts = malloc(sizeof(struct p_cutscene_s));
  self->mem = (p_cutscene_t *) cuts;
  cuts->engine = (engine_t *) self->pm->proc_list->mem;

  char clips[][128] = 
  {
    "./DATA/res0/intro.ogv",
    "./DATA/res0/death.ogv",
    "./DATA/res0/win.ogv",
    "./DATA/res0/credits.ogv"
  };

  // why doesn't it work?
  if (cuts->engine->ipc.target == 1)
  {
    cuts->video = al_open_video(clips[cuts->engine->ipc.code]);
  }
  else
  {
    printf("[cutscene] critical error, risen p_cutscene without ipc.msg to play\n");
  }

  if (cuts->video != NULL)
    al_start_video(cuts->video, al_get_default_mixer());
  else
    printf("Failed to load cutscene\n");
}

void p_cutscene_update (proc_t * self)
{
  p_cutscene_t * cuts = (p_cutscene_t *) self->mem;
  al_set_target_bitmap(cuts->engine->buffer);

  if (cuts->video != NULL)
  {
    ALLEGRO_BITMAP * frame = al_get_video_frame(cuts->video);
    if (frame != NULL)
    {
      al_draw_scaled_bitmap(frame, 0, 0, 640, 360, 0, 0, cuts->engine->buffer_width, cuts->engine->buffer_height, 0);
      if (al_is_video_playing(cuts->video) == false)
      {
        // move back to menu
        pm_run(self->pm, "menu");
        pm_stop(self->pm, "cutscene");
      }
    }
  }

  al_set_target_backbuffer(cuts->engine->display);
  al_draw_scaled_bitmap(cuts->engine->buffer, 0, 0, cuts->engine->buffer_width, cuts->engine->buffer_height, 0, 0, cuts->engine->width, cuts->engine->height, 0);

  ALLEGRO_KEYBOARD_STATE kbd;
  al_get_keyboard_state(&kbd);

  int i;
  for (i = 0; i < 0xFF; i++)
    if (al_key_down(&kbd, i) && cuts->engine->ipc.code != 2)
    {
      pm_stop(self->pm, "cutscene");
      pm_run(self->pm, "menu");
    }

}

void p_cutscene_dtor (proc_t * self)
{
  printf("cutscene dtor()\n");
  p_cutscene_t * cuts = (p_cutscene_t *) self->mem;
  al_close_video(cuts->video);
  free(cuts);
}
