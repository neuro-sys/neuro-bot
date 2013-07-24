#include "plugin_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

struct plugin_t * plugin;

void run(void)
{
    time_t current_time, mesai_time;
    struct tm * mesai_time_tm;
    double diff;

    current_time = time(NULL);
    mesai_time_tm = localtime(&current_time);

    if (mesai_time_tm->tm_hour >= 18)
        mesai_time_tm->tm_mday += 1;

    mesai_time_tm->tm_hour = 18;
    mesai_time_tm->tm_min = 0;
    mesai_time_tm->tm_sec = 0;

    mesai_time = mktime(mesai_time_tm);

    diff = difftime(mesai_time, current_time);

    sprintf(plugin->irc->response, "PRIVMSG %s :%d dakika, %d saniye kaldi.\n", plugin->irc->from, (int) diff / 60, (int) diff % 60);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "mesai";
    plugin->is_looper  = 0;
    plugin->is_command = 1;
    plugin->is_grep    = 0;

    return plugin;
}

#ifdef TEST_PLUGIN_MESAI
int main(int argc, char *argv[])
{
    struct plugin_t p;
    struct irc_t irc;

    plugin = &p;
    strcpy(irc.from, "Hede");
    plugin->irc = &irc;

    run();

    debug(irc.response);

    return 0;
}

#endif

