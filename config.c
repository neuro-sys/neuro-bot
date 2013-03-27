#include "config.h"
#include <stdlib.h>

#define CONFIG_FILE "neurobot.conf"

struct config_t {
    GKeyFile *key_file;
};

static struct config_t *config = NULL;

void config_init(void)
{
    if (!config) {
        static const char *dirs[2];
        gchar * full_path;
        gboolean success;

        config = (struct config_t*) calloc(1, sizeof(struct config_t));
        dirs[0] = g_get_home_dir();
        dirs[1] = g_get_current_dir();
        full_path = NULL;

        config->key_file = g_key_file_new();
        success = g_key_file_load_from_dirs (config->key_file,
                CONFIG_FILE,
                dirs,
                &full_path,
                G_KEY_FILE_NONE,
                NULL);
        if (success) {
            g_printerr("Loaded config file from: %s\n", full_path);
            g_free(full_path);

        } else {
            g_printerr("No config file, will use defaults.\n");
        }
        g_free((char*)dirs[1]);
    }

    return;
}

gchar* config_get_string(const gchar *group, const gchar *key)
{
    gchar * str;

    if (!config) {
        g_debug("config_init not called yet :/");
        return NULL;
    }

    str = g_key_file_get_string(config->key_file,
            group,
            key,
            NULL);
    return str;
}

gint config_get_integer(const gchar *group, const gchar *key)
{
    gint i;

    if (!config) {
        g_debug("config_init not called yet :/");
        return 0;
    }

    i = g_key_file_get_integer(config->key_file,
            group,
            key,
            NULL);
    return i;
}

void config_uninit(void)
{
    if (!config)
        return;

    if (config->key_file)
        g_key_file_free(config->key_file);

    free(config);
    config = NULL;
}

void config_load(struct session_t * session)
{

    session->network.host_name = config_get_string(GROUP_CLIENT, KEY_SERVER);
    if (!session->network.host_name)
        session->network.host_name = g_strdup("irc.freenode.net");

    session->network.port = config_get_integer(GROUP_CLIENT, KEY_PORT);
    if (!session->network.port)
        session->network.port = 6667;

    session->nickname = config_get_string(GROUP_CLIENT, KEY_NICK);
    if (!session->nickname)
        session->nickname = g_strdup("cafer");

    session->password = config_get_string(GROUP_CLIENT, KEY_PASS);
    if (!session->password)
        session->password = g_strdup("");

    session->admin = config_get_string(GROUP_CLIENT, KEY_ADMIN);
    if (!session->admin) {
        session->admin = g_strdup(""); /* wut? */
        g_warning("No admin in config file?");
    }
}

