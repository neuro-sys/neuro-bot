#ifndef __PLUGIN_CLIENT_H
#define __PLUGIN_CLIENT_H

struct plugin_t {
    char * name;

    void (*send_message)(char *); /* This is for the plugin to send message to server */
    void (*run)(char *, char *); /* This is for the bot to start the plugin to run */

    char ** keywords;
    int is_looper;
    int is_command;
    int is_grep;
    int is_manager;

    int (*manager_find)(char *);
};

#endif
