#include <string.h>
#include <stdlib.h>
#include "display.h"
#include "user_prefs.h"
#include "app_state.h"
#include "virt_file.h"
#include "search.h"

display_info_t display_info;
WINDOW *window_list[MAX_WINDOWS];
PANEL *panel_list[MAX_WINDOWS];

void msg_box(char *fmt, ...)
{
  WINDOW *msgbox;
  char *tok;
  char msgbox_str[MAX_MSG_BOX_LEN];
  char msgbox_line[MAX_MSG_BOX_LEN];
  const char delimiters[] = " \t";
  int x = 1, y = 1, len = 0;
  va_list args;

  memset(msgbox_str, 0, MAX_MSG_BOX_LEN);
  memset(msgbox_line, 0, MAX_MSG_BOX_LEN);

  msgbox = newwin(MSG_BOX_H, MSG_BOX_W, MSG_BOX_Y, MSG_BOX_X);
  box(msgbox, 0, 0);

  va_start(args, fmt);
  vsnprintf(msgbox_str, MAX_MSG_BOX_LEN, fmt, args);
  va_end(args);
  len = strlen(msgbox_str);

  tok = strtok(msgbox_str, delimiters);
  while(tok)
  {
    if ((strlen(tok) + strlen(msgbox_line)) < (MSG_BOX_W - 2))
    {
      if (strlen(msgbox_line))
        strncat(msgbox_line, " ", MAX_MSG_BOX_LEN);
      strncat(msgbox_line, tok, MAX_MSG_BOX_LEN);
    }
    else
    {
      x = 1 + ((MSG_BOX_W - 2) - strlen(msgbox_line))/2;
      mvwaddstr(msgbox, y, x, msgbox_line);
      y++;
      if (y > (MSG_BOX_H - 2))
        return;
      memset(msgbox_line, 0, MAX_MSG_BOX_LEN);
      strncat(msgbox_line, tok, MAX_MSG_BOX_LEN);
    }

    tok = strtok(NULL, delimiters);
  }

  if (strlen(msgbox_line))
  {
    x = ((MSG_BOX_W - 2) - strlen(msgbox_line))/2;
    mvwaddstr(msgbox, y, x, msgbox_line);
    y++;
    if (y > (MSG_BOX_H - 2))
      return;
    memset(msgbox_line, 0, MAX_MSG_BOX_LEN);
  }

  strncat(msgbox_line, "[PRESS ANY KEY]", MAX_MSG_BOX_LEN);
  x = ((MSG_BOX_W - 2) - strlen(msgbox_line))/2;
  mvwaddstr(msgbox, MSG_BOX_H - 2, x, msgbox_line);

  wrefresh(msgbox);
  getch();
  delwin(msgbox);
  print_screen(display_info.page_start);
}

void reset_display_info(void)
{
  vf_stat(current_file, &vfstat);
  display_info.file_size = vfstat.file_size;
  display_info.page_start = 0;
  display_info.page_end = PAGE_END;
  display_info.cursor_addr = 0;
  display_info.cursor_window = WINDOW_HEX;
  display_info.max_cols = 0;
  display_info.has_color = has_colors();
  display_info.visual_select_addr = -1;
}

void update_display_info(void)
{
  vf_stat(current_file, &vfstat);
  display_info.page_start -= display_info.page_start % BYTES_PER_LINE;
  display_info.file_size = vfstat.file_size;
  display_info.page_end = PAGE_END;
  display_info.has_color = has_colors();
  if (display_info.cursor_addr < display_info.page_start)
    display_info.cursor_addr = display_info.page_start;
  if (display_info.cursor_addr > display_info.page_end)
    display_info.cursor_addr = display_info.page_end;
  display_info.cursor_addr -= (display_info.cursor_addr % user_prefs[GROUPING].value);
}

