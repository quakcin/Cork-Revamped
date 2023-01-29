
#include "c_sched.h"
#include "p_engine.h"
#include "main.h"

#include <stdlib.h>
#include <stdio.h>


sched_t * sched_ctor (void)
{
  sched_t * self = (sched_t *) malloc(sizeof(struct sched_s));
  __GUARD(self, "sched_t obj");
  // self->tasks = NULL;
  self->align = 0;
  self->syncro = time(NULL) + 1;

  int i;
  for (i = 0; i < SCHED_SIZE; i++)
    self->busy[i] = false;

  return self;
}

void sched_dtor (sched_t * self)
{
  // task_t * ptr;
  // task_t * nxt;
  __UNGUARD(self, "sched_t obj");
  free(self);
}

void sched_update (sched_t * self, engine_t * engine)
{
  if (time(NULL) < self->syncro)
    return;

  self->align = 0;
  int throthle = 0;

  int i;
  for (i = 0; i < SCHED_SIZE; i++)
  {
    if (self->busy[i] != true)
      continue;

    task_t * ptr = &self->tasks[i];

    ptr->delay -= 1 / engine->delta; 

    if (ptr->delay <= 0)
    {
      ptr->funct(self, ptr->ptr, ptr->val);
      self->busy[i] = false;
      throthle++;
      if (throthle == SCHED_MAX_THROTHLE)
        return;
    }
  }
}

void yield (sched_t * self, void (*funct)( sched_t * sched, void * ptr, int val ), void * ptr, int val, float delay )
{
  // find any free spot
  int i;
  for (i = 0; i < SCHED_SIZE; i++)
    if (self->busy[i] == false)
      break;

  if (i >= SCHED_SIZE)
  {
    printf("[sched] out of space exception\n");
    return;
  }

  task_t * task = &self->tasks[i];

  task->delay = delay + self->align++;
  task->funct = funct;
  task->ptr = ptr;
  task->val = val;

  self->busy[i] = true; // make it happen
}


