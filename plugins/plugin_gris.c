#include "plugin_client.h"

#include "../socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define     GRIS_HOST "localhost"
#define     GRIS_PORT "45678" 

char * keywords[10] = { "http", "https" };

struct plugin_t * plugin;

static char * gris_search(char * url)
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
        return NULL;
    }
    
    return strdup(gris_response);; 
}

static char * extract_url(char * trailing)
{
    char * begin = NULL;
    char url[512];
    size_t len;

    begin = strstr(trailing, "http");
    if (begin == NULL) {
        return NULL;
    }

    len = strcspn(begin, " \r\n");

    strncpy(url, begin, len);
   
    return strdup(url); 
}

void run(void)
{
    char response[512];
    char * url = NULL;
    char * image_description = NULL;

    url = extract_url(plugin->irc->message.trailing);

    if (url == NULL) {
        sprintf(response, "PRIVMSG %s :URL can't be read.", plugin->irc->from);
        plugin->send_message(plugin->irc, response);
        return;
    }

    image_description = gris_search(url);
    if (image_description == NULL) {
        free(url);
        return;
    }

    sprintf(response, "PRIVMSG %s :(Image is likely: %s)", plugin->irc->from, image_description);
    plugin->send_message(plugin->irc, response);
    
    free(image_description);
    free(url);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "gris";
    plugin->is_looper  = 0;
    plugin->is_command = 0;
    plugin->is_grep    = 1;

    return plugin;
}

