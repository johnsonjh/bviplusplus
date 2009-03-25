#include <stdlib.h>
#include "actions.h"
#include "display.h"
#include "virt_file.h"
#include "app_state.h"

#define MARK_LIST_SIZE (26*2)
#define NUM_YANK_REGISTERS (26*2 + 10)

typedef struct yank_buf_s
{
  char *buf;
  int len;
} yank_buf_t;

static off_t mark_list[MARK_LIST_SIZE];
static yank_buf_t yank_buf[NUM_YANK_REGISTERS];
static int yank_register = 0;


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
  off_t addr;

  if (is_visual_on())
  {
    count = visual_span();
    addr = visual_addr();
    action_visual_select_off();
    place_cursor(addr, CALIGN_NONE);
  }
  else
  {
    if (count == 0)
      count = 1;
    addr = display_info.cursor_addr;
  }

  if (address_invalid(addr) == 0)
  {
    vf_delete(current_file, addr, count);
    update_display_info();
    print_screen(display_info.page_start);
  }

  return error;
}
action_code_t action_insert_before(int count, char *buf, int len)
{
  action_code_t error = E_SUCCESS;
  if (is_visual_on())
    return E_INVALID;
  vf_insert_before(current_file, buf, display_info.cursor_addr, len);
  place_cursor(display_info.cursor_addr + len - 1, CALIGN_NONE);
  print_screen(display_info.page_start);
  return error;
}

#define DEFAULT_INSERT_BUF_SIZE 512
int is_bin(c)
{
  if (c != '0' && c != '1')
    return 0;
  return 1;
}
int is_hex(int c)
{
  c = toupper(c);

  if (c < '0')
    return 0;
  if (c > '9' && c < 'A')
    return 0;
  if (c > 'F')
    return 0;

  return 1;

#if 0
  action_code_t error = E_SUCCESS;
  char *buf, *screen, tmp_buf[MAX_CMD_BUF];
  int c, buf_size = DEFAULT_INSERT_BUF_SIZE, insert_len = 0, screen_size;

  buf = (char *)malloc(buf_size);
  screen_size = HEX_COLS * user_prefs[GROUPING].value * HEX_LINES;
  screen = (char *)malloc(screen_size);

  vf_get_buf(current_file, screen, display_info.page_start, screen_size);

  /* create a blank at addr and move the rest of the screen down a byte */

  c = getch();
  while (c != ESC && c != KEY_ENTER)
  {
    if (display_info.cursor_window == WINDOW_HEX)
    {
      if (user_prefs[DISPLAY_BINARY].value)
      {
        if (is_bin(c))
      }
      else
      {
        if (is_hex(c))
          mvwaddch(window_list[WINDOW_HEX], y, x, c);
      }
    }
  }
#endif


}

action_code_t action_insert_after(int count, char *buf, int len)
{
  action_code_t error = E_SUCCESS;
  if (is_visual_on())
    return E_INVALID;
  vf_insert_after(current_file, buf, display_info.cursor_addr, len);
  place_cursor(display_info.cursor_addr + len, CALIGN_NONE);
  print_screen(display_info.page_start);
  return error;
}

action_code_t action_set_yank_register(int c)
{
  action_code_t error = E_SUCCESS;

  if (c >= '0' && c <= '9')
  {
    c -= '0';
    yank_register = c;
  }
  else if (c >= 'A' && c <= 'Z')
  {
    c -= 'A';
    yank_register = c;
  }
  else if (c >= 'a' && c <= 'z')
  {
    c -= 'a';
    c += 26;
    yank_register = c;
  }
  else
  {
    error = E_INVALID;
  }
  return error;
}

