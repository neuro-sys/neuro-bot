#include "irc_plugin.h"

#include "irc.h"
#include "plugin.h"
#include "socket.h"

#include <stdio.h>
#include <string.h>

void handle_plugin_command(struct irc_t * irc)
{
    struct plugin_t * plugin;
    char command_name[50];
    char plugin_response[MAX_IRC_MSG];
    char irc_response[MAX_IRC_MSG];

    command_name[0] = 0;
    plugin_response[0] = 0;
    irc_response[0] = 0;

    size_t n = strcspn(irc->message.trailing+1, " \r\n");
    strncpy(command_name, irc->message.trailing+1, n);
    command_name[n] = 0;

    plugin = plugin_find_command(command_name);

    if (plugin == NULL)
        return;

    fprintf(stderr, "handling plugin command. %s\n", plugin->name);

    plugin->run(irc->message.trailing, plugin_response);
    if (!plugin_response[0])
        return;

    sprintf(irc_response, "PRIVMSG %s :%s\r\n", irc->from, plugin_response);
    socket_send_message(&irc->session->socket, irc_response);
}

