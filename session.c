#include "session.h"
#include "network.h"
#include "irc.h"
#include "global.h"
#include "config.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void session_auth_to_network(struct session_t * session)
{
    char message[MAX_IRC_MSG];

    snprintf(message, sizeof message, "NICK %s\r\n" "USER %s 8 * :%s\r\n\r\n", 
            "neurobot", 
            "github.com/neuro-sys/neuro-bot", 
            session->nickname);

    network_send_message(&session->network, message);

    sprintf(message, "PRIVMSG NickServ :identify %s\r\n", session->password);
    network_send_message(&session->network, message);
}

static void session_run(struct session_t * session)
{ 
    char          * line;
    struct irc_t  irc;

    session_auth_to_network(session);

    while (1) 
    {
        memset(&irc, 0, sizeof irc);
        
        irc.admin = session->admin;
        
        if ( network_read_line(&session->network, &line) < 0 )
            break;

        irc_process_line(&irc, line);
        
        if ( irc.response != NULL )
            network_send_message(&session->network, irc.response);
    }
}


void session_create(struct session_t * session)
{ 
    network_connect(&session->network);
        
    session->run = &session_run;
}

void session_destroy(struct session_t * session)
{

}

