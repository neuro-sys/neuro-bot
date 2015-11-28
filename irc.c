#include "irc.h"

#include "global.h"
#include "irc_parser.h"
#include "channel.h"
#include "plugin.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>

#ifdef __WIN32__
#include <windows.h>
#endif // __WIN32__

jmp_buf jmp_buf_recover_plugin;

void plugin_sigsegv_handler(int s)
{
    if (s == SIGSEGV) {
        longjmp(jmp_buf_recover_plugin, 1);
    }
}

/* Send misc user commands. */
void irc_set_nick(struct irc_t * irc, char * nickname)
{
    char buffer[MAX_IRC_MSG];

    snprintf(buffer, MAX_IRC_MSG, "NICK %s\r\n", nickname);
    socket_write(irc->sockfd, buffer, strlen(buffer));
}

void irc_set_user(struct irc_t * irc, char * user, char * host)
{
    char buffer[MAX_IRC_MSG];

    snprintf(buffer, MAX_IRC_MSG, "USER %s 8 * :%s\r\n\r\n", user, host);
    socket_write(irc->sockfd, buffer, strlen(buffer));
}

void irc_identify_to_auth(struct irc_t * irc, char * password)
{
    char buffer[MAX_IRC_MSG];

    sprintf(buffer, "PRIVMSG NickServ :identify %s\r\n", password);
    socket_write(irc->sockfd, buffer, strlen(buffer));
}

void irc_join_channel(struct irc_t * irc, char * channel)
{
    char buffer[MAX_IRC_MSG];

    sprintf(buffer, "JOIN %s\r\n", channel);
    socket_write(irc->sockfd, buffer, strlen(buffer));
}

static void command_help(struct irc_t * irc)
{
    struct plugin_t * iterator;
    char message[500];

    sprintf(message, "PRIVMSG %s :Loaded plugins: ", irc->from);

    LIST_FOREACH(iterator, &plugin_slist_head, list) {
        struct plugin_t * plugin = iterator;

        if (LIST_FIRST(&plugin_slist_head) != iterator) {
            sprintf(message + strlen(message), ", ");
        }
        sprintf(message + strlen(message), "%s", plugin->name);

        if (plugin->type & PLUGIN_TYPE_DAEMON) {
            sprintf(message + strlen(message),  " (daemon)" );
        }
        if (plugin->type & PLUGIN_TYPE_GREP) {
            sprintf(message + strlen(message),  " (grep)" );
        }
        if (plugin->type & PLUGIN_TYPE_COMMAND) {
            sprintf(message + strlen(message),  " (command)" );
        }

    }
    sprintf(message + strlen(message), ".\r\n");

    socket_write(irc->sockfd, message, strlen(message));
}

static void irc_plugin_handle_command(struct irc_t * irc)
{
    char command_name[50];
    struct plugin_t * iterator;

    size_t n = strcspn(irc->message.trailing+1, " \r\n"); // substring(1, " \r\n")
    snprintf(command_name, n+1, "%s", irc->message.trailing+1);

    if (strcmp(command_name, "help") == 0) {
        command_help(irc);
    } else {
        LIST_FOREACH(iterator, &plugin_slist_head, list) {
            if (strcmp(iterator->name, command_name) == 0) {
                iterator->run(PLUGIN_TYPE_COMMAND);
            }
        }
    }

}

