#include "neurobotapi.h"

#include <string.h>

void n_strip_tags(char * dest, char * src)
{
    int inside = 0;

    while (*src != '\0')
    {

        if (*src == '>') {
            inside = 0;
            src++;
        } 
        
        if (*src == '<' || inside) {
            inside = 1;
            src++;
            continue;
        }

        *dest++ = *src++;
    }
    *dest = '\0';
}

char * n_get_tag_value(char * body, char * tagname)
{
    char * btag, * etag;
    char buf[1024];
    char * ret;

    while ( *body != '\0' )
    {
        btag = strchr(body, '<');
        body = btag;   
        if (!btag) return "";

        etag = strchr(body, '>');
        if (!etag) return "";

        strncpy(buf, btag+1, etag-btag);
        body = etag+1;

        if (!strcasestr(buf, tagname))
            continue;

        ret = strtok(body, "<");
    
        return ret;
    }
}

