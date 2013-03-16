#include "session.h"
#include "channel.h"
#include "user.h"
#include "global.h"
#include "config.h"
#include "py_wrap.h"

#include <stdio.h>
#include <glib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    struct session_t * session;
    gchar  * server, * nick, * pass;
    gint   port;

    config_init();

    if ( py_load_modules() < 0 )
        g_printerr("Could not load python modules, going on without them.\n");

    log_init(G_LOG_LEVEL_ERROR);

    server = config_get_string(GROUP_CLIENT, KEY_SERVER);
    if (!server)
        server = g_strdup("irc.freenode.net");

    port = config_get_integer(GROUP_CLIENT, KEY_PORT);
    if (!port)
        port = 6667;

    session = session_create(server, port);

    nick = config_get_string(GROUP_CLIENT, KEY_NICK);
    if (!nick)
        nick = g_strdup("cafer");
    pass = config_get_string(GROUP_CLIENT, KEY_PASS);
    if (!pass)
        pass = g_strdup("");

    session_run(session, nick, pass);

    g_free(nick);
    g_free(pass);
    g_free(server);

    session_destroy(session);

    return 0;
}
