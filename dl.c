#include "dl.h"

#ifdef __WIN32__
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif // __WIN32__

void * dl_open(char * dl)
{
#ifdef __WIN32__
    return LoadLibrary(dl);
#else
    return dlopen(dl, RTLD_LAZY);
#endif // __WIN32__
}

void * dl_sym(void * library, char * sym)
{
#ifdef __WIN32__
    return GetProcAddress(library, sym);
#else
    return dlsym(library, sym);
#endif
}
