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
    fprintf(stderr, "Inserted plugin %s\n", p->name);
}

void send_message(char *msg)
{
    puts(msg);
}

struct plugin_t plugin_list[100];

void plugin_load_file(char * file)
{
    void * plugin_file;
    struct plugin_t * (*init)(void);
    struct plugin_t * plugin;

    plugin_file = dlopen(file, RTLD_LAZY);
    if (!plugin_file) {
        puts("not opened");
        return;
    }

    init = dlsym(plugin_file, "init");

    /* initialize and get handle to the plugin */
    plugin = init();
    plugin->send_message = send_message;

    insert(plugin);
}

void plugin_init()
{
    DIR * dir;
    struct dirent * dirent;

    dir = opendir(PLUGIN_DIR);

	if (!dir)
	{
		fprintf(stderr, "no modules found, skipping.\n");
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

