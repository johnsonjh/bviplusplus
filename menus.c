#include "menus.h"
#include "string.h"

void m_open(void)
{
  return;
}

menu_t file_menu_items[] = {
  { NULL, "Open", 0, m_open, NULL, 0 },
  { NULL, "Close", 0, m_open, NULL, 0 },
  { NULL, "Exit", 0, m_open, NULL, 0 }
};

menu_t file_menu = {
   (WINDOW *)NULL, "File", 0, (void *)NULL, (menu_t *)&file_menu_items, 3
};


static void gethw_menu(menu_t *m, int *h, int *w)
{
  int width, i;
  *h = m->num_subs;
  *w = 0;
  for (i=0; i < *h; i++)
  {
    if ((width = strlen(m->sub_menu[i].text)) > *w)
      *w = width;
  }
}

void display_menu(menu_t *m, int x, int y)
{
  int width, height, i;
  gethw_menu(m, &height, &width);
  m->w = newwin(height, width, y, x);
  for (i=0; i<height; i++)
  {
    mvwaddstr(m->w, i, 0, m->sub_menu[i].text);
  }
  wrefresh(m->w);
}

