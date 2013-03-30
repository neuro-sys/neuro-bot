#ifndef __GLOBAL_H
#define __GLOBAL_H

#ifdef _WIN32
  #define __func__ __FUNCTION__
  #pragma warning(disable : 4996)
#define snprintf _snprintf
#endif

#endif
