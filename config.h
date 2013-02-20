#ifndef __CONFIG_H
#define __CONFIG_H

#include "global.h"
#include <glib.h>

#define GROUP_CLIENT "client"

#define KEY_NICK "nick"
#define KEY_PASS "pass"
#define KEY_SERVER "server"
#define KEY_PORT "port"
#define KEY_ADMIN "admin"

void config_init(void);
void config_uninit(void);

gchar* config_get_string(const gchar *group, const gchar *key);
gint config_get_integer(const gchar *group, const gchar *key);

#endif
