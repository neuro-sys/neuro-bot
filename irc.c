#include "irc.h"

#include "global.h"
#include "irc_parser.h"
#include "irc_plugin.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Send misc user commands. */
void irc_set_nick(struct irc_t * irc, char * nickname, char * buffer)
{
    snprintf(buffer, MAX_IRC_MSG, "NICK %s\r\n", nickname); 
    socket_send_message(&irc->session->socket, buffer);
}

void irc_set_user(struct irc_t * irc, char * user, char * host, char * buffer)
{
    snprintf(buffer, MAX_IRC_MSG, "USER %s 8 * :%s\r\n\r\n",
            user, host); 
    socket_send_message(&irc->session->socket, buffer);
}

void irc_identify_to_auth(struct irc_t * irc, char * password, char * buffer)
{
    sprintf(buffer, "PRIVMSG NickServ :identify %s\r\n", password);
    socket_send_message(&irc->session->socket, buffer);
}

void irc_join_channel(struct irc_t * irc, char * channel, char * buffer)
{
    sprintf(buffer, "JOIN %s\r\n", channel);
    socket_send_message(&irc->session->socket, buffer);
}

/* Process bot user commands */
static void process_bot_command_admin (struct irc_t * irc)
{
    char * t;
    char command[MAX_IRC_MSG], argument[MAX_IRC_MSG];
    char str[MAX_IRC_MSG];
    char response[512];

    command[0] = 0;
    argument[0] = 0;
    str[0] = 0;

    strncpy(str, irc->message.trailing, MAX_IRC_MSG);
    
    if (!(t = strtok(str, " ")))
        return;

    strncpy(command, t, MAX_IRC_MSG);
    if ((t = strtok(NULL, "\r\n")))
        strncpy(argument, t, MAX_IRC_MSG);
        
    if (!strncmp(".join", command, 5)) {
        sprintf(response, "JOIN %s\r\n", argument);
        socket_send_message(&irc->session->socket, response);
    }
    else if (!strncmp(".part", command, 5)) {
        sprintf(response, "PART %s\r\n", argument);
        socket_send_message(&irc->session->socket, response);
    }
    else if (!strncmp(".raw", command, 4)) {
        sprintf(response, "%s\r\n", argument);
        socket_send_message(&irc->session->socket, response);
    }
    else if (!strncmp(".reload", command, 7)) {
        //module_load();
    }
}

static void process_bot_command_user (struct irc_t * irc)
{
    irc_plugin_handle_command(irc);

    if ( !strncmp (irc->session->admin, irc->message.prefix.nickname.nickname, strlen(irc->session->admin)) ) 
        process_bot_command_admin (irc);
}

/* Control IRC server commands */
static void process_command_privmsg (struct irc_t * irc)
{
    /* Decide the sender if it's a query window or channel */
    strcpy(irc->from, irc->message.params[0]);

    if (!strcmp(irc->message.params[0], irc->session->nickname))
        strcpy(irc->from, irc->message.prefix.nickname.nickname);

    /* If it the trailing message starts with a period, it's a bot command */
    if ( irc->message.trailing[0] == '.' ) 
        process_bot_command_user (irc);

    irc_plugin_handle_grep(irc);
}

static void process_command_353_join(struct irc_t * irc)
{
    int i;
    char * channel;

    channel = irc->message.params[0]; 

    irc->channels = realloc(irc->channels, sizeof (irc->channels) * irc->channels_siz+1);
    irc->channels[irc->channels_siz] = strdup(channel);
    irc->channels_siz++;

    for (i = 0; i < irc->channels_siz; i++)
        puts(irc->channels[i]);
}

static void process_command_part(struct irc_t * irc)
{
    int i, j;
    char ** new_channels;
    char * channel;

    channel = irc->message.params[0];

    new_channels = malloc(sizeof (irc->channels) * irc->channels_siz-1);

    for (j = 0, i = 0; i < irc->channels_siz; i++) 
        if (!strstr(irc->channels[i], channel))
            new_channels[j] = irc->channels[i];
        else
            free(irc->channels[i]);

    free(irc->channels);
    irc->channels = new_channels;
    irc->channels_siz--;
}

static void process_protocol_commands (struct irc_t * irc)
{
    char response[512];

    if ( !strncmp ("PRIVMSG", irc->message.command, 7) ) 
        process_command_privmsg (irc);
    else if ( !strncmp ("PING", irc->message.command, 4)) {
        snprintf (response, MAX_IRC_MSG, "PONG %s\r\n", irc->message.trailing);
        socket_send_message(&irc->session->socket, response);
    }
    else if ( !strncmp ("001", irc->message.command, 3) ) {
        char message[MAX_IRC_MSG];
        char ** t;

        for (t = irc->session->channels_ajoin; *t != NULL; t++) {
            irc_join_channel(irc, *t, message);
        }
    } else if ( !strncmp("NOTICE", irc->message.command, 6) ) {
        if (strstr(irc->message.trailing, "registered" ) ) {
            char message[MAX_IRC_MSG];

            fprintf(stderr, "Auth to nickserv request received.\n");
            if (strcmp(irc->session->password, "")) {
                fprintf(stderr, "Authing to nickserv\n");
                irc_identify_to_auth(irc, irc->session->password, message);
            }
        }
    } else if (strstr(irc->message.command, "JOIN")) {
        if (!strcmp(irc->message.prefix.nickname.nickname, irc->session->nickname))
            process_command_353_join(irc);

    } else if (strstr(irc->message.command, "PART")) {
        if (!strcmp(irc->message.prefix.nickname.nickname, irc->session->nickname))
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


