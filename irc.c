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

struct network_t * network_g;

/*
:sdlkfjslkdjf MODE sdlkfjslkdjf :+i
:neuro_sys!~neuro_sys@unaffiliated/neurosys/x-283974 PRIVMSG sdlkfjslkdjf :foo
JOIN #test
:sdlkfjslkdjf!~sdflkjsdf@188.58.66.173 JOIN #test
*/

void irc_process_srv_message(struct srv_message_t * srv_msg, char * message)
{
  if (!strcmp ("PRIVMSG", srv_msg->command)) {
    char ** tokens = g_strsplit(srv_msg->prefix, "!", 2);
    char *  nick   = tokens[0];

    if (!g_ascii_strcasecmp ("neuro_sys", nick)) {
      char ** tokens = g_strsplit_set(message, " ", 2);
      if (!g_ascii_strcasecmp("join", tokens[0])) {
        char msg[20];

        sprintf(msg, "JOIN %s\r\n", tokens[1]);
        network_send_message(network_g, msg);
      } else if (!g_ascii_strcasecmp("part", tokens[0])) {
        char msg[20];

        sprintf(msg, "PART %s\r\n", tokens[1]);
        network_send_message(network_g, msg); 
      }
    }
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

  irc_process_srv_message(&srv_msg, tokens[2]);

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
  network_g = network;
  if (line[0] == ':') { /* irc server message */
    printf("%s", line);
    irc_process_server(line);
  } else {
    irc_process_other(network, line);   
  }
}

