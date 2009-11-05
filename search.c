/*************************************************************
 *
 * File:        search.c
 * Author:      David Kelley
 * Description: Search implimentation
 *
 *************************************************************/

#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "search.h"
#include "display.h"
#include "app_state.h"
#include "user_prefs.h"
#include "key_handler.h" /* for is_hex(), consider moving this func */

search_item_t search_item[MAX_SEARCHES];
int current_search = 0;

void search_state_reset(search_state_t *search_state)
{
  int i;
  search_state->criteria_index = 0;
  for (i=0; i<search_item[current_search].compiled_pattern.criteria_count; i++)
    search_state->match_state[i] = UNFULFILLED;
}

int inline matches(unsigned char byte, match_criteria_t *criteria)
{
  int i;

  if (search_item[current_search].search_window == SEARCH_ASCII &&
      user_prefs[IGNORECASE].value != 0)
  {
    for (i=0; i<criteria->range_count; i++)
    {
      if (toupper(byte) == toupper(criteria->range[i]))
        return 1;
    }
  }
  else
  {
    for (i=0; i<criteria->range_count; i++)
    {
      if (byte == criteria->range[i])
        return 1;
    }
  }

  return 0;
}

search_result_t rollback(unsigned char byte, search_state_t *search_state)
{
  int i;
  compiled_pattern_t *cpat = &search_item[current_search].compiled_pattern;

  for (i=search_state->criteria_index-1; i>=0; i--)
  {
    if (search_state->match_state[i] == FULFILLED &&
        cpat->criteria[i]->wildcard == NONE_OR_ONE)
    {
      search_state->match_state[i] = UNIQUELY_FULFILLED;
      return INCOMPLETE_MATCH;
    }

    if (search_state->match_state[i] != UNIQUELY_FULFILLED)
    {
      if (matches(byte, cpat->criteria[i]))
      {
        search_state->criteria_index = i+1;
        /* now try to advance to the next non-wildcard */
        if (matches(byte, cpat->criteria[i+1]))
        {
          search_state->criteria_index = i+2;
          search_state->match_state[i+1] = UNIQUELY_FULFILLED;
        }

        return INCOMPLETE_MATCH;
      }

      search_state->match_state[i] = UNFULFILLED;
    }
  }

  return NO_MATCH;
}

search_result_t feed_search(char byte, search_state_t *search_state)
{
  int i;
  compiled_pattern_t *cpat = &search_item[current_search].compiled_pattern;

  for (i=0; i<=search_state->criteria_index; i++)
  {
    if (i >= cpat->criteria_count)
      return NO_MATCH;

    if (search_state->match_state[i] != UNIQUELY_FULFILLED)
    {
      if (matches(byte, cpat->criteria[i]))
      {
        /* handle match */
        if ((i+1) == cpat->criteria_count)
          return MATCH_FOUND;

        switch (cpat->criteria[i]->wildcard)
        {
          case NO_WILDCARD:
          case ONE_ONLY:
            search_state->match_state[i] = UNIQUELY_FULFILLED;
            break;
          case NONE_OR_ONE:
            if (search_state->match_state[i] == UNFULFILLED)
            {
              search_state->match_state[i] = FULFILLED;
              if (i == search_state->criteria_index)
                search_state->criteria_index++; /* check also next criteria since
                                                   this can be an empty set */
            }
            else
              search_state->match_state[i] = UNIQUELY_FULFILLED;
            break;
          case NONE_OR_MORE:
            search_state->match_state[i] = FULFILLED;
            if (i == search_state->criteria_index)
              search_state->criteria_index++; /* check also next criteria since
                                                   this can be an empty set */
            break;
          case ONE_OR_MORE:
            if (search_state->match_state[i] == FULFILLED)
              search_state->criteria_index++; /* check also next criteria since
                                                   this can be an empty set */
            search_state->match_state[i] = FULFILLED;
            break;
          default:
            return MATCH_ERROR;
            break;
        }

        if (i == search_state->criteria_index)
        {
          search_state->criteria_index++;
          return INCOMPLETE_MATCH;
        }

      }
      else
      {
        /* handle unmatch */
        switch (cpat->criteria[i]->wildcard)
        {
          case NO_WILDCARD:
          case ONE_ONLY:
            if (search_state->match_state[i] == UNFULFILLED)
              return rollback(byte, search_state);
            break;
          case NONE_OR_ONE:
            if (search_state->match_state[i] == UNFULFILLED &&
                i == search_state->criteria_index)
                search_state->criteria_index++; /* check also next criteria since
                                                   this can be an empty set */
            search_state->match_state[i] = UNIQUELY_FULFILLED;
            break;
          case NONE_OR_MORE:
            search_state->match_state[i] = UNIQUELY_FULFILLED;
            if (i == search_state->criteria_index)
              search_state->criteria_index++; /* check also next criteria since
                                                   this can be an empty set */
            break;
          case ONE_OR_MORE:
            if (search_state->match_state[i] == UNFULFILLED)
              return rollback(byte, search_state);
            else
            {
              search_state->match_state[i] = UNIQUELY_FULFILLED;
              if (i == search_state->criteria_index)
                search_state->criteria_index++; /* check also next criteria since
                                                   this can be an empty set */
            }
            break;
          default:
            return MATCH_ERROR;
            break;
        }

      }
    }
  }

  return INCOMPLETE_MATCH;
}

