#include "channel.h"

#include <string.h>
#include <stdlib.h>


struct channel_t * channel_new(char * name)
{
    struct channel_t * channel = malloc(sizeof (struct channel_t));

    memset(channel, 0, sizeof (struct channel_t));
    channel->name = strdup(name);

    return channel;
}

struct channel_t * channel_find(struct channel_t ** head, char * name)
{
    struct channel_t ** iterator;

    for (iterator = head; *iterator != NULL; iterator++) {
        if (strcmp((*iterator)->name, name) == 0) {
            return *iterator;
        }
    }

    return NULL;
}

void channel_add_user(struct channel_t * channel, char * user)
{
    char ** iterator;
    int user_counter = 0;

    if (channel->users != NULL) {
        for (iterator = channel->users; *iterator != NULL; iterator++, user_counter++) {}
    }

    channel->users = realloc(channel->users, (user_counter+1) * sizeof (char *));
    channel->users[user_counter++] = strdup(user);

    channel->users = realloc(channel->users, (user_counter+1) * sizeof (char *));
    channel->users[user_counter] = NULL;
}

void channel_remove_user(struct channel_t * channel, char * user)
{
    char ** new_users_v = NULL, ** iterator;
    int user_counter = 0;


    for (iterator = channel->users; *iterator != NULL; iterator++) {
        if (strcmp(*iterator, user) == 0) {
            free(*iterator);
            continue;
        }

        new_users_v = realloc(new_users_v, (user_counter+1) * sizeof (char *));
        new_users_v[user_counter++] = *iterator;
    }

    new_users_v = realloc(new_users_v, (user_counter+1) * sizeof (char *));
    new_users_v[user_counter] = NULL;

    free(channel->users);
    channel->users = new_users_v;
}

void channel_free(struct channel_t * channel)
{
    char ** iterator;

    if (channel->users != NULL) {
        for (iterator = channel->users; *iterator != NULL; iterator++) {
            free(*iterator);
        } 
    }

    free(channel->users); 
    free(channel->name);
    free(channel);
}

#ifdef TEST_CHANNEL

void channel_print(struct channel_t * channel)
{
    char ** iterator;

    printf("Channel name: %s\n", channel->name);
    printf("Channel users:\n");
    for (iterator = channel->users; *iterator != NULL; iterator++) {
        printf("%s\n", *iterator);
    }

    return;
}

int main(int argc, char *argv[])
{
    struct channel_t * channel = channel_new("#archlinux-tr");

    channel_add_user(channel, "firat");
    channel_add_user(channel, "ahmet");
    channel_add_user(channel, "mehmet");
    channel_add_user(channel, "murat");

    channel_remove_user(channel, "mehmet");
    channel_remove_user(channel, "murat");
    channel_remove_user(channel, "firat");
    channel_remove_user(channel, "firat2");
    channel_remove_user(channel, "ahmet");

    channel_print(channel);

    channel_free(channel);
 
    return 0;
}
#endif


