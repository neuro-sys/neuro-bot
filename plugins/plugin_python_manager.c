#include "plugin_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <dlfcn.h>

#define PLUGIN_DIR "plugins"

/*
 * Python library functions.
 */

typedef     void *                      PyObject;
void        (*Py_Initialize)            (void);
void        (*PyList_Append)            (PyObject *, PyObject *);
PyObject    (*PySys_GetObject)          (char *);
PyObject    (*PyString_FromString)      (char *);
PyObject    (*PyImport_ImportModule)    (char *);
PyObject    (*PyObject_GetAttrString)   (PyObject *, char *);
int         (*PyCallable_Check)         (PyObject *);
PyObject    (*PyTuple_New)              (int);
PyObject    (*PyObject_CallObject)      (PyObject *, PyObject *);
char *      (*PyString_AsString)        (PyObject *);

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
    fprintf(stderr, "%s:%d:Inserted plugin %s to list. \n", __FILE__, __LINE__, p->name);
}

void plugin_load_file(char * fpath)
{
    struct py_module_t * mod;
    char mod_name[50];
    char mod_command_name[50];
    char * file_name;
    int k;

    if (!strstr(fpath, ".py") || fpath[strlen(fpath)-1] == 'c') 
        return;

    strncpy(mod_name, strchr(fpath, '/')+1, strcspn(fpath, "."));
    puts(mod_name);

    mod = malloc(sizeof (struct py_module_t));

    mod->pName = PyString_FromString(mod_name);
    *strchr(mod_name, '.') = 0;
    mod->pModule = PyImport_ImportModule(mod_name);

    if (!mod->pModule) {
        fprintf(stderr, "%s:%d:Can't load module: %s\n", __FILE__, __LINE__, mod_name);
        free(mod);
        return;
    }

    mod->pFunc = PyObject_GetAttrString(mod->pModule, mod_name);

    if (!mod->pFunc || !PyCallable_Check(mod->pFunc)) {
        fprintf(stderr, "%s:%d:Error python call method check.\n", __FILE__, __LINE__);
        free(mod);
        return;
    }


    mod_command_name[0] = 0;
    strncpy(mod_command_name, strchr(mod_name, '_')+1, strcspn(mod_name, ".")); 
    mod->name = strdup(mod_command_name);
    insert(mod);

    fprintf(stderr, "%s:%d:Python module loaded: [%s]\n", __FILE__, __LINE__, mod_name);
}

