#include "virt_file.h"
#include "windows.h"

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define HEX(x) ((x) < 0xA ? '0' + (x) : 'a' + (x) - 0xa)

typedef struct display_info_s
{
  off_t    file_size;
  off_t    page_start;
  off_t    page_end;
  off_t    cursor_addr;
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

void search_hl(BOOL on);
void blob_standout(BOOL on);
int print_line(off_t addr, int y);
void place_cursor(off_t addr, cursor_alignment_e calign);
void print_screen(off_t addr);

#endif /* __DISPLAY_H__ */
