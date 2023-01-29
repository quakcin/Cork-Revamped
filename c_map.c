
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

#include "p_engine.h"
#include "c_map.h"
#include "i_math.h"
#include "c_audio.h"
#include "p_game.h"
#include "c_point.h"
#include "main.h"
#include "c_sched.h"
#include "c_item.h"

#include "c_gen.h"

map_t * map_ctor (game_t * game)
{
  map_t * self = (map_t *) malloc(sizeof(struct map_s));
  __GUARD(self, "map_t obj");

  if (self == NULL)
  {
    IERROR("Failed to create map_t object!");
    return NULL;
  }

  int i, j;
  for (i = 0; i < 0xFF; i++)
    self->texture_buffer[i] = NULL;

  for (i = 0; i < 32; i++)
    for (j = 0; j < 32; j++)
      self->points[i][j] = NULL;

  self->colors[0][0] = al_map_rgb(5, 7, 10);   // ext.
  self->colors[1][0] = al_map_rgb(25, 26, 24); // salon
  self->colors[2][0] = al_map_rgb(26, 16, 31); // luxury
  self->colors[3][0] = al_map_rgb(50, 68, 69); // whatever

  self->colors[0][1] = al_map_rgb(9, 18, 15);
  self->colors[1][1] = al_map_rgb(31, 19, 16);
  self->colors[2][1] = al_map_rgb(47, 45, 51);
  self->colors[3][1] = al_map_rgb(38, 19, 19);

  self->color = 1;
  self->darkness = 255;
  self->lightness = 256;
  self->apprx_lightness = 512;
  self->points_count = 0;
  self->spawn = NULL;

  // if loading then use save based resspack, if not IPC one
  if (game->engine->ipc.target == 666 && game->engine->ipc.code > 90)
  {
    self->resspack = game->engine->ipc.code % 10; 
  }

  // generate / load map 
  // printf("Createing map obj %d %d\n", game->engine->ipc.target, game->engine->ipc.code);
  // printf("IPC %d %d\n", game->engine->ipc.target, game->engine->ipc.code);

  if (game->engine->ipc.target == 666 && game->engine->ipc.code > 50 && game->engine->ipc.code < 60)
  { /* load game */
    int no = game->engine->ipc.code % 10;
    map_load_from_save(self, no);
    points_load(self, no);
    // printf("Loading game from save file! %d\n", no);
  }
  else
  { /* new game */
    generate_map (self);
    // printf("Wrong call\n");
  }
  // printf("Ressource pack is set to %u\n", (unsigned int) self->resspack);

  // load static overlays
  for (i = 0; i < 5; i++)
  {
    char buff[0xFF];
    sprintf(buff, "./DATA/res0/b%d.png", i);
    self->overlays[i] = al_load_bitmap(buff);
    if (self->overlays[i] == NULL)
      printf("[map] [static overlays] failed to load %s\n", buff);
    al_lock_bitmap(self->overlays[i], ALLEGRO_PIXEL_FORMAT_ARGB_1555, ALLEGRO_LOCK_READONLY);
  }

  // load assets 
  map_load_texures(self);
  map_load_audio(game, self->resspack);
  // printf("finished loading audio\n");

  // if save, don't randomize cork position
  if (game->engine->ipc.target == 666 && game->engine->ipc.code > 50 && game->engine->ipc.code < 60)
    return self;

  // find random spot for cork OR LOAD it's position
  float c_dist = 0;
  do
  {
    game->item->x = (float) get_random_int(16, 511 - 16, 1) * 256;
    game->item->y = (float) get_random_int(16, 511 - 16, 1) * 256;
    c_dist = q_dist(game->item->x, game->item->y, 511 / 2, 511 / 2);
  }
  while (self->tiles[(int) game->item->x / 256][(int) game->item->y / 256] != 0 && c_dist < 20 * 512);

  return self;
}

