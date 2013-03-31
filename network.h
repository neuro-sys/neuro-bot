#ifndef __NETWORK_H
#define __NETWORK_H

struct network_t {
    char        * host_name;
    int         port;
    int         sockfd;
};

extern void     network_connect         (struct network_t * network);
extern int      network_read_line       (struct network_t * network, char * buf);
extern void     network_send_message    (struct network_t * network, char * message);

#endif
