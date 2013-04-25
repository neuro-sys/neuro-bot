#include "session.h"
#include "network.h"
#include "irc.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void session_init_irc(struct session_t * session)
{
    char message[MAX_IRC_MSG];
    char ** t;

    irc_set_nick(session->nickname, message);
    network_send_message(&session->network, message);

    irc_set_user("ircbot", "github.com/neuro-sys/neuro-bot", message);
    network_send_message(&session->network, message);

    if (!strcmp(session->password, "")) {
        irc_identify_to_auth(session->password, message);
        network_send_message(&session->network, message);
    }

    for (t = session->channels_ajoin; *t != NULL; t++) {
        irc_join_channel(*t, message);
        network_send_message(&session->network, message);
    }
}

static void session_run(struct session_t * session)
{ 
    char          line[MAX_IRC_MSG];
    struct irc_t  irc;

    session_init_irc(session);

    irc.session = session;        

    while (1) 
    {
        irc.response[0] = '\0';

        if ( network_read_line(&session->network, line) < 0 )
            break;

        irc_process_line(&irc, line);
        
        if (irc.response[0])
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

