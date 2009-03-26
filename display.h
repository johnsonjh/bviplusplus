#include "virt_file.h"
#include "windows.h"

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

#define HEX(x) ((x) < 0xA ? '0' + (x) : 'a' + (x) - 0xa)
#define MSG_BOX_H 8
#define MSG_BOX_W 50
#define MSG_BOX_Y (((HEX_BOX_H - MSG_BOX_H) / 2) + HEX_BOX_Y)
#define MSG_BOX_X (((HEX_BOX_W - MSG_BOX_W) / 2) + HEX_BOX_X)
#define MAX_MSG_BOX_LEN ((MSG_BOX_H - 3) * (MSG_BOX_W - 2))

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
int get_y_from_addr(off_t addr);
int get_x_from_addr(off_t addr);
off_t get_addr_from_xy(int x, int y);
void destroy_screen();
void create_screen();
void msg_box(char *fmt, ...);
void reset_display_info(void);
void update_display_info(void);
void search_hl(BOOL on);
void blob_standout(BOOL on);
int is_visual_on(void);
int visual_span(void);
off_t visual_addr(void);
int print_line(off_t addr, int y);
void place_cursor(off_t addr, cursor_alignment_e calign, cursor_t cursor);
void print_screen(off_t addr);

#endif /* __DISPLAY_H__ */
