#ifndef __NETWORK_H
#define __NETWORK_H
#include <glib.h>

extern struct network_t;

extern struct network_t * network_connect         (char * host_name, int port);
extern GIOChannel *       network_get_giochannel  (struct network_t * network);
extern int                network_read_line       (struct network_t * network, char ** buf);
extern void               network_auth            (struct network_t * network, char * nick, char * user, char * name);
#endif
