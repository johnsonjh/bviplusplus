/*************************************************************
 *
 * File:        display.c
 * Author:      David Kelley
 * Description: Defines, structures, and functions prototypes
 *              related to rendering the data on the screen
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

#include <ncurses.h>
#include <panel.h>
#include "virt_file.h"
#include "search.h"

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define MENU_BOX_X    0
#define MENU_BOX_Y    0
#define MENU_BOX_W    COLS
#define MENU_BOX_H    1

#define ADDR_DIGITS   8
#define ADDR_BOX_X    0
#define ADDR_BOX_Y    MENU_BOX_H
#define ADDR_BOX_W    (ADDR_DIGITS + 1)
#define ADDR_BOX_H_   (LINES - MENU_BOX_H - WINDOW_STATUS_H)
#define ADDR_BOX_H    (ADDR_BOX_H_ > 3 ? ADDR_BOX_H_ : 3)


#define STATUS_1_Y 0
#define STATUS_1_X (COLS - 30)
#define STATUS_2_Y 1
#define STATUS_2_X STATUS_1_X

#define WINDOW_STATUS_H 2
#define WINDOW_STATUS_W COLS
#define WINDOW_STATUS_Y MENU_BOX_H + ADDR_BOX_H
#define WINDOW_STATUS_X 0

#define HEX_BYTE_DIGITS   (user_prefs[DISPLAY_BINARY].value ? 8 : 2)
#define ASCII_BYTE_DIGITS 1
#define BYTE_DIGITS       (display_info.cursor_window == WINDOW_HEX ? HEX_BYTE_DIGITS : ASCII_BYTE_DIGITS )
#define SPACE_DIGITS      1

/* each hex byte is represented in hex by 2(hex)/8(binary) digits, plus each grouping has one trailing space */
#define HEX_BYTES_PER_GROUP   ((user_prefs[GROUPING].value * HEX_BYTE_DIGITS) + SPACE_DIGITS)
#define ASCII_BYTES_PER_GROUP (user_prefs[GROUPING].value * ASCII_BYTE_DIGITS)
#define BYTES_PER_GROUP       (display_info.cursor_window == WINDOW_HEX ? HEX_BYTES_PER_GROUP : ASCII_BYTES_PER_GROUP)

/* width of hex box + ascii box */
#define SHARED_WIDTH ((COLS - ADDR_BOX_W) < 4 ? 4 : (COLS - ADDR_BOX_W))

#define MAX_HEX_COLS    ((SHARED_WIDTH-4)/((3*ASCII_BYTES_PER_GROUP) + 1))
#define HEX_COLS        (user_prefs[MAX_COLS].value == 0 ? MAX_HEX_COLS : MAX_HEX_COLS > user_prefs[MAX_COLS].value ? user_prefs[MAX_COLS].value : MAX_HEX_COLS)
#define BYTES_PER_LINE (HEX_COLS * user_prefs[GROUPING].value)
#define HEX_LINES       (HEX_BOX_H - 2)


#define HEX_BOX_X     (ADDR_BOX_X + ADDR_BOX_W)
#define HEX_BOX_Y     MENU_BOX_H

#define HEX_BOX_W_    (HEX_COLS * HEX_BYTES_PER_GROUP + 2)

#define HEX_BOX_W     (HEX_BOX_W_ > (HEX_BYTES_PER_GROUP + 2) ? HEX_BOX_W_ : (HEX_BYTES_PER_GROUP + 2))
#define HEX_BOX_H     ADDR_BOX_H

#define ASCII_BOX_X   (HEX_BOX_X + HEX_BOX_W)
#define ASCII_BOX_Y   MENU_BOX_H
#define ASCII_BOX_W_  (SHARED_WIDTH - HEX_BOX_W)
#define ASCII_BOX_W   (ASCII_BOX_W_ > (user_prefs[GROUPING].value + 2) ? ASCII_BOX_W_ : (user_prefs[GROUPING].value + 2))
#define ASCII_BOX_H   ADDR_BOX_H

#define PAGE_SIZE (HEX_LINES * BYTES_PER_LINE)
#define _PAGE_END (display_info.page_start + PAGE_SIZE - 1)
#define PAGE_END  (_PAGE_END > display_info.file_size ? display_info.file_size - 1 : _PAGE_END)

