#ifndef __IRC_H
#define __IRC_H

#include "network.h"

extern void irc_process_line(struct network_t * network, char * line);

#endif
