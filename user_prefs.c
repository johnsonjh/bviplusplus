/*************************************************************
 *
 * File:        user_prefs.c
 * Author:      David Kelley
 * Description: User preferences and related functions
 *
 *************************************************************/

#include "user_prefs.h"

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

