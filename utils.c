#include <string.h>
#include <ctype.h>

#include "json.h"

json_value * n_json_get_object(json_value * object, char * name)
{
    int i;

    switch (object->type)
    {
    case json_object:
      for (i = 0; i < object->u.object.length; i++) 
        {
            if (strcmp(object->u.object.values[i].name, name))
              return object->u.object.values[i].value;
        }
    }
    
}

json_value * n_json_parse(char * data)
{
    return json_parse(data);
}

char * n_json_free(json_value * object)
{
    json_value_free(object);
}

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
        t = tag;
        while (*t) *t = toupper(*t), t++;

        if (!strstr(buf, tag))
            continue;

        ret = strtok(body, "<");

        return ret;
    }

    return "";
}

