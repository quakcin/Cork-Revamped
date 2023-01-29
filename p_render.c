
#include <math.h>
#include <stdio.h>

#include "pm.h"
#include "p_render.h"
#include "p_engine.h"
#include "p_game.h"
#include "c_player.h"
#include "c_enemy.h"
#include "c_map.h"

#include "main.h"
#include "i_math.h"

// #define GAMMA 30
// #define GAMMA 90
// #define GAMMA 120

void p_render_cap_init (render_t * self, engine_t * eng)
{
  if (self->font != NULL)
    al_destroy_font(self->font);

  self->font = al_load_ttf_font("./DATA/cap.ttf", eng->buffer_height / 25, ALLEGRO_TTF_NO_KERNING);
  self->cap_height = eng->buffer_height;
}

void p_render_init (proc_t * proc)
{
  engine_t * eng = (engine_t *) proc->pm->proc_list->mem;
  render_t * self = (render_t *) malloc(sizeof(struct render_s));
  proc->mem = (void *) self;
  self->cap_timer = 0;
  self->font = NULL;
  p_render_cap_init(self, eng);
}

static void intersect (float * ix, float * iy, float ra, float rb, float rc, float wa, float wb, float wc)
{
  *ix = (rb * wc - rc * wb) / (ra * wb - rb * wa);
  *iy = (rc * wa - ra * wc) / (ra * wb - rb * wa);
}

// keep this in fast memory

static long ZBUFFER [ZBUFFER_SIZE];
static int LIGHTBUFFER [ZBUFFER_SIZE];
static sprite_t SPRITES [SPRITE_BUFFER_SIZE];
static int SPRITE_ADDR [SPRITE_BUFFER_SIZE];
static unsigned int sprite_count;

void draw_sprites (game_t * game) // using globals for optimalization
{
  // sort sprites regarding z 
  int i, j;

  for (i = 0; i < sprite_count; i++)
    SPRITE_ADDR[i] = sprite_count - i - 1;

  for (i = 0; i < sprite_count; i++)
    for (j = i; j < sprite_count; j++)
      if (i != j)
        if (SPRITES[SPRITE_ADDR[i]].z < SPRITES[SPRITE_ADDR[j]].z)
        {
          int adr = SPRITE_ADDR[i];
          SPRITE_ADDR[i] = SPRITE_ADDR[j];
          SPRITE_ADDR[j] = adr;
        }

  // draw back to front, clipping edges
  const float view_dist = (game->engine->buffer_width / 2) / tan(ANG30);

  for (i = 0; i < sprite_count; i++)
  {
    sprite_t * spr = &SPRITES[SPRITE_ADDR[i]];
    const float ang = atan2(spr->y, spr->x) - game->player->ang;
    const int size = (int) ((view_dist / (cos(ang) * spr->z)) * 360);
    const float x = tan(ang) * view_dist;
    const int x_off = (int)(game->engine->buffer_width / 2 + x - (size / 2));
    const int y_off = (int)(game->engine->buffer_height - size) / 2;
    const float unit = 256 / (float) size;
    const int far = x_off + size > game->engine->buffer_width 
      ? game->engine->buffer_width - 1 : x_off + size;
    const int near = x_off < 0 ? 0 : x_off;
    int i;

    ALLEGRO_BITMAP * txt = game->map->texture_buffer[spr->txt]; // TODO: Make and Load Texture Buffer
    long s = 255; 
    
    if (txt == NULL)
      continue; // weird!
    
    if ((spr->txt > MAP_LAMPS_START && spr->txt < MAP_LAMPS_END || spr->txt == 0) == false)
    {
      s = (1 << 12) / (long) spr->z << 4;
      if (s < 0)
        s = 0;
      else if (s > 255)
        s = 255;
    }

    // // draw unless behind wall
    // for (i = near; i < far; i++)
    //   if (ZBUFFER[i] < size)
    //     al_draw_tinted_scaled_bitmap
    //     (
    //       txt, al_map_rgb(s, s, s), (i - x_off) * unit, 0, unit, 256,
    //       i, y_off + game->player->yoff, 1, size, 0
    //     );

    /*
      Clip from left and right
    */
   
    if (near > far)
      goto __escape_prop_drawing;

    if (ZBUFFER[near] > size && ZBUFFER[far - 1] > size)
      goto __escape_prop_drawing;

    int clip_left = near;
    int clip_right = far;

    for (i = near; i < far; i++)
      if (ZBUFFER[i] <= size)
      {
        clip_left = i;
        break;
      }

    for (i = far - 1; i >= near; i--)
      if (ZBUFFER[i] <= size)
      {
        clip_right = i;
        break;
      }    

    const int width = clip_right - clip_left;

    if (width > 0)
      al_draw_tinted_scaled_bitmap
      (
        txt, al_map_rgb(s, s, s), 
        (clip_left - x_off) * unit, 0, unit * width, 256,
        clip_left, y_off + game->player->yoff, width, size, 0
      );

    
    // restore sprite in map
    // map[spr->mx][spr->my] = spr->txt;
__escape_prop_drawing:
    game->map->tiles[spr->mx][spr->my] = spr->txt;
  }

}

