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
  cbreak();
  noecho();
  attrset(A_NORMAL);
  start_color();      /* Start color      */
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);

  reset_display_info();

  app_state.quit = FALSE;

//#define SHOW_DEBUG_SCREEN
#ifdef SHOW_DEBUG_SCREEN
  printw("COLS = %d\n", COLS);
  printw("SHARED_WIDTH = %d\n", SHARED_WIDTH);
  printw("BYTES_PER_GROUP = %d\n", BYTES_PER_GROUP);
  printw("a = (SHARED_WIDTH * BYTES_PER_GROUP) = %d\n", (SHARED_WIDTH * BYTES_PER_GROUP));
  printw("b = ((3 * BYTES_PER_GROUP) + 1) = %d\n", ((3 * BYTES_PER_GROUP) + 1));
  printw("a/b = HEX_BOX_W = %d\n", HEX_BOX_W);
  printw("PAGE_SIZE = %d\n", PAGE_SIZE);
  printw("_PAGE_END = %d\n", _PAGE_END);
  printw("PAGE_END = %d\n", PAGE_END);
  printw("PRESS ANY KEY TO CONTINUE\n", HEX_BOX_W);
  getch();
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
  status bar should indicate display options (little endian?, grouping?), #open buffers, repeat count?

  create warning("") function to show highlighted warning on status line until next key press, but does not capture cursor like msg_box(). Use this for things like soft quit with changes, that way user doesnt have to press a key specifically to clear the warning 

  create a warning/prompt on hard quit when an offscreen file has changes?

  search binary? (bit level)
  make more flexible output for data returned from external programs
  use readline library instead of custom cmd line and file save dialogue?

  search hl and visual hl interfere with eachother since they both use standout. Make a count for each kind of thing (standout/color/etc) and inc going on, dec going off. Only really turn off if count == 0

   Remember to add tab completion, macros, and a good system for command line parsing, .rc files
   Check bvi man page for min list of command line commands to support
   Handle KEY_RESIZE wherever we use looped getch for a while



File        Edit                       Tabs     Help
  Open       Copy                        Next
  Save       Paste                       Prev
  Save As    Visual Select
  Close      Cut
  Exit       Delete
             Run Script on Selected
             Preferences
             View Clipboard
#endif

