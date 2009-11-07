/*************************************************************************
 *
 * File:        vf_backend.h
 * Author:      David Kelley
 * Description: Defines, structures, and function prototypes
 *              related to the virtual file backend
 *
 * Copyright (C) 2009 David Kelley
 *
 * This file is part of bviplus.
 *
 * Bviplus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bviplus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bviplus.  If not, see <http://www.gnu.org/licenses/>.
 *
 ************************************************************************/

#ifndef __VIRT_FILE_H__
#define __VIRT_FILE_H__

/****************
    INCLUDES
 ***************/
#include "virt_file.h"


/****************
   PROTOTYPES
 ***************/
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

