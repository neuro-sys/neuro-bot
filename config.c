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
    if (!file) 
    {	
        fprintf(stderr, "No config file found, using defaults.	\n");
        session->nickname = NICKNAME;
        session->password = PASSWORD;
        session->admin = ADMIN;
        session->network.host_name = HOST;
        session->network.port = PORT;

        return;
    }
    while (fgets(buf, 1024, file))
    {
        token = strtok(buf, "=");
        if (!strcmp(token, "nick"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->nickname = strdup(token);
            else session->nickname = NICKNAME;
            fprintf(stderr, "nickname: %s\n", session->nickname);
        }
        else if (!strcmp(token, "pass"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->password = strdup(token);
            else session->password = PASSWORD;
            fprintf(stderr, "password: %s\n", session->password);
        }
        else if (!strcmp(token, "admin"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->admin = strdup(token);
            else session->admin = ADMIN;
            fprintf(stderr, "admin: %s\n", session->admin);
        }
        else if (!strcmp(token, "server"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->network.host_name = strdup(token);
            else session->network.host_name = HOST;
            fprintf(stderr, "host_name: %s\n", session->network.host_name);
        }
        else if (!strcmp(token, "port"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->network.port = strdup(token); 
            else session->network.port = PORT;
            fprintf(stderr, "port: %s\n", session->network.port);
        }
        else if (!strcmp(token, "channels"))
        {
            int i = 0;
            fprintf(stderr, "Autojoin channels:\n");
            while ( (token = strtok(NULL, ",\n")) != NULL)
            {
                if (token) {
                        session->channels_ajoin[i] = strdup(token);
                        fprintf(stderr, "%s\n", session->channels_ajoin[i]);
                        i++;
                }
            }
            session->channels_ajoin[i] = '\0';
        }
    }
    fclose(file);
}

