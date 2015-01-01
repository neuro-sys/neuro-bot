#include "socket.h"

#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <sys/types.h>

#if defined(unix) || defined(__MACH__)
    #include <sys/socket.h>
    #include <netdb.h>
    #include <unistd.h>
#elif defined(_WIN32)
    #include <io.h>
    #include <WS2tcpip.h>
#endif

int errno;

#if defined(_WIN32)
static void initWinSock(void)
{
    WSADATA wsa_data;

    if (WSAStartup(MAKEWORD(2, 1), &wsa_data) != 0)
        return;
}
#endif

int socket_internal_connect(char *host, char *port)
{
    struct addrinfo hints, *res;
    int sockfd;
#if defined(_WIN32)
    initWinSock();
#endif
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &res) != 0) {
#if defined(_WIN32)
        MessageBox(NULL, "getaddrinfo failed.", "Error", MB_OK);
#endif
        debug("getaddrinfo failed.\n");
        return -1;
    }
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
#if defined(_WIN32)
        MessageBox(NULL, "socket failed.", "Error", MB_OK);
#endif
        debug("socket call failed.\n");
        return -1;
    }
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
#if defined(_WIN32)
        MessageBox(NULL, "connect failed.", "Error", MB_OK);
#endif
        debug("connect call failed.\n");
        return -1;
    }

    freeaddrinfo(res);

    return sockfd;
}

/* K&R 8.2 */
int getchar_fd(int sockfd)
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

int socket_close(struct socket_t * socket)
{
    return close(socket->sockfd);
}

int socket_connect(struct socket_t * socket)
{
    int sockfd;

    sockfd = socket_internal_connect(socket->host_name, socket->port);
    socket->sockfd = sockfd;

    errno = 0;

    return sockfd;
}

int socket_read_line(struct socket_t * socket, char * buf)
{
    int ret;

    while( (ret = getchar_fd(socket->sockfd)) != '\n' )
        *buf++ = ret;

    if (errno < 0)
        return -1;

    *buf = '\0';

    return 0;
}

void socket_send_message(struct socket_t * socket, char * message)
{
    int len;

    /* Make sure the line ends with cr-lf */
    len = strlen(message);
    if (message[len-2] != '\r' && message[len-1] != '\n')
        strcat(message, "\r\n");

    len = send(socket->sockfd, message, strlen(message), 0);
}

