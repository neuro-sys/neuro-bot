#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "log.h"
#include "aconfig.h"

#if defined (HAVE_WINDOWS) || defined (WIN32)
  #define __func__ __FUNCTION__
  #pragma warning(disable : 4996)
#endif

#endif
