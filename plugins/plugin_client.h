#ifndef __PLUGIN_CLIENT_H
#define __PLUGIN_CLIENT_H

#include "../irc.h"

struct plugin_t {
    /* Attributes initialized by the plugin. */
    char * name;                  /* Plugin name, and the command name for command plugins. */
    char ** keywords;             /* Used by grep plugins. */

    int is_looper;                /* Runs in their own thread, are not trigger. */
    int is_command;               /* Triggered by users. */
    int is_grep;                  /* Triggered by grepping all lines for (char **) keywords. */
    int is_manager;               /* Manager plugins manage their own repository of */
                                  /* plugins. A manager plugin can load other plugins. */

    /* Handles provied to the plugin. Initialized by the bot. */
    void (*send_message)(struct irc_t *);   /* This is for the plugin to send message to server */
    struct irc_t * irc;

    /* Handles provied to the bot. Initialized by the plugin. */
    void (*run)(void);            /* This is for the bot to start the plugin to run */
    int (*manager_find)(char *);  /* Used if is_manager = 1. */

};

/* Interface the plugin should implement. */
extern struct plugin_t * init(void);

#endif
