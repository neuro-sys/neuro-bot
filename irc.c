#include "irc.h"
#include "network.h"

#include <glib.h>
#include <stdio.h>
#include <string.h>


static
void proc_admin_msg(char * request, char * response)
{
  char ** tokens = g_strsplit_set(request, " ", 2);
  if (!g_ascii_strcasecmp("join", tokens[0])) {
    sprintf(response, "JOIN %s\r\n", tokens[1]);
  } else if (!g_ascii_strcasecmp("part", tokens[0])) {
    sprintf(response, "PART %s\r\n", tokens[1]);
  } 
}

static
void proc_regular_msg(char * request, char * response)
{
  
}


static
void proc_privmsg(struct irc_t * irc)
{
  char ** tokens = g_strsplit(irc->srv_msg.prefix, "!", 2);
  char *  msgto  = irc->srv_msg.params;
  strcpy(irc->nick_to_msg, tokens[0]);

  /* Admin commands */
  if (irc->request[0] == '.') {
    proc_regular_msg(irc->request, irc->response);
    sprintf(irc->response, "PRIVMSG %s :Bakariz, %s.\r\n", msgto, irc->nick_to_msg);
  } else if (g_strrstr(irc->request, "http:")) {
    sprintf(irc->response, "PRIVMSG %s :Bir ara title'lara bakicam, %s.\r\n", msgto, irc->nick_to_msg);
  } else if (!g_ascii_strcasecmp ("neuro_sys", irc->nick_to_msg)) {
    proc_admin_msg(irc->request, irc->response);
  } 
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
  char **tokens = g_strsplit(line, ":", 2);

  if (!strncmp("PING", tokens[0], 4)) {
    strcpy(irc->srv_msg.command, tokens[0]);
    strcpy(irc->request, tokens[1]);
  }
}

/* message    =  [ ":" prefix SPACE ] command [ params ] crlf */
void irc_process_line(struct session_t * session, struct irc_t * irc, char * line)
{
  irc->session = session;
  
  printf("%s", line);
  if (line[0] == ':') {
    proc_msg_prefix(irc, line);
  } else {
    irc_process_other(irc, line);   
  }

  irc_proc(irc);
}

