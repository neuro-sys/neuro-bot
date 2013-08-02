#include "session.h"

#include "socket.h"
#include "irc.h"
#include "plugin.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void session_init_irc(struct session_t * session)
{
    char message[MAX_IRC_MSG];

    irc_set_nick(session->nickname, message);
    socket_send_message(&session->socket, message);

    irc_set_user("ircbot", "github.com/neuro-sys/neuro-bot", message);
    socket_send_message(&session->socket, message);
}


void session_run(struct session_t * session)
{ 
    char          line[MAX_IRC_MSG];
    struct irc_t  irc;

    memset(&irc, 0, sizeof(irc));
    irc.session = session;        
    plugin_attach_context(&irc);

    /* Conect to the server specified in socket_t struct. */
    if ( socket_connect(&irc.session->socket) < 0 ) {
        debug("Unable to connect to %s:%s\n", irc.session->socket.host_name, irc.session->socket.port);
        exit(EXIT_SUCCESS);
    }
    /* Do one time initialization work after connecting to the server. */
    session_init_irc(irc.session);

    plugin_start_loopers(&irc);

    while (1) 
    {

        irc.response[0] = 0;
        irc.from[0]     = 0;
        memset(&irc.message, 0, sizeof (irc.message));

        if (socket_read_line(&irc.session->socket, line) < 0)
            break;

        irc_process_line(&irc, line);

        if (irc.response[0])
            socket_send_message(&irc.session->socket, irc.response);
    }
}

