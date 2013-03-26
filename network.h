#ifndef __NETWORK_H
#define __NETWORK_H
#include <glib.h>

struct network_t {
    char        * host_name;
    int         port;
    int         sockfd;
    GIOChannel  * giochannel;
};

extern void               network_connect         (struct network_t * network, char * host_name, int port);
extern GIOChannel *       network_get_giochannel  (struct network_t * network);
extern int                network_read_line       (struct network_t * network, char ** buf);
extern void               network_auth            (struct network_t * network, char * nick, char * user, char * name);
extern void               network_send_message    (struct network_t * network, char * message);
#endif
