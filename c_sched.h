

// fn dupa (sched_t * sched, void * ptr, int val)
// {}
#include <stdbool.h>

#define SCHED_SIZE 64

// prevent scheduler from stealling
// all of the processing power
#define SCHED_MAX_THROTHLE 3

typedef struct sched_s sched_t;
typedef struct engine_s engine_t;

typedef struct task_s
{
  // struct task_s * next;
  void (*funct)( sched_t * sched, void * ptr, int val );
  void * ptr;
  int val;
  float delay;
} task_t;

typedef struct sched_s
{
  long syncro;
  task_t tasks [SCHED_SIZE];
  bool busy [SCHED_SIZE];
  // task_t * tasks;
  int align;
} sched_t;

sched_t * sched_ctor (void);
void sched_dtor (sched_t * self);
void sched_update (sched_t * self, engine_t * engine);
void yield (sched_t * self, void (*funct)( sched_t * sched, void * ptr, int val ), void * ptr, int val, float delay);