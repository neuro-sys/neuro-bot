#include "plugin_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct plugin_t * plugin;

struct argv_s {
    int argc;
    char ** argv;
};

struct argv_s * argv_parse(struct argv_s * param, char * str)
{
    char tokenize_buffer[MAX_IRC_MSG];
    char * token;

    memset(param, 0, sizeof(struct argv_s));

    snprintf(tokenize_buffer, MAX_IRC_MSG, "%s", str);

    token = strtok(tokenize_buffer, " ");
    if (token == NULL) {
        return NULL;
    }

    param->argv = realloc(param->argv, (param->argc + 1) * sizeof (char *));
    param->argv[param->argc++] = strdup(token);

    while ((token = strtok(NULL, " ")) != NULL) {
        param->argv = realloc(param->argv, (param->argc + 1) * sizeof (char *));
        param->argv[param->argc++] = strdup(token);
    }

    param->argv = realloc(param->argv, (param->argc + 1) * sizeof (char *));
    param->argv[param->argc] = NULL;

    return param;
}

void argv_free(struct argv_s * arg)
{
    char ** iterator;

    for (iterator = arg->argv; *iterator != NULL; iterator++) {
        free(*iterator);
    }

    free(arg->argv);
}

void print_user_list(char * channel_name)
{
    struct channel_t * channel = channel_find(plugin->irc->channels_v, channel_name);
    char ** iterator;
    char channel_users_buf[MAX_IRC_MSG];

    if (channel == NULL) {
        char response[512];

        sprintf(response, "PRIVMSG %s :Channel %s not found.", plugin->irc->from, channel_name);

        plugin->send_message(plugin->irc, response);

        return;
    }

    channel_users_buf[0] = 0;
    for (iterator = channel->users; *iterator != NULL; iterator++) {
        if (strlen(channel_users_buf) > MAX_IRC_MSG-50) {
            snprintf(channel_users_buf + strlen(channel_users_buf), MAX_IRC_MSG, "... ");
            break;
        }
        snprintf(channel_users_buf + strlen(channel_users_buf), MAX_IRC_MSG, "%s ", *iterator);
    } 

    char response[512];

    sprintf(response, "PRIVMSG %s :Users: %s", plugin->irc->from, channel_users_buf);

    plugin->send_message(plugin->irc, response);
}

void print_channels(void)
{
    struct channel_t ** iterator;
    char channel_names_list[MAX_IRC_MSG];

    channel_names_list[0] = 0;

    for (iterator = plugin->irc->channels_v; *iterator != NULL; iterator++) {
        struct channel_t * channel = *iterator;

        sprintf(channel_names_list + strlen(channel_names_list), "%s ", channel->name);                
    }

    char response[512];

    sprintf(response, "PRIVMSG %s :In channels: %s", plugin->irc->from, channel_names_list);

    plugin->send_message(plugin->irc, response);
}


void run(void)
{
    struct argv_s arg = { 0 };

    argv_parse(&arg, plugin->irc->message.trailing);

    if (arg.argc == 1) {
        print_channels();
    } else if (arg.argc == 2) {
        print_user_list(arg.argv[1]);
    }
    
    argv_free(&arg);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "channels";
    plugin->is_daemon  = 0;
    plugin->is_command = 1;
    plugin->is_grep    = 0;

    return plugin;
}

