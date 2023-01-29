
#define P_STOPPED  0  // process isn't running at all
#define P_STOPPING 1  // process will be stopped on next tick
#define P_RUNNING  2  // process is running allready
#define P_RISING   3  // process will be running on the next tick
#define P_HOLD     4  // process is being held down (not updated)
#define P_UNHLDING 5  // process is being released

#define PM_RUNNING 0
#define PM_STOPPED 1

typedef struct pm_s * pm_s;

typedef struct proc_s
{
  struct proc_s * next;
  struct pm_s * pm;

  // process handlers
  void (*init)( struct proc_s * self );
  void (*update)( struct proc_s * self );
  void (*dtor)( struct proc_s * self );

  void * mem;

  char name[0xF];
  char state;
} proc_t;

typedef struct pm_s
{
  struct proc_s * proc_list;
  struct proc_s * last;
  unsigned long long ticks;
  char state;
} pm_t;

// externs
pm_t * pm_ctor ();
void pm_dtor (pm_t * self);
void pm_update (pm_t * self);
void pm_run (pm_t * self, char * name);
void pm_hold (pm_t * self, char * name);
void pm_stop (pm_t * self, char * name);
void pm_release (pm_t * self, char * name);
proc_t * pm_get (pm_t * self, char * name);

int pm_push 
(
  pm_t * self, char * name,
  void (*init)( struct proc_s * self ),
  void (*update)( struct proc_s * self ),
  void (*dtor)( struct proc_s * self )
);