#include "plugin_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <dlfcn.h>

#define PLUGIN_DIR "plugins"

typedef     void *                      PyObject;
void        (*PyList_Append)            (PyObject *, PyObject *);
PyObject    (*PySys_GetObject)          (char *);
PyObject    (*PyString_FromString)      (char *);
PyObject    (*PyImport_ImportModule)    (char *);
PyObject    (*PyObject_GetAttrString)   (PyObject *, char *);
int         (*PyCallable_Check)         (PyObject *);

struct py_module_t {
    char * name;
    PyObject * pName, * pModule, * pFunc;
};

struct plugin_list_t * head;

struct plugin_list_t {
    struct py_module_t * cur;
    struct plugin_list_t * next;
};

void insert(struct py_module_t * p)
{
    if (head == NULL) {
        head = malloc(sizeof (struct plugin_list_t));
        head->cur = p;
        head->next = NULL;
    } else {
        struct plugin_list_t * it;

        for (it = head; it->next != NULL; it = it->next) {}

        it->next = malloc(sizeof (struct plugin_list_t));
        it->next->cur = p;
        it->next->next = NULL;
    }
    fprintf(stderr, "Inserted plugin %s\n", p->name);
}

void plugin_load_file(char * fpath)
{
    struct py_module_t * mod;
    char mod_name[50];
    char * file_name;
    int k;

    file_name = fpath;

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
        return;
    }

    mod->pFunc = PyObject_GetAttrString(mod->pModule, mod_name);

    if (!mod->pFunc || !PyCallable_Check(mod->pFunc)) {
        fprintf(stderr, "Error python call method check.\n");
        free(mod);
        return;
    }

    mod->name = mod_name;
    insert(mod);

    fprintf(stderr, "Python module loaded: [%s]\n", mod_name);
}

static void load_python_plugins()
{
    DIR * dir;
    struct dirent * dirent;

    dir = opendir(PLUGIN_DIR);

	if (!dir)
	{
		fprintf(stderr, "no modules found, skipping.\n");
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
    PyList_Append(sys_path, path);
}

static struct plugin_t * plugin;

void run (char * msg, char * res)
{

}

int manager_find (char * name) 
{

}

void init_python(void)
{
    void * sym;
    void * handle;

    handle = dlopen(NULL, RTLD_NOW|RTLD_GLOBAL);
    sym = dlsym(handle, "kernel32.dll");
    fprintf(stderr, "%p\n%p\n", handle, sym);
}

struct plugin_t * init(void)
{
    plugin = malloc(sizeof (struct plugin_t));
    memset(plugin, 0, sizeof *plugin);

    plugin->run        = run;
    plugin->name       = "python_loader";
    plugin->is_looper  = 1;
    plugin->is_command = 1;
    plugin->is_grep    = 1;
    plugin->is_manager = 1;

    plugin->manager_find = manager_find;

    init_python();
    load_python_plugins();

    return plugin;
}

int main(int argc, char *argv[])
{
    struct plugin_t * plugin;

    plugin = init();

    return 0;
}

