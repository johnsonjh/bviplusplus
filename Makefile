default:
	gcc -g -O0 -lpanel -lmenu -lncurses main.c menus.c virt_file.c vf_backend.c key_handler.c actions.c display.c user_prefs.c app_state.c -o myhex
