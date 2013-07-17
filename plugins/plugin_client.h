#ifndef __PLUGIN_CLIENT_H
#define __PLUGIN_CLIENT_H

#include "../irc.h"

struct plugin_t {
    char * name;                  /* Plugin name, and the command name for command plugins. */

    void (*send_message)(struct irc_t *); /* This is for the plugin to send message to server */
    void (*run)(struct irc_t *);  /* This is for the bot to start the plugin to run */

    char ** keywords;             /* Used by grep plugins. */

    int is_looper;                /* Runs in their own thread, are not trigger. */
    int is_command;               /* Triggered by users. */
    int is_grep;                  /* Triggered by grepping all lines for (char **) keywords. */
    int is_manager;               /* Manager plugins manage their own repository of */
                                  /* plugins. A manager plugin can load other plugins. */
    int (*manager_find)(char *);  /* Used if is_manager = 1. */
};

/* Interface the plugin should implement. */
extern struct plugin_t * init(void);

#endif