void buf_search(search_aid_t *search_aid)
{
  search_state_t search_state;
  search_result_t result;
  int start_offset = 0, end_offset = 0;

  if (search_aid == NULL)
    return;

  if (search_aid->buf_size < 1)
    return;

  if (search_item[current_search].used == FALSE)
  {
    search_aid->hl_start = -1;
    search_aid->hl_end   = -1;
    return;
  }

  if (search_aid->hl_start == -1)
    start_offset = 0;
  else
    start_offset = search_aid->hl_start - search_aid->buf_start_addr + 1;

  do
  {
    end_offset = 0;
    search_state_reset(&search_state);
    do
    {
      result = feed_search(search_aid->buf[start_offset + end_offset], &search_state);
      if (result == MATCH_ERROR)
      {
        update_status("SEARCH ERROR! BUG?");
        return;
      }
      end_offset++;
    } while( result == INCOMPLETE_MATCH                         &&
             (start_offset + end_offset) < search_aid->buf_size &&
             (user_prefs[MAX_MATCH].value ?
               (end_offset - start_offset) < user_prefs[MAX_MATCH].value :
                1)
           );

    if (result == MATCH_FOUND)
    {
      search_aid->hl_start = search_aid->buf_start_addr + start_offset;
      search_aid->hl_end = search_aid->hl_start + end_offset; /* -1? */
    }

    start_offset++;
  } while(result != MATCH_FOUND && start_offset < search_aid->buf_size);

  if (result != MATCH_FOUND)
  {
    search_aid->hl_start = -1;
    search_aid->hl_end = -1;
  }

}

void free_compiled_pattern(compiled_pattern_t *cpat)
{
  int i;
  for (i=0; i<cpat->criteria_count; i++)
    free(cpat->criteria[i]);
  cpat->criteria_count = 0;
}

