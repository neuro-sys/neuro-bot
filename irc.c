#include "irc.h"
#include "global.h"
#include "py_wrap.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "module.h"

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
    {
        sprintf(irc->response, "JOIN %s\r\n", tokens[1]);
    } 
    else if (!strncmp(".part", tokens[0], strlen(".part")))
    {
        sprintf(irc->response, "PART %s\r\n", tokens[1] );
    }
    else if ( !strncmp(".raw", tokens[0], strlen(".raw")) )
    {
        sprintf(irc->response, "%s\r\n", tokens[1]);
    }
    else if ( !strncmp(".reload", tokens[0], strlen(".reload")) )
    {
        module_load();
        py_load_mod_hash();
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
        char * ret = mod_c->func(irc);
        

        if (ret)
        {
            snprintf(irc->response, MAX_IRC_MSG, "PRIVMSG %s :%s\r\n", irc->from, ret);

            free(ret);
        }
    }
    else
    {
        char * ret;
        struct py_module_t * mod = py_find_loaded_name (token); 

        if (mod)
        {
            ret = py_call_module ( mod, irc );
            snprintf( irc->response, MAX_IRC_MSG , "PRIVMSG %s :%s\r\n", irc->from, ret );

            free(ret);
        }
    }

    if ( !strncmp (irc->session->admin, irc->nick_to_msg, strlen(irc->session->admin)) ) {
        irc_proc_cmd_privmsg_user_cmd_admin (irc);
    }
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

    strcpy (irc->from, irc->srv_msg.params);
    strcpy (irc->nick_to_msg, token);

    /* If the message sent in private, then reply to the sender instead */
    if (!strncmp(irc->srv_msg.params, irc->session->nickname, strlen(irc->srv_msg.params)))
        strcpy(irc->from, irc->nick_to_msg);

    if ( irc->request[0] == '.' ) 
    {
        irc_proc_cmd_privmsg_user_cmd (irc);
    } 
    else if ( strstr (irc->request, "http:") || strstr(irc->request, "https:") )
    {

        struct mod_c_t * mod;
        char * ret;

        ret = NULL;

        if ( strstr (irc->request, "youtube.com") || strstr(irc->request, "youtu.be") )
        {
            mod = module_find("youtube");
        }
        else
        {

            mod = module_find("title");
        }

        if (mod)
        {
            ret = mod->func(irc); 
        }

        if (ret)
        {
            snprintf(irc->response, MAX_IRC_MSG, "PRIVMSG %s :%s\r\n", irc->from, ret);

            free(ret);
        }
    }  
}

static void irc_proc_cmd (struct irc_t * irc)
{
    char * cmd;

    cmd = irc->srv_msg.command;

    if ( !strncmp ("PRIVMSG", cmd, strlen("PRIVMSG")) ) 
    {
        irc_proc_cmd_privmsg (irc);
    } 
    else if ( !strncmp ("PING", cmd, strlen("PING")) )
    {
        snprintf (irc->response, MAX_IRC_MSG, "PONG %s\r\n", irc->request);
    }
}

/*     message    =  [ ":" prefix SPACE ] command [ params ] crlf */
static int irc_parse_prefix (struct irc_t * irc, char * line)
{
    char tokens[4][MAX_IRC_MSG];
    char * t;

    tokens[0][0] = tokens[1][0] = tokens[2][0] = tokens[3][0] = '\0';

    if ( !(t = strtok(line, " ")) )
        return -1;

    strncpy(tokens[0], t, MAX_IRC_MSG);

    if ( !(t = strtok(NULL, " ")) )
        return -1;

    strncpy(tokens[1], t, MAX_IRC_MSG);

    if ( !(t = strtok(NULL, " ")) )
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
            strcpy (irc->request, tokens[3]+1);

    } 
    else 
    {
        strcpy (irc->request, tokens[2]+1);
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

/* message    =  [ ":" prefix SPACE ] command [ params ] crlf */
void irc_process_line(struct irc_t * irc, char * line)
{  
    fprintf(stderr, "%s\n", line);

    if ( line[0] == ':' ) 
    {
        if (irc_parse_prefix(irc, line) < 0)
            return;
    } 
    else 
    {
        irc_parse_other(irc, line);   
    }

    irc_proc_cmd(irc);
}

