#ifndef __PLUGIN_H
#define __PLUGIN_H

#include "plugins/plugin_client.h"

#include <sys/queue.h>

SLIST_HEAD(plugin_slist_head, plugin_slist_t) plugin_slist_head; 
struct plugin_slist_t {
    struct plugin_t * plugin;
    SLIST_ENTRY(plugin_slist_t) plugin_slist;
};

void plugin_insert(struct plugin_t * p);
void plugin_init();
struct plugin_t ** plugin_find_commands(char * name, struct plugin_t *** p_plugin_commands_v);
void plugin_start_loopers(struct irc_t * irc);
void plugin_attach_context(struct irc_t * irc);


#endif

