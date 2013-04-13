#include <string.h>
#include <ctype.h>

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

char * n_get_tag_value(char * body, const char * tagname)
{
    char * btag, * etag;
    char buf[1024];
    char * ret;
    char * t;
    char tag[1024];

    strncpy(tag, tagname, 1024);

    t = tag;
    while (*t) *t = toupper(*t), t++;

    while ( *body != '\0' )
    {
        btag = strchr(body, '<');
        body = btag;   
        if (!btag) return "";

        etag = strchr(body, '>');
        if (!etag) return "";

        strncpy(buf, btag+1, etag-btag);
        body = etag+1;

        /* upper-case everything for searching */
        t = buf;
        while (*t) *t = toupper(*t), t++;

        if (!strstr(buf, tag))
            continue;

        ret = strtok(body, "<");
        t = ret-1;
        while (*++t != '\0')
            if (*t == '\n' || *t == '\t')
                *t = ' ';

        return ret;
    }

    return "";
}

