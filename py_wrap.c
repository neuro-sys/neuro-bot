#include <Python.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <signal.h>
#include <assert.h>

#include "global.h"
#include "config.h"
#include "irc.h"
#include "module.h"


struct py_module_t {
    PyObject * pName, * pModule, * pFunc;
};

GHashTable * mod_hash_map;


char * get_loaded_module_names()
{
    char * buf;

    buf = malloc(510);

    buf[0] = '\0';

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, mod_hash_map);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        strcat(buf, " [");
        strcat(buf, key);
        strcat(buf, "]");
    }

    return buf;
}

static void signal_handler(int signum)
{
    exit(signum);
}

void py_unload_modules()
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, mod_hash_map);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        struct py_module_t * p;

        p = (struct py_module_t *) value;
        
        Py_DECREF(p->pFunc);
        Py_DECREF(p->pModule);
        Py_DECREF(p->pName);

        free(p);

        g_hash_table_iter_remove(&iter);
    }
}

struct py_module_t * find_module_from_command(char * cmd)
{
    struct py_module_t * mod = NULL;
    char t[50];
    gboolean is_found = FALSE;

    cmd++; /* skip the '.' prefix */

    strcpy(t, "mod_");
    strcat(t, cmd);

    is_found = g_hash_table_lookup_extended (mod_hash_map, t, NULL, (void **) &mod);
    if (is_found == FALSE)
        return NULL;

    return mod;

}

char * py_call_module(struct py_module_t * mod, struct irc_t * irc)
{
    PyObject    * p_args;
    PyObject    * p_val;
    char        * t;
    int n;

    p_args = PyTuple_New(2);                          
    t = strchr(irc->request, '\r');                   
    *t = '\0';
    p_val = PyString_FromString(irc->from);           
    n = PyTuple_SetItem(p_args, 0, p_val);            
    p_val = PyString_FromString(irc->request);        
    n = PyTuple_SetItem(p_args, 1, p_val);            

    p_val = PyObject_CallObject(mod->pFunc, p_args);  
    if (p_val)
        t = PyString_AsString(p_val);                     
    else
        t = "Python module crashed.";

#ifndef _WIN32
    Py_DECREF(p_args);
#endif
    return strdup(t);
}

static void set_pymodule_path(char * py_path)
{
    PyObject * sys_path = PySys_GetObject("path");        
    PyObject * path     = PyString_FromString(py_path);   
    PyList_Append(sys_path, path);
}

void py_load_callback(void *data)
{

    struct py_module_t * mod;
    char mod_name[50];
    char * file_name;

    file_name = (char *) data;
    if (!file_name) {
        return;
    }

    if (!g_strrstr(file_name, ".py") || file_name[strlen(file_name)-1] == 'c') 
        return;

    strcpy(mod_name, file_name);
    *strchr(mod_name, '.') = '\0';

    g_strstrip(mod_name);

    mod = malloc(sizeof (struct py_module_t));

    mod->pName = PyString_FromString(mod_name);
    mod->pModule = PyImport_ImportModule(mod_name);

    if (!mod->pModule) {
        g_printerr("Can't load module: %s\n", mod_name);
        free(mod);
        if (PyErr_Occurred()) PyErr_Print();
        return;
    }

    mod->pFunc = PyObject_GetAttrString(mod->pModule, mod_name);

    if (!mod->pFunc || !PyCallable_Check(mod->pFunc)) {
        g_printerr("Error python call method check.\n");
        free(mod);
        if (PyErr_Occurred()) PyErr_Print();
        return;
    }

    g_hash_table_insert(mod_hash_map, strdup(mod_name), mod);
    g_printerr("Module loaded: [%s]\n", mod_name);
}

void py_load_mod_hash(char * mod_dir)
{
    if (mod_hash_map)
        py_unload_modules();

    mod_hash_map = g_hash_table_new(g_str_hash, g_str_equal);

    module_iterate_files(py_load_callback);

}

int py_load_modules(void)
{
    char * mod_dir;

    init_module();

    mod_dir = module_get_dir();

    signal(SIGINT, signal_handler);
    signal(SIGABRT, signal_handler);

	g_type_init();
    Py_Initialize();

    set_pymodule_path(mod_dir);

    py_load_mod_hash(mod_dir);

    return 1;
}

