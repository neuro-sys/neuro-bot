#include "irc_plugin.h"

#include "global.h"

#include "irc.h"
#include "plugin.h"
#include "socket.h"

#include <stdio.h>
#include <string.h>

void irc_plugin_handle_command(struct irc_t * irc)
{
    struct plugin_t * plugin;
    char command_name[50];
    struct plugin_t * plugin_command_list[10];
    struct plugin_t ** it;

    size_t n = strcspn(irc->message.trailing+1, " \r\n"); // skip the initial period
    n = n >= 50 ? 50 : n; // CLAM
    strncpy(command_name, irc->message.trailing+1, n);
    command_name[n] = 0;

    plugin_command_list[0] = NULL;

    plugin_find_commands(command_name, plugin_command_list);

    if (plugin_command_list[0] == NULL)
        return;

    for (it = plugin_command_list; *it != NULL; it++) {
        plugin = *it;

#ifdef TEST_IRC_PLUGIN
        plugin->irc = irc;
#endif
        debug("Handling plugin command: %s\n", plugin->name);
        plugin->run();

#ifdef TEST_IRC_PLUGIN
        debug_ex("%s\n", irc->response);
#endif
    }
}

void irc_plugin_handle_grep(struct irc_t * irc)
{
    struct plugin_list_t * it;

    for (it = plugin_list_head; it != NULL; it = it->next) {
        struct plugin_t * plugin;
        char ** keywords;

        plugin = it->cur;

        if (!plugin->is_grep || plugin->is_manager)
            continue;

        keywords = plugin->keywords;

        while (*keywords) {
            char * keyword = *keywords++;

            if (strstr(irc->message.trailing, keyword))
                plugin->run();
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

