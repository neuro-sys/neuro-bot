#include "../utils.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

char * body = "  <foo></foo> <tiTLE 1293809;'\">TEST TITLE</title>";

int test_n_get_tag_value(void)
{
    char * tag = "title";
    char * ret;

    fprintf(stderr, "*** Testing n_get_tag_value\n");
    ret = n_get_tag_value(strdup(body), tag);
    assert(ret == NULL || !strcmp(ret, "TEST TITLE"));
    fprintf(stderr, "*** OK.\n");
    return 0;
}

int test_n_strip_tags()
{
    char * p;

    fprintf(stderr, "*** Testing n_strip_tags\n");
    p = malloc(strlen(body));
    n_strip_tags(p, body);
    assert(strchr(p, '<') == NULL && strchr(p, '>') == NULL);
    fprintf(stderr, "*** OK.\n");
    free(p);
}

int main(int argc, char *argv[])
{
    fprintf(stderr, "*** Test for utils.c\n");

    test_n_strip_tags();
    test_n_get_tag_value();

    return 0;
}

