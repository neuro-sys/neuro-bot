#include <stdio.h>
#include <glib.h>
#include <string.h>
#include "session.h"
#include "channel.h"
#include "user.h"
#include "global.h"
#include "config.h"

  int main(int argc, char *argv[])
  {
    struct session_t * session;

    config_init();

    log_init(G_LOG_LEVEL_DEBUG);

    gchar *server = config_get_string(GROUP_CLIENT, KEY_SERVER);
    if (!server)
      server = g_strdup("irc.freenode.net");

    gint port = config_get_integer(GROUP_CLIENT, KEY_PORT);
    if (!port)
      port = 6667;

    session = session_create(server, port);

    gchar *nick = config_get_string(GROUP_CLIENT, KEY_NICK);
    if (!nick)
      nick = g_strdup("cafer");
    gchar *pass = config_get_string(GROUP_CLIENT, KEY_PASS);
    if (!pass)
      pass = g_strdup("");

  session_run(session, nick, pass);

  g_free(nick);
  g_free(pass);
  g_free(server);

  session_destroy(session);

  return 0;
}
