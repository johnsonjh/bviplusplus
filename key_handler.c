#include <ncurses.h>
#include <string.h>
#include "key_handler.h"
#include "windows.h"
#include "user_prefs.h"
#include "display.h"
#include "actions.h"
#include "app_state.h"


#define MAX_CMD_BUF 256
#define MAX_CMD_HISTORY 100
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
          msg_box("Not enough parameters to 'set %s', using default value: %d",
                  user_prefs[option].name, user_prefs[option].def);
          user_prefs[option].value = user_prefs[option].def;
          break;
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

  //msg_box("%s", cbuff);

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

typedef struct cmd_hist_s
{
  char cbuff[MAX_CMD_BUF];
  int position;
  int count;
} cmd_hist_t;

action_code_t do_cmd_line(int s)
{
  static cmd_hist_t cmd_hist[MAX_CMD_HISTORY] = { 0 };
  cmd_hist_t tmp_cmd;
  static int hist_index = 0;
  char cmd[MAX_CMD_BUF];
  int entry_hist_index, tmp_hist_index, i = 0, c;

  werase(window_list[WINDOW_STATUS]);
  mvwaddch(window_list[WINDOW_STATUS], 0, 0, s);

  entry_hist_index = hist_index;
  tmp_cmd.count = 0;
  tmp_cmd.position = 0;

  do
  {
    update_panels();
    doupdate();
    c = getch();
    switch(c)
    {
      case KEY_UP:
        tmp_hist_index = hist_index-1;
        if (tmp_hist_index < 0)
          tmp_hist_index = MAX_CMD_HISTORY - 1;
        if (tmp_hist_index == entry_hist_index)
          break;
        if (cmd_hist[tmp_hist_index].count == 0)
          break;

        hist_index = tmp_hist_index;

        strncpy(tmp_cmd.cbuff, cmd_hist[hist_index].cbuff, MAX_CMD_BUF);
        tmp_cmd.count = cmd_hist[hist_index].count;
        tmp_cmd.position = cmd_hist[hist_index].position;

        werase(window_list[WINDOW_STATUS]);
        mvwaddch(window_list[WINDOW_STATUS], 0, 0, s);

        for (i=0; i<tmp_cmd.count; i++)
          mvwaddch(window_list[WINDOW_STATUS], 0, i+1, tmp_cmd.cbuff[i]);
        tmp_cmd.position = tmp_cmd.count;
        wmove(window_list[WINDOW_STATUS], 0, tmp_cmd.position+1);
        break;
      case KEY_DOWN:
        if (hist_index == entry_hist_index)
          break;
        tmp_hist_index = hist_index+1;
        tmp_hist_index = tmp_hist_index % MAX_CMD_HISTORY;

        hist_index = tmp_hist_index;

        strncpy(tmp_cmd.cbuff, cmd_hist[hist_index].cbuff, MAX_CMD_BUF);
        tmp_cmd.count = cmd_hist[hist_index].count;
        tmp_cmd.position = cmd_hist[hist_index].position;

        werase(window_list[WINDOW_STATUS]);
        mvwaddch(window_list[WINDOW_STATUS], 0, 0, s);

        for (i=0; i<tmp_cmd.count; i++)
          mvwaddch(window_list[WINDOW_STATUS], 0, i+1, tmp_cmd.cbuff[i]);
        tmp_cmd.position = tmp_cmd.count;
        wmove(window_list[WINDOW_STATUS], 0, tmp_cmd.position+1);
        break;
      case ESC:
        return E_NO_ACTION;
      case BVICTRL('H'):
      case KEY_BACKSPACE:
        if (tmp_cmd.position == 0)
          break;
        mvwaddch(window_list[WINDOW_STATUS], 0, tmp_cmd.position, ' ');
        wmove(window_list[WINDOW_STATUS], 0, tmp_cmd.position);
        tmp_cmd.position--;
        tmp_cmd.count--;
        break;
      case KEY_LEFT:
        if (--tmp_cmd.position < 0)
          tmp_cmd.position++;
        wmove(window_list[WINDOW_STATUS], 0, tmp_cmd.position+1);
        break;
      case KEY_RIGHT:
        if (++tmp_cmd.position > tmp_cmd.count)
          tmp_cmd.position--;
        wmove(window_list[WINDOW_STATUS], 0, tmp_cmd.position+1);
        break;
      case NL:
      case CR:
      case KEY_ENTER:
        break;
      default:
        if (tmp_cmd.count >= MAX_CMD_BUF)
          break;
        for (i=tmp_cmd.count; i>=tmp_cmd.position; i--)
          tmp_cmd.cbuff[i+1] = tmp_cmd.cbuff[i];
        tmp_cmd.cbuff[tmp_cmd.position] = (char)c;
        tmp_cmd.count++;
        for (i=tmp_cmd.position; i<tmp_cmd.count; i++)
          mvwaddch(window_list[WINDOW_STATUS], 0, i+1, tmp_cmd.cbuff[i]);
        tmp_cmd.position++;
        wmove(window_list[WINDOW_STATUS], 0, tmp_cmd.position+1);
        break;
    }
  } while(c != NL && c != CR && c != KEY_ENTER);

  tmp_cmd.cbuff[tmp_cmd.count] = '\0';

  if (tmp_cmd.count)
  {
    strncpy(cmd, tmp_cmd.cbuff, MAX_CMD_BUF);
    strncpy(cmd_hist[entry_hist_index].cbuff, tmp_cmd.cbuff, MAX_CMD_BUF);
    cmd_hist[entry_hist_index].count = tmp_cmd.count;
    cmd_hist[entry_hist_index].position = tmp_cmd.position;
    cmd_parse(cmd);
    hist_index = (entry_hist_index+1) % MAX_CMD_HISTORY;
  }
  else
  {
    hist_index = entry_hist_index;
  }

  return E_SUCCESS;
}

