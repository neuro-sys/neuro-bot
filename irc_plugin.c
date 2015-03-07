#include "irc_plugin.h"

#include "global.h"

#include "irc.h"
#include "plugin.h"
#include "socket.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void command_help(struct irc_t * irc)
{
    struct plugin_slist_t * iterator;
    char message[500];

    sprintf(message, "PRIVMSG %s :Loaded plugins: ", irc->from);

    SLIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator->plugin;
   
        if (SLIST_FIRST(&plugin_slist_head) != iterator) {
            sprintf(message + strlen(message), ", ");
        } 
        sprintf(message + strlen(message), "%s", plugin->name);

        if (plugin->is_looper) {
            sprintf(message + strlen(message), CYAN " (l)" CLEAR);
        } else if (plugin->is_grep) {
            sprintf(message + strlen(message), RED " (g)" CLEAR);
        } else if (plugin->is_command) {
            sprintf(message + strlen(message), YELLOW " (c)" CLEAR);
        }

    }
    sprintf(message + strlen(message), ".");

    socket_send_message(&irc->session->socket, message);

    message[0] = 0;
    sprintf(message, "PRIVMSG %s :(" CYAN "l" CLEAR ") is " CYAN "looper" CLEAR ", (" RED "g" CLEAR ") is " RED "grep" CLEAR ", (" YELLOW "c" CLEAR ") is " YELLOW "command" CLEAR " type of plugins. you can run " YELLOW "command" CLEAR " plugins with prefix `.'", irc->from);
    socket_send_message(&irc->session->socket, message);
}

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

    if (strcmp(command_name, "help") == 0) {
        command_help(irc);
    }
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

