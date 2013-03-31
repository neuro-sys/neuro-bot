#include "irc.h"
#include "global.h"
#include "curl_wrap.h"
#include "neurobotapi.h"

#include "json.h"

#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <stdio.h>

static char * parse_json_wiki(char * data)
{
    char * temp;
    json_value * root, * query, * pages, * id, * extract;
    int i;

    temp = malloc(MAX_IRC_MSG);

    root = json_parse(data);

    query = root->u.object.values[0].value;
    if (!query) return "";

    pages = query->u.object.values[1].value;
    if (!pages) return "";

    id = pages->u.object.values[0].value;
    if (!id || id->u.string.length == 3)
        return "Not found.";

    extract = id->u.object.values[3].value;
    if (!extract) return "";

    strncpy(temp, extract->u.string.ptr, MAX_IRC_MSG);

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

    free(t);
    free(content);

    return ret;
}
