#include "virt_file.h"
#include "app_state.h"

app_state_t     app_state;
file_manager_t *current_file;
vf_ring_t      *file_ring;
vf_stat_t       vfstat;

