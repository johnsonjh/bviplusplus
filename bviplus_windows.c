#include "windows.h"
#include "display.h"
#include "user_prefs.h"

WINDOW *window_list[MAX_WINDOWS];
PANEL *panel_list[MAX_WINDOWS];

int get_y_from_addr(off_t addr)
{
  off_t offset, page_size;
  int y = 1, z;

  if (addr < display_info.page_start)
    return -1;

  offset = addr - display_info.page_start;
  page_size = display_info.page_end - display_info.page_start;

  if (offset < user_prefs.grouping_offset)
    return 1; /* valid - first line */

  if (offset > page_size)
    return -1;

  if (display_info.page_start < user_prefs.grouping_offset)
  {
    offset -= user_prefs.grouping_offset;
    y++;
  }

  z = (HEX_COLS * user_prefs.grouping);
  y += offset / z;

  return y;
}

int get_x_from_addr(off_t addr)
{
  off_t offset;
  int x, y;
  y = get_y_from_addr(addr);

  if (y < 0)
    return -1;

  y--;

  offset = addr - display_info.page_start;
  offset -= y * HEX_COLS * user_prefs.grouping;

  x = 1 + (offset / user_prefs.grouping) * BYTES_PER_GROUP;

  return x;

}

off_t get_addr_from_xy(int x, int y)
{
  off_t addr = display_info.page_start;

  if (y < 1 || y > (HEX_BOX_H - 1))
    return -1;

  if (x < 1 || x > (HEX_BOX_W - 1))
    return -1;

  y--; /* top border */
  x--; /* left border */

  if (display_info.page_start < user_prefs.grouping_offset)
  {
    if (y)
    {
      addr += user_prefs.grouping_offset;
      y--;
    }
    else
    {
      /* only one valid address on the first line if there is a grouping offset */
      return display_info.page_start;
    }
  }

  addr += y * HEX_COLS * user_prefs.grouping;
  addr += (x / BYTES_PER_GROUP) * user_prefs.grouping;

  if (addr > display_info.page_end)
    return -1;

  return addr;
}

void destroy_screen()
{
  delwin(window_list[WINDOW_MENU]);
  delwin(window_list[WINDOW_ADDR]);
  delwin(window_list[WINDOW_HEX]);
  delwin(window_list[WINDOW_ASCII]);
}

void create_screen()
{
  window_list[WINDOW_MENU]  = newwin( MENU_BOX_H,  MENU_BOX_W,  MENU_BOX_Y,  MENU_BOX_X);
  window_list[WINDOW_ADDR]  = newwin( ADDR_BOX_H,  ADDR_BOX_W,  ADDR_BOX_Y,  ADDR_BOX_X);
  window_list[WINDOW_HEX]   = newwin(  HEX_BOX_H,   HEX_BOX_W,   HEX_BOX_Y,   HEX_BOX_X);
  window_list[WINDOW_ASCII] = newwin(ASCII_BOX_H, ASCII_BOX_W, ASCII_BOX_Y, ASCII_BOX_X);

  panel_list[WINDOW_MENU]  = new_panel(window_list[WINDOW_MENU]);
  panel_list[WINDOW_ADDR]  = new_panel(window_list[WINDOW_ADDR]);
  panel_list[WINDOW_HEX]   = new_panel(window_list[WINDOW_HEX]);
  panel_list[WINDOW_ASCII] = new_panel(window_list[WINDOW_ASCII]);

  keypad(window_list[WINDOW_MENU], TRUE);
  keypad(window_list[WINDOW_HEX], TRUE);
  keypad(window_list[WINDOW_ASCII], TRUE);

  box(window_list[WINDOW_HEX], 0, 0);
  box(window_list[WINDOW_ASCII], 0, 0);
}


