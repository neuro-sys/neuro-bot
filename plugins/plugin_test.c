#include "plugin_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct plugin_t * plugin;

void run(void)
{
    sprintf(plugin->irc->response, "PRIVMSG %s :Hello!", plugin->irc->from);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "command";
    plugin->is_looper  = 0;
    plugin->is_command = 1;
    plugin->is_grep    = 0;

    return plugin;
}

