/*************************************************************
 *
 * File:        user_prefs.c
 * Author:      David Kelley
 * Description: User preferences and related functions
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

#include "user_prefs.h"

 /*------------------------------------------------*/
/*  [name]                  [short_name]  [value]  [default]  [min]  [max]        [flags]*/
user_pref_t user_prefs[] = {
  { "binary",               "bin",              0,         0,     0,     0,       P_BOOL },
  { "little_endian",        "le",               0,         0,     0,     0,       P_BOOL },
  { "grouping",             "grp",              1,         1,     1,     MAX_GRP, P_INT },
  { "blob_grouping",        "blob",             0,         0,     0,     0,       P_INT },
  { "blob_grouping_offset", "bloboff",          0,         0,     0,     0,       P_INT },
  { "columns",              "cols",             0,         0,     1,     0,       P_INT },
  { "search_hl",            "hl",               1,         1,     0,     0,       P_BOOL },
  { "search_immediate",     "si",               1,         1,     0,     0,       P_BOOL },
  { "ignorecase",           "ic",               0,         0,     0,     0,       P_BOOL },
  { "max_match",            "mm",             256,       256,     0,     0,       P_INT },
  { "",                     "",                 0,         0,     0,     0,       P_NONE },
};

