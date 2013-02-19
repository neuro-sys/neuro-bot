#include <stdio.h>
#include <glib.h>
#include <string.h>
#include "session.h"
#include "channel.h"
#include "user.h"
#include "global.h"

int main(int argc, char *argv[])
{
  struct session_t * session;
  
  log_init(G_LOG_LEVEL_DEBUG);

  session = session_create("irc.freenode.net", 6667);

  session_run(session, "\\0x00", "c3po");

  session_destroy(session);

  return 0;
}
