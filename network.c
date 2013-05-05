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


/* K&R 8.2 */
int getchar_buf(int sockfd)
{
    static char buf[1024];
    static char * bufp;
    static unsigned int n;

    if (n == 0) {
        n = recv(sockfd, buf, 1024, 0);
        bufp = buf;
    }
    return (--n >= 0) ? (unsigned char) *bufp++ : EOF;
}

void network_connect(struct network_t * network)
{
    int sockfd;

    sockfd = t_connect(network->host_name, network->port);
    network->sockfd = sockfd;
}

void network_read_line(struct network_t * network, char * buf)
{
    int ret;

    while( (ret = getchar_buf(network->sockfd)) != '\n' )
        *buf++ = ret;

    *buf = '\0';
}

void network_send_message(struct network_t * network, char * message)
{
    int len;

    /* Make sure the line ends with cr-lf */
    len = strlen(message);
    if (message[len-2] == '\r' && message[len-1] != '\n')
        strcat(message, "\r\n");

    send(network->sockfd, message, strlen(message), 0);
}

