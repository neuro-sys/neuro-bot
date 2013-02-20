#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "log.h"

#if defined ( WIN32 )
  #define __func__ __FUNCTION__
#endif

#if defined (__APPLE__) || defined (__MACH__)
  #define unix 1
#endif

#endif
