#include "global.h"

#include "session.h"
#include "config.h"
#include "plugin.h"

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <signal.h>

void
termination_handler (int signum)
{
    fprintf(stdout, "Received signum %d\n", signum); 
}

void
set_signal_handlers(void)
{
  struct sigaction new_action, old_action;

  new_action.sa_handler = termination_handler;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = 0;

  sigaction (SIGINT, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGINT, &new_action, NULL);
  sigaction (SIGHUP, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGHUP, &new_action, NULL);
  sigaction (SIGTERM, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGTERM, &new_action, NULL);
}

int main(int argc, char *argv[])
{
    struct session_t session;

    memset(&session, 0, sizeof session);

    set_signal_handlers();

    setlocale(LC_CTYPE, "");

    config_load(&session);

    plugin_init();

    while ( session_run(&session) > 0 ) {
        /* loop until exit */
    }

    return 0;
}

