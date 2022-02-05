/*
 * Make available aplication state variables
 *
 * Copyright (C) 2008, 2009, 2010 David Kelley
 * Copyright (c) 2009 Steve Lewis
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

#include "creadline.h"
#include "virt_file.h"

#ifndef __APP_STATE_H__
# define __APP_STATE_H__

typedef struct app_state_s
{
  BOOL quit;
  BOOL search_since_cancel;
} app_state_t;

extern app_state_t app_state;
extern file_manager_t *current_file;
extern vf_ring_t *file_ring;
extern vf_stat_t vfstat;
extern cmd_hist_t *ascii_search_hist;
extern cmd_hist_t *hex_search_hist;
extern cmd_hist_t *cmd_hist;
extern cmd_hist_t *file_hist;

#endif /* __APP_STATE_H__ */
