#include "plugin.h"
#include "plugins/plugin_client.h"

#include "global.h"
#include "thread.h"
#include "dl.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <signal.h>
#include <dirent.h>

#define PLUGIN_DIR "plugins"

LIST_HEAD(plugin_threads_list_t, plugin_threads_t);
struct plugin_threads_t {
    thread_t thread;
    LIST_ENTRY(plugin_threads_t) list;
};

static struct plugin_threads_list_t plugin_threads_head;

static void *start_thread(void * pdata)
{
    ((struct plugin_t *)pdata)->run(0);
    return NULL;
}

void plugin_free()
{
    struct plugin_t * iterator, * temp;

    debug("Freeing plugin resources.\n");
    temp = NULL;
    LIST_FOREACH(iterator, &plugin_slist_head, list) {
        free(temp);
        temp = NULL;
        debug("Freeing plugin: %s\n", iterator->name);
        temp = iterator;
    }
    free(temp);

    struct plugin_threads_t * iterator_threads, * t_temp;

    debug("Killing threads.\n");
    t_temp = NULL;
    LIST_FOREACH(iterator_threads, &plugin_threads_head, list) {
        free(t_temp);
        thread_kill(iterator_threads->thread);
        t_temp = iterator_threads;
    }
    free(t_temp);
}

void plugin_start_daemons(struct irc_t * irc)
{
    int err;
    struct plugin_t * iterator;

    LIST_INIT(&plugin_threads_head);

    LIST_FOREACH(iterator, &plugin_slist_head, list) {
        struct plugin_t * plugin = iterator;
        struct plugin_threads_t * plugin_thread;

        if (!(plugin->type & PLUGIN_TYPE_DAEMON))
            continue;

        plugin_thread = malloc(sizeof(struct plugin_threads_t));
        memset(plugin_thread, 0, sizeof(struct plugin_threads_t));

        debug("Looper plugin [%s] is started\n", plugin->name);

        if ((err = thread_create(&plugin_thread->thread, start_thread, plugin)) != 0) {
            debug("Thread could not be started. pthread_create, errno = %d\n", err);
        }

        LIST_INSERT_HEAD(&plugin_threads_head, plugin_thread, list);
    }
}

struct plugin_list_t * plugin_find_commands(char * name, struct plugin_list_t * temp_plugin_list)
{
    struct plugin_t * iterator;

    LIST_INIT(temp_plugin_list);

    LIST_FOREACH(iterator, &plugin_slist_head, list) {
        struct plugin_t * plugin = iterator;

        if (!(plugin->type & PLUGIN_TYPE_COMMAND))
            continue;

        if (strcmp(plugin->name, name) == 0) {
            LIST_INSERT_HEAD(temp_plugin_list, plugin, list);
            debug("Found plugin %s\n", name);
        }
    }

    return temp_plugin_list;
}

void send_message(struct irc_t * irc, char * response)
{
    char buffer[4096];
    snprintf(buffer, sizeof buffer, "%s\r\n", response);
    socket_write(irc->sockfd, buffer, strlen(buffer));
}

static void plugin_load_file(char * file, struct irc_t * irc)
{
    dl_library_t plugin_file;
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
    plugin->irc = irc;

    /* If plugin is of type grep, acquire grep keywords. */
    if (plugin->type & PLUGIN_TYPE_GREP) {
        char ** keywords;

        if ((keywords = dl_sym(plugin_file, "keywords"))) {
            plugin->keywords = keywords;
        } else {
            debug("The plugin \"%s\" is of type `grep', but has no "
                  "exported grep keywords symbols found. Discarding.\n", file);
            return;
        }
    }

    LIST_INSERT_HEAD(&plugin_slist_head, plugin, list);
}

void plugin_init(struct irc_t * irc)
{
    DIR * dir;
    struct dirent * dirent;

    LIST_INIT(&plugin_slist_head);

    dir = opendir(PLUGIN_DIR);

	if (!dir) {
		debug("No plugins directory found. No plugins will be loaded.\n");
		return;
	}

    while ((dirent = readdir(dir)) != NULL) {
#ifdef __WIN32__
        if (strstr(dirent->d_name, ".dll")) {
#else
        if (strstr(dirent->d_name, ".so")) {
#endif // __WIN32__
            char plugin_path[200];
            snprintf(plugin_path, sizeof plugin_path, "%s/%s", PLUGIN_DIR, dirent->d_name);
            plugin_load_file(plugin_path, irc);
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

