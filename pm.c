/*
 *  Process Manager
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "pm.h"

pm_t * pm_ctor ()
{
  pm_t * pm = (pm_t *) malloc(sizeof(struct pm_s));
  __GUARD(pm, "pm_t obj");
  pm->state = PM_RUNNING;
  pm->proc_list = NULL;
  pm->last = NULL;
  pm->ticks = 0;
  return pm;
}

void pm_dtor (pm_t * self)
{
  // first destroy all of the processes
  proc_t * proc;
  proc_t * next;

  // destroy each process that is not engine
  for (proc = self->proc_list->next; proc != NULL; proc = next)
  {
    // execute dtor() in each process provided process is running FIXME: or halted!
    if (proc->state == P_RUNNING || proc->state == P_HOLD)
      proc->dtor(proc);

    // remember next process in list
    next = proc->next;
    // free current process

    __UNGUARD(proc, "proc_t obj");
    free(proc);
  }

  // destroy engine
  self->proc_list->dtor(self->proc_list);

  // destroy pm's allocation
  __UNGUARD(self, "pm_t obj");
  free(self);
}

int pm_push 
(
  pm_t * self, char * name,
  void (*init)( struct proc_s * self ),
  void (*update)( struct proc_s * self ),
  void (*dtor)( struct proc_s * self )
)
{
  // create process husk
  proc_t * proc = (proc_t *) malloc(sizeof(struct proc_s));
  __GUARD(proc, "proc_t obj");

  if (proc == NULL)
    return 1;

  // setup process informations
  strcpy(proc->name, name);
  proc->state = P_STOPPED;
  proc->update = update;
  proc->init = init;
  proc->dtor = dtor;
  proc->next = NULL;
  proc->mem = NULL;
  proc->pm = self;

  if (self->proc_list == NULL)
  {
    self->proc_list = proc;
    self->last = proc;
  }
  else
  {
    self->last->next = proc;
    self->last = proc;
  }

  return 0;
}

void pm_update (pm_t * self)
{
  proc_t * proc;
  for (proc = self->proc_list; proc != NULL; proc = proc->next)
  {
    // printf("Updating %s %p with status %d\n", proc->name, proc, (int) proc->state);
    switch (proc->state)
    {
      case P_RUNNING:
        proc->update(proc);
        break;
      
      case P_RISING:
        proc->init(proc);
        proc->state = P_RUNNING;
        break;
      
      case P_STOPPING:
        proc->dtor(proc);
        proc->state = P_STOPPED;
        break;

      case P_UNHLDING:
        proc->state = P_RUNNING;
        break;
      /* no default, might need f-no-crossjumping or sth 4 gcc */
    }
  }
  self->ticks++;
}

void pm_run (pm_t * self, char * name)
{
  proc_t * proc;
  for (proc = self->proc_list; proc != NULL; proc = proc->next)
    if (strcmp(proc->name, name) == 0)
    {
      if (proc->state == P_RUNNING)
        IERROR("Attempting to rerise running process!");
      else
      {
        proc->state = P_RISING;
        return;
      }
    }
  IPRINT("Failed to rise process!");
}

void pm_stop (pm_t * self, char * name)
{
  proc_t * proc;
  for (proc = self->proc_list; proc != NULL; proc = proc->next)
    if (strcmp(proc->name, name) == 0)
    {
      if (proc->state == P_STOPPED)
        IERROR("Attempting to stop dead process");
      else
        proc->state = P_STOPPING;
    }
}


void pm_hold (pm_t * self, char * name)
{
  proc_t * proc;
  for (proc = self->proc_list; proc != NULL; proc = proc->next)
    if (strcmp(proc->name, name) == 0)
      proc->state = P_HOLD;
}

void pm_release (pm_t * self, char * name)
{
  proc_t * proc;
  for (proc = self->proc_list; proc != NULL; proc = proc->next)
    if (strcmp(proc->name, name) == 0)
    {
      if (proc->state == P_HOLD)
        proc->state = P_UNHLDING;
      else
        IERROR("[pm] process isn't held back");
    }
}

proc_t * pm_get (pm_t * self, char * name)
{
  proc_t * proc;
  for (proc = self->proc_list; proc != NULL; proc = proc->next)
    if (strcmp(proc->name, name) == 0)
      return proc;
  return NULL; // tragedy
}