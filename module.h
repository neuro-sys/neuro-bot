#ifndef __MODULE_H
#define __MODULE_H

#include "irc.h"

struct mod_c_t {
    char * grep_keyword;
    void * mod;
    char * mod_name;
    char * (* func)(struct irc_t * irc);
};

extern char *           module_get_dir(void);
extern void             module_init(void);
extern struct mod_c_t * module_find(char * cmd);
extern void             module_load(void);
extern void             module_iterate_files(void (*callback)(void * data));

#endif