void search_hl(BOOL on)
{
  if (on && user_prefs[SEARCH_HL].value && search_item[current_search].highlight)
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

void visual_select_hl(BOOL on)
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

int is_visual_on(void)
{
  if (display_info.visual_select_addr == -1)
    return 0;
  else
    return 1;
}
int visual_span(void)
{
  if (display_info.cursor_addr < display_info.visual_select_addr)
    return (display_info.visual_select_addr - display_info.cursor_addr) + user_prefs[GROUPING].value;
  else
    return (display_info.cursor_addr - display_info.visual_select_addr) + user_prefs[GROUPING].value;
}
off_t visual_addr(void)
{
  if (display_info.cursor_addr < display_info.visual_select_addr)
    return display_info.cursor_addr;
  else
    return display_info.visual_select_addr;
}

/* returns the number of bytes displayed on that line */
int print_line(off_t page_addr, off_t line_addr, char *screen_buf, int screen_buf_size, search_aid_t *search_aid)
{
  int i, j, k,
      y, x = 1,
      byte_addr;
  char c, result,
       addr_text[ADDR_DIGITS + 1],
       bin_text[9];

  y = (line_addr - page_addr) / BYTES_PER_LINE;
  y++; /* line 0 is the box border */

  /* print address */
  sprintf(addr_text, "%08X", line_addr);
  mvwaddstr(window_list[WINDOW_ADDR], y, 1, addr_text);

  if (screen_buf == NULL)
  {
    if (address_invalid(line_addr))
      return 0;
  }
  else
  {
    if (line_addr < page_addr || line_addr >= page_addr + screen_buf_size)
      return 0;
  }

  for (i=0; i<HEX_COLS; i++)
  {
    /* print hex and ascii */
    for (j=0; j<user_prefs[GROUPING].value; j++)
    {
      if (user_prefs[LIL_ENDIAN].value)
        byte_addr = line_addr - 1 + (i*user_prefs[GROUPING].value) + (user_prefs[GROUPING].value - j);
      else
        byte_addr = line_addr + (i*user_prefs[GROUPING].value) + j;

      if (screen_buf == NULL)
      {
        if (address_invalid(line_addr))
        {
          search_hl(FALSE);
          visual_select_hl(FALSE);
          break;
        }
        else
          c = vf_get_char(current_file, &result, byte_addr);
      }
      else
      {
        if (byte_addr < page_addr || byte_addr >= page_addr + screen_buf_size)
        {
          search_hl(FALSE);
          visual_select_hl(FALSE);
          break;
        }
        else
          c = screen_buf[byte_addr - page_addr];

/* check for search highlighting */
        if (search_item[current_search].highlight &&
            search_aid != NULL)
        {
          if (search_aid->hl_start != -1)
          {
            if(search_aid->hl_start <= byte_addr)
            {
              if (search_aid->hl_end > byte_addr)
              {
                search_hl(TRUE);
              }
              else
              {
                search_hl(FALSE);
                buf_search(search_aid);
                if(search_aid->hl_start <= byte_addr &&
                   search_aid->hl_end > byte_addr)
                  search_hl(TRUE);
              }
            }
          }
        }
/**/
      }

      if (is_visual_on())
      {
        if ((display_info.visual_select_addr <= byte_addr &&
             display_info.cursor_addr + user_prefs[GROUPING].value - 1 >= byte_addr)        ||
            (display_info.cursor_addr <= byte_addr        &&
             display_info.visual_select_addr + user_prefs[GROUPING].value - 1 >= byte_addr))
        {
          visual_select_hl(TRUE);
        }
        else
        {
          visual_select_hl(FALSE);
        }
      }

      if (user_prefs[BLOB_GROUPING_OFFSET].value > byte_addr)
      {
        blob_standout(TRUE);
      }
      else
      {
        if (user_prefs[BLOB_GROUPING].value &&
           (((byte_addr - user_prefs[BLOB_GROUPING_OFFSET].value) / user_prefs[BLOB_GROUPING].value) & 1))
        {
          blob_standout(TRUE);
        }
      }

      if (user_prefs[DISPLAY_BINARY].value)
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

      if (user_prefs[LIL_ENDIAN].value)
        mvwaddch(window_list[WINDOW_ASCII], y, (1+i)*user_prefs[GROUPING].value-j, c);
      else
        mvwaddch(window_list[WINDOW_ASCII], y, 1+i*user_prefs[GROUPING].value+j, c);

      blob_standout(FALSE);

    }
    mvwaddch(window_list[WINDOW_HEX], y, x++, ' ');
  }

  return ((i-1) * user_prefs[GROUPING].value) + j;
}

void update_status_window(void)
{
  int y, x, i, result;
  unsigned char tmp[4], bin_text[9];

  y = get_y_from_addr(display_info.cursor_addr);
  x = get_x_from_addr(display_info.cursor_addr);

  result = vf_get_buf(current_file, tmp, display_info.cursor_addr, 4);

  if (result > 0)
  {
    for (i=0; i<8; i++)
    {
      if ((tmp[0] >> (7 - i)) & 1)
        snprintf(bin_text+i, 2, "%c", '1');
      else
        snprintf(bin_text+i, 2, "%c", '0');
    }
  }

  werase(window_list[WINDOW_STATUS]);

  mvwaddstr(window_list[WINDOW_STATUS], 0, 0, vf_get_fname(current_file));
  mvwprintw(window_list[WINDOW_STATUS], 1, 0,
            "Addr: %08x (%d)",
            display_info.cursor_addr,
            display_info.cursor_addr);

  if (result > 0)
    mvwprintw(window_list[WINDOW_STATUS], STATUS_1_Y, STATUS_1_X,
              "[/x %02x  /d %03u  /b %s  ]",
              tmp[0], tmp[0], bin_text);
  if (result == 4)
    mvwprintw(window_list[WINDOW_STATUS], STATUS_2_Y, STATUS_2_X,
              "[U32: %02x%02x%02x%02x (le: %02x%02x%02x%02x)]",
              tmp[0], tmp[1], tmp[2], tmp[3],
              tmp[3], tmp[2], tmp[1], tmp[0]);


  wmove(window_list[display_info.cursor_window], y, x);
}

void place_cursor(off_t addr, cursor_alignment_e calign, cursor_t cursor)
{
  int x, y;
  off_t new_screen_addr = 0;

  if (cursor == CURSOR_VIRTUAL)
  {
    if (address_invalid(addr) == 0)
      display_info.virtual_cursor_addr = addr;
    else
      display_info.virtual_cursor_addr = display_info.cursor_addr;
    return;
  }

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
    display_info.cursor_addr = get_addr_from_xy(x,y);
  }

  if (display_info.file_size == 0)
  {
    display_info.cursor_addr = 0;
    wmove(window_list[display_info.cursor_window], 1, 1);
  }

  if (is_visual_on())
    print_screen(display_info.page_start);

}


