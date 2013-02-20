#include "global.h"
#include "network.h"
#include "socket.h"
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
  g_debug("%zu\t%s\t%s", __LINE__, __FILE__, __func__);
  return network->giochannel;
}

struct network_t * network_connect(char * host_name, int port)
{
  char port_str[10];
  int sockfd;
  struct network_t * network;
  
  g_debug("%zu\t%s\t%s", __LINE__, __FILE__, __func__);
  network = malloc(sizeof * network);

  sprintf(port_str, "%d", port);
  sockfd = t_connect(host_name, port_str);

#if defined(WIN32)
  network->giochannel = g_io_channel_win32_new_socket(sockfd);
#elif defined(unix)
  network->giochannel = g_io_channel_unix_new(sockfd);
#endif
  network->port       = port;
  network->host_name  = host_name;

  g_io_channel_set_encoding(network->giochannel, NULL, NULL);
  return network;
}

int network_read_line(struct network_t * network, char ** buf)
{
  int len;
  int giostatus;
  GError * error = NULL;
  
  g_debug("%zu\t%s\t%s", __LINE__, __FILE__, __func__);
  if ( (giostatus = g_io_channel_read_line (network->giochannel, buf, &len, NULL, &error)) != G_IO_STATUS_NORMAL) {
    printf("%d\n", giostatus);
    if (giostatus == 0) printf("%s\n", error->message);
    return -1;
  }
  
  return len;
}

int network_read(struct network_t * network, char * str)
{
  int len;
  g_io_channel_read_to_end(network->giochannel, &str, &len, NULL);
  return len;
}

void network_send_message(struct network_t * network, char * message)
{
  int read;
  GError * error = NULL;
  GIOStatus status;

  g_debug("%zu\t%s\t%s", __LINE__, __FILE__, __func__);

  status = g_io_channel_write_chars(network->giochannel, message, strlen(message), &read, &error);

  if (status == G_IO_STATUS_NORMAL)
    g_io_channel_flush(network->giochannel, NULL);
  
}

void network_auth(struct network_t * network, char * nick, char * user, char * pass)
{
  char message[255];
  int read;

  sprintf(message, "PASS *\r\n"
                   "NICK %s\r\n"
                   "USER %s 8 * :%s\r\n\r\n", nick, user, user);

  network_send_message(network, message);
  sprintf(message, "PRIVMSG NickServ :identify %s\r\n", pass);
  network_send_message(network, message);

  printf("%s", message);
}


