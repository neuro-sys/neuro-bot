#include "neurobotapi.h"
#include "json.h"
#include "curl_wrap.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static char * parse_json_wiki(char * data)
{
    char * temp;
    json_value * root, * extract;
    int i;

    temp = malloc(MAX_IRC_MSG);

    root = json_parse(data);
    if (!root)
      return NULL;

    extract = n_json_find_object(root, "extract");
    if (!extract)
        return NULL;

    strncpy(temp, extract->u.string.ptr, MAX_IRC_MSG);

    json_value_free(root);

    return temp;
}

#ifdef _WIN32
__declspec(dllexport)
#endif
void mod_wiki(struct irc_t * irc, char * reply_msg)
{
    char * p, * t;
    struct http_req * http;
    char url_path[512];
    int i;

    /* strip the leading command prefix (.wiki) */
    p = strchr(irc->request, ' ');

    if (!p++)
        return;
    
    i = strlen(p);

    /* strip the trailing cr-lf */
    p[i-1] = '\0';

    t = p;

    while ((t = strchr(t, ' ')) != NULL)
        *t = '+';

    sprintf(url_path, "http://en.wikipedia.org/w/api.php?action=query&prop=extracts&exchars=350&titles=%s&format=json&redirects", p);

    http = curl_perform(url_path, NULL);

    t = http->body;
    while ( (t = strstr(t, "\\n")) != NULL) {
        t[0] = '-';
        t[1] = ' ';
    }

    p = parse_json_wiki(http->body);
   
    if (!p)
        return;

    n_strip_tags(reply_msg, p);

    free(http->base);
    free(http);
}
