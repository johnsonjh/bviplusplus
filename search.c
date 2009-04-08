#include <regex.h>

search_item_t search_item[MAX_SEARCHES];
int current_search = 0;
regmatch_t matches[MAX_SEARCH_MATCHES];


void buf_search(char *buf, int buf_size, int start, int *match_start, int *match_end)
{
  regexec (search_item[current_search].compiled,
           buf,
           MAX_SEARCH_MATCHES,
           matches,
           REG_NOTBOL|REG_NOTEOL);
//matches[0].rm_so; /* start */
//matches[0].rm_eo; /* end */
}

void set_search_term(char *pattern)
{
  regcomp(&search_item[current_search].compiled,
          pattern[current_search],
          REG_ICASE); /* should = 0, otherwise see error stuff below */
  //regerror();
  //char *get_regerror (int errcode, regex_t *compiled)
          //{
            //size_t length = regerror (errcode, compiled, NULL, 0);
            //char *buffer = malloc(length);
            //(void) regerror (errcode, compiled, buffer, length);
            //return buffer;
          //}
/* then free buffer */
  //regfree(regex);
}

void search_cleanup(void)
{
  //regfree(regex);
}




