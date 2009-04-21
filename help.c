#include <string.h>
#include <stdlib.h>
#include "display.h"
#include "user_prefs.h"
#include "app_state.h"
#include "virt_file.h"
#include "search.h"
#include "key_handler.h"
#include "help.h"


char *help_text[] = {
  "Bviplus is designed to be as similar to vim as possible.",
  "Here are some basics:",
  " ",
  "For this help dialogue:",
  "  :help",
  " ",
  "Command/Insert toggle",
  "  ESC             Exit insert mode",
  "  i               Insert before",
  "  a               Insert after",
  " ",
  "Saving/Quitting:",
  "  :e [filename]   Open 'filename' or newfile if none specified",
  "  :e!             Reload current file discarding changes",
  "  :q              Quit",
  "  :q!             Quit without saving",
  "  :qa             Quit all",
  "  :qa!            Quit all without saving",
  "  :w              Save",
  "  :wa             Save all",
  "  :wq             Save and quit",
  "  :wqa            Save all and quit",
  "  :waq",
  "  :w <name>       Save as <name>",
  " ",
  "Movement in command mode:",
  "  TAB KEY         Move between HEX/ASCII windows",
  "  k               Move up",
  "  j               Move down",
  "  h               Move left",
  "  l               Move right",
  "  (Arrow keys also work)",
  " ",
  "More movement:",
  "  <number>G       Jump to address <number>",
  "  :<number>       Jump to address <number>",
  "  :0x<hex number> Jump to address <hex number>",
  "  0               Move to beginning of line",
  "  HOME KEY",
  "  ^",
  "  $               Move to end of line",
  "  END KEY",
  "  m<0-9,a-z,A-Z>  Set mark at cursor location",
  "  `<0-9,a-z,A-Z>  Jump to mark",
  "  PAGE_DOWN KEY   Page down",
  "  PAGE_UP KEY     Page up",
  "  /               Ascii search",
  "  \\               Hex search",
  "  n               Next search match",
  "  N               Previous search match",
  " ",
  "Editing:",
  "  v               Toggle visual select mode",
  "                  (ESC exits visual select mode)",
  "  x               Delete byte or selection",
  "  X               Delete previous byte",
  "  r               Replace byte or selection",
  "  R               Overwrite",
  "  y               Yank byte or selection",
  "  Y",
  "  p               Paste after",
  "  P               Paste before",
  "  d               Delete to next motion",
  "  D",
  "  u               Undo",
  "  U               Redo",
  " ",
  "Settings:",
  "       Option               Arguments    Default   Alias      Effect",
  "       ------               ---------    -------   -----      ------",
  "  :set                                                        Show settings",
  "  :set binary               <on|off>     off       bin        Binary display *",
  "  :set little_endian        <on|off>     off       le         Little endian *",
  "  :set grouping             <1-16>       1         grp        Set byte grouping *",
  "  :set blob_grouping        <0-n>        0         blob       Highlight grouping",
  "  :set blob_grouping_offset <0-n>        0         bloboff    Highligh offset",
  "  :set columns              <1-n>        0         cols       Set the max cols",
  "  :set search_hl            <on|off>     on        hl         Search highlighting",
  "  :set ignorecase           <on|off>     on        case       Case sensativ search",
  "  :set extended_regex       <on|off>     on        eregex     Extended expressions",
  " ",
  "  >                Increase blob_grouping_offset",
  "  <                Decrease blob_grouping_offset",
  " ",
  "  * Experimental, may cause display problems ",
  " ",
  "ESC ESC temporarily turns off search highlighting (until next search)",
  " ",
  "Running external programs on data sets:",
  "  (First select 1 or more bytes using visual select)",
  "  :ex <external program>                    Run the external program on the visual select data.",
  "  :external <external program>              The program should be one which reads from",
  "                                            stdin and writes to stdout.",
  "                                            eg. ':ex md5sum'",
  " ",
  "Files:",
  "  :bn             Next file",
  "  :next",
  "  ~",
  " ",
  "  :bp             Last file",
  "  :prev",
  "  :previous",
  "  :last",
  " ",
  "  alt+<n>         Switch to <n>th open file",
  0,
};

void scrollable_window_display(char **text)
{
  WINDOW *scrollbox;
  int i, y = 0, c = 0, update = 1, scroll_lines = SCROLL_BOX_H - 4, len = 0;

  scrollbox = newwin(SCROLL_BOX_H, SCROLL_BOX_W, SCROLL_BOX_Y, SCROLL_BOX_X);

  while (len < MAX_LINES_SCROLLBOX && text[len] != NULL)
    len++;

  curs_set(0);

  do
  {
    switch(c)
    {
      case 'j':
      case KEY_DOWN:
        y++;
        if (y>(len - (SCROLL_BOX_H - 10)))
        {
          y--;
          flash();
        }
        else
          update = 1;
        break;
      case 'k':
      case KEY_UP:
        y--;
        if (y<0)
        {
          y++;
          flash();
        }
        else
          update = 1;
        break;
    case BVICTRL('f'):
    case KEY_NPAGE:
        if (y == (len - (SCROLL_BOX_H - 10)))
        {
          flash();
        }
        else
        {
          y+=(SCROLL_BOX_H-4);
          if (y > (len - (SCROLL_BOX_H - 10)))
            y = (len - (SCROLL_BOX_H - 10));
          update = 1;
        }
        break;
    case BVICTRL('b'):
    case KEY_PPAGE:
        if (y == 0)
        {
          flash();
        }
        else
        {
          y-=(SCROLL_BOX_H-4);
          if (y<0)
            y=0;
          update = 1;
        }
        break;
    }

    if (update)
    {
      update = 0;
      werase(scrollbox);
      box(scrollbox, 0, 0);
      for (i=0; i<scroll_lines; i++)
      {
        if ((i+y) < len)
          mvwaddstr(scrollbox, i+1, 1, text[i+y]);
      }
      mvwprintw(scrollbox, SCROLL_BOX_H - 3, 1, "_________________________________________");
      mvwprintw(scrollbox, SCROLL_BOX_H - 2, 1, " [j|DOWN] Down  [k|UP] Up  [q|ESC] Quit |");
      wrefresh(scrollbox);
    }
    c = wgetch(scrollbox);
  } while(c != 'q' && c != ESC);

  delwin(scrollbox);

  curs_set(1);
  print_screen(display_info.page_start);
}

void big_buf_display(char *buf, int max_size)
{
  char *end, *this, *next;
  char *text[MAX_LINES_SCROLLBOX];
  int index = 0, size;

  end = strchr(buf, EOF);
  if (end == NULL)
    end = buf+size;

  if (end > buf+size)
    end = buf+size;

  this = buf;
  next = strchr(this, '\n');
  while(next != NULL && next < end && index < MAX_LINES_SCROLLBOX)
  {
    size = (next - this);
    text[index] = (char *)malloc(size+1);
    memcpy(text[index], this, size);
    text[index][size] = '\0';
    this = next+1;
    if (this < end)
      next = strchr(this, '\n');
    else
      break;
    index++;
  }

  if (index >= MAX_LINES_SCROLLBOX)
    index = MAX_LINES_SCROLLBOX - 1;
  text[index] = 0;

  scrollable_window_display(text);

  index = 0;
  while (text[index] != 0)
    free(text[index++]);

}

