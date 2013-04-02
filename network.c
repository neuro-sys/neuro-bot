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
    int ret;
    char ch;
    while(1)
    {
      ret = recv(network->sockfd, &ch, 1, 0);
      if (ret > 0)
      {
        if (ch == '\n')
          break;
        *buf++ = ch;
      }
    }
    *buf = '\0';

    return 1;
}

void network_send_message(struct network_t * network, char * message)
{
    send(network->sockfd, message, strlen(message), 0);
}

