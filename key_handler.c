/*************************************************************
 *
 * File:        key_handler.c
 * Author:      David Kelley
 * Description: Handle key presses and other user input
 *
 *************************************************************/

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "key_handler.h"
#include "user_prefs.h"
#include "display.h"
#include "actions.h"
#include "app_state.h"
#include "help.h"

action_code_t show_set(void)
{
  action_code_t error = E_SUCCESS;
  int i = 0, num_elements = 0, eq_tab = 25, len;
  char **text;

  while (user_prefs[num_elements].flags != P_NONE)
    num_elements++;

  /* one extra for delimeter */
  text = malloc(sizeof(char *)*(num_elements+1));
  if (text == NULL)
  {
    msg_box("Could not allocate memory for display window");
    return E_INVALID;
  }

  for(i=0; i<num_elements; i++)
  {
    text[i] = (char *)malloc(256);
    snprintf(text[i], 256, " %s", user_prefs[i].name);
    len = strlen(text[i]);
    for (;len<eq_tab;len++)
      snprintf(text[i] + len, 256 - len, " ");
    if (user_prefs[i].flags == P_INT)
      snprintf(text[i] + len, 256 - len, "= %d",
               user_prefs[i].value);
    else if (user_prefs[i].flags == P_BOOL)
      snprintf(text[i] + len, 256 - len, "= %s",
               user_prefs[i].value == TRUE ? "TRUE" : "FALSE");
  }

  text[i] = NULL;
  scrollable_window_display(text);

  for(i=0; i<num_elements; i++)
    free(text[i]);

  free(text);

  return error;
}

action_code_t do_set(void)
{
  action_code_t error = E_SUCCESS;
  char *tok;
  const char delimiters[] = " =";
  int option, set = 1;
  long value;

  /* process same string as last strtok() call from cmd_parse()*/
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

      if (user_prefs[option].flags == P_INT)
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

/******* Print some warnings since this stuff is not tested ******/
      if (strncmp(user_prefs[option].short_name, "grp", MAX_CMD_BUF) == 0)
        msg_box("Warning, grouping other than 1 is experimental!!");
      if (strncmp(user_prefs[option].short_name, "bin", MAX_CMD_BUF) == 0)
        msg_box("Warning, binary display mode is experimental!!");
      if (strncmp(user_prefs[option].short_name, "le", MAX_CMD_BUF) == 0)
        msg_box("Warning, little endian display mode is experimental!!");
/*****************************************************************/

  action_do_resize();

  return error;
}

