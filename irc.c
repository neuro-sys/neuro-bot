#include "irc.h"

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

static
void proc_cmd_admin(char * request, char * response)
{
  char ** tokens = g_strsplit_set(request, " ", 2);
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

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
  GRegex * regex;
  GMatchInfo * match_info;
  struct irc_t * irc = (struct irc_t *) userp;
  char title[255];

  regex = g_regex_new("(?i)<TITLE>(.+?)</TITLE>", 0, 0, NULL);
  g_regex_match(regex, (char *) buffer, 0, &match_info);
  if (g_match_info_matches(match_info)) {
    char * t = g_match_info_fetch(match_info, 0);
    strncpy(title, t, 255);
    g_free(t);
    sprintf(irc->response, "PRIVMSG %s :%s\r\n", irc->from, title);
    g_match_info_next (match_info, NULL);
  }
  g_match_info_free(match_info);
  g_regex_unref(regex);

  return size * nmemb;
}

static
void proc_title(struct irc_t * irc)
{
	CURL *   curl;
	CURLcode res;
  char title[256];
  char response[256];
  size_t n;

  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, irc->request);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, irc);
  curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  
}

static
void proc_privmsg(struct irc_t * irc)
{
  char ** tokens = g_strsplit(irc->srv_msg.prefix, "!", 2);
  strcpy(irc->from, irc->srv_msg.params);
  strcpy(irc->nick_to_msg, tokens[0]);

  /* Admin commands */
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
  char ** tokens, ** srv_msg_tokens;
  
  tokens         = g_strsplit(line, ":", 3);
  srv_msg_tokens = g_strsplit(tokens[1], " ", 3);

  strcpy(irc->srv_msg.prefix, srv_msg_tokens[0]);
  strcpy(irc->srv_msg.command, srv_msg_tokens[1]);
  if (srv_msg_tokens[2] != NULL) /* Is optional as per RFC */
    strcpy(irc->srv_msg.params, srv_msg_tokens[2]);
 
  if (tokens[2] != NULL)  /* Is optional as per RFC */
    strcpy(irc->request, tokens[2]);

  g_strfreev (tokens);
  g_strfreev (srv_msg_tokens);
}

static
void irc_process_other(struct irc_t * irc, char * line)
{
  char ** tokens = g_strsplit(line, ":", 2);

  if (!strncmp("PING", tokens[0], 4)) {
    strcpy(irc->srv_msg.command, tokens[0]);
    strcpy(irc->request, tokens[1]);
  }

  g_strfreev(tokens);
}

/* message    =  [ ":" prefix SPACE ] command [ params ] crlf */
void irc_process_line(struct irc_t * irc, char * line)
{  
  printf("%s", line);
  if (line[0] == ':') {
    proc_msg_prefix(irc, line);
  } else {
    irc_process_other(irc, line);   
  }

  irc_proc(irc);
}

