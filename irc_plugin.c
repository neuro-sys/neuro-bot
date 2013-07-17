#include "irc_plugin.h"

#include "irc.h"
#include "plugin.h"
#include "socket.h"

#include <stdio.h>
#include <string.h>

void irc_plugin_handle_command(struct irc_t * irc)
{
    struct plugin_t * plugin;
    char command_name[50];

    command_name[0]     = 0;

    size_t n = strcspn(irc->message.trailing+1, " \r\n");
    strncpy(command_name, irc->message.trailing+1, n);
    command_name[n] = 0;

    plugin = plugin_find_command(command_name);

    if (plugin == NULL)
        return;

    fprintf(stderr, "%s:%d:Handling plugin command: %s\n", __FILE__, __LINE__, plugin->name);

    plugin->run();
}

