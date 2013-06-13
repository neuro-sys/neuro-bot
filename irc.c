#include "irc.h"

#include "global.h"
#include "py_wrap.h"
#include "module.h"
#include "irc_cmd.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* IRC PARSER */
static void parse_prefix_nickname(struct nickname_t * nickname, const char * src)
{
    char * c;
    size_t n;

    if (src == NULL)
        return;

    if ( (c = strchr(src, '@')) ) {
        char * k;
        size_t len;

        len = strcspn(src, " ");
        strncpy(nickname->host, c+1, len);
        nickname->host[len] = '\0';
        if ( (k = strchr(src, '!')) ) {
            strncpy(nickname->user, k+1, (c - k - 1));
            nickname->user[c - k - 1] = '\0';
        }
    } 
    n = strcspn(src, "!@ ");
    strncpy(nickname->nickname, src, n);
}

static void parse_prefix_servername(char * servername, const char * src)
{
    if (src == NULL)
        return;

    strcpy(servername, src);
}

/**
 * prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
 * Notes:
 * [21:26] < grawity> Server names always have a "." – nicknames never do.
 *
 */
static void parse_prefix(struct prefix_t * prefix, const char * src)
{
    if (src == NULL)
        return;

    if (!strchr(src, '@') && strchr(src, '.'))
        parse_prefix_servername(prefix->servername, src);
    else
        parse_prefix_nickname(&prefix->nickname, src);
}

/* 
 * message    =  [ ":" prefix SPACE ] command [ params ] crlf 
 *
 */
static void irc_parser(struct message_t * message, const char * line)
{
    char buffer[510];
    size_t n;

    if (line == NULL)
        return;

    /* If starts with ':', then has prefix. */
    if (line[0] == ':') {
        n = strcspn(line, " ");
        strncpy(buffer, line, n);
        buffer[n] = '\0';
        parse_prefix(&message->prefix, buffer+1);
        line += n + 1;
    } 

    /* Then get the command. */
    n = strcspn(line, " ");
    strncpy(message->command, line, n);
    message->command[n] = '\0';
    line += n+1;

    /* Get params if there's. */
    if ( (n = strcspn(line, " \r\n")) ) {
        int i = 0;
        char params[200];
        char * t;

        strcpy(params, line);
        /* Get the trailing if there is. */
        if ( (t = strchr(params, ':')) ) { 
            strcpy(message->trailing, t+1);
            *t = '\0';
        }
        t = strtok(params, " \r\n");
        strcpy(message->params.list[i++], t);
        while ( (t = strtok(NULL, " ")) && i <= 14) {
            strcpy(message->params.list[i++], t);
        }
        message->params.list[i][0] = '\0';
    }
}

void print_message_t(struct message_t * message)
{
    if (message->prefix.servername[0])
        printf("Serv: (%s) ", message->prefix.servername);
    else
        printf("Nick: (%s!%s@%s) ", message->prefix.nickname.nickname
                                , message->prefix.nickname.user
                                , message->prefix.nickname.host);

    if (message->command[0])
        printf("(%s) ", message->command);

    if (message->params.list[0][0]) {
        int i = 0;

        printf("{");
        while ( message->params.list[i][0] ) {
            printf("\"%s\"", message->params.list[i++]);
            if (message->params.list[i][0])
                printf(", ");
        }
        printf("}");
    }
    if (message->trailing[0])
        printf(" => %s", message->trailing);

    printf("\n");
}

/* Controllers */
static void control_admin_commands (struct irc_t * irc)
{
    char * t;
    char tokens[2][MAX_IRC_MSG];
    char str[MAX_IRC_MSG];

    strncpy(str, irc->message.trailing, MAX_IRC_MSG);
    
    if ( !(t = strtok(str, " ")) )
        return;

    strncpy(tokens[0], t, MAX_IRC_MSG);

    if ( (t = strtok(NULL, "\r\n")) )
        strncpy(tokens[1], t, MAX_IRC_MSG);
        
    if ( !strncmp(".join", tokens[0], strlen(".join")) )
        sprintf(irc->response, "JOIN %s\r\n", tokens[1]);
    else if (!strncmp(".part", tokens[0], strlen(".part")))
        sprintf(irc->response, "PART %s\r\n", tokens[1] );
    else if ( !strncmp(".raw", tokens[0], strlen(".raw")) )
        sprintf(irc->response, "%s\r\n", tokens[1]);
    else if ( !strncmp(".reload", tokens[0], strlen(".reload")) ) {
        module_load();
#ifdef USE_PYTHON_MODULES
        py_load_mod_hash();
#endif
    }
}

static void control_user_commands (struct irc_t * irc)
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
        control_admin_commands (irc);
}

static void control_message_line (struct irc_t * irc)
{
    strcpy(irc->from, irc->message.params.list[0]);
    strcpy(irc->nick_to_msg, irc->message.prefix.nickname.nickname);

    if (!strcmp(irc->message.params.list[0], irc->session->nickname))
        strcpy(irc->from, irc->message.prefix.nickname.nickname);

    if ( irc->request[0] == '.' ) 
        control_user_commands (irc);
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

static void control_protocol_commands (struct irc_t * irc)
{
    if ( !strncmp ("PRIVMSG", irc->message.command, 7) ) 
        control_message_line (irc);
    else if ( !strncmp ("PING", irc->message.command, 4))  
        snprintf (irc->response, MAX_IRC_MSG, "PONG %s\r\n", irc->message.trailing);
    else if ( !strncmp ("001", irc->message.command, 3) ) {
        char message[MAX_IRC_MSG];
        char ** t;

        for (t = irc->session->channels_ajoin; *t != NULL; t++) {
            irc_join_channel(*t, message);
            network_send_message(&irc->session->network, message);
        }
    } 
    else if ( !strncmp("NOTICE", irc->message.command, 6) 
            && strstr(irc->message.trailing, "registered" ) ) {
        char message[MAX_IRC_MSG];
            
        fprintf(stderr, "Auth to nickserv request received.\n");
        if (strcmp(irc->session->password, "")) {
            fprintf(stderr, "Authing to nickserv\n");
            irc_identify_to_auth(irc->session->password, message);
            network_send_message(&irc->session->network, message);
        }
    }
}


static void check_channel_join(struct irc_t * irc)
{
    int i;
    char * channel;

    if (!strstr(irc->message.command, "353"))
        return;

    channel = irc->message.params.list[0];

    irc->channels = realloc(irc->channels, sizeof (irc->channels) * irc->channels_siz+1);
    irc->channels[irc->channels_siz] = strdup(channel);
    irc->channels_siz++;

    for (i = 0; i < irc->channels_siz; i++)
        puts(irc->channels[i]);
}

static void check_channel_part(struct irc_t * irc)
{
    int i, j;
    char ** new_channels;
    char * channel;

    if (!strstr(irc->message.command, "PART"))
        return;

    channel = irc->message.params.list[0];

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

/* message    =  [ ":" prefix SPACE ] command [ params ] crlf */
void irc_process_line(struct irc_t * irc, char * line)
{  
    irc_parser(&irc->message, line);
    strcpy(irc->request, irc->message.trailing);
    print_message_t(&irc->message);

    check_channel_join(irc);
    check_channel_part(irc);
    control_protocol_commands(irc);

    if (irc->response[0])
        fprintf(stderr, "%s\n", irc->response);
}
