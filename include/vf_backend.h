/*
 * Defines/structures/prototypes related to the virtual file back-end
 *
 * Copyright (c) 2008, 2009, 2010 David Kelley
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

#ifndef __VIRT_FILE_H__
# define __VIRT_FILE_H__

# include "virt_file.h"

void cleanup(file_manager_t *f);
void compute_percent_complete(off_t offset, off_t size, int *complete);
void mod_start_offset(vbuf_t *vb, off_t shift, BOOL increase);
void mod_parent_size(vbuf_t *vb, off_t shift, BOOL increase);
void prune(vbuf_undo_list_t *undo_list);
size_t _insert_before(vbuf_t *vb, char *buf, off_t offset, size_t len,
                      vbuf_undo_list_t **undo_list);
size_t _replace(vbuf_t *vb, char *buf, off_t offset, size_t len,
                vbuf_list_t **vb_list);
size_t _delete(vbuf_t *vb, off_t offset, size_t len, vbuf_list_t **vb_list);
char _get_char(vbuf_t *vb, char *result, off_t offset);
size_t _get_buf(vbuf_t *vb, char *dest, off_t offset, size_t len);

#endif /* __VIRT_FILE_H__ */
