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
action_code_t action_move_to_hex(void)
{
}
action_code_t action_move_to_ascii(void)
{
}
action_code_t action_toggle_hex_ascii(void)
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

void handle_key(int c)
{
  int x, y;

  mvwprintw(window_list[WINDOW_STATUS], 0, 0, "%c", c);
  mvwprintw(window_list[WINDOW_STATUS], 0, 100, "HEX_COLS = %d, BYTES_PER_GROUP = %d, a*b = %d", HEX_COLS, BYTES_PER_GROUP, (HEX_COLS * BYTES_PER_GROUP));
  switch (c)
  {
    case KEY_DOWN:
      action_cursor_move_down();
      break;
    case KEY_UP:
      action_cursor_move_up();
      break;
    case KEY_LEFT:
      action_cursor_move_left();
      break;
    case KEY_RIGHT:
      action_cursor_move_right();
      break;
    default:
      break;
  }
}


