#include "neurobotapi.h"
#include "curl_wrap.h"

#include <libxml/HTMLparser.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char * keywords[MAX_KEYWORDS] = { "http", "https" };

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
    /* This is the same as a switch-case, in case you don't know */
    t =   (p = strchr(ret, ' '))  ? p 
        : (p = strchr(ret, '\r')) ? p
        : (p = strchr(ret, '\n')) ? p
        : 0;
    if (t) *t = '\0'; else return -1;
    memmove(line, ret, strlen(ret) + 1);

    return 1;
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
}

#ifdef _WIN32
__declspec(dllexport)
#endif
void mod_title(struct irc_t * irc, char * reply_msg)
{
    struct http_req * http;
    char * t;
    htmlSAXHandler saxHandler;
    htmlParserCtxt * ctx_ptr;
    struct curl_slist * slist = NULL;
    char reqbuf[256];

    title_buffer[0] = 0;
    title_len = 0;
    in_title = 0;

    memset(&saxHandler, 0, sizeof (htmlSAXHandler));

    saxHandler.startElement = startTag;
    saxHandler.endElement = endTag;
    saxHandler.characters = characters_callback;
    saxHandler.cdataBlock = characters_callback;

    if (validate_http(irc->request) < 0 )
        return;

    snprintf(reqbuf, 256, "Accept: text/plain, text/html");
    slist = curl_slist_append(slist, reqbuf);
    http = curl_perform(irc->request, slist);
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

    return;
}


