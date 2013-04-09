#if defined(_WIN32)
#include <stdio.h>
#include <WS2tcpip.h>
#include "socket.h"

static void initWinSock(void)
{
    WSADATA wsa_data;

    if (WSAStartup(MAKEWORD(2, 1), &wsa_data) != 0)
        return;
}

int t_connect_win(char *host, char *port)
{
    struct addrinfo hints, *res;
    int sockfd;

    initWinSock();
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &res) != 0) {
        MessageBox(NULL, "getaddrinfo failed.", "Error", MB_OK);
        return -1;
    }
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        MessageBox(NULL, "socket failed.", "Error", MB_OK);
        return -1;
    }
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        MessageBox(NULL, "socket failed.", "Error", MB_OK);
        return -1;
    }

    freeaddrinfo(res);

    return sockfd;
}
#endif
