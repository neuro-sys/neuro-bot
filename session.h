#ifndef __SESSION_H
#define __SESSION_H

#include "socket.h"

struct session_t {
    struct socket_t     socket;
    char                * nickname;
    char                * password;
    char                * admin;
    char                ** channels_ajoin_v;
};

extern int session_run (struct session_t * session);

#endif
