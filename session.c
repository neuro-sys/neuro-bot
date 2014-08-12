#include "session.h"

#include "socket.h"
#include "irc.h"
#include "plugin.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void session_init_irc(struct irc_t * irc, struct session_t * session)
{
    char message[MAX_IRC_MSG];

    irc_set_nick(irc, session->nickname, message);

    irc_set_user(irc, "ircbot", "github.com/neuro-sys/neuro-bot", message);

    plugin_start_loopers(irc);
}


int session_run(struct session_t * session)
{ 
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
    session_init_irc(&irc, irc.session);

    while (666) 
    {
        char line[MAX_IRC_MSG];

        if (socket_read_line(&irc.session->socket, line) < 0) /* blocking io */
            return -1;

        irc_process_line(&irc, line);
    }
}

