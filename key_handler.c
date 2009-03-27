#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "key_handler.h"
#include "user_prefs.h"
#include "display.h"
#include "actions.h"
#include "app_state.h"

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
  char *tok = 0, *endptr = 0;
  const char delimiters[] = " =";
  long long num = 0;

  //msg_box("%s", cbuff);

  tok = strtok(cbuff, delimiters);
  while (tok != NULL)
  {
    if (strncmp(tok,"0x",2) == 0)
      num = strtoll(tok, &endptr, 16);
    else
      num = strtoll(tok, &endptr, 10);
    if ((endptr - tok) == strlen(tok))
    {
      if (address_invalid(num))
        msg_box("Invalid jump address: %d", num);
      else
        action_jump_to(num, CURSOR_REAL);
    }

    if (strncmp(tok, "set", MAX_CMD_BUF) == 0)
    {
      error = do_set();
    }
    if (strncmp(tok, "next", MAX_CMD_BUF) == 0)
    {
      current_file = vf_get_next_fm_from_ring(file_ring);
      reset_display_info();
      print_screen(display_info.page_start);
    }
    if ((strncmp(tok, "prev",     MAX_CMD_BUF) == 0) ||
        (strncmp(tok, "previous", MAX_CMD_BUF) == 0) ||
        (strncmp(tok, "last",     MAX_CMD_BUF) == 0))
    {
      current_file = vf_get_last_fm_from_ring(file_ring);
      reset_display_info();
      print_screen(display_info.page_start);
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

off_t get_next_motion_addr(void)
{
  int x, y, c, int_c, mark;
  static int multiplier = 0;
  static int esc_count = 0;
  static off_t jump_addr = -1;

  c = getch();
  while (c != ESC)
  {
    if (c >= '0' && c <= '9')
    {
      int_c = c - '0';

      if (multiplier == 0 && int_c == 0)
      {
        action_cursor_move_line_start(CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      }

      multiplier *= 10;
      multiplier += int_c;

      if (jump_addr == -1)
        jump_addr = 0;
      else
        jump_addr *= 10;
      jump_addr += int_c;
    }

    switch (c)
    {
      case '`':
        mark = getch();
        jump_addr = action_get_mark(mark);
        action_jump_to(jump_addr, CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case 'G':
        if (jump_addr == -1)
          action_cursor_move_file_end(CURSOR_VIRTUAL);
        else
          action_jump_to(jump_addr, CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case 'j':
      case KEY_DOWN:
        action_cursor_move_down(multiplier, CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case 'k':
      case KEY_UP:
        action_cursor_move_up(multiplier, CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case 'h':
      case KEY_LEFT:
        action_cursor_move_left(multiplier, CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case 'l':
      case KEY_RIGHT:
        action_cursor_move_right(multiplier, CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case '$':
      case KEY_END:
        action_cursor_move_line_end(CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case KEY_HOME:
        action_cursor_move_line_start(CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case BVICTRL('f'):
      case KEY_NPAGE:
        action_cursor_move_page_down(CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case BVICTRL('b'):
      case KEY_PPAGE:
        action_cursor_move_page_up(CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case ':':
        do_cmd_line(c);
        return display_info.virtual_cursor_addr;
      case ESC:
        return display_info.cursor_addr;
      default:
        break;
    }

    if (c < '0' || c > '9')
    {
      jump_addr = -1;
      multiplier = 0;
    }

    flash();
    c = getch();
  }
}


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
}
void do_insert(int count, int c)
{
  WINDOW *insbox;
  char *ins_buf, *tmp_buf;
  char insbox_line[MAX_INSERT_BOX_LEN], tmp[3], tmpc;
  int i, c2, ins_buf_size = 4, chars_per_byte = 0;
  int char_count = 0, byte_count = 0, print_offset = 1, print_buf_offset = 0;

  if (display_info.cursor_window == WINDOW_HEX)
    chars_per_byte = 2;
  else
    chars_per_byte = 1;

  ins_buf = (char *)calloc(1, ins_buf_size);
  insbox = newwin(INSERT_BOX_H, INSERT_BOX_W, INSERT_BOX_Y, INSERT_BOX_X);
  box(insbox, 0, 0);
  snprintf(insbox_line, MAX_INSERT_BOX_LEN, "[Press ESC when done]");
  mvwaddstr(insbox, 2, (INSERT_BOX_W - strlen(insbox_line))/2, insbox_line);
  wmove(insbox, 1, 1);
  wrefresh(insbox);

  c2 = getch();

  while(c2 != ESC)
  {
    if (display_info.cursor_window == WINDOW_HEX)
    {
      if (is_hex(c) == 0)
      {
        flash();
        continue;
      }
      tmp[char_count % chars_per_byte] = (char)c;
      char_count++;
      mvwaddch(insbox, 1, print_offset++, c);
      wrefresh(insbox);
      if (print_offset >= MAX_INSERT_BOX_LEN)
      {
        werase(insbox);
        box(insbox, 0, 0);
        print_buf_offset += (MAX_INSERT_BOX_LEN/2);
        print_offset = 0;
        for (i=print_buf_offset; i<char_count; i++)
        {
           mvwaddch(insbox, 1, print_offset++, (ins_buf[print_buf_offset + print_offset]&0xF)<<4);
           mvwaddch(insbox, 1, print_offset++, (ins_buf[print_buf_offset + print_offset]&0xF));
          if (((print_buf_offset + i) % user_prefs[GROUPING].value) == 0)
            mvwaddch(insbox, 1, print_offset++,  ' ');
        }
        /* print tmp[0/1] if partial fill? */
        wrefresh(insbox);
      }
      if ((char_count % chars_per_byte) == 0)
      {
        tmp[2] = 0;
        tmpc = (char)strtol(tmp, NULL, 16);
        if (byte_count >= ins_buf_size)
        {
          tmp_buf = calloc(1, ins_buf_size * 2);
          memcpy(tmp_buf, ins_buf, ins_buf_size);
          free(ins_buf);
          ins_buf = tmp_buf;
        }
        ins_buf[byte_count] = tmpc;
        byte_count++;
        if ((byte_count % user_prefs[GROUPING].value) == 0)
        {
          mvwaddch(insbox, 1, print_offset++,  ' ');
          wrefresh(insbox);
          if (print_offset >= MAX_INSERT_BOX_LEN)
          {
            werase(insbox);
            box(insbox, 0, 0);
            print_buf_offset += (MAX_INSERT_BOX_LEN/2);
            print_offset = 0;
            for (i=print_buf_offset; i<char_count; i++)
            {
               mvwaddch(insbox, 1, print_offset++, (ins_buf[print_buf_offset + print_offset]&0xF)<<4);
               mvwaddch(insbox, 1, print_offset++, (ins_buf[print_buf_offset + print_offset]&0xF));
              if (((print_buf_offset + i) % user_prefs[GROUPING].value) == 0)
                mvwaddch(insbox, 1, print_offset++,  ' ');
            }
            /* print tmp[0/1] if partial fill? */
            wrefresh(insbox);
          }
        }
      }
    }
    else
    {
    }

 
    c2 = getch();
  }

  if (byte_count)
  {
    if (count == 0)
      count = 1;

    action_insert_before(count,ins_buf,byte_count);
  }

  free(ins_buf);

  delwin(insbox);
  place_cursor(display_info.cursor_addr, CALIGN_NONE, CURSOR_REAL);
  print_screen(display_info.page_start);

}
void do_yank(int count, int c)
{
  off_t end_addr = INVALID_ADDR;

  if (action_visual_select_check())
    end_addr = INVALID_ADDR;
  else if (c == 'Y')
    end_addr = display_info.cursor_addr;
  else
    end_addr = get_next_motion_addr();

  action_yank(count, end_addr);
}
void do_replace(int count)
{
  int hx, hy, ax, ay, c, i, char_count = 0, chars_per_byte = 0;
  char tmp[3], tmpc;
  char replace_buf[256];
  off_t tmp_addr = 0;

  if (count == 0)
    count = 1;

  if (display_info.cursor_window == WINDOW_HEX)
  {
    hy = get_y_from_addr(display_info.cursor_addr);
    hx = get_x_from_addr(display_info.cursor_addr);
    display_info.cursor_window = WINDOW_ASCII;
    ay = get_y_from_addr(display_info.cursor_addr);
    ax = get_x_from_addr(display_info.cursor_addr);
    display_info.cursor_window = WINDOW_HEX;
    chars_per_byte = 2;
  }
  else
  {
    display_info.cursor_window = WINDOW_HEX;
    hy = get_y_from_addr(display_info.cursor_addr);
    hx = get_x_from_addr(display_info.cursor_addr);
    display_info.cursor_window = WINDOW_ASCII;
    ay = get_y_from_addr(display_info.cursor_addr);
    ax = get_x_from_addr(display_info.cursor_addr);
    chars_per_byte = 1;
  }

  for (i=0; i<user_prefs[GROUPING].value; i++)
  {
    mvwaddch(window_list[WINDOW_HEX], hy, hx+(2*i), ' ');
    mvwaddch(window_list[WINDOW_HEX], hy, hx+(2*i)+1, ' ');
    mvwaddch(window_list[WINDOW_ASCII], ay, ax+i, ' ');
  }
  if (display_info.cursor_window == WINDOW_HEX)
    wmove(window_list[WINDOW_HEX], hy, hx);
  else
    wmove(window_list[WINDOW_ASCII], ay, ax);

  while(char_count < user_prefs[GROUPING].value * chars_per_byte)
  {
    update_panels();
    doupdate();
    c = getch();
    if (c == ESC)
      break;

    if (display_info.cursor_window == WINDOW_HEX)
    {
      if (is_hex(c) == 0)
      {
        flash();
        continue;
      }
      tmp[char_count % chars_per_byte] = (char)c;
      char_count++;
      if ((char_count % chars_per_byte) == 0)
      {
        tmp[2] = 0;
        tmpc = (char)strtol(tmp, NULL, 16);
        mvwaddch(window_list[WINDOW_HEX], hy, hx+char_count-2,  tmp[0]);
        mvwaddch(window_list[WINDOW_HEX], hy, hx+char_count-1, tmp[1]);
        if (isprint(tmpc))
          mvwaddch(window_list[WINDOW_ASCII], ay, ax+char_count/2-1, tmpc);
        else
          mvwaddch(window_list[WINDOW_ASCII], ay, ax+char_count/2-1, '.');

        replace_buf[char_count / chars_per_byte - 1] = tmpc;

      }
    }
    else
    {
        mvwaddch(window_list[WINDOW_HEX], hy, hx+char_count/2-1, (c&0xF)<<4);
        mvwaddch(window_list[WINDOW_HEX], hy, hx+char_count/2,   (c&0xF));
        if (isprint(tmpc))
          mvwaddch(window_list[WINDOW_ASCII], ay, ax+char_count/2-1, tmpc);
        else
          mvwaddch(window_list[WINDOW_ASCII], ay, ax+char_count/2-1, '.');

        replace_buf[char_count] = (char)c;
        char_count++;
    }
  }
  if (char_count >= user_prefs[GROUPING].value * chars_per_byte)
  {
    if (is_visual_on())
    {
      if (display_info.cursor_addr > display_info.visual_select_addr)
      {
        tmp_addr = display_info.visual_select_addr;
        count = display_info.cursor_addr - display_info.visual_select_addr + user_prefs[GROUPING].value;
      }
      else
      {
        tmp_addr = display_info.cursor_addr;
        count = display_info.visual_select_addr - display_info.cursor_addr + user_prefs[GROUPING].value;
      }
      count /= user_prefs[GROUPING].value;
      action_visual_select_off();
    }
    else
    {
      tmp_addr = display_info.cursor_addr;
    }

    for (i=0; i<count; i++)
    {
      if (address_invalid(tmp_addr) || address_invalid(tmp_addr + user_prefs[GROUPING].value))
        break;
      vf_replace(current_file, replace_buf, tmp_addr, user_prefs[GROUPING].value);
      tmp_addr += user_prefs[GROUPING].value;
    }
  }

  print_screen(display_info.page_start);

}
void do_overwrite(int count)
{
  char *tmp_buf;
  int tmp_buf_size = 4;

  tmp_buf = malloc(tmp_buf_size);

  free(tmp_buf);
}

void do_delete(int count, int c)
{
  off_t end_addr = INVALID_ADDR;

  if (action_visual_select_check())
    end_addr = INVALID_ADDR;
  else
    end_addr = get_next_motion_addr();

  action_delete(count, end_addr);
}

void handle_key(int c)
{
  int x, y, int_c, mark;
  static int multiplier = 0;
  static int esc_count = 0;
  static off_t jump_addr = -1;

  mvwprintw(window_list[WINDOW_STATUS], 0, 15, "%c", c);

  if (c >= '0' && c <= '9')
  {
    int_c = c - '0';

    if (multiplier == 0 && int_c == 0)
      action_cursor_move_line_start(CURSOR_REAL);

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
      action_jump_to(jump_addr, CURSOR_REAL);
      break;
    case 'm':
      mark = getch();
      action_set_mark(mark);
      break;
    case 'G':
      if (jump_addr == -1)
        action_cursor_move_file_end(CURSOR_REAL);
      else
        action_jump_to(jump_addr, CURSOR_REAL);
      jump_addr = -1;
      break;
    case 'j':
    case KEY_DOWN:
      action_cursor_move_down(multiplier, CURSOR_REAL);
      break;
    case 'k':
    case KEY_UP:
      action_cursor_move_up(multiplier, CURSOR_REAL);
      break;
    case 'h':
    case KEY_LEFT:
      action_cursor_move_left(multiplier, CURSOR_REAL);
      break;
    case 'l':
    case KEY_RIGHT:
      action_cursor_move_right(multiplier, CURSOR_REAL);
      break;
    case '$':
    case KEY_END:
      action_cursor_move_line_end(CURSOR_REAL);
      break;
    case KEY_HOME:
      action_cursor_move_line_start(CURSOR_REAL);
      break;
    case TAB:
      action_cursor_toggle_hex_ascii();
      break;
    case BVICTRL('f'):
    case KEY_NPAGE:
      action_cursor_move_page_down(CURSOR_REAL);
      break;
    case BVICTRL('b'):
    case KEY_PPAGE:
      action_cursor_move_page_up(CURSOR_REAL);
      break;
    case 'X':
      action_cursor_move_left(multiplier, CURSOR_REAL);
      /* no break */
    case 'x':
      action_yank(multiplier, INVALID_ADDR);
      action_delete(multiplier, INVALID_ADDR);
      break;
    case 'v':
      action_visual_select_toggle();
      break;
/* These are a real problem because they do not translate well between vi/emacs modes */
    case 'i':
    case 'I':
    case 'a':
    case 'A':
    case 'c':
    case 'C':
    case 's':
    case 'S':
      do_insert(multiplier, c);
      break;
    case 'R':
      do_overwrite(multiplier);
      break;
/*************************************************************************************/
    case 'r':
      do_replace(multiplier);
      break;
    case 'y': /* no separate behavior from Y, right now */
    case 'Y':
      do_yank(multiplier, c);
      break;
    case 'd':
    case 'D':
      do_delete(multiplier, c);
      break;
    case 'p':
      action_paste_after(multiplier);
      break;
    case 'P':
      action_paste_before(multiplier);
      break;
    case '"':
      mark = getch();
      action_set_yank_register(mark);
    case 'u':
      action_undo(multiplier);
      break;
    case 'U':
      action_redo(multiplier);
      break;
    case ':':
      do_cmd_line(c);
      break;
    case ESC:
      action_visual_select_off();
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

  mvwprintw(window_list[WINDOW_STATUS], 0, 30, "addr = %08x, vaddr = %08x",
            display_info.cursor_addr, visual_addr());

}

/*
 * use buffering for the screen mem, will help for doing group inserts
 * Remember to add tab completion, macros, and a good system for command line parsing, .rc files
 * Add options: columns, search hl, search ignorecase
 * Check bvi man page for min list of command line commands to support
 */

