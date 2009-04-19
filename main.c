#include <stdio.h>
#include <ncurses.h>
#include <panel.h>
#include <menu.h>
#include <unistd.h> /* usleep */
#include <stdlib.h> /* calloc */
#include <ctype.h> /* isprint */
#include "virt_file.h"
#include "menus.h"
#include "key_handler.h"
#include "display.h"
#include "app_state.h"

#define MILISECONDS(x) ((x) * 1000)
#define SECONDS(x) (MILISECONDS(x) * 1000)

int main(int argc, char **argv)
{
  int i, c;

  file_ring = vf_create_fm_ring();
  for (i=1; i<argc; i++)
  {
    printf("argv[%d] = %s\n", i, argv[i]);
    current_file = vf_add_fm_to_ring(file_ring);
    if (vf_init(current_file, argv[i]) == FALSE)
      fprintf(stderr, "Could not open %s\n", argv[i]);
  }
  current_file = vf_get_current_fm_from_ring(file_ring);
  if (current_file == NULL) /* no file specified in open */
  {
    current_file = vf_add_fm_to_ring(file_ring);
    if (vf_init(current_file, NULL) == FALSE)
      fprintf(stderr, "Empty file failed?\n");
  }

  action_init_yank();
  search_init();

  initscr();
  keypad(stdscr, TRUE);
  scrollok(stdscr, TRUE);
  nonl();
  //cbreak();
  noecho();
  raw();
  attrset(A_NORMAL);
  start_color();      /* Start color      */
  use_default_colors();
  init_pair(1, COLOR_YELLOW, -1);

  reset_display_info();

  app_state.quit = FALSE;

//#define SHOW_DEBUG_SCREEN
#ifdef SHOW_DEBUG_SCREEN
  printw("COLS = %d\n", COLS);
  printw("PRESS q to continue\n");
  while ((c = getch()) != 'q')
  {
    printw("PRESSED KEY = %x\n", c);
    refresh();
  }
#endif

  create_screen();
  print_screen(display_info.page_start);

  while (app_state.quit == FALSE)
  {
    update_status_window();
    update_panels();
    doupdate();
    place_cursor(display_info.cursor_addr, CALIGN_NONE, CURSOR_REAL);
    c = wgetch(window_list[display_info.cursor_window]);
    handle_key(c);
  }

  destroy_screen();
  endwin();

  search_cleanup();
  action_clean_yank();
  vf_destroy_fm_ring(file_ring);

  return 0;
}


#if 0
  create warning("") function to show highlighted warning on status line until next key press, but does not capture cursor like msg_box().

  search binary? (bit level)
  modify command line interface to be generic and make a library out of it
  Remember to add tab completion, macros, and a good system for command line parsing, .rc files

  search hl and visual hl interfere with eachother since they both use standout. Make a count for each kind of thing (standout/color/etc) and inc going on, dec going off. Only really turn off if count == 0

  Check bvi man page for min list of command line commands to support
  Handle KEY_RESIZE wherever we use looped getch for a while

#endif

