#include "global.h"

#include "irc.h"
#include "config.h"
#include "plugin.h"

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

struct irc_t irc;

void
termination_handler (int signum)
{
    fprintf(stdout, "Terminating application.\n");
    plugin_free();
    irc_free(&irc);
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, termination_handler);
    setlocale(LC_CTYPE, "");
    memset(&irc, 0, sizeof(struct irc_t));
    config_load(&irc);
    plugin_init(&irc);
    while ( irc_run(&irc) > 0 ) { }
    return 0;
}

