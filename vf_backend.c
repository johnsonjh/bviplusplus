/******************************************************
 *
 * Project: Virtual File
 * Author:  David Kelley
 * Date:    29 Sept, 2008
 *
 *****************************************************/

/****************
    INCLUDES
 ***************/
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "virt_file.h"
#include "vf_backend.h"


/****************
   PROTOTYPES
 ***************/
static off_t get_internal_shift(vbuf_t * vb);
void mod_start_offset(vbuf_t * vb, off_t shift, BOOL increase);
void mod_parent_size(vbuf_t * vb, off_t shift, BOOL increase);
void prune(vbuf_undo_list_t * undo_list);
static void _cleanup_vbuf(vbuf_t * vb);
static void _cleanup_undo_vb(vbuf_list_t * vb_list);
static void _cleanup_undo(vbuf_undo_list_t * undo_list);
void cleanup(file_manager_t * f);
inline void compute_percent_complete(off_t offset, off_t size, int *complete);
static void insert_new_vbuf(vbuf_t **new, vbuf_t *current, vbuf_t *parent, off_t offset,
                     size_t len, buf_type_e buf_type, char *src_buf);


/****************
    FUNCTIONS
 ***************/
/*---------------------------

  ---------------------------*/
static off_t get_internal_shift(vbuf_t * vb)
{
  off_t shift = 0;
  vbuf_t *tmp = vb->first_child;

  while(NULL != tmp)
  {
    switch (tmp->buf_type)
    {
      case TYPE_INSERT:
        shift -= tmp->size;
        break;
      case TYPE_REPLACE:
        shift += get_internal_shift(tmp);
        break;
      case TYPE_DELETE:
        shift += tmp->size;
        break;
      default:
        break;
    }
    tmp = tmp->next;
  }
  return shift;
}


/*---------------------------

  ---------------------------*/
void mod_start_offset(vbuf_t * vb, off_t shift, BOOL increase)
{
  vbuf_t *tmp = vb;

  while(NULL != tmp)
  {
    if(TRUE == increase)
      tmp->start += shift;
    else
      tmp->start -= shift;

    mod_start_offset(tmp->first_child, shift, increase);

    tmp = tmp->next;
  }
}


/*---------------------------

  ---------------------------*/
void mod_parent_size(vbuf_t * vb, off_t shift, BOOL increase)
{
  vbuf_t *tmp = vb;

  while(NULL != tmp)
  {
    if(TRUE == increase)
      tmp->size += shift;
    else
      tmp->size -= shift;

    mod_start_offset(tmp->next, shift, increase);
    tmp = tmp->parent;
  }
}


/*---------------------------

  ---------------------------*/
void prune(vbuf_undo_list_t * undo_list)
{
  vbuf_undo_list_t *tmp_undo_list = undo_list->last;
  vbuf_list_t *tmp_list = NULL;
  vbuf_t *tmp = NULL;

  if(NULL == tmp_undo_list)
    return;

  while(NULL != tmp_undo_list)
  {
    if(TRUE == tmp_undo_list->applied)
      break;

    tmp_list = tmp_undo_list->vb_list;

    while(NULL != tmp_list)
    {
      tmp = tmp_list->vb;

      /* no children left and we're not active either, delete self */
      if(NULL == tmp->first_child && FALSE == tmp->active)
      {
        if(NULL == tmp->next)
          tmp->parent->first_child->prev = tmp->prev;
        else
          tmp->next->prev = tmp->prev;

        if(tmp->parent->first_child == tmp)
          tmp->parent->first_child = tmp->next;

        if(NULL != tmp->prev)
        {
          if(NULL != tmp->prev->next)
            tmp->prev->next = tmp->next;
        }

        if(NULL != tmp->buf)
          free(tmp->buf);
        free(tmp);
      }

      tmp_list = tmp_list->next;
      free(tmp_undo_list->vb_list);
      tmp_undo_list->vb_list = tmp_list;

    }                           /* while (NULL != temp_list) */
    undo_list->last = tmp_undo_list->last;
    free(tmp_undo_list);
    tmp_undo_list = undo_list->last;

  }                             /* while (NULL != temp_undo_list) */

}


