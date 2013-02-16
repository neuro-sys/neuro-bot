#ifndef __NETWORK_H
#define __NETWORK_H
#include <glib.h>

extern struct network_t;

extern struct network_t * network_connect         (char * host_name, int port);
extern GIOChannel *       network_get_giochannel  (struct network_t * network);
#endif
