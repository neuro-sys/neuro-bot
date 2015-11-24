#include "dpl.h"

#include <stdio.h>

#include <windows.h>

static HMODULE module;

int get_symbol_address(char * sym_name)
{
    char msg_buf[1024];

    snprintf(msg_buf, sizeof (msg_buf), "Getting address of symbol: %s", sym_name);
    fprintf(stdout, "%s", msg_buf);

    if (!(Py_Initialize = (void *) GetProcAddress(module, sym_name))) {
        fprintf(stdout, " [FAIL]\n");
        return -1;
    }
    fprintf(stdout, " [OK]\n");

    return 0;
}

int load_dynamic_library(char * lib_name)
{
    char msg_buf[1024];

    snprintf(msg_buf, sizeof (msg_buf), "Loading dynamic shared library: %s", lib_name);
    fprintf(stdout, "%s", msg_buf);

    if (!(module = LoadLibrary(lib_name))) {
        fprintf(stdout, " [FAIL]\n");
        return -1;
    };
    fprintf(stdout, " [OK]\n");

    return 0;
}

int dpl_load_python(void)
{
    return
        (load_dynamic_library("python27.dll")) ||
        (get_symbol_address("Py_Initialize")) ||
        (get_symbol_address("PySys_GetObject")) ||
        (get_symbol_address("PyString_FromString")) ||
        (get_symbol_address("PyImport_ImportModule")) ||
        (get_symbol_address("PyObject_GetAttrString")) ||
        (get_symbol_address("PyTuple_SetItem")) ||
        (get_symbol_address("PyCallable_Check")) ||
        (get_symbol_address("PyTuple_New")) ||
        (get_symbol_address("PyObject_CallObject")) ||
        (get_symbol_address("PyString_AsString"))
    ;
}
