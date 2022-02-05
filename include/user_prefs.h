/*
 * Defines/structures/prototypes related to user preferences
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

#include "actions.h"
#include "virt_file.h"

#ifndef __USER_PREFS_H__
# define __USER_PREFS_H__

# define MAX_GRP 16

typedef enum
{
  P_NONE,
  P_BOOL,
  P_INT,
} p_flags_e;

typedef struct user_pref_s
{
  char *name;
  char *short_name;
  int value;
  int def;
  int min;
  int max;
  p_flags_e flags;
} user_pref_t;

typedef enum
{
  DISPLAY_BINARY,
  LIL_ENDIAN,
  GROUPING,
  BLOB_GROUPING,
  BLOB_GROUPING_OFFSET,
  MAX_COLS,
  SEARCH_HL,
  SEARCH_IMMEDIATE,
  IGNORECASE,
  MAX_MATCH
} user_pref_e;

extern user_pref_t user_prefs[];

action_code_t set_pref(const char *option, const char *value);
void read_rc_file(void);

#endif /* __USER_PREFS_H__ */
