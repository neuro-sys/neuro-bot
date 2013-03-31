#ifndef __CONFIG_H
#define __CONFIG_H

#include "global.h"
#include "session.h"

#include <glib.h>

#define GROUP_CLIENT "client"
#define GROUP_MODULES "modules"


#define KEY_PYPATH "pypath"

void config_load(struct session_t * session);

#endif
