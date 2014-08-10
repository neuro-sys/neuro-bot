#include "global.h"

#include "session.h"
#include "config.h"
#include "plugin.h"

#include <stdio.h>
#include <string.h>
#include <locale.h>

int main(int argc, char *argv[])
{
    struct session_t session;

    memset(&session, 0, sizeof session);

    /* Acquire system locale. */
    setlocale(LC_CTYPE, "");

    /* Load config file. */
    config_load(&session);

    /* Load plugin modules. */
    plugin_init();

    while ( session_run(&session) > 0 ) {
        /* loop until exit */
    }

    return 0;
}

