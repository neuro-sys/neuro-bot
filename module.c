#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#else
#include <windows.h>
#include <direct.h>
#endif

#include "config.h"
#include "global.h"
#include "irc.h"
#include "module.h"
#include "py_wrap.h"
#include "curl_wrap.h"
#include "utils.h"

static       char       * mod_dir;

#define MOD_MAX_NUM 50
void * mod_array[MOD_MAX_NUM][2];

struct mod_c_t * module_find(const char * cmd)
{
    char t[50];
    int k;

    snprintf(t, sizeof t, "mod_%s", cmd);

    for (k = 0; mod_array[k][0] != NULL; k++)
        if (!strcmp((char *) mod_array[k][0], t))
            return (struct mod_c_t *) mod_array[k][1];

    return NULL;
}

void module_load_callback(void * data)
{
    char * file_name;
    char file_full_path[250];
    void * mod;
    void * initializer;
    void (* init_fp) (void ** fp_list);
    void * fp_list[3];
    char * t;
    struct mod_c_t * mod_c;

    int k;

    mod_c = malloc(sizeof (struct mod_c_t));

    file_name = (char *) data;

#ifndef _WIN32
    if (!strstr(file_name, ".so")) 
#else
    if (!strstr(file_name, ".dll"))
#endif
      return;

    snprintf(file_full_path, 250, "%s/%s", mod_dir, file_name);
#ifdef _WIN32
    mod = LoadLibrary(file_full_path);
#else
    mod = dlopen(file_full_path, RTLD_LAZY);
#endif
    if (!mod)
    {
        fprintf(stderr, "%s could not be opened.\n", file_name);
        return;
    }

    mod_c->mod = mod;
    t = strchr(file_name, '.');
    *t = '\0';

    mod_c->mod_name = strdup(file_name);
#ifdef _WIN32
    initializer = GetProcAddress((HMODULE) mod, file_name);
#else
    initializer = dlsym(mod, file_name);
#endif
    if (!initializer)
    {
        fprintf(stderr, "entry point %s not found in %s.\n", file_name, file_full_path);
        return;
    }

    fp_list[0] = curl_perform;
    fp_list[1] = n_strip_tags;
    fp_list[2] = n_get_tag_value;

#ifdef _WIN32
    init_fp = (void (__cdecl *)(void **))GetProcAddress((HMODULE) mod, "init");
#else
    init_fp = dlsym(mod, "init");
#endif
    init_fp(fp_list);

    mod_c->func = (char * (*)(struct irc_t *)) initializer;

    for (k = 0; mod_array[k][0] != NULL; k++) {}
    mod_array[k][0] = strdup(file_name);
    mod_array[k][1] = mod_c;

    fprintf(stderr, "Shared-lib Module loaded: [%s]\n", file_name);
}

char * module_get_loaded_names(void)
{
    char * buf;
    int k;

    buf = malloc(510);

    buf[0] = '\0';

    for (k = 0; mod_array[k][0] != NULL; k++)
    {
        strcat(buf, " [");
        strcat(buf, (char *) mod_array[k][0]);
        strcat(buf, "]");
    }

    return buf;
}

void module_unload_all(void)
{
    int k;

    for (k = 0; mod_array[k][0] != NULL; k++)
    {
        free(mod_array[k][0]);
        free(mod_array[k][1]);

        mod_array[k][0] = NULL;
        mod_array[k][1] = NULL;
    }
}

void module_iterate_files(void (*callback)(void * data))
{
#ifndef _WIN32
    DIR * dir;
    struct dirent * dirent;

    dir = opendir(mod_dir);

	if (!dir)
	{
		fprintf(stderr, "no modules found, skipping.\n");
		return;
	}
    while ( (dirent = readdir(dir)) != NULL)
    {
        callback(dirent->d_name);
    }

    closedir(dir);
#else
  HANDLE hFile;
  WIN32_FIND_DATA  findData;
  char dir_buf[1024];

  snprintf(dir_buf, 1024, "%s\\*", mod_dir);
  hFile = FindFirstFile(dir_buf, &findData);
  if (hFile == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "no modules found, skipping.\n");
		return;
	}
  do {
      callback(findData.cFileName);
  } while (FindNextFile(hFile, &findData));
#endif
}

void module_load()
{
    module_unload_all();

    module_iterate_files(&module_load_callback);
}

void module_init()
{
    char cur_dir[1024];
    char buf[1024];

    getcwd(cur_dir, 1024);

    sprintf(buf, "%s/%s", cur_dir, "modules");
    
    mod_dir = strdup(buf);

    module_load();

#ifdef USE_PYTHON_MODULES
    if ( py_load_modules(mod_dir) < 0 )
        fprintf(stderr, "Could not load python modules, going on without them.\n");
#endif
}

