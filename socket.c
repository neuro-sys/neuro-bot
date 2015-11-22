#include "socket.h"

#include <errno.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#ifdef __WIN32__
    #include <winsock2.h>
    #include <WS2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netdb.h>
    #include <unistd.h>
#endif // __WIN32__

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

/* K&R 8.2 */
static int getchar_fd(int sockfd)
{
    static char buf[1024];
    static char * bufp;
    static int n;

    if (n == 0) {
        if ( (n = recv(sockfd, buf, 1024, 0)) <= 0 ) {
            return '\n';
        }
        bufp = buf;
    }
    return (--n >= 0) ? (unsigned char) *bufp++ : EOF;
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

int socket_connect(char * host_name, int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent * server;
    struct in_addr addr;

    memset(&serv_addr, 0, sizeof (serv_addr));
    memset(&addr, 0, sizeof (addr));

#ifdef __WIN32__
    initWinSock();
#endif // __WIN32__

    server = gethostbyname(host_name);
    if (server == NULL) {
        fprintf(stderr, "Failed to resolve hostname: %s\n", host_name);
        return -1;
    }

    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    serv_addr.sin_family = AF_INET;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        fprintf(stderr, "Failed to get socket fd. Errno: (%s)\n", strerror(errno));
        return -1;
    }

    if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
        fprintf(stderr, "Error connecting to hostname: %s, Errno: (%s)\n", host_name, strerror(errno));
        return -1;
    }

    return sockfd;
}

int socket_write(int sockfd, char * buf, int buf_len)
{
    int n = send(sockfd, buf, buf_len, 0);
    if (n < 0 || errno != 0) {
        fprintf(stderr, "Write error: %s (bytes written: %d)\n", strerror(errno), n);
    }
    return n;
}

int socket_readline(int sockfd, char * buf, int buf_len)
{
    int ret, i = 0;

    while( (ret = getchar_fd(sockfd)) != '\n' || ++i == buf_len - 1)
        *buf++ = ret;

    if (errno != 0) {
        fprintf(stderr, "Socket failed: ERRNO: %s\n", strerror(errno));
        return -1;
    }

    *buf = '\0';

    return 0;
}

int socket_close(int sockfd)
{
    return close(sockfd);
}
