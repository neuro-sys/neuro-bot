#include "global.h"
#include "irc.h"
#include "py_wrap.h"
#include "module.h"
#include "irc_cmd.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>



static void irc_proc_cmd_privmsg_user_cmd_admin (struct irc_t * irc)
{
    char * t;
    char tokens[2][MAX_IRC_MSG];
    char str[MAX_IRC_MSG];

    strncpy(str, irc->request, MAX_IRC_MSG);
    
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

static void irc_proc_cmd_privmsg_user_cmd (struct irc_t * irc)
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
        irc_proc_cmd_privmsg_user_cmd_admin (irc);
}

static void irc_proc_cmd_privmsg (struct irc_t * irc)
{
    char token[MAX_IRC_MSG];
    char str[MAX_IRC_MSG];
    char * t;

    strncpy(str, irc->srv_msg.prefix, MAX_IRC_MSG);

    if ( !(t = strtok(str, "!")) )
        return;
    
    strncpy(token, t, MAX_IRC_MSG);

    strcpy(irc->from, irc->srv_msg.params);
    strcpy(irc->nick_to_msg, token);

    /* If the message sent in private, then reply to the sender instead */
    if (!strncmp(irc->srv_msg.params, irc->session->nickname, strlen(irc->srv_msg.params)))
        strcpy(irc->from, irc->nick_to_msg);

    if ( irc->request[0] == '.' ) 
        irc_proc_cmd_privmsg_user_cmd (irc);
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

static void irc_proc_cmd (struct irc_t * irc)
{
    if ( !strncmp ("PRIVMSG", irc->srv_msg.command, strlen("PRIVMSG")) ) 
        irc_proc_cmd_privmsg (irc);
    else if ( !strncmp ("PING", irc->srv_msg.command, strlen("PING")) )
        snprintf (irc->response, MAX_IRC_MSG, "PONG %s\r\n", irc->request);
    else if ( !strncmp ("001", irc->srv_msg.command, strlen("001")) ) {
        char message[MAX_IRC_MSG];
        char ** t;

        for (t = irc->session->channels_ajoin; *t != NULL; t++) {
            irc_join_channel(*t, message);
            network_send_message(&irc->session->network, message);
        }
    } 
    else if ( !strncmp("NOTICE", irc->srv_msg.command, strlen("NOTICE")) 
            && strstr(irc->request, "registered" ) ) {
        char message[MAX_IRC_MSG];
            
        fprintf(stderr, "Auth to nickserv request received.\n");
        if (strcmp(irc->session->password, "")) {
            fprintf(stderr, "Authing to nickserv\n");
            irc_identify_to_auth(irc->session->password, message);
            network_send_message(&irc->session->network, message);
        }
    }
}

/*     message    =  [ ":" prefix SPACE ] command [ params ] crlf */
static int irc_parse_prefix (struct irc_t * irc, char * line)
{
    char tokens[4][MAX_IRC_MSG];
    char * t;

    tokens[0][0] = tokens[1][0] = tokens[2][0] = tokens[3][0] = '\0';

    /* prefix */
    if ( !(t = strtok(line, " ")) )
        return -1;
    strncpy(tokens[0], t, MAX_IRC_MSG);

    /* command */
    if ( !(t = strtok(NULL, " ")) )
        return -1;
    strncpy(tokens[1], t, MAX_IRC_MSG);

    /* params */
    if ( !(t = strtok(NULL, ":")) )
        return -1;

    strncpy(tokens[2], t, MAX_IRC_MSG);

    if ( (t = strtok(NULL, "")) )
        strncpy(tokens[3], t, MAX_IRC_MSG);

    strcpy (irc->srv_msg.prefix, tokens[0]+1); /* skip ':' from the prefix */
    strcpy (irc->srv_msg.command, tokens[1]);

    /* if the 3rd token does not start with a ':', there are params */
    if ( tokens[2][0] != ':' ) 
    { 

        /* this copies only the first param */
        strcpy(irc->srv_msg.params, tokens[2]);         

        /* the rest is request and optional */
        if ( tokens[3] != '\0' ) 
            strcpy (irc->request, tokens[3]);

    } 
    else 
    {
        strcpy (irc->request, tokens[2]);
    }

    return 1;
}

static void irc_parse_other (struct irc_t * irc, char * line)
{
    char tokens[2][MAX_IRC_MSG];
    char * t;

    if ( !(t = strtok(line, ":")) )
        return;

    strncpy(tokens[0], t, MAX_IRC_MSG);

    if ( !(t = strtok(NULL, "\r\n")) )
        return;

    strncpy(tokens[1], t, MAX_IRC_MSG);

    if ( !strncmp("PING", tokens[0], 4) ) 
    {
        strcpy (irc->srv_msg.command, tokens[0]);
        strcpy (irc->request, tokens[1]);
    }
}

void check_channel_join(struct irc_t * irc)
{
    char * t;
    char buf[250];
    int i;

    if (!strstr(irc->srv_msg.command, "353"))
        return;

    strcpy(buf, irc->srv_msg.params);

    t = strtok(buf, "=");
    t = strtok(NULL, "");

    irc->channels = realloc(irc->channels, sizeof (irc->channels) * irc->channels_siz+1);
    irc->channels[irc->channels_siz] = strdup(t+1);
    irc->channels_siz++;

    for (i = 0; i < irc->channels_siz; i++)
        puts(irc->channels[i]);
}

/* message    =  [ ":" prefix SPACE ] command [ params ] crlf */
void irc_process_line(struct irc_t * irc, char * line)
{  
    if ( line[0] == ':' ) {
        if (irc_parse_prefix(irc, line) < 0)
            return;
    } else {
        irc_parse_other(irc, line);   
    }

    check_channel_join(irc);

    fprintf(stderr, "%s => [%s] [%s] [%s] : %s\n", 
                                  irc->from,
                                  irc->srv_msg.prefix,
                                  irc->srv_msg.command, 
                                  irc->srv_msg.params,
                                  irc->request);
    irc_proc_cmd(irc);
    if (irc->response[0])
        fprintf(stderr, "%s\n", irc->response);
}

