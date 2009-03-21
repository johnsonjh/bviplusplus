#ifndef __ACTIONS_H__
#define __ACTIONS_H__

typedef enum action_code_e
{
  E_SUCCESS,
  E_NO_ACTION,
  E_INVALID,
} action_code_t;

action_code_t action_cursor_move_up(void);
action_code_t action_cursor_move_down(void);
action_code_t action_cursor_move_left(void);
action_code_t action_cursor_move_right(void);
action_code_t action_cursor_move_line_start(void);
action_code_t action_cursor_move_file_start(void);
action_code_t action_cursor_move_file_end(void);
action_code_t action_cursor_move_line_end(void);
action_code_t action_page_down(void);
action_code_t action_page_up(void);
action_code_t action_jump_to(void);
action_code_t action_align_top(void);
action_code_t action_align_middle(void);
action_code_t action_align_bottom(void);
action_code_t action_delete(void);
action_code_t action_insert_before(void);
action_code_t action_insert_after(void);
action_code_t action_append(void);
action_code_t action_replace(void);
action_code_t action_replace_insert(void);
action_code_t action_save(void);
action_code_t action_save_as(void);
action_code_t action_discard_changes(void);
action_code_t action_close_file(void);
action_code_t action_open_file(void);
action_code_t action_exit(void);
action_code_t action_cursor_to_hex(void);
action_code_t action_cursor_to_ascii(void);
action_code_t action_cursor_toggle_hex_ascii(void);
action_code_t action_cursor_move_address(void);
action_code_t action_visual_select_on(void);
action_code_t action_visual_select_off(void);
action_code_t action_block_visual_select_on(void);
action_code_t action_block_visual_select_off(void);

#endif /* __ACTIONS_H__ */
