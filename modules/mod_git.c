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

void parse_json_event(char * body, char * dest)
{
    json_value * root, * payload, * comitter_name, * message, * url;

    root = json_parse(body);
    if (!root) return;

    payload = n_json_find_object(root, "payload");
    comitter_name = n_json_find_object(payload, "name");
    message = n_json_find_object(payload, "message");
    url = n_json_find_object(payload, "url");

    sprintf(dest, "[%s] [%s] [%s]\n", comitter_name->u.string.ptr,
                                  message->u.string.ptr,
                                  url->u.string.ptr);
    json_value_free(root);
}

int parse_etag(char * _text)
{
    char * t;
    char text[1024];

    strncpy(text, _text, 1024);

    t = strstr(text, "X-RateLimit-Limit");
    if (NULL == t)
        goto SKIP;

    t = strtok(t, ":");
    if (NULL == t)
        goto SKIP;

    t = strtok(NULL, "\r\n");
    if (NULL == t)
        goto SKIP;

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

    memset(etag_last, 0, sizeof (etag_last));

    fprintf(stderr, "looper started\n");
    childPID = fork();

    if (childPID == 0)
    {
        while (1) { 
            struct http_req * http;

            /* The first time coming if there's no etag is set. */
            //if (0 == strcmp(etag_last, "")) {
                fprintf(stderr, "first coming\n");
                http = curl_perform(GIT_EVENT_API_URL, NULL);
#if 0
            } else {
                struct curl_slist * slist = NULL;
                char reqbuf[256];

                snprintf(reqbuf, 256, "If-None-Match: %s", etag_last);
                fprintf(stderr, reqbuf);
                slist = curl_slist_append(slist, reqbuf); 
                http = curl_perform(GIT_EVENT_API_URL, slist);
            }
#endif
            parse_etag(http->header);

            //if (!strstr(http->header, "304 Not Modified")) {
                char message[510], response[510];

                parse_json_event(http->body, message);
                sprintf(response, "PRIVMSG #gameover :%s\r\n", message);
                network_send_message(&irc->session->network, response);
            //}

            free(http->header);
            free(http->body);
            free(http);

            usleep(x_rate_limit *1000*1000);
        }
    } else if (childPID == -1) {
        fprintf(stderr, "The plugin mod_git is failed to fork\n");
    } else { 
        fprintf(stderr, "The plugin mod_git has started.\n");
    }
}