action_code_t cmd_parse(char *cbuff)
{
  action_code_t error = E_SUCCESS;
  char *tok = 0, *endptr = 0;
  const char delimiters[] = " =";
  long long num = 0;
  char fname[MAX_FILE_NAME];
  off_t caddrsave, paddrsave;

  tok = strtok(cbuff, delimiters);
  if (tok != NULL)
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
      return error;
    }

    if (strncmp(tok, "set", MAX_CMD_BUF) == 0)
    {
      error = do_set();
      return error;
    }
    if ((strncmp(tok, "next",     MAX_CMD_BUF) == 0) ||
        (strncmp(tok, "bn",       MAX_CMD_BUF) == 0))
    {
      action_load_next_file();
      return error;
    }
    if ((strncmp(tok, "prev",     MAX_CMD_BUF) == 0) ||
        (strncmp(tok, "previous", MAX_CMD_BUF) == 0) ||
        (strncmp(tok, "bp",       MAX_CMD_BUF) == 0) ||
        (strncmp(tok, "last",     MAX_CMD_BUF) == 0))
    {
      action_load_prev_file();
      return error;
    }
    if (strncmp(tok, "e", MAX_CMD_BUF) == 0)
    {
      tok = strtok(NULL, delimiters);
      if (tok == NULL)
      {
        current_file = vf_add_fm_to_ring(file_ring);
        if (vf_init(current_file, NULL) == FALSE)
          fprintf(stderr, "Empty file failed?\n");
        update_display_info();
        print_screen(0);
      }
      else
      {
        current_file = vf_add_fm_to_ring(file_ring);
        if (vf_init(current_file, tok) == FALSE)
          fprintf(stderr, "Could not open %s\n", tok);
        update_display_info();
        print_screen(0);
      }
      return error;
    }
    if (strncmp(tok, "e!", MAX_CMD_BUF) == 0)
    {
      snprintf(fname, MAX_FILE_NAME, "%s", vf_get_fname(current_file));
      caddrsave = display_info.cursor_addr;
      paddrsave = display_info.page_start;
      vf_term(current_file);
      vf_init(current_file, fname);
      update_display_info();
      place_cursor(caddrsave, CALIGN_NONE, CURSOR_REAL);
      if (address_invalid(paddrsave))
        print_screen(0);
      else
        print_screen(paddrsave);
      return error;
    }
    if (strncmp(tok, "q", MAX_CMD_BUF) == 0)
    {
      action_quit(FALSE);
      return error;
    }
    if (strncmp(tok, "q!", MAX_CMD_BUF) == 0)
    {
      action_quit(TRUE);
      return error;
    }
    if (strncmp(tok, "qa", MAX_CMD_BUF) == 0)
    {
      action_quit_all(FALSE);
      return error;
    }
    if (strncmp(tok, "qa!", MAX_CMD_BUF) == 0)
    {
      action_quit_all(TRUE);
      return error;
    }
    if (strncmp(tok, "wa", MAX_CMD_BUF) == 0)
    {
      action_save_all();
      return error;
    }
    if ((strncmp(tok, "wqa", MAX_CMD_BUF) == 0) ||
        (strncmp(tok, "waq", MAX_CMD_BUF) == 0))
    {
      action_save_all();
      action_quit_all(FALSE);
      return error;
    }
    if (strncmp(tok, "w", MAX_CMD_BUF) == 0)
    {
      tok = strtok(NULL, delimiters);
      if (tok == NULL)
        action_save();
      else
        action_save_as(tok);
      return error;
    }
    if ((strncmp(tok, "wq", MAX_CMD_BUF) == 0) ||
        (strncmp(tok, "qw", MAX_CMD_BUF) == 0))
    {
      tok = strtok(NULL, delimiters);
      if (tok == NULL)
        action_save();
      else
        action_save_as(tok);

      action_quit(FALSE);
      return error;
    }

    if ((strncmp(tok, "help", MAX_CMD_BUF) == 0) ||
        (strncmp(tok, "h", MAX_CMD_BUF) == 0))
    {
      scrollable_window_display(help_text);
      return error;
    }

    if ((strncmp(tok, "external", MAX_CMD_BUF) == 0) ||
        (strncmp(tok, "ex", MAX_CMD_BUF) == 0))
    {
      run_external();
      return error;
    }

  }

  flash();
  return error;
}

action_code_t do_search(int c, cursor_t cursor)
{
  cmd_hist_t *search_hist;
  char *cmd, prompt[2];

  if (c == '/')
    search_hist = ascii_search_hist;
  else
    search_hist = hex_search_hist;

  prompt[0] = c;
  prompt[1] = 0;

  werase(window_list[WINDOW_STATUS]);
  cmd = creadline(prompt, window_list[WINDOW_STATUS], 0, 0, search_hist);

  if (cmd)
  {
    action_do_search(c, cmd, cursor);
    free(cmd);
  }

  return E_SUCCESS;
}

action_code_t do_cmd_line(cursor_t cursor)
{
  char *cmd;

  werase(window_list[WINDOW_STATUS]);
  cmd = creadline(":", window_list[WINDOW_STATUS], 0, 0, cmd_hist);

  if (cmd)
  {
    cmd_parse(cmd);
    free(cmd);
  }

  return E_SUCCESS;
}

