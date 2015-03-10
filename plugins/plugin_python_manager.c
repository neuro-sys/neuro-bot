#include "plugin_client.h"

#include <Python.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/queue.h>

#define PLUGIN_DIR "plugins"

/**
 * Internal python plugins struct for each python plugin as all managed by this plugin. 
 */
struct py_module_t {
    char * name;

    int is_command;
    int is_grep;
    int is_daemon;

    PyObject * pModule, * pFunc;

    LIST_ENTRY(py_module_t) py_modules;
};

LIST_HEAD(py_module_list_head, py_module_t) py_module_list_head; 

static void plugin_python_free()
{
    struct py_module_t * iterator;

    LIST_FOREACH(iterator, &py_module_list_head, py_modules) {
        struct py_module_t * module = iterator;
        
        free(module->name);
        free(iterator);
    }
    
}

static void parse_mod_name(char * full_path, char * dest)
{
    char * begin = strrchr(full_path, '/')+1;
    char * end   = strrchr(full_path, '.');
    snprintf(dest, end-begin+1, "%s", begin);
}

static void parse_mod_command_name(char * mod_name, char * dest)
{
    char * end = strchr(mod_name, '_')+1;
    sprintf(dest, "%s", end);
}

static void plugin_load_file(char * full_path)
{
    struct py_module_t * mod;
    char mod_name[100];
    char mod_command_name[50];

    if (!strstr(full_path, ".py") || full_path[strlen(full_path)-1] != 'y') {
        return;
    }

    mod = malloc(sizeof (struct py_module_t));

    parse_mod_name(full_path, mod_name);
    parse_mod_command_name(mod_name, mod_command_name);

    debug("mod_name: %s\n", mod_name);
    debug("mod_command_name: %s\n", mod_command_name);

    mod->name = strdup(mod_command_name);
    mod->is_command = 1;

    mod->pModule = PyImport_ImportModule(mod_name);

    if (!mod->pModule) {
        debug("Can't load module: %s at %s\n", mod_name, full_path);
        free(mod);
        return;
    }

    mod->pFunc = PyObject_GetAttrString(mod->pModule, mod_name);

    if (!mod->pFunc || !PyCallable_Check(mod->pFunc)) {
        debug("Error python call method check for module %s and attr %s.\n", full_path, mod_name);
        free(mod);
        return;
    }

    LIST_INSERT_HEAD(&py_module_list_head, mod, py_modules);

    debug("Python module loaded: [%s]\n", full_path);
}

static void load_python_plugins()
{
    DIR * dir;
    struct dirent * dirent;

    LIST_INIT(&py_module_list_head);

    dir = opendir(PLUGIN_DIR);

    if (!dir) {
        debug(" no modules found, skipping.\n");
        return;
    }

    while ((dirent = readdir(dir)) != NULL)
    {
        if (strstr(dirent->d_name, ".py")) {
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

static void set_pymodule_path(char * py_path)
{
    PyObject * sys_path = PySys_GetObject("path");        
    PyObject * path     = PyString_FromString(py_path);   
    PyObject * syspath  = NULL;

    PyList_Append(sys_path, path);
    PySys_SetObject("path", sys_path);

    syspath = PyImport_ImportModule("sys");
    {
        PyObject * strObj = PyObject_GetAttrString(syspath, "path");
        PyObject * strRep = PyObject_Str(strObj);

        debug("Python sys.path: %s\n", PyString_AsString(strRep));
    }
}

/*
 * Build an object to pass as a single parameter to the python plugin.
 */
static void py_call_module(struct py_module_t * mod, struct irc_t * irc, char * res)
{
    PyObject    * p_args;
    PyObject    * p_val;
    char        * t;

    p_args = PyTuple_New(2);                          

    p_val = PyString_FromString(irc->from);        
    PyTuple_SetItem(p_args, 0, p_val);            

    p_val = PyString_FromString(irc->message.trailing);           
    PyTuple_SetItem(p_args, 1, p_val);            

    p_val = PyObject_CallObject(mod->pFunc, p_args);  
    if (p_val) {
        t = PyString_AsString(p_val);                     
        strcpy(res, t);
    } else {
        sprintf(res, "PRIVMSG %s :Python module returned null.", irc->from);
    }
}

static struct plugin_t * plugin;

static void run(void)
{
    char command_name[50];

    command_name[0] = 0;

    if (plugin->irc->message.trailing[0]) {
        size_t n = strcspn(plugin->irc->message.trailing+1, " \r\n");
        strncpy(command_name, plugin->irc->message.trailing+1, n);
        command_name[n] = 0;
    }

    struct py_module_t * iterator;

    LIST_FOREACH(iterator, &py_module_list_head, py_modules) {
        struct py_module_t * module = iterator;

        if (module->is_command && !strcmp(module->name, command_name)) {
            char response[512];
            char raw_response[512];

            py_call_module(module, plugin->irc, response);

            snprintf(raw_response, 512, "%s", plugin->irc->from, response);

            plugin->send_message(plugin->irc, raw_response);
        }
    }
}

static int manager_find (char * name) 
{
    struct py_module_t * iterator;

    LIST_FOREACH(iterator, &py_module_list_head, py_modules) {
        struct py_module_t * module = iterator;

        if (!strcmp(module->name, name))
            return 0;
    }

    return -1;
}

static int init_python(void)
{
    char buf[200];
    char pwd[100];

    buf[0] = 0;
    pwd[0] = 0;

    Py_Initialize();

    if (!Py_IsInitialized()) {
        debug("Python's not initialized.\n");
        return -1;
    }

    debug("Python's initialized...\n");

    getcwd(pwd, 1024);
    sprintf(buf, "%s/%s/", pwd, PLUGIN_DIR);
    debug("Setting python module path: %s\n", buf);
    set_pymodule_path(buf);

    return 0;
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "python_manager";
    plugin->is_daemon  = 1;
    plugin->is_command = 1;
    plugin->is_grep    = 1;
    plugin->is_manager = 1;

    plugin->manager_find = manager_find;

    if (init_python() != 0) {
        return NULL;    
    }
    load_python_plugins();

    return plugin;
}

#if TEST_PLUGIN_PYTHON_MANAGER
int main(int argc, char *argv[])
{
    struct plugin_t * plugin;

    plugin = init();

    int n = plugin->manager_find("example");
    printf("%d\n", n);
    return 0;
}
#endif