/*---------------------------

  ---------------------------*/
static void _cleanup_vbuf(vbuf_t * vb)
{
  if(NULL == vb)
    return;

  if(NULL != vb->first_child)
    _cleanup_vbuf(vb->first_child);

  if(NULL != vb->next)
    _cleanup_vbuf(vb->next);

  if(NULL != vb->buf)
  {
    free(vb->buf);
    vb->buf = NULL;            /* just in case */
  }
  free(vb);
}


/*---------------------------

  ---------------------------*/
static void _cleanup_undo_vb(vbuf_list_t * vb_list)
{
  if(NULL != vb_list->next)
    _cleanup_undo_vb(vb_list->next);

  free(vb_list);
}


/*---------------------------

  ---------------------------*/
static void _cleanup_undo(vbuf_undo_list_t * undo_list)
{
  if(NULL == undo_list)
    return;

  if(NULL != undo_list->last)
    _cleanup_undo(undo_list->last);

  _cleanup_undo_vb(undo_list->vb_list);

  free(undo_list);
}


/*---------------------------

  ---------------------------*/
void cleanup(file_manager_t * f)
{
  if (NULL == f)
    return;

  _cleanup_vbuf(f->fm.first_child);
  f->fm.first_child = NULL;
  _cleanup_undo(f->ul.last);
  f->ul.last = NULL;
}


/*---------------------------

  ---------------------------*/
inline void compute_percent_complete(off_t offset, off_t size, int *complete)
{
  if (size == 0)
  {
    return;
  }
  else if (size > 10000)
  {
    size = size/100;
    offset = offset/100;
  }
  *complete = (offset*100)/size;
}


/*---------------------------

  ---------------------------*/
/* make this not void, and handle mem alloc errors? */
static void insert_new_vbuf(vbuf_t **new, vbuf_t *current, vbuf_t *parent, off_t offset,
                     size_t len, buf_type_e buf_type, char *src_buf)
{
  *new = (vbuf_t *) malloc(sizeof(vbuf_t));
  (*new)->first_child = NULL;
  (*new)->start = offset;
  (*new)->buf_type = buf_type;
  (*new)->fp = NULL;
  (*new)->active = TRUE;
  (*new)->next = current;

  if (NULL != current) /* inserting into the middle of a list */
  {
    (*new)->prev = current->prev;
    if(NULL != (*new)->prev->next)
      (*new)->prev->next = *new;
    current->prev = *new;
    if(parent->first_child == current)
      parent->first_child = *new;
  }
  else /* appending to the end of a list, including the case of first member */
  {
    if(NULL == parent->first_child)
    {
      parent->first_child = *new;
      (*new)->prev = *new;
    }
    else
    {
      (*new)->prev = parent->first_child->prev;
      parent->first_child->prev->next = *new;
      parent->first_child->prev = *new;
    }
  }

  (*new)->parent = parent;

  (*new)->size = len;
  if (NULL != src_buf)
  {
    (*new)->buf = (char *)malloc((*new)->size);
    memcpy((*new)->buf, src_buf, (*new)->size);
  }
  else
  {
    (*new)->buf = NULL;
  }
}


/*---------------------------

  ---------------------------*/
