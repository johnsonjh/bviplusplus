#include <stdlib.h>
#include <string.h>
#include "actions.h"
#include "display.h"
#include "virt_file.h"
#include "app_state.h"
#include "user_prefs.h"
#include "key_handler.h"

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
action_code_t action_cursor_move_up(int count, cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  if (count == 0)
    count = 1;

  a = display_info.cursor_addr;
  a -= BYTES_PER_LINE * count;

  while (a < 0)
    a += BYTES_PER_LINE;

  if (a >= display_info.cursor_addr)
    error = E_NO_ACTION;
  else
    place_cursor(a, CALIGN_NONE, cursor);

  return error;
}

/** DOWN **/
action_code_t action_cursor_move_down(int count, cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  if (count == 0)
    count = 1;

  a = display_info.cursor_addr;
  a += BYTES_PER_LINE * count;

  while (a > display_info.file_size)
    a -= BYTES_PER_LINE;

  if (address_invalid(a))
    error = E_NO_ACTION;
  else
    place_cursor(a, CALIGN_NONE, cursor);

  return error;
}

/** LEFT **/
action_code_t action_cursor_move_left(int count, cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  if (count == 0)
    count = 1;

  a = display_info.cursor_addr;
  a -= user_prefs[GROUPING].value * count;

  while (a < 0)
    a +=  user_prefs[GROUPING].value;

  if (address_invalid(a))
    error = E_NO_ACTION;
  else
    place_cursor(a, CALIGN_NONE, cursor);

  return error;
}

/** RIGHT **/
action_code_t action_cursor_move_right(int count, cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  if (count == 0)
    count = 1;

  a = display_info.cursor_addr;
  a += user_prefs[GROUPING].value * count;

  while (a > display_info.file_size)
    a -=  user_prefs[GROUPING].value;

  if (address_invalid(a))
    error = E_NO_ACTION;
  else
    place_cursor(a, CALIGN_NONE, cursor);

  return error;
}

action_code_t action_cursor_move_line_start(cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  int x, y;
  off_t a;

  y = get_y_from_addr(display_info.cursor_addr);
  x = 1;

  a = get_addr_from_xy(x, y);
  place_cursor(a, CALIGN_NONE, cursor);

  return error;
}

action_code_t action_cursor_move_line_end(cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  int y;
  off_t a;

  y = get_y_from_addr(display_info.cursor_addr);
  a = get_addr_from_xy(1, y);
  a += (HEX_COLS-1) * user_prefs[GROUPING].value;

  if (address_invalid(a))
    a = display_info.file_size - 1;

  place_cursor(a, CALIGN_NONE, cursor);

  return error;
}

action_code_t action_cursor_move_file_start(cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  place_cursor(0, CALIGN_NONE, cursor);
  return error;
}
action_code_t action_cursor_move_file_end(cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  place_cursor(display_info.file_size-1, CALIGN_NONE, cursor);
  return error;
}

