#include <python2.6/Python.h>
#include <glib.h>

struct py_module_t {
  PyObject * pName, * pModule, * pFunc;
};

static struct py_module_t mod_test;

char * py_call_module(char * name)
{
  PyObject * pRet;

  pRet = PyObject_CallObject(mod_test.pFunc, NULL);

  return strdup(PyString_AsString(pRet));
}

int py_load_modules()
{
  Py_Initialize();

  mod_test.pName = PyString_FromString("irc_test");
  mod_test.pModule = PyImport_Import(mod_test.pName);

  if (!mod_test.pModule) {
    g_printerr("Can't load module.\n"); 
    return -1;
  }

  mod_test.pFunc = PyObject_GetAttrString(mod_test.pModule, "test");

  if (!mod_test.pFunc || !PyCallable_Check(mod_test.pFunc)) {
    g_printerr("Error in method of module.\n");
    return -1;
  }


  return 1;
}

