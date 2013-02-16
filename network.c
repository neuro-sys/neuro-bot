#include "network.h"
#include "socket_win.h"
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

struct network_t {
  char * host_name;
  int    port;

  GIOChannel * giochannel;
};

struct network_t * network_connect(char * host_name, int port)
{
  char port_str[10];
  int sockfd;
  struct network_t * network = malloc(sizeof * network);

  sprintf(port_str, "%d", port);
  sockfd = t_connect(host_name, port_str);
  if (sockfd < 0)
    return NULL;

  network->giochannel = g_io_channel_win32_new_messages(sockfd);
  network->port = port;
  network->host_name;

  return network;
}
