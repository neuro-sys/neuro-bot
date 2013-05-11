#ifndef __CONFIG_H
#define __CONFIG_H

#include "global.h"
#include "session.h"

void config_load(struct session_t * session);

#define NICKNAME "neurobot"
#define PASSWORD ""
#define ADMIN    "neuro_sys"
#define HOST     "irc.freenode.net"
#define PORT     "6667"

#endif
