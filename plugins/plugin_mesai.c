#include "plugin_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

struct plugin_t * plugin;

static void format_message(double diff, char * dest, size_t len)
{
    char buffer[512];

    sprintf(buffer, "bitimine %d saat, %d dakika, %d saniye kaldi.",
            ((int) diff / 60) / 60,
            ((int) diff / 60) % 60,
            ((int) diff % 60));

    strncpy(dest, buffer, len);
}

void run(void)
{
    time_t current_time, mesai_time;
    struct tm * mesai_time_tm;
    double diff;
    char buffer[MAX_IRC_MSG];
    char response[512];

    current_time = time(NULL);
    mesai_time_tm = localtime(&current_time);

    if (mesai_time_tm->tm_hour >= 18)
        mesai_time_tm->tm_mday += 1;

    mesai_time_tm->tm_hour = 18;
    mesai_time_tm->tm_min = 0;
    mesai_time_tm->tm_sec = 0;

    mesai_time = mktime(mesai_time_tm);

    diff = difftime(mesai_time, current_time);

    format_message(diff, buffer, MAX_IRC_MSG);
    sprintf(response, "PRIVMSG %s :%s\r\n", plugin->irc->from, buffer);
    plugin->send_message(plugin->irc, response);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "mesai";
    plugin->is_daemon  = 0;
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

