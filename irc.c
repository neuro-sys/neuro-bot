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

void irc_process_server(char * line)
{
  char **tokens = g_strsplit(line, ":", 3);
  struct srv_message_t srv_msg;
  char **srv_msg_tokens = g_strsplit(tokens[0], " ", 3);

  srv_msg.prefix = srv_msg_tokens[0];
  srv_msg.command = srv_msg_tokens[1];
  srv_msg.params = srv_msg_tokens[2];


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
    irc_process_server(line);
  } else {
    irc_process_other(network, line);   
  }
}

