/*
  Map generator -- direct 1 to 1 
  translation from JS prototype
*/

#include <stdio.h>

#include "c_gen.h"
#include "c_map.h"
#include "p_game.h"
#include "i_math.h"

void js_horizontall_wall_randomizer (uint8_t map[511][511])
{
  int xoff = 0;
  int yoff = 0;
  int cx = 0;
  int cy = 0;

  int x;
  for (x = 31; x < 500; x += 12)
  {
    cx = x - get_random_int(0, 16, 1);
    cy = 0;

    while (cy < 490)
    {
      yoff = cy + get_random_int(3, 12, 1);
      while (cy < yoff)
      {
        // console.log(cx, cy);
        map[cx][cy] = 9;
        map[cx + 1][cy] = 9;
        cy++;
      }
      xoff = x - get_random_int(0, 12, 1);
      const int xdel = cx < xoff ? 1 : -1;
      while (cx != xoff)
      {
        map[cx][cy] = 9;
        map[cx][cy + 1] = 9;
        cx += xdel;
      }
    }
  }
}


void js_make_doors (uint8_t map[511][511])
{
  // raster scan
  int x, y;

  for (y = 0; y < 511; y += get_random_int(3, 8, 1))
  {
    bool skip = true;

    for (x = 0; x < 511 - 4; x++)
    {
      if (map[x + 2][y - 1] != 0 && map[x + 2][y + 1] != 0 && map[x + 1][y - 1] != 0 && map[x + 1][y + 1] != 0 && map[x + 0][y] == 0 && map[x + 1][y] != 0 && map[x + 2][y] != 0 && map[x + 3][y] == 0  && map[x + 1][y - 1] != 0 && map[x + 1][y + 1] != 0 && map[x + 2][y - 1] != 0 && map[x + 2][y - 1] != 0)
      {
        if (skip)
        {
          skip = false;
          x += 3;
          continue;
        }
        const int door_type = get_random_int(5, 8, 1);
        // map[x][y] = randInt(1, 4);
        map[x + 1][y] = door_type;
        map[x + 2][y] = door_type;
        // map[x + 3][y] = randInt(1, 4);
        x += 3; // really good optimalization i bet
        if (get_random_int(0, 100, 1) < 50)
          skip = true;
      }
    }
  }
}

void js_gen_props (uint8_t map[511][511])
{
  int z;
  for (z = 0; z < 50000; z++)
  {
    const int x = get_random_int(10, 500, 1);
    const int y = get_random_int(10, 500, 1);

    if (x == 255 && y == 255)
      continue;

    // can spawn?

    bool spawn = true;
    int i;
    for (i = x - 1; i <= x + 1; i++)
    {
      int j;
      for (j = y - 1; j <= y + 1; j++)
        if (map[i][j] != 0)
        {
          spawn = false;
          break;
        }
    }

    if (spawn)
    {
      const int ch = get_random_int(1, 19, 1);
      if (ch < 15) // 15 from <21, 36> and 4 <41, 44>
        map[x][y] = get_random_int(21, 36, 1);
      else
        map[x][y] = get_random_int(41, 44, 1);
    }
  }
}

void js_map_generator (uint8_t map[511][511])
{
  js_horizontall_wall_randomizer(map);

  int doors[4096][2]; /* fails sometimes! */
  int doors_ptr = 0;
  int x, y, i, j;

  for (y = 0; y < 504; y += 16)
  {
    int nextDoorCheck = 0;
    for (x = 0; x < 511; x++)
    {
      int type = 9;
      if (x == nextDoorCheck)
      {
        if (map[x][y - 1] == 0 && map[x][y + 2] == 0)
        {
          type = 4;
          // doors.push([x, y]);
          doors[doors_ptr][0] = x;
          doors[doors_ptr][1] = y;
          doors_ptr++;
        }
        nextDoorCheck += get_random_int(6, 14, 1);
      }
      map[x][y] = type;
      map[x][y + 1] = type;
    }
  }

  for (i = 0; i < doors_ptr; i = i + 1)
  {
    // static walls only
    // const floor_color = randInt(1, 4);
    const int col = get_random_int(10, 16, 1);
    const int orn = (col <= 12) ? col : 17 + (col - 13) % 4;
    const int lampning = get_random_int(1, 3, 1) * 2 + 1;
    const int winding = get_random_int(3, 5, 1);
    // let markedlamps = [];
    int markedlamps[1024][2];
    int markedlamps_ptr = 0;

    for (y = doors[i][1] + 2; y < doors[i][1] + 32; y += 1)
    {
      // check
      if (y >= 510)
        break;

      if (map[doors[i][0]][y] != 0)
      {
        map[doors[i][0]][y] = col;
        break;
      }

      // to left
      for (x = doors[i][0] - 1; x > doors[i][0] - 32; x--)
      {
        if (x <= 10)
          break;

        if (map[x][y] != 0)
        {
          // map[x][y] = col;
          map[x][y] = (y % winding == 0) ? orn : col;
          break;
        }
        
        if (map[x][y - 1] != 0)
          map[x][y - 1] = col;
          
        if (map[x][y + 1] != 0)
          map[x][y + 1] = col;

        // if (map[x][y] == 0)
        //   map[x][y] = floor_color;

        if (x % lampning == 0 && y % lampning == 0)
        {
          markedlamps[markedlamps_ptr][0] = x;
          markedlamps[markedlamps_ptr][1] = y;
          markedlamps_ptr++;
        }
      }

      // to right
      for (x = doors[i][0]; x < doors[i][0] + 32; x++)
      {
        if (x >= 510)
          break;

        if (map[x][y] != 0)
        {
          map[x][y] = (y % winding == 0) ? orn : col;
          break;
        }

        if (map[x][y - 1] != 0)
          map[x][y - 1] = col;
          
        if (map[x][y + 1] != 0)
          map[x][y + 1] = col;

        // if (map[x][y] == 0)
        //   map[x][y] = floor_color;

        if (x % lampning == 0 && y % lampning == 0)
        {
          markedlamps[markedlamps_ptr][0] = x;
          markedlamps[markedlamps_ptr][1] = y;
          markedlamps_ptr++;
        }
      }
      // map[doors[i][0]][y] = 3;
    } // {for each doors}

    const int lamptype = get_random_int(37, 40, 1);
    // int j;
    for (j = 0; j < markedlamps_ptr; j++)
    {
      map[markedlamps[j][0]][markedlamps[j][1]] = lamptype;
    }
  }


  for (i = 0; i < doors_ptr; i++)
  {
    map[doors[i][0]][doors[i][1]] = get_random_int(5, 8, 1);
    map[doors[i][0]][doors[i][1] + 1] = get_random_int(5, 8, 1);
  }

  js_make_doors(map);
  js_gen_props(map);

  // let player out
  for (y = 255; y > 255 - 16; y--)
  {
    map[255][y] = 0;

    if (map[255 - 1][y] < MAP_DOOR_END && map[255 - 1][y] > MAP_DOOR_START)
      map[255 - 1][y] = 9;

    if (map[255 + 1][y] < MAP_DOOR_END && map[255 + 1][y] > MAP_DOOR_START)
      map[255 + 1][y] = 9;
  }

  // secure far lands
  for (i = 12; i < 511 - 12; i++)
  {
    map[i][12] = 9;
    map[i][511 - 12] = 9;
    map[12][i] = 9;
    map[511 - 12][i] = 9;
  }
}

void generate_map (map_t * map)
{
  memset(map->tiles, 0, sizeof(map->tiles));
  js_map_generator(map->tiles);
}