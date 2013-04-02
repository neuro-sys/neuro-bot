#include "global.h"
#include "network.h"
#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <unistd.h>
#else
#include <io.h>
#endif
#include <sys/types.h>

/*
 * via ii@suckless project.
 */
static int read_line(int fd, size_t res_len, char *buf) {
    size_t i = 0;
    char c = 0;
    do {
        if(read(fd, &c, sizeof(char)) != sizeof(char))
            return -1;
        buf[i++] = c;
    }
    while(c != '\n' && i < res_len);
    buf[i - 1] = 0;         /* eliminates '\n' */
    return 0;
}

void network_connect(struct network_t * network)
{
    char port_str[10];
    int sockfd;

    sprintf(port_str, "%d", network->port);
    sockfd = t_connect(network->host_name, port_str);
    network->sockfd = sockfd;
}

int network_read_line(struct network_t * network, char * buf)
{
    struct timeval tv;
    fd_set readfs;

    tv.tv_sec = 120;
    tv.tv_usec = 0;

    FD_ZERO(&readfs);
    FD_SET(network->sockfd, &readfs);

    select(network->sockfd+1, &readfs, NULL, NULL, &tv);
    if (FD_ISSET(network->sockfd, &readfs))
        return read_line(network->sockfd, 1024, buf);

    return -1;
}

void network_send_message(struct network_t * network, char * message)
{
    send(network->sockfd, message, strlen(message), 0);
}

