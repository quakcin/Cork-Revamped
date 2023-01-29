
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "c_point.h"
#include "p_game.h"
#include "c_player.h"
#include "c_map.h"

#include "i_math.h"
#include "c_sched.h"
#include "main.h"

#include "p_render.h"


void reset_points (game_t * game)
{
  int i, j;
  for (j = 0; j < 32; j++)
    for (i = 0; i < 32; i++)
    {
      point_t * ptr;
      for (ptr = game->map->points[i][j]; ptr != NULL; ptr = ptr->next)
        ptr->enabled = true;
    }
}

void try_creating_point (game_t * game)
{
  long px = (long) game->player->x;
  long py = (long) game->player->y;
  long mx = px >> 12;
  long my = py >> 12;

  if (px & 255 == 0 || py & 255 == 0)
    return; // block maps are magic

  point_t * point = NULL;

  if (game->map->points[mx][my] == NULL)
  {
    game->map->points[mx][my] = (point_t *) malloc(sizeof(struct point_s));
    __GUARD(game->map->points[mx][my], "point_t parent");
    // printf("Created new point head at %ld %ld for %f %f\n", mx, my, game->player->x, game->player->y);

    game->map->points[mx][my]->next = NULL;
    point = game->map->points[mx][my];
    if (game->map->spawn == NULL)
      game->map->spawn = point;
  }
  else
  {
    // if there is point nearby, don't create it!
    int i, j;
    for (i = -1; i < 2; i++)
      for(j = -1; j < 2; j++)
        for (point = game->map->points[mx + i][my + j]; point != NULL; point = point->next)
          if (get_dist(point->x, point->y, game->player->x, game->player->y) < MAP_POINTS_DIST)
          {
            // point->enabled = true; // FIXME pls
            return;
          }

    // if not, create
    point = (point_t *) malloc(sizeof(struct point_s));
    // printf("Spawned Point");
    __GUARD(point, "point_t child");
    point->next = game->map->points[mx][my];
    game->map->points[mx][my] = point;
    // printf("Child point %ld %ld %f %f\n", mx, my, game->player->x, game->player->y);
  }

  point->x = game->player->x;
  point->y = game->player->y;
  point->enabled = true;
  game->map->points_count++;

  // -- debug only
  // add point to vizplanes

  // viz_t * viz = viz_ctor(game);
  // viz->x = &point->x;
  // viz->y = &point->y;
  // viz->txt = 34; 
}

void task_create_point (sched_t * sched, void * ptr, int val)
{
  try_creating_point((game_t *) ptr);
  yield(sched, task_create_point, ptr, val, 100);
}

point_t * find_nearest_point (game_t * game, float x, float y)
{
  long mx = (long) x >> 12;
  long my = (long) y >> 12;
  int i, j;
  point_t * best = NULL;
  point_t * ptr;
  float bdist = FLT_MAX; 

  int depth = 2;
  int safe = 0;

  while (best == NULL)
  {
    for (i = -depth; i <= depth; i++)
      for (j = -depth; j <= depth; j++)
      {
        // printf("Checking block %ld %ld\n", mx - depth, my - depth);
        if (mx + i < 0 || mx + i >= 32 || my + j < 0 || my + j >= 32)
          continue;

        if (safe++ > MAX_POINT_ITER)
          return NULL;
          
        for (ptr = game->map->points[mx + i][my + j]; ptr != NULL; ptr = ptr->next)
        {
          float cdist = q_dist(x, y, ptr->x, ptr->y);

          if (cdist > MAP_POINTS_RESTORE)
            ptr->enabled = true;

          if (ptr->enabled == false)
            continue;

          if (best == NULL || cdist < bdist)
          {
            best = ptr;
            bdist = cdist;
          }
        }
      }
    if (best != NULL)
      break;
    else
      depth += 1;
  }

  return best;
}


void points_save (game_t * game, int save_no)
{
  /*
    FILE STRUCTURE:
    unsigned long   COUNT

    0               X       (float)
    0               Y       (float)
    0               ENABLED (bool)
    .
    COUNT
  */

  char save_path [0xFF];
  sprintf(save_path, "./DATA/sav/points%d.sav", save_no);

  #ifdef _WIN32
    int fd = open(save_path, O_CREAT | O_WRONLY | O_BINARY, 0600);
  #else
    int fd = open(save_path, O_CREAT | O_WRONLY, 0600);
  #endif

  /*
    Count points
  */

 unsigned long count = 0;
 int i, j;
 point_t * ptr;

 for (i = 0; i < 32; i++)
  for (j = 0; j < 32; j++)
    for (ptr = game->map->points[i][j]; ptr != NULL; ptr = ptr->next)
      count++;

  printf("[dbg] saving %lu\n", count);
  assert(write(fd, &count, sizeof(unsigned long)) == sizeof(unsigned long));

 for (i = 0; i < 32; i++)
  for (j = 0; j < 32; j++)
    for (ptr = game->map->points[i][j]; ptr != NULL; ptr = ptr->next)
    {
      assert(write(fd, &ptr->x, sizeof(float)) == sizeof(float));
      assert(write(fd, &ptr->y, sizeof(float)) == sizeof(float));
      assert(write(fd, &ptr->enabled, sizeof(bool)) == sizeof(bool));
    }

  close(fd);
}

void points_load (map_t * map, int save_no)
{
  char save_path [0xFF];
  sprintf(save_path, "./DATA/sav/points%d.sav", save_no);

  #ifdef _WIN32
    int fd = open(save_path, O_CREAT | O_RDONLY | O_BINARY, 0600);
  #else
    int fd = open(save_path, O_CREAT | O_RDONLY, 0600);
  #endif

  unsigned long count = 0;
  assert(read(fd, &count, sizeof(unsigned long)) == sizeof(unsigned long));

  printf("Loading GAME %lu points\n", count);

  unsigned long i;
  for (i = 0; i < count; i++)
  {
    point_t * point = (point_t *) malloc(sizeof(struct point_s));
    __GUARD(point, "point_t node (loaded)");
    point->next = NULL;
    assert(read(fd, &point->x, sizeof(float)) == sizeof(float));
    assert(read(fd, &point->y, sizeof(float)) == sizeof(float));
    assert(read(fd, &point->enabled, sizeof(bool)) == sizeof(bool));

    long mx = (long) point->x >> 12;
    long my = (long) point->y >> 12;

    if (map->points[mx][my] == NULL)
      map->points[mx][my] = point;
    else
    {
      // append to the beginning of the list
      point->next = map->points[mx][my];
      map->points[mx][my] = point;
    }
    map->points_count++;
  }


  close (fd);
}