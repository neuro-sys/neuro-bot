#include "network.h"
#include "socket_win.h"
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>

struct network_t {
  char * host_name;
  int    port;
  int    sockfd;
  GIOChannel * giochannel;
};

GIOChannel * network_get_giochannel(struct network_t * network)
{
  return network->giochannel;
}

struct network_t * network_connect(char * host_name, int port)
{
  char port_str[10];
  int sockfd;
  struct network_t * network = malloc(sizeof * network);

  sprintf(port_str, "%d", port);
  sockfd = t_connect(host_name, port_str);
  if (sockfd < 0)
    return NULL;

  network->giochannel = g_io_channel_win32_new_socket(sockfd);
  network->port = port;
  network->host_name;

  return network;
}

int network_read_line(struct network_t * network, char ** buf)
{
  int len;
  GError * error = NULL;

  g_io_channel_read_line (network->giochannel, buf, &len, NULL, &error);

  return len;
}

int network_read(struct network_t * network, char * str)
{
  int len;
  g_io_channel_read_to_end(network->giochannel, &str, &len, NULL);
  return len;
}

void network_auth(struct network_t * network, char * nick, char * user, char * name)
{
  char message[255];
  int read;
  GError * error = NULL;

  sprintf(message, "NICK %s\r\n"
                   "USER %s 8 * : %s\r\n\r\n", nick, user, name);

  g_io_channel_write_chars(network->giochannel, message, strlen(message), &read, &error);

}

