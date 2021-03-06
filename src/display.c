/*
 * Functions related to rendering the data on the screen
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

#include "virt_file.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "display.h"
#include "app_state.h"
#include "key_handler.h"
#include "search.h"
#include "user_prefs.h"

display_info_t display_info;
WINDOW *window_list[MAX_WINDOWS];
PANEL *panel_list[MAX_WINDOWS];

BOOL
msg_prompt(char *fmt, ...)
{
  WINDOW *msgbox;
  char *tok;
  char msgbox_str[MAX_MSG_BOX_LEN];
  char msgbox_line[MAX_MSG_BOX_LEN];
  const char delimiters[] = " \t";
  int x = 1, y = 1, len = 0, c;
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
  while (tok)
    {
      if (( strlen(tok) + strlen(msgbox_line)) < ( MSG_BOX_W - 2 ))
        {
          if (strlen(msgbox_line))
            {
              strncat(msgbox_line, " ", MAX_MSG_BOX_LEN);
            }

          strncat(msgbox_line, tok, MAX_MSG_BOX_LEN);
        }
      else
        {
          x = 1 + (( MSG_BOX_W - 2 ) - strlen(msgbox_line)) / 2;
          mvwaddstr(msgbox, y, x, msgbox_line);
          y++;
          if (y > ( MSG_BOX_H - 2 ))
            {
              break;
            }

          memset(msgbox_line, 0, MAX_MSG_BOX_LEN);
          strncat(msgbox_line, tok, MAX_MSG_BOX_LEN);
        }

      tok = strtok(NULL, delimiters);
    }

  if (strlen(msgbox_line))
    {
      x = 1 + (( MSG_BOX_W - 2 ) - strlen(msgbox_line)) / 2;
      mvwaddstr(msgbox, y, x, msgbox_line);
      memset(msgbox_line, 0, MAX_MSG_BOX_LEN);
    }

  strncat(msgbox_line, "Y / N", MAX_MSG_BOX_LEN);
  x = 1 + (( MSG_BOX_W - 2 ) - strlen(msgbox_line)) / 2;
  mvwaddstr(msgbox, MSG_BOX_H - 2, x, msgbox_line);

  curs_set(0);
  wrefresh(msgbox);
  c = mwgetch(msgbox);
  while (c != 'y' && c != 'Y' && c != 'n' && c != 'N')
    {
      c = mwgetch(msgbox);
    }
  curs_set(1);

  delwin(msgbox);
  print_screen(display_info.page_start);

  if (c == 'y' || c == 'Y')
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

void
pat_err(const char *error, const char *pattern, int index, int max_index)
{
  WINDOW *msgbox;
  int pat_len, pat_offset = 0, x;
  char msgbox_line[MAX_MSG_BOX_LEN];

  if (index > max_index)
    {
      return;
    }

  pat_len = strnlen(pattern, MSG_BOX_W - 6);
  strncpy(msgbox_line, pattern, MSG_BOX_W - 6);

  if (index > pat_len)
    {
      pat_offset = index - (( MSG_BOX_W - 6 ) / 2 );
      pat_len = strnlen(pattern + pat_offset, MSG_BOX_W - 6);
      snprintf(msgbox_line, MSG_BOX_W - 6, "...%s", pattern + pat_offset);
      pat_offset
        -= 3; /* used next for the '^' marker char, but we must place
               * it 3 chars out for the addition of '...' to the pattern */
    }

  msgbox = newwin(7, MSG_BOX_W, MSG_BOX_Y, MSG_BOX_X);
  box(msgbox, 0, 0);

  mvwaddstr(msgbox, 1, 1, "PAT ERR:");
  mvwaddstr(msgbox, 1, 10, error);

  mvwaddstr(msgbox, 3, 1, msgbox_line);
  mvwaddstr(msgbox, 4, index - pat_offset + 1, "^");

  memset(msgbox_line, 0, MAX_MSG_BOX_LEN);
  strncat(msgbox_line, "[PRESS ANY KEY]", MAX_MSG_BOX_LEN);
  x = (( MSG_BOX_W - 2 ) - strlen(msgbox_line)) / 2;
  mvwaddstr(msgbox, 5, x, msgbox_line);

  curs_set(0);
  wrefresh(msgbox);
  mwgetch(msgbox);
  curs_set(1);
  delwin(msgbox);
  print_screen(display_info.page_start);
}

