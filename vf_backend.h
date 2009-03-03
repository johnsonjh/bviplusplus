#include "virt_file.h"

#define MAP_SIZE_IN_PAGES 1000
#define MAP_OVERLAP_IN_PAGES 10


size_t _insert_before(vbuf_t * vb, char *buf, off_t offset, size_t len,
                      vbuf_undo_list_t ** undo_list);
size_t _replace(vbuf_t * vb, char *buf, off_t offset, size_t len,
                vbuf_list_t ** vb_list);
size_t _delete(vbuf_t * vb, off_t offset, size_t len,
                 vbuf_list_t ** vb_list);
char _get_char(vbuf_t * vb, char *result, off_t offset);
size_t _get_buf(vbuf_t * vb, char *dest, off_t offset, size_t len);