void handle_key(int c)
{
  int x, y, int_c, mark;
  static int multiplier = 0;
  static int esc_count = 0;
  static off_t jump_addr = -1;

  mvwprintw(window_list[WINDOW_STATUS], 0, 0, "%c", c);

  if (c >= '0' && c <= '9')
  {
    int_c = c - '0';

    if (multiplier == 0 && int_c == 0)
      action_cursor_move_line_start();

    multiplier *= 10;
    multiplier += int_c;

    if (jump_addr == -1)
      jump_addr = 0;
    else
      jump_addr *= 10;
    jump_addr += int_c;

    mvwprintw(window_list[WINDOW_STATUS], 0, 100, "jump_addr = %d (%x)",
              jump_addr, jump_addr);
  }

  switch (c)
  {
    case '`':
      mark = getch();
      jump_addr = action_get_mark(mark);
      action_jump_to(jump_addr);
      break;
    case 'm':
      mark = getch();
      action_set_mark(mark);
      break;
    case 'G':
      if (jump_addr == -1)
        action_cursor_move_file_end();
      else
        action_jump_to(jump_addr);
      jump_addr = -1;
      break;
    case 'j':
    case KEY_DOWN:
      action_cursor_move_down(multiplier);
      break;
    case 'k':
    case KEY_UP:
      action_cursor_move_up(multiplier);
      break;
    case 'h':
    case KEY_LEFT:
      action_cursor_move_left(multiplier);
      break;
    case 'l':
    case KEY_RIGHT:
      action_cursor_move_right(multiplier);
      break;
    case '$':
    case KEY_END:
      action_cursor_move_line_end();
      break;
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
    case 'X':
      action_cursor_move_left(multiplier);
      /* no break */
    case 'x':
      action_delete(multiplier);
      break;
    case ':':
      do_cmd_line(c);
      break;
    case ESC:
      if (esc_count)
      {
        action_clear_search_highlight();
        esc_count = 0;
      }
      else
      {
        esc_count ++;
      }
      multiplier = 0;
      jump_addr = -1;
      break;
    default:
      break;
  }

  if (c < '0' || c > '9')
  {
    jump_addr = -1;
    multiplier = 0;
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
 * use buffering for the screen mem, will help for doing group inserts
 * Remember to add tab completion, marks, macros, and a good system for command line parsing, and a good system for settings and .rc files
 * Add options: columns, search hl, search ignorecase
 * Check bvi man page for min list of command line commands to support
 * Fix jump to end of line in case of last line where bytes do not go to end (and for cols pref is set?)
 */

