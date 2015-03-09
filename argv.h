#ifndef __ARGV_H
#define __ARGV_H

struct argv_s {
    int     argc;
    char ** argv;
};

struct argv_s * argv_parse  (char * str);
void            argv_free   (struct argv_s * arg);

#endif
