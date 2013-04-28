#include "neurobotapi.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char * keywords[MAX_KEYWORDS] = { "http", "https" };

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

#ifdef _WIN32
__declspec(dllexport)
#endif
void mod_title(struct irc_t * irc, char * reply_msg)
{
    char * content = NULL;
    char * t;

    if (validate_http(irc->request) < 0 )
        return;

    content = curl_perform(irc->request);

    if (!content) return;

    if ( parse_title(reply_msg, content) > 0 ) {
        t = reply_msg;
        while (*t != '\0') { if (*t == '\n' || *t == '\t') t[0] = ' '; i++}
    }

    free(content);
    return;
}