action_code_t action_paste_before(int count)
{
  action_code_t error = E_SUCCESS;
  int i;

  if (is_visual_on())
    return E_INVALID;

  if (address_invalid(display_info.cursor_addr))
  {
    if (display_info.file_size == 0)
    {
      for (i=0; i<count; i++)
      {
        vf_insert_before(current_file,
                         yank_buf[yank_register].buf,
                         0,
                         yank_buf[yank_register].len);
      }
    }
    else
    {
      error = E_INVALID;
    }
  }
  else
  {
    for (i=0; i<count; i++)
    {
      vf_insert_before(current_file,
                       yank_buf[yank_register].buf,
                       display_info.cursor_addr,
                       yank_buf[yank_register].len);
    }
  }

  return error;
}
action_code_t action_paste_after(int count)
{
  action_code_t error = E_SUCCESS;
  int i;

  if (is_visual_on())
    return E_INVALID;

  if (address_invalid(display_info.cursor_addr))
  {
    if (display_info.file_size == 0)
    {
      for (i=0; i<count; i++)
      {
        vf_insert_before(current_file,
                         yank_buf[yank_register].buf,
                         0,
                         yank_buf[yank_register].len);
      }
    }
    else
    {
      error = E_INVALID;
    }
  }
  else
  {
    for (i=0; i<count; i++)
    {
      vf_insert_after(current_file,
                      yank_buf[yank_register].buf,
                      display_info.cursor_addr,
                      yank_buf[yank_register].len);
    }
  }

  return error;
}

action_code_t action_init_yank(void)
{
  action_code_t error = E_SUCCESS;
  int i;

  for (i=0; i<NUM_YANK_REGISTERS; i++)
  {
    yank_buf[i].buf = NULL;
    yank_buf[i].len = 0;
  }

  return error;
}
action_code_t action_clean_yank(void)
{
  action_code_t error = E_SUCCESS;
  int i;

  for (i=0; i<NUM_YANK_REGISTERS; i++)
  {
    if (yank_buf[i].len != 0)
      free(yank_buf[i].buf);

    yank_buf[i].buf = NULL;
    yank_buf[i].len = 0;
  }

  return error;
}

action_code_t action_yank(int count)
{
  action_code_t error = E_SUCCESS;
  
  return error;
}
action_code_t action_cut(int count)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_append(void)
{
  action_code_t error = E_SUCCESS;
  if (is_visual_on())
    return E_INVALID;
  return error;
}
action_code_t action_replace(int count)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_replace_insert(int count)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_overwrite(int count)
{
  action_code_t error = E_SUCCESS;
  return error;
}
action_code_t action_save(void)
{
  action_code_t error = E_SUCCESS;
  if (is_visual_on())
    return E_INVALID;
  return error;
}
action_code_t action_save_as(void)
{
  action_code_t error = E_SUCCESS;
  if (is_visual_on())
    return E_INVALID;
  return error;
}
action_code_t action_discard_changes(void)
{
  action_code_t error = E_SUCCESS;
  if (is_visual_on())
    return E_INVALID;
  return error;
}
action_code_t action_close_file(void)
{
  action_code_t error = E_SUCCESS;
  if (is_visual_on())
    return E_INVALID;
  return error;
}
action_code_t action_open_file(void)
{
  action_code_t error = E_SUCCESS;
  if (is_visual_on())
    return E_INVALID;
  return error;
}
action_code_t action_exit(void)
{
  action_code_t error = E_SUCCESS;
  if (is_visual_on())
    return E_INVALID;
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
  display_info.visual_select_addr = display_info.cursor_addr;
  return error;
}
action_code_t action_visual_select_off(void)
{
  action_code_t error = E_SUCCESS;
  display_info.visual_select_addr = -1;
  print_screen(display_info.page_start);
  return error;
}
action_code_t action_visual_select_toggle(void)
{
  action_code_t error = E_SUCCESS;

  if (display_info.visual_select_addr == -1)
    error = action_visual_select_on();
  else
    error = action_visual_select_off();

  return error;
}
action_code_t action_search_highlight(void)
{
  action_code_t error = E_SUCCESS;
  if (is_visual_on())
    return E_INVALID;
  return error;
}
action_code_t action_clear_search_highlight(void)
{
  action_code_t error = E_SUCCESS;
  if (is_visual_on())
    return E_INVALID;
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
action_code_t action_undo(int count)
{
  action_code_t error = E_SUCCESS;
  off_t caddr;

  if (is_visual_on())
    return E_INVALID;

  if (count == 0)
    count = 1;

  vf_undo(current_file, count, &caddr);
  update_display_info();
  place_cursor(caddr, CALIGN_NONE);
  print_screen(display_info.page_start);
  return error;
}
action_code_t action_redo(int count)
{
  action_code_t error = E_SUCCESS;
  off_t caddr;

  if (is_visual_on())
    return E_INVALID;

  if (count == 0)
    count = 1;

  vf_redo(current_file, count, &caddr);
  update_display_info();
  place_cursor(caddr, CALIGN_NONE);
  print_screen(display_info.page_start);
  return error;
}

