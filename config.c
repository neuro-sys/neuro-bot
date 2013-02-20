#include "config.h"
#include <stdlib.h>

#define CONFIG_FILE "irc-client.conf"

struct config_t {
  GKeyFile *key_file;
};

static struct config_t *config = NULL;

void config_init(void)
{
  if (!config) {
    config = (struct config_t*) calloc(1, sizeof(struct config_t));
    static const char *dirs[2];
    dirs[0] = g_get_home_dir();
    dirs[1] = g_get_current_dir();
    gchar *full_path = NULL;

    config->key_file = g_key_file_new();
    gboolean success = g_key_file_load_from_dirs (config->key_file,
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
  }

  return;
}

gchar* config_get_string(const gchar *group, const gchar *key)
{
  if (!config) {
    g_debug("config_init not called yet :/");
    return NULL;
  }

  gchar *str = g_key_file_get_string(config->key_file,
      group,
      key,
      NULL);
  return str;
}

gint config_get_integer(const gchar *group, const gchar *key)
{
  if (!config) {
    g_debug("config_init not called yet :/");
    return 0;
  }

  gint i = g_key_file_get_integer(config->key_file,
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
