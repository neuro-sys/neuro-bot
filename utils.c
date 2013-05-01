#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <regex.h>
#include <stdio.h>

void n_strip_tags(char * dest, char * src)
{
    int inside = 0;

    while (*src != '\0')
    {

        if (*src == '>') {
            inside = 0;
            src++;
        } 

        if (*src == '<' || inside) {
            inside = 1;
            src++;
            continue;
        }

        *dest++ = *src++;
    }
    *dest = '\0';
}

/*
 * Returns a string that matches `<tagname*>...<' in body,
 * NULL if not found.
 * (?i)<TITLE>(.+?)</TITLE>
 */
char * n_get_tag_value(char * body, const char * tagname)
{
    char pattern[50];
    int status;
    regex_t re;
    regmatch_t pmatch[2];

    sprintf(pattern, "<%s.*>(.*)</%s>", tagname, tagname);
    if (regcomp(&re, pattern, REG_ICASE|REG_EXTENDED) != 0) {
        fprintf(stderr, "Coult not compile regex pattern: %s\n", pattern);
        return NULL;
    }
    if ((status = regexec(&re, body, 2, pmatch, 0)) != 0)
        return NULL;
    body[pmatch[1].rm_eo] = '\0';
    body += pmatch[1].rm_so;
    return body;
}
