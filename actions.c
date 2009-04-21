#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "actions.h"
#include "display.h"
#include "virt_file.h"
#include "app_state.h"
#include "user_prefs.h"
#include "key_handler.h"

#define MARK_LIST_SIZE (26*2)
#define NUM_YANK_REGISTERS (26*2 + 10)

WINDOW *save_window;

typedef struct yank_buf_s
{
  char *buf;
  int len;
} yank_buf_t;

static off_t mark_list[MARK_LIST_SIZE];
static yank_buf_t yank_buf[NUM_YANK_REGISTERS];
static int yank_register = 0;


void sig_pipe_handler(int signum)
{
  msg_box("SIGPIPE (%d) received", signum);
}

void run_external()
{
  int outpipe[2], inpipe[2];
  int error, status, i = 0, eof = EOF, size = 0;
  off_t start = 0;
  char *tok, *delimiters = " ";
  char errstr[MAX_CMD_BUF], *arglist[MAX_CMD_BUF], dummy = 1;
  char *buf = NULL, *tmp_buf = NULL;
  pid_t pid;
  void (*s)(int);

  s = signal(SIGPIPE, sig_pipe_handler);

  if (!is_visual_on())
  {
    msg_box("Please first use visual select ('v') to select the bytes to send to the external program");
    signal(SIGPIPE, s);
    return;
  }
  else
  {
    start = visual_addr();
    size = visual_span();
  }

  tok = strtok(NULL, delimiters);
  if (tok == NULL)
  {
    msg_box("No external program specified, use ':external <program>'");
    signal(SIGPIPE, s);
    return;
  }
  else
  {
    while (tok != NULL)
    {
      arglist[i] = tok;
      tok = strtok(NULL, delimiters);
      i++;
    }
    arglist[i] = (char *)NULL;
    i = 0;
  }

  if (pipe(outpipe))
  {
    error = errno;
    msg_box("Could not creat outpipe: %s\n", strerror(error));
    signal(SIGPIPE, s);
    return;
  }
  if(pipe(inpipe))
  {
    error = errno;
    msg_box("Could not creat outpipe: %s\n", strerror(error));
    close(outpipe[0]);
    close(outpipe[1]);
    signal(SIGPIPE, s);
    return;
  }

  pid = fork();
  if (pid < 0)
  {
    error = errno;
    msg_box("Could not fork: %s\n", strerror(error));
    close(inpipe[0]);
    close(inpipe[1]);
    close(outpipe[0]);
    close(outpipe[1]);
    signal(SIGPIPE, s);
    return;
  }

  setvbuf(stdout,(char*)NULL,_IONBF,0);

  if(pid)
  {
    close(outpipe[0]);
    close(inpipe[1]);

    buf = (char *)malloc(size);
    if (buf == NULL)
    {
      msg_box("Error allocating buffer for external program output (size = %d)", size);
      free(buf);
      close(inpipe[0]);
      close(outpipe[1]);
      waitpid(pid, &status, 0);
      signal(SIGPIPE, s);
      return;
    }

    vf_get_buf(current_file, buf, start, size);

    write(outpipe[1], buf, size);
    close(outpipe[1]);

    while (read(inpipe[0], &buf[i], 1) > 0 && buf[i] != EOF)
    {
      i++;
      if (i >= size)
      {
        size *= 2;
        tmp_buf = malloc(size);
        if (tmp_buf == NULL)
        {
          msg_box("Error allocating buffer for external program output (size = %d)", size);
          free(buf);
          close(inpipe[0]);
          waitpid(pid, &status, 0);
          signal(SIGPIPE, s);
          return;
        }
        memcpy(tmp_buf, buf, i);
        free(buf);
        buf = tmp_buf;
        tmp_buf = NULL;
      }
    }

    big_buf_display(buf, i);

    free(buf);
    waitpid(pid, &status, 0);
    close(inpipe[0]);
    signal(SIGPIPE, s);
    return;
  }
  else
  {
    memset(errstr, 0, MAX_CMD_BUF);
    dup2(outpipe[0],0);
    dup2(inpipe[1],1);
    close(outpipe[1]);
    close(inpipe[0]);
    execvp(*arglist, arglist);
    error = errno;
    while (read(outpipe[0], &dummy, 1) > 0 && dummy != EOF) /* prevent SIGPIPE to parrent */
    snprintf(errstr, MAX_CMD_BUF, "'%s': %s", *arglist, strerror(error));
    write(inpipe[1], errstr, strlen(errstr));
    write(inpipe[1], &eof, 1);
    close(outpipe[0]);
    close(inpipe[1]);
    _exit(EXIT_SUCCESS);
  }
}

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
  if (display_info.file_size == 0)
  {
    msg_box("Cannot visual select on empty file");
    error = E_INVALID;
  }
  else
  {
    display_info.visual_select_addr = display_info.cursor_addr;
  }
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

