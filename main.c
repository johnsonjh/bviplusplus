#include <stdio.h>
#include <ncurses.h>
#include <panel.h>
#include <menu.h>
#include <unistd.h> /* usleep */
#include <stdlib.h> /* calloc */
#include <ctype.h> /* isprint */
#include "virt_file.h"
#include "menus.h"
#include "windows.h"
#include "key_handler.h"
#include "display.h"
#include "user_prefs.h"
#include "app_state.h"

#define MILISECONDS(x) ((x) * 1000)
#define SECONDS(x) (MILISECONDS(x) * 1000)

int main(int argc, char **argv)
{
  int i, c;

  vf_init(&file_manager, "test.bin");
  vf_stat(&file_manager, &vfstat);

  initscr();
  keypad(stdscr, TRUE);
  scrollok(stdscr, TRUE);
  nonl();
  cbreak();
  noecho();
  attrset(A_NORMAL);

  app_state.quit = FALSE;

  user_prefs.display_binary = FALSE;
  user_prefs.little_endian = FALSE;
  user_prefs.grouping = 1;
  user_prefs.grouping_offset = 0;
  user_prefs.blob_grouping = 20;
  user_prefs.blob_grouping_offset = 0;

  display_info.file_size = vfstat.file_size;
  display_info.page_start = 0;
  display_info.page_end = (PAGE_END > display_info.file_size ? display_info.file_size : PAGE_END);
  display_info.cursor_addr = 0;
  display_info.cursor_window = WINDOW_HEX;
  display_info.max_cols = 0;
  display_info.has_color = has_colors();

  start_color();      /* Start color      */
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);

#ifdef SHOW_DEBUG_SCREEN
  printw("COLS = %d\n", COLS);
  printw("SHARED_WIDTH = %d\n", SHARED_WIDTH);
  printw("BYTES_PER_GROUP = %d\n", BYTES_PER_GROUP);
  printw("a = (SHARED_WIDTH * BYTES_PER_GROUP) = %d\n", (SHARED_WIDTH * BYTES_PER_GROUP));
  printw("b = ((3 * BYTES_PER_GROUP) + 1) = %d\n", ((3 * BYTES_PER_GROUP) + 1));
  printw("a/b = HEX_BOX_W = %d\n", HEX_BOX_W);
  printw("PRESS ANY KEY TO CONTINUE\n", HEX_BOX_W);
  getch();
#endif

  create_screen();
  print_screen(display_info.page_start);
  place_cursor(display_info.page_start);

  while (app_state.quit == FALSE)
  {
    int x, y;
    int xx, yy;
    off_t a;

    update_panels();
    doupdate();
    wmove(window_list[WINDOW_HEX], y, x);
    c = wgetch(window_list[WINDOW_HEX]); /* wgetch =( */
    if (c == KEY_RESIZE)
    {
      destroy_screen();
      create_screen();
      getyx(window_list[WINDOW_HEX], y, x);
      mvwprintw(window_list[WINDOW_MENU], 0, 0, "%x      KEY_RESIZE");
      print_screen(display_info.page_start);
    }
    else
    {
      handle_key(c);
    }
    getyx(window_list[WINDOW_HEX], y, x);
    a = get_addr_from_xy(x, y);
    xx = get_x_from_addr(a);
    yy = get_y_from_addr(a);

    werase(window_list[WINDOW_MENU]);
    getyx(window_list[WINDOW_HEX], y, x);
    mvwprintw(window_list[WINDOW_MENU], 0, 0, "%x  a = %x, x = %d, y = %d", c, a, xx, yy);
  }

  destroy_screen();
  endwin();
  vf_term(&file_manager);
  return 0;
}


#if 0
  impliment printline and grouping start offset.... if start is 0 and line is 0 first line contains start offset bytes at end of line, then start normal on line 1
  //if a blob (rename to group) goes over a line what do do? ...  don't let grouping exceed 128bit [grouping = 16], instead impliment alternating color scheme with
  also alt color offset, allow truly arbitrary, but does not affect size of windows

  little endian is broken (drops a byte), also even if in little endian should ascii order change?
  limit 'grouping' to 1,2,4,8,16 (8 bit, 16 bit, 32 bit, 64 bit, 128 bit?)... i think so

  impliment minimum window widths based on grouping... do not resize smaller than these


search binary (bit level), hex (nibble level), ascii
run arbitrary scripts on visually selected areas (checksum, parsing, etc)
File tabs
Uses my file system (with buffering?)


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
