#ifndef __SESSION_H
#define __SESSION_H

#include "network.h"

struct session_t {
    struct network_t    network;
    char                * nickname;
    char                * password;
    char                * admin;
    char                * channels_ajoin[50];
};

extern void                 session_create                  (struct session_t * session);
extern void                 session_destroy                 (struct session_t * session);
extern void                 session_run                     (struct session_t * session);
#endif
