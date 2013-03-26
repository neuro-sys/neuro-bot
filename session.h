#ifndef __SESSION_H
#define __SESSION_H

#include "network.h"
#include "channel.h"

struct session_t {
    void                (* run) (struct session_t * session, char * nick, char * pass);
    struct network_t    network;
    char                * nickname;
};

extern void                 session_create                  (struct session_t * session, char * host, int port);
extern void                 session_destroy                 (struct session_t * session);
#endif
