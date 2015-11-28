#ifndef __PLUGIN_H
#define __PLUGIN_H

#include "plugins/plugin_client.h"

#include "queue.h"

LIST_HEAD(, plugin_t) plugin_slist_head;

void plugin_init(struct irc_t * irc);
struct plugin_list_t * plugin_find_commands(char * name, struct plugin_list_t * plugin_list_head);
void plugin_start_daemons(struct irc_t * irc);
void plugin_free(void);

#endif

