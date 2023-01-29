
#include <stdint.h>
#include <stdbool.h>

long get_random_long (long min, long max, long mul);
char get_random_char (char min, char max, char mul);
int get_random_int (int min, int max, int mul);
uint8_t get_random_uint8 (uint8_t min, uint8_t max, uint8_t mul);
float get_dist (float x, float y, float dx, float dy);
//long q_dist (long x, long y, long dx, long dy);
float q_dist (float x, float y, float dx, float dy);
static void i_math_init (void);
bool within_angle (float ang, float test, float limit);
int q_sqrt (int val);