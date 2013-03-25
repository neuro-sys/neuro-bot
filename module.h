#ifndef __MODULE_H
#define __MODULE_H

#include "irc.h"

struct module_t {
    char * mod_command;
};

struct mod_c_t {
    char * mod_name;
    char * (* func)(struct irc_t * irc);
};

extern char * module_get_dir();

extern void init_module();

struct mod_c_t * find_mod_c(char * cmd);

#endif

