#include "plugin_client.h"

#include "utils/curl_wrapper.h"

#include <libxml/HTMLparser.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char * keywords[10] = { "http", "https" };

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
    char * ret, * t, * p;

    ret = strstr(line, "http");
    if (!ret)
        return -1;

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
    offset = strspn(ch, " \t\r\n");
    strcat(title_buffer, (char *) ch + offset);
    fprintf(stderr, "%s", title_buffer);
}

struct plugin_t * plugin;

void run(void)
{
    struct http_req * http;
    char * t, reply_msg[MAX_IRC_MSG];
    htmlSAXHandler saxHandler;
    htmlParserCtxt * ctx_ptr;
    struct curl_slist * slist = NULL;
    char reqbuf[256];
    char trailing[510];

    strcpy(trailing, plugin->irc->message.trailing);
    title_buffer[0] = 0;
    title_len = 0;
    in_title = 0;

    memset(&saxHandler, 0, sizeof (htmlSAXHandler));

    saxHandler.startElement = startTag;
    saxHandler.endElement = endTag;
    saxHandler.characters = characters_callback;
    saxHandler.cdataBlock = characters_callback;

    if (validate_http(trailing) < 0 )
        return;

    snprintf(reqbuf, 256, "Accept: text/plain, text/html");
    slist = curl_slist_append(slist, reqbuf);
    http = curl_perform(trailing, slist);
    if (!http->body) return;


    ctx_ptr = htmlCreatePushParserCtxt(&saxHandler, NULL, "", 0, "", XML_CHAR_ENCODING_NONE);
    htmlParseChunk(ctx_ptr, http->body, http->body_len, 0);
    htmlParseChunk(ctx_ptr, "", 0, 1);
    
    htmlFreeParserCtxt(ctx_ptr);

    if (title_len > 1)
        sprintf(reply_msg, "Title: %s", title_buffer);

    free(http->header);
    free(http->body);
    free(http);

    sprintf(plugin->irc->response, "PRIVMSG %s :%s", plugin->irc->from, reply_msg);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "title";
    plugin->is_looper  = 0;
    plugin->is_command = 0;
    plugin->is_grep    = 1;

    return plugin;
}

