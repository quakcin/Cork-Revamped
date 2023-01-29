
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
// #include <unistd.h>
#include <fcntl.h>

#include "p_menu.h"
#include "p_engine.h"
#include "c_sched.h"
#include "main.h"
#include "pm.h"

button_t * button_ctor (void)
{
  button_t * self = (button_t *) malloc(sizeof(struct button_s));
  __GUARD(self, "button_t obj");
  self->next = NULL;
  self->value = 0;
  return self;
}

void button_dtor (button_t * self)
{
  button_t * ptr;
  button_t * nxt;
  for (ptr = self->next; ptr != NULL; ptr = nxt)
  {
    nxt = ptr->next;
    __UNGUARD(ptr, "button_t node");
    free(ptr);
  }

  __UNGUARD(self, "button_t obj");
  free(self);
}

void button_push (button_t * page, char * text, void (*event)( struct button_s * self, struct menu_s * menu ), int value, bool enabled)
{
  button_t * btn = button_ctor();
  page->value += 1;
  btn->next = page->next;
  page->next = btn;
  strcpy(btn->text, text);
  btn->event = event;
  btn->value = value;
  btn->enabled = enabled;
}

/*
  --------------------------------------------------------------------------------
  -- Pages
  --------------------------------------------------------------------------------
*/

// ----------------------------------------
// -- Menu
// ----------------------------------------

void page_menu_quit (button_t * self, menu_t * menu)
{
  menu->engine->pm->state = PM_STOPPED;
}

void OBSOLETE_page_menu_new_game (button_t * self, menu_t * menu)
{
  pm_run(menu->engine->pm, "game");
  pm_stop(menu->engine->pm, "menu");
}

void page_menu_switch_opt (button_t * self, menu_t * menu)
{
  menu->swap = page_options(menu);
}

void page_menu_switch_load (button_t * self, menu_t * menu)
{
  menu->swap = page_save_or_load(false);
}

void page_menu_switch_save (button_t * self, menu_t * menu)
{
  menu->swap = page_save_or_load(true);
}

void page_menu_resume (button_t * self, menu_t * menu)
{
  // resume game
  pm_stop(menu->pm, "menu");
  pm_release(menu->pm, "game");
  // restore mouse state

  engine_t * engine = (engine_t *) pm_get(menu->pm, "engine")->mem;
  al_hide_mouse_cursor(engine->display);
  al_set_mouse_xy(engine->display, menu->lmx, menu->lmy);
}

void page_menu_quite_during_game (button_t * self, menu_t * menu)
{
  menu->engine->ipc.target = 666; // game
  menu->engine->ipc.code = 21; // quit and go to menu
  pm_stop(menu->pm, "menu");
  pm_release(menu->pm, "game");
}

void page_menu_authors (button_t * self, menu_t * menu)
{
  menu->engine->ipc.target = 1;
  menu->engine->ipc.code = 3;
  pm_stop(menu->pm, "menu");
  pm_run(menu->pm, "cutscene");
}

void page_theme_select (button_t * self, menu_t * menu)
{
  /* rise ipc to reload prefetched buffers */
  menu->engine->ipc.target = 666;
  menu->engine->ipc.code = 90 + self->value;
  pm_stop(menu->pm, "menu");
  pm_run(menu->pm, "game");
}

void page_menu_theme_selector (button_t * self, menu_t * menu)
{
  button_t * theme_menu = button_ctor();

  button_push(theme_menu, "COFNIJ", page_opt_back, 0, true);
  button_push(theme_menu, "DWOREK", page_theme_select, 1, true);
  button_push(theme_menu, "KAZAMATY", page_theme_select, 2, true);
  button_push(theme_menu, "POSESJA", page_theme_select, 3, true);
  button_push(theme_menu, "WYBIERZ MAPĘ:", NULL, 0, false);

  menu->swap = theme_menu;
}



