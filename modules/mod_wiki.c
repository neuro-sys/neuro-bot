#include "irc.h"
#include "global.h"
#include "curl_wrap.h"

#include <string.h>
#include <stdlib.h>
#include <jansson.h>
#include <curl/curl.h>
#include <glib.h>
#include <stdio.h>

static char * parse_json_wiki(char * data)
{
    json_t        * root;
    json_error_t  error;
    json_t        * query, * search, * search_el, * snippet;
    char    * temp;

    root        = json_loads(data, JSON_DECODE_ANY | JSON_DISABLE_EOF_CHECK , &error);
    if (!root)
        return NULL;

    query     = json_object_get(root, "query");
    search    = json_object_get(query, "search");
    search_el = json_array_get(search, 0);
    snippet   = json_object_get(search_el, "snippet");

    temp = strdup(json_string_value(snippet));

    json_decref(snippet);
    json_decref(search_el);
    json_decref(search);
    json_decref(query);

    return temp;
}

void mod_line_wiki(struct irc_t * irc)
{
    char * p;
    char * content;
    char url_path[512];
    int i;

    p = strchr(irc->request, ' ');

    if (!p++)
        return;
    
    i = strlen(p);

    p[i-2] = '\0';

    sprintf(url_path, "http://en.wikipedia.org/w/api.php?action=query&list=search&srsearch=%s&format=json", p);

    content = curl_perform(url_path);

    p = parse_json_wiki(content);
   
    if (!p)
        return;
  
    sprintf(irc->response, "PRIVMSG %s :%s\r\n", irc->from, p);

    free(p);
    free(content);
}
