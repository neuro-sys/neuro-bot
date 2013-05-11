#include "neurobotapi.h"
#include "json.h"
#include "curl_wrap.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char * api_url = "http://api.ipinfodb.com/v3/ip-city/?key="
                 "0312bdbd898c56675f30f9239ff380a1165946795e94be5074bc57f1870749b0"
                 "&ip=%s&format=json";

void json(char * dest, char * content)
{
    json_value * root, * country, * city;

    root = json_parse(content);
    if (!root) return;

    country = n_json_find_object(root, "countryName");
    city = n_json_find_object(root, "cityName");
   
    snprintf(dest, MAX_IRC_MSG, "[%s - %s]", 
                country->u.string.ptr, 
                city->u.string.ptr);

    json_value_free(root);
}

char * parse_ip(char * line)
{
    char * t;

    t = strtok(line, " ");
    if (!t) return NULL;
    t = strtok(NULL, " \r\n");
    
    return t;
}

#ifdef _WIN32
__declspec(dllexport)
#endif
char * mod_whereis(struct irc_t * irc, char * reply_msg)
{
    struct http_req * http;
    char url[500];
    char * ip;

    ip = parse_ip(irc->request);
    if (!ip) return NULL;

    snprintf(url, 500, api_url, ip);
    http = curl_perform(url);
    json(reply_msg, http->body);
    free(http->base);
    free(http);
}
