#ifndef __IRC_H
#define __IRC_H

#include "irc_parser.h"
#include "channel.h"
#include "socket.h"

#define MAX_IRC_MSG 510


struct irc_t {
    struct message_t    message;               /* Holds the server response as parsed. */
    char                from[100];             /* A shortcut for by whom the msg is received. */
    struct channel_t    ** channels_v;
    int                 sockfd;
    char                * hostname;
    char                * port;
    char                * nickname;
    char                * password;
    char                * admin;
    char                ** channels_ajoin_v;
};

int     irc_run                 (struct irc_t * irc);
void    irc_free                (struct irc_t * irc);

#endif