action_code_t action_move_cursor_prev_search(cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  search_aid_t search_aid;
  off_t addr, tmp;

  addr = display_info.cursor_addr - PAGE_SIZE;
  if (address_invalid(addr))
    addr = 0;

/* now search past pages */
  while(!address_invalid(addr))
  {
    fill_search_buf(addr, PAGE_SIZE, &search_aid);

    do
    {
      tmp = search_aid.hl_start;
      buf_search(&search_aid);
    } while (search_aid.hl_start != -1 && search_aid.hl_start < display_info.cursor_addr);

    if (tmp != -1 && tmp < display_info.cursor_addr)
    {
      place_cursor(tmp, CALIGN_NONE, cursor);
      free_search_buf(&search_aid);
      return error;
    }

    free_search_buf(&search_aid);
    addr -= PAGE_SIZE;
  }

  msg_box("Beginning of file reached, wrapping");

  addr = display_info.file_size - PAGE_SIZE;
  if (address_invalid(addr))
    addr = 0;

/* now search future pages */
  while(addr >= display_info.cursor_addr)
  {
    fill_search_buf(addr, PAGE_SIZE, &search_aid);

    do
    {
      tmp = search_aid.hl_start;
      buf_search(&search_aid);
    } while (search_aid.hl_start != -1);

    if (tmp != -1 && tmp >= display_info.cursor_addr)
    {
      place_cursor(tmp, CALIGN_NONE, cursor);
      free_search_buf(&search_aid);
      return error;
    }

    free_search_buf(&search_aid);
    addr -= PAGE_SIZE;
  }

  msg_box("Term \"%s\" not found", search_item[current_search].pattern);
  return error;
}

