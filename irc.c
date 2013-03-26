#include "irc.h"
#include "global.h"
#include "py_wrap.h"

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "module.h"

static void irc_proc_cmd_privmsg_user_cmd_admin (struct irc_t * irc)
{
    char ** tokens;

    tokens = g_strsplit_set (irc->request, " ", 2);

    if ( !strncmp(".join", tokens[0], strlen(".join")) )
    {
        sprintf(irc->response, "JOIN %s\r\n", tokens[1]);
    } 
    else if (!strncmp(".part", tokens[0], strlen(".part")))
    {
        sprintf(irc->response, "PART %s\r\n", tokens[1] );
    }
    else if ( !strncmp(".privmsg", tokens[0], strlen(".privmsg")) )
    {
        sprintf(irc->response, "%s", tokens[1]);
    }
    else if ( !strncmp(".reload", tokens[0], strlen(".reload")) )
    {
        module_load();
        py_load_mod_hash();
    }

    g_strfreev (tokens);
}

static void irc_proc_cmd_privmsg_user_cmd (struct irc_t * irc)
{
    char ** tokens;
    struct mod_c_t * mod_c;
    char * t;

    tokens = g_strsplit_set (irc->request, " \r\n", 2);

    t = strdup(tokens[0]);
    if ( (mod_c = module_find(++t)) )
    {
        char * ret = mod_c->func(irc);
        

        if (ret)
        {
            snprintf(irc->response, MAX_MSG, "PRIVMSG %s :%s\r\n", irc->from, ret);

            free(ret);
        }
    }
    else
    {
        char * ret;
        struct py_module_t * mod = py_find_loaded_name (tokens[0]); 

        if (mod)
        {
            ret = py_call_module ( mod, irc );
            snprintf( irc->response, MAX_MSG , "PRIVMSG %s :%s\r\n", irc->from, ret );

            free(ret);
        }
    }

    if ( !strncmp (irc->admin, irc->nick_to_msg, strlen(irc->admin)) ) {
        irc_proc_cmd_privmsg_user_cmd_admin (irc);
    }

    g_strfreev(tokens);
}

static void irc_proc_cmd_privmsg (struct irc_t * irc)
{
    char ** tokens;

    tokens = g_strsplit (irc->srv_msg.prefix, "!", 2);
    strcpy (irc->from, irc->srv_msg.params);
    strcpy (irc->nick_to_msg, tokens[0]);

    if ( irc->request[0] == '.' ) 
    {
        irc_proc_cmd_privmsg_user_cmd (irc);
    } 
    else if ( g_strrstr (irc->request, "http:") || g_strrstr(irc->request, "https:") )
    {

        struct mod_c_t * mod;
        char * ret;

        if ( g_strrstr (irc->request, "youtube.com") || g_strrstr(irc->request, "youtu.be") )
        {
            mod = module_find("youtube");
        }
        else
        {

            mod = module_find("title");
        }

        ret = mod->func(irc); 

        if (ret)
        {
            snprintf(irc->response, MAX_MSG, "PRIVMSG %s :%s\r\n", irc->from, ret);

            free(ret);
        }
    }  

    g_strfreev (tokens);
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
        snprintf (irc->response, strlen(irc->response), "PONG %s\r\n", irc->request);
    }
}

/*     message    =  [ ":" prefix SPACE ] command [ params ] crlf */
static int irc_parse_prefix (struct irc_t * irc, char * line)
{
    char ** tokens;

    tokens = g_strsplit (line, " ", 4); 
    
    if (!tokens)
        return -1;

    strcpy (irc->srv_msg.prefix, tokens[0]+1); /* skip ':' from the prefix */
    strcpy (irc->srv_msg.command, tokens[1]);

    /* if the 3rd token does not start with a ':', there are params */
    if ( tokens[2][0] != ':' ) 
    { 

        /* this copies only the first param */
        strcpy(irc->srv_msg.params, tokens[2]);         

        /* the rest is request and optional */
        if ( tokens[3] != NULL ) 
            strcpy (irc->request, tokens[3]+1);

    } 
    else 
    {
        strcpy (irc->request, tokens[2]+1);
    }

    g_strfreev (tokens);

    return 1;
}

static void irc_parse_other (struct irc_t * irc, char * line)
{
    char ** tokens;

    tokens = g_strsplit (line, ":", 2);
    if ( !strncmp("PING", tokens[0], 4) ) 
    {
        strcpy (irc->srv_msg.command, tokens[0]);
        strcpy (irc->request, tokens[1]);
    }

    g_strfreev(tokens);
}

/* message    =  [ ":" prefix SPACE ] command [ params ] crlf */
void irc_process_line(struct irc_t * irc, char * line)
{  
    g_printerr(line);

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

