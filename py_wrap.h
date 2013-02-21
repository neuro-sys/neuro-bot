#ifndef __PY_WRAP_H
#define __PY_WRAP_H

GHashTable * mod_hash_map;

extern int    py_load_modules();
extern char * py_call_module(char * name);

#endif

