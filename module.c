#include "module.h"

#include "global.h"
#include "irc.h"
#include "py_wrap.h"
#include "network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#else
#include <windows.h>
#include <direct.h>
#endif

static       char       * mod_dir;

#define MOD_MAX_NUM 50
/* mod_array { "mod_name", struct mod_c_t *, char ** keywords */
enum mod_array_t { mod_array_t_mod_name, mod_array_t_mod_c, mod_array_t_keywords, mod_array_t_NUM };
void * mod_array[MOD_MAX_NUM][mod_array_t_NUM];

struct mod_c_t * module_find(const char * cmd)
{
    char t[50];
    int k;

    snprintf(t, sizeof t, "mod_%s", cmd);

    for (k = 0; mod_array[k][0] != NULL; k++)
        if (!((struct mod_c_t *) mod_array[k][mod_array_t_mod_c])->looper
        && !strcmp((char *) mod_array[k][mod_array_t_mod_name], t))
            return (struct mod_c_t *) mod_array[k][mod_array_t_mod_c];

    return NULL;
}

struct mod_c_t ** module_get_loopers(void)
{
    struct mod_c_t ** array = NULL;
    int i, k;

    array = malloc(sizeof (struct mod_c_t **) * 10);

    for (i = 0, k = 0; mod_array[k][0] != NULL; k++) {
        struct mod_c_t * mod = (struct mod_c_t *) mod_array[k][mod_array_t_mod_c];
        if (mod->looper) {
            array[i++] = mod;
        }
    }
    if (i)
        array[i] = NULL;

    if (i == 0) {
        free(array);
        array = NULL;
    }
    return array;
}

struct mod_c_t * module_find_by_keyword(const char * line)
{
    char ** keywords;
    int i;

    for (i = 0; *mod_array[i] != NULL; i++) {
        keywords = (char **) mod_array[i][mod_array_t_keywords];

        if (*keywords == NULL)
            continue;

        while (*keywords)
            if (strstr(line, *keywords++))
                return (struct mod_c_t *) mod_array[i][mod_array_t_mod_c];
    }
               
    return NULL;
}

void module_load_callback(void * data)
{
    char * file_name;
    char file_full_path[250];
    void * mod;
    void * initializer;
    void (* init_fp) (char ***, void (*)(struct network_t *, char *), int *);
    char * t;
    struct mod_c_t * mod_c;
    char ** keywords;
    int looper;

    int k;

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

    mod_c = malloc(sizeof (struct mod_c_t));

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

#ifdef _WIN32
    init_fp = (void (__cdecl *)(char ***, void (*)(struct network_t *, char *), int *))GetProcAddress((HMODULE) mod, "init");
#else
    init_fp = dlsym(mod, "init");
#endif
    init_fp(&keywords, &network_send_message, &looper);
    if (looper)
        mod_c->looper = 1;

    mod_c->keywords = keywords;
    mod_c->func = (void (*)(struct irc_t *, char *)) initializer;

    for (k = 0; mod_array[k][mod_array_t_mod_name] != NULL; k++) {}
    mod_array[k][mod_array_t_mod_name] = strdup(file_name);
    mod_array[k][mod_array_t_mod_c]    = mod_c;
    mod_array[k][mod_array_t_keywords] = keywords;

    fprintf(stderr, "Shared-lib Module loaded: [%s]", file_name);

    if (looper)
        fprintf(stderr, " (looper)");

    if (*keywords) {
        fprintf(stderr, " with keywords: ");
        while (*keywords != NULL) {
            fprintf(stderr, "\"%s\"", *keywords++);
            if (*keywords)
                fprintf(stderr, ", ");
        }
        fprintf(stderr, ".");
    }
    fprintf(stderr, "\n");
}

void module_unload_all(void)
{
    int k;

    for (k = 0; mod_array[k][0] != NULL; k++)
    {
        free(mod_array[k][mod_array_t_mod_name]);
        free(mod_array[k][mod_array_t_mod_c]);

        mod_array[k][mod_array_t_mod_name] = NULL;
        mod_array[k][mod_array_t_mod_c] = NULL;
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

