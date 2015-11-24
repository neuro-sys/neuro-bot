#ifndef __DPL_H_
#define __DPL_H_

#include <stdio.h>

typedef     void *                      PyObject;
void        (*Py_Initialize)            (void);
void        (*PyList_Append)            (PyObject *, PyObject *);
PyObject    (*PySys_GetObject)          (char *);
PyObject    (*PyString_FromString)      (char *);
PyObject    (*PyImport_ImportModule)    (char *);
PyObject    (*PyObject_GetAttrString)   (PyObject *, char *);
int         (*PyTuple_SetItem)          (PyObject *, size_t pos, PyObject *);
int         (*PyCallable_Check)         (PyObject *);
PyObject    (*PyTuple_New)              (int);
PyObject    (*PyObject_CallObject)      (PyObject *, PyObject *);
char *      (*PyString_AsString)        (PyObject *);

int dpl_load_python(void);

#endif // __DPL_H_
