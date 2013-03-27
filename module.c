#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include "config.h"
#include "global.h"
#include "irc.h"
#include "module.h"

static const char       * mod_path = "modules";

static       char       * mod_dir;

             GHashTable * mod_c_hash_map;

char * module_get_dir()
{
    return mod_dir;
}

struct mod_c_t * module_find(char * cmd)
{
    struct mod_c_t * mod;
    char t[50];
    gboolean is_found = FALSE;

    snprintf(t, sizeof t, "mod_%s", cmd);

    is_found = g_hash_table_lookup_extended (mod_c_hash_map, t, NULL, (void **) &mod);

    if (is_found == FALSE)
        return NULL;

    return mod;
}

void module_load_callback(void * data)
{
    char * file_name;
    char file_full_path[250];
    void * mod;
    void * initializer;
    char * t;
    struct mod_c_t * mod_c;

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

    g_hash_table_insert(mod_c_hash_map, strdup(file_name), mod_c);

    g_printerr("Module loaded: [%s]\n", file_name);
}

char * module_get_loaded_names(void)
{
    char * buf;

    buf = malloc(510);

    buf[0] = '\0';

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, mod_c_hash_map);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        strcat(buf, " [");
        strcat(buf, key);
        strcat(buf, "]");
    }

    return buf;
}

void module_unload_all(void)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, mod_c_hash_map);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        struct irc_c_t * mod_c;
        mod_c = (struct irc_c_t *) value;

        //dlclose(mod_c->mod);
        free(mod_c);

        g_hash_table_iter_remove(&iter);
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

    mod_c_hash_map = g_hash_table_new(g_str_hash, g_str_equal);

    module_load();
}

