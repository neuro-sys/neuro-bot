#include "irc_plugin.h"

#include "global.h"

#include "irc.h"
#include "plugin.h"
#include "socket.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void irc_plugin_handle_command(struct irc_t * irc)
{
#define MAX_COMMAND_NAME_SIZE 50
    char command_name[50];
    struct plugin_t ** plugin_commands_v = NULL, ** iterator;

    size_t n = strcspn(irc->message.trailing+1, " \r\n"); // substring(1, " \r\n")
    snprintf(command_name, n+1, "%s", irc->message.trailing+1);

    plugin_find_commands(command_name, &plugin_commands_v);

    if (plugin_commands_v == NULL)
        return;

    for (iterator = plugin_commands_v; *iterator != NULL; iterator++) {
        struct plugin_t * plugin = *iterator;

#ifdef TEST_IRC_PLUGIN
        plugin->irc = irc;
#endif
        debug("Handling plugin command: %s\n", plugin->name);
        plugin->is_command |= 1 << 2;
        plugin->run();
        plugin->is_command &= ~(1 << 2);

#ifdef TEST_IRC_PLUGIN
        debug_ex("%s\n", irc->response);
#endif
    }

    free(plugin_commands_v);
}

void irc_plugin_handle_grep(struct irc_t * irc)
{
    struct plugin_slist_t * iterator;

    SLIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator->plugin;
        char ** keywords;

        if (!plugin->is_grep || plugin->is_manager)
            continue;

        keywords = plugin->keywords;

        while (*keywords) {
            char * keyword = *keywords++;

            if (strstr(irc->message.trailing, keyword)) {
                plugin->is_grep |= 1 << 2;
                plugin->run();
                plugin->is_grep &= ~(1 << 2);
                break;
            }
        }
    }
}

#ifdef TEST_IRC_PLUGIN
int main(int argc, char *argv[])
{
    struct irc_t irc;

    memset(&irc, 0, sizeof irc);
    strcpy(irc.message.trailing, ".wiki irc bot");

    plugin_init();
    irc_plugin_handle_command(&irc); 

    return 0;
}

#endif

