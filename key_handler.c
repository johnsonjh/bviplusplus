#include "key_handler.h"
#include "bviplus_windows.h"
#include "user_prefs.h"
#include "display.h"
#include "actions.h"

void handle_key(int c)
{
  int x, y;
  getyx(window_list[WINDOW_HEX], y, x);

  switch (c)
  {
    case KEY_DOWN:
      y++;
      break;
    case KEY_UP:
      y--;
      break;
    case KEY_LEFT:
      x-=BYTES_PER_GROUP;
      break;
    case KEY_RIGHT:
      x+=BYTES_PER_GROUP;
      break;
    default:
      break;
  }

  wmove(window_list[display_info.cursor_window], y, x);
}