void print_screen_buf(off_t addr, char *screen_buf, int screen_buf_size, search_aid_t *search_aid)
{
  int i;
  off_t line_addr = addr;

  werase(window_list[WINDOW_HEX]);
  werase(window_list[WINDOW_ASCII]);
  werase(window_list[WINDOW_STATUS]);
  box(window_list[WINDOW_HEX], 0, 0);
  box(window_list[WINDOW_ASCII], 0, 0);

  for (i=0; i<HEX_LINES; i++)
  {
    line_addr += print_line(addr, line_addr, screen_buf, screen_buf_size, search_aid);
    if (screen_buf == NULL)
    {
      if (address_invalid(line_addr))
        break;
    }
    else
    {
      if (line_addr < addr || line_addr >= addr + screen_buf_size)
        break;
    }
  }

  search_hl(FALSE);

}

void print_screen(off_t addr)
{
  int i, screen_buf_size;
  off_t line_addr = addr;
  char *screen_buf;
  search_aid_t search_aid;

  display_info.page_start = addr;
  display_info.page_end = PAGE_END;

  screen_buf_size = PAGE_END - addr + 1;
  screen_buf = (char *)malloc(screen_buf_size);
  vf_get_buf(current_file, screen_buf, addr, screen_buf_size);
  fill_search_buf(addr, screen_buf_size, &search_aid);

  print_screen_buf(addr, screen_buf, screen_buf_size, &search_aid);

  free_search_buf(&search_aid);
  free(screen_buf);
}

off_t address_invalid(off_t addr)
{
  off_t dist_past_range = 0;

  if (addr < 0)
    dist_past_range = addr;
  if (addr >= display_info.file_size)
    dist_past_range = (addr - display_info.file_size) + 1;

  return dist_past_range;
}

