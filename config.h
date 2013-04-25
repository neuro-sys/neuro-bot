#ifndef __CONFIG_H
#define __CONFIG_H

#include "global.h"
#include "session.h"

void config_load(struct session_t * session);

extern char * config_channels[50];

#endif