size_t _insert_before(vbuf_t * vb, char *buf, off_t offset, size_t len,
                      vbuf_undo_list_t ** undo_list)
{
  vbuf_t *tmp = NULL,
    *new = NULL;
  vbuf_undo_list_t *new_list = NULL;

  /* make sure we're still in the buf */
  if(offset > vb->start + vb->size)
    return 0;

  tmp = vb->first_child;

  while(NULL != tmp)
  {
    switch (tmp->buf_type)
    {
      case TYPE_INSERT:
      case TYPE_REPLACE:
        if(offset < tmp->start)
        {
          insert_new_vbuf(&new, tmp, vb, offset, len, TYPE_INSERT, buf);
          mod_parent_size(new->parent, new->size, TRUE);
          mod_start_offset(new->next, new->size, TRUE);
          new_list = (vbuf_undo_list_t *) malloc(sizeof(vbuf_undo_list_t));
          new_list->applied = TRUE;
          new_list->saved = FALSE;
          new_list->last = *undo_list;
          *undo_list = new_list;
          new_list->vb_list = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
          new_list->vb_list->next = NULL;
          new_list->vb_list->vb = new;
          return len;
        }
        else if(offset < tmp->start + tmp->size)
        {
          return _insert_before(tmp, buf, offset, len, undo_list);
        }
        break;
      case TYPE_DELETE:
        if(offset < tmp->start)
        {
          insert_new_vbuf(&new, tmp, vb, offset, len, TYPE_INSERT, buf);
          mod_parent_size(new->parent, new->size, TRUE);
          mod_start_offset(new->next, new->size, TRUE);
          new_list = (vbuf_undo_list_t *) malloc(sizeof(vbuf_undo_list_t));
          new_list->applied = TRUE;
          new_list->saved = FALSE;
          new_list->last = *undo_list;
          *undo_list = new_list;
          new_list->vb_list = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
          new_list->vb_list->next = NULL;
          new_list->vb_list->vb = new;
          return len;
        }
        break;
      default:
        break;

    }
    tmp = tmp->next;
  }

  insert_new_vbuf(&new, NULL, vb, offset, len, TYPE_INSERT, buf);
  mod_parent_size(new->parent, new->size, TRUE);
  mod_start_offset(new->next, new->size, TRUE);

  new_list = (vbuf_undo_list_t *) malloc(sizeof(vbuf_undo_list_t));
  new_list->applied = TRUE;
  new_list->saved = FALSE;
  new_list->last = *undo_list;
  *undo_list = new_list;
  new_list->vb_list = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
  new_list->vb_list->next = NULL;
  new_list->vb_list->vb = new;

  return len;
}


/*---------------------------

  ---------------------------*/
