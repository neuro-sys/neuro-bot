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
        fprintf(stderr, "%25s:%4d:No config file found, using defaults.\n", __FILE__, __LINE__);
        session->nickname = NICKNAME;
        session->password = PASSWORD;
        session->admin = ADMIN;
        session->socket.host_name = HOST;
        session->socket.port = PORT;

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
            fprintf(stderr, "%25s:%4d:nickname         : %s\n", __FILE__, __LINE__, session->nickname);
        }
        else if (!strcmp(token, "pass"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->password = strdup(token);
            else session->password = PASSWORD;
            fprintf(stderr, "%25s:%4d:password         : %s\n", __FILE__, __LINE__, session->password);
        }
        else if (!strcmp(token, "admin"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->admin = strdup(token);
            else session->admin = ADMIN;
            fprintf(stderr, "%25s:%4d:admin            : %s\n", __FILE__, __LINE__, session->admin);
        }
        else if (!strcmp(token, "server"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->socket.host_name = strdup(token);
            else session->socket.host_name = HOST;
            fprintf(stderr, "%25s:%4d:host_name        : %s\n", __FILE__, __LINE__, session->socket.host_name);
        }
        else if (!strcmp(token, "port"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->socket.port = strdup(token); 
            else session->socket.port = PORT;
            fprintf(stderr, "%25s:%4d:port             : %s\n", __FILE__, __LINE__, session->socket.port);
        }
        else if (!strcmp(token, "channels"))
        {
            int i = 0;
            fprintf(stderr, "%25s:%4d:Autojoin channels:", __FILE__, __LINE__);
            while ( (token = strtok(NULL, ",\n")) != NULL)
            {
                if (i) fprintf(stderr, ", ");
                if (token) {
                        session->channels_ajoin[i] = strdup(token);
                        fprintf(stderr, "\"%s\"", session->channels_ajoin[i]);
                        i++;
                }

            }
            fprintf(stderr, "\n");
            session->channels_ajoin[i] = '\0';
        }
    }
    fclose(file);
}

