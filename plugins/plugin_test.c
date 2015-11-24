#include "plugin_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct plugin_t * plugin;

void run(int type)
{
    char response[512];

    sprintf(response, "PRIVMSG %s :Hello!\r\n", plugin->irc->from);

    plugin->send_message(plugin->irc, response);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "command";
    plugin->type       = PLUGIN_TYPE_COMMAND;

    return plugin;
}