size_t _replace(vbuf_t * vb, char *buf, off_t offset, size_t len,
                vbuf_list_t ** vb_list)
{
  vbuf_t *tmp = NULL,
    *new = NULL;
  vbuf_list_t *tmp_vb_list = NULL;
  off_t tmp_offset = 0;
  size_t tmp_len = 0,
    result = 0;

  /* make sure we're still in the buf */
  if(offset + len > vb->start + vb->size)
    return 0;

  tmp = vb->first_child;
  tmp_offset = offset;
  tmp_len = len;

  while(NULL != tmp && 0 != tmp_len)
  {
    switch (tmp->buf_type)
    {
      case TYPE_REPLACE:       /* no break */
      case TYPE_INSERT:
        if(tmp_offset < tmp->start)
        {
          if(tmp_offset + tmp_len < tmp->start)
            insert_new_vbuf(&new, tmp, vb, tmp_offset, tmp_len,
                            TYPE_REPLACE, buf + tmp_offset - offset);
          else
            insert_new_vbuf(&new, tmp, vb, tmp_offset, tmp->start - tmp_offset,
                            TYPE_REPLACE, buf + tmp_offset - offset);

          tmp_len -= new->size;
          tmp_offset += new->size;
          if(NULL == *vb_list)
          {
            *vb_list = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
            tmp_vb_list = *vb_list;
          }
          else
          {
            tmp_vb_list = *vb_list;
            while(NULL != tmp_vb_list->next)
              tmp_vb_list = tmp_vb_list->next;
            tmp_vb_list->next = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
            tmp_vb_list = tmp_vb_list->next;
          }
          tmp_vb_list->vb = new;
          tmp_vb_list->next = NULL;
        }
        else if(tmp_offset < tmp->start + tmp->size)
        {
          if(tmp_offset + tmp_len < tmp->start + tmp->size)
          {
            if(NULL == tmp_vb_list)
              result =
                _replace(tmp, buf + tmp_offset - offset, tmp_offset, tmp_len,
                         vb_list);
            else
              result =
                _replace(tmp, buf + tmp_offset - offset, tmp_offset, tmp_len,
                         &tmp_vb_list->next);
          }
          else
          {
            if(NULL == tmp_vb_list)
              result =
                _replace(tmp, buf + tmp_offset - offset, tmp_offset,
                         tmp->start + tmp->size - tmp_offset, vb_list);
            else
              result =
                _replace(tmp, buf + tmp_offset - offset, tmp_offset,
                         tmp->start + tmp->size - tmp_offset,
                         &tmp_vb_list->next);
          }
          tmp_len -= result;
          tmp_offset += result;
        }
        break;
      case TYPE_DELETE:
        if(tmp_offset < tmp->start)
        {
          if(tmp_offset + tmp_len < tmp->start)
            insert_new_vbuf(&new, tmp, vb, tmp_offset, tmp_len,
                            TYPE_REPLACE, buf + tmp_offset - offset);
          else
            insert_new_vbuf(&new, tmp, vb, tmp_offset, tmp->start - tmp_offset,
                            TYPE_REPLACE, buf + tmp_offset - offset);

          tmp_len -= new->size;
          tmp_offset += new->size;
          if(NULL == *vb_list)
          {
            *vb_list = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
            tmp_vb_list = *vb_list;
          }
          else
          {
            tmp_vb_list = *vb_list;
            while(NULL != tmp_vb_list->next)
              tmp_vb_list = tmp_vb_list->next;
            tmp_vb_list->next = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
            tmp_vb_list = tmp_vb_list->next;
          }
          tmp_vb_list->vb = new;
          tmp_vb_list->next = NULL;
        }
        break;
      default:
        break;
    }
    tmp = tmp->next;
  }

  if(0 != tmp_len)
  {
    if(tmp_offset < vb->start + vb->size)
    {

      if(tmp_offset + tmp_len < vb->start + vb->size)
        insert_new_vbuf(&new, NULL, vb, tmp_offset, tmp_len,
                        TYPE_REPLACE, buf + tmp_offset - offset);
      else                      /* copy as much as we can, but basically we fail */
        insert_new_vbuf(&new, NULL, vb, tmp_offset, vb->start + vb->size - tmp_offset,
                        TYPE_REPLACE, buf + tmp_offset - offset);

      tmp_len -= new->size;
      if(NULL == *vb_list)
      {
        *vb_list = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
        tmp_vb_list = *vb_list;
      }
      else
      {
        tmp_vb_list = *vb_list;
        while(NULL != tmp_vb_list->next)
          tmp_vb_list = tmp_vb_list->next;
        tmp_vb_list->next = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
        tmp_vb_list = tmp_vb_list->next;
      }
      tmp_vb_list->vb = new;
      tmp_vb_list->next = NULL;
    }
  }

  return len - tmp_len;
}


/*---------------------------

  ---------------------------*/
