#ifndef __CURL_WRAP_H
#define __CURL_WRAP_H

struct http_req {
    char * base;
    char * header[10][2]; /* request header map */
    char * body;
};

extern struct http_req * curl_perform(char * url);
#endif

