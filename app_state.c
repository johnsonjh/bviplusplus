/*************************************************************
 *
 * File:        app_state.c
 * Author:      David Kelley
 * Description: Aplication state variables
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
#include "app_state.h"

app_state_t     app_state;
file_manager_t *current_file;
vf_ring_t      *file_ring;
vf_stat_t       vfstat;
cmd_hist_t     *ascii_search_hist;
cmd_hist_t     *hex_search_hist;
cmd_hist_t     *cmd_hist;
cmd_hist_t     *file_hist;

