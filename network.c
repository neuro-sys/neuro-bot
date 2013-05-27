#include "global.h"
#include "network.h"
#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <unistd.h>
#else
#include <io.h>
#endif
#include <sys/types.h>

int errno;

/* K&R 8.2 */
int getchar_buf(int sockfd)
{
    static char buf[1024];
    static char * bufp;
    static int n;

    if (n == 0) {
        if ( (n = recv(sockfd, buf, 1024, 0)) <= 0 ) { 
            errno = -1;
            return '\n';
        }
        bufp = buf;
    }
    return (--n >= 0) ? (unsigned char) *bufp++ : EOF;
}

void network_connect(struct network_t * network)
{
    int sockfd;

    sockfd = t_connect(network->host_name, network->port);
    network->sockfd = sockfd;

    errno = 0;
}

int network_read_line(struct network_t * network, char * buf)
{
    int ret;

    while( (ret = getchar_buf(network->sockfd)) != '\n' )
        *buf++ = ret;

    if (errno < 0)
        return -1;

    *buf = '\0';

    return 0;
}

void network_send_message(struct network_t * network, char * message)
{
    int len;

    /* Make sure the line ends with cr-lf */
    len = strlen(message);
    if (message[len-2] == '\r' && message[len-1] != '\n')
        strcat(message, "\r\n");

    len = send(network->sockfd, message, strlen(message), 0);
}

