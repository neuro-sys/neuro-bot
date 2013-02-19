#include "channel.h"
#include "user.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib.h>

struct channel_t {
  char *    name;
  GSList *  user_list;
};

void channel_add_user(struct channel_t * channel, struct user_t * user)
{
  channel->user_list = g_slist_append(channel->user_list, user);
}

struct user_t * channel_find_user_by_name(struct channel_t * channel, char * name)
{
  GSList * user = g_slist_find_custom(channel->user_list, name, (int (*)(const void *, const void*)) &strcmp);
  if (user) {
    return user->data;
  }
  return NULL;
}

struct user_t * channel_remove_user_by_name(struct channel_t * channel, char * name)
{
  struct user_t * user  = channel_find_user_by_name(channel, name);
  channel->user_list    = g_slist_remove(channel->user_list, user);
  return user;
}

struct channel_t * channel_create(char * name)
{
  struct channel_t * channel = malloc(sizeof * channel);
  if (channel) {
    channel->name = name;
  }
  return channel;
}

const char * channel_get_name(const struct channel_t * channel)
{
  return channel->name;
}

void channel_destroy(struct channel_t * channel)
{
  //g_slist_free_full(channel->user_list, (void (*)(void *)) &user_destroy);
  free(channel);
}

int channel_cmp_by_name(const void * c1, const void * c2)
{
  char chan1[255];
  char chan2[255];

  strcpy(chan1, channel_get_name(c1));
  strcpy(chan2, channel_get_name(c2));

  return g_ascii_strncasecmp (chan1 + strspn(chan1, "#"), chan2 + strspn(chan2, "#"), 100);
}

int channel_cmp_by_name_real(const void * c1, const void * c2)
{
  return g_ascii_strncasecmp(channel_get_name(c1), channel_get_name(c2), 100);
}

void channels_print(GSList * list, FILE * fd)
{
  int i;

  for (i = 0; i < g_slist_length(list); i++) {
    struct channel_t * channel_p = g_slist_nth_data(list, i);

    fprintf(fd, "Channel Name: ");
    fprintf(fd, "%s\n", channel_get_name(channel_p));

    user_print(channel_p->user_list, fd);
  }
}
