#include "irc.h"
#include "curl_wrap.h"
#include "global.h"
#include "neurobotapi.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int validate_http(char * line)
{
    char * ret, * t, * p;

    ret = strstr(line, "http");
    
    if (!ret)
        return -1;

    /* This is the same as a switch-case, in case you don't know */
    t =   (p = strchr(ret, ' '))  ? p 
        : (p = strchr(ret, '\r')) ? p
        : (p = strchr(ret, '\n')) ? p
        : 0;

    if (t) *t = '\0'; else return -1;

    memmove(line, ret, strlen(ret) + 1);

    return 1;
}

static int parse_title(char * dest, char * src)
{
    char * t;

    t = n_get_tag_value(src, "title");
    if (t) {
        strcpy(dest, t); 
        return 1;
    }

    return -1;
}

char * mod_title(struct irc_t * irc)
{
    char  title[256];
    char  * content = NULL;

    if (validate_http(irc->request) < 0 )
        return strdup("invalid url.");

    content = curl_perform(irc->request);

    if (!content) return strdup("couldn't fetch the content");

    if ( parse_title(title, content) > 0 ) {
        free(content);
        return strdup(title);
    }

    return NULL;
}


