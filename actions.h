/*************************************************************
 *
 * File:        actions.h
 * Author:      David Kelley
 * Description: Defines, structures, and function prototypes
 *              related to program actions
 *
 *************************************************************/

#include "virt_file.h"
#include "display.h"

#ifndef __ACTIONS_H__
#define __ACTIONS_H__

#define INVALID_ADDR -1

typedef enum action_code_e
{
  E_SUCCESS,
  E_NO_ACTION,
  E_INVALID,
} action_code_t;


void          run_external(void);
action_code_t action_cursor_move_up(int count, cursor_t cursor);
action_code_t action_cursor_move_down(int count, cursor_t cursor);
action_code_t action_cursor_move_left(int count, cursor_t cursor);
action_code_t action_cursor_move_right(int count, cursor_t cursor);
action_code_t action_cursor_move_line_start(cursor_t cursor);
action_code_t action_cursor_move_file_start(cursor_t cursor);
action_code_t action_cursor_move_file_end(cursor_t cursor);
action_code_t action_cursor_move_line_end(cursor_t cursor);
action_code_t action_cursor_move_page_down(cursor_t cursor);
action_code_t action_cursor_move_page_up(cursor_t cursor);
action_code_t action_jump_to(off_t jump_addr, cursor_t cursor);
action_code_t action_align_top(void);
action_code_t action_align_middle(void);
action_code_t action_align_bottom(void);
action_code_t action_delete(int count, off_t end_addr); /* delete from cursor to end_addr (end_addr can be INVALID_ADDR) */
action_code_t action_insert_before(int count, char *buf, int len);
action_code_t action_insert_after(int count, char *buf, int len);
action_code_t action_set_yank_register(int c);
action_code_t action_paste_before(int count);
action_code_t action_paste_after(int count);
action_code_t action_init_yank(void);
action_code_t action_clean_yank(void);
action_code_t action_yank(int count, off_t end_addr, BOOL move_cursor); /* yank from cursor to end_addr (end_addr can be INVALID_ADDR) */
action_code_t action_append(void);
action_code_t action_replace(int count);
action_code_t action_discard_changes(void);
action_code_t action_close_file(void);
action_code_t action_open_file(void);
action_code_t action_exit(void);
action_code_t action_cursor_to_hex(void);
action_code_t action_cursor_to_ascii(void);
action_code_t action_cursor_toggle_hex_ascii(void);
int action_visual_select_check(void);
action_code_t action_visual_select_on(void);
action_code_t action_visual_select_off(void);
action_code_t action_visual_select_toggle(void);
action_code_t action_move_cursor_prev_search(cursor_t cursor);
action_code_t  action_move_cursor_next_search(cursor_t cursor, BOOL advance_if_current_match);
action_code_t action_do_search(int s, char *cmd, cursor_t cursor,
                               search_direction_t direction);
action_code_t action_search_highlight(void);
action_code_t action_clear_search_highlight(void);
off_t action_get_mark(int m);
action_code_t action_set_mark(int m);
action_code_t action_undo(int count);
action_code_t action_redo(int count);
action_code_t action_save(void);
action_code_t action_save_all(void);
action_code_t action_save_as(char *name);
action_code_t action_quit(BOOL force);
action_code_t action_quit_all(BOOL force);
action_code_t action_blob_shift_right(int count);
action_code_t action_blob_shift_left(int count);
action_code_t action_blob_inc(void);
action_code_t action_blob_dec(void);
action_code_t action_grp_inc(void);
action_code_t action_grp_dec(void);
action_code_t action_load_next_file(void);
action_code_t action_load_prev_file(void);
action_code_t action_do_resize(void);

#endif /* __ACTIONS_H__ */
