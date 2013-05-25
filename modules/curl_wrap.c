#include "curl_wrap.h"

#include <curl/curl.h>

#include <stdlib.h>
#include <string.h>

static size_t header_callback(void * ptr, size_t size, size_t nmemb, void * userdata)
{
    struct http_req * http;

    http = (struct http_req *) userdata;

    http->header = realloc(http->header, http->header_len + size * nmemb);
    memcpy(http->header + http->header_len, ptr, size * nmemb);
    http->header_len += size * nmemb;
    
    return size * nmemb;
}

static size_t body_callback(void * contents, size_t size, size_t nmemb, void * userp)
{
    struct http_req * http;

    http = (struct http_req *) userp;

    http->body = realloc(http->body, http->body_len + size * nmemb);
    memcpy(http->body + http->body_len, contents, size * nmemb);
    http->body_len += size * nmemb;

    if (http->body_len > 100000) {
        fprintf(stderr, "Passed the body size limit\n");
        return -1;
    }
    return size * nmemb;
}

struct http_req * curl_perform(char * url, struct curl_slist * slist)
{
    CURL * curl;
    struct http_req * http;

    http = malloc(sizeof (struct http_req));
    memset(http, 0, sizeof (struct http_req));

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL );
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "neuro_irc_bot");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, body_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, http);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, http);
    if (NULL != slist) {
        fprintf(stderr, "There are curl_slits's\n");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
    }
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return http;
}

