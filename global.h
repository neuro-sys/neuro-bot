#ifndef __GLOBAL_H
#define __GLOBAL_H

/* #define USE_PYTHON_MODULES */
#define debug(args...) \
    fprintf(stderr, "%25s:%4d:%25s:", __FILE__, __LINE__, __FUNCTION__); \
    fprintf(stderr, args);

#define debug_ex(args...) fprintf(stderr, args);

#ifdef _WIN32

  #pragma warning(disable : 4996)

  #define __func__ __FUNCTION__
  
  #define snprintf _snprintf
  #define getcwd _getcwd
  #define alloca _alloca

#endif

#endif
