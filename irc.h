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

#endif
