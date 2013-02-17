#include "session.h"
#include "channel.h"
#include "user.h"
#include "network.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct session_t {
  struct network_t *  network;
  GSList *            channel_list;
  char *              nickname;
};

void session_run(struct session_t * session, char * nick, char * pass)
{
  char * line;

  network_auth(session->network, nick, "ircbot", "ircbot");

  while (1) {
    network_read_line(session->network, &line);
    irc_process_line(session->network, line);
    g_free(line);
  }
}

struct session_t * session_create(char * host, int port)
{
  struct session_t * session = malloc(sizeof * session);
  if (!session) return NULL;
  session->network = network_connect(host, port);
  if (!session->network) return NULL;
  return session;
}

void session_add_channel(struct session_t * session, char * name)
{
  session->channel_list = g_slist_append(session->channel_list, channel_create(name));
}

struct channel_t * session_channel_find_by_name(struct session_t * session, char * name)
{
  int i;

  for (i = 0; i < g_slist_length(session->channel_list); i++) {
    struct channel_t * channel = g_slist_nth_data(session->channel_list, i);

    if (!strcmp(channel_get_name(channel), name))
      return channel;
  }

  return NULL;
}

struct channel_t * session_channel_remove_by_name(struct session_t * session, char * name)
{
  struct channel_t * channel = session_channel_find_by_name(session, name);
  session->channel_list = g_slist_remove(session->channel_list, channel);
  return channel;
}

void session_destroy(struct session_t * session)
{
  g_slist_free_full(session->channel_list, (void (*)(void *)) &channel_destroy);
  free(session);
}

void session_print_channels(struct session_t * session)
{
  channels_print(session->channel_list, stdout);
}

