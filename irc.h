#ifndef __IRC_H
#define __IRC_H

#include "session.h"

#define MAX_MSG 510

struct srv_message_t {
  char prefix[100];
  char command[20];
  char params[100];
};

struct irc_t {
  struct srv_message_t srv_msg;
  char                 request[MAX_MSG];
  char                 response[MAX_MSG];
  char                 nick_to_msg[30];
  char                 from[30];
  struct session_t *   session;
  char*                admin;
};

extern void irc_process_line(struct irc_t * irc, char * line);

#endif