action_code_t action_cursor_move_page_down(cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  a = display_info.cursor_addr;
  a += BYTES_PER_LINE * HEX_LINES;

  if (address_invalid(a))
    a = (display_info.file_size - 1);

  place_cursor(a, CALIGN_NONE, cursor);

  return error;
}
action_code_t action_cursor_move_page_up(cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  off_t a;

  a = display_info.cursor_addr;
  a -= BYTES_PER_LINE * HEX_LINES;

  if (address_invalid(a))
    a = 0;

  place_cursor(a, CALIGN_NONE, cursor);

  return error;
}
action_code_t action_jump_to(off_t jump_addr, cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  place_cursor(jump_addr, CALIGN_MIDDLE, cursor);
  return error;
}
action_code_t action_align_top(void)
{
  action_code_t error = E_SUCCESS;
  place_cursor(display_info.cursor_addr, CALIGN_TOP, CURSOR_REAL);
  return error;
}
action_code_t action_align_middle(void)
{
  action_code_t error = E_SUCCESS;
  place_cursor(display_info.cursor_addr, CALIGN_MIDDLE, CURSOR_REAL);
  return error;
}
action_code_t action_align_bottom(void)
{
  action_code_t error = E_SUCCESS;
  place_cursor(display_info.cursor_addr, CALIGN_BOTTOM, CURSOR_REAL);
  return error;
}
action_code_t action_delete(int count, off_t end_addr)
{
  action_code_t error = E_SUCCESS;
  off_t addr;
  int range;

  if (is_visual_on())
  {
    count = visual_span();
    addr = visual_addr();
  }
  else
  {
    if (count == 0)
      count = user_prefs[GROUPING].value;

    if (end_addr != INVALID_ADDR)
    {
      if (end_addr > display_info.cursor_addr)
      {
        addr = display_info.cursor_addr;
        range = end_addr - display_info.cursor_addr + 1;
        count *= range;
      }
      else
      {
        addr = end_addr;
        range = display_info.cursor_addr - end_addr + 1;
        count *= range;
      }
    }
    else
    {
      addr = display_info.cursor_addr;
    }
  }

  if (address_invalid(addr) == 0)
  {
    while(address_invalid(addr+count-1) && count > 0)
      count--;

    vf_delete(current_file, addr, count);
    update_display_info();

    if (address_invalid(addr))
      addr = display_info.file_size - 1;
    if (address_invalid(addr))
      addr = 0;

    place_cursor(addr, CALIGN_NONE, CURSOR_REAL);

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
  update_display_info();
  place_cursor(display_info.cursor_addr + len - 1, CALIGN_NONE, CURSOR_REAL);
  print_screen(display_info.page_start);
  return error;
}

action_code_t action_insert_after(int count, char *buf, int len)
{
  action_code_t error = E_SUCCESS;
  off_t ins_addr;
  if (is_visual_on())
    return E_INVALID;
  ins_addr = display_info.cursor_addr + user_prefs[GROUPING].value - 1;
  if (ins_addr >= display_info.file_size)
    ins_addr = display_info.file_size - 1;
  vf_insert_after(current_file, buf, ins_addr, len);
  update_display_info();
  place_cursor(display_info.cursor_addr + len, CALIGN_NONE, CURSOR_REAL);
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

  if (count == 0)
    count = 1;

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

  update_display_info();
  place_cursor(display_info.cursor_addr + yank_buf[yank_register].len - 1,
               CALIGN_NONE, CURSOR_REAL);
  print_screen(display_info.page_start);

  return error;
}
action_code_t action_paste_after(int count)
{
  action_code_t error = E_SUCCESS;
  int i;

  if (is_visual_on())
    return E_INVALID;

  if (count == 0)
    count = 1;

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

  update_display_info();
  place_cursor(display_info.cursor_addr + yank_buf[yank_register].len,
               CALIGN_NONE, CURSOR_REAL);
  print_screen(display_info.page_start);

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

action_code_t action_yank(int count, off_t end_addr, BOOL move_cursor)
{
  action_code_t error = E_SUCCESS;
  off_t addr;
  int range;

  if (is_visual_on())
  {
    count = visual_span();
    addr = visual_addr();
  }
  else
  {
    if (count == 0)
      count = user_prefs[GROUPING].value;

    if (end_addr != INVALID_ADDR)
    {
      if (end_addr > display_info.cursor_addr)
      {
        addr = display_info.cursor_addr;
        range = end_addr - display_info.cursor_addr + 1;
        count *= range;
      }
      else
      {
        addr = end_addr;
        range = display_info.cursor_addr - end_addr + 1;
        count *= range;
      }
    }
    else
    {
      addr = display_info.cursor_addr;
    }
  }

  if (address_invalid(addr) == 0)
  {
    while(address_invalid(addr+count-1) && count > 0)
      count--;

    if (yank_buf[yank_register].len != 0)
      free(yank_buf[yank_register].buf);

    yank_buf[yank_register].buf = malloc(count);
    yank_buf[yank_register].len = count;

    vf_get_buf(current_file, yank_buf[yank_register].buf, addr, count);

    if (move_cursor)
      place_cursor(addr, CALIGN_NONE, CURSOR_REAL);

  }

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
  place_cursor(display_info.cursor_addr, CALIGN_NONE, CURSOR_REAL);

  return error;
}
action_code_t action_cursor_to_ascii(void)
{
  action_code_t error = E_SUCCESS;

  display_info.cursor_window = WINDOW_ASCII;
  place_cursor(display_info.cursor_addr, CALIGN_NONE, CURSOR_REAL);

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

int action_visual_select_check(void)
{
  return is_visual_on();
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
  if (address_invalid(caddr))
    caddr = display_info.file_size - 1;
  if (caddr < 0)
    caddr = 0;
  place_cursor(caddr, CALIGN_NONE, CURSOR_REAL);
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
  place_cursor(caddr, CALIGN_NONE, CURSOR_REAL);
  print_screen(display_info.page_start);
  return error;
}

BOOL file_name_prompt(char *file_name)
{
  WINDOW *w;
  int c, i, y, position = 0, count = 0;
  char cbuff[MAX_FILE_NAME];

  w = newwin(FILE_BOX_H, FILE_BOX_W, FILE_BOX_Y, FILE_BOX_X);
  box(w, 0, 0);
  y = FILE_BOX_H-2;
  mvwprintw(w, y, 1, "Enter name of file [ESC] cancel, [ENTER] done");
  y--;
  wmove(w, y, 1);
  wrefresh(w);

  c = getch();
  while (c != ESC && c != NL && c != CR && c != KEY_ENTER)
  {
    switch(c)
    {
      case KEY_RESIZE:
        break;
      case BVICTRL('H'):
      case KEY_BACKSPACE:
        if (position == 0)
          break;
        mvwaddch(w, y, position, ' ');

        for (i=position; i<count; i++)
          mvwaddch(w, y, i, cbuff[i]);
        wclrtoeol(w);

        box(w, 0, 0);
        wmove(w, y, position);
        position--;
        count--;
        break;
      case KEY_LEFT:
        if (--position < 0)
          position++;
        wmove(w, y, position+1);
        break;
      case KEY_RIGHT:
        if (++position > count)
          position--;
        wmove(w, y, position+1);
        break;
      default:
        if (count >= MAX_FILE_NAME)
          break;
        for (i=count; i>=position; i--)
          cbuff[i+1] = cbuff[i];
        cbuff[position] = (char)c;
        count++;
        for (i=position; i<count; i++)
          mvwaddch(w, y, i+1, cbuff[i]);
        position++;
        wmove(w, y, position+1);
        break;
    }
    wrefresh(w);
    c = getch();
  }

  cbuff[count] = '\0';

  delwin(w);
  print_screen(display_info.page_start);

  if (c == ESC)
    return FALSE;

  if (count)
  {
    strncpy(file_name, cbuff, MAX_FILE_NAME);
    return TRUE;
  }

  return FALSE;
}

action_code_t action_save(void)
{
  action_code_t error = E_SUCCESS;
  int complete;
  char file_name[MAX_FILE_NAME];
  BOOL status;

  if (vf_need_create(current_file))
  {
    status = file_name_prompt(file_name);
    if (status == FALSE)
      return E_INVALID;
    vf_create_file(current_file, file_name);
    if (status == FALSE)
      return E_INVALID;
  }
  vf_save(current_file, NULL, &complete);
  return error;
}

action_code_t action_save_as(char *name)
{
  action_code_t error = E_SUCCESS;
  msg_box("\"Save As\"Not implimented yet, sucker!");
  return error;
}

action_code_t action_quit(BOOL force)
{
  action_code_t error = E_SUCCESS;
  if (vf_need_save(current_file) && force == FALSE)
    msg_box("File has unsaved chages");
  else
    app_state.quit = TRUE;
  return error;
}

action_code_t action_blob_shift_right(void)
{
  int new_blob_shift_value;

  new_blob_shift_value = user_prefs[BLOB_GROUPING_OFFSET].value;
  new_blob_shift_value++;

  if (new_blob_shift_value > user_prefs[BLOB_GROUPING_OFFSET].max)
    return E_INVALID;

  user_prefs[BLOB_GROUPING_OFFSET].max = new_blob_shift_value;

  print_screen(display_info.page_start);

  return E_SUCCESS;
}
action_code_t action_blob_shift_left(void)
{
  int new_blob_shift_value;

  new_blob_shift_value = user_prefs[BLOB_GROUPING_OFFSET].value;
  new_blob_shift_value--;

  if (new_blob_shift_value < user_prefs[BLOB_GROUPING_OFFSET].min)
    return E_INVALID;

  user_prefs[BLOB_GROUPING_OFFSET].max = new_blob_shift_value;

  print_screen(display_info.page_start);

  return E_SUCCESS;
}

action_code_t action_blob_inc(void)
{
  int new_blob_value;

  new_blob_value = user_prefs[BLOB_GROUPING].value;
  new_blob_value++;

  if (new_blob_value > user_prefs[BLOB_GROUPING].max)
    return E_INVALID;

  user_prefs[BLOB_GROUPING].max = new_blob_value;

  print_screen(display_info.page_start);

  return E_SUCCESS;
}
action_code_t action_blob_dec(void)
{
  int new_blob_value;

  new_blob_value = user_prefs[BLOB_GROUPING].value;
  new_blob_value--;

  if (new_blob_value < user_prefs[BLOB_GROUPING].min)
    return E_INVALID;

  user_prefs[BLOB_GROUPING].max = new_blob_value;

  print_screen(display_info.page_start);

  return E_SUCCESS;
}
action_code_t action_grp_inc(void)
{
  int new_grp_value;

  new_grp_value = user_prefs[GROUPING].value;
  new_grp_value++;

  if (new_grp_value > user_prefs[GROUPING].max)
    return E_INVALID;

  user_prefs[GROUPING].max = new_grp_value;

  print_screen(display_info.page_start);

  return E_SUCCESS;
}
action_code_t action_grp_dec(void)
{
  int new_grp_value;

  new_grp_value = user_prefs[GROUPING].value;
  new_grp_value--;

  if (new_grp_value < user_prefs[GROUPING].min)
    return E_INVALID;

  user_prefs[GROUPING].max = new_grp_value;

  print_screen(display_info.page_start);

  return E_SUCCESS;
}

action_code_t action_do_resize(void)
{
  action_code_t error = E_SUCCESS;
  update_display_info();
  destroy_screen();
  create_screen();
  print_screen(display_info.page_start);
  return error;
}

