
#ifndef __HELP_H__
#define __HELP_H__

#define MAX_LINES_SCROLLBOX 4096

extern char *help_text[];

void scrollable_window_display(char **text);
void big_buf_display(char *buf, int max_size);

#endif /* __HELP_H__ */
