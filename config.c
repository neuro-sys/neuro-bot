#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CONFIG_FILE "neurobot.conf"

void config_load(struct irc_t * irc)
{
    FILE * file;
    char buf[1024];
    char * token;

    file = fopen(CONFIG_FILE, "r");
    if (!file) 
    {	
        debug("No config file found, using defaults.\n");
        irc->nickname = NICKNAME;
        irc->password = PASSWORD;
        irc->admin = ADMIN;
        irc->socket.host_name = HOST;
        irc->socket.port = PORT;

        return;
    }
    while (fgets(buf, 1024, file))
    {
        if (buf[0] == '#')
            continue;

        token = strtok(buf, "=");

        if (token == NULL)
            continue;

        if (!strcmp(token, "nick"))
        {
            token = strtok(NULL, " \r\n");
            if (token) irc->nickname = strdup(token);
            else irc->nickname = NICKNAME;
            debug("nickname         : %s\n", irc->nickname);
        }
        else if (!strcmp(token, "pass"))
        {
            token = strtok(NULL, " \r\n");
            if (token) irc->password = strdup(token);
            else irc->password = PASSWORD;
            debug("password         : %s\n", irc->password);
        }
        else if (!strcmp(token, "admin"))
        {
            token = strtok(NULL, " \r\n");
            if (token) irc->admin = strdup(token);
            else irc->admin = ADMIN;
            debug("admin            : %s\n", irc->admin);
        }
        else if (!strcmp(token, "server"))
        {
            token = strtok(NULL, " \r\n");
            if (token) irc->socket.host_name = strdup(token);
            else irc->socket.host_name = HOST;
            debug("host_name        : %s\n", irc->socket.host_name);
        }
        else if (!strcmp(token, "port"))
        {
            token = strtok(NULL, " \r\n");
            if (token) irc->socket.port = strdup(token); 
            else irc->socket.port = PORT;
            debug("port             : %s\n", irc->socket.port);
        }
        else if (!strcmp(token, "channels"))
        {
            int channel_counter = 0;
            char print_buffer[2048];
            char ** channels_v = NULL;

            print_buffer[0] = 0;

            while ((token = strtok(NULL, ",\n")) != NULL)
            {
                char * channel = strdup(token);
       
                channels_v = realloc(channels_v, (channel_counter+1) * sizeof(char *));
                channels_v[channel_counter++] = channel;
            }
            channels_v = realloc(channels_v, (channel_counter+1) * sizeof(char *));
            channels_v[channel_counter++] = NULL;

            irc->channels_ajoin_v = channels_v;

            sprintf(print_buffer, "Autojoin channels: ");
            for(; *channels_v != NULL; channels_v++) {
                sprintf(print_buffer + strlen(print_buffer), "%s ", *channels_v);
            }
            sprintf(print_buffer + strlen(print_buffer), "\n");
            debug("%s", print_buffer);
        }
    }
    fclose(file);
}

#ifdef TEST_CONFIG
int main(int argc, char *argv[])
{
    struct irc_t irc;

    fprintf(stderr, "*** Test for config.c\n");
    config_load(&irc);
    fprintf(stderr, "*** OK?\n");
    return 0;
}

#endif

