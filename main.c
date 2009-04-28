/*************************************************************
 *
 * File:        main.c
 * Author:      David Kelley
 * Description: Program entry point. Contains initialization,
 *              main program loop, and termination code.
 *
 *************************************************************/

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
#include "actions.h"
#include "creadline.h"

#define MILISECONDS(x) ((x) * 1000)
#define SECONDS(x) (MILISECONDS(x) * 1000)

int main(int argc, char **argv)
{
  int i, c;

  /* Create a file ring to contain any open file references for this process */
  file_ring = vf_create_fm_ring();

  /* Fill the file ring with the files the user has listed on the command line */
  for (i=1; i<argc; i++)
  {
    printf("argv[%d] = %s\n", i, argv[i]);
    current_file = vf_add_fm_to_ring(file_ring);
    if (vf_init(current_file, argv[i]) == FALSE)
      fprintf(stderr, "Could not open %s\n", argv[i]);
  }

  /* Make sure we have at least one valid open file, otherwise init an empty file */
  current_file = vf_get_current_fm_from_ring(file_ring);
  if (current_file == NULL) /* no file specified in open */
  {
    current_file = vf_add_fm_to_ring(file_ring);
    if (vf_init(current_file, NULL) == FALSE)
      fprintf(stderr, "Empty file failed?\n");
  }

  /* Initialize yank, search, and comand line history support */
  action_init_yank();
  search_init();
  ascii_search_hist = new_history();
  hex_search_hist = new_history();
  cmd_hist = new_history();
  file_hist = new_history();

  /* Get our ncurses screen ready */
  initscr();
  keypad(stdscr, TRUE);
  scrollok(stdscr, TRUE);
  nonl();
  //cbreak();
  raw(); /* use raw instead of cbreak for alt+<key> support */
  noecho();
  attrset(A_NORMAL);
  start_color();                  /* Start color */
  use_default_colors();
  init_pair(1, COLOR_YELLOW, -1); /* for blob_grouping */

  reset_display_info();

  app_state.quit = FALSE;

/* print any debug here before we make our windows */
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

  /* Create the program windows, set up the pannels, and we're off */
  create_screen();

  /* Print our first screen with hex data */
  print_screen(display_info.page_start);

  /* Main program loop. We loop here until we are told to quit. */
  while (app_state.quit == FALSE)
  {
    /* Update the status window each keypress so we can always see our current cursor address */
    update_status_window();
    update_panels();
    doupdate();
    /* Replace the cursor after updating the screen */
    place_cursor(display_info.cursor_addr, CALIGN_NONE, CURSOR_REAL);
    /* Get and handle the users next key press */
    c = wgetch(window_list[display_info.cursor_window]);
    handle_key(c);
  }

  /* We're done, start breaking things down */
  destroy_screen();
  endwin();
  free_history(ascii_search_hist);
  free_history(hex_search_hist);
  free_history(cmd_hist);
  free_history(file_hist);
  search_cleanup();
  action_clean_yank();
  vf_destroy_fm_ring(file_ring);

  return 0;
}