void
msg_box(const char *fmt, ...)
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
  while (tok)
    {
      if (( strlen(tok) + strlen(msgbox_line) + 1 ) < ( MSG_BOX_W - 2 ))
        {
          if (strlen(msgbox_line))
            {
              strncat(msgbox_line, " ", MAX_MSG_BOX_LEN);
            }

          strncat(msgbox_line, tok, MAX_MSG_BOX_LEN);
        }
      else
        {
          x = 1 + (( MSG_BOX_W - 2 ) - strlen(msgbox_line)) / 2;
          mvwaddstr(msgbox, y, x, msgbox_line);
          y++;
          if (y > ( MSG_BOX_H - 2 ))
            {
              return;
            }

          memset(msgbox_line, 0, MAX_MSG_BOX_LEN);
          strncat(msgbox_line, tok, MAX_MSG_BOX_LEN);
        }

      tok = strtok(NULL, delimiters);
    }

  if (strlen(msgbox_line))
    {
      x = 1 + (( MSG_BOX_W - 2 ) - strlen(msgbox_line)) / 2;
      mvwaddstr(msgbox, y, x, msgbox_line);
      y++;
      if (y > ( MSG_BOX_H - 2 ))
        {
          return;
        }

      memset(msgbox_line, 0, MAX_MSG_BOX_LEN);
    }

  strncat(msgbox_line, "[PRESS ANY KEY]", MAX_MSG_BOX_LEN);
  x = 1 + (( MSG_BOX_W - 2 ) - strlen(msgbox_line)) / 2;
  mvwaddstr(msgbox, MSG_BOX_H - 2, x, msgbox_line);

  curs_set(0);
  wrefresh(msgbox);
  mwgetch(msgbox);
  curs_set(1);
  delwin(msgbox);
  print_screen(display_info.page_start);
}

void
reset_display_info(void)
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
  sprintf(display_info.percent, "Top");
  update_status(NULL);
}

void
update_display_info(void)
{
  vf_stat(current_file, &vfstat);
  display_info.page_start -= display_info.page_start % BYTES_PER_LINE;
  display_info.file_size = vfstat.file_size;
  display_info.page_end = PAGE_END;
  display_info.has_color = has_colors();
  if (display_info.cursor_addr < display_info.page_start)
    {
      display_info.cursor_addr = display_info.page_start;
    }

  if (display_info.cursor_addr > display_info.page_end)
    {
      display_info.cursor_addr = display_info.page_end;
    }

  display_info.cursor_addr
    -= ( display_info.cursor_addr % user_prefs[GROUPING].value );
  update_percent();
  update_status(NULL);
}

void
update_percent(void)
{
  if (display_info.page_start == 0)
    {
      sprintf(display_info.percent, "Top");
    }
  else if (display_info.page_end == display_info.file_size - 1)
    {
      sprintf(display_info.percent, "Bot");
    }
  else
    {
      sprintf(
        display_info.percent,
        "%2jd%%",
        ( display_info.page_start ) / ( display_info.file_size / 100 ));
    }
}

void
update_status(const char *msg)
{
  if (vf_need_save(current_file) > 0)
    {
      snprintf(display_info.status, MAX_STATUS, "[+] ");
    }
  else
    {
      display_info.status[0] = 0;
    }

  if (msg != NULL)
    {
      snprintf(display_info.status_msg, MAX_STATUS, "%s ", msg);
    }
  else
    {
      display_info.status_msg[0] = 0;
    }

  display_info.status[MAX_STATUS - 1] = 0;
}

