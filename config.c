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
        debug("No config file found, using defaults.\n");
        session->nickname = NICKNAME;
        session->password = PASSWORD;
        session->admin = ADMIN;
        session->socket.host_name = HOST;
        session->socket.port = PORT;

        return;
    }
    while (fgets(buf, 1024, file))
    {
        if (buf == NULL || buf[0] == '#')
            continue;

        token = strtok(buf, "=");

        if (token == NULL)
            continue;

        if (!strcmp(token, "nick"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->nickname = strdup(token);
            else session->nickname = NICKNAME;
            debug("nickname         : %s\n", session->nickname);
        }
        else if (!strcmp(token, "pass"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->password = strdup(token);
            else session->password = PASSWORD;
            debug("password         : %s\n", session->password);
        }
        else if (!strcmp(token, "admin"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->admin = strdup(token);
            else session->admin = ADMIN;
            debug("admin            : %s\n", session->admin);
        }
        else if (!strcmp(token, "server"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->socket.host_name = strdup(token);
            else session->socket.host_name = HOST;
            debug("host_name        : %s\n", session->socket.host_name);
        }
        else if (!strcmp(token, "port"))
        {
            token = strtok(NULL, " \r\n");
            if (token) session->socket.port = strdup(token); 
            else session->socket.port = PORT;
            debug("port             : %s\n", session->socket.port);
        }
        else if (!strcmp(token, "channels"))
        {
            int i = 0;
            char print_buffer[2048];
            char temp[512];

            print_buffer[0] = 0;
            strcat(print_buffer, "Autojoin channels:");
            while ( (token = strtok(NULL, ",\n")) != NULL)
            {
                if (i) strcat(print_buffer, ", ");
                if (token) {
                        session->channels_ajoin[i] = strdup(token);
                        sprintf(temp, "\"%s\"", session->channels_ajoin[i]);
                        strcat(print_buffer, temp);
                        i++;
                }

            }
            strcat(print_buffer, "\n");
            debug(print_buffer);
            session->channels_ajoin[i] = '\0';
        }
    }
    fclose(file);
}

#ifdef TEST_CONFIG
int main(int argc, char *argv[])
{
    struct session_t session;

    fprintf(stderr, "*** Test for config.c\n");
    config_load(&session);
    fprintf(stderr, "*** OK?\n");
    return 0;
}

#endif

