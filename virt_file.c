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
#include <sys/stat.h>

#include "virt_file.h"
#include "vf_backend.h"


/****************
  MACROS/DEFINES
 ***************/
#define BUF_TOGGLE(x) (x ^ 1)
#define MAX_SAVE_SHIFT (1024 * 1024) /* two megs (change save so that we can grow this dynamically if we need more? */


/****************
    FUNCTIONS
 ***************/
/*---------------------------

  ---------------------------*/
BOOL vf_init(file_manager_t * f, const char *file_name)
{
  struct stat stat_buf;

  /* If given a file name fill in some info.
     If not the user must open the stream and set the size.
     Filename is still required for saving at this point.
    (which reminds me, need to impliment saving to a new file name) */
  if (NULL != file_name)
  {
    snprintf(f->fname, MAX_PATH_LEN - 1, "%s", file_name);
    f->fname[MAX_PATH_LEN] = 0;

    f->fm.fp = fopen(f->fname, "r");
    if(NULL == f->fm.fp)
    {
      vf_term(f);
      return FALSE;
    }

    if (stat(f->fname, &stat_buf))
    {
      vf_term(f);
      return FALSE;
    }
    else
    {
      f->fm.size = stat_buf.st_size;
    }
  }

  f->fm.parent = NULL;
  f->fm.first_child = NULL;
  f->fm.next = NULL;
  f->fm.prev = NULL;
  f->fm.buf = NULL;
  f->fm.start = 0;
  f->fm.buf_type = TYPE_FILE;
  f->fm.active = TRUE;
  f->ul.last = NULL;
  f->ul.vb_list = NULL;
  f->ul.applied = FALSE;
  f->ul.saved = FALSE;

  return TRUE;
}


/*---------------------------

  ---------------------------*/
void vf_term(file_manager_t * f)
{
  if (NULL != f->fm.fp)
  {
    fclose(f->fm.fp);
    f->fm.fp = NULL;
  }
}


void vf_stat(file_manager_t * f, vf_stat_t * s)
{
  s->file_size = f->fm.size;
}
/*---------------------------

  ---------------------------*/
/* This may will take forever for large files.
   Also there is not nearly enough error
   checking! Also the total insert changes is
   limited to MAX_SAVE_SHIFT. Consider increasing
   this size dynamically when needed? */
