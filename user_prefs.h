#include "virt_file.h"

#ifndef __USER_PREFS_H__
#define __USER_PREFS_H__

#define MAX_GRP 16

typedef enum
{
  P_NONE,
  P_BOOL,
  P_LONG,
} p_flags_e;

typedef struct user_pref_s
{
  char *name;
  char *short_name;
  long value;
  long def;
  long min;
  long max;
  p_flags_e flags;
} user_pref_t;

typedef enum
{
  DISPLAY_BINARY,
  LIL_ENDIAN,
  GROUPING,
  BLOB_GROUPING,
  BLOB_GROUPING_OFFSET,
  MAX_COLS
} user_pref_e;

extern user_pref_t user_prefs[];

#endif /* __USER_PREFS_H__ */
