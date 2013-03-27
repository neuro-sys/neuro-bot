#include "irc.h"
#include "curl_wrap.h"
#include "global.h"

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int validate_http(char * line)
{
    char * ret, * t, * p;

    ret = strstr(line, "http");
    
    if (!ret)
        return -1;

    t = (p = strchr(ret, ' '))  ? p 
        : (p = strchr(ret, '\r')) ? p
        : (p = strchr(ret, '\n')) ? p
        : 0;

    if (t) *t = '\0'; else return -1;

    memmove(line, ret, strlen(ret) + 1);

    return 1;
}

static int parse_title(char * dest, char * src)
{
    GRegex     * regex;
    GMatchInfo * match_info;

    regex = g_regex_new("(?s)(?i)<title.*>(.*?)<\\/title>", G_REGEX_MULTILINE, 0, NULL);
    g_regex_match(regex, src, 0, &match_info);
    if (g_match_info_matches(match_info)) {
        char * t = g_match_info_fetch(match_info, 1);
        strcpy(dest, "[TITLE] :");
        strncat(dest, t, 255);
        g_free(t);
        g_match_info_free(match_info);
        g_regex_unref(regex);

        return 1;
    }
    g_match_info_free(match_info);
    g_regex_unref(regex);

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

    return strdup("could not parse title");
}


