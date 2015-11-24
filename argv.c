#include "argv.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct argv_s * argv_parse(char * str)
{
    struct argv_s * param;
    char tokenize_buffer[1024];
    char * token;
    struct argv_t * argv;

    param = malloc(sizeof (struct argv_s));
    memset(param, 0, sizeof(struct argv_s));

    snprintf(tokenize_buffer, sizeof tokenize_buffer, "%s", str);

    token = strtok(tokenize_buffer, " ");
    if (token == NULL) {
        return NULL;
    }

    TAILQ_INIT(&param->argv_list);

    argv = malloc(sizeof *argv);
    argv->value = strdup(token);
    TAILQ_INSERT_TAIL(&param->argv_list, argv, list);

    while ((token = strtok(NULL, " ")) != NULL) {
        argv = malloc(sizeof *argv);
        argv->value = strdup(token);
        TAILQ_INSERT_TAIL(&param->argv_list, argv, list);
    }

    return param;
}

void argv_free(struct argv_s * arg)
{
    struct argv_t * iterator, * temp;

    temp = NULL;
    TAILQ_FOREACH(iterator, &arg->argv_list, list) {
        free(temp);
        free(iterator->value);
        temp = iterator;
    }
    free(temp);
    free(arg);
}
