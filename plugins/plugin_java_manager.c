/**
 * gcc Sample2.c -L/lib/gcj-4.5.3-11 -ljvm
 *
 */

#include "plugin_client.h"

#include <jni.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>

#define PLUGIN_DIR "plugins"

struct plugin_t * plugin;

static JNIEnv *env;
static JavaVM *jvm;

struct java_plugin_t {
    char * name;

    int is_command;
    int is_grep;
    int is_daemon;

    jclass clazz;
    jmethodID method_id;
};

static struct java_plugin_list_t * head;

struct java_plugin_list_t {
    struct java_plugin_t        * cur;
    struct java_plugin_list_t   * next;
};

static void plugin_insert(struct java_plugin_t * p)
{
    if (head == NULL) {
        head = malloc(sizeof (struct java_plugin_t));
        head->cur = p;
        head->next = NULL;
    } else {
        struct java_plugin_list_t * it;

        for (it = head; it->next != NULL; it = it->next) {}

        it->next = malloc(sizeof (struct java_plugin_t));
        it->next->cur = p;
        it->next->next = NULL;
    }
}


static void plugin_load_file(char * full_path)
{
    struct java_plugin_t * plugin;
    char plugin_name[50];

    if (!strstr(full_path, ".class") || full_path[strlen(full_path)-1] != 's')
        return;

    {
        char * offset = strchr(full_path, '/')+1;
        int len = strcspn(offset, "."); 
        strncpy(plugin_name, offset, len);
        plugin_name[len] = 0;
    }

    plugin = malloc(sizeof (struct java_plugin_t));

    plugin->clazz = (*env)->FindClass(env, plugin_name);

    if (!plugin->clazz) {
        debug("Plugin %s could not be loaded.\n", plugin_name);
        return;
    }

    plugin->method_id = (*env)->GetStaticMethodID(env, plugin->clazz, "run", "()[C");

    if (!plugin->method_id) {
        debug("Static method run could not be found in plugin %s.\n", plugin_name);
        return;
    }

    debug("Plugin %s is loaded.\n", plugin_name);
}

int init_jvm(void)
{
    JavaVMOption options[1];
    JavaVMInitArgs vm_args;
    long status;
    static char classpath[256];

    sprintf(classpath, "-Djava.class.path=%s", PLUGIN_DIR);
    options[0].optionString = classpath;

    memset(&vm_args, 0, sizeof(vm_args));
    vm_args.version = JNI_VERSION_1_2;
    vm_args.nOptions = 1;
    vm_args.options = options;
    status = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);

    if (status == JNI_ERR) {
        debug("JVM could not be created.\n");
        return -1;
    }
    debug("JVM is created.\n");
}

void load_java_plugins(void)
{
    DIR * dir;
    struct dirent * dirent;

    dir = opendir(PLUGIN_DIR);

    if (!dir)
    {
        debug("The directory %s could not be opened.\n", PLUGIN_DIR);
        return;
    }

    while ((dirent = readdir(dir)) != NULL)
    {
        if (strstr(dirent->d_name, ".class")) {
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


static int manager_find (char * name) 
{

}

void run(void)
{
    sprintf(plugin->irc->response, "PRIVMSG %s :Hello!", plugin->irc->from);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "java_manager";
    plugin->is_daemon  = 1;
    plugin->is_command = 1;
    plugin->is_grep    = 1;
    plugin->is_manager = 1;

    plugin->manager_find = manager_find;

    init_jvm();

    load_java_plugins();

    return plugin;
}

#ifdef TEST_PLUGIN_JAVA_MANAGER
int main(int argc, char *argv[])
{
    struct plugin_t * plugin;

    plugin = init();

    return 0;
}

#endif

