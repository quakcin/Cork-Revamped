rule:
	gcc -o CRK -s ./main.c ./i_math.c ./pm.c ./p_menu.c ./p_engine.c ./c_sched.c ./c_audio.c ./c_point.c ./c_player.c ./c_enemy.c ./c_map.c ./p_game.c ./p_render.c ./p_cutscene.c ./c_item.c ./c_gen.c -I/usr/include/allegro5 -L/usr/lib -lallegro -lm -lallegro_image -lallegro_ttf -lallegro_font -lallegro_audio -lallegro_primitives -lallegro_acodec -lallegro_video -Ofast
