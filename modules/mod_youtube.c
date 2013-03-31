#include "irc.h"
#include "global.h"
#include "curl_wrap.h"

#include <string.h>
#include <stdlib.h>
#include <jansson.h>
#include <curl/curl.h>
#include <stdio.h>

struct youtube_t {
    double          rating;
    char            view_count[20];
    char            title[100];
    int             valid;
};

static void parse_json_youtube(char * data, struct youtube_t * youtube)
{
    json_t        * root;
    json_error_t  error;
    json_t        * entry, * rating, * statistics, * title, * average, * viewCount, * title_t;
    const char    * temp;

    root        = json_loads(data, JSON_DECODE_ANY | JSON_DISABLE_EOF_CHECK , &error);
    if (!root)
        return;
    entry       = json_object_get(root, "entry");
    rating      = json_object_get(entry, "gd$rating");
    statistics  = json_object_get(entry, "yt$statistics"); 
    title       = json_object_get(entry, "title");
    average     = json_object_get(rating, "average");

    json_unpack(average, "F", &youtube->rating);
    json_decref(average);
    json_decref(rating);

    if (statistics != NULL) 
    {
        viewCount = json_object_get(statistics, "viewCount");
        temp = strdup(json_string_value(viewCount));
        strcpy(youtube->view_count, temp);
    }
    else 
    {
        strcpy(youtube->view_count, "N/A");
    }

    title_t = json_object_get(title, "$t");
    temp = json_string_value(title_t); 
    strcpy(youtube->title, temp);

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


char * mod_youtube(struct irc_t * irc)
{
    char ret[510];
    struct youtube_t * youtube;

    youtube = malloc(sizeof (struct youtube_t));
    memset(youtube, 0, sizeof (struct youtube_t));
    proc_info_youtube(irc, youtube);

    if (youtube->valid)
    {
        sprintf(ret, "[%s] - [rating: %.2f/5, viewed: %s]", 
                youtube->title, youtube->rating, youtube->view_count);
    }
    free(youtube);

    return strdup(ret);
}
