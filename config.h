#ifndef __CONFIG_H
#define __CONFIG_H

#include "global.h"

#include "irc.h"

void config_load();

/* Defaults */
#define NICKNAME "neurobot"
#define PASSWORD ""
#define ADMIN    "neuro_sys"
#define HOST     "irc.freenode.net"
#define PORT     "6667"

#endif
