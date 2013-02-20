#include "irc.h"
#include "global.h"
#include "curl_wrap.h"

#include <string.h>
#include <stdlib.h>
#include <jansson.h>
#include <curl/curl.h>
#include <glib.h>
#include <stdio.h>

struct youtube_t {
  double          rating;
  char            view_count[20];
  char            title[100];
  int             valid;
};

static
void parse_json_youtube(char * data, struct youtube_t * youtube)
{
  json_t *      root;
  json_error_t  error;
  json_t * entry, * rating, * statistics, * title;

  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  root        = json_loads(data, JSON_DECODE_ANY | JSON_DISABLE_EOF_CHECK , &error);
  if (!root)
    return;
  entry       = json_object_get(root, "entry");
  rating      = json_object_get(entry, "gd$rating");
  statistics  = json_object_get(entry, "yt$statistics"); 
  title       = json_object_get(entry, "title");
  json_unpack(json_object_get(rating, "average"), "F", &youtube->rating);
  strcpy(youtube->view_count, json_string_value(json_object_get(statistics, "viewCount")));
  strcpy(youtube->title, json_string_value(json_object_get(title, "$t")));

  json_decref(root);
  youtube->valid = 1;
}

static
void proc_info_youtube(struct irc_t * irc, struct youtube_t * youtube)
{
  GRegex *         regex;
  GMatchInfo *     match_info;
  char             url_path[512];
  char *           content;

  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  regex = g_regex_new("(?<=v=)[a-zA-Z0-9-]+(?=&)|(?<=v\\/)[^&\\n]+|(?<=v=)[^&\\n]+|(?<=youtu.be/)[^&\\n]+", 0, 0, NULL);

  g_regex_match(regex, irc->request, 0, &match_info);
  if (g_match_info_matches(match_info)) {
    char * match = g_match_info_fetch(match_info, 0);
    char nasty_escape[20];

    strcpy(nasty_escape, match);
    nasty_escape[11] = '\0';
    sprintf(url_path, "http://gdata.youtube.com/feeds/api/videos/%s?alt=json&ver=2", nasty_escape);
    g_free(match);

    content = curl_perform(url_path);
    parse_json_youtube(content, youtube);
    free(content);
  } 

  g_regex_unref(regex);

  
}


void proc_youtube(struct irc_t * irc)
{
  struct youtube_t * youtube;

  youtube = malloc(sizeof (struct youtube_t));
  memset(youtube, 0, sizeof (struct youtube_t));
  proc_info_youtube(irc, youtube);

  if (youtube->valid)
    sprintf(irc->response, "PRIVMSG %s :[%s] - [rating: %f, viewed: %s\r\n", 
                                irc->from, youtube->title, youtube->rating, youtube->view_count);
  free(youtube);
}
