#include "session.h"
#include "channel.h"
#include "user.h"
#include "network.h"
#include "irc.h"
#include "global.h"
#include "config.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct session_t {
  struct network_t *  network;
  GSList *            channel_list;
  char *              nickname;
};

struct network_t * session_get_network(struct session_t * session)
{
  return session->network;
}

void session_run(struct session_t * session, char * nick, char * pass)
{ 
  char * line;
  struct irc_t irc;

  gchar *admin = config_get_string(GROUP_CLIENT, KEY_ADMIN);
  if (!admin) {
    admin = g_strdup(""); /* wut? */
    g_warning("No admin in config file?");
  }

  network_auth(session->network, nick, "ircbot", pass);

  while (1) {
    memset(&irc, 0, sizeof irc);
    irc.admin = admin;
    if ( network_read_line(session->network, &line) < 0 )
		continue;
    irc_process_line(&irc, line);
    if ( irc.response != NULL )
      network_send_message(session->network, irc.response);
      g_free(line);
  }

  g_free(admin);
}

struct session_t * session_create(char * host, int port)
{ 
  struct session_t * session = malloc(sizeof * session);
  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  if (!session) return NULL;
  session->network = network_connect(host, port);
  if (!session->network) return NULL;
  return session;
}

void session_add_channel(struct session_t * session, char * name)
{ 
  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  session->channel_list = g_slist_append(session->channel_list, channel_create(name));
}

struct channel_t * session_channel_find_by_name(struct session_t * session, char * name)
{
  guint i;

  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
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
  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  g_slist_free_full(session->channel_list, (void (*)(void *)) &channel_destroy);
  free(session);
}

void session_print_channels(struct session_t * session)
{
  g_debug("%u\t%s\t\t%s", __LINE__, __FILE__, __func__);
  channels_print(session->channel_list, stdout);
}

