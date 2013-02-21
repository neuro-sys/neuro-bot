#include <python2.6/Python.h>
#include <glib.h>
#include <gio/gio.h>

static const char * mod_path = "modules";

struct py_module_t {
  PyObject * pName, * pModule, * pFunc;
};

GHashTable * hash_table;

static struct py_module_t mod_test;

char * py_call_module(char * name)
{
  PyObject * pRet;

  pRet = PyObject_CallObject(mod_test.pFunc, NULL);

  return strdup(PyString_AsString(pRet));
}

void set_pymodule_path(char * py_path)
{
  PyObject *sys_path = PySys_GetObject("path");
  PyObject *path = PyString_FromString(py_path);
  PyList_Append(sys_path, path);
}

int py_load_modules()
{
  char * cur_dir = g_get_current_dir();
  GFile * mod_path_file;
  char mod_dir[50];
  GError * error;
  GFileEnumerator * enum_children;
  GFileInfo * fileInfo;

  g_type_init();
  Py_Initialize();

  sprintf(mod_dir, "%s%c%s", cur_dir, G_DIR_SEPARATOR, mod_path);
  g_printerr("Enumerating modules... %s\n", mod_dir); 

  set_pymodule_path(mod_dir);
  
  mod_path_file = g_file_new_for_path(mod_dir);

  enum_children = g_file_enumerate_children(mod_path_file, NULL, 0, NULL, &error);
  if (!enum_children) {
    g_printerr("Can't open the specified path: %s\n", mod_dir);
    return -1;
  }

  hash_table = g_hash_table_new(g_int_hash, g_int_equal);

  while ( (fileInfo = g_file_enumerator_next_file(enum_children, NULL, NULL)) != NULL) {
    struct py_module_t * mod;
    char * file_name_temp = (g_file_info_get_attribute_as_string(fileInfo, G_FILE_ATTRIBUTE_STANDARD_NAME));
    char mod_name[50];
 
    if (!g_strrstr(file_name_temp, ".py")) 
      continue;

    strcpy(mod_name, file_name_temp);
    *strchr(mod_name, '.') = '\0';
    g_free(file_name_temp);
    g_strstrip(mod_name);

    mod = malloc(sizeof (struct py_module_t));
    
    mod->pName = PyString_FromString(mod_name);
    mod->pModule = PyImport_ImportModule(mod_name);

    g_printerr("mod name: %s\n", mod_name);
    if (!mod->pModule) {
      g_printerr("Can't load module: %s\n", mod_name);
      free(mod);
      if (PyErr_Occurred()) PyErr_Print();
      continue;
    }

    mod->pFunc = PyObject_GetAttrString(mod->pModule, mod_name);

    if (!mod->pFunc || !PyCallable_Check(mod->pFunc)) {
      g_printerr("Error in method of the module.\n");
      free(mod);
      if (PyErr_Occurred()) PyErr_Print();
      continue;
    }

    g_hash_table_insert(hash_table, strdup(mod_name), mod);
    g_printerr("Module loaded: %s\n", mod_name);
  }

  return 1;
}

