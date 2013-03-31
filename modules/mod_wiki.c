#include "irc.h"
#include "global.h"
#include "curl_wrap.h"
#include "neurobotapi.h"

#include <string.h>
#include <stdlib.h>
#include <jansson.h>
#include <curl/curl.h>
#include <stdio.h>

static char * parse_json_wiki(char * data)
{
    json_t        * root;
    json_error_t  error;
    json_t        * query, * pages, * page_id, * extract;
    char          * temp;
    void          * iter;

    root        = json_loads(data, JSON_DECODE_ANY | JSON_DISABLE_EOF_CHECK , &error);
    if (!root)
        return NULL;

    query      = json_object_get(root, "query");
    pages      = json_object_get(query, "pages");

    iter = json_object_iter(pages);

    if (iter)
    {
        page_id = json_object_iter_value(iter);
    }

    extract = json_object_get(page_id, "extract");

    if (extract)
        temp = strdup(json_string_value(extract));
    else
        temp = strdup("This line intentionally left blank.");

    json_decref(root);
    json_decref(iter);

    return temp;
}

char * mod_wiki(struct irc_t * irc)
{
    char * ret;
    char * p, * t;
    char * content;
    char url_path[512];
    int i;

    /* strip the leading command prefix (.wiki) */
    p = strchr(irc->request, ' ');

    if (!p++)
        return strdup("");
    
    i = strlen(p);

    /* strip the trailing cr-lf */
    p[i-1] = '\0';

    t = p;

    while ((t = strchr(t, ' ')) != NULL)
        *t = '+';

    sprintf(url_path, "http://en.wikipedia.org/w/api.php?action=query&prop=extracts&exchars=350&titles=%s&format=json&redirects", p);

    content = curl_perform(url_path);

    t = content;
    while ( (t = strstr(t, "\\n")) != NULL) {
        t[0] = '-';
        t[1] = ' ';
    }

    p = parse_json_wiki(content);
   
    if (!p)
        return strdup("could not parse json wiki");

    t = malloc(strlen(p));

    n_strip_tags(t, p);

    ret = strdup(t);

    free(p);
    free(t);
    free(content);

    return ret;
}
