#include "socket.h"
#include "global.h"

#if defined (unix)
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int t_connect_unix(char *host, char *port)
{
  struct addrinfo hints, *res;
  int sockfd;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(host, port, &hints, &res) != 0) {
    return -1;
  }
    
  if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)  {
    return -1;
  }
  if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
    return -1;
  }

  return sockfd;
}

#endif
