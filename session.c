#include "session.h"
#include "channel.h"
#include "user.h"
#include "network.h"
#include "irc.h"
#include "global.h"
#include "config.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void session_run(struct session_t * session, char * nick, char * pass)
{ 
    char          * line;
    struct irc_t  irc;
    int           quit = 0;

    gchar *admin = config_get_string(GROUP_CLIENT, KEY_ADMIN);
    if (!admin) {
        admin = g_strdup(""); /* wut? */
        g_warning("No admin in config file?");
    }

    network_auth(&session->network, nick, "ircbot", pass);

    while (!quit) {
        memset(&irc, 0, sizeof irc);
        
        irc.admin = admin;
        
        if ( network_read_line(&session->network, &line) < 0 )
            quit = 1;

        irc_process_line(&irc, line);
        
        if ( irc.response != NULL )
            network_send_message(&session->network, irc.response);
        
        g_free(line);
    }

    g_free(admin);
}

void session_create(struct session_t * session, char * host, int port)
{ 
    network_connect(&session->network, host, port);
        
    session->run = &session_run;
}

void session_destroy(struct session_t * session)
{

}

