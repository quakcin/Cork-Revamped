

typedef struct player_s player_t;
typedef struct engine_s engine_t;
typedef struct audio_s audio_t;
typedef struct enemy_s enemy_t;
typedef struct sched_s sched_t;
typedef struct item_s item_t;
typedef struct proc_s proc_t;
typedef struct map_s map_t;
typedef struct viz_s viz_t;
typedef struct pm_s pm_t;

// this object helds everything
typedef struct game_s
{
  engine_t * engine;
  player_t * player;
  audio_t * audio;
  enemy_t * enemy;
  sched_t * sched;
  item_t * item;
  map_t * map;
  viz_t * viz;
  pm_t * pm;
} game_t;

void p_game_init (proc_t * self);
void p_game_update (proc_t * self);
void p_game_dtor (proc_t * self);
