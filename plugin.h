#ifndef __PLUGIN_H
#define __PLUGIN_H

#include "plugins/plugin_client.h"

struct plugin_list_t * plugin_list_head;
struct plugin_list_t {
    struct plugin_t * cur;
    struct plugin_list_t * next;
};

void plugin_insert(struct plugin_t * p);
void plugin_init();
struct plugin_t ** plugin_find_commands(char * name, struct plugin_t ** plugin_list);

#endif