void map_dtor (map_t * self)
{
  // first unlink animated textures
  map_unlink_animated_texures(self);

  // then remove all of the textures from the buffer
  int i, j;
  for (i = 0; i < 0xFF; i++)
    if (self->texture_buffer[i] != NULL)
    {
      al_unlock_bitmap(self->texture_buffer[i]);
      al_destroy_bitmap(self->texture_buffer[i]);
    }

  // remove overlays
  for (i = 0; i < 5; i++)
  {
    al_unlock_bitmap(self->overlays[i]);
    al_destroy_bitmap(self->overlays[i]);
  }

  // remove all of the map points
  point_t * ptr = NULL;
  point_t * nxt = NULL;

  for (j = 0; j < 32; j++)
    for (i = 0; i < 32; i++)
      if (self->points[i][j] != NULL)
        for (ptr = self->points[i][j]; ptr != NULL; ptr = nxt)
        {
          nxt = ptr->next;
          __UNGUARD(ptr, "point_t node");
          free(ptr);
        }

  __UNGUARD(self, "map_t obj");
  free(self);
}

void map_load_from_save (map_t * self, int no)
{
  char path[0xFF];
  sprintf(path, "./DATA/sav/map%d.sav", no);
  #ifdef _WIN32
    int fd = open(path, O_RDONLY | O_BINARY);
  #else
    int fd = open(path, O_RDONLY);
  #endif
  if (fd < 0)
  {
    IERROR("Failed to open map file");
    return;
  }

  int r = read(fd, self->tiles, 511 * 511);
  assert(r == 511 * 511);

  assert(read(fd, &self->resspack, sizeof(uint8_t)) == sizeof(uint8_t));

  close(fd);
  IPRINT("Map has been succesfully loaded!");
}

// map (res pack) related sounds
void map_load_audio (game_t * game, int respack)
{
  char path [0xFF];
  sprintf(path, "res%hhu", respack);

  printf("LOADING AUDIO FROM RESSOURCE PACK: %s\n", path);
  audio_load(game->audio, path, "door");
  audio_load(game->audio, path, "step");
}

void map_load_specific_texture 
(
  map_t * self, 
  uint8_t resid, 
  char * prefix, 
  int id, int frame, 
  int dest, 
  bool animated
)
{
  char path [0xFF];
  if (animated == false)
    sprintf(path, "./DATA/res%hhu/%s%d.png", resid, prefix, id);
  else
    sprintf(path, "./DATA/res%hhu/%s%d%d.png", resid, prefix, id, frame);

  ALLEGRO_BITMAP * bmp = al_load_bitmap(path);

  if (bmp == NULL)
    printf("\tFAILED TO LOAD TEXTURE: %s AT %d\n", path, dest);
  else
    printf("\tLOADED TEXTURE %s TO %d AS %p\n", path, dest, bmp);

  al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ARGB_1555, ALLEGRO_LOCK_READONLY);
  self->texture_buffer[dest] = bmp;

}

void map_load_texures (map_t * self)
{
  char tokens [][0xF][0xF] = 
  {
    {"air", {1}}, // 0
    {"sky", {4}}, // 1
    {"wd",  {4}}, // 2
    {"ws",  {8}}, // 3
    {"wa",  {4}}, // 4
    {"psc", {8}}, // 5
    {"pac", {4}}, // 6
    {"plc", {4}}, // 7
    {"pln", {4}}, // 8
    {"psn", {4}}  // 9
  };
  // where: {prefix, span, frames}

  // char path[0xFF];
  int i, j, k;
  int txc = MAP_SKY_END;
  int ano = MAP_ANIMATION_FRAMES_OFFSET; // animated offsets

  for (i = 2; i < 10; i++)
  {
    int count = (int) tokens[i][1][0];

    for (j = 0; j < count; j++)
    {
      const int frames = (i == 4 || i == 6 || i == 7 || i == 8) ? 4 : 1;
      
      for (k = 0; k < frames; k++)
      {
        if (frames == 1)
          map_load_specific_texture(self, self->resspack, tokens[i][0], j, k, txc, false);
        else
          map_load_specific_texture(self, self->resspack, tokens[i][0], j, k, ano++, true);
      }
      txc++; // main texure offsets
    }
  }

  // ----------------------------------------
  // -- Load Cork Textrues

  for (i = 0; i < MAP_CORK_SIZE; i++)
    map_load_specific_texture(self, 0, "cork", i >> 2, i & 3, MAP_CORK_BEG + i, true);
    
  // ----------------------------------------
  // -- Load Item And Explosion Textrues

  map_load_specific_texture(self, 0, "item", 0, 0, MAP_ITEM_BEG, false);

  for (i = 0; i < 7; i++)
    map_load_specific_texture(self, 0, "expl", 0, i, MAP_EXPL_BEG + i, true);
}

