#ifndef __SESSION_H
#define __SESSION_H

extern struct session_t;

extern struct session_t *   session_create                  (void);
extern void                 session_destroy                 (struct session_t * session);
extern struct channel_t *   session_channel_remove_by_name  (struct session_t * session, char * name);
extern struct channel_t *   session_channel_find_by_name    (struct session_t * session, char * name);
extern void                 session_channel_add             (struct session_t * session, char * name);
extern void                 session_channel_print           (struct session_t * session);
#endif
