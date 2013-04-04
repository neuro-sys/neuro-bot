#ifndef __GLOBAL_H
#define __GLOBAL_H

#define USE_PYTHON_MODULES

#ifdef _WIN32

  #pragma warning(disable : 4996)

  #define __func__ __FUNCTION__
  
  #define snprintf _snprintf
  #define getcwd _getcwd
  
#endif

#endif
