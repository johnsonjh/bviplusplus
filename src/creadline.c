/*
 * Functions for reading command line input, similar to realine
 *
 * Copyright (c) 2008, 2009, 2010 David Kelley
 * Copyright (c) 2009 Steve Lewis
 * Copyright (c) 2016 The Lemon Man
 * Copyright (c) 2021 Sergei Trofimovich
 * Copyright (c) 2021, 2022 Jeffrey H. Johnson <trnsz@pobox.com>
 *
 * This file is part of bviplusplus.
 *
 * Bviplusplus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bviplusplus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bviplusplus.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>

#include "creadline.h"
#include "key_handler.h"

cmd_hist_t *
new_history(void)
{
  cmd_hist_t *cmd_hist;
  cmd_item_t *cmd_item;

  cmd_hist = malloc(sizeof ( cmd_hist_t ));
  if (NULL == cmd_hist)
    {
      return NULL;
    }

  memset(cmd_hist, 0, sizeof ( cmd_hist_t ));

  cmd_item = malloc(sizeof ( cmd_item_t ) * MAX_CMD_HISTORY);
  if (NULL == cmd_item)
    {
      free(cmd_hist);
      return NULL;
    }

  memset(cmd_item, 0, sizeof ( cmd_item_t ) * MAX_CMD_HISTORY);

  cmd_hist->item = cmd_item;
  return cmd_hist;
}

void
free_history(cmd_hist_t *history)
{
  if (NULL != history)
    {
      if (NULL != history->item)
        {
          free(history->item);
        }

      free(history);
    }
}

char *
creadline(const char *prompt, WINDOW *w, int y, int x, cmd_hist_t *history)
{
  int i = 0, c = 0;
  int entry_hist_index, tmp_hist_index;
  cmd_item_t tmp_cmd;
  char *cmd;

  mvwprintw(w, y, x, "%s", prompt);
  x = x + strlen(prompt) - 1;

  entry_hist_index = history->hist_index;
  tmp_cmd.count = 0;
  tmp_cmd.position = 0;

  do
    {
      wrefresh(w);
      c = mgetch();
      switch (c)
        {
        case KEY_UP:
          if (NULL == history)
            {
              break;
            }

          tmp_hist_index = history->hist_index - 1;
          if (tmp_hist_index < 0)
            {
              tmp_hist_index = MAX_CMD_HISTORY - 1;
            }

          if (tmp_hist_index == entry_hist_index)
            {
              break;
            }

          if (history->item[tmp_hist_index].count == 0)
            {
              break;
            }

          history->hist_index = tmp_hist_index;

          strncpy(
            tmp_cmd.cbuff,
            history->item[history->hist_index].cbuff,
            MAX_CMD_BUF);
          tmp_cmd.count = history->item[history->hist_index].count;
          tmp_cmd.position = history->item[history->hist_index].position;

          wmove(w, y, x + 1);
          wclrtoeol(w);

          for (i = 0; i < tmp_cmd.count; i++)
            {
              mvwaddch(w, y, x + i + 1, tmp_cmd.cbuff[i]);
            }

          tmp_cmd.position = tmp_cmd.count;
          wmove(w, y, x + tmp_cmd.position + 1);
          break;

        case KEY_DOWN:
          if (NULL == history)
            {
              break;
            }

          if (history->hist_index == entry_hist_index)
            {
              break;
            }

          tmp_hist_index = history->hist_index + 1;
          tmp_hist_index = tmp_hist_index % MAX_CMD_HISTORY;

          history->hist_index = tmp_hist_index;

          strncpy(
            tmp_cmd.cbuff,
            history->item[history->hist_index].cbuff,
            MAX_CMD_BUF);
          tmp_cmd.count = history->item[history->hist_index].count;
          tmp_cmd.position = history->item[history->hist_index].position;

          wmove(w, y, x + 1);
          wclrtoeol(w);

          for (i = 0; i < tmp_cmd.count; i++)
            {
              mvwaddch(w, y, x + i + 1, tmp_cmd.cbuff[i]);
            }

          tmp_cmd.position = tmp_cmd.count;
          wmove(w, y, x + tmp_cmd.position + 1);
          break;

        case ESC:
          return NULL;

        case BVICTRL('H'):
        case KEY_BACKSPACE:
          if (tmp_cmd.position == 0)
            {
              return NULL;
            }

          for (i = tmp_cmd.position; i < tmp_cmd.count; i++)
            {
              tmp_cmd.cbuff[i - 1] = tmp_cmd.cbuff[i];
              mvwaddch(w, y, x + i, tmp_cmd.cbuff[i]);
            }

          mvwaddch(w, y, x + tmp_cmd.count, ' ');
          wclrtoeol(w);
          wmove(w, y, x + tmp_cmd.position);
          tmp_cmd.position--;
          tmp_cmd.count--;
          break;

        case KEY_LEFT:
          if (--tmp_cmd.position < 0)
            {
              tmp_cmd.position++;
            }

          wmove(w, y, x + tmp_cmd.position + 1);
          break;

        case KEY_RIGHT:
          if (++tmp_cmd.position > tmp_cmd.count)
            {
              tmp_cmd.position--;
            }

          wmove(w, y, x + tmp_cmd.position + 1);
          break;

        case NL:
        case CR:
        case KEY_ENTER:
          break;

        default:
          if (tmp_cmd.count >= MAX_CMD_BUF)
            {
              break;
            }

          for (i = tmp_cmd.count; i >= tmp_cmd.position; i--)
            {
              tmp_cmd.cbuff[i + 1] = tmp_cmd.cbuff[i];
            }

          tmp_cmd.cbuff[tmp_cmd.position] = (char)c;
          tmp_cmd.count++;
          for (i = tmp_cmd.position; i < tmp_cmd.count; i++)
            {
              mvwaddch(w, y, x + i + 1, tmp_cmd.cbuff[i]);
            }

          tmp_cmd.position++;
          wmove(w, y, x + tmp_cmd.position + 1);
          break;
        }
    }
  while ( c != NL && c != CR && c != KEY_ENTER );

  tmp_cmd.cbuff[tmp_cmd.count] = '\0';

  if (tmp_cmd.count)
    {
      cmd = malloc(MAX_CMD_BUF);
      if (cmd == NULL)
        {
          return NULL;
        }

      strncpy(cmd, tmp_cmd.cbuff, MAX_CMD_BUF);
      strncpy(
        history->item[entry_hist_index].cbuff,
        tmp_cmd.cbuff,
        MAX_CMD_BUF);
      history->item[entry_hist_index].count = tmp_cmd.count;
      history->item[entry_hist_index].position = tmp_cmd.position;
      history->hist_index = ( entry_hist_index + 1 ) % MAX_CMD_HISTORY;
      return cmd;
    }
  else
    {
      history->hist_index = entry_hist_index;
      return NULL;
    }
}
