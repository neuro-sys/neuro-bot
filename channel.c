#include "global.h"
#include "channel.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


struct channel_t * channel_new(char * name)
{
    struct channel_t * channel = malloc(sizeof (struct channel_t));

    memset(channel, 0, sizeof (struct channel_t));
    channel->name = strdup(name);

    LIST_INIT(&channel->user_list_head);

    return channel;
}

struct channel_t * channel_find(struct channel_list_t * head, char * name)
{
    struct channel_t * iterator;

    LIST_FOREACH(iterator, head, list) {
        if (strcmp(iterator->name, name) == 0) {
            return iterator;
        }
    }
    return NULL;
}

void channel_add_user(struct channel_t * channel, char * user)
{
    struct user_t * user_obj = malloc(sizeof *user_obj);
    user_obj->name = strdup(user);
    LIST_INSERT_HEAD(&channel->user_list_head, user_obj, list);
}

void channel_remove_user(struct channel_t * channel, char * user)
{
    struct user_t * iterator, * temp;

    temp = NULL;
    LIST_FOREACH(iterator, &channel->user_list_head, list) {
        free(temp);
        temp = NULL;
        if (strcmp(iterator->name, user) == 0) {
            LIST_REMOVE(iterator, list);
            free(iterator->name);
            temp = iterator;
        }
    }
    free(temp);
}

void channel_free(struct channel_t * channel)
{
    struct user_t * iterator;

    if (!channel) {
        return;
    }

    LIST_FOREACH(iterator, &channel->user_list_head, list) {
        channel_remove_user(channel, iterator->name);
    }

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


