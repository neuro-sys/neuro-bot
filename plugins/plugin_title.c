#include "plugin_client.h"

#include "utils/curl_wrapper.h"

#include <libxml/HTMLparser.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char * keywords[10] = { "http", "https", "spotify:track" };

#ifdef _MSC_VER
#define COMPARE(a, b) (!stricmp((a), (b)))
#else
#define COMPARE(a, b) (!strcasecmp((a), (b)))
#endif

static char title_buffer[512];
static int title_len = 0;
static int in_title = 0;

static int validate_http(char * line)
{
    char * ret;
    size_t z;

    ret = strstr(line, "http");
    if (!ret)
        return -1;

    z = strcspn(ret, " \r\n");

    ret[z] = 0;

    return 0;
}


static void    
startTag (void * ctx, 
          const xmlChar * name, 
          const xmlChar ** atts)
{
    if (COMPARE((char *) name, "TITLE"))
        in_title = 1;
}

static void 
endTag (void * ctx, 
        const xmlChar * name)
{
    if (COMPARE((char *) name, "TITLE"))
        in_title = 0;
}

static void
characters_callback (void * ctx, 
            const xmlChar * ch, 
            int len)
{
    char * t;
    size_t offset;

    if (!in_title)
        return;
    title_len += len;
    if (len > MAX_IRC_MSG)
        return;

    t = (char *) ch;
    while (*t) { if (*t == '\n' || *t == '\t') *t = ' '; t++; }
    offset = strspn((char *) ch, " \t\r\n");
    strcat(title_buffer, (char *) ch + offset);
    fprintf(stderr, "%s", title_buffer);
}

static char * make_tinyurl(char * url)
{
    struct http_req * http = NULL;
    char tinyurl_url[2048];
    char * response = NULL;

    tinyurl_url[0] = 0;

    sprintf(tinyurl_url, "http://tinyurl.com/api-create.php?url=");
    sprintf(tinyurl_url + strlen(tinyurl_url), "%s", url);

    http = curl_perform(tinyurl_url, NULL);

    if (http == NULL || http->body == NULL)
        return NULL;

    response = strdup(http->body);

    free(http->body);
    free(http->header);
    free(http);

    return response;
}

struct plugin_t * plugin;

int adapt_spotify_hotlink(char * trailing)
{
    char spotify_id[50];


    if (strstr(trailing, "spotify:track") == NULL) {
        return 0;
    }

    sscanf(trailing, "spotify:track:%s", spotify_id);
    snprintf(trailing, 512, "http://play.spotify.com/track/%s", spotify_id);

    return 1;
}

void run(void)
{
    struct http_req * http;
    char reply_msg[MAX_IRC_MSG];
    htmlSAXHandler saxHandler;
    htmlParserCtxt * ctx_ptr;
    struct curl_slist * slist = NULL;
    char request_header_str[256];
    char trailing_str[510];
    int is_spotify_hotlink = 0;

    title_buffer[0] = 0;
    title_len = 0;
    in_title = 0;
    trailing_str[0] = 0;
    reply_msg[0] = 0;

    strcpy(trailing_str, plugin->irc->message.trailing);

    is_spotify_hotlink = adapt_spotify_hotlink(trailing_str);

    if (strstr(trailing_str, "youtube.com") != NULL) {
        return;
    }

    memset(&saxHandler, 0, sizeof (htmlSAXHandler));

    saxHandler.startElement = startTag;
    saxHandler.endElement = endTag;
    saxHandler.characters = characters_callback;
    saxHandler.cdataBlock = characters_callback;

    if (validate_http(trailing_str) < 0 )
        return;

    snprintf(request_header_str, 256, "Accept: text/plain, text/html");
    slist = curl_slist_append(slist, request_header_str);
    http = curl_perform(trailing_str, slist);
    if (!http->body) return;


    ctx_ptr = htmlCreatePushParserCtxt(&saxHandler, NULL, "", 0, "", XML_CHAR_ENCODING_UTF8);
    htmlParseChunk(ctx_ptr, http->body, http->body_len, 0);
    htmlParseChunk(ctx_ptr, "", 0, 1);
    
    htmlFreeParserCtxt(ctx_ptr);

    if (is_spotify_hotlink) {
        size_t span_first_dash = strcspn(title_buffer,  "-");
        char temp[512];

        snprintf(temp, 512, "%s", title_buffer + span_first_dash + 2);
        snprintf(title_buffer, 512, "%s", temp);
    }

    if (title_len > 1)
        sprintf(reply_msg, "Title: %s", title_buffer);

    free(http->header);
    free(http->body);
    free(http);

    if (reply_msg[0]) {
        char response[512];
        char * tinyurl = NULL;

        sprintf(response, "PRIVMSG %s :%s", plugin->irc->from, reply_msg);

        if ( (tinyurl = make_tinyurl(trailing_str)) != NULL) {
            strcat(response, " - ");
            strcat(response, tinyurl);
            free(tinyurl);
        }

        plugin->send_message(plugin->irc, response);
    }

}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "title";
    plugin->is_daemon  = 0;
    plugin->is_command = 0;
    plugin->is_grep    = 1;

    return plugin;
}

