#include "irc.h"

#include "global.h"
#include "py_wrap.h"
#include "module.h"
#include "irc_cmd.h"
#include "irc_parser.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/* Control bot commands */
static void control_bot_command_admin (struct irc_t * irc)
{
    char * t;
    char command[MAX_IRC_MSG], argument[MAX_IRC_MSG];
    char str[MAX_IRC_MSG];

    strncpy(str, irc->message.trailing, MAX_IRC_MSG);
    
    if ( !(t = strtok(str, " ")) )
        return;

    strncpy(command, t, MAX_IRC_MSG);

    if ( (t = strtok(NULL, "\r\n")) )
        strncpy(argument, t, MAX_IRC_MSG);
        
    if ( !strncmp(".join", command, strlen(".join")) )
        sprintf(irc->response, "JOIN %s\r\n", argument);
    else if (!strncmp(".part", command, strlen(".part")))
        sprintf(irc->response, "PART %s\r\n", argument);
    else if ( !strncmp(".raw", command, strlen(".raw")) )
        sprintf(irc->response, "%s\r\n", argument);
    else if ( !strncmp(".reload", command, strlen(".reload")) ) {
        module_load();
#ifdef USE_PYTHON_MODULES
        py_load_mod_hash();
#endif
    }
}

static void control_bot_command_user (struct irc_t * irc)
{
    struct mod_c_t * mod_c;
    char token[MAX_IRC_MSG];
    char str[MAX_IRC_MSG];
    char * t;

    strncpy(str, irc->request, MAX_IRC_MSG);
    if ( !(t = strtok(str, " \r\n")) )
        return;

    strncpy(token, t, MAX_IRC_MSG);

    if ( (mod_c = module_find(token+1)) )
    {
        char reply_msg[MAX_IRC_MSG];

        reply_msg[0] = '\0';
        mod_c->func(irc, reply_msg);
        
        if (reply_msg[0])
            snprintf(irc->response, MAX_IRC_MSG, "PRIVMSG %s :%s\r\n", irc->from, reply_msg);
    }
#ifdef USE_PYTHON_MODULES
    else
    {
        char * ret;
        struct py_module_t * mod = py_find_loaded_name (token); 

        if (mod) {
            ret = py_call_module ( mod, irc );
            snprintf( irc->response, MAX_IRC_MSG , "PRIVMSG %s :%s\r\n", irc->from, ret );
            free(ret);
        }
    }
#endif
    if ( !strncmp (irc->session->admin, irc->nick_to_msg, strlen(irc->session->admin)) ) 
        control_bot_command_admin (irc);
}

/* Control IRC commands */
static void control_command_privmsg (struct irc_t * irc)
{
    strcpy(irc->from, irc->message.params.list[0]);
    strcpy(irc->nick_to_msg, irc->message.prefix.nickname.nickname);

    if (!strcmp(irc->message.params.list[0], irc->session->nickname))
        strcpy(irc->from, irc->message.prefix.nickname.nickname);

    if ( irc->request[0] == '.' ) 
        control_bot_command_user (irc);
    else {
        struct mod_c_t * mod;
        char ret[MAX_IRC_MSG];

        mod = module_find_by_keyword(irc->request);

        ret[0] = '\0';

        if (mod)
            mod->func(irc, ret); 

        if (ret[0])
            snprintf(irc->response, MAX_IRC_MSG, "PRIVMSG %s :%s\r\n", irc->from, ret);
    }  
}

static void control_command_353_join(struct irc_t * irc)
{
    int i;
    char * channel;

    channel = irc->message.params.list[2]; /* 353 nickname = #channel */

    irc->channels = realloc(irc->channels, sizeof (irc->channels) * irc->channels_siz+1);
    irc->channels[irc->channels_siz] = strdup(channel);
    irc->channels_siz++;

    for (i = 0; i < irc->channels_siz; i++)
        puts(irc->channels[i]);
}

static void control_command_part(struct irc_t * irc)
{
    int i, j;
    char ** new_channels;
    char * channel;

    channel = irc->message.params.list[2];

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
static void control_protocol_commands (struct irc_t * irc)
{
    if ( !strncmp ("PRIVMSG", irc->message.command, 7) ) 
        control_command_privmsg (irc);
    else if ( !strncmp ("PING", irc->message.command, 4))  
        snprintf (irc->response, MAX_IRC_MSG, "PONG %s\r\n", irc->message.trailing);
    else if ( !strncmp ("001", irc->message.command, 3) ) {
        char message[MAX_IRC_MSG];
        char ** t;

        for (t = irc->session->channels_ajoin; *t != NULL; t++) {
            irc_join_channel(*t, message);
            network_send_message(&irc->session->network, message);
        }
    } else if ( !strncmp("NOTICE", irc->message.command, 6) ) {
        if (strstr(irc->message.trailing, "registered" ) ) {
            char message[MAX_IRC_MSG];

            fprintf(stderr, "Auth to nickserv request received.\n");
            if (strcmp(irc->session->password, "")) {
                fprintf(stderr, "Authing to nickserv\n");
                irc_identify_to_auth(irc->session->password, message);
                network_send_message(&irc->session->network, message);
            }
        }
    } else if (strstr(irc->message.command, "353")) {
        control_command_353_join(irc);

    } else if (strstr(irc->message.command, "PART")) {
        control_command_part(irc);
    }
}


/* Main entry point */
void irc_process_line(struct irc_t * irc, const char * line)
{  
    irc_parser(&irc->message, line);
    strcpy(irc->request, irc->message.trailing);
    print_message_t(&irc->message);

    control_protocol_commands(irc);

    if (irc->response[0])
        fprintf(stderr, "%s\n", irc->response);
}
