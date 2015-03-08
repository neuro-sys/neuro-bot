#ifndef __CHANNEL_H_
#define __CHANNEL_H_

struct channel_t {
    char * name;
    char ** users;
};

struct channel_t * channel_new(char * name);
void channel_free(struct channel_t * channel);
void channel_add_user(struct channel_t * channel, char * user);
void channel_remove_user(struct channel_t * channel, char * user);
struct channel_t * channel_find(struct channel_t ** head, char * name);

#endif

