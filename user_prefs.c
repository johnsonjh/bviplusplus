#include "user_prefs.h"

 /*------------------------------------------------*/
/*  [name]                  [short_name]  [value]  [min]  [max]  [flags]*/
user_pref_t user_prefs[] = {
  { "binary",               "bin",              0,     0,     0, P_BOOL },
  { "little_endian",        "le",               0,     0,     0, P_BOOL },
  { "grouping",             "grp",              1,     1,    16, P_LONG },
  { "blob_grouping",        "blob",             4,     0,     0, P_LONG },
  { "blob_grouping_offset", "bloboff",          0,     0,     0, P_LONG },
  { "columns",              "col",              0,     0,     0, P_LONG },
  { "",                     "",                 0,     0,     0, P_NONE },
};

