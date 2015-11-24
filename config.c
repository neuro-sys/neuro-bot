#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CONFIG_FILE "neurobot.conf"

void config_free(struct irc_t * irc)
{
    struct ajoin_channel_t * iterator;

    LIST_FOREACH(iterator, &irc->ajoin_channels_head, list) {
        free(iterator->channel_name);
        free (iterator);
    }

    free(irc->nickname);
    free(irc->password);
    free(irc->admin);

    free(irc->hostname);
    free(irc->port);
}

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
        irc->hostname = HOST;
        irc->port = PORT;

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
            if (token) irc->hostname = strdup(token);
            else irc->hostname = HOST;
            debug("host_name        : %s\n", irc->hostname);
        }
        else if (!strcmp(token, "port"))
        {
            token = strtok(NULL, " \r\n");
            if (token) irc->port = strdup(token);
            else irc->port = PORT;
            debug("port             : %s\n", irc->port);
        }
        else if (!strcmp(token, "channels"))
        {
            char print_buffer[2048];

            LIST_INIT(&irc->ajoin_channels_head);

            while ((token = strtok(NULL, ",\n")) != NULL) {
                struct ajoin_channel_t * ajoin_channel = malloc(sizeof *ajoin_channel);
                ajoin_channel->channel_name = strdup(token);
                LIST_INSERT_HEAD(&irc->ajoin_channels_head, ajoin_channel, list);
            }

            sprintf(print_buffer, "Autojoin channels: ");
            struct ajoin_channel_t * iterator;
            LIST_FOREACH(iterator, &irc->ajoin_channels_head, list) {
                sprintf(print_buffer + strlen(print_buffer), "%s ", iterator->channel_name);
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

