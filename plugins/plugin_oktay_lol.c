#include "plugin_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

struct plugin_t * plugin;

static char * get_param(char * trailing)
{
    char * param= strchr(trailing, ' ');
    if (param == NULL || *++param == 0) {
        return NULL;
    }

    return param;
}

void run(void)
{
    char response[512], request[512], buffer[512];
    int n;
    char *level = NULL;

    response[0] = request[0] = buffer[0] = 0;

    level = get_param(plugin->irc->message.trailing);

    if (level == NULL) { // second param for the level of happiness in a scale of 10 to 500
        level = "300";
    }

    srand(time(NULL));

    n = rand() % atoi(level);
    if (n >= 300) n = 300; 

    while (n--) {
        int i;
        char hoi[] = "hoi";

        for (i = 0; i < sizeof(hoi); i++) {
            hoi[i] = rand() % 2 ? toupper(hoi[i]) : hoi[i];
        }

        snprintf(buffer + strlen(buffer), 512, "%s", hoi);
    }

    sprintf(response, "PRIVMSG %s :%s", plugin->irc->from, buffer);

    plugin->send_message(plugin->irc, response);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "oktay";
    plugin->is_looper  = 0;
    plugin->is_command = 1;
    plugin->is_grep    = 0;

    return plugin;
}

