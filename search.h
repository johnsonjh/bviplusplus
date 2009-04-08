#include <regex.h>
#include "virt_file.h"

#ifndef __SEARCH_H__
#define __SEARCH_H__

#define MAX_SEARCH_LEN 256
#define MAX_SEARCH_MATCHES 32

typedef struct search_item_s
{
  char pattern[MAX_SEARCH_LEN];
  regex_t compiled;
  BOOL highlight;
  int color;
} search_item_t;

void buf_search(char *buf, int buf_size, int start, int *match_start, int *match_end);

#endif /* __SEARCH_H__ */
