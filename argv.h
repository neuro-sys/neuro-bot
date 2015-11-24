#ifndef __ARGV_H
#define __ARGV_H

#include "queue.h"

TAILQ_HEAD(argv_list_t, argv_t);
struct argv_t {
    char * value;
    TAILQ_ENTRY(argv_t) list;
};

struct argv_s {
    int     argc;
    struct argv_list_t argv_list;
};

struct argv_s * argv_parse  (char * str);
void            argv_free   (struct argv_s * arg);

#endif
