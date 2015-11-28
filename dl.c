#include "dl.h"

#ifdef __WIN32__
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif // __WIN32__


dl_library_t dl_open(char * dl)
{
#ifdef __WIN32__
    return LoadLibrary(dl);
#else
    return dlopen(dl, RTLD_LAZY);
#endif // __WIN32__
}

void dl_close(dl_library_t library)
{
#ifdef __WIN32__
    FreeLibrary(library);
#else
    dlclose(library);
#endif // __WIN32__
}

void * dl_sym(dl_library_t library, char * sym)
{
#ifdef __WIN32__
    return GetProcAddress(library, sym);
#else
    return dlsym(library, sym);
#endif
}
