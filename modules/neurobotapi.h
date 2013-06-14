#ifndef __NEUROBOTAPI_H
#define __NEUROBOTAPI_H

#define MAX_IRC_MSG 510
#define MAX_KEYWORDS 10

#include "../irc_parser.h"

char * keywords[MAX_KEYWORDS];
int looper;

struct network_t {
    char        * host_name;
    int         port;
    int         sockfd;
};

struct session_t {
    struct network_t    network;
    char                * nickname;
    char                * password;
    char                * admin;
    char                * channels_ajoin[50];
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

void (*network_send_message) (struct network_t * network, char * message);

#endif
