#include "irc.h"

#include "global.h"
#include "irc_parser.h"
#include "irc_plugin.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* Send misc user commands. */
void irc_set_nick(struct irc_t * irc, char * nickname)
{
    char buffer[MAX_IRC_MSG];

    snprintf(buffer, MAX_IRC_MSG, "NICK %s\r\n", nickname); 
    socket_send_message(&irc->session->socket, buffer);
}

void irc_set_user(struct irc_t * irc, char * user, char * host)
{
    char buffer[MAX_IRC_MSG];

    snprintf(buffer, MAX_IRC_MSG, "USER %s 8 * :%s\r\n\r\n", user, host); 
    socket_send_message(&irc->session->socket, buffer);
}

void irc_identify_to_auth(struct irc_t * irc, char * password)
{
    char buffer[MAX_IRC_MSG];

    sprintf(buffer, "PRIVMSG NickServ :identify %s\r\n", password);
    socket_send_message(&irc->session->socket, buffer);
}

void irc_join_channel(struct irc_t * irc, char * channel)
{
    char buffer[MAX_IRC_MSG];

    sprintf(buffer, "JOIN %s\r\n", channel);
    socket_send_message(&irc->session->socket, buffer);
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
        socket_send_message(&irc->session->socket, response);
    } else if (strcmp(".part", command) == 0) {
        sprintf(response, "PART %s\r\n", argument);
        socket_send_message(&irc->session->socket, response);
    } else if (strcmp(".raw", command) == 0) {
        sprintf(response, "%s\r\n", argument);
        socket_send_message(&irc->session->socket, response);
    }
    else if (strcmp(".reload", command) == 0) {
        //module_load();
    }
}

static void process_bot_command_user (struct irc_t * irc)
{
    irc_plugin_handle_command(irc);

    if (strcmp(irc->session->admin, irc->message.prefix.nickname.nickname) == 0) 
        process_bot_command_admin (irc);
}

/* Control IRC server commands */
static void process_command_privmsg (struct irc_t * irc)
{
    /* Set the sender/receiver (channel or user) for convenience. That it's first param may be oblivious. */
    strcpy(irc->from, irc->message.params[0]);

    if (!strcmp(irc->message.params[0], irc->session->nickname))
        strcpy(irc->from, irc->message.prefix.nickname.nickname);

    /* If it the trailing message starts with a period, it's a bot command */
    if ( irc->message.trailing[0] == '.' ) 
        process_bot_command_user (irc);

    irc_plugin_handle_grep(irc);
}

static void process_command_join(struct irc_t * irc)
{
    int channel_counter = 0;
    char * channel;
    char ** iterator;

    channel = irc->message.params[0]; 
  
    if (irc->channels_v != NULL) {
        for (iterator = irc->channels_v; *iterator != NULL; iterator++, channel_counter++) {}
    }

    irc->channels_v = realloc(irc->channels_v, (channel_counter+1) * sizeof (char *)); 
    irc->channels_v[channel_counter] = strdup(channel);
    irc->channels_v[channel_counter+1] = NULL;

    for (iterator = irc->channels_v; *iterator != NULL; iterator++) {
        debug("In channel: %s\n", *iterator);
    }
}

static void process_command_part(struct irc_t * irc)
{
    int channels_counter = 0;
    char ** new_channels_v = NULL, ** iterator;
    char * channel;

    channel = irc->message.params[0];

    for (iterator = irc->channels_v; *iterator != NULL; iterator++) {
        char * temp_channel = *iterator;

        if (strcmp(temp_channel, channel) == 0) {
            continue;
        }

        new_channels_v = realloc(new_channels_v, (channels_counter+1) * sizeof (char *));
        new_channels_v[channels_counter++] = temp_channel;
    } 
    new_channels_v = realloc(new_channels_v, (channels_counter+1) * sizeof (char *));
    new_channels_v[channels_counter++] = NULL;
   
    free(irc->channels_v);
    irc->channels_v = new_channels_v; 

    for (iterator = irc->channels_v; *iterator != NULL; iterator++) {
        debug("In channel: %s\n", *iterator);
    }
}

static void process_protocol_commands (struct irc_t * irc)
{
    char response[512];

    if (strcmp("PRIVMSG", irc->message.command) == 0) {
        process_command_privmsg (irc);
    } else if (strcmp("PING", irc->message.command) == 0) {
        snprintf (response, MAX_IRC_MSG, "PONG %s\r\n", irc->message.trailing);
        socket_send_message(&irc->session->socket, response);
    } else if (strcmp("001", irc->message.command) == 0) {
        char ** channels_v;

        for (channels_v = irc->session->channels_ajoin_v; *channels_v != NULL; channels_v++) {
            irc_join_channel(irc, *channels_v);
        }
    } else if (strcmp("NOTICE", irc->message.command) == 0) {
        if (strstr(irc->message.trailing, "registered" ) ) {
            static int retry_count = 0;

            if (retry_count > 3) {
                return;
            }

            debug("This nickname seems to be registered. Trying to identify...\n");
            sleep(3);
            if (strcmp(irc->session->password, "")) {
                irc_identify_to_auth(irc, irc->session->password);
                retry_count++;
            }
        }
    } else if (strcmp(irc->message.command, "JOIN") == 0) {
        if (!strcmp(irc->message.prefix.nickname.nickname, irc->session->nickname))
            process_command_join(irc);

    } else if (strcmp(irc->message.command, "PART") == 0) {
        if (strcmp(irc->message.prefix.nickname.nickname, irc->session->nickname) == 0)
            process_command_part(irc);
    }
}


/* Main entry point */
void irc_process_line(struct irc_t * irc, const char * line)
{  
    irc_parser(&irc->message, line);
    print_message_t(&irc->message);

    process_protocol_commands(irc);
}