static void irc_plugin_handle_grep(struct irc_t * irc)
{
    struct plugin_t * iterator;

    LIST_FOREACH(iterator, &plugin_slist_head, list) {
        struct plugin_t * plugin = iterator;
        char ** keywords_v;

        if (!(plugin->type & PLUGIN_TYPE_GREP))
            continue;

        debug("Grepping plugin for: %s\n", plugin->name)
        for (keywords_v = plugin->keywords; *keywords_v != NULL; keywords_v++) {
            char * keyword = *keywords_v;

            if (strstr(irc->message.trailing, keyword) || strcmp("*", keyword) == 0) {
                plugin->run(PLUGIN_TYPE_GREP);
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
        socket_write(irc->sockfd, response, strlen(response));
    } else if (strcmp(".part", command) == 0) {
        sprintf(response, "PART %s\r\n", argument);
        socket_write(irc->sockfd, response, strlen(response));
    } else if (strcmp(".raw", command) == 0) {
        sprintf(response, "%s\r\n", argument);
        socket_write(irc->sockfd, response, strlen(response));
    } else if (strcmp(".reload", command) == 0) {
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

    signal(SIGSEGV, plugin_sigsegv_handler);

    if (!setjmp(jmp_buf_recover_plugin)) {
        /* If it the trailing message starts with a period, it's a bot command */
        if ( irc->message.trailing[0] == '.' )
            process_bot_command_user (irc);

        irc_plugin_handle_grep(irc);
    } else {
        debug("Recovered from fatal error.\n");
    }
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

    channel = channel_find(&irc->channel_list_head, channel_name);
    if (channel == NULL) {
        return;
    }

    channel_add_user(channel, nickname);
}

static void process_command_join_new_channel(struct irc_t * irc)
{
    char * channel_name;
    struct channel_t * channel;

    channel_name = irc->message.params[0];
    channel = channel_new(channel_name);

    LIST_INSERT_HEAD(&irc->channel_list_head, channel, list);
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

    channel = channel_find(&irc->channel_list_head, channel_name);
    if (channel == NULL) {
        return;
    }
    
    fprintf(stdout, "process_command_part_user, nickname: %s\n", nickname);
    channel_remove_user(channel, nickname);
}

static void process_command_part_channel(struct irc_t * irc)
{
    struct channel_t * iterator, * temp;
    char * channel_name;

    channel_name = irc->message.params[0];

    temp = NULL;
    LIST_FOREACH(iterator, &irc->channel_list_head, list) {
        channel_free(temp);
        temp = NULL;
        if (strcmp(iterator->name, channel_name) == 0) {
            LIST_REMOVE(iterator, list);
            temp = iterator;
        }
    }
    channel_free(temp);
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

    channel = channel_find(&irc->channel_list_head, channel_name);
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
    }
}

static void process_protocol_commands (struct irc_t * irc)
{
    char response[512];

    if (strcmp("PRIVMSG", irc->message.command) == 0) {
        process_command_privmsg (irc);
    } else if (strcmp("PING", irc->message.command) == 0) {
        snprintf (response, MAX_IRC_MSG, "PONG %s\r\n", irc->message.trailing);
        socket_write(irc->sockfd, response, strlen(response));
    } else if (strcmp("001", irc->message.command) == 0) {
        struct ajoin_channel_t * iterator;

        LIST_FOREACH(iterator, &irc->ajoin_channels_head, list) {
            irc_join_channel(irc, iterator->channel_name);
        }
    } else if (strcmp("353", irc->message.command) == 0) {
        process_command_353(irc);
    } else if (strcmp("NOTICE", irc->message.command) == 0) {
        if (strstr(irc->message.trailing, "registered" ) ) {
            // A NOTICE that contains the word "registered" has been received.
            static int retry_count = 0;

            if (retry_count > 3) {
                return;
            }

            debug("This nickname seems to be registered. Trying to identify...\n");
            #ifdef __WIN32__
            Sleep(3000);
            #else
            sleep(3);
            #endif // __WIN32__
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
    LIST_INIT(&irc->channel_list_head);
    plugin_start_daemons(irc);
}


void irc_free(struct irc_t * irc)
{
    struct channel_t * iterator, * temp;
    struct ajoin_channel_t * ajoin_iterator, * a_temp;

    if (&irc->channel_list_head == NULL) {
        return;
    }

    temp = NULL;
    LIST_FOREACH(iterator, &irc->channel_list_head, list) {
        if (temp) channel_free(temp);
        temp = iterator;
    }
    if (temp) channel_free(temp);

    a_temp = NULL;
    LIST_FOREACH(ajoin_iterator, &irc->ajoin_channels_head, list) {
        free(a_temp);
        a_temp = ajoin_iterator;
        debug("Removing ajoin channel: %s\n", ajoin_iterator->channel_name);
        free(ajoin_iterator->channel_name);
    }
    free(a_temp);

    free(irc->admin);
    free(irc->hostname);
    free(irc->nickname);
    free(irc->password);
    free(irc->port);
    debug("Closing socket\n");
    socket_close(irc->sockfd);
    debug("Closed socked.\n");
}

int irc_run(struct irc_t * irc)
{
    /* Conect to the server specified in socket_t struct. */
    if ( (irc->sockfd = socket_connect(irc->hostname, atoi(irc->port))) < 0 ) {
        debug("Unable to connect to %s:%s\n", irc->hostname, irc->port);
        exit(EXIT_SUCCESS);
    }

    /* Do one time initialization work after connecting to the server. */
    irc_init(irc);

    for (;;)
    {
        char line[MAX_IRC_MSG];
        if (socket_readline(irc->sockfd, line, sizeof(line)) < 0) {
            return -1;
        }
        irc_process_line(irc, line);
    }
}
