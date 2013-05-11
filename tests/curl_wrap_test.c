#include "../modules/curl_wrap.h"

#include <stdio.h>

char *test_urls[] = { 
    "http://www.freenode.net/",
    NULL
};

int main(int argc, char *argv[])
{
    char ** p;

    fprintf(stderr, "*** Test for curl_wrap.c\n");

    for (p = test_urls; *p != NULL; p++) {
        fprintf(stderr, "*** Performing %s\n", *p);
        curl_perform(*p);
        fprintf(stderr, "*** OK.\n");
    }
    return 0;
}

