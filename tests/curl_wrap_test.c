#include "../curl_wrap.h"

#include <stdio.h>

char *test_urls[] = { 
    "http://www.google.com/",
    "https://www.duckduckgo.com/",
    "http://www.freenode.net/",
};

int main(int argc, char *argv[])
{
    char ** p;

    fprintf(stderr, "*** Test for curl_wrap.c\n");

    for (p = test_urls; *p != NULL; p++)
        curl_perform(*p);
    return 0;
}

