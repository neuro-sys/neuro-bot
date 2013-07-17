#ifndef __IRC_H
#define __IRC_H

#include "session.h"
#include "irc_parser.h"

#define MAX_IRC_MSG 510

struct irc_t {
    struct message_t     message;               /* Holds the server response as parsed. */
    struct session_t     * session;             /* A connection session. */
    char                 response[MAX_IRC_MSG]; /* Bot's response to the server. */
    char                 from[100];             /* A shortcut for by whom the msg is received. */
    char                 ** channels;           /* Maintains the vector of channels joined. */
    int                  channels_siz;
};

extern void irc_process_line(struct irc_t * irc, const char * line);

extern void irc_set_nick(char * nickname, char * buffer);
extern void irc_set_user(char * user, char * host, char * buffer);
extern void irc_identify_to_auth(char * password, char * buffer);
extern void irc_join_channel(char * channel, char * buffer);

#endif
