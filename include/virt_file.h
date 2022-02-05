/*
 * Defines/structures/prototypes related to the virtual file interface
 *
 * Copyright (c) 2008, 2009, 2010 David Kelley
 * Copyright (c) 2009 Steve Lewis
 * Copyright (c) 2016 The Lemon Man
 * Copyright (c) 2022 Jeffrey H. Johnson <trnsz@pobox.com>
 *
 * This file is part of bviplusplus.
 *
 * Bviplusplus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bviplusplus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bviplusplus.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VIRT_FILE_H
# define __VIRT_FILE_H

# include <stdio.h>
# include <sys/types.h>

# ifdef TRUE
#  undef TRUE
# endif /* ifdef TRUE */

# define TRUE 1

# ifdef FALSE
#  undef FALSE
# endif /* ifdef FALSE */

# define FALSE 0

# define MAX_PATH_LEN 255

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
  /* Move fp from vb up to here?
   * Integrate fname into vb?
   * Do we want to support inserting a file into a file?
   */
  vbuf_t *vb;
  vbuf_list_t *next; /* move across this list */
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
  void *private_data;
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

vf_ring_t *vf_create_fm_ring(void);
BOOL vf_destroy_fm_ring(vf_ring_t *r);
file_manager_t *vf_add_fm_to_ring(vf_ring_t *r);
BOOL vf_remove_fm_from_ring(vf_ring_t *r, file_manager_t *fm);
file_manager_t *vf_get_next_fm_from_ring(vf_ring_t *r);
file_manager_t *vf_get_last_fm_from_ring(vf_ring_t *r);
file_manager_t *vf_get_current_fm_from_ring(vf_ring_t *r);
BOOL vf_set_current_fm_from_ring(vf_ring_t *r, file_manager_t *fm);
file_manager_t *vf_get_head_fm_from_ring(vf_ring_t *r);

BOOL vf_init(file_manager_t *f, const char *file_name);
void vf_term(file_manager_t *f);
void vf_stat(file_manager_t *f, vf_stat_t *s);
char vf_get_char(file_manager_t *f, char *result, off_t offset);
size_t vf_get_buf(file_manager_t *f, char *dest, off_t offset, size_t len);
size_t vf_insert_before(file_manager_t *f, char *buf, off_t offset,
                        size_t len);
size_t vf_insert_after(file_manager_t *f, char *buf, off_t offset,
                       size_t len);
size_t vf_replace(file_manager_t *f, char *buf, off_t offset, size_t len);
size_t vf_delete(file_manager_t *f, off_t offset, size_t len);
int vf_undo(file_manager_t *f, int count, off_t *undo_addr);
int vf_redo(file_manager_t *f, int count, off_t *redo_addr);
BOOL vf_need_create(file_manager_t *f);
BOOL vf_need_save(file_manager_t *f);
char *vf_get_fname(file_manager_t *f);
char *vf_get_fname_file(file_manager_t *f);
BOOL vf_create_file(file_manager_t *f, const char *file_name);
BOOL vf_copy_file(file_manager_t *f, const char *file_name);
off_t vf_save(file_manager_t *f, int *complete);

#endif /* __VIRT_FILE_H */
