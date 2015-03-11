#include "curl_wrapper.h"

#include <curl/curl.h>

#include <stdlib.h>
#include <string.h>

static size_t header_callback(void * ptr, size_t size, size_t nmemb, void * userdata)
{
    struct http_req * http;

    http = (struct http_req *) userdata;

    http->header = realloc(http->header, http->header_len + size * nmemb + 1);
    memcpy(http->header + http->header_len, ptr, size * nmemb);
    http->header_len += size * nmemb;
    http->header[http->header_len] = 0;
    
    return size * nmemb;
}

static size_t body_callback(void * contents, size_t size, size_t nmemb, void * userp)
{
    struct http_req * http;

    http = (struct http_req *) userp;

    http->body = realloc(http->body, http->body_len + size * nmemb + 1);
    memcpy(http->body + http->body_len, contents, size * nmemb);
    http->body_len += size * nmemb;
    http->body[http->body_len] = 0;

#define MAX_FETCH_BYTES 1 * 1024 * 1024  
    if (http->body_len > MAX_FETCH_BYTES) {
        fprintf(stderr, "Passed the body size limit\n");
        return -1;
    }
#undef MAX_FETCH_BYTES
    return size * nmemb;
}

struct http_req * curl_perform(const char * url, struct curl_slist * slist)
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
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.3; rv:36.0) Gecko/20100101 Firefox/36.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, body_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, http);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, http);
    if (NULL != slist) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
    }
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return http;
}
