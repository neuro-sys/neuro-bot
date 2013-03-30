#include <Python.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <signal.h>
#include <assert.h>

#include "khash.h"

#include "global.h"
#include "config.h"
#include "irc.h"
#include "module.h"


struct py_module_t {
    PyObject * pName, * pModule, * pFunc;
};

KHASH_MAP_INIT_STR(py_mod_hash_map, struct py_module_t *)
khash_t(py_mod_hash_map) * h_py_mod_hash_map;

char * py_get_loaded_names(void)
{
    char * buf;
    khiter_t k;

    buf = malloc(510);

    buf[0] = '\0';

    for (k = kh_begin(h_py_mod_hash_map); k != kh_end(h_py_mod_hash_map); k++)
    {
        if (!kh_exist(h_py_mod_hash_map, k))
            continue;

        strcat(buf, " [");
        strcat(buf, kh_key(h_py_mod_hash_map, k));
        strcat(buf, "]");
    }

    return buf;
}

static void signal_handler(int signum)
{
    exit(signum);
}

void py_unload_modules(void)
{
    khiter_t k;

    for (k = kh_begin(h_py_mod_hash_map); k != h_py_mod_hash_map; k++)
    {
        struct py_module_t * p;

        if (!kh_exist(h_py_mod_hash_map, k))
            continue;

        p = (struct py_module_t *) kh_value(h_py_mod_hash_map, k);
 
#ifndef _WIN32
        Py_DECREF(p->pFunc);
        Py_DECREF(p->pModule);
        Py_DECREF(p->pName);
#endif
        free(p);

        kh_del(py_mod_hash_map, h_py_mod_hash_map, k);
    }
}

struct py_module_t * py_find_loaded_name(char * cmd)
{
    char t[50];
    khiter_t k;

    cmd++; /* skip the '.' prefix */

    strcpy(t, "mod_");
    strcat(t, cmd);

    kh_get(py_mod_hash_map, h_py_mod_hash_map, t);

    if (k == kh_end(h_py_mod_hash_map))
        return NULL;

    return kh_value(h_py_mod_hash_map, k);

}

char * py_call_module(struct py_module_t * mod, struct irc_t * irc)
{
    PyObject    * p_args;
    PyObject    * p_val;
    char        * t;

    p_args = PyTuple_New(2);                          
    t = strchr(irc->request, '\r');                   
    *t = '\0';
    p_val = PyString_FromString(irc->from);           
    PyTuple_SetItem(p_args, 0, p_val);            
    p_val = PyString_FromString(irc->request);        
    PyTuple_SetItem(p_args, 1, p_val);            

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
    khiter_t k;
    int ret;

    file_name = (char *) data;

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

    k = kh_put(py_mod_hash_map, h_py_mod_hash_map, strdup(mod_name), &ret);

    kh_value(h_py_mod_hash_map, k) = mod;

    g_printerr("Module loaded: [%s]\n", mod_name);
}

void py_load_mod_hash(char * mod_dir)
{
    if (h_py_mod_hash_map)
        py_unload_modules();

    h_py_mod_hash_map = kh_init(py_mod_hash_map);

    module_iterate_files(py_load_callback);
}

int py_load_modules(void)
{
    char * mod_dir;

    mod_dir = module_get_dir();

    signal(SIGINT, signal_handler);
    signal(SIGABRT, signal_handler);

    Py_Initialize();

    set_pymodule_path(mod_dir);

    py_load_mod_hash(mod_dir);

    return 1;
}

