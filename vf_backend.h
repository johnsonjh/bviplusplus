#include "virt_file.h"

#ifndef __VIRT_FILE_H__
#define __VIRT_FILE_H__

#define MAP_SIZE_IN_PAGES 1000
#define MAP_OVERLAP_IN_PAGES 10


void cleanup(file_manager_t * f);
inline void compute_percent_complete(off_t offset, off_t size, int *complete);
void mod_start_offset(vbuf_t * vb, off_t shift, BOOL increase);
void mod_parent_size(vbuf_t * vb, off_t shift, BOOL increase);
void prune(vbuf_undo_list_t * undo_list);
size_t _insert_before(vbuf_t * vb, char *buf, off_t offset, size_t len,
                      vbuf_undo_list_t ** undo_list);
size_t _replace(vbuf_t * vb, char *buf, off_t offset, size_t len,
                vbuf_list_t ** vb_list);
size_t _delete(vbuf_t * vb, off_t offset, size_t len,
                 vbuf_list_t ** vb_list);
char _get_char(vbuf_t * vb, char *result, off_t offset);
size_t _get_buf(vbuf_t * vb, char *dest, off_t offset, size_t len);

#endif /* __VIRT_FILE_H__ */

