#ifndef __MODULE_H
#define __MODULE_H

#include "irc.h"

struct mod_c_t {
    char * grep_keyword;
    void * mod;
    char * mod_name;
    char * (* func)(struct irc_t * irc);
};

extern char *           module_get_dir();
extern void             module_init();
extern struct mod_c_t * module_find(char * cmd);
extern void             module_load();

#endif

