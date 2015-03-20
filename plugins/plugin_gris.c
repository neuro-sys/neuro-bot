#include "plugin_client.h"

#include "../socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define     GRIS_HOST "localhost"
#define     GRIS_PORT "45678" 

char * keywords[10] = { "http", "https" };

struct plugin_t * plugin;

static void gris_search(char * url, char * dest, size_t max)
{
    char gris_response[512];
    struct socket_t socket;

    gris_response[0] = 0;

    socket.host_name = GRIS_HOST;
    socket.port = GRIS_PORT;

    socket_connect(&socket);
    socket_send_message(&socket, url);
    socket_read_line(&socket, gris_response);
    socket_close(&socket);

    if (strlen(gris_response) == 0) {
        return;
    }
    snprintf(dest, max, "%s", gris_response);
}

static void extract_url(char * trailing, char * dest, size_t max)
{
    char * begin = NULL;
    char url[512];
    size_t len;

    begin = strstr(trailing, "http");
    if (begin == NULL) {
        return;
    }

    len = strcspn(begin, " \r\n");

    strncpy(url, begin, len);
    url[len] = 0; 

    snprintf(dest, max, "%s", url);
}

void run(void)
{
    char response[512];
    char url[512];
    char image_description[512];

    image_description[0] = 0;
    url[0] = 0;
    response[0] = 0; 

    extract_url(plugin->irc->message.trailing, url, 512);

    if (url[0] == 0) {
        sprintf(response, "PRIVMSG %s :URL can't be read.", plugin->irc->from);
        plugin->send_message(plugin->irc, response);
        return;
    }

    gris_search(url, image_description, 512);
    if (image_description[0] == 0) {
        return;
    }

    sprintf(response, "PRIVMSG %s :(Image is likely: %s)", plugin->irc->from, image_description);
    plugin->send_message(plugin->irc, response);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "gris";
    plugin->is_daemon  = 0;
    plugin->is_command = 0;
    plugin->is_grep    = 1;

    int ret = system("java -jar plugins/gris.jar &");
    debug("gris.jar status: %d\n", ret);
    return plugin;
}

