#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include "config.h"
#include "global.h"
#include "module.h"

static const char * mod_path = "modules";

static char * mod_dir;

char * module_get_dir()
{
    return mod_dir;
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

void init_module()
{

    char            * cur_dir;
    char            * modules_path;

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
}


struct module_t * create_module()
{
    struct module_t * mod = malloc(sizeof(struct module_t));


    return mod;
}