#define HEX(x) ((x) < 0xA ? '0' + (x) : 'a' + (x) - 0xa)
#define MSG_BOX_H 8
#define MSG_BOX_W 50
#define MSG_BOX_Y (((HEX_BOX_H - MSG_BOX_H) / 2) + HEX_BOX_Y)
#define MSG_BOX_X (((HEX_BOX_W - MSG_BOX_W) / 2) + HEX_BOX_X)
#define MAX_MSG_BOX_LEN ((MSG_BOX_H - 3) * (MSG_BOX_W - 2))


#define MAX_FILE_NAME 265
#define FILE_BOX_H 4
#define FILE_BOX_W ((HEX_BOX_W - 8) > 4 ? (HEX_BOX_W - 8) : 4)
#define FILE_BOX_Y (((HEX_BOX_H - FILE_BOX_H) / 2) + HEX_BOX_Y)
#define FILE_BOX_X (((HEX_BOX_W - FILE_BOX_W) / 2) + HEX_BOX_X)

#define INSERT_BOX_H 4
#define INSERT_BOX_W 50
#define INSERT_BOX_Y (((HEX_BOX_H - INSERT_BOX_H) / 2) + HEX_BOX_Y)
#define INSERT_BOX_X (((HEX_BOX_W - INSERT_BOX_W) / 2) + HEX_BOX_X)
#define MAX_INSERT_BOX_LEN (INSERT_BOX_W - 2)

#define SCROLL_BOX_H HEX_BOX_H
#define SCROLL_BOX_W HEX_BOX_W
#define SCROLL_BOX_Y HEX_BOX_Y
#define SCROLL_BOX_X HEX_BOX_X

#define SAVE_BOX_H 4
#define SAVE_BOX_W 52
#define SAVE_BOX_Y (((HEX_BOX_H - SAVE_BOX_H) / 2) + HEX_BOX_Y)
#define SAVE_BOX_X (((HEX_BOX_W - SAVE_BOX_W) / 2) + HEX_BOX_X)

#define MAX_STATUS 64

typedef enum window_e
{
  WINDOW_MENU,
  WINDOW_ADDR,
  WINDOW_HEX,
  WINDOW_ASCII,
  WINDOW_STATUS,
  MAX_WINDOWS
} window_t;

typedef struct display_info_s
{
  off_t    file_size;
  off_t    page_start;
  off_t    page_end;
  off_t    cursor_addr;
  off_t    virtual_cursor_addr;
  off_t visual_select_addr;
  window_t cursor_window;
  int      max_cols;
  BOOL     has_color;
  char     percent[4];
  char     status[MAX_STATUS];
} display_info_t;

typedef enum cursor_alignment
{
  CALIGN_TOP,
  CALIGN_MIDDLE,
  CALIGN_BOTTOM,
  CALIGN_NONE,
} cursor_alignment_e;

typedef enum cursor_e
{
  CURSOR_REAL,
  CURSOR_VIRTUAL
} cursor_t;


extern display_info_t display_info;
extern WINDOW *window_list[MAX_WINDOWS];
extern PANEL *panel_list[MAX_WINDOWS];

off_t address_invalid(off_t addr);
int get_y_from_page_offset(int offset);
int get_x_from_page_offset(int offset);
int get_y_from_addr(off_t addr);
int get_x_from_addr(off_t addr);
off_t get_addr_from_xy(int x, int y);
void destroy_screen(void);
void create_screen(void);
void pat_err(const char *error, const char *pattern, int index, int max_index);
void msg_box(const char *fmt, ...);
BOOL msg_prompt(char *fmt, ...);
void reset_display_info(void);
void update_display_info(void);
void update_percent(void);
void update_status(const char *msg);
void search_hl(BOOL on);
void blob_standout(BOOL on);
int is_visual_on(void);
int visual_span(void);
off_t visual_addr(void);
int print_line(off_t page_addr, off_t line_addr, char *screen_buf, int screen_buf_size, search_aid_t *search_aid);
void update_status_window(void);
void place_cursor(off_t addr, cursor_alignment_e calign, cursor_t cursor);
void print_screen_buf(off_t addr, char *screen_buf, int screen_buf_size, search_aid_t *search_aid);
void print_screen(off_t addr);

#endif /* __DISPLAY_H__ */
