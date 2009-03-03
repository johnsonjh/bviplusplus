#include <ncurses.h>

typedef struct menu_s menu_t;

struct menu_s
{
  WINDOW *w;               /* window to list the submenues */
  const char *text;              /* text to display for the menue */
  char key_position;       /* index into 'text' which is the hot key */
  void *action;            /* callback function when this menu is selected */
  menu_t *sub_menu;     /* array of menu pointers which are the items in this menu */
  int num_subs;          /* number of sub menues */
};


void m_open(void);
void display_menu(menu_t *m, int x, int y);

extern menu_t file_menu_items[];
extern menu_t file_menu;




