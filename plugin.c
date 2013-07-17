#include "plugins/plugin_client.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <dirent.h>
#include <dlfcn.h>

#define PLUGIN_DIR "plugins"

struct plugin_list_t * head;

struct plugin_list_t {
    struct plugin_t * cur;
    struct plugin_list_t * next;
};

/**
 * Returns a handle to the native plugin, for the given command name.
 */
struct plugin_t * plugin_find_command(char * name)
{
    struct plugin_list_t * it;

    for (it = head; it != NULL; it = it->next) {
        if (!strcmp(it->cur->name, name))
            return it->cur;
        if (it->cur->is_manager && !it->cur->manager_find(name))
            return it->cur;
    }
    return NULL;
}

void insert(struct plugin_t * p)
{
    if (head == NULL) {
        head = malloc(sizeof (struct plugin_list_t));
        head->cur = p;
        head->next = NULL;
    } else {
        struct plugin_list_t * it;

        for (it = head; it->next != NULL; it = it->next) {}

        it->next = malloc(sizeof (struct plugin_list_t));
        it->next->cur = p;
        it->next->next = NULL;
    }
}

/**
 * Callback send raw message function for plugins.
 *
 * TODO: It should utilize a message queue for delivering messages to the network.
 *
 */
void send_message(struct irc_t * irc)
{
    puts(irc->response);
}

struct plugin_t plugin_list[100];

void plugin_load_file(char * file)
{
    void * plugin_file;
    struct plugin_t * (*init)(void);
    struct plugin_t * plugin;

    fprintf(stderr, "%25s:%4d:Loading native plugin \"%s\"\n", __FILE__, __LINE__, file);
    if ((plugin_file = dlopen(file, RTLD_LAZY)) == NULL) {
        fprintf(stderr, "%25s:%4d: The plugin file \"%s\" could not be opened.\n", 
                __FILE__, __LINE__, file);
        return;
    }

    if ((init = dlsym(plugin_file, "init")) == NULL) { 
        fprintf(stderr, "%25s:%4d:The plugin \"%s\" has no exported init function symbol.\n",
                __FILE__, __LINE__, file);
    }

    /* initialize and get handle to the plugin */
    plugin = init();

    /* See warning message for commentary */
    if (!plugin->is_manager && (plugin->is_command + plugin->is_grep + plugin->is_looper) > 1) {
        fprintf(stderr, "%25s:%4d:The plugin \"%s\" is not valid."
                    "A plugin can only be one of type `command', `grep' and `looper'.\n",
                    __FILE__, __LINE__, file);
        /* TODO: Clean up. */
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
            fprintf(stderr, "%25s:%4d:The plugin \"%s\" is of type `grep', but has no "
                        "exported grep keywords symbols found. Discarding.\n",
                        __FILE__, __LINE__, file);
            /* TODO: Clean up. */
            return;
        }
    }

    insert(plugin);
}

void plugin_init()
{
    DIR * dir;
    struct dirent * dirent;

    dir = opendir(PLUGIN_DIR);

	if (!dir)
	{
		fprintf(stderr, "%25s:%4d:no modules found, skipping.\n", __FILE__, __LINE__);
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


#if 0
int main()
{
    struct plugin_list_t * it;

    plugin_init();

    puts("Iterating the plugins...");
    for (it = head; it != NULL; it = it->next) {
        printf("- %s\n", it->cur->name);
    }
    puts("Finished.");

    return 0;
}
#endif

