/*
 * Entry point / initialization / main program loop / termination code
 *
 * Copyright (c) 2008, 2009, 2010 David Kelley
 * Copyright (c) 2009 Steve Lewis
 * Copyright (c) 2016 The Lemon Man
 * Copyright (c) 2021, 2022 Jeffrey H. Johnson <trnsz@pobox.com>
 *
 * This file is part of bviplusplus.
 *
 * Bviplusplus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bviplusplus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bviplusplus.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include <locale.h>
#include <ncurses.h>
#include <panel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "actions.h"
#include "app_state.h"
#include "creadline.h"
#include "display.h"
#include "key_handler.h"
#include "user_prefs.h"
#include "virt_file.h"

#define MILISECONDS(x) (( x ) * 1000 )
#define SECONDS(x)     ( MILISECONDS(x) * 1000 )

int
main(int argc, char **argv)
{
  int i, c;
  file_manager_t *tmp_head;

  setlocale(LC_NUMERIC, "");

  /* Create a file ring to contain any open file references for this process */
  file_ring = vf_create_fm_ring();

  /* Fill the file ring with files the user has listed on the command line */
  for (i = 1; i < argc; i++)
    {
      printf("argv[%d] = %s\n", i, argv[i]);
      current_file = vf_add_fm_to_ring(file_ring);
      if (vf_init(current_file, argv[i]) == FALSE)
        {
          fprintf(stderr, "Could not open %s\n", argv[i]);
          vf_remove_fm_from_ring(file_ring, current_file);
        }
    }

  /* Make sure we have at least 1 valid open file, otherwise init empty file */
  current_file = vf_get_current_fm_from_ring(file_ring);
  if (current_file == NULL) /* no file specified in open */
    {
      current_file = vf_add_fm_to_ring(file_ring);
      if (vf_init(current_file, NULL) == FALSE)
        {
          fprintf(stderr, "Empty file failed?\n");
        }
    }

  /* Initialize macros, yank, search, and command line history support */
  memset(macro_record, 0, sizeof ( macro_record_t ) * 26);
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
  raw();  /* use raw instead of cbreak for alt+<key> support */
  noecho();
  attrset(A_NORMAL);
  start_color();  /* Start color */
  use_default_colors();
  init_pair(1, COLOR_YELLOW, -1);  /* for blob_grouping */

  /* Read user rc file and set preferences */
  read_rc_file();

  reset_display_info();

  app_state.quit = FALSE;

/* print any debug here before we make our windows */
#ifdef DEBUG
  printw("COLS = %'ld\n", (long)COLS);
  printw("PRESS q to continue\n");
  while (( c = getch()) != 'q')
    {
      printw("PRESSED KEY = %x\n", c);
      refresh();
    }
#endif /* ifdef DEBUG */

  /* Create the program windows, set up the panels, and we're off */
  create_screen();

  /* Print our first screen with hex data */
  print_screen(display_info.page_start);

  /* Main program loop. We loop here until we are told to quit. */
  while (app_state.quit == FALSE)
    {
      /*
       * Update the status window each key press so we
       * can always see our current cursor address
       */
      update_status_window();
      update_panels();
      doupdate();
      /* Replace the cursor after updating the screen */
      place_cursor(display_info.cursor_addr, CALIGN_NONE, CURSOR_REAL);
      /* Get and handle the users next key press */
      c = mwgetch(window_list[display_info.cursor_window]);
      update_status(NULL);
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

  tmp_head = vf_get_head_fm_from_ring(file_ring);
  current_file = vf_get_head_fm_from_ring(file_ring);
  do
    {
      if (current_file == NULL)
        {
          break;
        }

      if (current_file->private_data != NULL)
        {
          free(current_file->private_data);
        }

      current_file = vf_get_next_fm_from_ring(file_ring);
    }
  while ( current_file != tmp_head );

  vf_destroy_fm_ring(file_ring);

  return 0;
}
