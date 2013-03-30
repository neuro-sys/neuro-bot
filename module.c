#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <stdlib.h>
#include "khash.h"

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

KHASH_MAP_INIT_STR(mod_c_hash_map, struct mod_c_t *)
khash_t(mod_c_hash_map) * h_mod_c_hash_map;

char * module_get_dir()
{
    return mod_dir;
}

struct mod_c_t * module_find(const char * cmd)
{
    char t[50];
    khiter_t k;
    struct mod_c_t * mod;

    snprintf(t, sizeof t, "mod_%s", cmd);

    k = kh_get(mod_c_hash_map, h_mod_c_hash_map, t);

    if (k == kh_end(h_mod_c_hash_map))
        return NULL;

    mod = kh_value(h_mod_c_hash_map, k);

    puts(mod->mod_name);

    return mod;
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

    khiter_t k;
    int ret;

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

    k = kh_put(mod_c_hash_map, h_mod_c_hash_map, strdup(file_name), &ret);
    kh_value(h_mod_c_hash_map, k) = mod_c;

    g_printerr("Module loaded: [%s]\n", file_name);
}

char * module_get_loaded_names(void)
{
    char * buf;
    khiter_t k;

    buf = malloc(510);

    buf[0] = '\0';

    for (k = kh_begin(h_mod_c_hash_map); k != kh_end(h_mod_c_hash_map); k++) {
        if (!kh_exist(h_mod_c_hash_map, k))
            continue;
        strcat(buf, " [");
        strcat(buf, kh_key(h_mod_c_hash_map, k));
        strcat(buf, "]");
    }

    return buf;
}

void module_unload_all(void)
{
    khiter_t k;

    for (k = kh_begin(h_mod_c_hash_map); k != kh_end(h_mod_c_hash_map); k++)
    {
        struct irc_c_t * mod_c;

        if (!kh_exist(h_mod_c_hash_map, k))
            continue;

        mod_c = (struct irc_c_t *) kh_value(h_mod_c_hash_map, k);

        //dlclose(mod_c->mod);
        free(mod_c);

        kh_del(mod_c_hash_map, h_mod_c_hash_map, k);
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

    h_mod_c_hash_map = kh_init(mod_c_hash_map);

    module_load();

    if ( py_load_modules() < 0 )
        g_printerr("Could not load python modules, going on without them.\n");
}

