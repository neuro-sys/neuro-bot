#include "global.h"

#include "irc.h"
#include "config.h"
#include "plugin.h"

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <signal.h>

struct irc_t irc;

void
termination_handler (int signum)
{
    fprintf(stdout, "Received signum %d\n", signum); 
    plugin_free();
    irc_free(&irc);
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

    memset(&irc, 0, sizeof(struct irc_t));

//    set_signal_handlers();

    setlocale(LC_CTYPE, "");

    config_load(&irc);

    plugin_init();

    while ( irc_run(&irc) > 0 ) {
        /* loop until exit */
    }

    return 0;
}

