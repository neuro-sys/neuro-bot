#include "irc.h"
#include "network.h"

#include <glib.h>
#include <stdio.h>
#include <string.h>

struct srv_message_t {
  char * prefix;
  char * command;
  char * params;
};

/*
:sdlkfjslkdjf MODE sdlkfjslkdjf :+i
:neuro_sys!~neuro_sys@unaffiliated/neurosys/x-283974 PRIVMSG sdlkfjslkdjf :foo
JOIN #test
:sdlkfjslkdjf!~sdflkjsdf@188.58.66.173 JOIN #test
*/

void irc_process_srv_message(struct srv_message_t * srv_msg)
{
  if (!strcmp ("PRIVMSG", srv_msg->command)) {
    char ** tokens = g_strsplit(srv_msg->prefix, "!", 2);
    char *  nick   = tokens[0];
  }
}

void irc_process_server(char * line)
{
  struct srv_message_t srv_msg = { NULL, NULL, NULL };
  char ** tokens, ** srv_msg_tokens;
  
  tokens         = g_strsplit_set(line, ":", 3);
  srv_msg_tokens = g_strsplit_set(tokens[1], " ", 3);

  srv_msg.prefix  = srv_msg_tokens[0];
  srv_msg.command = srv_msg_tokens[1];
  srv_msg.params  = srv_msg_tokens[2];

  irc_process_srv_message(&srv_msg);

  g_strfreev (tokens);
}

void irc_process_other(struct network_t * network, char * line)
{
  char **tokens = g_strsplit(line, ":", 2);
  if (!strcmp("PING", tokens[0])) {
    char msg[255];
    sprintf(msg, "PONG %s\r\n", tokens[1]);
    network_send_message(network, msg);
  }
}

void irc_process_line(struct network_t * network, char * line)
{
  if (line[0] == ':') { /* irc server message */
    printf("%s", line);
    irc_process_server(line);
  } else {
    irc_process_other(network, line);   
  }
}

