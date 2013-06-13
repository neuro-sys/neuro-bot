#ifndef __IRC_H
#define __IRC_H

#include "session.h"

#define MAX_IRC_MSG 510


/** 
 *( nickname [ [ "!" user ] "@" host ] )
 *
 */
struct nickname_t {
    char nickname[100];
    char user[100];
    char host[100];
};

struct prefix_t {
    char servername[100];
    struct nickname_t nickname;
};

struct params_t {
    char list[14][50];
};

struct message_t {
    struct prefix_t prefix;
    char command[10];
    struct params_t params;
    char trailing[500];
};

struct irc_t {
    struct message_t     message;
    char                 request[MAX_IRC_MSG];
    char                 response[MAX_IRC_MSG];
    char                 nick_to_msg[100];
    char                 from[100];
    char                 ** channels;
    int                  channels_siz;
    struct session_t     * session;
};

extern void irc_process_line(struct irc_t * irc, char * line);

#endif
