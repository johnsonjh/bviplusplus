#include "display.h"
#include "user_prefs.h"
#include "app_state.h"

display_info_t display_info;

void search_hl(BOOL on)
{
  if (on)
  {
    wattron(window_list[WINDOW_HEX], A_STANDOUT);
    wattron(window_list[WINDOW_ASCII], A_STANDOUT);
  }
  else
  {
    wattroff(window_list[WINDOW_HEX], A_STANDOUT);
    wattroff(window_list[WINDOW_ASCII], A_STANDOUT);
  }
}

void blob_standout(BOOL on)
{
  if (display_info.has_color)
  {
    if (on)
    {
      wattron(window_list[WINDOW_HEX], COLOR_PAIR(1));
      wattron(window_list[WINDOW_ASCII], COLOR_PAIR(1));
    }
    else
    {
      wattroff(window_list[WINDOW_HEX], COLOR_PAIR(1));
      wattroff(window_list[WINDOW_ASCII], COLOR_PAIR(1));
    }
  }
  else
  {
    if (on)
    {
      wattron(window_list[WINDOW_HEX], A_BOLD);
      wattron(window_list[WINDOW_ASCII], A_BOLD);
    }
    else
    {
      wattroff(window_list[WINDOW_HEX], A_BOLD);
      wattroff(window_list[WINDOW_ASCII], A_BOLD);
    }
  }
}

/* returns the number of bytes displayed on that line */
int print_line(off_t addr, int y)
{
  int i, j, k,
      x = 1,
      byte_addr;
  char c, result,
       addr_text[ADDR_DIGITS + 1],
       bin_text[9];

  if (address_invalid(addr))
    return 0;

  y++; /* line 0 is the box border */

  if (addr < user_prefs[GROUPING_OFFSET].current_value)
  {
    /* print blank address */
    sprintf(addr_text, "        ");
    mvwaddstr(window_list[WINDOW_ADDR], y, 1, addr_text);

    /* find start of print area */
    x = ((HEX_COLS) * BYTES_PER_GROUP) - (user_prefs[GROUPING_OFFSET].current_value * BYTE_DIGITS);

    /* erase blank run in */
    for (i=1; i<x; i++)
    {
      mvwaddch(window_list[WINDOW_HEX], y, i, ' ');
      mvwaddch(window_list[WINDOW_ASCII], y, 1 + (i/2) + (i/user_prefs[GROUPING].current_value), ' ');
    }

    /* print dangling group */
    for (i=0; i<user_prefs[GROUPING_OFFSET].current_value; i++)
    {
      if (user_prefs[LIL_ENDIAN].current_value)
        byte_addr = addr - 1 + ((BYTES_PER_GROUP - (user_prefs[GROUPING_OFFSET].current_value * BYTE_DIGITS)) - i);
      else
        byte_addr = addr + i;

      if (address_invalid(byte_addr))
        break;

      c = vf_get_char(&file_manager, &result, byte_addr);

      if (user_prefs[BLOB_GROUPING_OFFSET].current_value > byte_addr)
      {
        blob_standout(TRUE);
      }
      else
      {
        if (user_prefs[BLOB_GROUPING].current_value &&
           (((byte_addr - user_prefs[BLOB_GROUPING_OFFSET].current_value) / user_prefs[BLOB_GROUPING].current_value) & 1))
        {
          blob_standout(TRUE);
        }
      }

      if (user_prefs[DISPLAY_BINARY].current_value)
      {
        snprintf(bin_text, 8, "%b", c);
        for (k=0; k<8; k++)
        {
          if ((c >> (7 - k)) & 1)
            mvwaddch(window_list[WINDOW_HEX], y, x, '1');
          else
            mvwaddch(window_list[WINDOW_HEX], y, x, '0');
          x++;
        }
      }
      else
      {
        mvwaddch(window_list[WINDOW_HEX], y, x, HEX(c>>4&0xF));
        x++;
        mvwaddch(window_list[WINDOW_HEX], y, x, HEX(c>>0&0xF));
        x++;
      }

      if (!isprint(c))
        c = '.';

      if (user_prefs[LIL_ENDIAN].current_value)
        mvwaddch(window_list[WINDOW_ASCII], y, 1-i+((HEX_COLS+1) * user_prefs[GROUPING].current_value - user_prefs[GROUPING_OFFSET].current_value), c);
      else
        mvwaddch(window_list[WINDOW_ASCII], y, 1+i+(HEX_COLS * user_prefs[GROUPING].current_value - user_prefs[GROUPING_OFFSET].current_value), c);

      blob_standout(FALSE);

    }
    return i;
  }

  for (i=0; i<HEX_COLS; i++)
  {
    /* print address */
    sprintf(addr_text, "%08X", addr);
    mvwaddstr(window_list[WINDOW_ADDR], y, 1, addr_text);

    /* print hex and ascii */
    for (j=0; j<user_prefs[GROUPING].current_value; j++)
    {
      if (user_prefs[LIL_ENDIAN].current_value)
        byte_addr = addr - 1 + (i*user_prefs[GROUPING].current_value) + (user_prefs[GROUPING].current_value - j);
      else
        byte_addr = addr + (i*user_prefs[GROUPING].current_value) + j;

      if (address_invalid(byte_addr))
        break;

      c = vf_get_char(&file_manager, &result, byte_addr);

      if (user_prefs[BLOB_GROUPING_OFFSET].current_value > byte_addr)
      {
        blob_standout(TRUE);
      }
      else
      {
        if (user_prefs[BLOB_GROUPING].current_value &&
           (((byte_addr - user_prefs[BLOB_GROUPING_OFFSET].current_value) / user_prefs[BLOB_GROUPING].current_value) & 1))
        {
          blob_standout(TRUE);
        }
      }

      if (user_prefs[DISPLAY_BINARY].current_value)
      {
        snprintf(bin_text, 8, "%b", c);
        for (k=0; k<8; k++)
        {
          if ((c >> (7 - k)) & 1)
            mvwaddch(window_list[WINDOW_HEX], y, x, '1');
          else
            mvwaddch(window_list[WINDOW_HEX], y, x, '0');
          x++;
        }
      }
      else
      {
        mvwaddch(window_list[WINDOW_HEX], y, x, HEX(c>>4&0xF));
        x++;
        mvwaddch(window_list[WINDOW_HEX], y, x, HEX(c>>0&0xF));
        x++;
      }

      if (!isprint(c))
        c = '.';

      if (user_prefs[LIL_ENDIAN].current_value)
        mvwaddch(window_list[WINDOW_ASCII], y, (1+i)*user_prefs[GROUPING].current_value-j, c);
      else
        mvwaddch(window_list[WINDOW_ASCII], y, 1+i*user_prefs[GROUPING].current_value+j, c);

      blob_standout(FALSE);

    }
    x++;
  }

  return ((i-1) * user_prefs[GROUPING].current_value) + j;
}

