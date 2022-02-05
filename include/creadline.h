/*
 * Defines/structures/prototypes relates to reading command line input
 *
 * Copyright (c) 2008, 2009, 2010 David Kelley
 * Copyright (c) 2016 The Lemon Man
 * Copyright (c) 2022 Jeffrey H. Johnson <trnsz@pobox.com>
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

#include <ncurses.h>

#ifndef __CREADLINE_H__
# define __CREADLINE_H__

# define MAX_CMD_BUF     256
# define MAX_CMD_HISTORY 100

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

char *creadline(const char *prompt, WINDOW *w, int y, int x,
                cmd_hist_t *history);
cmd_hist_t *new_history(void);
void free_history(cmd_hist_t *history);

#endif /* __CREADLINE_H__ */
