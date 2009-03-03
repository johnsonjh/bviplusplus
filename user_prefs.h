#include "virt_file.h"

#ifndef __USER_PREFS_H__
#define __USER_PREFS_H__

typedef struct user_prefs_s
{
  BOOL    display_binary;
  BOOL    little_endian;
  int     grouping;
  int     grouping_offset;
  int     blob_grouping;
  int     blob_grouping_offset;
} user_prefs_t;

extern user_prefs_t   user_prefs;

#endif /* __USER_PREFS_H__ */