off_t vf_save(file_manager_t * f, char *name, int *complete)
{
  static char move_buf[2][MAX_SAVE_SHIFT];  /* 2 x 2 megs from heap */
  int buf_select = 0;
  char *tmp_buf;
  off_t write_offset = 0;
  off_t shift = 0;
  vbuf_t *tmp = f->fm.first_child;
  size_t result = 0;
  int seek_result = 0;

  *complete = 0;

  prune(&f->ul);

  fclose(f->fm.fp);
  f->fm.fp = fopen(f->fname, "r+");

  while(NULL != tmp)
  {
    if(FALSE == tmp->active || 0 == tmp->size)
    {
      tmp = tmp->next;
      continue;
    }

    if(0 != shift)
    {
      if(shift > 0)             /* data has been inserted */
      {
        while(tmp->start - write_offset > shift)
        {
          seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
          fread(move_buf[buf_select], 1, shift, f->fm.fp);
          buf_select = BUF_TOGGLE(buf_select);
          seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
          result = fwrite(move_buf[buf_select], 1, shift, f->fm.fp);
          write_offset += shift;
          compute_percent_complete(write_offset, f->fm.size, complete);
        }

        seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
        fread(move_buf[buf_select], 1, tmp->start - write_offset, f->fm.fp);
        buf_select = BUF_TOGGLE(buf_select);
        seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
        result =
          fwrite(move_buf[buf_select], 1, tmp->start - write_offset, f->fm.fp);
        memcpy(move_buf[buf_select],
               move_buf[buf_select] + tmp->start - write_offset,
               shift - (tmp->start - write_offset));
        memcpy(move_buf[buf_select] + shift - (tmp->start - write_offset),
               move_buf[BUF_TOGGLE(buf_select)], tmp->start - write_offset);
        write_offset += shift;
        compute_percent_complete(write_offset, f->fm.size, complete);
      }
      else                      /* data has been deleted */
      {
        while(tmp->start - write_offset > MAX_SAVE_SHIFT)
        {
          seek_result = fseek(f->fm.fp, write_offset - shift, SEEK_SET);
          fread(move_buf[buf_select], 1, MAX_SAVE_SHIFT, f->fm.fp);
          seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
          result = fwrite(move_buf[buf_select], 1, MAX_SAVE_SHIFT, f->fm.fp);
          write_offset += MAX_SAVE_SHIFT;
          compute_percent_complete(write_offset, f->fm.size, complete);
        }

        seek_result = fseek(f->fm.fp, write_offset - shift, SEEK_SET);
        fread(move_buf[buf_select], 1, tmp->start - write_offset, f->fm.fp);
        seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
        result =
          fwrite(move_buf[buf_select], 1, tmp->start - write_offset, f->fm.fp);
        write_offset += tmp->start - write_offset;
        compute_percent_complete(write_offset, f->fm.size, complete);
      }
    }

    write_offset = tmp->start;
    compute_percent_complete(write_offset, f->fm.size, complete);

    switch (tmp->buf_type)
    {
      case TYPE_INSERT:
        tmp_buf = (char *)malloc(tmp->size);
        if(shift < 0)           /* data has been deleted */
        {
          if(tmp->size + shift > 0)
          {
            seek_result = fseek(f->fm.fp, write_offset - shift, SEEK_SET);
            fread(move_buf[buf_select], 1, tmp->size + shift, f->fm.fp);
          }
        }
        else                    /* data has been inserted */
        {
          seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
          fread(move_buf[buf_select] + shift, 1, tmp->size, f->fm.fp);
        }
        buf_select = BUF_TOGGLE(buf_select);
        vf_get_buf(f, tmp_buf, write_offset, tmp->size);
        seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
        result = fwrite(tmp_buf, 1, tmp->size, f->fm.fp);
        free(tmp_buf);
        shift += tmp->size;     /* if shift > MAX_SAVE_SHIFT we have a problem =( */
        write_offset += tmp->size;
        compute_percent_complete(write_offset, f->fm.size, complete);
        break;
      case TYPE_REPLACE:
        tmp_buf = (char *)malloc(tmp->size);
        if(shift != 0)
        {
          if(shift - tmp->size > 0)
          {
            memcpy(move_buf[buf_select], move_buf[buf_select] + tmp->size,
                   shift - tmp->size);
            seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
            fread(move_buf[buf_select] + shift - tmp->size, 1, tmp->size,
                  f->fm.fp);
          }
          else
          {
            seek_result =
              fseek(f->fm.fp, write_offset + tmp->size - shift, SEEK_SET);
            fread(move_buf[buf_select], 1, shift, f->fm.fp);
          }
          buf_select = BUF_TOGGLE(buf_select);
        }
        vf_get_buf(f, tmp_buf, write_offset, tmp->size);
        seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
        result = fwrite(tmp_buf, 1, tmp->size, f->fm.fp);
        free(tmp_buf);
        write_offset += tmp->size;
        compute_percent_complete(write_offset, f->fm.size, complete);
        break;
      case TYPE_DELETE:
        if(shift - tmp->size > 0)
          memcpy(move_buf[buf_select], move_buf[buf_select] + tmp->size,
                 shift - tmp->size);
        shift -= tmp->size;     /* if shift < -MAX_SAVE_SHIFT we have a problem =( */
        break;
      default:
        break;
    }

    tmp = tmp->next;
  }

  if(shift > 0)                 /* data has been inserted */
  {
    /* optimize this part to read larger chunks than 'shift'? */
    while(f->fm.size - write_offset > shift)
    {
      seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
      fread(move_buf[buf_select], 1, shift, f->fm.fp);
      buf_select = BUF_TOGGLE(buf_select);
      seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
      result = fwrite(move_buf[buf_select], 1, shift, f->fm.fp);
      write_offset += shift;
      compute_percent_complete(write_offset, f->fm.size, complete);
    }

    buf_select = BUF_TOGGLE(buf_select);
    seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
    result =
      fwrite(move_buf[buf_select], 1, f->fm.size - write_offset, f->fm.fp);
    write_offset += f->fm.size - write_offset;
    compute_percent_complete(write_offset, f->fm.size, complete);
  }
  else if(shift < 0)            /* data has been deleted */
  {
    while(f->fm.size - write_offset > MAX_SAVE_SHIFT)
    {
      seek_result = fseek(f->fm.fp, write_offset - shift, SEEK_SET);
      fread(move_buf[buf_select], 1, MAX_SAVE_SHIFT, f->fm.fp);
      seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
      result = fwrite(move_buf[buf_select], 1, MAX_SAVE_SHIFT, f->fm.fp);
      write_offset += MAX_SAVE_SHIFT;
      compute_percent_complete(write_offset, f->fm.size, complete);
    }

    seek_result = fseek(f->fm.fp, write_offset - shift, SEEK_SET);
    fread(move_buf[buf_select], 1, f->fm.size - write_offset, f->fm.fp);
    seek_result = fseek(f->fm.fp, write_offset, SEEK_SET);
    result =
      fwrite(move_buf[buf_select], 1, f->fm.size - write_offset, f->fm.fp);
    write_offset += f->fm.size - write_offset;
    compute_percent_complete(write_offset, f->fm.size, complete);
    ftruncate(fileno(f->fm.fp), write_offset);
  }

  fclose(f->fm.fp);
  f->fm.fp = fopen(f->fname, "r");

  cleanup(f);

  return write_offset;
}


