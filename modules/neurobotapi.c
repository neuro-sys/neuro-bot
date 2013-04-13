#include "neurobotapi.h"

char *  (*curl_perform)(char * url);
void    (*n_strip_tags)(char * dest, char * src);
char *  (*n_get_tag_value)(char * body, char * tagname);

#ifdef _WIN32
__declspec(dllexport)
#endif
void init(void ** fp_list, char *** keyword_ptr)
{
    curl_perform    = fp_list[0];
    n_strip_tags    = fp_list[1];
    n_get_tag_value = fp_list[2];

    *keyword_ptr = keywords;
}


