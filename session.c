#include "session.h"
#include "channel.h"
#include "user.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct session_t {
	GSList * channel_list;
};

struct session_t * session_create(void)
{
	struct session_t * session = malloc(sizeof * session);
	return session;
}

void session_channel_add(struct session_t * session, char * name)
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

void session_channel_print(struct session_t * session)
{
	channels_print(session->channel_list, stdout);
}