action_code_t  action_move_cursor_next_search(cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  search_aid_t search_aid;
  off_t addr;

  addr = display_info.cursor_addr;

  fill_search_buf(addr, PAGE_SIZE, &search_aid);

/* first search the current page */
  while (search_aid.hl_start != -1 && search_aid.hl_start <= display_info.cursor_addr)
    buf_search(&search_aid);

  if (search_aid.hl_start != -1)
  {
    place_cursor(search_aid.hl_start, CALIGN_NONE, cursor);
    free_search_buf(&search_aid);
    return error;
  }

  free_search_buf(&search_aid);

  addr += 2*PAGE_SIZE;

/* now search future pages */
  while(!address_invalid(addr))
  {
    fill_search_buf(addr, PAGE_SIZE, &search_aid);

    if (search_aid.hl_start != -1)
    {
      place_cursor(search_aid.hl_start, CALIGN_NONE, cursor);
      free_search_buf(&search_aid);
      return error;
    }

    free_search_buf(&search_aid);
    addr += 2*PAGE_SIZE;
  }

  msg_box("End of file reached, wrapping");

  addr = 0;

/* now search past pages */
  while(addr <= display_info.cursor_addr)
  {
    fill_search_buf(addr, PAGE_SIZE, &search_aid);

    if (search_aid.hl_start != -1 && search_aid.hl_start <= display_info.cursor_addr)
    {
      place_cursor(search_aid.hl_start, CALIGN_NONE, cursor);
      free_search_buf(&search_aid);
      return error;
    }

    free_search_buf(&search_aid);
    addr += 2*PAGE_SIZE;
  }

  msg_box("Term \"%s\" not found", search_item[current_search].pattern);
  return error;
}
action_code_t action_do_search(int s, char *cmd, cursor_t cursor)
{
  action_code_t error = E_SUCCESS;
  set_search_term(cmd);

  if (search_item[current_search].used == TRUE)
  {
    search_item[current_search].highlight = TRUE;
    search_item[current_search].color = 1;
    if (s == '/')
      search_item[current_search].search_window = SEARCH_ASCII;
    else
      search_item[current_search].search_window = SEARCH_HEX;

    print_screen(display_info.page_start);

    action_move_cursor_next_search(cursor);
  }

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

  search_item[current_search].highlight = FALSE;

  print_screen(display_info.page_start);

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

  if (vf_undo(current_file, count, &caddr))
  {
    update_display_info();
    if (address_invalid(caddr))
      caddr = display_info.file_size - 1;
    if (caddr < 0)
      caddr = 0;
    place_cursor(caddr, CALIGN_NONE, CURSOR_REAL);
    print_screen(display_info.page_start);
  }
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
        for (i=position; i<count; i++)
        {
          cbuff[i-1] = cbuff[i];
          mvwaddch(w, y, i, cbuff[i]);
        }
        mvwaddch(window_list[WINDOW_STATUS], 0, count, ' ');
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

void *save_status_update_thread(void *percent_complete)
{
  int *complete = percent_complete, i=0;
  struct timespec sleep;
  struct timespec slept;

  sleep.tv_sec = 0;
  sleep.tv_nsec = 100000000;

  while(*complete != 100)
  {
    nanosleep(&sleep, &slept);

    werase(save_window);
    box(save_window, 0, 0);
    wattron(save_window, A_STANDOUT);
    for (i=1; i<=((*complete * (SAVE_BOX_W-2))/100); i++)
      mvwprintw(save_window, 1, i, " ");
    wattroff(save_window, A_STANDOUT);
    mvwprintw(save_window, 2, 1, "Saving... %d%%", *complete);
    wrefresh(save_window);
  }
  pthread_exit(NULL);
}
action_code_t action_save(void)
{
  action_code_t error = E_SUCCESS;
  int complete, i=0;
  char file_name[MAX_FILE_NAME];
  BOOL status;
  off_t size;
  pthread_t save_status_thread;
  pthread_attr_t attr;
  void *pthread_status;
  struct timespec sleep;
  struct timespec slept;

  if (vf_need_create(current_file))
  {
    status = file_name_prompt(file_name);
    if (status == FALSE)
      return E_INVALID;
    status = vf_create_file(current_file, file_name);
    if (status == FALSE)
      return E_INVALID;
  }
  if (vf_need_save(current_file))
  {
    save_window = newwin(SAVE_BOX_H, SAVE_BOX_W, SAVE_BOX_Y, SAVE_BOX_X);
    box(save_window, 0, 0);
    curs_set(0);
    mvwprintw(save_window, 2, 1, "Saving... 0%%");

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&save_status_thread, &attr, save_status_update_thread,
                   (void *)&complete);
    pthread_attr_destroy(&attr);

    size = vf_save(current_file, &complete);
    if (size != display_info.file_size)
    {
      msg_box("Only saved %d bytes, should have saved %d bytes!!",
              size, display_info.file_size);
      curs_set(1);
      return E_INVALID;
    }

    pthread_join(save_status_thread, &pthread_status);
    werase(save_window);
    box(save_window, 0, 0);
    wattron(save_window, A_STANDOUT);
    for (i=1; i<(SAVE_BOX_W-1); i++)
      mvwprintw(save_window, 1, i, " ");
    wattroff(save_window, A_STANDOUT);
    mvwprintw(save_window, 2, 1, "Saving... 100%%");
    wrefresh(save_window);
    sleep.tv_sec = 1;
    sleep.tv_nsec = 0;
    nanosleep(&sleep, &slept);

    delwin(save_window);
    curs_set(1);
    print_screen(display_info.page_start);
  }
  return error;
}
action_code_t action_save_all(void)
{
  action_code_t error = E_SUCCESS;
  file_manager_t *tmp_file, *holder;

  holder = current_file;
  do
  {
    tmp_file = vf_get_next_fm_from_ring(file_ring);
    if (vf_need_save(tmp_file))
    {
      current_file = tmp_file;
      update_display_info();
      print_screen(0);
      action_save();
    }
  } while (tmp_file != holder);
  current_file = holder;

  return error;
}