void
search_hl(BOOL on)
{
  if (on && user_prefs[SEARCH_HL].value
      && search_item[current_search].highlight)
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

void
visual_select_hl(BOOL on)
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

void
blob_standout(BOOL on)
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

int
is_visual_on(void)
{
  if (display_info.visual_select_addr == -1)
    {
      return 0;
    }
  else
    {
      return 1;
    }
}
int
visual_span(void)
{
  if (display_info.cursor_addr < display_info.visual_select_addr)
    {
      return ( display_info.visual_select_addr - display_info.cursor_addr )
             + user_prefs[GROUPING].value;
    }
  else
    {
      return ( display_info.cursor_addr - display_info.visual_select_addr )
             + user_prefs[GROUPING].value;
    }
}
off_t
visual_addr(void)
{
  if (display_info.cursor_addr < display_info.visual_select_addr)
    {
      return display_info.cursor_addr;
    }
  else
    {
      return display_info.visual_select_addr;
    }
}

/* returns the number of bytes displayed on that line */
int
print_line(off_t page_addr, off_t line_addr, char *screen_buf,
           size_t screen_buf_size, search_aid_t *search_aid)
{
  int i, j, k, y, x = 1;
  off_t byte_addr;
  char c, result, addr_text[ADDR_DIGITS + 1], bin_text[9];

  y = ( line_addr - page_addr ) / BYTES_PER_LINE;
  y++; /* line 0 is the box border */

  /* print address */
  snprintf(addr_text, ADDR_BOX_W, "%08jX", line_addr);
  mvwaddstr(window_list[WINDOW_ADDR], y, 1, addr_text);

  if (screen_buf == NULL)
    {
      if (address_invalid(line_addr))
        {
          return 0;
        }
    }
  else
    {
      if (line_addr < page_addr || line_addr >= page_addr + screen_buf_size)
        {
          return 0;
        }
    }

  for (i = 0, j = 0; i < HEX_COLS; i++)
    {
      /* print hex and ascii */
      for (j = 0; j < user_prefs[GROUPING].value; j++)
        {
          if (user_prefs[LIL_ENDIAN].value)
            {
              byte_addr = line_addr - 1 + ( i * user_prefs[GROUPING].value )
                          + ( user_prefs[GROUPING].value - j );
            }
          else
            {
              byte_addr = line_addr + ( i * user_prefs[GROUPING].value ) + j;
            }

          if (screen_buf == NULL)
            {
              if (address_invalid(line_addr))
                {
                  search_hl(FALSE);
                  visual_select_hl(FALSE);
                  break;
                }
              else
                {
                  c = vf_get_char(current_file, &result, byte_addr);
                }
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
                {
                  c = screen_buf[byte_addr - page_addr];
                }

              /* check for search highlighting */
              if (search_item[current_search].highlight && search_aid != NULL)
                {
                  if (search_aid->hl_start != -1)
                    {
                      if (search_aid->hl_start <= byte_addr)
                        {
                          if (search_aid->hl_end > byte_addr)
                            {
                              search_hl(TRUE);
                            }
                          else
                            {
                              search_hl(FALSE);
                              buf_search(search_aid);
                              if (search_aid->hl_start <= byte_addr
                                  && search_aid->hl_end > byte_addr)
                                {
                                  search_hl(TRUE);
                                }
                            }
                        }
                    }
                }

              /**/
            }

          if (is_visual_on())
            {
              if (( display_info.visual_select_addr <= byte_addr
                    && display_info.cursor_addr + user_prefs[GROUPING].value - 1
                    >= byte_addr )
                  || ( display_info.cursor_addr <= byte_addr
                       && display_info.visual_select_addr + user_prefs[GROUPING].value
                       - 1
                       >= byte_addr ))
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
              if (user_prefs[BLOB_GROUPING].value
                  && ((( byte_addr - user_prefs[BLOB_GROUPING_OFFSET].value )
                       / user_prefs[BLOB_GROUPING].value )
                      & 1 ))
                {
                  blob_standout(TRUE);
                }
            }

          if (user_prefs[DISPLAY_BINARY].value)
            {
              snprintf(bin_text, 8, "%c", c);
              for (k = 0; k < 8; k++)
                {
                  if (( c >> ( 7 - k )) & 1)
                    {
                      mvwaddch(window_list[WINDOW_HEX], y, x, '1');
                    }
                  else
                    {
                      mvwaddch(window_list[WINDOW_HEX], y, x, '0');
                    }

                  x++;
                }
            }
          else
            {
              mvwaddch(window_list[WINDOW_HEX], y, x, HEX(c >> 4 & 0xF));
              x++;
              mvwaddch(window_list[WINDOW_HEX], y, x, HEX(c >> 0 & 0xF));
              x++;
            }

          if (!isprint(c))
            {
              c = '.';
            }

          if (user_prefs[LIL_ENDIAN].value)
            {
              mvwaddch(
                window_list[WINDOW_ASCII],
                y,
                ( 1 + i ) * user_prefs[GROUPING].value - j,
                c);
            }
          else
            {
              mvwaddch(
                window_list[WINDOW_ASCII],
                y,
                1 + i * user_prefs[GROUPING].value + j,
                c);
            }

          blob_standout(FALSE);
        }

      mvwaddch(window_list[WINDOW_HEX], y, x++, ' ');
    }

  return (( i - 1 ) * user_prefs[GROUPING].value ) + j;
}

void
update_file_tabs_window(void)
{
  file_manager_t *tmp_file, *head_file;
  int i = 1;

  head_file = vf_get_head_fm_from_ring(file_ring);
  tmp_file = head_file;

  werase(window_list[WINDOW_MENU]);
  mvwaddch(window_list[WINDOW_MENU], 0, 0, '|');

  do
    {
      waddch(window_list[WINDOW_MENU], ' ');
      if (tmp_file == current_file)
        {
          wattron(window_list[WINDOW_MENU], A_STANDOUT);
        }

      wprintw(
        window_list[WINDOW_MENU],
        "[%d] %s",
        i,
        vf_get_fname_file(tmp_file));
      i++;
      if (vf_need_save(tmp_file))
        {
          waddch(window_list[WINDOW_MENU], '*');
        }

      wattroff(window_list[WINDOW_MENU], A_STANDOUT);
      waddch(window_list[WINDOW_MENU], ' ');
      waddch(window_list[WINDOW_MENU], '|');
      tmp_file = vf_get_next_fm_from_ring(file_ring);
    }
  while ( tmp_file != head_file );
  wrefresh(window_list[WINDOW_MENU]);

  vf_set_current_fm_from_ring(file_ring, current_file);
}

void
update_status_window(void)
{
  int i, result, len;
  char tmp[4], bin_text[9], size_text[32];
  char line[MAX_FILE_NAME];
  const char metrics[] = " KMGTPE";
  off_t adjusted_size = 0, size_shadow = 0;

  werase(window_list[WINDOW_STATUS]);

  len = snprintf(line, MAX_FILE_NAME, "%s", vf_get_fname(current_file));
  if (len)
    {
      len += snprintf(line + len, MAX_FILE_NAME - len, " ");
    }

  len += snprintf(line + len, MAX_FILE_NAME - len, "%s", display_info.status);
  len += snprintf(
    line + len,
    MAX_FILE_NAME - len,
    "%s",
    display_info.status_msg);
  if (macro_key != -1)
    {
      len += snprintf(
        line + len,
        MAX_FILE_NAME - len,
        "[recording '%c']",
        macro_key + 'a');
    }

  if (is_visual_on())
    {
      off_t start = visual_addr();
      off_t size = visual_span();
      len += snprintf(
        line + len,
        MAX_FILE_NAME - len,
        "|%jx-%jx| (%jx/%'jd)",
        start,
        display_info.cursor_addr,
        size,
        size);
    }

  line[MAX_FILE_NAME - 1] = 0;
  mvwaddstr(window_list[WINDOW_STATUS], 0, 0, line);

  result = vf_get_buf(current_file, tmp, display_info.cursor_addr, 4);
  if (result > 0)
    {
      for (i = 0; i < 8; i++)
        {
          snprintf(bin_text + i, 2, "%c", (( tmp[0] >> ( 7 - i )) & 1 ) ? '1' : '0');
        }

      mvwprintw(
        window_list[WINDOW_STATUS],
        STATUS_1_Y,
        STATUS_1_X,
        "[/x %02x  /d %03u  /b %s  ]",
        (unsigned char)tmp[0],
        (unsigned char)tmp[0],
        bin_text);

      if (result == 4)
        {
          mvwprintw(
            window_list[WINDOW_STATUS],
            STATUS_2_Y,
            STATUS_2_X,
            "[U32: %02x%02x%02x%02x (le: %02x%02x%02x%02x)]",
            (unsigned char)tmp[0],
            (unsigned char)tmp[1],
            (unsigned char)tmp[2],
            (unsigned char)tmp[3],
            (unsigned char)tmp[3],
            (unsigned char)tmp[2],
            (unsigned char)tmp[1],
            (unsigned char)tmp[0]);
        }
    }

  adjusted_size = display_info.file_size;
  i = 0;
  while (adjusted_size / 1024 > 0 && i < 6)
    {
      size_shadow = adjusted_size;
      adjusted_size /= 1024;
      i++;
    }
  size_shadow %= 1024;
  while (size_shadow > 100)
    {
      size_shadow /= 10;
    }

  if (i == 0)
    {
      snprintf(size_text, 31, "(%'ju B)", adjusted_size);
    }
  else
    {
      snprintf(
        size_text,
        31,
        "(%'ju.%ju %ciB)",
        adjusted_size,
        size_shadow,
        metrics[i]);
    }

  mvwprintw(
    window_list[WINDOW_STATUS],
    1,
    0,
    "%s %08jx/%08jx  %'ju/%'ju %s",
    display_info.percent,
    display_info.cursor_addr,
    display_info.file_size,
    display_info.cursor_addr,
    display_info.file_size,
    size_text);

  /* reset cursor */
  wmove(
    window_list[display_info.cursor_window],
    get_y_from_addr(display_info.cursor_addr),
    get_x_from_addr(display_info.cursor_addr));
}

void
place_cursor(off_t addr, cursor_alignment_e calign, cursor_t cursor)
{
  int x, y;
  off_t new_screen_addr = 0;

  if (cursor == CURSOR_VIRTUAL)
    {
      if (address_invalid(addr) == 0)
        {
          display_info.virtual_cursor_addr = addr;
        }
      else
        {
          display_info.virtual_cursor_addr = display_info.cursor_addr;
        }

      return;
    }

  if (address_invalid(addr) == 0)
    {
      if (addr < display_info.page_start)
        {
          new_screen_addr = ( display_info.page_start - addr ) / BYTES_PER_LINE;
          if (( display_info.page_start - addr ) % BYTES_PER_LINE)
            {
              new_screen_addr++;
            }

          new_screen_addr
            = display_info.page_start - ( new_screen_addr * BYTES_PER_LINE );
          print_screen(new_screen_addr);
          update_status(NULL);
          update_percent();
        }
      else if (addr > display_info.page_end)
        {
          new_screen_addr = ( addr - display_info.page_end ) / BYTES_PER_LINE;
          if (( addr - display_info.page_end ) % BYTES_PER_LINE)
            {
              new_screen_addr++;
            }

          new_screen_addr
            = display_info.page_start + ( new_screen_addr * BYTES_PER_LINE );
          print_screen(new_screen_addr);
          update_status(NULL);
          update_percent();
        }

      x = get_x_from_addr(addr);
      y = get_y_from_addr(addr);
      display_info.cursor_addr = get_addr_from_xy(x, y);

      if (is_visual_on())
        {
          print_screen(display_info.page_start);
        }

      wmove(window_list[display_info.cursor_window], y, x);
    }

  if (display_info.file_size == 0)
    {
      display_info.cursor_addr = 0;
      wmove(window_list[display_info.cursor_window], 1, 1);
    }
}

void
print_screen_buf(off_t addr, char *screen_buf, size_t screen_buf_size,
                 search_aid_t *search_aid)
{
  int i;
  off_t line_addr = addr;

  werase(window_list[WINDOW_HEX]);
  werase(window_list[WINDOW_ASCII]);
  werase(window_list[WINDOW_STATUS]);
  box(window_list[WINDOW_HEX], 0, 0);
  box(window_list[WINDOW_ASCII], 0, 0);

  for (i = 0; i < HEX_LINES; i++)
    {
      line_addr += print_line(
        addr,
        line_addr,
        screen_buf,
        screen_buf_size,
        search_aid);
      if (screen_buf == NULL)
        {
          if (address_invalid(line_addr))
            {
              break;
            }
        }
      else
        {
          if (line_addr < addr || line_addr >= addr + screen_buf_size)
            {
              break;
            }
        }
    }

  search_hl(FALSE);
}

void
print_screen(off_t addr)
{
  int screen_buf_size;
  char *screen_buf;
  search_aid_t search_aid, *sa_p = NULL;

  display_info.page_start = addr;
  display_info.page_end = PAGE_END;

  screen_buf = (char *)malloc(PAGE_SIZE);
  screen_buf_size = vf_get_buf(current_file, screen_buf, addr, PAGE_SIZE);

  if (search_item[current_search].used == TRUE)
    {
      fill_search_buf(addr, screen_buf_size, &search_aid, SEARCH_FORWARD);
      sa_p = &search_aid;
    }

  print_screen_buf(addr, screen_buf, screen_buf_size, sa_p);

  if (search_item[current_search].used == TRUE)
    {
      free_search_buf(&search_aid);
    }

  free(screen_buf);
  update_file_tabs_window();
}

off_t
address_invalid(off_t addr)
{
  off_t dist_past_range = 0;

  if (addr < 0)
    {
      dist_past_range = addr;
    }

  if (addr >= display_info.file_size)
    {
      dist_past_range = ( addr - display_info.file_size ) + 1;
    }

  return dist_past_range;
}

int
get_y_from_page_offset(int offset)
{
  int y = 1, z;

  if (offset < 0)
    {
      return -1;
    }

  if (offset > PAGE_SIZE)
    {
      return -1;
    }

  z = ( HEX_COLS * user_prefs[GROUPING].value );
  y += offset / z;

  return y;
}

int
get_x_from_page_offset(int offset)
{
  int x, y;

  y = get_y_from_page_offset(offset);

  if (y < 0)
    {
      return -1;
    }

  y--;

  offset -= y * HEX_COLS * user_prefs[GROUPING].value;

  if (display_info.cursor_window == WINDOW_HEX)
    {
      x = 1 + ( offset / user_prefs[GROUPING].value ) * BYTES_PER_GROUP;
    }
  else
    {
      x = 1 + offset;
    }

  return x;
}

int
get_y_from_addr(off_t addr)
{
  off_t offset, page_size;
  int y = 1, z;

  if (addr < display_info.page_start)
    {
      return -1;
    }

  offset = addr - display_info.page_start;
  page_size = display_info.page_end - display_info.page_start;

  if (offset > page_size)
    {
      return -1;
    }

  z = ( HEX_COLS * user_prefs[GROUPING].value );
  y += offset / z;

  return y;
}

int
get_x_from_addr(off_t addr)
{
  off_t offset;
  int x, y;

  y = get_y_from_addr(addr);

  if (y < 0)
    {
      return -1;
    }

  y--;

  offset = addr - display_info.page_start;
  offset -= y * HEX_COLS * user_prefs[GROUPING].value;

  if (display_info.cursor_window == WINDOW_HEX)
    {
      x = 1 + ( offset / user_prefs[GROUPING].value ) * BYTES_PER_GROUP;
    }
  else
    {
      x = 1 + offset;
    }

  return x;
}

off_t
get_addr_from_xy(int x, int y)
{
  off_t addr = display_info.page_start;

  if (y < 1 || y > ( HEX_BOX_H - 1 ))
    {
      return -1;
    }

  if (x < 1 || x > ( HEX_BOX_W - 1 ))
    {
      return -1;
    }

  y--; /* top border */
  x--; /* left border */

  if (x > ( BYTES_PER_GROUP * HEX_COLS ) - BYTES_PER_GROUP)
    {
      return -1;
    }

  addr += y * HEX_COLS * user_prefs[GROUPING].value;
  addr += ( x / BYTES_PER_GROUP ) * user_prefs[GROUPING].value;

  if (addr > display_info.page_end || address_invalid(addr))
    {
      return -1;
    }

  return addr;
}

void
destroy_screen(void)
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

void
create_screen(void)
{
  int a, h;

  h = HEX_BOX_W;
  a = ASCII_BOX_W;

  window_list[WINDOW_MENU]
    = newwin(MENU_BOX_H, MENU_BOX_W, MENU_BOX_Y, MENU_BOX_X);
  window_list[WINDOW_ADDR]
    = newwin(ADDR_BOX_H, ADDR_BOX_W, ADDR_BOX_Y, ADDR_BOX_X);
  window_list[WINDOW_HEX]
    = newwin(HEX_BOX_H, HEX_BOX_W, HEX_BOX_Y, HEX_BOX_X);
  window_list[WINDOW_ASCII]
    = newwin(ASCII_BOX_H, ASCII_BOX_W, ASCII_BOX_Y, ASCII_BOX_X);
  window_list[WINDOW_STATUS] = newwin(
    WINDOW_STATUS_H,
    WINDOW_STATUS_W,
    WINDOW_STATUS_Y,
    WINDOW_STATUS_X);

  panel_list[WINDOW_MENU] = new_panel(window_list[WINDOW_MENU]);
  panel_list[WINDOW_ADDR] = new_panel(window_list[WINDOW_ADDR]);
  panel_list[WINDOW_HEX] = new_panel(window_list[WINDOW_HEX]);
  panel_list[WINDOW_ASCII] = new_panel(window_list[WINDOW_ASCII]);
  panel_list[WINDOW_STATUS] = new_panel(window_list[WINDOW_STATUS]);

  keypad(window_list[WINDOW_MENU], TRUE);
  keypad(window_list[WINDOW_HEX], TRUE);
  keypad(window_list[WINDOW_ASCII], TRUE);

  box(window_list[WINDOW_HEX], 0, 0);
  box(window_list[WINDOW_ASCII], 0, 0);
}
