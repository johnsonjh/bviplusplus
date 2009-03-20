#include "virt_file.h"

#ifndef __USER_PREFS_H__
#define __USER_PREFS_H__

typedef struct user_pref_s
{
  char *name;
  long current_value;
  long default_value;
} user_pref_t;

typedef enum
{
  DISPLAY_BINARY,
  LIL_ENDIAN,
  GROUPING,
  BLOB_GROUPING,
  BLOB_GROUPING_OFFSET
} user_pref_e;

extern user_pref_t user_prefs[];

#endif /* __USER_PREFS_H__ */