void place_cursor(off_t addr, cursor_alignment_e calign)
{
  int x, y;
  off_t new_screen_addr;

  if (address_invalid(addr) == 0)
  {
    if (addr < display_info.page_start)
    {
      new_screen_addr = (display_info.page_start - addr) / BYTES_PER_LINE;
      if ((display_info.page_start - addr) % BYTES_PER_LINE)
        new_screen_addr++;
      new_screen_addr = display_info.page_start - (new_screen_addr * BYTES_PER_LINE);
      print_screen(new_screen_addr);
    }
    if (addr > display_info.page_end)
    {
      new_screen_addr = (addr - display_info.page_end) / BYTES_PER_LINE;
      if ((addr - display_info.page_end) % BYTES_PER_LINE)
        new_screen_addr++;
      new_screen_addr = display_info.page_start + (new_screen_addr * BYTES_PER_LINE);
      print_screen(new_screen_addr);
    }

    x = get_x_from_addr(addr);
    y = get_y_from_addr(addr);
    wmove(window_list[display_info.cursor_window], y, x);
    display_info.cursor_addr = addr;
  }
}


void print_screen(off_t addr)
{
  int i;
  off_t line_addr = addr;

  display_info.page_start = addr;
  display_info.page_end = PAGE_END;

  for (i=0; i<HEX_LINES; i++)
  {
    if (address_invalid(line_addr))
      break;
    line_addr += print_line(line_addr, i);
  }
}


