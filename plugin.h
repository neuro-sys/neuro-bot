#ifndef __PLUGIN_H
#define __PLUGIN_H

#include "plugins/plugin_client.h"

void insert(struct plugin_t * p);
void plugin_init();
struct plugin_t * plugin_find_command(char * name);

#endif

