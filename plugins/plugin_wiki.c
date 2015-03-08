#include "plugin_client.h"

#include "utils/curl_wrapper.h"
#include "utils/json.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void n_strip_tags(char * dest, char * src)
{
    int inside = 0;

    while (*src != '\0')
    {
        if (*src == '>') {
            inside = 0;
            src++;
        } 
        if (*src == '<' || inside) {
            inside = 1;
            src++;
            continue;
        }
        *dest++ = *src++;
    }
    *dest = '\0';
}

static char * parse_json_wiki(char * data)
{
    char * temp;
    json_value * root, * extract;

    temp = malloc(MAX_IRC_MSG);

    root = json_parse(data);
    if (!root) {
        free(temp);
      return NULL;
    }

    extract = n_json_find_object(root, "extract");
    if (!extract) {
        free(temp);
        return NULL;
    }

    strncpy(temp, extract->u.string.ptr, MAX_IRC_MSG);

    json_value_free(root);

    return temp;
}


struct plugin_t * plugin;

void run(void)
{
    char * p, * t, reply_message[MAX_IRC_MSG];
    struct http_req * http;
    char url_path[512];
    char request[512];

    strcpy(request, plugin->irc->message.trailing);

    /* strip the leading command prefix (.wiki) */
    p = strchr(request, ' ');

    if (!p++)
        return;
    
    /* strip the trailing cr-lf */
    p[strcspn(p, "\r\n")] = '\0';

    t = p;

    while ((t = strchr(t, ' ')) != NULL)
        *t = '+';

    sprintf(url_path, "http://en.wikipedia.org/w/api.php?action=query&prop=extracts&exchars=350&titles=%s&format=json&redirects", p);

    http = curl_perform(url_path, NULL);

    t = http->body;
    while ( (t = strstr(t, "\\n")) != NULL) {
        t[0] = '-';
        t[1] = ' ';
    }

    p = parse_json_wiki(http->body);
   
    free(http->header);
    free(http->body);
    free(http);

    if (!p) {
        char response[512];

        sprintf(response, "PRIVMSG %s :Wiki article could not be found.", plugin->irc->from);
        return;
    }

    n_strip_tags(reply_message, p);

    free(p);

    {
        char response[512];
        sprintf(response, "PRIVMSG %s :Wikipedia: %s", plugin->irc->from, reply_message);
        plugin->send_message(plugin->irc, response);
    }
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "wiki";
    plugin->is_daemon  = 0;
    plugin->is_command = 1;
    plugin->is_grep    = 0;

    return plugin;
}