void map_unlink_animated_texures (map_t * self)
{
  int offsets [16];
  int i;

  // walls
  for (i = 0; i < 4; i++)
    offsets[i] = MAP_ANIM_WALL + i;

  // props
  for (i = 0; i < 4; i++)
    offsets[i + 4] = MAP_ANIM_PROP + i;

  // lamps
  for (i = 0; i < 8; i++)
    offsets[i + 8] = MAP_ANIM_LAMP + i;

  // now just update 'em
  for (i = 0; i < 16; i++)
    self->texture_buffer[offsets[i]] = NULL;
  
  // unlink cork
  self->texture_buffer[0] = NULL;

  // unlink explosion
  self->texture_buffer[MAP_EXPL] = NULL;
}

void map_update_animated_textures (sched_t * sched, void * ptr, int ticks)
{
  map_t * self = (map_t *) ptr;
  int frame = ticks & 3;
  // update texure buffer frames and pointers
  int offsets [16];
  int i;

  // walls
  for (i = 0; i < 4; i++)
    offsets[i] = MAP_ANIM_WALL + i;

  // props
  for (i = 0; i < 4; i++)
    offsets[i + 4] = MAP_ANIM_PROP + i;

  // lamps
  for (i = 0; i < 8; i++)
    offsets[i + 8] = MAP_ANIM_LAMP + i;

  // now just update 'em
  for (i = 0; i < 16; i++)
    self->texture_buffer[offsets[i]] = self->texture_buffer[(MAP_ANIMATION_FRAMES_OFFSET + i * 4) + frame];

  // explosions
  int explosion_frame = ticks % 7;
  self->texture_buffer[MAP_EXPL] = self->texture_buffer[MAP_EXPL_BEG + explosion_frame];

  yield(sched, map_update_animated_textures, ptr, ticks + 1, 250);
}

bool map_line_of_sight (map_t * self, float x, float y, float dx, float dy)
{
  const float ang = atan2(dy - y, dx - x);
  // float ang = atan2(y - dy, x - dx);
  // float dist = sqrt(pow(x - dx, 2) + pow(y - dy, 2));
  const float dist = q_dist(x, y, dx, dy);
  // printf("diff %f %f\n", dist, dist2);
  float r;

  const float vx = cos(ang) * 64;
  const float vy = sin(ang) * 64;

  for (r = 0; r < dist; r += 64)
  {
    const long mx = (long) x >> 8;
    const long my = (long) y >> 8;
    const uint8_t tile = self->tiles[mx][my];
    // printf("[checked] tile %ld\n", tile);
    // is it inside wall?
    if (tile > MAP_WALL_START && tile < MAP_WALL_END)
      return false;

    x += vx; y += vy;
  }

  return true;
}

void map_save (game_t * game, int no)
{
  // save map tiles as mapX.sav
  char map_save_path [0xFF];
  sprintf(map_save_path, "./DATA/sav/map%d.sav", no);

  #ifdef _WIN32
    int fd = open(map_save_path, O_CREAT | O_WRONLY | O_BINARY, 0600);
  #else
    int fd = open(map_save_path, O_CREAT | O_WRONLY, 0600);
  #endif

  int w = write(fd, game->map->tiles, 511 * 511);
  assert(w == 511 * 511);

  assert(write(fd, &game->map->resspack, sizeof(uint8_t)) == sizeof(uint8_t));
  close(fd);
}