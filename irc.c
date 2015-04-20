#include "irc.h"

#include "global.h"
#include "irc_parser.h"
#include "channel.h"
#include "plugin.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* Send misc user commands. */
void irc_set_nick(struct irc_t * irc, char * nickname)
{
    char buffer[MAX_IRC_MSG];

    snprintf(buffer, MAX_IRC_MSG, "NICK %s\r\n", nickname); 
    socket_send_message(&irc->socket, buffer);
}

void irc_set_user(struct irc_t * irc, char * user, char * host)
{
    char buffer[MAX_IRC_MSG];

    snprintf(buffer, MAX_IRC_MSG, "USER %s 8 * :%s\r\n\r\n", user, host); 
    socket_send_message(&irc->socket, buffer);
}

void irc_identify_to_auth(struct irc_t * irc, char * password)
{
    char buffer[MAX_IRC_MSG];

    sprintf(buffer, "PRIVMSG NickServ :identify %s\r\n", password);
    socket_send_message(&irc->socket, buffer);
}

void irc_join_channel(struct irc_t * irc, char * channel)
{
    char buffer[MAX_IRC_MSG];

    sprintf(buffer, "JOIN %s\r\n", channel);
    socket_send_message(&irc->socket, buffer);
}

static void command_help(struct irc_t * irc)
{
    struct plugin_t * iterator;
    char message[500];

    sprintf(message, "PRIVMSG %s :Loaded plugins: ", irc->from);

    LIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator;
   
        if (LIST_FIRST(&plugin_slist_head) != iterator) {
            sprintf(message + strlen(message), ", ");
        } 
        sprintf(message + strlen(message), "%s", plugin->name);

        if (plugin->is_daemon) {
            sprintf(message + strlen(message),  " (d)" );
        } 
        if (plugin->is_grep) {
            sprintf(message + strlen(message),  " (g)" );
        } 
        if (plugin->is_command) {
            sprintf(message + strlen(message),  " (c)" );
        }

    }
    sprintf(message + strlen(message), ".");

    socket_send_message(&irc->socket, message);

    message[0] = 0;
    sprintf(message, "PRIVMSG %s :(d) is daemon, (g) is grep, (c) is command type of plugins. you can run command plugins with prefix `.'", irc->from);
    socket_send_message(&irc->socket, message);
}

static void irc_plugin_handle_command(struct irc_t * irc)
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

        BIT_ON(plugin->is_command, 2);
        plugin->run();
        BIT_OFF(plugin->is_command, 2);
    }

    free(plugin_commands_v);

    if (strcmp(command_name, "help") == 0) {
        command_help(irc);
    }
#undef MAX_COMMAND_NAME_SIZE 
}

