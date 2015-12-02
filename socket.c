#include "socket.h"

#include <errno.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __WIN32__
    #include <winsock2.h>
    #include <WS2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netdb.h>
    #include <unistd.h>
#endif // __WIN32__

struct socket_t {
    char buf[1024];
    char * bufp;
    int n;

    int sockfd;
};

int socket_open (const char * path, int oflag)
{
#ifdef __WIN32__
    return _open(path, oflag);
#else
    return open(path, oflag);
#endif // __WIN32__
}

int socket_read (int handle, void * buffer, unsigned int nbyte)
{
#ifdef __WIN32__
    return _read(handle, buffer, nbyte);
#else
    return read(handle, buffer, nbyte);
#endif // __WIN32__
}

/* K&R 8.2 NOT THREAD-SAFE */
static int getchar_fd(socket_t sock)
{
    struct socket_t * socket = sock;

    if (socket->n == 0) {
        socket->n = recv(socket->sockfd, socket->buf, 1024, 0);
        socket->bufp = socket->buf;
    }
    return (--socket->n >= 0) ? (unsigned char) *socket->bufp++ : EOF;
}


#ifdef __WIN32__
static void initWinSock(void)
{
    WSADATA wsa_data;
    int n;

    if ((n = WSAStartup(MAKEWORD(2, 2), &wsa_data)) != 0) {
        fprintf(stderr, "WSAStartup failed: %d, %s\n", n, strerror(errno));
        return;
    }
}
#endif

socket_t socket_connect(char * host_name, int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent * server;
    struct in_addr addr;

    struct socket_t * sockets = malloc(sizeof *sockets);
    sockets->bufp = sockets->buf;
    memset(sockets, 0, sizeof (*sockets));

    memset(&serv_addr, 0, sizeof (serv_addr));
    memset(&addr, 0, sizeof (addr));

#ifdef __WIN32__
    initWinSock();
#endif // __WIN32__

    server = gethostbyname(host_name);
    if (server == NULL) {
        fprintf(stderr, "Failed to resolve hostname: %s\n", host_name);
        return NULL;
    }

    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    serv_addr.sin_family = AF_INET;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        fprintf(stderr, "Failed to get socket fd. Errno: (%s)\n", strerror(errno));
        return NULL;
    }

    if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
        fprintf(stderr, "Error connecting to hostname: %s, Errno: (%s)\n", host_name, strerror(errno));
        return NULL;
    }

    sockets->sockfd = sockfd;

    return sockets;
}

int socket_write(socket_t sock, char * buf, int buf_len)
{
    int n = send(((struct socket_t *) sock)->sockfd, buf, buf_len, 0);
    if (n < 0) {
        fprintf(stderr, "Send error: %s\n", strerror(errno));
    }
    return n;
}

int socket_readline(socket_t sock, char * buf, int buf_len)
{
    int ret, i = 0;

    while( (ret = getchar_fd(sock)) != '\n' && ret != EOF && ++i < buf_len)
        *buf++ = ret;

    *buf = '\0';

    return 0;
}

int socket_close(socket_t sock)
{
    close(((struct socket_t *) sock)->sockfd);
    free(sock);
    return 0;
}
