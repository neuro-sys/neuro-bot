#include <stdio.h>
#include <glib.h>
#include <string.h>
#include "session.h"
#include "channel.h"
#include "user.h"

#define IRCDEBUG

int main(int argc, char *argv[])
{
  struct session_t * session = session_create("irc.freenode.net", 6667, "mybot", "mypass");

  session_run(session);

  session_destroy(session);

  return 0;
}
