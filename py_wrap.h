#ifndef __PY_WRAP_H
#define __PY_WRAP_H

#include "irc.h"

struct py_module_t;

extern int                  py_load_modules(void);
extern char *               py_call_module(struct py_module_t * mod, struct irc_t * irc);
extern struct py_module_t * find_module_from_command(char * cmd);
extern int                  py_load_mod_hash(void);

#endif
