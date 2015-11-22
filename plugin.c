#include "plugin.h"
#include "plugins/plugin_client.h"

#include "global.h"
#include "thread.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include <signal.h>
#include <dirent.h>
#include "dl.h"

#define PLUGIN_DIR "plugins"

LIST_HEAD(plugin_threads_head, plugin_threads_t) plugin_threads_head;
struct plugin_threads_t {
    thread_t thread;
    LIST_ENTRY(plugin_threads_t) plugin_threads;
};

static void *start_thread(void * pdata)
{
    ((struct plugin_t *)pdata)->run();

    return NULL;
}

void plugin_free()
{
    struct plugin_t * iterator;

    debug("Freeing plugin resources...\n");
    LIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator;

        debug("Freeing plugin: %s\n", plugin->name);
        free(plugin);
        free(iterator);
    }

    struct plugin_threads_t * iterator_threads;

    debug("Killing threads...\n");
    LIST_FOREACH(iterator_threads, &plugin_threads_head, plugin_threads) {
        thread_kill(iterator_threads->thread);
        free(iterator_threads);
    }
}

void plugin_start_daemons(struct irc_t * irc)
{

    int err;
    struct plugin_t * iterator;

    LIST_INIT(&plugin_threads_head);

    LIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator;
        struct plugin_threads_t * plugin_thread = malloc(sizeof(struct plugin_threads_t));

        memset(plugin_thread, 0, sizeof(struct plugin_threads_t));

        if (!plugin->is_daemon || plugin->is_manager)
            continue;

        debug("Looper plugin [%s] is started\n", plugin->name);

        if ((err = thread_create(&plugin_thread->thread, start_thread, plugin)) != 0) {
            debug("Thread could not be started. pthread_create, errno = %d\n", err);
        }

        LIST_INSERT_HEAD(&plugin_threads_head, plugin_thread, plugin_threads);
    }
}

void plugin_attach_context(struct irc_t * irc)
{
    struct plugin_t * iterator;

    debug("Session preparing plugins before connecting to the server.\n");
    LIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator;
        debug("Name: [%s]\n", plugin->name);
        plugin->irc = irc;
    }
}

struct plugin_t ** plugin_find_commands(char * name, struct plugin_t *** p_plugin_commands_v)
{
    struct plugin_t * iterator;
    int command_counter = 0;
    struct plugin_t ** plugin_commands_v;

    plugin_commands_v = *p_plugin_commands_v;

    LIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator;

        if (!plugin->is_command)
            continue;

        if (plugin->is_manager && !plugin->manager_find(name)) {
            plugin_commands_v = realloc(plugin_commands_v, (command_counter+1) * sizeof (struct plugin_t *));
            plugin_commands_v[command_counter++] = plugin;
        }

        if (!strcmp(plugin->name, name)) {
            plugin_commands_v = realloc(plugin_commands_v, (command_counter+1) * sizeof (struct plugin_t *));
            plugin_commands_v[command_counter++] = plugin;
        }
    }

    plugin_commands_v = realloc(plugin_commands_v, (command_counter+1) * sizeof (struct plugin_t *));
    plugin_commands_v[command_counter++] = NULL;

    *p_plugin_commands_v = plugin_commands_v;

    return plugin_commands_v;
}

void send_message(struct irc_t * irc, char * response)
{
    char buffer[4096];
    buffer[0] = 0;
    sprintf(buffer, "%s\r\n", response);
    socket_write(irc->sockfd, buffer, strlen(buffer));
}

struct plugin_t plugin_list[100];

void plugin_load_file(char * file)
{
    void * plugin_file;
    struct plugin_t * (*init)(void);
    struct plugin_t * plugin;

    debug("Loading native plugin \"%s\"\n", file);
    if ((plugin_file = dl_open(file)) == NULL) {
        debug("The plugin file \"%s\" could not be opened.\n", file);
        return;
    }

    if ((init = dl_sym(plugin_file, "init")) == NULL) {
        debug("The plugin \"%s\" has no exported init function symbol.\n", file);
    }

    /* initialize and get handle to the plugin */
    if ((plugin = init()) == NULL) {
        debug("The plugin initialization is failed for reasons unknown."
                " Hope it's last words were meaningful, if any.\n");
        return;
    }

    /* Attach callbacks to be used by plugin. */
    plugin->send_message = send_message;

    /* If plugin is of type grep, acquire grep keywords. */
    if (!plugin->is_manager && plugin->is_grep) {
        char ** keywords;

        if ((keywords = dl_sym(plugin_file, "keywords"))) {
            plugin->keywords = keywords;
        } else {
            debug("The plugin \"%s\" is of type `grep', but has no "
                  "exported grep keywords symbols found. Discarding.\n", file);
            return;
        }
    }

    LIST_INSERT_HEAD(&plugin_slist_head, plugin, plugin_slist);
}

void plugin_init()
{
    DIR * dir;
    struct dirent * dirent;

    LIST_INIT(&plugin_slist_head);

    dir = opendir(PLUGIN_DIR);

	if (!dir)
	{
		debug("No modules directory found. No plugins will be loaded.\n");
		return;
	}

    while ((dirent = readdir(dir)) != NULL)
    {
        if (strstr(dirent->d_name, ".so")) {
            char plugin_path[200];
            plugin_path[0] = 0;
            strcpy(plugin_path, PLUGIN_DIR);
            strcat(plugin_path, "/");
            strcat(plugin_path, dirent->d_name);
            plugin_load_file(plugin_path);
        }
    }

    closedir(dir);
}


#if TEST_PLUGIN
int main()
{
    struct plugin_slist_t * iterator;

    LIST_INIT(&plugin_threads_head);

    plugin_init();

    puts("Iterating the plugins...");
    LIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator->plugin;
        debug("%s\n", plugin->name);
    }
    puts("Finished.");

    return 0;
}
#endif