button_t * page_main_menu (menu_t * menu)
{
  button_t * self = button_ctor();
  
  if (menu->during_game)
  {
    // button_push(self, "THEME", page_menu_theme_selector, 0, true); /* out of time exception */
    button_push(self, "ZAKOŃCZ", page_menu_quite_during_game, 0, true);
  }
  else
  {
    button_push(self, "WYJDZ", page_menu_quit, 0, true);
    button_push(self, "AUTORZY", page_menu_authors, 0, true);
  }

  button_push(self, "OPCJE", page_menu_switch_opt, 0, true);
  button_push(self, "ZAPISZ", page_menu_switch_save, 0, menu->during_game);

  button_push(self, "WCZYTAJ", page_menu_switch_load, 0, !menu->during_game);

  if (menu->during_game == false)
  {
    button_push(self, "NOWA   GRA", page_menu_theme_selector, 0, true);
  }
  else
  {
    button_push(self, "POWRÓT", page_menu_resume, 0, true);
  }

  return self;
}


// ----------------------------------------
// -- Opcje
// ----------------------------------------


void page_opt_back (button_t * self, menu_t * menu)
{
  menu->swap = page_main_menu(menu);
}

void page_opt_cap_toggle (button_t * self, menu_t * menu)
{
  menu->engine->use_captions = !menu->engine->use_captions;
  menu->swap = page_options (menu);
}

void page_opt_full_toggle (button_t * self, menu_t * menu)
{
  menu->engine->is_fullscreen = !menu->engine->is_fullscreen;
  // printf("Fullscreen status: %d\n", menu->engine->is_fullscreen);
  al_set_display_flag(menu->engine->display, ALLEGRO_FULLSCREEN_WINDOW, menu->engine->is_fullscreen);
  menu->swap = page_options(menu);
}

/*
  menu->font should be updated with every single menu_init() 
  TODO: move this to init()!!
*/
void page_opt_buff_toggle (button_t * self, menu_t * menu)
{
  static int opts [][3] = 
  {
    {1920, 1080, 42},
    {1280, 720, 24},
    {640, 320, 16},
    {320, 160, 8}
  };

  self->value = (self->value + 1) & 3;
  self->text[0] = '\0';
  sprintf(self->text, "BUFOR   %dx%d", opts[self->value][0], opts[self->value][1]);

  al_destroy_bitmap(menu->engine->buffer);
  menu->engine->buffer = al_create_bitmap(opts[self->value][0], opts[self->value][0]);
  menu->engine->buffer_width = opts[self->value][0];
  menu->engine->buffer_height = opts[self->value][1];
  
  if (menu->engine->is_fullscreen)
  {
    ALLEGRO_MONITOR_INFO minf;
    al_get_monitor_info(0, &minf);
    menu->engine->width = minf.x2 - minf.x1 + 1;
    menu->engine->height = minf.y2 - minf.y1 + 1;
  }
  else
  {
    menu->engine->width = menu->engine->buffer_width;
    menu->engine->height = menu->engine->buffer_height;
  }
  
  al_destroy_font(menu->font);
  menu->font = al_load_ttf_font("./DATA/font.ttf", opts[self->value][2], ALLEGRO_TTF_NO_KERNING);

  al_resize_display(menu->engine->display, menu->engine->buffer_width, menu->engine->buffer_height);
  // al_set_display_flag(menu->engine->display, ALLEGRO_FULLSCREEN_WINDOW, true);

  // sleep(1);
}

void page_opt_brightness (button_t * self, menu_t * menu)
{
  const long gma = ((menu->engine->gamma - 30) / 15);
  const long tgl = (gma + 1) % 6;
  // printf("New gma %ld\n", tgl);

  menu->engine->gamma = tgl * 15 + 30;
  menu->swap = page_options (menu);
}

