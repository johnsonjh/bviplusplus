#include <ncurses.h>
#include <string.h>
#include "key_handler.h"
#include "windows.h"
#include "user_prefs.h"
#include "display.h"
#include "actions.h"
#include "app_state.h"

typedef enum action_code_e
{
  E_SUCCESS,
  E_NO_ACTION,
  E_INVALID,
} action_code_t;

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


#define MAX_CMD_BUF 256
#define CR      '\r'
#define NL      '\n'
#define ESC     27
#define TAB     9
#define BVICTRL(n)    (n&0x1f)

action_code_t show_set(void)
{
  action_code_t error = E_SUCCESS;
  msg_box("show set");
  return error;
}

action_code_t do_set(void)
{
  action_code_t error = E_SUCCESS;
  char *tok;
  const char delimiters[] = " =";
  int option, set = 1;
  long value;

  tok = strtok(NULL, delimiters);

  if (tok == NULL)
  {
    error = show_set();
    return error;
  }

  if (strncmp(tok, "no", 2) == 0)
  {
    tok += 2;
    set = 0;
  }

  option = 0;
  while (strncmp(user_prefs[option].name, "", MAX_CMD_BUF))
  {
    if (strncmp(user_prefs[option].name,        tok, MAX_CMD_BUF) == 0 ||
        strncmp(user_prefs[option].short_name,  tok, MAX_CMD_BUF) == 0)
    {

      if (user_prefs[option].flags == P_BOOL)
      {
        user_prefs[option].value = set;
        break;
      }

      if (user_prefs[option].flags == P_LONG)
      {
        tok = strtok(NULL, delimiters);

        if (tok == NULL)
        {
          msg_box("Not enough parameters to 'set %s'",
                  user_prefs[option].name);
          return E_INVALID;
        }

        value = atol(tok);

        if ((value < user_prefs[option].min && user_prefs[option].min) ||
            (value > user_prefs[option].max && user_prefs[option].max))
        {
          msg_box("Value out of range for 'set %s' (min = %d, max = %d)",
                  user_prefs[option].name,
                  user_prefs[option].min,
                  user_prefs[option].max);
          return E_INVALID;
        }

        user_prefs[option].value = value;
        break;
      }

    }

    option++;

  }

  update_display_info();
  print_screen(display_info.page_start);

  return error;
}

action_code_t cmd_parse(char *cbuff)
{
  action_code_t error = E_SUCCESS;
  char *tok;
  const char delimiters[] = " =";

  tok = strtok(cbuff, delimiters);
  while (tok != NULL)
  {
    if (strncmp(tok, "set", MAX_CMD_BUF) == 0)
    {
      error = do_set();
    }
    if (strncmp(tok, "q", MAX_CMD_BUF) == 0)
    {
      app_state.quit = TRUE;
    }

    tok = strtok(NULL, delimiters);
  }
  return error;
}

action_code_t do_cmd_line(int c)
{
  char cbuff[MAX_CMD_BUF];
  int i = 0, count = 0, position = 0, tab_complete_len = 0;

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
        cbuff[position] = (char)c;
        count++;
        for (i=position; i<count; i++)
          mvwaddch(window_list[WINDOW_STATUS], 0, i+1, cbuff[i]);
        position++;
        wmove(window_list[WINDOW_STATUS], 0, position+1);
        break;
    }
  } while(c != NL && c != CR && c != KEY_ENTER);

  cbuff[count] = '\0';

  cmd_parse(cbuff);

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
    case '$':
    case KEY_END:
      action_cursor_move_line_end();
      break;
    case '0':
    case KEY_HOME:
      action_cursor_move_line_start();
      break;
    case TAB:
      action_cursor_toggle_hex_ascii();
      break;
    case BVICTRL('f'):
    case KEY_NPAGE:
      action_page_down();
      break;
    case BVICTRL('b'):
    case KEY_PPAGE:
      action_page_up();
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
 * Remember to add tab completion, marks, macros, and a good system for command line parsing, and a good system for settings and .rc files
 * Add options: columns, search hl, search ignorecase
 * Check bvi man page for min list of command line commands to support
 */

