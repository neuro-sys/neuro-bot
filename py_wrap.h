#ifndef __PY_WRAP_H
#define __PY_WRAP_H

#include "irc.h"

extern GHashTable * mod_hash_map;

extern int    py_load_modules(void);
extern char * py_call_module(char * name, struct irc_t * irc);

#endif

