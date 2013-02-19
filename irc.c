#include "irc.h"
#include "global.h"

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

static
void proc_cmd_admin(char * request, char * response)
{
  char ** tokens;
  
  g_debug("%zu\t%s\t\t%s", __LINE__, __FILE__, __func__);
  tokens = g_strsplit_set(request, " ", 2);
  if (!g_ascii_strcasecmp("join", tokens[0])) {
    sprintf(response, "JOIN %s\r\n", tokens[1]);
  } else if (!g_ascii_strcasecmp("part", tokens[0])) {
    sprintf(response, "PART %s\r\n", tokens[1]);
  } 
}

static
void proc_cmd(char * request, char * response)
{
  
}

struct mem_block_t {
  char * memory;
  size_t size;
};
 
static size_t
WriteMemoryCallback(void * contents, size_t size, size_t nmemb, void * userp)
{
  size_t realsize; 
  struct mem_block_t * mem;

  realsize = size * nmemb;
  mem = (struct mem_block_t *) userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL) {
    printf("not enough memory (realloc returned NULL)\n");
    exit(-1);
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  g_debug("Memory chunking: %zu bytes.", mem->size);
  if (mem->size > 16*1024) {
    g_debug("Passed chunking limit\n");
    return -1;
  }
  return realsize;
}


int parse_title(char * dest, char * src)
{
  GRegex *      regex;
  GMatchInfo *  match_info;

  g_debug("%zu\t%s\t\t%s", __LINE__, __FILE__, __func__);
  regex = g_regex_new("(?i)<TITLE>(.+?)</TITLE>", 0, 0, NULL);
  g_regex_match(regex, src, 0, &match_info);
  if (g_match_info_matches(match_info)) {
    char * t = g_match_info_fetch(match_info, 0);
    strncpy(dest, t, 255);
    g_free(t);
    g_match_info_free(match_info);
    g_regex_unref(regex);

    return 1;
  }
  g_match_info_free(match_info);
  g_regex_unref(regex);

  return -1;
}

struct youtube_t {
  double          rating;
  char            view_count[20];
};

static
void parse_json_youtube(char * data, struct youtube_t * youtube)
{
  json_t *      root;
  json_error_t  error;
  int i;
  json_t * entry, * rating, * statistics;

  g_debug("%zu\t%s\t\t%s", __LINE__, __FILE__, __func__);
  root        = json_loads(data, JSON_DECODE_ANY | JSON_DISABLE_EOF_CHECK , &error);
  entry       = json_object_get(root, "entry");
  rating      = json_object_get(entry, "gd$rating");
  statistics  = json_object_get(entry, "yt$statistics"); 
  json_unpack(json_object_get(rating, "average"), "F", &youtube->rating);
  strcpy(youtube->view_count, json_string_value(json_object_get(statistics, "viewCount")));

  json_decref(root);
}

static
char * fill_memory_url(char * url)
{
  CURL * curl;
  struct mem_block_t chunk;

  g_debug("%zu\t%s\t\t%s", __LINE__, __FILE__, __func__);
  chunk.memory = NULL;
  chunk.size   = 0;

  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL );
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
  //curl_easy_setopt(curl, CURLOPT_HEADER, 1);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
  curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  return chunk.memory;
}


static
void proc_info_youtube(struct irc_t * irc, struct youtube_t * youtube)
{
  CURL *           curl;
  GRegex *         regex;
  GMatchInfo *     match_info;
  char             url_path[512];
  char *           content;

  g_debug("%zu\t%s\t\t%s", __LINE__, __FILE__, __func__);
  if (g_strrstr(irc->request, "watch"))
    regex = g_regex_new("[^?(v=)]+$", 0, 0, NULL);
  else 
    regex = g_regex_new("[^/]+$", 0, 0, NULL);

  g_regex_match(regex, irc->request, 0, &match_info);
  if (g_match_info_matches(match_info)) {
    char * match = g_match_info_fetch(match_info, 0);
    sprintf(url_path, "http://gdata.youtube.com/feeds/api/videos/%s?alt=json&ver=2", match);
    g_free(match);
  }

  g_regex_unref(regex);

  content = fill_memory_url(url_path);
  parse_json_youtube(content, youtube);
  free(content);
}

