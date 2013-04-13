#include "neurobotapi.h"

#include "json.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char * keywords[MAX_KEYWORDS] = { "youtube.com", "youtu.be" };

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
