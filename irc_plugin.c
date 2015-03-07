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
            sprintf(message + strlen(message),  " (l)" );
        } 
        if (plugin->is_grep) {
            sprintf(message + strlen(message),  " (g)" );
        } 
        if (plugin->is_command) {
            sprintf(message + strlen(message),  " (c)" );
        }

    }
    sprintf(message + strlen(message), ".");

    socket_send_message(&irc->session->socket, message);

    message[0] = 0;
    sprintf(message, "PRIVMSG %s :(l) is looper, (g) is grep, (c) is command type of plugins. you can run command plugins with prefix `.'", irc->from);
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

        debug("Handling plugin command: %s\n", plugin->name);

#ifndef TEST_IRC_PLUGIN
        BIT_ON(plugin->is_command, 2);
        plugin->run();
        BIT_OFF(plugin->is_command, 2);
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
        char ** keywords_v;

        if (!plugin->is_grep || plugin->is_manager)
            continue;

        for (keywords_v = plugin->keywords; *keywords_v != NULL; keywords_v++) {
            char * keyword = *keywords_v;

            if (strstr(irc->message.trailing, keyword) || strcmp("*", keyword) == 0) {
                debug("Handling grep command: %s\n", plugin->name);

                BIT_ON(plugin->is_grep, 2);
                plugin->run();
                BIT_OFF(plugin->is_grep, 2);
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