void set_search_term(char *pattern)
{
  int i, j, k, v, len;
  match_criteria_t *c = NULL;
  int escape = 0, buildrange = 0, buildset = 0, not = 0;
  unsigned char tmp_range[MAX_RANGE_COUNT], str2hex[2], prev_char, value;
  compiled_pattern_t *cpat = &search_item[current_search].compiled_pattern;

  free_compiled_pattern(cpat);

  strncpy(search_item[current_search].pattern, pattern, MAX_SEARCH_PAT_LEN);

  len = strnlen(pattern, MAX_SEARCH_PAT_LEN);

  for (i=0; i<len; i++)
  {
    if (!escape)
    {
      switch(pattern[i])
      {
        case '\\':
          escape = 1;
          continue;
        case '.':
          if (buildset == 1 || buildrange == 1)
          {
            pat_err("Invalid char for set or range",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          c = malloc(sizeof(match_criteria_t));
          c->range_count = MAX_RANGE_COUNT;
          for (j=0; j<MAX_RANGE_COUNT; j++)
            c->range[j] = j;
          c->wildcard = ONE_ONLY;
          cpat->criteria[cpat->criteria_count] = c;
          cpat->criteria_count++;
          continue;
        case '?':
          if (buildset == 1 || buildrange == 1)
          {
            pat_err("Invalid char for set or range",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          if (c == NULL)
          {
            pat_err("? must proceed a valid set or char",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          c->wildcard = NONE_OR_ONE;
          c = NULL;
          continue;
        case '+':
          if (buildset == 1 || buildrange == 1)
          {
            pat_err("Invalid char for set or range",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          if (c == NULL)
          {
            pat_err("? must proceed a valid set or char",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          c->wildcard = ONE_OR_MORE;
          c = NULL;
          continue;
        case '*':
          if (buildset == 1 || buildrange == 1)
          {
            pat_err("Invalid char for set or range",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          if (c == NULL)
          {
            pat_err("? must proceed a valid set or char",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          c->wildcard = NONE_OR_MORE;
          c = NULL;
          continue;
        case '[':
          if (buildset == 1 || buildrange == 1)
          {
            pat_err("Invalid char for set or range",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          buildset = 1;
          c = malloc(sizeof(match_criteria_t));
          c->range_count = 0;
          c->wildcard = ONE_ONLY;
          cpat->criteria[cpat->criteria_count] = c;
          cpat->criteria_count++;
          continue;
        case ']':
          if (buildset == 0)
          {
            pat_err("No active set",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          if (buildrange == 1)
          {
            pat_err("Invalid char for range",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          if (c->range_count == 0)
          {
            pat_err("Empty range invalid",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          buildset = 0;
          if (not) /* invert the selection */
          {
            for(j=0, v=0; j<MAX_RANGE_COUNT; j++)
            {
              for(k=0; k<c->range_count; k++)
              {
                if (c->range[k] == j)
                  break;
              }
              if (k == c->range_count)
              {
                tmp_range[v] = j;
                v++;
              }
            }
            c->range_count = v;
            memcpy(c->range, tmp_range, v);
          }
          continue;
        case '-':
          if (buildset == 0)
          {
            pat_err("Can only build range within a set",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          if (buildrange == 1)
          {
            pat_err("Already building range",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          if (c == NULL)
          {
            pat_err("- must proceed a valid character",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          buildrange = 1;
          continue;
        case '^':
          if (buildset == 0)
          {
            pat_err("Can only negate within a set",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          if (buildrange == 1)
          {
            pat_err("Invalid char for range",
                    pattern, i, MAX_SEARCH_PAT_LEN);
            search_item[current_search].used = FALSE;
            return;
          }
          not = 1;
          continue;
        default:
          break;
      }
    }

    value = pattern[i];

    if (search_item[current_search].search_window == SEARCH_HEX)
    {
      if ((i+1) >= len)
      {
        pat_err("Hex chars should have two nibbles",
                pattern, i, MAX_SEARCH_PAT_LEN);
        search_item[current_search].used = FALSE;
        return;
      }

      if (!is_hex(pattern[i]))
      {
        pat_err("Invalid hex digit",
                pattern, i, MAX_SEARCH_PAT_LEN);
        search_item[current_search].used = FALSE;
        return;
      }

      if (!is_hex(pattern[i+1]))
      {
        pat_err("Invalid hex digit",
                pattern, i+1, MAX_SEARCH_PAT_LEN);
        search_item[current_search].used = FALSE;
        return;
      }

      str2hex[1] = 0;
      str2hex[0] = pattern[i++];
      value = (char)(strtol((char *)str2hex, NULL, 16) & 0xF);
      value = value << 4;
      str2hex[0] = pattern[i];
      value |= (char)(strtol((char *)str2hex, NULL, 16) & 0xF);
    }

    if (buildrange)
    {
      if (c->range_count < 1)
      {
        pat_err("Invalid range",
                pattern, i, MAX_SEARCH_PAT_LEN);
        search_item[current_search].used = FALSE;
        return;
      }
      prev_char = c->range[c->range_count - 1];
      if (value > prev_char)
      {
        for(j=value; j>prev_char; j--)
        {
          c->range[c->range_count] = j;
          c->range_count++;
        }
      }
      else
      {
        for(j=value; j<prev_char; j++)
        {
          c->range[c->range_count] = j;
          c->range_count++;
        }
      }
      buildrange = 0;
    }
    else if (buildset)
    {
      c->range[c->range_count] = value;
      c->range_count++;
    }
    else /* normal char or escaped special char not in a set/range */
    {
      c = malloc(sizeof(match_criteria_t));
      c->range[0] = value;
      c->range_count = 1;
      c->wildcard = ONE_ONLY;
      cpat->criteria[cpat->criteria_count] = c;
      cpat->criteria_count++;
    }
  }

  search_item[current_search].used = TRUE;
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
      free_compiled_pattern(&search_item[i].compiled_pattern);
    search_item[i].used = FALSE;
  }
}


void fill_search_buf(off_t addr, int display_size, search_aid_t *search_aid, search_direction_t direction)
{
  off_t a;
  search_aid_t tmp_aid;

  if (search_aid == NULL)
    return;

  search_aid->hl_start = -1;
  search_aid->hl_end = -1;

  search_aid->display_addr = addr;

  a = addr - user_prefs[MAX_MATCH].value;
  if (address_invalid(a))
    a = 0;

  search_aid->buf_start_addr = a;

  a = addr + display_size + user_prefs[MAX_MATCH].value;
  if (address_invalid(a))
    a = display_info.file_size - 1;

  search_aid->buf_size = a - search_aid->buf_start_addr;

  search_aid->buf = (char *)malloc(search_aid->buf_size + 1);
  if (search_aid->buf == NULL)
  {
    msg_box("Could not allocate memory for search buf");
    return;
  }
  vf_get_buf(current_file,
             search_aid->buf,
             search_aid->buf_start_addr,
             search_aid->buf_size);

  tmp_aid = *search_aid;

  buf_search(search_aid);              /* find the first valid match in the buffer */
  while (search_aid->hl_start != -1)   /* while we continue to have valid matches */
  {
    if (direction == SEARCH_FORWARD)
    {
      if (search_aid->hl_end >= addr)    /* find the first valid match which has an end
                                          p oint past the display addr */
        break;
    }
    else
    {
      if (search_aid->hl_start < (addr + display_size)) /* find the last valid match
                                                            which has a start point
                                                            before the display addr */
        tmp_aid = *search_aid;
      else
        break;
    }

    buf_search(search_aid);            /* if the condition is not met find the next match */
  }

  if (direction == SEARCH_BACKWARD)
    *search_aid = tmp_aid;

}


void free_search_buf(search_aid_t *search_aid)
{
  if (search_aid == NULL)
    return;

  if (search_aid->buf == NULL)
    return;

  free(search_aid->buf);

  search_aid->buf = NULL;
}

