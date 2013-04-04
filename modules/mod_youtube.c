#include "neurobotapi.h"

#include "json.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct youtube_t {
    double          rating;
    char            view_count[20];
    char            title[100];
    int             valid;
};

static void parse_json_youtube(char * data, struct youtube_t * youtube)
{
    json_value * root, * entry, * title, * rating, * viewcount;

    root = json_parse(data);

    entry = root->u.object.values[2].value;
    if (!entry) return;

    title = entry->u.object.values[8].value->u.object.values[0].value;
    if (!title) return;
    strcpy(youtube->title, title->u.string.ptr);

    rating = entry->u.object.values[14].value->u.object.values[0].value;
    if (!rating) return;
    youtube->rating = rating->u.dbl;

    viewcount = entry->u.object.values[15].value->u.object.values[1].value;
    if (!viewcount) return;
    strcpy(youtube->view_count, viewcount->u.string.ptr);

    youtube->valid = 1;
}

static void proc_info_youtube(struct irc_t * irc, struct youtube_t * youtube)
{
    char        url_path[512];
    char        * content;
    char *t;

    t = strstr(irc->request, "v=");
    if (t) t +=2;
    t = strtok(t, " &\r\n\\");

    sprintf(url_path, "http://gdata.youtube.com/feeds/api/videos/%s?alt=json&ver=2", t);

    content = curl_perform(url_path);
    parse_json_youtube(content, youtube);
    free(content);
}

#ifdef _WIN32
__declspec(dllexport)
#endif
char * mod_youtube(struct irc_t * irc, char * reply_msg)
{
    struct youtube_t * youtube;

    youtube = malloc(sizeof (struct youtube_t));
    memset(youtube, 0, sizeof (struct youtube_t));
    proc_info_youtube(irc, youtube);

    if (youtube->valid)
    {
        sprintf(reply_msg, "[%s] - [rating: %.2f/5, viewed: %s]", 
                youtube->title, youtube->rating, youtube->view_count);
    }
    free(youtube);

}
