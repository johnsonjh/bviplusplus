/*************************************************************
 *
 * File:        user_prefs.h
 * Author:      David Kelley
 * Description: Defines, structures, and function prototypes
 *              related to user preferences
 *
 * Copyright (C) 2009 David Kelley
 *
 * This file is part of bviplus.
 *
 * Bviplus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bviplus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bviplus.  If not, see <http://www.gnu.org/licenses/>.
 *
 *************************************************************/

#include "virt_file.h"
#include "actions.h"

#ifndef __USER_PREFS_H__
#define __USER_PREFS_H__

#define MAX_GRP 16

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

