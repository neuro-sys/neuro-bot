#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "neurobotapi.h"

#ifdef _WIN32
__declspec(dllexport)
#endif
void mod_time(struct irc_t * irc, char * reply_msg)
{
    char * ret;
    time_t now;

    time(&now);
    sprintf(reply_msg, "%s", ctime(&now));
}
