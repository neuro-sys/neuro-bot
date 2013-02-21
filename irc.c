#include "irc.h"
#include "global.h"
#include "py_wrap.h"

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "modules/mod_title.h"
#include "modules/mod_youtube.h"
#include "modules/mod_time.h"


/*
  Every function name is prefixed with either a `irc_proc' or `irc_parse'.
  
    irc_parse_* family of functions are for parsing the raw irc massages 
  into a struct srv_msg_t type.

    irc_proc_* family of functions are for processing the struct srv_msg_t
  type previously obtained by irc_parse_*, and responding the server with
  the appropriate reply.

    irc_proc_cmd_* family of functions are for processing the 
  srv_msg_t.command message (e.g. PRIVMSG, PING, CTCP). Depending on the srv
  command, irc_proc_cmd_{privmsg,ping,ctcp} functions are called respectively.

*/

static
void irc_proc_cmd_privmsg_user_cmd_admin(struct irc_t * irc)
{
  char ** tokens;
  
  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  tokens = g_strsplit_set(irc->request, " ", 2);
  if        (!g_ascii_strcasecmp(".join", tokens[0])) {
    sprintf(irc->response, "JOIN %s\r\n", tokens[1]);
  } else if (!g_ascii_strcasecmp(".part", tokens[0])) {
    sprintf(irc->response, "PART %s\r\n", tokens[1]);
  } 

  g_strfreev(tokens);
}

static
void irc_proc_cmd_privmsg_user_cmd(struct irc_t * irc)
{
  char ** tokens;

  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  tokens = g_strsplit_set(irc->request, " ", 2);

  if (!strncmp(".time", tokens[0], strlen(".time"))) {
    mod_cmd_time(irc);
  } 
  
  if (!strncmp(".test", tokens[0], strlen(".test"))) {
    char * ret = py_call_module("mod_test");
    sprintf(irc->response, "PRIVMSG %s :%s\r\n", irc->from, ret);
    free(ret);
  } 

  if (!strncmp(irc->admin, irc->nick_to_msg, strlen(irc->admin))) {
    irc_proc_cmd_privmsg_user_cmd_admin(irc);
  }

  g_strfreev(tokens);
}

static
void irc_proc_cmd_privmsg(struct irc_t * irc)
{
  char ** tokens;

  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  tokens = g_strsplit(irc->srv_msg.prefix, "!", 2);
  strcpy(irc->from, irc->srv_msg.params);
  strcpy(irc->nick_to_msg, tokens[0]);

  if        (irc->request[0] == '.') {
    irc_proc_cmd_privmsg_user_cmd(irc);
  } else if (g_strrstr(irc->request, "http:") || g_strrstr(irc->request, "https:")) {
    if (g_strrstr(irc->request, "youtube.com") || g_strrstr(irc->request, "youtu.be"))
      mod_line_youtube(irc); 
    else  
      mod_line_title(irc);
  }  

  g_strfreev(tokens);
}

static
void irc_proc_cmd(struct irc_t * irc)
{
  char * cmd;

  cmd = irc->srv_msg.command;
  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  if        (!strncmp("PRIVMSG", cmd, strlen("PRIVMSG"))) {
    irc_proc_cmd_privmsg(irc);
  } else if (!strncmp("PING", cmd, strlen("PING"))) {
    sprintf(irc->response, "PONG %s\r\n", irc->request);
  }
 printf("%s", irc->response);
}

/*     message    =  [ ":" prefix SPACE ] command [ params ] crlf */
static
void irc_parse_prefix(struct irc_t * irc, char * line)
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
void irc_parse_other(struct irc_t * irc, char * line)
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
    irc_parse_prefix(irc, line);
  } else {
    irc_parse_other(irc, line);   
  }

  irc_proc_cmd(irc);
}

