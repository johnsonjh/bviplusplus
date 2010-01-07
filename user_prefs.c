/*************************************************************
 *
 * File:        user_prefs.c
 * Author:      David Kelley
 * Description: User preferences and related functions
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
 *************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "user_prefs.h"
#include "creadline.h"

 /*------------------------------------------------*/
/*  [name]                  [short_name]  [value]  [default]  [min]  [max]        [flags]*/
user_pref_t user_prefs[] = {
  { "binary",               "bin",              0,         0,     0,     0,       P_BOOL },
  { "little_endian",        "le",               0,         0,     0,     0,       P_BOOL },
  { "grouping",             "grp",              1,         1,     1,     MAX_GRP, P_INT },
  { "blob_grouping",        "blob",             0,         0,     0,     0,       P_INT },
  { "blob_grouping_offset", "bloboff",          0,         0,     0,     0,       P_INT },
  { "columns",              "cols",             0,         0,     1,     0,       P_INT },
  { "search_hl",            "hl",               1,         1,     0,     0,       P_BOOL },
  { "search_immediate",     "si",               1,         1,     0,     0,       P_BOOL },
  { "ignorecase",           "ic",               0,         0,     0,     0,       P_BOOL },
  { "max_match",            "mm",             256,       256,     0,     0,       P_INT },
  { "",                     "",                 0,         0,     0,     0,       P_NONE },
};

action_code_t set_pref(const char *option, const char *value)
{
  char tmp[MAX_CMD_BUF], *ptr = NULL;
  int opt, set = 1;
  long val = 0;

  if (option == NULL)
    return E_INVALID;

  if (strncmp(option, "no", 2) == 0)
  {
    option += 2;
    set = 0;
  }

  opt = 0;
  while (strncmp(user_prefs[opt].name, "", MAX_CMD_BUF))
  {
    if (strncmp(user_prefs[opt].name,        option, MAX_CMD_BUF) == 0 ||
        strncmp(user_prefs[opt].short_name,  option, MAX_CMD_BUF) == 0)
    {

      if (user_prefs[opt].flags == P_BOOL)
      {
        if (value == NULL)
        {
          user_prefs[opt].value = set;
        }
        else
        {
          strncpy(tmp, value, MAX_CMD_BUF-1);
          for(ptr=tmp;*ptr;ptr++)
            *ptr=toupper(*ptr);
          if (strncmp("FALSE",  tmp, MAX_CMD_BUF) == 0  ||
              strncmp("OFF",    tmp, MAX_CMD_BUF) == 0  ||
              strncmp("NO",     tmp, MAX_CMD_BUF) == 0)
            set = set == 0 ? 1 : 0;

          user_prefs[opt].value = set;
        }

        break;
      }

      if (user_prefs[opt].flags == P_INT)
      {
        if (value == NULL)
        {
          msg_box("Not enough parameters to 'set %s', using default value: %d",
                  user_prefs[opt].name, user_prefs[opt].def);
          user_prefs[opt].value = user_prefs[opt].def;
          break;
        }

        val = atol(value);

        if ((val < user_prefs[opt].min && user_prefs[opt].min) ||
            (val > user_prefs[opt].max && user_prefs[opt].max))
        {
          msg_box("Value out of range for 'set %s' (min = %d, max = %d)",
                  user_prefs[opt].name,
                  user_prefs[opt].min,
                  user_prefs[opt].max);
          return E_INVALID;
        }

        user_prefs[opt].value = val;
        break;
      }

    }

    opt++;

  }

/******* Print some warnings since this stuff is not tested ******/
      if (strncmp(user_prefs[opt].short_name, "grp", MAX_CMD_BUF) == 0)
        msg_box("Warning, grouping other than 1 is experimental!!");
      if (strncmp(user_prefs[opt].short_name, "bin", MAX_CMD_BUF) == 0)
        msg_box("Warning, binary display mode is experimental!!");
      if (strncmp(user_prefs[opt].short_name, "le", MAX_CMD_BUF) == 0)
        msg_box("Warning, little endian display mode is experimental!!");
/*****************************************************************/

  action_do_resize();

  return E_SUCCESS;
}

void read_rc_file(void)
{
  FILE *fp;
  const char *home;
  char rcfile[FILENAME_MAX+1];
  char *line, *option, *value;
  size_t line_len = 256;
  int ret = 0;
  action_code_t error = E_SUCCESS;

  home = getenv("HOME");

  snprintf(rcfile, FILENAME_MAX, "%s/.bviplusrc", home);

  fp = fopen(rcfile, "r");
  if (fp == NULL)
  {
    printf("No rc file found at %s\n", rcfile);
    return;
  }

  line = malloc(line_len);

  while (!feof(fp))
  {
    ret = getline(&line, &line_len, fp);
    if (ret > 0)
    {
      option = strtok(line, " \t\n=");
      value = strtok(NULL, " \t\n=");
      if (option != NULL && option[0] != '#')
      {
        error = set_pref(option, value);
        if (error != E_SUCCESS)
          printf("Error parsing commands near character %ld in file %s\n",
                 ftell(fp), rcfile);
      }
    }
  }

  free(line);
  fclose(fp);

}

