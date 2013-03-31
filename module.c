#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
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
#ifdef _WIN32
    HMODULE mod;
    FARPROC initializer;
#else
    void * mod;
    void * initializer;
#endif
   
    char * t;
    struct mod_c_t * mod_c;

    int k;

    mod_c = malloc(sizeof (struct mod_c_t));

    file_name = (char *) data;

#ifdef _WIN32
    if (!g_strrstr(file_name, ".dll")) 
#else
    if (!g_strrstr(file_name, ".so")) 
#endif
      return;


    snprintf(file_full_path, 250, "%s/%s", mod_dir, file_name);
#ifdef _WIN32
    mod = LoadLibrary(file_full_path);
#else
    mod = dlopen(file_full_path, RTLD_LAZY);
#endif
    if (!mod)
    {
        g_printerr("%s could not be opened.\n", file_name);
        return;
    }

    mod_c->mod = mod;
    t = strchr(file_name, '.');
    *t = '\0';

    mod_c->mod_name = strdup(file_name);
#ifdef _WIN32
    initializer = GetProcAddress(mod, file_name);
#else
    initializer = dlsym(mod, file_name);
#endif
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
    GFileEnumerator * enum_children;
    GFile           * mod_path_file;
    GError          * error = NULL;
    GFileInfo       * fileInfo;

    mod_path_file = g_file_new_for_path(mod_dir);

    enum_children = g_file_enumerate_children(mod_path_file, "*", 0, NULL, &error);
    if (!enum_children || error) {
        g_printerr("Can't open the specified path: %s\n", mod_dir);
        if (error) {
            g_error_free(error);
            error = NULL;
        }
        return;
    }

    while ( (fileInfo = g_file_enumerator_next_file(enum_children, NULL, &error)) != NULL) {
        char * file_name;

        file_name = strdup(g_file_info_get_name (fileInfo));

        callback(file_name);
    }

    if (!g_file_enumerator_close(enum_children, NULL, NULL))
        g_printerr("The file handle resource cannot be freed.\n");

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

	g_type_init();

    cur_dir = g_get_current_dir();

    modules_path = config_get_string(GROUP_MODULES, KEY_PYPATH);
    if (!modules_path)
        modules_path = g_strdup(mod_path);

    if (modules_path[0] == '/')
        mod_dir = g_strdup(modules_path);
    else
        mod_dir = g_strdup_printf("%s%c%s", cur_dir, G_DIR_SEPARATOR, modules_path);
    g_free(cur_dir);
    g_free(modules_path);

    module_load();

    if ( py_load_modules() < 0 )
        g_printerr("Could not load python modules, going on without them.\n");
}