static void load_python_plugins()
{
    DIR * dir;
    struct dirent * dirent;

    dir = opendir(PLUGIN_DIR);

	if (!dir)
	{
		fprintf(stderr, "%s:%d: no modules found, skipping.\n", __FILE__, __LINE__);
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

static void py_call_module(struct py_module_t * mod, char * msg, char * res)
{
    PyObject    * p_args;
    PyObject    * p_val;
    char        * t;

    p_args = PyTuple_New(2);                          
    t = strchr(msg, '\r');                   
    *t = '\0';
    p_val = PyString_FromString("");           
    PyTuple_SetItem(p_args, 0, p_val);            
    p_val = PyString_FromString(msg);        
    PyTuple_SetItem(p_args, 1, p_val);            

    p_val = PyObject_CallObject(mod->pFunc, p_args);  
    if (p_val)
        t = PyString_AsString(p_val);                     
    else
        t = "Python module crashed.";

    strcpy(res, t);
}

static struct plugin_t * plugin;

void run(char * msg, char * res)
{
    struct plugin_list_t * it;
    char command_name[50];

    command_name[0] = 0;

    size_t n = strcspn(msg+1, " \r\n");
    strncpy(command_name, msg+1, n);

    for (it = head; it != NULL; it = it->next) {
        if (!strcmp(it->cur->name, command_name)) {
            struct py_module_t * module = it->cur;

            py_call_module(module, msg, res);
            puts(res);
        }
    }
}

int manager_find (char * name) 
{
    struct plugin_list_t * it;

    puts("searching...");
    for (it = head; it != NULL; it = it->next) {
        if (!strcmp(it->cur->name, name))
            return 0;
    }

    return -1;
}

int init_python(void)
{
    char buf[100];
    char pwd[100];
    void * sym;
    void * handle;

    buf[0] = 0;

    if ( (handle = dlopen("python27.dll", RTLD_NOW|RTLD_GLOBAL)) == NULL) {
        return -1;
    }
    fprintf(stderr, "%s:%d:Python found.\n", __FILE__, __LINE__);

    if ( (sym = dlsym(handle, "Py_Initialize")) == NULL) {
        fprintf(stderr, "Symbol not found: Py_Initialize\n");
        return -1;
    }
    fprintf(stderr, "%s:%d:Attached Py_Initailize.\n", __FILE__, __LINE__);
    Py_Initialize = sym;
   

    if ( (sym = dlsym(handle, "PyList_Append")) == NULL) {
        fprintf(stderr, "Symbol not found: PyList_Append\n");
        return -1;
    }
    fprintf(stderr, "%s:%d:Attached PyList_Append.\n", __FILE__, __LINE__);
    PyList_Append = sym;

    if ( (sym = dlsym(handle, "PyString_FromString")) == NULL) {
        fprintf(stderr, "Symbol not found: PyString_FromString\n");
        return -1;
    }
    fprintf(stderr, "%s:%d:Attached PyString_FromString.\n", __FILE__, __LINE__);
    PyString_FromString = sym;

    if ( (sym = dlsym(handle, "PySys_GetObject")) == NULL) {
        fprintf(stderr, "Symbol not found: PySys_GetObject\n");
        return -1;
    }
    fprintf(stderr, "%s:%d:Attached PySys_GetObject.\n", __FILE__, __LINE__);
    PySys_GetObject = sym;

    if ( (sym = dlsym(handle, "PyObject_GetAttrString")) == NULL) {
        fprintf(stderr, "Symbol not found: PyObject_GetAttrString\n");
        return -1;
    }
    fprintf(stderr, "%s:%d:Attached PyObject_GetAttrString.\n", __FILE__, __LINE__);
    PyObject_GetAttrString = sym;

    if ( (sym = dlsym(handle, "PyImport_ImportModule")) == NULL) {
        fprintf(stderr, "Symbol not found: PyImport_ImportModule\n");
        return -1;
    }
    fprintf(stderr, "%s:%d:Attached PyImport_ImportModule.\n", __FILE__, __LINE__);
    PyImport_ImportModule = sym;


    if ( (sym = dlsym(handle, "PyCallable_Check")) == NULL) {
        fprintf(stderr, "Symbol not found: PyCallable_Check\n");
        return -1;
    }
    fprintf(stderr, "%s:%d:Attached PyCallable_Check.\n", __FILE__, __LINE__);
    PyCallable_Check = sym;

    if ( (sym = dlsym(handle, "PyTuple_New")) == NULL) {
        fprintf(stderr, "Symbol not found: PyTuple_New\n");
        return -1;
    }
    fprintf(stderr, "%s:%d:Attached PyTuple_New.\n", __FILE__, __LINE__);
    PyTuple_New = sym;

    if ( (sym = dlsym(handle, "PyObject_CallObject")) == NULL) {
        fprintf(stderr, "Symbol not found: PyObject_CallObject\n");
        return -1;
    }
    fprintf(stderr, "%s:%d:Attached PyObject_CallObject.\n", __FILE__, __LINE__);
    PyObject_CallObject = sym;

    if ( (sym = dlsym(handle, "PyString_AsString")) == NULL) {
        fprintf(stderr, "Symbol not found: PyString_AsString\n");
        return -1;
    }
    fprintf(stderr, "%s:%d:Attached PyString_AsString.\n", __FILE__, __LINE__);
    PyString_AsString = sym;

    Py_Initialize();
    
    getcwd(pwd, 1024);
    sprintf(buf, "%s/%s", pwd, PLUGIN_DIR);
    set_pymodule_path(buf);
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

    int n = manager_find("example");
    printf("%d\n", n);
    return 0;
}

