#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CONFIG_FILE "neurobot.conf"

void config_load(struct session_t * session)
{
    FILE * file;
    char buf[1024];
    char * token;

    file = fopen(CONFIG_FILE, "r");

    while (fgets(buf, 1024, file))
    {
        token = strtok(buf, "=");
        if (!strcmp(token, "nick"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->nickname = strdup(token);
            else session->nickname = "neurobot";
        }
        else if (!strcmp(token, "pass"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->password = strdup(token);
            else session->password = "";
        }
        else if (!strcmp(token, "admin"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->admin = strdup(token);
            else session->admin = "neuro_sys";
        }
        else if (!strcmp(token, "server"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->network.host_name = strdup(token);
            else session->network.host_name = "irc.freenode.net";
        }
        else if (!strcmp(token, "port"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->network.port = atoi(token);
            else session->network.port = 6667;
        }

    }

    fclose(file);
}

