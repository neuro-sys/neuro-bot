#include "../config.h"

#include "../session.h"

#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    struct session_t session;

    fprintf(stderr, "*** Test for config.c\n");
    config_load(&session);
    fprintf(stderr, "*** OK?\n");
    return 0;
}

