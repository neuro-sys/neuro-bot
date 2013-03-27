#ifndef __SESSION_H
#define __SESSION_H

#include "network.h"

struct session_t {
    void                (* run) (struct session_t * session);
    struct network_t    network;
    char                * nickname;
    char                * password;
};

extern void                 session_create                  (struct session_t * session);
extern void                 session_destroy                 (struct session_t * session);
#endif
