
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
#include "main.h"
#include "c_sched.h"

#include "p_render.h"
#include "c_player.h"


map_t * map_ctor (game_t * game)
{
  map_t * self = (map_t *) malloc(sizeof(struct map_s));
  __GUARD(self, "map_t obj");

  if (self == NULL)
  {
    IERROR("Failed to create map_t object!");
    return NULL;
  }

  // 1. load textures
  int i;
  for (i = 0; i < 256; i++)
  {
    char path[0xFF];
    sprintf(path, "./DATA/res2/%d.png", i);
    ALLEGRO_BITMAP * bmp = al_load_bitmap(path);

    if (bmp == NULL)
      printf("\tFAILED TO LOAD TEXTURE: %s AT %d\n", path, i);
    
    self->texture_buffer[i] = bmp;
  }

  // 2. load mapfile.map
  char path[0xFF];
  sprintf(path, "./DATA/res2/mapfile.map");
  #ifdef _WIN32
    int fd = open(path, O_RDONLY | O_BINARY);
  #else
    int fd = open(path, O_RDONLY);
  #endif
  if (fd < 0)
  {
    IERROR("Failed to open map file");
    return self;
  }

  int r = read(fd, self->tiles, 512 * 512);
  // assert(read(fd, &self->resspack, sizeof(uint8_t)) == sizeof(uint8_t));

  close(fd);
  IPRINT("Map has been succesfully loaded!");

  // 3. fin player spawn + endgame spawn
  int x, y;
  for (y = 0; y < 512; y++)
    for (x = 0; x < 512; x++)
    {
      /* is player spawn? */
      if (self->tiles[x][y] == 255)
      {
        game->player->x = x * BMW + (BMW / 2);
        game->player->y = y * BMW + (BMW / 2);
        break;
      }
    }

  /* set colors for fun*/

  self->colors[0][0] = al_map_rgb(5, 7, 10);   // ext.
  self->colors[1][0] = al_map_rgb(25, 26, 24); // salon
  self->colors[2][0] = al_map_rgb(26, 16, 31); // luxury
  self->colors[3][0] = al_map_rgb(50, 68, 69); // whatever

  self->colors[0][1] = al_map_rgb(9, 18, 15);
  self->colors[1][1] = al_map_rgb(31, 19, 16);
  self->colors[2][1] = al_map_rgb(47, 45, 51);
  self->colors[3][1] = al_map_rgb(38, 19, 19);
  self->color = 0;

  IPRINT("REACHED END OF MAP_CTOR");
  return self; // swistu swistu logika poszla w pizdu
}

void map_dtor (map_t * self)
{
  __UNGUARD(self, "map_t obj");
  free(self);
}
