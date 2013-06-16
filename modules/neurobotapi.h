#ifndef __NEUROBOTAPI_H
#define __NEUROBOTAPI_H

#define MAX_IRC_MSG 510
#define MAX_KEYWORDS 10

#include "../irc_parser.h"
#include "../irc.h"

char * keywords[MAX_KEYWORDS];
int looper;
void (*irc_send) (struct socket_t * socket, char * message);

#endif
