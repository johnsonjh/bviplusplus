#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include "search.h"
#include "display.h"
#include "app_state.h"
#include "user_prefs.h"

search_item_t search_item[MAX_SEARCHES];
int current_search = 0;
regmatch_t matches[MAX_SEARCH_MATCHES];


void buf_search(search_aid_t *search_aid)
{
  int next_search, error, start_remainder, end_remainder;

  if (search_aid == NULL)
    return;

  if (search_item[current_search].used == FALSE)
  {
    search_aid->hl_start = -1;
    search_aid->hl_end   = -1;
    return;
  }

  do
  {
    if (search_aid->hl_end == -1) /* could wrap! , might not want this */
      next_search = 0;
    else
      next_search = search_aid->hl_start - search_aid->buf_start_addr + 1;

    if (search_item[current_search].search_window == SEARCH_HEX)
      next_search *= 2;

    error = regexec(&search_item[current_search].compiled,
                    search_aid->buf + next_search,
                    MAX_SEARCH_MATCHES,
                    matches,
                    REG_NOTBOL|REG_NOTEOL);
    if (error == REG_NOMATCH)
    {
      search_aid->hl_start = -1;
      search_aid->hl_end   = -1;
      return;
    }

    if (search_item[current_search].search_window == SEARCH_HEX)
    {
      search_aid->hl_start = matches[0].rm_so + next_search;
      start_remainder = search_aid->hl_start % 2;
      search_aid->hl_start /= 2;
      search_aid->hl_start += search_aid->buf_start_addr;
      search_aid->hl_end   =  matches[0].rm_eo + next_search;
      end_remainder = search_aid->hl_end % 2;
      search_aid->hl_end   /= 2;
      search_aid->hl_end   += search_aid->buf_start_addr;
    }
    if (search_item[current_search].search_window == SEARCH_ASCII)
    {
      search_aid->hl_start = search_aid->buf_start_addr + matches[0].rm_so + next_search;
      search_aid->hl_end   = search_aid->buf_start_addr + matches[0].rm_eo + next_search;
      start_remainder = 0;
      end_remainder = 0;
    }
  } while ((start_remainder != 0 || end_remainder != 0) && error != REG_NOMATCH);
}

void set_search_term(char *pattern)
{
  int error, len, flags = 0;
  char *error_text;

  if (user_prefs[IGNORECASE].value)
    flags |= REG_ICASE;
  if (user_prefs[IGNORECASE].value)
    flags |= REG_EXTENDED;

  regfree(&search_item[current_search].compiled);

  error = regcomp(&search_item[current_search].compiled,
                  pattern,
                  flags);
  if (error)
  {
    search_item[current_search].used = FALSE;

    len = regerror(error, &search_item[current_search].compiled, NULL, 0);
    error_text = (char *)malloc(len);
    regerror(error, &search_item[current_search].compiled, error_text, len);

    msg_box("Invalid search: \"%s\" %s", pattern, error_text);

    free(error_text);

  }
  else
  {
    search_item[current_search].used = TRUE;
    strncpy(search_item[current_search].pattern, pattern, MAX_SEARCH_LEN);
  }
}

void search_init(void)
{
  int i = 0;

  for (i=0; i<MAX_SEARCHES; i++)
    search_item[i].used = FALSE;
}

void search_cleanup(void)
{
  int i = 0;

  for (i=0; i<MAX_SEARCHES; i++)
  {
    if (search_item[i].used == TRUE)
    {
      regfree(&search_item[i].compiled);
      search_item[i].used = FALSE;
    }
  }
}


void fill_search_buf(off_t addr, int display_size, search_aid_t *search_aid)
{
  off_t a;
  char *tmp;
  int i, size;

  if (search_aid == NULL)
    return;

  search_aid->hl_start = -1;
  search_aid->hl_end = -1;
  search_aid->remainder = 0;

  search_aid->display_addr = addr;

  a = addr - display_size;
  if (address_invalid(a))
    a = 0;

  search_aid->buf_start_addr = a;

  a = addr + (2*display_size);
  if (address_invalid(a))
    a = display_info.file_size;

  search_aid->buf_size = a - search_aid->buf_start_addr;

  if (search_item[current_search].search_window == SEARCH_ASCII)
  {
    search_aid->buf = (char *)malloc(search_aid->buf_size + 1);
    vf_get_buf(current_file,
               search_aid->buf,
               search_aid->buf_start_addr,
               search_aid->buf_size);
    search_aid->buf[search_aid->buf_size] = 0;
  }
  else
  {
    tmp = (char *)malloc(search_aid->buf_size);
    vf_get_buf(current_file,
               tmp,
               search_aid->buf_start_addr,
               search_aid->buf_size);
    size = search_aid->buf_size;
    search_aid->buf_size *= 2;
    search_aid->buf = (char *)malloc(search_aid->buf_size + 1);
    search_aid->buf[search_aid->buf_size] = 0;
    for (i=0; i<size; i++)
    {
      search_aid->buf[(2*i)  ] = HEX((tmp[i] >> 4) & 0xF);
      search_aid->buf[(2*i)+1] = HEX((tmp[i] >> 0) & 0xF);
    }
    free(tmp);
  }

  buf_search(search_aid);
  while (search_aid->hl_start != -1)
  {
    if (search_aid->hl_end >= addr)
      break;

    buf_search(search_aid);
  }

}


void free_search_buf(search_aid_t *search_aid)
{
  if (search_aid == NULL)
    return;

  if (search_aid->buf == NULL)
    return;

  free(search_aid->buf);
}

