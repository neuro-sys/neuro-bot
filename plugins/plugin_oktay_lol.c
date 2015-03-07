#include "plugin_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

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
#define MAX_HOI_NUM 50 // 3 * X <=~ 500
    char response[512], request[512], buffer[512];
    long n = MAX_HOI_NUM;
    char *level = NULL;

    response[0] = request[0] = buffer[0] = 0;

    level = get_param(plugin->irc->message.trailing);
    if (level != NULL) { 
        n = strtol(level, NULL, 10);
        if (n <= 0 || n > MAX_HOI_NUM) n = MAX_HOI_NUM;
    }


    size_t written = 0;
    char hoi[] = "hohiohoih";
    char fmt[] = {'%', '.', '1', 's', '\0'};
    const size_t hoilen = strlen(hoi);
    fprintf(stderr, "oktay -- n: %ld hoilen: %zu\n", n, hoilen);
    while (n-- && written < 450) {
        int i;
        int start = rand() % 5;
        int end = hoilen-1;

        for (i = start; i < end; i++) {
            hoi[i] = rand() & 1 ? toupper(hoi[i]) : tolower(hoi[i]);
        }

        const size_t lenpos = 2;
        size_t hl = end - start;
        fmt[lenpos] = '0' + hl;

        snprintf(buffer + written, hl, fmt, hoi);;
        written += hl - 1;
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

