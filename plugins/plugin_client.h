#ifndef __PLUGIN_CLIENT_H
#define __PLUGIN_CLIENT_H

#include "../irc.h"
#include "../global.h"
#include "../argv.h"

#include "../queue.h"

struct plugin_t {
    char * name;                  /* Plugin name, and the command name for command plugins. */
    char ** keywords;             /* Used if is_grep is 1. */

    enum plugin_type { PLUGIN_TYPE_DAEMON = 1,
                       PLUGIN_TYPE_COMMAND = 1 << 2,
                       PLUGIN_TYPE_GREP = 1 << 3} type;

    /* Handles provied to the plugin. Initialized by the bot. */
    void (*send_message)(struct irc_t *, char * message);   /* This is for the plugin to send message to server */
    struct irc_t * irc;

    /* Handles provied to the bot. Initialized by the plugin. */
    void (*run)(int type);            /* This is for the bot to start the plugin to run */

    LIST_ENTRY(plugin_t) plugin_slist;
};

/* Interface the plugin should implement. */
extern struct plugin_t * init(void);

#endif
