
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "i_math.h"
#include <math.h>
#include "main.h"


// call this asap
static void i_math_init (void)
{
  srand(time(NULL));
}

long get_random_long (long min, long max, long mul)
{
  return ((long) rand() % (max - min + 1) + min) * mul;
}

char get_random_char (char min, char max, char mul)
{
  return ((char) rand() % (max - min + 1) + min) * mul;
}

int get_random_int (int min, int max, int mul)
{
  return ((int) rand() % (max - min + 1) + min) * mul;
}

uint8_t get_random_uint8 (uint8_t min, uint8_t max, uint8_t mul)
{
  return ((uint8_t) rand() % (max - min + 1) + min) * mul;
}

float q_dist (float x, float y, float dx, float dy)
{
  const float d1 = fabs(x - dx) * 1.41421356237;
  const float d2 = fabs(y - dy) * 1.41421356237;
  const float k = fabs(d1 - d2) * 0.125;
  return ((d1 + d2) * 0.5) + k;
}

float get_dist (float x, float y, float dx, float dy)
{
  return sqrt(pow(x - dx, 2) + pow(y - dy, 2));
}

/*
  limit must not exceed 360deg
  probably not even 180deg
*/
bool within_angle (float ang, float test, float limit)
{
  const float a = fmod(ang, PI2);
  const float t = fmod(test, PI2);

  if (a + limit > PI2)
  {
    // <0, n1> || <n2, PI2>
    return t < fmod(a + limit, PI2) || t > (a - limit);
  }
  else
  {
    // |A - T| < L
    return fabs(a - t) < limit;
  }
}


int q_sqrt (int n) // using Newton-Raphson method
{
  return sqrt(n);
    // int a, b;

    // if (n < 2) 
    //   return n;

    // a = 0x4E7;

    // b = n / a; 
    // a = (a + b) >> 1;
    // b = n / a; 
    // a = (a + b) >> 1;
    // b = n / a; 
    // a = (a + b) >> 1;
    // b = n / a;  // 3 iterations is enaugh
    // a = (a + b) >> 1; // but whatever

    // return a;
}