/*
 * Defines/structures/prototypes related to searching
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

#include "virt_file.h"

#ifndef __SEARCH_H__
# define __SEARCH_H__

# define MAX_SEARCHES              8
# define MAX_RANGE_COUNT         256
# define MAX_SEARCH_PAT_LEN      256
# define LONG_SEARCH_BUF_SIZE ( 1024 * 1024 * 2 )

typedef enum
{
  SEARCH_HEX,
  SEARCH_ASCII
} search_window_t;

typedef enum
{
  SEARCH_BACKWARD,
  SEARCH_FORWARD
} search_direction_t;

typedef enum
{
  NO_MATCH,
  INCOMPLETE_MATCH,
  MATCH_FOUND,
  MATCH_ERROR
} search_result_t;

typedef enum
{
  NO_WILDCARD,
  NONE_OR_ONE,
  NONE_OR_MORE,
  ONE_OR_MORE,
  ONE_ONLY
} wildcard_t;

typedef enum
{
  UNFULFILLED,
  FULFILLED,
  UNIQUELY_FULFILLED
} match_state_t;

typedef struct search_state_s
{
  int criteria_index;
  match_state_t match_state[MAX_SEARCH_PAT_LEN];
} search_state_t;

typedef struct match_criteria_s
{
  unsigned char range[256];
  int range_count;
  wildcard_t wildcard;
} match_criteria_t;

typedef struct compiled_pattern_s
{
  int criteria_count;
  match_criteria_t *criteria[MAX_SEARCH_PAT_LEN];
} compiled_pattern_t;

typedef struct search_item_s
{
  char pattern[MAX_SEARCH_PAT_LEN];
  compiled_pattern_t compiled_pattern;
  BOOL used;
  BOOL highlight;
  int color;
  search_window_t search_window;
} search_item_t;

typedef struct search_aid_s
{
  char *buf;
  int buf_size;
  off_t buf_start_addr;
  off_t display_addr;
  off_t hl_start;
  off_t hl_end;
} search_aid_t;

extern search_item_t search_item[];
extern int current_search;

void buf_search(search_aid_t *search_aid);
void set_search_term(char *pattern);
void search_init(void);
void search_cleanup(void);
void fill_search_buf(off_t addr, int display_size, search_aid_t *search_aid,
                     search_direction_t direction);
void free_search_buf(search_aid_t *search_aid);

#endif /* __SEARCH_H__ */
