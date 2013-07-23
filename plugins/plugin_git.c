/**
 * gcc plugin_git.c utils/curl_wrapper.c utils/json.c \
 *      `pkg-config.exe libcurl --libs --cflags` \
 *      --shared -oplugin_git.so
 *
 */
#include "plugin_client.h"

#include "utils/curl_wrapper.h"
#include "utils/json.h"

#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#define GIT_EVENT_API_URL "https://api.github.com/repos/neuro-sys/neuro-bot/events"

static char etag_last[1024];
static int x_rate_limit;

#if 0
static void sanitize_json_response(char *body)
{
    size_t len = strlen(body);
    int i = 0;
 
    while (i < len && body[i++] != '[') {}
    while (i+1 < len && !(body[i] == ']' && body[i+1] != '}')) i++;

    body[i+1] = 0;
}
#endif

static void parse_json_event(char * body, char * dest)
{
    json_value * root, * payload, * comitter_name, * message, *name, * sha;

    //sanitize_json_response(body);
    root = json_parse(body);
    if (!root) return;

    payload = n_json_find_object(root, "payload");
    comitter_name = n_json_find_object(payload, "name");
    message = n_json_find_object(payload, "message");
    name = n_json_find_object(root, "name");
    sha = n_json_find_object(payload, "sha");

    sprintf(dest, "* Commiter: [%s] - Msg: [%s] Url: [http://github.com/%s/commit/%s]", 
                                  comitter_name->u.string.ptr,
                                  message->u.string.ptr,
                                  name->u.string.ptr,
                                  sha->u.string.ptr);
    json_value_free(root);
}

static void parse_etag(char * _text)
{
    char * t;
    char text[1024];

    if (_text == NULL || strlen(_text) < 1)
        return;

    strncpy(text, _text, 1024);

    t = strstr(text, "X-RateLimit-Limit");
    if (NULL != t) {
        t = strtok(t, ":");
        if (NULL != t) {
            t = strtok(NULL, "\r\n");
            if (NULL != t)
                x_rate_limit = atoi(t+1);
        }
    }

    strncpy(text, _text, 1024);

    t = strstr(text, "ETag");
    if (NULL == t)
        return;

    t = strtok(t, ":");
    if (NULL == t)
        return;

    t = strtok(NULL, "\r\n");
    if (NULL == t)
        return;

    strcpy(etag_last, t+1);
}

struct plugin_t * plugin;

void run(void)
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

        if (http == NULL || http->body == NULL || http->header == NULL)
            goto SKIP;

        parse_etag(http->header);

        if (!strstr(http->header, "304 Not Modified")) {
            char message[510];
            int i;

            parse_json_event(http->body, message);
            for (i = 0; i < plugin->irc->channels_siz; i++) {
                char * chan = plugin->irc->channels[i];
                sprintf(plugin->irc->response, "PRIVMSG %s :%s\r\n", chan, message);
                plugin->send_message(plugin->irc);
            }
        } 

        if (http->header != NULL) free(http->header);
        if (http->body != NULL) free(http->body);
        if (http != NULL) free(http);

SKIP:
        debug("I'm polling every %d seconds as per github wants me to.\n", x_rate_limit);
        usleep(x_rate_limit *1000*1000);
    }
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "github";
    plugin->is_looper  = 1;
    plugin->is_command = 0;
    plugin->is_grep    = 0;

    return plugin;
}

