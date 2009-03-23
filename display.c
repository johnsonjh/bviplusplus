#include <string.h>
#include "display.h"
#include "user_prefs.h"
#include "app_state.h"
#include "virt_file.h"

display_info_t display_info;

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

void update_display_info(void)
{
  vf_stat(current_file, &vfstat);
  display_info.file_size = vfstat.file_size;
  display_info.page_start = 0;
  display_info.page_end = PAGE_END;
  display_info.cursor_addr = 0;
  display_info.cursor_window = WINDOW_HEX;
  display_info.max_cols = 0;
  display_info.has_color = has_colors();
}

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

  /* print address */
  sprintf(addr_text, "%08X", addr);
  mvwaddstr(window_list[WINDOW_ADDR], y, 1, addr_text);

  for (i=0; i<HEX_COLS; i++)
  {
    /* print hex and ascii */
    for (j=0; j<user_prefs[GROUPING].value; j++)
    {
      if (user_prefs[LIL_ENDIAN].value)
        byte_addr = addr - 1 + (i*user_prefs[GROUPING].value) + (user_prefs[GROUPING].value - j);
      else
        byte_addr = addr + (i*user_prefs[GROUPING].value) + j;

      if (address_invalid(byte_addr))
        break;

      c = vf_get_char(current_file, &result, byte_addr);

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
    x++;
  }

  return ((i-1) * user_prefs[GROUPING].value) + j;
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

  werase(window_list[WINDOW_HEX]);
  werase(window_list[WINDOW_ASCII]);
  box(window_list[WINDOW_HEX], 0, 0);
  box(window_list[WINDOW_ASCII], 0, 0);

  for (i=0; i<HEX_LINES; i++)
  {
    if (address_invalid(line_addr))
      break;
    line_addr += print_line(line_addr, i);
  }
}