action_code_t action_save_as(char *name)
{
  action_code_t error = E_SUCCESS;
  int complete;
  BOOL status;

  if (vf_need_create(current_file))
  {
    status = vf_create_file(current_file, name);
    if (status == FALSE)
    {
      msg_box("Could not create \"%s\" (does the file already exist?)", name);
      return E_INVALID;
    }
  }
  else
  {
    status = vf_copy_file(current_file, name);
    if (status == FALSE)
    {
      msg_box("Could not create \"%s\" (does the file already exist?)", name);
      return E_INVALID;
    }
  }

  vf_save(current_file, &complete);
  return error;
}

action_code_t action_quit(BOOL force)
{
  action_code_t error = E_SUCCESS;
  if (vf_need_save(current_file) && force == FALSE)
    msg_box("File has unsaved chages");
  else
  {
    vf_remove_fm_from_ring(file_ring, current_file);
    current_file = vf_get_current_fm_from_ring(file_ring);
    if (current_file == NULL)
    {
      app_state.quit = TRUE;
    }
    else
    {
      update_display_info();
      print_screen(display_info.page_start);
    }
  }

  return error;
}
action_code_t action_quit_all(BOOL force)
{
  action_code_t error = E_SUCCESS;
  file_manager_t *tmp_file;

  if (force != TRUE)
  {
    while ((tmp_file = vf_get_next_fm_from_ring(file_ring)) != current_file)
    {
      if (vf_need_save(tmp_file))
      {
        msg_box("Files have unsaved chages");
        return E_INVALID;
      }
    }
  }

  app_state.quit = TRUE;
  return error;
}

action_code_t action_blob_shift_right(void)
{
  int new_blob_shift_value;

  if (user_prefs[BLOB_GROUPING].value == 0)
    return E_INVALID;

  new_blob_shift_value = user_prefs[BLOB_GROUPING_OFFSET].value;
  new_blob_shift_value++;

  if (new_blob_shift_value >= user_prefs[BLOB_GROUPING].value)
    new_blob_shift_value = 0;

  user_prefs[BLOB_GROUPING_OFFSET].value = new_blob_shift_value;

  print_screen(display_info.page_start);

  return E_SUCCESS;
}
action_code_t action_blob_shift_left(void)
{
  int new_blob_shift_value;

  if (user_prefs[BLOB_GROUPING].value == 0)
    return E_INVALID;

  new_blob_shift_value = user_prefs[BLOB_GROUPING_OFFSET].value;
  new_blob_shift_value--;

  if (new_blob_shift_value <= -user_prefs[BLOB_GROUPING].value)
    new_blob_shift_value = user_prefs[BLOB_GROUPING].value-1;

  user_prefs[BLOB_GROUPING_OFFSET].value = new_blob_shift_value;

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

  user_prefs[BLOB_GROUPING].value = new_blob_value;

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

  user_prefs[BLOB_GROUPING].value = new_blob_value;

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

action_code_t action_load_next_file(void)
{
  current_file = vf_get_next_fm_from_ring(file_ring);
  reset_display_info();
  print_screen(display_info.page_start);
}
action_code_t action_load_prev_file(void)
{
  current_file = vf_get_last_fm_from_ring(file_ring);
  reset_display_info();
  print_screen(display_info.page_start);
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