size_t _delete(vbuf_t * vb, off_t offset, size_t len,
                 vbuf_list_t ** vb_list)
{
  vbuf_t *tmp = NULL,
    *new = NULL;
  vbuf_list_t *tmp_vb_list = NULL;
  off_t tmp_offset = 0;
  size_t tmp_len = 0,
    result = 0;

  /* make sure we're still in the buf */
  if(offset + len > vb->start + vb->size)
    return 0;

  tmp = vb->first_child;
  tmp_offset = offset;
  tmp_len = len;

  while(NULL != tmp && 0 != tmp_len)
  {
    switch (tmp->buf_type)
    {
      case TYPE_REPLACE:       /* no break */
      case TYPE_INSERT:
        if(tmp_offset < tmp->start)
        {
          if(tmp_offset + tmp_len < tmp->start)
            insert_new_vbuf(&new, tmp, vb, tmp_offset, tmp_len,
                            TYPE_DELETE, NULL);
          else
            insert_new_vbuf(&new, tmp, vb, tmp_offset, tmp->start - tmp_offset,
                            TYPE_DELETE, NULL);

          tmp_len -= new->size;
          mod_parent_size(new->parent, new->size, FALSE);
          mod_start_offset(new->next, new->size, FALSE);
          if(NULL == *vb_list)
          {
            *vb_list = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
            tmp_vb_list = *vb_list;
          }
          else
          {
            tmp_vb_list = *vb_list;
            while(NULL != tmp_vb_list->next)
              tmp_vb_list = tmp_vb_list->next;
            tmp_vb_list->next = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
            tmp_vb_list = tmp_vb_list->next;
          }
          tmp_vb_list->vb = new;
          tmp_vb_list->next = NULL;
        }
        else if(tmp_offset < tmp->start + tmp->size)
        {
          if(tmp_offset + tmp_len < tmp->start + tmp->size)
          {
            if(NULL == tmp_vb_list)
              result = _delete(tmp, tmp_offset, tmp_len, vb_list);
            else
              result = _delete(tmp, tmp_offset, tmp_len, &tmp_vb_list->next);
          }
          else
          {
            if(NULL == tmp_vb_list)
              result =
                _delete(tmp, tmp_offset, tmp->start + tmp->size - tmp_offset,
                          vb_list);
            else
              result =
                _delete(tmp, tmp_offset, tmp->start + tmp->size - tmp_offset,
                          &tmp_vb_list->next);
          }
          tmp_len -= result;
        }
        break;
      case TYPE_DELETE:
        if(tmp_offset < tmp->start)
        {
          if(tmp_offset + tmp_len < tmp->start)
            insert_new_vbuf(&new, tmp, vb, tmp_offset, tmp_len, TYPE_DELETE, NULL);
          else
            insert_new_vbuf(&new, tmp, vb, tmp_offset, tmp->start - tmp_offset,
                            TYPE_DELETE, NULL);

          tmp_len -= new->size;
          mod_parent_size(new->parent, new->size, FALSE);
          mod_start_offset(new->next, new->size, FALSE);
          if(NULL == *vb_list)
          {
            *vb_list = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
            tmp_vb_list = *vb_list;
          }
          else
          {
            tmp_vb_list = *vb_list;
            while(NULL != tmp_vb_list->next)
              tmp_vb_list = tmp_vb_list->next;
            tmp_vb_list->next = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
            tmp_vb_list = tmp_vb_list->next;
          }
          tmp_vb_list->vb = new;
          tmp_vb_list->next = NULL;
        }
        break;
      default:
        break;
    }
    tmp = tmp->next;
  }

  if(0 != tmp_len)
  {
    if(tmp_offset < vb->start + vb->size)
    {

      if(tmp_offset + tmp_len < vb->start + vb->size)
        insert_new_vbuf(&new, NULL, vb, tmp_offset, tmp_len, TYPE_DELETE, NULL);
      else                      /* copy as much as we can, but basically we fail */
        insert_new_vbuf(&new, NULL, vb, tmp_offset,
                        vb->start + vb->size - tmp_offset, TYPE_DELETE, NULL);

      tmp_len -= new->size;
      mod_parent_size(new->parent, new->size, FALSE);
      mod_start_offset(new->next, new->size, FALSE);
      if(NULL == *vb_list)
      {
        *vb_list = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
        tmp_vb_list = *vb_list;
      }
      else
      {
        tmp_vb_list = *vb_list;
        while(NULL != tmp_vb_list->next)
          tmp_vb_list = tmp_vb_list->next;
        tmp_vb_list->next = (vbuf_list_t *) malloc(sizeof(vbuf_list_t));
        tmp_vb_list = tmp_vb_list->next;
      }
      tmp_vb_list->vb = new;
      tmp_vb_list->next = NULL;

    }
  }

  return len - tmp_len;
}


/*---------------------------

  ---------------------------*/
/* must keep a running offset cause but[offset] is not right if there are inserts/dels */
char _get_char(vbuf_t * vb, char *result, off_t offset)
{
  off_t shift = 0;
  vbuf_t *tmp = vb->first_child;
  char value;

  /* Offset is not in this set! */
  if(offset >= vb->start + vb->size)
  {
    *result = 0;
    return 0;
  }

  /* Look for offset, is a sub vb or what? */
  while(NULL != tmp)
  {

    if(FALSE == tmp->active)
    {
      tmp = tmp->next;
      continue;
    }

    if(offset < tmp->start)
      break;

    switch (tmp->buf_type)
    {
      case TYPE_INSERT:
        if(offset < tmp->start + tmp->size)
          return _get_char(tmp, result, offset);

        shift -= tmp->size;
        break;
      case TYPE_REPLACE:
        if(offset < tmp->start + tmp->size)
          return _get_char(tmp, result, offset);

        shift += get_internal_shift(tmp);
        break;
      case TYPE_DELETE:
        shift += tmp->size;
        break;
      default:
        break;
    }

    tmp = tmp->next;
  }

  /* It is definitely in this set and we didn't find it in the sub vbs, so it must be this one */
  if(TYPE_FILE == vb->buf_type)
  {
    fseeko(vb->fp, offset + shift, SEEK_SET);
    fread(&value, 1, 1, vb->fp);
  }
  else
  {
    value = vb->buf[offset - vb->start + shift];
  }

  *result = 1;
  return value;
}


