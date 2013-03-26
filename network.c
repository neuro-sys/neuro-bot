#include "global.h"
#include "network.h"
#include "socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>

void network_connect(struct network_t * network, char * host_name, int port)
{
    char port_str[10];
    int sockfd;

    sprintf(port_str, "%d", port);
    sockfd = t_connect(host_name, port_str);

#if defined (_WIN32)
    network->giochannel = g_io_channel_win32_new_socket(sockfd);
#elif defined(unix)
    network->giochannel = g_io_channel_unix_new(sockfd);
#else
#error WUT?
#endif
    network->port       = port;
    network->host_name  = host_name;

    g_io_channel_set_encoding(network->giochannel, NULL, NULL);
}

int network_read_line(struct network_t * network, char ** buf)
{
    gsize len;
    GIOStatus giostatus;
    GError * error = NULL;

    if ( (giostatus = g_io_channel_read_line (network->giochannel, buf, &len, NULL, &error)) != G_IO_STATUS_NORMAL) {
        printf("%d\n", giostatus);
        if (giostatus == 0) printf("%s\n", error->message);
        return -1;
    }

    return len;
}

void network_send_message(struct network_t * network, char * message)
{
    gsize     read;
    GError    * error = NULL;
    GIOStatus status;

    status = g_io_channel_write_chars(network->giochannel, message, strlen(message), &read, &error);

    if (status == G_IO_STATUS_NORMAL)
        g_io_channel_flush(network->giochannel, NULL);

}

void network_auth(struct network_t * network, char * nick, char * user, char * pass)
{
    char message[255];

    snprintf(message, sizeof message, "NICK %s\r\n" "USER %s 8 * :%s\r\n\r\n", nick, user, user);

    network_send_message(network, message);
    if (pass && *pass != '\0') {
        sprintf(message, "PRIVMSG NickServ :identify %s\r\n", pass);
        network_send_message(network, message);
    }

    printf("%s", message);
}
