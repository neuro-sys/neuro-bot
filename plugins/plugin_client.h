#ifndef __PLUGIN_CLIENT_H
#define __PLUGIN_CLIENT_H

#include "../irc.h"
#include "../global.h"
#include "../argv.h"

#include "../queue.h"

struct plugin_t {
    /* Attributes initialized by the plugin. */
    char * name;                  /* Plugin name, and the command name for command plugins. */
    char ** keywords;             /* Used if is_grep is 1. */

    int is_daemon;                /* runs as a daemon in its own thread */
    int is_command;               /* executed when trailing contains (*name) prefixed with `.' */
    int is_grep;                  /* executed when trailing contains one of the (** keywords) */
    int is_manager;               /* Manager plugins manage their own repository of */

    /* Handles provied to the plugin. Initialized by the bot. */
    void (*send_message)(struct irc_t *, char * message);   /* This is for the plugin to send message to server */
    struct irc_t * irc;

    /* Handles provied to the bot. Initialized by the plugin. */
    void (*run)(void);            /* This is for the bot to start the plugin to run */
    int (*manager_find)(char *);  /* Used if is_manager = 1. */

    LIST_ENTRY(plugin_t) plugin_slist;
};

/* Interface the plugin should implement. */
extern struct plugin_t * init(void);

#endif