/*---------------------------

  ---------------------------*/
size_t _get_buf(vbuf_t * vb, char *dest, off_t offset, size_t len)
{
  vbuf_t *tmp = NULL;
  off_t tmp_offset = 0, shift = 0;
  size_t tmp_len = 0, read_len = 0, result = 0;

  /* make sure we're still in the buf */
  if(offset + len > vb->start + vb->size)
    return 0;

  tmp = vb->first_child;
  tmp_offset = offset;
  tmp_len = len;

  while(NULL != tmp && 0 != tmp_len)
  {
    if (tmp->active == 0)
    {
      tmp = tmp->next;
      continue;
    }

    if(tmp_offset < tmp->start)
    {
      if(tmp_offset + tmp_len < tmp->start)
        read_len = tmp_len;
      else
        read_len = tmp->start - tmp_offset;

      /* switch current type and cpy all data */
      switch(vb->buf_type)
      {
        case TYPE_FILE:
          fseeko(vb->fp, tmp_offset + shift, SEEK_SET);
          result = fread(dest + len - tmp_len, 1, read_len, vb->fp);
          break;
        case TYPE_INSERT: /* no break */
        case TYPE_REPLACE:
          memcpy(dest + len - tmp_len, vb->buf + tmp_offset + shift - vb->start, read_len);
          result = read_len;
          break;
        case TYPE_DELETE: /* no break -- this should not occur */
        default:
          return len - tmp_len; /* error */
      }

      tmp_len -= result;
      tmp_offset += result;
    }
    else if(tmp_offset < tmp->start + tmp->size)
    {
      switch (tmp->buf_type)
      {
        case TYPE_REPLACE:       /* no break */
        case TYPE_INSERT:

          if (tmp->buf_type == TYPE_INSERT)
            shift -= tmp->size;

          if(tmp_offset + tmp_len < tmp->start + tmp->size)
            read_len = tmp_len;
          else
            read_len = tmp->start + tmp->size - tmp_offset;

          result = _get_buf(tmp, dest + len - tmp_len, tmp_offset, read_len);

          tmp_len -= result;
          tmp_offset += result;
          break;
        case TYPE_DELETE:
          shift += tmp->size;
          break;
        default:
          break;
      }
      tmp = tmp->next;
    }
    else
    {
      if (tmp->buf_type == TYPE_INSERT)
        shift -= tmp->size;
      else if (tmp->buf_type == TYPE_DELETE)
        shift += tmp->size;

      tmp = tmp->next;
    }
  }

  if(0 != tmp_len)
  {
    if(tmp_offset < vb->start + vb->size)
    {

      if(tmp_offset + tmp_len < vb->start + vb->size)
        read_len = tmp_len;
      else
        read_len = vb->start + vb->size - tmp_offset;

      /* switch current type and cpy all data */
      switch(vb->buf_type)
      {
        case TYPE_FILE:
          fseeko(vb->fp, tmp_offset + shift, SEEK_SET);
          result = fread(dest + len - tmp_len, 1, read_len, vb->fp);
          break;
        case TYPE_INSERT: /* no break */
        case TYPE_REPLACE:
          memcpy(dest + len - tmp_len, vb->buf + tmp_offset + shift - vb->start, read_len);
          result = read_len;
          break;
        case TYPE_DELETE: /* no break -- this should not occur */
        default:
          return len - tmp_len; /* error */
      }

      tmp_len -= result;
      tmp_offset += result;
    }
  }

  return len - tmp_len;
}


