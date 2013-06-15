#include "global.h"
#ifdef USE_PYTHON_MODULES

#include <Python.h>
#include <signal.h>
#include <assert.h>

#include "config.h"
#include "irc.h"
#include "module.h"


struct py_module_t {
    PyObject * pName, * pModule, * pFunc;
};

void * mod_array_py[100][2];

static void signal_handler(int signum)
{
    exit(signum);
}

void py_unload_modules(void)
{
    int k;

    for (k = 0; mod_array_py[k][0] != NULL; k++)
    {
        struct py_module_t * p;

        p = mod_array_py[k][1];
#ifndef _WIN32
        Py_DECREF(p->pFunc);
        Py_DECREF(p->pModule);
        Py_DECREF(p->pName);
#endif
        free(mod_array_py[k][0]);
        free(mod_array_py[k][1]);

        mod_array_py[k][0] = NULL;
        mod_array_py[k][1] = NULL;
    }
}

struct py_module_t * py_find_loaded_name(char * cmd)
{
    char t[50];
    int k;

    cmd++; /* skip the '.' prefix */

    strcpy(t, "mod_");
    strcat(t, cmd);

    for (k = 0; mod_array_py[k][0] != NULL; k++)
        if (!strcmp((char *) mod_array_py[k][0], t))
            return (struct py_module_t *) mod_array_py[k][1];

    return NULL;
}

char * py_call_module(struct py_module_t * mod, struct irc_t * irc)
{
    PyObject    * p_args;
    PyObject    * p_val;
    char        * t;

    p_args = PyTuple_New(2);                          
    t = strchr(irc->message.trailing, '\r');                   
    *t = '\0';
    p_val = PyString_FromString(irc->from);           
    PyTuple_SetItem(p_args, 0, p_val);            
    p_val = PyString_FromString(irc->message.trailing);        
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
    int k;

    file_name = (char *) data;

    if (!strstr(file_name, ".py") || file_name[strlen(file_name)-1] == 'c') 
        return;

    strcpy(mod_name, file_name);
    *strchr(mod_name, '.') = '\0';

    mod = malloc(sizeof (struct py_module_t));

    mod->pName = PyString_FromString(mod_name);
    mod->pModule = PyImport_ImportModule(mod_name);

    if (!mod->pModule) {
        fprintf(stderr, "Can't load module: %s\n", mod_name);
        free(mod);
        if (PyErr_Occurred()) PyErr_Print();
        return;
    }

    mod->pFunc = PyObject_GetAttrString(mod->pModule, mod_name);

    if (!mod->pFunc || !PyCallable_Check(mod->pFunc)) {
        fprintf(stderr, "Error python call method check.\n");
        free(mod);
        if (PyErr_Occurred()) PyErr_Print();
        return;
    }

    for (k = 0; mod_array_py[k][0] != NULL; k++) {}
    mod_array_py[k][0] = strdup(mod_name);
    mod_array_py[k][1] = mod;

    fprintf(stderr, "Python module loaded: [%s]\n", mod_name);
}

void py_load_mod_hash(void)
{
    py_unload_modules();

    module_iterate_files(py_load_callback);
}

int py_load_modules(char * mod_dir)
{
    signal(SIGINT, signal_handler);
    signal(SIGABRT, signal_handler);

    Py_Initialize();

    set_pymodule_path(mod_dir);

    py_load_mod_hash();

    return 1;
}

#endif
