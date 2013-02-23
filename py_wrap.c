#include <python2.7/Python.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <signal.h>

#include "config.h"

static const char * mod_path = "modules";

struct py_module_t {
  PyObject * pName, * pModule, * pFunc;
};

GHashTable * mod_hash_map;

static
void signal_handler(int signum)
{
  exit(signum);
}

char * py_call_module(char * name)
{
  struct py_module_t * mod   = g_hash_table_lookup(mod_hash_map, name);
  PyObject           * p_ret = PyObject_CallObject(mod->pFunc, NULL); 
  return strdup(PyString_AsString(p_ret));
}

static
void set_pymodule_path(char * py_path)
{
  PyObject * sys_path = PySys_GetObject("path");
  PyObject * path     = PyString_FromString(py_path);
  PyList_Append(sys_path, path);
}

int py_load_modules(void)
{
  char            * cur_dir = g_get_current_dir();
  char            * mod_dir;
  GFileEnumerator * enum_children;
  GFile           * mod_path_file;
  GError          * error = NULL;
  GFileInfo       * fileInfo;
  char            * modules_path;

  modules_path = config_get_string(GROUP_MODULES, KEY_PYPATH);
  if (!modules_path)
    modules_path = g_strdup(mod_path);

  signal(SIGINT, signal_handler);
  g_type_init();
  Py_Initialize();

  if (modules_path[0] == '/')
    mod_dir = g_strdup(modules_path);
  else
    mod_dir = g_strdup_printf("%s%c%s", cur_dir, G_DIR_SEPARATOR, modules_path);
  g_free(cur_dir);
  g_free(modules_path);
  g_printerr("Scanning python modules in: %s\n", mod_dir); 

  set_pymodule_path(mod_dir);
  
  mod_path_file = g_file_new_for_path(mod_dir);
  g_free(mod_dir);

  enum_children = g_file_enumerate_children(mod_path_file, NULL, 0, NULL, &error);
  if (!enum_children || error) {
    g_printerr("Can't open the specified path: %s\n", mod_dir);
    if (error) {
      g_error_free(error);
      error = NULL;
    }
    return -1;
  }

  mod_hash_map = g_hash_table_new(g_int_hash, g_int_equal);

  while ( (fileInfo = g_file_enumerator_next_file(enum_children, NULL, NULL)) != NULL) {
    struct py_module_t * mod;
    char * file_name_temp = (g_file_info_get_attribute_as_string(fileInfo, G_FILE_ATTRIBUTE_STANDARD_NAME));
    char mod_name[50];

    if (!file_name_temp) {
      continue;
    }
 
    if (!g_strrstr(file_name_temp, ".py")) 
      continue;

    strcpy(mod_name, file_name_temp);
    *strchr(mod_name, '.') = '\0';
    g_free(file_name_temp);
    g_strstrip(mod_name);

    mod = malloc(sizeof (struct py_module_t));
    
    mod->pName = PyString_FromString(mod_name);
    mod->pModule = PyImport_ImportModule(mod_name);

    if (!mod->pModule) {
      g_printerr("Can't load module: %s\n", mod_name);
      free(mod);
      if (PyErr_Occurred()) PyErr_Print();
      continue;
    }

    mod->pFunc = PyObject_GetAttrString(mod->pModule, mod_name);

    if (!mod->pFunc || !PyCallable_Check(mod->pFunc)) {
      g_printerr("Error python call method check.\n");
      free(mod);
      if (PyErr_Occurred()) PyErr_Print();
      continue;
    }

    g_hash_table_insert(mod_hash_map, strdup(mod_name), mod);
    g_printerr("Module loaded: [%s]\n", mod_name);
  }

  if (!g_file_enumerator_close(enum_children, NULL, NULL))
    g_printerr("The file handle resource cannot be freed.\n");

  return 1;
}

