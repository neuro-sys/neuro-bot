#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>

#include "config.h"
#include "global.h"
#include "irc.h"
#include "module.h"
#include "py_wrap.h"

static const char       * mod_path = "modules";

static       char       * mod_dir;

#define MOD_MAX_NUM 50
void * mod_array[MOD_MAX_NUM][2];

char * module_get_dir()
{
    return mod_dir;
}

struct mod_c_t * module_find(const char * cmd)
{
    char t[50];
    int k;

    snprintf(t, sizeof t, "mod_%s", cmd);

    for (k = 0; mod_array[k][0] != NULL; k++)
        if (!strcmp(mod_array[k][0], t))
            return mod_array[k][1];

    return NULL;
}

void module_load_callback(void * data)
{
    char * file_name;
    char file_full_path[250];
    void * mod;
    void * initializer;
   
    char * t;
    struct mod_c_t * mod_c;

    int k;

    mod_c = malloc(sizeof (struct mod_c_t));

    file_name = (char *) data;

    if (!g_strrstr(file_name, ".so")) 
      return;


    snprintf(file_full_path, 250, "%s/%s", mod_dir, file_name);
    mod = dlopen(file_full_path, RTLD_LAZY);
    if (!mod)
    {
        g_printerr("%s could not be opened.\n", file_name);
        return;
    }

    mod_c->mod = mod;
    t = strchr(file_name, '.');
    *t = '\0';

    mod_c->mod_name = strdup(file_name);
    initializer = dlsym(mod, file_name);
    if (!initializer)
    {
        g_printerr("entry point %s not found in %s.\n", file_name, file_full_path);
        return;
    }

    mod_c->func = (char * (*)(struct irc_t *)) initializer;

    for (k = 0; mod_array[k][0] != NULL; k++) {}
    mod_array[k][0] = strdup(file_name);
    mod_array[k][1] = mod_c;

    g_printerr("Module loaded: [%s]\n", file_name);
}

char * module_get_loaded_names(void)
{
    char * buf;
    int k;

    buf = malloc(510);

    buf[0] = '\0';

    for (k = 0; mod_array[k][0] != NULL; k++)
    {
        strcat(buf, " [");
        strcat(buf, mod_array[k][0]);
        strcat(buf, "]");
    }

    return buf;
}

void module_unload_all(void)
{
    int k;

    for (k = 0; mod_array[k][0] != NULL; k++)
    {
        free(mod_array[k][0]);
        free(mod_array[k][1]);

        mod_array[k][0] = NULL;
        mod_array[k][1] = NULL;
    }
}

void module_iterate_files(void (*callback)(void * data))
{
    DIR * dir;
    struct dirent * dirent;

    dir = opendir(mod_dir);

    while ( (dirent = readdir(dir)) != NULL)
    {
        callback(dirent->d_name);
    }

    closedir(dir);
}

void module_load()
{
    module_unload_all();

    module_iterate_files(&module_load_callback);
}

void module_init()
{
    char * cur_dir;
    char * modules_path;

    cur_dir = get_current_dir_name();

    modules_path = config_get_string(GROUP_MODULES, KEY_PYPATH);
    if (!modules_path)
        modules_path = strdup(mod_path);

    mod_dir = strdup(modules_path);

    g_free(cur_dir);
    g_free(modules_path);

    module_load();

    if ( py_load_modules() < 0 )
        g_printerr("Could not load python modules, going on without them.\n");
}