off_t get_next_motion_addr(void)
{
  int c, int_c, mark;
  static int multiplier = 0;
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
      case 'n':
        action_move_cursor_next_search(CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case 'N':
        action_move_cursor_prev_search(CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case ':':
        do_cmd_line(CURSOR_VIRTUAL);
        return display_info.virtual_cursor_addr;
      case '/':
      case '\\':
        do_search(c, CURSOR_VIRTUAL);
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
  char *screen_buf, *ins_buf, *tmp_ins_buf;
  char tmp[9], tmp2[MAX_GRP], tmpc;
  int c2 = 0, i;
  int hy, hx, ay, ax;
  int ins_buf_size;
  int chars_per_byte, char_count = 0, tmp_char_count = 0, low_tmp_char_count = 0;
  int offset = 0, len1, ins_buf_offset, len2, len3;
  off_t ins_addr, page_start;

  screen_buf = (char *)malloc(2 * PAGE_SIZE); /* fix this later, but make it big for now */
  ins_buf_size = user_prefs[GROUPING].value;
  ins_buf = (char *)malloc(ins_buf_size);

  /* later mod this depending on c = a/i/A/I */
  switch (c)
  {
    case 'A': /* no break */
    case 'a': /* no break */
      if (display_info.file_size == 0)
        ins_addr = display_info.cursor_addr;
      else
        ins_addr = display_info.cursor_addr + user_prefs[GROUPING].value;
      break;
    case INS:
    case 'I': /* no break */
    case 'i': /* no break */
    default:
      ins_addr = display_info.cursor_addr;
      break;
#if 0
    case 'c':
    case 'C':
    case 's':
    case 'S':
#endif
  }

  page_start = display_info.page_start;

  while (c2 != ESC)
  {
    if ((offset + char_count + 1) > PAGE_SIZE)
      page_start += BYTES_PER_LINE;
    offset = ins_addr - page_start;
    if (offset < 0)
    {
      len1 = 0;
      ins_buf_offset = page_start - ins_addr;
      len2 = char_count - ins_buf_offset;
    }
    else
    {
      len1 = offset;
      len2 = char_count;
      ins_buf_offset = 0;
    }
    if ((len1 + len2) > PAGE_SIZE)
      len3 = 0;
    else
    {
      len3 = (PAGE_END - page_start) - len1 + user_prefs[GROUPING].value;
      if ((len1 + len2 + len3) > PAGE_SIZE)
        len3 = PAGE_SIZE - (len1 + len2);
    }

    if (len1 != 0)
      vf_get_buf(current_file, screen_buf, page_start, len1);
    if (len2 != 0)
      memcpy(screen_buf + len1, ins_buf + ins_buf_offset, len2);
    if (len3 > 0)
      vf_get_buf(current_file, screen_buf + len1 + len2 + user_prefs[GROUPING].value, ins_addr, len3);
    else
      len3 = 0;

    print_screen_buf(page_start, screen_buf, len1+len2+1+len3, NULL);

    if (display_info.cursor_window == WINDOW_HEX)
    {
      hy = get_y_from_page_offset(len1+len2);
      hx = get_x_from_page_offset(len1+len2);
      display_info.cursor_window = WINDOW_ASCII;
      ay = get_y_from_page_offset(len1+len2);
      ax = get_x_from_page_offset(len1+len2);
      display_info.cursor_window = WINDOW_HEX;
      chars_per_byte = 2;
    }
    else
    {
      display_info.cursor_window = WINDOW_HEX;
      hy = get_y_from_page_offset(len1+len2);
      hx = get_x_from_page_offset(len1+len2);
      display_info.cursor_window = WINDOW_ASCII;
      ay = get_y_from_page_offset(len1+len2);
      ax = get_x_from_page_offset(len1+len2);
      chars_per_byte = 1;
    }

    for (i=0; i<user_prefs[GROUPING].value; i++) /* print from temp buf here to clear or print partial insert */
    {
      if (i>=tmp_char_count)
      {
        if (low_tmp_char_count && i == tmp_char_count)
          mvwaddch(window_list[WINDOW_HEX], hy, hx+(2*i), tmp[0]);
        else
          mvwaddch(window_list[WINDOW_HEX], hy, hx+(2*i), ' ');
        mvwaddch(window_list[WINDOW_HEX], hy, hx+(2*i)+1, ' ');
        mvwaddch(window_list[WINDOW_ASCII], ay, ax+i, ' ');
      }
      else
      {
        mvwaddch(window_list[WINDOW_HEX], hy, hx+(2*i), HEX(tmp2[i]>>4&0xF));
        mvwaddch(window_list[WINDOW_HEX], hy, hx+(2*i)+1, HEX(tmp2[i]>>0&0xF));
        if (isprint(tmp2[i]))
          mvwaddch(window_list[WINDOW_ASCII], ay, ax+i, tmp2[i]);
        else
          mvwaddch(window_list[WINDOW_ASCII], ay, ax+i, '.');
      }
    }

    update_panels();
    doupdate();
    if (display_info.cursor_window == WINDOW_HEX)
      wmove(window_list[WINDOW_HEX], hy, hx);
    else
      wmove(window_list[WINDOW_ASCII], ay, ax);
    c2 = getch();
    switch (c2)
    {
      case KEY_BACKSPACE:
        break;
      case KEY_RESIZE:
        break;
      case ESC:
        break;
      default:
        if (display_info.cursor_window == WINDOW_HEX)
        {
          if (is_hex(c2) == 0)
          {
            flash();
            continue;
          }
          tmp[low_tmp_char_count] = (char)c2;
          low_tmp_char_count++;
          if ((low_tmp_char_count % chars_per_byte) == 0)
          {
            low_tmp_char_count = 0;
            tmp[chars_per_byte] = 0;
            tmpc = (char)strtol(tmp, NULL, 16);
            tmp2[tmp_char_count % user_prefs[GROUPING].value] = tmpc;
            tmp_char_count++;

            if ((tmp_char_count % user_prefs[GROUPING].value) == 0)
            {
              while (char_count + tmp_char_count >= ins_buf_size)
              {
                tmp_ins_buf = calloc(1, ins_buf_size * 2);
                memcpy(tmp_ins_buf, ins_buf, ins_buf_size);
                ins_buf_size *= 2;
                free(ins_buf);
                ins_buf = tmp_ins_buf;
              }

              memcpy(ins_buf + char_count, tmp2, tmp_char_count);
              char_count += tmp_char_count;
              tmp_char_count = 0;
            }
          }
        }
        else /* cursor in ascii window */
        {
          tmp2[tmp_char_count % user_prefs[GROUPING].value] = (char)c2;
          tmp_char_count++;

          if ((tmp_char_count % user_prefs[GROUPING].value) == 0)
          {
            while (char_count + tmp_char_count >= ins_buf_size)
            {
              tmp_ins_buf = calloc(1, ins_buf_size * 2);
              memcpy(tmp_ins_buf, ins_buf, ins_buf_size);
              ins_buf_size *= 2;
              free(ins_buf);
              ins_buf = tmp_ins_buf;
            }

            memcpy(ins_buf + char_count, tmp2, tmp_char_count);
            char_count += tmp_char_count;
            tmp_char_count = 0;
          }
        }
        break;
    }
  }

  if (char_count)
  {
    if (count == 0)
      count = 1;

    switch (c)
    {
      case 'A': /* no break */
      case 'a': /* no break */
        if (display_info.file_size == 0)
        {
          for (i=0; i<count; i++)
            action_insert_before(count,ins_buf,char_count);
        }
        else
        {
          for (i=0; i<count; i++)
            action_insert_after(count,ins_buf,char_count);
        }
        break;
      case 'I': /* no break */
      case 'i': /* no break */
      default:
        for (i=0; i<count; i++)
          action_insert_before(count,ins_buf,char_count);
        break;
    }
  }

  free(ins_buf);
  free(screen_buf);

  place_cursor(ins_addr+char_count, CALIGN_NONE, CURSOR_REAL);
  print_screen(page_start);
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

  action_yank(count, end_addr, TRUE);
}
void do_replace(int count)
{
  int hx, hy, ax, ay, c, i, char_count = 0, chars_per_byte = 0;
  char tmp[3], tmpc, *tmp_fill;
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
      mvwaddch(window_list[WINDOW_HEX], hy, hx+char_count, tmp[char_count % chars_per_byte]);
      char_count++;
      if ((char_count % chars_per_byte) == 0)
      {
        tmp[2] = 0;
        tmpc = (char)strtol(tmp, NULL, 16);
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
      tmp_addr = visual_addr();
      count = visual_span();
      count /= user_prefs[GROUPING].value;
      action_visual_select_off();
    }
    else
    {
      tmp_addr = display_info.cursor_addr;
    }

    if (address_invalid(tmp_addr) == 0)
    {
      count *= user_prefs[GROUPING].value;
      while (address_invalid(tmp_addr + count - 1) && count > 0)
        count -= user_prefs[GROUPING].value;

      if (count > 0)
      {
        tmp_fill = (char *)malloc(count);
        for (i=0; i<count/user_prefs[GROUPING].value; i++)
          memcpy(tmp_fill+i*(user_prefs[GROUPING].value), replace_buf, user_prefs[GROUPING].value);
        vf_replace(current_file, tmp_fill, tmp_addr, count);
        free(tmp_fill);
      }
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
  int int_c, mark, tab;
  static int multiplier = 0;
  static int esc_count = 0;
  static off_t jump_addr = -1;

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

    if (esc_count)
    {
      tab = c - '0';
      current_file = vf_get_head_fm_from_ring(file_ring);
      tab--;
      while (tab > 0)
      {
        tab--;
        current_file = vf_get_next_fm_from_ring(file_ring);
      }
      esc_count = 0;
      multiplier = 0;
      jump_addr = -1;
      reset_display_info();
      print_screen(display_info.page_start);
    }
  }

  switch (c)
  {
    case '`':
      mark = getch();
      jump_addr = action_get_mark(mark);
      action_jump_to(jump_addr, CURSOR_REAL);
      break;
    case '<':
      action_blob_shift_left(multiplier);
      break;
    case '>':
      action_blob_shift_right(multiplier);
      break;
    case 'm':
      mark = getch();
      action_set_mark(mark);
      break;
    case 'g':
      c = getch();
      if (c > '0' && c <= '9')
      {
        flash();
        break;
      }
      else if (c != 'g')
      {
        ungetch(c);
        jump_addr = get_next_motion_addr();
      }
      if (jump_addr == -1)
        action_cursor_move_file_start(CURSOR_REAL);
      else
        action_jump_to(jump_addr, CURSOR_REAL);
      multiplier = 0;
      jump_addr = -1;
      break;
    case 'G':
      if (jump_addr == -1)
        action_cursor_move_file_end(CURSOR_REAL);
      else
        action_jump_to(jump_addr, CURSOR_REAL);
      jump_addr = -1;
      break;
    case BVICTRL('n'):
    case 'j':
    case KEY_DOWN:
      action_cursor_move_down(multiplier, CURSOR_REAL);
      break;
    case BVICTRL('p'):
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
    case '^':
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
      action_yank(multiplier, INVALID_ADDR, FALSE);
      action_delete(multiplier, INVALID_ADDR);
      action_visual_select_off();
      break;
    case 'v':
      action_visual_select_toggle();
      break;
/* These are not handled yet */
    case 'c':
    case 'C':
    case 's':
    case 'S':
/*****************************/
    case INS:
    case 'i':
    case 'I':
    case 'a':
    case 'A':
      do_insert(multiplier, c);
      break;
    case 'R':
      do_overwrite(multiplier);
      break;
    case 'r':
      do_replace(multiplier);
      action_visual_select_off();
      break;
    case 'y': /* no separate behavior from Y, right now */
    case 'Y':
      do_yank(multiplier, c);
      action_visual_select_off();
      break;
    case 'd':
    case 'D':
      do_delete(multiplier, c);
      action_visual_select_off();
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
    case BVICTRL('r'):
    case 'U':
      action_redo(multiplier);
      break;
    case 'n':
      action_move_cursor_next_search(CURSOR_REAL);
      break;
    case 'N':
      action_move_cursor_prev_search(CURSOR_REAL);
      break;
    case '?':
    case ':':
      do_cmd_line(CURSOR_REAL);
      break;
    case '/':
    case '\\':
      do_search(c, CURSOR_REAL);
      break;
    case '~':
      action_load_next_file();
      break;
    case ESC:
      if (action_visual_select_check())
      {
        action_visual_select_off();
        esc_count = 0;
      }
      else if (esc_count)
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
    case KEY_RESIZE:
      action_do_resize();
      break;
    default:
      break;
  }

  if (c != ESC)
    esc_count = 0;

  if (c < '0' || c > '9')
  {
    jump_addr = -1;
    multiplier = 0;
  }
}


