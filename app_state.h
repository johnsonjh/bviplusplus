/*************************************************************
 *
 * File:        app_state.h
 * Author:      David Kelley
 * Description: Make available aplication state variables
 *
 *************************************************************/

#include "virt_file.h"
#include "creadline.h"

#ifndef __APP_STATE_H__
#define __APP_STATE_H__

typedef struct app_state_s
{
  BOOL    quit;
  BOOL    search_since_cancel;
} app_state_t;

extern app_state_t     app_state;
extern file_manager_t *current_file;
extern vf_ring_t      *file_ring;
extern vf_stat_t       vfstat;
extern cmd_hist_t     *ascii_search_hist;
extern cmd_hist_t     *hex_search_hist;
extern cmd_hist_t     *cmd_hist;
extern cmd_hist_t     *file_hist;

#endif /* __APP_STATE_H__ */

