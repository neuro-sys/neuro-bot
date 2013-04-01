#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "neurobotapi.h"

char * mod_time(struct irc_t * irc)
{
    char * ret;
    time_t now;

    ret = malloc(510);

    time(&now);
    sprintf(ret, "%s", ctime(&now));

    return ret;
}
