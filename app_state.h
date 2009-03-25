#include "virt_file.h"

#ifndef __APP_STATE_H__
#define __APP_STATE_H__

typedef struct app_state_s
{
  BOOL    quit;
} app_state_t;

extern app_state_t     app_state;
extern file_manager_t *current_file;
vf_ring_t             *file_ring;
extern vf_stat_t       vfstat;

#endif /* __APP_STATE_H__ */

