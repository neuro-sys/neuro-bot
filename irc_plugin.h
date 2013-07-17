#ifndef __IRC_PLUGIN_H
#define __IRC_PLUGIN_H

#include "irc.h"

void irc_plugin_handle_command(struct irc_t * irc);
void irc_plugin_handle_grep(struct irc_t * irc);
#endif

