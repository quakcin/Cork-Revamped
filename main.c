#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "pm.h"

#include "p_engine.h"
#include "p_game.h"
#include "p_render.h"
#include "p_menu.h"

#include "p_cutscene.h"

void IERROR (char * msg)
{
	printf("[ERROR] %s!\n", msg);
}

void IPRINT (char * msg)
{
	printf("[LOG] %s!\n", msg);
}
void spawn_processes (pm_t * pm)
{
	int errlv = 0;
	errlv |= pm_push(pm, "engine", 		p_engine_init, 		p_engine_update, 		p_engine_dtor		);
	errlv |= pm_push(pm, "render", 		p_render_init, 		p_render_update, 		p_render_dtor		);
	errlv |= pm_push(pm, "game",   		p_game_init,   		p_game_update,   		p_game_dtor  		);
	errlv |= pm_push(pm, "menu",   		p_menu_init,   		p_menu_update,   		p_menu_dtor  		);
	errlv |= pm_push(pm, "cutscene", 	p_cutscene_init, 	p_cutscene_update, 	p_cutscene_dtor );

	if (errlv != 0)
		IERROR("Failed to push certain processes!");
}

void main_loop (pm_t * pm)
{
	while (pm->state == PM_RUNNING)
	{
		pm_update(pm);
	}
}

int main (int argc, char * argv[])
{
	// create process manager
	srand(time(NULL));
	pm_t * pm = pm_ctor();

	if (pm == NULL)
	{
		IERROR("pm did not allocate");
		return 1;
	}

	// create processes list
	spawn_processes(pm);

	// run engine process
	pm_run(pm, "engine");

	// enter main loop
	main_loop(pm);

	// cleanup memory and exit
	pm_dtor(pm);
	return 0;
}

