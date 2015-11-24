#ifndef __IRC_H
#define __IRC_H

#include "irc_parser.h"
#include "channel.h"
#include "socket.h"
#include "queue.h"

LIST_HEAD(ajoin_channel_list_t, ajoin_channel_t);

struct ajoin_channel_t {
    char * channel_name;
    LIST_ENTRY(ajoin_channel_t) list;
};

struct irc_t {
    struct message_t    message;               /* Holds the server response as parsed. */
    char                from[100];             /* A shortcut for by whom the msg is received. */
    struct channel_list_t channel_list_head;
    socket_t            sockfd;
    char                * hostname;
    char                * port;
    char                * nickname;
    char                * password;
    char                * admin;

    struct ajoin_channel_list_t ajoin_channels_head;
};

int     irc_run                 (struct irc_t * irc);
void    irc_free                (struct irc_t * irc);

#endif
