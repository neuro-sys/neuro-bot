#ifndef __IRC_H
#define __IRC_H

#include "session.h"
#include "irc_parser.h"

#define MAX_IRC_MSG 510

struct irc_t {
    struct message_t     message;               /* Holds the server response as parsed. */
    struct session_t     * session;             /* A connection session. */
    char                 from[100];             /* A shortcut for by whom the msg is received. */
    char                 ** channels_v;           /* Maintains the vector of channels joined. */
};

extern void irc_process_line(struct irc_t * irc, const char * line);

extern void irc_set_nick(struct irc_t * irc, char * nickname, char * buffer);
extern void irc_set_user(struct irc_t * irc, char * user, char * host, char * buffer);
extern void irc_identify_to_auth(struct irc_t * irc, char * password, char * buffer);
extern void irc_join_channel(struct irc_t * irc, char * channel, char * buffer);

#endif