void draw_viz_buffer (game_t * game)
{
  const float la = fmod(game->player->ang + ANGN30, PI2);
  const float ra = fmod(game->player->ang + ANG30, PI2);

  viz_t * ptr;
  for (ptr = game->viz; ptr != NULL; ptr = ptr->next)
  {
    const float ma = atan2(*ptr->y - game->player->y, *ptr->y - game->player->x);
    if (true || ma > la && ma < ra)
    {
      SPRITES[sprite_count].mx = 0;
      SPRITES[sprite_count].my = 0;
      SPRITES[sprite_count].x = *ptr->x - game->player->x;
      SPRITES[sprite_count].y = *ptr->y - game->player->y;
      SPRITES[sprite_count].z = sqrt(
        SPRITES[sprite_count].x * SPRITES[sprite_count].x +
        SPRITES[sprite_count].y * SPRITES[sprite_count].y 
      );
      SPRITES[sprite_count].txt = ptr->txt;
      if (SPRITES[sprite_count].z <= 0)
        continue; // debug
      sprite_count++;
    }
  }
}

void draw_light (game_t * game)
{
  int x;
  for (x = 0; x < game->engine->buffer_width; x++)
  {
    int light = LIGHTBUFFER[x] * game->map->apprx_lightness / 512;

    if (light < 0)
      light = 0;
    else if (light > 255)
      light = 255;

    al_draw_line(x, 0, x, game->engine->buffer_height, al_premul_rgba(0, 0, 0, light), 1);
  }
}

void calc_lightness (game_t * game)
{
  // find lights in spritebuffer
  game->map->lightness = 512 + 256;

  int i;
  for (i = 0; i < sprite_count; i++)
    if (SPRITES[i].txt > MAP_LAMPS_START && SPRITES[i].txt < MAP_LAMPS_END)
      game->map->lightness = 512 - 32;
  
  if (game->map->apprx_lightness < game->map->lightness)
    game->map->apprx_lightness += 2;
  else
    game->map->apprx_lightness -= 3;
}

void draw_overlay (game_t * game)
{
  if (game->player->health > 80)
    return;

  int idx = 5 - (game->player->health / 16);

  if (game->player->health <= 0)
    idx = 4;
  
  al_draw_scaled_bitmap
  (
    game->map->overlays[idx], 
    0, 0, 
    1920, 1080, 
    0, 0, 
    game->engine->buffer_width, game->engine->buffer_height, 
    0
  );
}

void draw_captions (render_t * self, engine_t * engine)
{
  if (self->cap_timer > 0)
  {
    if (engine->buffer_height != self->cap_height)
      p_render_cap_init(self, engine);

    int w = al_get_text_width(self->font, self->captions) + 25;
    int h = al_get_font_line_height(self->font) + 10;
    int xoff = (engine->buffer_width - w) >> 1;
    int yoff = (engine->buffer_height) - ((engine->buffer_height - h) >> 3) - 8;
    al_draw_filled_rectangle(xoff, yoff, xoff + w, yoff + h, al_map_rgb(0, 0, 0));
    al_draw_text
    (
      self->font, 
      al_map_rgb(255, 255, 255), 
      engine->buffer_width >> 1, 
      engine->buffer_height - (engine->buffer_height >> 3), 
      ALLEGRO_ALIGN_CENTER, 
      self->captions
    );
    self->cap_timer--;
  }
}