/*---------------------------

  ---------------------------*/
BOOL vf_need_save(file_manager_t * f)
{
  vbuf_undo_list_t *tmp = f->ul.last;

  while(NULL != tmp)
  {
    if(FALSE == tmp->saved && TRUE == tmp->applied)
      return TRUE;
    tmp = tmp->last;
  }

  return FALSE;
}


/*---------------------------

  ---------------------------*/
/* returns number of changes undone */
/* undo_addr is set to the start address of the last change undone */
int vf_undo(file_manager_t * f, int count, off_t * undo_addr)
{
  vbuf_undo_list_t *tmp_undo_list = f->ul.last;
  vbuf_list_t *tmp_list = NULL;
  vbuf_t *tmp = NULL;
  int undo_count;

  /* look for the first change still applied */
  while(NULL != tmp_undo_list)
  {
    if(TRUE == tmp_undo_list->applied)
      break;
    tmp_undo_list = tmp_undo_list->last;
  }

  /* now start to undo */
  for(undo_count = 0; undo_count < count; undo_count++)
  {
    if(NULL == tmp_undo_list)
      return undo_count;

    tmp_list = tmp_undo_list->vb_list;

    while(NULL != tmp_list)
    {
      tmp = tmp_list->vb;

      switch (tmp->buf_type)
      {
        case TYPE_INSERT:
          mod_parent_size(tmp->parent, tmp->size, FALSE);
          mod_start_offset(tmp->next, tmp->size, FALSE);
          break;
        case TYPE_DELETE:
          mod_parent_size(tmp->parent, tmp->size, TRUE);
          mod_start_offset(tmp->next, tmp->size, TRUE);
          break;
        case TYPE_REPLACE:     /* no break */
        default:
          break;
      }

      tmp->active = FALSE;
      if(NULL != undo_addr)
        *undo_addr = tmp->start;

      tmp_list = tmp_list->next;
    }

    tmp_undo_list->applied = FALSE;

    tmp_undo_list = tmp_undo_list->last;
  }

  return undo_count;
}


/*---------------------------

  ---------------------------*/
