#include <ncurses.h>
#include <string.h>
#include "key_handler.h"
#include "windows.h"
#include "user_prefs.h"
#include "display.h"
#include "actions.h"

typedef enum action_code_e
{
  E_SUCCESS,
  E_NO_ACTION,
  E_INVALID,
} action_code_t;

/** UP **/
action_code_t _action_cursor_move_up(int *y, int *x)
{
  window_t w;
  action_code_t error = E_SUCCESS;

  w = display_info.cursor_window;

  switch (w)
  {
    case WINDOW_HEX:
    case WINDOW_ASCII:
      if (get_addr_from_xy(*x, (*y) - 1) == -1)
        error = E_NO_ACTION;
      else
        *y = *y - 1;
      break;
    default:
      error = E_INVALID;
      break;
  }

  return error;
}
action_code_t action_cursor_move_up(void)
{
  int x, y;
  window_t w;
  action_code_t error = E_SUCCESS;

  w = display_info.cursor_window;
  getyx(window_list[w], y, x);

  error = _action_cursor_move_up(&y, &x);

  if (error == E_SUCCESS)
    wmove(window_list[w], y, x);

  return error;
}

/** DOWN **/
action_code_t _action_cursor_move_down(int *y, int *x)
{
  window_t w;
  action_code_t error = E_SUCCESS;

  w = display_info.cursor_window;

  switch (w)
  {
    case WINDOW_HEX:
    case WINDOW_ASCII:
      if (get_addr_from_xy(*x, (*y) + 1) == -1)
        error = E_NO_ACTION;
      else
        (*y) = *y + 1;
      break;
    default:
      error = E_INVALID;
      break;
  }

  return error;
}
action_code_t action_cursor_move_down(void)
{
  int x,y;
  window_t w;
  action_code_t error = E_SUCCESS;

  w = display_info.cursor_window;
  getyx(window_list[w], y, x);

  error = _action_cursor_move_down(&y, &x);

  if (error == E_SUCCESS)
    wmove(window_list[w], y, x);

  return error;
}

/** LEFT **/
action_code_t _action_cursor_move_left(int *y, int *x)
{
  int tmp;
  window_t w;
  action_code_t error = E_SUCCESS;

  tmp = (HEX_COLS * BYTES_PER_GROUP) - (user_prefs.grouping * BYTE_DIGITS);
  w = display_info.cursor_window;

  switch (w)
  {
    case WINDOW_HEX:
      if (get_addr_from_xy(*x - BYTES_PER_GROUP, *y) == -1)
      {
        error = _action_cursor_move_up(y, &tmp);
        if (error == E_SUCCESS)
          *x = tmp;
      }
      else
        *x = *x - BYTES_PER_GROUP;
      break;
    case WINDOW_ASCII:
      *x = *x - BYTES_PER_GROUP;
      if (get_addr_from_xy(*x - BYTES_PER_GROUP, *y) == -1)
      {
        error = _action_cursor_move_up(y, &tmp);
        if (error == E_SUCCESS)
          *x = tmp;
      }
      break;
    default:
      error = E_INVALID;
  }

  return error;
}
action_code_t action_cursor_move_left(void)
{
  int x,y;
  window_t w;
  action_code_t error = E_SUCCESS;

  w = display_info.cursor_window;
  getyx(window_list[w], y, x);

  error = _action_cursor_move_left(&y, &x);

  if (error == E_SUCCESS)
    wmove(window_list[w], y, x);

  return error;
}

/** RIGHT **/
action_code_t _action_cursor_move_right(int *y, int *x)
{
  int tmp;
  window_t w;
  action_code_t error = E_SUCCESS;

  tmp = 1;
  w = display_info.cursor_window;

  switch (w)
  {
    case WINDOW_HEX:
      if (get_addr_from_xy(*x + BYTES_PER_GROUP, *y) == -1)
      {
        error = _action_cursor_move_down(y, &tmp);
        if (error == E_SUCCESS)
          *x = tmp;
      }
      else
        *x = *x + BYTES_PER_GROUP;
      break;
    case WINDOW_ASCII:
      x+=BYTES_PER_GROUP;
      if (*x > (HEX_COLS * BYTES_PER_GROUP))
      {
        error = _action_cursor_move_down(y, &tmp);
        if (error == E_SUCCESS)
          *x=1;
        else
          error = E_NO_ACTION;
      }
      break;
    default:
      error = E_INVALID;
  }

  return error;
}
action_code_t action_cursor_move_right(void)
{
  int x,y,tmp = 1;
  window_t w;
  action_code_t error = E_SUCCESS;

  w = display_info.cursor_window;
  getyx(window_list[w], y, x);

  mvwprintw(window_list[WINDOW_STATUS], 0, 10, "%03d", x);
  error = _action_cursor_move_right(&y, &x);
  mvwprintw(window_list[WINDOW_STATUS], 0, 20, "%03d", x);
  mvwprintw(window_list[WINDOW_STATUS], 0, 30, "page_end = %d, PAGE_END = %d", display_info.page_end, PAGE_END);

  if (error == E_SUCCESS)
    wmove(window_list[w], y, x);

  return E_SUCCESS;
}
action_code_t action_page_down(void)
{
}
action_code_t action_page_up(void)
{
}
action_code_t action_jump_to(void)
{
}
action_code_t action_align_top(void)
{
}
action_code_t action_align_middle(void)
{
}
action_code_t action_align_bottom(void)
{
}
action_code_t action_delete(void)
{
}
action_code_t action_insert_before(void)
{
}
action_code_t action_insert_after(void)
{
}
action_code_t action_append(void)
{
}
action_code_t action_replace(void)
{
}
action_code_t action_replace_insert(void)
{
}
action_code_t action_save(void)
{
}
action_code_t action_save_as(void)
{
}
action_code_t action_discard_changes(void)
{
}
action_code_t action_close_file(void)
{
}
action_code_t action_open_file(void)
{
}
action_code_t action_exit(void)
{
}
action_code_t action_cursor_to_hex(void)
{
}
action_code_t action_cursor_to_ascii(void)
{
}
action_code_t action_cursor_toggle_hex_ascii(void)
{
}
action_code_t action_cursor_move_address(void)
{
}
action_code_t action_visual_select_on(void)
{
}
action_code_t action_visual_select_off(void)
{
}
action_code_t action_block_visual_select_on(void)
{
}
action_code_t action_block_visual_select_off(void)
{
}

