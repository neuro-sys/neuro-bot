#include "log.h"
#include <glib.h>

static void _dummy(const gchar *log_domain,
        GLogLevelFlags log_level,
        const gchar *message,
        gpointer user_data )

{
    /* Dummy does nothing */ 
    return ;      
}

void log_init(GLogLevelFlags level)
{
    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MASK, _dummy, NULL);

    g_log_set_handler(G_LOG_DOMAIN, level, g_log_default_handler, NULL);
}
