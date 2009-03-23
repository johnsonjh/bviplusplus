#include "actions.h"
#include "display.h"
#include "virt_file.h"
#include "app_state.h"

#define MARK_LIST_SIZE (26*2)
off_t mark_list[MARK_LIST_SIZE ];


/** UP **/
action_code_t action_cursor_move_up(int count)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  if (count == 0)
    count = 1;

  a = display_info.cursor_addr;
  a -= BYTES_PER_LINE * count;

  while (address_invalid(a))
    a += BYTES_PER_LINE;

  if (a >= display_info.cursor_addr)
    error = E_NO_ACTION;
  else
    place_cursor(a, CALIGN_NONE);

  return error;
}

/** DOWN **/
action_code_t action_cursor_move_down(int count)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  if (count == 0)
    count = 1;

  a = display_info.cursor_addr;
  a += BYTES_PER_LINE * count;

  while (address_invalid(a))
    a -= BYTES_PER_LINE;

  if (address_invalid(a))
    error = E_NO_ACTION;
  else
    place_cursor(a, CALIGN_NONE);

  return error;
}

/** LEFT **/
action_code_t action_cursor_move_left(int count)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  if (count == 0)
    count = 1;

  a = display_info.cursor_addr;
  a -= user_prefs[GROUPING].value * count;

  while (address_invalid(a))
    a +=  user_prefs[GROUPING].value;

  if (address_invalid(a))
    error = E_NO_ACTION;
  else
    place_cursor(a, CALIGN_NONE);

  return error;
}

/** RIGHT **/
action_code_t action_cursor_move_right(int count)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  if (count == 0)
    count = 1;

  a = display_info.cursor_addr;
  a += user_prefs[GROUPING].value * count;

  while (address_invalid(a))
    a -=  user_prefs[GROUPING].value;

  if (address_invalid(a))
    error = E_NO_ACTION;
  else
    place_cursor(a, CALIGN_NONE);

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

action_code_t action_cursor_move_line_end(void)
{
  action_code_t error = E_SUCCESS;
  int y;
  off_t a;

  y = get_y_from_addr(display_info.cursor_addr);
  a = get_addr_from_xy(1, y);
  a += (HEX_COLS-1) * user_prefs[GROUPING].value;

  if (address_invalid(a))
    a = display_info.file_size - 1;

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
  place_cursor(display_info.file_size-1, CALIGN_NONE);
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
action_code_t action_jump_to(off_t jump_addr)
{
  action_code_t error = E_SUCCESS;
  place_cursor(jump_addr, CALIGN_MIDDLE);
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
action_code_t action_delete(int count)
{
  action_code_t error = E_SUCCESS;

  if (count == 0)
    count = 1;

  vf_delete(current_file, display_info.cursor_addr, count);
  print_screen(display_info.page_start);

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
action_code_t action_search_highlight(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_clear_search_highlight(void)
{
  action_code_t error = E_SUCCESS;
  return error;
}

off_t action_get_mark(int m)
{
  int index;

  index = tolower(m);
  if (index != m)
    index += 26;
  index -= 'a';

  mvwprintw(window_list[WINDOW_STATUS], 0, 30, "get index = %02d", index);

  if (index < 0 || index >= MARK_LIST_SIZE)
    return -1;

  return mark_list[index];
}

action_code_t action_set_mark(int m)
{
  action_code_t error = E_SUCCESS;
  int index;

  index = tolower(m);
  if (index != m)
    index += 26;
  index -= 'a';

  mvwprintw(window_list[WINDOW_STATUS], 0, 30, "set index = %02d", index);

  if (index < 0 || index >= MARK_LIST_SIZE)
    error = E_INVALID;
  else
    mark_list[index] = display_info.cursor_addr;;

  return error;
}

