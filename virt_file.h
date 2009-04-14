/******************************************************
 *
 * Project: Virtual File
 * Author:  David Kelley
 * Date:    29 Sept, 2008
 *
 *****************************************************/

#ifndef __VIRT_FILE_H
# define __VIRT_FILE_H

/****************
    INCLUDES
 ***************/
#include <stdio.h>
#include <sys/types.h>

/****************
  MACROS/DEFINES
 ***************/
# ifdef TRUE
#   undef TRUE
# endif
# define TRUE 1
# ifdef FALSE
#   undef FALSE
# endif
# define FALSE 0

#define MAX_PATH_LEN 255

# define MAP_SIZE_IN_PAGES 1000
# define MAP_OVERLAP_IN_PAGES 10

/****************
     TYPES
 ***************/
typedef int BOOL;

typedef enum
{
  TYPE_FILE,
  TYPE_INSERT,
  TYPE_REPLACE,
  TYPE_DELETE,
  MAX_TYPES
} buf_type_e;

typedef struct vbuf_s vbuf_t;
struct vbuf_s
{
  vbuf_t *parent;
  vbuf_t *first_child;
  vbuf_t *next;
  vbuf_t *prev;
  char *buf;
  off_t start;
  off_t size;
  buf_type_e buf_type;
  FILE *fp;
  BOOL active;
};

typedef struct vbuf_list_s vbuf_list_t;
struct vbuf_list_s
{
  /* move fp from vb up to here? or integrate fname into vb? do we want to support inserting a file into a file? */
  vbuf_t *vb;
  vbuf_list_t *next;            /* move across this list */
};

typedef struct vbuf_undo_list_s vbuf_undo_list_t;
struct vbuf_undo_list_s
{
  vbuf_undo_list_t *last;
  vbuf_list_t *vb_list;
  BOOL applied;
  BOOL saved;
};

typedef struct file_manager_s file_manager_t;
struct file_manager_s
{
  char fname[MAX_PATH_LEN + 1];
  vbuf_t fm;
  vbuf_undo_list_t ul;
};

typedef struct vf_stat_s vf_stat_t;
struct vf_stat_s
{
  off_t file_size;
};

typedef struct vf_ring_s vf_ring_t;
struct vf_ring_s
{
  vf_ring_t *next;
  vf_ring_t *last;
  vf_ring_t *head;
  file_manager_t fm;
};


/****************
   PROTOTYPES
 ***************/
vf_ring_t      *vf_create_fm_ring(void);
BOOL            vf_destroy_fm_ring(vf_ring_t *r);
file_manager_t *vf_add_fm_to_ring(vf_ring_t *r);
BOOL vf_remove_fm_from_ring(vf_ring_t *r, file_manager_t *fm);
file_manager_t *vf_get_next_fm_from_ring(vf_ring_t *r);
file_manager_t *vf_get_last_fm_from_ring(vf_ring_t *r);
file_manager_t *vf_get_current_fm_from_ring(vf_ring_t *r);
file_manager_t *vf_get_head_fm_from_ring(vf_ring_t *r);

BOOL   vf_init(file_manager_t * f, const char *file_name);
void   vf_term(file_manager_t * f);
void   vf_stat(file_manager_t * f, vf_stat_t * s);
char   vf_get_char(file_manager_t * f, char *result, off_t offset);
size_t vf_get_buf(file_manager_t * f, char *dest, off_t offset, size_t len);
size_t vf_insert_before(file_manager_t * f, char *buf, off_t offset, size_t len);
size_t vf_insert_after(file_manager_t * f, char *buf, off_t offset, size_t len);
size_t vf_replace(file_manager_t * f, char *buf, off_t offset, size_t len);
size_t vf_delete(file_manager_t * f, off_t offset, size_t len);
int    vf_undo(file_manager_t * f, int count, off_t * undo_addr);
int    vf_redo(file_manager_t * f, int count, off_t * redo_addr);
BOOL   vf_need_save(file_manager_t * f);
char *vf_get_fname(file_manager_t * f);
char *vf_get_fname_file(file_manager_t * f);
BOOL   vf_create_file(file_manager_t * f, const char *file_name);
off_t  vf_save(file_manager_t * f, int *complete);

#endif /* __VIRT_FILE_H */

