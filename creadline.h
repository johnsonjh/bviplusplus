#include <ncurses.h>

#ifndef __CREADLINE_H__
#define __CREADLINE_H__

#define MAX_CMD_BUF 256
#define MAX_CMD_HISTORY 100

typedef struct cmd_item_s
{
  char cbuff[MAX_CMD_BUF];
  int position;
  int count;
} cmd_item_t;

typedef struct cmd_hist_s
{
  cmd_item_t *item;
  int hist_index;
} cmd_hist_t;

char *creadline(const char *prompt, WINDOW *w, int y, int x, cmd_hist_t *history);
cmd_hist_t *new_history(void);
void free_history(cmd_hist_t *history);

#endif /* __CREADLINE_H__ */

