#include "neurobotapi.h"
#include "json.h"
#include "curl_wrap.h"

#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>

#define GIT_EVENT_API_URL "https://api.github.com/repos/neuro-sys/neuro-bot/events"

int looper = 1;

static char etag_last[1024];
static int x_rate_limit;

void parse_json_event(char * body, char * dest)
{
    json_value * root, * payload, * comitter_name, * message, * sha;

    root = json_parse(body);
    if (!root) return;

    payload = n_json_find_object(root, "payload");
    comitter_name = n_json_find_object(payload, "name");
    message = n_json_find_object(payload, "message");
    sha = n_json_find_object(payload, "sha");

    sprintf(dest, "Commiter: [%s] - Msg: [%s] Sha: [%s]", comitter_name->u.string.ptr,
                                  message->u.string.ptr,
                                  sha->u.string.ptr);
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
    memset(etag_last, 0, sizeof (etag_last));

    while (1) { 
        struct http_req * http;

        /* The first time coming if there's no etag is set. */
        if (0 == strcmp(etag_last, "")) {
            http = curl_perform(GIT_EVENT_API_URL, NULL);
        } else {
            /* Other times set the ETag header to check for 304 Notified in the response. */
            struct curl_slist * slist = NULL;
            char reqbuf[256];

            snprintf(reqbuf, 256, "If-None-Match: %s", etag_last);
            slist = curl_slist_append(slist, reqbuf); 
            http = curl_perform(GIT_EVENT_API_URL, slist);
        }
        parse_etag(http->header);

        if (!strstr(http->header, "304 Not Modified")) {
            char message[510], response[510];
            int i;

            parse_json_event(http->body, message);
            for (i = 0; i < irc->channels_siz; i++) {
                char * chan = irc->channels[i];
                sprintf(response, "PRIVMSG %s :%s\r\n", chan, message);
                network_send_message(&irc->session->network, response);
            }
        }

        free(http->header);
        free(http->body);
        free(http);

        usleep(x_rate_limit *1000*1000);
    }
}
