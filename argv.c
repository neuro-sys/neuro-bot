#include "argv.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct argv_s * argv_parse(char * str)
{
    struct argv_s * param;
    char tokenize_buffer[1024];
    char * token;
    
    param = malloc(sizeof (struct argv_s));
    memset(param, 0, sizeof(struct argv_s));

    snprintf(tokenize_buffer, 1024, "%s", str);

    token = strtok(tokenize_buffer, " ");
    if (token == NULL) {
        return NULL;
    }

    param->argv = realloc(param->argv, (param->argc + 1) * sizeof (char *));
    param->argv[param->argc++] = strdup(token);

    while ((token = strtok(NULL, " ")) != NULL) {
        param->argv = realloc(param->argv, (param->argc + 1) * sizeof (char *));
        param->argv[param->argc++] = strdup(token);
    }

    param->argv = realloc(param->argv, (param->argc + 1) * sizeof (char *));
    param->argv[param->argc] = NULL;

    return param;
}

void argv_free(struct argv_s * arg)
{
    char ** iterator;

    for (iterator = arg->argv; *iterator != NULL; iterator++) {
        free(*iterator);
    }

    free(arg->argv);
    free(arg);
}
