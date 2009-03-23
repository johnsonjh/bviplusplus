#include "virt_file.h"
#include "windows.h"

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define HEX(x) ((x) < 0xA ? '0' + (x) : 'a' + (x) - 0xa)
#define MSG_BOX_H 8
#define MSG_BOX_W 50
#define MSG_BOX_Y (((HEX_BOX_H - MSG_BOX_H) / 2) + HEX_BOX_Y)
#define MSG_BOX_X (((HEX_BOX_W - MSG_BOX_W) / 2) + HEX_BOX_X)
#define MAX_MSG_BOX_LEN ((MSG_BOX_H - 3) * (MSG_BOX_W - 2))

typedef struct display_info_s
{
  off_t    file_size;
  off_t    page_start;
  off_t    page_end;
  off_t    cursor_addr;
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


extern display_info_t display_info;

void msg_box(char *fmt, ...);
void reset_display_info(void);
void update_display_info(void);
void search_hl(BOOL on);
void blob_standout(BOOL on);
int is_visual_on(void);
int visual_span(void);
off_t visual_addr(void);
int print_line(off_t addr, int y);
void place_cursor(off_t addr, cursor_alignment_e calign);
void print_screen(off_t addr);

#endif /* __DISPLAY_H__ */