/* returns number of changes redone */
/* redo_addr is set to the start address of the last change redone */
int vf_redo(file_manager_t * f, int count, off_t * redo_addr)
{
  vbuf_undo_list_t *tmp_undo_list = f->ul.last;
  vbuf_list_t *tmp_list = NULL;
  vbuf_t *tmp = NULL;
  int redo_count;

  if(NULL == tmp_undo_list)
    return 0;

  /* no changes have been undone */
  if(TRUE == tmp_undo_list->applied)
    return 0;

  /* look for the last change undone */
  while(NULL != tmp_undo_list)
  {
    if(NULL == tmp_undo_list->last)
      break;
    if(tmp_undo_list->last->applied)
      break;
    tmp_undo_list = tmp_undo_list->last;
  }

  /* now start to redo */
  for(redo_count = 0; redo_count < count; redo_count++)
  {
    if(NULL == tmp_undo_list)
      return redo_count;

    tmp_list = tmp_undo_list->vb_list;

    while(NULL != tmp_list)
    {
      tmp = tmp_list->vb;

      switch (tmp->buf_type)
      {
        case TYPE_INSERT:
          mod_parent_size(tmp->parent, tmp->size, TRUE);
          mod_start_offset(tmp->next, tmp->size, TRUE);
          break;
        case TYPE_DELETE:
          mod_parent_size(tmp->parent, tmp->size, FALSE);
          mod_start_offset(tmp->next, tmp->size, FALSE);
          break;
        case TYPE_REPLACE:     /* no break */
        default:
          break;
      }

      tmp->active = TRUE;
      if(NULL != redo_addr)
        *redo_addr = tmp->start;

      tmp_list = tmp_list->next;
    }

    tmp_undo_list->applied = TRUE;

    tmp_undo_list = f->ul.last;

    /* no changes have been undone */
    if(TRUE == tmp_undo_list->applied)
      return redo_count + 1;

    /* look for the last change undone */
    while(NULL != tmp_undo_list)
    {
      if(NULL == tmp_undo_list->last)
        break;
      if(tmp_undo_list->last->applied)
        break;
      tmp_undo_list = tmp_undo_list->last;
    }
  }

  return redo_count;
}


/*---------------------------

  ---------------------------*/
size_t vf_insert_before(file_manager_t * f, char *buf, off_t offset, size_t len)
{
  prune(&f->ul);
  return _insert_before(&f->fm, buf, offset, len, &f->ul.last);
}


/*---------------------------

  ---------------------------*/
size_t vf_insert_after(file_manager_t * f, char *buf, off_t offset, size_t len)
{
  prune(&f->ul);
  return _insert_before(&f->fm, buf, offset + 1, len, &f->ul.last);
}


/*---------------------------

  ---------------------------*/
size_t vf_replace(file_manager_t * f, char *buf, off_t offset, size_t len)
{
  size_t rep_size = 0;
  vbuf_list_t *vb_list = NULL;
  vbuf_undo_list_t *new_list;

  prune(&f->ul);

  rep_size = _replace(&f->fm, buf, offset, len, &vb_list);

  if(NULL != vb_list)
  {
    new_list = (vbuf_undo_list_t *) malloc(sizeof(vbuf_undo_list_t));
    new_list->applied = TRUE;
    new_list->saved = FALSE;
    new_list->last = f->ul.last;
    f->ul.last = new_list;
    new_list->vb_list = vb_list;
  }

  return rep_size;
}


/*---------------------------

  ---------------------------*/
size_t vf_delete(file_manager_t * f, off_t offset, size_t len)
{
  size_t del_size = 0;
  vbuf_list_t *vb_list = NULL;
  vbuf_undo_list_t *new_list;

  prune(&f->ul);

  del_size = _delete(&f->fm, offset, len, &vb_list);

  if(NULL != vb_list)
  {
    new_list = (vbuf_undo_list_t *) malloc(sizeof(vbuf_undo_list_t));
    new_list->applied = TRUE;
    new_list->saved = FALSE;
    new_list->last = f->ul.last;
    f->ul.last = new_list;
    new_list->vb_list = vb_list;
  }

  return del_size;
}


/*---------------------------

  ---------------------------*/
char vf_get_char(file_manager_t * f, char *result, off_t offset)
{
  return _get_char(&f->fm, result, offset);
}


/*---------------------------

  ---------------------------*/
size_t vf_get_buf(file_manager_t * f, char *dest, off_t offset, size_t len)
{
  return _get_buf(&f->fm, dest, offset, len);
}

