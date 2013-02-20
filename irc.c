#include "irc.h"
#include "global.h"

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "mod_title.h"
#include "mod_youtube.h"


static
void proc_cmd_admin(struct irc_t * irc)
{
  char ** tokens;
  
  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  tokens = g_strsplit_set(irc->request, " ", 2);
  if (!g_ascii_strcasecmp(".join", tokens[0])) {
    sprintf(irc->response, "JOIN %s\r\n", tokens[1]);
  } else if (!g_ascii_strcasecmp(".part", tokens[0])) {
    sprintf(irc->response, "PART %s\r\n", tokens[1]);
  } 
}

static
void proc_cmd(struct irc_t * irc)
{
  if (!g_strcasecmp("neuro_sys", irc->nick_to_msg)) {
    proc_cmd_admin(irc);
  }
}

static
void proc_privmsg(struct irc_t * irc)
{
  char ** tokens;

  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  tokens = g_strsplit(irc->srv_msg.prefix, "!", 2);
  strcpy(irc->from, irc->srv_msg.params);
  strcpy(irc->nick_to_msg, tokens[0]);

  if (irc->request[0] == '.') {
    proc_cmd(irc);
  } else if (g_strrstr(irc->request, "http:") || g_strrstr(irc->request, "https:")) {
    if (g_strrstr(irc->request, "youtube.com") || g_strrstr(irc->request, "youtu.be"))
      proc_youtube(irc); 
    else  
      proc_title(irc);
  }  

  g_strfreev(tokens);
}

static
void irc_proc(struct irc_t * irc)
{
  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
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

  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
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
  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  printf("%s", line);
  if (line[0] == ':') {
    proc_msg_prefix(irc, line);
  } else {
    irc_process_other(irc, line);   
  }

  irc_proc(irc);
}

