default:
	gcc -Wall -D_FILE_OFFSET_BITS=64 -g -O0 -lpanel -lncurses -lpthread main.c virt_file.c vf_backend.c key_handler.c actions.c display.c user_prefs.c app_state.c search.c help.c creadline.c -o bviplus
