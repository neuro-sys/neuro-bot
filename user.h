#ifndef __USER_H
#define __USER_H
#include <stdio.h>
#include <glib.h>

extern struct user_t;

extern void 			      user_destroy	(struct user_t * user);
extern struct user_t *  user_create		(char * name);
extern void 			      user_print		(GSList * user_list, FILE * fd);
#endif