action_code_t cmd_parse(int *cbuff, int count)
{
  int i = 0;
  for (i=0; i<count; i++)
    mvwaddch(window_list[WINDOW_STATUS], 0, 50+i, cbuff[i]);
}

action_code_t do_cmd_line(int c)
{
#define MAX_CMD_BUF 256
#define CR      '\r'
#define NL      '\n'
#define ESC     27
#define BVICTRL(n)    (n&0x1f)

  int cbuff[MAX_CMD_BUF];
  int i = 0, count = 0, position = 0;

  werase(window_list[WINDOW_STATUS]);
  mvwaddch(window_list[WINDOW_STATUS], 0, 0, c);

  do
  {
    update_panels();
    doupdate();
    c = getch();
    switch(c)
    {
      case ESC:
        return E_NO_ACTION;
      case BVICTRL('H'):
      case KEY_BACKSPACE:
        mvwaddch(window_list[WINDOW_STATUS], 0, position, ' ');
        wmove(window_list[WINDOW_STATUS], 0, position);
        position--;
        count--;
        break;
      case KEY_LEFT:
        if (--position < 0)
          position++;
        wmove(window_list[WINDOW_STATUS], 0, position+1);
        break;
      case KEY_RIGHT:
        if (++position > count)
          position--;
        wmove(window_list[WINDOW_STATUS], 0, position+1);
        break;
      case NL:
      case CR:
      case KEY_ENTER:
        break;
      default:
        for (i=count; i>=position; i--)
          cbuff[i+1] = cbuff[i];
        cbuff[position] = c;
        count++;
        for (i=position; i<count; i++)
          mvwaddch(window_list[WINDOW_STATUS], 0, i+1, cbuff[i]);
        position++;
        wmove(window_list[WINDOW_STATUS], 0, position+1);
        break;
    }
  } while(c != NL && c != CR && c != KEY_ENTER);

  cmd_parse(cbuff, count);

  return E_SUCCESS;
}

void handle_key(int c)
{
  int x, y;

  mvwprintw(window_list[WINDOW_STATUS], 0, 0, "%c", c);
  mvwprintw(window_list[WINDOW_STATUS], 0, 100, "HEX_COLS = %d, BYTES_PER_GROUP = %d, a*b = %d", HEX_COLS, BYTES_PER_GROUP, (HEX_COLS * BYTES_PER_GROUP));
  switch (c)
  {
    case 'j':
    case KEY_DOWN:
      action_cursor_move_down();
      break;
    case 'k':
    case KEY_UP:
      action_cursor_move_up();
      break;
    case 'h':
    case KEY_LEFT:
      action_cursor_move_left();
      break;
    case 'l':
    case KEY_RIGHT:
      action_cursor_move_right();
      break;
    case ':':
      do_cmd_line(c);
      break;
    default:
      break;
  }
}

/* global for vis select on,
 * modify screen print so that:
 * if (cursor < vis_byte) {
 *   if (byte >= cursor && byte <= vis_byte)
 *      set highlight on
 *   else
 *      set highlight off
 * }
 * else {
 *   if (byte <= cursor && byte >= vis_byte) {
 *      set highlight on
 *   else
 *      set highlight off
 *   }
 * }
 * Define separate actions for each thing (cut, copy, etc.) if vis select is on (operate on range)
 * Or, always operate on range, and if vis select is set update range every time the cursor moves
 * fix up/down in case of going off the screen!
 * use buffering for the screen mem, will help for doing group inserts
 */

