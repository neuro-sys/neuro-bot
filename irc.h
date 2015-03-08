#ifndef __IRC_H
#define __IRC_H

#include "irc_parser.h"
#include "socket.h"

#define MAX_IRC_MSG 510

struct irc_t {
    struct message_t    message;               /* Holds the server response as parsed. */
    char                from[100];             /* A shortcut for by whom the msg is received. */
    char                ** channels_v;         /* Maintains the vector of channels joined. */
    struct socket_t     socket;
    char                * nickname;
    char                * password;
    char                * admin;
    char                ** channels_ajoin_v;
};

void    irc_process_line        (struct irc_t * irc, const char * line);

void    irc_set_nick            (struct irc_t * irc, char * nickname);
void    irc_set_user            (struct irc_t * irc, char * user, char * host);
void    irc_identify_to_auth    (struct irc_t * irc, char * password);
void    irc_join_channel        (struct irc_t * irc, char * channel);
int     irc_run                 (struct irc_t * irc);

#endif
