/*************************************************************
 *
 * File:        help.h
 * Author:      David Kelley
 * Description: Defines, structures, and function prototypes
 *              related to in program help
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

#ifndef __HELP_H__
#define __HELP_H__

#define MAX_LINES_SCROLLBOX 4096

extern char *help_text[];

void scrollable_window_display(char **text);
void big_buf_display(char *buf, int max_size);

#endif /* __HELP_H__ */
