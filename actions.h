#include "virt_file.h"

#ifndef __ACTIONS_H__
#define __ACTIONS_H__

typedef enum action_code_e
{
  E_SUCCESS,
  E_NO_ACTION,
  E_INVALID,
} action_code_t;

action_code_t action_cursor_move_up(int count);
action_code_t action_cursor_move_down(int count);
action_code_t action_cursor_move_left(int count);
action_code_t action_cursor_move_right(int count);
action_code_t action_cursor_move_line_start(void);
action_code_t action_cursor_move_file_start(void);
action_code_t action_cursor_move_file_end(void);
action_code_t action_cursor_move_line_end(void);
action_code_t action_page_down(void);
action_code_t action_page_up(void);
action_code_t action_jump_to(off_t jump_addr);
action_code_t action_align_top(void);
action_code_t action_align_middle(void);
action_code_t action_align_bottom(void);
action_code_t action_delete(int count);
action_code_t action_insert_before(int count, char *buf, int len);
action_code_t action_insert_after(int count, char *buf, int len);
action_code_t action_paste_before(int count);
action_code_t action_paste_after(int count);
action_code_t action_yank(int count);
action_code_t action_append(void);
action_code_t action_replace(int count);
action_code_t action_replace_insert(int count);
action_code_t action_overwrite(int count);
action_code_t action_save(void);
action_code_t action_save_as(void);
action_code_t action_discard_changes(void);
action_code_t action_close_file(void);
action_code_t action_open_file(void);
action_code_t action_exit(void);
action_code_t action_cursor_to_hex(void);
action_code_t action_cursor_to_ascii(void);
action_code_t action_cursor_toggle_hex_ascii(void);
action_code_t action_visual_select_on(void);
action_code_t action_visual_select_off(void);
action_code_t action_visual_select_toggle(void);
action_code_t action_search_highlight(void);
action_code_t action_clear_search_highlight(void);
off_t action_get_mark(int m);
action_code_t action_set_mark(int m);
action_code_t action_undo(int count);

#endif /* __ACTIONS_H__ */