static int
validate_http(char * line)
{
  GRegex * regex;
  GMatchInfo * match_info;
  char * match;

  g_debug("%zu\t%s\t\t%s", __LINE__, __FILE__, __func__);
  regex = g_regex_new("http:\\/\/\\S*", 0, 0, NULL);
  g_regex_match(regex, line, 0, &match_info);

  if (!g_match_info_matches(match_info))
    return -1;

  match = g_match_info_fetch(match_info, 0);
  strcpy(line, match);
  g_free(match);
  g_regex_unref(regex);

  return 1;
}


static
void proc_title(struct irc_t * irc)
{
  char title[256];
  char * content = NULL;
  struct youtube_t youtube;

  g_debug("%zu\t%s\t\t%s", __LINE__, __FILE__, __func__);
  if (validate_http(irc->request) < 0 )
    return;

  if (g_strrstr(irc->request, "youtu")) {
    proc_info_youtube(irc, &youtube);
  } 

  content = fill_memory_url(irc->request);
  
  if (!content) return;
  
  if ( parse_title(title, content) > 0 ) {
    sprintf(irc->response, "PRIVMSG %s :%s [rating: %.2f, viewed: %s]\r\n", irc->from, title, youtube.rating, youtube.view_count);
  }

  free(content);
}

static
void proc_privmsg(struct irc_t * irc)
{
  char ** tokens;

  g_debug("%zu\t%s\t\t%s", __LINE__, __FILE__, __func__);
  tokens = g_strsplit(irc->srv_msg.prefix, "!", 2);
  strcpy(irc->from, irc->srv_msg.params);
  strcpy(irc->nick_to_msg, tokens[0]);

  if (irc->request[0] == '.') {
    proc_cmd(irc->request, irc->response);
    sprintf(irc->response, "PRIVMSG %s :Bakariz, %s.\r\n", irc->from, irc->nick_to_msg);
  } else if (g_strrstr(irc->request, "http:")) {
    proc_title(irc);
  } else if (!g_ascii_strcasecmp ("neuro_sys", irc->nick_to_msg)) {
    proc_cmd_admin(irc->request, irc->response);
  } 

  g_strfreev(tokens);
}

static
void irc_proc(struct irc_t * irc)
{
  g_debug("%zu\t%s\t\t%s", __LINE__, __FILE__, __func__);
  if (!strncmp("PRIVMSG", irc->srv_msg.command, 7)) {
    proc_privmsg(irc);
  } else if (!strncmp("PING", irc->srv_msg.command, 4)) {
    sprintf(irc->response, "PONG %s\r\n", irc->request);
  }
 printf("%s", irc->response);
}

/*     message    =  [ ":" prefix SPACE ] command [ params ] crlf */
static
void proc_msg_prefix(struct irc_t * irc, char * line)
{
  char ** tokens;
  
  tokens = g_strsplit(line, " ", 4); 

  strcpy(irc->srv_msg.prefix, tokens[0]+1); /* skip ':' from the prefix */
  strcpy(irc->srv_msg.command, tokens[1]);
  if (tokens[2][0] != ':') { /* if the 3rd token does not start with a ':', there are params */
    strcpy(irc->srv_msg.params, tokens[2]); /* this copies only the first param */
    if (tokens[3] != NULL) /* the rest is optional */
      strcpy(irc->request, tokens[3]+1);
  } else {
    strcpy(irc->request, tokens[2]+1);
  }

  g_strfreev (tokens);
}

static
void irc_process_other(struct irc_t * irc, char * line)
{
  char ** tokens;

  g_debug("%zu\t%s\t\t%s", __LINE__, __FILE__, __func__);
  tokens = g_strsplit(line, ":", 2);
  if (!strncmp("PING", tokens[0], 4)) {
    strcpy(irc->srv_msg.command, tokens[0]);
    strcpy(irc->request, tokens[1]);
  }

  g_strfreev(tokens);
}

/* message    =  [ ":" prefix SPACE ] command [ params ] crlf */
void irc_process_line(struct irc_t * irc, char * line)
{  
  g_debug("%zu\t%s\t\t%s", __LINE__, __FILE__, __func__);
  printf("%s", line);
  if (line[0] == ':') {
    proc_msg_prefix(irc, line);
  } else {
    irc_process_other(irc, line);   
  }

  irc_proc(irc);
}