static void irc_plugin_handle_grep(struct irc_t * irc)
{
    struct plugin_t * iterator;

    LIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator;
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


/* Process bot user commands */
static void process_bot_command_admin (struct irc_t * irc)
{
    char * token;
    char command[MAX_IRC_MSG], argument[MAX_IRC_MSG];
    char tokenize_buffer[MAX_IRC_MSG];
    char response[512];

    snprintf(tokenize_buffer, MAX_IRC_MSG, "%s", irc->message.trailing);
    
    if ((token = strtok(tokenize_buffer, " \r\n")) == NULL)
        return;

    snprintf(command, MAX_IRC_MSG, "%s", token);

    if ((token = strtok(NULL, "\r\n"))) {
        snprintf(argument, MAX_IRC_MSG, "%s", token);
    } 

    if (strcmp(".join", command) == 0) {
        sprintf(response, "JOIN %s\r\n", argument);
        socket_send_message(&irc->socket, response);
    } else if (strcmp(".part", command) == 0) {
        sprintf(response, "PART %s\r\n", argument);
        socket_send_message(&irc->socket, response);
    } else if (strcmp(".raw", command) == 0) {
        sprintf(response, "%s\r\n", argument);
        socket_send_message(&irc->socket, response);
    }
    else if (strcmp(".reload", command) == 0) {
        //module_load();
    }
}

static void process_bot_command_user (struct irc_t * irc)
{
    irc_plugin_handle_command(irc);

    if (strcmp(irc->admin, irc->message.prefix.nickname.nickname) == 0) 
        process_bot_command_admin (irc);
}

/* Control IRC server commands */
static void process_command_privmsg (struct irc_t * irc)
{
    /* Set the sender/receiver (channel or user) for convenience. That it's first param may be oblivious. */
    strcpy(irc->from, irc->message.params[0]);

    if (!strcmp(irc->message.params[0], irc->nickname))
        strcpy(irc->from, irc->message.prefix.nickname.nickname);

    /* If it the trailing message starts with a period, it's a bot command */
    if ( irc->message.trailing[0] == '.' ) 
        process_bot_command_user (irc);

    irc_plugin_handle_grep(irc);
}

static void process_command_join_new_user(struct irc_t * irc)
{
    struct channel_t * channel;
    char * channel_name, * nickname;

    channel_name = irc->message.params[0];
    if (channel_name == NULL) {
        return;
    }

    nickname = irc->message.prefix.nickname.nickname;
    if (nickname == NULL) {
        return;
    }

    channel = channel_find(irc->channels_v, channel_name);
    if (channel == NULL) {
        return;
    }

    channel_add_user(channel, nickname);
}

static void process_command_join_new_channel(struct irc_t * irc)
{
    int channel_counter = 0;
    char * channel_name;
    struct channel_t * channel, ** iterator;

    channel_name = irc->message.params[0]; 
    debug("%s\n", irc->message.trailing);
 
    if (irc->channels_v != NULL) {
        for (iterator = irc->channels_v; *iterator != NULL; iterator++, channel_counter++) {}
    }

    irc->channels_v = realloc(irc->channels_v, (channel_counter+1) * sizeof (struct channel_t *)); 
    channel = channel_new(channel_name);
    irc->channels_v[channel_counter++] = channel;

    irc->channels_v = realloc(irc->channels_v, (channel_counter+1) * sizeof (struct channel_t *)); 
    irc->channels_v[channel_counter++] = NULL;

    for (iterator = irc->channels_v; *iterator != NULL; iterator++) {
        debug("In channel: %s\n", (*iterator)->name);
    }
}

static void process_command_part_user(struct irc_t * irc)
{
    struct channel_t * channel;
    char * channel_name, * nickname;

    channel_name = irc->message.params[0];
    if (channel_name == NULL) {
        return;
    }

    nickname = irc->message.prefix.nickname.nickname;
    if (nickname == NULL) {
        return;
    }

    channel = channel_find(irc->channels_v, channel_name);
    if (channel == NULL) {
        return;
    }

    channel_remove_user(channel, nickname);
}

static void process_command_part_channel(struct irc_t * irc)
{
    int channels_counter = 0;
    struct channel_t ** new_channels_v = NULL, ** iterator;
    char * channel_name;

    channel_name = irc->message.params[0];

    for (iterator = irc->channels_v; *iterator != NULL; iterator++) {
        struct channel_t * temp_channel = *iterator;

        if (strcmp(temp_channel->name, channel_name) == 0) {
            free(temp_channel);
            continue;
        }

        new_channels_v = realloc(new_channels_v, (channels_counter+1) * sizeof (struct channel_t *));
        new_channels_v[channels_counter++] = temp_channel;
    } 
    new_channels_v = realloc(new_channels_v, (channels_counter+1) * sizeof (struct channel_t *));
    new_channels_v[channels_counter++] = NULL;
   
    free(irc->channels_v);
    irc->channels_v = new_channels_v; 

    for (iterator = irc->channels_v; *iterator != NULL; iterator++) {
        debug("In channel: %s\n", (*iterator)->name);
    }
}

static void process_command_353(struct irc_t * irc)
{
    struct channel_t * channel;
    char * channel_name;
    int param_counter = 0;

    while ((channel_name = irc->message.params[param_counter++]) != NULL) {
        if (channel_name[0] == '#') {
            break;
        }
    }

    channel = channel_find(irc->channels_v, channel_name);
    if (channel == NULL) {
        return;
    }

    char * token;

    token = strtok(irc->message.trailing, " ");
    if (token == NULL) {
        return;
    }
    channel_add_user(channel, token);

    while ((token = strtok(NULL, " ")) != NULL) {
        channel_add_user(channel, token);
        debug("%s\n", token);
    }
}

static void process_protocol_commands (struct irc_t * irc)
{
    char response[512];

    if (strcmp("PRIVMSG", irc->message.command) == 0) {
        process_command_privmsg (irc);
    } else if (strcmp("PING", irc->message.command) == 0) {
        snprintf (response, MAX_IRC_MSG, "PONG %s\r\n", irc->message.trailing);
        socket_send_message(&irc->socket, response);
    } else if (strcmp("001", irc->message.command) == 0) {
        char ** channels_v;

        for (channels_v = irc->channels_ajoin_v; *channels_v != NULL; channels_v++) {
            irc_join_channel(irc, *channels_v);
        }
    } else if (strcmp("353", irc->message.command) == 0) {
            process_command_353(irc);
    } else if (strcmp("NOTICE", irc->message.command) == 0) {
        if (strstr(irc->message.trailing, "registered" ) ) {
            static int retry_count = 0;

            if (retry_count > 3) {
                return;
            }

            debug("This nickname seems to be registered. Trying to identify...\n");
            sleep(3);
            if (strcmp(irc->password, "")) {
                irc_identify_to_auth(irc, irc->password);
                retry_count++;
            }
        }
    } else if (strcmp(irc->message.command, "JOIN") == 0) {
        if (strcmp(irc->message.prefix.nickname.nickname, irc->nickname) == 0) {
            process_command_join_new_channel(irc);
        } else {
            process_command_join_new_user(irc); 
        }
    } else if (strcmp(irc->message.command, "PART") == 0) {
        if (strcmp(irc->message.prefix.nickname.nickname, irc->nickname) == 0) {
            process_command_part_channel(irc);
        } else {
            process_command_part_user(irc);
        }
    } else if (strcmp(irc->message.command, "QUIT") == 0) {
        if (strcmp(irc->message.prefix.nickname.nickname, irc->nickname) == 0) {

        } else {
            process_command_part_user(irc);
        }
    }
}


static void irc_process_line(struct irc_t * irc, const char * line)
{  
    irc_parser(&irc->message, line);
    print_message_t(&irc->message);

    process_protocol_commands(irc);
}


static void irc_init(struct irc_t * irc)
{
    irc_set_nick(irc, irc->nickname);

    irc_set_user(irc, "ircbot", "github.com/neuro-sys/neuro-bot");

    plugin_start_daemons(irc);
}


void irc_free(struct irc_t * irc)
{
    struct channel_t ** iterator;

    if (irc->channels_v == NULL) {
        return;
    }

    for (iterator = irc->channels_v; *iterator != NULL; iterator++) {
        channel_free(*iterator);
    }

    free(irc->channels_v);

}

int irc_run(struct irc_t * irc)
{ 
    plugin_attach_context(irc);

    /* Conect to the server specified in socket_t struct. */
    if ( socket_connect(&irc->socket) < 0 ) {
        debug("Unable to connect to %s:%s\n", irc->socket.host_name, irc->socket.port);
        exit(EXIT_SUCCESS);
    }

    /* Do one time initialization work after connecting to the server. */
    irc_init(irc);

    while (666) 
    {
        char line[MAX_IRC_MSG];

        if (socket_read_line(&irc->socket, line) < 0) /* blocking io */
            return -1;
            
        irc_process_line(irc, line);
    }
}

