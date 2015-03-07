#include "plugin.h"
#include "plugins/plugin_client.h"

#include "global.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

#include <dirent.h>
#include <dlfcn.h>

#define PLUGIN_DIR "plugins"

SLIST_HEAD(plugin_threads_head, plugin_threads_t) plugin_threads_head; 
struct plugin_threads_t {
    pthread_t thread;
    SLIST_ENTRY(plugin_threads_t) plugin_threads;
};

static void *start_thread(void * pdata)
{
    ((struct plugin_t *)pdata)->run();

    return NULL;
}

void plugin_start_loopers(struct irc_t * irc)
{

    int err;
    struct plugin_slist_t * iterator;

    SLIST_INIT(&plugin_threads_head);

    SLIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator->plugin;
        struct plugin_threads_t * plugin_thread = malloc(sizeof(plugin_thread));

        memset(plugin_thread, 0, sizeof(struct plugin_threads_t));

        if (!plugin->is_looper || plugin->is_manager)
            continue;

        debug("Looper plugin [%s] is started\n", plugin->name);
        
        if ((err = pthread_create(&plugin_thread->thread, NULL, start_thread, plugin)) != 0) {
            debug("Thread could not be started. pthread_create, errno = %d\n", err);
        }

        SLIST_INSERT_HEAD(&plugin_threads_head, plugin_thread, plugin_threads);
    }
}

void plugin_attach_context(struct irc_t * irc)
{
    struct plugin_slist_t * iterator;

    debug("Session preparing plugins before connecting to the server.\n");
    SLIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator->plugin;
        debug("Name: [%s]\n", plugin->name);
        plugin->irc = irc;
    }
}

struct plugin_t ** plugin_find_commands(char * name, struct plugin_t *** p_plugin_commands_v)
{
    struct plugin_slist_t * iterator;
    int command_counter = 0;
    struct plugin_t ** plugin_commands_v;

    plugin_commands_v = *p_plugin_commands_v;

    SLIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator->plugin;

        if (!plugin->is_command)
            continue;

        if (plugin->is_manager && !plugin->manager_find(name)) {
            plugin_commands_v = realloc(plugin_commands_v, (command_counter+1) * sizeof (struct plugin_t *));
            *(plugin_commands_v + command_counter++) = plugin;
        }

        if (!strcmp(plugin->name, name)) {
            plugin_commands_v = realloc(plugin_commands_v, (command_counter+1) * sizeof (struct plugin_t *));
            *(plugin_commands_v + command_counter++) = plugin;
        }
    }

    plugin_commands_v = realloc(plugin_commands_v, (command_counter+1) * sizeof (struct plugin_t *));
    *(plugin_commands_v + command_counter++) = NULL;
    
    *p_plugin_commands_v = plugin_commands_v;

    return plugin_commands_v;
}

void plugin_insert(struct plugin_t * p)
{
    struct plugin_slist_t * node = malloc(sizeof(node));
    node->plugin = p;
    SLIST_INSERT_HEAD(&plugin_slist_head, node, plugin_slist);
}

void send_message(struct irc_t * irc, char * response)
{
    debug("%s\n", response);
    socket_send_message(&irc->session->socket, response);
}

struct plugin_t plugin_list[100];

void plugin_load_file(char * file)
{
    void * plugin_file;
    struct plugin_t * (*init)(void);
    struct plugin_t * plugin;

    debug("Loading native plugin \"%s\"\n", file);
    if ((plugin_file = dlopen(file, RTLD_LAZY)) == NULL) {
        debug("The plugin file \"%s\" could not be opened.\n", file);
        return;
    }

    if ((init = dlsym(plugin_file, "init")) == NULL) { 
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

        if ((keywords = dlsym(plugin_file, "keywords"))) {
            plugin->keywords = keywords;
        } else { 
            debug("The plugin \"%s\" is of type `grep', but has no "
                  "exported grep keywords symbols found. Discarding.\n", file);
            return;
        }
    }

    plugin_insert(plugin);
}

void plugin_init()
{
    DIR * dir;
    struct dirent * dirent;

    SLIST_INIT(&plugin_slist_head);

    dir = opendir(PLUGIN_DIR);

	if (!dir)
	{
		debug("no modules found, skipping.\n");
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

    SLIST_INIT(&plugin_threads_head);

    plugin_init();

    puts("Iterating the plugins...");
    SLIST_FOREACH(iterator, &plugin_slist_head, plugin_slist) {
        struct plugin_t * plugin = iterator->plugin;
        debug("%s\n", plugin->name); 
    }
    puts("Finished.");

    return 0;
}
#endif