button_t * page_options (menu_t * menu)
{
  button_t * self = button_ctor();
  char buff[0xFF];

  button_push(self, "COFNIJ", page_opt_back, 0, true);

  const int gma = ((menu->engine->gamma - 30) / 15) - 3;

  sprintf(buff, "JASNOŚĆ  %s%d", 
    gma < 0 ? " " : " +", 
    gma >= 0 ? gma + 1 : gma
  );
  button_push(self, buff, page_opt_brightness, 0, true);

  sprintf(buff, "NAPISY:   %s", menu->engine->use_captions ? "WŁ" : "WYŁ");
  button_push(self, buff, page_opt_cap_toggle, 0, true);

  button_push(self, menu->engine->is_fullscreen 
    ? "TRYB W OKNIE" 
    : "PEŁEN EKRAN", 
    page_opt_full_toggle, 0, true);

  sprintf(buff, "BUFOR   %dx%d", 
    menu->engine->buffer_width, 
    menu->engine->buffer_height
  );
  button_push(self, buff, page_opt_buff_toggle, 1, true);

  return self;
}


/*
  --------------------------------------------------------------------------------
  -- Save / Load:
  --------------------------------------------------------------------------------
*/

void page_save (button_t * self, menu_t * menu)
{
  menu->engine->ipc.target = 666;
  menu->engine->ipc.code = 60 + self->value;
  // printf("self value set := %d\n", menu->engine->ipc.code);
  pm_release(menu->engine->pm, "game");
  pm_stop(menu->engine->pm, "menu");
}

void page_load (button_t * self, menu_t * menu)
{
  // 1. set IPC for 666 and 5x (x - game save no.)
  // 2. rise game & rednerer, kill menu

  menu->engine->ipc.target = 666;
  menu->engine->ipc.code = 50 + self->value;

  pm_run(menu->engine->pm, "game");
  pm_stop(menu->engine->pm, "menu");
}

button_t * page_save_or_load (bool is_saving)
{
  button_t * self = button_ctor();

  // there are 4 save spots
  button_push(self, "COFNIJ", page_opt_back, 0, true);
  int i;

  for (i = 0; i < 4; i++)
  {
    int idx = 4 - i;
    char path[0xFF];
    sprintf(path, "./DATA/sav/map%d.sav", idx);
    bool file_exists = access(path, 0) == 0;

    if (file_exists)
      printf("File %s exists!!!\n", path);

    char msg[0xFF];
    if (file_exists == false)
    {
      sprintf(msg, "%d.   %s", idx, "Brak");
    }
    else
    {
      struct stat fs;
      int fd = open(path, O_RDONLY);
      fstat(fd, &fs);
      close(fd);

      #ifdef _WIN32
        time_t ltime = (time_t) fs.st_mtime; /* Must be POSIX compatabile */
      #else
        time_t ltime = (time_t) fs.st_mtim.tv_sec;
      #endif
      struct tm * tmt = localtime(&ltime);
      sprintf(msg, "%d.   %s %d/%d/%d %d:%d",                                                                                                                                    idx, "Zapis", 
        tmt->tm_mday,
        tmt->tm_mon + 1, 
        tmt->tm_year + 1900, 
        tmt->tm_hour, 
        tmt->tm_min 
      );
    }

    button_push(self, msg, is_saving ? page_save : page_load, idx, is_saving ? true : file_exists);
  }

  return self;
}

/*
  --------------------------------------------------------------------------------
  -- Proces Stuff:
  --------------------------------------------------------------------------------
*/

void p_menu_ghosting (sched_t * sched, void * ptr, int val)
{
  menu_t * self = (menu_t *) ptr;
  self->ghosting = false;
}

void p_menu_init (proc_t * self)
{
  engine_t * engine = (engine_t *) pm_get(self->pm, "engine")->mem;
  menu_t * menu = (menu_t *) malloc(sizeof(struct menu_s));
  __GUARD(menu, "menu_t obj");
  self->mem = menu;
  menu->pm = self->pm;
  menu->swap = NULL;
  menu->video = NULL;
  menu->font = al_load_ttf_font("./DATA/font.ttf", 24, ALLEGRO_TTF_NO_KERNING);
  menu->during_game = ( pm_get(self->pm, "game")->state == P_HOLD );
  menu->ghosting = true;
  menu->sched = sched_ctor();
  yield(menu->sched, p_menu_ghosting, menu, 0, 500);

  if (menu->during_game == false)
  {
    menu->video = al_open_video("./DATA/res0/menu.ogv");
    if (menu->video != NULL)
      al_start_video(menu->video, al_get_default_mixer());
  }

  // remember mouse position
  ALLEGRO_MOUSE_STATE mstate;
  al_get_mouse_state(&mstate);

  menu->lmx = mstate.x;
  menu->lmy = mstate.y;

  menu->buttons = page_main_menu(menu);
  menu->engine = engine;
  al_show_mouse_cursor(engine->display);
}

