#include "irc.h"
#include "curl_wrap.h"
#include "global.h"

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int
validate_http(char * line)
{
  char * ret, * t, * p;

  ret = strstr(line, "http");

  t = (p = strchr(ret, ' '))  ? p 
    : (p = strchr(ret, '\r')) ? p
    : (p = strchr(ret, '\n')) ? p
    : 0;

  if (t) *t = '\0';

  memmove(line, ret, strlen(ret) + 1);
}


static int
validate_http2(char * line)
{
  GRegex      * regex;
  GMatchInfo  * match_info;
  char        * match;

  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  regex = g_regex_new("http:\\/\\/\\S*", 0, 0, NULL);
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
int parse_title(char * dest, char * src)
{
  GRegex     * regex;
  GMatchInfo * match_info;

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

void mod_line_title(struct irc_t * irc)
{
  char  title[256];
  char  * content = NULL;

  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  if (validate_http(irc->request) < 0 )
    return;

  content = curl_perform(irc->request);
  
  if (!content) return;
  
  if ( parse_title(title, content) > 0 ) {
    sprintf(irc->response, "PRIVMSG %s :%s\r\n", irc->from, title);
  }

  free(content);
}


