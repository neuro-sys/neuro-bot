#ifndef __MODULE_H
#define __MODULE_H

struct module_t {
    char * mod_command;
};

extern char * module_get_dir();

extern void init_module();

#endif

