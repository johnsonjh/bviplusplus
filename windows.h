#include <ncurses.h>
#include <panel.h>
#include <unistd.h>
#include "user_prefs.h"

#ifndef __BVIPLUS_WINDOWS_H__
#define __BVIPLUS_WINDOWS_H__

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

#define WINDOW_STATUS_H 1
#define WINDOW_STATUS_W COLS
#define WINDOW_STATUS_Y MENU_BOX_H + ADDR_BOX_H
#define WINDOW_STATUS_X 0

#define BYTE_DIGITS   (user_prefs[DISPLAY_BINARY].value ? 8 : 2)

/* each byte is represented in hex by two digits, plus each grouping has one trailing space */
#define BYTES_PER_GROUP ((user_prefs[GROUPING].value * BYTE_DIGITS) + 1)

/* width of hex box + ascii box */
#define SHARED_WIDTH (COLS - ADDR_BOX_W)

#define HEX_BOX_X     (ADDR_BOX_X + ADDR_BOX_W)
#define HEX_BOX_Y     MENU_BOX_H
//#define HEX_BOX_W     ((SHARED_WIDTH * BYTES_PER_GROUP) / (BYTES_PER_GROUP + user_prefs.grouping))
//#define HEX_BOX_W     ((((SHARED_WIDTH - 2) * BYTES_PER_GROUP) + 2 * user_prefs[GROUPING].value) / (BYTES_PER_GROUP + user_prefs[GROUPING].value))
#define HEX_BOX_W_    ((((SHARED_WIDTH - 2) * BYTES_PER_GROUP) + 2 * user_prefs[GROUPING].value) / (BYTES_PER_GROUP + user_prefs[GROUPING].value))
#define HEX_BOX_W     (HEX_BOX_W_ > (BYTES_PER_GROUP + 2) ? HEX_BOX_W_ : (BYTES_PER_GROUP + 2))
#define HEX_BOX_H     ADDR_BOX_H

#define ASCII_BOX_X   (HEX_BOX_X + HEX_BOX_W)
#define ASCII_BOX_Y   MENU_BOX_H
#define ASCII_BOX_W_  (SHARED_WIDTH - HEX_BOX_W)
#define ASCII_BOX_W   (ASCII_BOX_W_ > (user_prefs[GROUPING].value + 2) ? ASCII_BOX_W_ : (user_prefs[GROUPING].value + 2))
#define ASCII_BOX_H   ADDR_BOX_H

//#define MAX_HEX_COLS    ((HEX_BOX_W - 3) / BYTES_PER_GROUP)
#define MAX_HEX_COLS    ((HEX_BOX_W - 2) / BYTES_PER_GROUP)
#define HEX_COLS        (user_prefs[MAX_COLS].value == 0 ? MAX_HEX_COLS : MAX_HEX_COLS > user_prefs[MAX_COLS].value ? user_prefs[MAX_COLS].value : MAX_HEX_COLS)
#define BYTES_PER_LINE  (HEX_COLS * user_prefs[GROUPING].value)
#define HEX_LINES       (HEX_BOX_H - 2)

#define PAGE_SIZE ((HEX_LINES * BYTES_PER_LINE) - 1)
#define _PAGE_END (display_info.page_start + PAGE_SIZE)
#define PAGE_END  (_PAGE_END > display_info.file_size ? display_info.file_size : _PAGE_END)

typedef enum window_e
{
  WINDOW_MENU,
  WINDOW_ADDR,
  WINDOW_HEX,
  WINDOW_ASCII,
  WINDOW_STATUS,
  MAX_WINDOWS
} window_t;


extern WINDOW *window_list[MAX_WINDOWS];
extern PANEL *panel_list[MAX_WINDOWS];

off_t address_invalid(off_t addr);
int get_y_from_addr(off_t addr);
int get_x_from_addr(off_t addr);
off_t get_addr_from_xy(int x, int y);
void destroy_screen();
void create_screen();

#endif /* __BVIPLUS_WINDOWS_H__ */

