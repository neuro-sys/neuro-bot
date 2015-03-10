#include "plugin_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

struct plugin_t * plugin;

static char * get_param(char * trailing)
{
    char * param= strchr(trailing, ' ');
    if (param == NULL || *++param == 0) {
        return NULL;
    }

    return param;
}

#define MAX_HOI_NUM 50 // 3 * X <=~ 500
static void lolz(char *buffer, size_t size, int level)
{
    int n = level;
    size_t written = 0;
    char hoi[] = "hohiohoih";
    char fmt[] = {'%', '.', '1', 's', '\0'};
    const size_t hoilen = strlen(hoi);
    while (n-- && written < size) {
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
}

static void highlight_channel(char *buffer, size_t size, struct channel_t *channel)
{
    char *buf = buffer;
    char **nick = channel->users;
    if (!channel->users)
        return;
    size_t written = 0;

    while (written < size && *nick) {
        char *n = *nick;
        nick++;
        if (!strcmp(plugin->irc->nickname, n))
            continue;
        int s = snprintf(buf, size - written, "%s ", n);
        written += s;
        buf += s;
    }
}

static void greet(char *buffer, size_t size, const char *name)
{
    const char *greetings[] = {
        "sen ne guzel bir insansin %s",
        "gotunu esek siksin %s",
        "abin senin billigini yesin %s",
        "ben geldim %s",
    };
    const size_t num_greetings = (sizeof greetings) / (sizeof greetings[0]);

    int index = rand() % num_greetings;

    snprintf(buffer, size, greetings[index], name);
}

static time_t s_last_highlight = 0;
#define HIGHLIGHT_MIN_INTERVAL (60*30)

void run(void)
{
    char response[512], request[512], buffer[512];
    long n = MAX_HOI_NUM;
    char *param = NULL;
    struct channel_t *channel = NULL;

    response[0] = request[0] = buffer[0] = 0;

    param = get_param(plugin->irc->message.trailing);
    if (param != NULL) {
        n = strtol(param, NULL, 10);
        if (n == 0 ) {
            greet(buffer, 450, param);
        } else {
            if (n <= 0 || n > MAX_HOI_NUM) n = MAX_HOI_NUM;
            lolz(buffer, 450, n);
        }
        sprintf(response, "PRIVMSG %s :%s", plugin->irc->from, buffer);

        plugin->send_message(plugin->irc, response);
    } else if (plugin->irc->from[0] == '#' &&
               time(NULL) - s_last_highlight > HIGHLIGHT_MIN_INTERVAL ) {
        struct channel_t **c = plugin->irc->channels_v;
        s_last_highlight = time(NULL);
        while (*c) {
            if (!strcmp((*c)->name, plugin->irc->from)) {
                channel = *c;
                break;
            }
        }
        int count = 3 + (rand() % 5);
        int i;
        highlight_channel(buffer, 450, channel);
        for (i = 0; i < count; ++i) {
            sprintf(response, "PRIVMSG %s :%s", plugin->irc->from, buffer);

            plugin->send_message(plugin->irc, response);
            sleep(1);
        }
    }

}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "oktay";
    plugin->is_daemon  = 0;
    plugin->is_command = 1;
    plugin->is_grep    = 0;

    return plugin;
}