int get_y_from_page_offset(int offset)
{
  int y = 1, z;

  if (offset < 0)
    return -1;

  if (offset > PAGE_SIZE)
    return -1;

  z = (HEX_COLS * user_prefs[GROUPING].value);
  y += offset / z;

  return y;
}

int get_x_from_page_offset(int offset)
{
  int x, y;
  y = get_y_from_page_offset(offset);

  if (y < 0)
    return -1;

  y--;

  offset -= y * HEX_COLS * user_prefs[GROUPING].value;

  if (display_info.cursor_window == WINDOW_HEX)
    x = 1 + (offset / user_prefs[GROUPING].value) * BYTES_PER_GROUP;
  else
    x = 1 + offset;

  return x;
}

int get_y_from_addr(off_t addr)
{
  off_t offset, page_size;
  int y = 1, z;

  if (addr < display_info.page_start)
    return -1;

  offset = addr - display_info.page_start;
  page_size = display_info.page_end - display_info.page_start;

  if (offset > page_size)
    return -1;

  z = (HEX_COLS * user_prefs[GROUPING].value);
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
  offset -= y * HEX_COLS * user_prefs[GROUPING].value;

  if (display_info.cursor_window == WINDOW_HEX)
    x = 1 + (offset / user_prefs[GROUPING].value) * BYTES_PER_GROUP;
  else
    x = 1 + offset;

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

  if (x > (BYTES_PER_GROUP * HEX_COLS) - BYTES_PER_GROUP)
    return -1;

  addr += y * HEX_COLS * user_prefs[GROUPING].value;
  addr += (x / BYTES_PER_GROUP) * user_prefs[GROUPING].value;

  if (addr > display_info.page_end || address_invalid(addr))
    return -1;

  return addr;
}

void destroy_screen()
{
  del_panel(panel_list[WINDOW_MENU]);
  del_panel(panel_list[WINDOW_ADDR]);
  del_panel(panel_list[WINDOW_HEX]);
  del_panel(panel_list[WINDOW_ASCII]);
  del_panel(panel_list[WINDOW_STATUS]);

  delwin(window_list[WINDOW_MENU]);
  delwin(window_list[WINDOW_ADDR]);
  delwin(window_list[WINDOW_HEX]);
  delwin(window_list[WINDOW_ASCII]);
  delwin(window_list[WINDOW_STATUS]);
}

void create_screen()
{
  int a, h;

  h = HEX_BOX_W;
  a = ASCII_BOX_W;

  window_list[WINDOW_MENU]  = newwin( MENU_BOX_H,  MENU_BOX_W,  MENU_BOX_Y,  MENU_BOX_X);
  window_list[WINDOW_ADDR]  = newwin( ADDR_BOX_H,  ADDR_BOX_W,  ADDR_BOX_Y,  ADDR_BOX_X);
  window_list[WINDOW_HEX]   = newwin(  HEX_BOX_H,   HEX_BOX_W,   HEX_BOX_Y,   HEX_BOX_X);
  window_list[WINDOW_ASCII] = newwin(ASCII_BOX_H, ASCII_BOX_W, ASCII_BOX_Y, ASCII_BOX_X);
  window_list[WINDOW_STATUS] = newwin(WINDOW_STATUS_H, WINDOW_STATUS_W, WINDOW_STATUS_Y, WINDOW_STATUS_X);

  panel_list[WINDOW_MENU]  = new_panel(window_list[WINDOW_MENU]);
  panel_list[WINDOW_ADDR]  = new_panel(window_list[WINDOW_ADDR]);
  panel_list[WINDOW_HEX]   = new_panel(window_list[WINDOW_HEX]);
  panel_list[WINDOW_ASCII] = new_panel(window_list[WINDOW_ASCII]);
  panel_list[WINDOW_STATUS] = new_panel(window_list[WINDOW_STATUS]);

  keypad(window_list[WINDOW_MENU], TRUE);
  keypad(window_list[WINDOW_HEX], TRUE);
  keypad(window_list[WINDOW_ASCII], TRUE);

  box(window_list[WINDOW_HEX], 0, 0);
  box(window_list[WINDOW_ASCII], 0, 0);
}