/*
  (c) 2022 PśK Algorytm Ślusarczyk-Bandura
      Efektywne trawersowanie przestrzeni polinominalnych.
*/

void p_render_update (proc_t * self)
{
  game_t * game = (game_t *) (pm_get(self->pm, "game")->mem);
  engine_t * engine = (engine_t *) self->pm->proc_list->mem;

  if (engine->refresh <= 0)
    return;

  al_set_target_bitmap(engine->buffer);

  float a, da;
  int i, x;
  a = ANG360 + (game->player->ang - ANG30);
  da = 1 * (ANG60 / game->engine->buffer_width);
  sprite_count = 0;

  // paint floor and ceiling
  al_draw_filled_rectangle(0, 0, game->engine->buffer_width, game->engine->buffer_height >> 1, game->map->colors[game->map->color][0]);
  al_draw_filled_rectangle(0, game->engine->buffer_height >> 1, game->engine->buffer_width, game->engine->buffer_height, game->map->colors[game->map->color][1]);

  // cast a to <0, 2pi> space
  a = fmod(a, 2 * 3.1415);

  // -- dereferenced variables outside loop
  const int buffer_width = game->engine->buffer_width;
  const int buffer_height = game->engine->buffer_height;
  const float player_x = game->player->x;
  const float player_y = game->player->y;
  const int player_yoff = (int) game->player->yoff;
  map_t * map = game->map; // shorter dereference still

  ALLEGRO_BITMAP ** texture_buffer = (void *) &(map->texture_buffer);

  for (x = 0; x < buffer_width; x += 1)
  {
    float rx = player_x;
    float ry = player_y;

    // find any far point in raycast path
    const float frx = player_x + cos(a) * 0xFFFF;
    const float fry = player_y + sin(a) * 0xFFFF;

    // basic line equation
    const float ra = player_y - fry;
    const float rb = frx - player_x;
    const float rc = player_x * fry - player_y * frx;

    // find tile offsets with possible intersections
    const int bmxoff = ((int) rx) & 255 == 0
      ? (a > ANGN90 || a < ANG90 ? 1 : -1) : (frx - rx) < 0 ? 0 : 1;

    const int bmyoff = ((int) ry) & 255 == 0
      ? (a > ANG0 && a < ANG180 ? 1 : -1) : (fry - ry) < 0 ? 0 : 1;

    // begin traversing plane
    for (i = 0; i < 32; i++)
    {
      float vix, viy, hix, hiy;
      // intersect(&hix, &hiy, ra, rb, rc, -1 * (float) buffer_width, 0, (float) buffer_width * (float) (((int) rx >> 8) + bmxoff) * 256);
      // intersect(&vix, &viy, ra, rb, rc, 0, (float) buffer_width, -1 * (float) ((((int) ry >> 8) + bmyoff) * buffer_width) * 256);
      intersect(&hix, &hiy, ra, rb, rc, -1 * (float) buffer_width, 0, (float) buffer_width * (float) ((((int) rx >> 8) + bmxoff) << 8));
      intersect(&vix, &viy, ra, rb, rc, 0, (float) buffer_width, -1 * (float) (((((int) ry >> 8) + bmyoff) * buffer_width) << 8));

      const int omx = (int) rx >> 8;
      const int omy = (int) ry >> 8;

      // nearest one is the correct one
      const float dv = abs((vix - rx) * (viy - ry));
      const float dh = abs((hix - rx) * (hiy - ry));

      // if ((dvx * dvx) + (dvy * dvy) < (dhx * dhx) + (dhy * dhy))
      if (dv < dh)
      {
        rx = vix;
        ry = viy;
      }
      else
      {
        rx = hix;
        ry = hiy;
      }

      int mx = (int) rx >> 8;
      int my = (int) ry >> 8;

      // unstuck if stuck

      if (mx == omx && my == omy)
      {
        // one more cast
        rx += cos(a);
        ry += sin(a);
        mx = (int) rx >> 8;
        my = (int) ry >> 8;
      }

      if (mx < 0 || my < 0 || mx > 510 || my > 510)
        continue;

      const uint8_t tile = map->tiles[mx][my];

      if (tile > MAP_SPRITES_START)
      {
        // buffer up sprite
        // printf("WID: %hhd, \n", tile);
        SPRITES[sprite_count].mx = mx;
        SPRITES[sprite_count].my = my;
        SPRITES[sprite_count].x = (float)(mx << 8) + 128 - player_x;
        SPRITES[sprite_count].y = (float)(my << 8) + 128 - player_y;
        SPRITES[sprite_count].z = sqrt(
          SPRITES[sprite_count].x * SPRITES[sprite_count].x +
          SPRITES[sprite_count].y * SPRITES[sprite_count].y 
        );
        SPRITES[sprite_count].txt = map->tiles[mx][my];
        
        // HACK: Remove momentarly sprite evidence from map heap
        // and restore as soon as finished drawing
        game->map->tiles[mx][my] = 0;
        sprite_count++;
        
      }
      else if (tile > MAP_WALL_START)
      {
        // printf("TID: %hhd, \n", tile);
        // calculate texture offset
        const int mxs = ((int) rx) & 255;
        const int tx = (mxs == 0 || mxs == 255)
          ? (((int) ry) & 255)
          : mxs;

        // begin drawing
        const int r = q_sqrt((int)(pow(player_x - rx, 2) + pow(player_y - ry, 2)));
        // const int r = q_dist(player_x, player_y, rx, ry);
        const int h = (buffer_height << 9) / (int) (cos(a - game->player->ang) * r);

        // calculate lightness
        const int s = ((((r << 6) / engine->gamma) << 4) >> 7);
        if (s < 0)
          LIGHTBUFFER[x] = 0;
        else if (s > 255)
          LIGHTBUFFER[x] = 255;
        else
          LIGHTBUFFER[x] = s;

        // draw it and buffer rest  
        const int y_off = (((int) buffer_height - h) >> 1) + player_yoff;
        al_draw_scaled_bitmap(texture_buffer[tile], tx, 0, 1, 256, x, y_off, 1, h, 0);
        ZBUFFER[x] = h;
        break;
      }
    }
    a += da; // next angle
  }

  // draw_enemy(game); TODO: Remove depriciated
  draw_viz_buffer(game);
  calc_lightness(game);
  draw_light(game);
  draw_sprites(game); // shift stack org, so compiler doesn't complain anymore.
  draw_overlay(game);

  if (engine->use_captions)
    draw_captions(self->mem, engine);

  // draw effects

  if (game->map->darkness > 0)
  {
    al_draw_filled_rectangle(0, 0, game->engine->buffer_width, game->engine->buffer_height, al_map_rgba(0, 0, 0, game->map->darkness));
    game->map->darkness -= 3;
  }

  // swap buffers

  al_set_target_backbuffer(engine->display);
  al_draw_scaled_bitmap(engine->buffer, 0, 0, game->engine->buffer_width, game->engine->buffer_height, 0, 0, engine->width, engine->height, 0);
}

void p_render_dtor (proc_t * self)
{
  render_t * rend = (render_t *) self->mem;
  al_destroy_font(rend->font);
  free(self->mem);
}

viz_t * viz_ctor (game_t * game)
{
  viz_t * self = (viz_t *) malloc(sizeof(struct viz_s));
  __GUARD(self, "viz_t obj");
  self->next = game->viz; // 1 depth ptr, might segv
  game->viz = self;
  return self;
}

void viz_dtor (game_t * game)
{
  viz_t * ptr;
  viz_t * nxt;
  for (ptr = game->viz; ptr != NULL; ptr = nxt)
  {
    nxt = ptr->next;
    __UNGUARD(ptr, "viz_t obj");
    free(ptr);
  }
}

void render_use_captions (pm_t * pm, char * msg, long time)
{
  engine_t * eng = (engine_t *) pm->proc_list->mem;
  render_t * render = (render_t *) pm_get(pm, "render")->mem;
  memcpy(render->captions, msg, strlen(msg) + 1);
  render->cap_timer = time * 60;
}