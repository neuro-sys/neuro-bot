#include "plugin_client.h"

#include "utils/curl_wrapper.h"
#include "utils/json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * keywords[10] = { "youtube.com", "youtu.be" };

struct youtube_t {
    double          rating;
    char            view_count[20];
    char            title[100];
    int             valid;
};

static void parse_json_youtube(char * data, struct youtube_t * youtube)
{
    json_value * root, * title, * _t, * rating, * viewcount;

    root = json_parse(data);

    title = n_json_find_object(root, "title");

    if (!title) return;

    _t = n_json_find_object(title, "$t");
    strcpy(youtube->title, _t->u.string.ptr);

    rating = n_json_find_object(root, "average");
    if (!rating) return;
    youtube->rating = rating->u.dbl;

    viewcount = n_json_find_object(root, "viewCount");

    if (!viewcount) return;
    strcpy(youtube->view_count, viewcount->u.string.ptr);

    youtube->valid = 1;

    json_value_free(root);
}

static void proc_info_youtube(struct irc_t * irc, struct youtube_t * youtube)
{
    char        url_path[512];
    struct http_req * http;
    char *t;
    char request[512];

    strcpy(request, irc->message.trailing);
    t = strstr(request, "v=");
    if (t) t +=2;
    t = strtok(t, " &\r\n\\");

    sprintf(url_path, "http://gdata.youtube.com/feeds/api/videos/%s?alt=json&ver=2", t);

    http = curl_perform(url_path, NULL);
    parse_json_youtube(http->body, youtube);
    free(http->header);
    free(http->body);
    free(http);
}

struct plugin_t * plugin;

void run(void)
{
    struct youtube_t * youtube;

    youtube = malloc(sizeof (struct youtube_t));
    memset(youtube, 0, sizeof (struct youtube_t));
    proc_info_youtube(plugin->irc, youtube);

    if (youtube->valid)
    {
        char response[512];
        sprintf(response, "PRIVMSG %s :[%s] - [rating: %.2f/5, viewed: %s]", 
                plugin->irc->from,
                youtube->title, youtube->rating, youtube->view_count);
        plugin->send_message(plugin->irc, response);
    }
    free(youtube);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "youtube";
    plugin->is_daemon  = 0;
    plugin->is_command = 0;
    plugin->is_grep    = 1;

    return plugin;
}

