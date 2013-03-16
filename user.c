#include "user.h"
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>

struct user_t {
    char * name;
};

struct user_t * user_create(char * name)
{
    struct user_t * user = malloc(sizeof (struct user_t *));
    if (user) {
        user->name = name;
    }
    return user;
}

void user_destroy(struct user_t * user)
{
    free(user);
}

void user_print(GSList * user_list, FILE * fd)
{
    int i;

    for (i = 0; i < g_slist_length(user_list); i++) {
        struct user_t * user = g_slist_nth_data(user_list, i);
        fprintf(fd, "%d: %s\n", i, user->name);
    }
}


