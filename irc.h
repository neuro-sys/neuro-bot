#ifndef __IRC_H
#define __IRC_H

#include "session.h"
#include "irc_parser.h"

#define MAX_IRC_MSG 510

struct irc_t {
    struct message_t     message;
    char                 response[MAX_IRC_MSG];
    char                 from[100];
    char                 ** channels;
    int                  channels_siz;
    struct session_t     * session;
};

extern void irc_process_line(struct irc_t * irc, const char * line);

extern void irc_set_nick(char * nickname, char * buffer);
extern void irc_set_user(char * user, char * host, char * buffer);
extern void irc_identify_to_auth(char * password, char * buffer);
extern void irc_join_channel(char * channel, char * buffer);

#endif
