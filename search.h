/*************************************************************
 *
 * File:        search.h
 * Author:      David Kelley
 * Description: Defines, structures, and function prototypes
 *              related to searching
 *
 *************************************************************/

#include <regex.h>
#include "virt_file.h"

#ifndef __SEARCH_H__
#define __SEARCH_H__

#define MAX_SEARCHES 8
#define MAX_SEARCH_LEN 256
#define MAX_SEARCH_MATCHES 32

typedef enum
{
  SEARCH_HEX,
  SEARCH_ASCII
} search_window_t;

typedef struct search_item_s
{
  char pattern[MAX_SEARCH_LEN];
  regex_t compiled;
  BOOL used;
  BOOL highlight;
  int color;
  search_window_t search_window;
} search_item_t;

typedef struct search_aid_s
{
  char *buf;
  int buf_size;
  off_t buf_start_addr;
  off_t display_addr;
  off_t hl_start;
  off_t hl_end;
  int remainder;
} search_aid_t;

extern search_item_t search_item[];
extern int current_search;

void buf_search(search_aid_t *search_aid);
void fill_search_buf(off_t addr, int display_size, search_aid_t *search_aid);
void free_search_buf(search_aid_t *search_aid);

#endif /* __SEARCH_H__ */
