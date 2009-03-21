#include "actions.h"
#include "display.h"

/** UP **/
action_code_t action_cursor_move_up(void)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  a = display_info.cursor_addr;
  a -= BYTES_PER_LINE;

  if (address_invalid(a) == 0)
    place_cursor(a, CALIGN_NONE);
  else
    error = E_NO_ACTION;

  return error;
}

/** DOWN **/
action_code_t action_cursor_move_down(void)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  a = display_info.cursor_addr;
  a += BYTES_PER_LINE;

  if (address_invalid(a) == 0)
    place_cursor(a, CALIGN_NONE);
  else
    error = E_NO_ACTION;

  return error;
}

/** LEFT **/
action_code_t action_cursor_move_left(void)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  a = display_info.cursor_addr;
  a -= user_prefs[GROUPING].value;

  if (address_invalid(a) == 0)
    place_cursor(a, CALIGN_NONE);
  else
    error = E_NO_ACTION;

  return error;
}

/** RIGHT **/
action_code_t action_cursor_move_right(void)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  a = display_info.cursor_addr;
  mvwprintw(window_list[WINDOW_STATUS], 0, 10, "%04x", a);
  a += user_prefs[GROUPING].value;
  mvwprintw(window_list[WINDOW_STATUS], 0, 16, "%04x", a);

  if (address_invalid(a) == 0)
    place_cursor(a, CALIGN_NONE);
  else
    error = E_NO_ACTION;

  return error;
}

action_code_t action_cursor_move_line_start(void)
{
  action_code_t error = E_SUCCESS;
  int x, y;
  off_t a;

  y = get_y_from_addr(display_info.cursor_addr);
  x = 1;

  a = get_addr_from_xy(x, y);
  place_cursor(a, CALIGN_NONE);

  return error;
}

action_code_t action_cursor_move_file_start(void)
{
  action_code_t error = E_SUCCESS;
  place_cursor(0, CALIGN_NONE);
  return error;
}
action_code_t action_cursor_move_file_end(void)
{
  action_code_t error = E_SUCCESS;
  place_cursor(display_info.file_size, CALIGN_NONE);
  return error;
}

action_code_t action_cursor_move_line_end(void)
{
  action_code_t error = E_SUCCESS;
  int x, y;
  off_t a;

  y = get_y_from_addr(display_info.cursor_addr);
  x = ((HEX_COLS - 1) * BYTES_PER_GROUP) + 1;

  a = get_addr_from_xy(x, y);
  place_cursor(a, CALIGN_NONE);

  return error;
}

action_code_t action_page_down(void)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  a = display_info.cursor_addr;
  a += BYTES_PER_LINE * HEX_LINES;

  if (address_invalid(a))
    a = (display_info.file_size - 1);

  place_cursor(a, CALIGN_NONE);

  return error;
}
action_code_t action_page_up(void)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  a = display_info.cursor_addr;
  a -= BYTES_PER_LINE * HEX_LINES;

  if (address_invalid(a))
    a = 0;

  place_cursor(a, CALIGN_NONE);

  return error;
}
action_code_t action_jump_to(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_align_top(void)
{
  action_code_t error = E_SUCCESS;
  place_cursor(display_info.cursor_addr, CALIGN_TOP);
  return error;
}
action_code_t action_align_middle(void)
{
  action_code_t error = E_SUCCESS;
  place_cursor(display_info.cursor_addr, CALIGN_MIDDLE);
  return error;
}
action_code_t action_align_bottom(void)
{
  action_code_t error = E_SUCCESS;
  place_cursor(display_info.cursor_addr, CALIGN_BOTTOM);
  return error;
}
action_code_t action_delete(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_insert_before(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_insert_after(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_append(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_replace(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_replace_insert(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_save(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_save_as(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_discard_changes(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_close_file(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_open_file(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_exit(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_cursor_to_hex(void)
{
  action_code_t error = E_SUCCESS;

  display_info.cursor_window = WINDOW_HEX;
  place_cursor(display_info.cursor_addr, CALIGN_NONE);

  return error;
}
action_code_t action_cursor_to_ascii(void)
{
  action_code_t error = E_SUCCESS;

  display_info.cursor_window = WINDOW_ASCII;
  place_cursor(display_info.cursor_addr, CALIGN_NONE);

  return error;
}
action_code_t action_cursor_toggle_hex_ascii(void)
{
  action_code_t error = E_SUCCESS;

  if (display_info.cursor_window == WINDOW_HEX)
    error = action_cursor_to_ascii();
  else
    error = action_cursor_to_hex();

  return error;
}
action_code_t action_cursor_move_address(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_visual_select_on(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_visual_select_off(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_block_visual_select_on(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_block_visual_select_off(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}