void p_menu_update (proc_t * self)
{
  // ----------------------------------------
  // -- Swap old buttons

  menu_t * menu = (menu_t *) self->mem;

  sched_update(menu->sched, menu->engine);

  if (menu->swap != NULL)
  {
    button_dtor(menu->buttons);
    menu->buttons = menu->swap;
    menu->swap = NULL;
  }

  // ----------------------------------------
  // -- Proceed

  al_set_target_bitmap(menu->engine->buffer);

  // Draw Background Movie

  button_t * ptr;
  int yoff = al_get_font_line_height(menu->font) + menu->engine->buffer_height / 30;
  float h = (float) (menu->engine->buffer_height - yoff * menu->buttons->value) / 2;
  int i = 0;

  if (menu->video != NULL || menu->during_game == false)
  {
    ALLEGRO_BITMAP * frame = al_get_video_frame(menu->video);
    if (frame != NULL)
    {
      al_draw_scaled_bitmap(frame, 0, 0, 1280, 720, 0, 0, menu->engine->buffer_width, menu->engine->buffer_height, 0);
      if (al_is_video_playing(menu->video) == false)
      {
        al_seek_video(menu->video, 0.0);
        al_set_video_playing(menu->video, true);
      }
    }
  }

  // Draw Menu

  ALLEGRO_MOUSE_STATE mstate;
  al_get_mouse_state(&mstate);

  float dx = ((float) menu->engine->width / (float) menu->engine->buffer_width);
  float dy = ((float) menu->engine->height / (float) menu->engine->buffer_height); 

  float mx = (float) mstate.x;
  float my = (float) mstate.y;

  for (ptr = menu->buttons->next; ptr != NULL; ptr = ptr->next)
  {
    float w = (float) al_get_text_width(menu->font, ptr->text);
    bool hover = mx > 50 * dx
              && mx < (50 + w) * dx
              && my > (h + (float) yoff * i) * dy
              && my < (h + (float) yoff * (i + 1)) * dy;

    ALLEGRO_COLOR color = (ptr->enabled == false) ? al_map_rgb(46, 46, 46) : hover
      ? al_map_rgb(0, 0, 0)
      : al_map_rgb(255, 255, 255);

    al_draw_text(menu->font, color, 50, h + (float) (yoff * i), ALLEGRO_ALIGN_LEFT, ptr->text);
    i++;

    if (mstate.buttons & 1 && hover && ptr->enabled && menu->ghosting == false)
    {
      // onclick!
      if (ptr->event == NULL)
      {
        printf("'%s' => even() is null\n", ptr->text);
      }
      else
      {
        ptr->event(ptr, menu);
      }

      menu->ghosting = true;
      yield(menu->sched, p_menu_ghosting, menu, 0, 500);
    }
  }

  // BackBuffer
  al_set_target_backbuffer(menu->engine->display);
  al_draw_scaled_bitmap(menu->engine->buffer, 0, 0, menu->engine->buffer_width, menu->engine->buffer_height, 0, 0, menu->engine->width, menu->engine->height, 0);

}

void p_menu_dtor (proc_t * self)
{
  menu_t * menu = (menu_t *) self->mem;
  al_destroy_font(menu->font);
  if (menu->during_game == false)
    al_close_video(menu->video);
  button_dtor(menu->buttons);
  if (menu->swap != NULL)
    button_dtor(menu->swap); // highly unlikelly

  sched_dtor(menu->sched);

  __UNGUARD(menu, "menu_t obj");
  free(menu);
}