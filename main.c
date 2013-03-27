#include "session.h"
#include "global.h"
#include "config.h"
#include "module.h"

#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <locale.h>

int main(int argc, char *argv[])
{
    struct session_t session;

    setlocale(LC_CTYPE, "");

    config_init();
    config_load(&session);
    module_init();
    log_init(G_LOG_LEVEL_ERROR);

    while (1)
    {
        session_create(&session);
        session.run(&session);
    }
    
    config_uninit();
    session_destroy(&session);

    return 0;
}
