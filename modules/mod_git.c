#include "neurobotapi.h"
#include "json.h"
#include "curl_wrap.h"

#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#define GIT_EVENT_API_URL "https://api.github.com/repos/neuro-sys/neuro-bot/events"

int looper = 1;

static char etag_last[1024];
static int x_rate_limit;

int parse_etag(char * _text)
{
    char * t;
    char text[1024];

    strncpy(text, _text, 1024);

    t = strstr(text, "X-RateLimit-Limit");
    if (NULL == t)
        goto SKIP;

    fprintf(stderr, "x-rate found.\n");

    t = strtok(t, ":");
    if (NULL == t)
        goto SKIP;

    t = strtok(NULL, "\r\n");
    if (NULL == t)
        goto SKIP;

    fprintf(stderr, "it's :%s\n", t+1);
    x_rate_limit = atoi(t+1);

SKIP:
    strncpy(text, _text, 1024);

    t = strstr(text, "ETag");
    if (NULL == t)
        return -1;

    t = strtok(t, ":");
    if (NULL == t)
        return -1;

    t = strtok(NULL, "\r\n");
    if (NULL == t)
        return -1;

    strcpy(etag_last, t+1);

    return 0;
}

#ifdef _WIN32
__declspec(dllexport)
#endif
void mod_git(struct irc_t * irc, char * reply_msg)
{
    pid_t childPID;
    int var_lcl = 0;

    fprintf(stderr, "looper started\n");
    childPID = fork();

    if (childPID == 0)
    {
        while (1) { 
            struct http_req * http;

            /* The first time coming. */
            if (0 == strcmp(etag_last, "")) {
                http = curl_perform(GIT_EVENT_API_URL, NULL);
            } else {
                struct curl_slist * slist = NULL;
                char reqbuf[256];

                snprintf(reqbuf, 256, "If-None-Match: %s", etag_last);
                slist = curl_slist_append(slist, reqbuf); 
                http = curl_perform(GIT_EVENT_API_URL, slist);
            }
            parse_etag(http->base);

            //snprintf(reply_msg, MAX_IRC_MSG, "%s - %d", etag_last, x_rate_limit);
            fprintf(stderr, "%s - %d\n", etag_last, x_rate_limit);
            free(http);

            usleep(x_rate_limit *1000*1000);
        }
    } else if (childPID == -1) {
        fprintf(stderr, "The plugin mod_git is failed to fork\n");
    }
}
