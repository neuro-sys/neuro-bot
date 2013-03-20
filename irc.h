#ifndef __IRC_H
#define __IRC_H

#include "session.h"

#define MAX_MSG 510
#define MAX_LEN 100

struct srv_message_t {
    char prefix[MAX_LEN];
    char command[MAX_LEN];
    char params[MAX_LEN];
};

struct irc_t {
    struct srv_message_t srv_msg;
    char                 request[MAX_MSG];
    char                 response[MAX_MSG];
    char                 nick_to_msg[MAX_LEN];
    char                 from[MAX_LEN];
    struct session_t     * session;
    char                 * admin;
};

extern void irc_process_line(struct irc_t * irc, char * line);

#endif
