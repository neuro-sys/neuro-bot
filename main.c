#include "global.h"

#include "session.h"
#include "config.h"
#include "module.h"

#include <stdio.h>
#include <string.h>
#include <locale.h>

int main(int argc, char *argv[])
{
    struct session_t session;

    setlocale(LC_CTYPE, "");
    memset(&session, 0, sizeof session);
    config_load(&session);
    module_init();

    while (1)
    {
        session_create(&session);
        session.run(&session);
    }

    session_destroy(&session);

    return 0;
}

