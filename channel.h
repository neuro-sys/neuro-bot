#ifndef __CHANNEL_H_
#define __CHANNEL_H_

#include "queue.h"

LIST_HEAD(channel_list_t, channel_t);

struct user_t {
    char * name;
    LIST_ENTRY(user_t) list;
};

struct channel_t {
    char * name;
    LIST_HEAD(, user_t) user_list_head;
    LIST_ENTRY(channel_t) list;
};

struct channel_t * channel_new(char * name);
void channel_free(struct channel_t * channel);
void channel_add_user(struct channel_t * channel, char * user);
void channel_remove_user(struct channel_t * channel, char * user);
struct channel_t * channel_find(struct channel_list_t * head, char * name);

#endif

