#include "irc.h"
#include "global.h"
#include "curl_wrap.h"

#include <string.h>
#include <stdlib.h>
#include <jansson.h>
#include <curl/curl.h>
#include <glib.h>
#include <stdio.h>

void strip_html_tags(char * dest, char * src)
{
    int inside = 0;

    while (*src != '\0')
    {

        if (*src == '>') {
            inside = 0;
            src++;
        } else if (*src == '<' || inside) {
            inside = 1;
            src++;
            continue;
        }

        *dest++ = *src++;
    }

    *dest = '\0';
}

static char * parse_json_wiki(char * data)
{
    json_t        * root;
    json_error_t  error;
    json_t        * query, * search, * search_el, * snippet;
    char          * temp;

    root = json_loads(data, JSON_DECODE_ANY | JSON_DISABLE_EOF_CHECK , &error);
    if (!root)
        return NULL;

    query     = json_object_get(root, "query");
    search    = json_object_get(query, "search");
    search_el = json_array_get(search, 0);
    snippet   = json_object_get(search_el, "snippet");

    temp = strdup(json_string_value(snippet));

    json_decref(root);

    return temp;
}

void mod_line_wiki(struct irc_t * irc)
{
    char * p, * t;
    char * content;
    char url_path[512];
    int i;

    /* strip the leading command prefix (.wiki) */
    p = strchr(irc->request, ' ');

    if (!p++)
        return;
    
    i = strlen(p);

    /* strip the trailing cr-lf */
    p[i-2] = '\0';

    t = p;

    while ((t = strchr(t, ' ')) != NULL)
        *t = '_';

    sprintf(url_path, "http://en.wikipedia.org/w/api.php?action=query&list=search&srsearch=%s&format=json", p);

    content = curl_perform(url_path);

    p = parse_json_wiki(content);
   
    if (!p)
        return;

    t = malloc(strlen(p));

    strip_html_tags(t, p);

    sprintf(irc->response, "PRIVMSG %s :%s\r\n", irc->from, t);

    free(p);
    free(t);
    free(content);
}
