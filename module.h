#ifndef __MODULE_H
#define __MODULE_H

#include "irc.h"

struct mod_c_t {
    char * grep_keyword;
    void * mod;
    char * mod_name;
    void (* func)(struct irc_t * irc, char * reply_msg);
};

extern void             module_init(void);
extern struct mod_c_t * module_find(const char * cmd);
extern void             module_load(void);
extern void             module_iterate_files(void (*callback)(void * data));
extern int              py_load_modules(char * mod_dir);
#endif

