#ifndef __CURL_WRAP_H
#define __CURL_WRAP_H

#include <curl/curl.h>

struct http_req {
    char * header;
    int header_len;
    char * body;
    int body_len;
};

extern struct http_req * curl_perform(char * url, struct curl_slist * slist);

#endif
