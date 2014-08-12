#include "plugin_client.h"

#include <Python.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>

#define PLUGIN_DIR "plugins"

/**
 * Internal python plugins struct for each python plugin as all managed by this plugin. 
 */
struct py_module_t {
    char * name;

    int is_command;
    int is_grep;
    int is_looper;

    PyObject * pModule, * pFunc;
};

static struct python_plugin_list_t * head;

struct python_plugin_list_t {
    struct py_module_t      * cur;
    struct python_plugin_list_t    * next;
};

static void insert(struct py_module_t * p)
{
    if (head == NULL) {
        head = malloc(sizeof (struct python_plugin_list_t));
        head->cur = p;
        head->next = NULL;
    } else {
        struct python_plugin_list_t * it;

        for (it = head; it->next != NULL; it = it->next) {}

        it->next = malloc(sizeof (struct python_plugin_list_t));
        it->next->cur = p;
        it->next->next = NULL;
    }
}

static void plugin_load_file(char * full_path)
{
    struct py_module_t * mod;
    char mod_name[50];
    char mod_command_name[50];

    if (!strstr(full_path, ".py") || full_path[strlen(full_path)-1] != 'y') 
        return;

    {
        char * offset = strchr(full_path, '/')+1;
        int len = strcspn(full_path, ".") - (offset-full_path); 
        strncpy(mod_name, offset, len);
        mod_name[len] = 0;
    }
    mod = malloc(sizeof (struct py_module_t));

    mod->pModule = PyImport_ImportModule(mod_name);

    if (!mod->pModule) {
        debug("Can't load module: %s at %s\n", mod_name, full_path);
        //PyErr_Print();
        free(mod);
        return;
    }

    mod->pFunc = PyObject_GetAttrString(mod->pModule, mod_name);

    if (!mod->pFunc || !PyCallable_Check(mod->pFunc)) {
        debug("Error python call method check for module %s and attr %s.\n", 
                        full_path, mod_name);
        free(mod);
        return;
    }

    mod_command_name[0] = 0;
    strncpy(mod_command_name, strchr(mod_name, '_')+1, strcspn(mod_name, ".")); 
    mod->name = strdup(mod_command_name);
    mod->is_command = 1;
    insert(mod);

    debug("Python module loaded: [%s]\n", full_path);
}

static void load_python_plugins()
{
    DIR * dir;
    struct dirent * dirent;

    dir = opendir(PLUGIN_DIR);

    if (!dir)
    {
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
    struct python_plugin_list_t * it;
    char command_name[50];

    command_name[0] = 0;

    if (plugin->irc->message.trailing[0]) {
        size_t n = strcspn(plugin->irc->message.trailing+1, " \r\n");
        strncpy(command_name, plugin->irc->message.trailing+1, n);
        command_name[n] = 0;
    }

    for (it = head; it != NULL; it = it->next) {
        /* Run command. */
        if (it->cur->is_command && !strcmp(it->cur->name, command_name)) {
            struct py_module_t * module = it->cur;
            char response[512];
            char raw_response[512];

            py_call_module(module, plugin->irc, response);

            snprintf(raw_response, 512, "PRIVMSG %s :%s\r\n", plugin->irc->from, response);

            plugin->send_message(plugin->irc, raw_response);
        }

        /**
         * TODO: Run python loopers and greps.
         */
    }
}

static int manager_find (char * name) 
{
    struct python_plugin_list_t * it;

    for (it = head; it != NULL; it = it->next) {
        if (!strcmp(it->cur->name, name))
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
    plugin->is_looper  = 1;
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

