#ifdef USE_PYTHON_MODULES

#ifndef __PY_WRAP_H
#define __PY_WRAP_H

#include "irc.h"

struct py_module_t;

extern int                  py_load_modules(char *);
extern char *               py_call_module(struct py_module_t * mod, struct irc_t * irc);
extern struct py_module_t * py_find_loaded_name(char * cmd);
extern int                  py_load_mod_hash(void);

#endif

#endif

