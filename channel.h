#ifndef __CHANNEL_H
#define __CHANNEL_H

#include "user.h"
#include <glib.h>
#include <stdio.h>

extern struct channel_t;

extern struct channel_t *   channel_create            (char * name);
extern const char *         channel_get_name          (const struct channel_t * channel);
extern void                 channel_destroy           (struct channel_t * channel);
extern int                  channel_cmp_by_name       (const void * c1, const void * c2);
extern void                 channels_print            (GSList * list, FILE * fd);
extern int                  channel_cmp_by_name_real  (const void * c1, const void * c2);
extern void                 channel_user_add          (struct channel_t * channel, struct user_t * user);
#endif
