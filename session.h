#ifndef __SESSION_H
#define __SESSION_H

#include "network.h"
#include "channel.h"

struct session_t {
    void             (* run) (struct session_t * session, char * nick, char * pass);

    struct network_t  * network;
    GSList            * channel_list;
    char              * nickname;
};

extern struct session_t *   session_create                  (char * host, int port);
extern void                 session_destroy                 (struct session_t * session);
extern struct channel_t *   session_channel_remove_by_name  (struct session_t * session, char * name);
extern struct channel_t *   session_channel_find_by_name    (struct session_t * session, char * name);
extern void                 session_add_channel             (struct session_t * session, char * name);
extern void                 session_print_channels          (struct session_t * session);
extern struct network_t *   session_get_network             (struct session_t * session);
#endif
