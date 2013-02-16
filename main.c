#include <stdio.h>
#include <glib.h>
#include <string.h>
#include "session.h"
#include "channel.h"
#include "user.h"
#include "network.h"


int main(int argc, char *argv[])
{
  struct session_t * session = session_create();
  struct network_t * network;

  session_channel_add(session, "#archlinux-tr");
  session_channel_add(session, "##penis");

  channel_user_add(session_channel_find_by_name(session, "#archlinux-tr"), user_create("neuro_sys"));
  channel_user_add(session_channel_find_by_name(session, "#archlinux-tr"), user_create("mrcan"));
  channel_user_add(session_channel_find_by_name(session, "#archlinux-tr"), user_create("emrahnzm"));
  channel_user_add(session_channel_find_by_name(session, "#archlinux-tr"), user_create("aykut"));

  channel_user_add(session_channel_find_by_name(session, "##penis"), user_create("ras0ir"));
  channel_user_add(session_channel_find_by_name(session, "##penis"), user_create("decaf"));

  session_channel_print(session);

  network = network_connect("irc.freenode.net", 6667);

  network_auth(network, "sabribeyler", "localhost", "ve sabribeyler");

  while (1) {
    char * msg;
    int len;
  
    len = network_read_line(network, &msg);

    printf("%s", msg);

    g_free(msg);
  }

  session_destroy(session);

  return 0;
}
